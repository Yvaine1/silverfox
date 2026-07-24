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
 * bot_app.c
 * Bulk Only Transport is a part of mass storage application responsible
 * for SCSI command, data and status transport
 *
 *****************************************************************************/

#include <stdlib.h>                        // used for malloc
#include <stdio.h>                         // standard library
#include <string.h>                        // standard library
#include "cdn_log.h"
#include "fmsh_usb_data.h"
#include "fmsh_gic.h" 
#include "byteorder.h"

#include "bot.h"         // BOT protocol
#include "storage.h"     // storage layer
#include "scsi.h"        // SCSI layer
#include "msc_config.h"  // application's configuration

extern uint8_t *StorageBase;
extern uintptr_t StorageBasePhyAddr;

/*USB application debugging*/
#define DBG_USB_APP          0x000000010
#define DBG_USB_APP_VERBOSE  0x000000020
#define DBG_USB_BOT_APP      0x000000040
#define DBG_USB_SCSI_APP     0x000000008
#define REQ_INFO_BUFFER_SZ   256

//-------------- definitions used during configuration -------------------------
#define BCD_USB_HS_ONLY         0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BULK_EP_IN 0x81
#define BULK_EP_OUT 0x01

#define BOT_EP_MASK     0xC
#define BOT_CBW_BUF_SZ  128     /* Size of CBW buffer for fetching request */

//------------------------------------------------------------------------------
//extern uintptr_t CPS_GetPhyAddrOfVPtr(const void *ptr);

extern USBSSP_DriverResourcesT * get_USBSSP_Obj(CUSBD_PrivateData * pD);
/******************************************************************************
 * Local static data structures
 * ****************************************************************************/
// ---------- Super Speed USB driver configuration -----------------------------
//static CUSBD_OBJ * drv; // driver pointer
static CUSBD_PrivateData cusbdPrivData;

// ------------- driver's resources --------------------------------------------
static CUSBD_Ep * epIn, *epOut; // endpoint objects

// static allocation of request for default, IN and OUT endpoints
static CUSBD_Req *ep0Req, *bulkInReq, *bulkOutReq;
static CUSBD_Req ep0ReqAlloc, bulkInReqAlloc, bulkOutReqAlloc;

//static memory allocation for default endpoint data buffer
static uint8_t *ep0Buff;
static uint8_t ep0Buff_PhyAddr;

/** EP0 resp buffer */
uint8_t g_AppEp0ResBuffer[USBSSP_EP0_DATA_BUFF_SIZE] __attribute__((aligned(64)))={0U};
/** EP0 buffer */
uint8_t g_AppEp0Buffer[USBSSP_EP0_DATA_BUFF_SIZE] __attribute__((aligned(64)))={0U};

/** msc resp buffer */
uint8_t g_mscCmdBuffer[512] __attribute__((aligned(64)))={0U};

uint8_t g_mscRespBuffer[512] __attribute__((aligned(64)))={0U};

uint8_t g_scsiRespBuffer[1024] __attribute__((aligned(64)))={0U};


// --------flags used for synchronization---------------------------------------
static volatile uint8_t configValue = 0; // keeps actual configuration value
static volatile uint8_t packet_received = 0; // flag active when received packet
static volatile uint8_t packet_sent = 0; // flag active when sent packet
static volatile uint8_t packet_aborted = 0; // flag active when packet aborted

// BOT protocol buffers
static uint8_t* command_buff; // buffer for command packets
static uint8_t* response_buff; // buffer for csw packets
static uintptr_t command_buff_phy_addr;
static uintptr_t response_buff_phy_addr;

// mass storage auxiliary variables - used during MSC error recovery
static uint32_t host_num_of_bytes;
static uint8_t data_dir;
static uint8_t dir_error;

static uint32_t cmdIdx = 0;
static bot_app_state_t botAppState = BOT_UNINIT_STATE;

static int ignoreBulkOut = 0;
static uint32_t configMask;
/******************************************************************************
 * Local static functions
 * ****************************************************************************/
static uint32_t queue_cbw(CUSBD_PrivateData *pD);
static void onDataXferCmpl(CUSBD_Ep *ep, CUSBD_Req * req);
static void process_cbw(CUSBD_Ep *ep, CUSBD_Req * req);

// functions used for transferring SCSI data
static uint32_t send_data(void *buff, uint32_t size, uint16_t sid);

// debug functions
static void displayRequestinfo(CUSBD_Req * req);
static void displayEndpointInfo(CUSBD_Ep * ep);
static void displayDeviceInfo(CUSBD_Dev * dev);


//------------------- descriptor set--------------------------------------------
// device descriptor for SuperSpeed mode
static CH9_UsbDeviceDescriptor devSsDesc = {CH9_USB_DS_DEVICE,
    CH9_USB_DT_DEVICE, cpuToLe16(BCD_USB_SS), 0, 0, 0, 9, cpuToLe16(ID_VENDOR),
    cpuToLe16(ID_PRODUCT), cpuToLe16(BCD_DEVICE_SS), 1, 2, 3, 1};

// device descriptor for HighSpeed mode
static CH9_UsbDeviceDescriptor devHsDesc = {CH9_USB_DS_DEVICE,
    CH9_USB_DT_DEVICE, cpuToLe16(BCD_USB_HS), 0, 0, 0, 64, cpuToLe16(ID_VENDOR),
    cpuToLe16(ID_PRODUCT), cpuToLe16(BCD_DEVICE_HS), 1, 2, 3, 1};
//------------- Start of Super Speed configuration descriptors -----------------
// configuration descriptors for SuperSpeed mode
static CH9_UsbConfigurationDescriptor confSsDesc = {CH9_USB_DS_CONFIGURATION,
    CH9_USB_DT_CONFIGURATION,
    cpuToLe16(
    CH9_USB_DS_CONFIGURATION + CH9_USB_DS_INTERFACE + 2 * CH9_USB_DS_ENDPOINT + 2 * CH9_USB_DS_SS_USB_EP_COMPANION),
    1, 1, 0,
    CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED, 1};

