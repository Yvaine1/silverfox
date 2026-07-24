#ifndef MEMORY_TO_BIN_H
#define MEMORY_TO_BIN_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ff.h"
#include "fmsh_common.h"
#include "fmsh_fatfs_example.h" 

#define BASE_ADDR         0x800000000ULL
#define TOTAL_SIZE_MB     750
#define BATCH_SIZE_BYTES  ((uint64_t)TOTAL_SIZE_MB * 1024 * 1024)
#define FILE_PATH         "0:/samplingData.bin"
#define CHUNK_SIZE        (25 * 1024 * 1024)

void mem2bin();

#endif  // MEMORY_TO_BIN_H
