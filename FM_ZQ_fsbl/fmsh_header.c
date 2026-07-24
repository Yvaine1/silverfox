/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_header.c
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

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern BootPs BootInstance;

extern FQspiPsu_T qspi0;

Ps_BootHeader BootHeaderInfo;
u8 ImageHeaderBuffer[8 * 1024] __attribute__((aligned(4)))={0U};
u8 BootHeaderBuffer[8 * 1024] __attribute__((aligned(4)))={0U};
u8 *ImageHdr = ImageHeaderBuffer;
u8 *BootHdr = BootHeaderBuffer;
u32 BootHeaderSize = 0U;
static Ps_ATFHandoffParams ATFHandoffParams
    __attribute__((section(".bss.handoff_params")));

/************************** Function Prototypes ******************************/

/******************************************************************************
 * This function validates the image header
 *
 * @param	BootInstancePtr is pointer to the BootPs Instance
 *
 * @return	returns the error codes described in fmsh_error.h on any error
 * 			returns FMSH_SUCCESS on success
 *
 ******************************************************************************/
u32 FmshFsbl_ValidateHeader (BootPs *BootInstancePtr)
{
    u32 Status = FMSH_SUCCESS;
    u32 MultiBootReg = 0U;
    u32 FlashImageOffsetAddress = 0U;
    u32 EncrytionAlg = 0U;
    u32 Reg = 0U;
#ifndef FSBL_QSPI_XIP_EXCLUDE    
    u32 WidthDetection = 0U;
#endif   
    /**
     * Read the Multiboot Register
     */
    if ((ReadReg(SAC_CFG_REG) & SAC_MULTIBOOT_EN_MASK) >>
        SAC_MULTIBOOT_EN_SHIFT)
    {
        MultiBootReg = ReadReg(SAC_MULTI_BOOT_REG);
    }
    UART_LOG_OUT(DEBUG_INFO, "Multiboot register: 0x%x\r\n", MultiBootReg);

    BootInstancePtr->ImageOffsetAddress = FlashImageOffsetAddress;

    /**
     *  Calculate the Flash Offset Address
     *  For file system based devices, Flash Offset Address should be 0 always
     */
    if (BootInstancePtr->SecondaryBootDevice == 0U)
    {
        if (!((BootInstancePtr->PrimaryBootDevice == SD0_BOOT_MODE) ||
              (BootInstancePtr->PrimaryBootDevice == EMMC_BOOT_MODE) ||
              (BootInstancePtr->PrimaryBootDevice == SD1_BOOT_MODE) ||
              (BootInstancePtr->PrimaryBootDevice == SD1_LS_BOOT_MODE) ||
              (BootInstancePtr->PrimaryBootDevice == USB0_BOOT_MODE)))
        {
            BootInstancePtr->ImageOffsetAddress = MultiBootReg *
                                                  SEARCH_STEP_SIZE;
        }
    }
    else
    {
        if (!((BootInstancePtr->SecondaryBootDevice == SD0_BOOT_MODE) ||
              (BootInstancePtr->SecondaryBootDevice == EMMC_BOOT_MODE) ||
              (BootInstancePtr->SecondaryBootDevice == SD1_BOOT_MODE) ||
              (BootInstancePtr->SecondaryBootDevice == SD1_LS_BOOT_MODE) ||
              (BootInstancePtr->SecondaryBootDevice == USB0_BOOT_MODE)))
        {
            BootInstancePtr->ImageOffsetAddress = MultiBootReg *
                                                  SEARCH_STEP_SIZE;
        }
    }

    FlashImageOffsetAddress = BootInstancePtr->ImageOffsetAddress;

    /* Load boot header information to global array*/
    Status = BootInstancePtr->DeviceOps.DeviceCopy(
        FlashImageOffsetAddress + IH_BH_OFFSET, (UINTPTR)(&BootHeaderInfo),
        sizeof(Ps_BootHeader));

    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO, "Device Copy Failed \n\r");
        return Status;
    }

    Status = FmshFsbl_ValidataBootHeader(&BootHeaderInfo);
    if (Status != FMSH_SUCCESS)
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "Boot header validate failed,increment 32KB to search "
                     "next boot header\r\n");
        return Status;
    }

    UART_LOG_OUT(DEBUG_INFO,
                 "Boot header validate success, this is a valid image!!!\r\n");

    BootInstancePtr->BootHdrAttributes = BootHeaderInfo.ImageAttr;
    BootInstancePtr->EncryptionStatus = BootHeaderInfo.EncryptionStatus;
    /* QSPI X4 mode DETECT*/
    if ((BootInstancePtr->BootMode == QSPI24_BOOT_MODE) ||
        (BootInstancePtr->BootMode == QSPI32_BOOT_MODE))
    {
#ifndef FSBL_QSPI_XIP_EXCLUDE
        (void)FQspiPsu_Nor_ChangeReadMode(&qspi0, QSPI_RD_QOR);
        BootInstancePtr->DeviceOps.DeviceCopy(
            BootInstancePtr->ImageOffsetAddress + IH_BH_WIDTH_DETECTION,
            (uintptr_t)&WidthDetection, 4);
        if (WidthDetection != QSPI_WIDTH_DETECT_WORD)
        {
            (void)FQspiPsu_Nor_ChangeReadMode(&qspi0, QSPI_RD_DOR);
            BootInstancePtr->DeviceOps.DeviceCopy(
                BootInstancePtr->ImageOffsetAddress + IH_BH_WIDTH_DETECTION,
                (uintptr_t)&WidthDetection, 4);
            if (WidthDetection != QSPI_WIDTH_DETECT_WORD)
            {
                (void)FQspiPsu_Nor_ChangeReadMode(&qspi0, QSPI_RD_FR);
            }
        }
#endif
    }

    if (BootInstancePtr->EncryptionStatus)
    {
        BootInstancePtr->SecureModeFlag = 1U;
        WriteReg(SAC_CFG_REG, ReadReg(SAC_CFG_REG) | SAC_SECURE_MODE_MASK);
        BootInstancePtr->DeviceOps.DeviceCopy(
            FlashImageOffsetAddress + IH_BH_ENC_ALG_OFFSET,
            (uintptr_t)&EncrytionAlg, 4U);
        BootInstancePtr->EncrytionAlgorithm = EncrytionAlg;
    }
    else
    {
        if ((FmshFsbl_FindOneInNumber(ReadReg(SAC_EFUSE_SECURE_BOOT_EN) &
                                      SAC_MULTIBOOT_EN_MASK) > 0x8))
        {
            UART_LOG_OUT(DEBUG_INFO, "Secure boot is force!\r\n");
            BootInstancePtr->ErrorCode = FSBL_ERROR_SECURE_BOOT_FORCE;
            Status = FMSH_FAILURE;
            BootInstancePtr->SecureModeFlag = 1U;
            return Status;
        }

        BootInstancePtr->SecureModeFlag = 0U;
    }
    if ( BootInstancePtr->SecureModeFlag == 0x1U )
    {
	Reg = ReadReg(PMU_GLOBAL_GLOB_GEN_STORAGE5);
	WriteReg(PMU_GLOBAL_GLOB_GEN_STORAGE5,  Reg |
				FSBL_FSBL_ENCRYPTED_MASK);
    }
    /**
     * Read Image Header and validate Image Header Table
     */
    Status = FmshFsbl_ReadImageHeader(
        &BootInstancePtr->ImageHeader, &BootInstancePtr->DeviceOps,
        FlashImageOffsetAddress, BootInstancePtr->ProcessorID);
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }

    return Status;
}

