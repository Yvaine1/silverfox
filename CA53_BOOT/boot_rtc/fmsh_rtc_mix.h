/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_rtc_mix.h
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
* 0.01   tyf  05/06/2023  First Release
*</pre>
******************************************************************************/

#ifndef _FMSH_RTC_MIX_H_ /* prevent circular inclusions */
#define _FMSH_RTC_MIX_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif
 
/***************************** Include Files *********************************/  
        
#include "fmsh_rtc_lib.h"
  
/***************************** Include Files *********************************/          

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
           
/***************** Macros (Inline Functions) Definitions *********************/  
        
/************************** Variable Definitions *****************************/
  
extern FRtcPs_T g_RTC;
extern u8 g_rtc_SecFlag, g_rtc_AlrFlag;

/************************** Function Prototypes ******************************/
u8 rtc_init();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
