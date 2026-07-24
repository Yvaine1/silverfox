
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

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "string.h"
#include "fmsh_mailbox.h"
#include "fmsh_gic.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 IpiPs_Init(Mailbox *InstancePtr, u8 DeviceId);
static u32 IpiPs_Send(Mailbox *InstancePtr, u8 Is_Blocking);
static u32 IpiPs_SendData(Mailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
                          u8 BufferType, u8 Is_Blocking);
static u32 IpiPs_PollforDone(Mailbox *InstancePtr);
static u32 IpiPs_RecvData(Mailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
                          u8 BufferType);
static int IpiPs_RegisterIrq(FGicPs *IntcInstancePtr, Mailbox *InstancePtr,
                             u32 IpiIntrId);
static void IpiPs_ErrorIntrHandler(void *MailboxPtr);
static void IpiPs_IntrHandler(void *MailboxPtr);

/****************************************************************************/
/**
 * Initialize the Mailbox Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	DeviceId is the IPI Instance to be worked on
 *
 * @return	FMSH_SUCCESS if initialization was successful
 * 		FMSH_FAILURE in case of failure
 */
/****************************************************************************/
u32 Mailbox_Initialize (Mailbox *InstancePtr, u8 DeviceId)
{
    u32 Status = FMSH_FAILURE;

    /* Verify arguments. */
    Fmsh_AssertNonvoid(InstancePtr != NULL);

    memset(InstancePtr, 0, sizeof(Mailbox));

    InstancePtr->Mbox_IPI_SendData = IpiPs_SendData;
    InstancePtr->Mbox_IPI_Send = IpiPs_Send;
    InstancePtr->Mbox_IPI_Recv = IpiPs_RecvData;

    Status = IpiPs_Init(InstancePtr, DeviceId);
    return Status;
}

/****************************************************************************/
/**
 * Initialize the FMZQ Mailbox Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	DeviceId is the IPI Instance to be worked on
 *
 * @return	FMSH_SUCCESS if initialization was successful
 * 		FMSH_FAILURE in case of failure
 */
/****************************************************************************/
static u32 IpiPs_Init (Mailbox *InstancePtr, u8 DeviceId)
{
    u32 Status = FMSH_FAILURE;
    IpiPsu_Config *CfgPtr;
    Mailbox_Agent *DataPtr = &InstancePtr->Agent;
    IpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

    CfgPtr = IpiPsu_LookupConfig(DeviceId);
    if (NULL == CfgPtr)
    {
        return Status;
    }

    Status = IpiPsu_CfgInitialize(IpiInstancePtr, CfgPtr, CfgPtr->BaseAddress);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    /* Enable reception of IPI from all CPUs */
    IpiPsu_InterruptEnable(IpiInstancePtr, IPIPSU_ALL_MASK);

    /* Clear Any existing Interrupts */
    IpiPsu_ClearInterruptStatus(IpiInstancePtr, IPIPSU_ALL_MASK);

    /* Register IRQ */
    Status = IpiPs_RegisterIrq(&IntcInstance, InstancePtr, CfgPtr->IntId);

    return Status;
}

/*****************************************************************************/
/**
 * This function triggers an IPI to a destnation CPU
 *
 * @param InstancePtr Pointer to the Mailbox instance.
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return	FMSH_SUCCESS in case of success
 * 		FMSH_FAILURE in case of failure
 */
/****************************************************************************/
static u32 IpiPs_Send (Mailbox *InstancePtr, u8 Is_Blocking)
{
    Mailbox_Agent *DataPtr = &InstancePtr->Agent;
    IpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
    u32 Status = FMSH_SUCCESS;

    IpiPsu_TriggerIpi(IpiInstancePtr, DataPtr->RemoteId);
    if (Is_Blocking != 0U)
    {
        Status = IpiPs_PollforDone(InstancePtr);
    }

    return Status;
}

/*****************************************************************************/
/**
 * This function sends an IPI message to a destnation CPU
 *
 * @param InstancePtr Pointer to the Mailbox instance
 * @param MsgBufferPtr is the pointer to Buffer which contains the message to
 *	  be sent
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer
 * @param Is_Blocking if set trigger the notification in blocking mode
 *
 * @return	FMSH_SUCCESS in case of success
 * 		FMSH_FAILURE in case of failure
 */
