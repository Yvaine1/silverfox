/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_usb_verify.h
*
* This file contains 
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   zzq  10/3/2023  First Release
*</pre>
******************************************************************************/

#ifndef FMSH_USB_VERIFY_H /* prevent circular inclusions */
#define FMSH_USB_VERIFY_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif
        
/**************************
*** Include Files *********************************/

#include "fmsh_common.h"
#include "dfu.h"

/************************** Constant Definitions *****************************/      

/**************************** Type Definitions *******************************/
#define USB0
#define HOST_TEST 0
#define DEVICE_TEST 1
/***************** Macros (Inline Functions) Definitions *********************/  

#define USB0_AXI_PORT_CTRL 0x210
#define USB1_AXI_PORT_CTRL 0x214
  
#define USB0_SLCR           0xFF9D0000
#define USB1_SLCR           0xFF9E0000
#define CRL_APB             0xFF5E0000
#define USB_SOF_REF_CLK     0x4c
#define RST_LPD_TOP         0x23c
#define USB_BUS_CLKACT      BIT(25)

#ifdef  USB0
#define USB_BASEADDR        0xFE200000
#define USB_REGS_BASE       0xFE200000
#define USB_SLCR            USB0_SLCR
#define USB_AXI_PORT_CTRL   USB0_AXI_PORT_CTRL
#define USB_MASTER_ID       0x60
#define USB_BUS_REF_CTRL    0x60
#define USB_SOF_CLKACT      BIT(25)
#define USB_APB_RESET       BIT(10)
#define USB_PWRUP_RESET     BIT(8)
#define USB_BRIDGE_RESET    BIT(6)
#else
#define USB_BASEADDR        0xFE300000
#define USB_REGS_BASE       0xFE300000
#define USB_SLCR            USB1_SLCR
#define USB_AXI_PORT_CTRL   USB1_AXI_PORT_CTRL
#define USB_MASTER_ID       0x61
#define USB_BUS_REF_CTRL    0x64
#define USB_SOF_CLKACT      BIT(26)
#define USB_APB_RESET       BIT(11)
#define USB_PWRUP_RESET     BIT(9)
#define USB_BRIDGE_RESET    BIT(7)
#endif
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

int FUsbPsu_example();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */