#include <string.h>
#include "fmsh_usb_data.h"

#ifdef NONE_CACHE_USB_DATA
/** event ring segment entry */
USBSSP_RingElementT
    g_epRingPool[USBSSP_PRODUCER_QUEUE_SIZE * (USBSSP_MAX_EP_CONTEXT_NUM + 2U)]
    __attribute__((section(".ddr_noncache_data"), aligned(1024)));
/** Event Ring */
USBSSP_RingElementT g_eventPool[USBSSP_EVENT_QUEUE_SIZE]
    __attribute__((section(".ddr_noncache_data"), aligned(1024)));

/** Device context base array structure */
USBSSP_DcbaaT g_dcbaa __attribute__((section(".ddr_noncache_data"),
                                     aligned(USBSSP_DCBAA_ALIGNMENT)));

/** Input context structure */
USBSSP_InputContexT g_inputContext
    __attribute__((section(".ddr_noncache_data"), aligned(USBSSP_PAGE_SIZE)));
/** Output context structure */
USBSSP_OutputContexT g_outputContext
    __attribute__((section(".ddr_noncache_data"), aligned(USBSSP_PAGE_SIZE)));
/** Scratch pad buffers (extra element for last pointer = NULL) */
uint64_t g_scratchpad[USBSSP_SCRATCHPAD_BUFF_NUM + 1U]
    __attribute__((section(".ddr_noncache_data"), aligned(USBSSP_PAGE_SIZE)));
/** Scratch pad buffers pool */
uint8_t g_scratchpadPool[USBSSP_SCRATCHPAD_BUFF_NUM * USBSSP_PAGE_SIZE]
    __attribute__((section(".ddr_noncache_data"), aligned(USBSSP_PAGE_SIZE)));
/** event ring segment entry */
uint64_t g_eventRingSegmentEntry[USBSSP_INTERRUPTER_COUNT * 2U] __attribute__((
    section(".ddr_noncache_data"), aligned(USBSSP_ERST_ALIGNMENT)));
/** allocated memory for stream objects */
USBSSP_ProducerQueueT g_streamMemoryPool[USBSSP_MAX_EP_NUM_STRM_EN]
                                        [USBSSP_STREAM_ARRAY_SIZE];
/** allocation memory for stream rings */
USBSSP_RingElementT g_streamRing[USBSSP_MAX_EP_NUM_STRM_EN]
                                [USBSSP_STREAM_ARRAY_SIZE]
                                [USBSSP_PRODUCER_QUEUE_SIZE]
    __attribute__((section(".ddr_noncache_data"), aligned(1024)));
/** EP0 buffer */
uint8_t g_ep0Buffer[USBSSP_EP0_DATA_BUFF_SIZE]
    __attribute__((section(".ddr_noncache_data"), aligned(64)));
///** EP0 resp buffer */
//uint8_t g_ep0ResBuffer[USBSSP_EP0_DATA_BUFF_SIZE]
//    __attribute__((section(".ddr_noncache_data"), aligned(64)));
///** EP0 buffer */
//uint8_t g_AppEp0Buffer[USBSSP_EP0_DATA_BUFF_SIZE]
//    __attribute__((section(".ddr_noncache_data"), aligned(64)));
//
///** msc cmd buffer */
//uint8_t g_mscCmdBuffer[512]
//    __attribute__((section(".ddr_noncache_data"), aligned(64)));
///** msc resp buffer */
//uint8_t g_mscRespBuffer[512]
//    __attribute__((section(".ddr_noncache_data"), aligned(64)));
//
//uint8_t g_scsiRespBuffer[1024]
//    __attribute__((section(".ddr_noncache_data"), aligned(64)));
#endif

USBSSP_XhciResourcesT g_xhciMemRes;


void* allocate_xhci_data_address(u32 start_addr,u32 length,u32 alignment)
{
    uintptr_t aligned_address= (start_addr+alignment-1) & ~(alignment-1);
    void *allocated_memory=NULL;
    
    if(aligned_address+length>0x7FFFFFFF){
        return NULL;
    }
    
    allocated_memory=(void *)aligned_address;
    
    (void)memset(allocated_memory,0,length);
    
    return allocated_memory;
}



void xhciEpRingFlush(void)
{
#ifdef CACHE_USB_DATA
    Fmsh_DCacheFlushRange((uintptr_t)(g_xhciMemRes.epRingPool),USBSSP_PRODUCER_QUEUE_SIZE * (USBSSP_MAX_EP_CONTEXT_NUM + 2U)*sizeof(USBSSP_RingElementT));
#endif
}

void xhciEventRingFlush(void)
{
#ifdef CACHE_USB_DATA  
    Fmsh_DCacheFlushRange((uintptr_t)(g_xhciMemRes.eventPool),USBSSP_EVENT_QUEUE_SIZE*sizeof(USBSSP_RingElementT));
    Fmsh_DCacheFlushRange((uintptr_t)(g_xhciMemRes.eventRingSegmentEntry),sizeof(uint64_t)*USBSSP_INTERRUPTER_COUNT * 2U);
#endif
}

void xhciDCBAAFlush(void)
{
#ifdef CACHE_USB_DATA  
    Fmsh_DCacheFlushRange((uintptr_t)(g_xhciMemRes.dcbaa),sizeof(USBSSP_DcbaaT));
#endif    
}

