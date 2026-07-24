/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_initialization.c
 *
 * This file contains boot_main.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  01/01/2024  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"
#include "fmsh_cci_reg.h"
/************************** Constant Definitions *****************************/
#define FSBL_APU_RESET_MASK (1U << 16U)
#define FSBL_APU_RESET_BIT  16U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static u32 FmshFsbl_SystemInit(BootPs *BootInstancePtr);

/************************** Variable Definitions *****************************/
extern BootPs BootInstance;
extern Ps_BootHeader BootHeaderInfo;

u8 __data_start;
u8 __data_end;
u8 __dup_data_start;

#ifdef FSBL_REMOVE_ISO_START
extern unsigned long psu_ps_pl_isolation_removal_data(void);
extern unsigned long psu_ps_pl_reset_config_data(void);
#endif
/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 * This function is used to save the data section into duplicate data section
 * so that it can be restored from in case of subsequent warm restarts
 *
 * @param  None
 *
 * @return None
 *
 * @note
 *
 *****************************************************************************/
void Fsbl_SaveData (void)
{
    const u8 *MemPtr;

    u8 *ContextMemPtr = (u8 *)&__dup_data_start;

    for (MemPtr = &__data_start; MemPtr < &__data_end;
         MemPtr++, ContextMemPtr++)
    {
        *ContextMemPtr = *MemPtr;
    }
}

/****************************************************************************/
/**
 * This function is used to restore the data section from duplicate data section
 * in case of warm restart.
 *
 * @param  None
 *
 * @return None
 *
 * @note
 *
 *****************************************************************************/
void Fsbl_RestoreData (void)
{
    u8 *MemPtr;

    u8 *ContextMemPtr = (u8 *)&__dup_data_start;

    for (MemPtr = &__data_start; MemPtr < &__data_end;
         MemPtr++, ContextMemPtr++)
    {
        *MemPtr = *ContextMemPtr;
    }
}

/****************************************************************************/
/**
 * This function is used to get the Reset Reason.
 *
 * @param  None
 *
 * @return Reset Reason
 *
 * @note
 *
 *****************************************************************************/
static u32 FmshFsbl_GetResetReason (void)
{
    u32 Ret = 0;


        /* Clear the PS Only reset bit as it is sticky */
        Ret = (ReadReg(PMU_GLOBAL_GLOB_GEN_STORAGE4) & FSBL_APU_RESET_MASK) >>
              (FSBL_APU_RESET_BIT);

        if (Ret == FSBL_SYSTEM_RESET)
        {
            Fsbl_SaveData();
        }
        else
        {
            Ret = FSBL_MASTER_ONLY_RESET;
            Fsbl_RestoreData();
    }

    return Ret;
}

void Fsbl_PrintFsblBanner (BootPs *BootInstancePtr)
{
    UART_LOG_OUT(DEBUG_INFO, "\r\n====MPSOC FSBL BOOTING====\r\n");
    UART_LOG_OUT(DEBUG_INFO, "====FSBL Version: %3.2f====\r\n",
                 BootInstance.Version);
    UART_LOG_OUT(DEBUG_INFO,
                 "====Vulture SVN Version: %d  Release Date: %s====\r\n",
                 VULTURE_SVN_VERSION, VULTURE_RELEASE_DATE);
    UART_LOG_OUT(DEBUG_INFO, "====Build Date: %s  %s====\r\n", __DATE__,
                 __TIME__);
    
    if (BootInstancePtr->ResetReason == FSBL_SYSTEM_RESET)
    {
        UART_LOG_OUT(DEBUG_INFO, "System reset\r\n");
    }
}

/*****************************************************************************/
/**
 * This function enables the propagation of the PROG signal to PL after
 * PS-only reset
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
static void Fsbl_EnableProgToPL (void)
{
    u32 RegVal = 0x0U;

    /*
     * PROG control to PL.
     */
    //RegVal = Fmsh_In32(SAC_CFG_REG);
    //Fmsh_Out32(SAC_CFG_REG, RegVal | SAC_PROG_B_MASK);

    /*
     * Enable the propagation of the PROG signal to the PL after PS-only reset
     * */
    RegVal = Fmsh_In32(PMU_GLOBAL_PS_CNTRL);

    RegVal &= ~(PMU_GLOBAL_PS_CNTRL_PROG_GATE_MASK);
    RegVal |= (PMU_GLOBAL_PS_CNTRL_PROG_ENABLE_MASK);

    Fmsh_Out32(PMU_GLOBAL_PS_CNTRL, RegVal);
}