/*******************************************************************************
 *
 *  This function validates the image identification in boot header.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static u32 ValidateImageID (Ps_BootHeader *Header)
{
    if ((Header->ImageId != FMSH_IMAGE_ID))
    /*0x584c4e58*/ /*0x464d5348*/ {
        UART_LOG_OUT(DEBUG_INFO,
                     "Error: Image Identification 0x%8.8x != 0x%8.8x\r\n",
                     Header->ImageId, "XLNX");
        return FMSH_FAILURE;
    }
    UART_LOG_OUT(DEBUG_INFO, "Image ID verified success!!!\r\n");

    return FMSH_SUCCESS;
}

/*******************************************************************************
 *
 *  This function is used to validate the word checksum for the image header
 *  table and partition headers.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static u32 ValidateChecksum (u32 Buffer[], u32 Length)
{
    u32 Status = FMSH_SUCCESS;
    u32 Checksum = 0U;
    u32 Count = 0U;

    /**
     * Length has to be atleast equal to 2,
     */
    if (Length < 2U)
    {
        Status = FMSH_FAILURE;
        return Status;
    }

    /**
     * Checksum = ~(X1 + X2 + X3 + .... + Xn)
     * Calculate the checksum
     */
    for (Count = 0U; Count < (Length - 1U); Count++)
    {
        /**
         * Read the word from the header
         */
        Checksum += Buffer[Count];
    }

    /**
     * Invert checksum
     */
    Checksum ^= 0xFFFFFFFF;

    /**
     * Validate the checksum
     */
    if (Buffer[Length - 1U] != Checksum)
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "Error: Calculated Checksum 0x%0lx !=  %0lx located in "
                     "BootROM Header\r\n",
                     Checksum, Buffer[Length - 1U]);
        Status = FMSH_FAILURE;
    }
    else
    {
        UART_LOG_OUT(DEBUG_INFO, "Checksum verified success!!!\r\n");
        Status = FMSH_SUCCESS;
    }

    return Status;
}