//0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
static CH9_UsbInterfaceDescriptor interfaceDesc = {CH9_USB_DS_INTERFACE,
    CH9_USB_DT_INTERFACE, 0, 0, 2,
    CH9_USB_CLASS_MASS_STORAGE, 0x06, 0x50, 0};

static CH9_UsbEndpointDescriptor endpointEpInDesc = {CH9_USB_DS_ENDPOINT,
    CH9_USB_DT_ENDPOINT, BULK_EP_IN, CH9_USB_EP_BULK, cpuToLe16(512), 0};

static CH9_UsbEndpointDescriptor endpointEpOutDesc = {CH9_USB_DS_ENDPOINT,
    CH9_USB_DT_ENDPOINT, BULK_EP_OUT, CH9_USB_EP_BULK, cpuToLe16(512), 0};

//0x06, 0x30, 0x00, 0x00, 0x00, 0x00
static CH9_UsbSSEndpointCompanionDescriptor compDesc = {
    CH9_USB_DS_SS_USB_EP_COMPANION,
    CH9_USB_DT_SS_USB_EP_COMPANION, 0, 0, cpuToLe16(0)
};
//------------------- End of Super Speed configuration -------------------------

//------------- Start of High Speed configuration descriptors -----------------

// configuration descriptors for HighSpeed mode
static CH9_UsbConfigurationDescriptor confHsDesc = {CH9_USB_DS_CONFIGURATION,
    CH9_USB_DT_CONFIGURATION,
    cpuToLe16(
    CH9_USB_DS_CONFIGURATION + CH9_USB_DS_INTERFACE + 2 * CH9_USB_DS_ENDPOINT),
    1, 1, 0, CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
    0};

// language descriptor for english
static uint8_t languageDesc[] = {4, CH9_USB_DT_STRING, 0x09, 0x04};

// string will be filled then in initializing section
static char vendorDesc[sizeof (USB_MANUFACTURER_STRING) * 2 + 2];
static char productDesc[sizeof (USB_PRODUCT_STRING) * 2 + 2];
static char serialDesc[sizeof (USB_SERIAL_NUMBER_STRING) * 2 + 2];

//-------------- BOS descriptor set start --------------------------------------

static CH9_UsbBosDescriptor bosDesc = {
    CH9_USB_DS_BOS,
    CH9_USB_DT_BOS,
    cpuToLe16(
    CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_30 + CH9_USB_DS_DEVICE_CAPABILITY_20),
    2
};

static CH9_UsbSSDeviceCapabilityDescriptor capabilitySsDesc = {CH9_USB_DS_DEVICE_CAPABILITY_30, CH9_USB_DT_DEVICE_CAPABILITY,
    CH9_USB_DCT_SS_USB,
    0, // LTM not supported
    cpuToLe16(
    CH9_USB_SS_CAP_SUPPORT_SS | CH9_USB_SS_CAP_SUPPORT_HS | CH9_USB_SS_CAP_SUPPORT_FS),
    1, // 1 us
    4, // 4 us
    cpuToLe16(512) // 512 us
};

static CH9_UsbCapabilityDescriptor capabilityExtDesc = {
    CH9_USB_DS_DEVICE_CAPABILITY_20,
    CH9_USB_DT_DEVICE_CAPABILITY,
    CH9_USB_DCT_USB20_EXTENSION, 0

};

//-------------- BOS descriptor set end ----------------------------------------

static CH9_UsbDeviceQualifierDescriptor qualifierDesc = {
    CH9_USB_DS_DEVICE_QUALIFIER, CH9_USB_DT_DEVICE_QUALIFIER, cpuToLe16(0x0200),
    0x00, 0x00, 0x00, 64, 0x01, 0x00
};

static uint8_t setInterfaceFlag = 0;

/**
 * Mass storage function to handle error
 */
static void msc_failed() {

    ignoreBulkOut = 1;
    if ((botAppState != BOT_ERROR_STATE) && (botAppState != BOT_UNINIT_STATE)) {
        epIn->ops->epSetWedge(&cusbdPrivData, epIn);
        botAppState = BOT_ERROR_STATE;
        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Warning botAppState:%s\n", "BOT_ERROR_STATE");
    } else {
        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Skipping HALT botAppState:%d\n", botAppState);        
    }
}

/**
 * EP0 completion routine
 */
static void reqComplete(CUSBD_Ep *ep, CUSBD_Req * req) {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Request on endpoint %s completed\n", ep->name);
}

/**
 * function returns unicode string for english version
 */
static void get_unicode_string(char * target, const char * src) {

    size_t src_len = strlen(src) * 2;
    int i;

    *target++ = src_len + 2;
    *target++ = CH9_USB_DT_STRING;

    if (src_len > 100)
        src_len = 100;
    for (i = 0; i < src_len; i += 2) {
        *target++ = *src++;
        *target++ = 0;
    }
}

/**
 * configure the device
 */
static void configureDev(CUSBD_PrivateData *pD, uint8_t ctrlValue) {

    CUSBD_Ep * ep;
    CUSBD_ListHead *list;
    uint8_t epDesc[CH9_USB_DS_ENDPOINT + CH9_USB_DS_SS_USB_EP_COMPANION];
    CUSBD_Dev * dev;
    CUSBD_GetDevInstance(pD, &dev);
    
    configValue = 1;
    
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        uint8_t epDesc[CH9_USB_DS_ENDPOINT + CH9_USB_DS_SS_USB_EP_COMPANION];

        ep = (CUSBD_Ep *) list;
        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "SET CONFIGURATION(%d) epAddress(%x)\n", le16ToCpu(ctrlValue), ep->address);
        if (ep->address == BULK_EP_IN) {
            memmove(epDesc, &endpointEpInDesc, CH9_USB_DS_ENDPOINT);
            if (dev->speed >= CH9_USB_SPEED_SUPER) {
                memmove(&epDesc[CH9_USB_DS_ENDPOINT], &compDesc, CH9_USB_DS_SS_USB_EP_COMPANION);
            }
            ep->ops->epEnable(pD, ep, epDesc);
            displayEndpointInfo(ep);
            break;
        }
    }    
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        ep = (CUSBD_Ep *) list;
        if (ep->address == BULK_EP_OUT) {
            memmove(epDesc, &endpointEpOutDesc, CH9_USB_DS_ENDPOINT);
            if (dev->speed >= CH9_USB_SPEED_SUPER) {
                memmove(&epDesc[CH9_USB_DS_ENDPOINT], &compDesc, CH9_USB_DS_SS_USB_EP_COMPANION);
            }
            ep->ops->epEnable(pD, ep, epDesc);
            displayEndpointInfo(ep);
            break;
        }
    }
}

