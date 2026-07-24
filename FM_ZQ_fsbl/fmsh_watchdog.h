/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_watchdog.h
 *
 * This file contains "fmsh_wdt_lib.h".
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

#ifndef _FMSH_WATCHDOG_H_ /* prevent circular inclusions */
#define _FMSH_WATCHDOG_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
// Register offset
#define SLCR_WDT_RST_OFFSET (0x330)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void FmshFsbl_WdtInit(void);

void FmshFsbl_WdtClose(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
