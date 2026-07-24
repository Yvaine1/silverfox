/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    dg_utils.c
 * @brief   This head file provide UIO fuction, which include FPGA registers writing/reading and PL-PS interrupt. 
 *          The shared libarary should contstuct the UIO data form kernel in constructior.  
 */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

/*
#include <unistd.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include "dg_utils_local_api.h"
#include "dg_utils_unique_api.h"
*/
#include "dg_utils.h"

#define UIO_MAX_NAME_SIZE 32
#define UIO_MAX_COUNT 255
#define UIO_IRQ_MAX_NUM 255

extern int errno ;

typedef struct {
  INT32 fd;
  CHAR name[UIO_MAX_NAME_SIZE];  
  void*  vaddr;    //virtual address
  size_t  paddr;    //physical address
  size_t  size;     //size
} UIO_MEM_MAP;

typedef struct
{
    INT32   fd;
    CHAR    uioName[32];      // uio name
    UINT32   irq;              // IRQ number
    UINT32    irqEnableFlag;    //IRQ enable flag
    isr_callback pisr_entry;  //irq call back function
}UIO_IRQ_MAP;


UIO_MEM_MAP uio_mem_map[UIO_MAX_COUNT];
UIO_IRQ_MAP uio_irq_map[UIO_MAX_COUNT];
UINT32      uio_cnt        = 0;
UINT32      uio_mem_cnt    = 0;
UINT32      uio_int_cnt    = 0;
UINT32      fpga_mem_id=0xffffffff;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*
static INT32 get_uio_number(void);
static STATUS uio_intr_init_descr(INT32 id, INT32 intr_count);
static STATUS uio_mem_init_descr(INT32 id, INT32 mem_count);
static STATUS uio_init_descr(INT32 id);
static STATUS line_from_file(CHAR *filename, CHAR *linebuf);
static INT32 check_uio_type(INT32 id);
static STATUS uio_get_name_from_id(INT32 id, INT32 mem_count);
static STATUS uio_get_mem_size_from_id(INT32 id, INT32 mem_count);
static STATUS uio_get_mem_addr_from_id(INT32 id, INT32 mem_count);
static STATUS uio_mem_index_search(CHAR *str, UINT32 *index);
static STATUS uio_get_irqnum_from_name(CHAR *irq_name, UINT32 *irq_num);
*/

static INT32 dg_utils_semid = -1;

/**
  * @brief  Read the 32bit value from a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  dat_out: The pointer to sotre the data read from FPGA 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS read_fpga_reg(UINT32 addr, UINT32 *pdat_out)
{
    void* base_addr;

    dg_utils_lock(0);
    if(fpga_mem_id == 0xffffffff)
    {
        utils_err("No FPGA UIO\n");
        dg_utils_unlock(0);
        return ERROR;
    }

    if(addr >= uio_mem_map[fpga_mem_id].size)
    {
        utils_err("Address out of range\n");
        dg_utils_unlock(0);
        return ERROR;
    }

    if (addr % 4 != 0)
    {
        utils_err("Address error\n");
        return ERROR;
    }

    base_addr = uio_mem_map[fpga_mem_id].vaddr;
    *pdat_out = *((volatile UINT32*)(base_addr + addr));
    dg_utils_unlock(0);
    utils_debug("read_fpga_reg:base 0x%p, addr 0x%x, data 0x%x\n", base_addr, addr,*pdat_out);
    return  OK;
}

/**
  * @brief  Write the 32bit value to a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  dat_in: The value wirte to FPGA 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS write_fpga_reg(UINT32 addr, UINT32 dat_in)
{
    void* base_addr;
    dg_utils_lock(0);
    if(fpga_mem_id == 0xffffffff)
    {
        utils_err("No FPGA UIO\n");
        dg_utils_unlock(0);
        return ERROR;
    }

    if(addr >= uio_mem_map[fpga_mem_id].size)
    {
        utils_err("Address out of range\n");
        dg_utils_unlock(0);
        return ERROR;
    }

    if (addr % 4 != 0)
    {
        utils_err("Address error\n");
        return ERROR;
    }

    base_addr = uio_mem_map[fpga_mem_id].vaddr;
    *((volatile UINT32*)(base_addr + addr)) = dat_in;
    dg_utils_unlock(0);
    utils_debug("write_fpga_reg:addr 0x%x,dat=0x%x\n",addr,dat_in);
    return  OK;
}

/**
  * @brief  Read the a bit value from a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  bit_num: the bit index of the FPGA register 
  * @param  pbit_value: The pointer to sotre the data read from FPGA 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS read_fpga_reg_bit(UINT32 addr, UINT32 bit_num, BitAction *pbit_value)
{
    UINT32 rdvalue;
    STATUS ret;
    if( addr >= uio_mem_map[fpga_mem_id].size)
    {
        utils_err("FPGA address(0x%x) out of range. The max addr is %zu.\n", addr, uio_mem_map[fpga_mem_id].size); 
        return ERROR;
    }  

    if(bit_num > 31)
    {
        utils_err("Bit index out of range\n"); 
        return ERROR;
    }
    ret = read_fpga_reg(addr, &rdvalue);
    *pbit_value = (rdvalue & (((UINT32)1)<<bit_num))?BIT_SET:BIT_RESET;
    
    return ret;
}

/**
  * @brief  write the a bit value to a FPGA register 
  * @param  addr: FPGA register address in 32bit 
  * @param  bit_num: the bit index of the FPGA register 
  * @param  bit_value: the bit value to write. 
  * @retval Indictate the function execution status(OK or ERROR)
  */
