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
 * USB 3.0 specification USB 2.0 specification And others Specification
 * defined by USB organization  Because of when this file was creating
 * USB 3.1 specification was the latest published so it mostly based
 * on this specification. USB 3.1 specification does not contains all
 * descriptors and definitions included in older specification so
 * cusb_ch9_if.h file also refers to other specification defined by
 * USB organization.
 **********************************************************************/

#ifndef CUSB_CH9_IF_H
#define CUSB_CH9_IF_H

#include "cdn_stdtypes.h"

/** @defgroup ConfigInfo  Configuration and Hardware Operation Information
 *  The following definitions specify the driver operation environment that
 *  is defined by hardware configuration or client code. These defines are
 *  located in the header file of the core driver.
 *  @{
 */

/**********************************************************************
 * Defines
 **********************************************************************/
/** Data transfer direction */
#define CH9_USB_DIR_HOST_TO_DEVICE      0U

#define CH9_USB_DIR_DEVICE_TO_HOST      (1U << 7U)

/** Type of request */
#define CH9_USB_REQ_TYPE_MASK           (3U << 5U)

#define CH9_USB_REQ_TYPE_STANDARD       (0U << 5U)

#define CH9_USB_REQ_TYPE_CLASS          (1U << 5U)

#define CH9_USB_REQ_TYPE_VENDOR         (2U << 5U)

#define CH9_USB_REQ_TYPE_OTHER          (3U << 5U)

/** Recipient of request */
#define CH9_REQ_RECIPIENT_MASK          0x0fU

#define CH9_USB_REQ_RECIPIENT_DEVICE    0U

#define CH9_USB_REQ_RECIPIENT_INTERFACE 1U

#define CH9_USB_REQ_RECIPIENT_ENDPOINT  2U

#define CH9_USB_REQ_RECIPIENT_OTHER     3U

/** Standard  Request Code (chapter 9.4, Table 9-5 of USB Spec) */
#define CH9_USB_REQ_GET_STATUS          0U

#define CH9_USB_REQ_CLEAR_FEATURE       1U

#define CH9_USB_REQ_SET_FEATURE         3U

#define CH9_USB_REQ_SET_ADDRESS         5U

#define CH9_USB_REQ_GET_DESCRIPTOR      6U

#define CH9_USB_REQ_SET_DESCRIPTOR      7U

#define CH9_USB_REQ_GET_CONFIGURATION   8U

#define CH9_USB_REQ_SET_CONFIGURATION   9U

#define CH9_USB_REQ_GET_INTERFACE       10U

#define CH9_USB_REQ_SET_INTERFACE       11U

#define CH9_USB_REQ_SYNCH_FRAME         12U

#define CH9_USB_REQ_SET_ENCRYPTION      13U

#define CH9_USB_REQ_GET_ENCRYPTION      14U

#define CH9_USB_REQ_SET_HANDSHAKE       15U

#define CH9_USB_REQ_GET_HANDSHAKE       16U

#define CH9_USB_REQ_SET_CONNECTION      17U

#define CH9_USB_REQ_SET_SCURITY_DATA    18U

#define CH9_USB_REQ_GET_SCURITY_DATA    19U

#define CH9_USB_REQ_SET_WUSB_DATA       20U

#define CH9_USB_REQ_LOOPBACK_DATA_WRITE 21U

#define CH9_USB_REQ_LOOPBACK_DATA_READ  22U

#define CH9_USB_REQ_SET_INTERFACE_DS    23U

#define CH9_USB_REQ_SET_SEL             48U

#define CH9_USB_REQ_ISOCH_DELAY         49U

/** Standard Descriptor Types (chapter 9.4 - Table 9-6 of USB Spec) */
#define CH9_USB_DT_DEVICE               1U

#define CH9_USB_DT_CONFIGURATION        2U

#define CH9_USB_DT_STRING               3U

#define CH9_USB_DT_INTERFACE            4U

#define CH9_USB_DT_ENDPOINT             5U

#define CH9_USB_DT_DEVICE_QUALIFIER     6U

#define CH9_USB_DT_USB2_HUB             41U

#define CH9_USB_DT_USB3_HUB             42U

/** USB 2 */
#define CH9_USB_DT_OTHER_SPEED_CFG      7U

/** USB 2 */
#define CH9_USB_DT_INTERFACE_POWER      8U

#define CH9_USB_DT_OTG                  9U

#define CH9_USB_DT_DEBUG                10U

