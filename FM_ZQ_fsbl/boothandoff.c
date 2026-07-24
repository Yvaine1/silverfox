/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  boothandoff.c
 *
 * This file contains boot_main.h.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/28/2022  First Release
 * 0.02   lq  08/16/2023  Fix BootHandoff function for multicore startup.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"
#include "pm_api_sys.h"
#include "pm_cfg_obj.h"

/************************** Constant Definitions *****************************/
#define FSBL_CPU_POWER_UP (0x1U)
#define FSBL_CPU_SWRST    (0x2U)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern u8 SecureFlag;
extern BootPs BootInstance;

u32 Pm_ConfigObject[100]={0U};
#ifdef ARMR5
/* Variables defined in xfsbl_partition_load.c */
extern u8 R5LovecBuffer[32];
extern u32 TcmSkipLength;
extern UINTPTR TcmSkipAddress;
#endif

#define DECFG_BASEADDR    0xF8007000
#define ROM_SHADOW_OFFSET 0x28

#define DECFG_CTRL_OFFSET 0x0

/************************** Function Prototypes ******************************/
extern void FMSH_EXIT(UINTPTR HandOffAddr, u32 Flag);
extern unsigned long psu_ps_pl_isolation_removal_data(void);
extern unsigned long psu_ps_pl_reset_config_data(void);
extern int psu_protection_lock(void);

void HandoffJtagExit (void)
{
    Fmsh_ICacheInvalidate();
    Fmsh_ICacheDisable();
    asm("LOOP:\n\t"
        "wfe\n\t"
        "b LOOP");
}

/******************************************************************************
 *
 * This function is used to notify PMU firmware (if present) that initialization
 * of all PM related register is completed
 *
 * @param	None
 *
 * @return	Success or FSBL_ERROR_PM_INIT in case of any error
 *
 * @note		None
 *
 *******************************************************************************/
u32 FmshFsbl_PmInit (void)
{
    u32 Status = FMSH_SUCCESS;

#ifdef IPIPS_0_DEVICE_ID
    IpiPsu_Config *Config = NULL;
    IpiPsu IpiInstance;
#if 1    
#ifdef __aarch64__
    u32 CfgCmd = (u32)((u64)&FPm_ConfigObject[0]);
#else
    u32 CfgCmd = (u32)&FPm_ConfigObject[0];
#endif
#endif
#endif
    /**
     * Check if PMU FW is present
     * If PMU FW is present, but IPI device does not exist, report an error
     * If IPI device exists, but PMU FW is not present, do not issue IPI
     */
    if ((ReadReg(PMU_GLOBAL_GLOBAL_CNTRL) &
         PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) !=
        PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "PMU-FW is not running, certain applications may not be "
                     "supported.\n\r");
        Status = FMSH_SUCCESS;
        return Status;
    }
#ifndef IPIPS_0_DEVICE_ID
	else {
		Status = FSBL_ERROR_PM_INIT;
		 UART_LOG_OUT(DEBUG_INFO,
			"PMU firmware is present, but IPI is disabled\r\n");
		return Status;
	}
#endif
#ifdef IPIPS_0_DEVICE_ID
	Config = IpiPsu_LookupConfig(IPIPS_0_DEVICE_ID);
	if (Config == NULL) {
		Status = FSBL_ERROR_PM_INIT;
		return Status;
	}

	Status = IpiPsu_CfgInitialize(&IpiInstance, Config,
			Config->BaseAddress);
	if (FMSH_SUCCESS != Status) {
		Status = FSBL_ERROR_PM_INIT;
		return Status;
	}
        Status = FPm_InitFmshpm(&IpiInstance);
        if (FMSH_SUCCESS != Status) {
		Status = FSBL_ERROR_PM_INIT;
		return Status;
	}
//TODO
#if 1
        Status = FPm_SetConfiguration(CfgCmd);
	if (FMSH_SUCCESS != Status) {
		Status = FSBL_ERROR_PM_INIT;
		return Status;
	}
#endif
#endif 
    UART_LOG_OUT(DEBUG_INFO, "PM Init Success\r\n");
    Status = FMSH_SUCCESS;

    return Status;
}

/*****************************************************************************
 *
 * This function checks if a given CPU is supported.
 *
 * @param	none
 *
 * @return	FMSH_SUCCESS if supported CPU, FMSH_FAILURE if not.
 *
 ******************************************************************************/
static u32 FmshFsbl_CheckSupportedCpu (u32 CpuId)
{
    u32 Status = FMSH_SUCCESS;

    if ((CpuId != IH_PH_ATTRB_DEST_CPU_A53_0) &&
        (CpuId != IH_PH_ATTRB_DEST_CPU_A53_1) &&
        (CpuId != IH_PH_ATTRB_DEST_CPU_A53_2) &&
        (CpuId != IH_PH_ATTRB_DEST_CPU_A53_3))
    {
        if ((CpuId != IH_PH_ATTRB_DEST_CPU_R5_0) &&
            (CpuId != IH_PH_ATTRB_DEST_CPU_R5_1) &&
            (CpuId != IH_PH_ATTRB_DEST_CPU_R5_L) &&
            (CpuId != IH_PH_ATTRB_DEST_CPU_PMU))
        {
            Status = FMSH_FAILURE;
            return Status;
        }
    }

    /* Add code to check for support of other CPUs/cores in future */

    return Status;
}

/*****************************************************************************/
/**
 * This function determines if the given CPU needs early handoff or not.
 * Currently early handoff is provided for R5
 *
 * @param	CpuId is Mask of CPU Id in partition attributes
 *
 * @return	TRUE if this CPU needs early handoff, and FALSE if not
 *
 *****************************************************************************/
