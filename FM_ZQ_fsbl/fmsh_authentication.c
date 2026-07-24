/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_authentication.c
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
extern u8 *BootHdr;
extern u32 BootHeaderSize;
extern u8 BootPerformanceTest;
extern Ps_BootHeader BootHeaderInfo;

u8 EfusePpkHash[HASH_TYPE_SHA3] __attribute__((aligned(4))) = {0U};
u8 EfuseSpkID[SPKID_AC_ALIGN] = {0U};
static u8 *PpkModular = NULL;
// static u8 *PpkModularEx;
static u32 PpkExp = 0U;

FSecureRsa_Config_T Secure_Rsa;

/************************** Function Prototypes ******************************/
/*******************************************************************************
 *
 * This function is used to swap data.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
void ByteSwap (u8 *KeyPtr)
{
    u8 Temp1 = 0U;
    Temp1 = *KeyPtr;
    *KeyPtr = *(KeyPtr + 3U);
    *(KeyPtr + 3U) = Temp1;
    Temp1 = *(KeyPtr + 1U);
    *(KeyPtr + 1U) = *(KeyPtr + 2U);
    *(KeyPtr + 2U) = Temp1;
}

/*******************************************************************************
 * This function is used to read PPK0/PPK1 hash from efuse based on PPK
 * selection.
 *
 * @param	PpkHash is a pointer to an array which holds the readback
 *		PPK hash in.
 * @param	PpkSelect is a u8 variable which has to be provided by user
 *		based on this input reading is happens from efuse PPK0 or PPK1
 * @param	SpKId is a pointer to an array in which SPKID from eFUSE will
 *			stored.
 *
 * @return	None.
 *
 * @note		None.
 *
 *******************************************************************************/
static void ReadPpkHashSpkID (u32 *PpkHash, u8 PpkSelect, u32 *SpkId,
                              u32 SHA2_Select)
{
    u32 RegNum = 0U;
    u32 *DataRead = PpkHash;

    u32 eFuse_row_num = 0U;
    eFuse_row_num = SHA2_Select ? 8 : 12;

    if (PpkSelect == 0U)
    {
        for (RegNum = 0U; RegNum < eFuse_row_num; RegNum++)
        {
            *DataRead = Fmsh_In32(EFUSE_PPK0_START + (RegNum * 4));

            DataRead++;
        }
    }
    else
    {
        for (RegNum = 0U; RegNum < eFuse_row_num; RegNum++)
        {
            *DataRead = Fmsh_In32(EFUSE_PPK1_START + (RegNum * 4));

            DataRead++;
        }
    }

    /* Reading SPK ID */

    *SpkId = Fmsh_In32(EFUSE_SPKID);
}

/*******************************************************************************
 *
 * This function is used to set ppk pointer to ppk in OCM
 *
 * @param	None
 *
 * @return
 *
 * @note		None
 *
 *******************************************************************************/
static void SetPpk (u32 AcOffset)
{
    u8 *PpkPtr = NULL;

    /*
     * Set PpkPtr to PPK in OCM
     */
    PpkPtr = (u8 *)(AcOffset);
    PpkPtr += RSA_HEADER_SIZE;

    /*
     * Increment the pointer by Magic word size
     */
    PpkPtr += RSA_MAGIC_WORD_SIZE;

    /*
     * Set pointer to PPK
     */
    PpkModular = (u8 *)PpkPtr;
    PpkPtr += RSA_PPK_MODULAR_SIZE;

    PpkPtr += RSA_PPK_MODULAR_EXT_SIZE;
    PpkExp = *((u32 *)PpkPtr);

    return;
}
/******************************************************************************
 *
 * This function compares the hashs
 *
 * @param	Hash1 stores the hash to be compared.
 * @param	Hash2 stores the hash to be compared.
 *
 * @return
 * 		Error code on failure
 * 		SUCESS on success
 *
 * @note		None.
 *
 ******************************************************************************/
static u32 CompareHashs (u8 *Hash1, u8 *Hash2, u32 SHA2_Select)
{
    u8 Index = 0U;
    u32 *HashOne = (u32 *)Hash1;
    u32 *HashTwo = (u32 *)Hash2;
    u32 SHA_Length = SHA2_Select ? HASH_TYPE_SHA2 : HASH_TYPE_SHA3;
    for (Index = 0; Index < SHA_Length / 4; Index++)
    {
        if (HashOne[Index] != HashTwo[Index])
        {
            return FMSH_FAILURE;
        }
    }

    return FMSH_SUCCESS;
}
/*******************************************************************************
 *
 * This function is used to verify PPK hash and SPK ID of the partition with
 * the values stored on eFUSE.
 *
 * @param	AcOffset is the Authentication certificate offset which has
 *		AC.
 * @param	HashLen holds the type of authentication enabled.
 *
 * @return	None.
 *
 * @note		None.
 *
 *******************************************************************************/
