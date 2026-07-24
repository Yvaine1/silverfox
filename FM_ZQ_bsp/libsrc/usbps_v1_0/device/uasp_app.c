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
 * uasp_app.c
 * uasp application
 *
 *****************************************************************************/

#include <stdlib.h>                        // used for malloc
#include <stdio.h>                         // standard library
#include <string.h>                        // standard library
#include "cdn_log.h"

#include "usbData.h"

#include "bot.h"         // BOT protocol
#include "storage.h"     // storage layer
#include "scsi.h"        // SCSI layer
#include "msc_config.h"  // application's configuration

//#include "cusbd_app_config.h"
#include "cdn_xhci_priv.h"
#include "uasp.h"
#include "fmsh_gic.h" 



#define EPOUT_AUX_BUFFER_SZ  0x00001000

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
/* UASP endpoints addresses */
#define EP_COMMAND_ADDR  0x02
#define EP_STATUS_ADDR   0x82
#define EP_DATA_IN_ADDR  0x81
#define EP_DATA_OUT_ADDR 0x01


#define UASP_EP_MASK    0x3C
#define BOT_EP_MASK     0xC
#define BOT_CBW_BUF_SZ  128     /* Size of CBW buffer for fetching request */

/* container for SCSI commands */
uasp_transfer_t uasp_command_container[CMD_QUE_DEEP];
sense_iu_struct hwBuffer;

//------------------------------------------------------------------------------
//extern uintptr_t CPS_GetPhyAddrOfVPtr(const void *ptr);

extern USBSSP_DriverResourcesT * get_USBSSP_Obj(CUSBD_PrivateData * pD);
/******************************************************************************
 * Local static data structures
 * ****************************************************************************/
// ---------- Super Speed USB driver configuration -----------------------------
static CUSBD_OBJ * drv; // driver pointer
static CUSBD_PrivateData cusbdPrivData;
static uint8_t appInitializedFlag = 0;

// static allocation of request for default, IN and OUT endpoints
static CUSBD_Req *ep0Req;//, *bulkInReq, *bulkOutReq;
static CUSBD_Req ep0ReqAlloc;//, bulkInReqAlloc, bulkOutReqAlloc;

//static memory allocation for default endpoint data buffer
static uint8_t *ep0Buff;
static uintptr_t ep0Buff_PhyAddr;

static CUSBD_Req uaspCmdReq;

// endpoint objects
static CUSBD_Ep * epStatus = NULL;
static CUSBD_Ep * epCommand = NULL;
static CUSBD_Ep * epDataIn = NULL;
static CUSBD_Ep * epDataOut = NULL;

// endpoint descriptors
static uint8_t * epStatusDesc = NULL;
static uint8_t * epCommandDesc = NULL;
static uint8_t * epDataInDesc = NULL;
static uint8_t * epDataOutDesc = NULL;

// --------flags used for synchronization---------------------------------------
static volatile uint8_t configValue = 0; // keeps actual configuration value
static uint8_t current_speed = CH9_USB_SPEED_UNKNOWN; // keeps actual speed value
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

static uint32_t cmdIdx = 0;
static UASP_CMD_STATE_T uaspCmdState = UASP_CMD_STATE_INIT;

static int ignoreBulkOut = 0;
static MSC_MODE_T msc_mode = MSC_MODE_BOT;

static uint8_t tagInUse[16];
static uint32_t configMask;

/******************************************************************************
 * Local static functions
 * ****************************************************************************/
static uint32_t uasp_queue_cmd(CUSBD_PrivateData *pD);
static void sendStatus(uasp_transfer_t * uasp_obj, uint16_t code);

static void onDataXferCmpl(CUSBD_Ep *ep, CUSBD_Req * req);
static void process_cbw(CUSBD_Ep *ep, CUSBD_Req * req);

// functions used for transferring SCSI data
static uint32_t send_data(void *buff, uint32_t size, uint16_t sid);

// debug functions
static void displayRequestinfo(CUSBD_Req * req);
static void displayEndpointInfo(CUSBD_Ep * ep);
static void displayDeviceInfo(CUSBD_Dev * dev);


//------------------------------------------------------------------------------
// USB30 device descriptor
static const uint8_t dev_desc_rom[] = {
    0x12, // bLength: 18
    0x01, // bDescriptorType: DEVICE
    0x20,
    0x03, // bcdUSB: Version 3.0
    0x00, // bDeviceClass: 0x00
    0x00, // bDeviceSubClass: 0x00
    0x00, // bDeviceProtocol: 0x00
    0x9, // bMaxPacketSize0: 16
    0x59,
    0x05, // idVendor: Cadence
    0x02,
    0x40, // idProduct
    0x01,
    0x00, // bcdDevice: 1.0
    0x01, // iManufacturer: String 1
    0x02, // iProduct: String 2
    0x03, // iSerialNumber:
    0x01 // bNumConfigurations: 1
};
static uint8_t* dev_desc;


//------------------------------------------------------------------------------
// USB20 device descriptor
static const uint8_t dev_hs_desc_rom[] = {
    0x12, // bLength: 18
    0x01, // bDescriptorType: DEVICE
    0x10,
    0x02, // bcdUSB: Version 2.0
    0x00, // bDeviceClass: 0x00
    0x00, // bDeviceSubClass: 0x00
    0x00, // bDeviceProtocol: 0x00
    64, // bMaxPacketSize0: 16
    0x59,
    0x05, // idVendor: Cadence
    0x40,
    0x02, // idProduct:
    0x01,
    0x00, // bcdDevice: 1.0
    0x01, // iManufacturer: String 1
    0x02, // iProduct: String 2
    0x03, // iSerialNumber:
    0x01 // bNumConfigurations: 1
};
static uint8_t* dev_hs_desc;