/******************************************************************************

* This function initializes the system.
*
* @param	BootInstancePtr is pointer to the BootPs Instance
*
* @return	returns the error codes described in error.h on any error
* 			returns FMSH_SUCCESS on success
*
******************************************************************************/
static u32 FmshFsbl_SystemInit (BootPs *BootInstancePtr)
{
    // Get the PS_VERSION on run time
    int Status = FMSH_SUCCESS;

#ifdef FSBL_ENABLE_DDR_SR
    u32 RegVal;
	/* Check if DDR is in self refresh mode */
	RegVal = Fmsh_In32(FSBL_DDR_STATUS_REGISTER_OFFSET) &
		DDR_STATUS_FLAG_MASK;
	if (RegVal) {
		Status = psu_init_ddr_self_refresh();
	} else {
		Status = psu_init();
	}
#else
	Status = psu_init();
#endif
    
    if (Status != PSU_INIT_SUCCESS)
    {
        Status |= FSBL_PSU_INIT_FAILED;
        return Status;
    }
#ifdef CORTEX_A53
    CCIreg_init();
#endif
    /*
     * Enables the propagation of the PROG signal to PL
     */
    Fsbl_EnableProgToPL();
    
    Fmsh_ICacheEnable();
    
#if (USE_DDR == 1) && (PS_PREINITED == 0)
#ifdef CORTEX_A53  
#if (DDR_SIZE >= 0x80000000)
    Fmsh_SetTlbAttributesRange(0x0, 0x7f000000, ATTR_MEM);
    Fmsh_SetTlbAttributesRange(0x7f000000, 0x1000000, ATTR_MEM_NC);
#else /*DDR_SIZE*/
    Fmsh_SetTlbAttributesRange(0x0, DDR_SIZE - 0x1000000, ATTR_MEM);
    Fmsh_SetTlbAttributesRange(DDR_SIZE - 0x1000000, 0x1000000, ATTR_MEM_NC);
#endif
#else    
    Fmsh_SetAttribute(0x0, REGION_2G , 0, NORM_NSHARED_WB_WA | PRIV_RW_USER_RW);
#endif
#endif

#if DCACHE_ENABLE == 1
    Fmsh_DCacheEnable();
#endif
#if FSBL_WDT_ENABLE
    FmshFsbl_WdtInit();
#endif

#if DEBUG_INFO | DEBUG_PERF
    Status = FmshFsbl_UartInit();
    if (Status != FMSH_SUCCESS)
    {
        Status = FSBL_ERROR_UART_INIT;
        return Status;
    }
#endif

#ifdef FSBL_REMOVE_ISO_START    
    if (FmshFsbl_IsBitDone())
    {
       /**Remove PS-PL isolation as bitstream is loaded*/
       psu_ps_pl_isolation_removal_data();
       psu_ps_pl_reset_config_data();
    }
    else
    {
       // enable isolation
       FMSH_WriteReg(FPS_PMU_GLOBAL_BASEADDR, 0x318, 1 << 2);
       FMSH_WriteReg(FPS_PMU_GLOBAL_BASEADDR, 0x320, 1 << 2);
    }
#endif
    
    Fsbl_PrintFsblBanner(BootInstancePtr);
    UART_LOG_OUT(DEBUG_INFO, "======= In BootStage 1 ======= \r\n");
    UART_LOG_OUT(DEBUG_INFO, "====UART initialized success!!!====\r\n");
    
    FMSH_ExceptionEnable();
    Status = FGicPs_SelfTest(&IntcInstance);
    if (Status == GIC_FAILURE)
    {
        Status = FSBL_ERROR_GIC_INIT;
        UART_LOG_OUT(DEBUG_INFO, "GIC IRQ selftest failed !\r\n");
    }
    else
    {
        UART_LOG_OUT(DEBUG_INFO, "GIC IRQ selftest pass !\r\n");
    }

    return Status;
}

/*****************************************************************************
 * This function initializes the processor and updates the cluster id
 * which indicates CPU on which fsbl is running
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @return	returns the error codes described in fmsh_error.h on any error
 * 			returns FMSH_SUCCESS on success
 *
 ******************************************************************************/
