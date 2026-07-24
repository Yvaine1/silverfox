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
 * @file fmsh_delay.c
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
#include "cortexr5/fmsh_pmu.h"
#include "fmsh_parameters.h"
#include "psu_init.h"

#ifdef CYCLECNT_GEN_DELAY
#define DELAYCNT_FREQ_MHZ ((float)RPU_FREQ / 64 / 1000000)
#else
#define DELAYCNT_FREQ_MHZ ((float)GTC_FREQ / 1000000)
#endif

void global_timer_enable (void)
{
#ifdef CYCLECNT_GEN_DELAY
    Fmsh_EnableCycleCounter();
    Fmsh_ResetCycleCounter();
#else
    *(uint32_t *)FPS_GTC_BASEADDR = 0x1;
#endif
}

void global_timer_disable ()
{
#ifdef CYCLECNT_GEN_DELAY
    Fmsh_DisableCycleCounter();
#else
    *(uint32_t *)FPS_GTC_BASEADDR = 0x0;
#endif
}

uint64_t get_current_time (void)
{
    uint64_t value = 0U;
    uint32_t value_l = 0U, value_h = 0U;

#ifdef CYCLECNT_GEN_DELAY
    value_h = 0U;
    Fmsh_GetCycleCounter(&value_l);
#else
    do
    {
        value_h = *(uint32_t *)(FPS_GTC_BASEADDR + 0xc);
        value_l = *(uint32_t *)(FPS_GTC_BASEADDR + 0x8);
    } while (*(uint32_t *)(FPS_GTC_BASEADDR + 0xc) != value_h);

#endif

    value = ((uint64_t)value_h << 32U) | value_l;

    return value;
}

__attribute__((unused)) static void gtc_count (uint64_t counts)
{
    uint64_t start, end;
    global_timer_enable();
    start = get_current_time();
    while (1)
    {
        end = get_current_time();
        if ((end - start) > counts)
        {
            break;
        }
    }
}

void delay_ms (uint32_t time_ms)
{
    uint64_t counts;

    counts = (uint64_t)(time_ms * 1000 * DELAYCNT_FREQ_MHZ);

#ifdef CYCLECNT_GEN_DELAY
    Fmsh_DelayCnt(counts);
#else
    gtc_count(counts);
#endif
}

void delay_us (uint32_t time_us)
{
    uint64_t counts;

    counts = (uint64_t)(time_us * DELAYCNT_FREQ_MHZ);

#ifdef CYCLECNT_GEN_DELAY
    Fmsh_DelayCnt(counts);
#else
    gtc_count(counts);
#endif
}

void delay_ns (uint32_t time_ns)
{
    int i, counts;

    counts = (uint64_t)(time_ns * (DELAYCNT_FREQ_MHZ / 1000 / 8));
    for (i = 0; i < counts; i++){};
}

void delay_1ms (void)
{
    uint64_t counts;

    counts = (uint64_t)(1000 * DELAYCNT_FREQ_MHZ);

#ifdef CYCLECNT_GEN_DELAY
    Fmsh_DelayCnt(counts);
#else
    gtc_count(counts);
#endif
}

void delay_1us (void)
{
    uint64_t counts;

    counts = (uint64_t)(DELAYCNT_FREQ_MHZ);

#ifdef CYCLECNT_GEN_DELAY
    Fmsh_DelayCnt(counts);
#else
    gtc_count(counts);
#endif
}
