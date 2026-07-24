/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_rtc_mix.c
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

/***************************** Include Files *********************************/
//#include "verification_config.h"

#include "fmsh_rtc_mix.h"
#include "fmsh_rtc_lib.h"
#include "fmsh_gic.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XRTC_CTL_OSC_SHIFT   25U
#define XRTCPSU_CRYSTAL_OSC_EN		((u32)1 << XRTC_CTL_OSC_SHIFT)
#define XRTC_CTL_BATTERY_EN_MASK    0x80000000U
#define XRTC_CTL_OUT_EN_MASK    0x2U
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
FRtcPs_T g_RTC;
u8 g_rtc_SecFlag, g_rtc_AlrFlag;

/****************************************************************************/
/**
*
* This function is used to handle g_RTC alarm interrupt
*
* @param    None.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
void rtcAlarmHanlder (void *InstancePtr)
{
    g_rtc_AlrFlag++;
    /*clean the interrupt bit*/
    FRtcPs_clearAInterruptStatus(&g_RTC);
}

/****************************************************************************/
/**
*
* This function is used to handle g_RTC alarm interrupt
*
* @param    None.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
void rtcSecondsHanlder (void *InstancePtr)
{
    g_rtc_SecFlag++;
    /*clean the interrupt bit*/
    FRtcPs_clearSInterruptStatus(&g_RTC);
   //FRtcPs_seconds_irq_enable(&g_RTC, 0);
}

/****************************************************************************/
/**
*
* This function is used to initial g_RTC, register interrupt.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_init()
{   
    u32 reg;
    u32 value;
    // rtc_time ctm;
    FRtcPs_portmap_T *portmap;
    FRtcPs_Config *cfg = NULL;
    cfg = FRtcPs_LookupConfig(0);
    
    FRtcPs_init(&g_RTC, cfg);
    cfg = NULL;
    
    FGicPs_Connect(&IntcInstance, RTC_Alarm_INT_ID, (FMSH_InterruptHandler)rtcAlarmHanlder, &g_RTC);
    FGicPs_Enable(&IntcInstance, RTC_Alarm_INT_ID);
    FGicPs_Connect(&IntcInstance, RTC_Seconds_INT_ID, (FMSH_InterruptHandler)rtcSecondsHanlder, &g_RTC);
    FGicPs_Enable(&IntcInstance, RTC_Seconds_INT_ID);
    
    
    portmap = (FRtcPs_portmap_T *)g_RTC.base_address; 
    reg = RTC_IN32P(portmap->CONTROL);
    /*	Set the Oscillator crystal and Battery switch enable
     *	in control register.
     */
    /*
    portmap = (FRtcPs_portmap_T *)g_RTC.base_address; 
    reg = RTC_IN32P(portmap->CONTROL);
    RTC_OUT32P(reg|(u32)XRTCPSU_CRYSTAL_OSC_EN|(u32)XRTC_CTL_BATTERY_EN_MASK,portmap->CONTROL);
    */
    

    RTC_OUT32P(reg|(u32)XRTCPSU_CRYSTAL_OSC_EN|(u32)XRTC_CTL_BATTERY_EN_MASK|(u32)XRTC_CTL_OUT_EN_MASK, portmap->CONTROL);
    return 0;
}