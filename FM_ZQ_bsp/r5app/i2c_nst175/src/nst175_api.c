/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */
#include "nst175_api.h"

#define I2C_LOOP_TIMEOUT 1000  // us

FI2cPs_T g_I2c_dev;
FIicPs_Instance_T g_I2c_Instance;


static u8 FI2c1Ps_DeviceInit (FI2cPs_T *pDev, void *pI2cInstance, void *I2cParam, int bus)
{
    u8 ret = FMSH_SUCCESS;
    FI2cPs_Config *Config = NULL;
    Config = FI2cPs_LookupConfig(bus);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }
    ret = FI2cPs_init(pDev, Config, pI2cInstance, I2cParam);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }
    return ret;
}

static void FI2cPs_ControllerInit(FI2cPs_T *dev, u8 device_address)
{
    /* Set up the clock count register.  The argument I2C1_CLOCK is
     specified as the I2C dev input clock.*/
    FI2cPs_ClockSetup(dev, (dev->input_clock) / 1000000);
    /* set the speed mode to standard*/
    FI2cPs_setSpeedMode(dev, I2c_speed_standard);
    /* use 7-bit addressing*/
    FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
    FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);
    /* enable restart conditions*/
    FI2cPs_enableRestart(dev);
    /* enable master FSM*/
    FI2cPs_enableMaster(dev);
    FI2cPs_disableSlave(dev);

    // Use the start byte protocol with the target address when
    // initiating transfer.
    FI2cPs_setTxMode(dev, I2c_tx_target);

    /* set target address to the I2C slave address*/
    FI2cPs_setTargetAddress(dev, device_address);

    /*enable the dev I2C device*/
    FI2cPs_enable(dev);

    return;
}

static u8 FI2cPs_PageWrite (FI2cPs_T *master, u16 iaddress, u8 addr_len,
                                   u8 *buffer, u32 len)
{
    u8 i = 0;
    u8 addr_buf[2] = {0};
    u32 timeout_cnt = I2C_LOOP_TIMEOUT;   
    addr_buf[1] =  iaddress & 0x00ff;
    addr_buf[0] = (iaddress & 0xff00) >> 8;
 
    /* write internal address */
    for (i = 0; i < addr_len; i++)
    {
      FI2cPs_write(master, addr_buf[i]);
      delay_1ms();
    }
 

    for (i = 0; i < len; i++)
    {
        timeout_cnt = I2C_LOOP_TIMEOUT;
        while (FI2cPs_isTxFifoEmpty(master) != true)
        {
            delay_1us();
            timeout_cnt--;
            if (timeout_cnt == 0)
            {
                return FMSH_FAILURE;
            }
        }
        if (i == len - 1)
        {
            FI2cPs_issueSTOP(master, buffer[i]);
        }
        else
        {
            FI2cPs_write(master, buffer[i]);
        }
    }
    return FMSH_SUCCESS;
}

static u8 FI2cPs_PageRead (FI2cPs_T *master, u16 iaddress, u8 addr_len,
                                       u8 *buffer, u32 len)
{
    u32 i = 0;
    u8 add_buf[2] = {0};
    u32 timeout_cnt = I2C_LOOP_TIMEOUT;
    
    add_buf[1] =  iaddress & 0x00ff;
    add_buf[0] = (iaddress & 0xff00) >> 8;
    /*dummy write*/
    for (i = 0; i < addr_len; i++)
    {
      FI2cPs_write(master, add_buf[i]);
      delay_1ms();
      
    }

    /*Issue read*/
    for (i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            FI2cPs_issueReadStop(master);
        }
        else
        {
            FI2cPs_issueRead(master);
        }
        timeout_cnt = I2C_LOOP_TIMEOUT;
        while (FI2cPs_isRxFifoEmpty(master) == true)
        {
            delay_1us();
            timeout_cnt--;
            if (timeout_cnt == 0)
            {
                return FMSH_FAILURE;
            }
        }
        buffer[i] = FI2cPs_read(master);
    }

    return FMSH_SUCCESS;
}

void i2c_init(u8 bus, u8 device_address)
{
    u8 ret = FMSH_SUCCESS;
    
    FI2cPs_T *pI2c_dev = &g_I2c_dev;
    FIicPs_Instance_T *pI2c_Instance = &g_I2c_Instance;
    FIicPs_Param_T I2c_Param;

    FI2c1Ps_DeviceInit(pI2c_dev, pI2c_Instance, &I2c_Param, bus);
    FI2cPs_ControllerInit(pI2c_dev, device_address);

    fmsh_print("I2C Initialize %s\r\n", (ret == FMSH_SUCCESS) ? "Success" : "Failed");
}

u8 i2c_write(u16 addr, u8 addr_len, u8 *data, u32 lenth)
{
    u8 ret = FMSH_FAILURE;

    ret = FI2cPs_PageWrite(&g_I2c_dev, addr, addr_len, data, lenth);
    return ret;
}

u8 i2c_read(u16 addr, u8 addr_len, u8 *data, u32 lenth)
{
    u8 ret = FMSH_FAILURE;

    ret = FI2cPs_PageRead(&g_I2c_dev, addr, addr_len, data, lenth);
    return ret;
}

int nst175_i2c_read_temp(int *data)
{
    s8 value[CONFIG_NST175_ADDR_LEN] = {0};
    
    if (i2c_read(CONFIG_NST175_TEMP_REG, 1, (u8*)value, CONFIG_NST175_ADDR_LEN) != FMSH_SUCCESS) 
    {
        fmsh_print("NST175 ERR: I2C read failed\r\n");
        return FMSH_FAILURE;
    }

    *data = (int)((s16)((value[0] << 8) | (value[1] & 0xFF)) >> 8);

    return FMSH_SUCCESS;
}