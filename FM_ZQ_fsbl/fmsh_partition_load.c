/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_partition_load.c
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
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
extern FDevcPs_T g_DEVC;
/***************** Macros (Inline Functions) Definitions *********************/
// #define FSBL_IVT_LENGTH	(u32)(0x20U)
// #define FSBL_R5_HIVEC    	(u32)(0xffff0000U)
// #define FSBL_R5_LOVEC		(u32)(0x0U)
#define FSBL_SET_R5_SCTLR_VECTOR_BIT (u32)(1 << 13)

/************************** Variable Definitions *****************************/
extern u32 BootHeaderSize;  // header.c
extern u32 FlashReadBaseAddress;
extern BootPs BootInstance;
extern u8 *BootHdr;

#ifdef ARMR5
u8 R5LovecBuffer[32] = {0U};
u8 R5HivecBuffer[32] = {0U};
u32 TcmSkipLength = 0U;
UINTPTR TcmSkipAddress = 0U;
u8 IsR5IvtBackup = FALSE;
#endif

u8 AuthBuffer[RSA_SIGNATURE_SIZE] __attribute__((aligned(4))) = {0};
EncData EncryptionData;

/************************** Function Prototypes ******************************/
#ifdef ARMR5

/*****************************************************************************/
/**
 * This function set the vector bit of SCTLR.
 * It will configure R5,so that R5 will jump to
 * HIVEC when exception arise.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
static void FmshFsbl_SetR5ExcepVectorHiVec (void)
{
    u32 RegVal;
    // mfcp(REG_CP15_SYS_CONTROL,RegVal);
    RegVal |= FSBL_SET_R5_SCTLR_VECTOR_BIT;
    // mtcp(REG_CP15_SYS_CONTROL,RegVal);
}

/*****************************************************************************/
/**
 * This function reset the vector bit of SCTLR.
 * It will configure R5,so that R5 will jump to
 * LOVEC when exception arise.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/

static void FmshFsbl_SetR5ExcepVectorLoVec (void)
{
    u32 RegVal;
    mfcp(REG_CP15_SYS_CONTROL, RegVal);
    RegVal &= (~(FSBL_SET_R5_SCTLR_VECTOR_BIT));
    mtcp(REG_CP15_SYS_CONTROL, RegVal);
}

#endif
/*****************************************************************************/
/**
 *
 * This function checks the power state of one or more power islands and
 * powers them up if required.
 *
 * @param	Mask of Island(s) that need to be powered up
 *
 * @return	FMSH_SUCCESS for successful power up or
 * 		FMSH_FAILURE otherwise.
 *
 * @note		None.
 *
 ****************************************************************************/
u32 FmshFsbl_PowerUpIsland (u32 PwrIslandMask)
{
    u32 RegVal;
    u32 Status = FMSH_SUCCESS;

    /* Skip power-up request for QEMU */
    // if (XGet_Zynq_UltraMp_Platform_info() != (u32)XPLAT_ZYNQ_ULTRA_MPQEMU)
    {
        /* There is a single island for both R5_0 and R5_1 */
        if ((PwrIslandMask & PMU_GLOBAL_PWR_STATE_R5_1_MASK) ==
            PMU_GLOBAL_PWR_STATE_R5_1_MASK)
        {
            PwrIslandMask &= ~(PMU_GLOBAL_PWR_STATE_R5_1_MASK);
            PwrIslandMask |= PMU_GLOBAL_PWR_STATE_R5_0_MASK;
        }

        /* Power up request enable */
        WriteReg(PMU_GLOBAL_REQ_PWRUP_INT_EN, PwrIslandMask);

        /* Trigger power up request */
        WriteReg(PMU_GLOBAL_REQ_PWRUP_TRIG, PwrIslandMask);

        /* Poll for Power up complete */
        do
        {
            RegVal = ReadReg(PMU_GLOBAL_REQ_PWRUP_STATUS) & PwrIslandMask;
        } while (RegVal != 0x0U);
    }

    return Status;
}
/*****************************************************************************/
/**
 * This function checks the power state and reset for the memory type
 * and release the reset if required
 *
 * @param	MemoryType is the memory to be checked
 * 			- FSBL_R5_0_TCM
 * 			- FSBL_R5_1_TCM
 *				(to be added)
 *			- FSBL_R5_0_TCMA
 *			- FSBL_R5_0_TCMB
 *			- FSBL_PS_DDR
 *			- FSBL_PS_DDR
 *
 * @return	none
 *****************************************************************************/
