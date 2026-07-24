/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_mailbox_example.h
 *
 * This file contains qspi lib
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   HZQ  12/20/2018  First Release
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_MAILBOX_EXAMPLE_H_
#define _FMSH_MAILBOX_EXAMPLE_H_

#include "fmsh_mailbox.h"
#include "fmsh_ipi.h"

/************************** Constant Definitions *****************************/
#define IPI_ID_APU0      PAR_PSU_IPI_0_DEVICE_ID
#define IPI_ID_RPU0      PAR_PSU_IPI_1_DEVICE_ID
#define IPI_ID_RPU1      PAR_PSU_IPI_2_DEVICE_ID
#define IPI_ID_PMU0      PAR_PSU_IPI_3_DEVICE_ID
#define IPI_ID_PMU1      PAR_PSU_IPI_4_DEVICE_ID
#define IPI_ID_PMU2      PAR_PSU_IPI_5_DEVICE_ID
#define IPI_ID_PMU3      PAR_PSU_IPI_6_DEVICE_ID
#define IPI_ID_APU1      PAR_PSU_IPI_7_DEVICE_ID
#define IPI_ID_APU2      PAR_PSU_IPI_8_DEVICE_ID
#define IPI_ID_APU3      PAR_PSU_IPI_9_DEVICE_ID
#define IPI_ID_PL0       PAR_PSU_IPI_10_DEVICE_ID

#define REMOTE_MASK_APU0 MAILBOX_IPI0
#define REMOTE_MASK_RPU0 MAILBOX_IPI1
#define REMOTE_MASK_RPU1 MAILBOX_IPI2
#define REMOTE_MASK_PMU0 MAILBOX_IPI3
#define REMOTE_MASK_PMU1 MAILBOX_IPI4
#define REMOTE_MASK_PMU2 MAILBOX_IPI5
#define REMOTE_MASK_PMU3 MAILBOX_IPI6
#define REMOTE_MASK_APU1 MAILBOX_IPI7
#define REMOTE_MASK_APU2 MAILBOX_IPI8
#define REMOTE_MASK_APU3 MAILBOX_IPI9
#define REMOTE_MASK_PL0  MAILBOX_IPI10

/**************************** Type Definitions *******************************/

#define TEST_LOCAL_CPUID IPI_ID_APU0
#define TEST_REMOTE_CPU  REMOTE_MASK_APU0
#define TEST_REMOTE_CPU1 REMOTE_MASK_RPU0

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
extern int Mailbox_Example(u8 deviceId);

#endif
