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

#include <stdio.h>   // standard library
#include <stdlib.h>  // used for malloc
#include <string.h>  // standard library

#include "byteorder.h"
#include "cdn_log.h"
#include "cdn_xhci_if.h"
#include "cdn_xhci_structs_if.h"
#include "cusb_ch9_if.h"
#include "cusb_ch9_structs_if.h"
#include "cusbd_if.h"
#include "cusbd_obj_if.h"
#include "cusbd_structs_if.h"
#include "dfucfg.h"
#include "fmsh_common.h"
#include "fmsh_usb_data.h"
#include "fmsh_gic.h" 


#define FSBL_DOWNLOAD_COMPLETE 2U

/* DFU status */
#define DFU_STATUS_OK           0x00U
/* DFU commands */
#define DFU_DETACH              0x0U
#define DFU_DNLOAD              0x1U
#define DFU_GETSTATUS           0x3U
#define DFU_ALT_SETTING         0x1U

/* DFU states */
#define STATE_APP_IDLE          0x00U
#define STATE_APP_DETACH        0x01U
#define STATE_DFU_IDLE          0x02U
#define STATE_DFU_DOWNLOAD_SYNC 0x03U
#define STATE_DFU_DOWNLOAD_BUSY 0x04U
#define STATE_DFU_DOWNLOAD_IDLE 0x05U
#define STATE_DFU_ERROR         0x0AU

#define EPOUT_AUX_BUFFER_SZ     0x00001000

/*USB application debugging*/
#define DBG_USB_APP             0x000000010U
#define DBG_USB_APP_VERBOSE     0x000000020U
#define DBG_USB_BOT_APP         0x000000040U
#define DBG_USB_SCSI_APP        0x000000008U
#define REQ_INFO_BUFFER_SZ      256U

//-------------- definitions used during configuration -------------------------
#define BULK_EP_IN              0x81U
#define BULK_EP_OUT             0x01U

//------------------------------------------------------------------------------
extern uintptr_t CPS_GetPhyAddrOfVPtr(const void *ptr);

extern USBSSP_DriverResourcesT *get_USBSSP_Obj(CUSBD_PrivateData *pD);
/******************************************************************************
 * Local static data structures
 * ****************************************************************************/
// ---------- Super Speed USB driver configuration -----------------------------
CUSBD_PrivateData
    cusbdPrivData;  // __attribute__((section(".ocm_usb_data"),aligned(64)));

// ------------- driver's resources --------------------------------------------

// static allocation of request for default, IN and OUT endpoints
static CUSBD_Req *ep0Req = NULL;

// static memory allocation for default endpoint data buffer
static uint8_t *ep0Buff = NULL;
static uintptr_t ep0Buff_PhyAddr = 0U;

static CUSBD_Req CmdReq;

/** EP0 resp buffer */
uint8_t g_DfuEp0ResBuffer[USBSSP_EP0_DATA_BUFF_SIZE] __attribute__((aligned(64)))={0U};
/** EP0 buffer */
uint8_t g_DfuEp0Buffer[USBSSP_EP0_DATA_BUFF_SIZE] __attribute__((aligned(64)))={0U};

// --------flags used for synchronization---------------------------------------
static volatile uint8_t configValue = 0U;  // keeps actual configuration value

struct FsblPs_DfuIf {
    u8 CurrState;
    u8 NextState;
    u8 CurrStatus;
    u8 GotReset;
    u32 CurrentInf; /* current interface */
    u8 GotDnloadRqst;
    u32 TotalBytesDnloaded;
    u8 DfuWaitForInterrupt;
    u8 RuntimeToDfu;
};

u8 *DfuVirtFlash = NULL;

u32 DownloadDone = 0U;
struct FsblPs_DfuIf DfuObj;

/******************************************************************************
 * Local static functions
 * ****************************************************************************/

