/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_timer.h
*
* This file contains ......
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

#ifndef FMSH_RTC_VERIFY_H /* prevent circular inclusions */
#define FMSH_RTC_VERIFY_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif
    
/***************************** Include Files *********************************/  
  
#include "fmsh_rtc_lib.h"
#include "fmsh_rtc_mix.h"
  
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
           
/***************** Macros (Inline Functions) Definitions *********************/  
        
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

u8 fmsh_rtc_verify();
  
u8 rtc_regResetRead_verify(FRtcPs_T *dev);
u8 rtc_por_reset_verify(FRtcPs_T *dev);
u8 rtc_hr_reset_verify(FRtcPs_T *dev);
u8 rtc_RdWrReg_verify(FRtcPs_T *dev);
u8 rtc_integer_calib_verify(FRtcPs_T *dev);
u8 rtc_frac_calib_verify(FRtcPs_T *dev);
u8 rtc_alrm_int_verify(FRtcPs_T *dev);
u8 rtc_slcr_reset_verify(FRtcPs_T *dev);

  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
  