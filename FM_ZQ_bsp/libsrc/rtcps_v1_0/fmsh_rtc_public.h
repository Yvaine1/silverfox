/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gtc_pbulic.h
 *
 * This file contains public constant & function define of gtc
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

#ifndef _FMSH_RTC_PUBLIC_H_ /* prevent circular inclusions */
#define _FMSH_RTC_PUBLIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_psu_parameters.h"
#include "fmsh_rtc_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
    u16 DeviceId; /**< Unique ID of device */
    u32 BaseAddr; /**< Base address of the device */
} FRtcPs_Config;

typedef struct rtc_time_T {
    u32 tm_sec;
    u32 tm_min;
    u32 tm_hour;
    u32 tm_mday;
    u32 tm_mon;
    u32 tm_year;
    u32 tm_wday;
    u32 tm_yday;
    u32 tm_isdst;
} rtc_time;

typedef struct rtc_wkalrm_T {
    u8 enabled;    /* 0 = alarm disabled, 1 = alarm enabled */
    u8 pending;    /* 0 = alarm not pending, 1 = alarm pending */
    rtc_time time; /* time the alarm is set to */
} rtc_wkalrm;

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
u8 FRtcPs_reset(void);
FRtcPs_Config *FRtcPs_LookupConfig(u16 DeviceId);
u8 FRtcPs_init(FRtcPs_T *dev, FRtcPs_Config *cfg);
u8 FRtcPs_enableCounter(FRtcPs_T *dev);
u8 FRtcPs_disableCounter(FRtcPs_T *dev);
u8 FRtcPs_set_time(FRtcPs_T *dev, rtc_time *tm);
u8 FRtcPs_read_time(FRtcPs_T *dev, rtc_time *tm);
u8 FRtcPs_read_alarm(FRtcPs_T *dev, rtc_wkalrm *alrm);
u8 FRtcPs_alarm_irq_enable(FRtcPs_T *dev, u32 enabled);
u8 FRtcPs_set_alarm(FRtcPs_T *dev, rtc_wkalrm *alrm);
u8 FRtcPs_seconds_irq_enable(FRtcPs_T *dev, u32 enabled);
u32 FRtcPs_clearSInterruptStatus(FRtcPs_T *dev);
u32 FRtcPs_clearAInterruptStatus(FRtcPs_T *dev);
u8 lpd_rtc_enter_apbRefRst(void);
u8 lpd_rtc_exit_apbRefRst(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
