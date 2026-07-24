/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_dma.h
 *
 * This file contains header fmsh_uart_common.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  12/28/2023  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_PMU_INTERACTIVE_H_
#define _FMSH_PMU_INTERACTIVE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * To indicate usage of RPU cores to PMU, PMU_GLOBAL_GLOB_GEN_STORAGE4 is used
 */
#define FSBL_R5_USAGE_STATUS_REG (PMU_GLOBAL_GLOB_GEN_STORAGE4)
/* Bit 1 of rpu uasge status register is used for R50 status */
#define FSBL_R5_0_STATUS_MASK    (1U << 1)
/* Bit 2 of rpu usage status register is used for R51 status */
#define FSBL_R5_1_STATUS_MASK    (1U << 2)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
