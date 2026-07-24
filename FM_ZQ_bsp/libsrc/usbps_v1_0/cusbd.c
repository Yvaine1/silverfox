/******************************************************************************
 *
 * Copyright (C) 2014-2020 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 *
 ******************************************************************************
 * cusbd.c
 * Cadence-USB-Device interface implementation
 *
 * Main source file
 *****************************************************************************/
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "cdn_errno.h"
#include "cdn_log.h"
#include "trb.h"
#include "cps.h"
#include "cusbd_if.h"
#include "cusbd_structs_if.h"
#include "cusbd_sanity.h"
#include "cusb_ch9_if.h"
#include "cusb_ch9_structs_if.h"
/* Local Defines */
#define EP0_ADDRESS            0x80
#define CUSBSS_U1_DEV_EXIT_LAT 4
#define CUSBSS_U2_DEV_EXIT_LAT 512

/* Forward declaration of local and external functions */
USBSSP_DriverResourcesT *get_USBSSP_Obj(CUSBD_PrivateData *pD);
static uint32_t setupDevice(CUSBD_PrivateData *pD, CH9_UsbSetup *ctrl);
static void transferComplete(USBSSP_DriverResourcesT *arg);
static uint32_t CUSBD_RunHwTransfer(USBSSP_DriverResourcesT *sspRes,
                                    const CUSBD_Ep *ep, CUSBD_Req *req);

/**
 * Endpoint operation functions
 */
static const CUSBD_EpOps epOps = {
    CUSBD_EpEnable,     CUSBD_EpDisable,   CUSBD_EpSetHalt, CUSBD_EpSetWedge,
    CUSBD_EpFifoStatus, CUSBD_EpFifoFlush, CUSBD_ReqQueue,  CUSBD_ReqDequeue,
};

#ifdef DEBUG

static const char *get_dev_state_str (CH9_UsbState state)
{
    static const char *s_dev_state_str[8] = {
        [CH9_USB_STATE_NONE] = "None",
        [CH9_USB_STATE_ATTACHED] = "Attached",
        [CH9_USB_STATE_POWERED] = "Powered",
        [CH9_USB_STATE_DEFAULT] = "Default",
        [CH9_USB_STATE_ADDRESS] = "Address",
        [CH9_USB_STATE_CONFIGURED] = "Configured",
        [CH9_USB_STATE_SUSPENDED] = "Suspended",
        [CH9_USB_STATE_ERROR] = "Error"};

    const char *state_str = (((uint32_t)state < (sizeof(s_dev_state_str) /
                                                 sizeof(s_dev_state_str[0]))))
                                ? s_dev_state_str[state]
                                : NULL;

    return (state_str != NULL) ? state_str : "<invalid>";
}
#endif

/**
 * Constructs a uint32_t value from uint8_t pointer
 *
 * @param dataPtr Pointer to uint8 buffer
 * @return uint32_t value constructed from uint8 buffer
 */
static inline uint32_t getU32ValFromU8Ptr (const uint8_t *dataPtr)
{
    // Constructs a uint32_t value from uint8_t pointer
#ifdef CPU_BIG_ENDIAN
    uint32_t value = (uint32_t)dataPtr[3];
    uint32_t byte0 = (uint32_t)dataPtr[0] << 24U;
    uint32_t byte1 = (uint32_t)dataPtr[1] << 16U;
    uint32_t byte2 = (uint32_t)dataPtr[2] << 8U;

    value += byte0 | byte1 | byte2;
    return value;
#else
    uint32_t value = (uint32_t)dataPtr[0];
    uint32_t byte3 = (uint32_t)dataPtr[3] << 24U;
    uint32_t byte2 = (uint32_t)dataPtr[2] << 16U;
    uint32_t byte1 = (uint32_t)dataPtr[1] << 8U;

    value += byte1 | byte2 | byte3;
    return value;
#endif
}

/*
 * Returns XHCI endpoint index respectively to given endpoint address
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static uint8_t getEpIndex (uint8_t epAddress)
{
    uint8_t epNum = epAddress & 0x7FU;
    // check if IN endpoint
    uint8_t epIdx = ((epNum * 2U) + (((epAddress & 0x80U) != 0U) ? 1U : 0U));

    if (epIdx == 0U)
    {
        epIdx = 1U;
    }

    return epIdx;
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Function returns bitwise Right shifted epIndex
 */
static uint8_t getEpNum (uint8_t epIndex) { return (epIndex >> 1); }

/*
 * Returns endpoint direction respectively to given XHCI endpoint index
 * 1 - IN, 0 - OUT
 */
static uint8_t getEpDir (uint8_t epIndex) { return (epIndex & 1U); }

/*Function initialize list*/
static inline void listInit (CUSBD_ListHead *list)
{
    list->next = list;
    list->prev = list;
}

/**
 * Function adds a new element to the list between prev and next
 * parameters CUSBD_ListHead where the element needs to be added
 */
static inline void listAddElement (CUSBD_ListHead *newList, CUSBD_ListHead *prevList,
                                   CUSBD_ListHead *nextList)
{
    nextList->prev = newList;
    newList->next = nextList;
    newList->prev = prevList;
    prevList->next = newList;
}
/**
 * Function adds new entry before a specified head
 */
static inline void listAddTail (CUSBD_ListHead *newList, CUSBD_ListHead *head)
{
    listAddElement(newList, head->prev, head);
}

#ifdef DEBUG
/**
 * Function adds new entry after a specified head
 */
static inline void listAdd (CUSBD_ListHead *newList, CUSBD_ListHead *head)
{
    listAddElement(newList, head, head->next);
}

/**
 * Function removes a list
 */
static inline void listDelete (CUSBD_ListHead *list)
{
    list->next->prev = list->prev;
    list->prev->next = list->next;

    list->prev = NULL;
    list->next = NULL;
}

static inline uint8_t isListEmpty (const CUSBD_ListHead *list)
{
    uint8_t empty;

    // Check if list is empty
    if (list->next == list)
    {
        empty = 1U;
    }
    else
    {
        empty = 0U;
    }
    return empty;
}

static void displayReq (const CUSBD_Req *req, const char *reqState)
{
    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
            "%s Req(%p) { buf=%p dma=0x%LX length=%d actual=%d status=%d "
            "numOfSgs=%d numMappedSgs=%d "
            "streamId=%d noInterrupt=%d noDorbell=%d context=%p}\n",
            reqState, req, req->buf, req->dma, req->length, req->actual,
            req->status, req->numOfSgs, req->numMappedSgs, req->streamId,
            req->noInterrupt, req->noDorbell, req->context);
}
#endif

/**
 * Function adds the item at tail of doubly-linked list
 */
static inline void reqListAddTail (CUSBD_Req **headReq, CUSBD_Req *item)
{
    // Check for an empty list
    if (*headReq == NULL)
    {
        *headReq = item;
        item->nextReq = item;
        item->prevReq = item;
    }
    else
    {
        CUSBD_Req *last = (*headReq)->prevReq;
        item->nextReq = *headReq;
        item->prevReq = last;
        last->nextReq = item;
        (*headReq)->prevReq = item;
    }
}

/**
 * Function deletes the corresponding item
 */
