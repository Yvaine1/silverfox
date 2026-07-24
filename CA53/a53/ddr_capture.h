#ifndef MEMORY_TO_BIN_H
#define MEMORY_TO_BIN_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ff.h"
#include "fmsh_common.h"
#include "fmsh_fatfs_example.h" 

#define FPGA_CLKREG       0x224
#define FPGA_BASEADDR     0xA0000000
#define BASE_ADDR         0x800000000ULL  // DDR start address (64-bit)
#define FILE_PATH         "0:/samplingData.bin"
#define CHUNK_SIZE        (25 * 1024 * 1024)  // Chunk size for writing (25MB)

void ddr_capture();

#endif  // MEMORY_TO_BIN_H