/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_bd.c
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
#include "microchip_ksz9031RNX.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function *****************************************/
u16 mic9031RNX_read_MMD (FGmacPs *InstancePtr, u16 PhyAddress, u16 MMDAddress,
                         u16 RegAddress)
{
    u16 ret;
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_MMDC, MMDAddress);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_MMDD, RegAddress);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_MMDC,
                     0x4000 + MMDAddress);
    FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_MMDD, &ret);
    return ret;
}

u8 mic9031RNX_write_MMD (FGmacPs *InstancePtr, u16 PhyAddress, u16 MMDAddress,
                         u16 RegAddress, u16 data)
{
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_MMDC, MMDAddress);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_MMDD, RegAddress);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_MMDC,
                     0x4000 + MMDAddress);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_MMDD, data);
    return ETHERNET_PHY_OK;
}

u8 mic9031RNX_clk_pad_skew (FGmacPs *InstancePtr, u16 PhyAddress, u16 tx,
                            u16 rx)
{
    mic9031RNX_write_MMD(InstancePtr, PhyAddress, 2, KSZ9031RNX_CLK_PAD_SKEW,
                         (tx << 5) | (rx));
    return ETHERNET_PHY_OK;
}

u8 mic9031RNX_ctrl_pad_skew (FGmacPs *InstancePtr, u16 PhyAddress, u16 rx,
                             u16 tx)
{
    mic9031RNX_write_MMD(InstancePtr, PhyAddress, 2,
                         KSZ9031RNX_CONTROL_PAD_SKEW, (rx << 4) | (tx));
    return ETHERNET_PHY_OK;
}

u8 mic9031RNX_tx_data_pad_skew (FGmacPs *InstancePtr, u16 PhyAddress, u16 data3,
                                u16 data2, u16 data1, u16 data0)
{
    mic9031RNX_write_MMD(InstancePtr, PhyAddress, 2,
                         KSZ9031RNX_TX_DATA_PAD_SKEW,
                         data0 | (data1 << 4) | (data2 << 8) | (data3 << 12));
    return ETHERNET_PHY_OK;
}

u8 mic9031RNX_rx_data_pad_skew (FGmacPs *InstancePtr, u16 PhyAddress, u16 data3,
                                u16 data2, u16 data1, u16 data0)
{
    mic9031RNX_write_MMD(InstancePtr, PhyAddress, 2,
                         KSZ9031RNX_RX_DATA_PAD_SKEW,
                         data0 | (data1 << 4) | (data2 << 8) | (data3 << 12));
    return ETHERNET_PHY_OK;
}

u8 mic9031RNX_MDIautoX (FGmacPs *InstancePtr, u16 PhyAddress, u8 enable,
                        u8 MDI_set)
{
    u16 reg;
    FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_AMDIX, &reg);
    if (enable == 1)
    {
        RESET_BIT(reg, KSZ9031RNX_AMDIX_SW_OFF);
    }
    else
    {
        SET_BIT(reg, KSZ9031RNX_AMDIX_SW_OFF);
        if (MDI_set == 1)
        {
            SET_BIT(reg, KSZ9031RNX_AMDIX_MDI_SET);
        }
        else
        {
            RESET_BIT(reg, KSZ9031RNX_AMDIX_MDI_SET);
        }
    }
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_AMDIX, reg);

    return ETHERNET_PHY_OK;
}

u8 mic9031RNX_AN_cfg (FGmacPs *InstancePtr, u16 PhyAddress, u8 enable)
{
    u16 reg;
    FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
    if (enable)
    {
        SET_BIT(reg, KSZ9031RNX_CTRL_AN_EN);
    }
    else
    {
        RESET_BIT(reg, KSZ9031RNX_CTRL_AN_EN);
    }
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);

    return ETHERNET_PHY_OK;
}

