/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_slcr.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lsq  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_SLCR_H_ /* prevent circular inclusions */
#define _FMSH_SLCR_H_ /* by using protection macros */

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

#define SLCR_LOCK         (0x004)
#define SLCR_UNLOCK       (0x008)
#define PSS_RST_CTRL      (0x200)

#define SLCR_DDR_CTRL     (0x21C)
#define SLCR_GEM0_CTRL    (0x270)
#define SLCR_GEM1_CTRL    (0x270)
#define SLCR_SMC_CTRL     (0x278)
#define SLCR_NFC_CTRL     (0x280)
#define SLCR_QSPI_CTRL    (0x288)
#define SLCR_SDIO0_CTRL   (0x290)
#define SLCR_SDIO1_CTRL   (0x290)
#define SLCR_UART0_CTRL   (0x298)
#define SLCR_UART1_CTRL   (0x298)
#define SLCR_SPI0_CTRL    (0x300)
#define SLCR_SPI1_CTRL    (0x300)
#define SLCR_CAN0_CTRL    (0x308)
#define SLCR_CAN1_CTRL    (0x308)
#define SLCR_GPIO_CTRL    (0x310)
#define SLCR_I2C0_CTRL    (0x318)
#define SLCR_I2C1_CTRL    (0x318)
#define SLCR_USB0_CTRL    (0x320)
#define SLCR_USB1_CTRL    (0x320)
#define SLCR_WDT_CTRL     (0x330)
#define SLCR_TTC_CTRL     (0x338)
#define SLCR_PCAP_CTRL    (0x340)
#define SLCR_HP_RST_CTRL  (0x344)
#define SLCR_FIC_RST_CTRL (0x348)
#define SLCR_MIO_LOOPBACK (0x804)
#define SLCR_APU_BRDC     (0xA00)

#define DDR_RST           (0x0)
#define DDR_APB_RST       (0x1)
#define DDR_AXI_RST       (0x2)
#define GEM0_AXI_RST      (0x0)
#define GEM0_AHB_RST      (0x1)
#define GEM0_RX_RST       (0x2)
#define GEM0_TX_RST       (0x3)
#define GEM1_AXI_RST      (0x5)
#define GEM1_AHB_RST      (0x6)
#define GEM1_RX_RST       (0x7)
#define GEM1_TX_RST       (0x8)
#define SMC_AHB_RST       (0x0)
#define NFC_AHB_RST       (0x0)
#define NFC_REF_RST       (0x1)
#define QSPI_APB_RST      (0x0)
#define QSPI_AHB_RST      (0x1)
#define QSPI_REF_RST      (0x2)
#define SDIO0_AHB_RST     (0x0)
#define SDIO1_AHB_RST     (0x1)
#define UART0_APB_RST     (0x0)
#define UART1_APB_RST     (0x1)
#define UART0_REF_RST     (0x2)
#define UART1_REF_RST     (0x3)
#define SPI0_APB_RST      (0x0)
#define SPI1_APB_RST      (0x1)
#define SPI0_REF_RST      (0x2)
#define SPI1_REF_RST      (0x3)
#define CAN0_APB_RST      (0x0)
#define CAN1_APB_RST      (0x1)
#define GPIO_APB_RST      (0x0)
#define I2C0_APB_RST      (0x0)
#define I2C1_APB_RST      (0x1)
#define USB0_AHB_RST      (0x0)
#define USB1_AHB_RST      (0x1)
#define USB0_PHY_RST      (0x2)
#define USB1_PHY_RST      (0x3)
#define WDT_APB_RST       (0x0)
#define WDT_RST           (0x1)
#define TTC1_CLK3RST      (0x7)
#define TTC1_CLK2RST      (0x6)
#define TTC1_CLK1RST      (0x5)
#define TTC0_CLK3RST      (0x4)
#define TTC0_CLK2RST      (0x3)
#define TTC0_CLK1RST      (0x2)
#define TTC1_APB_RST      (0x1)
#define TTC0_APB_RST      (0x0)
#define PCAP_AHB_RST      (0x0)
#define AXI_HP0_RST       (0x0)
#define AXI_HP1_RST       (0x1)
#define AXI_HP2_RST       (0x2)
#define AXI_HP3_RST       (0x3)
#define FIC0_SFP_RST      (0x0)
#define FIC0_MFP_RST      (0x1)
#define FIC1_SFP_RST      (0x2)
#define FIC1_MFP_RST      (0x3)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/*Public drivers*/
void FSlcrPs_lock(void);
void FSlcrPs_unlock(void);
void FSlcrPs_softRst(u32 soft_rst_en);
void FSlcrPs_ipSetRst(u32 rst_id, u32 rst_mode);
void FSlcrPs_ipReleaseRst(u32 rst_id, u32 rst_mode);
void FSlcrPS_setI2cLoop(u32 loop_en);
void FSlcrPS_setCanLoop(u32 loop_en);
void FSlcrPS_setUartLoop(u32 loop_en);
void FSlcrPS_setSpiLoop(u32 loop_en);

/*Private drivers*/
void FSlcrPs_setBitTo0(u32 baseAddr, u32 offSet, u32 bit_num);
void FSlcrPs_setBitTo1(u32 baseAddr, u32 offSet, u32 bit_num);
u32 FSlcrPS_regRead(u32 addr);
void FSlcrPS_rsvRegPrint(void);

#endif
