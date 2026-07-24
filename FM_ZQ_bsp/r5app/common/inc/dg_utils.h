/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    dg_utils.h
 * @brief   This head file provide UIO fuction, which include FPGA registers writing/reading and PL-PS interrupt. 
 *          The shared library should contstuct the UIO data form kernel in constructor.
 */


#ifndef _DG_UTILS_H_
#define _DG_UTILS_H_

/* Includes ------------------------------------------------------------------*/
#include "dg_common.h"

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  Bit SET and Bit RESET enumeration 
  */

typedef enum
{
  BIT_RESET = 0,
  BIT_SET
} BitAction;

typedef enum
{
    MODULE_BOARD_DRIVER_COMMON = 0,
    MODULE_DGUTILS,
    MODULE_FLASH,
    MODULE_EEPROM,
    MODULE_LM75,
    MODULE_MP5023,
    MODULE_INA226,
    MODULE_MPL3115A2,
    MODULE_SFP,
    MODULE_SIC45X,
    MODULE_SWITCH_KSZ9477,
    MODULE_AS5003,
    MODULE_FANCONTROL,
    MODULE_RFCHAINSWITCH,
    MODULE_IRQCONTROL,
    MODULE_PA_RS485,
    MODULE_AISG_RS485,
    MODULE_RS422,
    MODULE_MODEM,
    MODULE_RS485,
    MODULE_WIFI,
    MODULE_HANRU,
    MODULE_AD9362,
    MODULE_AD9528,
    MODULE_ADRV9025,
    MODULE_DAC80501,
    MODULE_AMC7834,
    MODULE_CAT25256,
    MODULE_ERC866X,
    MODULE_ECT8676,
    MODULE_IDT,
    MODULE_LMX2572,
    MODULE_PA_SPI,
    MODULE_SI55XX,
    MODULE_WATCHDOG,
    MODULE_BOARD_DRIVER_UNIQUE,
    MODULE_BATTERY,
    MODULE_TOD,
    MODULE_TTC,
    MODULE_RESHAPE_MAN,
    MODULE_SHARED_MEM,
    MODULE_CX8242K,
    MODULE_AU5508,
    MODULE_EXT_FPGA,
    MODULE_RFDC,
    MODULE_SYSMON,
    MODULE_XN406,
    MODULE_QSFP,
    MODULE_DPD,
    MODULE_NETWORK,
    MODULE_ATGM332D,
    MODULE_POWER_CALIBRATION,
    MODULE_LTB20,
    MODULE_DEVICE_INFO,
    MODULE_CX6242Q,
    MODULE_CX7442,
    MODULE_CX4E04,
    MODULE_CX9261A,
    MODULE_ARW621A,
    MODULE_SIM_MDIO,
    MODULE_GBQ6600,
    MODULE_NETWORK_CONTROL_G3,
    MODULE_ADF4368,
    MODULE_LMX2594,
    MODULE_MAX_NUM
}MODULE_E;

typedef struct
{
    INT32 semid;
    char lib_name[50];
}sem_info_s;

#define utils_debug(fmt, ...) {pf_detail(MODULE_DGUTILS,fmt, ##__VA_ARGS__);}
#define utils_err(fmt, ...) {pf_err(MODULE_DGUTILS,fmt, ##__VA_ARGS__);}

#define GPIO_DIR_IN                   0
#define GPIO_DIR_OUT                  1

#define GPIO_DAT_OFF                   0
#define GPIO_DAT_ON                    1

/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/

#define GPIO_INIT_COMPLETE 123

typedef enum
{
  DG_UTILS_SHM_ID_GPIO_INIT = 10
} DG_UTILS_SHM_ID_MAP;