static u32 PpkSpkIdVer (u32 AcOffset, u32 SHA2_Select)
{
    u8 PpkHash[HASH_TYPE_SHA3] __attribute__((aligned(4)));
    //	void * ShaCtx = (void * )NULL;
    u8 *AcPtr = (u8 *)AcOffset;
    u32 Status = FMSH_SUCCESS;
    u8 *SpkId = (u8 *)(AcPtr + SPKID_AC_ALIGN);
    u32 *SpkIdEfuse = (u32 *)EfuseSpkID;

    (void)memset(PpkHash, 0U, sizeof(PpkHash));
    if (SHA2_Select)
    {
        /* Hash calculation on PPK */
        Status = FmshFsbl_sha256(
            PpkModular,
            RSA_PPK_MODULAR_SIZE + RSA_PPK_MODULAR_EXT_SIZE + RSA_PPK_EXPO_SIZE,
            PpkHash);
    }
    //    else
    //    {
    //        /* Hash calculation on PPK */
    //          Status = FmshFsbl_sha384(PpkModular,
    //          RSA_PPK_MODULAR_SIZE+RSA_PPK_MODULAR_EXT_SIZE+
    //          RSA_PPK_EXPO_SIZE,PpkHash);
    //    }
    if (Status)
    {
        BootInstance.ErrorCode = FSBL_ERROR_PPK_HASH_CAL_TIME_OUT;
        return Status;
    }
    UART_LOG_OUT(DEBUG_INFO, "Calculated PPK hash:\r\n");
    PRINT_ARRAY(DEBUG_INFO, (u8 *)PpkHash, sizeof(PpkHash));
    UART_LOG_OUT(DEBUG_INFO, "Read eFuse PPK hash:\r\n");
    PRINT_ARRAY(DEBUG_INFO, (u8 *)EfusePpkHash, sizeof(EfusePpkHash));

    /* Compare hashs */
    Status = CompareHashs(PpkHash, EfusePpkHash, SHA2_Select);
    if (Status != FMSH_SUCCESS)
    {
        BootInstance.ErrorCode = FSBL_ERROR_PPK_HASH_MISMATCH;
        UART_LOG_OUT(DEBUG_INFO,
                     "PPK hash is not matched and  Verification failed\r\n");
        // ErrorLockDown();
        return Status;
    }
    UART_LOG_OUT(DEBUG_INFO, "PPK hash verification success\r\n");
    /* Compare SPK ID */
    if (*SpkIdEfuse != *(u32 *)SpkId)
    {
        UART_LOG_OUT(
            DEBUG_INFO,
            "SPK ID in eFUSE is:0x%08x,SPK ID in Certificate is:0x%08x. \r\n",
            *SpkIdEfuse, *(u32 *)SpkId);
        UART_LOG_OUT(DEBUG_INFO, "SPK ID verification failed\r\n");
        BootInstance.ErrorCode = FSBL_ERROR_SPK_ID_MISMATCH;
        // ErrorLockDown();
        return Status;
    }
    UART_LOG_OUT(DEBUG_INFO, "SPK ID verification success\r\n");

    return Status;
}

/*******************************************************************************
 *
 * This function recreates the and check signature
 *
 * @param	Partition signature
 * @param	Partition hash value which includes boot header, partition data
 * @return
 *		- SUCCESS if check passed
 *		- FAILURE if check failed
 *
 * @note		None
 *
 *******************************************************************************/
/* PKCS padding for SHA-3 */
static const u8 FSecure_TPadSha3[] = {
    0x30U, 0x41U, 0x30U, 0x0DU, 0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U,
    0x65U, 0x03U, 0x04U, 0x02U, 0x09U, 0x05U, 0x00U, 0x04U, 0x30U};

/* PKCS padding scheme for SHA-2 */
static const u8 FSecure_TPadSha2[] = {
    0x30U, 0x31U, 0x30U, 0x0DU, 0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U,
    0x65U, 0x03U, 0x04U, 0x02U, 0x01U, 0x05U, 0x00U, 0x04U, 0x20U};
