/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_sac.c
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
 * 0.01   lq  08/28/2022  First Release.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef FSBL_PS_DDR
extern u32 AesSm4GenTag(u8 Alg, const u8* In, u32 InByteLen, u32 KeySel, u8* Iv,
                        u8* Tag);
#endif
#ifndef FSBL_PS_DDR
__attribute__((aligned(32))) u8 g_devcCfgTmpBuffer[TMP_PL_BUF_LEN] = {0};
#endif
/************************** Variable Definitions *****************************/
FDevcPs_T g_DEVC;

/************************** Function Prototypes ******************************/
/******************************************************************************
 *
 * This function is used to initialize sac.
 *
 * @param    InitFlag is aes enable flag.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_SacInit (u32 InitFlag)
{
    u32 CfgReg = 0;
    FmshFsbl_SacUnlock();
    FmshFsbl_SacAesSwitch(InitFlag);
    CfgReg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET);
    FMSH_WriteReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET, CfgReg | 0x000000c0);
}

/******************************************************************************
 *
 * This function is used to unlock sac.
 *
 * @param         None.
 *
 * @return	 None.
 *
 ******************************************************************************/
void FmshFsbl_SacUnlock (void)
{
    FMSH_WriteReg(FPS_CSU_BASEADDR, SAC_UNLOCK_CONS_REG_OFFSET, SAC_UNLOCK);
}

/******************************************************************************
 *
 * This function is used to set the mode of sac.
 *
 * @param         Mode is AES-GCM mode.
 *
 * @return
 *               - FMSH_SUCCESS if set correctly
 *		- FMSH_FAILURE if set failed
 *
 *******************************************************************************/
u32 FmshFsbl_SetSacMode (u32 Mode)
{
    u32 CfgReg = 0U;
    u32 Status = FMSH_SUCCESS;
    CfgReg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET);
    CfgReg &= ~SAC_DATA_OP_MASK;
    CfgReg |= Mode; /*config AES-GCM mode*/
    FMSH_WriteReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET, CfgReg);
    CfgReg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET);
    CfgReg &= SAC_DATA_OP_MASK;
    if (CfgReg != Mode)
    {
        Status = FMSH_FAILURE;
    }

    return Status;
}

/******************************************************************************
 *
 * This function is used to set data swap mode of sac.
 *
 * @param         Mode is data swap mode.
 *
 * @return
 *               - FMSH_SUCCESS if set correctly
 *		- FMSH_FAILURE if set failed
 *
 *******************************************************************************/
u32 FmshFsbl_SetSacDataSwap (u32 Mode)
{
    u32 CfgReg = 0U;
    u32 Status = FMSH_SUCCESS;
    CfgReg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET);
    CfgReg &= ~SAC_DATA_SWAP_MASK;
    CfgReg |= Mode; /*config AES-GCM mode*/
    FMSH_WriteReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET, CfgReg);
    if ((FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_CFG_REG_OFFSET) &
         SAC_DATA_SWAP_MASK) != Mode)
    {
        Status = FMSH_FAILURE;
    }

    return Status;
}

