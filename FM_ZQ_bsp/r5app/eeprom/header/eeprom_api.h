/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    eeprom_api.h
 * @brief   This file porvide all EEPROM I2C firmware functions. 
 */


#ifndef _EEPROM_API_H_
#define _EEPROM_API_H_

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "fmsh_common.h"
#include "fmsh_i2c_private.h"
#include "fmsh_i2c_public.h"
#include "dg_common.h"
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
#define CONFIG_EEPROM_I2C_BUS                   1
#define CONFIG_EEPROM_I2C_ADDR                  0x50
#define CONFIG_EEPROM_ADDR_LEN                  2

#define EEPROM_BAND_INFO_ADDR                   0xFE
#define DONE                                    1
#define NO_DONE                                 0

#define EEPROM_ANT_TYPE_ADDR                    0xF0
#define EEPROM_BOARD_TYPE_ADDR     	            0xFF 
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
STATUS eeprom_write_bytes(u16 addr, u8 addr_len, u8 *data, u32 lenth);
STATUS eeprom_read_bytes(u16 addr, u8 addr_len, u8 *data, u32 lenth);
u16 eeprom_get_reg (u8 iaddress);
STATUS eeprom_get_bandinfo(u8 *bandid);
STATUS eeprom_set_bandinfo(u8 bandid);
void eeprom_i2c_init(void);
STATUS eeprom_set_ant_type(u8 ant_type);
STATUS eeprom_get_ant_type(u8 *ant_type);
STATUS eeprom_set_board_type(u8 board_type);
STATUS eeprom_get_board_type(u8 *board_type);
#ifdef __cplusplus
}
#endif


#endif /*_I2C_EEPROM_API_H_*/