STATUS write_fpga_reg_bit(UINT32 addr, UINT32 bit_num, BitAction bit_value)
{
    UINT32 rdvalue;
    STATUS ret;
    if( addr >= uio_mem_map[fpga_mem_id].size)
    {
        utils_err("FPGA address(0x%x) out of range. The max addr is %zu.\n", addr, uio_mem_map[fpga_mem_id].size);  
        return ERROR;
    }  

    if(bit_num > 31)
    {
        utils_err("Bit index out of range\n"); 
        return ERROR;
    }
    ret = read_fpga_reg(addr, &rdvalue);
    if(ret == ERROR)
    {
        utils_err("FPGA read failed\n"); 
        return ERROR;
    }
    if(bit_value == BIT_SET)
        rdvalue |= ((UINT32) 1) << bit_num;
    else
    {
        rdvalue &= ~(((UINT32) 1) << bit_num);
    }

    return write_fpga_reg(addr, rdvalue);;
}

//dg_util lock encapsulation
INT32 dg_utils_lock(INT32 index)
{
   return get_lock(dg_utils_semid, index);    
}

INT32 dg_utils_unlock(INT32 index)
{
    return release_lock(dg_utils_semid, index);
}

UINT32 get_hwid(void)
{
    UINT32 hwid = 0;
    #ifdef GPIO_HWID_0_INDEX
        hwid |= get_gpio_data(0, GPIO_HWID_0_INDEX);
        utils_debug("hwid0:%x",hwid);
        #ifdef GPIO_HWID_1_INDEX
            hwid |= (get_gpio_data(0, GPIO_HWID_1_INDEX) << 1);
            utils_debug("hwid1:%x",hwid);
            #ifdef GPIO_HWID_2_INDEX
                hwid  |= (get_gpio_data(0, GPIO_HWID_2_INDEX) << 2);
                utils_debug("hwid2:%x",hwid);
                #ifdef GPIO_HWID_3_INDEX
                    hwid  |= (get_gpio_data(0, GPIO_HWID_3_INDEX) << 3);
                    utils_debug("hwid3:%x",hwid);
                    #ifdef GPIO_HWID_4_INDEX
                        hwid |= (get_gpio_data(0, GPIO_HWID_4_INDEX) << 4);
                        utils_debug("hwid4:%x",hwid);
                        #ifdef GPIO_HWID_5_INDEX
                            hwid |= (get_gpio_data(0, GPIO_HWID_5_INDEX) << 5);
                            utils_debug("hwid5:%x",hwid);
                                #ifdef GPIO_HWID_6_INDEX
                                hwid  |= (get_gpio_data(0, GPIO_HWID_6_INDEX) << 6);
                                utils_debug("hwid6:%x",hwid);
                                    #ifdef GPIO_HWID_7_INDEX
                                    hwid  |= (get_gpio_data(0, GPIO_HWID_7_INDEX) << 7);
                                    utils_debug("hwid7:%x",hwid);
                                    #endif
                                #endif
                        #endif
                    #endif
                #endif
            #endif
        #endif
    #endif
    utils_debug("hwid:0x%x",hwid);
    return hwid;
}