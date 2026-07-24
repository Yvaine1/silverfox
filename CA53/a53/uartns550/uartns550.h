/** 
 * @file   uartns550.h
 * @note   Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.
 * @brief   
 *
 * @author guodecai	
 * @date   2026/05/09
 *
 * @version
 *  date        |version |author              |message
 *  :----       |:----   |:----               |:------
 *  2026/05/09  |V1.0    |guodecai            |create base code
 * @warning 
 */

#ifndef _UARTNS550_H_
#define _UARTNS550_H_

#include "fmsh_common.h"
#include "FreeRTOS.h"
#include "xuartns550_parameters.h"
#include "gps.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UARTNS550_EN
//#define PRINTF_TOD_TEST
//#define API_TEST
//#define PPS_TIME_TEST
//#define UDP_TOD_TEST

#define MAX_UARTNS550_DEVICE_ID                   1
#define UARTNS550_DEVICE_0_ID	                  XPAR_UARTNS550_0_DEVICE_ID
#define UARTNS550_DEVICE_0_INT_PRIOIRTY           (21 * 8)
#define PPS_INT_PRIOIRTY                          (21 * 8)
#define UARTNS550_DEVICE_0_INT                    PL6_INT_ID
#define PPS_INT                                   PL3_INT_ID

#define TOD_RECV_SIZE                             (2048)
#define STABLE_CNT                                (2)
#define SLEEP_TIMES                               (12)
#define SLEEP_TIME_MS                             (50)
#define NMEA_MAX_LEN                              (256)
#define CALI_FPGA_SECOND                          (60)
#define PPS_CYCLE_BUF_SIZE                        (4096)

#define W_SECOND_IN_REG                           (0x8000558C)
#define R_SECOND_IN_REG                           (0x80005594)
#define W_SECOND_REG                              (0x80005590)
#define R_SECOND_REG                              (0x80005598)

#define FRAME_SYNC                                0x7E   
#define VALID_CMD                                 0x84   
#define MAX_FRAME_SIZE                            1024
#define XL_TMP_SIZE                               41

enum PRINT_LEVEL
{
    DFT_NONE = 0,
    ERRORS,
    DEBUG,
    INFO,
    ALL,
    MAX_LEVEL,
};

enum ANT_TYPE
{
    XL_TYPE = 0,
    STD_TYPE = 1,
    MAX_TYPE_NUM,
};

extern enum PRINT_LEVEL g_print_type;
extern enum ANT_TYPE g_ant_type;

#ifdef UARTNS550_EN
u8 tod_uart_init(u32 deviceID);
void uartns550_intr_handle(void *pvParameters);
#endif

#ifdef PPS_EN
u8 pps_intr_init(u32 intId);
void pps_intr_handler(void *pvParameters);
#endif

#ifdef PRINTF_TOD_TEST
void printf_tod_handler(void *pvParameters);
#endif

#ifdef TIMEOUT_EN
void pps_tod_timeout_handler(void *pvParameters);
#endif

#ifdef API_TEST
void api_test_handler(void *pvParameters);
#endif

#ifdef PPS_TIME_TEST
void analyze_pps_cycles(void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif 