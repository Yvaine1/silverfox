/******************************************************************************
*
* Copyright (C) 2015-2019 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/

/*****************************************************************************/
/**
 * @file pm_common.h
 *
 * Definitions of commonly used macros and data types needed for
 * PU Power Management. This file should be common for all PU's.
 *****************************************************************************/

#ifndef PM_COMMON_H
#define PM_COMMON_H

#include "fmsh_common_types.h"
#include "exception_handler.h"
#include "pm_status.h"
#include "fmsh_ipi.h"
#include "pm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PM_ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))

/* 1 for API ID + 5 for API arguments + 1 for Reserved + 1 for CRC */
#define PAYLOAD_ARG_CNT		8U

/* 1 for status + 3 for values + 3 for Reserved + 1 for CRC */
#define RESPONSE_ARG_CNT	8U

#define PM_IPI_TIMEOUT		(~0U)

#define IPI_PMU_PM_INT_MASK	PAR_PSU_IPI_3_BIT_MASK

/**
 * FPm_Master - Master structure
 */
struct FPm_Master {
	const enum FPmNodeId node_id; /**< Node ID */
	const u32 pwrctl;             /** < Power Control Register Address */
	const u32 pwrdn_mask;         /**< Power Down Mask */
	IpiPsu *ipi;                 /**< IPI Instance */
};

enum FPmNodeId pm_get_subsystem_node(void);
struct FPm_Master *pm_get_master(const u32 cpuid);
struct FPm_Master *pm_get_master_by_node(const enum FPmNodeId nid);

#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK	0x00000001U
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK	0x00000002U
#define APU_2_PWRCTL_CPUPWRDWNREQ_MASK	0x00000004U
#define APU_3_PWRCTL_CPUPWRDWNREQ_MASK	0x00000008U

#define IPI_W0_TO_W6_SIZE		28U
#define IPI_RPU_MASK			0x00000100U

#define UNDEFINED_CPUID		(~0U)

#define pm_read(p)         (uint32_t) * ((uint32_t *)(p))
#define pm_write(p, v)     *((volatile uint32_t *)(p)) = (v)
#define pm_enable_int()		FMSH_ExceptionEnable()
#define pm_disable_int()	FMSH_ExceptionDisable()

/* Conditional debugging prints */
#ifdef DEBUG_MODE
#define pm_dbg fmsh_print
#else
	#define pm_dbg(...)	{}
#endif

#ifndef bool
	#define bool	u8
	#define true	1U
	#define false	0U
#endif

void FPm_ClientSuspend(const struct FPm_Master *const master);
void FPm_ClientAbortSuspend(void);
void FPm_ClientWakeup(const struct FPm_Master *const master);
void FPm_ClientSuspendFinalize(void);
void FPm_ClientSetPrimaryMaster(void);

/* Do not modify below this line */
extern const enum FPmNodeId subsystem_node;
extern struct FPm_Master *primary_master;

#ifdef __cplusplus
}
#endif

#endif /* PM_COMMON_H */
