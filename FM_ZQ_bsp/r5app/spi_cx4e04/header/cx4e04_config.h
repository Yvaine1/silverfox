/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */
#ifndef _AU5329_CONFIG_H_
#define _AU5329_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#define MAX_DATA		32

static u8 ReadBuffer[MAX_DATA];
static u8 WriteBuffer[MAX_DATA];

#define CX4E04_PLL1_INPUT_CLOCK_STATE_REG   0x065
#define CX4E04_LOPLL_LOCK_STATE_REG         0x0e8
#define CX4E04_LOPLL_AAC_READY_REG          0X0e9
#define CX4E04_SPI_DEV                      1

/* HWID definitions */
#define HWID_GPIO_BASE_ADDRESS		        0xE0003100
#define HWID_GPIO_DATA0_RO_OFFSET	        0x50
#define HWID_GPIO_DATA_BIT0			        (50-32)



#define HWID_VERSION_1  1
#define HWID_VERSION_0  0



u8 cx4e04_read(u8 addr);
void cx4e04_write(u8 addr, u8 dat);
void cx4e04_config();
void cx4e04_init();
void cx4e04_pll_status(void);


  
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */

