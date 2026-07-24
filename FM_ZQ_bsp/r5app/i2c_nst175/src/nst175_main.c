#include "nst175_main.h"

static u8 nst175_i2c_init_done = FMSH_FAILURE;

void nst175_get_temp()
{
    s32 state = FMSH_FAILURE;
   
    int data = 0;
    
    if (FMSH_SUCCESS != nst175_i2c_init_done)
    {
        i2c_init(CONFIG_NST175_I2C_BUS, CONFIG_NST175_I2C_ADDR);
    }
    
    state = nst175_i2c_read_temp(&data);
    if(state == FMSH_SUCCESS)
    {
      fmsh_print("NST175[%s]: get temp success, temp: %d!\r\n",__func__, data);
    }
    return;
}


