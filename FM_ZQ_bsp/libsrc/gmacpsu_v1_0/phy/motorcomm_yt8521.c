/******************************************************************************
 *
 * Copyright (C) 2021 - 2031 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  motorcomm_yt8521.c
 *
 * motorcomm yt8521 phy driver, also be suited to JCX8211,they phys seems
 * as same as realtek rtl8211 phys.
 *
 * @note     None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   Meng Fanqiang  12/27/2021  First Release
 *</pre>
 ******************************************************************************/

#include "fmsh_gmac_mdio.h"
#include "motorcomm_yt8521.h"

// yt8521 phy reg write with page
u8 yt8521_PhyWrite (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
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
        Status = FGmacPs_PhyWrite(InstancePtr, PhyAddress, YT8521_PAGE_SELECT,
                                  Page);
        Status |= FGmacPs_PhyWrite(InstancePtr, PhyAddress, RegisterNum,
                                   PhyData);
        Status |= FGmacPs_PhyWrite(InstancePtr, PhyAddress, YT8521_PAGE_SELECT,
                                   0);
    }

    return Status;
}

// yt8521 phy reg read with page
u8 yt8521_PhyRead (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
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
        Status = FGmacPs_PhyWrite(InstancePtr, PhyAddress, YT8521_PAGE_SELECT,
                                  Page);
        Status |= FGmacPs_PhyRead(InstancePtr, PhyAddress, RegisterNum,
                                  PhyDataPtr);
        Status |= FGmacPs_PhyWrite(InstancePtr, PhyAddress, YT8521_PAGE_SELECT,
                                   0);
    }

    return Status;
}

// yt8521 phy reg read EXT
u8 yt8521_PhyRead_ext (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                       u16 RegisterNum, u16 *PhyDataPtr)
{
    LONG Status;

    Status = yt8521_PhyWrite(InstancePtr, PhyAddress, Page,
                             REG_DEBUG_ADDR_OFFSET, RegisterNum);
    Status |= yt8521_PhyRead(InstancePtr, PhyAddress, Page, REG_DEBUG_DATA,
                             PhyDataPtr);

    return Status;
}

// yt8521 phy reg write EXT
u8 yt8521_PhyWrite_ext (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                        u16 RegisterNum, u16 PhyData)
{
    LONG Status;

    Status = yt8521_PhyWrite(InstancePtr, PhyAddress, Page,
                             REG_DEBUG_ADDR_OFFSET, RegisterNum);
    Status |= yt8521_PhyWrite(InstancePtr, PhyAddress, Page, REG_DEBUG_DATA,
                              PhyData);

    return Status;
}

// yt8521 phy copper reg dump
u8 yt8521_reg_dump (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    u8 i = 0;
    u16 reg;
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "yt8521 reg:\r\n");
    for (i = 0; i < 32; i++)
    {
        yt8521_PhyRead(InstancePtr, PhyCfgPtr->phy_address, YT8521_PAGE0, i,
                       &reg);
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "reg%d : %x\r\n", i, reg);
    }
    return ETHERNET_PHY_OK;
}

// yt8521 phy autodetect
u8 yt8521_detect (FGmacPs *InstancePtr)
{
    u16 PhyAddr;
    LONG Status;
    u16 PhyReg1;
    u16 PhyReg2;

    for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++)
    {
        Status = yt8521_PhyRead(InstancePtr, PhyAddr, YT8521_PAGE0, YT8521_ID1,
                                &PhyReg1);
        Status |= yt8521_PhyRead(InstancePtr, PhyAddr, YT8521_PAGE0, YT8521_ID2,
                                 &PhyReg2);
        if ((Status == FMSH_SUCCESS) && (PhyReg1 == YT8521_ID1_VAL) &&
            (PhyReg2 == YT8521_ID2_VAL))
        {
            /* Found a valid PHY address */
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "YT8521 or alike PHY detected, Addr%d.\r\n", PhyAddr);
            return PhyAddr;
        }
    }

    /* PhyAddr default to 0 */
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                  "YT8521 PHY detect fail, set phyaddr to 0.\r\n");
    return 0;
}

