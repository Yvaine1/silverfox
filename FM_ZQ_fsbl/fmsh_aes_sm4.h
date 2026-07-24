/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_aes_sm4.h
 *
 * This file contains header boot_main.h
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
#ifndef _FMSH_AES_SM4_H_
#define _FMSH_AES_SM4_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "boot_main.h"
#include "fmsh_common.h"
#include "fmsh_csudma.h"
#include "fmsh_header.h"
#include "fmsh_sac.h"

/************************** Constant Definitions *****************************/
/*SAC register offset definition*/
#define AES_CR_ADDR_OFFSET                (0x100U)

#define SAC_AES_IVR0_ADDR_OFFSET          (0x130U)
#define SAC_AES_IVR1_ADDR_OFFSET          (0x134U)
#define SAC_AES_IVR2_ADDR_OFFSET          (0x138U)
#define SAC_AES_IVR3_ADDR_OFFSET          (0x13CU)

#define SAC_AES_DEV_KEY0_ADDR_OFFSET      (0x44U)
#define SAC_AES_DEV_KEY1_ADDR_OFFSET      (0x48U)
#define SAC_AES_DEV_KEY2_ADDR_OFFSET      (0x4CU)
#define SAC_AES_DEV_KEY3_ADDR_OFFSET      (0x50U)
#define SAC_AES_DEV_KEY4_ADDR_OFFSET      (0x54U)
#define SAC_AES_DEV_KEY5_ADDR_OFFSET      (0x58U)
#define SAC_AES_DEV_KEY6_ADDR_OFFSET      (0x5CU)
#define SAC_AES_DEV_KEY7_ADDR_OFFSET      (0x60U)
#define SAC_AES_KEYUP0_ADDR_OFFSET        (0x110U)
#define SAC_AES_KEYUP1_ADDR_OFFSET        (0x114U)
#define SAC_AES_KEYUP2_ADDR_OFFSET        (0x118U)
#define SAC_AES_KEYUP3_ADDR_OFFSET        (0x11CU)
#define SAC_AES_KEYUP4_ADDR_OFFSET        (0x120U)
#define SAC_AES_KEYUP5_ADDR_OFFSET        (0x124U)
#define SAC_AES_KEYUP6_ADDR_OFFSET        (0x128U)
#define SAC_AES_KEYUP7_ADDR_OFFSET        (0x12CU)
#define SAC_AES_MULTH_H0_ADDR_OFFSET      (0x64U)
#define SAC_AES_MULTH_H1_ADDR_OFFSET      (0x68U)
#define SAC_AES_MULTH_H2_ADDR_OFFSET      (0x6CU)
#define SAC_AES_MULTH_H3_ADDR_OFFSET      (0x70U)
#define AES_MULTH_TAG0_ADDR_OFFSET        (0x74U)
#define AES_MULTH_TAG1_ADDR_OFFSET        (0x78U)
#define AES_MULTH_TAG2_ADDR_OFFSET        (0x7CU)
#define AES_MULTH_TAG3_ADDR_OFFSET        (0x80U)
#define SECURE_CSU_AES_KEY_SRC_OFFSET     (0x88U) /**< AES Key Source */
#define SECURE_CSU_AES_KEY_IV_LOAD_OFFSET (0x84U) /**< AES Key IV Load Reg */

#define AES_DEC_FLAG_ADDR                 (0x90U)

#define FAMILY_KEY0_ADDR                  (0x180U)

#define AES_CHMODE_MASK                   (0x00000060U)
#define ENC_DEC_MODE_MASK                 (0x00000018U)

#define ALG_MODE_MASK                     (0x00000002U)
#define AES_MODE                          (0x00000000U)
#define SM4_MODE                          (0x00000002U)
#define CTR_MODE                          (0x00000040U)
#define ECB_MODE                          (0x00000000U)
#define MULTIH_MODE                       (0x00000060U)
#define AES_EN                            (0x00000001U)
#define AES_DIS                           (0xFFFFFFFEU)
#define AES_DEC_FLAG_MASK                 0xFU
#define AES_SECURE_HEADER_OP_KEY_FLAG     0xeU
#define AES_SECURE_HEADER_NOT_OP_KEY_FLAG 0xaU
#define AES_KUP_IV_WRITE_FLAG             0x3U

#define ENCRYPTION_MODE                   (0x00000000U)
#define DECRYPTION_MODE                   (0x00000018U)

#define SECURE_CSU_AES_KEY_SRC_KUP        (0x2U) /**< KUP key source */
#define SECURE_CSU_AES_KEY_SRC_DEV        (0x1U) /**< Device Key source */
#define SECURE_CSU_AES_KEY_SRC_MULT_H     (0x3U)
#define SECURE_CSU_AES_KEY_LOAD           (1U << 1)
/**< Load AES key from Source */

#define SECURE_CSU_AES_IV_LOAD            (1U << 0)
/**< Load AES key from Source */

#define ENC_DEC_MODE_MISMATCH             2U

#define ALGORITHMIC_MISMATCH              3U

#define ALG_SM4                           0x69U
#define ALG_AES                           0xA5U
#define SECURE_IV_SIZE                    12U
#define SECURE_MULTI_H_SIZE               16U
#define SECURE_Y_SIZE                     16U
#define SECURE_GCM_TAG_SIZE               16U
#define SECURE_HDR_SIZE                   (48U)
/**< Secure Header Size in Bytes*/
#define MAXIMUM_GCM_PADDING_SIZE          16U
#define GCM_BLK_SIZE                      16U
#define SM4_KEY_SIZE                      16U
#define AES_KEY_SIZE                      32U
#define SECURE_DESTINATION_PCAP_ADDR      (0XFFFFFFFFU)
#define AES_GCM_KEY_IV_SIZE               48U
#define LEN_A                             8U
#define LEN_C                             8U
#define OP_KEY_USING                      0U

#define GCM_ENCRYPT                       0x1E
#define GCM_DECRYPT                       0xD2

#define GCM_OK                            'O'
#define GCM_ERROR                         'e'

/**************************** Type Definitions *******************************/

typedef struct {
    u8 Alg;        
    u32 SrcAddr;   
    u32 LoadAddr;  
    u32 UnencryptedLength;  
    u8 OpKeyUsing;          
} EncData;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
u32 SetSacIv(u8* Iv);

u32 GetSacIv(u8* Iv);

void IvToY1(u8* Iv, u8* Y1);

u32 FmshFsbl_Secure_AesDecrypt(EncData* EncryptionData);

#ifndef FSBL_PS_DDR
u32 PlCalcTagNoLinear(u8 Alg, const u8* In, u32 InByteLen, u32 KeySel, u8* Iv,
                      u8* Tag);
#endif

// u32 AesSm4GcmDec(u8 Alg,u32 DecFlag, const u8* In, u32 InByteLen, u8* Out,
// u32 KeySel, u8* Iv,const u8* Tag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