static inline void reqListDeleteItem (CUSBD_Req **headReq, CUSBD_Req *item)
{
    // If there is only one item in the list
    if ((item->nextReq == NULL) || (item->prevReq == NULL))
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
                "Warning: item already removed 0x%08X\n", (uintptr_t)item);
    }
    else if (item->nextReq == item)
    {
        *headReq = NULL;
    }
    // Adjustments done with next and previous items
    else
    {
        CUSBD_Req *lastReq = item->prevReq;
        CUSBD_Req *firstReq = item->nextReq;
        firstReq->prevReq = lastReq;
        lastReq->nextReq = firstReq;
        if (*headReq == item)
        {
            *headReq = firstReq;
        }
    }

    item->nextReq = NULL;
    item->prevReq = NULL;
}

/**
 * Function flush ep requests
 * @param res driver resources
 * @param epp pointer to ep data
 * @param epIndex endpoint index
 */
static inline void epFlushRequests (const USBSSP_DriverResourcesT *res,
                                    CUSBD_EpPrivate *epp, uint8_t epIndex)
{
    CUSBD_Req *head;
    uint16_t sid = res->ep[epIndex].eventSID;
    if (sid == 0U)
    {
        head = epp->headReq;
    }
    else
    {
        head = epp->streamHeadReq[sid - 1U];
    }

    while (head != NULL)
    {
        /* remove this item from the list */
        if (sid == 0U)
        {
            reqListDeleteItem(&epp->headReq, head);
        }
        else
        {
            reqListDeleteItem(&epp->streamHeadReq[sid - 1U], head);
        }

        // call complete callback if registered
        if (head->complete != NULL)
        {
            head->status = CDN_ECANCELED;
            head->complete(&epp->ep, head);
        }

        if (sid == 0U)
        {
            head = epp->headReq;
        }
        else
        {
            head = epp->streamHeadReq[sid - 1U];
        }
    }
}

/**
 * Function sets the device state to a new USB state
 */
static void setDevState (CUSBD_PrivateData *dev, CH9_UsbState new_state)
{
    CH9_UsbState prev_state = dev->device.state;

    if (new_state != prev_state)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "Device state changing: %s -> %s\n",
                get_dev_state_str(prev_state), get_dev_state_str(new_state));

        dev->device.state = new_state;
    }
}

/**
 * Function disconnects the device and set the state to Default
 * This callback is called even when Device Port is reset by the Host
 */
static void disconnectDevice (CUSBD_PrivateData *pD)
{
    uint8_t i;
    pD->device.speed = pD->xhciDriverResources.actualSpeed;

    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "Disconnected ...\n", 0);
    /* Set u1 u2 value to default value*/
    pD->u1_value = 0;
    pD->u2_value = 0;

    // Set state to default
    setDevState(pD, CH9_USB_STATE_DEFAULT);
    pD->callbacks.disconnect(pD);
    for (i = 0U; i < 15U; i++)
    {
        pD->ep_in_container[i].headReq = NULL;
        pD->ep_out_container[i].headReq = NULL;
    }
}

/**
 * Connect the device
 */
static void connectDevice (CUSBD_PrivateData *pD)
{
    pD->device.speed = pD->xhciDriverResources.actualSpeed;
    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "Connect at speed %d\n",
            pD->device.speed);
    if (pD->callbacks.connect != NULL)
    {
        pD->callbacks.connect(pD);
    }
}

/**
 * handle endpoint state change
 */
static void handleEpStateChange (CUSBD_PrivateData *pD, uint32_t enabledEpMask)
{
    if (pD->callbacks.configured != NULL)
    {
        pD->callbacks.configured(pD, enabledEpMask);
    }
}

/**
 * Display the parameters of endpoint
 * param[in] pointer to the endpoint
 * param[in] USB speed
 * param[in] pointer to the descriptor
 */
/* parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter ep not used in
 * function displayEp, DRV-5631" */
static void displayEp (const CUSBD_Ep *ep, CH9_UsbSpeed speed,
                       const uint8_t *desc)
{
    const uint8_t *compDesc = NULL;

    if (speed >= CH9_USB_SPEED_SUPER)
    {
        compDesc = &desc[CH9_USB_DS_ENDPOINT];
    }

    // display endpoint
    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
            "Ep { address=0x%02X "
            "desc=[%02X][%02X][%02X][%02X][%02X][%02X][%02X]}\n",
            ep->address, desc[0], desc[1], desc[2], desc[3], desc[4], desc[5],
            desc[6]);

    if (compDesc != NULL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
                "Ep_companion {[%02X][%02X][%02X][%02X][%02X][%02X]}\n",
                compDesc[0], compDesc[1], compDesc[2], compDesc[3], compDesc[4],
                compDesc[5]);
    }
}
/* parasoft-end-suppress MISRA2012-RULE-2_7-4 */

static uint32_t initSwEp (CUSBD_PrivateData *dev, uint8_t dir)
{
    uint32_t ret = CDN_EOK;
    uint8_t i = 0U;
    CUSBD_EpPrivate *epPriv;
    while ((i < (USBSSP_MAX_EP_CONTEXT_NUM / 2U)) && (ret == (uint32_t)CDN_EOK))
    {
        // add endpoint to endpoint list
        if (dir != 0U)
        {
            epPriv = &dev->ep_in_container[i];
        }
        else
        {
            epPriv = &dev->ep_out_container[i];
        }

        if (epPriv == NULL)
        {
            ret = CDN_EINVAL;
            break;
        }
        // clear endpoint object - this will ALSO set headReq pointers to NULL
        (void)memset(epPriv, 0, sizeof(CUSBD_EpPrivate));
        // set endpoint address
        epPriv->ep.address = ((dir != 0U) ? CH9_USB_EP_DIR_IN : 0U) | (i + 1U);
        // set endpoint pseudo-class public methods
        epPriv->ep.ops = &epOps;
        // set pointer to epPriv
        epPriv->ep.epPrivate = epPriv;
        // add endpoint to endpoint list
        listAddTail(&epPriv->ep.epList, &dev->device.epList);

        i++;
    }
    return ret;
}

/**
 * handle U1 feature
 * @param dev pointer to driver object
 * @param set 1 for set, 0 for clear
 * @return CDN_EOK for success, error code elsewhere
 */
static uint32_t handleFeatureDevU1 (CUSBD_PrivateData *dev, uint8_t set)
{
    uint32_t ret = CDN_EOK;
    CH9_UsbState state = dev->device.state;

    /* request only valid for configured device and check actual speed */
    if ((state != CH9_USB_STATE_CONFIGURED) ||
        (dev->device.speed < CH9_USB_SPEED_SUPER))
    {
        ret = CDN_EINVAL;
    }
    /* set U1 value as per get_status response */
    if (set != 0U)
    {
        dev->u1_value = CH9_USB_STS_DEV_U1_ENABLE;
    }
    else
    {
        dev->u1_value = 0U;
    }
    return ret;
}

/**
 * handle U2 feature
 * @param dev pointer to driver object
 * @param set 1 for set, 0 for clear
 * @return CDN_EOK for success, error code elsewhere
 */