/**
 * un-configure the device
 */
static void unConfigureDev(CUSBD_PrivateData *pD) {
    CUSBD_Dev * dev;
    CUSBD_Ep * ep;
    CUSBD_ListHead *list;
    configValue = 0;
    CUSBD_GetDevInstance(pD, &dev);
    botAppState = BOT_UNINIT_STATE;    
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        ep = (CUSBD_Ep *) list;
        if (ep->address == BULK_EP_IN) {
            ep->ops->epDisable(pD, ep);
            break;
        }
    }
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        ep = (CUSBD_Ep *) list;
        if (ep->address == BULK_EP_OUT) {
            ep->ops->epDisable(pD, ep);
            break;
        }
    }
    dev->state = CH9_USB_STATE_ADDRESS;
}

// -------------- driver callback functions ------------------------------------
static void setInterfaceCallback (USBSSP_DriverResourcesT* res, USBSSP_SetInterfaceState *configEpCmd, uint32_t dropMask) {
    if (setInterfaceFlag) {
        //printf("BOT_APP: SetInterface callback\r\n");
        configMask |= dropMask;
        if (configMask == BOT_EP_MASK) {
            if (*configEpCmd == USBSSP_EP_DISABLE) {                  
                *configEpCmd = USBSSP_EP_ENABLE;
                configMask = 0;
                configureDev(&cusbdPrivData, 1);  
            } else if (*configEpCmd == USBSSP_EP_ENABLE) {
                setInterfaceFlag = 0;
                *configEpCmd = USBSSP_EP_CONFIGURE;
                configMask = 0;
            }                    
        }
    } else {
        //printf("BOT_APP: botAppState %d \r\n", botAppState); 
        if ((botAppState == BOT_ERROR_STATE) || (botAppState == BOT_CBW_STATE)){
            queue_cbw(res->privateData);
        }
    }

}

/**
 * User callback for CONNECT
 */
static void connect(CUSBD_PrivateData *pD) {
    CUSBD_Dev *dev;

    CUSBD_GetDevInstance(pD, &dev);

    printf("BOT_APP: Application connect at %d speed\r\n", dev->speed);
}

/**
 * User callback for DISCONNECT
 */
static void disconnect(CUSBD_PrivateData *pD) {
    CUSBD_Dev *dev;
    
    CUSBD_GetDevInstance(pD, &dev);

    printf("BOT_APP: Application Disconnect %c\r\n", ' ');

    displayDeviceInfo(dev);
    if (configValue != 0) {
        configValue = 0;
        unConfigureDev(pD);
    }
    packet_received = 0;
    packet_sent = 0;

    botAppState = BOT_UNINIT_STATE;
}

/**
 * User callback for RESUME
 */
static void resume(CUSBD_PrivateData *pD) {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Application: resume %c\n", ' ');
}

/**
 * User callback for CONFIGURED
 */
static void configured(CUSBD_PrivateData *pD, uint32_t enabledEpMask) {
    vDbgMsg(DBG_USB_APP, 1, "Application: configured 0x%X\n", enabledEpMask);

    if ((enabledEpMask & BOT_EP_MASK) == BOT_EP_MASK) {

        if (botAppState == BOT_UNINIT_STATE) {
            // queue CBW command 
            queue_cbw(pD);
            ignoreBulkOut = 0;
        } else {
            vDbgMsg(DBG_USB_APP, DBG_HIVERB, "bot_app: Warning: botAppState %d\n", botAppState);
        }
    }
}

/**
 * User callback for SETUP
 */