//------------------------------------------------------------------------------
// mass storage super speed configuration descriptor
static const uint8_t config_desc_rom[] = {
    0x09, // bLength: 
    0x02, // bDescriptorType: CH9_USB_DT_CONFIGURATION
    0x79, // wTotalLength_l
    0x00, // wTotalLength_h
    0x01, // bNumInterfaces
    0x01, // bConfigurationValue
    0x00, // iConfiguration:
    0xC0, // bmAttributes
    0x00, // bMaxPower

    /* BOT bInterfaceNumber: 0 bAlternateSetting: 0 */
    0x09, // bLength: 
    0x04, // bDescriptorType: CH9_USB_DT_INTERFACE
    0x00, // bInterfaceNumber: 0
    0x00, // bAlternateSetting: 0
    0x02, // bNumEndpoints: 2
    0x08, // bInterfaceClass:(MassStorage) https://www.usb.org/defined-class-codes
    0x06, // bInterfaceSubClass: (SCSI transparent command set) Mass storage specification overview (Ch 2)
    0x50, // bInterfaceProtocol: BOT (usbmassbulk_10.pdf)
    0x00, // iInterface: Index of string descriptor describing this interface

    // EP2_IN: STATUS + DATA-IN
    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x82, // bEndpointAddress EP2_IN
    0x02, // bmAttributes 0x2: BULK
    0x00, // wMaxPacketSize_L
    0x04, // wMaxPacketSize_H 0x400 = 1024
    0x00, // bInterval

    0x06, // bLength:
    0x30, // bDescriptorType: CH9_USB_DT_SS_USB_EP_COMPANION
    0x0F, // bMaxBurst:
    0x00, // bmAttributes: 
    0x00, // wBytesPerInterval_L: not needed for BULK
    0x00, // wBytesPerInterval_H

    // EP2_OUT: Command (CBW) + DATA-OUT
    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x02, // bEndpointAddress EP2_OUT
    0x02, // bmAttributes 0x2: BULK
    0x00, // wMaxPacketSize_L
    0x04, // wMaxPacketSize_H 0x400 = 1024
    0x00, // bInterval

    0x06, // bLength:
    0x30, // bDescriptorType: CH9_USB_DT_SS_USB_EP_COMPANION
    0x0F, // bMaxBurst:
    0x00, // bmAttributes: 
    0x00, // wBytesPerInterval_L: not needed for BULK
    0x00, // wBytesPerInterval_H

    /* UASP: bInterfaceNumber: 0 bAlternateSetting: 1 */
    0x09, // bLength:
    0x04, // bDescriptorType: CH9_USB_DT_INTERFACE
    0x00, // bInterfaceNumber: 0
    0x01, // bAlternateSetting: 1
    0x04, // bNumEndpoints: 4
    0x08, // bInterfaceClass:(MassStorage) https://www.usb.org/defined-class-codes
    0x06, // bInterfaceSubClass: (SCSI transparent command set) Mass storage specification overview (Ch 2)
    0x62, // bInterfaceProtocol: UAS: (INCTIS_471-2010)
    0x00, // iInterface: Index of string descriptor describing this interface

    /* EP1_IN: DATA-IN Endpoint */
    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x81, // bEndpointAddress: EP1_IN 
    0x02, // bmAttributes 0x2: BULK
    0x00, // wMaxPacketSize_L
    0x04, // wMaxPacketSize_H 0x400 = 1024
    0x00, // bInterval

    0x06, // bLength:
    0x30, // bDescriptorType: CH9_USB_DT_SS_USB_EP_COMPANION
    0x0F, // bMaxBurst:
    NUM_STREAMS_LG2, // bmAttributes: NumStreams = 2^3 = 8
    0x00, // wBytesPerInterval_L: not needed for BULK
    0x00, // wBytesPerInterval_H

    0x04, // bLength:
    0x24, // bDescriptorType: Pipe Usage Class Descriptor (INCTIS_471-2010)
    0x03, // bPipeId: 3- Data-in Pipe (INCTIS_471-2010)
    0x00, // Reserved

    /* EP1_OUT DATA-OUT Endpoint */
    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x01, // bEndpointAddress: EP1_OUT
    0x02, // bmAttributes 0x2: BULK
    0x00, // wMaxPacketSize_L
    0x04, // wMaxPacketSize_H 0x400 = 1024
    0x00, // bInterval

    0x06, // bLength:
    0x30, // bDescriptorType: CH9_USB_DT_SS_USB_EP_COMPANION
    0x0F, // bMaxBurst:
    NUM_STREAMS_LG2, // bmAttributes: NumStreams = 2^3 = 8
    0x00, // wBytesPerInterval_L: not needed for BULK
    0x00, // wBytesPerInterval_H

    0x04, // bLength:
    0x24, // bDescriptorType: Pipe Usage Class Descriptor (INCTIS_471-2010)
    0x04, // bPipeId: 4 - Data-out pipe
    0x00, // Reserved

    /* EP2_OUT Command Endpoint (non-stream)*/
    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x02, // bEndpointAddress: EP2_OUT
    0x02, // bmAttributes 0x2: BULK
    0x00, // wMaxPacketSize_L
    0x04, // wMaxPacketSize_H 0x400 = 1024
    0x00, // bInterval

    0x06, // bLength:
    0x30, // bDescriptorType: CH9_USB_DT_SS_USB_EP_COMPANION
    0x00, // bMaxBurst:
    0x00, // bmAttributes: 0
    0x00, // wBytesPerInterval_L: not needed for BULK
    0x00, // wBytesPerInterval_H

    0x04, // bLength:
    0x24, // bDescriptorType: Pipe Usage Class Descriptor (INCTIS_471-2010)
    0x01, // bPipeId: 1-Command Pipe
    0x00, // Reserved

    /* EP2_IN Status Endpoint */
    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x82, // bEndpointAddress
    0x02, // bmAttributes 0x2: BULK
    0x00, // wMaxPacketSize_L
    0x04, // wMaxPacketSize_H 0x400 = 1024
    0x00, // bInterval

    0x06, // bLength:
    0x30, // bDescriptorType: CH9_USB_DT_SS_USB_EP_COMPANION
    0x0F, // bMaxBurst:
    NUM_STREAMS_LG2, // bmAttributes: NumStreams = 2^3 = 8
    0x00, // wBytesPerInterval_L: not needed for BULK
    0x00, // wBytesPerInterval_H

    0x04, // bLength:
    0x24, // bDescriptorType: Pipe Usage Class Descriptor (INCTIS_471-2010)
    0x02, // bPipeId: 2- Status Pipe
    0x00 // Reserved
};