u8 yt8521_setup (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    u16 PhyReg;
    u32 Phy_timeout;
    LONG Status;

    // soft reset
    Status = yt8521_PhyRead(InstancePtr, PhyCfgPtr->phy_address, YT8521_PAGE0,
                            YT8521_CTRL, &PhyReg);
    Status |= yt8521_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, YT8521_PAGE0,
                              YT8521_CTRL, (PhyReg | YT8521_COPPER_CTRL_RESET));
    // wait for rest done
    Phy_timeout = 0;
    do
    {
        Status = yt8521_PhyRead(InstancePtr, PhyCfgPtr->phy_address,
                                YT8521_PAGE0, YT8521_CTRL, &PhyReg);
        Phy_timeout++;
        if (Phy_timeout > FGMACPS_PHY_TIMEOUT)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: YT8521 reset timeout \r\n");
            return ETHERNET_PHY_TIMEOUT;
        }
        delay_ms(1);
    } while ((PhyReg & YT8521_COPPER_CTRL_RESET) != 0);
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "YT8521 reset ok \r\n");

    switch (InstancePtr->Config.InterFaceType)
    {
    case gmac_path_rgmii:
    {
        /* utp=>rgmii */
        /* NOTE: this function should not be called more than one for each chip.
         */
        Status = yt8521_PhyRead_ext(InstancePtr, PhyCfgPtr->phy_address,
                                    YT8521_PAGE0, 0xa001, &PhyReg);
        PhyReg &= (~7);
        Status |= yt8521_PhyWrite_ext(InstancePtr, PhyCfgPtr->phy_address,
                                      YT8521_PAGE0, 0xa001, PhyReg);

        /* set tx delay, rx delay no need to be tuned */
        Status |= yt8521_PhyWrite_ext(InstancePtr, PhyCfgPtr->phy_address,
                                      YT8521_PAGE0, 0xa003, 0x4cf0);

        /* acess phy register */
        Status |= yt8521_PhyWrite_ext(InstancePtr, PhyCfgPtr->phy_address,
                                      YT8521_PAGE0, 0xa000, 0);

        /* disable auto sleep */
        Status |= yt8521_PhyRead_ext(InstancePtr, PhyCfgPtr->phy_address,
                                     YT8521_PAGE0, YT8521_EXTREG_SLEEP_CONTROL1,
                                     &PhyReg);
        PhyReg &= ~YT8521_EN_SLEEP_SW_BIT;
        Status |= yt8521_PhyWrite_ext(InstancePtr, PhyCfgPtr->phy_address,
                                      YT8521_PAGE0,
                                      YT8521_EXTREG_SLEEP_CONTROL1, PhyReg);

        /* enable RXC clock when no wire plug */

        Status |= yt8521_PhyRead_ext(InstancePtr, PhyCfgPtr->phy_address,
                                     YT8521_PAGE0, 0xc, &PhyReg);
        PhyReg &= ~(1 << 12);
        Status |= yt8521_PhyWrite_ext(InstancePtr, PhyCfgPtr->phy_address,
                                      YT8521_PAGE0, 0xc, PhyReg);
        if (Status != FMSH_SUCCESS)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: YT8521 RGMII setup failure \r\n");
            return Status;
        }
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "YT8521 RGMII setup OK \r\n");
        break;
    }
    case gmac_path_sgmii:
    {
        /* utp=>sgmii */
        /* NOTE: this function should not be called more than one for each chip.
         */
        Status = yt8521_PhyRead_ext(InstancePtr, PhyCfgPtr->phy_address,
                                    YT8521_PAGE0, 0xa001, &PhyReg);
        PhyReg &= (~7);
        PhyReg |= 3;
        Status |= yt8521_PhyWrite_ext(InstancePtr, PhyCfgPtr->phy_address,
                                      YT8521_PAGE0, 0xa001, PhyReg);
        if (Status != FMSH_SUCCESS)
        {
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "Error: YT8521 SGMII loopback setup failure \r\n");
            return Status;
        }
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "YT8521 SGMII setup OK \r\n");
        break;
    }
    default:
    {
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                      "Error: YT8521 InterFaceType not recognized \r\n");
        Status = FMSH_FAILURE;
        break;
    }
    }

    return Status;
}