static u32 RecreatePaddingAndCheck (u8 *Signature, u8 *Hash, u32 HashLen)
{
    /* Assert validates the input arguments */
    FMSH_ASSERT(Signature != NULL);
    FMSH_ASSERT(Hash != NULL);

    u8 *Tpadding = (u8 *)NULL;
    u32 Pad = FSECURE_FSBL_SIG_SIZE - 3U - 19U - HashLen;
    u8 *PadPtr = Signature;
    u32 sign_index = 0U;
    u32 Status = FMSH_SUCCESS;

    /*
     * Use the latest NIST approved SHA-3 id for padding
     */
    if (FSECURE_HASH_TYPE_SHA3 == HashLen)
    {
        Tpadding = (u8 *)FSecure_TPadSha3;
    }
    else
    {
        Tpadding = (u8 *)FSecure_TPadSha2;
    }

    /*
     * Re-Create PKCS#1v1.5 Padding
     * MSB  ------------------------------------------------------------LSB
     * 0x0 || 0x1 || 0xFF(for 202 bytes) || 0x0 || T_padding || SHA384 Hash
     */

    if (0x00U != *PadPtr)
    {
        Status = FMSH_FAILURE;
        // goto ENDF;
    }
    PadPtr++;

    if (0x01U != *PadPtr)
    {
        Status = FMSH_FAILURE;
        // goto ENDF;
    }
    PadPtr++;

    for (sign_index = 0U; sign_index < Pad; sign_index++)
    {
        if (0xFFU != *PadPtr)
        {
            Status = FMSH_FAILURE;
            // goto ENDF;
        }
        PadPtr++;
    }

    if (0x00U != *PadPtr)
    {
        Status = FMSH_FAILURE;
        // goto ENDF;
    }
    PadPtr++;

    for (sign_index = 0U; sign_index < 19U; sign_index++)
    {
        if (*PadPtr != Tpadding[sign_index])
        {
            Status = FMSH_FAILURE;
            // goto ENDF;
        }
        PadPtr++;
    }

    for (sign_index = 0U; sign_index < HashLen; sign_index++)
    {
        if (*PadPtr != Hash[sign_index])
        {
            Status = FMSH_FAILURE;
            // goto ENDF;
        }
        PadPtr++;
    }

    return Status;
}