void xhciInputContextFlush(void)
{
#ifdef CACHE_USB_DATA  
    Fmsh_DCacheFlushRange((uintptr_t)(g_xhciMemRes.inputContext),sizeof(USBSSP_InputContexT)); 
#endif    
}

void xhciOutputContextFlush(void)
{
#ifdef CACHE_USB_DATA  
    Fmsh_DCacheFlushRange((uintptr_t)(g_xhciMemRes.outputContext),sizeof(USBSSP_OutputContexT)); 
#endif    
}

void xhciEvntRingSegmentFlush(void)
{
#ifdef CACHE_USB_DATA  
    Fmsh_DCacheFlushRange((uintptr_t)(g_xhciMemRes.eventRingSegmentEntry),sizeof(uint64_t)*USBSSP_INTERRUPTER_COUNT * 2U);

#endif
}


u32 assignXhciMemory (void)
{
#if (DCACHE_ENABLE == 1)
    u32 base_addr= USB_DATA_BASE;
    u32 size=0;
    
    size= USBSSP_PRODUCER_QUEUE_SIZE * (USBSSP_MAX_EP_CONTEXT_NUM + 2U) *sizeof(USBSSP_RingElementT);
    g_xhciMemRes.epRingPool = (USBSSP_RingElementT* )allocate_xhci_data_address(base_addr,size,1024);
    if(g_xhciMemRes.epRingPool==NULL){
        return FMSH_FAILURE;
    }
    
    base_addr = (uintptr_t)(g_xhciMemRes.epRingPool)+size;
    size=USBSSP_EVENT_QUEUE_SIZE*sizeof(USBSSP_RingElementT);
    g_xhciMemRes.eventPool = (USBSSP_RingElementT* )allocate_xhci_data_address(base_addr,size,1024);
    if(g_xhciMemRes.eventPool==NULL){
        return FMSH_FAILURE;
    }
    
    base_addr = (uintptr_t)(g_xhciMemRes.eventPool)+size;
    size = sizeof(USBSSP_DcbaaT);
    g_xhciMemRes.dcbaa = (USBSSP_DcbaaT* )allocate_xhci_data_address(base_addr,size,USBSSP_DCBAA_ALIGNMENT);
    if(g_xhciMemRes.dcbaa == NULL)
        return FMSH_FAILURE;
    
    base_addr = (uintptr_t)(g_xhciMemRes.dcbaa)+size;
    size = sizeof(USBSSP_InputContexT);
    g_xhciMemRes.inputContext = (USBSSP_InputContexT* )allocate_xhci_data_address(base_addr,size,USBSSP_PAGE_SIZE);
    if(g_xhciMemRes.inputContext == NULL){
        return FMSH_FAILURE;
    }
    
    base_addr = (uintptr_t)(g_xhciMemRes.inputContext)+size;
    size = sizeof(USBSSP_OutputContexT);
    g_xhciMemRes.outputContext = (USBSSP_OutputContexT* )allocate_xhci_data_address(base_addr,size,USBSSP_PAGE_SIZE);
    if(g_xhciMemRes.outputContext == NULL){
        return FMSH_FAILURE;
    }
    
    base_addr = (uintptr_t)(g_xhciMemRes.outputContext)+size;
    size = sizeof(uint64_t)*(USBSSP_SCRATCHPAD_BUFF_NUM + 1);
    g_xhciMemRes.scratchpad = (uint64_t* )allocate_xhci_data_address(base_addr,size,USBSSP_PAGE_SIZE);
    if(g_xhciMemRes.scratchpad == NULL){
        return FMSH_FAILURE;
    }
    
    base_addr = (uintptr_t)(g_xhciMemRes.scratchpad)+size;
    size = sizeof(uint64_t)*USBSSP_INTERRUPTER_COUNT * 2U;
    g_xhciMemRes.eventRingSegmentEntry = (uint64_t* )allocate_xhci_data_address(base_addr,size,USBSSP_ERST_ALIGNMENT);
    if(g_xhciMemRes.eventRingSegmentEntry == NULL){
        return FMSH_FAILURE;
    }
    
    base_addr = (uintptr_t)(g_xhciMemRes.eventRingSegmentEntry)+size;
    size = USBSSP_EP0_DATA_BUFF_SIZE;
    g_xhciMemRes.ep0Buffer = (uint8_t* )allocate_xhci_data_address(base_addr,size,64);
    if(g_xhciMemRes.ep0Buffer == NULL){
        return FMSH_FAILURE;
    }
    
#else    
    g_xhciMemRes.epRingPool =
        g_epRingPool;  //(USBSSP_RingElementT*)
                       // aligned_alloc(16,USBSSP_EVENT_QUEUE_SIZE*16);//devMemoryBlocks[MEMBLK_EPRINGPOOL_OFFSET].virAddress;
    g_xhciMemRes.eventPool = g_eventPool;
    g_xhciMemRes.dcbaa = &g_dcbaa;
    g_xhciMemRes.inputContext = &g_inputContext;
    g_xhciMemRes.outputContext = &g_outputContext;
    g_xhciMemRes.scratchpad = g_scratchpad;
    g_xhciMemRes.eventRingSegmentEntry = g_eventRingSegmentEntry;
    g_xhciMemRes.ep0Buffer = g_ep0Buffer;
#endif
    
    return FMSH_SUCCESS;
}