//------------------------------------------------------------------------------
// mass storage high speed configuration descriptor
static const uint8_t config_hs_desc_rom[] = {
    0x09, // bLength:
    0x02, // bDescriptorType: CH9_USB_DT_CONFIGURATION
    32,
    0x00,
    0x01,
    0x01,
    0x00,
    0xC0,
    0x00,

    0x09, // bLength:
    0x04, // bDescriptorType: CH9_USB_DT_INTERFACE
    0x00, // bInterfaceNumber: 0
    0x00, // bAlternateSetting: 0
    0x02, // bNumEndpoints: 2
    0x08, // bInterfaceClass:(MassStorage) https://www.usb.org/defined-class-codes
    0x06, // bInterfaceSubClass: (SCSI transparent command set) Mass storage specification overview (Ch 2)
    0x50, // bInterfaceProtocol: BOT (usbmassbulk_10.pdf)
    0x00, // iInterface: Index of string descriptor describing this interface

    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x81,
    0x02,
    0x00,
    0x02,
    0x00,

    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x01,
    0x02,
    0x00,
    0x02,
    0x00,
};


//------------------------------------------------------------------------------
// mass storage high speed configuration descriptor
static const uint8_t config_fs_desc_rom[] = {
    0x09, // bLength:
    0x02, // bDescriptorType: CH9_USB_DT_CONFIGURATION
    32,
    0x00,
    0x01,
    0x01,
    0x00,
    0xC0,
    0x00,

    0x09, // bLength:
    0x04, // bDescriptorType: CH9_USB_DT_INTERFACE
    0x00, // bInterfaceNumber: 0
    0x00, // bAlternateSetting: 0
    0x02, // bNumEndpoints: 2
    0x08, // bInterfaceClass:(MassStorage) https://www.usb.org/defined-class-codes
    0x06, // bInterfaceSubClass: (SCSI transparent command set) Mass storage specification overview (Ch 2)
    0x50, // bInterfaceProtocol: BOT (usbmassbulk_10.pdf)
    0x00, // iInterface: Index of string descriptor describing this interface

    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x81,
    0x02,
    0x40,
    0x00,
    0x00,

    0x07, // bLength:
    0x05, // bDescriptorType: CH9_USB_DT_ENDPOINT
    0x01,
    0x02,
    0x40,
    0x00,
    0x00,
};


//------------------------------------------------------------------------------
// language string descriptor
static const uint8_t language_string_rom[] = {
    // String Descriptor Zero (Index 0)
    0x04,
    0x03,
    0x09, 0x04,
};


//------------------------------------------------------------------------------
// manufacturer string descriptor
static const uint8_t desc_manufacturer_string_rom[] = {
    // Manufacturer String Descriptor (Index 1)
    0x10,
    0x03,
    'C', 0x00,
    'a', 0x00,
    'd', 0x00,
    'e', 0x00,
    'n', 0x00,
    'c', 0x00,
    'e', 0x00
};


//------------------------------------------------------------------------------
// product string descriptor
static const uint8_t desc_product_string_rom[] = {
    // Product String Descriptor (Index 2)
    0x1A,
    0x03,
    'M', 0x00,
    'a', 0x00,
    's', 0x00,
    's', 0x00,
    '-', 0x00,
    's', 0x00,
    't', 0x00,
    'o', 0x00,
    'r', 0x00,
    'a', 0x00,
    'g', 0x00,
    'e', 0x00,
};


//------------------------------------------------------------------------------
// product serial string descriptor
static const uint8_t desc_serial_string_rom[] = {
    // Product String Descriptor (Index 3)
    0x1A,
    0x03,
    '1', 0,
    '1', 0,
    '1', 0,
    '1', 0,

    '1', 0,
    '1', 0,
    '1', 0,
    '1', 0,

    '1', 0,
    '1', 0,
    '1', 0,
    '1', 0,
};



//------------------------------------------------------------------------------
// BOS descriptor
static const uint8_t desc_bos_rom[] = {
    0x05, // bLength: 5
    CH9_USB_DT_BOS, // bDescriptorType: BOS
    0x16, 0x00, // wTotalLength: 22 (5+10+7)
    0x02, // bNumDeviceCaps
    // Device Capabiity Descriptor
    0x0a, // bLength : 10
    CH9_USB_DT_DEVICE_CAPABILITY, // bDescriptorType : DEVICE CAPABILITY
    0x03, // bDevCapabilityType : SUPERSPEED_USB
    0x00, // bmAttributes : LTM not supported
    0x0E, 0x00, // wSpeedSupported SuperSpeed only (ToDo : FullSpeed & HighSpeed)
    0x01, // bFunctionalitySupport : SupperSpeed (ToDo : >= FullSpeed)
    0x04, // bU1DevExitLat : 4 us
    0x00, 0x02, // wU2DevExitLat : 512 us
    // Device Capabiity Descriptor
    0x07, // bLength : 7
    CH9_USB_DT_DEVICE_CAPABILITY, // bDescriptorType : DEVICE CAPABILITY
    0x02, // bDevCapabilityType : USB 2.0 EXTENSION
    0x02, // bmAttributes : LPM supported
    0x00, //
    0x00, //
    0x00, //
};

