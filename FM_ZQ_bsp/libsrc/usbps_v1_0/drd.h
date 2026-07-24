
#ifndef CDN_DRD_H
#define CDN_DRD_H

#include "fmsh_common.h"
#include "fmsh_usb_data.h"

#define OTGSTS_STRAP(p)           (((p) & GENMASK(14, 12)) >> 12)

/* Host mode is turned on. */
#define OTGSTS_CDNSP_XHCI_READY   BIT(27)

/* "Device mode is turned on .*/
#define OTGSTS_CDNSP_DEV_READY    BIT(26)

#define CDNS_DID                  0x00
#define CDNS_RID                  0x04
#define CDNS_CFGS1                0x08
#define CDNS_CFGS2                0x0C
#define OTGCMD                    0x10
#define OTGSTS                    0x14
#define OTGSTATE                  0x18
#define OTGIEN                    0x1C
#define OTGIVECT                  0x20
#define OTGTMR                    0x24

/* OTGCMD - bitmasks */
/* "Request the bus for Device mode. */
#define OTGCMD_DEV_BUS_REQ        BIT(0)
/* Request the bus for Host mode */
#define OTGCMD_HOST_BUS_REQ       BIT(1)
/* Enable OTG mode. */
#define OTGCMD_OTG_EN             BIT(2)
/* Disable OTG mode */
#define OTGCMD_OTG_DIS            BIT(3)
/*"Configure OTG as A-Device. */
#define OTGCMD_A_DEV_EN           BIT(4)
/*"Configure OTG as A-Device. */
#define OTGCMD_A_DEV_DIS          BIT(5)
/* Drop the bus for Device mod	e. */
#define OTGCMD_DEV_BUS_DROP       BIT(8)
/* Drop the bus for Host mode*/
#define OTGCMD_HOST_BUS_DROP      BIT(9)
/* Power Down USBSS-DEV - only for CDNS3.*/
#define OTGCMD_DEV_POWER_OFF      BIT(11)
/* Power Down CDNSXHCI - only for CDNS3. */
#define OTGCMD_HOST_POWER_OFF     BIT(12)

/* OTGIEN - bitmasks */
/* ID change interrupt enable */
#define OTGIEN_ID_CHANGE_INT      BIT(0)
/* Vbusvalid fall detected interrupt enable.*/
#define OTGIEN_VBUSVALID_RISE_INT BIT(4)
/* Vbusvalid fall detected interrupt enable */
#define OTGIEN_VBUSVALID_FALL_INT BIT(5)

/* OTGSTS - bitmasks */
/*
 * Current value of the ID pin. It is only valid when idpullup in
 *  OTGCTRL1_TYPE register is set to '1'.
 */
#define OTGSTS_ID_VALUE           BIT(0)
/* Current value of the vbus_valid */
#define OTGSTS_VBUS_VALID         BIT(1)
/* Current value of the b_sess_vld */
#define OTGSTS_SESSION_VALID      BIT(2)
/*Device mode is active*/
#define OTGSTS_DEV_ACTIVE         BIT(3)
/* Host mode is active. */
#define OTGSTS_HOST_ACTIVE        BIT(4)
/* OTG Controller not ready. */
#define OTGSTS_OTG_NRDY_MASK      BIT(11)
#define OTGSTS_OTG_NRDY(p)        ((p) & OTGSTS_OTG_NRDY_MASK)

void cdns_drd_irq(void);

#endif /* CDN_DRD */