/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  marvell_88e1512.c
 *
 * marvell 8821512 driver
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

#include "fmsh_gmac_mdio.h"
#include "marvell_88e1512.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function *****************************************/

/**
 * @brief 88E1512 PHY Driver
 **/

// 88e1512 phy reg write with page
u8 mv88e1512_PhyWrite (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                       u16 RegisterNum, u16 PhyData)
{
    LONG Status;
    if (Page == 0)
    {
        Status = FGmacPs_PhyWrite(InstancePtr, PhyAddress, RegisterNum,
                                  PhyData);
    }
    else
    {
        Status = FGmacPs_PhyWrite(InstancePtr, PhyAddress, MV88E1512_PAGSR,
                                  Page);
        delay_ms(10);
        Status |= FGmacPs_PhyWrite(InstancePtr, PhyAddress, RegisterNum,
                                   PhyData);
        delay_ms(10);
        Status |= FGmacPs_PhyWrite(InstancePtr, PhyAddress, MV88E1512_PAGSR, 0);
    }

    return Status;
}

// 88e1512 phy reg read with page
u8 mv88e1512_PhyRead (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                      u16 RegisterNum, u16 *PhyDataPtr)
{
    LONG Status;
    if (Page == 0)
    {
        Status = FGmacPs_PhyRead(InstancePtr, PhyAddress, RegisterNum,
                                 PhyDataPtr);
    }
    else
    {
        Status = FGmacPs_PhyWrite(InstancePtr, PhyAddress, MV88E1512_PAGSR,
                                  Page);
        delay_ms(10);
        Status |= FGmacPs_PhyRead(InstancePtr, PhyAddress, RegisterNum,
                                  PhyDataPtr);
        delay_ms(10);
        Status |= FGmacPs_PhyWrite(InstancePtr, PhyAddress, MV88E1512_PAGSR, 0);
    }

    return Status;
}

// 88e1512 phy setup
u8 mv88e1512_setup_full (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u16 PhyReg;
    u32 Phy_timeout;

    // set operation mode
    switch (InstancePtr->Config.InterFaceType)
    {
    case gmac_path_rgmii:
    {
        Status = mv88e1512_setup_rgmii(InstancePtr, PhyCfgPtr);
        // Status |= mv88e1512_setup_rgmii_speed(InstancePtr, PhyCfgPtr);
        if (Status != FMSH_SUCCESS)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: mv88e1512 RGMII setup failure \r\n");
            return Status;
        }
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "mv88e1512 RGMII setup OK \r\n");
        break;
    }
    case gmac_path_sgmii:
    {
        Status = mv88e1512_setup_sgmii(InstancePtr, PhyCfgPtr);
        if (Status != FMSH_SUCCESS)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: mv88e1512 SGMII setup failure \r\n");
            return Status;
        }
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "mv88e1512 SGMII setup OK \r\n");
        break;
    }
    default:
    {
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                      "Error: mv88e1512 InterFaceType not recognized \r\n");
        Status = FMSH_FAILURE;
        break;
    }
    }

    // soft reset
    Status |= mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                MV88E1512_PAGE0, MV88E1512_COPPER_CTRL,
                                &PhyReg);
    Status |= mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                 MV88E1512_PAGE0, MV88E1512_COPPER_CTRL,
                                 (PhyReg | MV88E1512_COPPER_CTRL_RESET));
    // wait for rest done
    Phy_timeout = 0;
    do
    {
        Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                   MV88E1512_PAGE0, MV88E1512_COPPER_CTRL,
                                   &PhyReg);
        Phy_timeout++;
        if (Phy_timeout > FGMACPS_PHY_TIMEOUT)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: mv88e1512 reset timeout \r\n");
            return ETHERNET_PHY_TIMEOUT;
        }
        delay_ms(1);
    } while ((PhyReg & MV88E1512_COPPER_CTRL_RESET) != 0);
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, " ok \r\n");

    return Status;
}

