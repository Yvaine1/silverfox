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
 * Layer interface for the Cadence USB device controller's family
 **********************************************************************/
#ifndef CUSBD_STRUCTS_IF_H
#define CUSBD_STRUCTS_IF_H

#include "cdn_stdtypes.h"
#include "cusbd_if.h"

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
/**
 * Configuration of device.
 * Object of this type is used for probe and init functions when checking
 * memory requirements of device object and for hardware controller
 * configuration. Size of device object in bytes is returned on probe function
 * call in CUSBD_SysReq.privDataSize field. Remember that privDataSize returns
 * size of device object only - it doesn't endpoints vector object size. It
 * means that whole memory requirements = device size + (number of endpoints x
 * single endpoint size)
 */
struct CUSBD_Config_s {
    /** base address of device controller registers */
    uintptr_t regBase;
    /** Extended TBC enable mode if ETC enabled (0: ETE always disabled, 1:ETE
     * enabled for all speeds) */
    uint8_t extendedTBCMode;
    /** Forced USB mode (0 - none (default), 2 - USB2/HS, 3 - USB3/SS, other -
     * reserved) */
    uint8_t forcedUsbMode;
    /** Pointer to buffer used for setup response */
    uint8_t* ep0RespBuffer;
    /** Structure represents USBSSP XHCI driver configuration. */
    USBSSP_DriverConfigT xhciConfig;
};

/** System requirements returned by probe */
struct CUSBD_SysReq_s {
    /** size of memory required for driver's private data */
    uint32_t privDataSize;
    /** size of memory required for endpoint object */
    uint32_t epObjSize;
    /**
     * size of memory required for USB Transfer Request Descriptors.
     * This memory will be used by DMA controller,
     * so it should be suitable for DMA operation.
     */
    uint32_t trbMemSize;
};

/**
 * structure helpful when object are organized in  linked list
 * Device contains list of endpoints, in turn: endpoint contains request list
 */
struct CUSBD_ListHead_s {
    /** pointer to next element in list */
    CUSBD_ListHead* next;
    /** pointer to previous element in list */
    CUSBD_ListHead* prev;
};

/**
 * structure contains information about memory slices for scatter/gather
 * transfer
 */
struct CUSBD_SgList_s {
    /** link to the next elemen */
    uintptr_t link;
    /** offset within memory page */
    uint32_t offset;
    /** data length of this transfer slice */
    uint32_t length;
    /** physical address of buffer to read/write */
    uintptr_t dmaAddress;
};

/**
 * Transfer request structure.
 * I/O transfers of higher layers are realized with the help of CUSBD_Req
 * objects. CUSBD_Req is associated with endpoints.
 */
struct CUSBD_Req_s {
    /** pointer to the previous request in the linked list */
    CUSBD_Req* prevReq;
    /** pointer to the next request in the linked list */
    CUSBD_Req* nextReq;
    /** buffer with data to transfer - virtual address */
    uint8_t* buf;
    /** This transfer data length */
    uint32_t length;
    /** physical address of buf buffer */
    uintptr_t dma;
    /** scatter/gather list */
    CUSBD_SgList* sg;
    /** number of scatter/gather entries */
    uint32_t numOfSgs;
    /** number of scatter/gather entries mapped for hardware controller */
    uint32_t numMappedSgs;
    /** stream id - used only in Super Speed mode */
    uint16_t streamId;
    /** Setting this flag to 1 disables generating of transfer completion
     * interrupt */
    uint8_t noInterrupt;
    /**
     * Setting this flag to 1 causes adding zero length data packet at the
     * end of data transfer when data transfer length is a multiple of
     * MaxPacketSize for endpoint
     */
    uint8_t zero;
    /**
     * Setting this flag causes all reading transfers with short packet as
     * erroneous
     */
    uint8_t shortNotOk;
    /** Callback function called on the transfer completion event */
    CUSBD_CbReqComplete complete;
    /**
     * general-use data hook
     * driver doesn't make use of this field. May be dsed in complete
     * callback function
     */
    void* context;
    /** keeps actual status of request */
    uint32_t status;
    /** number of actually processed bytes */
    uint32_t actual;
    /** USBSSP controller testing extension functions */
    uint8_t noDorbell;
    uint8_t forceHeaderCommandFlag;
    uint32_t eventDataFlags;
    uint32_t frameID;
};

