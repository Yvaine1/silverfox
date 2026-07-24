/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_wdt_private.h
 *
 * This file contains private constant & function define of wdt.
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

#ifndef _FMSH_WDT_PRIVATE_H_ /* prevent circular inclusions */
#define _FMSH_WDT_PRIVATE_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_wdt_common.h"

/************************** Constant Definitions *****************************/

// #define WDT_IN32P FMSH_IN32_32
// #define WDT_OUT32P FMSH_OUT32_32

#define WDT_IN32P   FMSH_CAN_IN32_32
#define WDT_OUT32P  FMSH_CAN_OUT32_32

// WDT_CR register
#define bfoWDT_EN   0
#define bfwWDT_EN   1
#define bfoRMOD     1
#define bfwRMOD     1
#define bfoRPL      2
#define bfwRPL      3

// WDT_TOR register
#define bfoTOP      0
#define bfwTOP      4
#define bfoTOP_INIT 4
#define bfwTOP_INIT 4

/**************************** Type Definitions *******************************/

typedef struct FWdtPs_Portmap {
    volatile u32 cr;            // control register (0x00)
    volatile u32 torr;          // timeout range register (0x04)
    volatile u32 ccvr;          // current counter value register (0x08)
    volatile u32 ccr;           // counter restart register (0x0c)
    volatile u32 stat;          // interrupt status register (0x00)
    volatile u32 eio;           // interrupt clear register (0x04)
    volatile u32 comp_version;  // component vision  register (0x08)
} FWdtPs_Portmap_T;

/***************** Macros (Inline Functions) Definitions *********************/

#define FWDTPS_COMMON_ASSERT(p)                     \
    do                                              \
    {                                               \
        FMSH_ASSERT(p != NULL);                     \
        FMSH_ASSERT(p->base_address != NULL);       \
        FMSH_ASSERT(p->comp_version == 0x3130392a); \
        FMSH_ASSERT(p->comp_type == 0x44570120);    \
    } while (0)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static u32 FWdtPs_compVersion(FWdtPs_T *dev);
static u32 FWdtPs_compType(FWdtPs_T *dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
