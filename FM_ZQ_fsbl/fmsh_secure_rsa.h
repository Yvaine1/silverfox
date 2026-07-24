/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_secure_rsa.h
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   jzf  09/26/2023  First Release
 *
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_SECURE_RSA_H
#define _FMSH_SECURE_RSA_H

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/
#define RSA_ENGINE_REG_BASE_ADDR                  0xFFCE0000 /* CSU RSA base address */
/***********************************************
 *
 * @name CTRL Register
 *
 * CTRL Register Bit Definition
 ***********************************************/
#define FSECURE_CSU_RSA_CTRL                      0x0
#define FSECURE_CSU_RSA_CTRL_GO                   0x80000000
#define FSECURE_CSU_RSA_CTRL_STOP_RQST            0x10000000
#define FSECURE_CSU_RSA_CTRL_M521_MODE            0x001F0000
#define FSECURE_CSU_RSA_CTRL_BASE_RADIX           0x00000300
#define FSECURE_CSU_RSA_CTRL_PARTIAL_RADIX        0x000000FF

/* BASE_RADIX */
#define FSECURE_RSA_512_KEY_SIZE                  (0x3) /* RSA 512 key size */
#define FSECURE_RSA_1024_KEY_SIZE                 (0x4) /* RSA 1024 key size */
#define FSECURE_RSA_2048_KEY_SIZE                 (0x5) /* RSA 2048 key size */
#define FSECURE_RSA_4096_KEY_SIZE                 (0x6) /* RSA 4096 key size */

#define FSECURE_RSA_512_SIZE_WORDS                (16) /* RSA 512 Size in words */
#define FSECURE_RSA_1024_SIZE_WORDS               (32) /* RSA 1024 Size in words */
#define FSECURE_RSA_2048_SIZE_WORDS               (64) /* RSA 2048 Size in words */
#define FSECURE_RSA_4096_SIZE_WORDS               (128) /* RSA 4096 Size in words */

/***********************************************
 *
 * @name ENTRY_PNT Register
 *
 * ENTRY_PNT Definition
 ***********************************************/
#define FSECURE_CSU_RSA_ENTRY_PNTR                0x4
#define FSECURE_CSU_RSA_ENTRY_PNTR_MASK           0x000003FF

/* Function Pointer */
#define ENTRY_CALC_R_INV                          (0x11)
#define ENTRY_CALC_MP                             (0x10)
#define ENTRY_CALC_R_SQR                          (0x12)
#define ENTRY_MODEXP                              (0x16)

/***********************************************
 *
 * @name RTN_CODE Register
 *
 * RTN_CODE Register Bit Definition
 ***********************************************/
#define FSECURE_CSU_RSA_RTN_CODE                  0x8
#define FSECURE_CSU_RSA_RTN_CODE_STOP_REASON_MASK 0x00FF0000

/* Stop Reason */
#define STOP_REASON_NORMAL_STOP                   (0)
#define STOP_REASON_INVALID_OPCODE                (1)
#define STOP_REASON_STACK_UNDERFLOW               (2)
#define STOP_REASON_STACK_OVERFLOW                (3)
#define STOP_REASON_WATCHDOG                      (4)
#define STOP_REASON_HOST_REQUEST                  (5)
#define STOP_REASON_MEMORY_PORT_COLLISION         (8)

/***********************************************
 *
 * @name BUILD_CONFIG Register
 *
 * BUILD_CONFIG Address & Default Value
 ***********************************************/
#define FSECURE_CSU_RSA_BUILD_CONFIG              0xC

/* Defult Value */
#define FSECURE_CSU_RSA_BUILD_CONFIG_DEFAULT      0x80542000

/***********************************************
 *
 * @name STACK_PNTR Register
 *
 * STACK_PNTR Address
 ***********************************************/
#define FSECURE_CSU_RSA_STACK_PNTR                0x10
#define FSECURE_CSU_RSA_STACK_PNTR_MASK           0x000003FF

/***********************************************
 *
 * @name CONFIG Register
 *
 * CONFIG Address
 ***********************************************/
#define FSECURE_CSU_RSA_CONFIG                    0x10
#define FSECURE_CSU_RSA_CONFIG_ENDIAN_SWAP        0x04000000
#define FSECURE_CSU_RSA_CONFIG_ALT_ACCESS         0x00000001

/* Endian Format */
#define RSA_LITTLE_ENDIAN_FORMAT                  0U
#define RSA_BIG_ENDIAN_FORMAT                     1U

/***********************************************
 *
 * @name STAT Register
 *
 * STAT Register Bit Definition
 ***********************************************/
#define FSECURE_CSU_RSA_STAT                      0x20
#define FSECURE_CSU_RSA_STAT_DONE                 0x40000000

/***********************************************
 *
 * @name FLAGS Register
 *
 * FLAGS Register Bit Definition
 ***********************************************/
