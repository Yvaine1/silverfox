/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gtc.c
 *
 * This file contains all private & pbulic functions of gtc.
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

/***************************** Include Files *********************************/

#include "fmsh_rtc_lib.h"

/************************** Constant Definitions *****************************/
static u8 rtc_days_in_month[] = {31, 28, 31, 30, 31, 30,
                                 31, 31, 30, 31, 30, 31};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/******************************************************************************
 *
 *
 * This function is used to init RTC
 *
 * @param    dev  is a pointer to the instance of RTC.
 * @param    addr is baseaddr of RTC.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_init (FRtcPs_T *dev, FRtcPs_Config *cfg)
{
    dev->base_address = (void *)(cfg->BaseAddr);

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to enable RTC
 *
 * @param    dev  is a pointer to the instance of RTC.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_enableCounter (FRtcPs_T *dev)
{
    FRtcPs_portmap_T *portmap;
    u32 val;
    u32 stat=1;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    val = RTC_IN32P(portmap->CONTROL);
    // enable the Crystal (default value is 2)
    FMSH_BIT_SET(val, CONTROL_Osc_Cntrl, 2);
    // RTC_OUT32P(val,portmap->CONTROL);
    // enable the battery
    FMSH_BIT_SET(val, CONTROL_Battery_Enable, stat);
    RTC_OUT32P(val, portmap->CONTROL);

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to disenable RTC
 *
 * @param    dev  is a pointer to the instance of RTC.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_disableCounter (FRtcPs_T *dev)
{
    FRtcPs_portmap_T *portmap;
    u32 val;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    val = RTC_IN32P(portmap->CONTROL);
    // disable the Crystal
    FMSH_BIT_CLEAR(val, CONTROL_Osc_Cntrl);
    RTC_OUT32P(val, portmap->CONTROL);
    // disable the battery
    FMSH_BIT_CLEAR(val, CONTROL_Battery_Enable);
    RTC_OUT32P(val, portmap->CONTROL);

    return 0;
}

/******************************************************************************
 *
 * mktime64 - Converts date to seconds.
 * Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * A leap second can be indicated by calling this function with sec as
 * 60 (allowable under ISO 8601).  The leap second is treated the same
 * as the following second since they don't exist in UNIX time.
 *
 * An encoding of midnight at the end of the day as 24:00:00 - ie. midnight
 * tomorrow - (allowable under ISO 8601) is supported.
 *
 * @param    as its name describes.
 *
 * @return   seconds.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 mktime64 (const u32 year0, const u32 mon0, const u32 day, const u32 hour,
              const u32 min, const u32 sec)
{
    u32 mon = mon0, year = year0;
    /* 1..12 -> 11,12,1..10 */
    if (0 >= (int)(mon -= 2))
    {
        mon += 12; /* Puts Feb last since it has leap day */
        year -= 1;
    }
    return ((((u32)(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) +
              year * 365 - 719499) *
                 24 +
             hour /* now have hours - midnight tomorrow handled here */
             ) * 60 +
            min   /* now have minutes */
            ) * 60 +
           sec;   /* finally seconds */
}

/******************************************************************************
 *
 *
 * This function is used to converts rtc_time to time64_t.
 * Convert Gregorian date to seconds since 01-01-1970 00:00:00.
 *
 * @param    tm is a Gregorian date.
 *
 * @return   seconds.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 rtc_tm_to_time64 (rtc_time *tm)
{
    // tm_year is the year -1900, as a example if the year is 2023, then tm_year
    // need to be 123.
    return mktime64(((u32)tm->tm_year + 1900), tm->tm_mon + 1, tm->tm_mday,
                    tm->tm_hour, tm->tm_min, tm->tm_sec);
}

/******************************************************************************
 *
 *
 * This function is used to set RTC current time
 *
 * @param    dev is a pointer to the instance of RTC.
 *           tm  is the set time value.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_set_time (FRtcPs_T *dev, rtc_time *tm)
{
    FRtcPs_portmap_T *portmap;
    u32 val;
    u32 new_time;
    portmap = (FRtcPs_portmap_T *)dev->base_address;
    /*
     * The value written will be updated after 1 sec into the
     * seconds read register, so we need to program time +1 sec
     * to get the correct time on read.
     */
    new_time = rtc_tm_to_time64(tm) + 1;
    /*
     * Writing into calibration register will clear the Tick Counter and
     * force the next second to be signaled exactly in 1 second period
     */
    // val = RTC_IN32P(portmap->CALIB_READ);
    val = 0x8000U;  // 0x8000U is the default calibration value of ZU+
    // RTC_OUT32P(val, portmap->CALIB_WRITE);
    RTC_OUT32P(new_time, portmap->SET_TIME_WRITE);

    /*
     * Clear the rtc interrupt status register after setting the
     * time. During a read_time function, the code should read the
     * RTC_INT_STATUS register and if bit 0 is still 0, it means
     * that one second has not elapsed yet since RTC was set and
     * the current time should be read from SET_TIME_READ register;
     * otherwise, CURRENT_TIME register is read to report the time
     */
    val = RTC_IN32P(portmap->RTC_INT_STATUS);
    FMSH_BIT_SET(val, RTC_INT_Seconds, 1);
    RTC_OUT32P(val, portmap->RTC_INT_STATUS);

    return 0;
}
/******************************************************************************
 *
 *
 * div_s32_rem - signed 32bit divide with 32bit divisor with remainder.
 *
 *
 * @param    dividend: signed 32bit dividend
 *           divisor: signed 32bit divisor
 *           remainder: pointer to signed 32bit remainder
 *
 * @return   sets ``*remainder``, then returns dividend / divisor.
 *
 * @note     None.
 *
 ******************************************************************************/
s32 div_s32_rem (u32 dividend, u32 divisor, u32 *remainder)
{
    *remainder = dividend % divisor;
    return dividend / divisor;
}

/******************************************************************************
 *
 *
 * This function is used to determine whether it is a leap year.
 *
 *
 * @param    as its name describes.
 *
 * @return   0 if not leap year, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
s32 is_leap_year (u32 year)
{
    return (!(year % 4) && (year % 100)) || !(year % 400);
}

/******************************************************************************
 *
 *
 * This function is used to caculate the number of days in the month.
 *
 *
 * @param    as its name describes.
 *
 * @return   days in month.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 rtc_month_days (u32 month, u32 year)
{
    return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/******************************************************************************
 *
 *
 * This function is used to converts time64_t to rtc_time.
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 *
 * @param    time is in seconds form
 *           tm is a Gregorian date.
 *
 * @return   seconds.
 *
 * @note     None.
 *
 ******************************************************************************/
void rtc_time64_to_tm (u32 time, rtc_time *tm)
{
    u32 month, year, secs;
    s32 days, newdays;

    /* time must be positive */
    days = div_s32_rem(time, 86400, &secs);

    /* day of the week, 1970-01-01 was a Thursday */
    tm->tm_wday = (days + 4) % 7;

    year = 1970 + days / 365;
    days -= (year - 1970) * 365 + LEAPS_THRU_END_OF(year - 1) -
            LEAPS_THRU_END_OF(1970 - 1);
    while (days < 0)
    {
        year -= 1;
        days += 365 + is_leap_year(year);
    }
    tm->tm_year = year - 1900;
    tm->tm_yday = days + 1;

    for (month = 0; month < 11; month++)
    {
        newdays = days - rtc_month_days(month, year);
        if (newdays < 0)
        {
            break;
        }
        days = newdays;
    }
    tm->tm_mon = month;
    tm->tm_mday = days + 1;

    tm->tm_hour = secs / 3600;
    secs -= tm->tm_hour * 3600;
    tm->tm_min = secs / 60;
    tm->tm_sec = secs - tm->tm_min * 60;

    tm->tm_isdst = 0;
}

/******************************************************************************
 *
 *
 * This function is used to read RTC current time
 *
 * @param    dev is a pointer to the instance of RTC.
 *           tm  is the read time value.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_read_time (FRtcPs_T *dev, rtc_time *tm)
{
    FRtcPs_portmap_T *portmap;
    u32 status;
    u32 read_time;
    portmap = (FRtcPs_portmap_T *)dev->base_address;
    status = RTC_IN32P(portmap->RTC_INT_STATUS);
    status = FMSH_BIT_GET(status, RTC_INT_Seconds);
#if 0
    if(status)
    {
        /*
         * RTC has updated the CURRENT_TIME with the time written into
	 * SET_TIME_WRITE register.
	 */
        read_time = RTC_IN32P(portmap->CURRENT_TIME);
        rtc_time64_to_tm(read_time, tm);
    }
    else
    {
       /*
	* Time written in SET_TIME_WRITE has not yet updated into
	* the seconds read register, so read the time from the
	* SET_TIME_WRITE instead of CURRENT_TIME register.
	* Since we add +1 sec while writing, we need to -1 sec while
	* reading.
        */
        read_time = RTC_IN32P(portmap->SET_TIME_READ) - 1;
	rtc_time64_to_tm(read_time, tm);
    }