//------------------- descriptor set--------------------------------------------
// device descriptor for SuperSpeed mode
static CH9_UsbDeviceDescriptor devSsDesc = {CH9_USB_DS_DEVICE,
                                            CH9_USB_DT_DEVICE,
                                            cpuToLe16(BCD_USB_SS),
                                            0U,
                                            0U,
                                            0U,
                                            9U,
                                            cpuToLe16(ID_VENDOR),
                                            cpuToLe16(ID_PRODUCT),
                                            cpuToLe16(BCD_DEVICE_SS),
                                            1U,
                                            2U,
                                            3U,
                                            1U};

// device descriptor for HighSpeed mode
static CH9_UsbDeviceDescriptor devHsDesc = {CH9_USB_DS_DEVICE,
                                            CH9_USB_DT_DEVICE,
                                            cpuToLe16(BCD_USB_HS),
                                            0U,
                                            0U,
                                            0U,
                                            64U,
                                            cpuToLe16(ID_VENDOR),
                                            cpuToLe16(ID_PRODUCT),
                                            cpuToLe16(BCD_DEVICE_HS),
                                            1U,
                                            2U,
                                            3U,
                                            1U};

//------------- Start of Super Speed configuration descriptors -----------------
// configuration descriptors for SuperSpeed mode
static CH9_UsbConfigurationDescriptor confSsDesc = {
    CH9_USB_DS_CONFIGURATION,
    CH9_USB_DT_CONFIGURATION,
    cpuToLe16(CH9_USB_DS_CONFIGURATION + CH9_USB_DS_INTERFACE +
              2 * CH9_USB_DS_ENDPOINT + CH9_USB_DS_INTERFACE * 2),
    1U,
    1U,
    0U,
    CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
    1U};

// 0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
static CH9_UsbInterfaceDescriptor interfaceDesc = {CH9_USB_DS_INTERFACE,
                                                   CH9_USB_DT_INTERFACE,
                                                   0U,
                                                   0U,
                                                   0U,
                                                   CH9_USB_CLASS_APPLICATION,
                                                   01U,
                                                   02U,
                                                   02U};

static CH9_UsbEndpointDescriptor endpointEpInDesc = {
    CH9_USB_DS_ENDPOINT, CH9_USB_DT_ENDPOINT, BULK_EP_IN,
    CH9_USB_EP_BULK,     cpuToLe16(512),      0U};

static CH9_UsbEndpointDescriptor endpointEpOutDesc = {
    CH9_USB_DS_ENDPOINT, CH9_USB_DT_ENDPOINT, BULK_EP_OUT,
    CH9_USB_EP_BULK,     cpuToLe16(512),      0U};
/*
static CH9_UsbInterfaceDescriptor dfuInterfaceDesc = {CH9_USB_DS_INTERFACE,
    CH9_USB_DT_INTERFACE, 0, 1, 0,
    CH9_USB_CLASS_APPLICATION, 01, 02, 04};
*/

static CH9_DfuFuncDescs dfuFuncDesc = {
    CH9_USB_DS_INTERFACE,
    0x21U,
    0x3U,   /* bmAttributes Device is only download capable bitCanDnload */
    8192U,  /*wDetatchTimeOut 8192 ms*/
    1024U,   /*wTransferSize DFU block size 1024*/
    0x0110U /*bcdDfuVersion 1.1 */
};

// 0x06, 0x30, 0x00, 0x00, 0x00, 0x00
static CH9_UsbSSEndpointCompanionDescriptor compDesc = {
    CH9_USB_DS_SS_USB_EP_COMPANION, CH9_USB_DT_SS_USB_EP_COMPANION, 0, 0,
    cpuToLe16(0)};
//------------------- End of Super Speed configuration -------------------------

//------------- Start of High Speed configuration descriptors -----------------

// configuration descriptors for HighSpeed mode
static CH9_UsbConfigurationDescriptor confHsDesc = {
    CH9_USB_DS_CONFIGURATION,
    CH9_USB_DT_CONFIGURATION,
    cpuToLe16(CH9_USB_DS_CONFIGURATION + CH9_USB_DS_INTERFACE +
              CH9_USB_DS_INTERFACE),
    1U,
    1U,
    4U,
    CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
    1U};

