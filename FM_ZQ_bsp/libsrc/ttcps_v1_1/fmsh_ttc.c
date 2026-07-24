/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_ttc.c
 *
 * This file contains all private & pbulic functions of ttc.
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

/***************************** Include Files *********************************/

#include "fmsh_ttc_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/******************************************************************************
 *
 *
 * This function initializes timer.
 *
 * @param    dev is ttc device handle.
 * @param    id is ttc device id 0 or 1.
 *
 * @return   TRUE if successful, otherwise do not support.
 *
 * @note     None.
 *
 ******************************************************************************/

u8 FTtcPs_init (FTtcPs_T *dev, FTtcPs_Config *cfg)
{
    dev->base_address = (void *)(cfg->BaseAddr);
    dev->id = cfg->DeviceId;
    dev->IntId[0] = cfg->IntId[0];
    dev->IntId[1] = cfg->IntId[1];
    dev->IntId[2] = cfg->IntId[2];
    return 0;
}

u8 FTtcPs_dev_init (FTtcPs_T *dev, u16 DeviceId)
{
    FTtcPs_Config *cfg = NULL;
    cfg = FTtcPs_LookupConfig(DeviceId);
    FTtcPs_init(dev, cfg);

    return 0;
}

u8 FTtcPs_reset (FTtcPs_T *dev)
{
    u32 reg;
    u32 reset_bit;

    switch (dev->id)
    {
    case 0:
        reset_bit = 11;
        break;
    case 1:
        reset_bit = 12;
        break;
    case 2:
        reset_bit = 13;
        break;
    case 3:
        reset_bit = 14;
        break;
    default:
        return 1;
    }
    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg | (1 << reset_bit));

    delay_us(10);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg & ~(1 << reset_bit));
    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to enable timer.
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 * @param    mode is enable if 1, disable if 0.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FTtcPs_setTimerEnble (FTtcPs_T *dev, u8 ttcNo, u8 mode)
{
    u32 reg = 0;
    FTtcPs_Portmap_T *portmap;

    FTTCPS_COMMON_ASSERT(dev);

    portmap = (FTtcPs_Portmap_T *)dev->base_address;
    if (ttcNo == timer1)
    {
        reg = TIMER_IN32P(portmap->Timer1ControlReg);
    }
    else if (ttcNo == timer2)
    {
        reg = TIMER_IN32P(portmap->Timer2ControlReg);
    }
    else if (ttcNo == timer3)
    {
        reg = TIMER_IN32P(portmap->Timer3ControlReg);
    }
    else
    {
        return 1;
    }
    if (FMSH_BIT_GET(reg, TIMER_TimerNControlReg_TimerEnable) != mode)
    {
        FMSH_BIT_SET(reg, TIMER_TimerNControlReg_TimerEnable, mode);
        if (ttcNo == timer1)
        {
            TIMER_OUT32P(reg, portmap->Timer1ControlReg);
        }
        else if (ttcNo == timer2)
        {
            TIMER_OUT32P(reg, portmap->Timer2ControlReg);
        }
        else if (ttcNo == timer3)
        {
            TIMER_OUT32P(reg, portmap->Timer3ControlReg);
        }
        else{
            ;/* no deal with */
        }
    }

    return 0;
}