/*******************************************************************************
 *
 *  This function checks the fields of the image header table and validates
 *  them.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static u32 ValidateImageHeaderTable (Ps_ImageHeaderTable *ImageHeaderTable)
{
    u32 Status = FMSH_SUCCESS;
    u32 ImageHeaderAddr = 0;
#ifdef FSBL_SECURE
    u32 EfuseCtrl = 0;
    u32 AcOffset = 0U;
    u32 Size = 0;
#endif

    /**
     * Check the check sum of the image header table
     */
    Status = ValidateChecksum((u32 *)ImageHeaderTable, IH_IHT_LEN / 4U);
    if (FMSH_SUCCESS != Status)
    {
        BootInstance.ErrorCode = FSBL_ERROR_IMG_HEADER_CHECKSUM;
        UART_LOG_OUT(DEBUG_INFO, "Checksum validate failed!!!\r\n");
        return Status;
    }
    UART_LOG_OUT(DEBUG_INFO, "Checksum validate success!!!\r\n");

    /**
     * Read the Image Header Table offset from Boot Header
     */
    Status = BootInstance.DeviceOps.DeviceCopy(
        BootInstance.ImageOffsetAddress + IH_BH_IH_TABLE_OFFSET,
        (uintptr_t)&ImageHeaderAddr, 4U);
    if (Status != FMSH_SUCCESS)
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "Copy Image Header Table Offset Word failed\r\n");
        return Status;
    }

#ifdef FSBL_SECURE
    /**
     * Read Efuse bit and check Boot Header for Authentication
     */
    EfuseCtrl = ReadReg(SAC_EFUSE_SEC_CTRL) & SAC_EFUSE_SEC_CTRL_RSA_EN_MASK;
    if (((FmshFsbl_FindOneInNumber(EfuseCtrl)) > 0x8) ||
        ((BootHeaderInfo.ImageAttr & IH_BH_IMAGE_ATTRB_RSA_MASK) ==
         IH_BH_IMAGE_ATTRB_RSA_MASK))
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "RSA Authentication Enabled,Preparing Boot Image Header "
                     "Authentication...... \r\n");
        /**
         * Read the Image Header Table offset from Boot Header
         */
        Status = BootInstance.DeviceOps.DeviceCopy(
            BootInstance.ImageOffsetAddress + IH_BH_IH_TABLE_OFFSET,
            (uintptr_t)&ImageHeaderAddr, 4);
        if (Status != FMSH_SUCCESS)
        {
            UART_LOG_OUT(DEBUG_INFO,
                         "Copy Image Header Table Offset Word failed\r\n");
            return Status;
        }

        /**
         *  Authenticate the image header , Read AC offset from Image header
         * table
         */
        Status = BootInstance.DeviceOps.DeviceCopy(
            BootInstance.ImageOffsetAddress + ImageHeaderAddr +
                IH_IHT_AC_OFFSET,
            (uintptr_t)(&AcOffset), IH_FIELD_LEN);
        if (FMSH_SUCCESS != Status)
        {
            UART_LOG_OUT(DEBUG_INFO,
                         "Device Copy Image Header AC Offset Word Failed \n\r");
            return Status;
        }
        /*Check if AC offset equal zero*/
        if (AcOffset == 0)
        {
            // BootInstance.ErrorCode = NO_IMG_HEADER_AC_ERROR;
            UART_LOG_OUT(DEBUG_INFO,
                         "ERROR: RSA authentication certificate dose not exist "
                         "in boot image...... \r\n");
            Status = FMSH_FAILURE;
            return Status;
        }
        /*Calculate Boot Header Size */
        BootHeaderSize = ImageHeaderAddr;

        /* Copy the Boot header to OCM */
        Status = BootInstance.DeviceOps.DeviceCopy(
            BootInstance.ImageOffsetAddress, (UINTPTR)BootHdr, BootHeaderSize);
        if (Status != FMSH_SUCCESS)
        {
            UART_LOG_OUT(DEBUG_INFO, "Copy Boot Header failed\r\n");
            return Status;
        }

        /*Total size of Image header may vary depending on padding so  size = AC
         * address - Start address;*/
        // imageHeader+rsa
        Size = (AcOffset * IH_PARTITION_WORD_LENGTH) - ImageHeaderAddr +
               RSA_CERTIFICATE_SIZE;

        /* Copy the Image header and AC to OCM */
        Status = BootInstance.DeviceOps.DeviceCopy(
            BootInstance.ImageOffsetAddress + ImageHeaderAddr,
            (UINTPTR)ImageHdr, Size);
        if (Status != FMSH_SUCCESS)
        {
            UART_LOG_OUT(DEBUG_INFO, "Copy Image Header and AC  Failed\r\n");
            return Status;
        }
        // imageHeader
        Size -= RSA_CERTIFICATE_SIZE;

        AcOffset = (uintptr_t)ImageHdr + Size;
        // SIZE: imageHeader- 0xA40-0X8C0=0X180
        // AcOffset: Ptr->imageHeader RSA
        Status = FmshFsbl_AuthenticateHeader(BootHdr, Size, AcOffset);
        if (Status != FMSH_SUCCESS)
        {
            BootInstance.ErrorCode = FSBL_ERROR_BOOT_HEADER_SIGNATURE;
            return Status;
        }
    }