static uint8_t setInterfaceFlag = 0;

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
    //drv->getDevInstance(pD, &dev);
    CUSBD_GetDevInstance(pD, &dev);
    configValue = (uint8_t) ctrlValue;

    /* find command endpoint */
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        ep = (CUSBD_Ep *) list;
        if (ep->address == EP_COMMAND_ADDR) {
            ep->ops->epEnable(pD, ep, (uint8_t*) epCommandDesc);
            epCommand = ep;
            break;
        }
    }

    /* find status endpoint */
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        ep = (CUSBD_Ep *) list;
        if (ep->address == EP_STATUS_ADDR) {
            ep->ops->epEnable(pD, ep, (uint8_t*) epStatusDesc);
            epStatus = ep;
            break;
        }
    }

    /* find data IN endpoint */
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        ep = (CUSBD_Ep *) list;
        if (ep->address == EP_DATA_IN_ADDR) {
            ep->ops->epEnable(pD, ep, (uint8_t*) epDataInDesc);
            epDataIn = ep;
            break;
        }
    }

    /* find data OUT endpoint */
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        ep = (CUSBD_Ep *) list;
        if (ep->address == EP_DATA_OUT_ADDR) {
            ep->ops->epEnable(pD, ep, (uint8_t*) epDataOutDesc);
            epDataOut = ep;
            break;
        }
    }
}

/**
 * un-configure the device
 */
static void uasp_UnConfigureDev(CUSBD_PrivateData *pD) {
    CUSBD_Dev * dev;
    configValue = 0;
    //drv->getDevInstance(pD, &dev);
    CUSBD_GetDevInstance(pD, &dev);
    if (epCommand != NULL) {
        epCommand->ops->epDisable(pD, epCommand);
        epCommand = NULL;
    }
    if (epStatus != NULL) {
        epStatus->ops->epDisable(pD, epStatus);
        epStatus = NULL;
    }
    if (epDataIn != NULL) {
        epDataIn->ops->epDisable(pD, epDataIn);
        epDataIn = NULL;
    }
    if (epDataOut != NULL) {
        epDataOut->ops->epDisable(pD, epDataOut);
        epDataOut = NULL;
    }

    dev->state = CH9_USB_STATE_ADDRESS;
}

// -------------- driver callback functions ------------------------------------
static void uasp_setInterfaceCallback(USBSSP_DriverResourcesT* res, USBSSP_SetInterfaceState *configEpCmd, uint32_t addMask) 
{
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "UASP_APP: SetInterface callback %d\n", setInterfaceFlag);        
    if (setInterfaceFlag) { 
        configMask |= addMask;
        if (configMask == UASP_EP_MASK) {
            if (*configEpCmd == USBSSP_EP_DISABLE) {
                *configEpCmd = USBSSP_EP_ENABLE;
                configMask = 0;  
                if (msc_mode == MSC_MODE_BOT) {
                    configureDev(res->privateData, 1);
                }
            } else if (*configEpCmd == USBSSP_EP_ENABLE) {
                setInterfaceFlag = 0;
                *configEpCmd = USBSSP_EP_CONFIGURE;
                uaspCmdState = UASP_CMD_STATE_ACTIVE;
                configMask = 0;
            }
        }
    }
    return ;
}

/**
 * User callback for CONNECT
 */
static void connect(CUSBD_PrivateData *pD) {
    //CUSBD_Dev *dev;

    //CUSBD_GetDevInstance(pD, &dev);

   // printf("UASP_APP: Application connect at %d speed\r\n", dev->speed);
}

/**
 * User callback for DISCONNECT
 */