/******************************************************************************
 *
 * This function is used to swap data.
 *
 * @param         KeyPtr is a pointer to key.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_ByteSwap (u8* KeyPtr)
{
    u8 Temp1 = 0U;
    Temp1 = *KeyPtr;
    *KeyPtr = *(KeyPtr + 3U);
    *(KeyPtr + 3U) = Temp1;
    Temp1 = *(KeyPtr + 1U);
    *(KeyPtr + 1U) = *(KeyPtr + 2U);
    *(KeyPtr + 2U) = Temp1;
}

/******************************************************************************
 *
 * This function is used to enable or disable aes.
 *
 * @param         Flag is AES enable flag.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_SacAesSwitch (u32 Flag)
{
    u32 CfgReg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_CTRL_REG_OFFSET);
    CfgReg &= ~SAC_AES_EN_MASK;
    if (Flag == SAC_AES_EN_FLAG)
    {
        CfgReg |= SAC_AES_EN;
    }
    FMSH_WriteReg(FPS_CSU_BASEADDR, SAC_CTRL_REG_OFFSET, CfgReg);
}

/******************************************************************************
 *
 * This function is used to open cfg lvl shifter.
 *
 * @param         None.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_OpenCfgLevelShifter (void) {}

/******************************************************************************
 *
 * This function is used to open pl por lvl shifter.
 *
 * @param         None.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_OpenPlPorLevelShifter (void) {}

/******************************************************************************
 *
 * This function is used to close usr lvl shifter.
 *
 * @param         None.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_CloseUsrLevelShifter (void) {}

/******************************************************************************
 *
 * This function is used to open usr lvl shifter.
 *
 * @param         None.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_OpenUsrLevelShifter (void) {}
/******************************************************************************
 *
 * This function is used to open usr lvl shifter.
 *
 * @param         None.
 *
 * @return	 None.
 *
 *******************************************************************************/
void FmshFsbl_EnablePJtag (void) {}

/******************************************************************************
 *
 * This function is used to close usr lvl shifter.
 *
 * @param         None.
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_IsBitDone (void)
{
    u32 Reg = FMSH_ReadReg(FPS_CSU_BASEADDR, SAC_STATUS_REG_OFFSET);
    if ((Reg & SAC_PCFG_DONE_MASK) == SAC_PCFG_DONE_MASK)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/******************************************************************************
 *
 * This function is used to initialize g_DEVC.
 *
 * @param         None.
 *
 * @return
 *               - FMSH_SUCCESS if set correctly
 *		- FMSH_FAILURE if set failed
 *
 *******************************************************************************/
u32 FmshFsbl_InitDevc (void)
{
    u32 Status = FMSH_SUCCESS;
    FDevcPs_Config* Config = NULL;
    Config = FDevcPs_LookupConfig(FPAR_DEVCPS_DEVICE_ID);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }

    Status = FDevcPs_init(&g_DEVC, Config);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    return FMSH_SUCCESS;
}

u32 FDevcPs_noneSecureDownload (FDevcPs_T* pDevc, u32 srcAddress, u32 len)
{
    u32 Status = FMSH_SUCCESS;

    FDevcPs_readFifoThre(pDevc, (enum readFifoThre)0);
    Status = FDevcPs_fabricInit(pDevc, FMSH_NON_SECURE_PCAP_WRITE);
    if (FMSH_SUCCESS != Status)
    {
        UART_LOG_OUT(DEBUG_INFO, "Devc init failed!\r\n");
        return Status;
    }
#ifdef FSBL_PS_DDR
    Status = FDevcPs_pcapLoadPartition(pDevc, srcAddress, PCAP_WR_DATA_ADDR,
                                       len, len, DOWNLOAD_BITSTREAM);
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }
#else
    u32 i = 0;
    if (BootInstance.BootMode == NAND_FLASH || BootInstance.BootMode == SD_CARD)
    {
        for (i = 0; i < len * 4 / TMP_PL_BUF_LEN; i++)
        {
            BootInstance.DeviceOps.DeviceCopy(srcAddress + TMP_PL_BUF_LEN * i,
                                              (uintptr_t)g_devcCfgTmpBuffer,
                                              TMP_PL_BUF_LEN);
            Status = FDevcPs_pcapLoadPartition(
                pDevc, (uintptr_t)g_devcCfgTmpBuffer, PCAP_WR_DATA_ADDR,
                TMP_PL_BUF_LEN / 4, TMP_PL_BUF_LEN / 4, DOWNLOAD_BITSTREAM);
            if (FMSH_SUCCESS != Status)
            {
                return Status;
            }
        }
        if (len * 4 % TMP_PL_BUF_LEN != 0)
        {
            BootInstance.DeviceOps.DeviceCopy(srcAddress + TMP_PL_BUF_LEN * i,
                                              (uintptr_t)g_devcCfgTmpBuffer,
                                              TMP_PL_BUF_LEN);
            Status = FDevcPs_pcapLoadPartition(
                pDevc, (uintptr_t)g_devcCfgTmpBuffer, PCAP_WR_DATA_ADDR,
                len % (TMP_PL_BUF_LEN / 4), len % (TMP_PL_BUF_LEN / 4),
                DOWNLOAD_BITSTREAM);
            if (FMSH_SUCCESS != Status)
            {
                return Status;
            }
        }
    }
    else if (BootInstance.BootMode == QSPI_FLASH)
    {
        Status = FDevcPs_pcapLoadPartition(
            pDevc, FPS_QSPI0_D_BASEADDR + srcAddress, PCAP_WR_DATA_ADDR, len,
            len, DOWNLOAD_BITSTREAM);
        if (FMSH_SUCCESS != Status)
        {
            return Status;
        }
    }
    else
    {
        return FMSH_FAILURE;
    }