static char *StringList[2][6] = {
    {"UNUSED", "FMSH INC", "DFU 2.0 emulation v 1.1", "2A49876D9CC1AA4",
     "FMSH DFU Downloader", "7ABC7ABC7ABC7ABC7ABC7ABC"},
    {"UNUSED", "FMSH INC", "DFU 3.0 emulation v 1.1", "2A49876D9CC1AA4",
     "FMSH DFU Downloader", "7ABC7ABC7ABC7ABC7ABC7ABC"},
};
//-------------- BOS descriptor set start --------------------------------------

static CH9_UsbBosDescriptor bosDesc = {
    CH9_USB_DS_BOS, CH9_USB_DT_BOS,
    cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_30 +
              CH9_USB_DS_DEVICE_CAPABILITY_20),
    2U};

static CH9_UsbSSDeviceCapabilityDescriptor capabilitySsDesc = {
    CH9_USB_DS_DEVICE_CAPABILITY_30,
    CH9_USB_DT_DEVICE_CAPABILITY,
    CH9_USB_DCT_SS_USB,
    0U,              // LTM not supported
    cpuToLe16(CH9_USB_SS_CAP_SUPPORT_SS | CH9_USB_SS_CAP_SUPPORT_HS |
              CH9_USB_SS_CAP_SUPPORT_FS),
    1U,              // 1 us
    4U,              // 4 us
    cpuToLe16(512)  // 512 us
};

static CH9_UsbCapabilityDescriptor capabilityExtDesc = {
    CH9_USB_DS_DEVICE_CAPABILITY_20, CH9_USB_DT_DEVICE_CAPABILITY,
    CH9_USB_DCT_USB20_EXTENSION, 0

};

//-------------- BOS descriptor set end ----------------------------------------

static CH9_UsbDeviceQualifierDescriptor qualifierDesc = {
    CH9_USB_DS_DEVICE_QUALIFIER,
    CH9_USB_DT_DEVICE_QUALIFIER,
    cpuToLe16(0x0200),
    0xFFU,
    0x00U,
    0x00U,
    0X10U,
    0x00U,
    0x00U};

/**
 * EP0 completion routine
 */
static void reqComplete (CUSBD_Ep *ep, CUSBD_Req *req)
{
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Request on endpoint %s completed\n",
            ep->name);
}

/**
 * configure the device
 */
static void configureDev (CUSBD_PrivateData *pD, uint8_t ctrlValue)
{

}

/**
 * un-configure the device
 */
static void unConfigureDev (CUSBD_PrivateData *pD)
{
   
}
/*******************************************************************************
 *
 * This function waits for DFU reset.
 *
 * @param  None
 *
 * @return None
 *
 * @note   None.
 *
 ******************************************************************************/
static void Fsbl_DfuWaitForReset (void)
{
    /* This bit would be cleared when reset happens*/
    DfuObj.DfuWaitForInterrupt = 1U;
    dmb();
    while (DfuObj.DfuWaitForInterrupt == 0U)
    {
        ;
    }
}

/*****************************************************************************
 * This function handles setting of DFU state.
 *
 * @param	dfu_state is a value of the DFU state to be set
 *
 * @return
 *				None.
 *
 * @note		None.
 *
 ******************************************************************************/