#endif

    /**
     * Print the Image header table details
     * Print the Bootgen version
     */
    UART_LOG_OUT(DEBUG_INFO, "Image Header Table Details\r\n");
    UART_LOG_OUT(DEBUG_INFO, "Boot Gen Ver: 0x%0lx \r\n",
                 ImageHeaderTable->Version);
    UART_LOG_OUT(DEBUG_INFO, "Number of Partitions: 0x%0lx \r\n",
                 ImageHeaderTable->NoOfPartitions);
    UART_LOG_OUT(DEBUG_INFO, "Partition Header Address: 0x%0lx \r\n",
                 ImageHeaderTable->PartitionHeaderAddress);
    UART_LOG_OUT(DEBUG_INFO, "Partition Present Device: 0x%0lx \r\n",
                 ImageHeaderTable->PartitionPresentDevice);

    return Status;
}

/*******************************************************************************
 *
 *  This function is used to check memory address.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static u32 CheckValidMemoryAddress (u32 Address, u32 CpuId, u32 DevId)
{
    u32 Status = FMSH_SUCCESS;

    /**
     * If destination device is PL and load address is not configured,
     * don't consider this as error as we will use temp load address
     * to load PL bitstream
     */
    if ((DevId == IH_PH_ATTRB_DEST_DEVICE_PL) &&
        (Address == FSBL_DUMMY_PL_ADDR))
    {
        return FMSH_SUCCESS;
    }

    /* Check if Address is in the range of PMU RAM for PMU FW */
    if (CpuId == IH_PH_ATTRB_DEST_CPU_PMU)
    {
        if ((Address >= FSBL_PMU_RAM_START_ADDRESS) &&
            (Address < FSBL_PMU_RAM_END_ADDRESS))
        {
            Status = FMSH_SUCCESS;
            return Status;
        }
    }

    /* Check if Address is in the range of TCM for R5_0/R5_1 */
    if ((CpuId == IH_PH_ATTRB_DEST_CPU_R5_0) ||
        (CpuId == IH_PH_ATTRB_DEST_CPU_R5_1))
    {
        if ((Address == FSBL_R5_TCM_START_ADDRESS) ||
            (((Address > FSBL_R5_TCM_START_ADDRESS) &&
              (Address <
               (FSBL_R5_TCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH))) ||
             ((Address >= FSBL_R5_BTCM_START_ADDRESS) &&
              (Address <
               (FSBL_R5_BTCM_START_ADDRESS + FSBL_R5_TCM_BANK_LENGTH)))))
        {
            Status = FMSH_SUCCESS;
            return Status;
        }
    }

    /* Check if Address is in the range of TCM for R5_L */
    if (CpuId == IH_PH_ATTRB_DEST_CPU_R5_L)
    {
        if ((Address == FSBL_R5_TCM_START_ADDRESS) ||
            ((Address > FSBL_R5_TCM_START_ADDRESS) &&
             (Address <
              (FSBL_R5_TCM_START_ADDRESS + (FSBL_R5_TCM_BANK_LENGTH * 4U)))))
        {
            Status = FMSH_SUCCESS;
            return Status;
        }
    }

    /* Check if Address is in the range of DDR */
#ifdef FSBL_PS_DDR
    /**
     * Check if Address is in the range of PS DDR
     */
#if (FSBL_PS_DDR_START_ADDRESS == 0)  
    if(Address <= FSBL_PS_DDR_END_ADDRESS )
    {
        return FMSH_SUCCESS;
    }
#else    
    if ((Address >= FSBL_PS_DDR_START_ADDRESS) &&
        (Address <= FSBL_PS_DDR_END_ADDRESS))
    {
        return FMSH_SUCCESS;
    }
#endif
#endif

    /**
     * Check if Address is in the range of last bank of OCM
     */
    if ((DevId == IH_PH_ATTRB_DEST_DEVICE_PS) &&
        (Address >= FSBL_PS_OCM_START_ADDRESS) &&
        (Address < FSBL_PS_OCM_END_ADDRESS))
    {
        return FMSH_SUCCESS;
    }

    /**
     * Not a valid address
     */
    Status = FSBL_ERROR_INVALID_EXCUTION_ADDRESS;
    UART_LOG_OUT(DEBUG_INFO, "FMSH_FSBL_ERROR_ADDRESS: %llx\n\r", Address);

    return Status;
}

/*******************************************************************************
 *
 *  This function is used to display the information of the header.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static void DisplayHeaderInfo (Ps_PartitionHeader *PartitionHeader)
{
    /**
     * Print Partition Header Details
     */
    UART_LOG_OUT(DEBUG_INFO, "UnEncrypted data Length: 0x%08x \r\n",
                 PartitionHeader->UnEncryptedDataWordLength);
    UART_LOG_OUT(DEBUG_INFO, "Data word offset: 0x%08x \r\n",
                 PartitionHeader->EncryptedDataWordLength);
    UART_LOG_OUT(DEBUG_INFO, "Total Data word length: 0x%08x \r\n",
                 PartitionHeader->TotalDataWordLength);
    UART_LOG_OUT(DEBUG_INFO, "Destination Load Address: 0x%08x \r\n",
                 (UINTPTR)PartitionHeader->DestinationLoadAddress);
    UART_LOG_OUT(DEBUG_INFO, "Execution Address: 0x%08x \r\n",
                 (UINTPTR)PartitionHeader->DestinationExecutionAddress);
    UART_LOG_OUT(DEBUG_INFO, "Data word offset: 0x%08x \r\n",
                 PartitionHeader->DataWordOffset);
    UART_LOG_OUT(DEBUG_INFO, "Partition Attributes: 0x%08x \r\n",
                 PartitionHeader->PartitionAttributes);
}

