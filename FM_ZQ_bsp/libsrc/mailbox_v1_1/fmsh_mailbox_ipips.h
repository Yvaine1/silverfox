
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

#ifndef FMSH_MAILBOX_IPIPS_H
#define FMSH_MAILBOX_IPIPS_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_ipi.h"
// #include "xscugic.h"

/**************************** Type Definitions *******************************/
typedef struct {
    IpiPsu IpiInst;
    //	FGicPs *GicInst;
    u32 SourceId;
    u32 RemoteId;
} Mailbox_Agent;
/************************** Constant Definitions *****************************/
// #define BIT(x)                 	(1 << (x))
#define IPI_DONE_TIMEOUT_VAL 3000000

#define MAILBOX_IPI0         BIT(0)
#define MAILBOX_IPI1         BIT(8)
#define MAILBOX_IPI2         BIT(9)
#define MAILBOX_IPI3         BIT(16)
#define MAILBOX_IPI4         BIT(17)
#define MAILBOX_IPI5         BIT(18)
#define MAILBOX_IPI6         BIT(19)
#define MAILBOX_IPI7         BIT(24)
#define MAILBOX_IPI8         BIT(25)
#define MAILBOX_IPI9         BIT(26)
#define MAILBOX_IPI10        BIT(27)
#define MAILBOX_MAX_CHANNELS 11U
#define MAILBOX_INTR_ID      22U

/* Error Handling */
#define IPI_BASEADDRESS      0xFF380000U
#define IPI_ADDRDECODE_ERROR BIT(0)

/*
#define REMOTE_MASK_APU0    MAILBOX_IPI0
#define REMOTE_MASK_RPU0    MAILBOX_IPI1
#define REMOTE_MASK_RPU1    MAILBOX_IPI2
#define REMOTE_MASK_PMU0    MAILBOX_IPI3
#define REMOTE_MASK_PMU1    MAILBOX_IPI4
#define REMOTE_MASK_PMU2    MAILBOX_IPI5
#define REMOTE_MASK_PMU3    MAILBOX_IPI6
#define REMOTE_MASK_APU1    MAILBOX_IPI7
#define REMOTE_MASK_APU2    MAILBOX_IPI8
#define REMOTE_MASK_APU3    MAILBOX_IPI9
#define REMOTE_MASK_PL0     MAILBOX_IPI10
*/
/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* FMSH_MAILBOX_IPIPS_H */