static u32 FmshFsbl_CheckEarlyHandoffCpu (u32 CpuId)
{
    u32 CpuNeedEarlyHandoff = FALSE;
#if defined(FSBL_EARLY_HANDOFF)
    if ((CpuId == IH_PH_ATTRB_DEST_CPU_R5_0) ||
        (CpuId == IH_PH_ATTRB_DEST_CPU_R5_1) ||
        (CpuId == IH_PH_ATTRB_DEST_CPU_R5_L))
    {
        CpuNeedEarlyHandoff = TRUE;
    }
#endif
    return CpuNeedEarlyHandoff;
}
/*****************************************************************************
 *
 * FSBL exit function before the assembly code
 *
 * @param HandoffAddress is handoff address for the FSBL running cpu
 *
 * @param Flag is to determine whether to handoff to application or
 * 			to be in wfe state
 *
 * @return None
 *
 *****************************************************************************/
void FmshFsbl_HandoffExit (UINTPTR HandoffAddress, u32 Flag)
{
    u32 RegVal = 0;

    /*
     * Write 1U to PMU GLOBAL general storage register 5 to indicate
     * PMU Firmware that FSBL completed execution
     */
    RegVal = ReadReg(PMU_GLOBAL_GLOB_GEN_STORAGE5);
    RegVal &= ~(FSBL_EXEC_COMPLETED);
    RegVal |= FSBL_EXEC_COMPLETED;
    WriteReg(PMU_GLOBAL_GLOB_GEN_STORAGE5, RegVal);

    UART_LOG_OUT(DEBUG_INFO, "Exit from FSBL \n\r");

    /**
     * Exit to handoff address
     * PTRSIZE is used since handoff is in same running cpu
     * and address is of PTRSIZE
     */
    FMSH_EXIT(HandoffAddress, Flag);

    /**
     * should not reach here
     */
    return;
}
/****************************************************************************
 *
 * This function is used to update reset vector.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 *
 *****************************************************************************/