static u32 FmshFsbl_ProcessorInit (BootPs *BootInstancePtr)
{
    u32 Status = FMSH_SUCCESS;
    u64 ClusterId = 0U;
    u32 FsblProcType = 0;
    u32 RegValue = 0;
    u32 Index=0;
    /**
     * Read the cluster ID and Update the Processor ID
     * Initialize the processor settings that are not done in
     * BSP startup code
     */
#ifdef __aarch64__
    __asm volatile("mrs %0,midr_el1" : "=r"(ClusterId));
#else
    __asm volatile("mrc p15,0,%0,c0,c0,0" : "=r"(ClusterId));
#endif

    UART_LOG_OUT(DEBUG_INFO, "Cluster ID 0x%x\n\r", ClusterId);

    /* store the processor ID based on the cluster ID */
    if ((ClusterId & FSBL_CLUSTER_ID_MASK) == FSBL_A53_PROCESSOR)
    {
        UART_LOG_OUT(DEBUG_INFO, "Running on A53-0\r\n");
        BootInstancePtr->ProcessorID = ATTRB_DEST_CPU_A53_0;
        BootInstancePtr->A53ExecState = ATTRB_A53_EXEC_ST_AA64;
        FsblProcType = FSBL_RUNNING_ON_A53 << FSBL_STATE_PROC_SHIFT;
    }
    else if ((ClusterId & FSBL_CLUSTER_ID_MASK) == FSBL_R5_PROCESSOR)
    {
        BootInstancePtr->A53ExecState = ATTRB_INVALID_EXEC_ST;
        RegValue = ReadReg(RPU_RPU_GLBL_CNTL);
        if ((RegValue & RPU_RPU_GLBL_CNTL_SLSPLIT_MASK) == 0U)
        {
            UART_LOG_OUT(DEBUG_INFO, "Running on R5 Processor in Lockstep");
            BootInstancePtr->ProcessorID = ATTRB_DEST_CPU_R5_L;
            FsblProcType = FSBL_RUNNING_ON_R5_L << FSBL_STATE_PROC_SHIFT;
        }
        else
        {
            UART_LOG_OUT(DEBUG_INFO, "Running on R5-0 Processor");
            BootInstancePtr->ProcessorID = ATTRB_DEST_CPU_R5_0;
            FsblProcType = FSBL_RUNNING_ON_R5_0 << FSBL_STATE_PROC_SHIFT;
            while (Index<32U) {
		Fmsh_Out32(Index, FSBL_R5_LOVEC_VALUE);
		Index += 4U;
            }
        }
    }
    else
    {
        UART_LOG_OUT(DEBUG_INFO, "Invalid ID!\r\n");
        BootInstancePtr->ErrorCode = FSBL_ERROR_INVALID_ID;
        Status = FMSH_FAILURE;
    }

    /*
     * Update FSBL processor information to PMU Global Reg5
     * as PMU require this during boot for warm-restart feature.
     */
    FsblProcType |= (ReadReg(PMU_GLOBAL_GLOB_GEN_STORAGE5) &
                     ~(FSBL_STATE_PROC_INFO_MASK));

    WriteReg(PMU_GLOBAL_GLOB_GEN_STORAGE5, FsblProcType);

    return Status;
}

/*****************************************************************************/
/**
 * This function does ECC Initialization of memory
 *
 * @param	DestAddr is start address from where to calculate ECC
 * @param	LengthBytes is length in bytes from start address to calculate ECC
 *
 * @return
 * 		- FMSH_SUCCESS for successful ECC Initialization
 * 		- errors as mentioned in fsbl_error.h
 *
 *****************************************************************************/