static void Fsbl_DfuSetState (u32 DfuState)
{
    switch (DfuState)
    {
    case STATE_APP_IDLE:
    {
        DfuObj.CurrState = STATE_APP_IDLE;
        DfuObj.NextState = STATE_APP_DETACH;
        DfuObj.CurrStatus = DFU_STATUS_OK;
        DfuObj.RuntimeToDfu = 0U;
    }
    break;

    case STATE_APP_DETACH:
    {
        if (DfuObj.CurrState == STATE_APP_IDLE)
        {
            DfuObj.CurrState = STATE_APP_DETACH;
            DfuObj.NextState = STATE_DFU_IDLE;

            /* Wait For USB Reset to happen */
            Fsbl_DfuWaitForReset();

            /* Set this flag to indicate we are going from runtime to dfu mode
             */
            DfuObj.RuntimeToDfu = 1U;

            /* fall through */
        }
        else if (DfuObj.CurrState == STATE_DFU_IDLE)
        {
            /* Wait For USB Reset to happen */
            Fsbl_DfuWaitForReset();

            DfuObj.CurrState = STATE_APP_IDLE;
            DfuObj.NextState = STATE_APP_DETACH;
            DfuObj.CurrStatus = DFU_STATUS_OK;
            break;
        }
        else
        {
            /* Unsupported command. Stall the end point.*/
            DfuObj.CurrState = STATE_DFU_ERROR;
        }
    }
    break;
    case STATE_DFU_IDLE:
    {
        DfuObj.CurrState = STATE_DFU_IDLE;
        DfuObj.NextState = STATE_DFU_DOWNLOAD_SYNC;
    }
    break;
    case STATE_DFU_DOWNLOAD_SYNC:
    {
        DfuObj.CurrState = STATE_DFU_DOWNLOAD_SYNC;
    }
    break;

    case STATE_DFU_DOWNLOAD_BUSY: /* shared */
    case STATE_DFU_DOWNLOAD_IDLE:
    case STATE_DFU_ERROR:
    default:
    {
        /* Unsupported command. Stall the end point.*/
        DfuObj.CurrState = STATE_DFU_ERROR;
        // XUsbPsu_EpSetStall(&UsbInstance, 0U, XUSBPSU_EP_DIR_IN);
    }
    break;
    }
}
/**
 * User callback for CONNECT
 */
static void connect (CUSBD_PrivateData *pD)
{
    CUSBD_Dev *dev = NULL;

    CUSBD_GetDevInstance(pD, &dev);
    /*  if (dev != NULL) {
          cusbdss_devcfg_updateSpeed(dev->speed);
          vDbgMsg(DBG_GEN_MSG, DBG_FYI, "Application: connect at %d speed\n",
      dev->speed);
      }
      printf("BOT_APP: Application connect at %d speed\r\n", dev->speed);*/
}

/**
 * User callback for DISCONNECT
 */
static void disconnect (CUSBD_PrivateData *pD)
{
    CUSBD_Dev *dev = NULL; 

    CUSBD_GetDevInstance(pD, &dev);
    if (configValue != 0U)
    {
        configValue = 0U;
        unConfigureDev(pD);
    }
}

/**
 * User callback for RESUME
 */
static void resume (CUSBD_PrivateData *pD)
{
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Application: resume %c\n", ' ');
}

/**
 * User callback for CONFIGURED
 */
static void configured (CUSBD_PrivateData *pD, uint32_t enabledEpMask)
{
    vDbgMsg(DBG_USB_APP, 1, "Application: configured 0x%X\n", enabledEpMask);
}
typedef struct {
    u8 Length;
    u8 DescriptorType;
    u16 LangId[128];
} __attribute__((__packed__)) FsblPs_UsbStdStringDesc;
/**
 * process download
 */
static void process_dfu_download (CUSBD_Ep *ep, CUSBD_Req *req) { return; }
/**
 * User callback for SETUP
 */
