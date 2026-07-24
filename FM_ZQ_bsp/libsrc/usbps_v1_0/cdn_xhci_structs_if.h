/* parasoft suppress item  MISRA2012-DIR-4_8 "Consider hiding implementation of
 * structure" */
/**********************************************************************
 * Copyright (C) 2014-2021 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 **********************************************************************
 * WARNING: This file is auto-generated using api-generator utility.
 *          api-generator: 12.02.13bb8d5
 *          Do not edit it manually.
 **********************************************************************
 * XHCI driver for both host and device mode header file
 **********************************************************************/
#ifndef CDN_XHCI_STRUCTS_IF_H
#define CDN_XHCI_STRUCTS_IF_H

#include "cdn_stdtypes.h"
#include "cdn_xhci_if.h"

/** @defgroup DataStructure Dynamic Data Structures
 *  This section defines the data structures used by the driver to provide
 *  hardware information, modification and dynamic operation of the driver.
 *  These data structures are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
 * Structures and unions
 **********************************************************************/
/** Transfer request block structure, spec 4.11.1 */
struct USBSSP_RingElementT_s {
    /** TRB parameter */
    uint32_t dword0;
    uint32_t dword1;
    /** TRB status */
    uint32_t dword2;
    /** TRB control */
    uint32_t dword3;
};

/** Input context structure */
struct USBSSP_InputContexT_s {
    /** Input control context structure */
    uint32_t inputControlContext[USBSSP_CONTEXT_WIDTH];
    /** Slot context structure */
    uint32_t slot[USBSSP_CONTEXT_WIDTH];
    /** Endpoint 0 context structure */
    uint32_t ep0Context[USBSSP_CONTEXT_WIDTH];
    /** Endpoints context structures */
    uint32_t epContext[USBSSP_MAX_EP_CONTEXT_NUM][USBSSP_CONTEXT_WIDTH];
} __attribute__((packed));

/** Output context structure */
struct USBSSP_OutputContexT_s {
    /** Slot context structure */
    uint32_t slot[USBSSP_CONTEXT_WIDTH];
    /** Endpoint 0 context structure */
    uint32_t ep0Context[USBSSP_CONTEXT_WIDTH];
    /** Endpoints context structures */
    uint32_t epContext[USBSSP_MAX_EP_CONTEXT_NUM][USBSSP_CONTEXT_WIDTH];
} __attribute__((packed));

/**
 * Structure describes element of producer queue in conjuction to associated
 * endpoint. Note that the same structure is used in command ring - in this
 * case, fields referred to endpoint are not used
 */
struct USBSSP_ProducerQueueT_s {
    /** Memory buffer for ring used by hardware */
    USBSSP_RingElementT* ring;
    /** Pointer to enqueued element */
    USBSSP_RingElementT* enqueuePtr;
    /** Pointer to dequeued element */
    USBSSP_RingElementT* dequeuePtr;
    /** Pointer to last completed queue element */
    USBSSP_RingElementT* completePtr;
    /** Keeps actual value of Cycle bit inserted to TRB */
    uint8_t toogleBit;
    /** Callback function called on TRB complete event */
    USBSSP_Complete complete;
    /** Callback function called on an aggregated transfer completion */
    USBSSP_Complete aggregatedComplete;
    /** Keeps Device context index value of endpoint associated with this object
     */
    uint8_t contextIndex;
    /**
     * Auxiliary flag, set to 1 when any TD associated with this object is
     * issued to DMA and flag is set to 0 on complete event
     */
    uint8_t isRunningFlag;
    /** Flag is active when endpoint is in stopped state */
    uint8_t isDisabledFlag;
    /** Keeps copy of endpoint descriptor of associated endpoint */
    uint8_t epDesc[CH9_USB_DS_ENDPOINT + CH9_USB_DS_SS_USB_EP_COMPANION +
                   CH9_USB_DS_SSP_ISO_EP_COMPANION];
    /** Number of bytes for last transfer */
    uint32_t numOfBytes;
    /** Number of residue bytes of last transfer */
    uint32_t numOfResidue;
    /** Buffer address for last successful TD */
    uint64_t lastXferBufferPhyAddr;
    /** Number of bytes actually transferred */
    uint32_t lastXferActualLength;
    /** Completion code of last transfer */
    uint8_t completionCode;
    /** Interrupter Index of the target interrupter */
    uint32_t interrupterIdx;
    /** points to the first queuued TRB in a Ring */
    USBSSP_RingElementT* firstQueuedTRB;
    /** points to the last queuued TRB in a Ring */
    USBSSP_RingElementT* lastQueuedTRB;
    uint8_t extraFlags;
    uint32_t frameID;
    /** Streams container */
    USBSSP_ProducerQueueT* stream[USBSSP_STREAM_ARRAY_SIZE];
    /** Number of streams available */
    uint32_t streamCount;
    /** Keeps actually selected stream ID */
    uint16_t actualSID;
    /** Keeps stream ID of the last event received from HW */
    uint16_t eventSID;
    /** Points to hardware endpoint context according to spec 6.2.3 */
    uint32_t* hwContext;
    /** Points to this object owner */
    USBSSP_DriverResourcesT* parent;
    /** Blocks calling complete callback when set to 1 */
    uint8_t ignoreShortPacket;
    /** First prime determinant */
    uint8_t first_prime_det;
    /** Determines if stream is active */
    uint8_t stream_active;
    /** Determines if stream is rejected */
    uint8_t stream_rejected;
    /** Determines if there is a pending request */
    USBSSP_ReqState req_pending;
    /** Number of doorbells rung */
    uint8_t drbls_count;
};

