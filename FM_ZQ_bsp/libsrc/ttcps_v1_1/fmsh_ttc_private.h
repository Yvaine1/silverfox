/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_ttc_private.h
 *
 * This file contains private constant & function define of ttc.
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

#ifndef _FMSH_TTC_PRIVATE_H_ /* prevent circular inclusions */
#define _FMSH_TTC_PRIVATE_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/

#include "fmsh_ttc_common.h"

/************************** Constant Definitions *****************************/

/* This macro is used to hardcode the APB data accesses */
#define TIMER_IN32P(v)                               (v)
#define TIMER_OUT32P(ret, v)                         (v) = (ret)

// TimerXControlReg register
#define bfoTIMER_TimerNControlReg_TimerEnable        0
#define bfwTIMER_TimerNControlReg_TimerEnable        1
#define bfoTIMER_TimerNControlReg_TimerMode          1
#define bfwTIMER_TimerNControlReg_TimerMode          1
#define bfoTIMER_TimerNControlReg_TimerInterruptMask 2
#define bfwTIMER_TimerNControlReg_TimerInterruptMask 1
#define bfoTIMER_TimerNControlReg_TIMERPWM           3
#define bfwTIMER_TimerNControlReg_TIMERPWM           1

/**************************** Type Definitions *******************************/

/* This is the structure used for accessing the gpio memory map. */
typedef struct FTtcPs_Portmap {
    volatile u32 Timer1LoadCount;      // 0x00
    volatile u32 Timer1CurrentValue;   // 0x04
    volatile u32 Timer1ControlReg;     // 0x08
    volatile u32 Timer1EOI;            // 0x0c
    volatile u32 Timer1IntStatus;      // 0x10

    volatile u32 Timer2LoadCount;      // 0x14
    volatile u32 Timer2CurrentValue;   // 0x18
    volatile u32 Timer2ControlReg;     // 0x1c
    volatile u32 Timer2EOI;            // 0x20
    volatile u32 Timer2IntStatus;      // 0x24

    volatile u32 Timer3LoadCount;      // 0x28
    volatile u32 Timer3CurrentValue;   // 0x2c
    volatile u32 Timer3ControlReg;     // 0x30
    volatile u32 Timer3EOI;            // 0x34
    volatile u32 Timer3IntStatus;      // 0x38
    volatile u32 rsv0[25];             // 0x3c-0x9c
    volatile u32 TIMERSIntStatus;      // 0xa0
    volatile u32 TIMERSEOI;            // 0xa4
    volatile u32 TIMERSRawIntStatus;   // 0xa8
    volatile u32 TIMERS_COMP_VERSION;  // 0xac

    volatile u32 Timer1LoadCount2;     // 0xb0
    volatile u32 Timer2LoadCount2;     // 0xb4
    volatile u32 Timer3LoadCount2;     // 0xb8
} FTtcPs_Portmap_T;

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * DESCRIPTION
 *  These are the common preconditions which must be met for all driver
 *  functions.  Primarily, they check that a function has been passed
 *  a legitimate device structure.
 */
#define FTTCPS_COMMON_ASSERT(p)               \
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
