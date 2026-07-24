/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gpio_common.h
 *
 * This file contains common type define of gpio
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

#ifndef _FMSH_GPIO_COMMON_H_ /* prevent circular inclusions */
#define _FMSH_GPIO_COMMON_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_common.h"
#include "fmsh_psu_parameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* This structure contains variables which relate to each individual can
 * instance */
typedef struct FGpioPs_Instance {
    void *txCallbacRef;  // user Tx callback function
    void *rxCallbacRef;  // user Rx callback function
} FGpioPs_Instance_T;

/**
 * PARAMETERS
 *  baseAddress     physical base address of device
 *  instance        device private data structure pointer
 *  compVersion     device version identification number
 *  compType        device identification number
 *  max_pin         maximum number of pins that a bank can control
 */
typedef struct FGpioPs {
    void *base_address;
    FGpioPs_Instance_T instance;
    uint32_t comp_version;
    uint32_t comp_type;
    uint32_t max_pin;
} FGpioPs_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
