/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_ttc_example.c
*
* This file contains a example of ttc.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   zyh  03/11/2025  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_ttc_public.h"  
#include "fmsh_gic.h"  
#include "fmsh_psu_parameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FTtcPs_T g_ttc_dev;
volatile u8 g_ttc0_flag = 0;

/************************** Function Prototypes ******************************/

void timer1_handler(FTtcPs_T *DevPtr)
{
    FTtcPs_ClearTimerNInterrupt(DevPtr, 1);
    FTtcPs_setTimerEnble(DevPtr, timer1, FMSH_clear);
    g_ttc0_flag = 1;
}

/******************************************************************************
*
* @description
*    A example of ttc, when gtc current value reach 0, trigger interrupt.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FTtcPs_example(u16 deviceId)
{
    FTtcPs_dev_init (&g_ttc_dev, deviceId);

    FGicPs_Connect(&IntcInstance, g_ttc_dev.IntId[0], (FMSH_InterruptHandler)timer1_handler, &g_ttc_dev); //timer1
    FGicPs_Enable(&IntcInstance, g_ttc_dev.IntId[0]);   
    
    FTtcPs_setTimerEnble(&g_ttc_dev, timer1, FMSH_clear);
    FTtcPs_setTimerMode(&g_ttc_dev, timer1, user_DefinedCount_mode);
    FTtcPs_setTimerInterruptMask(&g_ttc_dev, timer1, FMSH_clear); 
    FTtcPs_setTIMERPWM(&g_ttc_dev, timer1, FMSH_clear); 
    FTtcPs_TimerNLoadCount(&g_ttc_dev, timer1, FPAR_TTCPS_0_TTC_CLK_FREQ_HZ);   //one second timer
    
    g_ttc0_flag = 0;

    FTtcPs_setTimerEnble(&g_ttc_dev, timer1, FMSH_set);
    
    delay_ms(1500);
    
    if (g_ttc0_flag){
        return 0;
    }
    else{
        printf("Timer interrupt timeout\r\n");
        return 1;
    }
}
