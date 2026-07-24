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
 * msc_config.h
 * BOT protocol configuration file
 *
 *****************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

#define MSC_ENDPOINT_IN                 0x81
#define MSC_ENDPOINT_OUT                0x01

#define BCD_USB_SS                      0x0320  // 3.00 version USB
#define ID_VENDOR                       0x04e8  // FMSH

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1002  // Mass storage product
#else
#define ID_PRODUCT                      0x4002  // Mass storage product
#endif

#define BCD_DEVICE_SS                   0x0010  // 0.1

#ifdef HS_FS_ONLY
#define BCD_USB_HS                      0x0200  // 2.00
#else
#define BCD_USB_HS                      0x0210  // 2.10
#endif

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "FMSH"
#define USB_PRODUCT_STRING              "Mass storage device"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long

#define SCSI_VENDOR_ID_STRING           "Generic"
#define SCSI_PRODUCT_ID_STRING          "Mass-Storage"
#define SCSI_PRODUCT_REV_LEFEL_STRING   "0100"

typedef enum {
    MSC_MODE_BOT = 0U,
    MSC_MODE_UASP = 1U
} MSC_MODE_T;

#endif // Config_H