u32 FmshFsbl_PowerUpMemory (u32 MemoryType)
{
    u32 RegValue = 0U;
    u32 Status = FMSH_FAILURE;
    u32 PwrStateMask = 0U;

    /**
     * Check the power status of the memory
     * Power up if required
     *
     * Release the reset of the memory if present
     */
    switch (MemoryType)
    {
    case FSBL_R5_0_TCM:
    {
        PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_0_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM0A_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM0B_MASK);

        Status = FmshFsbl_PowerUpIsland(PwrStateMask);

        if (Status != FMSH_SUCCESS)
        {
            Status = FSBL_ERROR_R5_0_TCM_POWER_UP;
            UART_LOG_OUT(DEBUG_INFO, "R5_0_TCM power up failed!\r\n");
            return Status;
        }

        /**
         * To access TCM,
         * 	Release reset to R5 and enable the clk
         * 	R5 is under halt state
         *
         * 	If R5 are out of reset and clk is enabled so doing
         * 	again is no issue. R5 might be under running state
         */

        /**
         * Place R5, TCM in split mode
         */
        RegValue = ReadReg(RPU_RPU_GLBL_CNTL);
        RegValue |= RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
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
        RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
        WriteReg(CRL_APB_CPU_R5_CTRL, RegValue);

        /**
         * Provide some delay,
         * so that clock propagates properly.
         */
        delay_us(0x50U);

        /**
         * Release reset to R5-0
         */
        RegValue = ReadReg(CRL_APB_RST_LPD_TOP);
        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
        WriteReg(CRL_APB_RST_LPD_TOP, RegValue);
    }
    break;

    case FSBL_R5_1_TCM:
    {
        PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_1_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM1A_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM1B_MASK);

        Status = FmshFsbl_PowerUpIsland(PwrStateMask);

        if (Status != FMSH_SUCCESS)
        {
            Status = FSBL_ERROR_R5_1_TCM_POWER_UP;
            UART_LOG_OUT(DEBUG_INFO, "R5_1_TCM power up failed!\r\n");
            return Status;
        }

        /**
         * Place R5 in split mode
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
        delay_us(0x50U);

        /**
         * Release reset to R5-1
         */
        RegValue = ReadReg(CRL_APB_RST_LPD_TOP);
        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
        WriteReg(CRL_APB_RST_LPD_TOP, RegValue);
    }
    break;

    case FSBL_R5_L_TCM:
    {
        PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_0_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM0A_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM0B_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM1A_MASK |
                        PMU_GLOBAL_PWR_STATE_TCM1B_MASK);

        Status = FmshFsbl_PowerUpIsland(PwrStateMask);

        if (Status != FMSH_SUCCESS)
        {
            Status = FSBL_ERROR_R5_L_TCM_POWER_UP;
            UART_LOG_OUT(DEBUG_INFO, "R5_L_TCM power up failed!\r\n");
            return Status;
        }

        /**
         * Place R5 in lock step mode
         * Combine TCM's
         */
        RegValue = ReadReg(RPU_RPU_GLBL_CNTL);
        RegValue |= RPU_RPU_GLBL_CNTL_SLCLAMP_MASK;
        RegValue &= ~(RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
        RegValue |= RPU_RPU_GLBL_CNTL_TCM_COMB_MASK;
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
         * Release reset to R5-0,R5-1
         */
        RegValue = ReadReg(CRL_APB_RST_LPD_TOP);
        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
        RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
        WriteReg(CRL_APB_RST_LPD_TOP, RegValue);
    }
    break;

    default:
        /* nothing to do */
        Status = FMSH_SUCCESS;
        break;
    }

    return Status;
}

/**
 * This function validates the partition
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @param	LoadAddress Load address of partition
 *
 * @param	PartitionNum is the partition number to calculate checksum
 *
 * @param	ShaType is either SHA2/SHA3
 *
 * @return	returns FMSH_SUCCESS on success
 * 			returns FMSH_FAILURE on failure
 *
 *****************************************************************************/
static u32 FmshFsbl_CalcualteSHA (const BootPs *BootInstancePtr,
                                  UINTPTR LoadAddress, u32 PartitionNum)
{
    u8 PartitionHash[HASH_TYPE_SHA3] __attribute__((aligned(4))) = {0};
    u8 Hash[HASH_TYPE_SHA3] __attribute__((aligned(4))) = {0};
    u32 HashOffset;
    u32 Index = 0U;
    u32 Length = 0U;
    u32 Status = FMSH_SUCCESS;
    u32 HashLen = 0U;
    const Ps_PartitionHeader *PartitionHeader;

    /**
     * Update the variables
     */
    PartitionHeader = &BootInstancePtr->ImageHeader
                           .PartitionHeader[PartitionNum];
    Length = PartitionHeader->TotalDataWordLength * 4U - 0x20;

    HashOffset = BootInstancePtr->ImageOffsetAddress + PartitionHeader->ChecksumWordOffset * 4U;

    if ((PartitionHeader->PartitionAttributes & IH_PH_ATTRB_CHECKSUM_MASK) !=
        IH_PH_ATTRB_NOCHECKSUM)
    {
        HashLen = HASH_TYPE_SHA2;
        /* Start the SHA engine */
        Status = FmshFsbl_sha256((u8 *)LoadAddress, Length, PartitionHash);
        if (Status != FMSH_SUCCESS)
        {
            return FMSH_FAILURE;
        }
    }
    /*  else if( (PartitionHeader->PartitionAttributes &
IH_PH_ATTRB_CHECKSUM_MASK) == IH_PH_ATTRB_HASH_SHA3 )
    {
    HashLen= HASH_TYPE_SHA3;
    FmshFsbl_sha384((u8*)LoadAddress, Length, PartitionHash);
}
    */
    Status = BootInstancePtr->DeviceOps.DeviceCopy(HashOffset, (UINTPTR)Hash,
                                                   HashLen);

    if (Status != FMSH_SUCCESS)
    {
        UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_HASH_COPY_FAILED \r\n");
        return FMSH_FAILURE;
    }
    for (Index = 0U; Index < HashLen; Index++)
    {
        if (PartitionHash[Index] != Hash[Index])
        {
            UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_HASH_FAILED \r\n");
            return FMSH_FAILURE;
        }
    }
    return Status;
}
/****************************************************************************/
/**
 * This function is used to check whether cpu has handoff address stored
 * in the handoff structure
 *
 * @param FsblInstancePtr is pointer to the Fsbl Instance
 *
 * @param DestinationCpu is the cpu which needs to be checked
 *
 * @return
 * 		- FMSH_SUCCESS if cpu handoff address is not present
 * 		- FMSH_FAILURE if cpu handoff address is present
 *
 * @note
 *
 *****************************************************************************/
static u32 FmshFsbl_CheckHandoffCpu (BootPs *BootInstancePtr,
                                     u32 DestinationCpu)
{
    u32 ValidHandoffCpuNo = 0U;
    u32 Status = FMSH_SUCCESS;
    u32 Index = 0U;
    u32 CpuId = 0U;

    ValidHandoffCpuNo = BootInstancePtr->HandoffCpuNo;

    for (Index = 0U; Index < ValidHandoffCpuNo; Index++)
    {
        CpuId = BootInstancePtr->HandoffValues[Index].CpuSettings &
                IH_PH_ATTRB_DEST_CPU_MASK;
        if (CpuId == DestinationCpu)
        {
            Status = FMSH_FAILURE;
            break;
        }
    }
    return Status;
}

