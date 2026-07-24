
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
 * @file fmsh_ipi_buf.h
 * @addtogroup ipipsu_v2_6
 * @{
 * @details
 *
 * This is the header file for implementation of IPIPSU driver get buffer
 *functions. Inter Processor Interrupt (IPI) is used for communication between
 * different processors.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 *****************************************************************************/
#ifndef FMSH_IPI_BUF_H_
#define FMSH_IPI_BUF_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Function Prototypes *****************************/

u32 *IpiPsu_GetBufferAddress(IpiPsu *InstancePtr, u32 SrcCpuMask,
                             u32 DestCpuMask, u32 BufferType);

u32 IpiPsu_GetBufferIndex(const IpiPsu *InstancePtr, u32 CpuMask);
#ifdef __cplusplus
}
#endif

#endif /* FMSH_IPI_BUF_H_ */
/** @} */
