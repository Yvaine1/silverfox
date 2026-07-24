/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. FAN
 */
#ifndef _nst175_MAIN_H_
#define _nst175_MAIN_H_

#include <stdio.h>
#include "fmsh_common.h"
#include "fmsh_i2c_private.h"
#include "fmsh_i2c_public.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"
#include "nst175_api.h"

/* Exported macro ------------------------------------------------------------*/
#define DEBUG           0   
  
#define CONFIG_NST175_I2C_BUS                   1
#define CONFIG_NST175_I2C_ADDR               0x48

/* Exported functions --------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif


void nst175_get_temp(void);

#ifdef __cplusplus
}
#endif

#endif