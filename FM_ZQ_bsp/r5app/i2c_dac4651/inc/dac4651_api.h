/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */

 #ifndef _DAC4651_API_H_
 #define _DAC4651_API_H_
 
#include <stdio.h>
#include "fmsh_common.h"
#include "fmsh_i2c_private.h"
#include "fmsh_i2c_public.h"
 
#ifdef __cplusplus
extern "C" {
#endif
  
#define IIC_DEVICE_ID		        XPAR_IIC_0_DEVICE_ID
#define INTC_DEVICE_ID		       XPAR_INTC_0_DEVICE_ID
#define IIC_INTR_ID		                  PL0_INT_ID
#define IIC_SLAVE_ADDR		                        0x49
#define EEPROM_DAC4651M_INFO_ADDR                       0xF5

#define RECEIVE_COUNT		                           2
#define SEND_COUNT		                           3
   
#define DATA_DIV_VALUE                                     1
#define DATA_VREFIO_VALUE                                2.5
#define DATA_GAIN_VALUE                                    1
   
#define GAIN_REG                                         0x4
#define DAC_REG                                          0x8
#define DEVICE_REG                                       0x1

s32 i2c_dac4651_set_reg(u8 command, u16 *data);
u32 dac4651_set_v_out(double *v_out);
s32 dac4651_get_dac_reg(u16 *data);
s32 dac4651_set_dac_by_eeprom(void);

#ifdef __cplusplus
}
#endif

#endif