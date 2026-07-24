#ifndef _MICROCHIP_KSZ9031RNX_H_
#define _MICROCHIP_KSZ9031RNX_H_
/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  microchip_ksz9031RNX.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   Danyang Wang  6/25/2023  First Release
 *</pre>
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_gmac_mdio.h"
/************************** Constant Definitions *****************************/
/* ID */
#define KSZ9031_ID                  0x22
/* status & function */
#define PHY_TIME_OUT                FMSH_KSZ9031_TIMEOUT
/* timing ctrl */

/* reg map - IEEE */
#define KSZ9031RNX_CTRL             0 /* Control Register */
#define KSZ9031RNX_STAT             1 /* Status Register */
#define KSZ9031RNX_ID1              2 /* PHY ID */
#define KSZ9031RNX_ID2              3 /* PHY ID */
#define KSZ9031RNX_ANA              4 /* Auto-Neg Advertisement Register */
#define KSZ9031RNX_LPA              5 /* Auto-Neg Link Partner Ability Register */
#define KSZ9031RNX_ANE              6 /* Auto-Neg Expansion Register */
#define KSZ9031RNX_NPTX             7 /* Auto-Neg Link Next Page */
#define KSZ9031RNX_LPNPA            8 /* Auto-Neg Link Partner Next Page Ability */
#define KSZ9031RNX_1000CT           9    /* 1000BASE-T Control Register */
#define KSZ9031RNX_1000ST           0xA  /* 1000BASE-T Status Register */
#define KSZ9031RNX_MMDC             0xD  /* MMD ctrl */
#define KSZ9031RNX_MMDD             0xE  /* mmd reg/data */
#define KSZ9031RNX_ES               0xF  /* extended status */
#define KSZ9031RNX_RMLP             0x11 /* remote loopback */
#define KSZ9031RNX_LMDCD            0x12 /* LinkMD Cable Diagnostic */
#define KSZ9031RNX_DGPS             0x13 /* Digital PMA/PCS Status */
#define KSZ9031RNX_RXERC            0x15 /* RXER Counter */
#define KSZ9031RNX_ITCS             0x1B /* Interrupt Ctrl/Status */
#define KSZ9031RNX_AMDIX            0x1C /* Auto MDI/MDI-X */
#define KSZ9031RNX_PHYC             0x1F /* PHY ctrl */
/* reg map - MMD */
/* MMD Address 0x0 */
#define KSZ9031RNX_FLP_BURST_TX_LO  3 /* AN FLP Burst Transmit-LO */
#define KSZ9031RNX_FLP_BURST_TX_HI  4 /* AN FLP Burst Tramsmit-HI */

/* MMD Address 0x2 */
#define KSZ9031RNX_CONTROL_PAD_SKEW 4
#define KSZ9031RNX_RX_DATA_PAD_SKEW 5
#define KSZ9031RNX_TX_DATA_PAD_SKEW 6
#define KSZ9031RNX_CLK_PAD_SKEW     8

/* MMD Address 0x1C */
#define KSZ9031RNX_EDPD             0x23

/* reg define */
/******   Control Register - 0x0   ****/
#define KSZ9031RNX_CTRL_RST         (0x1u << 15) /* PHY software reset:1 for reset */
#define KSZ9031RNX_CTRL_LOOPBK      (0x1u << 14) /* loopback TXD to RXD */
#define KSZ9031RNX_CTRL_SPEED_L                                      \
    (0x1u << 13)                                 /* speed select LSB \
                                                    [bit6,bit13]*/
                                                 /* 10=1000; 01=100; 00=10 */
#define KSZ9031RNX_CTRL_AN_EN      (0x1u << 12)  /* auto-negotiation enable */
#define KSZ9031RNX_CTRL_POWER_DOWN (0x1u << 11)  /* power down */
#define KSZ9031RNX_CTRL_RST_CP_AN \
    (0x1u << 9) /* Restart Copper Auto-Negotiation */
#define KSZ9031RNX_CTRL_CP_DPLX_MOD \
    (0x1u << 8) /* Copper Duplex Mode. 1=full; 0=half */
#define KSZ9031RNX_CTRL_SPEED_H (0x1u << 6) /* speed select hsb */
/******   1000BASE-T ctrl - 0x9   ****/
#define KSZ9031RNX_1000CT_MSMCE \
    (0x1u << 12) /* master-slave manual cfg enable */
#define KSZ9031RNX_1000CT_MSMCV                  \
    (0x1u << 11) /* master-slave manual cfg data \
                  */
/******   remote loopback - 0x11   ****/
#define KSZ9031RNX_RMLP_RMLPEN   (0x1u << 8) /* remote loopback enable */
/******   Auto MDI/MDI-X - 0x1C   ****/
#define KSZ9031RNX_AMDIX_MDI_SET (0x1u << 7) /* MDI set, 1 : MDI, 0 : MDI-X */
#define KSZ9031RNX_AMDIX_SW_OFF \
    (0x1u << 6) /* Swap-off, 1: dis auto X, 0: enable auto X */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u16 mic9031RNX_read_MMD(FGmacPs *InstancePtr, u16 PhyAddress, u16 MMDAddress,
                        u16 RegAddress);
u8 mic9031RNX_write_MMD(FGmacPs *InstancePtr, u16 PhyAddress, u16 MMDAddress,
                        u16 RegAddress, u16 data);
u8 mic9031RNX_clk_pad_skew(FGmacPs *InstancePtr, u16 PhyAddress, u16 tx,
                           u16 rx);
u8 mic9031RNX_ctrl_pad_skew(FGmacPs *InstancePtr, u16 PhyAddress, u16 rx,
                            u16 tx);
u8 mic9031RNX_tx_data_pad_skew(FGmacPs *InstancePtr, u16 PhyAddress, u16 data3,
                               u16 data2, u16 data1, u16 data0);
u8 mic9031RNX_rx_data_pad_skew(FGmacPs *InstancePtr, u16 PhyAddress, u16 data3,
                               u16 data2, u16 data1, u16 data0);
u8 mic9031RNX_setup(FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr);
u8 mic9031RNX_digital_loopback(FGmacPs *InstancePtr,
                               FGmacPs_PhyConfig *PhyCfgPtr);
u8 mic9031RNX_analog_loopback(FGmacPs *InstancePtr,
                              FGmacPs_PhyConfig *PhyCfgPtr);
/************************** Variable Definitions *****************************/
#endif
