/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_board.h
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
 * 0.01   lq  12/31/2023  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_BOARD_H_
#define _FMSH_BOARD_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
u32 FmshFsbl_BoardInit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
