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
 * @file fmsh_mailbox.h
 * @addtogroup xilmailbox_v1_1
 * @{
 * @details
 *
 * The Mailbox library provides the top-level hooks for sending or receiving
 * an inter-processor interrupt (IPI) message using the Zynq® UltraScale+ MPSoC
 * IPI hardware.
 *
 *</pre>
 *
 *@note
 *****************************************************************************/
#ifndef MAILBOX_H
#define MAILBOX_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
// #include "fmsh_io.h"
// #include "fmsh_types.h"
// #include "fmsh_assert.h"
#include "fmsh_print.h"
// #include "parameters.h"
#include "fmsh_mailbox_ipips.h"

/************************** Constant Definitions *****************************/
#define MBOX_MSG_TYPE_REQ              (0x00000001U)
#define MBOX_MSG_TYPE_RESP             (0x00000002U)
#define MAILBOX_MAX_MSG_LEN            8U

#define Fmsh_AssertNonvoid(Expression) FMSH_ASSERT(Expression)

/**************************** Type Definitions *******************************/
typedef void (*Mailbox_RecvHandler)(void *CallBackRefPtr);
typedef void (*Mailbox_ErrorHandler)(void *CallBackRefPtr, u32 ErrorMask);

/**
 * @Mbox_IPI_Send:	    Triggers an IPI to a destination CPU
 * @Mbox_IPI_SendData:     Sends an IPI message to a destination CPU
 * @Mbox_IPI_Recv:         Reads an IPI message
 * @RecvHandler:            Callback for rx IPI event
 * @ErrorHandler:           Callback for error event
 * @ErroRef:                To be passed to the error interrupt callback
 * @RecvRef:                To be passed to the receive interrupt callback.
 * @Agent:                  Used to store IPI Channel information.
 */
typedef struct MboxTag {
    u32 (*Mbox_IPI_Send)(struct MboxTag *InstancePtr, u8 Is_Blocking);
    u32 (*Mbox_IPI_SendData)(struct MboxTag *InstancePtr, void *BufferPtr,
                             u32 MsgLen, u8 BufferType, u8 Is_Blocking);
    u32 (*Mbox_IPI_Recv)(struct MboxTag *InstancePtr, void *BufferPtr,
                         u32 MsgLen, u8 BufferType);
    Mailbox_RecvHandler RecvHandler;
    Mailbox_ErrorHandler ErrorHandler;
    void *ErrorRefPtr;
    void *RecvRefPtr;
    Mailbox_Agent Agent;
} Mailbox;

/**
 * This typedef contains MAILBOX Handler Types.
 */
typedef enum {
    MAILBOX_RECV_HANDLER,  /**< For Recv Handler */
    MAILBOX_ERROR_HANDLER, /**< For Error Handler */
} Mailbox_Handler;

/************************** Function Prototypes ******************************/
u32 Mailbox_Initialize(Mailbox *InstancePtr, u8 DeviceId);
u32 Mailbox_Send(Mailbox *InstancePtr, u32 RemoteId, u8 Is_Blocking);
u32 Mailbox_SendData(Mailbox *InstancePtr, u32 RemoteId, void *BufferPtr,
                     u32 MsgLen, u8 BufferType, u8 Is_Blocking);
u32 Mailbox_Recv(Mailbox *InstancePtr, u32 SourceId, void *BufferPtr,
                 u32 MsgLen, u8 BufferType);
s32 Mailbox_SetCallBack(Mailbox *InstancePtr, Mailbox_Handler HandlerType,
                        void *CallBackFuncPtr, void *CallBackRefPtr);

#ifdef __cplusplus
}
#endif

#endif /* MAILBOX_H */