u8 yt8521_system_interface_loopback (FGmacPs *InstancePtr,
                                     FGmacPs_PhyConfig *PhyCfgPtr)
{
    u16 PhyReg;
    LONG Status;

    /* close autoneg */
    Status = yt8521_PhyRead(InstancePtr, PhyCfgPtr->phy_address, YT8521_PAGE0,
                            YT8521_CTRL, &PhyReg);
    PhyReg &= (0xFFFF & ~YT8521_COPPER_CTRL_AN_EN);
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "YT8521 close autoneg \r\n");

    /* set speed */
    if (PhyCfgPtr->speed == speed_10)
    {
        PhyReg &= (~YT8521_COPPER_CTRL_SPEED_SEL_LSB);
        PhyReg &= (~YT8521_COPPER_CTRL_SPEED_SEL_MSB);
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "set phy speed 10M \r\n");
    }
    if (PhyCfgPtr->speed == speed_100)
    {
        PhyReg |= YT8521_COPPER_CTRL_SPEED_SEL_LSB;
        PhyReg &= (~YT8521_COPPER_CTRL_SPEED_SEL_MSB);
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "set phy speed 100M \r\n");
    }
    if (PhyCfgPtr->speed == speed_1000)
    {
        PhyReg &= (~YT8521_COPPER_CTRL_SPEED_SEL_LSB);
        PhyReg |= YT8521_COPPER_CTRL_SPEED_SEL_MSB;
        PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "set phy speed 1000M \r\n");
    }

    /* loop back */
    PhyReg |= YT8521_COPPER_CTRL_LOOPBACK;
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "enable PHY loopback \r\n");
    Status |= yt8521_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, YT8521_PAGE0,
                              YT8521_CTRL, (PhyReg));

    return Status;
}

// u8 yt8521_reset(FGmacPs * InstancePtr)
//{
//     u16 reg;
//     u32 phy_timeout=0;
//     reg=yt8521_reg_read(InstancePtr,PAGE0,YT8521_CTRL);
//     SET_BIT(reg,0x8000);
//     yt8521_reg_write(InstancePtr,PAGE0,YT8521_CTRL,reg);
//
//     /* wait reset done */
//     phy_timeout=0;
//     do{
//         reg=yt8521_reg_read(InstancePtr,PAGE0,YT8521_CTRL);
//         phy_timeout++;
//         if(phy_timeout>YT8521_PHY_TIME_OUT){
//             PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "PHY reset timeout \r\n");
//             return ETHERNET_PHY_TIMEOUT;
//         }
//         delay_ms(1);
//     }while((reg&0x8000) != 0);
//     PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "PHY reset ok \r\n");
//
//     /* wait link up */
//     phy_timeout=0;
//     do{
//         reg = yt8521_reg_read(InstancePtr,PAGE0,YT8521_STAT);
//         phy_timeout++;
//         if(phy_timeout>YT8521_PHY_LINK_TIME_OUT){
//             PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "PHY YT8521 link timeout
//             \r\n"); return ETHERNET_PHY_TIMEOUT;
//         }
//         delay_ms(1);
//     }while((reg&0x4) == 0);
//     PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "PHY link up \r\n");
//     return ETHERNET_PHY_OK;
// }

// void yt8521_MDIautoX(FGmacPs * InstancePtr, u8 mode)
//{
//     u16 reg;
//     reg = mvl88e1111_reg_read(InstancePtr,YT8521_PAGE0,PHY_88E1111_SCT);
//     RESET_BIT(reg,PHY_88E1111_SCT_MDIX);
//     reg|=(mode<<5);
//     mvl88e1111_reg_write(InstancePtr,YT8521_PAGE0,PHY_88E1111_SCT,reg);
// }

