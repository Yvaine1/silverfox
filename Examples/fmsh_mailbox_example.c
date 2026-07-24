
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

#include <stdlib.h>
// #include "xparameters.h"
#include "fmsh_gic.h"
#include "fmsh_mailbox.h"
#include "fmsh_mailbox_example.h"

/************************* Test Configuration ********************************/
/* IPI device ID to use for this test */
#define TEST_CHANNEL_ID PAR_IPIPSU_0_DEVICE_ID
/* Test message length in words. Max is 8 words (32 bytes) */
#define TEST_MSG_LEN    8

/*****************************************************************************/
Mailbox MboxInstance;
static volatile int RecvDone = 0;    /**< Done flag */
static volatile int RecvDone1 = 0;   /**< Done flag */
static volatile int ErrorStatus = 0; /**< Error Status flag*/
static u32 ReqBuffer[TEST_MSG_LEN];
static u32 RespBuffer[TEST_MSG_LEN];

// int Mailbox_Example(Mailbox *InstancePtr, u8 DeviceId);
static void DoneHandler(void *CallBackRefPtr);
static void ErrorHandler(void *CallBackRefPtr, u32 Mask);
//static void DoneHandler1(void *CallBackRefPtr);
/*
int main(void)
{
    int Status;

    fmsh_print("Inside Mailbox Example\r\n");

    ret =  FGicPs_SelfTest(&IntcInstance);
    if(ret != GIC_SUCCESS)
        fmsh_print("GIC Setup Failed!\r\n");
    else
        fmsh_print("GIC Setup pass!\r\n");

    Status = Mailbox_Example(&MboxInstance, TEST_CHANNEL_ID);
    if (Status != FMSH_SUCCESS) {
        fmsh_print("Mailbox Example Failed\n\r");
        return FMSH_FAILURE;
    }

    fmsh_print("Successfully ran Mailbox Example\n\r");
    return FMSH_SUCCESS;
}
*/

extern IpiPsu_Config IpiPsu_ConfigTable[];

static void Irq_Target_Config (FGicPs *InstancePtr)
{
#if USE_AMP == 1
    /*
     * The target cpu register of gic distrubutor initialized by primary cpu.
     */
    return;
#endif
    IpiPsu_Config *CfgPtr;

#if defined(CORTEX_A53)
    CfgPtr = IpiPsu_LookupConfig(IPI_ID_APU0);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 1 << 0, CfgPtr->IntId);
    }
    CfgPtr = IpiPsu_LookupConfig(IPI_ID_APU1);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 1 << 1, CfgPtr->IntId);
    }
    CfgPtr = IpiPsu_LookupConfig(IPI_ID_APU2);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 1 << 2, CfgPtr->IntId);
    }
    CfgPtr = IpiPsu_LookupConfig(IPI_ID_APU3);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 1 << 3, CfgPtr->IntId);
    }
    FGicPs_InterruptMaptoCpu(InstancePtr, 1, MAILBOX_INTR_ID);

#elif defined(CORTEX_R5)
    CfgPtr = IpiPsu_LookupConfig(IPI_ID_RPU0);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 1 << 0, CfgPtr->IntId);
    }
    CfgPtr = IpiPsu_LookupConfig(IPI_ID_RPU1);
    if (CfgPtr)
    {
        FGicPs_InterruptMaptoCpu(InstancePtr, 1 << 1, CfgPtr->IntId);
    }

#endif
}