/******************************************************************************
 *
 *
 * Set timer operation mode.
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 * @param    mode  --  free_running_mode/user_DefinedCount_mode.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FTtcPs_setTimerMode (FTtcPs_T *dev, u8 ttcNo, enum FTimerPs_run_mode mode)
{
    u32 reg = 0;
    FTtcPs_Portmap_T *portmap;
    portmap = (FTtcPs_Portmap_T *)dev->base_address;

    FTTCPS_COMMON_ASSERT(dev);

    if (ttcNo == timer1)
    {
        reg = TIMER_IN32P(portmap->Timer1ControlReg);
    }
    else if (ttcNo == timer2)
    {
        reg = TIMER_IN32P(portmap->Timer2ControlReg);
    }
    else if (ttcNo == timer3)
    {
        reg = TIMER_IN32P(portmap->Timer3ControlReg);
    }
    else
    {
        return 1;
    }

    if (FMSH_BIT_GET(reg, TIMER_TimerNControlReg_TimerMode) != mode)
    {
        FMSH_BIT_SET(reg, TIMER_TimerNControlReg_TimerMode, mode);
        if (ttcNo == timer1)
        {
            TIMER_OUT32P(reg, portmap->Timer1ControlReg);
        }
        else if (ttcNo == timer2)
        {
            TIMER_OUT32P(reg, portmap->Timer2ControlReg);
        }
        else if (ttcNo == timer3)
        {
            TIMER_OUT32P(reg, portmap->Timer3ControlReg);
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

/******************************************************************************
 *
 *
 * Set timer interrupt mask mode.
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 * @param    mode  --  0, no mask.
 *           mode  --  1, mask.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FTtcPs_setTimerInterruptMask (FTtcPs_T *dev, u8 ttcNo, u8 mode)
{
    u32 reg = 0;
    FTtcPs_Portmap_T *portmap;

    FTTCPS_COMMON_ASSERT(dev);

    portmap = (FTtcPs_Portmap_T *)dev->base_address;

    if (ttcNo == timer1)
    {
        reg = TIMER_IN32P(portmap->Timer1ControlReg);
    }
    else if (ttcNo == timer2)
    {
        reg = TIMER_IN32P(portmap->Timer2ControlReg);
    }
    else if (ttcNo == timer3)
    {
        reg = TIMER_IN32P(portmap->Timer3ControlReg);
    }
    else
    {
        return 1;
    }

    if (FMSH_BIT_GET(reg, TIMER_TimerNControlReg_TimerInterruptMask) != mode)
    {
        FMSH_BIT_SET(reg, TIMER_TimerNControlReg_TimerInterruptMask, mode);
        if (ttcNo == timer1)
        {
            TIMER_OUT32P(reg, portmap->Timer1ControlReg);
        }
        else if (ttcNo == timer2)
        {
            TIMER_OUT32P(reg, portmap->Timer2ControlReg);
        }
        else if (ttcNo == timer3)
        {
            TIMER_OUT32P(reg, portmap->Timer3ControlReg);
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

/******************************************************************************
 *
 *
 * Set timer PWM mode.
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 * @param    mode  --  0, off PWM.
 *           mode  --  1, on PWM.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FTtcPs_setTIMERPWM (FTtcPs_T *dev, u8 ttcNo, u8 mode)
{
    u32 reg = 0;
    FTtcPs_Portmap_T *portmap;

    FTTCPS_COMMON_ASSERT(dev);

    portmap = (FTtcPs_Portmap_T *)dev->base_address;
    if (ttcNo == timer1)
    {
        reg = TIMER_IN32P(portmap->Timer1ControlReg);
    }
    else if (ttcNo == timer2)
    {
        reg = TIMER_IN32P(portmap->Timer2ControlReg);
    }
    else if (ttcNo == timer3)
    {
        reg = TIMER_IN32P(portmap->Timer3ControlReg);
    }
    else
    {
        return 1;
    }

    if (FMSH_BIT_GET(reg, TIMER_TimerNControlReg_TIMERPWM) != mode)
    {
        FMSH_BIT_SET(reg, TIMER_TimerNControlReg_TIMERPWM, mode);
        if (ttcNo == timer1)
        {
            TIMER_OUT32P(reg, portmap->Timer1ControlReg);
        }
        else if (ttcNo == timer2)
        {
            TIMER_OUT32P(reg, portmap->Timer2ControlReg);
        }
        else if (ttcNo == timer3)
        {
            TIMER_OUT32P(reg, portmap->Timer3ControlReg);
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

/******************************************************************************
 *
 *
 * Set timer counter.
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 * @param    counter  --  32 bits timer reload value.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FTtcPs_TimerNLoadCount (FTtcPs_T *dev, u8 ttcNo, u32 counter)
{
    FTtcPs_Portmap_T *portmap;
    portmap = (FTtcPs_Portmap_T *)dev->base_address;

    FTTCPS_COMMON_ASSERT(dev);

    if (ttcNo == timer1)
    {
        TIMER_OUT32P(counter, portmap->Timer1LoadCount);
    }
    else if (ttcNo == timer2)
    {
        TIMER_OUT32P(counter, portmap->Timer2LoadCount);
    }
    else if (ttcNo == timer3)
    {
        TIMER_OUT32P(counter, portmap->Timer3LoadCount);
    }
    else
    {
        return 1;
    }

    return 0;
}

/******************************************************************************
 *
 *
 * Set timer LoadCount2 value(HIGH LEVEL), only active as PWM on
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 * @param    counter  --  32 bits timer reload value.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FTtcPs_TimerNLoadCount2 (FTtcPs_T *dev, u8 ttcNo, u32 counter)
{
    FTtcPs_Portmap_T *portmap;
    portmap = (FTtcPs_Portmap_T *)dev->base_address;

    FTTCPS_COMMON_ASSERT(dev);

    if (ttcNo == timer1)
    {
        TIMER_OUT32P(counter, portmap->Timer1LoadCount2);
    }
    else if (ttcNo == timer2)
    {
        TIMER_OUT32P(counter, portmap->Timer2LoadCount2);
    }
    else if (ttcNo == timer3)
    {
        TIMER_OUT32P(counter, portmap->Timer3LoadCount2);
    }
    else
    {
        return 1;
    }

    return 0;
}

/******************************************************************************
 *
 *
 * Clear all timer interrupt.
 *
 * @param    dev is ttc device handle.
 *
 * @return   return value is meaningless.
 *
 * @note     None.
 *
 ******************************************************************************/