static void FmshFsbl_UpdateResetVector (u64 HandOffAddress, u32 CpuSettings,
                                        u32 Vector)
{
    u32 HandOffAddressLow = 0U;
    u32 HandOffAddressHigh = 0U;
    u32 LowAddressReg = 0U;
    u32 HighAddressReg = 0U;
    u32 CpuId = 0U;
    u32 RegVal = 0U;
    u32 ExecState = 0U;

    CpuId = CpuSettings & IH_PH_ATTRB_DEST_CPU_MASK;
    ExecState = CpuSettings & IH_PH_ATTRB_A53_EXEC_ST_MASK;

    /**
     * Put R5 or A53-32 in Lovec/Hivec
     */
    if ((CpuId == IH_PH_ATTRB_DEST_CPU_R5_0) ||
        (CpuId == IH_PH_ATTRB_DEST_CPU_R5_L))
    {
        RegVal = ReadReg(CRL_APB_RST_LPD_TOP);
        RegVal |= (CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
        WriteReg(CRL_APB_RST_LPD_TOP, RegVal);
        
        RegVal = ReadReg(RPU_RPU_0_CFG);
        RegVal &= ~RPU_RPU_0_CFG_VINITHI_MASK;
        RegVal |= (Vector << RPU_RPU_0_CFG_VINITHI_SHIFT);
        WriteReg(RPU_RPU_0_CFG, RegVal);
    }

    else if ((CpuId == IH_PH_ATTRB_DEST_CPU_R5_1) ||
             (CpuId == IH_PH_ATTRB_DEST_CPU_R5_L))
    {
        RegVal = ReadReg(CRL_APB_RST_LPD_TOP);
        RegVal |= (CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
        WriteReg(CRL_APB_RST_LPD_TOP, RegVal);
        
        RegVal = ReadReg(RPU_RPU_1_CFG);
        RegVal &= ~RPU_RPU_1_CFG_VINITHI_MASK;
        RegVal |= (Vector << RPU_RPU_1_CFG_VINITHI_SHIFT);
        WriteReg(RPU_RPU_1_CFG, RegVal);
    }

    else if ((CpuId == IH_PH_ATTRB_DEST_CPU_A53_0) &&
             (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32))
    {
        RegVal = ReadReg(APU_CONFIG_0);
        RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU0;
        RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU0);
        WriteReg(APU_CONFIG_0, RegVal);
    }

    else if ((CpuId == IH_PH_ATTRB_DEST_CPU_A53_1) &&
             (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32))
    {
        RegVal = ReadReg(APU_CONFIG_0);
        RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU1;
        RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU1);
        WriteReg(APU_CONFIG_0, RegVal);
    }

    else if ((CpuId == IH_PH_ATTRB_DEST_CPU_A53_2) &&
             (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32))
    {
        RegVal = ReadReg(APU_CONFIG_0);
        RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU2;
        RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU2);
        WriteReg(APU_CONFIG_0, RegVal);
    }

    else if ((CpuId == IH_PH_ATTRB_DEST_CPU_A53_3) &&
             (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32))
    {
        RegVal = ReadReg(APU_CONFIG_0);
        RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU3;
        RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU3);
        WriteReg(APU_CONFIG_0, RegVal);
    }

    else
    {
        /* for MISRA C compliance */
    }

    if ((CpuId == IH_PH_ATTRB_DEST_CPU_R5_0) ||
        (CpuId == IH_PH_ATTRB_DEST_CPU_R5_1) ||
        (CpuId == IH_PH_ATTRB_DEST_CPU_R5_L))
    {
        if (Vector == 0U)
        {
          if(CpuId == IH_PH_ATTRB_DEST_CPU_R5_1)
          {
            FmshFsbl_PowerUpMemory(FSBL_R5_1_TCM);
            if( HandOffAddress < (4 * FSBL_R5_TCM_BANK_LENGTH) )
            {
                (void)memcpy((void*)(FSBL_R50_HIGH_BTCM_START_ADDRESS),(void*)(FSBL_R50_HIGH_BTCM_START_ADDRESS+HandOffAddress),FSBL_IVT_LENGTH);
            }
            else
            {
                (void)memcpy((void*)(FSBL_R50_HIGH_BTCM_START_ADDRESS),(void*)HandOffAddress,FSBL_IVT_LENGTH);
            }
          }
          else
          {
            FmshFsbl_PowerUpMemory(CpuId == IH_PH_ATTRB_DEST_CPU_R5_0?FSBL_R5_0_TCM:FSBL_R5_L_TCM);
            if( HandOffAddress < (4 * FSBL_R5_TCM_BANK_LENGTH) )
            {
                (void)memcpy((void*)(FSBL_R50_HIGH_ATCM_START_ADDRESS),(void*)(FSBL_R50_HIGH_ATCM_START_ADDRESS+HandOffAddress),FSBL_IVT_LENGTH);
            }
            else
            {
                (void)memcpy((void*)(FSBL_R50_HIGH_ATCM_START_ADDRESS),(void*)HandOffAddress,FSBL_IVT_LENGTH);
            }
          }
        }
        else
        {
            (void)memcpy((void *)FSBL_R5_HIVEC, (void *)(u32)HandOffAddress,
                   FSBL_IVT_LENGTH);
        }
    }
    if ((CpuId != IH_PH_ATTRB_DEST_CPU_R5_0) &&
        (CpuId != IH_PH_ATTRB_DEST_CPU_R5_1) &&
        (CpuId != IH_PH_ATTRB_DEST_CPU_R5_L) &&
        (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA64))
    {
        /**
         * for A53 cpu, write 64bit handoff address
         * to the RVBARADDR in APU
         */

        HandOffAddressLow = (u32)(HandOffAddress & 0xFFFFFFFFU);
        HandOffAddressHigh = (u32)((HandOffAddress >> 32) & 0xFFFFFFFFU);
        switch (CpuId)
        {
        case IH_PH_ATTRB_DEST_CPU_A53_0:
            LowAddressReg = APU_RVBARADDR0L;
            HighAddressReg = APU_RVBARADDR0H;
            break;
        case IH_PH_ATTRB_DEST_CPU_A53_1:
            LowAddressReg = APU_RVBARADDR1L;
            HighAddressReg = APU_RVBARADDR1H;
            break;
        case IH_PH_ATTRB_DEST_CPU_A53_2:
            LowAddressReg = APU_RVBARADDR2L;
            HighAddressReg = APU_RVBARADDR2H;
            break;
        case IH_PH_ATTRB_DEST_CPU_A53_3:
            LowAddressReg = APU_RVBARADDR3L;
            HighAddressReg = APU_RVBARADDR3H;
            break;
        default:
            /**
             * error can be triggered here
             */
            LowAddressReg = 0U;
            HighAddressReg = 0U;
            break;
        }
        WriteReg(LowAddressReg, HandOffAddressLow);
        WriteReg(HighAddressReg, HandOffAddressHigh);
    }
    return;
}
/****************************************************************************/
/**
 * This function will set up the settings for the CPU's
 * This can power up the CPU or do a soft reset to the CPU's
 *
 * @param CpuId specifies for which CPU settings should be done
 *
 * @param Flags is used to specify the settings for the CPU
 * 			FSBL_CPU_POWER_UP - This is used to power up the CPU
 * 			FSBL_CPU_SWRST - This is used to trigger the reset to CPU
 *
 * @return
 * 		- FMSH_SUCCESS on successful settings
 * 		- FMSH_FAILURE
 *
 * @note
 *
 *****************************************************************************/
