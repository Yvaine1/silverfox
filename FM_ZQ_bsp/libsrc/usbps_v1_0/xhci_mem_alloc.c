/******************************************************************************
 *
 * Copyright (C) 2014-2021 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 *
 ******************************************************************************
 * xhci_mem_alloc.c
 * Host/Device Super Speed Plus controller driver,
 *
 * Memory allocation for USBSSP hardware controller
 *****************************************************************************/

#include "cdn_log.h"
#include "cdn_xhci_structs_if.h"
#include <string.h>
/**
 * Maximal number of driver instances used in demoTB
 */
#define MAX_INSTANCE_NUM 1U

void SetMemResCallback(USBSSP_DriverResourcesT *res);

/*
 * see table 54 in chapter 6 of XHCI specification for allocation requirements
 * details
 */
typedef struct {
    /** event ring segment entry */
    USBSSP_RingElementT epRingPool[USBSSP_PRODUCER_QUEUE_SIZE *
                                   (USBSSP_MAX_EP_CONTEXT_NUM + 2U)]
        __attribute__((aligned(1024)));
    /** Event Ring */
    USBSSP_RingElementT eventPool[USBSSP_EVENT_QUEUE_SIZE]
        __attribute__((aligned(1024)));
    /** Device context base array structure */
    USBSSP_DcbaaT dcbaa __attribute__((aligned(USBSSP_DCBAA_ALIGNMENT)));
    /** Input context structure */
    USBSSP_InputContexT inputContext
        __attribute__((aligned(USBSSP_CONTEXT_ALIGNMENT)));
    /** Output context structure */
    USBSSP_OutputContexT outputContext
        __attribute__((aligned(USBSSP_PAGE_SIZE)));
    /** Scratch pad buffers (extra element for last pointer = NULL) */
    uint64_t scratchpad[USBSSP_SCRATCHPAD_BUFF_NUM + 1U]
        __attribute__((aligned(USBSSP_PAGE_SIZE)));
    /** Scratch pad buffers pool */
    uint8_t scratchpadPool[USBSSP_SCRATCHPAD_BUFF_NUM * USBSSP_PAGE_SIZE]
        __attribute__((aligned(USBSSP_PAGE_SIZE)));
    /** event ring segment entry */
    uint64_t eventRingSegmentEntry[USBSSP_INTERRUPTER_COUNT * 2U]
        __attribute__((aligned(USBSSP_ERST_ALIGNMENT)));
    /** allocated memory for stream objects */
    USBSSP_ProducerQueueT streamMemoryPool[USBSSP_MAX_EP_NUM_STRM_EN]
                                          [USBSSP_STREAM_ARRAY_SIZE];
    /** allocation memory for stream rings */
    USBSSP_RingElementT
        streamRing[USBSSP_MAX_EP_NUM_STRM_EN][USBSSP_STREAM_ARRAY_SIZE]
                  [USBSSP_PRODUCER_QUEUE_SIZE] __attribute__((aligned(1024)));
    /** EP0 buffer */
    uint8_t ep0Buffer[USBSSP_EP0_DATA_BUFF_SIZE] __attribute__((aligned(64)));

} XhciResT;

/**
 * Function called from driver for clearing res->xhciMemRes
 * @param res
 */
/* parasoft-begin-suppress METRICS-39-3 "The value of VOCF metric for a function
 * should not be higher than 4, DRV-4790" */
static void cleanMemRes (XhciResT *memRes)
{
    /* set values to zero */
    (void)memset(&memRes->dcbaa, 0, sizeof(memRes->dcbaa));
    /** clear endpoint xfer ring memory for all EPs*/
    (void)memset(memRes->epRingPool, 0, sizeof(memRes->epRingPool));
    (void)memset(memRes->eventPool, 0, sizeof(memRes->eventPool));
    (void)memset(memRes->eventRingSegmentEntry, 0,
                 sizeof(memRes->eventRingSegmentEntry));
    (void)memset(&memRes->inputContext, 0, sizeof(memRes->inputContext));
    (void)memset(&memRes->outputContext, 0, sizeof(memRes->outputContext));
    (void)memset(memRes->scratchpad, 0, sizeof(memRes->scratchpad));
    (void)memset(memRes->scratchpadPool, 0, sizeof(memRes->scratchpadPool));
    (void)memset(memRes->streamMemoryPool, 0, sizeof(memRes->streamMemoryPool));
    (void)memset(memRes->streamRing, 0, sizeof(memRes->streamRing));
    (void)memset(memRes->ep0Buffer, 0, sizeof(memRes->ep0Buffer));
}
/* parasoft-end-suppress METRICS-39-3 */

/**
 * Function called from driver for setting res->xhciMemRes
 * @param res
 */
/* parasoft-begin-suppress METRICS-39-3 "The value of VOCF metric for a function
 * should not be higher than 4, DRV-4790" */
void SetMemResCallback (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;

    /* parasoft-begin-suppress MISRA2012-RULE-1_1_a_c90-2 "C90 - limits",
     * DRV-5635 */
    /* parasoft-begin-suppress MISRA2012-RULE-1_1_a_c99-2 "C99 - limits",
     * DRV-5636 */
    static XhciResT xhciResArray[MAX_INSTANCE_NUM];
    /* parasoft-end-suppress MISRA2012-RULE-1_1_a_c99-2 */
    /* parasoft-end-suppress MISRA2012-RULE-1_1_a_c90-2 */

    /*
     * resources are organised in array for many driver resources instances
     */
    static USBSSP_XhciResourcesT memResArray[MAX_INSTANCE_NUM];

    if (res->instanceNo > (MAX_INSTANCE_NUM - 1U))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Instance incorrectly selected : %d, max number is: %d\n",
                res->instanceNo, MAX_INSTANCE_NUM);
        ret = CDN_EINVAL;
    }

    if (ret == CDN_EOK)
    {
        // resource allocation
        memResArray[res->instanceNo].dcbaa = &xhciResArray[res->instanceNo]
                                                  .dcbaa;
        memResArray[res->instanceNo].epRingPool = xhciResArray[res->instanceNo]
                                                      .epRingPool;
        memResArray[res->instanceNo].eventPool = xhciResArray[res->instanceNo]
                                                     .eventPool;
        memResArray[res->instanceNo]
            .eventRingSegmentEntry = xhciResArray[res->instanceNo]
                                         .eventRingSegmentEntry;
        memResArray[res->instanceNo]
            .inputContext = &xhciResArray[res->instanceNo].inputContext;
        memResArray[res->instanceNo]
            .outputContext = &xhciResArray[res->instanceNo].outputContext;
        memResArray[res->instanceNo].scratchpad = xhciResArray[res->instanceNo]
                                                      .scratchpad;
        memResArray[res->instanceNo]
            .scratchpadPool = xhciResArray[res->instanceNo].scratchpadPool;
        memResArray[res->instanceNo]
            .streamMemoryPool = &xhciResArray[res->instanceNo].streamMemoryPool;
        memResArray[res->instanceNo].streamRing = &xhciResArray[res->instanceNo]
                                                       .streamRing;
        memResArray[res->instanceNo].ep0Buffer = xhciResArray[res->instanceNo]
                                                     .ep0Buffer;

        res->xhciMemRes = &memResArray[res->instanceNo];
        cleanMemRes(&xhciResArray[res->instanceNo]);
    }
}
/* parasoft-end-suppress METRICS-39-3 */