u8 mic9031RNX_setup (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    u16 reg;
    u32 Status;
    u32 phy_timeout = 0;
    u8 PhyAddress = PhyCfgPtr->phy_address;

    /* MDI crossover */
    Status = mic9031RNX_MDIautoX(InstancePtr, PhyAddress, 1, 0);

    /* AN and speed cfg */
    if (PhyCfgPtr->speed == speed_1000)
    {
        /* open AN */
        Status &= mic9031RNX_AN_cfg(InstancePtr, PhyAddress, 1);
        /* speed */
        FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
        SET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);
    }
    else if (PhyCfgPtr->auto_nag_en == 0)
    {
        /* AN */
        Status &= mic9031RNX_AN_cfg(InstancePtr, PhyAddress, 0);
        /* speed */
        FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        if (PhyCfgPtr->speed == speed_10)
        {
            RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
            RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        }
        else if (PhyCfgPtr->speed == speed_100)
        {
            SET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
            RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        }
        FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);
    }
    else
    {
        Status &= mic9031RNX_AN_cfg(InstancePtr, PhyAddress, 1);
    }

    /* timing ctrl */
    u16 s_tx_clk_skew = 0x1f;
    u16 s_rx_clk_skew = 0x1f;
    u16 s_tx_data_skew = 0x0;
    u16 s_rx_data_skew = 0x0;
    mic9031RNX_clk_pad_skew(InstancePtr, PhyAddress, s_tx_clk_skew,
                            s_rx_clk_skew);
    mic9031RNX_ctrl_pad_skew(InstancePtr, PhyAddress, s_rx_data_skew,
                             s_tx_data_skew);
    mic9031RNX_tx_data_pad_skew(InstancePtr, PhyAddress, s_tx_data_skew,
                                s_tx_data_skew, s_tx_data_skew, s_tx_data_skew);
    mic9031RNX_rx_data_pad_skew(InstancePtr, PhyAddress, s_rx_data_skew,
                                s_rx_data_skew, s_rx_data_skew, s_rx_data_skew);

    /* fix bug */
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xd, 0);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xe, 4);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xd, 0x4000);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xe, 6);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xd, 0);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xe, 3);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xd, 0x4000);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, 0xe, 0x1a80);
    FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
    reg |= (1 << 9);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);

    /* wait link up */
    do
    {
        FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_STAT, reg);
        phy_timeout++;
        if (phy_timeout > FGMACPS_PHY_LINK_TIMEOUT)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "PHY link timeout \r\n");
            return ETHERNET_PHY_TIMEOUT;
        }
        delay_ms(1);
    } while ((reg & IEEE_STAT_LINK_STATUS) == 0);
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "PHY link up \r\n");
    return Status;
}

/* local loopback */
u8 mic9031RNX_digital_loopback (FGmacPs *InstancePtr,
                                FGmacPs_PhyConfig *PhyCfgPtr)
{
    u16 reg;
    u8 PhyAddress = PhyCfgPtr->phy_address;

    /* setup loopback for different link speed */
    if (PhyCfgPtr->speed == speed_1000)
    {
        FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
        SET_BIT(reg, KSZ9031RNX_CTRL_LOOPBK);
        SET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
        RESET_BIT(reg, KSZ9031RNX_CTRL_AN_EN);
        SET_BIT(reg, KSZ9031RNX_CTRL_CP_DPLX_MOD);
        FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);

        FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_1000CT, &reg);
        SET_BIT(reg, KSZ9031RNX_1000CT_MSMCE);
        RESET_BIT(reg, KSZ9031RNX_1000CT_MSMCV);
        FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_1000CT, reg);
    }
    else if (PhyCfgPtr->speed == speed_100)
    {
        FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
        SET_BIT(reg, KSZ9031RNX_CTRL_LOOPBK);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        SET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
        RESET_BIT(reg, KSZ9031RNX_CTRL_AN_EN);
        SET_BIT(reg, KSZ9031RNX_CTRL_CP_DPLX_MOD);
        FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);
    }
    else
    {
        FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
        SET_BIT(reg, KSZ9031RNX_CTRL_LOOPBK);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
        RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
        RESET_BIT(reg, KSZ9031RNX_CTRL_AN_EN);
        SET_BIT(reg, KSZ9031RNX_CTRL_CP_DPLX_MOD);
        FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);
    }
    return ETHERNET_PHY_OK;
}

/* remote */
u8 mic9031RNX_analog_loopback (FGmacPs *InstancePtr,
                               FGmacPs_PhyConfig *PhyCfgPtr)
{
    u16 reg;
    u8 PhyAddress = PhyCfgPtr->phy_address;

    /* setup analog loopback */
    FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, &reg);
    SET_BIT(reg, KSZ9031RNX_CTRL_SPEED_H);
    RESET_BIT(reg, KSZ9031RNX_CTRL_SPEED_L);
    RESET_BIT(reg, KSZ9031RNX_CTRL_AN_EN);
    SET_BIT(reg, KSZ9031RNX_CTRL_CP_DPLX_MOD);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_CTRL, reg);

    FGmacPs_PhyRead(InstancePtr, PhyAddress, KSZ9031RNX_RMLP, &reg);
    SET_BIT(reg, KSZ9031RNX_RMLP_RMLPEN);
    FGmacPs_PhyWrite(InstancePtr, PhyAddress, KSZ9031RNX_RMLP, reg);
    return ETHERNET_PHY_OK;
}
