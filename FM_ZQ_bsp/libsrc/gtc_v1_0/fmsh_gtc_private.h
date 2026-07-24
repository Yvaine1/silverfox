/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gtc_private.h
 *
 * This file contains private constant & function define of gtc
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

#ifndef _FMSH_GTC_PRIVATE_H_ /* prevent circular inclusions */
#define _FMSH_GTC_PRIVATE_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_gtc_common.h"

/************************** Constant Definitions *****************************/

/* This macro is used to hardcode the APB data accesses */
// #define GTC_IN32P FMSH_IN32_32
// #define GTC_OUT32P FMSH_OUT32_32
#define GTC_IN32P     FMSH_CAN_IN32_32
#define GTC_OUT32P    FMSH_CAN_OUT32_32

#define CNT_ID_OFFSET 0xFFC

/**************************** Type Definitions *******************************/

/* This is the structure used for accessing the gpio memory map. */
typedef struct FGtcPs_portmap {
    volatile u32 CNTCR;    // 0x00
    volatile u32 CNTSR;    // 0x04
    volatile u32 CNTCV_L;  // 0x08
    volatile u32 CNTCV_H;  // 0x0c
} FGtcPs_portmap_T;

/* This is the structure used for accessing the gpio memory map. */
typedef struct FGtcPs_ns_portmap {
    volatile u32 CNTCV_L;  // 0x00
    volatile u32 CNTCV_H;  // 0x04
} FGtcPs_ns_portmap_T;
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
