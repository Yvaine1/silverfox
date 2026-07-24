/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_sha256.c
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

/************************** Function Prototypes ******************************/
/*******************************************************************************
 *
 * This function is used to reset SHA.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static void ShaReset (void)
{
    u32 RstReg = ReadReg(FPS_CSU_BASEADDR + SHA2_COMPUTE_RST_REG);
    WriteReg(FPS_CSU_BASEADDR + SHA2_COMPUTE_RST_REG, RstReg & ~0x00000001);
    WriteReg(FPS_CSU_BASEADDR + SHA2_COMPUTE_RST_REG, RstReg | 0x00000001);
    WriteReg(FPS_CSU_BASEADDR + SHA2_COMPUTE_RST_REG, RstReg & ~0x00000001);
}
/*******************************************************************************
 *
 * This function is used to calculate SHA.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_sha256 (u8* Message, u32 MessageByteLen, u8* Digest)
{
    u32 Status = FMSH_SUCCESS;
    u32 PaddingByteLen = 0U;

    u8 BackUpBuff[72];

    FmshFsbl_SacInit(SAC_AES_DIS_FLAG);
    (void)FmshFsbl_SetSacMode(SAC_SHA256_MODE);

    Status = FmshFsbl_SetSacDataSwap(SAC_RX_FIFO_DATA_BYTE_SWAP);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }
    ShaReset();

    PaddingByteLen = MessageByteLen % SHA_BLOCK_SIZE < SHA_PADDING_BOUNDARY_SIZE
                         ? SHA_BLOCK_SIZE - MessageByteLen % SHA_BLOCK_SIZE
                         : SHA_BLOCK_SIZE * 2 - MessageByteLen % SHA_BLOCK_SIZE;

    /*save data zone */
    (void)memcpy(BackUpBuff, (u8*)((uintptr_t)Message + MessageByteLen),
           PaddingByteLen);
    (void)memset((u8*)((uintptr_t)Message + MessageByteLen), 0U, PaddingByteLen);

    Fmsh_Out8((uintptr_t)Message + MessageByteLen, SHA_FILL_DATA);

    WriteReg((uintptr_t)Message + MessageByteLen + PaddingByteLen - 4U,
             MessageByteLen * 8U);

    FmshFsbl_ByteSwap(
        (u8*)((uintptr_t)Message + MessageByteLen + PaddingByteLen - 4U));

    // CsuInitiateDma((u32)0x20000, (u32) Digest, MessageByteLen/4U,
    // MessageByteLen/4U);
    FmshFsbl_CsuInitiateDma((uintptr_t)Message, (uintptr_t)Digest,
                            (MessageByteLen + PaddingByteLen) / 4U, NULL);

    Status = FmshFsbl_CsuDmaPollDone(IXR_DMA_DONE_MASK, MAX_COUNT);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    *(u32*)Digest = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG0_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG1_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG2_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG3_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG4_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG5_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG6_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG7_OFFSET);
    FmshFsbl_ByteSwap(Digest);

    (void)memcpy((u8*)((uintptr_t)Message + MessageByteLen), BackUpBuff,
           PaddingByteLen);

    return PUF_SHA_OK;
}
/*******************************************************************************
 *
 * This function is used to calculate SHA partly.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_part_sha256 (u8* Message, u32 MessageByteLen, u8* Digest)
{
    u32 Status = FMSH_SUCCESS;
    if (MessageByteLen > 3)
    {
        FmshFsbl_CsuInitiateDma((uintptr_t)Message, (uintptr_t)Digest,
                                MessageByteLen / 4U, NULL);

        Status = FmshFsbl_CsuDmaPollDone(IXR_DMA_DONE_MASK, MAX_COUNT);
        if (Status != FMSH_SUCCESS)
        {
            return FMSH_FAILURE;
        }
    }

    return FMSH_SUCCESS;
}

/*******************************************************************************
 *
 * This function is used to calculate SHA linearly.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
#define TMP_RSA_MESSAGE_SIZE  1024
__attribute__((aligned(32))) u8 tmpMessage[TMP_RSA_MESSAGE_SIZE] = {0};
__attribute__((aligned(32))) u8 padMessage[128] = {0};
u32 FmshFsbl_LinearBurstSha256 (u8* Message, u32 MessageByteLen, u8* Digest)
{
    u32 i = 0;
    u32 indx = 0;
    u32 Status = FMSH_SUCCESS;
    u32 PaddingByteLen = 0U;

    PaddingByteLen = MessageByteLen % SHA_BLOCK_SIZE < SHA_PADDING_BOUNDARY_SIZE
                         ? SHA_BLOCK_SIZE - MessageByteLen % SHA_BLOCK_SIZE
                         : SHA_BLOCK_SIZE * 2 - MessageByteLen % SHA_BLOCK_SIZE;

    FmshFsbl_SacInit(SAC_AES_DIS_FLAG);
    (void)FmshFsbl_SetSacMode(SAC_SHA256_MODE);

    Status = FmshFsbl_SetSacDataSwap(SAC_RX_FIFO_DATA_BYTE_SWAP);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }
    ShaReset();

    for (i = 0; i < MessageByteLen; i++)
    {
        tmpMessage[indx++] = *(Message + i);
        if (indx == TMP_RSA_MESSAGE_SIZE)
        {
            indx = 0;
            (void)FmshFsbl_part_sha256(tmpMessage, TMP_RSA_MESSAGE_SIZE, Digest);
        }
    }
    (void)FmshFsbl_part_sha256(tmpMessage, indx, Digest);

    indx = 0;
    padMessage[indx++] = SHA_FILL_DATA;
    for (i = 1; i < PaddingByteLen - 4; i++)
    {
        padMessage[indx++] = 0;
    }
    padMessage[indx++] = ((MessageByteLen * 8U) >> 24) & 0xff;
    padMessage[indx++] = ((MessageByteLen * 8U) >> 16) & 0xff;
    padMessage[indx++] = ((MessageByteLen * 8U) >> 8) & 0xff;
    padMessage[indx++] = (MessageByteLen * 8U) & 0xff;

    (void)FmshFsbl_part_sha256(padMessage, indx, Digest);

    *(u32*)Digest = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG0_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG1_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG2_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG3_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG4_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG5_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG6_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG7_OFFSET);
    FmshFsbl_ByteSwap(Digest);

    return 0;
}
/*******************************************************************************
 *
 * This function is used to calculate SHA partly.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_NoneLinearBurstSha256 (u8* Message, u32 MessageByteLen, u8* Digest)
{
    u32 i = 0;
    u32 indx = 0;
    u32 Status = FMSH_SUCCESS;
    u32 PaddingByteLen;
    u32 ScrAddr = (uintptr_t)(Message);
    u32 RemainLen=0;

    PaddingByteLen = MessageByteLen % SHA_BLOCK_SIZE < SHA_PADDING_BOUNDARY_SIZE
                         ? SHA_BLOCK_SIZE - MessageByteLen % SHA_BLOCK_SIZE
                         : SHA_BLOCK_SIZE * 2 - MessageByteLen % SHA_BLOCK_SIZE;

    FmshFsbl_SacInit(SAC_AES_DIS_FLAG);
    (void)FmshFsbl_SetSacMode(SAC_SHA256_MODE);

    Status = FmshFsbl_SetSacDataSwap(SAC_RX_FIFO_DATA_BYTE_SWAP);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }
    ShaReset();

    for (i = 0; i < (MessageByteLen / TMP_RSA_MESSAGE_SIZE); i++)
    {
        Status = BootInstance.DeviceOps.DeviceCopy(ScrAddr + TMP_RSA_MESSAGE_SIZE * i,
                                                   (uintptr_t)tmpMessage, TMP_RSA_MESSAGE_SIZE);
        (void)FmshFsbl_part_sha256(tmpMessage, TMP_RSA_MESSAGE_SIZE, Digest);
    }
    RemainLen=MessageByteLen % TMP_RSA_MESSAGE_SIZE;
    if ( RemainLen!= 0)
    {
      if(RemainLen+PaddingByteLen<=TMP_RSA_MESSAGE_SIZE)
      {
        Status = BootInstance.DeviceOps.DeviceCopy(
            ScrAddr + TMP_RSA_MESSAGE_SIZE * i, (uintptr_t)tmpMessage, RemainLen);
        indx = 0;
        tmpMessage[RemainLen+indx++] = SHA_FILL_DATA;
        for (i = 1; i < PaddingByteLen - 4; i++)
        {
            tmpMessage[RemainLen+indx++] = 0;
        }
        tmpMessage[RemainLen+indx++] = ((MessageByteLen * 8U) >> 24) & 0xff;
        tmpMessage[RemainLen+indx++] = ((MessageByteLen * 8U) >> 16) & 0xff;
        tmpMessage[RemainLen+indx++] = ((MessageByteLen * 8U) >> 8) & 0xff;
        tmpMessage[RemainLen+indx++] = (MessageByteLen * 8U) & 0xff;
        (void)FmshFsbl_part_sha256(tmpMessage, RemainLen+PaddingByteLen, Digest);
    }
      else
      {
        Status = BootInstance.DeviceOps.DeviceCopy(
            ScrAddr + TMP_RSA_MESSAGE_SIZE * i, (uintptr_t)tmpMessage, TMP_RSA_MESSAGE_SIZE-SHA_BLOCK_SIZE);
        (void)FmshFsbl_part_sha256(tmpMessage, TMP_RSA_MESSAGE_SIZE-SHA_BLOCK_SIZE, Digest);
        RemainLen=RemainLen+SHA_BLOCK_SIZE-TMP_RSA_MESSAGE_SIZE;
        Status = BootInstance.DeviceOps.DeviceCopy(
            ScrAddr + TMP_RSA_MESSAGE_SIZE * i + TMP_RSA_MESSAGE_SIZE-SHA_BLOCK_SIZE, (uintptr_t)tmpMessage, RemainLen);
        indx = 0;
        tmpMessage[RemainLen+indx++] = SHA_FILL_DATA;
        for (i = 1; i < PaddingByteLen - 4; i++)
        {
            tmpMessage[RemainLen+indx++] = 0;
        }
        tmpMessage[RemainLen+indx++] = ((MessageByteLen * 8U) >> 24) & 0xff;
        tmpMessage[RemainLen+indx++] = ((MessageByteLen * 8U) >> 16) & 0xff;
        tmpMessage[RemainLen+indx++] = ((MessageByteLen * 8U) >> 8) & 0xff;
        tmpMessage[RemainLen+indx++] = (MessageByteLen * 8U) & 0xff;
        (void)FmshFsbl_part_sha256(tmpMessage, SHA_BLOCK_SIZE, Digest);
      }
    }
    else{
    indx = 0;
    padMessage[indx++] = SHA_FILL_DATA;
    for (i = 1; i < PaddingByteLen - 4; i++)
    {
        padMessage[indx++] = 0;
    }
    padMessage[indx++] = ((MessageByteLen * 8U) >> 24) & 0xff;
    padMessage[indx++] = ((MessageByteLen * 8U) >> 16) & 0xff;
    padMessage[indx++] = ((MessageByteLen * 8U) >> 8) & 0xff;
    padMessage[indx++] = (MessageByteLen * 8U) & 0xff;

    (void)FmshFsbl_part_sha256(padMessage, indx, Digest);
    }

    *(u32*)Digest = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG0_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG1_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG2_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG3_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG4_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG5_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG6_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG7_OFFSET);
    FmshFsbl_ByteSwap(Digest);

    return FMSH_SUCCESS;
}

/*******************************************************************************
 *
 * This function is used to calculate SHA.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_SubgroupSha256 (u8* Message, u32 MessageByteLen, u8* Digest)
{
    u32 Status = 0;
    // QSPI NOR
    if (BootInstance.BootMode == QSPI_FLASH)
    {
        Status = FmshFsbl_LinearBurstSha256(Message, MessageByteLen, Digest);
    }
    // SD NAND
    else
    {
        Status = FmshFsbl_NoneLinearBurstSha256(Message, MessageByteLen,
                                                Digest);
    }

    return Status;
}

/*******************************************************************************
 *
 * This function is used to calculate SHA block.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_PlBlockSha256 (u8* Message, u32 MessageByteLen, u32 AcOffset,
                           u8* Digest)
{
    u32 i = 0;
    u32 indx = 0;
    u32 Status = FMSH_SUCCESS;
    u32 PaddingByteLen = 0U;
    u32 PartitionLen = MessageByteLen - RSA_CERTIFICATE_SIZE +
                       RSA_PARTITION_SIGNATURE_SIZE;

    u32 ScrAddr = (uintptr_t)(Message);

    PaddingByteLen = MessageByteLen % SHA_BLOCK_SIZE < SHA_PADDING_BOUNDARY_SIZE
                         ? SHA_BLOCK_SIZE - MessageByteLen % SHA_BLOCK_SIZE
                         : SHA_BLOCK_SIZE * 2 - MessageByteLen % SHA_BLOCK_SIZE;

    FmshFsbl_SacInit(SAC_AES_DIS_FLAG);
    (void)FmshFsbl_SetSacMode(SAC_SHA256_MODE);

    Status = FmshFsbl_SetSacDataSwap(SAC_RX_FIFO_DATA_BYTE_SWAP);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }
    ShaReset();

#ifdef FSBL_PS_DDR
    FmshFsbl_CsuInitiateDma(ScrAddr, (uintptr_t)Digest, PartitionLen / 4U,
                            NULL);

    Status = FmshFsbl_CsuDmaPollDone(IXR_DMA_DONE_MASK, MAX_COUNT);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    // AC HASH
    /*  for(i=0;i<RSA_CERTIFICATE_SIZE  - RSA_PARTITION_SIGNATURE_SIZE;i++)
      {
        tmpMessage[indx++]=*(u8*)(AcOffset+i);
        if(indx==1024)
        {
          indx=0;
          FmshFsbl_part_sha256(tmpMessage,1024,Digest);
        }
      }
      FmshFsbl_part_sha256(tmpMessage,indx,Digest);
     */