#endif

    Status = FDevcPs_pollFpgaDone(pDevc, DEVC_POLL_DONE_MS);
    if (FMSH_SUCCESS != Status)
    {
        return Status;
    }
    return FMSH_SUCCESS;
}

u32 GcmDecTag (u8 Alg, const u8* In, u32 InByteLen, u32 KeySel, u8* Iv,
               const u8* Tag)
{
    u32 Status = FMSH_SUCCESS;
    u32 cnt = 0U;

    u8 GenTag[SECURE_GCM_TAG_SIZE];

    FmshFsbl_SacInit(SAC_AES_EN_FLAG);
#ifdef FSBL_PS_DDR
    Status = AesSm4GenTag(Alg, In, InByteLen, KeySel, Iv, GenTag);
#else
    if (BootInstance.BootMode == NAND_FLASH || BootInstance.BootMode == SD_CARD)
    {
        u8 tmpTag[16] = {0};
        BootInstance.DeviceOps.DeviceCopy((uintptr_t)Tag, (uintptr_t)tmpTag,
                                          16);
        Tag = tmpTag;
        Status = PlCalcTagNoLinear(Alg, In, InByteLen, KeySel, Iv, GenTag);
    }
#endif
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    UART_LOG_OUT(DEBUG_INFO, "Ciphertext GCM Tag is :\r\n");
    PRINT_ARRAY(DEBUG_INFO, Tag, 16);
    // LOG_OUT (DEBUG_INFO, "Calculated Ciphertext GCM Tag is :\r\n");
    // PRINT_ARRAY (DEBUG_INFO, GenTag, 16);

    /*compare generated tag and received tag */
    for (cnt = 0; cnt < SECURE_GCM_TAG_SIZE; cnt++)
    {
        if (GenTag[cnt] != *Tag++)
        {
            // LOG_OUT(DEBUG_INFO,"GCM Tag is not Matched \r\n");

            Status = GCM_ERROR;
            return Status;
        }
    }
    // LOG_OUT(DEBUG_INFO,"GCM Tag is Matched, Starting Decryption...... \r\n");

    return Status;
}
/****************************************************************************/
/**
 *
 * This function will handle the AES/SM4-GCM Decryption.
 *
 * The Multiple key(a.k.a Key Rolling) or Single key
 * Encrypted images will have the same format,
 * such that it will have the following:
 *
 * Secure head -->	Dummy AES/SM4 Key of 32byte +
 * 					Block 0 IV of 12byte +
 * 					DLC for Block 0 of 4byte +
 * 					GCM tag of 16byte(Un-Enc).
 * Block N --> Boot Image Data for Block N of n size +
 * 			Block N+1 AES key of 32byte +
 * 			Block N+1 IV of 12byte +
 * 			GCM tag for Block N of 16byte(Un-Enc).
 *
 * The Secure head and Block 0 will be decrypted using
 * Device key or user provide key.
 * If more than 1 blocks are found then the key and IV
 * obtained from previous block will be used for decryption
 *
 *
 * 1> Read the 1st 64bytes and decrypt 48 bytes using
 * 	the selected Device key.
 * 2> Decrypt the 0th block using the IV + Size from step 2
 * 	and selected device key.
 * 3> After decryption we will get decrypted data+KEY+IV+Blk
 * 	Size so store the KEY/IV into KUP/IV registers.
 * 4> Using Blk size, IV and Next Block Key information
 * 	start decrypting the next block.
 * 5> if the Current Image size > Total image length,
 * 	go to next step 8. Else go back to step 5
 * 6> If there are failures, return error code
 * 7> If we have reached this step means the decryption is SUCCESS.
 *
 * @para
 * @para
 * @para
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u32 keyRolling_download (FDevcPs_T* pDevc, enum ALG alg, enum DECFLAG decflag,
                         u32 srcPtr, u32 bitlen)
{
    u8 Status = FMSH_SUCCESS;
    u32 block_no = 0U;
    u32 head = 0U;
    u32 next_len = 0U;
    u32 current_totalLen = 0U;
    u8 Y1[16];

    u8* In = (u8*)(srcPtr);
    u8* Tag = (u8*)(srcPtr + 48);
    u8 Iv[12];
    u8 Alg = 0U;

    if (alg == AES)
    {
        Alg = ALG_AES;
    }
    else
    {
        Alg = ALG_SM4;
    }

    /*pull down Prog_B, then pull up*/
    // FDevcPs_Prog_B(pDevc);

    FDevcPs_smap32Swap(pDevc, smap32_swap_enable);
    FDevcPs_secDownDataByteSwap(pDevc, byte_swap);
    FDevcPs_txDataSwap(pDevc, byte_swap);
    FDevcPs_rxDataSwap(pDevc, byte_swap);

    FDevcPs_RDWR_B_LOW(pDevc);
    FDevcPs_CSI_B_LOW(pDevc);

    /*Verify Secure Header Tag*/
    FmshFsbl_SacInit(SAC_AES_EN_FLAG);
    FDevcPs_setKeySource(pDevc, DEV_KEY);
    FDevcPs_loadKeyIV(pDevc);

    /*Geg IV from SAC IV register*/
    GetSacIv(Iv);
    Status = GcmDecTag(Alg, In, 48, SECURE_CSU_AES_KEY_SRC_DEV, Iv, Tag);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    /*GCM_EN = 0*/
    FDevcPs_disableGcm(pDevc);
    FDevcPs_downloadMode(pDevc, SECURE_DOWNLOAD_BITSTREAM);
    FDevcPs_setGcmChMode(pDevc, CTR);
    FDevcPs_setGcmAlg(pDevc, alg);
    FDevcPs_setGcmMode(pDevc, DCODE);
    FDevcPs_setKeySource(pDevc, DEV_KEY);
    IvToY1(Iv, Y1);
    SetSacIv(Y1);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_IVUP0_OFFSET, 0x2);
    FDevcPs_loadKeyIV(pDevc);

    /*set dec flag = secure head,only iv_write_en,no use op_key*/
    FDevcPs_setDecFlag(pDevc, decflag);

    /*GCM_EN = 1*/
    FDevcPs_enableGcm(pDevc);
    /***********************HEADER********************************/
    head = 0;

    // head is 12 word