#define CH9_USB_DT_INTERF_ASSOCIATION   11U

#define CH9_USB_DT_BOS                  15U

#define CH9_USB_DT_DEVICE_CAPABILITY    16U

#define CH9_USB_DT_SS_USB_EP_COMPANION  48U

#define CH9_USB_DT_SSP_ISO_EP_COMPANION 49U

/** Descriptor size */
#define CH9_USB_DS_DEVICE               18U

#define CH9_USB_DS_BOS                  5U

/** Capability type: USB 2.0 EXTENSION */
#define CH9_USB_DS_DEVICE_CAPABILITY_20 7U

/** Capability type: SUPERSPEED_USB */
#define CH9_USB_DS_DEVICE_CAPABILITY_30 10U

/** Capability type: CONTAINER_ID */
#define CH9_USB_DS_DEV_CAP_CONTAINER_ID 21U

/** Capability type: Capability type: PRECISION_TIME_MEASUREMENT */
#define CH9_USB_DS_DEV_CAP_PR_TIME_MEAS 4U

/** Capability type: SUPERSPEED_PLUS. Number of SSID: 1 */
#define CH9_USB_DS_DEV_CAP_SSP          20U

#define CH9_USB_DS_CONFIGURATION        9U

#define CH9_USB_DS_USB2_HUB             7U

#define CH9_USB_DS_USB3_HUB             12U

#define CH9_USB_DS_INTERF_ASSOCIATION   8U

#define CH9_USB_DS_SS_USB_EP_COMPANION  6U

#define CH9_USB_DS_SSP_ISO_EP_COMPANION 8U

#define CH9_USB_DS_INTERFACE            9U

#define CH9_USB_DS_ENDPOINT             7U

#define CH9_USB_DS_STRING               3U

#define CH9_USB_DS_OTG                  5U

/** USB2 */
#define CH9_USB_DS_DEVICE_QUALIFIER     10U

/** USB2 */
#define CH9_USB_DS_OTHER_SPEED_CFG      7U

#define CH9_USB_DS_INTERFACE_POWER      8U

/** Standard Feature Selectors (chapter 9.4, Table 9-7 of USB Spec) */
#define CH9_USB_FS_ENDPOINT_HALT        0U

#define CH9_USB_FS_FUNCTION_SUSPEND     0U

#define CH9_USB_FS_DEVICE_REMOTE_WAKEUP 1U

#define CH9_USB_FS_TEST_MODE            2U

#define CH9_USB_FS_B_HNP_ENABLE         3U

#define CH9_USB_FS_A_HNP_SUPPORT        4U

#define CH9_USB_FS_A_ALT_HNP_SUPPORT    5U

#define CH9_USB_FS_WUSB_DEVICE          6U

#define CH9_USB_FS_U1_ENABLE            48U

#define CH9_USB_FS_U2_ENABLE            49U

#define CH9_USB_FS_LTM_ENABLE           50U

#define CH9_USB_FS_B3_NTF_HOST_REL      51U

#define CH9_USB_FS_B3_RESP_ENABLE       52U

#define CH9_USB_FS_LDM_ENABLE           53U

/** Recipient Device (Figure 9-4 of USB Spec) */
#define CH9_USB_STS_DEV_SELF_POWERED    (1U << 0U)

#define CH9_USB_STS_DEV_REMOTE_WAKEUP   (1U << 1U)

#define CH9_USB_STS_DEV_U1_ENABLE       (1U << 2U)

#define CH9_USB_STS_DEV_U2_ENABLE       (1U << 3U)

#define CH9_USB_STS_DEV_LTM_ENABLE      (1U << 4U)

/** Recipient Interface (Figure 9-5 of USB Spec) */
#define CH9_USB_STS_INT_REMOTE_WAKE_CAP (1U << 0U)

#define CH9_USB_STS_INT_REMOTE_WAKEUP   (1U << 1U)

/** Recipient Endpoint (Figure 9-6 of USB Spec) */
#define CH9_USB_STS_EP_HALT             (1U << 1U)

/** Recipient Endpoint - PTM GetStatus Request(Figure 9-7 of USB Spec) */
#define CH9_USB_STS_EP_PTM_ENABLE       (1U << 0U)

#define CH9_USB_STS_EP_PTM_VALID        (1U << 1U)

#define CH9_USB_STS_EP_PTM_LNK_DLY_OFST (16U)

#define CH9_USB_STS_EP_PTM_LNK_DLY_MASK (0xFFFFU << 16U)