/** Set of functions allowing to operate on endpoints */
struct CUSBD_EpOps_s {
    CUSBD_CbEpEnable epEnable;
    CUSBD_CbEpDisable epDisable;
    CUSBD_CbEpSetHalt epSetHalt;
    CUSBD_CbEpSetWedge epSetWedge;
    CUSBD_CbEpFifoStatus epFifoStatus;
    CUSBD_CbEpFifoFlush epFifoFlush;
    CUSBD_CbReqQueue reqQueue;
    CUSBD_CbReqDequeue reqDequeue;
};

/** Structure represents device USB endpoint */
struct CUSBD_Ep_s {
    /** enables organization of endpoints in linked list */
    CUSBD_ListHead epList;
    /** not used by CUSBD, used by higher layers */
    void* driverData;
    /** pointer to the private data of this endpoint */
    CUSBD_EpPrivate* epPrivate;
    /** keeps name of endpoint */
    char name[100];
    /**
     * endpoint driver, contains methods for all operation required on
     * endpoint
     */
    const CUSBD_EpOps* ops;
    /**
     * Endpoint address, the oldest bit refers to endpoint direction.
     * For example:
     * address = 0x82 refers to endpoint 2 IN
     * address = 0x03 refers to endpoint 3 OUT
     */
    uint8_t address;
};

/** Structure represents USB device */
struct CUSBD_Dev_s {
    /** allows for manipulation on endpoints organized as linked list */
    CUSBD_ListHead epList;
    /**
     * pointer to bidirectional default endpoint, this endpoint should
     * be always available after driver start
     */
    CUSBD_Ep* ep0;
    /** speed value in which device actually works */
    CH9_UsbSpeed speed;
    /** maximal speed the device is able to work */
    CH9_UsbSpeed maxSpeed;
    /** actual state in which device actually works */
    CH9_UsbState state;
    /** Flags informs if device is scatter/gather transfer capable */
    uint8_t sgSupported;
    /** Full name of USB device controller */
    char name[100];
};

/**
 * struct containing function pointers for event notification callbacks issued
 * by isr().
 * Each call passes the driver's privateData pointer for instance
 * identification if necessary, and may also pass data related to the event.
 */
struct CUSBD_Callbacks_s {
    CUSBD_CbDisconnect disconnect;
    CUSBD_CbConnect connect;
    CUSBD_CbSetup setup;
    CUSBD_CbConfigured configured;
    CUSBD_CdSuspend suspend;
    CUSBD_CbResume resume;
    CUSBD_CbbusInterval busInterval;
};

struct CUSBD_EpPrivate_s {
    CUSBD_Req* headReq;
    CUSBD_Req* streamHeadReq[USBSSP_STREAM_ARRAY_SIZE];
    CUSBD_Ep ep;
    CUSBD_epState ep_state;
    uint8_t wedgeFlag;
};

/**
 * Driver private data. That structure has to be provided by application
 * and must be aligned to USBSSP_PAGE_SIZE. Because of boundary requirements
 * it is better to align it to USBSSP_RING_BOUNDARY.
 */
struct CUSBD_PrivateData_s {
    /** USB SSP controller resources including XHCI resources */
    USBSSP_DriverResourcesT xhciDriverResources;
    /** this 'class must implement' CUSBD class, this class is an extension of
     * CUSBD */
    CUSBD_Dev device;
    /** copy of user configuration */
    CUSBD_Config config;
    /** copy of user callback */
    CUSBD_Callbacks callbacks;
    /**
     * these variables reflect device state
     *
     * keeps U2 value
     */
    uint8_t u2_value;
    /** keeps U1 value */
    uint8_t u1_value;
    /** keeps suspend value */
    uint16_t suspend;
    uint16_t isoch_delay;
    /** bidirectional endpoint 0 */
    CUSBD_EpPrivate ep0;
    USBSSP_Ep0StateEnum ep0NextState;
    uint8_t ep0DataDirFlag;
    CUSBD_EpPrivate ep_in_container[16U];
    CUSBD_EpPrivate ep_out_container[16U];
    CUSBD_Callbacks internalCallbacks;
    uint8_t objOffset;
    /**  Pointer to the EP0 Response buffer */
    uint8_t* ep0RespBuff;
};

/**
 *  @}
 */

#endif /* CUSBD_STRUCTS_IF_H */