#ifdef FSBL_PS_DDR
    Status = FDevcPs_initiateDma(pDevc, (u32)srcPtr, PCAP_WR_DATA_ADDR, 12, 12);
#else
    if (BootInstance.BootMode == NAND_FLASH || BootInstance.BootMode == SD_CARD)
    {
        BootInstance.DeviceOps.DeviceCopy((uintptr_t)srcPtr,
                                          (uintptr_t)g_devcCfgTmpBuffer, 48);
        Status = FDevcPs_initiateDma(pDevc, (uintptr_t)g_devcCfgTmpBuffer,
                            PCAP_WR_DATA_ADDR, 12, 12);
    }
    else
    {
        Status =  FDevcPs_initiateDma(pDevc, (u32)srcPtr, PCAP_WR_DATA_ADDR, 12, 12);
    }
#endif
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }
    
    /*GCM_EN = 0*/
    FDevcPs_disableGcm(pDevc);
    /***********************BLOCK0 - N********************************/
    block_no = 0;
    head += 16 * 4;
    current_totalLen = 16;
    next_len = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_IVUP0_OFFSET);

    devc_byte_swap((u32*)&next_len, 1);  // get next block length

    while ((current_totalLen < bitlen) && (next_len != 0))
    {
        FDevcPs_setDecFlag(pDevc, clear);
        if (block_no == 0)
        {
            /*Verify Tag 0*/
            FmshFsbl_SacInit(SAC_AES_EN_FLAG);
            GetSacIv(Iv);
            In = (u8*)(srcPtr + head);
            Tag = (u8*)(srcPtr + head + next_len * 4 + 48);
            Status = GcmDecTag(Alg, In, next_len * 4 + 48,
                               SECURE_CSU_AES_KEY_SRC_DEV, Iv, Tag);
            if (Status != FMSH_SUCCESS)
            {
                return Status;
            }
            /*GCM_EN = 0*/
            FDevcPs_disableGcm(pDevc);
            FDevcPs_downloadMode(pDevc, SECURE_DOWNLOAD_BITSTREAM);
            FDevcPs_setGcmChMode(pDevc, CTR);
            FDevcPs_setGcmAlg(pDevc, alg);
            FDevcPs_setGcmMode(pDevc, DCODE);

            /***********************BLOCK0********************************/
            // set key_src=DEV 4'b0001
            FDevcPs_setKeySource(pDevc, DEV_KEY);
            IvToY1(Iv, Y1);
            SetSacIv(Y1);
            // load the key and iv
            FDevcPs_loadKeyIV(pDevc);
            // set dec flag = blk0
            FDevcPs_setDecFlag(pDevc, ivup_kup_wr_en);
            // set en=1
            FDevcPs_enableGcm(pDevc);

#ifdef FSBL_PS_DDR
            Status = FDevcPs_initiateDma(pDevc, (u32)srcPtr + head, PCAP_WR_DATA_ADDR,
                                next_len + 12, next_len + 12);
            if (Status != FMSH_SUCCESS)
            {
                return Status;
            }
#else
            if (BootInstance.BootMode == NAND_FLASH ||
                BootInstance.BootMode == SD_CARD)
            {
                u32 i = 0;
                FDevcPs_setDecFlag(pDevc, clear);
                for (i = 0; i < next_len * 4 / TMP_PL_BUF_LEN; i++)
                {
                    BootInstance.DeviceOps.DeviceCopy(
                        (uintptr_t)srcPtr + head + TMP_PL_BUF_LEN * i,
                        (uintptr_t)g_devcCfgTmpBuffer, TMP_PL_BUF_LEN);
                    Status = FDevcPs_initiateDma(pDevc, (uintptr_t)g_devcCfgTmpBuffer,
                                        PCAP_WR_DATA_ADDR, TMP_PL_BUF_LEN / 4,
                                        TMP_PL_BUF_LEN / 4);
                    if (Status != FMSH_SUCCESS)
                    {
                        return Status;
                    }
                }
                if (next_len * 4 % TMP_PL_BUF_LEN != 0)
                {
                    BootInstance.DeviceOps.DeviceCopy(
                        (uintptr_t)srcPtr + head + TMP_PL_BUF_LEN * i,
                        (uintptr_t)g_devcCfgTmpBuffer,
                        next_len * 4 % TMP_PL_BUF_LEN);
                    Status = FDevcPs_initiateDma(pDevc, (uintptr_t)g_devcCfgTmpBuffer,
                                        PCAP_WR_DATA_ADDR,
                                        next_len % (TMP_PL_BUF_LEN / 4),
                                        next_len % (TMP_PL_BUF_LEN / 4));
                    if (Status != FMSH_SUCCESS)
                    {
                        return Status;
                    }
                }
                // KEY & IV
                BootInstance.DeviceOps.DeviceCopy(
                    (uintptr_t)srcPtr + head + next_len * 4,
                    (uintptr_t)g_devcCfgTmpBuffer, 48);
                FDevcPs_setDecFlag(pDevc, ivup_kup_wr_en);
                Status = FDevcPs_initiateDma(pDevc, (uintptr_t)g_devcCfgTmpBuffer,
                                    PCAP_WR_DATA_ADDR, 12, 12);
                if (Status != FMSH_SUCCESS)
                {
                    return Status;
                }
            }
            else
            {
                Status = FDevcPs_initiateDma(pDevc, (u32)srcPtr + head,
                                    PCAP_WR_DATA_ADDR, next_len + 12,
                                    next_len + 12);
                if (Status != FMSH_SUCCESS)
                {
                    return Status;
                }
            }
#endif
            // set en=0
            FDevcPs_disableGcm(pDevc);
        }
        else
        {
            /*Verify Tag */
            FmshFsbl_SacInit(SAC_AES_EN_FLAG);
            GetSacIv(Iv);
            In = (u8*)(srcPtr + head);
            Tag = (u8*)(srcPtr + head + next_len * 4 + 48);

            Status = GcmDecTag(Alg, In, next_len * 4 + 48,
                               SECURE_CSU_AES_KEY_SRC_KUP, Iv, Tag);
            // Status=AesSm4GcmDec(Alg,AES_KUP_IV_WRITE_FLAG,In,next_len*4+48
            // ,Out,SECURE_CSU_AES_KEY_SRC_DEV,Iv,Tag);
            if (Status != FMSH_SUCCESS)
            {
                return Status;
            }
            /*GCM_EN = 0*/
            FDevcPs_disableGcm(pDevc);
            FDevcPs_downloadMode(pDevc, SECURE_DOWNLOAD_BITSTREAM);
            FDevcPs_setGcmChMode(pDevc, CTR);
            FDevcPs_setGcmAlg(pDevc, alg);
            FDevcPs_setGcmMode(pDevc, DCODE);

            /***********************BLOCK0********************************/
            // set key_src=DEV 4'b0001
            FDevcPs_setKeySource(pDevc, KUP);
            IvToY1(Iv, Y1);
            SetSacIv(Y1);

            // load the key and iv
            FDevcPs_loadKeyIV(pDevc);
            // set dec flag = blk0
            FDevcPs_setDecFlag(pDevc, ivup_kup_wr_en);
            /*GCM_EN = 1*/
            FDevcPs_enableGcm(pDevc);

            /***********************BLOCK1-N********************************/
            // set key_src=DEV 4'b0001
            // FDevcPs_setKeySource(pDevc, KUP);

            // DEVC_OUT32P(0x2, portmap->IVUP0);
            // load the key and iv
            // FDevcPs_loadKeyIV(pDevc);

            // set en=1
            // FDevcPs_enableGcm(pDevc);
            // set dec flag = blk1
            // FDevcPs_setDecFlag(pDevc, ivup_kup_wr_en);

#ifdef FSBL_PS_DDR
            Status = FDevcPs_initiateDma(pDevc, (u32)srcPtr + head, PCAP_WR_DATA_ADDR,
                                next_len + 12, next_len + 12);
            if (Status != FMSH_SUCCESS)
            {
                return Status;
            }
#else
            if (BootInstance.BootMode == NAND_FLASH ||
                BootInstance.BootMode == SD_CARD)
            {
                u32 j = 0;
                FDevcPs_setDecFlag(pDevc, clear);
                for (j = 0; j < next_len * 4 / TMP_PL_BUF_LEN; j++)
                {
                    BootInstance.DeviceOps.DeviceCopy(
                        (uintptr_t)srcPtr + head + TMP_PL_BUF_LEN * j,
                        (uintptr_t)g_devcCfgTmpBuffer, TMP_PL_BUF_LEN);
                    Status = FDevcPs_initiateDma(pDevc, (uintptr_t)g_devcCfgTmpBuffer,
                                        PCAP_WR_DATA_ADDR, TMP_PL_BUF_LEN / 4,
                                        TMP_PL_BUF_LEN / 4);
                    if (Status != FMSH_SUCCESS)
                    {
                        return Status;
                    }
                }
                if (next_len * 4 % TMP_PL_BUF_LEN != 0)
                {
                    BootInstance.DeviceOps.DeviceCopy(
                        (uintptr_t)srcPtr + head + TMP_PL_BUF_LEN * j,
                        (uintptr_t)g_devcCfgTmpBuffer,
                        next_len * 4 % TMP_PL_BUF_LEN);
                    Status = FDevcPs_initiateDma(pDevc, (uintptr_t)g_devcCfgTmpBuffer,
                                        PCAP_WR_DATA_ADDR,
                                        next_len % (TMP_PL_BUF_LEN / 4),
                                        next_len % (TMP_PL_BUF_LEN / 4));
                    if (Status != FMSH_SUCCESS)
                    {
                        return Status;
                    }
                }
                // key & IV
                BootInstance.DeviceOps.DeviceCopy(
                    (uintptr_t)srcPtr + head + next_len * 4,
                    (uintptr_t)g_devcCfgTmpBuffer, 48);
                FDevcPs_setDecFlag(pDevc, ivup_kup_wr_en);
                Status = FDevcPs_initiateDma(pDevc, (uintptr_t)g_devcCfgTmpBuffer,
                                    PCAP_WR_DATA_ADDR, 12, 12);
                if (Status != FMSH_SUCCESS)
                {
                    return Status;
                }
            }
            else
            {
                Status = FDevcPs_initiateDma(pDevc, (u32)srcPtr + head,
                                    PCAP_WR_DATA_ADDR, next_len + 12,
                                    next_len + 12);
                if (Status != FMSH_SUCCESS)
                {
                    return Status;
                }
            }
#endif
            // set en=0
            FDevcPs_disableGcm(pDevc);
        }
        block_no++;

        head += (next_len + 16) * 4;
        current_totalLen += (next_len + 16);

        next_len = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_IVUP0_OFFSET);
        devc_byte_swap((u32*)&next_len, 1);  // get next block length
    }

    FDevcPs_setDecFlag(pDevc, clear);        /*clear DEC flag*/

    Status = FDevcPs_pollFpgaDone(pDevc, DEVC_POLL_DONE_MS);

    return Status;
}

