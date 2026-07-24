/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_devc_common.h
 *
 * This file contains common type define
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   wfb  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_DEVC_COMMON_H_ /* prevent circular inclusions */
#define _FMSH_DEVC_COMMON_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u16 DeviceId;          /**< Unique ID  of device */
    uintptr_t BaseAddress; /**< Base address of device (IPIF) */
} FDevcPs_Config;

/**************************** Type Definitions *******************************/

/**
 * DESCRIPTION
 *  This is the primary structure used when dealing with all devices.
 *  It serves as a hardware abstraction layer for driver code and also
 *  allows this code to support more than one device of the same type
 *  simultaneously.  This structure needs to be initialized with
 *  meaningful values before a pointer to it is passed to a driver
 *  initialization function.
 * PARAMETERS
 *  baseAddress     physical base address of device
 */
typedef struct FDevcPs {
    u32 id;
    void *base_address;
    u8 scu_alg_flag;
    u8 scu_opkey_flag;
    u8 scu_srcaddr_flag;
    u8 scu_part_flag;
    u32 scu_part_size;
} FDevcPs_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
