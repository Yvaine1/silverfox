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

#include "fmsh_gmac_mdio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * Set the MDIO clock divisor.
 *
 * Calculating the divisor:
 *
 * <pre>
 *              f[HOSTCLK]
 *   f[MDC] = -----------------
 *            (1 + Divisor) * 2
 * </pre>
 *
 * where f[HOSTCLK] is the bus clock frequency in MHz, and f[MDC] is the
 * MDIO clock frequency in MHz to the PHY. Typically, f[MDC] should not
 * exceed 2.5 MHz. Some PHYs can tolerate faster speeds which means faster
 * access. Here is the table to show values to generate MDC,
 *
 * <pre>
 * 000 : divide pclk by   8 (pclk up to  20 MHz)
 * 001 : divide pclk by  16 (pclk up to  40 MHz)
 * 010 : divide pclk by  32 (pclk up to  80 MHz)
 * 011 : divide pclk by  48 (pclk up to 120 MHz)
 * 100 : divide pclk by  64 (pclk up to 160 MHz)
 * 101 : divide pclk by  96 (pclk up to 240 MHz)
 * 110 : divide pclk by 128 (pclk up to 320 MHz)
 * 111 : divide pclk by 224 (pclk up to 540 MHz)
 * </pre>
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Divisor is the divisor to set. Range is 0b000 to 0b111.
 *
 *****************************************************************************/
void FGmacPs_SetMdioDivisor (FGmacPs *InstancePtr, FGmacPs_MdcDiv Divisor)
{
    u32 Reg;
    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);
    // FGmacPs_AssertVoid(Divisor <= (FGmacPs_MdcDiv)0x7); /* only last three
    // bits are valid */

    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                          FGMACPS_NWCFG_OFFSET);
    /* clear these three bits, could be done with mask */
    Reg &= (u32)(~FGMACPS_NWCFG_MDCCLKDIV_MASK);

    Reg |= ((u32)Divisor << FGMACPS_NWCFG_MDC_SHIFT_MASK);

    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCFG_OFFSET,
                     Reg);
}

/*****************************************************************************/
/**
 * Read the current value of the PHY register indicated by the PhyAddress and
 * the RegisterNum parameters. The MAC provides the driver with the ability to
 * talk to a PHY that adheres to the Media Independent Interface (MII) as
 * defined in the IEEE 802.3 standard.
 *
 * Prior to PHY access with this function, the user should have setup the MDIO
 * clock with FGmacPs_SetMdioDivisor().
 *
 * @param InstancePtr is a pointer to the FGmacPs instance to be worked on.
 * @param PhyAddress is the address of the PHY to be read (supports multiple
 *        PHYs)
 * @param RegisterNum is the register number, 0-31, of the specific PHY register
 *        to read
 * @param PhyDataPtr is an output parameter, and points to a 16-bit buffer into
 *        which the current value of the register will be copied.
 *
 * @return
 *
 * - FMSH_SUCCESS if the PHY was read from successfully
 * - FGMACPS_MII_BUSY if there is another PHY operation in progress
 *
 * @note
 *
 * This function is not thread-safe. The user must provide mutually exclusive
 * access to this function if there are to be multiple threads that can call it.
 *
 * There is the possibility that this function will not return if the hardware
 * is broken (i.e., it never sets the status bit indicating that the read is
 * done). If this is of concern to the user, the user should provide a mechanism
 * suitable to their needs for recovery.
 *
 * For the duration of this function, all host interface reads and writes are
 * blocked to the current FGmacPs instance.
 *
 ******************************************************************************/
