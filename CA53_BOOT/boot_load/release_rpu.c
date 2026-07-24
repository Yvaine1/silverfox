// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 FMSH, Inc.
 */
#include <stdlib.h>
#include "fmsh_common.h"
#include "platform.h"
#include "release_rpu.h"



extern s32 fmsh_pm_request(u32 api_id, u32 arg0, u32 arg1, u32 arg2,
				     u32 arg3, u32 *ret_payload);

void fmzq_mmio_write(const u32 address,const u32 mask, const u32 value)
{
  fmsh_pm_request(PM_MMIO_WRITE, address, mask, value, 0, NULL);
}


static void set_r5_halt_mode(u32 nr, u8 halt, u8 mode)
{
	u32 tmp;

	if (mode == LOCK || nr == FMZQ_CORE_RPU0) {
		tmp = FMSH_ReadReg(&rpu_base->rpu0_cfg, 0);
		if (halt == HALT)
			tmp &= ~FMZQ_RPU_CFG_CPU_HALT_MASK;
		else
			tmp |= FMZQ_RPU_CFG_CPU_HALT_MASK;
		FMSH_WriteReg(&rpu_base->rpu0_cfg, 0, tmp);
	}

	if (mode == LOCK || nr == FMZQ_CORE_RPU1) {
		tmp = FMSH_ReadReg(&rpu_base->rpu1_cfg, 0);
		if (halt == HALT)
			tmp &= ~FMZQ_RPU_CFG_CPU_HALT_MASK;
		else
			tmp |= FMZQ_RPU_CFG_CPU_HALT_MASK;
		FMSH_WriteReg(&rpu_base->rpu1_cfg, 0, tmp);
	}
}

static void set_r5_tcm_mode(u8 mode)
{
	u32 tmp;

	tmp = FMSH_ReadReg(&rpu_base->rpu_glbl_ctrl, 0);

        tmp |= FMZQ_RPU_GLBL_CTRL_SPLIT_LOCK_MASK;
        tmp &= ~(FMZQ_RPU_GLBL_CTRL_TCM_COMB_MASK |
               FMZQ_RPU_GLBL_CTRL_SLCLAMP_MASK);

	FMSH_WriteReg(&rpu_base->rpu_glbl_ctrl, 0 , tmp);
}

static void set_r5_reset(u32 nr, u8 mode)
{
	u32 tmp;

	tmp = FMSH_ReadReg(&crlapb_base->rst_lpd_top, 0);
	if (mode == LOCK) {
		tmp |= (FMZQ_CRLAPB_RST_LPD_R50_RST_MASK |
			FMZQ_CRLAPB_RST_LPD_R51_RST_MASK);
	} else {
		if (nr == FMZQ_CORE_RPU0) {
			tmp |= FMZQ_CRLAPB_RST_LPD_R50_RST_MASK;
		} else {
			tmp |= FMZQ_CRLAPB_RST_LPD_R51_RST_MASK;
		}
	}

    FMSH_WriteReg(&crlapb_base->rst_lpd_top, 0 , tmp);
}

static void release_r5_reset(u32 nr, u8 mode)
{
	u32 tmp;

	tmp = FMSH_ReadReg(&crlapb_base->rst_lpd_top, 0);
	if (mode == LOCK || nr == FMZQ_CORE_RPU0)
		tmp &= ~FMZQ_CRLAPB_RST_LPD_R50_RST_MASK;

	if (mode == LOCK || nr == FMZQ_CORE_RPU1)
		tmp &= ~FMZQ_CRLAPB_RST_LPD_R51_RST_MASK;

	FMSH_WriteReg(&crlapb_base->rst_lpd_top, 0 , tmp);
}

