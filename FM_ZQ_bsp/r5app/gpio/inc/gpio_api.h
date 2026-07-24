/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    gpio_api.h
 * @brief   This file porvide all gpio functions. 
 */


#ifndef _GPIO_API_H_
#define _GPIO_API_H_

typedef enum
{
    HW_ID_00 = 0,   //yinhu_V2
    HW_ID_01 = 1,   //LRM
    HW_ID_UNKNOWN,
} HW_ID;


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
HW_ID mw_get_hwid(void);

#ifdef __cplusplus
}
#endif


#endif /*_I2C_EEPROM_API_H_*/