static uint32_t handleFeatureDevU2 (CUSBD_PrivateData *dev, uint8_t set)
{
    uint32_t ret = CDN_EOK;
    CH9_UsbState state = dev->device.state;
    /* request only valid for configured device and check actual speed */
    if ((state != CH9_USB_STATE_CONFIGURED) ||
        (dev->device.speed < CH9_USB_SPEED_SUPER))
    {
        ret = CDN_EINVAL;
    }

    /* set U1 value as per get_status response */
    if (set != 0U)
    {
        dev->u2_value = CH9_USB_STS_DEV_U2_ENABLE;
    }
    else
    {
        dev->u2_value = 0U;
    }
    return ret;
}

/**
 * handle set/clear feature standard setup request for device recipient
 * @param dev pointer to driver object
 * @param ctrl pointer to setup object
 * @param set 1 for setting feature, 0 for clearing feature
 * @return CDN_EOK for success, error code elsewhere
 */
static uint32_t handleFeatureRecipientDevice (CUSBD_PrivateData *dev,
                                              const CH9_UsbSetup *ctrl,
                                              uint8_t set)
{
    uint32_t ret = CDN_EOK;
    uint32_t wValue = ctrl->wValue;

    switch (wValue)
    {
    case CH9_USB_FS_DEVICE_REMOTE_WAKEUP:
        break;

        /* handle U1 feature */
    case CH9_USB_FS_U1_ENABLE:
        ret = handleFeatureDevU1(dev, set);
        break;

        /* handle U2 feature */
    case CH9_USB_FS_U2_ENABLE:
        ret = handleFeatureDevU2(dev, set);
        break;

    case CH9_USB_FS_LTM_ENABLE:
        ret = CDN_EINVAL;
        break;

        /* handle test mode */
    case CH9_USB_FS_TEST_MODE:
        ret = CDN_EINVAL;
        break;

    default:
        ret = CDN_EINVAL;
        break;
    }
    return ret;
}

/**
 * handle set/clear feature standard setup request for recipient interface
 * @param dev pointer to driver object
 * @param ctrl pointer to setup object
 * @return CDN_EOK for success, error code elsewhere
 */
static uint32_t handleFeatureRecipientInterface (CUSBD_PrivateData *dev,
                                                 const CH9_UsbSetup *ctrl)
{
    uint32_t wIndex;
    uint32_t wValue;
    uint32_t ret = CDN_EOK;

    wValue = ctrl->wValue;
    wIndex = ctrl->wIndex;

    switch (wValue)
    {
    case CH9_USB_FS_FUNCTION_SUSPEND:
        // set feature
        if ((wIndex & CH9_USB_SF_LOW_PWR_SUSP_STATE) > 0U)
        {
            dev->suspend = (ctrl->wIndex & 0x0100U);
        }
        // break
        if ((wIndex & CH9_USB_SF_REMOTE_WAKE_ENABLED) > 0U)
        {
            break;
        }
        break;
    default:
        ret = CDN_EINVAL;
        break;
    }
    return ret;
}

/**
 * Feature handling
 */
static uint32_t handleFeatureRecipientEndpoint (CUSBD_PrivateData *dev,
                                                const CH9_UsbSetup *ctrl,
                                                uint8_t set)
{
    uint32_t ret = CDN_EINVAL;
    uint8_t epIndex = getEpIndex((uint8_t)ctrl->wIndex);
    CUSBD_EpPrivate *epp;

    /* get endpoint from endpoint container */
    if (getEpDir(epIndex) != 0U)
    {
        epp = &dev->ep_in_container[getEpNum(epIndex) - 1U];
    }
    else
    {
        epp = &dev->ep_out_container[getEpNum(epIndex) - 1U];
    }

    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "Handle Feature (%s) on EP%0d_%s\n",
            (set > 0) ? "set" : "clear", ctrl->wIndex & 0x7FU,
            (((ctrl->wIndex & 0x80U) != 0U) ? "IN" : "OUT"));

    switch (ctrl->wValue)
    {
    case CH9_USB_FS_ENDPOINT_HALT:
        if (set > 0U)
        {
            /* issue set stall to hardware controller */
            ret = USBSSP_EndpointSetFeature(&dev->xhciDriverResources, epIndex,
                                            set);
        }
        else
        {
            if ((epIndex > USBSSP_EP0_CONTEXT_OFFSET) && (epp->wedgeFlag == 0U))
            {
                /* handle clear feature*/
                ret = USBSSP_EndpointSetFeature(&dev->xhciDriverResources,
                                                epIndex, set);
            }
            else
            {
                /* do nothing */
                vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
                        "CAN'T CLEAR ENDPOINT STALL %c\n", ' ');
                ret = USBSSP_EndpointSetFeature(&dev->xhciDriverResources,
                                                epIndex, 0);
                ret = USBSSP_EndpointSetFeature(&dev->xhciDriverResources,
                                                epIndex, 1);
            }
        } /*else (set) */
        break;

    default:
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
                "Handle Feature (%s) on EP%0d_%s Incorrect Value\n",
                (set > 0) ? "set" : "clear", ctrl->wIndex & 0x7FU,
                (((ctrl->wIndex & 0x80U) != 0U) ? "IN" : "OUT"));
        ret = CDN_EINVAL;
        break;
    }

    // send status stage
    if (ret == (uint32_t)CDN_EOK)
    {
        ret = USBSSP_ControlTransferDev(&dev->xhciDriverResources, NULL, 0, 0);
    }
    return ret;
}

/**
 * handle set/clear feature standard request
 * @param dev pointer to driver object
 * @param ctrl pointer to setup object
 * @param set 1 for set, 0 for clear
 * @return CDN_EOK for success, error code elsewhere
 */
static uint32_t handleFeature (CUSBD_PrivateData *dev, const CH9_UsbSetup *ctrl,
                               uint8_t set)
{
    uint32_t stat = CDN_EOK;

    switch (ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK)
    {
        /* recipient device */
    case CH9_USB_REQ_RECIPIENT_DEVICE:
        stat = handleFeatureRecipientDevice(dev, ctrl, set);
        break;

        /*recipient interface */
    case CH9_USB_REQ_RECIPIENT_INTERFACE:
        stat = handleFeatureRecipientInterface(dev, ctrl);
        break;

        /*recipient endpoint*/
    case CH9_USB_REQ_RECIPIENT_ENDPOINT:
        stat = handleFeatureRecipientEndpoint(dev, ctrl, set);
        break;
    default:
        stat = CDN_EINVAL;
        break;
    };

    return stat;
}

/**
 * Set the address
 */
static uint32_t setAddress (CUSBD_PrivateData *dev, const CH9_UsbSetup *ctrl)
{
    uint8_t addr = (uint8_t)(ctrl->wValue & 0xFFU);

    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "setAddress: %d\n", addr);

    setDevState(dev,
                (addr != 0U) ? CH9_USB_STATE_ADDRESS : CH9_USB_STATE_DEFAULT);

    return CDN_EOK;
}

/**
 * Set the configuration
 */
static uint32_t setConfig (CUSBD_PrivateData *dev, const CH9_UsbSetup *ctrl)
{
    uint8_t config_no = (uint8_t)(ctrl->wValue & 0xFFU);

    setDevState(dev, (config_no != 0U) ? CH9_USB_STATE_CONFIGURED
                                       : CH9_USB_STATE_ADDRESS);

    return 0;
}

/**
 * Set SEL
 */