static void enable_clock_r5(void)
{
	u32 tmp;

	tmp = FMSH_ReadReg(&crlapb_base->cpu_r5_ctrl, 0);
	tmp |= FMZQ_CRLAPB_CPU_R5_CTRL_CLKACT_MASK;
    FMSH_WriteReg(&crlapb_base->cpu_r5_ctrl, 0 , tmp);
	/* Give some delay for clock
	 * to propagate
	 */
	delay_us(0x500);
}

static int check_r5_mode(void)
{
	u32 tmp;

	tmp = FMSH_ReadReg(&rpu_base->rpu_glbl_ctrl, 0);
	if (tmp & FMZQ_RPU_GLBL_CTRL_SPLIT_LOCK_MASK)
		return SPLIT;

	return LOCK;
}

int cpu_disable(u32 nr)
{
        set_r5_reset(nr, check_r5_mode());
	return 0;
}

static void set_r5_start(u8 high)
{
	u32 tmp;

	tmp = FMSH_ReadReg(&rpu_base->rpu0_cfg, 0);
	if (high)
		tmp |= FMZQ_RPU_CFG_HIVEC_MASK;
	else
		tmp &= ~FMZQ_RPU_CFG_HIVEC_MASK;
	FMSH_WriteReg(&rpu_base->rpu0_cfg, 0, tmp);

	tmp = FMSH_ReadReg(&rpu_base->rpu1_cfg, 0);
	if (high)
		tmp |= FMZQ_RPU_CFG_HIVEC_MASK;
	else
		tmp &= ~FMZQ_RPU_CFG_HIVEC_MASK;
	FMSH_WriteReg(&rpu_base->rpu1_cfg, 0, tmp);
}

static void write_tcm_boot_trampoline(u32 nr, u32 boot_addr)
{
	if (boot_addr) {
		u64 tcm_start_addr = FMZQ_R5_0_TCM_START_ADDR;

		if (nr == FMZQ_CORE_RPU1)
			tcm_start_addr = FMZQ_R5_1_TCM_START_ADDR;

		/*
		 * Boot trampoline is simple ASM code below.
		 *
		 *		b over;
		 *	label:
		 *	.word	0
		 *	over:
	     *   	; set SCTLR.V =1
		 *		mrc     p15, 0, r0, c1, c0, 0
		 *		mov     r1, #0x2000
		 *		orr     r0, r0, r1
		 *		mcr     p15, 0, r0, c1, c0, 0
		 *
		 *		ldr	r0, =label
		 *		ldr	r1, [r0]
		 *		bx	r1
		 */
		fmsh_print("Write boot trampoline for %x\r\n", boot_addr);
		FMSH_WriteReg(tcm_start_addr, 0 ,0xea000000);

        FMSH_WriteReg(tcm_start_addr, 0x4 ,boot_addr);
        FMSH_WriteReg(tcm_start_addr, 0x8 ,0xee110f10);
        FMSH_WriteReg(tcm_start_addr, 0xc ,0xe3a01d80);
        FMSH_WriteReg(tcm_start_addr, 0x10 ,0xe1800001);
        FMSH_WriteReg(tcm_start_addr, 0x14 ,0xee010f10);
        FMSH_WriteReg(tcm_start_addr, 0x18 ,0xe59f0004);
        FMSH_WriteReg(tcm_start_addr, 0x1c ,0xe5901000);
        FMSH_WriteReg(tcm_start_addr, 0x20 ,0xe12fff11);
        FMSH_WriteReg(tcm_start_addr, 0x24 ,0x00000004);
	}
}

void initialize_tcm(BOOL mode)
{
	if (!mode) {
		set_r5_tcm_mode(LOCK);
		set_r5_halt_mode(FMZQ_CORE_RPU0, HALT, LOCK);
		enable_clock_r5();
		release_r5_reset(FMZQ_CORE_RPU0, LOCK);
	} else {
		set_r5_tcm_mode(SPLIT);
		set_r5_halt_mode(FMZQ_CORE_RPU0, HALT, SPLIT);
		set_r5_halt_mode(FMZQ_CORE_RPU1, HALT, SPLIT);
		enable_clock_r5();
		release_r5_reset(FMZQ_CORE_RPU0, SPLIT);
		release_r5_reset(FMZQ_CORE_RPU1, SPLIT);
	}
}