/*******************************************************************************
 *
 *  This function is used to validate partition header.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static u32 ValidatePartitionHeader (Ps_PartitionHeader *PartitionHeader,
                                    u32 RunningCpu)
{
    u32 Status = FMSH_SUCCESS;
    u8 IsEncrypted = FALSE;
    u8 IsAuthenticated = FALSE;
    u32 DestinationCpu = 0U;
    u32 DestinationDevice = 0;

    if (FmshFsbl_IsEncryptedPresent(PartitionHeader) == IH_PH_ATTRB_ENCRYPTION)
    {
        IsEncrypted = TRUE;
        UART_LOG_OUT(DEBUG_INFO,
                     "Partition is encrypted, enter secure mode ......\r\n");
    }
    else
    {
        IsEncrypted = FALSE;
        UART_LOG_OUT(DEBUG_INFO, "Partition is unencrypted......\r\n");
    }

    if (FmshFsbl_IsRsaSignaturePresent(PartitionHeader) ==
        IH_PH_ATTRB_RSA_SIGNATURE)
    {
        IsAuthenticated = TRUE;
        UART_LOG_OUT(DEBUG_INFO,
                     "Partition is Authenticated, enter secure mode......\r\n");
    }
    else
    {
        IsAuthenticated = FALSE;
        UART_LOG_OUT(DEBUG_INFO, "Partition is unauthenticated......\r\n");
    }

    if ((FmshFsbl_FindOneInNumber(ReadReg(SAC_EFUSE_SECURE_BOOT_EN) &
                                  SAC_MULTIBOOT_EN_MASK) > 0x8) &&
        (IsEncrypted == FALSE))
    {
        Status = FSBL_ERROR_ENC_IS_MANDATORY;
        UART_LOG_OUT(
            DEBUG_INFO,
            "FSBL_ERROR_ENC_IS_MANDATORY as eFUSE ENC_ONLY bit is set\r\n");
        Status = FMSH_FAILURE;
        return Status;
    }

    DestinationCpu = FmshFsbl_GetDestinationCpu(PartitionHeader);
    /* if destination cpu is not present, it means it is for same cpu */
    if (DestinationCpu == IH_PH_ATTRB_DEST_CPU_NONE)
    {
        DestinationCpu = RunningCpu;
    }
    DestinationDevice = FmshFsbl_GetDestinationDevice(PartitionHeader);

    /* if destination cpu is not present, it means it is for same cpu */
    if (DestinationCpu == IH_PH_ATTRB_DEST_CPU_NONE)
    {
        DestinationCpu = RunningCpu;
    }

    /**
     * check for XIP image - partition lengths should be zero
     * execution address should be in QSPI
     */
    if (PartitionHeader->UnEncryptedDataWordLength == 0U)
    {
        UART_LOG_OUT(DEBUG_INFO, "FSBL will execute in place\r\n");
        if ((IsAuthenticated == TRUE) || (IsEncrypted == TRUE))
        {
            Status = FSBL_ERROR_XIP_AUTH_ENC_PRESENT;
            UART_LOG_OUT(DEBUG_INFO, "ERROR_XIP_AUTH_ENC_PRESENT\r\n");
            DisplayHeaderInfo(PartitionHeader);
            return Status;
        }
        if ((PartitionHeader->DestinationExecutionAddress <
             QSPI_LINEAR_BASE_ADDRESS_START) ||
            (PartitionHeader->DestinationExecutionAddress >
             QSPI_LINEAR_BASE_ADDRESS_END))
        {
            Status = FSBL_ERROR_APU_XIP_EXCUTION_ADDRESS;
            UART_LOG_OUT(DEBUG_INFO, "ERROR_APU_XIP_EXCUTION_ADDRESS\r\n");
            DisplayHeaderInfo(PartitionHeader);
            return Status;
        }
        /* Re-initialize QSPI xip MODE */
        // FQspiPs_EnterXIP(QspiInstancePtr,DIOR_CMD);
    }
    /**
     * check for authentication and encryption length
     */
    if ((IsAuthenticated == FALSE) && (IsEncrypted == FALSE))
    {
        /**
         * all lengths should be equal
         */
        /*    if ((PartitionHeader->UnEncryptedDataWordLength !=
                 PartitionHeader->EncryptedDataWordLength) ||
                (PartitionHeader->EncryptedDataWordLength !=
                 PartitionHeader->TotalDataWordLength))
        {
                Status = ERROR_MISMATCH_PARTITION_LENGTH;
            UART_LOG_OUT(DEBUG_INFO,"ERROR_PARTITION_LENGTH\r\n");
            DisplayHeaderInfo(PartitionHeader);
                return Status;
        }*/
        /*  if( (PartitionHeader->DestinationLoadAddress==0)&&
          (PartitionHeader->DestinationExecutionAddress==0) &&
          (DestinationDevice == IH_PH_ATTRB_DEST_DEVICE_PS))
          {
              Status = PARTITION_SKIP_LOAD;
          UART_LOG_OUT(DEBUG_INFO,"Skip load this partition!!\r\n");
          DisplayHeaderInfo(PartitionHeader);
              return Status;
          }*/
    }
    else if ((IsAuthenticated == TRUE) && (IsEncrypted == FALSE))
    {
        /**
         * TotalDataWordLength should be more
         */
        if ((PartitionHeader->UnEncryptedDataWordLength !=
             PartitionHeader->EncryptedDataWordLength) ||
            (PartitionHeader->EncryptedDataWordLength >=
             PartitionHeader->TotalDataWordLength))
        {
            Status = FSBL_ERROR_MISMATCH_PARTITION_LENGTH;
            UART_LOG_OUT(DEBUG_INFO, "ERROR_PARTITION_LENGTH\r\n");
            DisplayHeaderInfo(PartitionHeader);
            return Status;
        }
    }
    else if ((IsAuthenticated == FALSE) && (IsEncrypted == TRUE))
    {
        /**
         * EncryptedDataWordLength should be more
         */
        if ((PartitionHeader->UnEncryptedDataWordLength >=
             PartitionHeader->EncryptedDataWordLength) ||
            (PartitionHeader->EncryptedDataWordLength !=
             PartitionHeader->TotalDataWordLength))
        {
            Status = FSBL_ERROR_MISMATCH_PARTITION_LENGTH;
            UART_LOG_OUT(DEBUG_INFO, "ERROR_PARTITION_LENGTH\r\n");
            DisplayHeaderInfo(PartitionHeader);
            return Status;
        }
    }
    else /* Authenticated and Encrypted */
    {
        /**
         * TotalDataWordLength should be more
         */
        if ((PartitionHeader->UnEncryptedDataWordLength >=
             PartitionHeader->EncryptedDataWordLength) ||
            (PartitionHeader->EncryptedDataWordLength >=
             PartitionHeader->TotalDataWordLength))
        {
            Status = FSBL_ERROR_MISMATCH_PARTITION_LENGTH;
            UART_LOG_OUT(DEBUG_INFO, "ERROR_PARTITION_LENGTH\r\n");
            DisplayHeaderInfo(PartitionHeader);
            return Status;
        }
    }

    Status = CheckValidMemoryAddress(PartitionHeader->DestinationLoadAddress,
                                     DestinationCpu, DestinationDevice);

    DisplayHeaderInfo(PartitionHeader);

    return Status;
}