// 88e1512 phy rgmii setup
u8 mv88e1512_setup_rgmii (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u16 PhyReg;
    u32 Phy_timeout;

    // set operation mode
    Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                               &PhyReg);
    Status |= mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                 MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                 ((PhyReg & 0xFFF8) | MV88E1512_MODE_RGMII));

    // mode reset
    Status |= mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                &PhyReg);
    Status |= mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                 MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                 (PhyReg | MV88E1512_COPPER_CTRL_RESET));

    // wait for rest done
    Phy_timeout = 0;
    do
    {
        Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                   MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                   &PhyReg);
        Phy_timeout++;
        if (Phy_timeout > FGMACPS_PHY_TIMEOUT)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: mv88e1512 mode reset timeout \r\n");
            return ETHERNET_PHY_TIMEOUT;
        }
        delay_ms(1);
    } while ((PhyReg & MV88E1512_COPPER_CTRL_RESET) != 0);
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "mv88e1512 mode reset OK \r\n");

    // Adding Tx and Rx delay. Configuring loopback speed.
    PhyReg = (MV88E1512_MAC_CTRL2_DEFAULT_MAC_IF_SPEED_MSB |
              MV88E1512_MAC_CTRL2_RGMII_RX_TIMING_CTRL |
              MV88E1512_MAC_CTRL2_RGMII_TX_TIMING_CTRL);
    Status = mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                MV88E1512_PAGE2, MV88E1512_MAC_CTRL2, PhyReg);

    /*
     * Make sure new configuration is in effect
     */
    Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_PAGE2, MV88E1512_MAC_CTRL2, &PhyReg);
    if (Status != FMSH_SUCCESS)
    {
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                      "Error setting Reg MV88E1512_MAC_CTRL2 in Page 2");
        return FMSH_FAILURE;
    }

    return Status;
}

// 88e1512 phy sgmii setup
u8 mv88e1512_setup_sgmii (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u16 PhyReg;
    u32 Phy_timeout;

    // set operation mode
    Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                               &PhyReg);
    Status |= mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                 MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                 ((PhyReg & 0xFFF8) | MV88E1512_MODE_SGMII));

    // mode reset
    Status |= mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                &PhyReg);
    Status |= mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                 MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                 (PhyReg | MV88E1512_COPPER_CTRL_RESET));

    // wait for rest done
    Phy_timeout = 0;
    do
    {
        Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                   MV88E1512_PAGE18, MV88E1512_GENERAL_CTRL_1,
                                   &PhyReg);
        Phy_timeout++;
        if (Phy_timeout > FGMACPS_PHY_TIMEOUT)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: mv88e1512 mode reset timeout \r\n");
            return ETHERNET_PHY_TIMEOUT;
        }
        delay_ms(1);
    } while ((PhyReg & MV88E1512_COPPER_CTRL_RESET) != 0);

    return Status;
}

// marvelll 88e1512 rgmii speed setup
u8 mv88e1512_setup_rgmii_speed (FGmacPs *InstancePtr,
                                FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;

    Status = 0;

    return Status;
}

// marvelll 88e1512 phy autodetect
u8 mv88e1512_PHYDetect (FGmacPs *InstancePtr)
{
    u16 PhyAddr;
    u32 Status;
    u16 PhyReg1;
    u16 PhyReg2;

    for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++)
    {
        Status = mv88e1512_PhyRead(InstancePtr, PhyAddr, MV88E1512_PAGE0,
                                   MV88E1512_COPPER_PHYID1, &PhyReg1);
        Status |= mv88e1512_PhyRead(InstancePtr, PhyAddr, MV88E1512_PAGE0,
                                    MV88E1512_COPPER_PHYID2, &PhyReg2);
        if ((Status == FMSH_SUCCESS) &&
            ((PhyReg1 & MV88E1512_COPPER_PHYID1_OUI_MSB) ==
             MV88E1512_COPPER_PHYID1_OUI_MSB_DEFAULT) &&
            ((PhyReg2 & MV88E1512_COPPER_PHYID2_OUI_LSB) ==
             MV88E1512_COPPER_PHYID2_OUI_LSB_DEFAULT) &&
            ((PhyReg2 & MV88E1512_COPPER_PHYID2_MODEL_NUM) ==
             MV88E1512_COPPER_PHYID2_MODEL_NUM_DEFAULT))
        {
            /* Found a valid PHY address */
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "mv88e1512 PHY detected, Rev %x, Addr%d.\r\n",
                          (PhyReg2 & MV88E1512_COPPER_PHYID2_REVISION_NUM),
                          PhyAddr);
            return PhyAddr;
        }
    }

    /* PhyAddr default to 0 */
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                  "Error: mv88e1512 PHY detect fail, set phyaddr to 0.\r\n");
    return 0;
}

// marvelll 88e1512 system interface loopback
u8 mv88e1512_system_interface_loopback (FGmacPs *InstancePtr,
                                        FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u16 PhyReg;

    // set system interface loopback
    switch (InstancePtr->Config.InterFaceType)
    {
    case gmac_path_rgmii:
    {
        Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                   MV88E1512_PAGE0, MV88E1512_COPPER_CTRL,
                                   &PhyReg);
        Status |= mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                     MV88E1512_PAGE0, MV88E1512_COPPER_CTRL,
                                     (PhyReg | MV88E1512_COPPER_CTRL_LOOPBACK));
        if (Status != FMSH_SUCCESS)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: mv88e1512 RGMII loopback setup failure \r\n");
            return Status;
        }
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                      "mv88e1512 RGMII loopback setup OK \r\n");
        break;
    }
    case gmac_path_sgmii:
    {
        Status = mv88e1512_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                   MV88E1512_PAGE0, MV88E1512_COPPER_CTRL,
                                   &PhyReg);
        Status |= mv88e1512_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                                     MV88E1512_PAGE0, MV88E1512_COPPER_CTRL,
                                     (PhyReg | MV88E1512_COPPER_CTRL_LOOPBACK));
        if (Status != FMSH_SUCCESS)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: mv88e1512 SGMII loopback setup failure \r\n");
            return Status;
        }
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                      "mv88e1512 SGMII loopback setup OK \r\n");
        break;
    }
    default:
    {
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                      "Error: mv88e1512 InterFaceType not recognized \r\n");
        Status = FMSH_FAILURE;
        break;
    }
    }

    return Status;
}

