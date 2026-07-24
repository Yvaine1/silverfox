/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_ttc_common.h
 *
 * This file contains common type define of ttc.
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

#ifndef _FMSH_TTC_COMMON_H_ /* prevent circular inclusions */
#define _FMSH_TTC_COMMON_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* This structure contains variables which relate to each individual can
 * instance */
typedef struct FTtcPs_Instance {
    FMSH_callback timer1Callback;
    FMSH_callback timer2Callback;
    FMSH_callback timer3Callback;
    u32 ID; /*0 - TIMER0, 1 - TIMER1*/
    u8 intrStatus;
} FTtcPs_Instance_T;

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
 *  compVersion     device version identification number
 */
typedef struct FTtcPs {
    void *base_address;
    FTtcPs_Instance_T instance;
    uint32_t comp_version;
    uint32_t id;
    uint32_t IntId[3];
} FTtcPs_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