static uint32_t setup(CUSBD_PrivateData *pD, CH9_UsbSetup *ctrl) {

    // get device reference
    CUSBD_Dev * dev;
    int length;

    CH9_UsbConfigurationDescriptor *configDesc = NULL;
    CH9_UsbDeviceDescriptor * devDesc = NULL;

    ctrl->wIndex = le16ToCpu(ctrl->wIndex);
    ctrl->wLength = le16ToCpu(ctrl->wLength);
    ctrl->wValue = le16ToCpu(ctrl->wValue);

    CUSBD_GetDevInstance(pD, &dev);

    configDesc = &confHsDesc;
    devDesc = &devHsDesc;
    // select descriptors according to actual speed
    switch (dev->speed) {

        case CH9_USB_SPEED_FULL:
            configDesc = &confHsDesc;
            devDesc = &devHsDesc;
            break;

        case CH9_USB_SPEED_HIGH:
            configDesc = &confHsDesc;
            devDesc = &devHsDesc;
            break;

        case CH9_USB_SPEED_SUPER:
            configDesc = &confSsDesc;
            devDesc = &devSsDesc;
            break;

        default:
            break;
    }

    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Device-Speed %d: \n", dev->speed);

    ep0Req->buf = ep0Buff;
    ep0Req->dma = ep0Buff_PhyAddr;
    ep0Req->complete = reqComplete;

    if ((devDesc == NULL) || (configDesc == NULL)) {
        return CDN_EINVAL;
    }

    switch (ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK) {

        case CH9_USB_REQ_TYPE_STANDARD:

            switch (ctrl->bRequest) {

                case CH9_USB_REQ_GET_DESCRIPTOR:
                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "GET DESCRIPTOR %c\n", ' ');
                    if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_INTERFACE) {
                        switch (ctrl->wValue >> 8) {
                            default:
                                return -1;
                        }
                    } else if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_DEVICE) {
                        switch (ctrl->wValue >> 8) {

                            case CH9_USB_DT_DEVICE:
                                length = CH9_USB_DS_DEVICE;
                                memmove(ep0Buff, devDesc, 18);
                               /* printf("CH9_USB_DT_DEVICE: bLength = %hhu\r\n",
                                        devDesc->bLength);*/
                                break;

                            case CH9_USB_DT_CONFIGURATION:
                            {
                                int offset = 0;

                                length = le16ToCpu(configDesc->wTotalLength);
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "GET DESCRIPTOR: CH9_USB_DT_CONFIGURATION current_speed(%d)\n", dev->speed);
                                // select descriptors according to actual speed
                                switch (dev->speed) {

                                    case CH9_USB_SPEED_FULL:
                                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(64);
                                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(64);
                                        break;

                                    case CH9_USB_SPEED_HIGH:
                                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(512);
                                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(512);
                                        break;
                                    case CH9_USB_SPEED_SUPER:
                                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(1024);
                                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(1024);
                                        break;

                                    default:
                                        break;
                                }


                                memmove(&ep0Buff[offset], configDesc, CH9_USB_DS_CONFIGURATION);
                                offset += CH9_USB_DS_CONFIGURATION;

                                memmove(&ep0Buff[offset], &interfaceDesc, CH9_USB_DS_INTERFACE);
                                offset += CH9_USB_DS_INTERFACE;

                                memmove(&ep0Buff[offset], &endpointEpInDesc, CH9_USB_DS_ENDPOINT);
                                offset += CH9_USB_DS_ENDPOINT;

                                if (dev->speed >= CH9_USB_SPEED_SUPER) {
                                    memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_EP_COMPANION);
                                    offset += CH9_USB_DS_SS_USB_EP_COMPANION;
                                }

                                memmove(&ep0Buff[offset], &endpointEpOutDesc, CH9_USB_DS_ENDPOINT);
                                offset += CH9_USB_DS_ENDPOINT;

                                if (dev->speed >= CH9_USB_SPEED_SUPER) {
                                    memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_EP_COMPANION);
                                    offset += CH9_USB_DS_SS_USB_EP_COMPANION;
                                }
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "ConfDesc[0] = %02X\n", configDesc->bLength);
                                break;
                            }

                            case CH9_USB_DT_STRING:
                            {
                                uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
                                char *strDesc;
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "StringDesc %c\n", ' ');
                                switch (descIndex) {
                                    case 0:
                                        strDesc = (char*) &languageDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "language %c\n", ' ');
                                        break;

                                    case 1:
                                        strDesc = (char*) &vendorDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "vendor %c\n", ' ');
                                        break;

                                    case 2:
                                        strDesc = (char*) &productDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "product %c\n", ' ');
                                        break;

                                    case 3:
                                        strDesc = (char*) &serialDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "serial %c\n", ' ');
                                        break;

                                    default:
                                        return -1;
                                }
                                memmove(ep0Buff, strDesc, length);
                                break;
                            }

                            case CH9_USB_DT_BOS:
                            {
                                int offset = 0;
                                length = le16ToCpu(bosDesc.wTotalLength);

                                memmove(ep0Buff, &bosDesc, CH9_USB_DS_BOS);
                                offset += CH9_USB_DS_BOS;
                                /*Only USB3 should support CH9_USB_DS_DEVICE_CAPABILITY_30 descriptor*/
                                if (dev->maxSpeed == CH9_USB_SPEED_SUPER || dev->maxSpeed == CH9_USB_SPEED_SUPER_PLUS) {
                                    memmove(&ep0Buff[offset], &capabilitySsDesc, CH9_USB_DS_DEVICE_CAPABILITY_30);
                                    offset += CH9_USB_DS_DEVICE_CAPABILITY_30;
                                }

                                memmove(&ep0Buff[offset], &capabilityExtDesc, CH9_USB_DS_DEVICE_CAPABILITY_20);
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "bosDesc[0] = %02X\n", bosDesc.bLength);     
                                break;
                            }

                            case CH9_USB_DT_DEVICE_QUALIFIER:
                                length = CH9_USB_DS_DEVICE_QUALIFIER;
                                memmove(ep0Buff, &qualifierDesc, length);
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "QualifierDesc %c\n", ' ');
                                break;

                            case CH9_USB_DT_OTHER_SPEED_CFG:
                            {
                                int offset = 0;

                                length = le16ToCpu(configDesc->wTotalLength);

                                if (dev->speed == CH9_USB_SPEED_SUPER)
                                    return -1;

                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "GET DESCRIPTOR: CH9_USB_DT_OTHER_SPEED_CFG speed(%d)\n", dev->speed);
                                switch (dev->speed) {

                                    case CH9_USB_SPEED_FULL:
                                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(512);
                                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(512);
                                        break;

                                    case CH9_USB_SPEED_HIGH:
                                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(64);
                                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(64);
                                        break;

                                    default:
                                        return -1;
                                }

                                memmove(&ep0Buff[offset], configDesc, CH9_USB_DS_CONFIGURATION);
                                ep0Buff[1] = CH9_USB_DS_OTHER_SPEED_CFG;
                                offset += CH9_USB_DS_CONFIGURATION;

                                memmove(&ep0Buff[offset], &interfaceDesc, CH9_USB_DS_INTERFACE);
                                offset += CH9_USB_DS_INTERFACE;

                                memmove(&ep0Buff[offset], &endpointEpInDesc, CH9_USB_DS_ENDPOINT);
                                offset += CH9_USB_DS_ENDPOINT;

                                if (dev->speed == CH9_USB_SPEED_SUPER) {
                                    memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_EP_COMPANION);
                                    offset += CH9_USB_DS_SS_USB_EP_COMPANION;
                                }

                                memmove(&ep0Buff[offset], &endpointEpOutDesc,
                                        CH9_USB_DS_ENDPOINT);
                                offset += CH9_USB_DS_ENDPOINT;

                                memmove(&ep0Buff[offset], &endpointEpOutDesc,
                                        CH9_USB_DS_ENDPOINT);
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "OtherSpeedDesc[0] = %02X\n", configDesc->bLength);
                                break;
                            }

                            default:
                                vDbgMsg(DBG_USB_APP, DBG_CRIT, "Error_1 %c\n", ' ');
                                return -1;

                        } //switch
                    } //if
                    break;

                case CH9_USB_REQ_SET_CONFIGURATION:
                {

                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
                    if (ctrl->wValue > 1)
                        return -1; // no such configuration

                    // un-configure device
                    if (ctrl->wValue == 0) {
                        if (configValue == 1) {
                            unConfigureDev(pD);
                        } else {
                            // device already un-configured
                        }
                        return 0;
                    }

                    // device already configured
                    if (configValue == 1 && ctrl->wValue == 1) {
                        return 0;
                    }

                    // configure device
                    configValue = (uint8_t) ctrl->wValue;

                    configureDev(pD, ctrl->wValue);

                    dev->state = CH9_USB_STATE_CONFIGURED;
                    /*Code control  Self powered feature of USB*/
                    if (configDesc->bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
                       /* if (drv->dSetSelfpowered) {
                            drv->dSetSelfpowered(pD);
                        }*/
                    } else {
                       /* if (drv->dClearSelfpowered) {
                            drv->dSetSelfpowered(pD);
                        }*/
                    }
                }
                    return 0;

                case CH9_USB_REQ_GET_CONFIGURATION:
                    length = 1;
                    ep0Buff[0] = configValue;
                    break;

                case CH9_USB_REQ_SET_INTERFACE:
                    if ((ctrl->wValue != 0) || (ctrl->wIndex != 0) || (ctrl->wLength != 0))return -1;
                    length = 0;
                    setInterfaceFlag = 1;
                    unConfigureDev(pD);
                    break;

                case CH9_USB_REQ_GET_INTERFACE:
                    if ((ctrl->wValue != 0) || (ctrl->wIndex != 0) || (ctrl->wLength != 1))return -1;
                    length = 1;
                    ep0Buff[0] = 0;
                    break;

                case CH9_USB_REQ_GET_STATUS:
                    if ((ctrl->wValue != 0) || (ctrl->wIndex != 0) || (ctrl->wLength != 2))return -1;
                    length = 2;
                    ep0Buff[0] = 0;
                    ep0Buff[1] = 0;
                    break;

                default:
                    vDbgMsg(DBG_USB_APP, DBG_CRIT, "Error_2 %c\n", ' ');
                    return -1; //return error
            }
            break;

        case CH9_USB_REQ_TYPE_CLASS:

            if (ctrl->bmRequestType & CH9_USB_EP_DIR_MASK) {
                if ((ctrl->bRequest == 0xFE) && (ctrl->wValue == 0) && (ctrl->wLength == 1) && (ctrl->wIndex == 0)) {
                    devGetMaxLun(ep0Buff);
                    //printf("[bot_app.c]: devGetMaxLun: %hhu\r\n", ep0Buff[0]);
                    length = 1;
                } else {
                    return -1;
                }
            } else {
                if ((ctrl->bRequest != 0xFF) || (ctrl->wValue != 0) || (ctrl->wLength != 0) || (ctrl->wIndex != 0)) {
                    return -1;
                } else {
                    //printf("[bot_app.c]: MSC RESET-start\r\n");
                    epIn->ops->epSetHalt(pD, epIn, 0);
                    //printf("[bot_app.c]: MSC RESET-end\r\n");
                    length = 0;
                    ignoreBulkOut = 0;
                    if (botAppState != BOT_UNINIT_STATE) {
                        queue_cbw(pD);
                    }
                }
            }
            break;
    }

    if (length > 0) {
        ep0Req->length = ctrl->wLength < length ? ctrl->wLength : length;
        dev->ep0->ops->reqQueue(pD, dev->ep0, ep0Req);
    }
    return 0;
}

