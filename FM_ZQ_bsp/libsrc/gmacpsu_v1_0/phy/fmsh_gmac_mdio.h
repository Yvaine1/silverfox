/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac.h
 *
 * gmac driver
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 1_0   Danyang Wang  6/25/2023  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_gmac.h"
#include "marvell_88e1512.h"
#include "microchip_ksz9031RNX.h"
#include "motorcomm_yt8521.h"
#include "jl2xx1.h"

/************************** Constant Definitions *****************************/

#define FMSH_ENET_PHY_DEBUG                 1

/* phy vendors */
#define PHY_ID_MICROCHIP                    0x22
#define PHY_ID_MARVELL                      0x141
#define PHY_ID_TI                           0x2000

/* phy device */
#define PHY_GENERIC                         0
#define PHY_88E1111                         1
#define PHY_88E1116R                        2
#define PHY_KSZ9031RNX                      3
#define PHY_88E1512                         4
#define PHY_YT8521                          5
#define PHY_JL2XX1                          6

/* phy timeout value ms*/
#define FGMACPS_PHY_TIMEOUT                 1000
#define FGMACPS_PHY_LINK_TIMEOUT            1000

/* return code */
#define ETHERNET_PHY_OK                     GMAC_RETURN_CODE_OK
#define ETHERNET_PHY_PARAM_ERR              GMAC_RETURN_CODE_PARAM_ERR
#define ETHERNET_PHY_ERR                    GMAC_RETURN_CODE_ERR
#define ETHERNET_PHY_TIMEOUT                GMAC_RETURN_CODE_TIME_OUT

/* IEEE phy define */
#define IEEE_CONTROL_REG_OFFSET             0
#define IEEE_STATUS_REG_OFFSET              1
#define IEEE_PHY_DETECT_REG1                2
#define IEEE_PHY_DETECT_REG2                3
#define IEEE_AUTONEGO_ADVERTISE_REG         4
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET 5
#define IEEE_PARTNER_ABILITIES_2_REG_OFFSET 8
#define IEEE_PARTNER_ABILITIES_3_REG_OFFSET 10
#define IEEE_1000_ADVERTISE_REG_OFFSET      9
#define IEEE_MMD_ACCESS_CONTROL_REG         13
#define IEEE_MMD_ACCESS_ADDRESS_DATA_REG    14
#define IEEE_COPPER_SPECIFIC_CONTROL_REG    16
#define IEEE_SPECIFIC_STATUS_REG            17
#define IEEE_COPPER_SPECIFIC_STATUS_REG_2   19
#define IEEE_EXT_PHY_SPECIFIC_CONTROL_REG   20
#define IEEE_CONTROL_REG_MAC                21
#define IEEE_PAGE_ADDRESS_REGISTER          22

#define IEEE_CTRL_1GBPS_LINKSPEED_MASK      0x2040
#define IEEE_CTRL_LINKSPEED_MASK            0x0040
#define IEEE_CTRL_LINKSPEED_1000M           0x0040
#define IEEE_CTRL_LINKSPEED_100M            0x2000
#define IEEE_CTRL_LINKSPEED_10M             0x0000
#define IEEE_CTRL_FULL_DUPLEX               0x100
#define IEEE_CTRL_RESET_MASK                0x8000
#define IEEE_CTRL_AUTONEGOTIATE_ENABLE      0x1000
#define IEEE_STAT_AUTONEGOTIATE_CAPABLE     0x0008
#define IEEE_STAT_AUTONEGOTIATE_COMPLETE    0x0020
#define IEEE_STAT_AUTONEGOTIATE_RESTART     0x0200
#define IEEE_STAT_LINK_STATUS               0x0004
#define IEEE_STAT_1GBPS_EXTENSIONS          0x0100
#define IEEE_AN1_ABILITY_MASK               0x1FE0
#define IEEE_AN3_ABILITY_MASK_1GBPS         0x0C00
#define IEEE_AN1_ABILITY_MASK_100MBPS       0x0380
#define IEEE_AN1_ABILITY_MASK_10MBPS        0x0060
#define IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK  0x0030

#define IEEE_SPEED_MASK                     0xC000
#define IEEE_SPEED_1000                     0x8000
#define IEEE_SPEED_100                      0x4000

#define IEEE_ASYMMETRIC_PAUSE_MASK          0x0800
#define IEEE_PAUSE_MASK                     0x0400
#define IEEE_AUTONEG_ERROR_MASK             0x8000

#define IEEE_MMD_ACCESS_CTRL_DEVAD_MASK     0x1F
#define IEEE_MMD_ACCESS_CTRL_PIDEVAD_MASK   0x801F
#define IEEE_MMD_ACCESS_CTRL_NOPIDEVAD_MASK 0x401F

/***************** Macros (Inline Functions) Definitions *********************/
#define SET_BIT(a, b)                       (a) |= (b)
#define RESET_BIT(a, b)                     (a) &= ~(b)

#define PHY_TRACE_OUT(flag, ...) \
    do                           \
    {                            \
        if (flag)                \
        {                        \
            fmsh_print(__VA_ARGS__); \
        }                        \
    } while (0)

/************************** Function Prototypes *****************************/

void FGmacPs_SetMdioDivisor(FGmacPs *InstancePtr, FGmacPs_MdcDiv Divisor);
LONG FGmacPs_PhyRead(FGmacPs *InstancePtr, u16 PhyAddress, u16 RegisterNum,
                     u16 *PhyDataPtr);
LONG FGmacPs_PhyWrite(FGmacPs *InstancePtr, u16 PhyAddress, u16 RegisterNum,
                      u16 PhyData);
u16 FGmacPs_PHYDetect(FGmacPs *InstancePtr);
LONG FGmacPs_PHYInit(FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr);
LONG FGmacPs_PHYloopback(FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr);
