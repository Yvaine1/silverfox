/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_can_lib.h
 *
 * This file contains header fmsh_can_private.h & fmsh_can_public.h
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

#ifndef _FMSH_I2C_COMMON_H_ /* prevent circular inclusions */
#define _FMSH_I2C_COMMON_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/
#define FMSH_APB_I2C 0x44570140
/**************************** Type Definitions *******************************/
/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u16 DeviceId;     /**< Unique ID  of device */
    u32 BaseAddress;  /**< Base address of device (IPIF) */
    u32 InputClockHz; /**< Input clock frequency */
} FI2cPs_Config;

typedef struct FI2cPs {
    const char *name;
    u32 id;
    void *base_address;
    u32 input_clock;
    void *instance;
    void *comp_param;
    uint32_t comp_version;
    u32 comp_type;
    FMSH_listHead list;
} FI2cPs_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