LONG FGmacPs_PhyRead (FGmacPs *InstancePtr, u16 PhyAddress, u16 RegisterNum,
                      u16 *PhyDataPtr)
{
    u32 Mgtcr;
    volatile u32 Ipisr;
    u32 IpReadTemp;
    LONG Status;

    // FGmacPs_AssertNonvoid(InstancePtr != NULL);

    /* Make sure no other PHY operation is currently in progress */
    if ((!(FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                           FGMACPS_NWSR_OFFSET) &
           FGMACPS_NWSR_MDIOIDLE_MASK)) == TRUE)
    {
        Status = (LONG)(FGMACPS_MII_BUSY);
    }
    else
    {
        /* Construct Mgtcr mask for the operation */
        Mgtcr = FGMACPS_PHYMNTNC_OP_MASK | FGMACPS_PHYMNTNC_OP_R_MASK |
                (PhyAddress << FGMACPS_PHYMNTNC_PHAD_SHFT_MSK) |
                (RegisterNum << FGMACPS_PHYMNTNC_PREG_SHFT_MSK);

        /* Write Mgtcr and wait for completion */
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_PHYMNTNC_OFFSET, Mgtcr);

        do
        {
            Ipisr = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                    FGMACPS_NWSR_OFFSET);
            IpReadTemp = Ipisr;
        } while ((IpReadTemp & FGMACPS_NWSR_MDIOIDLE_MASK) == 0x00000000U);

        /* Read data */
        *PhyDataPtr = (u16)FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                           FGMACPS_PHYMNTNC_OFFSET);
        Status = (LONG)(FMSH_SUCCESS);
    }
    return Status;
}

/*****************************************************************************/
/**
 * Write data to the specified PHY register. The Ethernet driver does not
 * require the device to be stopped before writing to the PHY.  Although it is
 * probably a good idea to stop the device, it is the responsibility of the
 * application to deem this necessary. The MAC provides the driver with the
 * ability to talk to a PHY that adheres to the Media Independent Interface
 * (MII) as defined in the IEEE 802.3 standard.
 *
 * Prior to PHY access with this function, the user should have setup the MDIO
 * clock with FGmacPs_SetMdioDivisor().
 *
 * @param InstancePtr is a pointer to the FGmacPs instance to be worked on.
 * @param PhyAddress is the address of the PHY to be written (supports multiple
 *        PHYs)
 * @param RegisterNum is the register number, 0-31, of the specific PHY register
 *        to write
 * @param PhyData is the 16-bit value that will be written to the register
 *
 * @return
 *
 * - FMSH_SUCCESS if the PHY was written to successfully. Since there is no
 *error status from the MAC on a write, the user should read the PHY to verify
 *the write was successful.
 * - FGMACPS_MII_BUSY if there is another PHY operation in progress
 *
 * @note
 *
 * This function is not thread-safe. The user must provide mutually exclusive
 * access to this function if there are to be multiple threads that can call it.
 *
 * There is the possibility that this function will not return if the hardware
 * is broken (i.e., it never sets the status bit indicating that the write is
 * done). If this is of concern to the user, the user should provide a mechanism
 * suitable to their needs for recovery.
 *
 * For the duration of this function, all host interface reads and writes are
 * blocked to the current FGmacPs instance.
 *
 ******************************************************************************/
LONG FGmacPs_PhyWrite (FGmacPs *InstancePtr, u16 PhyAddress, u16 RegisterNum,
                       u16 PhyData)
{
    u32 Mgtcr;
    volatile u32 Ipisr;
    u32 IpWriteTemp;
    LONG Status;

    // FGmacPs_AssertNonvoid(InstancePtr != NULL);

    /* Make sure no other PHY operation is currently in progress */
    if ((!(FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                           FGMACPS_NWSR_OFFSET) &
           FGMACPS_NWSR_MDIOIDLE_MASK)) == TRUE)
    {
        Status = (LONG)(FGMACPS_MII_BUSY);
    }
    else
    {
        /* Construct Mgtcr mask for the operation */
        Mgtcr = FGMACPS_PHYMNTNC_OP_MASK | FGMACPS_PHYMNTNC_OP_W_MASK |
                (PhyAddress << FGMACPS_PHYMNTNC_PHAD_SHFT_MSK) |
                (RegisterNum << FGMACPS_PHYMNTNC_PREG_SHFT_MSK) | (u32)PhyData;

        /* Write Mgtcr and wait for completion */
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_PHYMNTNC_OFFSET, Mgtcr);

        do
        {
            Ipisr = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                    FGMACPS_NWSR_OFFSET);
            IpWriteTemp = Ipisr;
        } while ((IpWriteTemp & FGMACPS_NWSR_MDIOIDLE_MASK) == 0x00000000U);

        Status = (LONG)(FMSH_SUCCESS);
    }
    return Status;
}

/****************************************************************************/
/**
 *
 * This function detects the PHY address by looking for successful MII status
 * register contents.
 *
 * @param    The FGMACPS driver instance
 *
 * @return   The address of the PHY (defaults to 32 if none detected)
 *
 * @note     None.
 *
 *****************************************************************************/