/****************************************************************************/
static u32 IpiPs_SendData (Mailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
                           u8 BufferType, u8 Is_Blocking)
{
    Mailbox_Agent *DataPtr = &InstancePtr->Agent;
    IpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
    u32 Status = FMSH_SUCCESS;

    IpiPsu_WriteMessage(IpiInstancePtr, DataPtr->RemoteId, MsgBufferPtr, MsgLen,
                        BufferType);
    IpiPsu_TriggerIpi(IpiInstancePtr, DataPtr->RemoteId);
    if (Is_Blocking != 0U)
    {
        Status = IpiPs_PollforDone(InstancePtr);
    }

    return Status;
}

/*****************************************************************************/
/**
 * Poll for an acknowledgement using Observation Register.
 *
 * @param InstancePtr Pointer to the Mailbox instance
 *
 * @return	FMSH_SUCCESS in case of success
 * 		FMSH_FAILURE in case of failure
 */
/****************************************************************************/
static u32 IpiPs_PollforDone (Mailbox *InstancePtr)
{
    Mailbox_Agent *DataPtr = &InstancePtr->Agent;
    IpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
    u32 Timeout = IPI_DONE_TIMEOUT_VAL;
    u32 Status = FMSH_SUCCESS;
    u32 Flag;

    do
    {
        Flag = (IpiPsu_ReadReg(IpiInstancePtr->Config.BaseAddress,
                               IPIPSU_OBS_OFFSET)) &
               (DataPtr->RemoteId);
        if (Flag == 0U)
        {
            break;
        }
        delay_us(100);
        Timeout--;
    } while (Timeout != 0U);

    if (Timeout == 0U)
    {
        Status = FMSH_FAILURE;
    }

    return Status;
}

/*****************************************************************************/
/**
 * This function reads an IPI message
 *
 * @param InstancePtr Pointer to the Mailbox instance
 * @param MsgBufferPtr is the pointer to Buffer to which the read message needs
 *	  to be stored
 * @param MsgLen is the length of the buffer/message
 * @param BufferType is the type of buffer
 *
 * @return
 *	- FMSH_SUCCESS if successful
 *	- FMSH_FAILURE if unsuccessful
 *
 ****************************************************************************/
static u32 IpiPs_RecvData (Mailbox *InstancePtr, void *MsgBufferPtr, u32 MsgLen,
                           u8 BufferType)
{
    u32 Status = FMSH_FAILURE;
    Mailbox_Agent *DataPtr = &InstancePtr->Agent;
    IpiPsu *IpiInstancePtr = &DataPtr->IpiInst;

    Status = IpiPsu_ReadMessage(IpiInstancePtr, DataPtr->SourceId, MsgBufferPtr,
                                MsgLen, BufferType);
    return Status;
}

static int IpiPs_RegisterIrq (FGicPs *IntcInstancePtr, Mailbox *InstancePtr,
                              u32 IpiIntrId)
{
    u32 Status = FMSH_FAILURE;

    Status = FGicPs_Connect(IntcInstancePtr, IpiIntrId,
                            (FMSH_InterruptHandler)IpiPs_IntrHandler,
                            (void *)InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    Status = FGicPs_Connect(IntcInstancePtr, MAILBOX_INTR_ID,
                            (FMSH_InterruptHandler)IpiPs_ErrorIntrHandler,
                            (void *)InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    /* Enable the interrupt for the device */
    FGicPs_Enable(IntcInstancePtr, IpiIntrId);
    FGicPs_Enable(IntcInstancePtr, MAILBOX_INTR_ID);

    return Status;
}

static void IpiPs_IntrHandler (void *MailboxPtr)
{
    Mailbox *InstancePtr = (Mailbox *)((void *)MailboxPtr);
    Mailbox_Agent *DataPtr = &InstancePtr->Agent;
    IpiPsu *IpiInstancePtr = &DataPtr->IpiInst;
    u32 IntrStatus;

    IntrStatus = IpiPsu_GetInterruptStatus(IpiInstancePtr);
    IpiPsu_ClearInterruptStatus(IpiInstancePtr, IntrStatus);
    DataPtr->RemoteId = IntrStatus;
    if (InstancePtr->RecvHandler != NULL)
    {
        InstancePtr->RecvHandler(InstancePtr->RecvRefPtr);
    }
}

static void IpiPs_ErrorIntrHandler (void *MailboxPtr)
{
    Mailbox *InstancePtr = (Mailbox *)((void *)MailboxPtr);
    u32 Status = FMSH_FAILURE;

    Status = IpiPsu_ReadReg(IPI_BASEADDRESS, IPIPSU_ISR_OFFSET);
    IpiPsu_WriteReg(IPI_BASEADDRESS, IPIPSU_ISR_OFFSET, Status);
    if (InstancePtr->ErrorHandler != NULL)
    {
        InstancePtr->ErrorHandler(InstancePtr->ErrorRefPtr, Status);
    }
}