static void disconnect(CUSBD_PrivateData *pD) {
    CUSBD_Dev *dev;
   // if (!drv)
    //    return;
    CUSBD_GetDevInstance(pD, &dev);

    printf("UASP_APP: Application Disconnect %c\r\n", ' ');
    
    uaspCmdState = UASP_CMD_STATE_INIT;

    displayDeviceInfo(dev);
    if (configValue != 0) {
        configValue = 0;

        uasp_UnConfigureDev(pD);
    }
    packet_received = 0;
    packet_sent = 0;
    appInitializedFlag = 0;
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

    if (((enabledEpMask & UASP_EP_MASK) == UASP_EP_MASK) && (msc_mode == MSC_MODE_UASP)) {

        if (uaspCmdState == UASP_CMD_STATE_ACTIVE) {
            for (int i = 0; i < 16; i++)
                tagInUse[i] = 0U;
            // queue CBW command 
            uasp_queue_cmd(pD);
        } else {
            vDbgMsg(DBG_USB_APP, DBG_HIVERB, "UASP_app: Warning: AppState %d\n", uaspCmdState);
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

    // select descriptors according to actual speed
    switch (dev->speed) 
    {

        case CH9_USB_SPEED_FULL:
            configDesc = (CH9_UsbConfigurationDescriptor *) config_fs_desc_rom;
            devDesc = (CH9_UsbDeviceDescriptor *) dev_hs_desc_rom;
            epStatusDesc = (uint8_t *) & config_fs_desc_rom[18];
            epCommandDesc = (uint8_t *) & config_fs_desc_rom[25];
            break;

        case CH9_USB_SPEED_HIGH:
            configDesc = (CH9_UsbConfigurationDescriptor *) config_hs_desc_rom;
            devDesc = (CH9_UsbDeviceDescriptor *) dev_hs_desc_rom;
            epStatusDesc = (uint8_t *) & config_hs_desc_rom[18];
            epCommandDesc = (uint8_t *) & config_hs_desc_rom[25];
            break;

        case CH9_USB_SPEED_SUPER:
            configDesc = (CH9_UsbConfigurationDescriptor *) config_desc_rom;
            devDesc = (CH9_UsbDeviceDescriptor *) dev_desc_rom;
            epStatusDesc = (uint8_t *) & config_desc_rom[104];
            epCommandDesc = (uint8_t *) & config_desc_rom[87];
            epDataInDesc = (uint8_t *) & config_desc_rom[53];
            epDataOutDesc = (uint8_t *) & config_desc_rom[70];
            break;

        default:
            configDesc = (CH9_UsbConfigurationDescriptor *) config_desc_rom;
            devDesc = (CH9_UsbDeviceDescriptor *) dev_desc_rom;
            epStatusDesc = (uint8_t *) & config_desc_rom[104];
            epCommandDesc = (uint8_t *) & config_desc_rom[87];
            epDataInDesc = (uint8_t *) & config_desc_rom[53];
            epDataOutDesc = (uint8_t *) & config_desc_rom[70];
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
                                memmove(ep0Buff, devDesc, length);
                                printf("CH9_USB_DT_DEVICE: bLength = %hhu\r\n",
                                        devDesc->bLength);
                                break;

                            case CH9_USB_DT_CONFIGURATION:
                            {
                                int offset = 0;

                                length = le16ToCpu(configDesc->wTotalLength);
                                current_speed = dev->speed;
                                memmove(ep0Buff, configDesc, length);
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
                                        strDesc = (char*) language_string_rom;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "language %c\n", ' ');
                                        break;

                                    case 1:
                                        strDesc = (char*) desc_manufacturer_string_rom;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "vendor %c\n", ' ');
                                        break;

                                    case 2:
                                        strDesc = (char*) desc_product_string_rom;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "product %c\n", ' ');
                                        break;

                                    case 3:
                                        strDesc = (char*) desc_serial_string_rom;
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
                                length = le16ToCpu(*((uint16_t*) & desc_bos_rom[2]));

                                memmove(ep0Buff, desc_bos_rom, CH9_USB_DS_BOS);
                                offset += CH9_USB_DS_BOS;
                                /*Only USB3 should support CH9_USB_DS_DEVICE_CAPABILITY_30 descriptor*/
                                if (dev->maxSpeed == CH9_USB_SPEED_SUPER || dev->maxSpeed == CH9_USB_SPEED_SUPER_PLUS)
                                {
                                    memmove(&ep0Buff[offset], &desc_bos_rom[5], CH9_USB_DS_DEVICE_CAPABILITY_30);
                                    offset += CH9_USB_DS_DEVICE_CAPABILITY_30;
                                }

                                memmove(&ep0Buff[offset], &desc_bos_rom[15], CH9_USB_DS_DEVICE_CAPABILITY_20);
                                break;
                            }

                            case CH9_USB_DT_DEVICE_QUALIFIER:
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Error QualifierDesc %c\n", ' ');
                                return -1;


                            case CH9_USB_DT_OTHER_SPEED_CFG:
                            {
                                length = le16ToCpu(configDesc->wTotalLength);

                                if (dev->speed == CH9_USB_SPEED_SUPER) return -1;

                                switch (dev->speed) {

                                    case CH9_USB_SPEED_FULL:
                                        memmove(ep0Buff, config_hs_desc_rom, length);
                                        break;

                                    case CH9_USB_SPEED_HIGH:
                                        memmove(ep0Buff, config_fs_desc_rom, length);
                                        break;

                                    default:
                                        return -1;
                                }
                                ep0Buff[1] = CH9_USB_DS_OTHER_SPEED_CFG;
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
                    CUSBD_Ep * ep;
                    CUSBD_ListHead *list;

                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
                    if (ctrl->wValue > 1) return -1; // no such configuration

                    // un-configure device
                    if (ctrl->wValue == 0) {
                        configValue = 0;
                        for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                            ep = (CUSBD_Ep *) list;
                            if (ep->address == EP_COMMAND_ADDR) {
                                ep->ops->epDisable(pD, ep);
                                break;
                            }
                        }
                        for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                            ep = (CUSBD_Ep *) list;
                            if (ep->address == EP_STATUS_ADDR) {
                                ep->ops->epDisable(pD, ep);
                                break;
                            }
                        }
                        dev->state = CH9_USB_STATE_ADDRESS;
                        return 0;
                    }

                    // device already configured
                    if (configValue == 1 && ctrl->wValue == 1) {
                        return 0;
                    }

                    /* configure device */
                    configureDev(pD, ctrl->wValue);
                    /* set configured state flag */
                    dev->state = CH9_USB_STATE_CONFIGURED;

                    /*Code control  Self powered feature of USB*/
                    if (configDesc->bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
                        /*if (drv->dSetSelfpowered) {
                            drv->dSetSelfpowered(pD);
                        }*/
                    } else {
                       /* if (drv->dClearSelfpowered) {
                            drv->dSetSelfpowered(pD);
                        }*/
                    }
                }
                    break;

                case CH9_USB_REQ_GET_CONFIGURATION:
                    length = 1;
                    ep0Buff[0] = configValue;
                    break;

                case CH9_USB_REQ_SET_INTERFACE:
                {
                    if (ctrl->wLength != 0U) {
                        // illegal value of wLength
                        return -1;
                    } else if ((ctrl->wIndex == 0) && (ctrl->wValue == 0U)) {
                        // BOT mode
                        msc_mode = MSC_MODE_BOT;
                        setInterfaceFlag = 1U;
                        uasp_UnConfigureDev(pD);
                    } else if ((ctrl->wIndex == 0) && (ctrl->wValue == 1U)) {
                        // UASP mode
                        msc_mode = MSC_MODE_UASP;
                        setInterfaceFlag = 1U; 
                        if (epCommand != NULL)                        
                            epCommand->ops->epSetHalt(pD, epCommand, 0);
                        if (epStatus != NULL)                         
                            epStatus->ops->epSetHalt(pD, epStatus, 0);
                        if (epDataIn != NULL)                         
                            epDataIn->ops->epSetHalt(pD, epDataIn, 0);
                        if (epDataOut != NULL)                         
                            epDataOut->ops->epSetHalt(pD, epDataOut, 0);                      
                        configureDev(pD, 1);                        
                    } else {
                        return -1;
                    }
                    length = 0;
                    break;
                }

                case CH9_USB_REQ_GET_INTERFACE:
                    if ((ctrl->wValue != 0) || (ctrl->wIndex != 0) || (ctrl->wLength != 1))return -1;
                    length = 1;
                    if (msc_mode == MSC_MODE_UASP) ep0Buff[0] = 1;
                    else ep0Buff[0] = 0;
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
                   // printf("[uasp_app.c]: devGetMaxLun: %hhu\r\n", ep0Buff[0]);
                    length = 1;
                } else {
                    return -1;
                }
            } else {
                if ((ctrl->bRequest != 0xFF) || (ctrl->wValue != 0) || (ctrl->wLength != 0) || (ctrl->wIndex != 0)) {
                    return -1;
                } else {
                    printf("[uasp_app.c]: UASP RESET-start\r\n");
                    epCommand->ops->epSetHalt(pD, epCommand, 0);
                    epStatus->ops->epSetHalt(pD, epStatus, 0);
                    printf("[uasp_app.c]: MSC RESET-end\r\n");
                    length = 0;
                    ignoreBulkOut = 0;
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

/** UASP FUNCTIONS **/

/**
 * Data transfer completion callback
 * @param ep endpoint on which transfer completes
 * @param req request which has been completed
 */
static void dataCmpl(CUSBD_Ep *ep, CUSBD_Req * req) {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Transfer complete on ep:%02X %08X req \n", ep->address, (uintptr_t) req);
    /*
     * send status only when data completed successfully
     */
    if (req->status == 0) {
        uasp_transfer_t * uasp_obj = &uasp_command_container[req->streamId - 1];
        uasp_obj->status.streamId = req->streamId;
        sendStatus(uasp_obj, SAM_STAT_GOOD);
    } else {
        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "req status on ep:%02X %d req\n", ep->address, req->status);    
    }
}

/**
 * sends SCSI non-data-xfer response on OUT-EP
 */
static uint32_t rec_data(void *buff, uint32_t size, uint16_t sid) {

    uint32_t status = CDN_EOK;
    printf("[uasp_app.c] cmdIdx(%u) BOT DATA OUT-> size(%u)\r\n", cmdIdx, size);

    uasp_transfer_t * uasp_obj = &uasp_command_container[sid - 1U];
    CUSBD_Req * req = &uasp_obj->reqOut;

    req->buf = buff;
    req->dma = (uintptr_t) buff;//CPS_GetPhyAddrOfVPtr((void *) buff);
    req->complete = dataCmpl;
    req->length = size;
    req->streamId = 0;

    status = epDataOut->ops->reqQueue(&cusbdPrivData, epDataOut, req);

    if (status == CDN_EOK) {
        return req->length;
    }
    return 0;
}

/**
 * callback for SCSI driver for sending SCSI data to host
 * @param buff place in memory where SCSI data is located
 * @param size size in bytes of SCSI data
 * @param sid stream ID on which data should be transfered
 * @return number of bytes transfered: here we return number of bytes which we
 *         assume will be transfered successfully
 */
static uint32_t send_data(void *buff, uint32_t size, uint16_t sid) {

    uasp_transfer_t * uasp_obj = &uasp_command_container[sid - 1U];
    CUSBD_Req * req = &uasp_obj->reqIn;

    req->buf = (uint8_t*) buff;
    req->dma = (uintptr_t) buff;
    req->length = size;
    req->streamId = sid;
    req->complete = dataCmpl;

    epDataIn->ops->reqQueue(&cusbdPrivData, epDataIn, req);
    return size;
}


/**
 * function returns filled transfer request for transferring data to/from storage device
 * @param start_sec starting sector
 * @param num_of_sec number of sectors
 * @param sid stream on which data should be moved
 * @param retReq returns request object for transferring data to/from storage device
 * @return number of transfered bytes
 */
static uint32_t fillReq(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid, CUSBD_Req * req) {

    uint32_t size = num_of_sec * SECTOR_SIZE;

    req->buf = (uint8_t *) StorageBase + start_sec * SECTOR_SIZE;
    req->dma = (uintptr_t) StorageBasePhyAddr + start_sec * SECTOR_SIZE;
    req->complete = dataCmpl;
    req->length = size;
    req->streamId = sid;

    return size;
}

/**
 * callback for storage driver moving data from storage to endpoint
 * @param start_sec starting sector
 * @param num_of_sec number of sectors
 * @param sid stream on which data should be moved
 * @return number of transfered bytes
 */
static uint32_t storage_to_usb(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid) {

    uasp_transfer_t * uasp_obj = &uasp_command_container[sid - 1U];    
    CUSBD_Req * req = &uasp_obj->reqIn;
    uint32_t size = fillReq(start_sec, num_of_sec, sid, req);
    epDataIn->ops->reqQueue(&cusbdPrivData, epDataIn, req);
    return size;
}

/**
 * callback for storage driver moving data from endpoint to storage
 * @param start_sec starting sector
 * @param num_of_sec number of sectors
 * @param sid stream on which data should be moved
 * @return number of transfered bytes
 */
static uint32_t usb_to_storage(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid) {

    uasp_transfer_t * uasp_obj = &uasp_command_container[sid - 1U];    
    CUSBD_Req * req = &uasp_obj->reqOut;
    uint32_t size = fillReq(start_sec, num_of_sec, sid, req);
    epDataOut->ops->reqQueue(&cusbdPrivData, epDataOut, req);
    return size;
}


/**
 * Status completion
 */
static void statusCmpl(CUSBD_Ep *ep, CUSBD_Req * req) {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "UASP status complete: %d\n", req->status);
    tagInUse[req->streamId] = 0U;    
}

