#ifndef MEMORY_TO_BIN_H
#define MEMORY_TO_BIN_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ff.h"
#include "fmsh_common.h"
#include "fmsh_fatfs_example.h" 


#define FPGA_BASEADDR     0x80000000
#define DDR_CAP_BASE_ADDR         0x940000000ULL  // DDR start address (64-bit)
#define DDR_CAP_MAX_SIZE   (750 * 1024 * 1024)  //(750MB)
#define DDR_CAP_END_ADDR         0x96EDFFFFFULL
#define FRAME_SIZE         (1*80*240*1024)
#define MAX_FRAME_NUM      (DDR_CAP_MAX_SIZE / FRAME_SIZE)  // 最大帧数：40帧
#define FPGA_FRAME_NUM_REG           0x230
#define FPGA_DDR_ADDR_HIGH_REG       0x228
#define FPGA_DDR_ADDR_LOW_REG        0x22C
#define FPGA_DDR_ADDR_CLK_NUM_REG    0x400
#define ALIGNMENT_SIZE    4  
#define DDR_4BYTE_ALIGN_MASK  0x3ULL

typedef enum
{
    FRAME_NUMBER_LT_40 = 0,
    FRAME_NUMBER_GT_40,
}FRAME_NUMBER_LIST;


typedef enum
{
    GRANULARITY_SLOT = 0,
    GRANULARITY_FRAME,
}GRANULARITY_LIST;

typedef enum
{
    DIR_FORWARD = 0,
    DIR_BACKWARD,
}DIR_LIST;


void mem2bin(u8 granularity, u8 direction, u32 start_index, u8 frame_count);

#endif  // MEMORY_TO_BIN_H