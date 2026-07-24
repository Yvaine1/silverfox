/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    i2c_eeprom_main.c
 * @brief   This file porvide EEPROM I2C shell cmmoand access. 
 */


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eeprom_main.h"
#include "eeprom_api.h"
#include "fmsh_common.h"


static int eeprom_write_u16(u16 addr, u8 addr_len, u16 *data)
{
    STATUS state = FMSH_FAILURE;
    u8 w_buf[2] = {0};
    
    w_buf[0] = (u8)((*data) >> 8); 
    w_buf[1] = (u8)((*data) & 0xFF); 
    state = eeprom_write_bytes(addr, addr_len, w_buf, 2);
    if (OK != state)
    {
        fmsh_print("EEPROM-ERR[%s]: write failed!\r\n",__func__);
        return FMSH_FAILURE;
    }
#if EEPROM_DEBUG
    fmsh_print("EEPROM[%s]: write success! reg:0x%x, data:0x%x\r\n",__func__, addr, *data);
#endif
    delay_ms(5);
    return FMSH_SUCCESS;
}

static int eeprom_read_u16(u16 addr, u8 addr_len, u16 *data)
{
    u8 r_buf[CONFIG_EEPROM_ADDR_LEN] = {0};
    STATUS state = FMSH_FAILURE;
    
    if (NULL == data)
    {
        fmsh_print("EEPROM-ERR[%s]: nullptr!\r\n",__func__);
        return FMSH_FAILURE;
    }
    
    state = eeprom_read_bytes(addr, addr_len, r_buf, 2);
    if (OK != state)
    {
        fmsh_print("EEPROM-ERR[%s]: read failed!\r\n",__func__);
        return FMSH_FAILURE;
    }

    *data = (r_buf[1]) | (r_buf[0] << 8);
#if EEPROM_DEBUG
    fmsh_print("EEPROM[%s]: read success! reg:0x%x, data:0x%x\r\n",__func__, addr, *data);
#endif   

    return FMSH_SUCCESS;
}

/***********DAC4651***********/
void eeprom_set_dac4651(u16 *data)
{
    eeprom_write_u16(EEPROM_DAC4651M_INFO_ADDR, CONFIG_EEPROM_ADDR_LEN, data);
}

void eeprom_get_dac4651(u16 *data)
{
    eeprom_read_u16(EEPROM_DAC4651M_INFO_ADDR, CONFIG_EEPROM_ADDR_LEN, data);
}

int eeprom_set_tx_dsa(tx_gain_eeprom_t* data)
{
    STATUS state      = FMSH_FAILURE;
    u16    write_addr = EEPROM_TX_DSA_ADDR;
    state = eeprom_write_bytes(write_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->dsa_idx, CONFIG_EEPROM_ADDR_LEN);
    if (FMSH_SUCCESS != state)
    {
        fmsh_print("EEPROM-ERR[%s]: write failed!\r\n",__func__);
        return state;
    }

    delay_ms(5);

    write_addr += 2;
    state = eeprom_write_bytes(write_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->gain_val, CONFIG_EEPROM_ADDR_LEN);
    if (FMSH_SUCCESS != state)
    {
        fmsh_print("EEPROM-ERR[%s]: write failed!\r\n",__func__);
        return state;
    }

    // 这里需要delay,有可能会导致read失败
    delay_ms(5);

    return state;
}

int eeprom_get_tx_dsa(tx_gain_eeprom_t* data)
{
    STATUS state      = FMSH_FAILURE;
    u16    read_addr  = EEPROM_TX_DSA_ADDR;
    state = eeprom_read_bytes(read_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->dsa_idx, CONFIG_EEPROM_ADDR_LEN);
    if (FMSH_SUCCESS != state)
    {
        fmsh_print("EEPROM-ERR[%s]: read failed!\r\n", __func__);
        return state;
    }

    read_addr += 2;
    state = eeprom_read_bytes(read_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->gain_val, CONFIG_EEPROM_ADDR_LEN);
    if (FMSH_SUCCESS != state)
    {
        fmsh_print("EEPROM-ERR[%s]: read failed!\r\n", __func__);
        return state;
    }

    return state;
}