static uint32_t setSel (CUSBD_PrivateData *dev, CH9_UsbSetup *ctrl)
{
    uint32_t retVal = CDN_EOK;
    // The device should not be in default state
    if (dev->device.state == CH9_USB_STATE_DEFAULT)
    {
        retVal = CDN_EINVAL;
    }
    else if (ctrl->wLength != 6U)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
                "Set SEL should be 6 bytes, got %d\n", ctrl->wLength);
        retVal = CDN_EINVAL;
    }
    else
    {
        retVal = USBSSP_ControlTransferDev(&dev->xhciDriverResources,
                                           dev->ep0RespBuff, ctrl->wLength, 0U);
    }
    return retVal;
}

/**
 * Set the ISOCH delay
 */
static uint32_t setIsochDelay (CUSBD_PrivateData *dev, const CH9_UsbSetup *ctrl)
{
    uint32_t retVal = CDN_EOK;

    if ((ctrl->wIndex != 0U) || (ctrl->wLength != 0U))
    {
        retVal = CDN_EINVAL;
    }
    else
    {
        dev->isoch_delay = ctrl->wValue;
    }
    return retVal;
}

/**
 *
 * @param pD
 * @param ctrl
 * @return CDN_EOK
 */
static uint32_t handleGetStatus (CUSBD_PrivateData *pD, CH9_UsbSetup *ctrl)
{
    uint8_t recip = (ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK);
    uint16_t status_value = 0;
    uint32_t ret = CDN_EOK;

    uint8_t *dmaBuf = pD->ep0RespBuff;

    switch (recip)
    {
        /* recipient device */
    case CH9_USB_REQ_RECIPIENT_DEVICE:
        status_value = cpuToLe16((uint16_t)pD->u2_value |
                                 (uint16_t)pD->u1_value | 1U);
        /* write status in LE order */
        dmaBuf[0] = (uint8_t)status_value;
        dmaBuf[1] = (uint8_t)((status_value >> 8) & 0x00FFU);
        ret = USBSSP_ControlTransferDev(&pD->xhciDriverResources, dmaBuf,
                                        ctrl->wLength, pD->ep0DataDirFlag);
        break;

        /* recipient interface */
    case CH9_USB_REQ_RECIPIENT_INTERFACE:
        ctrl->wIndex = cpuToLe16(ctrl->wIndex);
        ctrl->wLength = cpuToLe16(ctrl->wLength);
        ctrl->wValue = cpuToLe16(ctrl->wValue);
        ret = pD->callbacks.setup(pD, ctrl);
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT, "getStatus res: %d\n", ret);
        break;

        /* recipient endpoint */
    case CH9_USB_REQ_RECIPIENT_ENDPOINT:
    {
        uint32_t epIndex = getEpIndex((uint8_t)le16ToCpu(ctrl->wIndex));
        uint32_t epState = USBSSP_GetEpState(&pD->xhciDriverResources, epIndex);
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "CH9_USB_REQ_RECIPIENT_ENDPOINT epIndex(%d) epState(%d)\n",
                epIndex, epState);
        if ((epState == (uint32_t)USBSSP_EP_CONTEXT_EP_STATE_RUNNING) ||
            (epState == (uint32_t)USBSSP_EP_CONTEXT_EP_STATE_STOPPED))
        {
            status_value = cpuToLe16(0U);
        }
        else
        {
            status_value = cpuToLe16(1U);
        }

        /* write status in LE order */
        dmaBuf[0] = (uint8_t)status_value;
        dmaBuf[1] = (uint8_t)((status_value >> 8) & 0x00FFU);
        ret = USBSSP_ControlTransferDev(&pD->xhciDriverResources, dmaBuf,
                                        ctrl->wLength, pD->ep0DataDirFlag);
        break;
    }

    default:
        ret = CDN_EINVAL;
        break;
    }

    return ret;
}

/**
 * Sets up the configuration
 * @param pD
 * @param ctrl
 * @return uint32_t
 */
static uint32_t handleSetConfiguration (CUSBD_PrivateData *pD,
                                        CH9_UsbSetup *ctrl)
{
    // Set configuration
    uint32_t res = CDN_EOK;
    ctrl->wIndex = cpuToLe16(ctrl->wIndex);
    ctrl->wLength = cpuToLe16(ctrl->wLength);
    ctrl->wValue = cpuToLe16(ctrl->wValue);
    res = pD->callbacks.setup(pD, ctrl);

    if (res == CDN_EOK)
    {
        res = setConfig(pD, ctrl);
    }
    return res;
}

static uint32_t setupDevStand (CUSBD_PrivateData *dev, CH9_UsbSetup *ctrl)
{
    uint32_t res = CDN_EOK;

    switch (ctrl->bRequest)
    {
        /* get status request */
    case CH9_USB_REQ_GET_STATUS:
        res = handleGetStatus(dev, ctrl);
        break;

        /* clear feature request */
    case CH9_USB_REQ_CLEAR_FEATURE:
        res = handleFeature(dev, ctrl, 0U);
        break;

        /* set feature request */
    case CH9_USB_REQ_SET_FEATURE:
        res = handleFeature(dev, ctrl, 1U);
        break;

        /* set address request */
    case CH9_USB_REQ_SET_ADDRESS:
        res = setAddress(dev, ctrl);
        break;

        /* set configuration request */
    case CH9_USB_REQ_SET_CONFIGURATION:
        res = handleSetConfiguration(dev, ctrl);
        break;

        /* set sel request */
    case CH9_USB_REQ_SET_SEL:
        res = setSel(dev, ctrl);
        break;

        /* set iso delay request */
    case CH9_USB_REQ_ISOCH_DELAY:
        res = setIsochDelay(dev, ctrl);
        break;

    default:
        /* handle class or vendor specific request */
        ctrl->wIndex = cpuToLe16(ctrl->wIndex);
        ctrl->wLength = cpuToLe16(ctrl->wLength);
        ctrl->wValue = cpuToLe16(ctrl->wValue);
        res = dev->callbacks.setup(dev, ctrl);
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "res: 0x%08X\n", res);
        break;
    }
    return res;
}

/**
 * Setup functionality
 */
static uint32_t setupDevice (CUSBD_PrivateData *pD, CH9_UsbSetup *ctrl)
{
    uint32_t res = 0;
    uint16_t size = ctrl->wLength;
    pD->device.speed = pD->xhciDriverResources.actualSpeed;
    // Check for the size
    if (size > 0U)
    {
        pD->ep0NextState = USBSSP_EP0_DATA_PHASE;
    }
    else
    {
        pD->ep0NextState = USBSSP_EP0_STATUS_PHASE;
    }
    // Check for setting the flag
    if ((ctrl->bmRequestType & CH9_USB_DIR_DEVICE_TO_HOST) != 0U)
    {
        pD->ep0DataDirFlag = 1;
    }
    else
    {
        pD->ep0DataDirFlag = 0;
    }
    if ((ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK) ==
        CH9_USB_REQ_TYPE_STANDARD)
    {
        // check if request is a standard request
        res = setupDevStand(pD, ctrl);
    }
    else
    {
        // delegate setup for class and vendor specific requests
        ctrl->wIndex = cpuToLe16(ctrl->wIndex);
        ctrl->wLength = cpuToLe16(ctrl->wLength);
        ctrl->wValue = cpuToLe16(ctrl->wValue);
        res = pD->callbacks.setup(pD, ctrl);
    }

    return res;
}