static u32 FmshFsbl_GetLoadAddress (u32 DestinationCpu, UINTPTR *LoadAddressPtr,
                                    u32 Length)
{
    u32 Status;
    u32 Address;

    Address = *LoadAddressPtr;

    /* Update for R50 TCM address if the partition fits with in a TCM bank */
    if ((DestinationCpu == IH_PH_ATTRB_DEST_CPU_R5_0) &&
        ((Address < (FSBL_R5_TCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH)) ||
         ((Address >= FSBL_R5_BTCM_START_ADDRESS) &&
          (Address < (FSBL_R5_BTCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH)))))
    {
        /* Check if fits in to a single TCM bank or not */
        if (Length > FSBL_R5_TCM_BANK_LENGTH)
        {
            Status = FSBL_ERROR_LOAD_ADDRESS;
            UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_LOAD_ADDRESS\r\n");
            goto END;
        }

        /**
         * Update Address to the higher TCM address
         */
        Address = FSBL_R50_HIGH_ATCM_START_ADDRESS + Address;
    }
    else
        /* Update for R51 TCM address if the partition fits with in a TCM bank
         */
        if ((DestinationCpu == IH_PH_ATTRB_DEST_CPU_R5_1) &&
            ((Address <
              (FSBL_R5_TCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH)) ||
             ((Address >= FSBL_R5_BTCM_START_ADDRESS) &&
              (Address <
               (FSBL_R5_BTCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH)))))
        {
            /* Check if fits in to a single TCM bank or not */
            if (Length > FSBL_R5_TCM_BANK_LENGTH)
            {
                Status = FSBL_ERROR_LOAD_ADDRESS;
                UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_LOAD_ADDRESS\r\n");
                goto END;
            }
            /**
             * Update Address to the higher TCM address
             */
            Address = FSBL_R51_HIGH_ATCM_START_ADDRESS + Address;
        }
        else
            /**
             * Update for the R5-L TCM address
             */
            if ((DestinationCpu == IH_PH_ATTRB_DEST_CPU_R5_L) &&
                (Address <
                 (FSBL_R5_TCM_START_ADDRESS + (FSBL_R5_TCM_BANK_LENGTH * 4U))))
            {
                /**
                 * Check if fits to TCM or not
                 */
                if (Length > (FSBL_R5_TCM_BANK_LENGTH * 4U))
                {
                    Status = FSBL_ERROR_LOAD_ADDRESS;
                    UART_LOG_OUT(DEBUG_INFO, "FSBL_ERROR_LOAD_ADDRESS\r\n");
                    goto END;
                }
                /**
                 * Update Address to the higher TCM address
                 */
                Address = FSBL_R50_HIGH_ATCM_START_ADDRESS + Address;
            }
            else
            {
                /**
                 * For MISRA complaince
                 */
            }

    /**
     * Update the LoadAddress
     */
    *LoadAddressPtr = Address;

    Status = FMSH_SUCCESS;
END:
    return Status;
}
/*****************************************************************************/
/**
 * This function calculates the load address based on the destination
 * cpu. For R5 cpu's TCM address is remapped to the higher TCM address
 * so that any cpu can globally access it
 *
 * @param	DestinationCpu is the cpu which partition will run
 *
 * @param	LoadAddress will be updated according to the cpu and address
 *
 * @param	Length of the data to be copied. This is required only to
 *              check for the error case
 *
 * @return	returns the error codes described in fsbl_error.h on any error
 * 			returns FMSH_SUCCESS on success
 *****************************************************************************/
static u32 FmshFsbl_ConfigureMemory (BootPs *BootInstancePtr, u32 RunningCpu,
                                     u32 DestinationCpu, u64 Address)
{
    u32 Status;
    /**
     * Configure R50 TCM Memory
     */
    if ((DestinationCpu == IH_PH_ATTRB_DEST_CPU_R5_0) &&
        (((Address >= FSBL_R50_HIGH_ATCM_START_ADDRESS) &&
          (Address <
           (FSBL_R50_HIGH_ATCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH))) ||
         ((Address >= FSBL_R50_HIGH_BTCM_START_ADDRESS) &&
          (Address <
           (FSBL_R50_HIGH_BTCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH)))))
    {
        /**
         * Power up and release reset to the memory
         */
        if (RunningCpu != DestinationCpu)
        {
            Status = FmshFsbl_PowerUpMemory(FSBL_R5_0_TCM);
            if (Status != FMSH_SUCCESS)
            {
                goto END;
            }
        }

        /**
         * ECC initialize TCM
         */
        if ((BootInstancePtr->TcmEccInitStatus &
             FSBL_R50_TCM_ECC_INIT_STATUS) == FALSE)
        {
            Status = FmshFsbl_TcmEccInit(BootInstancePtr, DestinationCpu);
            if (FMSH_SUCCESS != Status)
            {
                goto END;
            }
        }
    }
    else
        /**
         * Update for R5-1 TCM address
         */
        if ((DestinationCpu == IH_PH_ATTRB_DEST_CPU_R5_1) &&
            (((Address >= FSBL_R51_HIGH_ATCM_START_ADDRESS) &&
              (Address <
               (FSBL_R51_HIGH_ATCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH))) ||
             ((Address >= FSBL_R51_HIGH_BTCM_START_ADDRESS) &&
              (Address <
               (FSBL_R51_HIGH_BTCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH)))))

        {
            /**
             * Power up and release reset to the memory
             */
            if (RunningCpu != DestinationCpu)
            {
                Status = FmshFsbl_PowerUpMemory(FSBL_R5_1_TCM);
                if (Status != FMSH_SUCCESS)
                {
                    goto END;
                }
            }

            /**
             * ECC initialize TCM
             */
            if ((BootInstancePtr->TcmEccInitStatus &
                 FSBL_R51_TCM_ECC_INIT_STATUS) == FALSE)
            {
                Status = FmshFsbl_TcmEccInit(BootInstancePtr, DestinationCpu);
                if (FMSH_SUCCESS != Status)
                {
                    goto END;
                }
            }
        }
        else
            /**
             * Update for the R5-L TCM address
             */
            if ((DestinationCpu == IH_PH_ATTRB_DEST_CPU_R5_L) &&
                (Address >= FSBL_R50_HIGH_ATCM_START_ADDRESS) &&
                (Address < (FSBL_R50_HIGH_ATCM_START_ADDRESS +
                            (FSBL_R5_TCM_BANK_LENGTH * 4U))))
            {
                /**
                 * Power up and release reset to the memory
                 */
                if (RunningCpu != DestinationCpu)
                {
                    Status = FmshFsbl_PowerUpMemory(FSBL_R5_L_TCM);
                    if (Status != FMSH_SUCCESS)
                    {
                        goto END;
                    }
                }

                /**
                 * ECC initialize TCM
                 */
                if ((BootInstancePtr->TcmEccInitStatus &
                     (FSBL_R50_TCM_ECC_INIT_STATUS |
                      FSBL_R51_TCM_ECC_INIT_STATUS)) !=
                    (FSBL_R50_TCM_ECC_INIT_STATUS |
                     FSBL_R51_TCM_ECC_INIT_STATUS))
                {
                    Status = FmshFsbl_TcmEccInit(BootInstancePtr,
                                                 DestinationCpu);
                    if (FMSH_SUCCESS != Status)
                    {
                        goto END;
                    }
                }
            }
            else
            {
                /**
                 * For MISRA complaince
                 */
            }

    Status = FMSH_SUCCESS;
END:
    return Status;
}
/****************************************************************************/
/**
 * This function copies the partition to specified destination
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @param	PartitionNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in error.h on any error
 * 			returns SUCCESS on success
 *****************************************************************************/