static uint32_t setup (CUSBD_PrivateData *pD, CH9_UsbSetup *ctrl)
{
    // get device reference
    CUSBD_Dev *dev = NULL;
    u32 length = 0U;
    u32 RxBytesLeft = 0U;
    // s32 Result;

    CH9_UsbConfigurationDescriptor *configDesc = NULL;
    CH9_UsbDeviceDescriptor *devDesc = NULL;

    ctrl->wIndex = le16ToCpu(ctrl->wIndex);
    ctrl->wLength = le16ToCpu(ctrl->wLength);
    ctrl->wValue = le16ToCpu(ctrl->wValue);

    CUSBD_GetDevInstance(pD, &dev);

    // select descriptors according to actual speed
    switch (dev->speed)
    {
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

    if ((devDesc == NULL) || (configDesc == NULL))
    {
        return CDN_EINVAL;
    }

    switch (ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK)
    {
    case CH9_USB_REQ_TYPE_STANDARD:

        switch (ctrl->bRequest)
        {
        case CH9_USB_REQ_GET_DESCRIPTOR:
            vDbgMsg(DBG_USB_APP, DBG_HIVERB, "GET DESCRIPTOR %c\n", ' ');
            if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) ==
                CH9_USB_REQ_RECIPIENT_INTERFACE)
            {
                switch (ctrl->wValue >> 8U)
                {
                default:
                    return -1;
                }
            }
            else if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) ==
                     CH9_USB_REQ_RECIPIENT_DEVICE)
            {
                switch (ctrl->wValue >> 8U)
                {
                case CH9_USB_DT_DEVICE:
                    length = CH9_USB_DS_DEVICE;
                    (void)memmove(ep0Buff, devDesc, 18U);
                    break;

                case CH9_USB_DT_CONFIGURATION:
                {
                    uint32_t offset = 0;

                    length = le16ToCpu(configDesc->wTotalLength);
                    vDbgMsg(DBG_USB_APP, DBG_HIVERB,
                            "GET DESCRIPTOR: CH9_USB_DT_CONFIGURATION "
                            "current_speed(%d)\n",
                            current_speed);
                    // select descriptors according to actual speed
                    switch (dev->speed)
                    {
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

                    (void)memmove(&ep0Buff[offset], configDesc,
                            CH9_USB_DS_CONFIGURATION);
                    offset += CH9_USB_DS_CONFIGURATION;

                    (void)memmove(&ep0Buff[offset], &interfaceDesc,
                            CH9_USB_DS_INTERFACE);
                    offset += CH9_USB_DS_INTERFACE;

                    (void)memmove(&ep0Buff[offset], &dfuFuncDesc,
                            CH9_USB_DS_INTERFACE);
                    offset += CH9_USB_DS_INTERFACE;

                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "ConfDesc[0] = %02X\n",
                            configDesc->bLength);

                    break;
                }

                case CH9_USB_DT_STRING:
                {
                    FsblPs_UsbStdStringDesc StringDesc;
                    u32 LoopVar = 0U;
                    uint8_t descIndex = (uint8_t)(ctrl->wValue & 0xFFU);
                    char *strDesc = StringList[0][descIndex];
                    uint8_t StringLen = (uint8_t)strlen(strDesc);
                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "StringDesc %c\n", ' ');
                    if (0U == descIndex)
                    {
                        StringDesc.Length = 4U;
                        StringDesc.DescriptorType = 0x03U;
                        StringDesc.LangId[0] = 0x0409U;
                        /*for(descIndex=1U; descIndex < 128; ++descIndex) {
                            StringDesc.LangId[descIndex] = 0U;
                        }*/
                    }
                    /* All other strings can be pulled from the table above.*/
                    else
                    {
                        StringDesc.Length = (StringLen * 2U) + 2U;
                        StringDesc.DescriptorType = 0x03U;

                        for (LoopVar = 0U; LoopVar < StringLen; ++LoopVar)
                        {
                            StringDesc.LangId[LoopVar] = (u16)strDesc[LoopVar];
                        }
                        /* for(;LoopVar < 128; ++LoopVar) {
                                 StringDesc.LangId[LoopVar] = 0U;
                         }*/
                    }
                    length = StringDesc.Length;
                    (void)memmove(ep0Buff, &StringDesc, StringDesc.Length);

                    break;
                }

                case CH9_USB_DT_BOS:
                {
                    uint32_t offset = 0U;
                    length = le16ToCpu(bosDesc.wTotalLength);

                    (void)memmove(ep0Buff, &bosDesc, CH9_USB_DS_BOS);
                    offset += CH9_USB_DS_BOS;

                    (void)memmove(&ep0Buff[offset], &capabilityExtDesc,
                            CH9_USB_DS_DEVICE_CAPABILITY_20);
                    offset += CH9_USB_DS_DEVICE_CAPABILITY_20;

                    /*Only USB3 should support CH9_USB_DS_DEVICE_CAPABILITY_30
                     * descriptor*/
//                    if ( (dev->maxSpeed == CH9_USB_SPEED_SUPER) ||
//                        (dev->maxSpeed == CH9_USB_SPEED_SUPER_PLUS) )
                    {
                        (void)memmove(&ep0Buff[offset], &capabilitySsDesc,
                                CH9_USB_DS_DEVICE_CAPABILITY_30);
                        offset += CH9_USB_DS_DEVICE_CAPABILITY_30;
                    }

                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "bosDesc[0] = %02X\n",
                            bosDesc.bLength);
                    break;
                }

                case CH9_USB_DT_DEVICE_QUALIFIER:
                    length = CH9_USB_DS_DEVICE_QUALIFIER;
                    (void)memmove(ep0Buff, &qualifierDesc, length);
                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "QualifierDesc %c\n", ' ');
                    break;

                case CH9_USB_DT_OTHER_SPEED_CFG:
                {
                    uint32_t offset = 0U;

                    length = le16ToCpu(configDesc->wTotalLength);

                    if (dev->speed == CH9_USB_SPEED_SUPER)
                    {
                        return -1;
                    }

                    vDbgMsg(DBG_USB_APP, DBG_HIVERB,
                            "GET DESCRIPTOR: CH9_USB_DT_OTHER_SPEED_CFG "
                            "speed(%d)\n",
                            dev->speed);
                    switch (dev->speed)
                    {
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

                    (void)memmove(&ep0Buff[offset], configDesc,
                            CH9_USB_DS_CONFIGURATION);
                    ep0Buff[1] = CH9_USB_DS_OTHER_SPEED_CFG;
                    offset += CH9_USB_DS_CONFIGURATION;

                    (void)memmove(&ep0Buff[offset], &interfaceDesc,
                            CH9_USB_DS_INTERFACE);
                    offset += CH9_USB_DS_INTERFACE;

                    (void)memmove(&ep0Buff[offset], &endpointEpInDesc,
                            CH9_USB_DS_ENDPOINT);
                    offset += CH9_USB_DS_ENDPOINT;

                    if (dev->speed == CH9_USB_SPEED_SUPER)
                    {
                        (void)memmove(&ep0Buff[offset], &compDesc,
                                CH9_USB_DS_SS_USB_EP_COMPANION);
                        offset += CH9_USB_DS_SS_USB_EP_COMPANION;
                    }

                    (void)memmove(&ep0Buff[offset], &endpointEpOutDesc,
                            CH9_USB_DS_ENDPOINT);
                    offset += CH9_USB_DS_ENDPOINT;

                    (void)memmove(&ep0Buff[offset], &endpointEpOutDesc,
                            CH9_USB_DS_ENDPOINT);
                    vDbgMsg(DBG_USB_APP, DBG_HIVERB,
                            "OtherSpeedDesc[0] = %02X\n", configDesc->bLength);
                    break;
                }

                default:
                    vDbgMsg(DBG_USB_APP, DBG_CRIT, "Error_1 %c\n", ' ');
                    return -1;

                }  // switch
            }  // if
            else
            {}
            break;

        case CH9_USB_REQ_SET_CONFIGURATION:
        {
            vDbgMsg(DBG_USB_APP, DBG_HIVERB, "SET CONFIGURATION(%d)\n",
                    le16ToCpu(ctrl->wValue));
            if (ctrl->wValue > 1U)
            {
                return -1;  // no such configuration
            }

            // un-configure device
            if (ctrl->wValue == 0U)
            {
                if (configValue == 1U)
                {
                    unConfigureDev(pD);
                }
                else
                {
                    // device already un-configured
                }
                return 0;
            }

            // device already configured
            if ( (configValue == 1U) && (ctrl->wValue == 1U) )
            {
                return 0;
            }

            // configure device
            configValue = (uint8_t)ctrl->wValue;

            configureDev(pD, configValue);

            dev->state = CH9_USB_STATE_CONFIGURED;
            /*Code control  Self powered feature of USB*/
            if (configDesc->bmAttributes & CH9_USB_CONFIG_SELF_POWERED)
            {
                CUSBD_DSetSelfpowered(pD);
            }
            else
            {
                CUSBD_DSetSelfpowered(pD);
            }
        }
            return 0;

        case CH9_USB_REQ_GET_CONFIGURATION:
            length = 1;
            ep0Buff[0] = configValue;
            break;

        case CH9_USB_REQ_SET_INTERFACE:
            if ((ctrl->wValue != 0U) || (ctrl->wIndex != 0U) ||
                (ctrl->wLength != 0U))
            {
                return -1;
            }
            length = 0;
            /* Setting the alternate setting requested */
            DfuObj.CurrentInf = ctrl->wValue;
            if ((DfuObj.CurrentInf >= DFU_ALT_SETTING) ||
                (DfuObj.RuntimeToDfu == 1U))
            {
                /* Clear the flag , before entering into DFU mode from runtime
                 * mode */
                if (DfuObj.RuntimeToDfu == 1U)
                {
                    DfuObj.RuntimeToDfu = 0U;
                }

                /* Entering DFU_IDLE state */
                Fsbl_DfuSetState(STATE_DFU_IDLE);
            }
            else
            {
                /* Entering APP_IDLE state */
                Fsbl_DfuSetState(STATE_APP_IDLE);
            }
            unConfigureDev(pD);
            break;

        case CH9_USB_REQ_GET_INTERFACE:
            if ((ctrl->wValue != 0U) || (ctrl->wIndex != 0U) ||
                (ctrl->wLength != 1U))
            {
                return -1;
            }
            length = 1;
            ep0Buff[0] = 0;
            break;

        case CH9_USB_REQ_GET_STATUS:
            if ((ctrl->wValue != 0U) || (ctrl->wIndex != 0U) ||
                (ctrl->wLength != 2U))
            {
                return -1;
            }
            length = 2;
            ep0Buff[0] = 0;
            ep0Buff[1] = 0;
            break;

        default:
            vDbgMsg(DBG_USB_APP, DBG_CRIT, "Error_2 %c\n", ' ');
            return -1;  // return error
        }
        break;

    case CH9_USB_REQ_TYPE_CLASS:

        switch (ctrl->bRequest)
        {
        case DFU_DETACH:
            return -1;

        case DFU_DNLOAD:  // not supported in winXP
        {
            if (ctrl->wValue == 0U)
            {
                /* we are the start of the data, clear the download counter  */
                DfuObj.TotalBytesDnloaded = 0U;
            }

            RxBytesLeft = (u32)(ctrl->wLength);

            if (RxBytesLeft > 0U)
            {
                /*do {
                    Result = XUsbPsu_EpBufferRecv(&UsbInstance, 0U,
                &DfuVirtFlash[DfuObj.TotalBytesDnloaded], RxBytesLeft);
                }while(Result != XST_SUCCESS);*/

                ep0Req->buf = &DfuVirtFlash[DfuObj.TotalBytesDnloaded];
                ep0Req
                    ->dma = (uintptr_t)&DfuVirtFlash[DfuObj.TotalBytesDnloaded];
                ep0Req->complete = process_dfu_download;
                ep0Req->length = RxBytesLeft;
                ep0Req->streamId = 0;
                CUSBD_ReqQueue(pD, dev->ep0, ep0Req);
                DfuObj.TotalBytesDnloaded += RxBytesLeft;
                DfuObj.CurrState = STATE_DFU_DOWNLOAD_IDLE;
                DfuObj.GotDnloadRqst = 0U;
                return 0;
            }
            else
            { /*if (RxBytesLeft == 0U)*/
                DfuObj.CurrState = STATE_DFU_IDLE;
                DfuObj.GotDnloadRqst = 0U;
                // Result = 0;
                // loop test
                DfuObj.TotalBytesDnloaded = 0;
            }
        }

        break;

        case DFU_GETSTATUS:
        {
            if (DfuObj.CurrState == STATE_DFU_IDLE)
            {
                DfuObj.CurrState = STATE_DFU_DOWNLOAD_SYNC;
                ++DownloadDone;
            }
            else if (DfuObj.CurrState == STATE_DFU_DOWNLOAD_SYNC)
            {
                DfuObj.CurrState = STATE_DFU_DOWNLOAD_BUSY;
            }
            else
            {
                /*Misra C compliance*/
            }
            ep0Buff[0] = DfuObj.CurrStatus;
            ep0Buff[1] = 0U;
            ep0Buff[2] = 0U;
            ep0Buff[3] = 0U;
            ep0Buff[4] = DfuObj.CurrState;
            ep0Buff[5] = 0U;
            length = (u32)ctrl->wLength;
        }
        break;

        default:
            return -1;
        }
        break;
      default:
        return -1; 
    }

    if (length > 0U)
    {
        ep0Req->length = ctrl->wLength < length ? ctrl->wLength : length;

        return CUSBD_ReqQueue(pD, dev->ep0, ep0Req);
    }

    return 0;
}