// u8 yt8521_cfg(FGmacPs * InstancePtr)
//{
//     FGmacPs_PhyConfig_T *pPhyConfig = InstancePtr->phy_cfg;
//     u32 reg;
//
//     reg=yt8521_reg_read(InstancePtr,PAGE0,YT8521_ANA);
//     reg = 0x1de1;
//     yt8521_reg_write(InstancePtr,PAGE0,YT8521_ANA,reg);
//
//     /* auto MDI crossover */
//     //mvl88e1111_MDIautoX(InstancePtr,3);
//     /* AN and speed cfg */
//     if (pPhyConfig->auto_nag_en==0){
//         /* AN disable */
//         reg=yt8521_reg_read(InstancePtr,PAGE0,YT8521_CTRL);
//         RESET_BIT(reg,1<<12);
//         yt8521_reg_write(InstancePtr,PAGE0,YT8521_CTRL,reg);
//         /* speed */
//         reg=yt8521_reg_read(InstancePtr,PAGE0,YT8521_CTRL);
//         RESET_BIT(reg,1<<13);
//         RESET_BIT(reg,1<<6);
//         if (pPhyConfig->speed==speed_10)
//         {
//             RESET_BIT(reg,1<<13);
//             RESET_BIT(reg,1<<6);
//			PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "set phy speed 10M \r\n");
//         }
//         if (pPhyConfig->speed==speed_100)
//         {
//             SET_BIT(reg,1<<13);
//             RESET_BIT(reg,1<<6);
//		    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "set phy speed 100M \r\n");
//         }
//         if (pPhyConfig->speed==speed_1000)
//         {
//             RESET_BIT(reg,1<<13);
//             SET_BIT(reg,1<<6);
//		    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "set phy speed 1000M \r\n");
//         }
//         /* loop back */
//		PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "enable PHY loopback \r\n");
//         SET_BIT(reg,1<<14);
//         PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "Write PHY reg0 = 0x%x \r\n",reg);
//         yt8521_reg_write(InstancePtr,PAGE0,YT8521_CTRL,reg);
//     }
//     else{
//         reg=yt8521_reg_read(InstancePtr,PAGE0,YT8521_CTRL);
//		/* to enable auto-negotiation */
//         SET_BIT(reg,1<<12);
//
//         yt8521_reg_write(InstancePtr,PAGE0,YT8521_CTRL,reg);
//     }
//
//     /* utp=>rgmii */
//     reg=ytphy_read_ext(InstancePtr,PAGE0, 0xa001);    /* NOTE: this function
//     should not be called more than one for each chip. */ reg &=(~7);;
//     ytphy_write_ext(InstancePtr,PAGE0, 0xa001, reg);
//
//     /* set tx delay, rx delay no need to be tuned */
//     ytphy_write_ext(InstancePtr,PAGE0, 0xa003, 0xff);
//
//  	/* acess phy register */
//     ytphy_write_ext(InstancePtr,PAGE0, 0xa000, 0);
//
//     /* disable auto sleep */
//
//     reg=ytphy_read_ext(InstancePtr,PAGE0, YT8521_EXTREG_SLEEP_CONTROL1);
//     reg &=~YT8521_EN_SLEEP_SW_BIT;
//     ytphy_write_ext(InstancePtr,PAGE0, YT8521_EXTREG_SLEEP_CONTROL1, reg);
//
//     /* enable RXC clock when no wire plug */
//
//     reg=ytphy_read_ext(InstancePtr,PAGE0, 0xc);
//     reg &= ~(1 << 12);
//     ytphy_write_ext(InstancePtr,PAGE0, 0xc, reg);
//
//
//     /* timing ctrl */
//     ///mvl88e1111_timing_ctrl(InstancePtr,1,1);
//     /* sw reset */
//
//     /* do not reset again*/
//     //yt8521_reset(InstancePtr);
//
//     return ETHERNET_PHY_OK;
// }
