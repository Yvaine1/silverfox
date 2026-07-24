/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    eeprom_api.h
 * @brief   This file porvide all EEPROM I2C firmware functions. 
 */


#ifndef _NST175_API_H_
#define _NST175_API_H_

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "fmsh_gic.h"
#include "fmsh_common.h"
#include "fmsh_i2c_private.h"
#include "fmsh_i2c_public.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
#ifndef EEPROM_START_ADDR
    #define EEPROM_START_ADDR  0x0
#endif

#ifndef CONFIG_EEPROM_ADDR_LEN 
    #define CONFIG_EEPROM_ADDR_LEN      2
#endif


#define CONFIG_NST175_ADDR_LEN                  2
#define CONFIG_NST175_TEMP_REG                0x0

#define DONE                                    1
#define NO_DONE                                 0
/* Exported functions --------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  
  * @param  
  * @param   
  * @retval 
  */
void i2c_init(u8 bus, u8 device_address);
int nst175_i2c_read_temp(int *data);

#ifdef __cplusplus
}
#endif


#endif /*_I2C_EEPROM_API_H_*/