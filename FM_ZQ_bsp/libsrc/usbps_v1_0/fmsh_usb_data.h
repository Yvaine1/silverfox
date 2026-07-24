/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_usb_data.h
 *
 * This file contains spi lib
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   HZQ  12/20/2018  First Release
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_USB_DATA_H_
#define _FMSH_USB_DATA_H_

#include "bspconfig.h"
#include "fmsh_common.h"
#include "fmsh_cache.h"
#include "cdn_xhci_if.h"
#include "cdn_xhci_structs_if.h"
#include "cusbd_if.h"
#include "cusbd_structs_if.h"
#include "cusbd_obj_if.h"
#include "cusb_ch9_if.h"
#include "cusb_ch9_structs_if.h"
   
#define USB_REG_BASE 0xfe200000

#if (DCACHE_ENABLE == 1)
#define CACHE_USB_DATA
#ifdef CACHE_USB_DATA
#define USB_DATA_BASE 0x100000
#else
#define USB_DATA_BASE 0x7fe00000
#endif
#else
#define NONE_CACHE_USB_DATA
#endif

#ifdef NONE_CACHE_USB_DATA
extern USBSSP_RingElementT
    g_epRingPool[USBSSP_PRODUCER_QUEUE_SIZE * (USBSSP_MAX_EP_CONTEXT_NUM + 2U)]
    __attribute__((aligned(USBSSP_PAGE_SIZE)));
/** Event Ring */
extern USBSSP_RingElementT g_eventPool[USBSSP_EVENT_QUEUE_SIZE]
    __attribute__((aligned(USBSSP_PAGE_SIZE)));

/** Device context base array structure */
extern USBSSP_DcbaaT g_dcbaa __attribute__((aligned(USBSSP_DCBAA_ALIGNMENT)));

/** Input context structure */
extern USBSSP_InputContexT g_inputContext
    __attribute__((aligned(USBSSP_PAGE_SIZE)));
/** Output context structure */
extern USBSSP_OutputContexT g_outputContext
    __attribute__((aligned(USBSSP_PAGE_SIZE)));
/** Scratch pad buffers (extra element for last pointer = NULL) */
extern uint64_t g_scratchpad[USBSSP_SCRATCHPAD_BUFF_NUM + 1U]
    __attribute__((aligned(USBSSP_PAGE_SIZE)));
/** Scratch pad buffers pool */
extern uint8_t g_scratchpadPool[USBSSP_SCRATCHPAD_BUFF_NUM * USBSSP_PAGE_SIZE]
    __attribute__((aligned(USBSSP_PAGE_SIZE)));
/** event ring segment entry */
extern uint64_t g_eventRingSegmentEntry[USBSSP_INTERRUPTER_COUNT * 2U]
    __attribute__((aligned(USBSSP_ERST_ALIGNMENT)));
/** allocated memory for stream objects */
extern USBSSP_ProducerQueueT g_streamMemoryPool[USBSSP_MAX_EP_NUM_STRM_EN]
                                               [USBSSP_STREAM_ARRAY_SIZE];
/** allocation memory for stream rings */
extern USBSSP_RingElementT
    g_streamRing[USBSSP_MAX_EP_NUM_STRM_EN][USBSSP_STREAM_ARRAY_SIZE]
                [USBSSP_PRODUCER_QUEUE_SIZE] __attribute__((aligned(1024)));
#endif

extern USBSSP_XhciResourcesT g_xhciMemRes;

/************************** Function Prototypes ******************************/

void xhciEpRingFlush(void);

void xhciEventRingFlush(void);

void xhciDCBAAFlush(void);

void xhciInputContextFlush(void);

void xhciOutputContextFlush(void);

void xhciEvntRingSegmentFlush(void);

u32 assignXhciMemory(void);

#endif