#ifdef __aarch64__
__attribute__((aligned(32))) const u64 s_mem_initValue[2]={0xDEADBEEFDEADBEEFU,0xDEADBEEFDEADBEEFU};
static u32 FmshFsbl_EccInit (u64 DestAddr, u64 LengthBytes)
{
    u32 Status=FMSH_SUCCESS;

    UART_LOG_OUT(DEBUG_DETAILED,
                 "Address 0x%0lx, Length %0lx, ECC initialized \r\n", DestAddr,
                 LengthBytes);
    //Status = FmshFsbl_InitMem((u64)(s_mem_initValue), DestAddr, LengthBytes);
    (void)memset((void*)DestAddr,0x55,LengthBytes);
    return Status;
}
#else
static u32 FmshFsbl_EccInit (u32 DestAddr, u32 LengthBytes)
{
    u32 Status=FMSH_SUCCESS;
    UART_LOG_OUT(DEBUG_DETAILED,
                 "Address 0x%0lx, Length %0lx, ECC initialized \r\n", DestAddr,
                 LengthBytes);
    (void)memset((void*)DestAddr,0x55,LengthBytes);
    return Status;
}
#endif
#define DMA_MAX_TRAN_BYTE_LEN    (0x4000000)
static u32 FmshFsbl_DdrEccInit(void)
{
	u32 Status= FMSH_SUCCESS;
#if 0 //FPAR_PSU_DDRC_0_HAS_ECC
	u64 DestAddr = FSBL_PS_DDR_START_ADDRESS;
        u64 NumBytes = 0;
#if DCACHE_ENABLE == 1
    Fmsh_DCacheDisable();
#endif
	UART_LOG_OUT(DEBUG_INFO,"Initializing DDR ECC\n\r");
        NumBytes=DDR_SIZE>FSBL_PS_DDR_END_ADDRESS?(FSBL_PS_DDR_END_ADDRESS-FSBL_PS_DDR_START_ADDRESS+1):DDR_SIZE;
        (void)memset((void*)DestAddr,0x55,NumBytes);
        
#ifdef FSBL_PS_HI_DDR_START_ADDRESS
#if DDR_SIZE>FSBL_PS_DDR_END_ADDRESS
        DestAddr = FSBL_PS_HI_DDR_START_ADDRESS;
        NumBytes=DDR_SIZE-FSBL_PS_DDR_END_ADDRESS-1;
        (void)memset((void*)DestAddr,0x55,NumBytes);

#endif
#endif
#if DCACHE_ENABLE == 1
    Fmsh_DCacheEnable();
#endif
#else
	Status = FMSH_SUCCESS;
#endif
    return Status;
}

/*****************************************************************************/
/**
 * This function does ECC Initialization of TCM memory
 *
 * @param FsblInstancePtr is pointer to the Fsbl Instance
 * @param CpuId One of R5-0, R5-1, R5-LS, A53-0
 *
 * @return
 * 		- FMSH_SUCCESS for successful ECC Initialization
 * 		-               or if ECC Initialization is not enabled
 * 		- errors as mentioned in fsbl_error.h
 *
 *****************************************************************************/