/**
 * Macros describing information for SetFeauture Request and FUCTION_SUSPEND
 * selector (chapter 9.4.9, Table 9-9 of USB Spec)
 */
#define CH9_USB_SF_LOW_PWR_SUSP_STATE   0x1U

#define CH9_USB_SF_REMOTE_WAKE_ENABLED  0x2U

/**
 * Standard Class Code defined by usb.org
 * (link: www.usb.org/developers/defined_class)
 */
#define CH9_USB_CLASS_INTERFACE         0x0U

#define CH9_USB_CLASS_AUDIO             0x01U

#define CH9_USB_CLASS_CDC               0x02U

#define CH9_USB_CLASS_COMMUNICATION     0x01U

#define CH9_USB_CLASS_HID               0x03U

#define CH9_USB_CLASS_PHYSICAL          0x05U

#define CH9_USB_CLASS_IMAGE             0x06U

#define CH9_USB_CLASS_PRINTER           0x07U

#define CH9_USB_CLASS_MASS_STORAGE      0x08U

#define CH9_USB_CLASS_HUB               0x09U

#define CH9_USB_CLASS_CDC_DATA          0x0AU

#define CH9_USB_CLASS_SMART_CARD        0x0BU

#define CH9_USB_CLASS_CONTENT_SEECURITY 0x0DU

#define CH9_USB_CLASS_VIDEO             0x0EU

#define CH9_USB_CLASS_HEALTHCARE        0x0FU

#define CH9_USB_CLASS_AUDIO_VIDEO       0x10U

#define CH9_USB_CLASS_DIAGNOSTIC        0xDCU

#define CH9_USB_CLASS_WIRELESS          0xE0U

#define CH9_USB_CLASS_MISCELLANEOUS     0xEFU

#define CH9_USB_CLASS_APPLICATION       0xFEU

#define CH9_USB_CLASS_VENDOR            0xFFU

/** Device Capability Types Codes (see Table 9-14 of USB Spec 3.1 */
#define CH9_USB_DCT_WIRELESS_USB        0x01U

#define CH9_USB_DCT_USB20_EXTENSION     0x02U

#define CH9_USB_DCT_SS_USB              0x03U

#define CH9_USB_DCT_CONTAINER_ID        0x04U

#define CH9_USB_DCT_PLATFORM            0x05U

#define CH9_USB_DCT_POWER_DELIVERY_CAP  0x06U

#define CH9_USB_DCT_BATTERY_INFO_CAP    0x07U

#define CH9_USB_DCT_PD_CONS_PORT_CAP    0x08U

#define CH9_USB_DCT_PD_PROV_PORT_CAPAB  0x09U

#define CH9_USB_DCT_SS_PLUS             0x0AU

#define CH9_USB_DCT_PR_TIME_MEAS        0x0BU

#define CH9_USB_DCT_WIRELESS_USB_EXT    0x0CU

/** Describe supports LPM defined in bmAttribues field of
 * USBSSP_Usb20ExtensionDescriptor */
#define CH9_USB_USB20_EXT_LPM_SUPPORT   (1U << 1U)

#define CH9_USB_USB20_EXT_BESL_ALT_HIRD (1U << 2U)

/**
 * Describe supports LTM defined in bmAttribues field
 * of USBSSP_UsbSuperSpeedDeviceCapabilityDescriptor
 */
#define CH9_USB_SS_CAP_LTM              (1U << 1U)

/**
 * Describe speed supported defined in wSpeedSupported field
 * of USBSSP_UsbSuperSpeedDeviceCapabilityDescriptor
 */
#define CH9_USB_SS_CAP_SUPPORT_LS       (1U << 0U)

#define CH9_USB_SS_CAP_SUPPORT_FS       (1U << 1U)

#define CH9_USB_SS_CAP_SUPPORT_HS       (1U << 2U)

#define CH9_USB_SS_CAP_SUPPORT_SS       (1U << 3U)

/** Describe encoding of bmSublinkSpeedAttr0 filed from
 * USBSSP_UsbSuperSpeedPlusDescriptor */
#define CH9_USB_SSP_SID_OFFSET          0U

#define CH9_USB_SSP_SID_MASK            0U 0x0000000fU

#define CH9_USB_SSP_LSE_OFFSET          4U

#define CH9_USB_SSP_LSE_MASK            (0x00000003U << CH9_USB_SSP_LSE_OFFSET)

#define CH9_USB_SSP_ST_OFFSET           6U

