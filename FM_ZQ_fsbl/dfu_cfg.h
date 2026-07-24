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
 * hid_cfg.h
 * Example HID application for Device mode.
 *
 *****************************************************************************/

#ifndef DFU_CFG_H
#define DFU_CFG_H

#include "cusb_ch9_if.h"
#include "cusb_ch9_structs_if.h"

//-------------- definitions used during configuration -------------------------
#define ID_VENDOR       0x246C  //   VID
#define ID_PRODUCT      0xDF01  //   PID

#define BCD_DEVICE_SS   0x0001  // 0.1
#define BCD_DEVICE_HS   0x0200  // 2.00

#define BCD_USB_SS      0x0320  // 3.00 version USB
#define BCD_USB_HS_ONLY 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BCD_USB_HS      0x0210  // 2.10

#endif
