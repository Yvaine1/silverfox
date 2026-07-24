#ifndef DFU_APP_H
#define DFU_APP_H

#include <stdint.h>
#include <stdio.h>

#define USB_CRL_APB       (0xFF5E0000U)
#define USB_SLCR          (0xFF9D0000U)
#define USB_SOF_REF_CLK   (0x4cU)
#define USB_AXI_PORT_CTRL (0x210U)
#define RST_LPD_TOP       (0x23cU)
#define USB_MASTER_ID     (0x60U)
#define USB_BUS_REF_CTRL  (0x60U)
#define USB_BUS_CLKACT    BIT(25)
#define USB_SOF_CLKACT    BIT(25)
#define USB_APB_RESET     BIT(10)
#define USB_PWRUP_RESET   BIT(8)
#define USB_BRIDGE_RESET  BIT(6)
uint32_t FmshFsbl_InitDfu(u32 DeviceFlags);
u32 FmshFsbl_UsbAccess(u32 SrcAddress, u32 DestAddress, u32 Length);

#endif