/**
 * User callback for SUSPEND
 */
static void suspend(CUSBD_PrivateData *pD) {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Application: suspend %c\n", ' ');
}

// --------------------- debug functions ---------------------------------------
/**
 * DEBUG FUNCTION: Displays request information
 */
static void displayRequestinfo(CUSBD_Req * req) {
    char requestInfo[REQ_INFO_BUFFER_SZ + 1];
    int strOffset = 0;

    memset(requestInfo, 0, REQ_INFO_BUFFER_SZ + 1);

    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "buf: 0x%lX    ", (uintptr_t) req->buf);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "length: %d    ", req->length);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dma: 0x%lX    ", req->dma);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "streamId: %04X    ", req->streamId);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "noInterrupt: %d    ", req->noInterrupt);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "status: %d    ", req->status);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "actual: %d    ", req->actual);

    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "%s\n", requestInfo);
}

/**
 * DEBUG FUNCTION: Displays CBW request
 */
static void displayCBWRequest(cbw_t * cbw) {
    char requestInfo[REQ_INFO_BUFFER_SZ + 1];
    int strOffset = 0;

    memset(requestInfo, 0, REQ_INFO_BUFFER_SZ + 1);

    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dCBWSignature: 0x%X    ", le32ToCpu(cbw->dCBWSignature));
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dCBWTag: 0x%X    ", le32ToCpu(cbw->dCBWTag));
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dCBWDataTransferLength: %u    ", le32ToCpu(cbw->dCBWDataTransferLength));
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "bmCBWFlags: 0x%hhX    ", cbw->bmCBWFlags);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "bmCBWLUN: %hhu    ", cbw->bmCBWLun);
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "bmCBWLength: %hhu    ", cbw->bmCBWLength);
    printf( "%s\n", requestInfo);
}

/**
 * DEBUG FUNCTION: Displays CSW response
 */
