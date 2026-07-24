/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_gpio_example.c
*
* This file contains a example of gpio.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   zyh  08/29/2024  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_gpio_public.h"
#include "fmsh_psu_parameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Local Function ***********************************/

/************************** Extern Function **********************************/

/******************************************************************************
*
* @description
*    A minimal test of gpio, configure GPIO 0 as output and drive.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FGpioPs_output_example(u8 deviceId)
{
    FGpioPs_T gpio0;
    FGpioPs_bank_init(deviceId, &gpio0);  /*gpio bank0, mio 0-25*/
                                          /*gpio bank3, emio 0-31*/
    FGpioPs_setDirection(&gpio0, 0xFFFFFFFF);    /*set gpio direction all output */
    FGpioPs_write(&gpio0, 0x00000000);       /*output low level to gpio pins*/
    delay_ms(1000);
    FGpioPs_write(&gpio0, 0xFFFFFFFF);       /*output high level to gpio pins*/
    
    delay_ms(1000);
    
    FGpioPs_writeBit(&gpio0, Gpio_low,       /*set gpio pin 0 to low while other pins remain*/
                    Gpio_bit_0);

    return 0;
}

/******************************************************************************
*
* @description
*    A minimal test of gpio, configure GPIO 0 as input and get ext data.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FGpioPs_input_example(u8 deviceId)
{
    u32 readback;
    
    FGpioPs_T gpio0;
    FGpioPs_bank_init(deviceId, &gpio0);     /*gpio bank0, mio 0-25*/
                                             /*gpio bank3, emio 0-31*/
    FGpioPs_setDirection(&gpio0, 0x0);       /*set gpio direction all input */
    delay_ms(10);
    
    readback = FGpioPs_read(&gpio0);         /*get gpio bank0 pin values */
    printf("input data is : %x\r\n",readback);
    delay_ms(100);
    
    enum FGpioPs_state pinLevel;
    pinLevel = FGpioPs_readBit(&gpio0,  Gpio_bit_0);
    
    printf("MIO %d, input data is : %x\r\n",Gpio_bit_0, pinLevel);
    
    return 0;
}

u8 FGpioPs_example(u8 deviceId){
    u8 ret = 0;
    ret |= FGpioPs_output_example( deviceId);
    ret |= FGpioPs_input_example(deviceId);
    return ret;
}