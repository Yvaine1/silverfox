/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_sha.h
 *
 * This file contains header fmsh_uart_private.h & fmsh_uart_public.h
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
#ifndef _FMSH_SHA_H_
#define _FMSH_SHA_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include <stdint.h>
#include <stdio.h>

#include "fmsh_common.h"
#include "fmsh_csudma.h"
#include "fmsh_sac.h"
#include "string.h"

/************************** Constant Definitions *****************************/
#define SHA2_COMPUTE_RST_REG           0x144

#define SHA2_DIGEST_REG0_OFFSET        0x148
#define SHA2_DIGEST_REG1_OFFSET        0x14c
#define SHA2_DIGEST_REG2_OFFSET        0x150
#define SHA2_DIGEST_REG3_OFFSET        0x154
#define SHA2_DIGEST_REG4_OFFSET        0x158
#define SHA2_DIGEST_REG5_OFFSET        0x15c
#define SHA2_DIGEST_REG6_OFFSET        0x160
#define SHA2_DIGEST_REG7_OFFSET        0x164
#define MSG                            "51525354555657585960"
#define PCFG_DATA_OP_AES_GCM           0x00400000

#define PCFG_DATA_OP_MASK              0x00f00000
#define PCFG_AES_MODE_MASK             0x00000018

#define PCFG_AES_CHMODE_MASK           0x00000060
#define PCFG_AES_EN                    0x00000001
#define PCFG_AES_DIS                   0xFFFFFFF0

#define MAXIMUM_SHA_PADDING_LEN        72U

#define SHA_BLOCK_SIZE                 64U
#define SHA_PADDING_BOUNDARY_SIZE      56U
#define SHA_FILL_DATA                  0x80U

#define PUF_SHA_OK                     FMSH_SUCCESS

#define SPAcc_BASEADDR                 0xFFCC0000
#define CUS_SHA3_IRQ_ADDR              0xFFCA0010
#define SPAcc_KEY_SZ_ADDR              (SPAcc_BASEADDR + 0x00100)
#define SPAcc_IRQ_EN_ADDR              (SPAcc_BASEADDR + 0x00000)
#define SPAcc_IRQ_STAT_ADDR            (SPAcc_BASEADDR + 0x00004)
#define SPAcc_IRQ_CTRL_ADDR            (SPAcc_BASEADDR + 0x00008)
#define SPAcc_FIFO_STAT_ADDR           (SPAcc_BASEADDR + 0x0000C)
#define SPAcc_SRC_PTR_ADDR             (SPAcc_BASEADDR + 0x00020)
#define SPAcc_DST_PTR_ADDR             (SPAcc_BASEADDR + 0x00024)
#define SPAcc_OFFSET_ADDR              (SPAcc_BASEADDR + 0x00028)
#define SPAcc_PRE_AAD_LEN_ADDR         (SPAcc_BASEADDR + 0x0002C)
#define SPAcc_POST_AAD_LEN_ADDR        (SPAcc_BASEADDR + 0x00030)
#define SPAcc_PROC_LEN_ADDR            (SPAcc_BASEADDR + 0x00034)
#define SPAcc_ICV_LEN_ADDR             (SPAcc_BASEADDR + 0x00038)
#define SPAcc_ICV_OFFSET_ADDR          (SPAcc_BASEADDR + 0x0003C)
#define SPAcc_IV_OFFSET_ADDR           (SPAcc_BASEADDR + 0x00040)
#define SPAcc_SW_CTRL_ADDR             (SPAcc_BASEADDR + 0x00044)
#define SPAcc_AUX_INFO_ADDR            (SPAcc_BASEADDR + 0x00048)
#define SPAcc_CTRL_ADDR                (SPAcc_BASEADDR + 0x0004C)
#define SPAcc_STAT_POP_ADDR            (SPAcc_BASEADDR + 0x00050)
#define SPAcc_STATUS_ADDR              (SPAcc_BASEADDR + 0x00054)

#define CUS_INT_STATUS_MASK_ADDR       0xFFCA0014
#define CUS_UNLOCK_ADDR                0xFFCA002C

#define SHA3_POLL_TIMEOUT_MICROSECONDS (80000)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
u32 FmshFsbl_sha256(u8* Message, u32 MessageByteLen, u8* Digest);

u32 FmshFsbl_BurstSha256(u8* Message, u32 MessageByteLen, u8* Digest);

u32 FmshFsbl_SubgroupSha256(u8* Message, u32 MessageByteLen, u8* Digest);

u32 FmshFsbl_LinearBurstSha256(u8* Message, u32 MessageByteLen, u8* Digest);

u32 FmshFsbl_sha384(u8* Message, u32 MessageByteLen, u8* Digest);

u32 FmshFsbl_PlBlockSha256(u8* Message, u32 MessageByteLen, u32 AcOffset,
                          u8* Digest);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
