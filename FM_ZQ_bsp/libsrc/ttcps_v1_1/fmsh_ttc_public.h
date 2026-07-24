/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_ttc_public.h
 *
 * This file contains public constant & function define of ttc.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   wfb  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_TTC_PUBLIC_H_ /* prevent circular inclusions */
#define _FMSH_TTC_PUBLIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_psu_parameters.h"
#include "fmsh_ttc_common.h"

/************************** Constant Definitions *****************************/

#define timer1 1
#define timer2 2
#define timer3 3

/* timer run mode */
enum FTimerPs_run_mode { free_running_mode = 0, user_DefinedCount_mode = 1 };

/**************************** Type Definitions *******************************/
/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u16 DeviceId; /**< Unique ID of device */
    u32 BaseAddr; /**< Base address of the device */
    u32 IntId[3]; /**<  List of interupts id of three timers */
} FTtcPs_Config;

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
FTtcPs_Config *FTtcPs_LookupConfig(u16 DeviceId);
u8 FTtcPs_dev_init(FTtcPs_T *dev, u16 DeviceId);
u8 FTtcPs_init(FTtcPs_T *dev, FTtcPs_Config *cfg);
u8 FTtcPs_reset(FTtcPs_T *dev);
u8 FTtcPs_setTimerEnble(FTtcPs_T *dev, u8 ttcNo, u8 mode);
u8 FTtcPs_setTimerMode(FTtcPs_T *dev, u8 ttcNo, enum FTimerPs_run_mode mode);
u8 FTtcPs_setTimerInterruptMask(FTtcPs_T *dev, u8 ttcNo, u8 mode);
u8 FTtcPs_setTIMERPWM(FTtcPs_T *dev, u8 ttcNo, u8 mode);
u8 FTtcPs_TimerNLoadCount(FTtcPs_T *dev, u8 ttcNo, u32 counter);
u8 FTtcPs_TimerNLoadCount2(FTtcPs_T *dev, u8 ttcNo, u32 counter);
void FTtcPs_ClearAllInterrupt(FTtcPs_T *dev);
u32 FTtcPs_ClearTimerNInterrupt(FTtcPs_T *dev, u8 ttcNo);
u32 FTtcPs_getTimerNStatus(FTtcPs_T *dev, u8 ttcNo);
u32 FTtcPs_getTimerNCurrentValue(FTtcPs_T *dev, u8 ttcNo);
u32 FTtcPs_setTimerNContinue(FTtcPs_T *dev, u8 ttcNo);
u32 FTtcPs_setTimerNPause(FTtcPs_T *dev, u8 ttcNo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
