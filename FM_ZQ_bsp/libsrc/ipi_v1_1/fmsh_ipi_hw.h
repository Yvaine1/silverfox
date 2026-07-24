
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

/**
*
* @file ipi_hw.h
* @{
*
* This file contains macro definitions for low level HW related params
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.

* </pre>
*
******************************************************************************/
#ifndef IPIPSU_HW_H_ /* prevent circular inclusions */
#define IPIPSU_HW_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
/* Message RAM related params */
#define IPIPSU_MSG_RAM_BASE           0xFF990000U
#define IPIPSU_MSG_BUF_SIZE           8U /* Size in Words */
#define IPIPSU_MAX_BUFF_INDEX         7U

/* EIGHT pairs of TWO buffers(msg+resp) of THIRTY TWO bytes each */
#define IPIPSU_BUFFER_OFFSET_GROUP    (8U * 2U * 32U)
#define IPIPSU_BUFFER_OFFSET_TARGET   (32U * 2U)
#define IPIPSU_BUFFER_OFFSET_RESPONSE (32U)

/* Number of IPI slots enabled on the device */
#define IPIPSU_MAX_TARGETS            PAR_IPIPSU_NUM_TARGETS

/* Register Offsets for each member  of IPI Register Set */
#define IPIPSU_TRIG_OFFSET            0x00
#define IPIPSU_OBS_OFFSET             0x04
#define IPIPSU_ISR_OFFSET             0x10
#define IPIPSU_IMR_OFFSET             0x14
#define IPIPSU_IER_OFFSET             0x18
#define IPIPSU_IDR_OFFSET             0x1C

/* MASK of all valid IPI bits in above registers */
#define IPIPSU_ALL_MASK               0x0F0F0301U

#ifdef __cplusplus
}
#endif

#endif /* IPIPSU_HW_H_ */
/** @} */