static u32 FmshFsbl_PartitionCopy (BootPs *BootInstancePtr, u32 PartitionNum)
{
    u32 Status = FMSH_SUCCESS;
    Ps_PartitionHeader *PartitionHeaderPtr = NULL;
    u32 SrcAddress = 0U;
    UINTPTR LoadAddress = 0U;
    u32 Length = 0U;
    u32 RunningCpu = 0U;
    u32 DestinationCpu = 0U;
    u32 CpuNo = 0U;
    u32 DestinationDevice = 0U;
    u32 ExecState = 0U;
    u32 RegVal = 0;

#ifdef ARMR5
    u32 Index = 0U;
#endif

    /**
     * Assign the partition header to local variable
     */
    PartitionHeaderPtr = &(
        BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum]);

    RunningCpu = BootInstancePtr->ProcessorID;

    /**
     * Check for XIP image
     * No need to copy for XIP image
     */
    DestinationCpu = FmshFsbl_GetDestinationCpu(PartitionHeaderPtr);

    /**
     * if destination cpu is not present, it means it is for same cpu
     */
    if (DestinationCpu == IH_PH_ATTRB_DEST_CPU_NONE)
    {
        DestinationCpu = BootInstancePtr->ProcessorID;
    }

    /**
     * Get the execution state
     */
    ExecState = FmshFsbl_GetExecState(PartitionHeaderPtr);

    if (PartitionHeaderPtr->UnEncryptedDataWordLength == 0U)
    {
        /**
         * Update the Handoff address only for the first application
         * of that cpu
         * This is for XIP image. For other partitions it handoff
         * address is updated after partition validation
         */
        CpuNo = BootInstancePtr->HandoffCpuNo;
        if (FmshFsbl_CheckHandoffCpu(BootInstancePtr, DestinationCpu) ==
            FMSH_SUCCESS)
        {
            BootInstancePtr->HandoffValues[CpuNo].CpuSettings = DestinationCpu |
                                                                ExecState;
            BootInstancePtr->HandoffValues[CpuNo].PartitionAttributes = PartitionHeaderPtr->PartitionAttributes;
            BootInstancePtr->HandoffValues[CpuNo]
                .HandoffAddress = PartitionHeaderPtr
                                      ->DestinationExecutionAddress;
            BootInstancePtr->HandoffCpuNo += 1U;
        }
        else
        {
            /**
             *
             * if two partitions has same destination cpu, error can
             * be triggered here
             */
        }
        Status = FMSH_SUCCESS;
        return Status;
    }

    /**
     * Get the source(flash offset) address where it needs to copy
     */
    SrcAddress = BootInstancePtr->ImageOffsetAddress +
                 ((PartitionHeaderPtr->DataWordOffset) *
                  IH_PARTITION_WORD_LENGTH);

    /**
     * Length of the partition to be copied
     */
    Length = (PartitionHeaderPtr->TotalDataWordLength) *
             IH_PARTITION_WORD_LENGTH;

    LoadAddress = (UINTPTR)PartitionHeaderPtr->DestinationLoadAddress;

    DestinationDevice = FmshFsbl_GetDestinationDevice(PartitionHeaderPtr);

    /**
     * Copy the PL to temporary DDR Address
     */
    if (DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PL)
    {
#ifdef FSBL_BS
        /**
         *  if pl has bitstream, skip copying the PL bitstream
         */

#ifdef FSBL_PL_SKIP_CONFIGED
        if (ReadReg(SAC_STATUS_REG) & SAC_STATUS_PCFG_DONE_MASK)
        {
            Status = PARTITION_SKIP_LOAD;
            UART_LOG_OUT(DEBUG_INFO,
                         "Bitstream is done , skip copying Partition!\r\n");
            return Status;
        }
#endif
        if (LoadAddress == FSBL_DUMMY_PL_ADDR)
        {
            LoadAddress = FSBL_PL_TEMP_DDRADDR;

#ifndef FSBL_PS_DDR
            /* In case of DDR less system, skip copying */
            return Status;
#endif
        }
#else
        UART_LOG_OUT(DEBUG_INFO, "EXCLUDE Bitsream!!\r\n");
        return FMSH_FAILURE;
#endif  // FSBL_BS
    }
    else if (DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PS)
    {
        /* Copy the partition to OCM */
        LoadAddress += 0;

        if ((LoadAddress > QSPI_LINEAR_BASE_ADDRESS_START) &&
             (LoadAddress < QSPI_LINEAR_BASE_ADDRESS_END))
        {
            UART_LOG_OUT(DEBUG_INFO,
                         "LoadAddress in the linear flash,skip copy!!\r\n");
            return Status;
        }

        /**
         * When destination device is R5-0/R5-1/R5-L and load address is in TCM
         * copy to high address of TCM address map
         * Update the LoadAddress
         */
        Status = FmshFsbl_GetLoadAddress(DestinationCpu, &LoadAddress, Length);
        if (FMSH_SUCCESS != Status)
        {
            return Status;
        }

        /**
         * Configure the memory
         */
        Status = FmshFsbl_ConfigureMemory(BootInstancePtr, RunningCpu,
                                          DestinationCpu, LoadAddress);
        if (FMSH_SUCCESS != Status)
        {
            return Status;
        }

#ifdef ARMR5

        /*Disable IsR5IvtBackup */
        IsR5IvtBackup = FALSE;

        /**
         *
         * Enable IsR5IvtBackup,if FSBL is running in R5-0/R5-L at 0x0 TCM
         * Store HIVEC 32 byte data to R5HivecBuffer,
         * Update the High Vector locations for R5,
         * set Exception Vector to HIVEC,based on above condition.
         */
        if (((BootInstancePtr->ProcessorID == IH_PH_ATTRB_DEST_CPU_R5_0) ||
             (BootInstancePtr->ProcessorID == IH_PH_ATTRB_DEST_CPU_R5_L)) &&
            ((LoadAddress >= FSBL_R50_HIGH_ATCM_START_ADDRESS) &&
             (LoadAddress <
              (FSBL_R50_HIGH_ATCM_START_ADDRESS + FSBL_IVT_LENGTH))))
        {
            /**
             * Enable IsR5IvtBackup,this will used in
             * Fsbl_PartitionLoad for restoring R5 vectors
             */
            IsR5IvtBackup = TRUE;

            /**
             * Get the length of the IVT area to be
             * skipped from Load Address
             */
            TcmSkipAddress = LoadAddress % FSBL_IVT_LENGTH;
            TcmSkipLength = FSBL_IVT_LENGTH - TcmSkipAddress;
            UART_LOG_OUT(DEBUG_INFO,
                         "Fsbl_PartitionCopy:Going for LOVEC HIGHVEC Mechanism "
                         "for R5.\n\r");

            /**
             * Check if Length is less than SkipLength
             */
            if (TcmSkipLength > Length)
            {
                TcmSkipLength = Length;
            }

            /*Store HIVEC 32 bytes data to R5HivecBuffer*/
            (void)memcpy((u8 *)R5HivecBuffer, (u8 *)FSBL_R5_HIVEC,
                         FSBL_IVT_LENGTH);

            /* Update the High Vector locations for R5.*/

            Index = FSBL_R5_HIVEC;
            while (Index < (FSBL_R5_HIVEC + 32U))
            {
                Fmsh_Out32(Index, FSBL_R5_HIVEC_VALUE);
                Index += 4U;
            }

            /**
             * Make sure that Low Vector locations are written Properly
             * Flush the cache
             */
            Fmsh_DCacheFlush();

            /*set exception vector to HIVEC */
            FmshFsbl_SetR5ExcepVectorHiVec();
        }
#endif
    }
    else if(DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PMU)
    {
        /* Trigger IPI only for first PMUFW partition */
        if (PartitionHeaderPtr->DestinationExecutionAddress != 0U)
        {
            /* Enable PMU_0 IPI */
            WriteReg(IPI_PMU_0_IER, IPI_PMU_0_IER_PMU_0_MASK);

            /* Trigger PMU0 IPI in PMU IPI TRIG Reg */
            WriteReg(IPI_PMU_0_TRIG, IPI_PMU_0_TRIG_PMU_0_MASK);
        }

        /**
         * Wait until PMU goes to sleep state,
         * before starting firmware download to PMU RAM
         */
        do
        {
            RegVal = ReadReg(PMU_GLOBAL_GLOBAL_CNTRL);
            if ((RegVal & PMU_GLOBAL_GLOBAL_CNTRL_MB_SLEEP_MASK) ==
                PMU_GLOBAL_GLOBAL_CNTRL_MB_SLEEP_MASK)
            {
                break;
            }
        } while (1);
        delay_1ms();
   }
   else{
        ;/* no deal with */
    }
   
    /**
     * Copy the partition to PS_DDR/PL_DDR/TCM
     */
    Status = BootInstancePtr->DeviceOps.DeviceCopy(SrcAddress, LoadAddress,
                                                   Length);
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO, "Copy Partition failure!!\r\n");
        return Status;
    }
    UART_LOG_OUT(DEBUG_INFO, "Copy Partition success!!\r\n");

    return Status;
}

