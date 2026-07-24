/******************************************************************************
 *
 * Copyright (C) 2009 - 2023 FMSH, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a FMSH device, or
 * (b) that interact with a FMSH device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file fmsh_gtc_delay.c
 *
 * This file contains the delay impl for the processor
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------- -------- ---------------------------------------------------
 * 1.00  hzq     22/12/02 Initial version
 *
 * </pre>
 *
 * @note
 *
 * None.
 *
 ******************************************************************************/
#include <stdint.h>

#include "bspconfig.h"
#include "fmsh_psu_parameters.h"
#include "psu_init.h"

#define GTC_CLK_FREQ     (GTC_FREQ)
#define GTC_CLK_FREQ_MHZ (GTC_CLK_FREQ / 1000000)
#define GTC_BASE         FPS_GTC_BASEADDR
#define GTC_BASE_NS      FPS_IOU_SCNTR_BASEADDR

/*
 * This function must call with secure mode.
 */
__weak void global_timer_enable (void)
{
    volatile uint32_t *p_ctrl = (volatile uint32_t *)(GTC_BASE + 0x0);
    if (*p_ctrl & 0x01)
    {
        return;
    }

    *p_ctrl |= 0x01;
}

__weak uint64_t get_current_time (void)
{
    uint64_t value = 0;
    uint32_t value_l, value_h;

    // volatile uint32_t *p_value_l = (volatile uint32_t *)(GTC_BASE + 0x8);
    // volatile uint32_t *p_value_h = (volatile uint32_t *)(GTC_BASE + 0xc);

    volatile uint32_t *p_value_l = (volatile uint32_t *)(GTC_BASE_NS + 0x0);
    volatile uint32_t *p_value_h = (volatile uint32_t *)(GTC_BASE_NS + 0x4);

    // gtc_enable();

    do
    {
        value_h = *p_value_h;
        value_l = *p_value_l;
    } while (value_h != *p_value_h);

    value = ((uint64_t)value_h << 32) | value_l;

    return value;
}

__weak void delay_ms (uint32_t time_ms)
{
    uint64_t counts;
    uint64_t cur_count;

    cur_count = get_current_time();
    counts = time_ms * 1000 * GTC_CLK_FREQ_MHZ + cur_count;

    while (get_current_time() < counts){};
}

__weak void delay_us (uint32_t time_us)
{
    uint64_t counts;
    uint64_t cur_count;

    cur_count = get_current_time();
    counts = time_us * GTC_CLK_FREQ_MHZ + cur_count;

    while (get_current_time() < counts){};
}

__weak void delay_1ms () { delay_ms(1); }

__weak void delay_1us () { delay_us(1); }

__weak void delay_ns (uint32_t time_ns)
{
    int i, counts;

    counts = time_ns * GTC_CLK_FREQ_MHZ;
    for (i = 0; i < counts; i++){};
}