static void mark_r5_used(u32 nr, u8 mode)
{
	u32 mask = 0;

	switch (nr) {
		case FMZQ_CORE_RPU0:
			mask = FMZQ_RPU0_USE_MASK;
			break;
		case FMZQ_CORE_RPU1:
			mask = FMZQ_RPU1_USE_MASK;
			break;
		default:
			return;
	}
	
	fmzq_mmio_write((ULONG)&pmu_base->gen_storage4, mask, mask);
}

#define FSBL_R5_0_STATUS_MASK    (1U << 1)
#define FSBL_R5_1_STATUS_MASK    (1U << 2)
void MarkUsedRPUCores (u32 nr)
{
    u32 RegValue;
    RegValue = FMSH_ReadReg((ULONG)&pmu_base->gen_storage4, 0);
    if (FMZQ_CORE_RPU0 == nr)
    {
      FMSH_WriteReg((ULONG)&pmu_base->gen_storage4, 0, RegValue | FSBL_R5_0_STATUS_MASK);
    } 
    else 
    {
      FMSH_WriteReg((ULONG)&pmu_base->gen_storage4, 0, RegValue | FSBL_R5_1_STATUS_MASK);
    }
    
}

void initialize_rpu_tcm(u32 nr)
{
    u32 atcm_load_addr;
    u32 btcm_load_addr;

    switch (nr) 
    {
        case FMZQ_CORE_RPU0:
            atcm_load_addr = FMZQ_R5_0_TCM_START_ADDR;
            btcm_load_addr = FMZQ_R5_0_BTCM_START_ADDR;
            break;
        case FMZQ_CORE_RPU1:
            atcm_load_addr = FMZQ_R5_1_TCM_START_ADDR;
            btcm_load_addr = FMZQ_R5_1_BTCM_START_ADDR;
            break;
        default:
            fmsh_print("nr is %d !\r\n", nr);
            return;
    }

    set_r5_reset(nr, SPLIT);
    set_r5_tcm_mode(SPLIT);
    set_r5_halt_mode(nr, HALT, SPLIT);

    enable_clock_r5();
    release_r5_reset(nr, SPLIT);

    for(int i = 0; i < RPU_TCM_SIZE/4; i++)
    {
        FMSH_WriteReg(atcm_load_addr, 4*i , 0x0);
        FMSH_WriteReg(btcm_load_addr, 4*i , 0x0);
    }

}

int cpu_release(u32 nr, u32 boot_addr)
{
    u32 boot_addr_uniq = 0;
    if (!(boot_addr == FMZQ_R5_LOVEC_ADDR ||
            boot_addr == FMZQ_R5_HIVEC_ADDR)) {
        fmsh_print("Using TCM jump trampoline for address 0x%x\r\n",
                boot_addr);
        /* Save boot address for later usage */
        boot_addr_uniq = boot_addr;
        /*
            * R5 needs to start from LOVEC at TCM
            * OCM will be probably occupied by ATF
            */
        boot_addr = FMZQ_R5_LOVEC_ADDR;
    }

    /*
        * Since we don't know where the user may have loaded the image
        * for an R5 we have to flush all the data cache to ensure
        * the R5 sees it.
        */
    Fmsh_DCacheFlush();

    fmsh_print("R5 split mode\r\n");
	set_r5_start(boot_addr);
    Fmsh_DCacheDisable();
    //write_tcm_boot_trampoline(nr, boot_addr_uniq);
    Fmsh_DCacheEnable();
    set_r5_halt_mode(nr, RELEASE, SPLIT);
 //   mark_r5_used(nr, SPLIT);
    MarkUsedRPUCores (nr);

    return 0;
}