/*******************************************************************************
*
* This function is used to authenticate partition.
*
* @param	PartitionOffset

* @return
*		- SUCCESS if check passed
*		- FAILURE if check failed
*
* @note		None
*
*******************************************************************************/
#ifndef FSBL_PS_DDR
__attribute__((aligned(32))) u8 AcBuffer[RSA_CERTIFICATE_SIZE] = {0};
#endif
u32 FmshFsbl_AuthenticatePartition (u32 PartitionOffset, u32 PartitionLen,
                                    u32 AcOffset, u8 PlFlag)
{
    u8 DecryptedSignature[512];
    u8 HashResult[48];
    u8 *SpkModular = NULL;

    // u8 *SpkModularEx;

    u32 SpkExp = 0;
    u8 *SignaturePtr = (u8 *)AcOffset;
    u8 *AcPtr = (u8 *)AcOffset;
    u32 Status = FMSH_SUCCESS;
    u32 EfuseRsaEn = ReadReg(SAC_EFUSE_SEC_CTRL) &
                     SAC_EFUSE_SEC_CTRL_RSA_EN_MASK;
    u32 PartitionLenValid = 0;
    u8 SHA2Select = 0;
    SHA2Select = ((BootInstance.BootHdrAttributes &
                   IH_BH_IMAGE_ATTRB_SHA2_MASK) == IH_BH_IMAGE_ATTRB_SHA2_MASK)
                     ? 0x1
                     : 0x0;
    PartitionLenValid = PartitionLen + RSA_CERTIFICATE_SIZE -
                        RSA_PARTITION_SIGNATURE_SIZE;

#ifndef FSBL_PS_DDR
    if ( (PlFlag & 0x10)  == 0x10)
    {
        // SD NAND
        if ((BootInstance.BootMode == SD_CARD) ||
            (BootInstance.BootMode == NAND_FLASH))
        {
            Status = BootInstance.DeviceOps.DeviceCopy(AcOffset, (u32)AcBuffer,
                                                       RSA_CERTIFICATE_SIZE);
            AcPtr = AcBuffer;
        }
    }
#endif

    if (((FmshFsbl_FindOneInNumber(EfuseRsaEn)) > 0x8) ||
        ((BootHeaderInfo.ImageAttr & IH_BH_IMAGE_ATTRB_RSA_MASK) !=
         IH_BH_IMAGE_ATTRB_RSA_MASK))
    {
        if ((*(u32 *)(AcPtr)&IH_AC_ATTRB_PPK_SELECT_MASK) == 0x00U)
        {
            /* PPK 0 */
            ReadPpkHashSpkID((u32 *)EfusePpkHash, 0U, (u32 *)EfuseSpkID,
                             SHA2Select);
        }
        else
        {
            /* PPK 1 */
            ReadPpkHashSpkID((u32 *)EfusePpkHash, 1U, (u32 *)EfuseSpkID,
                             SHA2Select);
        }

        Status = PpkSpkIdVer((uintptr_t)AcPtr, SHA2Select);
        if (Status != FMSH_SUCCESS)
        {
            return Status;
        }
    }

    SignaturePtr = (void *)((uintptr_t)AcPtr + RSA_HEADER_SIZE +
                            RSA_MAGIC_WORD_SIZE + RSA_PPK_MODULAR_SIZE +
                            RSA_PPK_MODULAR_EXT_SIZE + RSA_PPK_EXPO_SIZE);
    SpkModular = SignaturePtr;
    SignaturePtr += (RSA_SPK_MODULAR_SIZE + RSA_SPK_MODULAR_EXT_SIZE);
    SpkExp = *((u32 *)SignaturePtr);
    SignaturePtr += (RSA_SPK_EXPO_SIZE + RSA_SPK_SIGNATURE_SIZE +
                     RSA_BOOTHEADER_SIGNATURE_SIZE);

    FSecure_RsaInitialize(&Secure_Rsa, (void *)SpkModular, (void *)&SpkExp);
    if (FMSH_SUCCESS != FSecure_RsaPublicEncrypt(&Secure_Rsa,
                                                 (void *)SignaturePtr,
                                                 FSECURE_RSA_4096_SIZE_WORDS,
                                                 (void *)DecryptedSignature))
    {
        UART_LOG_OUT(DEBUG_INFO, "Failed at BH signature decryption\n\r");
        BootInstance.ErrorCode = FSBL_ERROR_RSA_ENCRYPT;
        Status = FMSH_FAILURE;
    }

    /*
     * Calculate partition Hash
     */
#ifdef FSBL_PS_DDR
    if (PlFlag == 0x11)  // pl block
    {
        // Status = FmshFsbl_PlBlockSha256((u8 *)PartitionOffset,
        // PartitionLenValid, AcOffset, HashResult);
        Status = FmshFsbl_sha256((u8 *)PartitionOffset, PartitionLen,
                                 HashResult);
    }
    else
    {
        Status = FmshFsbl_sha256((u8 *)PartitionOffset, PartitionLenValid,
                                 HashResult);
    }
#else
    if (PlFlag == 0x11)  // pl block
    {
        PartitionLenValid = AcOffset - PartitionOffset + RSA_CERTIFICATE_SIZE -
                            RSA_PARTITION_SIGNATURE_SIZE;
        Status = FmshFsbl_PlBlockSha256(
            (u8 *)PartitionOffset, PartitionLenValid, AcOffset, HashResult);
    }
    else if (PlFlag == 0x10)
    {
        Status = FmshFsbl_SubgroupSha256(
            (u8 *)PartitionOffset,
            PartitionLen + RSA_CERTIFICATE_SIZE - RSA_PARTITION_SIGNATURE_SIZE,
            HashResult);
    }
    else
    {
        Status = FmshFsbl_sha256(
            (u8 *)PartitionOffset,
            PartitionLen + RSA_CERTIFICATE_SIZE - RSA_PARTITION_SIGNATURE_SIZE,
            HashResult);
    }
#endif
    if (Status)
    {
        BootInstance.ErrorCode = FSBL_ERROR_SHA_CAL;
        return FMSH_FAILURE;
    }

    UART_LOG_OUT(DEBUG_INFO, "Calculated Partition Hash is :\r\n");
    PRINT_ARRAY(DEBUG_INFO, HashResult, sizeof(HashResult));
    Status = RecreatePaddingAndCheck(
        DecryptedSignature, HashResult,
        SHA2Select ? HASH_TYPE_SHA2 : HASH_TYPE_SHA3);
    if (Status != FMSH_SUCCESS)
    {
        BootInstance.ErrorCode = FSBL_ERROR_PARTITION_SIGNATURE;
        UART_LOG_OUT(DEBUG_INFO,
                     "Partition  Signature Authentication failed\r\n");
        return FMSH_FAILURE;
    }

    UART_LOG_OUT(DEBUG_INFO, "Partition  Signature Authentication Success\r\n");

    return Status;
}

