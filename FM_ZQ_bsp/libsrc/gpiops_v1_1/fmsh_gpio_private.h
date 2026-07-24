/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file fmsh_gpio_private.h
 *
 * This file contains private constant & function define of gpio
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

#ifndef _FMSH_GPIO_PRIVATE_H_ /* prevent circular inclusions */
#define _FMSH_GPIO_PRIVATE_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_gpio_common.h"

/************************** Constant Definitions *****************************/

/* This macro is used to hardcode the APB data accesses */
#define GPIO_IN32P(var)           (var)
#define GPIO_OUT32P(data, var)    (var = data)

#define bfoGPIO_LS_SYNC_SYNCLEVEL ((u32)0)
#define bfwGPIO_LS_SYNC_SYNCLEVEL ((u32)1)

/**************************** Type Definitions *******************************/

/* This is the structure used for accessing the gpio memory map. */
typedef struct FGpioPs_Portmap {
    volatile u32 swport_dr;      // 0x00
    volatile u32 swport_ddr;     // 0x04
    volatile u32 rsv0[10];
    volatile u32 inten;          // 0x30
    volatile u32 intmask;        // 0x34
    volatile u32 inttype_level;  // 0x38
    volatile u32 int_polarity;   // 0x3c
    volatile u32 intstatus;      // 0x40
    volatile u32 raw_intstatus;  // 0x44
    volatile u32 debounce;       // 0x48
    volatile u32 port_eoi;       // 0x4c
    volatile u32 ext_port;       // 0x50
    volatile u32 rsv1[3];
    volatile u32 ls_sync;        // 0x60
    volatile u32 id_code;        // 0x64
    volatile u32 int_bothedge;   // 0x68
    volatile u32 ver_id_code;    // 0x6c
    volatile u32 config_reg2;    // 0x74
    volatile u32 config_reg1;    // 0x70
} FGpioPs_Portmap_T;

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * DESCRIPTION
 *  These are the common preconditions which must be met for all driver
 *  functions.  Primarily, they check that a function has been passed
 *  a legitimate device structure.
 */
#define FGPIOPS_COMMON_ASSERT(p)              \
    do                                        \
    {                                         \
        FMSH_ASSERT(p != NULL);               \
        FMSH_ASSERT(p->base_address != NULL); \
    } while (0)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
