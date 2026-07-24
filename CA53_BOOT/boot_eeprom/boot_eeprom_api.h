/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    boot_eeprom_api.h
 * @brief   This file porvide all EEPROM I2C firmware functions. 
 */


#ifndef _BOOT_EEPROM_API_H_
#define _BOOT_EEPROM_API_H_

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
#ifndef CONFIG_EEPROM_PAGE_SIZE 
    #define CONFIG_EEPROM_PAGE_SIZE      128
#endif

#ifndef EEPROM_MAX_BLOCK_SIZE
    #define EEPROM_MAX_BLOCK_SIZE  1024
#endif

#ifndef CONFIG_EEPROM_EEPROM_SIZE 
    #define CONFIG_EEPROM_EEPROM_SIZE      (64*1024)
#endif

#ifndef CONFIG_EEPROM_ADDR_LEN 
    #define CONFIG_EEPROM_ADDR_LEN      2
#endif


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

int mw_eeprom_read_bin(u32 start_addr, u32 end_addr, const char *cnf_file);
int mw_eeprom_write_bin(u32 start_addr, u32 end_addr, const char *cnf_file);
int mw_eeprom_read(u32 addr, u8 *data, int lenth);
int mw_eeprom_write(u32 addr, u8 *data, int lenth);
#ifdef __cplusplus
}
#endif


#endif /*_BOOT_EEPROM_API_H_*/