/**
 * Probe the driver.
 * @param[in,out] config specifies driver/hardware configuration
 * @param[in] sysReq system requirement
 * @return CDN_EOK on success
 * @return CDN_EINVAL if illegal/inconsistent values
 */
uint32_t CUSBD_Probe (const CUSBD_Config *config)
{
    uint32_t ret = CUSBD_ProbeSF(config);

    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_HIVERB, "CUSBD_Probe\n", 0);

        // Driver memory consumption is equal to sum of driver taken memory +
        // DMA memory
    }

    return ret;
}

/**
 * Initialize the endpoint
 * @param[in] pD driver state info specific to this instance
 * @param[in,out] config specifies driver/hardware configuration
 * @return CDN_EOK on success
 */
static uint32_t initEp (CUSBD_PrivateData *pD)
{
    uint32_t ret = CDN_EOK;

    // set device general device state to initial state
    pD->device.speed = CH9_USB_SPEED_UNKNOWN;
    pD->device.maxSpeed = CH9_USB_SPEED_HIGH;  // CH9_USB_SPEED_SUPER_PLUS;
    pD->device.state = CH9_USB_STATE_DEFAULT;  // <----- check if we set status,
                                               // probably higher layer sets it

    // initialize endpoint 0 - software
    pD->ep0.ep.address = EP0_ADDRESS;
    pD->ep0.ep.epPrivate = &(pD->ep0);
    pD->ep0.ep.ops = &epOps;
    pD->device.ep0 = &pD->ep0.ep;
    //    pD->ep0RespBuff = config->ep0RespBuffer;

    // initialize endpoints given in configuration
    listInit(&pD->device.epList);

    ret = initSwEp(pD, 1);
    if (ret == 0U)
    {
        ret = initSwEp(pD, 0);
        if (ret == 0U)
        {
            // reset all internal state variable
            pD->suspend = 0;
            pD->u1_value = 0;
            pD->u2_value = 0;
        }
    }
    return ret;
}

/**
 * Initialize the driver instance and state, configure the USB device
 * as specified in the 'config' settings, initialize locks used by the
 * driver.
 * @param[in] pD driver state info specific to this instance
 * @param[in,out] config specifies driver/hardware configuration
 * @param[in] callbacks client-supplied callback functions
 * @return CDN_EOK on success
 * @return CDN_ENOTSUP if hardware has an inconsistent configuration or doesn't
 * support feature(s) required by 'config' parameters
 * @return CDN_ENOENT if insufficient locks were available (i.e. something
 * allocated locks between probe and init)
 * @return CDN_EIO if driver encountered an error accessing hardware
 * @return CDN_EINVAL if illegal/inconsistent values in 'config'
 */
uint32_t CUSBD_Init (CUSBD_PrivateData *pD, CUSBD_Callbacks *callbacks)
{
    uint32_t ret = CDN_EOK;

    // Check for the parameters

    ret = CUSBD_InitSF(pD, NULL, callbacks);

    if (ret == (uint32_t)CDN_EOK)
    {
        /* if (config->ep0RespBuffer == NULL) {
             ret = CDN_EINVAL;
         }*/
    }

    // If the parameters are correct
    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "CUSBD_Init\n", 0);

        (void)memset(pD, 0, sizeof(CUSBD_PrivateData));

        (void)memcpy(&pD->callbacks, callbacks, sizeof(CUSBD_Callbacks));
        //   (void) memcpy(&pD->config, config, sizeof (CUSBD_Config));

        pD->internalCallbacks.disconnect = disconnectDevice;
        pD->internalCallbacks.connect = connectDevice;
        pD->internalCallbacks.setup = setupDevice;
        pD->internalCallbacks.configured = handleEpStateChange;

        // register CUSBD callback in SSP driver
        pD->xhciDriverResources.cusbdCallbacks = &pD->internalCallbacks;
        pD->xhciDriverResources.privateData = pD;
        pD->xhciDriverResources.deviceModeFlag = 1U;
        pD->xhciDriverResources
            .extendedTBCMode = 0;  // config->extendedTBCMode;
        pD->xhciDriverResources.usbModeFlag = 2;
    }

    // Check for the value to be error free
    if (ret == (uint32_t)CDN_EOK)
    {
        ret = initEp(pD);
    }

    return ret;
}

/**
 * Destroy the driver (automatically performs a stop)
 * @param[in] pD driver state info specific to this instance
 * @return CDN_EOK on success
 * @return CDN_EINVAL if illegal/inconsistent values in 'config'
 */
uint32_t CUSBD_Destroy (const CUSBD_PrivateData *pD)
{
    uint32_t ret = CUSBD_DestroySF(pD);

    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    return ret;
}

/**
 * Start the USB driver.
 * @param[in] pD driver state info specific to this instance
 * @return CDN_EOK on success
 * @return CDN_EINVAL if illegal/inconsistent values in 'config'
 */
uint32_t CUSBD_Start (CUSBD_PrivateData *pD)
{
    uint32_t ret = CUSBD_StartSF(pD);

    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_HIVERB, "CUSBD_Start\n", 0);

        // initialize SSP controller
        ret = USBSSP_Init(&pD->xhciDriverResources, &pD->config.xhciConfig);

        if (ret != (uint32_t)CDN_EOK)
        {
            vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT, "SSP initializing error: %d\n",
                    ret);
        }
    }

    return ret;
}

/**
 * Stop the driver. This should disable the hardware, including its
 * interrupt at the source, and make a best-effort to cancel any
 * pending transactions.
 * @param[in] pD driver state info specific to this instance
 * @return CDN_EOK on success
 * @return CDN_EINVAL if illegal/inconsistent values in 'config'
 */
uint32_t CUSBD_Stop (const CUSBD_PrivateData *pD)
{
    uint32_t ret = CUSBD_StopSF(pD);

    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    return ret;
}

/**
 * Driver ISR.  Platform-specific code is responsible for ensuring
 * this gets called when the corresponding hardware's interrupt is
 * asserted. Registering the ISR should be done after calling init,
 * and before calling start. The driver's ISR will not attempt to lock
 * any locks, but will perform client callbacks. If the client wishes
 * to defer processing to non-interrupt time, it is responsible for
 * doing so.
 * @param[in] pD driver state info specific to this instance
 */
void CUSBD_Isr (CUSBD_PrivateData *pD)
{
     if (CUSBD_IsrSF(pD) != (uint32_t) CDN_EOK) {

     } else {
    (void)USBSSP_Isr(&pD->xhciDriverResources);
     }
}

/**
 * Enable Endpoint.  This function should be called within
 * SET_CONFIGURATION(configNum > 0) request context. It configures
 * required hardware controller endpoint with parameters given in
 * desc.
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint being configured
 * @param[in] desc endpoint descriptor
 * @return 0 on success
 * @return CDN_EINVAL if pD, ep or desc is NULL or desc is not a endpoint
 * descriptor
 */
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter ep with const
 * specifier, DRV-3806" */
