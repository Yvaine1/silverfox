/******************************************************************************
 *
 * Copyright (C) 2009 - 2022 FMSH, Inc.  All rights reserved.
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
 *
 * @file fmsh_pmu.h
 *
 * This header file contains cortex-r5 pmu related APIs.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who      Date     Changes
 * ----- -------- -------- -----------------------------------------------
 * 1.00  hzq     22/12/02  Initial Version
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_PMU_H_
#define _FMSH_PMU_H_

/***************************** Include Files ********************************/
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
void Fmsh_DisableCycleCounter();
void Fmsh_EnableCycleCounter();
void Fmsh_ResetCycleCounter();
void Fmsh_GetCycleCounter(uint32_t *value);

void Fmsh_DisablePerfCounter();
void Fmsh_EnableEventCounter();
void Fmsh_ResetPerfCounter();
void Fmsh_GetPerfCounter(uint32_t *counters);
void Fmsh_SetEvents(uint32_t *events);
void Fmsh_DelayCnt(uint64_t cnt);

#ifdef __cplusplus
}
#endif

#endif