u32 FmshFsbl_TcmEccInit (BootPs *BootInstancePtr, u32 CpuId)
{
    u32 Status = FMSH_FAILURE;
    u32 LengthBytes = 0U;
    u32 ATcmAddr = 0U;
    u32 BTcmAddr = 0U;
    u32 EccInitStatus = 0U;
    u8 FlagReduceAtcmLength = FALSE;

#if DCACHE_ENABLE == 1
    Fmsh_DCacheDisable();
#endif
    UART_LOG_OUT(DEBUG_INFO, "Initializing TCM ECC\n\r");

    /**
     * If for A53, TCM ECC need to be initialized, do it for all banks
     * of TCM.for R5-L,R5-0 processor don't initialize initial 32 bytes of TCM.
     * For R5-0,R5-1 initialize corresponding banks of TCM.*/

    /**
     * For R5-L,R5-0 don't initialize initial 32 bytes of TCM,
     * because initial 32 bytes are holding R5 vectors.
     */

    if (CpuId == ATTRB_DEST_CPU_A53_0)
    {
        ATcmAddr = FSBL_R50_HIGH_ATCM_START_ADDRESS;
        LengthBytes = FSBL_R5_TCM_BANK_LENGTH * 4U;
        Status = FmshFsbl_EccInit(ATcmAddr, LengthBytes);
        EccInitStatus = FSBL_R50_TCM_ECC_INIT_STATUS |
                        FSBL_R51_TCM_ECC_INIT_STATUS;
    }
    else if (CpuId == ATTRB_DEST_CPU_R5_L)
    {
        if (BootInstancePtr->ProcessorID != ATTRB_DEST_CPU_A53_0)
        {
            ATcmAddr = FSBL_R50_HIGH_ATCM_START_ADDRESS +
                       32U; /* Not to overwrite R5 vectors */
            LengthBytes = (FSBL_R5_TCM_BANK_LENGTH * 4U) - 32U;
        }
        else
        {
            ATcmAddr = FSBL_R50_HIGH_ATCM_START_ADDRESS;
            LengthBytes = FSBL_R5_TCM_BANK_LENGTH * 4U;
        }
        Status = FmshFsbl_EccInit(ATcmAddr, LengthBytes);
        EccInitStatus = FSBL_R50_TCM_ECC_INIT_STATUS |
                        FSBL_R51_TCM_ECC_INIT_STATUS;
    }
    else
    {
        if (CpuId == ATTRB_DEST_CPU_R5_0)
        {
            if (BootInstancePtr->ProcessorID != ATTRB_DEST_CPU_A53_0)
            {
                ATcmAddr = FSBL_R50_HIGH_ATCM_START_ADDRESS +
                           32U; /* Not to overwrite R5 vectors */
                BTcmAddr = FSBL_R50_HIGH_BTCM_START_ADDRESS;
                LengthBytes = FSBL_R5_TCM_BANK_LENGTH;
                FlagReduceAtcmLength = TRUE;
            }
            else
            {
                ATcmAddr = FSBL_R50_HIGH_ATCM_START_ADDRESS;
                BTcmAddr = FSBL_R50_HIGH_BTCM_START_ADDRESS;
                LengthBytes = FSBL_R5_TCM_BANK_LENGTH;
            }
            EccInitStatus = FSBL_R50_TCM_ECC_INIT_STATUS;
        }
        else if (CpuId == ATTRB_DEST_CPU_R5_1)
        {
            ATcmAddr = FSBL_R51_HIGH_ATCM_START_ADDRESS;
            BTcmAddr = FSBL_R51_HIGH_BTCM_START_ADDRESS;
            EccInitStatus = FSBL_R51_TCM_ECC_INIT_STATUS;
            LengthBytes = FSBL_R5_TCM_BANK_LENGTH;
        }
        else
        {
            /* for MISRA-C */
            ATcmAddr = 0U;
            BTcmAddr = 0U;
            EccInitStatus = 0U;
            LengthBytes = 0U;
        }

        if (FlagReduceAtcmLength == TRUE)
        {
            Status = FmshFsbl_EccInit(ATcmAddr, LengthBytes - 32U);
        }
        else
        {
            Status = FmshFsbl_EccInit(ATcmAddr, LengthBytes);
        }

        if (FMSH_SUCCESS == Status)
        {
            Status = FmshFsbl_EccInit(BTcmAddr, LengthBytes);
        }
    }

    if (FMSH_SUCCESS == Status)
    {
        /* Indicate in flag that TCM ECC is initialized */
        BootInstancePtr->TcmEccInitStatus = EccInitStatus;
    }
    else
    {
        Status = FSBL_ERROR_TCM_ECC_INIT;
        UART_LOG_OUT(DEBUG_INFO, "TCM ECC INIT FAILED!\n\r");
        goto END;
    }

END:
#if DCACHE_ENABLE == 1
    Fmsh_DCacheEnable();
#endif  
    return Status;
}

/*****************************************************************************/
/**
 * This function adds additional steps for TCM ECC Initialization for A53.
 * These are to power-up TCM before actual ECC calculation and after it is done,
 * to keep RPU in reset
 *
 * @param FsblInstancePtr is pointer to the Fsbl Instance
 *
 * @return
 * 		- FMSH_SUCCESS for success
 * 		- errors as mentioned in fsbl_error.h
 *
 *****************************************************************************/
static u32 FmshFsbl_TcmInit (BootPs *BootInstancePtr)
{
    u32 Status = FMSH_SUCCESS;
    u32 RegValue = 0U;

    if (BootInstancePtr->ProcessorID == ATTRB_DEST_CPU_A53_0)
    {
#ifdef FSBL_A53_TCM_ECC_EXCLUDE        
        Status = FMSH_SUCCESS;
        return Status;
#endif
    }

    if (BootInstancePtr->ProcessorID == ATTRB_DEST_CPU_A53_0)
    {
        /* If TCM ECC has to be initialized for A53, power it up first */
        Status = FmshFsbl_PowerUpMemory(FSBL_R5_L_TCM);
        if (Status != FMSH_SUCCESS)
        {
            return Status;
        }
    }

    /* Do ECC Initialization of TCM if required */
    Status = FmshFsbl_TcmEccInit(BootInstancePtr, BootInstancePtr->ProcessorID);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    /* Place the RPU back in reset, to let user power it up when required */
    if (BootInstancePtr->ProcessorID == ATTRB_DEST_CPU_A53_0)
    {
        RegValue = FMSH_ReadReg(CRL_APB_BASEADDR, 0x23c);
        RegValue |= (CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK |
                     CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK );
        FMSH_WriteReg(CRL_APB_BASEADDR, 0x23c, RegValue);
    }

    return Status;
}