#define TEST_SELF
int Mailbox_Example (u8 DeviceId)
{
    u32 Index;
    u32 Status;
    u32 TmpBufPtr[TEST_MSG_LEN];
#ifdef TEST_SEND    
    u32 TmpBufPtr1[TEST_MSG_LEN];
#endif
    Mailbox *InstancePtr = &MboxInstance;

    Irq_Target_Config(&IntcInstance);
    Status = Mailbox_Initialize(InstancePtr, DeviceId);
    if (Status != FMSH_SUCCESS)
    {
        goto Done;
    }

    /* Register callbacks for Error and Read */
    Mailbox_SetCallBack(InstancePtr, MAILBOX_RECV_HANDLER, (void *)DoneHandler,
                        InstancePtr);
    Mailbox_SetCallBack(InstancePtr, MAILBOX_ERROR_HANDLER,
                        (void *)ErrorHandler, InstancePtr);

#ifdef TEST_SELF
    fmsh_print_dbg("Req Message Content:\r\n");
    for (Index = 0; Index < TEST_MSG_LEN; Index++)
    {
        ReqBuffer[Index] = rand();
        fmsh_print_dbg("W%d: 0x%08x\r\n", Index, ReqBuffer[Index]);
    }

    /* Send an IPI Req Message */
    Status = Mailbox_SendData(InstancePtr, TEST_REMOTE_CPU, ReqBuffer,
                              TEST_MSG_LEN, MBOX_MSG_TYPE_REQ, 1);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print("Sending Req Message Failed\n\r");
        goto Done;
    }

    while (!ErrorStatus && !RecvDone);

    if (ErrorStatus)
    {
        fmsh_print("Error occurred during IPI transfer\n\r");
        Status = FMSH_FAILURE;
        goto Done;
    }

    RecvDone = 0;
    ErrorStatus = 0;

    /* Read an IPI Message */
    Status = Mailbox_Recv(InstancePtr, TEST_REMOTE_CPU, TmpBufPtr, TEST_MSG_LEN,
                          MBOX_MSG_TYPE_REQ);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print("Reading an IPI Req message Failed\n\r");
        goto Done;
    }

    fmsh_print_dbg("Message Received:\r\n");
    for (Index = 0; Index < TEST_MSG_LEN; Index++)
    {
        fmsh_print_dbg("W%d: 0x%08x\r\n", Index, TmpBufPtr[Index]);
    }

    /* Response Message */
    for (Index = 0; Index < TEST_MSG_LEN; Index++)
    {
        RespBuffer[Index] = ~TmpBufPtr[Index];
    }

    /* Send an IPI Response Message */
    Status = Mailbox_SendData(InstancePtr, TEST_REMOTE_CPU, RespBuffer,
                              TEST_MSG_LEN, MBOX_MSG_TYPE_RESP, 0);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print("Sending Resp Message Failed\n\r");
        goto Done;
    }

    while (!ErrorStatus && !RecvDone);
    if (ErrorStatus)
    {
        Status = FMSH_FAILURE;
        fmsh_print("Error occurred during IPI transfer\n\r");
        goto Done;
    }

    /* Read an IPI Resp Message */
    Status = Mailbox_Recv(InstancePtr, TEST_REMOTE_CPU, TmpBufPtr, TEST_MSG_LEN,
                          MBOX_MSG_TYPE_RESP);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print("Reading an IPI Resp message Failed\n\r");
        goto Done;
    }

    /* Compare Data */
    for (Index = 0; Index < TEST_MSG_LEN; Index++)
    {
        if (ReqBuffer[Index] != (~TmpBufPtr[Index]))
        {
            fmsh_print(
                "Data Mismatch Expected: 0x%08x"
                "Got: 0x%08x\r\n",
                RespBuffer[Index], TmpBufPtr[Index]);
            goto Done;
        }
    }
#endif