static void displayCSWResponse(csw_t * csw) {
    char requestInfo[REQ_INFO_BUFFER_SZ + 1];
    int strOffset = 0;
    memset(requestInfo, 0, REQ_INFO_BUFFER_SZ + 1);

    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dCSWSignature: 0x%X    ", le32ToCpu(csw->dCSWSignature));
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dCSWTag: 0x%X    ", le32ToCpu(csw->dCSWTag));
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dCSWDataResidue: %d    ", le32ToCpu(csw->dCSWDataResidue));
    strOffset += snprintf(&requestInfo[strOffset], REQ_INFO_BUFFER_SZ - strOffset, "dCSWStatus: %d    ", csw->dCSWStatus);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "%s\n", requestInfo);
}

/**
 * DEBUG FUNCTION: Displays endpoint information
 */
static void displayEndpointInfo(CUSBD_Ep * ep) {
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "-------Endpoint INFO------------- %c\n", ' ');
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "address: %02X\n", ep->address);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "epList: %08X\n", (uintptr_t) & ep->epList);
}

/**
 * DEBUG FUNCTION: Displays device information
 */
static void displayDeviceInfo(CUSBD_Dev * dev) {
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "-------Device INFO--------------- %c\n", ' ');
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "epList: prev = %08X, next = %08X\n", (uintptr_t) dev->epList.prev, (uintptr_t) dev->epList.next);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "ep0: %08X\n", (uintptr_t) dev->ep0);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "speed: %d\n", dev->speed);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "maxSpeed: %d\n", dev->maxSpeed);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "state: %d\n", dev->state);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "sgSupported: %d\n", dev->sgSupported);
}

//user callbacks
static CUSBD_Callbacks callback = {disconnect, connect, setup, configured, suspend, resume};

/**
 * reads data from storage and sends it to EP-IN endpoint
 */