#else
    for (i = 0; i < (PartitionLen / TMP_RSA_MESSAGE_SIZE); i++)
    {
        Status = BootInstance.DeviceOps.DeviceCopy(ScrAddr + TMP_RSA_MESSAGE_SIZE * i,
                                                   (uintptr_t)tmpMessage, TMP_RSA_MESSAGE_SIZE);
        if (Status != FMSH_SUCCESS)
        {
            return Status;
        }
        (void)FmshFsbl_part_sha256(tmpMessage, 1024, Digest);
    }
    if (PartitionLen % TMP_RSA_MESSAGE_SIZE != 0)
    {
        Status = BootInstance.DeviceOps.DeviceCopy(
            ScrAddr + TMP_RSA_MESSAGE_SIZE * i, (uintptr_t)tmpMessage, (PartitionLen % TMP_RSA_MESSAGE_SIZE));
        if (Status != FMSH_SUCCESS)
        {
            return Status;
        }
        (void)FmshFsbl_part_sha256(tmpMessage, PartitionLen % 1024, Digest);
    }
    /*PartitionLen=RSA_CERTIFICATE_SIZE  - RSA_PARTITION_SIGNATURE_SIZE;
    for(i=0;i<(PartitionLen/1024);i++)
    {
       Status =
    BootInstance.DeviceOps.DeviceCopy(AcOffset+1024*i,(uintptr_t)tmpMessage,
    1024); if (Status != FMSH_SUCCESS)
         {
            return Status;
         }
       FmshFsbl_part_sha256(tmpMessage,1024,Digest);
    }
    if(PartitionLen%1024!=0)
    {
       Status =
    BootInstance.DeviceOps.DeviceCopy(AcOffset+1024*i,(uintptr_t)tmpMessage,
    (PartitionLen%1024)); if (Status != FMSH_SUCCESS)
         {
            return Status;
         }
       FmshFsbl_part_sha256(tmpMessage,PartitionLen%1024,Digest);
    }*/
#endif

    indx = 0;
    padMessage[indx++] = SHA_FILL_DATA;
    for (i = 1; i < PaddingByteLen - 4; i++)
    {
        padMessage[indx++] = 0;
    }
    padMessage[indx++] = ((MessageByteLen * 8U) >> 24) & 0xff;
    padMessage[indx++] = ((MessageByteLen * 8U) >> 16) & 0xff;
    padMessage[indx++] = ((MessageByteLen * 8U) >> 8) & 0xff;
    padMessage[indx++] = (MessageByteLen * 8U) & 0xff;

    (void)FmshFsbl_part_sha256(padMessage, indx, Digest);

    *(u32*)Digest = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG0_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG1_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG2_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG3_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG4_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG5_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG6_OFFSET);
    FmshFsbl_ByteSwap(Digest);
    Digest += 4;
    *(u32*)(Digest) = ReadReg(FPS_CSU_BASEADDR + SHA2_DIGEST_REG7_OFFSET);
    FmshFsbl_ByteSwap(Digest);

    return FMSH_SUCCESS;
}