/******************************************************************************
 * This function is initializes the processor and system.
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @return
 *          - returns the error codes described in error.h on any error
 * 			- returns SUCCESS on success
 *
 *****************************************************************************/
u32 FmshFsbl_BootInitialize (BootPs *BootInstancePtr)
{
    u32 Status = FMSH_SUCCESS;

    BootInstancePtr->ResetReason = FmshFsbl_GetResetReason();

    /**
     * Configure the system as in PSU
     */
    if (BootInstancePtr->ResetReason != FSBL_MASTER_ONLY_RESET)
    {
        Status = FmshFsbl_SystemInit(BootInstancePtr);
        if (FMSH_SUCCESS != Status)
        {
            return Status;
        }
    }

    /* Initialize the processor */
    Status = FmshFsbl_ProcessorInit(BootInstancePtr);
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }

    if (FSBL_MASTER_ONLY_RESET != BootInstancePtr->ResetReason)
    {
        /* Do ECC Initialization of TCM if required */
        Status = FmshFsbl_TcmInit(BootInstancePtr);
        if (FMSH_SUCCESS != Status)
        {
            return Status;
        }

        /*
         * Read PMU register bit value that indicates DDR is in self
         * refresh mode.
         */
        /*
         * Skip ECC initialization if DDR is in self refresh
         * mode.
         */

        /* Do ECC Initialization of DDR if required */
        Status = FmshFsbl_DdrEccInit();
        if (FMSH_SUCCESS != Status)
        {
            return Status;
        }
        /* Do board specific initialization if any */
        Status = FmshFsbl_BoardInit();
        if (FMSH_SUCCESS != Status)
        {
            return Status;
        }
    }

#if DEBUG_PERF
    gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
    gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
    UART_LOG_OUT(DEBUG_PERF, "Processor Initialization Done at the %.2f ms\r\n",
                 gtc_time);
#endif

    return Status;
}

/******************************************************************************
 * This function initializes the primary boot device
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @return	returns the error codes described in error.h on any error
 * 			returns SUCCESS on success
 *
 ******************************************************************************/
static u32 FmshFsbl_PrimaryBootDeviceInit (BootPs *BootInstancePtr)
{
    u32 Status = FMSH_SUCCESS;
    u32 DeviceFlags = 0;
    /**
     * Read Boot Mode register and update the value
     */
    BootInstancePtr->PrimaryBootDevice = ReadReg(CRL_APB_BOOT_MODE_USER) &
                                         CRL_APB_BOOT_MODE_USER_BOOT_MODE_MASK;

#ifdef FSBL_PARTITION_LOAD_EXCLUDE
    BootInstancePtr->BootMode = JTAG;
    BootInstancePtr->SecureModeFlag = 0;
    Status = BOOT_STATUS_JTAG;
#else
    /**
     * Enable drivers only if they are device boot modes
     * Not required for JTAG modes
     */
    switch (BootInstancePtr->PrimaryBootDevice)
    {
    case JTAG_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "In JTAG Boot Mode \r\n");
        BootInstancePtr->BootMode = JTAG;
        BootInstancePtr->SecureModeFlag = 0;
        Status = BOOT_STATUS_JTAG;
    }
    break;