/****************************************************************************/
/**
 * This function validates the partition
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @param	PartitionNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in error.h on any error
 * 			returns SUCCESS on success
 *
 *****************************************************************************/
static u32 FmshFsbl_PartitionValidation (BootPs *BootInstancePtr,
                                         u32 PartitionNum)
{
    u32 Status = FMSH_SUCCESS;
    u32 LoadAddress = 0U;
    UINTPTR PsLoadAddress = 0U;
    u32 Size = 0U;
    u32 DestinationDevice = 0U;
    u32 DestinationCpu = 0U;
    u32 ExecState = 0U;
    u32 CpuNo = 0U;

#ifdef FSBL_SECURE
    u32 AcOffset = 0U;
    u32 EncryptedKeyIv[3];
    u8 IsEncrypted = FALSE;
    u8 IsPlPartition = 0;
    u32 Index = 0;
#endif

    Ps_PartitionHeader *PartitionHeader = NULL;
    PartitionHeader = &BootInstancePtr->ImageHeader
                           .PartitionHeader[PartitionNum];

    DestinationDevice = FmshFsbl_GetDestinationDevice(PartitionHeader);
    DestinationCpu = FmshFsbl_GetDestinationCpu(PartitionHeader);
    /**
     * if destination cpu is not present, it means it is for same cpu
     */
    if (DestinationCpu == IH_PH_ATTRB_DEST_CPU_NONE)
    {
        DestinationCpu = BootInstancePtr->ProcessorID;
    }

#ifdef FSBL_SECURE
    /**
     * check the authentication status
     */
    if (FmshFsbl_IsRsaSignaturePresent(PartitionHeader) ==
        IH_PH_ATTRB_RSA_SIGNATURE)
    {
        LoadAddress = BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum]
                          .DestinationLoadAddress;

        if (DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PL &&
            LoadAddress == FSBL_DUMMY_PL_ADDR)
        {
            LoadAddress = FSBL_PL_TEMP_DDRADDR;
            IsPlPartition = 0x10;  // pl
        }
        Size = (BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum]
                    .AuthCertificateOffset -
                BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum]
                    .DataWordOffset) *
               4U;
        AcOffset = LoadAddress + Size;

        UART_LOG_OUT(DEBUG_DETAILED, "Partition Authentication Enabled \r\n");

        if (IsPlPartition == 0x10)
        {
            if (FmshFsbl_GetBlockSize(PartitionHeader) == 0x00U)
            {
#ifdef FSBL_PS_DDR
                Status = FmshFsbl_AuthenticatePartition(
                    LoadAddress, Size, AcOffset, IsPlPartition);
                if (Status != FMSH_SUCCESS)
                {
                    BootInstancePtr->ErrorCode = FSBL_ERROR_PARTITION_AUTHENTICATE;
                    return Status;
                }
#else

                LoadAddress = BootInstancePtr->ImageOffsetAddress +
                              ((PartitionHeader->DataWordOffset) *
                               IH_PARTITION_WORD_LENGTH);
                if (BootInstancePtr->BootMode == QSPI_FLASH)
                {
                    LoadAddress += FPS_QSPI0_D_BASEADDR;
                    AcOffset = LoadAddress + Size;
                }
                else if (BootInstancePtr->BootMode == NAND_FLASH ||
                         BootInstancePtr->BootMode == SD_CARD)
                {
                    AcOffset = LoadAddress + Size;
                }

                Status = FmshFsbl_AuthenticatePartition(
                    LoadAddress, Size, AcOffset, IsPlPartition);
                if (Status != FMSH_SUCCESS)
                {
                    BootInstancePtr->ErrorCode = FSBL_ERROR_PARTITION_AUTHENTICATE;
                    return Status;
                }

#endif
            }
            else                       // block
            {
                IsPlPartition = 0x11;  // pl block
#ifdef FSBL_PS_DDR
                for (; LoadAddress < AcOffset; LoadAddress += 0x800000)
                {
                    if (Size >= 0x800000)
                    {
                        Status = FmshFsbl_AuthenticatePartition(
                            LoadAddress, 0x800000,
                            AcOffset + Index * RSA_CERTIFICATE_SIZE,
                            IsPlPartition);
                        if (Status != FMSH_SUCCESS)
                        {
                            BootInstancePtr
                                ->ErrorCode = FSBL_ERROR_PARTITION_AUTHENTICATE;
                            return Status;
                        }
                        Index++;
                        Size -= 0x800000;
                    }
                    else
                    {
                        Status = FmshFsbl_AuthenticatePartition(
                            LoadAddress, Size,
                            AcOffset + Index * RSA_CERTIFICATE_SIZE,
                            IsPlPartition);
                        if (Status != FMSH_SUCCESS)
                        {
                            BootInstancePtr
                                ->ErrorCode = FSBL_ERROR_PARTITION_AUTHENTICATE;
                            return Status;
                        }
                    }
                }

#else

                LoadAddress = BootInstancePtr->ImageOffsetAddress +
                              ((PartitionHeader->DataWordOffset) *
                               IH_PARTITION_WORD_LENGTH);
                if (BootInstancePtr->BootMode == QSPI_FLASH)
                {
                    LoadAddress += FPS_QSPI0_D_BASEADDR;
                    AcOffset = LoadAddress + Size;
                }
                else if (BootInstancePtr->BootMode == NAND_FLASH ||
                         BootInstancePtr->BootMode == SD_CARD)
                {
                    AcOffset = LoadAddress + Size;
                }

                for (; LoadAddress < AcOffset; LoadAddress += 0x800000)
                {
                    Status = FmshFsbl_AuthenticatePartition(
                        LoadAddress, 0x800000,
                        AcOffset + Index * RSA_CERTIFICATE_SIZE, IsPlPartition);
                    if (Status != FMSH_SUCCESS)
                    {
                        BootInstancePtr
                            ->ErrorCode = FSBL_ERROR_PARTITION_AUTHENTICATE;
                        return Status;
                    }
                    Index++;
                    Size -= 0x800000;
                }
                Status = FmshFsbl_AuthenticatePartition(
                    LoadAddress - 0x800000, Size,
                    AcOffset + Index * RSA_CERTIFICATE_SIZE, IsPlPartition);
                if (Status != FMSH_SUCCESS)
                {
                    BootInstancePtr->ErrorCode = FSBL_ERROR_PARTITION_AUTHENTICATE;
                    return Status;
                }

#endif
            }
        }  // is pl
        else
        {
            Status = FmshFsbl_AuthenticatePartition(LoadAddress, Size, AcOffset,
                                                    IsPlPartition);
            if (Status != FMSH_SUCCESS)
            {
                BootInstancePtr->ErrorCode = FSBL_ERROR_PARTITION_AUTHENTICATE;
                return Status;
            }
        }
