/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_sd.h
 *
 * This file contains header fmsh_uart_lib.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   xx  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_PRINT_H_
#define _FMSH_PRINT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
int init_uart();
void fmsh_print(const char *ptr, ...);
unsigned char uart_getc(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