uint32_t CUSBD_EpEnable (CUSBD_PrivateData *pD, CUSBD_Ep *ep,
                         const uint8_t *desc)
{
    uint32_t ret = CUSBD_EpEnableSF(pD, ep, desc);
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EOK)
    {
        uint16_t maxPacketSize = ((uint16_t)desc[5] << 8U) | (uint16_t)desc[4];
        if (desc[1] != CH9_USB_DT_ENDPOINT)
        {
            ret = CDN_EINVAL;
        }

        if (maxPacketSize == 0U)
        {
            vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT, "Missing wMaxPacketSize\n", 0);
            ret = CDN_EINVAL;
        }
    }

    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        displayEp(ep, pD->device.speed, desc);
        // Passing to SSP
        ret = USBSSP_EnableEndpoint(&(pD->xhciDriverResources), desc);
    }

    /* clear wedge flag */
    ep->epPrivate->wedgeFlag = 0;

    return ret;
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */

/**
 * Disable Endpoint. Functions unconfigures hardware endpoint.
 * Endpoint will not respond to any host packets. This function should
 * be called within SET_CONFIGURATION(configNum = 0) request context
 * or on disconnect event. All queued requests on endpoint are
 * completed with CDN_ECANCELED status.
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint being unconfigured
 * @return 0 on success
 * @return CDN_EINVAL if pD or ep is NULL
 */
uint32_t CUSBD_EpDisable (CUSBD_PrivateData *pD, CUSBD_Ep *ep)
{
    uint32_t ret = CUSBD_EpDisableSF(pD, ep);

    if (ret == (uint32_t)CDN_EOK)
    {
        // check if endpoint exist and return if it doesn't
        if (ep->address == 0U)
        {
            ret = CDN_EINVAL;
        }
    }

    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP%0d_%s\n", ep->address & 0x7FU,
                (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));

        // Passing to SSP
        ret = USBSSP_DisableEndpoint(&(pD->xhciDriverResources), ep->address);
    }
    if (ret == CDN_EOK)
    {
        epFlushRequests(&(pD->xhciDriverResources), ep->epPrivate,
                        getEpIndex(ep->address));
    }
    return ret;
}

/**
 * Set halt or clear halt state on endpoint. When setting halt, device
 * will respond with STALL packet to each host packet. When clearing
 * halt, endpoint returns to normal operating.
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint being setting or clearing halt state
 * @param[in] value if 1 sets halt, if 0 clears halt on endpoint
 * @return 0 on success
 * @return CDN_EPERM if endpoint is disabled (has not been configured yet)
 * @return CDN_EINVAL if pD or ep is NULL
 */
uint32_t CUSBD_EpSetHalt (CUSBD_PrivateData *pD, const CUSBD_Ep *ep,
                          uint8_t value)
{
    uint32_t ret = CUSBD_EpSetHaltSF(pD, ep);
    CUSBD_EpPrivate *epp;
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }
    else
    {
        epp = ep->epPrivate;
        if (epp == NULL)
        {
            vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                    "Critical error! EpPrivate is NULL\n", 0);
            ret = CDN_EINVAL;
        }
    }
    // If the parameters are correct
    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP%0d_%s\n", ep->address & 0x7FU,
                (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));

        ret = USBSSP_EndpointSetFeature(&(pD->xhciDriverResources),
                                        getEpIndex(ep->address), value);

        if (value == 0U)
        {
            /* clear stall */
            ep->epPrivate->wedgeFlag = 0;
        }
    }

    return ret;
}

/**
 * Set halt on endpoint. Function sets endpoint to permanent halt
 * state. State can not be changed even with epSetHalt(pD, ep, 0)
 * function. Endpoint returns automatically to its normal state on
 * disconnect event.
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint being setting halt state
 * @return 0 on success
 * @return CDN_EPERM if endpoint is disabled (has not been configured yet)
 * @return CDN_EINVAL if pD or ep is NULL
 */
uint32_t CUSBD_EpSetWedge (CUSBD_PrivateData *pD, const CUSBD_Ep *ep)
{
    uint32_t ret = CUSBD_EpSetWedgeSF(pD, ep);
    CUSBD_EpPrivate *epp;
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }
    else
    {
        epp = ep->epPrivate;
        if (epp == NULL)
        {
            vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                    "Critical error! EpPrivate is NULL\n", 0);
            ret = CDN_EINVAL;
        }
    }
    // If the parameters are correct
    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP%0d_%s\n", ep->address & 0x7FU,
                (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));

        epp->wedgeFlag = 1;
        ret = CUSBD_EpSetHalt(pD, ep, 1);
    }

    return ret;
}

/**
 * Flush hardware endpoint FIFO.
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint which FIFO is to be flushed
 */
void CUSBD_EpFifoFlush (const CUSBD_PrivateData *pD, const CUSBD_Ep *ep)
{
    uint32_t ret = CUSBD_EpFifoFlushSF(pD, ep);
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP%0d_%s\n", ep->address & 0x7FU,
                (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));
    }
}

/**
 * Returns number of bytes in hardware endpoint FIFO. Function useful
 * in application where exact number of data bytes is required. In
 * some situation software higher layers must be aware of number of
 * data bytes issued but not transfered by hardware because of aborted
 * transfers, for example on disconnect event. *
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint which status is to be returned
 * @return number of bytes in hardware endpoint FIFO
 * @return CDN_EINVAL if pD or ep is NULL
 */
uint32_t CUSBD_EpFifoStatus (const CUSBD_PrivateData *pD, const CUSBD_Ep *ep)
{
    uint32_t ret = CUSBD_EpFifoStatusSF(pD, ep);
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP%0d_%s\n", ep->address & 0x7FU,
                (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));
    }

    return ret;
}

/**
 * Handling any pending transfer request with noDoorbell set to 0
 *
 * @param dev Pointer to the device private data.
 * @param epIndex Endpoint index (context index)
 * @param ep Pointer to the endpoint data
 * @param req Pointer to the request structure
 */
static uint32_t handleCUSBDReq (CUSBD_PrivateData *dev, uint8_t epIndex,
                                const CUSBD_Ep *ep, CUSBD_Req *req)
{
    uint32_t res = CDN_EOK;
    uint8_t *reqBuf = req->buf;

    if (req->forceHeaderCommandFlag != 0U)
    {
        USBSSP_dword word;
        word.dword0 = getU32ValFromU8Ptr(&reqBuf[0]);
        word.dword1 = getU32ValFromU8Ptr(&reqBuf[4]);
        word.dword2 = getU32ValFromU8Ptr(&reqBuf[8]);
        word.dword3 = getU32ValFromU8Ptr(&reqBuf[12]);
        // handle force header command flag
        res = USBSSP_ForceHeader(&dev->xhciDriverResources, word,
                                 transferComplete);
    }
    else if (req->eventDataFlags > 0U)
    {
        // putting data event TRB doesn't doorbell transfer on endpoint
        res = USBSSP_AddEventDataTRB(
            &dev->xhciDriverResources, epIndex, getU32ValFromU8Ptr(&reqBuf[0]),
            getU32ValFromU8Ptr(&reqBuf[4]), req->eventDataFlags);
    }
    else
    {
        res = CUSBD_RunHwTransfer(&dev->xhciDriverResources, ep, req);
    }
    if (res != CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT, "Critical error: %d!\n", res);
    }
    return res;
}