/**
 * sending status to host
 * @param uasp_obj uasp object
 */
static void sendStatus(uasp_transfer_t * uasp_obj, uint16_t code) {

    CUSBD_PrivateData *pD = &cusbdPrivData;
    CUSBD_Req * req = &uasp_obj->status;
    
    sense_iu_struct * status_ptr;
    
    memset(response_buff + (sizeof(sense_iu_struct) * req->streamId), 0, sizeof(sense_iu_struct));    
    status_ptr = (sense_iu_struct *) (response_buff  + (sizeof(sense_iu_struct) * req->streamId));

    /* fill status structure */
    if (code == SAM_STAT_CHECK_CONDITION) {
        status_ptr->length = be16ToCpu(18);
        status_ptr->sense_data[0] = 0x70;	/* fixed, current */
        status_ptr->sense_data[2] = 0x05;
        status_ptr->sense_data[7] = 0xa;
        status_ptr->sense_data[12] = 0x20;
        status_ptr->sense_data[13] = 0;
    } else {
        status_ptr->length = 0x0000;
    }
    status_ptr->status = code; 
    status_ptr->iu_header.iu_id = SENSE_IU;
    status_ptr->status_qualifier = 0;
    status_ptr->iu_header.tag = be16ToCpu(req->streamId);
    
    /* fill request object */
    req->buf = (uint8_t*) response_buff + (sizeof(sense_iu_struct) * req->streamId);
    req->dma = (uintptr_t) response_buff_phy_addr  + (sizeof(sense_iu_struct) * req->streamId);
    req->length = 16 + cpuToBe16(status_ptr->length);
    req->complete = statusCmpl;
    
    /* send request to driver */
    epStatus->ops->reqQueue(pD, epStatus, req);
}