// 88e1512 phy simple setup
u8 mv88e1512_setup_simple (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u16 PhyReg;

    // set operation mode
    Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_PAGSR, 0x12);

    Status |= FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_GENERAL_CTRL_1, 0x0);

    Status |= FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_PAGSR, 0x0);
    // soft reset
    Status |= FGmacPs_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_COPPER_CTRL, &PhyReg);
    Status |= FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_COPPER_CTRL,
                               (PhyReg | MV88E1512_COPPER_CTRL_RESET));

    return Status;
}

// 88e1512 phy sgmii to copper simple setup
u8 mv88e1512_sgmii_setup_simple (FGmacPs *InstancePtr,
                                 FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u16 PhyReg;

    // set operation mode
    Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_PAGSR, 0x12);
    Status |= FGmacPs_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_GENERAL_CTRL_1, &PhyReg);
    Status |= FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_GENERAL_CTRL_1,
                               ((PhyReg & 0xFFF8) | 0x1));

    // soft reset
    Status |= FGmacPs_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_GENERAL_CTRL_1, &PhyReg);
    Status |= FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_GENERAL_CTRL_1,
                               (PhyReg | MV88E1512_COPPER_CTRL_RESET));

    return Status;
}

u8 mv88e1512_setup (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u16 PhyReg0 = 0;
    u16 PhyReg21 = 0;
    u16 PhyReg22 = 0;

    /*
     * Setup speed and duplex
     */
    switch (PhyCfgPtr->speed)
    {
    case speed_10:
        PhyReg0 |= PHY_REG0_10;
        PhyReg21 |= PHY_REG21_10;
        break;
    case speed_100:
        PhyReg0 |= PHY_REG0_100;
        PhyReg21 |= PHY_REG21_100;
        break;
    case speed_1000:
        PhyReg0 |= PHY_REG0_1000;
        PhyReg21 |= PHY_REG21_1000;
        break;
    default:
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "Error: speed not recognized ");
        return FMSH_FAILURE;
    }

    // set operation mode
    Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_PAGSR, 0x12);

    Status |= FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_GENERAL_CTRL_1, 0x0);

    Status |= FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                               MV88E1512_PAGSR, 0x0);

    // Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
    // MV88E1512_COPPER_CTRL, PhyReg0);
    /*
     * Make sure new configuration is in effect
     */
    Status = FGmacPs_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                             MV88E1512_COPPER_CTRL, &PhyReg0);
    if (Status != FMSH_SUCCESS)
    {
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "Error setup phy speed");
        return FMSH_FAILURE;
    }

    /*
     * Switching to PAGE2
     */
    PhyReg22 = 0x2;
    Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_PAGSR, PhyReg22);

    /*
     * Adding Tx and Rx delay. Configuring loopback speed.
     */
    Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_MAC_CTRL2, PhyReg21);
    /*
     * Make sure new configuration is in effect
     */
    Status = FGmacPs_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                             MV88E1512_MAC_CTRL2, &PhyReg21);
    if (Status != FMSH_SUCCESS)
    {
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                      "Error setting Reg MV88E1512_MAC_CTRL2 in Page 2");
        return FMSH_FAILURE;
    }
    /*
     * Switching to PAGE0
     */
    PhyReg22 = 0x0;
    Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_PAGSR, PhyReg22);

    /*
     * Issue a reset to phy
     */
    Status = FGmacPs_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                             MV88E1512_COPPER_CTRL, &PhyReg0);
    PhyReg0 |= PHY_REG0_RESET;
    Status = FGmacPs_PhyWrite(InstancePtr, PhyCfgPtr->phy_address,
                              MV88E1512_COPPER_CTRL, PhyReg0);

    Status = FGmacPs_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                             MV88E1512_COPPER_CTRL, &PhyReg0);
    if (Status != FMSH_SUCCESS)
    {
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "Error reset phy");
        return FMSH_FAILURE;
    }
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "88e1512 initial set OK.\r\n");
    return Status;
}