/**
 * structure keeps all descriptors pointers for USBSSP operating in device mode
 * index of array correspond to USB speed: devDesc[1] - low speed, devDesc[2] -
 * full speed arrays elements indexed 0 should be set to NULL
 */
struct USBSSP_DescT_s {
    /** Table keeps pointers to device descriptors */
    uint8_t* devDesc[USBSSP_MAX_SPEED];
    /** Table keeps pointers to configuration descriptors */
    uint8_t* confDesc[USBSSP_MAX_SPEED];
    /** Table keeps pointers to BOS descriptors */
    uint8_t* bosDesc[USBSSP_MAX_SPEED];
    /** Table keeps pointers to string descriptors */
    uint8_t* string[USBSSP_MAX_STRING_NUM];
};

/** Device context base array pointer structure */
struct USBSSP_DcbaaT_s {
    /** Address of scratch pad pointers container */
    uint64_t scratchPadPtr;
    /** Address of device slot 1 */
    uint64_t deviceSlot[USBSSP_MAX_DEVICE_SLOT_NUM];
};

/**
 * Structure represents USB SSP memory required by XHCI specification.
 * That structure has to be provided by application and must be aligned to
 * USBSSP_PAGE_SIZE.
 */
struct USBSSP_XhciResourcesT_s {
    /** event ring segment entry */
    USBSSP_RingElementT* epRingPool;
    /** Event Ring */
    USBSSP_RingElementT* eventPool;
    /** Device context base array structure */
    USBSSP_DcbaaT* dcbaa;
    /** Input context structure */
    USBSSP_InputContexT* inputContext;
    /** Output context structure */
    USBSSP_OutputContexT* outputContext;
    /** Scratchpad buffers (extra element for last pointer = NULL) */
    uint64_t* scratchpad;
    /** Scratchpad buffers pool */
    uint8_t* scratchpadPool;
    /** event ring segment entry */
    uint64_t* eventRingSegmentEntry;
    /** allocated memory for stream objects */
    USBSSP_ProducerQueueT (
        *streamMemoryPool)[USBSSP_MAX_EP_NUM_STRM_EN][USBSSP_STREAM_ARRAY_SIZE];
    /** allocation memory for stream's rings */
    USBSSP_RingElementT (
        *streamRing)[USBSSP_MAX_EP_NUM_STRM_EN][USBSSP_STREAM_ARRAY_SIZE]
                    [USBSSP_PRODUCER_QUEUE_SIZE];
    /** Pointer to EP0 buffer of size USBSSP_EP0_DATA_BUFF_SIZE */
    uint8_t* ep0Buffer;
} __attribute__((packed));

/** USB SSP callbacks */
struct USBSSP_Callbacks_s {
    /** Used for testing purposes */
    USBSSP_GenericCallback genericCallback;
    /** Used for testing purposes */
    USBSSP_PostCallback postCallback;
    /** Used for testing purposes */
    USBSSP_PreportChangeDetectCallback preportChangeDetectCallback;
    /** Used for testing purposes */
    USBSSP_InputContextCallback inputContextCallback;
    /** Used for testing purposes */
    USBSSP_SetInterfaceCallback setInterfaceCallback;
};

/**
 * Structure represents USB SSP controller resources.
 * That structure must be aligned to USBSSP_PAGE_SIZE because of xhciResources.
 */
