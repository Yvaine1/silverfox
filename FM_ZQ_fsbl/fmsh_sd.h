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
 * @file fmsh_sdmmc_verify.h
 * @addtogroup sdmmc
 * @{
 *
 *  This header file contains the sdmmc verification functions (or macros).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2023/02/16  First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_SDMMC_VERIFY_H_ /* prevent circular inclusions */
#define _FMSH_SDMMC_VERIFY_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************Include
 * File*********************************/
#include "fmsh_sdhci.h"
#include "fmsh_sdhci_card.h"
#include "fmsh_sdhci_hw.h"

/**********************************Constant
 * Definition**************************/

/**********************************Type
 * Definition******************************/

/**********************************Macro (inline function)
 * Definition***********/

/**********************************Variable
 * Definition**************************/

/**********************************Function
 * Prototype***************************/
int fmsh_sdmmc_verify(void);
__attribute__((unused)) static int filesystem_test(FSdPsu_T *sdPtr);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
