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

#ifndef _FMSH_DMA_H_
#define _FMSH_DMA_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_axidmapsu_lib.h"
#include "fmsh_cache.h"
#include "fmsh_uart_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
int FmshFsbl_InitMem (u64 SrcAddr, u64 DestAddr, u32 LengthBytes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