struct USBSSP_DriverResourcesT_s {
    /** Structure represents USB SSP memory required by XHCI specification. */
    USBSSP_XhciResourcesT* xhciMemRes;
    /** Input context structure */
    USBSSP_InputContexT* inputContext;
    /** Input context copy structure */
    USBSSP_InputContexT inputContextCopy;
    /** Pointer to actual event ring element */
    USBSSP_RingElementT* eventPtr;
    /** Keeps actual value of cycle bit for event ring */
    uint8_t eventToogleBit;
    /** Command queue object */
    USBSSP_ProducerQueueT commandQ;
    /** Default endpoint queue object */
    USBSSP_ProducerQueueT ep0;
    /** Container of non default endpoint objects */
    USBSSP_ProducerQueueT
        ep[(USBSSP_MAX_EP_CONTEXT_NUM + USBSSP_EP_CONT_OFFSET)];
    /** Keeps actual speed the port operate in */
    CH9_UsbSpeed actualSpeed;
    /** Keeps port ID */
    uint8_t actualPort;
    /** Keeps actual device slot, when USBSSP works in device mode it is 1 */
    uint8_t actualdeviceSlot;
    /** Indicates whether ENABLE_SLOT_COMMAND was sent but slot ID has not been
     * set yet */
    uint8_t enableSlotInProgress;
    /** Indicates whether device disconnect is pending */
    uint8_t devDisconnectCBPendingFlag;
    /** Flag is active when USB SSP is in configured state */
    uint8_t devConfigFlag;
    /** 1 - USB SSP works in device mode, 0 - host mode */
    uint8_t deviceModeFlag;
    /** Indicates USB mode (2 - forced USB2 mode, 3 - forced USB3 mode, others -
     * default) */
    uint8_t usbModeFlag;
    /** When set to 1 indicates initializing will be performed only on connected
     * device, not on SSP controller itself */
    uint8_t noControllerSetup;
    /** Extended TBC enable mode if ETC enabled (0: ETE always disabled, 1:ETE
     * enabled for all speeds) */
    uint8_t extendedTBCMode;
    /** Internal buffer for control transfer */
    uint8_t* ep0Buff;
    /** Pointer to device descriptors container */
    USBSSP_DescT* devDesc;
    /** Stores actual value of context Entry */
    uint32_t contextEntries;
    /** Reflects actual state of Current Connect Status of PORTSC register, 0 -
     * disconnected, 1 - connected */
    uint8_t connected;
    /** Local copy of some registers - for quick access */
    USBSSP_QuickAccessRegs qaRegs;
    /** Keeps addresses of all SFR's */
    USBSSP_SfrT regs;
    /** NOP complete callback function, diagnostic function */
    USBSSP_NopComplete nopComplete;
    /** NOP complete callback function, diagnostic function */
    USBSSP_ForceHeaderComplete forceHeaderComplete;
    /**
     * Mask with enabled endpoints for last SET_CONFIGURATION request
     * \remarks Follows InputControlContext.DWORD1
     */
    uint32_t enabledEndpsMask;
    /** The maximum number of Device Context Structures and Doorbell Array
     * entries this controller can support */
    uint32_t maxDeviceSlot;
    /** flag active when setup packet received and enable_slot command isn't
     * completed yet, it delegates setup handling to enable slot command
     * completion */
    USBSSP_Ep0StateEnum ep0State;
    /**
     * Keep device address sent during SET_ADDRESS setup request - is active
     * only in device mode
     */
    uint8_t devAddress;
    /** Keeps setupId value of actually handled setup packet */
    uint8_t setupID;
    /** stores the setup request currently being handled in device mode */
    __attribute__ ((aligned(32))) CH9_UsbSetup devSetupReq;
    /** Keeps index of non zero endpoint which generated latest interrupt */
    uint8_t lastEpIntIndex;
    /** Keeps index of instance associated with this resource */
    uint8_t instanceNo;
    /** Device configuration value, used in host mode */
    uint16_t configValue;
    /** Keeps reference to CUSBD callback object */
    CUSBD_Callbacks* cusbdCallbacks;
    CUSBD_PrivateData* privateData;
    /**  USBSSP Call backs registered in host mode  */
    USBSSP_Callbacks usbsspCallbacks;
};

/** This Structure holds paramters needed for initial configuration of the
 * driver.. */
struct USBSSP_DriverConfigT_s {
    /** Structure represents USB SSP memory required by XHCI specification. If
     * NULL, driver should be enabled to perform device memory allocation. */
    USBSSP_XhciResourcesT* xhciMemRes;
    /** Base address of the OTG registers. */
    uintptr_t otgRegs;
    /** Base address of the device registers. If NULL then this is calculated
     * using OTG base address. */
    uintptr_t deviceRegs;
    /** Base address of the host registers. If NULL then this is calculated
     * using OTG base address. */
    uintptr_t hostRegs;
    /** Indicates whether XHCI interface is acting in device mode */
    uint8_t deviceModeFlag;
};

/** This Structure holds the saved USB context, which can be used for restore.
 */
struct USBSSP_DriverContextT_s {
    /** USB Command Register */
    uint32_t usbcmd;
    /** Device Notification Control Register */
    uint32_t dnctrl;
    /** Device Context Base Address Array Pointer Register */
    uint64_t dcbaap;
    /** Configure Register */
    uint32_t config;
    /** Interrupter Management */
    uint32_t iman;
    /** Interrupter Moderation */
    uint32_t imod;
    /** Event Ring Segment Table Size */
    uint32_t erstsz;
    /** Event Ring Segment Table Base Address */
    uint64_t erstba;
    /** Event Ring Dequeue Pointer */
    uint64_t erdp;
};

struct USBSSP_dword_s {
    uint32_t dword0;
    uint32_t dword1;
    uint32_t dword2;
    uint32_t dword3;
} __attribute__((packed));

struct USBSSP_param_s {
    const uintptr_t* buffVec;
    const uint32_t* sizeVec;
    uint32_t sgElementsSum;
} __attribute__((packed));

/**
 *  @}
 */

#endif /* CDN_XHCI_STRUCTS_IF_H */
