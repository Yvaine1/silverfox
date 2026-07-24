/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    eeprom_main.h
 * @brief   This file porvide EEPROM I2C shell cmmoand access. 
 */


#ifndef _EEPROM_MAIN_H_
#define _EEPROM_MAIN_H_

#include "fmsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif
  
#define EEPROM_DAC4651M_INFO_ADDR                               0xF5
#define EEPROM_PAGE_SIZE                                       0x400
#define INT_BYTE_LEN                                               4      
#define EEPROM_DEBUG                                              0U

#define EEPROM_TX_DSA_ADDR                                      0x4000
#define EEPROM_RX_LEVEL1_DSA_ADDR                               0x4004
#define EEPROM_RX_LEVEL2_DSA_ADDR                               0x4008

#define EEPROM_INT_ADDR(page, offset)  ((u16)((page - 1) * EEPROM_PAGE_SIZE + (offset * INT_BYTE_LEN)))

typedef enum {
    RX_GAIN_LEVEL_DEFAULT,
    RX_GAIN_LEVEL_1,
    RX_GAIN_LEVEL_2,
    NUM_RX_GAIN_LEVEL
} rx_gain_level_t;

// TX gain 存储结构
typedef struct {
    uint16_t dsa_idx;    // TX DSA 索引
    int16_t  gain_val;   // TX 实际增益值
} tx_gain_eeprom_t;

// RX gain 单级存储结构
typedef struct {
    uint16_t dsa_idx;    // RX DSA 索引
    int16_t  gain_val;   // RX 实际增益值
} rx_gain_level_eeprom_t;

// RX gain 完整存储结构（包含 Level1 和 Level2）
typedef struct {
    rx_gain_level_eeprom_t level1;
    rx_gain_level_eeprom_t level2;
} rx_gain_eeprom_t;

/**
  * @brief  
  * @param  
  * @param   
  * @retval 
  */
void eeprom_set_dac4651(u16 * data);
void eeprom_get_dac4651(u16 *data);

/**
  * @brief  将tx gain配置持久化在eeprom中.eeprom接口一次写入16位.
  * @param  data,tx gain配置值.
  * @return 成功返回0,失败返回1
  */
int eeprom_set_tx_dsa(tx_gain_eeprom_t* data);
/**
  * @brief  从eeprom获取tx gain配置.eeprom接口一次读16位.
  * @param  data,读取到的tx gain配置值.
  * @return 成功返回0,失败返回1
  */
int eeprom_get_tx_dsa(tx_gain_eeprom_t* data);
/**
  * @brief  将rx gain配置持久化在eeprom中.eeprom接口一次写入16位.
  * @param  data,rx gain配置值.
  * @param  level,rx gain配置分两级,该参数指示需要配置的rx gain的level
  * @return 成功返回0,失败返回1
  */
int eeprom_set_rx_dsa(rx_gain_level_eeprom_t* data, u8 level);
/**
  * @brief  从eeprom获取rx gain配置.eeprom接口一次读16位.
  * @param  data,读取到的rx gain配置值.
  * @param  level,rx gain配置分两级,该参数指示需要读取的rx gain的level
  * @return 成功返回0,失败返回1
  */
int eeprom_get_rx_dsa(rx_gain_level_eeprom_t* data, u8 level);

int eeprom_write_dac_calib_table(u8 page, u16 offset, int *data);
int eeprom_read_dac_calib_table(u8 page, u16 offset, int *data);

#ifdef __cplusplus
}
#endif


#endif /*_I2C_EEPROM_MAIN_H_*/