#define CH9_USB_SSP_ST_MASK             (0x00000003U << CH9_USB_SSP_ST_OFFSET)

#define CH9_USB_SSP_LP_OFFSET           14U

#define CH9_USB_SSP_LP_MASK             (0x00000003U << CH9_USB_SSP_LP_OFFSET)

#define CH9_USB_SSP_LSM_OFFSET          16U

#define CH9_USB_SSP_LSM_MASK            (0x0000FFFFU << CH9_USB_SSP_LSM_OFFSET)

/** Description of bmAttributes field from  Configuration Description */
#define CH9_USB_CONFIG_RESERVED         (1U << 7U)

/** Self Powered */
#define CH9_USB_CONFIG_SELF_POWERED     (1U << 6U)

/** Remote Wakeup */
#define CH9_USB_CONFIG_REMOTE_WAKEUP    (1U << 5U)

/** Definitions for bEndpointAddress field from  Endpoint descriptor */
#define CH9_USB_EP_DIR_MASK             0x80U

#define CH9_USB_EP_DIR_IN               0x80U

#define CH9_USB_EP_NUMBER_MASK          0x0fU

/** Endpoint attributes from Endpoint descriptor - bmAttributes field */
#define CH9_USB_EP_TRANSFER_MASK        0x03U

#define CH9_USB_EP_CONTROL              0x0U

#define CH9_USB_EP_ISOCHRONOUS          0x01U

#define CH9_USB_EP_BULK                 0x02U

#define CH9_USB_EP_INTERRUPT            0x03U

/** Synchronization types for ISOCHRONOUS endpoints */
#define CH9_USB_EP_SYNC_MASK            0xCU

#define CH9_USB_EP_SYNC_NO              (0x0U >> 2U)

#define CH9_USB_EP_SYNC_ASYNCHRONOUS    (0x1U >> 2U)

#define CH9_USB_EP_SYNC_ADAPTIVE        (0x02U >> 2U)

#define CH9_USB_EP_SYNC_SYNCHRONOUS     (0x03U >> 2U)

#define CH9_USB_EP_USAGE_MASK           (0x3U >> 4U)

/** Usage types for ISOCHRONOUS endpoints */
#define CH9_USB_EP_USAGE_DATA           (0U >> 4U)

#define CH9_USB_EP_USAGE_FDBCK          (0x01U >> 4U)

#define CH9_USB_EP_USAGE_IMPLICIT_FDBCK (0x02U >> 4U)

/** Usage types for INTERRUPTS endpoints */
#define CH9_USB_EP_USAGE_PERIODIC       (0U >> 4U)

#define CH9_USB_EP_USAGE_NOTIFICATION   (0x01U >> 4U)

/** Description of fields bmAttributes from OTG descriptor */
#define CH9_USB_OTG_ADP_MASK            0x4U

#define CH9_USB_OTG_HNP_MASK            0x2U

#define CH9_USB_OTG_SRP_MASK            0x1U

/**
 * Test Mode Selectors
 * See USB 2.0 spec Table 9-7
 */
#define CH9_TEST_J                      1U

#define CH9_TEST_K                      2U

#define CH9_TEST_SE0_NAK                3U

#define CH9_TEST_PACKET                 4U

#define CH9_TEST_FORCE_EN               5U

#define CH9_MAX_PACKET_SIZE_MASK        0x7ffU

#define CH9_PACKET_PER_FRAME_SHIFT      11U

/**
 * OTG status selector
 * See USB_OTG_AND_EH_2-0 spec Table 6-4
 */
#define CH9_OTG_STATUS_SELECTOR         0xF000U

/**
 *  @}
 */

/** @defgroup DataStructure Dynamic Data Structures
 *  This section defines the data structures used by the driver to provide
 *  hardware information, modification and dynamic operation of the driver.
 *  These data structures are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
 * Forward declarations
 **********************************************************************/
typedef struct CH9_UsbSetup_s CH9_UsbSetup;
typedef struct CH9_UsbDeviceDescriptor_s CH9_UsbDeviceDescriptor;
typedef struct CH9_UsbBosDescriptor_s CH9_UsbBosDescriptor;
typedef struct CH9_UsbCapabilityDescriptor_s CH9_UsbCapabilityDescriptor;
typedef struct CH9_Usb20ExtensionDescriptor_s CH9_Usb20ExtensionDescriptor;
typedef struct CH9_UsbSSDeviceCapabilityDescriptor_s
    CH9_UsbSSDeviceCapabilityDescriptor;
