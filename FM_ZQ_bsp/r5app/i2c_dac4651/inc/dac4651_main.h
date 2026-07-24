/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. FAN
 */
#ifndef _DAC4651_MAIN_H_
#define _DAC4651_MAIN_H_

#include <stdio.h>
#include "fmsh_common.h"
#include "fmsh_i2c_private.h"
#include "fmsh_i2c_public.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"
#include "dac4651_api.h"

/* Exported macro ------------------------------------------------------------*/
#define DEBUG           0   

#define DATA_DIV_VALUE         1
#define DATA_VREFIO_VALUE      2.5
#define DATA_GAIN_VALUE        1
 
#define GAIN_REG                0x4
#define DAC_REG                 0x8
#define DEVICE_REG              0x1

/* Exported functions --------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

void get_dac4651_dac(void);
void set_dac4651_dac(u8 v);
void set_dac4651_by_eeprom(void);

#ifdef __cplusplus
}
#endif

#endif