static u32 FmshFsbl_SetCpuPwrSettings (u32 CpuSettings, u32 Flags)
{
    u32 RegValue = 0;
    u32 Status = 0;
    u32 CpuId = 0;
    u32 ExecState = 0;
    u32 PwrStateMask = 0;

    if ((Flags & FSBL_CPU_SWRST) != 0U)
    {
        CpuId = CpuSettings & IH_PH_ATTRB_DEST_CPU_MASK;
        ExecState = CpuSettings & IH_PH_ATTRB_A53_EXEC_ST_MASK;
        switch (CpuId)
        {
        case IH_PH_ATTRB_DEST_CPU_A53_0:

            PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU0_MASK |
                           PMU_GLOBAL_PWR_STATE_FP_MASK |
                           PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

            Status = FmshFsbl_PowerUpIsland(PwrStateMask);
            if (Status != FMSH_SUCCESS)
            {
                Status = FSBL_ERROR_A53_0_POWER_UP;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_A53_0_POWER_UP\r\n");
                return Status;
            }

            /**
             * Set to Aarch32 if enabled
             */
            if (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32)
            {
                RegValue = ReadReg(APU_CONFIG_0);
                RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU0);
                WriteReg(APU_CONFIG_0, RegValue);
            }

            /**
             *  Enable the clock
             */
            RegValue = ReadReg(CRF_APB_ACPU_CTRL);
            RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
                         CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
            WriteReg(CRF_APB_ACPU_CTRL, RegValue);

            /**
             * Release reset
             */
            RegValue = ReadReg(CRF_APB_RST_FPD_APU);
            RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU0_RESET_MASK |
                          CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
                          CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK);
            WriteReg(CRF_APB_RST_FPD_APU, RegValue);

            break;

        case IH_PH_ATTRB_DEST_CPU_A53_1:

            PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU1_MASK |
                           PMU_GLOBAL_PWR_STATE_FP_MASK |
                           PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

            Status = FmshFsbl_PowerUpIsland(PwrStateMask);
            if (Status != FMSH_SUCCESS)
            {
                Status = FSBL_ERROR_A53_1_POWER_UP;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_A53_1_POWER_UP\r\n");
                return Status;
            }

            /**
             * Set to Aarch32 if enabled
             */
            if (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32)
            {
                RegValue = ReadReg(APU_CONFIG_0);
                RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU1);
                WriteReg(APU_CONFIG_0, RegValue);
            }

            /**
             *  Enable the clock
             */
            RegValue = ReadReg(CRF_APB_ACPU_CTRL);
            RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
                         CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
            WriteReg(CRF_APB_ACPU_CTRL, RegValue);

            /**
             * Release reset
             */
            RegValue = ReadReg(CRF_APB_RST_FPD_APU);
            RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU1_RESET_MASK |
                          CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
                          CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK);
            WriteReg(CRF_APB_RST_FPD_APU, RegValue);

            break;

        case IH_PH_ATTRB_DEST_CPU_A53_2:

            PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU2_MASK |
                           PMU_GLOBAL_PWR_STATE_FP_MASK |
                           PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

            Status = FmshFsbl_PowerUpIsland(PwrStateMask);
            if (Status != FMSH_SUCCESS)
            {
                Status = FSBL_ERROR_A53_2_POWER_UP;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_A53_2_POWER_UP\r\n");
                return Status;
            }

            /**
             * Set to Aarch32 if enabled
             */
            if (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32)
            {
                RegValue = ReadReg(APU_CONFIG_0);
                RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU2);
                WriteReg(APU_CONFIG_0, RegValue);
            }

            /**
             *  Enable the clock
             */
            RegValue = ReadReg(CRF_APB_ACPU_CTRL);
            RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
                         CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
            WriteReg(CRF_APB_ACPU_CTRL, RegValue);

            /**
             * Release reset
             */
            RegValue = ReadReg(CRF_APB_RST_FPD_APU);
            RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU2_RESET_MASK |
                          CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
                          CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK);

            WriteReg(CRF_APB_RST_FPD_APU, RegValue);

            break;

        case IH_PH_ATTRB_DEST_CPU_A53_3:

            PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU3_MASK |
                           PMU_GLOBAL_PWR_STATE_FP_MASK |
                           PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

            Status = FmshFsbl_PowerUpIsland(PwrStateMask);
            if (Status != FMSH_SUCCESS)
            {
                Status = FSBL_ERROR_A53_3_POWER_UP;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_A53_3_POWER_UP\r\n");
                return Status;
            }

            /**
             * Set to Aarch32 if enabled
             */
            if (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32)
            {
                RegValue = ReadReg(APU_CONFIG_0);
                RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU3);
                WriteReg(APU_CONFIG_0, RegValue);
            }

            /**
             *  Enable the clock
             */
            RegValue = ReadReg(CRF_APB_ACPU_CTRL);
            RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
                         CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
            WriteReg(CRF_APB_ACPU_CTRL, RegValue);

            /**
             * Release reset
             */
            RegValue = ReadReg(CRF_APB_RST_FPD_APU);
            RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU3_RESET_MASK |
                          CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
                          CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK);

            WriteReg(CRF_APB_RST_FPD_APU, RegValue);

            break;

        case IH_PH_ATTRB_DEST_CPU_R5_0:

            Status = FmshFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_R5_0_MASK);
            if (Status != FMSH_SUCCESS)
            {
                Status = FSBL_ERROR_R5_0_POWER_UP;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_R5_0_POWER_UP\r\n");
                return Status;
            }

            /**
             * Place R5, TCM's in split mode
             */
            RegValue = ReadReg(RPU_RPU_GLBL_CNTL);
            RegValue |= (RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
            RegValue &= ~(RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
            RegValue &= ~(RPU_RPU_GLBL_CNTL_SLCLAMP_MASK);
            WriteReg(RPU_RPU_GLBL_CNTL, RegValue);

            /**
             * Place R5-0 in HALT state
             */
            RegValue = ReadReg(RPU_RPU_0_CFG);
            RegValue &= ~(RPU_RPU_0_CFG_NCPUHALT_MASK);
            WriteReg(RPU_RPU_0_CFG, RegValue);

            /**
             *  Enable the clock
             */
            RegValue = ReadReg(CRL_APB_CPU_R5_CTRL);
            RegValue |= (CRL_APB_CPU_R5_CTRL_CLKACT_MASK);
            WriteReg(CRL_APB_CPU_R5_CTRL, RegValue);

            /**
             * Provide some delay,
             * so that clock propagates properly.
             */
            (void)delay_us(0x50U);

            /**
             * Release reset to R5-0
             */
            RegValue = ReadReg(CRL_APB_RST_LPD_TOP);
            RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
            RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
            WriteReg(CRL_APB_RST_LPD_TOP, RegValue);

            /**
             * Take R5-0 out of HALT state
             */
            RegValue = ReadReg(RPU_RPU_0_CFG);
            RegValue |= RPU_RPU_0_CFG_NCPUHALT_MASK;
            WriteReg(RPU_RPU_0_CFG, RegValue);
            break;

        case IH_PH_ATTRB_DEST_CPU_R5_1:

            Status = FmshFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_R5_1_MASK);
            if (Status != FMSH_SUCCESS)
            {
                Status = FSBL_ERROR_R5_1_POWER_UP;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_R5_1_POWER_UP\r\n");
                return Status;
            }

            /**
             * Place R5, TCM's in split mode
             */
            RegValue = ReadReg(RPU_RPU_GLBL_CNTL);
            RegValue |= RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
            RegValue &= ~(RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
            RegValue &= ~(RPU_RPU_GLBL_CNTL_SLCLAMP_MASK);
            WriteReg(RPU_RPU_GLBL_CNTL, RegValue);

            /**
             * Place R5-1 in HALT state
             */
            RegValue = ReadReg(RPU_RPU_1_CFG);
            RegValue &= ~(RPU_RPU_1_CFG_NCPUHALT_MASK);
            WriteReg(RPU_RPU_1_CFG, RegValue);

            /**
             *  Enable the clock
             */
            RegValue = ReadReg(CRL_APB_CPU_R5_CTRL);
            RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
            WriteReg(CRL_APB_CPU_R5_CTRL, RegValue);

            /**
             * Provide some delay,
             * so that clock propagates properly.
             */
            (void)delay_us(0x50U);

            /**
             * Release reset to R5-1
             */
            RegValue = ReadReg(CRL_APB_RST_LPD_TOP);
            RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
            RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
            WriteReg(CRL_APB_RST_LPD_TOP, RegValue);

            /**
             * Take R5-1 out of HALT state
             */
            RegValue = ReadReg(RPU_RPU_1_CFG);
            RegValue |= RPU_RPU_1_CFG_NCPUHALT_MASK;
            WriteReg(RPU_RPU_1_CFG, RegValue);
            break;
        case IH_PH_ATTRB_DEST_CPU_R5_L:

            Status = FmshFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_R5_0_MASK);
            if (Status != FMSH_SUCCESS)
            {
                Status = FSBL_ERROR_R5_L_POWER_UP;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_R5_L_POWER_UP\r\n");
                return Status;
            }

            /**
             * Place R5, TCM's in safe mode
             */
            RegValue = ReadReg(RPU_RPU_GLBL_CNTL);
            RegValue &= ~(RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
            RegValue |= RPU_RPU_GLBL_CNTL_TCM_COMB_MASK;
            RegValue |= RPU_RPU_GLBL_CNTL_SLCLAMP_MASK;
            WriteReg(RPU_RPU_GLBL_CNTL, RegValue);

            /**
             * Place R5-0 in HALT state
             */
            RegValue = ReadReg(RPU_RPU_0_CFG);
            RegValue &= ~(RPU_RPU_0_CFG_NCPUHALT_MASK);
            WriteReg(RPU_RPU_0_CFG, RegValue);

            /**
             * Place R5-1 in HALT state
             */
            RegValue = ReadReg(RPU_RPU_1_CFG);
            RegValue &= ~(RPU_RPU_1_CFG_NCPUHALT_MASK);
            WriteReg(RPU_RPU_1_CFG, RegValue);

            /**
             *  Enable the clock
             */
            RegValue = ReadReg(CRL_APB_CPU_R5_CTRL);
            RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
            WriteReg(CRL_APB_CPU_R5_CTRL, RegValue);

            /**
             * Provide some delay,
             * so that clock propagates properly.
             */
            (void)delay_us(0x50U);

            /**
             * Release reset to R5-0, R5-1
             */
            RegValue = ReadReg(CRL_APB_RST_LPD_TOP);
            RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
            RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
            RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
            WriteReg(CRL_APB_RST_LPD_TOP, RegValue);

            /**
             * Take R5-0 out of HALT state
             */
            RegValue = ReadReg(RPU_RPU_0_CFG);
            RegValue |= RPU_RPU_0_CFG_NCPUHALT_MASK;
            WriteReg(RPU_RPU_0_CFG, RegValue);

            /**
             * Take R5-1 out of HALT state
             */
            RegValue = ReadReg(RPU_RPU_1_CFG);
            RegValue |= RPU_RPU_1_CFG_NCPUHALT_MASK;
            WriteReg(RPU_RPU_1_CFG, RegValue);
            break;

        default:
            UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_HANDOFF_CPUID\n\r");
            Status = FSBL_ERROR_HANDOFF_CPUID;
            break;
        }
    }
    else
    {
        Status = FMSH_SUCCESS;
    }

    return Status;
}