#endif
    read_time = RTC_IN32P(portmap->CURRENT_TIME);
    rtc_time64_to_tm(read_time, tm);
    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to read RTC alarm value
 *
 * @param    dev is a pointer to the instance of RTC.
 *           alrm  is the read alarm value.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_read_alarm (FRtcPs_T *dev, rtc_wkalrm *alrm)
{
    FRtcPs_portmap_T *portmap;
    u32 alrm_time, enabled;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    alrm_time = RTC_IN32P(portmap->ALARM);
    rtc_time64_to_tm(alrm_time, &alrm->time);
    enabled = RTC_IN32P(portmap->RTC_INT_MASK);
    enabled = FMSH_BIT_GET(enabled, RTC_INT_Alarm);
    alrm->enabled = enabled;

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to enable/disable RTC alarm irq
 *
 * @param    dev is a pointer to the instance of RTC.
 *           RTC alarm irq is enabled when enabled is 1, otherwise it is
 *disabled .
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_alarm_irq_enable (FRtcPs_T *dev, u32 enabled)
{
    FRtcPs_portmap_T *portmap;
    u32 reg;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    reg = RTC_IN32P(portmap->RTC_INT_EN);
    if (enabled)
    {
        FMSH_BIT_SET(reg, RTC_INT_Alarm, 1);
        RTC_OUT32P(reg, portmap->RTC_INT_EN);
    }
    else
    {
        FMSH_BIT_SET(reg, RTC_INT_Alarm, 1);
        RTC_OUT32P(reg, portmap->RTC_INT_DIS);
    }

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to set RTC alarm  value
 *
 * @param    dev is a pointer to the instance of RTC.
 *           alrm  is the set alarm value.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_set_alarm (FRtcPs_T *dev, rtc_wkalrm *alrm)
{
    FRtcPs_portmap_T *portmap;
    u32 alarm_time;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    alarm_time = rtc_tm_to_time64(&alrm->time);
    RTC_OUT32P(alarm_time, portmap->ALARM);

    FRtcPs_alarm_irq_enable(dev, alrm->enabled);

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to enable/disable RTC seconds irq
 *
 * @param    dev is a pointer to the instance of RTC.
 *           RTC seconds irq is enabled when enabled is 1, otherwise it is
 *disabled .
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_seconds_irq_enable (FRtcPs_T *dev, u32 enabled)
{
    FRtcPs_portmap_T *portmap;
    u32 reg;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    reg = RTC_IN32P(portmap->RTC_INT_EN);
    if (enabled)
    {
        FMSH_BIT_SET(reg, RTC_INT_Seconds, 1);
        RTC_OUT32P(reg, portmap->RTC_INT_EN);
    }
    else
    {
        FMSH_BIT_SET(reg, RTC_INT_Seconds, 1);
        RTC_OUT32P(reg, portmap->RTC_INT_DIS);
    }

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function clear the second interrupt
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FRtcPs_clearSInterruptStatus (FRtcPs_T *dev)
{
    FRtcPs_portmap_T *portmap;
    u32 reg = 0;
    u32 seconds_int = 0;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    reg = RTC_IN32P(portmap->RTC_INT_STATUS);
    seconds_int = FMSH_BIT_GET(reg, RTC_INT_Seconds);

    if (seconds_int)
    {
        FMSH_BIT_SET(reg, RTC_INT_Seconds, 1);
    }

    RTC_OUT32P(reg, portmap->RTC_INT_STATUS);
    return reg;
}

/*****************************************************************************
 *
 * @description
 * This function clear the alarm interrupt
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FRtcPs_clearAInterruptStatus (FRtcPs_T *dev)
{
    FRtcPs_portmap_T *portmap;
    u32 reg = 0;
    u32 alarm_int = 0;

    portmap = (FRtcPs_portmap_T *)dev->base_address;
    reg = RTC_IN32P(portmap->RTC_INT_STATUS);
    alarm_int = FMSH_BIT_GET(reg, RTC_INT_Alarm);

    if (alarm_int)
    {
        FMSH_BIT_SET(reg, RTC_INT_Alarm, 1);
    }

    RTC_OUT32P(reg, portmap->RTC_INT_STATUS);
    return reg;
}

/*****************************************************************************
 *
 * @description
 * This function set CRL_APB.RST_LPD_TOP.RTC_RESET to reset rtc
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 lpd_rtc_enter_apbRefRst (void)
{
    u32 reg;

    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x23c);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x23c, reg | 0x10000);

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function clear CRL_APB.RST_LPD_TOP.RTC_RESET to run rtc
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 lpd_rtc_exit_apbRefRst (void)
{
    u32 reg;

    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x23c);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x23c, reg & ~0x10000);

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function reset rtc
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FRtcPs_reset (void)
{
    lpd_rtc_enter_apbRefRst();
    lpd_rtc_exit_apbRefRst();
    return 0;
}