void FTtcPs_ClearAllInterrupt (FTtcPs_T *dev)
{
    FTtcPs_Portmap_T *portmap;

    FTTCPS_COMMON_ASSERT(dev);

    portmap = (FTtcPs_Portmap_T *)dev->base_address;
    TIMER_IN32P(portmap->TIMERSEOI);
}

/******************************************************************************
 *
 *
 * Clear timerX interrupt.
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 *
 * @return   return value is meaningless.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FTtcPs_ClearTimerNInterrupt (FTtcPs_T *dev, u8 ttcNo)
{
    u32 reg;
    FTtcPs_Portmap_T *portmap;
    portmap = (FTtcPs_Portmap_T *)dev->base_address;

    FTTCPS_COMMON_ASSERT(dev);

    if (ttcNo == timer1)
    {
        reg = TIMER_IN32P(portmap->Timer1EOI);
    }
    else if (ttcNo == timer2)
    {
        reg = TIMER_IN32P(portmap->Timer2EOI);
    }
    else if (ttcNo == timer3)
    {
        reg = TIMER_IN32P(portmap->Timer3EOI);
    }
    else
    {
        return 1;
    }

    return reg;
}

/******************************************************************************
 *
 *
 * get timer status.
 *
 * @param    dev is ttc device handle.
 * @param    ttcNo is timer number 1-3.
 *
 * @return   return status of ttc.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FTtcPs_getTimerNStatus (FTtcPs_T *dev, u8 ttcNo)
{
    u32 reg;
    FTtcPs_Portmap_T *portmap;

    FTTCPS_COMMON_ASSERT(dev);

    portmap = (FTtcPs_Portmap_T *)dev->base_address;

    if (ttcNo == timer1)
    {
        reg = TIMER_IN32P(portmap->Timer1IntStatus);
    }
    else if (ttcNo == timer2)
    {
        reg = TIMER_IN32P(portmap->Timer2IntStatus);
    }
    else if (ttcNo == timer3)
    {
        reg = TIMER_IN32P(portmap->Timer3IntStatus);
    }
    else
    {
        return -1;
    }

    return reg;
}

u32 FTtcPs_getTimerNCurrentValue (FTtcPs_T *dev, u8 ttcNo)
{
    u32 reg;
    FTtcPs_Portmap_T *portmap;

    FTTCPS_COMMON_ASSERT(dev);

    portmap = (FTtcPs_Portmap_T *)dev->base_address;

    if (ttcNo == timer1)
    {
        reg = TIMER_IN32P(portmap->Timer1CurrentValue);
    }
    else if (ttcNo == timer2)
    {
        reg = TIMER_IN32P(portmap->Timer2CurrentValue);
    }
    else if (ttcNo == timer3)
    {
        reg = TIMER_IN32P(portmap->Timer3CurrentValue);
    }
    else
    {
        return 1;
    }

    return reg;
}

u32 FTtcPs_setTimerNPause (FTtcPs_T *dev, u8 ttcNo)
{
    u32 reg;
    u32 deviceId = dev->id;
    if( (ttcNo != timer1) || (ttcNo != timer2) || (ttcNo != timer3) )
    {
        return -1;
    }
    reg = FMSH_ReadReg(0xFF180000, 0x9274);

    reg |= 1 << (deviceId * 3 + ttcNo - 1);
    FMSH_WriteReg(0xFF180000, 0x9274, reg);
    return 0;
}

u32 FTtcPs_setTimerNContinue (FTtcPs_T *dev, u8 ttcNo)
{
    u32 reg;
    u32 deviceId = dev->id;
    reg = FMSH_ReadReg(0xFF180000, 0x9274);
    if( (ttcNo != timer1) || (ttcNo != timer2) || (ttcNo != timer3) )
    {
        return -1;
    }
    reg &= ~(1 << (deviceId * 3 + ttcNo - 1));
    FMSH_WriteReg(0xFF180000, 0x9274, reg);
    return 0;
}