/**
 * User callback for SUSPEND
 */
static void suspend (CUSBD_PrivateData *pD)
{
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Application: suspend %c\n", ' ');
}

// user callbacks
static CUSBD_Callbacks callback = {disconnect, connect, setup,
                                   configured, suspend, resume};

/**
 * interrupt handler
 */
void dfu_app_Isr (void *deviceID) 
{ 
    xhciEventRingFlush();
    xhciInputContextFlush();
    CUSBD_Isr(&cusbdPrivData); 
}

void init_dfu_config (CUSBD_PrivateData *pD)
{
    (void)memset(&(pD->config), 0, sizeof(CUSBD_Config));
    pD->config.forcedUsbMode = 1;
    pD->config.regBase = USB_REG_BASE;
    pD->config.xhciConfig.deviceModeFlag = 1;
    pD->config.xhciConfig.xhciMemRes = &g_xhciMemRes;
    pD->config.xhciConfig.otgRegs = USB_REG_BASE;
    pD->config.xhciConfig.hostRegs = USB_REG_BASE + 0x8000U;
    pD->config.xhciConfig.deviceRegs = USB_REG_BASE + 0x4000U;
}


u32 Fmsh_InitDfu (u32 DeviceFlags)
{
    u32 status=FMSH_SUCCESS;
    
    delay_ms(1);
    DfuVirtFlash = (u8 *)0x4000000;

    CUSBD_PrivateData *pD = &cusbdPrivData;

    status=assignXhciMemory();
    if(status != FMSH_SUCCESS){
        return FMSH_FAILURE;
    }
    else{
        /* Init cusbd */
        status = CUSBD_Init(pD, &callback);
        if(status != FMSH_SUCCESS){
            return FMSH_FAILURE;
        }
        else{
          ep0Buff = g_DfuEp0Buffer ;
          ep0Buff_PhyAddr = (uintptr_t)g_DfuEp0Buffer;
          pD->ep0RespBuff = g_DfuEp0ResBuffer;

          vDbgMsg(DBG_USB_APP, 1, "Initializing OK! %s\n", " ");
          init_dfu_config(pD);
          Fsbl_DfuSetState(STATE_DFU_IDLE);
          ep0Req = &CmdReq;

          FGicPs_Connect(&IntcInstance, USB0_INT_ID,
                         (FMSH_InterruptHandler)dfu_app_Isr, NULL);
          FGicPs_Enable(&IntcInstance, USB0_INT_ID);
          status=CUSBD_Start(pD); // start driver
          while(1)
          {
          } 
        }
    return FMSH_FAILURE;
}
}