int eeprom_set_rx_dsa(rx_gain_level_eeprom_t* data, u8 level)
{
    STATUS state      = FMSH_FAILURE;
    if (RX_GAIN_LEVEL_1 == level)
    {
        u16 write_addr = EEPROM_RX_LEVEL1_DSA_ADDR;
        state = eeprom_write_bytes(write_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->dsa_idx, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: write failed!\r\n", __func__);
            return state;
        }

        delay_ms(5);

        write_addr += 2;
        state = eeprom_write_bytes(write_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->gain_val, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: write failed!\r\n", __func__);
            return state;
        }
    }
    else if (RX_GAIN_LEVEL_2 == level)
    {
        u16 write_addr = EEPROM_RX_LEVEL2_DSA_ADDR;
        state = eeprom_write_bytes(write_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->dsa_idx, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: write failed!\r\n", __func__);
            return state;
        }

        delay_ms(5);

        write_addr += 2;
        state = eeprom_write_bytes(write_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->gain_val, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: write failed!\r\n", __func__);
            return state;
        }
    }
    else
    {
        fmsh_print("Unsupported rx gain level: %d\r\n", level);
    }

    delay_ms(5);

    return state;
}

int eeprom_get_rx_dsa(rx_gain_level_eeprom_t* data, u8 level)
{
    STATUS state = FMSH_FAILURE;
    if (RX_GAIN_LEVEL_1 == level)
    {
        u16 read_addr  = EEPROM_RX_LEVEL1_DSA_ADDR;
        state = eeprom_read_bytes(read_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->dsa_idx, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: read failed!\r\n", __func__);
            return state;
        }

        read_addr += 2;
        state = eeprom_read_bytes(read_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->gain_val, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: read failed!\r\n", __func__);
            return state;
        }
    }
    else if (RX_GAIN_LEVEL_2 == level)
    {
        u16 read_addr  = EEPROM_RX_LEVEL2_DSA_ADDR;
        state = eeprom_read_bytes(read_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->dsa_idx, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: read failed!\r\n", __func__);
            return state;
        }

        read_addr += 2;
        state = eeprom_read_bytes(read_addr, CONFIG_EEPROM_ADDR_LEN, (u8*)&data->gain_val, CONFIG_EEPROM_ADDR_LEN);
        if (FMSH_SUCCESS != state)
        {
            fmsh_print("EEPROM-ERR[%s]: read failed!\r\n", __func__);
            return state;
        }
    }
    else
    {
        fmsh_print("Unsupported rx gain level: %d\r\n", level);
    }

    return state;
}

/***********FREQ_OFFSET_CALIBRATION***********/
int eeprom_read_dac_calib_table(u8 page, u16 offset, int *data)
{
    int ret = FMSH_FAILURE;
    u16 addr = 0;
    u16 val_high = 0;
    u16 val_low = 0;

    if (data == NULL) {
        fmsh_print("EEPROM-ERR: %s nullptr\r\n", __func__);
        return FMSH_FAILURE;
    }

    addr = EEPROM_INT_ADDR(page, offset);

    ret = eeprom_read_u16(addr, CONFIG_EEPROM_ADDR_LEN, &val_high);
    ret |= eeprom_read_u16(addr + 2, CONFIG_EEPROM_ADDR_LEN, &val_low);
    if (FMSH_FAILURE == ret)
    {
        return FMSH_FAILURE;
    }
    *data = (int)((val_high << 16) | val_low);
#if EEPROM_DEBUG
    fmsh_print("page %u offset %u: %d\r\n", page, offset, *data);
#endif
    return FMSH_SUCCESS;
}

int eeprom_write_dac_calib_table(u8 page, u16 offset, int *data)
{
    int ret;
    u16 addr = 0;
    u16 val_high = 0;
    u16 val_low = 0;
    u8 write_buf[2] = {0};

    if (data == NULL) {
        fmsh_print("EEPROM-ERR: %s nullptr\r\n", __func__);
        return FMSH_FAILURE;
    }

    addr = EEPROM_INT_ADDR(page, offset);
    
    val_high = (u16)((data[0] >> 16) & 0xFFFF);
    val_low = (u16)(data[0] & 0xFFFF);

    ret = eeprom_write_u16(addr, CONFIG_EEPROM_ADDR_LEN, &val_high);
    ret |= eeprom_write_u16(addr + 2, CONFIG_EEPROM_ADDR_LEN, &val_low);

    ret |= eeprom_read_u16(addr, CONFIG_EEPROM_ADDR_LEN, &val_high);
    ret |= eeprom_read_u16(addr + 2, CONFIG_EEPROM_ADDR_LEN, &val_low);
    if (FMSH_FAILURE == ret)
    {
        return FMSH_FAILURE;
    }
    *data = (int)((val_high << 16) | val_low);
#if EEPROM_DEBUG
    fmsh_print("page %u offset %u: %d\r\n", page, offset, *data);
#endif
    return FMSH_SUCCESS;
}
