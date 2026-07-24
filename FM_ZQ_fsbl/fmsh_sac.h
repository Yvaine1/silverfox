/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_sac.h
 *
 * This file contains header fmsh_common.h
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

#ifndef _FMSH_SAC_H_
#define _FMSH_SAC_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_devc_lib.h"

/************************** Constant Definitions *****************************/
#define SAC_CTRL_REG_OFFSET                (0x08)
#define SAC_CFG_REG_OFFSET                 (0x0C)
#define SAC_STATUS_REG_OFFSET              (0x18)
#define SAC_UNLOCK_CONS_REG_OFFSET         (0x2c)

#define SAC_UNLOCK                         (0x757bdf0dU)
#define SAC_PCFG_DONE_MASK                 (0x0200)
#define SAC_AES_EN_MASK                    (0x00000e00)
#define SAC_AES_EN                         (0x00000e00)
#define SAC_AES_EN_FLAG                    (1U)
#define SAC_AES_DIS_FLAG                   (0U)

#define SAC_DATA_OP_MASK                   (0x00f00000)
#define SAC_DATA_SWAP_MASK                 (0x03060000)

#define SAC_AES_GCM_MODE                   (0x00400000)
#define SAC_SECURE_BITSTREAM_DOWNLOAD_MODE (0x00800000)
#define SAC_SHA256_MODE                    (0x00600000)
#define SAC_MULTH_MODE                     (0x00700000)

#define SAC_TX_FIFO_DATA_BYTE_SWAP         (0x02000000)
#define SAC_RX_FIFO_DATA_BYTE_SWAP         (0x00040000)
#define SAC_NO_FIFO_DATA_BYTE_SWAP         (0x00000000)

/*SAC register offset definition*/

/*DMA register*/
#define SAC_DMA_SRC_ADDR_OFFSET            (0x1C)
#define SAC_DMA_DEST_ADDR_OFFSET           (0x20)
#define SAC_DMA_SRC_LEN_OFFSET             (0x24)
#define SAC_DMA_DEST_LEN_OFFSET            (0x28)
#define SAC_INT_STS_OFFSET                 (0x10)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void FmshFsbl_SacInit(u32 InitFlag);
void FmshFsbl_SacUnlock(void);
u32 FmshFsbl_SetSacMode(u32 Mode);
u32 FmshFsbl_SetSacDataSwap(u32 Mode);
void FmshFsbl_ByteSwap(u8 *KeyPtr);
void FmshFsbl_SacAesSwitch(u32 Flag);
void FmshFsbl_OpenCfgLevelShifter(void);
void FmshFsbl_OpenPlPorLevelShifter(void);
void FmshFsbl_CloseUsrLevelShifter(void);
void FmshFsbl_OpenUsrLevelShifter(void);
void FmshFsbl_EnablePJtag(void);
u32 FmshFsbl_IsBitDone(void);
u32 FDevcPs_noneSecureDownload(FDevcPs_T *pDevc, u32 srcAddress, u32 len);
u32 FDevcPs_encryptDownload_AES_NoOp(FDevcPs_T *pDevc, u32 *devc_iv, u32 srcPtr,
                                     u32 bitlen);
u32 FDevcPs_encryptDownload_AES_UseOp(FDevcPs_T *pDevc, u32 *devc_iv,
                                      u32 srcPtr, u32 bitlen);
u32 FDevcPs_encryptDownload_SM4_NoOp(FDevcPs_T *pDevc, u32 *devc_iv, u32 srcPtr,
                                     u32 bitlen);
u32 FDevcPs_encryptDownload_SM4_UseOp(FDevcPs_T *pDevc, u32 *devc_iv,
                                      u32 srcPtr, u32 bitlen);
u32 FmshFsbl_InitDevc(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