typedef struct CH9_UsbContainerIdDescriptor_s CH9_UsbContainerIdDescriptor;
typedef struct CH9_UsbPlatformDescriptor_s CH9_UsbPlatformDescriptor;
typedef struct CH9_UsbSSPlusDescriptor_s CH9_UsbSSPlusDescriptor;
typedef struct CH9_UsbPTMCapabilityDescriptor_s CH9_UsbPTMCapabilityDescriptor;
typedef struct CH9_UsbConfigurationDescriptor_s CH9_UsbConfigurationDescriptor;
typedef struct CH9_UsbInterfaceAssociationDescriptor_s
    CH9_UsbInterfaceAssociationDescriptor;
typedef struct CH9_UsbInterfaceDescriptor_s CH9_UsbInterfaceDescriptor;
typedef struct CH9_UsbEndpointDescriptor_s CH9_UsbEndpointDescriptor;
typedef struct CH9_UsbSSEndpointCompanionDescriptor_s
    CH9_UsbSSEndpointCompanionDescriptor;
typedef struct CH9_UsbSSPlusIsocEndpointCompanionDescriptor_s
    CH9_UsbSSPlusIsocEndpointCompanionDescriptor;
typedef struct CH9_UsbStringDescriptor_s CH9_UsbStringDescriptor;
typedef struct CH9_UsbDeviceQualifierDescriptor_s
    CH9_UsbDeviceQualifierDescriptor;
typedef struct CH9_UsbOtherSpeedConfigurationDescriptor_s
    CH9_UsbOtherSpeedConfigurationDescriptor;
typedef struct CH9_UsbHeaderDescriptor_s CH9_UsbHeaderDescriptor;
typedef struct CH9_UsbOtgDescriptor_s CH9_UsbOtgDescriptor;
typedef struct CH9_ConfigParams_s CH9_ConfigParams;
typedef struct CH9_DfuFuncDesc_s CH9_DfuFuncDescs;

/**********************************************************************
 * Enumerations
 **********************************************************************/
/** USB States defined in USB Specification */
typedef enum {
    /** Device not attached yet */
    CH9_USB_STATE_NONE = 0U,
    /** see Figure 9-1 of USB Spec */
    CH9_USB_STATE_ATTACHED = 1U,
    CH9_USB_STATE_POWERED = 2U,
    CH9_USB_STATE_DEFAULT = 3U,
    CH9_USB_STATE_ADDRESS = 4U,
    CH9_USB_STATE_CONFIGURED = 5U,
    CH9_USB_STATE_SUSPENDED = 6U,
    CH9_USB_STATE_ERROR = 7U
} CH9_UsbState;

/** Speeds defined in USB Specification */
typedef enum {
    /** unknow speed - before enumeration */
    CH9_USB_SPEED_UNKNOWN = 0U,
    /** (1,5Mb/s) */
    CH9_USB_SPEED_LOW = 1U,
    /** usb 1.1 (12Mb/s) */
    CH9_USB_SPEED_FULL = 2U,
    /** usb 2.0 (480Mb/s) */
    CH9_USB_SPEED_HIGH = 3U,
    /** usb 3.0 GEN 1  (5Gb/s) */
    CH9_USB_SPEED_SUPER = 4U,
    /** usb 3.1 GEN2 (10Gb/s) */
    CH9_USB_SPEED_SUPER_PLUS = 5U
} CH9_UsbSpeed;

/** Max packet 0 size defined in USB Specification */
typedef enum {
    /** unknow speed - before enumeration */
    CH9_USB_EP0_MAX_UNKNOWN = 0U,
    /** (1,5Mb/s) */
    CH9_USB_EP0_MAX_LOW = 8U,
    /** usb 1.1 (12Mb/s) */
    CH9_USB_EP0_MAX_FULL = 64U,
    /** usb 2.0 (480Mb/s) */
    CH9_USB_EP0_MAX_HIGH = 64U,
    /** usb 2.5 wireless */
    CH9_USB_EP0_MAX_WIRELESS = 512U,
    /** usb 3.0 GEN 1  (5Gb/s) */
    CH9_USB_EP0_MAX_SUPER = 512U,
    /** usb 3.1 GEN2 (10Gb/s) */
    CH9_USB_EP0_MAX_SUPER_PLUS = 512U
} CH9_UsbEP0Max;

/**
 *  @}
 */

#endif /* CUSB_CH9_IF_H */