#ifdef FSBL_QSPI
    case QSPI24_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "QSPI24 Boot Mode \r\n");
        /* Update the deviceops structure with necessary values*/
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitQspi;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_QspiAccess;
        BootInstancePtr->BootMode = QSPI_FLASH;
        BootInstancePtr->SearchRange = QSPI_24B_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;

    case QSPI32_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "QSPI32 Boot Mode \r\n");
        /* Update the deviceops structure with necessary values*/
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitQspi;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_QspiAccess;
        BootInstancePtr->BootMode = QSPI_FLASH;
        BootInstancePtr->SearchRange = QSPI_32B_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_NAND
    case NAND_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "NAND Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitNand;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_NandAccess;
        BootInstancePtr->BootMode = NAND_FLASH;
        BootInstancePtr->SearchRange = NAND_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_SD_0
    case EMMC_BOOT_MODE:
    case SD0_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "SD0/eMMC Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitSd;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_SdAccess;
        BootInstancePtr->BootMode = SD_CARD;
        BootInstancePtr->SearchRange = SD_LIMITATION;
        DeviceFlags = FSBL_SD_DRV_NUM_0;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_SD_1
    case SD1_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "SD1 Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitSd;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_SdAccess;
        BootInstancePtr->BootMode = SD_CARD;
        BootInstancePtr->SearchRange = SD_LIMITATION;
        DeviceFlags = FSBL_SD_DRV_NUM_1;
        Status = FMSH_SUCCESS;
    }
    break;
    case SD1_LS_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "SD1 LS Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitSd;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_SdAccess;
        BootInstancePtr->BootMode = SD_CARD;
        BootInstancePtr->SearchRange = SD_LIMITATION;
        DeviceFlags = FSBL_SD_DRV_NUM_1;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_USB
    case USB0_BOOT_MODE:
    {
        UART_LOG_OUT(DEBUG_INFO, "USB Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitDfu;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_UsbAccess;
        BootInstancePtr->BootMode = USB_CON;
        BootInstancePtr->SearchRange = USB_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

    default:
    {
        UART_LOG_OUT(DEBUG_INFO, "BOOT ERROR: UNSUPPORTED BOOT MODE\r\n");
        BootInstancePtr->ErrorCode = FSBL_ERROR_INVALID_BOOT_MODE;
        Status = FMSH_FAILURE;
    }
    break;
    }
#endif

    /*In case of error or Jtag boot*/
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }

    /**
     * Initialize the Device Driver
     */
    Status = BootInstancePtr->DeviceOps.DeviceInit(DeviceFlags);
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO, "Boot device initialization failed......\r\n");
        switch (BootInstancePtr->PrimaryBootDevice)
        {
        case QSPI24_BOOT_MODE:
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_QSPI_24B_INIT;
        }
        break;
        case QSPI32_BOOT_MODE:
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_QSPI_32B_INIT;
        }
        break;
        case NAND_BOOT_MODE:
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_NAND_INIT;
        }
        break;
        case USB0_BOOT_MODE:
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_USB_INIT;
        }
        break;

        case SD0_BOOT_MODE:
        case SD1_BOOT_MODE:
        case SD1_LS_BOOT_MODE:
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_SD_INIT;
        }
        break;
        case EMMC_BOOT_MODE:
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_EMMC_INIT;
        }
        break;
        default:
            break;
        }
        return Status;
    }

    UART_LOG_OUT(DEBUG_INFO, "Boot device  initialization success......\r\n");

    return Status;
}

/******************************************************************************
 * This function initializes the primary boot device
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @return	returns the error codes described in error.h on any error
 * 			returns SUCCESS on success
 *
 ******************************************************************************/
