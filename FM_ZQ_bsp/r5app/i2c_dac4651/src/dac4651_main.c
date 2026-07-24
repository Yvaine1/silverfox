#include "dac4651_main.h"

static u8 dac4651_i2c_init_done = FMSH_FAILURE;

void get_dac4651_dac(void)
{
    s32 state = FMSH_FAILURE;
    u16 data = 0;
    
    state = dac4651_get_dac_reg(&data);
    if(state != FMSH_SUCCESS)
    {
        fmsh_print("DAC4651[%s]: get dac success!\r\n",__func__);
    }
    return;
}

void set_dac4651_dac(u8 v)
{
    int state = FMSH_FAILURE;
    double data = (double) v/100.0;
    if (data < 0 || data > 2.49) 
    {
        fmsh_print("Error: V_out value must be between 0 and 2.49\r\n");
        return;
    }
    
    state = dac4651_set_v_out(&data);
#if DEBUG
    if(state == FMSH_SUCCESS)
    {
        fmsh_print("DAC4651[%s]: set dac success!\r\n",__func__);
    }
    else
    {
        fmsh_print("DAC4651[%s]: set dac failed!\r\n",__func__);
    }
#endif
    return;
}

void set_dac4651_by_eeprom(void)
{
    int state = FMSH_FAILURE;
    
    state = dac4651_set_dac_by_eeprom();
#if DEBUG
    if(state == FMSH_SUCCESS)
    {
        fmsh_print("DAC4651[%s]: set dac by eeprom success!\r\n",__func__);
    }
    else
    {
        fmsh_print("DAC4651[%s]: set dac by eeprom failed!\r\n",__func__);
    }
#endif
    return;
}