/*******************************************************************************
 *
 *  This function is used to get partition owner.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_GetPartitionOwner (const Ps_PartitionHeader *PartitionHeader)
{
    return PartitionHeader->PartitionAttributes & IH_PH_ATTRB_PART_OWNER_MASK;
}

/*******************************************************************************
 *
 *  This function is used to get rsa signature flag.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_IsRsaSignaturePresent (const Ps_PartitionHeader *PartitionHeader)
{
    return PartitionHeader->PartitionAttributes &
           IH_PH_ATTRB_RSA_SIGNATURE_MASK;
}

/*******************************************************************************
 *
 *  This function is used to get checksum type.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_GetChecksumType (Ps_PartitionHeader *PartitionHeader)
{
    return PartitionHeader->PartitionAttributes & IH_PH_ATTRB_CHECKSUM_MASK;
}
/*******************************************************************************
 *
 *  This function is used to get cpu destination.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_GetDestinationCpu (Ps_PartitionHeader *PartitionHeader)
{
    return PartitionHeader->PartitionAttributes & IH_PH_ATTRB_DEST_CPU_MASK;
}
/*******************************************************************************
 *
 *  This function is used to get encrypted flag.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_IsEncryptedPresent (const Ps_PartitionHeader *PartitionHeader)
{
    return PartitionHeader->PartitionAttributes & IH_PH_ATTRB_ENCRYPTION_MASK;
}
/*******************************************************************************
 *
 *  This function is used to get destination device.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_GetDestinationDevice (const Ps_PartitionHeader *PartitionHeader)
{
    return PartitionHeader->PartitionAttributes & IH_PH_ATTRB_DEST_DEVICE_MASK;
}
/*******************************************************************************
 *
 *  This function is used to get state.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_GetExecState (const Ps_PartitionHeader *PartitionHeader)
{
    return PartitionHeader->PartitionAttributes & IH_PH_ATTRB_A53_EXEC_ST_MASK;
}

u32 FmshFsbl_GetVectorLocation (const Ps_PartitionHeader *PartitionHeader)
{
    return (PartitionHeader->PartitionAttributes &
            IH_PH_ATTRB_VEC_LOCATION_MASK);
}
/*******************************************************************************
 *
 *  This function is used to get destination device.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_GetBlockSize (const Ps_PartitionHeader *PartitionHeader)
{
    u32 Size = ((PartitionHeader->PartitionAttributes) &
                IH_PH_ATTR_BLOCK_SIZE_MASK) >>
               IH_ATTRB_BLOCK_SIZE_SHIFT;

    if (Size != 0x00U)
    {
        Size = ((u32)2 << Size) * FSBL_MUL_MEGABYTES;
    }

    return Size;
}
/*******************************************************************************
 *
 *  This function is used to validate boot header.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_ValidataBootHeader (Ps_BootHeader *Header)
{
    u32 Status = FMSH_SUCCESS;

    Status = ValidateImageID(Header);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    Status = ValidateChecksum((u32 *)Header, sizeof(Ps_BootHeader) / 4U);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    return Status;
}
/*******************************************************************************
 *
 *  This function is used to validate partition header.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_PartitionHeaderValidation (BootPs *BootInstance, u32 PartitionNum)
{
    u32 Status = FMSH_SUCCESS;
    Ps_PartitionHeader *PartitionHeader = NULL;

    /**
     * Assign the partition header to local variable
     */
    PartitionHeader = &(
        BootInstance->ImageHeader.PartitionHeader[PartitionNum]);

    /**
     * Check the check sum of the partition header
     */
    Status = ValidateChecksum((u32 *)PartitionHeader, IH_PH_LEN / 4U);
    if (FMSH_SUCCESS != Status)
    {
        BootInstance->ErrorCode = FSBL_ERROR_PH_CHECKSUM;
        UART_LOG_OUT(DEBUG_INFO,
                     "Partition Header Checksum Verify Failure!\r\n");
        Status = FMSH_FAILURE;
        return Status;
    }

    /**
     * Check if partition belongs to FSBL
     */
    if (FmshFsbl_GetPartitionOwner(PartitionHeader) !=
        IH_PH_ATTRB_PART_OWNER_FSBL)
    {
        /**
         * If the partition doesn't belong to FSBL, skip the partition
         */
        UART_LOG_OUT(DEBUG_INFO, "Skipping the Partition 0x%0lx\n",
                     PartitionNum);
        Status = PARTITION_SKIP_LOAD;
        return Status;
    }

    /**
     * Validate the fields of partition
     */
    Status = ValidatePartitionHeader(PartitionHeader,
                                     BootInstance->ProcessorID);
    if (FMSH_SUCCESS != Status)
    {
        BootInstance->ErrorCode = Status;
        return Status;
    }

    return Status;
}
/****************************************************************************/
/**
 * This function sets the handoff parameters to the ARM Trusted Firmware (ATF)
 * Some of the inputs for this are taken from FSBL partition header
 * A pointer to the structure containing these parameters is stored in the
 * PMU_GLOBAL.GLOBAL_GEN_STORAGE6 register, which ATF reads.
 *
 * @param PartitionHeader is pointer to the FsblPs_PartitionHeader structure
 *
 * @return None
 *
 * @note
 *
 *****************************************************************************/
