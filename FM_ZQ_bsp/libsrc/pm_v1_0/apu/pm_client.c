/******************************************************************************
*
* Copyright (C) 2015 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/

/*
 * CONTENT
 * Each PU client in the system have such file with definitions of
 * masters in the subsystem and functions for getting informations
 * about the master.
 */

#include "pm_client.h"
#include "armv8/fmsh_cache.h"
#include "armv8/cortexa53.h"

/* Mask to get affinity level 0 */
#define PM_AFL0_MASK   0xFF

static struct FPm_Master pm_apu_0_master = {
	.node_id = NODE_APU_0,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_0_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

static struct FPm_Master pm_apu_1_master = {
	.node_id = NODE_APU_1,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_1_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

static struct FPm_Master pm_apu_2_master = {
	.node_id = NODE_APU_2,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_2_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

static struct FPm_Master pm_apu_3_master = {
	.node_id = NODE_APU_3,
	.pwrctl = APU_PWRCTL,
	.pwrdn_mask = APU_3_PWRCTL_CPUPWRDWNREQ_MASK,
	.ipi = NULL,
};

/* Order in pm_master_all array must match cpu ids */
static struct FPm_Master *const pm_masters_all[] = {
	&pm_apu_0_master,
	&pm_apu_1_master,
	&pm_apu_2_master,
	&pm_apu_3_master,
};

/**
 * pm_get_master() - returns pointer to the master structure
 * @cpuid:	id of the cpu whose master struct pointer should be returned
 *
 * Return: pointer to a master structure if master is found, otherwise NULL
 */
struct FPm_Master *pm_get_master(const u32 cpuid)
{
	struct FPm_Master *master = NULL;
	if (cpuid < PM_ARRAY_SIZE(pm_masters_all)) {
		master = pm_masters_all[cpuid];
		goto done;
	}
done:
	return master;
}

/**
 * pm_get_master_by_node() - returns pointer to the master structure
 * @nid:	ndoe id of the cpu master
 *
 * Return: pointer to a master structure if master is found, otherwise NULL
 */
struct FPm_Master *pm_get_master_by_node(const enum FPmNodeId nid)
{
	u8 i;
	struct FPm_Master *master = NULL;

	for (i = 0U; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (nid == pm_masters_all[i]->node_id) {
			master = pm_masters_all[i];
			goto done;
		}
	}

done:
	return master;
}

static u32 pm_get_cpuid(const enum FPmNodeId node)
{
	u32 i;
	u32 ret;

	for (i = 0U; i < PM_ARRAY_SIZE(pm_masters_all); i++) {
		if (pm_masters_all[i]->node_id == node) {
			ret = i;
			goto done;
		}
	}

	ret = UNDEFINED_CPUID;

done:
	return ret;
}

const enum FPmNodeId subsystem_node = NODE_APU;
struct FPm_Master *primary_master = &pm_apu_0_master;

void FPm_ClientSuspend(const struct FPm_Master *const master)
{
	u32 pwrdn_req;

	/* Disable interrupts at processor level */
	pm_disable_int();
	/* Set powerdown request */
	if (NULL != master) {
		pwrdn_req = pm_read(master->pwrctl);
		pwrdn_req |= master->pwrdn_mask;
		pm_write(master->pwrctl, pwrdn_req);
	}
}

void FPm_ClientAbortSuspend(void)
{
	u32 pwrdn_req;
	if (NULL != primary_master) {
		pwrdn_req = pm_read(primary_master->pwrctl);

		/* Clear powerdown request */
		pwrdn_req &= ~primary_master->pwrdn_mask;
		pm_write(primary_master->pwrctl, pwrdn_req);
		/* Enable interrupts at processor level */
		pm_enable_int();
	}
}

void FPm_ClientWakeup(const struct FPm_Master *const master)
{
	u32 cpuid = pm_get_cpuid(master->node_id);

	if (UNDEFINED_CPUID != cpuid) {
		u32 val = pm_read(master->pwrctl);
		val &= ~(master->pwrdn_mask);
		pm_write(master->pwrctl, val);
	}
}

/**
 * FPm_ClientSuspendFinalize() - Finalize suspend procedure by executing
 * 				 wfi instruction
 */
void FPm_ClientSuspendFinalize(void)
{

	/* Flush the data cache only if it is enabled */
#ifdef __aarch64__
        u64 ctrlReg;
	mfcp(SCTLR_EL3, ctrlReg);
	if ((CONTROL_DCACHE_BIT & ctrlReg) != 0U) {
		Fmsh_DCacheFlush();
	}
#else
        u32 ctrlReg;
	ctrlReg = mfcp(XREG_CP15_SYS_CONTROL);
	if ((XREG_CP15_CONTROL_C_BIT & ctrlReg) != 0U) {
		Fmsh_DCacheFlush();
	}
#endif

	pm_dbg("Going to WFI...\n\r");
	asm("wfi");
	pm_dbg("WFI exit...\n\r");
}

/**
 *  FPm_ClientSetPrimaryMaster() - Set primary master based on master ID
 */
void FPm_ClientSetPrimaryMaster(void)
{
	
#ifdef __aarch64__
        u64 master_id;
	mfcp(MPIDR_EL1, master_id);
#else
	u32 master_id;
	master_id = mfcp(CP15_MULTI_PROC_AFFINITY);
#endif

	master_id &= PM_AFL0_MASK;
	primary_master = pm_masters_all[master_id];
}
