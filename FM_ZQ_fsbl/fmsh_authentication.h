/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_authentication.h
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
 * 0.01   lq  08/28/2022  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_AUTHENTICATION_H_
#define _FMSH_AUTHENTICATION_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "boot_main.h"
#include "fmsh_common.h"
#include "fmsh_efuse.h"
#include "fmsh_error.h"
#include "fmsh_header.h"
#include "fmsh_sac.h"
#include "fmsh_sha.h"

/************************** Constant Definitions *****************************/

/***************************** Type defines *********************************/
#define RSA_CERTIFICATE_SIZE          0xEC0 /* Signature size in bytes */
#define RSA_HEADER_SIZE               4U    /* Signature header size in bytes */
#define RSA_SPKID_SIZE                4U
#define RSA_MAGIC_WORD_SIZE           60    /* Magic word size in bytes */
#define RSA_PPK_MODULAR_SIZE          512
#define RSA_PPK_MODULAR_EXT_SIZE      512
#define RSA_PPK_EXPO_SIZE             64
#define RSA_SPK_MODULAR_SIZE          512
#define RSA_SPK_MODULAR_EXT_SIZE      512
#define RSA_SPK_EXPO_SIZE             64
#define RSA_SPK_SIGNATURE_SIZE        512
#define RSA_SIGNATURE_SIZE            512
#define RSA_BOOTHEADER_SIGNATURE_SIZE 512
#define RSA_PARTITION_SIGNATURE_SIZE  512
#define HASH_TYPE_SHA2                32U
#define HASH_TYPE_SHA3                48U
#define SPKID_AC_ALIGN                4U
#define BOOT_HEADER_SIZE              0xEC0
#define BOOT_HEADER_WITHOUT_PUF_SIZE  0x8c0

/***************** Macros (Inline Functions) Definitions *********************/
// #define RSA_LIB_USED
#define RSA_4096_LIB

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
u32 FmshFsbl_AuthenticatePartition(u32 PartitionOffset, u32 PartitionLen,
                                   u32 AcOffset, u8 PlFlag);

u32 FmshFsbl_AuthenticateHeader(u8* BootHeaderPtr, u32 PartitionLen,
                                u32 AcOffset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