/**
 * sending response to host
 * @param uasp_obj uasp object
 */
static void sendResponse(uasp_transfer_t * uasp_obj, uint16_t code) {
    
    CUSBD_PrivateData *pD = &cusbdPrivData;
    CUSBD_Req * req = &uasp_obj->response;
    
    response_iu_struct * response_ptr;
    
    memset(response_buff + (sizeof(response_iu_struct) * req->streamId), 0, sizeof(response_iu_struct));
    response_ptr = (response_iu_struct *) (response_buff  + (sizeof(response_iu_struct) * req->streamId));

    /* fill response structure */
    response_ptr->iu_header.iu_id = RESPONSE_IU;
    response_ptr->iu_header.tag = be16ToCpu(req->streamId);
    response_ptr->response_code = be16ToCpu(code);
    
    /* fill request object */
    req->buf = (uint8_t*) response_buff  + (sizeof(response_iu_struct) * req->streamId);
    req->dma = (uintptr_t) response_buff_phy_addr  + (sizeof(response_iu_struct) * req->streamId);
    req->length = 8;
    req->complete = statusCmpl;
    
    /* send request to driver */
    epStatus->ops->reqQueue(pD, epStatus, req);   
}

/**
 * command completion handler
 * @param ep endpoint on which command has completed
 * @param req request which has been completed
 */
static void commandCmpl(CUSBD_Ep *ep, CUSBD_Req * req) {

    if (req->status == 0) {

        /* keeps current UASP object */
        uasp_transfer_t * uasp_obj;

        /* returns then number of bytes transfered on SCSI command */
        uint32_t data_transfered;

        /* get command's tag ID which is a stream number */
        uint16_t sid = (((uint16_t) req->buf[2]) << 8) | ((uint16_t) req->buf[3]);

        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "UASP Command for SID: %d\n", sid);

        command_iu_struct *cmd = (command_iu_struct *) req->buf;
        
        /* get UASP object from object container */
        uasp_obj = &uasp_command_container[sid - 1U];
        
        uasp_obj->response.streamId = sid;
        uasp_obj->status.streamId = sid;
        
        CUSBD_PrivateData *pD = &cusbdPrivData;
        
        // queue CBW command 
        uasp_queue_cmd(pD);
                  
        if (cmd->iu_header.iu_id == COMMAND_IU) {
            if ((tagInUse[sid] == TASK_MANAGEMENT_IU) || (tagInUse[sid] == COMMAND_IU)) {                       
                sendResponse(uasp_obj, OVERLAPPED_TAG_ATTEMPTED);                
            } else {
                /* execute SCSI command */
                uint16_t ret = scsiExecCmd((uint8_t*) & cmd->cdb, &data_transfered, sid);    
                tagInUse[sid] = COMMAND_IU;
                /**
                 * Some SCSI commands don't have data phase and status should be send
                 * back to host just after command completion
                 */
                if (data_transfered == 0U) {
                    if (ret == ERR_SCSI_UNKNOWN_COMMAND) {
                        sendStatus(uasp_obj, SAM_STAT_CHECK_CONDITION);                    
                    } else {
                        sendStatus(uasp_obj, SAM_STAT_GOOD);
                    }
                }
            }
	} else if (cmd->iu_header.iu_id == TASK_MANAGEMENT_IU){
            task_management_iu_struct *tmiu = (task_management_iu_struct *) cmd;
            if ((tagInUse[sid] == TASK_MANAGEMENT_IU) || (tagInUse[sid] == COMMAND_IU)) {
               sendResponse(uasp_obj, OVERLAPPED_TAG_ATTEMPTED);                      
            } else if ((tmiu->logical_unit_number) != 0) {
                sendResponse(uasp_obj, INCORRECT_LOGICAL_UNIT_NUMBER);
            } else if (tmiu->task_management_function == 0xFF){                
                sendResponse(uasp_obj, TASK_MANAGEMENT_FUNCTION_NOT_SUPPORTED);             
            } else { 
                sendResponse(uasp_obj, TASK_MANAGEMENT_FUNCTION_COMPLETE);
            }
            tagInUse[sid] = TASK_MANAGEMENT_IU;
        } else {
            sendResponse(uasp_obj, INVALID_INFORMATION_UNIT);          
        }
    } else {
        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "UASP Command Error: %d\n", req->status);
    }
}