/****************************************************************************/
/**
 *
 * @param
 *
 * @return
 *
 * @note
 *
 *
 *****************************************************************************/
static u32 FmshFsbl_ProtectionConfig (void)
{
    u32 CfgRegVal1 = 0U;
    u32 CfgRegVal3 = 0U;
    u32 Status = FMSH_SUCCESS;
    /* Disable Tamper responses*/
    CfgRegVal1 = ReadReg(FSBL_PS_SYSMON_CONFIGREG1);
    CfgRegVal3 = ReadReg(FSBL_PS_SYSMON_CONFIGREG3);

    WriteReg(FSBL_PS_SYSMON_CONFIGREG1,
             CfgRegVal1 | FSBL_PS_SYSMON_CFGREG1_ALRM_DISBL_MASK);
    WriteReg(FSBL_PS_SYSMON_CONFIGREG3,
             CfgRegVal3 | FSBL_PS_SYSMON_CFGREG3_ALRM_DISBL_MASK);

    /* FSBL shall bypass XPPU and FPD XMPU configuration BY DEFAULT.
     *  This means though the Isolation configuration through hdf is used
     * throughout the software flow, for the hardware, isolation will only be
     * limited to just OCM.
     */
#ifdef FSBL_PROT_BYPASS
    psu_apply_master_tz();
    psu_ocm_protection();
#else
    /* Apply protection configuration */
    Status = (u32)psu_protection();
    if (Status != FMSH_SUCCESS)
    {
        Status = FSBL_ERROR_PROTECTION_CFG;
        UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_PROTECTION_CFG\r\n");
        return Status;
    }

    /* Lock XMPU/XPPU for further access */
    Status = (u32)psu_protection_lock();
    if (Status != FMSH_SUCCESS)
    {
        Status = FSBL_ERROR_PROTECTION_CFG;
        UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_PROTECTION_CFG\r\n");
        return Status;
    }
#endif

    /*Enable Tamper responses*/

    WriteReg(FSBL_PS_SYSMON_CONFIGREG1, CfgRegVal1);
    WriteReg(FSBL_PS_SYSMON_CONFIGREG3, CfgRegVal3);
    Status = FMSH_SUCCESS;

    return Status;
}