/*******************************************************************************
*
* This function is used to authenticate header.
*
* @param	PartitionOffset

* @return
*		- SUCCESS if check passed
*		- FAILURE if check failed
*
* @note		None
*
*******************************************************************************/
u32 FmshFsbl_AuthenticateHeader (u8 *BootHeaderPtr, u32 PartitionLen,
                                 u32 AcOffset)
{
    u8 DecryptedSignature[512];
    u8 HashResult[48];
    u32 BackUpBuffer[2];
    u32 BootHeaderLength = 0U;
    u8 *SpkModular = NULL;
    // u8 *SpkModularEx;
    u32 SpkExp = 0U;
    u8 *SignaturePtr = (u8 *)AcOffset;
    u32 Status = FMSH_SUCCESS;
    u32 EfuseRsaEn = ReadReg(SAC_EFUSE_SEC_CTRL);
    u8 SHA2Select = 0;
    SHA2Select = ((BootHeaderInfo.ImageAttr & IH_BH_IMAGE_ATTRB_SHA2_MASK) ==
                  IH_BH_IMAGE_ATTRB_SHA2_MASK)
                     ? 0x1
                     : 0x0;

    SetPpk(AcOffset);
    if ((BootHeaderInfo.ImageAttr & IH_BH_IMAGE_ATTRB_RSA_MASK) !=
        IH_BH_IMAGE_ATTRB_RSA_MASK)
    {
        if ((FmshFsbl_FindOneInNumber(EfuseRsaEn) &
             SAC_EFUSE_SEC_CTRL_RSA_EN_MASK) > 0x8)
        {
            if ((*(u32 *)(AcOffset)&IH_AC_ATTRB_PPK_SELECT_MASK) == 0x00U)
            {
                /* PPK 0 */
                ReadPpkHashSpkID((u32 *)EfusePpkHash, 0U, (u32 *)EfuseSpkID,
                                 SHA2Select);
            }
            else
            {
                /* PPK 1 */
                ReadPpkHashSpkID((u32 *)EfusePpkHash, 1U, (u32 *)EfuseSpkID,
                                 SHA2Select);
            }

            Status = PpkSpkIdVer(AcOffset, SHA2Select);
            if (Status != FMSH_SUCCESS)
            {
                return Status;
            }
        }
    }
    SignaturePtr = (u8 *)AcOffset;
    /*
     * Increment the pointer by authentication Header size
     */
    SignaturePtr += RSA_HEADER_SIZE;

    /*
     * Increment the pointer by Magic word size
     */
    SignaturePtr += RSA_MAGIC_WORD_SIZE;

    /*
     * Increment the pointer beyond the PPK
     */
    SignaturePtr += RSA_PPK_MODULAR_SIZE;
    SignaturePtr += RSA_PPK_MODULAR_EXT_SIZE;
    PpkExp = *((u32 *)SignaturePtr);
    SignaturePtr += RSA_PPK_EXPO_SIZE;

    /*
     * Extract SPK signature
     */
    SpkModular = (u8 *)SignaturePtr;
    SignaturePtr += RSA_SPK_MODULAR_SIZE;
    SignaturePtr += RSA_SPK_MODULAR_EXT_SIZE;
    SpkExp = *((u32 *)SignaturePtr);
    SignaturePtr += RSA_SPK_EXPO_SIZE;

    /*
     * Decrypt SPK Signature
     */
    FSecure_RsaInitialize(&Secure_Rsa, (void *)PpkModular,
                          &PpkExp);  
    if (FMSH_SUCCESS != FSecure_RsaPublicEncrypt(&Secure_Rsa,
                                                 (void *)SignaturePtr,
                                                 FSECURE_RSA_4096_SIZE_WORDS,
                                                 (void *)DecryptedSignature))
    {
        UART_LOG_OUT(DEBUG_INFO,
                     "Failed at RSA signature decryption......\n\r");
        BootInstance.ErrorCode = FSBL_ERROR_RSA_ENCRYPT;
        return FMSH_FAILURE;
    }

    (void)memcpy(BackUpBuffer, SignaturePtr, RSA_SPKID_SIZE + RSA_HEADER_SIZE);
    (void)memcpy((void *)(SignaturePtr), (u8 *)AcOffset,
           RSA_SPKID_SIZE + RSA_HEADER_SIZE);

    /* Start the SHA engine */
    if (SHA2Select)
    {
        Status = FmshFsbl_sha256(
            (u8 *)SignaturePtr - (RSA_SPK_MODULAR_EXT_SIZE + RSA_SPK_EXPO_SIZE +
                                  RSA_SPK_MODULAR_SIZE),
            (RSA_SPK_MODULAR_EXT_SIZE + RSA_SPK_EXPO_SIZE +
             RSA_SPK_MODULAR_SIZE + RSA_SPKID_SIZE + RSA_HEADER_SIZE),
            HashResult);
    }
    //        else{
    //            Status = FmshFsbl_sha384((u8 *)SignaturePtr -
    //            (RSA_SPK_MODULAR_EXT_SIZE + RSA_SPK_EXPO_SIZE +
    //            RSA_SPK_MODULAR_SIZE),\
//                      (RSA_SPK_MODULAR_EXT_SIZE + RSA_SPK_EXPO_SIZE +
    //                      RSA_SPK_MODULAR_SIZE + RSA_SPKID_SIZE +
    //                      RSA_HEADER_SIZE),\ HashResult);
    //        }
    if (Status)
    {
        BootInstance.ErrorCode = FSBL_ERROR_SHA_CAL;
        return FMSH_FAILURE;
    }
    Status = RecreatePaddingAndCheck(
        DecryptedSignature, HashResult,
        SHA2Select ? HASH_TYPE_SHA2 : HASH_TYPE_SHA3);
    if (Status != FMSH_SUCCESS)
    {
        BootInstance.ErrorCode = FSBL_ERROR_SPK_SIGNATURE;
        UART_LOG_OUT(DEBUG_INFO,
                     "Partition SPK Signature Authentication failed\r\n");
        // ErrorLockDown();
        return FMSH_FAILURE;
    }
    UART_LOG_OUT(DEBUG_INFO, "SPK Signature Authentication Success\r\n");

    (void)memcpy(SignaturePtr, BackUpBuffer, RSA_HEADER_SIZE + RSA_SPKID_SIZE);
    /* Boot Header signature verification */
    SignaturePtr += RSA_SPK_SIGNATURE_SIZE;

    FSecure_RsaInitialize(&Secure_Rsa, (void *)SpkModular, (void *)&SpkExp);
    if (FMSH_SUCCESS != FSecure_RsaPublicEncrypt(&Secure_Rsa,
                                                 (void *)SignaturePtr,
                                                 FSECURE_RSA_4096_SIZE_WORDS,
                                                 (void *)DecryptedSignature))
    {
        UART_LOG_OUT(DEBUG_INFO, "Failed at RSA signature decryption\n\r");
        BootInstance.ErrorCode = FSBL_ERROR_RSA_ENCRYPT;
        return FMSH_FAILURE;
    }

    /*
     * Calculate BOOT Header Hash
     */
    /* Copy Boot Header to memory.*/
    if ((BootHeaderInfo.ImageAttr & (0x3 << 6)) ==
        (0x3 << 6))  // Boot header has puf helpdata
    {
        BootHeaderLength = BOOT_HEADER_SIZE;
    }
    else
    {
        BootHeaderLength = BOOT_HEADER_WITHOUT_PUF_SIZE;
    }

    if (SHA2Select)
    {
        Status = FmshFsbl_sha256(BootHeaderPtr, BootHeaderLength, HashResult);
    }
    //        else{
    //            Status = FmshFsbl_sha384(BootHeaderPtr, BootHeaderLength,
    //            HashResult);
    //        }

    if (Status)
    {
        BootInstance.ErrorCode = FSBL_ERROR_SHA_CAL;
        return FMSH_FAILURE;
    }

    Status = RecreatePaddingAndCheck(
        DecryptedSignature, HashResult,
        SHA2Select ? HASH_TYPE_SHA2 : HASH_TYPE_SHA3);
    if (Status != FMSH_SUCCESS)
    {
        BootInstance.ErrorCode = FSBL_ERROR_BOOT_HEADER_SIGNATURE;
        UART_LOG_OUT(DEBUG_INFO,
                     "Boot Header Signature Authentication failed\r\n");
        return FMSH_FAILURE;
    }

    UART_LOG_OUT(DEBUG_INFO,
                 "Boot Header  Signature Authentication Success\r\n");

    return Status;
}
