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
 *
 * @file fmsh_cache.h
 *
 * This header file contains specific cache related APIs.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who      Date     Changes
 * ----- -------- -------- -----------------------------------------------
 * 1.00  hzq     22/11/22    Initial Version
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_CACHE_H_
#define _FMSH_CACHE_H_

/***************************** Include Files ********************************/
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions ****************************/
#define ICACHE_LINE_SIZE (32)
#define DCACHE_LINE_SIZE (32)

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
void Fmsh_DCacheEnable(void);
void Fmsh_DCacheDisable(void);
void Fmsh_DCacheInvalidate(void);
void Fmsh_DCacheInvalidateRange(uintptr_t adr, uint32_t len);
void Fmsh_DCacheFlush(void);
void Fmsh_DCacheFlushRange(uintptr_t adr, uint32_t len);
void Fmsh_DCacheInvalidateLine(uintptr_t adr);
void Fmsh_DCacheFlushLine(uintptr_t adr);
void Fmsh_DCacheStoreLine(uintptr_t adr);

void Fmsh_ICacheEnable(void);
void Fmsh_ICacheDisable(void);
void Fmsh_ICacheInvalidate(void);
void Fmsh_ICacheInvalidateRange(uintptr_t adr, uint32_t len);
void Fmsh_ICacheInvalidateLine(uintptr_t adr);

#ifdef __cplusplus
}
#endif

#endif
