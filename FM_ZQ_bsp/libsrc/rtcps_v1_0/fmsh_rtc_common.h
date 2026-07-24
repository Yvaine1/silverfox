/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_rtc_common.h
 *
 * This file contains common type define of gtc
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   tyf  04/24/2023  First Release
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_RTC_COMMON_H_ /* prevent circular inclusions */
#define _FMSH_RTC_COMMON_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_common.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * PARAMETERS
 *  baseAddress     physical base address of device
 *  instance        device private data structure pointer
 */
typedef struct FRtcPs {
    void *base_address;
    void *instance;

} FRtcPs_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