#if DEBUG_PERF
        gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
        gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
        UART_LOG_OUT(DEBUG_PERF,
                     "Authencicatie Partition is done at the %.2f ms\r\n",
                     gtc_time);
#endif

        UART_LOG_OUT(DEBUG_INFO, "Authencicatie Partition success!! \r\n");
    }

    /**
     *check encryption status
     */
    if (FmshFsbl_IsEncryptedPresent(PartitionHeader) == IH_PH_ATTRB_ENCRYPTION)
    {
        IsEncrypted = TRUE;

        UART_LOG_OUT(DEBUG_DETAILED, "Encryption Enabled\r\n");

        EncryptionData.Alg = BootInstancePtr->EncrytionAlgorithm;
        EncryptionData.SrcAddr = BootInstancePtr->ImageHeader
                                     .PartitionHeader[PartitionNum]
                                     .DestinationLoadAddress;
        EncryptionData.LoadAddr = BootInstancePtr->ImageHeader
                                      .PartitionHeader[PartitionNum]
                                      .DestinationLoadAddress;
        EncryptionData.UnencryptedLength = (BootInstancePtr->ImageHeader
                                                .PartitionHeader[PartitionNum]
                                                .UnEncryptedDataWordLength) *
                                           IH_PARTITION_WORD_LENGTH;

        if (EncryptionData.Alg == ALG_AES)
        {
            UART_LOG_OUT(DEBUG_INFO, "Encryption Algorithm is AES256 !! \r\n");
        }
        else
        {
            UART_LOG_OUT(DEBUG_INFO, "Encryption Algorithm is SM4 !! \r\n");
        }
    }

    /**
     *use SAC_RED KEY to decrypt image
     */
    if ((BootInstance.BootHdrAttributes & IH_BH_IMAGE_ATTRB_OPKEY_MASK) ==
        IH_BH_IMAGE_ATTRB_OPKEY_MASK)
    {
        UART_LOG_OUT(DEBUG_DETAILED, "Optional Key is  used !! \r\n");
        EncryptionData.OpKeyUsing = 1;
    }