/**
 * Handle status
 */
static void handleReqStatus (const USBSSP_DriverResourcesT *const arg,
                             CUSBD_Req *req,
                             const USBSSP_ProducerQueueT *const epSsp,
                             CUSBD_Ep *ep)
{
    uint8_t epIndex = arg->lastEpIntIndex;
    uint16_t sid = arg->ep[epIndex].eventSID;
    CUSBD_EpPrivate *epp;
    epp = ep->epPrivate;

    // Coding status code:
    //   - "Success" is passed as CDN_EOK
    //   - "Short Packet" is passed as CDN_EOK for DEVICE mode only
    if ((epSsp->completionCode == (uint8_t)USBSSP_TRB_COMPLETE_SUCCESS) ||
        ((arg->deviceModeFlag == 1U) &&
         (epSsp->completionCode == (uint8_t)USBSSP_TRB_CMPL_SHORT_PKT)))
    {
        req->status = CDN_EOK;
    }
    else if (epSsp->completionCode == (uint8_t)USBSSP_TRB_COMPLETE_INVALID)
    {
        req->status = CDN_EIO;  // what error return in CUSBD ???
    }
    else
    {
        req->status = epSsp->completionCode;
    }

    if (sid == 0U)
    {
        reqListDeleteItem(&epp->headReq, req);
    }
    else
    {
        reqListDeleteItem(&epp->streamHeadReq[sid - 1U], req);
    }

    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "- REQ: %p\n", (void *)req);
    if (req->complete != NULL)
    {
        req->complete(ep, req);
    }
}

/**
 * Complete the transfer
 */
static void transferComplete (USBSSP_DriverResourcesT *arg)
{
    uint8_t epIndex = arg->lastEpIntIndex;
    uint8_t epNo = getEpNum(epIndex);
    CUSBD_EpPrivate *epp;

    USBSSP_ProducerQueueT *epSsp;
    CUSBD_Req *req;
    CUSBD_PrivateData *dev;
    CUSBD_Ep *ep;
    uint16_t sid = arg->ep[epIndex].eventSID;
    // Displaying the EP index and the Stream ID
    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP index %d, streamID: %d\n", epIndex,
            sid);

    dev = arg->privateData;

    if (getEpDir(epIndex) != 0U)
    {
        ep = &dev->ep_in_container[epNo - 1U].ep;
    }
    else
    {
        ep = &dev->ep_out_container[epNo - 1U].ep;
    }
    vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP%0d_%s\n", ep->address & 0x7FU,
            (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));
    epp = ep->epPrivate;

    // Check the value
    if (sid == 0U)
    {
        epSsp = &arg->ep[epIndex];
        req = epp->headReq;
    }
    else
    {
        epSsp = arg->ep[epIndex].stream[sid - 1U];
        req = epp->streamHeadReq[sid - 1U];
    }

    // check if there is really interrupt from complete, return if no
    if (req == NULL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "DB RC REQ==NULL EP%0d_%s\n",
                ep->address & 0x7FU,
                (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));
    }
    else
    {
#ifdef DEBUG
        // display request
        displayReq(req, "Completing");
#endif
        req->actual = epSsp->numOfBytes - epSsp->numOfResidue;

        handleReqStatus(arg, req, epSsp, ep);
    }
    return;
}

/**
 * Callback function
 */
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter buffer with
 * const specifier, DRV-3806" */
/* parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter actualLength not used
 * in function dataTransferComplete, DRV-5631" */
static void dataTransferComplete (USBSSP_DriverResourcesT *arg, uint32_t status,
                                  const USBSSP_RingElementT *eventPtr,
                                  uint8_t *buffer, uint32_t actualLength)
{
    if (status != CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "status %d\n", status);
    }

    if (eventPtr != NULL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "eventPtr %p\n", eventPtr);
    }

    if (buffer != NULL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "buffer %p\n", buffer);
    }
    // transfer complete function
    transferComplete(arg);
}
/* parasoft-end-suppress MISRA2012-RULE-2_7-4 */
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */

/**
 * Running the Hardware Transfer
 */
static uint32_t RunHwTransferSG (USBSSP_DriverResourcesT *sspRes,
                                 const CUSBD_Ep *ep, const CUSBD_Req *const req)
{
    uint32_t res;
    uint8_t epIndex = getEpIndex(ep->address);

    // Check for the number of entries
    if (req->numOfSgs > 32U)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Number of scatter/gather elements > 32\n", 0);
        res = (uint32_t)CDN_EINVAL;
    }
    else
    {
        uint32_t i;
        uintptr_t bufPtr[33];
        uint32_t bufSize[33];
        for (i = 0U; i < req->numOfSgs; i++)
        {
            bufPtr[i] = req->sg[i].dmaAddress;
            bufSize[i] = req->sg[i].length;
        }
        bufPtr[i] = (uintptr_t)0U;
        bufSize[i] = 0U;
        USBSSP_param paramT;
        paramT.buffVec = bufPtr;
        paramT.sizeVec = bufSize;
        paramT.sgElementsSum = req->numOfSgs;
        // transfer the data
        res = USBSSP_TransferData2(
            sspRes, epIndex, paramT,
            (req->noInterrupt != 0U) ? NULL : &dataTransferComplete);
        // Check for error
        if (res != CDN_EOK)
        {
            vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                    "Critical error (%d) when data transfer on EP%0d_%s\n", res,
                    ep->address & 0x7FU,
                    (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));
        }
    }

    return res;
}

/**
 * Running the Hardware Transfer
 */
static uint32_t CUSBD_RunHwTransfer (USBSSP_DriverResourcesT *sspRes,
                                     const CUSBD_Ep *ep, CUSBD_Req *req)
{
    uint32_t res;
    CUSBD_EpPrivate *epp = ep->epPrivate;
    uint8_t epIndex = getEpIndex(ep->address);

    req->numMappedSgs = 0;
    req->actual = 0;

#ifdef DEBUG
    displayReq(req, "Queuing");
#endif
    // Check for the value of epIndex and private data not being Null
    if ((epIndex < (USBSSP_MAX_EP_CONTEXT_NUM + USBSSP_EP_CONT_OFFSET)) &&
        (epp != NULL))
    {
        if (req->numOfSgs == 0U)
        {
            if (req->noDorbell != 0U)
            {
                (void)USBSSP_SetEndpointExtraFlag(
                    sspRes, epIndex, USBSSP_EXTRAFLAGSENUMT_NODORBELL);
            }
            else
            {
                (void)USBSSP_CleanEndpointExtraFlag(
                    sspRes, epIndex, USBSSP_EXTRAFLAGSENUMT_NODORBELL);
            }
            sspRes->ep[epIndex].frameID = req->frameID;
            sspRes->ep[epIndex].actualSID = req->streamId;
            res = USBSSP_TransferData(
                sspRes, epIndex, req->dma, req->length,
                (req->noInterrupt != 0U) ? NULL : &dataTransferComplete);
            // Check for error
            if (res != CDN_EOK)
            {
                vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                        "Critical error (%d) when data transfer on EP%0d_%s\n",
                        res, ep->address & 0x7FU,
                        (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));
            }
        }
        else
        {
            res = RunHwTransferSG(sspRes, ep, req);
        }
    }
    else
    {
        // Presence of an error
        res = CDN_EINVAL;
    }
    return res;
}

