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
 * @file fmsh_mpu.h
 *
 * This header file contains specific mpu related APIs.
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
#ifndef _FMSH_MPU_H_
#define _FMSH_MPU_H_

/***************************** Include Files ********************************/

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
void Fmsh_EnableMPU(void);
void Fmsh_DisableMPU(void);
void Fmsh_DisableMPURegions(void);
void Fmsh_EnableMPURegion(int region);
void Fmsh_DisableMPURegion(int region);
void Fmsh_EnableBackgroundRegion(void);
void Fmsh_DisableBackgroundRegion(void);
void Fmsh_SetAttribute(uint32_t addr, uint32_t region_size, int region_num,
                       uint32_t attrib);
int Fmsh_FindSetAttribute(uint32_t addr, uint32_t region_size, uint32_t attrib);
int Fmsh_InitMPU(void);

#ifdef __cplusplus
}
#endif

#endif
