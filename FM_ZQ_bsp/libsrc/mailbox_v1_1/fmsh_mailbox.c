
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
 * @file mailbox.c
 * @addtogroup mailbox_v1_1
 * @{
 * @details
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------

 *</pre>
 *
 *@note
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "fmsh_mailbox.h"

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function triggers an IPI to a destination CPU
 *
 * @param InstancePtr Pointer to the Mailbox instance
 * @param RemoteId is the Mask of the CPU to which IPI is to be triggered
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return
 *	- FMSH_SUCCESS if successful
 *	- FMSH_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 Mailbox_Send (Mailbox *InstancePtr, u32 RemoteId, u8 Is_Blocking)
{
    u32 Status = FMSH_FAILURE;

    /* Verify arguments. */
    Fmsh_AssertNonvoid(InstancePtr != NULL);

    InstancePtr->Agent.RemoteId = RemoteId;
    Status = InstancePtr->Mbox_IPI_Send(InstancePtr, Is_Blocking);
    return Status;
}

/*****************************************************************************/
/**
 * This function sends an IPI message to a destination CPU
 *
 * @param InstancePtr Pointer to the Mailbox instance
 * @param RemoteId is the Mask of the CPU to which IPI is to be triggered
 * @param BufferPtr is the pointer to Buffer which contains the message to be
 *sent
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer (MBOX_MSG_TYPE_REQ (OR)
 *	  MBOX_MSG_TYPE_RESP)
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return
 *	- FMSH_SUCCESS if successful
 *	- FMSH_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 Mailbox_SendData (Mailbox *InstancePtr, u32 RemoteId, void *BufferPtr,
                      u32 MsgLen, u8 BufferType, u8 Is_Blocking)
{
    u32 Status = FMSH_FAILURE;

    /* Verify arguments. */
    Fmsh_AssertNonvoid(InstancePtr != NULL);
    Fmsh_AssertNonvoid(BufferPtr != NULL);
    Fmsh_AssertNonvoid(MsgLen <= MAILBOX_MAX_MSG_LEN);
    Fmsh_AssertNonvoid((BufferType == MBOX_MSG_TYPE_REQ) ||
                       (BufferType == MBOX_MSG_TYPE_RESP));

    InstancePtr->Agent.RemoteId = RemoteId;
    Status = InstancePtr->Mbox_IPI_SendData(InstancePtr, BufferPtr, MsgLen,
                                            BufferType, Is_Blocking);
    return Status;
}

/*****************************************************************************/
/**
 * This function reads an IPI message
 *
 * @param InstancePtr Pointer to the Mailbox instance
 * @param SourceId is the Mask for the CPU which has sent the message
 * @param BufferPtr is the pointer to Buffer to which the read message needs
 *	  to be stored
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer (MBOX_MSG_TYPE_REQ or
 *	  MBOX_MSG_TYPE_RESP)
 *
 * @return
 *	- FMSH_SUCCESS if successful
 *	- FMSH_FAILURE if unsuccessful
 *
 ****************************************************************************/
u32 Mailbox_Recv (Mailbox *InstancePtr, u32 SourceId, void *BufferPtr,
                  u32 MsgLen, u8 BufferType)
{
    u32 Status = FMSH_FAILURE;

    /* Verify arguments. */
    Fmsh_AssertNonvoid(InstancePtr != NULL);
    Fmsh_AssertNonvoid(BufferPtr != NULL);
    Fmsh_AssertNonvoid(MsgLen <= MAILBOX_MAX_MSG_LEN);
    Fmsh_AssertNonvoid((BufferType == MBOX_MSG_TYPE_REQ) ||
                       (BufferType == MBOX_MSG_TYPE_RESP));

    InstancePtr->Agent.SourceId = SourceId;
    Status = InstancePtr->Mbox_IPI_Recv(InstancePtr, BufferPtr, MsgLen,
                                        BufferType);
    if (Status != (u32)FMSH_SUCCESS)
    {
        fmsh_print("Error while receiving message %s", __func__);
    }
    return Status;
}

/*****************************************************************************/
/**
 *
 * This routine installs an asynchronous callback function for the given
 * HandlerType.
 *
 * <pre>
 * HandlerType              Callback Function Type
 * -----------------------  --------------------------------------------------
 * MAILBOX_RECV_HANDLER	   Recv handler
 * MAILBOX_ERROR_HANDLER   Error handler
 *
 * </pre>
 *
 * @param	InstancePtr is a pointer to the Mailbox instance.
 * @param	HandlerType specifies which callback is to be attached.
 * @param	CallBackFunc is the address of the callback function.
 * @param	CallBackRef is a user data item that will be passed to the
 * 		callback function when it is invoked.
 *
 * @return
 *		- FMSH_SUCCESS when handler is installed.
 *		- FMSH_INVALID_PARAM when HandlerType is invalid.
 *
 * @note		Invoking this function for a handler that already has been
 *		installed replaces it with the new handler.
 *
 ******************************************************************************/
s32 Mailbox_SetCallBack (Mailbox *InstancePtr, Mailbox_Handler HandlerType,
                         void *CallBackFuncPtr, void *CallBackRefPtr)
{
    s32 Status;

    /* Verify arguments. */
    Fmsh_AssertNonvoid(InstancePtr != NULL);
    Fmsh_AssertNonvoid(CallBackFuncPtr != NULL);
    Fmsh_AssertNonvoid(CallBackRefPtr != NULL);
    Fmsh_AssertNonvoid((HandlerType == MAILBOX_RECV_HANDLER) ||
                       (HandlerType == MAILBOX_ERROR_HANDLER));

    /*
     * Calls the respective callback function corresponding to
     * the handler type
     */
    switch (HandlerType)
    {
    case MAILBOX_RECV_HANDLER:
        InstancePtr->RecvHandler = (Mailbox_RecvHandler)((
            void *)CallBackFuncPtr);
        InstancePtr->RecvRefPtr = CallBackRefPtr;
        Status = (FMSH_SUCCESS);
        break;

    case MAILBOX_ERROR_HANDLER:
        InstancePtr->ErrorHandler = (Mailbox_ErrorHandler)((
            void *)CallBackFuncPtr);
        InstancePtr->ErrorRefPtr = CallBackRefPtr;
        Status = (FMSH_SUCCESS);
        break;
    default:
        Status = (FMSH_INVALID_PARAM);
        break;
    }

    return Status;
}
