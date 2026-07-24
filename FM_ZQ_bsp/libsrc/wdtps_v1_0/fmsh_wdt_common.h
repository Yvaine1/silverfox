/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_wdt_common.h
 *
 * This file contains common type define of uart.
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

#ifndef _FMSH_WDT_COMMON_H_ /* prevent circular inclusions */
#define _FMSH_WDT_COMMON_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

typedef struct FWdtPs_Instance {
    FMSH_callback CallbacRef;
} FWdtPs_Instance_T;

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
 *  instance        device private data structure pointer
 *  compParam       pointer to structure containing device's
 *                  coreConsultant configuration parameters structure
 *  compVersion     device version identification number
 *  compType        device identification number
 */
typedef struct FWdtPs {
    void *base_address;
    FWdtPs_Instance_T instance;
    void *comp_param;
    uint32_t comp_version;
    uint32_t comp_type;
} FWdtPs_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