u16 FGmacPs_PHYDetect (FGmacPs *InstancePtr)
{
    u16 PhyAddr;
    u32 Status;
    u16 PhyReg1;
    u16 PhyReg2;

    for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++)
    {
        Status = FGmacPs_PhyRead(InstancePtr, PhyAddr, IEEE_PHY_DETECT_REG1,
                                 &PhyReg1);
        Status |= FGmacPs_PhyRead(InstancePtr, PhyAddr, IEEE_PHY_DETECT_REG2,
                                  &PhyReg2);
        if ((Status == FMSH_SUCCESS) && (PhyReg1 > 0x0000) &&
            (PhyReg1 < 0xffff) && (PhyReg2 > 0x0000) && (PhyReg2 < 0xffff))
        {
            /* Found a valid PHY address */
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "PHY detected, addr%d.\r\n",
                          PhyAddr);
            return PhyAddr;
        }
    }

    /* PhyAddr default to 32(max of iteration) */
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                  "PHY detect fail, set phyaddr to 0.\r\n");
    return 0;
}

/****************************************************************************/
/**
 *
 * This function detects the PHY address by looking for successful MII status
 * register contents.
 *
 * @param    The FGMACPS driver instance
 *
 * @return   The address of the PHY (defaults to 32 if none detected)
 *
 * @note     None.
 *
 *****************************************************************************/
u16 FGmacPs_PHYDetect_full (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    u16 PhyAddr;

    switch (PhyCfgPtr->phy_device)
    {
    case PHY_88E1512:
        PhyAddr = mv88e1512_PHYDetect(InstancePtr);
        break;
    case PHY_KSZ9031RNX:
        PhyAddr = FGmacPs_PHYDetect(InstancePtr);
        break;
    case PHY_YT8521:
        PhyAddr = yt8521_detect(InstancePtr);
        break;
    case PHY_JL2XX1:
        PhyAddr = jl2xx1_detect(InstancePtr);
        break;
    default:
        PhyAddr = FGmacPs_PHYDetect(InstancePtr);
        break;
    }

    return PhyAddr;
}

/****************************************************************************/
/**
 *
 * This function detects the PHY address by looking for successful MII status
 * register contents.
 *
 * @param    The FGMACPS driver instance
 *
 * @return   The address of the PHY (defaults to 32 if none detected)
 *
 * @note     None.
 *
 *****************************************************************************/
LONG FGmacPs_PHYInit (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    u32 Status;

    switch (PhyCfgPtr->phy_device)
    {
    case PHY_88E1512:
        Status = mv88e1512_setup_full(InstancePtr, PhyCfgPtr);
        break;
    case PHY_KSZ9031RNX:
        Status = mic9031RNX_setup(InstancePtr, PhyCfgPtr);
        break;
    case PHY_YT8521:
        Status = yt8521_setup(InstancePtr, PhyCfgPtr);
        break;
    case PHY_JL2XX1:
        Status = jl2xx1_setup(InstancePtr, PhyCfgPtr);
        PhyCfgPtr->phy_address = 3;
        jl2xx1_setup(InstancePtr,PhyCfgPtr);
        break;
    default:
        Status = FMSH_SUCCESS;
        break;
    }

    return Status;
}

/****************************************************************************/
/**
 *
 * This function detects the PHY address by looking for successful MII status
 * register contents.
 *
 * @param    The FGMACPS driver instance
 *
 * @return   The address of the PHY (defaults to 32 if none detected)
 *
 * @note     None.
 *
 *****************************************************************************/
LONG FGmacPs_PHYloopback (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    u32 Status;

    switch (PhyCfgPtr->phy_device)
    {
    case PHY_88E1512:
        Status = mv88e1512_system_interface_loopback(InstancePtr, PhyCfgPtr);
        break;
    case PHY_KSZ9031RNX:
        Status = mic9031RNX_digital_loopback(InstancePtr, PhyCfgPtr);
        break;
    case PHY_YT8521:
        Status = yt8521_system_interface_loopback(InstancePtr, PhyCfgPtr);
        break;
    default:
        Status = FMSH_SUCCESS;
        break;
    }

    return Status;
}
