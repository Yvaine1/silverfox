/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common_delay.h
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 *
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_COMMON_DELAY_H_
#define _FMSH_COMMON_DELAY_H_

#include "fmsh_common_types.h"

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void global_timer_enable(void);
u64 get_current_time(void);
void delay_ms(uint32_t time_ms);
void delay_us(uint32_t time_us);
void delay_ns(uint32_t time_ns);
void delay_1ms(void);
void delay_1us(void);

#endif /* #ifndef _FMSH_COMMON_DELAY_H_ */
