/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file fmsh_axidmac_example.h
 * @{
 *
 * Contains example of the FAxidmaPsu driver.
 *
 *<pre>
 * Ver   Who    Date     Changes
 * ----- ---  --------   -----------------------------------------------
 * 1.00  whn  2025/02/24  First Release.
 *
 *</pre>
 *
 ******************************************************************************/
#ifndef _FMSH_AXIDMA_EXAMPLE_H_
#define _FMSH_AXIDMA_EXAMPLE_H_

/***************************** Include Files *********************************/
#include "fmsh_axidmapsu_lib.h"

#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"
#include "fmsh_cache.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 FAxidmaPsu_example(u16 deviceId);

#endif /* #ifndef _FMSH_AXIDMA_EXAMPLE_H_ */