/*****************************************************************************/
/**
 * This function determines if the given partition needs early handoff
 *
 * @param	FsblInstancePtr is pointer to the Fsbl Instance
 *
 * @param	PartitionNum is the partition number of the image
 *
 * @return	TRUE if this partitions needs early handoff, and FALSE if not
 *
 *****************************************************************************/
u32 FmshFsbl_CheckEarlyHandoff (BootPs *BootInstancePtr, u32 PartitionNum)
{
    u32 Status = FALSE;
#if defined(FSBL_EARLY_HANDOFF)
    u32 CpuNeedsEarlyHandoff = FALSE;
    u32 DestinationCpu = 0;
    u32 DestinationDev = 0;
    u32 DestinationCpuNxt = 0;
    u32 DestinationDevNxt = 0;

    DestinationCpu = FmshFsbl_GetDestinationCpu(
        &BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum]);
    DestinationDev = FmshFsbl_GetDestinationDevice(
        &BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum]);
    if ((DestinationCpu == IH_PH_ATTRB_DEST_CPU_NONE) &&
        ((DestinationDev == IH_PH_ATTRB_DEST_DEVICE_PS) ||
         (DestinationDev == IH_PH_ATTRB_DEST_DEVICE_NONE)))
    {
        /* If dest device is not PS, retain the dest CPU as NONE/0 */
        DestinationCpu = BootInstancePtr->ProcessorID;
    }

    if ((PartitionNum + 1) <=
        (BootInstancePtr->ImageHeader.ImageHeaderTable.NoOfPartitions - 1U))
    {
        DestinationCpuNxt = FmshFsbl_GetDestinationCpu(
            &BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum + 1]);
        DestinationDevNxt = FmshFsbl_GetDestinationDevice(
            &BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum + 1]);

        if ((DestinationCpuNxt == IH_PH_ATTRB_DEST_CPU_NONE) &&
            ((DestinationDevNxt == IH_PH_ATTRB_DEST_DEVICE_PS) ||
             (DestinationDevNxt == IH_PH_ATTRB_DEST_DEVICE_NONE)))
        {
            DestinationCpuNxt = BootInstancePtr->ProcessorID;
        }
    }

    /**
     *  Early handoff needed if destination CPU needs early handoff AND
     *  if handoff CPU is not same as running CPU AND
     *  if this is the last partition of this application
     */
    CpuNeedsEarlyHandoff = FmshFsbl_CheckEarlyHandoffCpu(DestinationCpu);
    if ((CpuNeedsEarlyHandoff == TRUE) &&
        (DestinationCpu != BootInstancePtr->ProcessorID) &&
        (DestinationCpuNxt != DestinationCpu))
    {
        Status = TRUE;
    }
#endif
    return Status;
}

/****************************************************************************
 *
 * This function is used to hand off cpu.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 *
 *****************************************************************************/