#endif  // FSBL_SECURE

    if (DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PL)
    {
#ifdef FSBL_BS
        Status = FmshFsbl_InitDevc();
        if (Status == FMSH_FAILURE)
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_DEVC_INIT;
            return Status;
        }

        Status = FDevcPs_getPlPowerStatus(&g_DEVC);
        if (Status == FMSH_FAILURE)
        {
            BootInstancePtr->ErrorCode = FSBL_ERROR_PL_POWER;
            return Status;
        }

        FmshFsbl_OpenCfgLevelShifter();

        Size = PartitionHeader->UnEncryptedDataWordLength;
        LoadAddress = BootInstancePtr->ImageOffsetAddress +
                      ((PartitionHeader->DataWordOffset) *
                       IH_PARTITION_WORD_LENGTH);

#ifdef FSBL_PS_DDR
        LoadAddress = FSBL_PL_TEMP_DDRADDR;
#endif

        if ((PartitionHeader->PartitionAttributes &
             IH_PH_ATTRB_CHECKSUM_MASK) != IH_PH_ATTRB_NOCHECKSUM)
        {
            Status = FmshFsbl_CalcualteSHA(BootInstancePtr, LoadAddress,
                                           PartitionNum);
            if (Status != FMSH_SUCCESS)
            {
                BootInstancePtr->ErrorCode = FSBL_ERROR_PARTITION_CHECKSUM;
                UART_LOG_OUT(DEBUG_INFO,
                             "FSBL_ERROR_PARTITION_CHECKSUM_FAILED \r\n");
                Status = FMSH_FAILURE;
                return Status;
            }
            UART_LOG_OUT(DEBUG_INFO, "PL partition SHA verify success!!! \r\n");
        }

        UART_LOG_OUT(DEBUG_INFO, "Prepare downloading bitstream.....\r\n");

#ifdef FSBL_SECURE
        if (IsEncrypted == TRUE)
        {
            EncryptionData.SrcAddr = LoadAddress;
            EncryptionData.UnencryptedLength = Size;

            UART_LOG_OUT(DEBUG_DETAILED,
                         "Get Secure Header IV from  Boot Header!!! \r\n");
            BootInstance.DeviceOps.DeviceCopy(
                BootInstance.ImageOffsetAddress + IH_BH_SECURE_HEADER_IV_OFFSET,
                (uintptr_t)EncryptedKeyIv, 12U);
            UART_LOG_OUT(DEBUG_DETAILED, "Secure Header IV is :\r\n");

            if (EncryptionData.Alg == ALG_AES)
            {
                if (EncryptionData.OpKeyUsing == 1)
                {
                    Status = FDevcPs_encryptDownload_AES_UseOp(
                        &g_DEVC, EncryptedKeyIv, LoadAddress, Size);
                }
                else
                {
                    UART_LOG_OUT(DEBUG_DETAILED,
                                 "Aes no op 0x%x,len = 0x%x\r\n", LoadAddress,
                                 Size);
                    Status = FDevcPs_encryptDownload_AES_NoOp(
                        &g_DEVC, EncryptedKeyIv, LoadAddress, Size);
                }
            }
            else
            {
                if (EncryptionData.OpKeyUsing == 1)
                {
                    Status = FDevcPs_encryptDownload_SM4_UseOp(
                        &g_DEVC, EncryptedKeyIv, LoadAddress, Size);
                }
                else
                {
                    Status = FDevcPs_encryptDownload_SM4_NoOp(
                        &g_DEVC, EncryptedKeyIv, LoadAddress, Size);
                }
            }
        }
        else
#endif  // FSBL_SECURE
        {
            Status = FDevcPs_noneSecureDownload(&g_DEVC, LoadAddress, Size);
        }

#if DEBUG_PERF
        gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
        gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
        UART_LOG_OUT(DEBUG_PERF,
                     "Download the PL bitstream is done at the %.2f ms\r\n",
                     gtc_time);
#endif

        return Status;