/**
 * Submits IN/OUT transfer request to an endpoint.
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint associated with the request
 * @param[in] req request being submitted
 * @return 0 on success
 * @return CDN_EPROTO only on default endpoint if endpoint is not in data stage
 * phase
 * @return CDN_EPERM if endpoint is not enabled
 * @return CDN_EINVAL if pD, ep, or req is NULL
 */
uint32_t CUSBD_ReqQueue (CUSBD_PrivateData *pD, const CUSBD_Ep *ep,
                         CUSBD_Req *req)
{
    uint32_t ret = CUSBD_ReqQueueSF(pD, ep, req);
    CUSBD_EpPrivate *epp;
    // Check for correctness of parameters //
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }
    else
    {
        epp = ep->epPrivate;
        if (epp == NULL)
        {
            vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                    "Critical error! EpPrivate is NULL\n", 0);
            ret = CDN_EINVAL;
        }
    }
    // If the parameters are correct
    if (ret == (uint32_t)CDN_EOK)
    {
        req->actual = 0;
        req->status = CDN_EINPROGRESS;

        if (ep->address == (uint8_t)EP0_ADDRESS)
        {
            ret = USBSSP_ControlTransferDev(&pD->xhciDriverResources, req->buf,
                                            req->length, pD->ep0DataDirFlag);
        }
        else
        {
            uint8_t epIndex = getEpIndex(ep->address);
            vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI,
                    "ep_address: EP%0d_%s, req = %p, complete = %p\n",
                    ep->address & 0x7FU,
                    (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"), req,
                    (void *)req->complete);

            // add request to list at end
            if (req->streamId == 0U)
            {
                reqListAddTail(&epp->headReq, req);
            }
            else
            {
                reqListAddTail(&epp->streamHeadReq[req->streamId - 1U], req);
            }
            // Handle request
            ret = handleCUSBDReq(pD, epIndex, ep, req);
        }
    }
    return ret;
}
/**
 * Dequeues IN/OUT transfer request from an endpoint. Function
 * completes all queued request with CDN_ECANCELED status.
 * @param[in] pD driver state info specific to this instance
 * @param[in] ep endpoint associated with the request
 * @param[in] req request being dequeued
 * @return 0 on success
 * @return CDN_EINVAL if pD or ep is NULL
 */
uint32_t CUSBD_ReqDequeue (const CUSBD_PrivateData *pD, const CUSBD_Ep *ep,
                           const CUSBD_Req *req)
{
    uint32_t ret = CUSBD_ReqDequeueSF(pD, ep, req);
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }
    // If the parameters are correct
    if (ret == (uint32_t)CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_FYI, "EP%0d_%s\n", ep->address & 0x7FU,
                (((ep->address & 0x80U) != 0U) ? "IN" : "OUT"));
#ifdef DEBUG
        displayReq(req, "Dequeuing");
#endif
    }

    return ret;
}

/**
 * Returns pointer to CUSBD object. CUSBD object is a logical
 * representation of USB device. CUSBD contains endpoint collection
 * accessed through epList field. Endpoints collection is organized as
 * double linked list.
 * @param[in] pD driver state info specific to this instance
 * @param[out] dev returns pointer to CUSBD instance
 */
void CUSBD_GetDevInstance (CUSBD_PrivateData *pD, CUSBD_Dev **dev)
{
    // Check for correctness of parameters
    uint32_t ret = ((pD == NULL) || (dev == NULL)) ? CDN_EINVAL : CDN_EOK;
    if (CDN_EOK == ret)
    {
        *dev = &(pD->device);
    }
}

/**
 * Returns number of frame. Some controllers have counter of SOF
 * packets or ITP packets in the Super Speed case. Function returns
 * value of this counter. This counter can be used for time
 * measurement. Single FS frame is 1 ms measured, for HS and SS is
 * 125us.
 * @param[in] pD driver state info specific to this instance
 * @param[out] numOfFrame returns number of USB frame
 * @return CDN_EOK on success
 * @return CDN_EOPNOTSUPP if feature is not supported
 * @return CDN_EINVAL if pD or numOfFrame is NULL
 */
uint32_t CUSBD_DGetFrame (CUSBD_PrivateData *pD, uint32_t *numOfFrame)
{
    uint32_t ret = CDN_EOK;
    // Check for correctness of parameters
    if (CUSBD_DGetFrameSF(pD, numOfFrame) != CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
        ret = CDN_EINVAL;
    }
    else
    {
        if (USBSSP_GetMicroFrameIndex(&pD->xhciDriverResources, numOfFrame) !=
            CDN_EOK)
        {
            ret = CDN_EINVAL;
        }
        else
        {
            ret = CDN_EOK;
        }
    }

    return ret;
}

/**
 * Sets the device self powered feature.
 * @param[in] pD driver state info specific to this instance
 * @return CDN_EOK on success
 * @return CDN_EOPNOTSUPP if feature is not supported
 * @return CDN_EINVAL if pD is NULL
 */
uint32_t CUSBD_DSetSelfpowered (const CUSBD_PrivateData *pD)
{
    uint32_t ret = CUSBD_DSetSelfpoweredSF(pD);
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        ret = CDN_ENOTSUP;
    }

    return ret;
}

/**
 * Clear the device self powered feature.
 * @param[in] pD driver state info specific to this instance
 * @return CDN_EOK on success
 * @return CDN_EOPNOTSUPP if feature is not supported
 * @return CDN_EINVAL if pD is NULL
 */
uint32_t CUSBD_DClearSelfpowered (const CUSBD_PrivateData *pD)
{
    uint32_t ret = CUSBD_DClearSelfpoweredSF(pD);
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        ret = CDN_ENOTSUP;
    }

    return ret;
}

/**
 * Returns configuration parameters: U1 exit latency and U2 exit
 * latency Function useful only in Super Speed mode.
 * @param[in] pD driver state info specific to this instance
 * @param[out] configParams pointer to CH9_ConfigParams structure
 * @return CDN_EOK on success
 * @return CDN_EINVAL if illegal/inconsistent values in 'config'
 */
uint32_t CUSBD_DGetConfigParams (const CUSBD_PrivateData *pD,
                                 CH9_ConfigParams *configParams)
{
    uint32_t ret = CUSBD_DGetConfigParamsSF(pD, configParams);
    // Check for correctness of parameters
    if (ret == (uint32_t)CDN_EINVAL)
    {
        vDbgMsg(USBSSP_DBG_CUSBD, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters\n",
                0);
    }

    if (ret == (uint32_t)CDN_EOK)
    {
        configParams->bU1devExitLat = CUSBSS_U1_DEV_EXIT_LAT;
        configParams->bU2DevExitLat = CUSBSS_U2_DEV_EXIT_LAT;
    }

    return ret;
}

USBSSP_DriverResourcesT *get_USBSSP_Obj (CUSBD_PrivateData *pD)
{
    USBSSP_DriverResourcesT *driverResources;
    // If the data is NULL
    if (pD == NULL)
    {
        driverResources = NULL;
    }
    else
    {
        driverResources = &pD->xhciDriverResources;
    }

    return driverResources;
}