static void FmshFsbl_SetATFHandoffParameters (
    const Ps_PartitionHeader *PartitionHeader, u32 EntryCount)
{
    u32 PartitionAttributes = 0U;
    u32 PartitionFlags = 0U;

    PartitionAttributes = PartitionHeader->PartitionAttributes;

    PartitionFlags = (((PartitionAttributes & IH_PH_ATTRB_A53_EXEC_ST_MASK) >>
                       IH_ATTRB_A53_EXEC_ST_SHIFT_DIFF) |
                      ((PartitionAttributes & IH_PH_ATTRB_ENDIAN_MASK) >>
                       IH_ATTRB_ENDIAN_SHIFT_DIFF) |
                      ((PartitionAttributes & IH_PH_ATTRB_TR_SECURE_MASK)
                       << IH_ATTRB_TR_SECURE_SHIFT_DIFF) |
                      ((PartitionAttributes & IH_PH_ATTRB_TARGET_EL_MASK)
                       << IH_ATTRB_TARGET_EL_SHIFT_DIFF));

    /* Update CPU number based on destination CPU */
    if ((PartitionAttributes & IH_PH_ATTRB_DEST_CPU_MASK) ==
        IH_PH_ATTRB_DEST_CPU_A53_0)
    {
        PartitionFlags |= IH_PART_FLAGS_DEST_CPU_A53_0;
    }
    else if ((PartitionAttributes & IH_PH_ATTRB_DEST_CPU_MASK) ==
             IH_PH_ATTRB_DEST_CPU_A53_1)
    {
        PartitionFlags |= IH_PART_FLAGS_DEST_CPU_A53_1;
    }
    else if ((PartitionAttributes & IH_PH_ATTRB_DEST_CPU_MASK) ==
             IH_PH_ATTRB_DEST_CPU_A53_2)
    {
        PartitionFlags |= IH_PART_FLAGS_DEST_CPU_A53_2;
    }
    else
    {
        PartitionFlags |= IH_PART_FLAGS_DEST_CPU_A53_3;
    }

    /* Insert magic string */
    if (EntryCount == 0U)
    {
        ATFHandoffParams.MagicValue[0] = 'F';
        ATFHandoffParams.MagicValue[1] = 'M';
        ATFHandoffParams.MagicValue[2] = 'F';
        ATFHandoffParams.MagicValue[3] = 'H';
    }

    ATFHandoffParams.NumEntries = EntryCount + 1U;

    ATFHandoffParams.Entry[EntryCount]
        .EntryPoint = PartitionHeader->DestinationExecutionAddress;
    ATFHandoffParams.Entry[EntryCount].PartitionFlags = PartitionFlags;
}