#define FSECURE_CSU_RSA_FLAGS                     0x24
#define FSECURE_CSU_RSA_FLAGS_F3                  0x00000080 /* User Flag3 */
#define FSECURE_CSU_RSA_FLAGS_F2                  0x00000040 /* User Flag2 */
#define FSECURE_CSU_RSA_FLAGS_F1                  0x00000020 /* User Flag1 */
#define FSECURE_CSU_RSA_FLAGS_F0                  0x00000010 /* User Flag0 */
#define FSECURE_CSU_RSA_FLAGS_C                   0x00000008 /* Carry Flag */
#define FSECURE_CSU_RSA_FLAGS_B                   0x00000004 /* Borrow Flag */
#define FSECURE_CSU_RSA_FLAGS_M                   0x00000002 /* Memory-test Flag */
#define FSECURE_CSU_RSA_FLAGS_Z                   0x00000001 /* Zero Flag */

/***********************************************
 *
 * @name WATCHDOG Register
 *
 * WATCHDOG Register Bit Definition
 ***********************************************/
#define FSECURE_CSU_RSA_WATCHDOG                  0x28
#define FSECURE_CSU_RSA_WATCHDOG_MASK             0xFFFFFFFF

/***********************************************
 *
 * @name IRQ_EN Register
 *
 * IRQ_EN Register Bit Definition
 ***********************************************/
#define FSECURE_CSU_RSA_IRQ_EN                    0x40
#define FSECURE_CSU_RSA_IRQ_EN_IE                 0x40000000

#define DISABLE_INTERRUPT_SIGNAL                  0x00000000
#define ENABLE_INTERRPUT_SIGNAL                   0x40000000

/***********************************************
 *
 * @name Data RAM
 *
 * RAM starting address
 ***********************************************/
#define RSA_REG_A0                                0x400
#define RSA_REG_C0                                0xC00
#define RSA_REG_D0                                0x1000
#define RSA_REG_D1                                0x1200
#define RSA_REG_D2                                0x1400
#define RSA_REG_D3                                0x1600

#define FSECURE_CSU_RSA_RAM_EXPO                  RSA_REG_D2
#define FSECURE_CSU_RSA_RAM_MOD                   RSA_REG_D0
#define FSECURE_CSU_RSA_RAM_RES                   RSA_REG_C0
#define FSECURE_CSU_RSA_RAM_DIGEST                RSA_REG_A0

/**********************************************/
// #define FSECURE_RSA_SIGN_ENC		0U
// #define FSECURE_RSA_SIGN_DEC		1U

#define DWORDS_BIT                                0xFFFFFFFF

#define FSECURE_ENC                               1
#define FSECURE_DEC                               0

#define FSECURE_RSA_OPERATION                     1

#define FSECURE_HASH_TYPE_SHA3                    (48U) /* SHA-3 hash size */
#define FSECURE_HASH_TYPE_SHA2                    (32U) /* SHA-2 hash size */
#define FSECURE_FSBL_SIG_SIZE                     (512U) /* FSBL signature size */

/***********************************************
 *
 * Status Code
 *
 ***********************************************/
#define FSECURE_INVALID_FLAG                      (0x80)

#define FSECURE_CSU_RSA_STATUS_DONE               (0xF1U)
#define FSECURE_CSU_RSA_STATUS_BUSY               (0xF2U)
#define FSECURE_CSU_RSA_STATUS_ERROR              (0xF4U)

/**************************** Type Definitions *******************************/
typedef struct {
    u32 BaseAddress; /* Device Base Address */
    u32 *Mod;        /* Modulus */
    u32 *ModExpo;    /* Exponent */
    u8 EncDec;       /* 0 for signature verification and 1 for generation */
    u32 SizeInWords; /* RSA key size in words */
    u8 Endianness;
} FSecureRsa_Config_T;

#define FSecure_WriteReg(BaseAddress, RegOffset, RegisterValue) \
    Fmsh_Out32((BaseAddress) + (RegOffset), (RegisterValue))

#define FSecure_ReadReg(BaseAddress, RegOffset) \
    Fmsh_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ******************************/
/* Initialization */
s32 FSecure_RsaInitialize(FSecureRsa_Config_T *InstancePtr, u32 *Mod,
                          u32 *ModExpo);

/* RSA Encryption */
s32 FSecure_RsaPublicEncrypt(FSecureRsa_Config_T *InstancePtr, u32 *Input,
                             u32 Size, u32 *Result);

/* RSA Decryption */
s32 FSecure_RsaPrivateDecrypt(FSecureRsa_Config_T *InstancePtr, u32 *Input,
                              u32 Size, u32 *Result);

u32 FSecure_RsaSignVerification(u8 *Signature, u8 *Hash, u32 HashLen);

/***********************************************
 *
 *static functions
 *
 ***********************************************/
static void FSecure_RsaWriteMem(FSecureRsa_Config_T *InstancePtr, u32 *WrData,
                                u32 RamOffset);
static void FSecure_RsaPutData(FSecureRsa_Config_T *InstancePtr);
static void FSecure_RsaGetData(FSecureRsa_Config_T *InstancePtr, u32 *RdData);
static s32 FSecure_RsaOperation(FSecureRsa_Config_T *InstancePtr, u32 *Input,
                                u32 *Result);
static s32 FSecure_RsaStart(FSecureRsa_Config_T *InstancePtr);
static u32 FEndianSwap32(u32 Data);
static s32 FSecure_RsaConfig(FSecureRsa_Config_T *InstancePtr);

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* _FMSH_XSECURE_RSA_H */
