/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_error.h
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
#ifndef _FMSH_ERROR_H_
#define _FMSH_ERROR_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/
#define FSBL_PSU_INIT_FAILED                 (0x0800U)

#define FSBL_SUCCESS_NOT_PARTITION_OWNER     (0x2U)
#define FSBL_STATUS_CONTINUE_PARTITION_LOAD  (0x3U)
#define FSBL_STATUS_CONTINUE_OTHER_HANDOFF   (0x4U)
#define FSBL_STATUS_SECONDARY_BOOT_MODE      (0x5U)

#define FSBL_ERROR_UART_INIT                 (0x10U)
#define FSBL_ERROR_GIC_INIT                  (0x11U)
#define FSBL_ERROR_QSPI_24B_INIT             (0x12U)
#define FSBL_ERROR_QSPI_32B_INIT             (0x13U)

#define FSBL_ERROR_EMMC_INIT                 (0x14U)
#define FSBL_ERROR_USB_INIT                  (0x15U)
#define FSBL_ERROR_QSPI_INIT                 (0x16U)
#define FSBL_ERROR_NAND_INIT                 (0x17U)
#define FSBL_ERROR_SD_INIT                   (0x18U)

#define FSBL_ERROR_PWR_UP_CPU                (0x19U)   
#define FSBL_ERROR_ERROR_UNAVAILABLE_CPU     (0x1AU)
#define FSBL_ERROR_UNAVAILABLE_CPU           (0x1BU)
#define FSBL_ERROR_ENC_IS_MANDATORY          (0x1CU)
#define FSBL_ERROR_PARTITION_SIGNATURE       (0x1DU)
#define FSBL_ERROR_INVALID_BOOT_MODE         (0x1EU)
#define FSBL_ERROR_INVALID_ID                (0x1FU)
#define FSBL_ERROR_IMG_HEADER_CHECKSUM       (0x20U)
#define FSBL_ERROR_PPK_HASH_MISMATCH         (0x21U)
#define FSBL_ERROR_SPK_ID_MISMATCH           (0x22U)
#define FSBL_ERROR_SPK_SIGNATURE             (0x23U)
#define FSBL_ERROR_BOOT_HEADER_SIGNATURE     (0x24U)
#define FSBL_ERROR_RSA_ENCRYPT               (0x25U)
#define FSBL_ERROR_SHA_CAL                   (0x26U)

#define FSBL_ERROR_PH_CHECKSUM               (0x27U)
#define FSBL_ERROR_XIP_AUTH_ENC_PRESENT      (0x28U)
#define FSBL_ERROR_APU_XIP_EXCUTION_ADDRESS  (0x29U)
#define FSBL_ERROR_MISMATCH_PARTITION_LENGTH (0x2AU)
#define FSBL_ERROR_PARTITION_AUTHENTICATE    (0x2BU)
#define FSBL_ERROR_INVALID_EXCUTION_ADDRESS  (0x2CU)
#define FSBL_ERROR_DECYPTION                 (0x2DU)
#define FSBL_ERROR_PARTITION_CHECKSUM        (0x2EU)
#define FSBL_ERROR_SECURE_BOOT_FORCE         (0x2FU)
#define FSBL_ERROR_PL_POWER                  (0x30U)
#define FSBL_ERROR_PL_CONFIG                 (0x31U)
#define FSBL_ERROR_DEVC_INIT                 (0x32U)
#define FSBL_ERROR_PPK_HASH_CAL_TIME_OUT     (0x33U)
#define FSBL_ERROR_HANDOFF_CPUID             (0x34U)
#define FSBL_ERROR_LOAD_ADDRESS              (0x35U)
#define FSBL_ERROR_PL_NOT_ENABLED            (0x3DU)
#define FSBL_ERROR_PL_POWER_UP               (0x3EU)
#define FSBL_ERROR_A53_0_POWER_UP            (0x3FU)
#define FSBL_ERROR_A53_1_POWER_UP            (0x40U)
#define FSBL_ERROR_A53_2_POWER_UP            (0x41U)
#define FSBL_ERROR_A53_3_POWER_UP            (0x42U)
#define FSBL_ERROR_R5_0_POWER_UP             (0x43U)
#define FSBL_ERROR_R5_1_POWER_UP             (0x44U)
#define FSBL_ERROR_R5_L_POWER_UP             (0x45U)
#define FSBL_ERROR_R5_0_TCM_POWER_UP         (0x46U)
#define FSBL_ERROR_R5_1_TCM_POWER_UP         (0x47U)
#define FSBL_ERROR_R5_L_TCM_POWER_UP         (0x48U)
#define FSBL_ERROR_DDR_ECC_INIT              (0x49U)  
#define FSBL_ERROR_TCM_ECC_INIT              (0x4AU)
#define FSBL_ERROR_UNSUPPORTED_HANDOFF       (0x4BU)
#define FSBL_ERROR_PM_INIT                   (0x50U)
#define FSBL_ERROR_PROTECTION_CFG            (0x51U)



#define PARTITION_SKIP_LOAD                  0x55

#define WriteErrorCode(p)         \
    Fmsh_Out32(REBOOT_STATUS_REG, \
               Fmsh_In32(REBOOT_STATUS_REG) & 0xFFFF0000U | p)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* FSBL_ERROR_H */