static u32 FmshFsbl_SecondaryBootDeviceInit (BootPs *BootInstancePtr)
{
    u32 Status = FMSH_SUCCESS;
    u32 DeviceFlags = 0U;
    /**
     * Enable drivers only if they are device boot modes
     * Not required for JTAG modes
     */
    switch (BootInstancePtr->SecondaryBootDevice)
    {
    case IH_IHT_PPD_SAME:
        return Status;
        break;

#ifdef FSBL_QSPI
    case IH_IHT_PPD_QSPI24:
    {
        UART_LOG_OUT(DEBUG_INFO, "QSPI24 Boot Mode \r\n");
        /* Update the deviceops structure with necessary values*/
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitQspi;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_QspiAccess;
        BootInstancePtr->BootMode = QSPI_FLASH;
        BootInstancePtr->SearchRange = QSPI_24B_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;

    case IH_IHT_PPD_QSPI32:
    {
        UART_LOG_OUT(DEBUG_INFO, "QSPI32 Boot Mode \r\n");
        /* Update the deviceops structure with necessary values*/
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitQspi;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_QspiAccess;
        BootInstancePtr->BootMode = QSPI_FLASH;
        BootInstancePtr->SearchRange = QSPI_32B_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_NAND
    case IH_IHT_PPD_NAND:
    {
        UART_LOG_OUT(DEBUG_INFO, "NAND Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitNand;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_NandAccess;
        BootInstancePtr->BootMode = NAND_FLASH;
        BootInstancePtr->SearchRange = NAND_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_SD_0
    case IH_IHT_PPD_MMC:
    case IH_IHT_PPD_SD_0:
    {
        UART_LOG_OUT(DEBUG_INFO, "SD0/eMMC Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitSd;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_SdAccess;
        BootInstancePtr->BootMode = SD_CARD;
        BootInstancePtr->SearchRange = SD_LIMITATION;
        DeviceFlags = FSBL_SD_DRV_NUM_0;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_SD_1
    case IH_IHT_PPD_SD_1:
    {
        UART_LOG_OUT(DEBUG_INFO, "SD1 Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitSd;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_SdAccess;
        BootInstancePtr->BootMode = SD_CARD;
        BootInstancePtr->SearchRange = SD_LIMITATION;
        DeviceFlags = FSBL_SD_DRV_NUM_1;
        Status = FMSH_SUCCESS;
    }
    break;
    case IH_IHT_PPD_SD_LS:
    {
        UART_LOG_OUT(DEBUG_INFO, "SD1 LS Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitSd;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_SdAccess;
        BootInstancePtr->BootMode = SD_CARD;
        BootInstancePtr->SearchRange = SD_LIMITATION;
        DeviceFlags = FSBL_SD_DRV_NUM_1;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

#ifdef FSBL_USB
    case IH_IHT_PPD_USB:
    {
        UART_LOG_OUT(DEBUG_INFO, "USB Boot Mode \r\n");
        /* Update the deviceops structure with necessary values */
        BootInstancePtr->DeviceOps.DeviceInit = FmshFsbl_InitDfu;
        BootInstancePtr->DeviceOps.DeviceCopy = FmshFsbl_UsbAccess;
        BootInstancePtr->BootMode = SD_CARD;
        BootInstancePtr->SearchRange = SD_LIMITATION;
        Status = FMSH_SUCCESS;
    }
    break;
#endif

    default:
    {
        UART_LOG_OUT(DEBUG_INFO, "BOOT ERROR: UNSUPPORTED BOOT MODE\r\n");
        BootInstancePtr->ErrorCode = FSBL_ERROR_INVALID_BOOT_MODE;
        Status = FMSH_FAILURE;
    }
    break;
    }

    /*In case of error*/
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }

    /**
     * Initialize the Device Driver
     */
    Status = BootInstancePtr->DeviceOps.DeviceInit(DeviceFlags);
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "Second Boot device initialization failed......\r\n");
        return Status;
    }

    UART_LOG_OUT(DEBUG_INFO,
                 "Second Boot device initialization success......\r\n");

    Status = FmshFsbl_ValidateHeader(BootInstancePtr);

    return Status;
}

/******************************************************************************
 * This function initializes the primary and secondary boot devices
 * and validates the image header
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @return	returns the error codes described in fmsh_error.h on any error
 * 			returns FMSH_SUCCESS on success
 ******************************************************************************/
u32 FmshFsbl_BootDeviceInitAndValidate (BootPs *BootInstancePtr)
{
    u32 Status = FMSH_SUCCESS;

    /**
     * Configure the  boot device
     */
    UART_LOG_OUT(DEBUG_DETAILED,
                 "Preparing boot device initialization......\r\n");
    Status = FmshFsbl_PrimaryBootDeviceInit(BootInstancePtr);
    if (FMSH_SUCCESS != Status)
    {
        goto END;
    }

#if DEBUG_PERF
    gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
    gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
    UART_LOG_OUT(DEBUG_PERF,
                 "Boot device initialization is done at the %.2f ms\r\n",
                 gtc_time);
#endif

    /**
     * Validate the header
     */
    Status = FmshFsbl_ValidateHeader(BootInstancePtr);
    if (FMSH_SUCCESS != Status)
    {
        goto END;
    }

    /**
     * Update the secondary boot device
     */
    BootInstancePtr->SecondaryBootDevice = BootInstancePtr->ImageHeader
                                               .ImageHeaderTable
                                               .PartitionPresentDevice;

    /**
     *  Configure the secondary boot device if required
     */
    Status = FmshFsbl_SecondaryBootDeviceInit(BootInstancePtr);
    if (FMSH_SUCCESS != Status)
    {
        goto END;
    }
  

END:

#if DEBUG_PERF
    gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
    gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
    UART_LOG_OUT(DEBUG_PERF, "Boot header validate is done at the %.2f ms\r\n",
                 gtc_time);
#endif

#if FSBL_WDT_ENABLE
    (void)FWdtPs_restart(&g_WDT);
#endif

    return Status;
}
