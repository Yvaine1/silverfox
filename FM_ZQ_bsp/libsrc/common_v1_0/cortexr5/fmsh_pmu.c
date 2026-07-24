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
 * @file fmsh_pmu.c
 *
 * This file contains the cortex-r5 pmu functions for the processor
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

#include "cortexr5/cortexr5.h"
#include "fmsh_pseudo_asm.h"

/****************************************************************************/
/**
 * @brief    Disable cycle counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_DisableCycleCounter () { mtcp(CP15_COUNT_ENABLE_CLR, 0x80000000); }

/****************************************************************************/
/**
 * @brief    Enable cycle counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_EnableCycleCounter () { mtcp(CP15_COUNT_ENABLE_SET, 0x80000000); }

/****************************************************************************/
/**
 * @brief    Reset cycle counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_ResetCycleCounter ()
{
    uint32_t value;

    mfcp(CP15_PERF_MONITOR_CTRL, value);
    value |= PMU_CYCLE_CNT_RESET;
    mtcp(CP15_PERF_MONITOR_CTRL, value);
}

/****************************************************************************/
/**
 * @brief    Read cycle counter value.
 *
 * @param	value points to cycle counter value.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_GetCycleCounter (uint32_t *value)
{
    uint32_t temp;

    mfcp(CP15_PERF_CYCLE_COUNTER, temp);
    *value = temp;
}

/****************************************************************************/
/**
 * @brief    Disable all performance event counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_DisablePerfCounter () { mtcp(CP15_COUNT_ENABLE_CLR, 0x7); }

/****************************************************************************/
/**
 * @brief    Enable all performance event counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_EnableEventCounter () { mtcp(CP15_COUNT_ENABLE_SET, 0x7); }

/****************************************************************************/
/**
 * @brief    Reset all performance event counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_ResetPerfCounter ()
{
    uint32_t value;

    mfcp(CP15_PERF_MONITOR_CTRL, value);
    value |= PMU_EVENT_CNT_RESET;
    mtcp(CP15_PERF_MONITOR_CTRL, value);
}

/****************************************************************************/
/**
 * @brief    Disable and Read all performance event counter.
 *
 * @param	counters points to all counter value.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_GetPerfCounter (uint32_t *counters)
{
    int i;
    uint32_t value;

    Fmsh_DisablePerfCounter();

    for (i = 0; i < 3; i++)
    {
        mtcp(CP15_EVENT_CNTR_SEL, i);
        mfcp(CP15_PERF_MONITOR_COUNT, value);
        counters[i] = value;
    }
}

/****************************************************************************/
/**
 * @brief    Set event group and start performance event counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_SetEvents (uint32_t *events)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        mtcp(CP15_EVENT_CNTR_SEL, i);
        mtcp(CP15_EVENT_TYPE_SEL, events[i]);
    }

    Fmsh_EnableCycleCounter();
    Fmsh_ResetPerfCounter();
}

/****************************************************************************/
/**
 * @brief    Delay using cycle counter.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_DelayCnt (uint64_t cnt)
{
    uint64_t t_cur, t_end;
    uint32_t value_h = 0;
    uint32_t value_l1 = 0, value_l2 = 0;

    Fmsh_GetCycleCounter(&value_l1);
    t_end = (uint64_t)value_l1 + cnt;

    while (1)
    {
        Fmsh_GetCycleCounter(&value_l2);
        if (value_l2 < value_l1)
        {
            value_h++;
        }
        value_l1 = value_l2;
        t_cur = ((uint64_t)value_h << 32) | value_l2;
        if (t_cur >= t_end)
        {
            break;
        }
    }
}