#ifdef TEST_SEND
    // ping test
    fmsh_print_dbg("Req Message Content:\r\n");
    for (Index = 0; Index < TEST_MSG_LEN; Index++)
    {
        TmpBufPtr[Index] = rand();
        fmsh_print_dbg("W%d: 0x%08x\r\n", Index, TmpBufPtr[Index]);
    }

    /* Send an IPI Req Message */
    Status = Mailbox_SendData(InstancePtr, TEST_REMOTE_CPU, TmpBufPtr,
                              TEST_MSG_LEN, MBOX_MSG_TYPE_REQ, 1);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print("Sending Req Message Failed\n\r");
        goto Done;
    }

    while (!ErrorStatus && !RecvDone);

    if (ErrorStatus)
    {
        fmsh_print("Error occurred during IPI transfer\n\r");
        Status = FMSH_FAILURE;
        goto Done;
    }

    // clear
    memset(TmpBufPtr1, 0, sizeof(TmpBufPtr1));

    /* Read an IPI Message */
    Status = Mailbox_Recv(InstancePtr, TEST_REMOTE_CPU, TmpBufPtr1,
                          TEST_MSG_LEN, MBOX_MSG_TYPE_RESP);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print("Reading an IPI Req message Failed\n\r");
        goto Done;
    }

    fmsh_print_dbg("Message Received:\r\n");
    for (Index = 0; Index < TEST_MSG_LEN; Index++)
    {
        fmsh_print_dbg("W%d: 0x%08x\r\n", Index, TmpBufPtr1[Index]);
        if (TmpBufPtr1[Index] != TmpBufPtr[Index])
        {
            fmsh_print("Recv data error!\n\r");
            Status = FMSH_FAILURE;
            goto Done;
        }
    }
    fmsh_print("Mail ping pass!\n\r");

#endif
#ifdef TEST_RECV

    while (1)
    {
        if (RecvDone)
        {
            /* Read an IPI Message */
            Status = Mailbox_Recv(InstancePtr, TEST_REMOTE_CPU, TmpBufPtr,
                                  TEST_MSG_LEN, MBOX_MSG_TYPE_REQ);
            if (Status != FMSH_SUCCESS)
            {
                fmsh_print("Reading an IPI Req message Failed\n\r");
                goto Done;
            }

            fmsh_print_dbg("Message Received:\r\n");
            for (Index = 0; Index < TEST_MSG_LEN; Index++)
            {
                fmsh_print_dbg("W%d: 0x%08x\r\n", Index, TmpBufPtr[Index]);
            }

            /* Send an IPI Response Message */
            Status = Mailbox_SendData(InstancePtr, TEST_REMOTE_CPU, TmpBufPtr,
                                      TEST_MSG_LEN, MBOX_MSG_TYPE_RESP, 0);
            if (Status != FMSH_SUCCESS)
            {
                fmsh_print("Sending Resp Message Failed\n\r");
                goto Done;
            }
            RecvDone = 0;
            fmsh_print("Sending Resp Message Passed\n\r");
        }
        if (RecvDone1)
        {
            /* Read an IPI Message */
            Status = Mailbox_Recv(InstancePtr, TEST_REMOTE_CPU1, TmpBufPtr,
                                  TEST_MSG_LEN, MBOX_MSG_TYPE_REQ);
            if (Status != FMSH_SUCCESS)
            {
                fmsh_print("Reading an IPI Req message1 Failed\n\r");
                goto Done;
            }

            fmsh_print_dbg("Message1 Received:\r\n");
            for (Index = 0; Index < TEST_MSG_LEN; Index++)
            {
                fmsh_print_dbg("W%d: 0x%08x\r\n", Index, TmpBufPtr[Index]);
            }

            /* Send an IPI Response Message */
            Status = Mailbox_SendData(InstancePtr, TEST_REMOTE_CPU1, TmpBufPtr,
                                      TEST_MSG_LEN, MBOX_MSG_TYPE_RESP, 0);
            if (Status != FMSH_SUCCESS)
            {
                fmsh_print("Sending Resp Message1 Failed\n\r");
                goto Done;
            }
            RecvDone1 = 0;
            fmsh_print("Sending Resp Message1 Passed\n\r");
        }
    }

#endif
Done:
    return Status;
}

static void DoneHandler (void *CallBackRef)
{
    Mailbox *InstancePtr = (Mailbox *)CallBackRef;
    if (InstancePtr->Agent.RemoteId == TEST_REMOTE_CPU)
    {
        RecvDone = 1;
    }
    else if (InstancePtr->Agent.RemoteId == TEST_REMOTE_CPU1)
    {
        RecvDone1 = 1;
    }
    else
    {
        // mailbox conflict
        ErrorStatus = InstancePtr->Agent.RemoteId;
    }
}

static void ErrorHandler (void *CallBackRef, u32 Mask) { ErrorStatus = Mask; }
