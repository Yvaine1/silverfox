/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fsbl_config.h
 *
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
#ifndef _FSBL_CONFIG_H_
#define _FSBL_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* This is the address in DDR where bitstream will be copied temporarily */
#define FSBL_PL_TEMP_DDRADDR            (0x100000U)

/* This is the address in DDR where boot.bin will be copied in USB boot mode */
#define FSBL_DDR_TEMP_BUFFER_ADDRESS    (0x4000000U)

#define DEBUG_INFO                      (1U)
#define DEBUG_PERF                      (1U)
#define DEBUG_DETAILED                  (0U)

#define FSBL_NAND_EXCLUDE_VAL           (0U)
#define FSBL_QSPI_EXCLUDE_VAL           (0U)
#define FSBL_SD_EXCLUDE_VAL             (0U)
#define FSBL_SECURE_EXCLUDE_VAL         (1U)
#define FSBL_BS_EXCLUDE_VAL             (0U)
#define FSBL_EARLY_HANDOFF_EXCLUDE_VAL  (1U)
#define FSBL_WDT_EXCLUDE_VAL            (1U)
#define FSBL_A53_TCM_ECC_EXCLUDE_VAL    (0U)
#define FSBL_PL_SKIP_EXCLUDE_VAL        (1U)
#define FSBL_USB_EXCLUDE_VAL            (1U)
#define FSBL_PARTITION_LOAD_EXCLUDE_VAL (0U)
#define FSBL_DDR_SR_EXCLUDE_VAL         (1U)
#define FSBL_QSPI_XIP_EXCLUDE_VAL       (1U)   
#define FSBL_MULTI_BOOT_EXCLUDE_VAL     (0U)   
#define FSBL_PROT_BYPASS_EXCLUDE_VAL	(1U)    
#define FSBL_REMISO_START_EXCLUDE_VAL	(0U)   
  
#if FSBL_NAND_EXCLUDE_VAL
#define FSBL_NAND_EXCLUDE
#endif

#if FSBL_QSPI_EXCLUDE_VAL
#define FSBL_QSPI_EXCLUDE
#endif

#if FSBL_SD_EXCLUDE_VAL
#define FSBL_SD_EXCLUDE
#endif

#if FSBL_SECURE_EXCLUDE_VAL
#define FSBL_SECURE_EXCLUDE
#endif

#if FSBL_BS_EXCLUDE_VAL
#define FSBL_BS_EXCLUDE
#endif

#if FSBL_EARLY_HANDOFF_EXCLUDE_VAL
#define FSBL_EARLY_HANDOFF_EXCLUDE
#endif

#if FSBL_WDT_EXCLUDE_VAL
#define FSBL_WDT_EXCLUDE
#endif

#if FSBL_A53_TCM_ECC_EXCLUDE_VAL
#define FSBL_A53_TCM_ECC_EXCLUDE
#endif

#if FSBL_PL_SKIP_EXCLUDE_VAL
#define FSBL_PL_SKIP_EXCLUDE
#endif

#if FSBL_USB_EXCLUDE_VAL
#define FSBL_USB_EXCLUDE
#endif

#if FSBL_PARTITION_LOAD_EXCLUDE_VAL
#define FSBL_PARTITION_LOAD_EXCLUDE
#endif

#if (FSBL_DDR_SR_EXCLUDE_VAL == 0U)
#define FSBL_ENABLE_DDR_SR
#endif

#if defined(FSBL_QSPI_EXCLUDE) || defined(FSBL_QSPI_XIP_EXCLUDE_VAL)
#define FSBL_QSPI_XIP_EXCLUDE 
#endif
#if FSBL_MULTI_BOOT_EXCLUDE_VAL
#define FSBL_MULTI_BOOT_EXCLUDE
#endif 
#if (FSBL_PROT_BYPASS_EXCLUDE_VAL == 0U)
#define FSBL_PROT_BYPASS
#endif
  
#if (FSBL_REMISO_START_EXCLUDE_VAL == 0U)
#define FSBL_REMOVE_ISO_START
#endif
  
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