static uint32_t storage_to_usb(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid) {

    uint32_t status = CDN_EOK;
    uint32_t size = num_of_sec * SECTOR_SIZE;

    if (!data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if ((dev_num_of_bytes == 0) || (host_num_of_bytes == 0) || (StorageBase == NULL)) {
        return 0;
    }

    bulkInReq->buf = StorageBase + start_sec * SECTOR_SIZE;
    bulkInReq->dma = StorageBasePhyAddr + start_sec * SECTOR_SIZE;
    bulkInReq->complete = onDataXferCmpl;
    bulkInReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkInReq->streamId = 0;

    status = epIn->ops->reqQueue(&cusbdPrivData, epIn, bulkInReq);

    if (status == CDN_EOK) {
        return bulkInReq->length;
    }
    return 0;
}

/**
 * sends data from EP-OUT to storage memory
 */
static uint32_t usb_to_storage(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid) {

    uint32_t status = CDN_EOK;
    uint32_t size = num_of_sec * SECTOR_SIZE;

    if (data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if (dev_num_of_bytes == 0 || host_num_of_bytes == 0) {
        return 0;
    }

    bulkOutReq->buf = StorageBase + start_sec * SECTOR_SIZE;
    bulkOutReq->dma = StorageBasePhyAddr + start_sec * SECTOR_SIZE;
    bulkOutReq->complete = onDataXferCmpl;
    bulkOutReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkOutReq->streamId = 0;

    status = epOut->ops->reqQueue(&cusbdPrivData, epOut, bulkOutReq);
    if (status == CDN_EOK) {
        return bulkOutReq->length;
    }
    return 0;
}

/**
 * sends SCSI non-data-xfer response on IN-EP
 */
static uint32_t send_data(void *buff, uint32_t size, uint16_t sid) {

    uint32_t status = CDN_EOK;
    //printf("[bot_app.c] cmdIdx(%u) BOT DATA IN<- size(%u)\r\n", cmdIdx, size);

    if (!data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if (dev_num_of_bytes == 0 || host_num_of_bytes == 0) {
        return 0;
    }

    bulkInReq->buf = buff;
    bulkInReq->dma = (uintptr_t)buff;//CPS_GetPhyAddrOfVPtr((void *) buff);
    bulkInReq->complete = onDataXferCmpl;
    bulkInReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkInReq->streamId = 0;

    status = epIn->ops->reqQueue(&cusbdPrivData, epIn, bulkInReq);

    if (status == CDN_EOK) {
        return bulkInReq->length;
    }
    return 0;
}

/**
 * sends SCSI non-data-xfer response on OUT-EP
 */
static uint32_t rec_data(void *buff, uint32_t size, uint16_t sid) {

    uint32_t status = CDN_EOK;
    //printf("[bot_app.c] cmdIdx(%u) BOT DATA OUT-> size(%u)\r\n", cmdIdx, size);

    if (data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if (dev_num_of_bytes == 0 || host_num_of_bytes == 0) {
        return 0;
    }
    
    bulkOutReq->buf = buff;
    bulkOutReq->dma = (uintptr_t)buff;//CPS_GetPhyAddrOfVPtr((void *) buff);
    bulkOutReq->complete = onDataXferCmpl;
    bulkOutReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkOutReq->streamId = 0;

    status = epOut->ops->reqQueue(&cusbdPrivData, epOut, bulkOutReq);

    if (status == CDN_EOK) {
        return bulkOutReq->length;
    }
    return 0;
}

/**
 * queue CBW
 */
static uint32_t queue_cbw(CUSBD_PrivateData *pD) {

    uint32_t status;

    bulkOutReq->buf = command_buff;
    bulkOutReq->dma = command_buff_phy_addr;
    bulkOutReq->complete = process_cbw;
    bulkOutReq->length = BOT_CBW_BUF_SZ;
    bulkOutReq->streamId = 0;

    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Queuing CBW cmdIdx(%d)\n", cmdIdx);
    cmdIdx++;

    status = epOut->ops->reqQueue(&cusbdPrivData, epOut, bulkOutReq);
    if (status == CDN_EOK) {
        botAppState = BOT_CBW_STATE;
    } else {
        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Queuing CBW Error cmdIdx(%d) status(%d) \n", cmdIdx, status);
        msc_failed();
    }
    return status;
}

/**
 * CSW transfer complete callback
 */
static void onCSWXferCmpl(CUSBD_Ep *ep, CUSBD_Req * req) {

    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "status:%d actual:%d\n", req->status, req->actual);
    if (req->status == CDN_EOK) {
        /* transfer success */
        queue_cbw(&cusbdPrivData);
    } else {
        msc_failed();
    }
}

/**
 * send CBW
 */
static void sendCSW(uint8_t *csw_buffer, uintptr_t csw_buffer_phy_addr) {

    uint32_t status = CDN_EOK;

    displayCSWResponse((csw_t *) csw_buffer);

    bulkInReq->buf = csw_buffer; // ( == csw)
    bulkInReq->dma = (uintptr_t) csw_buffer_phy_addr;
    bulkInReq->complete = onCSWXferCmpl;
    bulkInReq->length = CSW_STRUCT_SZ;
    bulkInReq->streamId = 0;
    status = epIn->ops->reqQueue(&cusbdPrivData, epIn, bulkInReq);

    if (status == CDN_EOK) {
        botAppState = BOT_CSW_STATE;
    }
}

/**
 * process CBW
 */
static void processCSW(csw_t * csw, uintptr_t csw_buffer_phy_addr) {
    CUSBD_PrivateData *pD = &cusbdPrivData;
    // ------- case 1, 2, 3 - host expects no data transfers
    if (host_num_of_bytes == 0) {

        // no need to handle case 1 - no errors
        // case 2 and 3
        if (dev_num_of_bytes > 0) {
            vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 2,3%s\n", "");
            csw->dCSWStatus = 2;
        }
    } else {

        // ---- case 4, 5, 6, 7, 8 - host expect to recieve data
        if ((host_num_of_bytes > 0) && data_dir) {

            // case 8
            if (dir_error) {
                printf("[bot_app.c] dir_error = 1\r\n");
                epIn->ops->epSetHalt(pD, epIn, 1);
                csw->dCSWStatus = 2;
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 8%s\n", "");
            } else
                // case 4, 5
                if (host_num_of_bytes > dev_num_of_bytes) {
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 4,5%s\n", "");

                epIn->ops->epSetHalt(pD, epIn, 1);
            } else

                // no need to handle case 6 - no errors

                // case 7
                if (host_num_of_bytes < dev_num_of_bytes) {
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 7%s\n", "");
                if ((host_num_of_bytes % 512) == 0) {
                    vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STATUS=2%s\n", "");
                    csw->dCSWStatus = 2;
                }
            }

        } else {
            // ---- case 9, 10, 11, 12, 13 - host expect to send data
            if (host_num_of_bytes > 0 && !data_dir) {

                // case 10
                if (dir_error) {
                    vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 10%s\n", "");
                    epOut->ops->epSetHalt(pD, epOut, 1);
                    vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STATUS=2%s\n", "");
                    csw->dCSWStatus = 2;
                } else {
                    // case 9, 11
                    if (host_num_of_bytes > dev_num_of_bytes) {
                        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 9,11%s\n", "");
                        printf("[bot_app.c] host_num_of_bytes(%u) dev_num_of_bytes(%u)\r\n", host_num_of_bytes, dev_num_of_bytes);
                        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STALL EP-OUT%s\n", "");
                        epOut->ops->epSetHalt(pD, epOut, 1);
                    } else {
                        // case 13
                        if (host_num_of_bytes < dev_num_of_bytes) {
                            vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 13%s\n", "");
                            vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STATUS=2%s\n", "");
                            csw->dCSWStatus = 2;
                        }
                    }
                }
            }
        }
    }

    sendCSW((uint8_t *) csw, csw_buffer_phy_addr);
}

/**
 * bulk OUT completion routine
 */
static void onDataXferCmpl(CUSBD_Ep *ep, CUSBD_Req * req) {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    displayRequestinfo(req);
    if (req->status != 0) {
        msc_failed();
    } else {
        csw_t * csw; // Command Block Status auxiliary structure
        // send CSW
        csw = (csw_t *) response_buff;
        csw->dCSWStatus = 0; /* set status to default, could be modified by processCSW */
        csw->dCSWDataResidue = cpuToLe32(host_num_of_bytes - req->actual);
        processCSW(csw, response_buff_phy_addr);
    }
}

/**
 * process CBW
 */
static void process_cbw(CUSBD_Ep *ep, CUSBD_Req * req) {
    cbw_t * cbw; // Command Block Wrapper auxiliary structure
    csw_t * csw; // Command Block Status auxiliary structure
    uint32_t data_transfered; // Number of bytes transferred on data phase
    uint16_t error_code; // error code returned from SCSI command parser
    uint32_t cbwTag;

    if ((ignoreBulkOut == 1) || (req->status))
        return;
    
    // check if we have valid command
    if (bulkOutReq->actual != 31) {
        msc_failed();
        vDbgMsg(DBG_USB_APP, DBG_CRIT, "Bad CBW length %d\n", bulkOutReq->actual);
        return; 
    }

    cbw = (cbw_t *) (bulkOutReq->buf);
    displayCBWRequest((cbw_t *) cbw);

    if (le32ToCpu(cbw->dCBWSignature) != CBW_SIGNATURE) {
        vDbgMsg(DBG_USB_APP, DBG_CRIT, "CBW Bad signature 0x%X", cbw->dCBWSignature);
        for (int idx = 0; idx < 31; idx++) {
            if ((idx % 16) == 0) printf("\r\n    +%05d..%05d: ", idx, idx + 16);
            printf("%hhX ", command_buff[idx]);
        }
        printf("\r\n");

        msc_failed();
        vDbgMsg(DBG_USB_APP, DBG_CRIT, "Bad signature!!!%s\n", "");
        return;
    }

    cbwTag = le32ToCpu(cbw->dCBWTag);

    // Execute command SCSI
    dev_num_of_bytes = 0;
    dir_error = 0;
    host_num_of_bytes = le32ToCpu(cbw->dCBWDataTransferLength);
    data_dir = cbw->bmCBWFlags & CBW_FLAG_DIR_MASK;

   // printf("[bot_app.c] cmdIdx(%u) host_num_of_bytes(%u) \r\n",
   //         cmdIdx, host_num_of_bytes);

    // handle CBW command
    error_code = scsiExecCmd((uint8_t*) cbw->cbwcb, &data_transfered, 0);

    /* Initialize the response structure */
    csw = (csw_t *) response_buff;
    csw->dCSWSignature = cpuToLe32(CSW_SIGNATURE);
    csw->dCSWTag = cbw->dCBWTag;
    csw->dCSWDataResidue = cpuToLe32(host_num_of_bytes - data_transfered);
    csw->dCSWStatus = error_code; 
    csw->dCSWTag = cpuToLe32(cbwTag);

    if (error_code == ERR_SCSI_UNKNOWN_COMMAND) {

        /* After recognizing that a CBW is valid and meaningful, the 
         * device may still fail in its attempt to satisfy the command.
         * The device shall report this condition by returning a
         * Command Failed status (bCSWStatus = 01h). However this requires 
         * driver to perform data tx/rx in order to move from data phase to 
         * status phase.
         * Hence we will simply stall
         */
        msc_failed();
    } else if (data_transfered == 0) {
        processCSW(csw, response_buff_phy_addr);
    } else {
        /* if the SCSI command needs data xfer.. wait for xfer complete */
        if (data_dir == CBW_FLAG_DIR_IN) {
            botAppState = BOT_DATAIN_STATE;
        } else {
            botAppState = BOT_DATAOUT_STATE;
        }
    }
}

/**
 * interrupt handler
 */
void bot_app_Isr(void* deviceID) {
    xhciEventRingFlush();
    xhciInputContextFlush();
    CUSBD_Isr(&cusbdPrivData);
}

#define BOT_STORAGE_BASEADDR 0x100000
/**
 * msc app initialization
 */
int mscAppInit() {

    CUSBD_Dev * dev; // USB device pointer
    CUSBD_ListHead *list; // used in for_each loop
    CUSBD_PrivateData *pD = &cusbdPrivData;

    // set unicode strings
    get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
    get_unicode_string(productDesc, USB_PRODUCT_STRING);
    get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

    // align buffers to modulo8 address
    ep0Buff = g_AppEp0Buffer;
    command_buff = g_mscCmdBuffer;
    response_buff = g_mscRespBuffer;
    StorageBase =  (uint8_t *)BOT_STORAGE_BASEADDR;
    
    ep0Buff_PhyAddr = (uintptr_t)g_AppEp0Buffer;
    command_buff_phy_addr = (uintptr_t)g_mscCmdBuffer;
    response_buff_phy_addr = (uintptr_t)g_mscRespBuffer;
    StorageBasePhyAddr = BOT_STORAGE_BASEADDR;

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    CUSBD_GetDevInstance(pD, &dev);
    displayDeviceInfo(dev);

    // allocate request for ep0
    ep0Req = &ep0ReqAlloc;

    /*Change descriptor for maxSpeed == HS only Device*/
    /*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
    if (dev->maxSpeed < CH9_USB_SPEED_SUPER) {
        bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
        bosDesc.bNumDeviceCaps = 1;
        devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
    }

    scsiInit(g_scsiRespBuffer); // init SCSI driver
    scsi_send_data = send_data;
    scsi_rec_data = rec_data;
    storage_send_data = storage_to_usb;
    storage_rec_data = usb_to_storage;

    // find IN endpoint
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epIn = (CUSBD_Ep *) list;
        displayEndpointInfo(epIn);
        if (epIn->address == BULK_EP_IN) {
            bulkInReq = &bulkInReqAlloc;
            break;
        }
    }

    // find OUT endpoint
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epOut = (CUSBD_Ep *) list;
        displayEndpointInfo(epIn);
        if (epOut->address == BULK_EP_OUT) {
            bulkOutReq = &bulkOutReqAlloc;
            break;
        }
    }
    return 0;
}
void init_bot_config(CUSBD_PrivateData *pD)
{
    memset(&(pD->config), 0, sizeof(CUSBD_Config));
    pD->config.forcedUsbMode=1;  //high speed
    pD->config.regBase=USB_REG_BASE;
    pD->config.xhciConfig.deviceModeFlag=1;
    pD->config.xhciConfig.xhciMemRes=&g_xhciMemRes;
    pD->config.xhciConfig.otgRegs=USB_REG_BASE;
    pD->config.xhciConfig.hostRegs=USB_REG_BASE+0x8000;
    pD->config.xhciConfig.deviceRegs=USB_REG_BASE+0x4000;
}
/*
 * Mass storage application
 */
int bot_app_Init(u32 usbid) {

    uint32_t res; // keeps result of last operation on driver
    
    CUSBD_PrivateData *pD = &cusbdPrivData;
    USBSSP_DriverResourcesT *sspRef=&pD->xhciDriverResources;
    
    assignXhciMemory();

    memset(&ep0ReqAlloc, 0, sizeof (ep0ReqAlloc));
    memset(&bulkInReqAlloc, 0, sizeof (bulkInReqAlloc));
    memset(&bulkOutReqAlloc, 0, sizeof (bulkOutReqAlloc));

    pD->ep0RespBuff = g_AppEp0ResBuffer;
    /* Init cusbd */
    res = CUSBD_Init(pD, &callback);

    if (res != 0) goto error;
    vDbgMsg(DBG_USB_APP, 1, "Initializing OK! %s\n", " ");

    mscAppInit();
    sspRef = get_USBSSP_Obj(pD);
    sspRef->usbsspCallbacks.setInterfaceCallback = setInterfaceCallback;    
    init_bot_config(pD);
    
    CUSBD_Start(pD); // start driver

    if (usbid == 1) {
      FGicPs_Connect(&IntcInstance, USB1_INT_ID, (FMSH_InterruptHandler)bot_app_Isr, NULL);
      FGicPs_Enable(&IntcInstance, USB1_INT_ID);
    } else {
      FGicPs_Connect(&IntcInstance, USB0_INT_ID, (FMSH_InterruptHandler)bot_app_Isr, NULL);
      FGicPs_Enable(&IntcInstance, USB0_INT_ID);
    }

    while (1) {

    }
error:
    return (res);
}