#endif  // FSBL_BS
    }
    else if( (DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PS) ||
              (DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PMU) )
    {
        /*
         * sha256 verification
         */
        if ((PartitionHeader->PartitionAttributes &
             IH_PH_ATTRB_CHECKSUM_MASK) != IH_PH_ATTRB_NOCHECKSUM)
        {
            PsLoadAddress = PartitionHeader->DestinationLoadAddress;
            Status = FmshFsbl_GetLoadAddress(DestinationCpu, &PsLoadAddress, (PartitionHeader->TotalDataWordLength) * IH_PARTITION_WORD_LENGTH);
            if (FMSH_SUCCESS != Status)
            {
                return Status;
            }
            Status = FmshFsbl_CalcualteSHA(BootInstancePtr, PsLoadAddress,
                                           PartitionNum);
            if (Status != FMSH_SUCCESS)
            {
                BootInstancePtr->ErrorCode = FSBL_ERROR_PARTITION_CHECKSUM;
                UART_LOG_OUT(DEBUG_INFO,
                             "FSBL_ERROR_PARTITION_CHECKSUM_FAILED \r\n");
                Status = FSBL_ERROR_PARTITION_CHECKSUM;
                return Status;
            }
            UART_LOG_OUT(DEBUG_INFO, "PS partition SHA verify success!!! \r\n");
        }
#ifdef FSBL_SECURE
        /*trigger KEY rolling feature */
        if (IsEncrypted == TRUE)
        {
            UART_LOG_OUT(DEBUG_DETAILED,
                         "Preparing Key Rolling Decryption !! \r\n");
            Status = FmshFsbl_Secure_AesDecrypt(&EncryptionData);
            if (Status != FMSH_SUCCESS)
            {
                BootInstancePtr->ErrorCode = FSBL_ERROR_DECYPTION;
                UART_LOG_OUT(DEBUG_INFO, "Usr App Decryption failed!!! \r\n");
                return Status;
            }

            UART_LOG_OUT(DEBUG_INFO, "Usr App Decryption success!!! \r\n");
        }
#endif
        if( DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PS )
        {
          CpuNo = BootInstancePtr->HandoffCpuNo;
          if (FmshFsbl_CheckHandoffCpu(BootInstancePtr, DestinationCpu) ==
              FMSH_SUCCESS)
          {
              BootInstancePtr->HandoffValues[CpuNo].CpuSettings = DestinationCpu |
                                                                  ExecState;
              BootInstancePtr->HandoffValues[CpuNo].PartitionAttributes = PartitionHeader->PartitionAttributes;
              BootInstancePtr->HandoffValues[CpuNo]
                  .HandoffAddress = PartitionHeader->DestinationExecutionAddress;
              BootInstancePtr->HandoffCpuNo += 1U;
          }
        }
#if DEBUG_PERF
        gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
        gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
        UART_LOG_OUT(DEBUG_PERF,
                     "Download the Application is done at the %.2f ms\r\n",
                     gtc_time);
#endif
    }
    else{
        ;/* no deal with */
    }

    return Status;
}


/******************************************************************************
 * This function checks if PMU FW is loaded and gives handoff to PMU 
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @param	PartitionNum is the partition number of the image
 *
 * @return	None
 *
 *****************************************************************************/
static void FmshFsbl_CheckPmuFw(BootPs *BootInstancePtr, u32 PartitionNum)
{

	u32 DestinationCpu = 0U;
	u32 RegVal = 0U;

	DestinationCpu =FmshFsbl_GetDestinationCpu(
			&BootInstancePtr->ImageHeader.PartitionHeader[PartitionNum]);

	if (DestinationCpu == IH_PH_ATTRB_DEST_CPU_PMU) {
		/* Wakeup the processor */
		WriteReg(PMU_GLOBAL_GLOBAL_CNTRL,
				ReadReg(PMU_GLOBAL_GLOBAL_CNTRL) | 0x1);

		/* wait until done waking up */
		do {
				RegVal = ReadReg(PMU_GLOBAL_GLOBAL_CNTRL);
				if ((RegVal & PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)
					== PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) {
					break;
				}
		} while(1);
	}
}

/******************************************************************************
 * This function loads the partition
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @param	PartitionNum is the partition number in the image to be loaded
 *
 * @return	returns the error codes described in error.h on any error
 * 			returns SUCCESS on success
 *
 *****************************************************************************/
u32 FmshFsbl_PartitionLoad (BootPs *BootInstancePtr, u32 PartitionNum)
{
    u32 Status = FMSH_SUCCESS;

    /**
     * Partition Header Validation
     */
    UART_LOG_OUT(DEBUG_DETAILED, "Partition header validate......\r\n");
    Status = FmshFsbl_PartitionHeaderValidation(BootInstancePtr, PartitionNum);
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO, "Partition header validate failure!!\r\n");
        return Status;
    }
    UART_LOG_OUT(DEBUG_INFO, "Partition header validate SUCCESS!!\r\n");

#if DEBUG_PERF
    gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
    gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
    UART_LOG_OUT(DEBUG_PERF,
                 "Partition header validate is done at the %.2f ms\r\n",
                 gtc_time);
#endif

    /**
     * Partition Copy
     */
    UART_LOG_OUT(DEBUG_DETAILED, "Prepare Copy Partition......\r\n");
    Status = FmshFsbl_PartitionCopy(BootInstancePtr, PartitionNum);
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }

#if DEBUG_PERF
    gtc_count1 = *(u64 *)(FPS_GTC_BASEADDR + 0x8);
    gtc_time = (gtc_count1 - gtc_count0) / (float)(GTC_FREQ / 1000);
    UART_LOG_OUT(DEBUG_PERF, "Copy Partition is done at the %.2f ms\r\n",
                 gtc_time);
#endif

    /**
     * Partition Validation
     */
    Status = FmshFsbl_PartitionValidation(BootInstancePtr, PartitionNum);
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }

    FmshFsbl_CheckPmuFw(BootInstancePtr, PartitionNum);
    
#if FSBL_WDT_ENABLE
    (void)FWdtPs_restart(&g_WDT);
#endif

    return Status;
}