/*******************************************************************************
 *
 *  This function is used to read image header.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
u32 FmshFsbl_ReadImageHeader (Ps_ImageHeader *ImageHeader,
                              Ps_DeviceOps *DeviceOps,
                              u32 FlashImageOffsetAddress, u32 RunningCpu)
{
    u32 Status = FMSH_SUCCESS;
    u32 ImageHeaderTableAddressOffset = 0U;
    u32 PartitionHeaderAddress = 0U;
    u32 PartitionIndex = 0U;
    u32 GetDstCpu = 0U;
    u32 EntryCount = 0U;
    u32 DestCPU = 0U;
    Ps_PartitionHeader *CurrPartitionHdr = NULL;

    /**
     * Read the Image Header Table offset from
     * Boot Header
     */
    Status = DeviceOps->DeviceCopy(
        FlashImageOffsetAddress + IH_BH_IH_TABLE_OFFSET,
        (uintptr_t)&ImageHeaderTableAddressOffset, 4);
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO, "Device Copy Failed \n\r");
        return Status;
    }
    UART_LOG_OUT(DEBUG_INFO, "Image Header Table Offset 0x%0lx \n\r",
                 ImageHeaderTableAddressOffset);

    /**
     * Read the Image header table of 64 bytes
     * and update the image header table structure
     */
    Status = DeviceOps->DeviceCopy(
        FlashImageOffsetAddress + ImageHeaderTableAddressOffset,
        (uintptr_t) & (ImageHeader->ImageHeaderTable), IH_IHT_LEN);
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO, "Device Copy Failed \n\r");
        return Status;
    }

    /**
     * Check the validity of Image Header Table
     */
    Status = ValidateImageHeaderTable(&(ImageHeader->ImageHeaderTable));
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "Image Header Table "
                     "Validation failed \n\r");
        return Status;
    }

    /**
     * Update the first partition address
     */
    PartitionHeaderAddress = (ImageHeader->ImageHeaderTable
                                  .PartitionHeaderAddress) *
                             4;

    /**
     * Read the partitions based on the partition offset
     * and update the partition header structure
     */
    for (PartitionIndex = 0U;
         PartitionIndex < ImageHeader->ImageHeaderTable.NoOfPartitions;
         PartitionIndex++)
    {
        /**
         * Read the Image header table of 64 bytes
         * and update the image header table structure
         */
        Status = DeviceOps->DeviceCopy(
            FlashImageOffsetAddress + PartitionHeaderAddress,
            (uintptr_t) & (ImageHeader->PartitionHeader[PartitionIndex]), 64);
        if (FMSH_SUCCESS != Status)
        {
            UART_LOG_OUT(DEBUG_INFO, "Device Copy Failed \n\r");
            return Status;
        }

        /**
         * Update the next partition present address
         */
        PartitionHeaderAddress = (ImageHeader->PartitionHeader[PartitionIndex]
                                      .NextPartitionOffset) *
                                 4;

        CurrPartitionHdr = &ImageHeader->PartitionHeader[PartitionIndex];

        GetDstCpu = FmshFsbl_GetDestinationCpu(CurrPartitionHdr);

        if (GetDstCpu == IH_PH_ATTRB_DEST_CPU_NONE)
        {
            DestCPU = RunningCpu;
        }
        else
        {
            DestCPU = GetDstCpu;
        }

        if ((PartitionIndex > 1U) && (EntryCount < 8) &&
            (CurrPartitionHdr->DestinationExecutionAddress != 0U) &&
            (((DestCPU >= IH_PH_ATTRB_DEST_CPU_A53_0) &&
              (DestCPU <= IH_PH_ATTRB_DEST_CPU_A53_3))))
        {
            FmshFsbl_SetATFHandoffParameters(CurrPartitionHdr, EntryCount);
            EntryCount++;
        }

        /**
         * Update the next partition present address
         */
        PartitionHeaderAddress = (ImageHeader->PartitionHeader[PartitionIndex]
                                      .NextPartitionOffset) *
                                 IH_PARTITION_WORD_LENGTH;
    }

    //WriteReg(PMU_GLOBAL_GLOB_GEN_STORAGE6, (u32)((uintptr_t)&ATFHandoffParams));

    return Status;
}