/**
 * queue command
 */
static uint32_t uasp_queue_cmd(CUSBD_PrivateData *pD) {

    uint32_t ret = CDN_EOK;
    /*
     * On FPGA platform we need to set buffer for SCSI command to common
     * memory where USB controller has access to it
     */
    static int i = 0;
    if ((i*32) > 512)
        i = 0;
    uaspCmdReq.buf = (uint8_t*) command_buff + (i * 32);
    uaspCmdReq.dma = (uintptr_t) command_buff_phy_addr + (i * 32);
    uaspCmdReq.length = 32U; /* command size is 32 in UASP class */
    uaspCmdReq.complete = commandCmpl;
    uaspCmdReq.streamId = 0U; /* command channel doesn't use stream */
    
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "UASP Command queue: %d %d\n", cmdIdx++, i);
    i++;
    /* send request to driver */
    ret = epCommand->ops->reqQueue(pD, epCommand, &uaspCmdReq);

    return ret;
}

/**
 * interrupt handler
 */
void uasp_app_Isr(void* deviceID) {
    CUSBD_Isr(&cusbdPrivData);
}


#define UASP_STORAGE_BASEADDR 0x100000

/**
 * uasp app initialization
 */
int uaspInit() {

    CUSBD_Dev * dev; // USB device pointer
    CUSBD_ListHead *list; // used in for_each loop
    CUSBD_PrivateData *pD = &cusbdPrivData;

    // align buffers to modulo8 address
    ep0Buff = g_AppEp0Buffer;
    command_buff = g_mscCmdBuffer;
    response_buff = g_mscRespBuffer;
    StorageBase = (uint8_t *)UASP_STORAGE_BASEADDR;

    ep0Buff_PhyAddr = (uintptr_t)g_AppEp0Buffer;
    command_buff_phy_addr = (uintptr_t)g_mscCmdBuffer;
    response_buff_phy_addr = (uintptr_t)g_mscRespBuffer;
    StorageBasePhyAddr = UASP_STORAGE_BASEADDR;

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    //drv->getDevInstance(pD, &dev);
    //displayDeviceInfo(dev);

    // allocate request for ep0
    ep0Req = &ep0ReqAlloc;

    scsiInit(g_scsiRespBuffer); // init SCSI driver
    scsi_send_data = send_data;
    scsi_rec_data = rec_data;
    storage_send_data = storage_to_usb;
    storage_rec_data = usb_to_storage;

    for (int i = 0; i < CMD_QUE_DEEP; i++)
        uasp_command_container[i].hw_buff = &hwBuffer;
    return 0;
}
void init_uasp_config(CUSBD_PrivateData *pD)
{
    memset(&(pD->config), 0, sizeof(CUSBD_Config));
    pD->config.forcedUsbMode=1;  //high speed
    pD->config.regBase=USB_BASEADDR;
    pD->config.xhciConfig.deviceModeFlag=1;
    pD->config.xhciConfig.xhciMemRes=&xhciMemRes;
    pD->config.xhciConfig.otgRegs=USB_BASEADDR;
    pD->config.xhciConfig.hostRegs=USB_BASEADDR+0x8000;
    pD->config.xhciConfig.deviceRegs=USB_BASEADDR+0x4000;
}
/*
 * UASP application
 */
int uasp_app_Init() {

    uint32_t res; // keeps result of last operation on driver
   
    CUSBD_PrivateData *pD = &cusbdPrivData;
    USBSSP_DriverResourcesT *sspRef=&pD->xhciDriverResources;
    
    assignXhciMemory();

    pD->ep0RespBuff = g_ep0ResBuffer;
    /* Init cusbd */
    res = CUSBD_Init(pD, &callback);

    if (res != 0) goto error;
    vDbgMsg(DBG_USB_APP, 1, "Initializing OK! %s\n", " ");
    uaspInit();
    sspRef->usbsspCallbacks.setInterfaceCallback = uasp_setInterfaceCallback;
    init_uasp_config(pD);
    CUSBD_Start(pD); // start driver
     
    FGicPs_Connect(&IntcInstance, USB0_INT_ID, (FMSH_InterruptHandler)uasp_app_Isr, NULL);
    FGicPs_Enable(&IntcInstance, USB0_INT_ID);
   
    while (1) {

    }
error:
    return (res);
}
