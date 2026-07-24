#include "fmsh_common.h"
#include "xparameters.h"
#include "xiic.h"
#include "dac4651_api.h"
#include "eeprom_main.h"

XIic IicInstance;		/* The instance of the IIC device. */

u8 WriteBuffer[SEND_COUNT];	/* Write buffer for writing a page. */
u8 ReadBuffer[RECEIVE_COUNT];	/* Read buffer for reading a page. */

int dac4651_i2c_init(void)
{
    int Status;
    XIic_Config *ConfigPtr;	

    ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
    if (ConfigPtr == NULL)
    {
       Status = FMSH_FAILURE;
    }
    
    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr, ConfigPtr->BaseAddress);

    XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE, IIC_SLAVE_ADDR);
    XIic_SetGpOutput(&IicInstance, 0x0);

    fmsh_print("DAC4651[%s]: Initialize %s\r\n", __func__, (Status == FMSH_SUCCESS) ? "Success" : "Failed");

    return FMSH_SUCCESS;
}

s32 i2c_dac4651_set_reg(u8 command, u16 *data)
{
    WriteBuffer[0] = (u8)(command & 0xFF);
    WriteBuffer[1] = (u8)((*data >> 8) );
    WriteBuffer[2] = (u8)( *data & 0xFF);
    
    int RecvCount;
    RecvCount=XIic_Send(IicInstance.BaseAddress,IIC_SLAVE_ADDR, WriteBuffer, SEND_COUNT, XIIC_STOP);
    if (RecvCount != SEND_COUNT) 
    {
        fmsh_print("DAC4651[%s]: IIC read error, got %d/%d bytes\r\n", __func__, RecvCount, SEND_COUNT);
        return XST_FAILURE;
    }

    return FMSH_SUCCESS;
}

u32 dac4651_set_v_out(double *v_out)
{
    int ret = FMSH_SUCCESS;
    u16 DAC_DATA;
    u8 data_buff[2] = {0};
    u16 GAIN_REGISTER_VALUE = 0x101;

    DAC_DATA = (u16)((*v_out * (1 << 16) * DATA_DIV_VALUE) / (DATA_VREFIO_VALUE * DATA_GAIN_VALUE));
    ret = i2c_dac4651_set_reg(GAIN_REG, &GAIN_REGISTER_VALUE);
    if ( ret != FMSH_SUCCESS)
    {
        fmsh_print("Error set gain data");
    }

    fmsh_print("DAC4651[%s]: dac data = 0x%x\r\n", __func__, DAC_DATA);
    eeprom_set_dac4651(&DAC_DATA);
    ret = i2c_dac4651_set_reg(DAC_REG, &DAC_DATA);
    if ( ret != FMSH_SUCCESS)
    {
        fmsh_print("DAC4651[%s]: Error set dac data", __func__);
    }

    return ret;
}

int dac4651_get_dac_reg(u16 *data)
{
    int RecvCount;
    float v_out;
    WriteBuffer[0] = DAC_REG;
    XIic_Send(IicInstance.BaseAddress,IIC_SLAVE_ADDR, WriteBuffer, 1, XIIC_STOP);
    
    RecvCount = XIic_Recv(IicInstance.BaseAddress, IIC_SLAVE_ADDR, ReadBuffer, RECEIVE_COUNT, XIIC_STOP);
    *data = ((u16)ReadBuffer[0] << 8) + (u16)ReadBuffer[1];
    if (RecvCount != RECEIVE_COUNT) {
        fmsh_print("DAC4651[%s]: IIC read error, got %d/%d bytes\r\n", __func__, RecvCount, RECEIVE_COUNT);
        return FMSH_FAILURE;
    }
    v_out = (*data)*DATA_VREFIO_VALUE*DATA_GAIN_VALUE/DATA_DIV_VALUE/65536;
    fmsh_print("DAC4651[%s]: Get dac4651m dac register success, VOUT is %.2f V.\r\n", __func__, v_out);
    return FMSH_SUCCESS;
}

int dac4651_set_dac_by_eeprom(void)
{
    u32 ret = FMSH_SUCCESS;
    u16 DAC_DATA;
    u16 GAIN_REGISTER_VALUE = 0x101;
    float v_out = 0;
    
    eeprom_get_dac4651(&DAC_DATA);
    fmsh_print("DAC4651[%s]: eeprom data: 0x%x\r\n", __func__, DAC_DATA);
    if ((0xFFFF == DAC_DATA) || (0x0 == DAC_DATA))
    {
        fmsh_print("DAC4651[%s]: dac hasn't been configured, please config first!", __func__);
        return FMSH_FAILURE;
    }

    ret = i2c_dac4651_set_reg(GAIN_REG, &GAIN_REGISTER_VALUE);
    if ( ret != FMSH_SUCCESS)
    {
        fmsh_print("DAC4651[%s]: Error set gain data", __func__);
        return FMSH_FAILURE;
    }

    
    v_out = DAC_DATA*DATA_VREFIO_VALUE*DATA_GAIN_VALUE/DATA_DIV_VALUE/65536;
    fmsh_print("DAC4651[%s]: dac_data = %.2f\r\n", __func__, v_out);

    ret = i2c_dac4651_set_reg(DAC_REG, &DAC_DATA);
    if ( ret != FMSH_SUCCESS)
    {
        fmsh_print("DAC4651[%s]: Error set dac data", __func__);
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}