/* Exported functions --------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  Read the 32bit value from a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  dat_out: The pointer to sotre the data read from FPGA 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS read_fpga_reg(UINT32 addr, UINT32 *pdat_out);

/**
  * @brief  Write the 32bit value to a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  dat_in: The value wirte to FPGA 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS write_fpga_reg(UINT32 addr, UINT32 dat_in);

/**
  * @brief  Read the a bit value from a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  bit_num: the bit index of the FPGA register 
  * @param  pbit_value: The pointer to sotre the data read from FPGA 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS read_fpga_reg_bit(UINT32 addr, UINT32 bit_num, BitAction *pbit_value);

/**
  * @brief  write the a bit value to a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  bit_num: the bit index of the FPGA register 
  * @param  bit_value: the bit value to write. 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS write_fpga_reg_bit(UINT32 addr, UINT32 bit_num, BitAction bit_value);

/**
  * @brief  Set the interrupt service routine callback function 
  * @param  irq_num: The interrupt number 
  * @param  pisr_entry: The address of the callback function 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS set_irq_callback(UINT32 irq_num, isr_callback pisr_entry);

/**
  * @brief  Enable the specified interrupt  
  * @param  irq_num: The interrupt number to enable 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS enable_irq(UINT32 irq_num);

/**
  * @brief  Disable the specified interrupt  
  * @param  irq_num: The interrupt number to disable 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS disable_irq(UINT32 irq_num);


INT32 set_gpio_dir(UINT8 gpio_num, UINT16 pin_num, UINT8 operation);
INT8 get_gpio_dir(UINT8 gpio_num, UINT16 pin_num);
INT32 set_gpio_data(UINT8 gpio_num, UINT16 pin_num, UINT8 operation);
INT8 get_gpio_data(UINT8 gpio_num, UINT16 pin_num);
INT8 export_gpio(UINT8 gpio_num, UINT16 pin_num);
INT8 unexport_gpio(UINT8 gpio_num, UINT16 pin_num);
INT8 init_gpio_file_fd();
UINT8 gpio_initialize(UINT8 gpio_num, UINT16 pin_num);

// log对外日志接口
typedef enum PF_LOG_TYPE
{
    PF_LOG_NONE = 0,
    PF_LOG_FATAL ,
    PF_LOG_ERROR,
    PF_LOG_WARING,
    PF_LOG_NOTICE,
    PF_LOG_INFO,
    PF_LOG_DETAIL
} pf_log_level;

void pf_print_set_file_name(const CHAR *name);

void pf_print_set_function_name(const CHAR *tag);

void pf_print_set_debug_flag(INT32 value);

void pf_print_log(const CHAR* tmp_file_name,const CHAR* tmp_func_name, UINT32 line_num, UINT32 level, UINT8 module, CHAR const *format, ...);

void mw_set_platform_trace_cb(void (*cb)(const CHAR* file_name,const CHAR* func_name, unsigned int line_num, unsigned int level, const CHAR *msg));

void mw_set_log_level(MODULE_E module, INT32 level);

void mw_set_log_callback_log_switch(INT32 value);

#define pf_fatal(module,x...)   pf_print_log(__FILE__,__func__, __LINE__, PF_LOG_FATAL, module, x)
#define pf_err(module,x...)     pf_print_log(__FILE__,__func__, __LINE__, PF_LOG_ERROR, module, x)
#define pf_warning(module,x...) pf_print_log(__FILE__,__func__, __LINE__, PF_LOG_WARING, module, x)
#define pf_notice(module,x...)  pf_print_log(__FILE__,__func__, __LINE__, PF_LOG_NOTICE, module, x)
#define pf_info(module,x...)    pf_print_log(__FILE__,__func__, __LINE__, PF_LOG_INFO, module, x)
#define pf_detail(module,x...)   pf_print_log(__FILE__,__func__, __LINE__, PF_LOG_DETAIL, module, x)

INT32 create_lock(INT32 num, const CHAR *libname);
INT32 destroy_lock(INT32 semid, INT32 num);
INT32 get_lock(INT32 semid, INT32 index);
INT32 release_lock(INT32 semid, INT32 index);
UINT32 get_hwid(void);

/**
  * @brief  Get the full path where so is located
  * @param  libname: so name
  * @retval pointer contain path，Note：The returned pointer space needs to be freed by the caller
  */
CHAR* get_so_path(const CHAR *libname);

INT32 dg_utils_lock(INT32 index);    
INT32 dg_utils_unlock(INT32 index);
INT32 get_sem_info(sem_info_s *sem_info);

#ifdef __cplusplus
}
#endif


#endif /*_DG_UIO_H_*/