u32 BootHandoff (const BootPs *BootInstancePtr, u32 PartitionNum,
                 u32 EarlyHandoff)
{
    u32 Status = FMSH_SUCCESS;
    u32 CpuIndex = 0U;
    u32 CpuId = 0U;
    u32 ExecState = 0;
    u32 CpuSettings = 0U;
    u64 HandoffAddress = 0U;
    u64 RunningCpuHandoffAddress = 0U;
    u32 RunningCpuExecState = 0U;
    u32 RunningCpuHandoffAddressPresent = FALSE;
    u32 CpuNeedsEarlyHandoff = 0U;
    static u32 CpuIndexEarlyHandoff = 0U;

    if (FmshFsbl_IsBitDone())
    {
       /**Remove PS-PL isolation as bitstream is loaded*/
       psu_ps_pl_isolation_removal_data();
       psu_ps_pl_reset_config_data();
    }
#if DCACHE_ENABLE == 1
    Fmsh_DCacheDisable();
#endif
    if (BootInstancePtr->ResetReason != FSBL_MASTER_ONLY_RESET) 
    {

    Status = FmshFsbl_PmInit();
    if (Status != FMSH_SUCCESS)
    {
        Status = FSBL_ERROR_PM_INIT;
        UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_PM_INIT\r\n");
        return Status;
        }

    }
    Status = FmshFsbl_ProtectionConfig();
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    UART_LOG_OUT(DEBUG_INFO, "Protection configuration applied\r\n");

    /**
     * if JTAG bootmode, be in while loop as of now
     * Check if Process can be parked in HALT state
     */
    if (BootInstancePtr->PrimaryBootDevice == JTAG_BOOT_MODE)
    {
        
        /**
         * Mark Error status with Fsbl completed
         */
        WriteReg(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4, FSBL_COMPLETED);

        if (0)
        {
            UART_LOG_OUT(DEBUG_INFO, "Exit from FSBL. \n\r");
#ifdef __aarch64__
            Fmsh_Out32(0xFFFC0000U, 0x14000000U);
#else
            Fmsh_Out32(0xFFFC0000U, 0xEAFFFFFEU);
#endif
            FMSH_EXIT(0xFFFC0000U, FSBL_HANDOFFEXIT);
        }
        else
        {
            /**
             * Exit from FSBL
             */
            FmshFsbl_HandoffExit(0U, FSBL_NO_HANDOFFEXIT);
        }
    }

    /**
     * If we are doing early handoff, remember the CPU index to avoid
     * traversing through for the next early handoff
     */
    if (EarlyHandoff == TRUE)
    {
        CpuIndex = CpuIndexEarlyHandoff;
    }
    else
    {
        CpuIndex = 0U;
    }

    while (CpuIndex < BootInstancePtr->HandoffCpuNo)
    {
        CpuSettings = BootInstancePtr->HandoffValues[CpuIndex].CpuSettings;

        CpuId = CpuSettings & IH_PH_ATTRB_DEST_CPU_MASK;
        ExecState = CpuSettings & IH_PH_ATTRB_A53_EXEC_ST_MASK;

        /**
         * Run the code in this loop in the below conditions:
         * - This function called for early handoff and CPU needs early handoff
         * - This function called for regular handoff and CPU doesn't need early
         *   handoff
         * - This function called for regular handoff and CPU needs early
         *   handoff AND if handoff is to running CPU
         *
         */

        CpuNeedsEarlyHandoff = FmshFsbl_CheckEarlyHandoffCpu(CpuId);
        if (((CpuNeedsEarlyHandoff == TRUE) && (EarlyHandoff == TRUE)) ||
            ((EarlyHandoff != TRUE) && (CpuNeedsEarlyHandoff != TRUE)) ||
            (((EarlyHandoff != TRUE) && (CpuNeedsEarlyHandoff == TRUE)) &&
             (CpuId == BootInstancePtr->ProcessorID)))
        {
            /**
             * Check if handoff address is present
             */
            if (CpuId != BootInstancePtr->ProcessorID)
            {
                /* Check if handoff CPU is supported */
                Status = FmshFsbl_CheckSupportedCpu(CpuId);
                if (FMSH_SUCCESS != Status)
                {
                    UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_UNAVAILABLE_CPU\n\r");
                    Status = FSBL_ERROR_UNAVAILABLE_CPU;
                    return Status;
                }

                /**
                 * Check for power status of the cpu
                 * Update the IVT
                 * Take cpu out of reset
                 */
                Status = FmshFsbl_SetCpuPwrSettings(CpuSettings,
                                                    FSBL_CPU_POWER_UP);
                if (FMSH_SUCCESS != Status)
                {
                    UART_LOG_OUT(DEBUG_INFO,
                                 "Power Up "
                                 "Cpu 0x%0lx failed \n\r",
                                 CpuId);

                    UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_PWR_UP_CPU\n\r");
                    Status = FSBL_ERROR_PWR_UP_CPU;
                    return Status;
                }

                /**
                 * Read the handoff address from structure
                 */
                HandoffAddress = (u64)BootInstancePtr->HandoffValues[CpuIndex]
                                     .HandoffAddress;

                /**
                 * Update the handoff address at reset vector address
                 */
                FmshFsbl_UpdateResetVector(
                    HandoffAddress, CpuSettings,
                    ((BootInstancePtr->HandoffValues[CpuIndex]
                          .PartitionAttributes) &
                     IH_PH_ATTRB_VEC_LOCATION_MASK) >>
                        IH_PH_ATTRB_VECTOR_LOCATION_SHIFT);

                UART_LOG_OUT(DEBUG_INFO,
                             "CPU 0x%0lx reset release, "
                             "Exec State 0x%0lx, HandoffAddress: %0lx\n\r",
                             CpuId, ExecState, (UINTPTR)HandoffAddress);

                /**
                 * Take CPU out of reset
                 */
                Status = FmshFsbl_SetCpuPwrSettings(CpuSettings,
                                                    FSBL_CPU_SWRST);
                if (FMSH_SUCCESS != Status)
                {
                    return Status;
                }
            }
            else
            {
                /**
                 * Update the running cpu handoff address
                 */
                RunningCpuHandoffAddressPresent = TRUE;
                RunningCpuHandoffAddress = BootInstancePtr
                                               ->HandoffValues[CpuIndex]
                                               .HandoffAddress;
                RunningCpuExecState = ExecState;

                /**
                 * Update reset vector address for
                 * - FSBL running on A53-0 (64bit), handoff to A53-0 (32 bit)
                 * - FSBL running on A53-0 (32bit), handoff to A53-0 (64 bit)
                 */
                if ((BootInstancePtr->A53ExecState ==
                     IH_PH_ATTRB_A53_EXEC_ST_AA64) &&
                    (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32))
                {
                    Status = FSBL_ERROR_UNSUPPORTED_HANDOFF;
                    UART_LOG_OUT(DEBUG_INFO,
                                 "FSBL_ERROR_UNSUPPORTED_HANDOFF : A53-0 64 "
                                 "bit to 32 bit\n\r");
                    return Status;
                }
                else if ((BootInstancePtr->A53ExecState ==
                          IH_PH_ATTRB_A53_EXEC_ST_AA32) &&
                         (ExecState == IH_PH_ATTRB_A53_EXEC_ST_AA64))
                {
                    Status = FSBL_ERROR_UNSUPPORTED_HANDOFF;
                    UART_LOG_OUT(DEBUG_INFO,
                                 "FSBL_ERROR_UNSUPPORTED_HANDOFF : A53-0 32 "
                                 "bit to 64 bit\n\r");
                    return Status;
                }
                else
                {
                    /* for MISRA C compliance */
                }
            }
        }
        if ((EarlyHandoff == TRUE) && (CpuNeedsEarlyHandoff == TRUE))
        {
            /* Enable cache again as we will continue loading partitions */
            Fmsh_DCacheEnable();

            if (PartitionNum <
                (BootInstancePtr->ImageHeader.ImageHeaderTable.NoOfPartitions -
                 1U))
            {
                /**
                 * If this is not the last handoff CPU, return back and continue
                 * loading remaining partitions in stage 3
                 */
                CpuIndexEarlyHandoff++;
                Status = FSBL_STATUS_CONTINUE_PARTITION_LOAD;
            }
            else
            {
                /**
                 * Early handoff to all required CPUs is done, continue with
                 * regular handoff for remaining applications, as applicable
                 */
                Status = FSBL_STATUS_CONTINUE_OTHER_HANDOFF;
            }
            return Status;
        }
        /**
         * Go to the next cpu
         */
        CpuIndex++;
        CpuIndexEarlyHandoff++;
    }

#ifdef ARMR5

    /**
     * Remove the R5 vectors from TCM and load APP data
     * if present
     */

    if (TcmSkipLength != 0U)
    {
        /* Restore R5LovecBuffer to LOVEC
         * This will store partitions vectors to LOVEC
         * TcmSkipAddress is always 0x0,TcmSkipLength is 32.
         */
        (void)memcpy((u8 *)TcmSkipAddress, (u8 *)R5LovecBuffer, TcmSkipLength);
        UART_LOG_OUT(
            DEBUG_INFO,
            "Fsbl_Handoff:Restored R5LovecBuffer to LOVEC for R5.\n\r");
    }
#endif
    /**
     * Mark Error status with Fsbl completed
     */
    WriteReg(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4, FSBL_COMPLETED);

#if FSBL_WDT_ENABLE
    FmshFsbl_WdtClose();
#endif

    if (FmshFsbl_IsBitDone() == FALSE)
    {
        FmshFsbl_CloseUsrLevelShifter();
    }

    /**
     * call to the handoff routine
     * which will never return
     */
    if (RunningCpuHandoffAddressPresent == TRUE)
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "Running Cpu Handoff address: 0x%0lx,Exec state:%08x\n\r",
                     (UINTPTR)RunningCpuHandoffAddress, RunningCpuExecState);
        if (RunningCpuExecState == IH_PH_ATTRB_A53_EXEC_ST_AA32)
        {
            UART_LOG_OUT(DEBUG_INFO, "Exec State: 32bit\n\r");
            FmshFsbl_HandoffExit(RunningCpuHandoffAddress, FSBL_HANDOFFEXIT_32);
        }
        else
        {
            UART_LOG_OUT(DEBUG_INFO, "Exec State: 64bit\n\r");
            FmshFsbl_HandoffExit(RunningCpuHandoffAddress, FSBL_HANDOFFEXIT);
        }
    }
    else
    {
        FmshFsbl_HandoffExit(0U, FSBL_NO_HANDOFFEXIT);
    }

    return Status;
}

/****************************************************************************
 *
 * This function is used to default exit.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 *
 *****************************************************************************/
void DefaultHandoffExit (void)
{
#if FSBL_WDT_ENABLE
    FmshFsbl_WdtClose();
#endif

#if DCACHE_ENABLE == 1
    Fmsh_DCacheDisable();
#endif

    asm("LOOP:\n\t"
        "wfe\n\t"
        "b LOOP");
}
/****************************************************************************
 *
 * This function is used to lock down.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 *
 *****************************************************************************/
void ErrorLockDown (void)
{
    u32 BootDevice = 0;
    u32 RegValue = 0;

#if FSBL_WDT_ENABLE
    FmshFsbl_WdtClose();
#endif

#if DCACHE_ENABLE == 1
    Fmsh_DCacheDisable();
#endif

    /**
     * Print the FSBL error
     */
    UART_LOG_OUT(DEBUG_INFO, "FSBL Boot error code is 0x%8x\n\r",BootInstance.ErrorCode);
    BootDevice = BootInstance.BootMode;
    if ((BootDevice == QSPI_FLASH) || (BootDevice == NAND_FLASH) ||
        (BootDevice == USB_CON) ||(BootDevice == SD_CARD))
    {
#ifndef FSBL_MULTI_BOOT_EXCLUDE
        /* Read the Multiboot register */
        RegValue = ReadReg(SAC_MULTI_BOOT_REG);
        WriteReg(MULTI_BOOT_REG, RegValue + 1U);
        // WriteReg(MULTI_BOOT_REG,GOLDEN_IMG_ADDRESS/GOLDEN_IMAGE_OFFSET);
        WriteReg(SAC_CFG_REG, ReadReg(SAC_CFG_REG) | SAC_MULTIBOOT_EN_MASK);
#endif
        /* make sure every thing completes */
        dsb();
        isb();
        /* Soft reset the system */
        UART_LOG_OUT(DEBUG_INFO, "Performing System Soft Reset\n\r");
        RegValue = ReadReg(CRL_APB_RESET_CTRL);
        WriteReg(CRL_APB_RESET_CTRL,
                 RegValue | CRL_APB_RESET_CTRL_SOFT_RESET_MASK);

        while (1){}
    }
    else
    {
        HandoffJtagExit();
    }
}