u32 FDevcPs_encryptDownload_AES_NoOp (FDevcPs_T* pDevc, u32* devc_iv,
                                      u32 srcPtr, u32 bitlen)
{
    u8 Status = FMSH_SUCCESS;

    FDevcPs_IV(pDevc, devc_iv, 3);
    Status = keyRolling_download(pDevc, AES, no_opkey, srcPtr, bitlen);
    return Status;
}

u32 FDevcPs_encryptDownload_AES_UseOp (FDevcPs_T* pDevc, u32* devc_iv,
                                       u32 srcPtr, u32 bitlen)
{
    u8 Status = FMSH_SUCCESS;

    FDevcPs_IV(pDevc, devc_iv, 3);
    Status = keyRolling_download(pDevc, AES, use_opkey, srcPtr, bitlen);
    return Status;
}

u32 FDevcPs_encryptDownload_SM4_NoOp (FDevcPs_T* pDevc, u32* devc_iv,
                                      u32 srcPtr, u32 bitlen)
{
    u8 Status = FMSH_SUCCESS;

    FDevcPs_IV(pDevc, devc_iv, 3);
    Status = keyRolling_download(pDevc, SM4, no_opkey, srcPtr, bitlen);
    return Status;
}

u32 FDevcPs_encryptDownload_SM4_UseOp (FDevcPs_T* pDevc, u32* devc_iv,
                                       u32 srcPtr, u32 bitlen)
{
    u8 Status = FMSH_SUCCESS;

    FDevcPs_IV(pDevc, devc_iv, 3);
    Status = keyRolling_download(pDevc, SM4, use_opkey, srcPtr, bitlen);
    return Status;
}
