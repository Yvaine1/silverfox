
#ifndef __MEM_COMMON_H__
#define __MEM_COMMON_H__



#include <stdio.h>
#include <string.h>
#include "fmsh_print.h"

#define IPI_ID_CH0                 PAR_PSU_IPI_0_DEVICE_ID
#define IPI_ID_CH1                 PAR_PSU_IPI_1_DEVICE_ID
#define IPI_ID_CH2                 PAR_PSU_IPI_2_DEVICE_ID
#define IPI_ID_PMU0                PAR_PSU_IPI_3_DEVICE_ID
#define IPI_ID_PMU1                PAR_PSU_IPI_4_DEVICE_ID
#define IPI_ID_PMU2                PAR_PSU_IPI_5_DEVICE_ID
#define IPI_ID_PMU3                PAR_PSU_IPI_6_DEVICE_ID
#define IPI_ID_CH7                 PAR_PSU_IPI_7_DEVICE_ID
#define IPI_ID_CH8                 PAR_PSU_IPI_8_DEVICE_ID
#define IPI_ID_CH9                 PAR_PSU_IPI_9_DEVICE_ID
#define IPI_ID_CH10                PAR_PSU_IPI_10_DEVICE_ID

#define REMOTE_MASK_CH0            MAILBOX_IPI0
#define REMOTE_MASK_CH1            MAILBOX_IPI1
#define REMOTE_MASK_CH2            MAILBOX_IPI2
#define REMOTE_MASK_PMU0           MAILBOX_IPI3
#define REMOTE_MASK_PMU1           MAILBOX_IPI4
#define REMOTE_MASK_PMU2           MAILBOX_IPI5
#define REMOTE_MASK_PMU3           MAILBOX_IPI6
#define REMOTE_MASK_CH7            MAILBOX_IPI7
#define REMOTE_MASK_CH8            MAILBOX_IPI8
#define REMOTE_MASK_CH9            MAILBOX_IPI9
#define REMOTE_MASK_CH10           MAILBOX_IPI10

#define SHMEM_ADDR_A53_TO_CR5_0             0x3ed80000
#define SHMEM_ADDR_CR5_0_TO_A53             0x3ed90000

#define SHMEM_ADDR_A53_TO_CR5_1             0x3edA0000
#define SHMEM_ADDR_CR5_1_TO_A53             0x3edB0000

#define SHMEM_ADDR_CR5_0_TO_1               0x3edC0000
#define SHMEM_ADDR_CR5_1_TO_0               0x3edD0000

#define SHMEM_SIZE                          0x10000
#define TOTAL_SHMEM_SIZE                    0x60000

















#endif