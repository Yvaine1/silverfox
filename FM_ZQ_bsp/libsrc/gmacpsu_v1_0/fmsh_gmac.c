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

#include "fmsh_gmac.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void FGmacPs_StubHandler(void); /* Default handler routine */

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * Initialize a specific FGmacPs instance/driver. The initialization entails:
 * - Initialize fields of the FGmacPs instance structure
 * - Reset hardware and apply default options
 * - Configure the DMA channels
 *
 * The PHY is setup independently from the device. Use the MII or whatever other
 * interface may be present for setup.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param CfgPtr is the device configuration structure containing required
 *        hardware build data.
 * @param EffectiveAddress is the base address of the device. If address
 *        translation is not utilized, this parameter can be passed in using
 *        CfgPtr->Config.BaseAddress to specify the physical base address.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/
LONG FGmacPs_CfgInitialize (FGmacPs *InstancePtr, FGmacPs_Config *CfgPtr,
                            UINTPTR EffectiveAddress)
{
    /* Verify arguments */
    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(CfgPtr != NULL);

    /* Set device base address and ID */
    InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
    InstancePtr->Config.BaseAddress = EffectiveAddress;
    InstancePtr->Config.IsCacheCoherent = CfgPtr->IsCacheCoherent;
    InstancePtr->Config.Speed = CfgPtr->Speed;
    InstancePtr->Config.InterFaceType = CfgPtr->InterFaceType;

    /* Set callbacks to an initial stub routine */
    InstancePtr->SendHandler = ((FGmacPs_Handler)((void *)FGmacPs_StubHandler));
    InstancePtr->RecvHandler = ((FGmacPs_Handler)(void *)FGmacPs_StubHandler);
    InstancePtr->ErrorHandler = ((
        FGmacPs_ErrHandler)(void *)FGmacPs_StubHandler);

    /* Reset the hardware and set default options */
    InstancePtr->IsReady = COMPONENT_IS_READY;
    FGmacPs_Reset(InstancePtr);

    return (LONG)(FMSH_SUCCESS);
}

/*****************************************************************************/
/**
 * Start the Ethernet controller as follows:
 *   - Enable transmitter if XTE_TRANSMIT_ENABLE_OPTION is set
 *   - Enable receiver if XTE_RECEIVER_ENABLE_OPTION is set
 *   - Start the SG DMA send and receive channels and enable the device
 *     interrupt
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return N/A
 *
 * @note
 * Hardware is configured with scatter-gather DMA, the driver expects to start
 * the scatter-gather channels and expects that the user has previously set up
 * the buffer descriptor lists.
 *
 * This function makes use of internal resources that are shared between the
 * Start, Stop, and Set/ClearOptions functions. So if one task might be setting
 * device options while another is trying to start the device, the user is
 * required to provide protection of this shared data (typically using a
 * semaphore).
 *
 * This function must not be preempted by an interrupt that may service the
 * device.
 *
 ******************************************************************************/
void FGmacPs_Start (FGmacPs *InstancePtr)
{
    u32 Reg;

    /* Assert bad arguments and conditions */
    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Start DMA */
    /* When starting the DMA channels, both transmit and receive sides
     * need an initialized BD list.
     */
    if (InstancePtr->Version == 2)
    {
        // FGmacPs_AssertVoid(InstancePtr->RxBdRing.BaseBdAddr != 0);
        // FGmacPs_AssertVoid(InstancePtr->TxBdRing.BaseBdAddr != 0);
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_RXQBASE_OFFSET,
                         InstancePtr->RxBdRing.BaseBdAddr);

        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_TXQBASE_OFFSET,
                         InstancePtr->TxBdRing.BaseBdAddr);
    }

    /* clear any existed int status */
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_ISR_OFFSET,
                     FGMACPS_IXR_ALL_MASK);

    /* Enable transmitter if not already enabled */
    if ((InstancePtr->Options & (u32)FGMACPS_TRANSMITTER_ENABLE_OPTION) !=
        0x00000000U)
    {
        Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                              FGMACPS_NWCTRL_OFFSET);
        if ((!(Reg & FGMACPS_NWCTRL_TXEN_MASK)) == TRUE)
        {
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCTRL_OFFSET,
                             Reg | (u32)FGMACPS_NWCTRL_TXEN_MASK);
        }
    }

    /* Enable receiver if not already enabled */
    if ((InstancePtr->Options & FGMACPS_RECEIVER_ENABLE_OPTION) != 0x00000000U)
    {
        Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                              FGMACPS_NWCTRL_OFFSET);
        if ((!(Reg & FGMACPS_NWCTRL_RXEN_MASK)) == TRUE)
        {
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCTRL_OFFSET,
                             Reg | (u32)FGMACPS_NWCTRL_RXEN_MASK);
        }
    }

    /* Enable TX and RX interrupts */
    FGmacPs_IntEnable(
        InstancePtr,
        (FGMACPS_IXR_TX_ERR_MASK | FGMACPS_IXR_RX_ERR_MASK |
         (u32)FGMACPS_IXR_FRAMERX_MASK | (u32)FGMACPS_IXR_TXCOMPL_MASK));

    /* Enable TX Q1 Interrupts */
    if (InstancePtr->Version > 2)
    {
        FGmacPs_IntQ1Enable(InstancePtr, FGMACPS_INTQ1_IXR_ALL_MASK);
    }

    /* Mark as started */
    InstancePtr->IsStarted = COMPONENT_IS_STARTED;

    return;
}

/*****************************************************************************/
/**
 * Gracefully stop the Ethernet MAC as follows:
 *   - Disable all interrupts from this device
 *   - Stop DMA channels
 *   - Disable the tansmitter and receiver
 *
 * Device options currently in effect are not changed.
 *
 * This function will disable all interrupts. Default interrupts settings that
 * had been enabled will be restored when FGmacPs_Start() is called.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @note
 * This function makes use of internal resources that are shared between the
 * Start, Stop, SetOptions, and ClearOptions functions. So if one task might be
 * setting device options while another is trying to start the device, the user
 * is required to provide protection of this shared data (typically using a
 * semaphore).
 *
 * Stopping the DMA channels causes this function to block until the DMA
 * operation is complete.
 *
 ******************************************************************************/
void FGmacPs_Stop (FGmacPs *InstancePtr)
{
    u32 Reg;

    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Disable all interrupts */
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_IDR_OFFSET,
                     FGMACPS_IXR_ALL_MASK);

    /* Disable the receiver & transmitter */
    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                          FGMACPS_NWCTRL_OFFSET);
    Reg &= (u32)(~FGMACPS_NWCTRL_RXEN_MASK);
    Reg &= (u32)(~FGMACPS_NWCTRL_TXEN_MASK);
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCTRL_OFFSET,
                     Reg);

    /* Mark as stopped */
    InstancePtr->IsStarted = 0U;
}

/*****************************************************************************/
/**
 * Perform a graceful reset of the Ethernet MAC. Resets the DMA channels, the
 * transmitter, and the receiver.
 *
 * Steps to reset
 * - Stops transmit and receive channels
 * - Stops DMA
 * - Configure transmit and receive buffer size to default
 * - Clear transmit and receive status register and counters
 * - Clear all interrupt sources
 * - Clear phy (if there is any previously detected) address
 * - Clear MAC addresses (1-4) as well as Type IDs and hash value
 *
 * All options are placed in their default state. Any frames in the
 * descriptor lists will remain in the lists. The side effect of doing
 * this is that after a reset and following a restart of the device, frames
 * were in the list before the reset may be transmitted or received.
 *
 * The upper layer software is responsible for re-configuring (if necessary)
 * and restarting the MAC after the reset. Note also that driver statistics
 * are not cleared on reset. It is up to the upper layer software to clear the
 * statistics if needed.
 *
 * When a reset is required, the driver notifies the upper layer software of
 * this need through the ErrorHandler callback and specific status codes.
 * The upper layer software is responsible for calling this Reset function
 * and then re-configuring the device.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 ******************************************************************************/
void FGmacPs_Reset (FGmacPs *InstancePtr)
{
    u32 Reg;
    u8 i;
    s8 EmacPs_zero_MAC[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Stop the device and reset hardware */
    FGmacPs_Stop(InstancePtr);
    InstancePtr->Options = FGMACPS_DEFAULT_OPTIONS;

    InstancePtr->Version = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                           0xFC);

    InstancePtr->Version = (InstancePtr->Version >> 16) & 0xFFF;

    InstancePtr->MaxMtuSize = FGMACPS_MTU;
    InstancePtr->MaxFrameSize = FGMACPS_MTU + FGMACPS_HDR_SIZE +
                                FGMACPS_TRL_SIZE;
    InstancePtr->MaxVlanFrameSize = InstancePtr->MaxFrameSize +
                                    FGMACPS_HDR_VLAN_SIZE;
    InstancePtr->RxBufMask = FGMACPS_RXBUF_LEN_MASK;

    /* Setup hardware with default values */
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCTRL_OFFSET,
                     (FGMACPS_NWCTRL_STATCLR_MASK | FGMACPS_NWCTRL_MDEN_MASK) &
                         (u32)(~FGMACPS_NWCTRL_LOOPEN_MASK));

    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                          FGMACPS_NWCFG_OFFSET);
    Reg &= FGMACPS_NWCFG_MDCCLKDIV_MASK;

    Reg = Reg | (u32)FGMACPS_NWCFG_100_MASK | (u32)FGMACPS_NWCFG_FDEN_MASK |
          (u32)FGMACPS_NWCFG_UCASTHASHEN_MASK;

    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCFG_OFFSET,
                     Reg);
    if (InstancePtr->Version > 2)
    {
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCFG_OFFSET,
                         (FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                          FGMACPS_NWCFG_OFFSET) |
                          FGMACPS_NWCFG_DWIDTH_64_MASK));
    }

    FGmacPs_WriteReg(
        InstancePtr->Config.BaseAddress, FGMACPS_DMACR_OFFSET,
        (((((u32)FGMACPS_RX_BUF_SIZE / (u32)FGMACPS_RX_BUF_UNIT) +
           (((((u32)FGMACPS_RX_BUF_SIZE % (u32)FGMACPS_RX_BUF_UNIT)) != (u32)0)
                ? 1U
                : 0U))
          << (u32)(FGMACPS_DMACR_RXBUF_SHIFT)) &
         (u32)(FGMACPS_DMACR_RXBUF_MASK)) |
            (u32)FGMACPS_DMACR_RXSIZE_MASK | (u32)FGMACPS_DMACR_TXSIZE_MASK);

    if (InstancePtr->Version > 2)
    {
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_DMACR_OFFSET,
                         (FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                          FGMACPS_DMACR_OFFSET) |
#if defined(__aarch64__) || defined(__arch64__)
                          (u32)FGMACPS_DMACR_ADDR_WIDTH_64 |
#endif
                          (u32)FGMACPS_DMACR_INCR16_AHB_BURST));
    }

    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_TXSR_OFFSET,
                     0x0U);

    FGmacPs_SetQueuePtr(InstancePtr, 0, 0x00U, (u16)FGMACPS_SEND);
    if (InstancePtr->Version > 2)
    {
        FGmacPs_SetQueuePtr(InstancePtr, 0, 0x01U, (u16)FGMACPS_SEND);
    }
    FGmacPs_SetQueuePtr(InstancePtr, 0, 0x00U, (u16)FGMACPS_RECV);

    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_RXSR_OFFSET,
                     0x0U);

    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_IDR_OFFSET,
                     FGMACPS_IXR_ALL_MASK);

    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress, FGMACPS_ISR_OFFSET);
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_ISR_OFFSET, Reg);

    FGmacPs_ClearHash(InstancePtr);

    for (i = 1U; i < 5U; i++)
    {
        (void)FGmacPs_SetMacAddress(InstancePtr, EmacPs_zero_MAC, i);
        (void)FGmacPs_SetTypeIdCheck(InstancePtr, 0x00000000U, i);
    }

    /* clear all counters */
    for (i = 0U; i < (u8)((FGMACPS_LAST_OFFSET - FGMACPS_OCTTXL_OFFSET) / 4U);
         i++)
    {
        (void)FGmacPs_ReadReg(
            InstancePtr->Config.BaseAddress,
            FGMACPS_OCTTXL_OFFSET + (u32)(((u32)i) * ((u32)4)));
    }

    /* Disable the receiver */
    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                          FGMACPS_NWCTRL_OFFSET);
    Reg &= (u32)(~FGMACPS_NWCTRL_RXEN_MASK);
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCTRL_OFFSET,
                     Reg);

    /* Sync default options with hardware but leave receiver and
     * transmitter disabled. They get enabled with FGmacPs_Start() if
     * FGMACPS_TRANSMITTER_ENABLE_OPTION and
     * FGMACPS_RECEIVER_ENABLE_OPTION are set.
     */
    (void)FGmacPs_SetOptions(
        InstancePtr,
        InstancePtr->Options & ~((u32)FGMACPS_TRANSMITTER_ENABLE_OPTION |
                                 (u32)FGMACPS_RECEIVER_ENABLE_OPTION));

    (void)FGmacPs_ClearOptions(InstancePtr, ~InstancePtr->Options);
}

/******************************************************************************/
/**
 * This is a stub for the asynchronous callbacks. The stub is here in case the
 * upper layer forgot to set the handler(s). On initialization, all handlers are
 * set to this callback. It is considered an error for this handler to be
 * invoked.
 *
 ******************************************************************************/
void FGmacPs_StubHandler (void) { FGmacPs_AssertVoidAlways(); }

/*****************************************************************************/
/**
 * This function sets the start address of the transmit/receive buffer queue.
 *
 * @param	InstancePtr is a pointer to the instance to be worked on.
 * @param	QPtr is the address of the Queue to be written
 * @param	QueueNum is the Buffer Queue Index
 * @param	Direction indicates Transmit/Recive
 *
 * @note
 * The buffer queue addresses has to be set before starting the transfer, so
 * this function has to be called in prior to FGmacPs_Start()
 *
 ******************************************************************************/
void FGmacPs_SetQueuePtr (FGmacPs *InstancePtr, UINTPTR QPtr, u8 QueueNum,
                          u16 Direction)
{
    /* Assert bad arguments and conditions */
    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* If already started, then there is nothing to do */
    if (InstancePtr->IsStarted == (u32)COMPONENT_IS_STARTED)
    {
        return;
    }

    if (QueueNum == 0x00U)
    {
        if (Direction == FGMACPS_SEND)
        {
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_TXQBASE_OFFSET, (QPtr & ULONG64_LO_MASK));
        }
        else
        {
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_RXQBASE_OFFSET, (QPtr & ULONG64_LO_MASK));
        }
    }
    else
    {
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_TXQ1BASE_OFFSET, (QPtr & ULONG64_LO_MASK));
    }
#ifdef __aarch64__
    if (Direction == FGMACPS_SEND)
    {
        /* Set the MSB of TX Queue start address */
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_MSBBUF_TXQBASE_OFFSET,
                         (u32)((QPtr & ULONG64_HI_MASK) >> 32U));
    }
    else
    {
        /* Set the MSB of RX Queue start address */
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_MSBBUF_RXQBASE_OFFSET,
                         (u32)((QPtr & ULONG64_HI_MASK) >> 32U));
    }
#endif
}

/*****************************************************************************/
/**
 * Set the MAC address for this driver/device.  The address is a 48-bit value.
 * The device must be stopped before calling this function.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is a pointer to a 6-byte MAC address.
 * @param Index is a index to which MAC (1-4) address.
 *
 * @return
 * - FMSH_SUCCESS if the MAC address was set successfully
 * - FGMACPS_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 *****************************************************************************/
LONG FGmacPs_SetMacAddress (FGmacPs *InstancePtr, void *AddressPtr, u8 Index)
{
    u32 MacAddr;
    u8 *Aptr = (u8 *)(void *)AddressPtr;
    u8 IndexLoc = Index;
    LONG Status;
    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(Aptr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);
    // FGmacPs_AssertNonvoid((IndexLoc <= (u8)FGMACPS_MAX_MAC_ADDR) && (IndexLoc
    // > 0x00U));

    /* Be sure device has been stopped */
    if (InstancePtr->IsStarted == (u32)COMPONENT_IS_STARTED)
    {
        Status = (LONG)(FGMACPS_DEVICE_IS_STARTED);
    }
    else
    {
        /* Index ranges 1 to 4, for offset calculation is 0 to 3. */
        IndexLoc--;

        /* Set the MAC bits [31:0] in BOT */
        MacAddr = *(Aptr);
        MacAddr |= ((u32)(*(Aptr + 1)) << 8U);
        MacAddr |= ((u32)(*(Aptr + 2)) << 16U);
        MacAddr |= ((u32)(*(Aptr + 3)) << 24U);
        FGmacPs_WriteReg(
            InstancePtr->Config.BaseAddress,
            ((u32)FGMACPS_LADDR1L_OFFSET + ((u32)IndexLoc * (u32)8)), MacAddr);

        /* There are reserved bits in TOP so don't affect them */
        MacAddr = FGmacPs_ReadReg(
            InstancePtr->Config.BaseAddress,
            ((u32)FGMACPS_LADDR1H_OFFSET + ((u32)IndexLoc * (u32)8)));

        MacAddr &= (u32)(~FGMACPS_LADDR_MACH_MASK);

        /* Set MAC bits [47:32] in TOP */
        MacAddr |= (u32)(*(Aptr + 4));
        MacAddr |= (u32)(*(Aptr + 5)) << 8U;

        FGmacPs_WriteReg(
            InstancePtr->Config.BaseAddress,
            ((u32)FGMACPS_LADDR1H_OFFSET + ((u32)IndexLoc * (u32)8)), MacAddr);

        Status = (LONG)(FMSH_SUCCESS);
    }
    return Status;
}

/*****************************************************************************/
/**
 * Get the MAC address for this driver/device.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is an output parameter, and is a pointer to a buffer into
 *        which the current MAC address will be copied.
 * @param Index is a index to which MAC (1-4) address.
 *
 *****************************************************************************/
void FGmacPs_GetMacAddress (FGmacPs *InstancePtr, void *AddressPtr, u8 Index)
{
    u32 MacAddr;
    u8 *Aptr = (u8 *)(void *)AddressPtr;
    u8 IndexLoc = Index;
    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(Aptr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);
    // FGmacPs_AssertVoid((IndexLoc <= (u8)FGMACPS_MAX_MAC_ADDR) && (IndexLoc >
    // 0x00U));

    /* Index ranges 1 to 4, for offset calculation is 0 to 3. */
    IndexLoc--;

    MacAddr = FGmacPs_ReadReg(
        InstancePtr->Config.BaseAddress,
        ((u32)FGMACPS_LADDR1L_OFFSET + ((u32)IndexLoc * (u32)8)));
    *Aptr = (u8)MacAddr;
    *(Aptr + 1) = (u8)(MacAddr >> 8U);
    *(Aptr + 2) = (u8)(MacAddr >> 16U);
    *(Aptr + 3) = (u8)(MacAddr >> 24U);

    /* Read MAC bits [47:32] in TOP */
    MacAddr = FGmacPs_ReadReg(
        InstancePtr->Config.BaseAddress,
        ((u32)FGMACPS_LADDR1H_OFFSET + ((u32)IndexLoc * (u32)8)));
    *(Aptr + 4) = (u8)MacAddr;
    *(Aptr + 5) = (u8)(MacAddr >> 8U);
}

/*****************************************************************************/
/**
 * Set 48-bit MAC addresses in hash table.
 * The device must be stopped before calling this function.
 *
 * The hash address register is 64 bits long and takes up two locations in
 * the memory map. The least significant bits are stored in hash register
 * bottom and the most significant bits in hash register top.
 *
 * The unicast hash enable and the multicast hash enable bits in the network
 * configuration register enable the reception of hash matched frames. The
 * destination address is reduced to a 6 bit index into the 64 bit hash
 * register using the following hash function. The hash function is an XOR
 * of every sixth bit of the destination address.
 *
 * <pre>
 * hash_index[05] = da[05]^da[11]^da[17]^da[23]^da[29]^da[35]^da[41]^da[47]
 * hash_index[04] = da[04]^da[10]^da[16]^da[22]^da[28]^da[34]^da[40]^da[46]
 * hash_index[03] = da[03]^da[09]^da[15]^da[21]^da[27]^da[33]^da[39]^da[45]
 * hash_index[02] = da[02]^da[08]^da[14]^da[20]^da[26]^da[32]^da[38]^da[44]
 * hash_index[01] = da[01]^da[07]^da[13]^da[19]^da[25]^da[31]^da[37]^da[43]
 * hash_index[00] = da[00]^da[06]^da[12]^da[18]^da[24]^da[30]^da[36]^da[42]
 * </pre>
 *
 * da[0] represents the least significant bit of the first byte received,
 * that is, the multicast/unicast indicator, and da[47] represents the most
 * significant bit of the last byte received.
 *
 * If the hash index points to a bit that is set in the hash register then
 * the frame will be matched according to whether the frame is multicast
 * or unicast.
 *
 * A multicast match will be signaled if the multicast hash enable bit is
 * set, da[0] is logic 1 and the hash index points to a bit set in the hash
 * register.
 *
 * A unicast match will be signaled if the unicast hash enable bit is set,
 * da[0] is logic 0 and the hash index points to a bit set in the hash
 * register.
 *
 * To receive all multicast frames, the hash register should be set with
 * all ones and the multicast hash enable bit should be set in the network
 * configuration register.
 *
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is a pointer to a 6-byte MAC address.
 *
 * @return
 * - FMSH_SUCCESS if the HASH MAC address was set successfully
 * - FGMACPS_DEVICE_IS_STARTED if the device has not yet been stopped
 * - FGMACPS_INVALID_PARAM if the HASH MAC address passed in does not meet
 *   requirement after calculation
 *
 * @note
 * Having Aptr be unsigned type prevents the following operations from sign
 * extending.
 *****************************************************************************/
LONG FGmacPs_SetHash (FGmacPs *InstancePtr, void *AddressPtr)
{
    u32 HashAddr;
    u8 *Aptr = (u8 *)(void *)AddressPtr;
    u8 Temp1, Temp2, Temp3, Temp4, Temp5, Temp6, Temp7, Temp8;
    u32 Result;
    LONG Status;

    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(AddressPtr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Be sure device has been stopped */
    if (InstancePtr->IsStarted == (u32)COMPONENT_IS_STARTED)
    {
        Status = (LONG)(FGMACPS_DEVICE_IS_STARTED);
    }
    else
    {
        Temp1 = (*(Aptr + 0)) & 0x3FU;
        Temp2 = ((*(Aptr + 0) >> 6U) & 0x03U) | ((*(Aptr + 1) & 0x0FU) << 2U);

        Temp3 = ((*(Aptr + 1) >> 4U) & 0x0FU) | ((*(Aptr + 2) & 0x3U) << 4U);
        Temp4 = ((*(Aptr + 2) >> 2U) & 0x3FU);
        Temp5 = (*(Aptr + 3)) & 0x3FU;
        Temp6 = ((*(Aptr + 3) >> 6U) & 0x03U) | ((*(Aptr + 4) & 0x0FU) << 2U);
        Temp7 = ((*(Aptr + 4) >> 4U) & 0x0FU) | ((*(Aptr + 5) & 0x03U) << 4U);
        Temp8 = ((*(Aptr + 5) >> 2U) & 0x3FU);

        Result = (u32)((u32)Temp1 ^ (u32)Temp2 ^ (u32)Temp3 ^ (u32)Temp4 ^
                       (u32)Temp5 ^ (u32)Temp6 ^ (u32)Temp7 ^ (u32)Temp8);

        if (Result >= (u32)FGMACPS_MAX_HASH_BITS)
        {
            Status = (LONG)(FGMACPS_INVALID_PARAM);
        }
        else
        {
            if (Result < (u32)32)
            {
                HashAddr = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                           FGMACPS_HASHL_OFFSET);
                HashAddr |= (u32)(0x00000001U << Result);
                FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                                 FGMACPS_HASHL_OFFSET, HashAddr);
            }
            else
            {
                HashAddr = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                           FGMACPS_HASHH_OFFSET);
                HashAddr |= (u32)(0x00000001U << (u32)(Result - (u32)32));
                FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                                 FGMACPS_HASHH_OFFSET, HashAddr);
            }
            Status = (LONG)(FMSH_SUCCESS);
        }
    }
    return Status;
}

/*****************************************************************************/
/**
 * Delete 48-bit MAC addresses in hash table.
 * The device must be stopped before calling this function.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is a pointer to a 6-byte MAC address.
 *
 * @return
 * - FMSH_SUCCESS if the HASH MAC address was deleted successfully
 * - FGMACPS_DEVICE_IS_STARTED if the device has not yet been stopped
 * - FGMACPS_INVALID_PARAM if the HASH MAC address passed in does not meet
 *   requirement after calculation
 *
 * @note
 * Having Aptr be unsigned type prevents the following operations from sign
 * extending.
 *****************************************************************************/
LONG FGmacPs_DeleteHash (FGmacPs *InstancePtr, void *AddressPtr)
{
    u32 HashAddr;
    u8 *Aptr = (u8 *)(void *)AddressPtr;
    u8 Temp1, Temp2, Temp3, Temp4, Temp5, Temp6, Temp7, Temp8;
    u32 Result;
    LONG Status;

    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(Aptr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Be sure device has been stopped */
    if (InstancePtr->IsStarted == (u32)COMPONENT_IS_STARTED)
    {
        Status = (LONG)(FGMACPS_DEVICE_IS_STARTED);
    }
    else
    {
        Temp1 = (*(Aptr + 0)) & 0x3FU;
        Temp2 = ((*(Aptr + 0) >> 6U) & 0x03U) | ((*(Aptr + 1) & 0x0FU) << 2U);
        Temp3 = ((*(Aptr + 1) >> 4U) & 0x0FU) | ((*(Aptr + 2) & 0x03U) << 4U);
        Temp4 = ((*(Aptr + 2) >> 2U) & 0x3FU);
        Temp5 = (*(Aptr + 3)) & 0x3FU;
        Temp6 = ((*(Aptr + 3) >> 6U) & 0x03U) | ((*(Aptr + 4) & 0x0FU) << 2U);
        Temp7 = ((*(Aptr + 4) >> 4U) & 0x0FU) | ((*(Aptr + 5) & 0x03U) << 4U);
        Temp8 = ((*(Aptr + 5) >> 2U) & 0x3FU);

        Result = (u32)((u32)Temp1 ^ (u32)Temp2 ^ (u32)Temp3 ^ (u32)Temp4 ^
                       (u32)Temp5 ^ (u32)Temp6 ^ (u32)Temp7 ^ (u32)Temp8);

        if (Result >= (u32)(FGMACPS_MAX_HASH_BITS))
        {
            Status = (LONG)(FGMACPS_INVALID_PARAM);
        }
        else
        {
            if (Result < (u32)32)
            {
                HashAddr = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                           FGMACPS_HASHL_OFFSET);
                HashAddr &= (u32)(~(0x00000001U << Result));
                FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                                 FGMACPS_HASHL_OFFSET, HashAddr);
            }
            else
            {
                HashAddr = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                           FGMACPS_HASHH_OFFSET);
                HashAddr &= (u32)(~(0x00000001U << (u32)(Result - (u32)32)));
                FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                                 FGMACPS_HASHH_OFFSET, HashAddr);
            }
            Status = (LONG)(FMSH_SUCCESS);
        }
    }
    return Status;
}
/*****************************************************************************/
/**
 * Clear the Hash registers for the mac address pointed by AddressPtr.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 *****************************************************************************/
void FGmacPs_ClearHash (FGmacPs *InstancePtr)
{
    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_HASHL_OFFSET,
                     0x0U);

    /* write bits [63:32] in TOP */
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_HASHH_OFFSET,
                     0x0U);
}

/*****************************************************************************/
/**
 * Get the Hash address for this driver/device.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param AddressPtr is an output parameter, and is a pointer to a buffer into
 *        which the current HASH MAC address will be copied.
 *
 *****************************************************************************/
void FGmacPs_GetHash (FGmacPs *InstancePtr, void *AddressPtr)
{
    u32 *Aptr = (u32 *)(void *)AddressPtr;

    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(AddressPtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    *(Aptr + 0) = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_HASHL_OFFSET);

    /* Read Hash bits [63:32] in TOP */
    *(Aptr + 1) = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_HASHH_OFFSET);
}

/*****************************************************************************/
/**
 * Set the Type ID match for this driver/device.  The register is a 32-bit
 * value. The device must be stopped before calling this function.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Id_Check is type ID to be configured.
 * @param Index is a index to which Type ID (1-4).
 *
 * @return
 * - FMSH_SUCCESS if the MAC address was set successfully
 * - FGMACPS_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 *****************************************************************************/
LONG FGmacPs_SetTypeIdCheck (FGmacPs *InstancePtr, u32 Id_Check, u8 Index)
{
    u8 IndexLoc = Index;
    LONG Status;
    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);
    // FGmacPs_AssertNonvoid((IndexLoc <= (u8)FGMACPS_MAX_TYPE_ID) && (IndexLoc
    // > 0x00U));

    /* Be sure device has been stopped */
    if (InstancePtr->IsStarted == (u32)COMPONENT_IS_STARTED)
    {
        Status = (LONG)(FGMACPS_DEVICE_IS_STARTED);
    }
    else
    {
        /* Index ranges 1 to 4, for offset calculation is 0 to 3. */
        IndexLoc--;

        /* Set the ID bits in MATCHx register */
        FGmacPs_WriteReg(
            InstancePtr->Config.BaseAddress,
            ((u32)FGMACPS_MATCH1_OFFSET + ((u32)IndexLoc * (u32)4)), Id_Check);

        Status = (LONG)(FMSH_SUCCESS);
    }
    return Status;
}

/*****************************************************************************/
/**
 * Set options for the driver/device. The driver should be stopped with
 * FGmacPs_Stop() before changing options.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Options are the options to set. Multiple options can be set by OR'ing
 *        XTE_*_OPTIONS constants together. Options not specified are not
 *        affected.
 *
 * @return
 * - FMSH_SUCCESS if the options were set successfully
 * - FGMACPS_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 * @note
 * See fmsh_gmac.h for a description of the available options.
 *
 *****************************************************************************/
LONG FGmacPs_SetOptions (FGmacPs *InstancePtr, u32 Options)
{
    u32 Reg;          /* Generic register contents */
    u32 RegNetCfg;    /* Reflects original contents of NET_CONFIG */
    u32 RegNewNetCfg; /* Reflects new contents of NET_CONFIG */
    LONG Status;
    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Be sure device has been stopped */
    if (InstancePtr->IsStarted == (u32)COMPONENT_IS_STARTED)
    {
        Status = (LONG)(FGMACPS_DEVICE_IS_STARTED);
    }
    else
    {
        /* Many of these options will change the NET_CONFIG registers.
         * To reduce the amount of IO to the device, group these options here
         * and change them all at once.
         */

        /* Grab current register contents */
        RegNetCfg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                    FGMACPS_NWCFG_OFFSET);
        RegNewNetCfg = RegNetCfg;

        /*
         * It is configured to max 1536.
         */
        if ((Options & FGMACPS_FRAME1536_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= (FGMACPS_NWCFG_1536RXEN_MASK);
        }

        /* Turn on VLAN packet only, only VLAN tagged will be accepted */
        if ((Options & FGMACPS_VLAN_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_NVLANDISC_MASK;
        }

        /* Turn on FCS stripping on receive packets */
        if ((Options & FGMACPS_FCS_STRIP_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_FCSREM_MASK;
        }

        /* Turn on length/type field checking on receive packets */
        if ((Options & FGMACPS_LENTYPE_ERR_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_LENERRDSCRD_MASK;
        }

        /* Turn on flow control */
        if ((Options & FGMACPS_FLOW_CONTROL_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_PAUSEEN_MASK;
        }

        /* Turn on promiscuous frame filtering (all frames are received) */
        if ((Options & FGMACPS_PROMISC_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_COPYALLEN_MASK;
        }

        /* Allow broadcast address reception */
        if ((Options & FGMACPS_BROADCAST_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_BCASTDI_MASK);
        }

        /* Allow multicast address filtering */
        if ((Options & FGMACPS_MULTICAST_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_MCASTHASHEN_MASK;
        }

        /* enable RX checksum offload */
        if ((Options & FGMACPS_RX_CHKSUM_ENABLE_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_RXCHKSUMEN_MASK;
        }

        /* Enable jumbo frames */
        if (((Options & FGMACPS_JUMBO_ENABLE_OPTION) != 0x00000000U) &&
            (InstancePtr->Version > 2))
        {
            RegNewNetCfg |= FGMACPS_NWCFG_JUMBO_MASK;
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_JUMBOMAXLEN_OFFSET,
                             FGMACPS_RX_BUF_SIZE_JUMBO);
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_DMACR_OFFSET);
            Reg &= ~FGMACPS_DMACR_RXBUF_MASK;
            Reg |= (((((u32)FGMACPS_RX_BUF_SIZE_JUMBO /
                       (u32)FGMACPS_RX_BUF_UNIT) +
                      (((((u32)FGMACPS_RX_BUF_SIZE_JUMBO %
                          (u32)FGMACPS_RX_BUF_UNIT)) != (u32)0)
                           ? 1U
                           : 0U))
                     << (u32)(FGMACPS_DMACR_RXBUF_SHIFT)) &
                    (u32)(FGMACPS_DMACR_RXBUF_MASK));
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_DMACR_OFFSET, Reg);
            InstancePtr->MaxMtuSize = FGMACPS_MTU_JUMBO;
            InstancePtr->MaxFrameSize = FGMACPS_MTU_JUMBO + FGMACPS_HDR_SIZE +
                                        FGMACPS_TRL_SIZE;
            InstancePtr->MaxVlanFrameSize = InstancePtr->MaxFrameSize +
                                            FGMACPS_HDR_VLAN_SIZE;
            InstancePtr->RxBufMask = FGMACPS_RXBUF_LEN_JUMBO_MASK;
        }

        if (((Options & FGMACPS_SGMII_ENABLE_OPTION) != 0x00000000U) &&
            (InstancePtr->Version > 2))
        {
            RegNewNetCfg |= (FGMACPS_NWCFG_SGMIIEN_MASK |
                             FGMACPS_NWCFG_PCSSEL_MASK);
        }

        /* Officially change the NET_CONFIG registers if it needs to be
         * modified.
         */
        if (RegNetCfg != RegNewNetCfg)
        {
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCFG_OFFSET, RegNewNetCfg);
        }

        /* Enable TX checksum offload */
        if ((Options & FGMACPS_TX_CHKSUM_ENABLE_OPTION) != 0x00000000U)
        {
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_DMACR_OFFSET);
            Reg |= FGMACPS_DMACR_TCPCKSUM_MASK;
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_DMACR_OFFSET, Reg);
        }

        /* Enable transmitter */
        if ((Options & FGMACPS_TRANSMITTER_ENABLE_OPTION) != 0x00000000U)
        {
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_NWCTRL_OFFSET);
            Reg |= FGMACPS_NWCTRL_TXEN_MASK;
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCTRL_OFFSET, Reg);
        }

        /* Enable receiver */
        if ((Options & FGMACPS_RECEIVER_ENABLE_OPTION) != 0x00000000U)
        {
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_NWCTRL_OFFSET);
            Reg |= FGMACPS_NWCTRL_RXEN_MASK;
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCTRL_OFFSET, Reg);
        }

        /* The remaining options not handled here are managed elsewhere in the
         * driver. No register modifications are needed at this time. Reflecting
         * the option in InstancePtr->Options is good enough for now.
         */

        /* Set options word to its new value */
        InstancePtr->Options |= Options;

        Status = (LONG)(FMSH_SUCCESS);
    }
    return Status;
}

/*****************************************************************************/
/**
 * Clear options for the driver/device
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Options are the options to clear. Multiple options can be cleared by
 *        OR'ing FGMACPS_*_OPTIONS constants together. Options not specified
 *        are not affected.
 *
 * @return
 * - FMSH_SUCCESS if the options were set successfully
 * - FGMACPS_DEVICE_IS_STARTED if the device has not yet been stopped
 *
 * @note
 * See fmsh_gmac.h for a description of the available options.
 *
 *****************************************************************************/
LONG FGmacPs_ClearOptions (FGmacPs *InstancePtr, u32 Options)
{
    u32 Reg;          /* Generic */
    u32 RegNetCfg;    /* Reflects original contents of NET_CONFIG */
    u32 RegNewNetCfg; /* Reflects new contents of NET_CONFIG */
    LONG Status;
    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Be sure device has been stopped */
    if (InstancePtr->IsStarted == (u32)COMPONENT_IS_STARTED)
    {
        Status = (LONG)(FGMACPS_DEVICE_IS_STARTED);
    }
    else
    {
        /* Many of these options will change the NET_CONFIG registers.
         * To reduce the amount of IO to the device, group these options here
         * and change them all at once.
         */

        /* Grab current register contents */
        RegNetCfg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                    FGMACPS_NWCFG_OFFSET);
        RegNewNetCfg = RegNetCfg;

        /* There is only RX configuration!?
         * It is configured in two different length, upto 1536 and 10240 bytes
         */
        if ((Options & FGMACPS_FRAME1536_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_1536RXEN_MASK);
        }

        /* Turn off VLAN packet only */
        if ((Options & FGMACPS_VLAN_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_NVLANDISC_MASK);
        }

        /* Turn off FCS stripping on receive packets */
        if ((Options & FGMACPS_FCS_STRIP_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_FCSREM_MASK);
        }

        /* Turn off length/type field checking on receive packets */
        if ((Options & FGMACPS_LENTYPE_ERR_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_LENERRDSCRD_MASK);
        }

        /* Turn off flow control */
        if ((Options & FGMACPS_FLOW_CONTROL_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_PAUSEEN_MASK);
        }

        /* Turn off promiscuous frame filtering (all frames are received) */
        if ((Options & FGMACPS_PROMISC_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_COPYALLEN_MASK);
        }

        /* Disallow broadcast address filtering => broadcast reception */
        if ((Options & FGMACPS_BROADCAST_OPTION) != 0x00000000U)
        {
            RegNewNetCfg |= FGMACPS_NWCFG_BCASTDI_MASK;
        }

        /* Disallow multicast address filtering */
        if ((Options & FGMACPS_MULTICAST_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_MCASTHASHEN_MASK);
        }

        /* Disable RX checksum offload */
        if ((Options & FGMACPS_RX_CHKSUM_ENABLE_OPTION) != 0x00000000U)
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_RXCHKSUMEN_MASK);
        }

        /* Disable jumbo frames */
        if (((Options & FGMACPS_JUMBO_ENABLE_OPTION) != 0x00000000U) &&
            (InstancePtr->Version > 2))
        {
            RegNewNetCfg &= (u32)(~FGMACPS_NWCFG_JUMBO_MASK);
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_DMACR_OFFSET);
            Reg &= ~FGMACPS_DMACR_RXBUF_MASK;
            Reg |= (((((u32)FGMACPS_RX_BUF_SIZE / (u32)FGMACPS_RX_BUF_UNIT) +
                      (((((u32)FGMACPS_RX_BUF_SIZE %
                          (u32)FGMACPS_RX_BUF_UNIT)) != (u32)0)
                           ? 1U
                           : 0U))
                     << (u32)(FGMACPS_DMACR_RXBUF_SHIFT)) &
                    (u32)(FGMACPS_DMACR_RXBUF_MASK));
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_DMACR_OFFSET, Reg);
            InstancePtr->MaxMtuSize = FGMACPS_MTU;
            InstancePtr->MaxFrameSize = FGMACPS_MTU + FGMACPS_HDR_SIZE +
                                        FGMACPS_TRL_SIZE;
            InstancePtr->MaxVlanFrameSize = InstancePtr->MaxFrameSize +
                                            FGMACPS_HDR_VLAN_SIZE;
            InstancePtr->RxBufMask = FGMACPS_RXBUF_LEN_MASK;
        }

        if (((Options & FGMACPS_SGMII_ENABLE_OPTION) != 0x00000000U) &&
            (InstancePtr->Version > 2))
        {
            RegNewNetCfg &= (u32)(~(FGMACPS_NWCFG_SGMIIEN_MASK |
                                    FGMACPS_NWCFG_PCSSEL_MASK));
        }

        /* Officially change the NET_CONFIG registers if it needs to be
         * modified.
         */
        if (RegNetCfg != RegNewNetCfg)
        {
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCFG_OFFSET, RegNewNetCfg);
        }

        /* Disable TX checksum offload */
        if ((Options & FGMACPS_TX_CHKSUM_ENABLE_OPTION) != 0x00000000U)
        {
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_DMACR_OFFSET);
            Reg &= (u32)(~FGMACPS_DMACR_TCPCKSUM_MASK);
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_DMACR_OFFSET, Reg);
        }

        /* Disable transmitter */
        if ((Options & FGMACPS_TRANSMITTER_ENABLE_OPTION) != 0x00000000U)
        {
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_NWCTRL_OFFSET);
            Reg &= (u32)(~FGMACPS_NWCTRL_TXEN_MASK);
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCTRL_OFFSET, Reg);
        }

        /* Disable receiver */
        if ((Options & FGMACPS_RECEIVER_ENABLE_OPTION) != 0x00000000U)
        {
            Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                  FGMACPS_NWCTRL_OFFSET);
            Reg &= (u32)(~FGMACPS_NWCTRL_RXEN_MASK);
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCTRL_OFFSET, Reg);
        }

        /* The remaining options not handled here are managed elsewhere in the
         * driver. No register modifications are needed at this time. Reflecting
         * option in InstancePtr->Options is good enough for now.
         */

        /* Set options word to its new value */
        InstancePtr->Options &= ~Options;

        Status = (LONG)(FMSH_SUCCESS);
    }
    return Status;
}

/*****************************************************************************/
/**
 * Get current option settings
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * A bitmask of XTE_*_OPTION constants. Any bit set to 1 is to be interpreted
 * as a set opion.
 *
 * @note
 * See fmsh_gmac.h for a description of the available options.
 *
 *****************************************************************************/
u32 FGmacPs_GetOptions (FGmacPs *InstancePtr)
{
    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    return (InstancePtr->Options);
}

/*****************************************************************************/
/**
 * Send a pause packet
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if pause frame transmission was initiated
 * - FGMACPS_DEVICE_IS_STOPPED if the device has not been started.
 *
 *****************************************************************************/
LONG FGmacPs_SendPausePacket (FGmacPs *InstancePtr)
{
    u32 Reg;
    LONG Status;

    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* Make sure device is ready for this operation */
    if (InstancePtr->IsStarted != (u32)COMPONENT_IS_STARTED)
    {
        Status = (LONG)(FGMACPS_DEVICE_IS_STOPPED);
    }
    else
    {
        /* Send flow control frame */
        Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                              FGMACPS_NWCTRL_OFFSET);
        Reg |= FGMACPS_NWCTRL_PAUSETX_MASK;
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCTRL_OFFSET,
                         Reg);
        Status = (LONG)(FMSH_SUCCESS);
    }
    return Status;
}

/*****************************************************************************/
/**
 * FGmacPs_GetOperatingSpeed gets the current operating link speed. This may
 * be the value set by FGmacPs_SetOperatingSpeed() or a hardware default.
 *
 * @param InstancePtr references the TEMAC channel on which to operate.
 *
 * @return FGmacPs_GetOperatingSpeed returns the link speed in units of
 *         megabits per second.
 *
 * @note
 *
 *****************************************************************************/
u16 FGmacPs_GetOperatingSpeed (FGmacPs *InstancePtr)
{
    u32 Reg;
    u16 Status;

    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                          FGMACPS_NWCFG_OFFSET);

    if ((Reg & FGMACPS_NWCFG_1000_MASK) != 0x00000000U)
    {
        Status = (u16)(1000);
    }
    else
    {
        if ((Reg & FGMACPS_NWCFG_100_MASK) != 0x00000000U)
        {
            Status = (u16)(100);
        }
        else
        {
            Status = (u16)(10);
        }
    }
    return Status;
}

/*****************************************************************************/
/**
 * FGmacPs_SetOperatingSpeed sets the current operating link speed. For any
 * traffic to be passed, this speed must match the current MII/GMII/SGMII/RGMII
 * link speed.
 *
 * @param InstancePtr references the TEMAC channel on which to operate.
 * @param Speed is the speed to set in units of Mbps. Valid values are 10, 100,
 *        or 1000. FGmacPs_SetOperatingSpeed ignores invalid values.
 *
 * @note
 *
 *****************************************************************************/
void FGmacPs_SetOperatingSpeed (FGmacPs *InstancePtr, u16 Speed)
{
    u32 Reg;
    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);
    // FGmacPs_AssertVoid((Speed == (u16)10) || (Speed == (u16)100) || (Speed ==
    // (u16)1000));

    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                          FGMACPS_NWCFG_OFFSET);
    Reg &= (u32)(~(FGMACPS_NWCFG_1000_MASK | FGMACPS_NWCFG_100_MASK));

    switch (Speed)
    {
    case (u16)10:
        break;

    case (u16)100:
        Reg |= FGMACPS_NWCFG_100_MASK;
        break;

    case (u16)1000:
        Reg |= FGMACPS_NWCFG_1000_MASK;
        break;
    }

    /* Set register and return */
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_NWCFG_OFFSET,
                     Reg);
}

/*****************************************************************************/
/**
 * API to update the Burst length in the DMACR register.
 *
 * @param InstancePtr is a pointer to the FGmacPs instance to be worked on.
 * @param BLength is the length in bytes for the dma burst.
 *
 * @return None
 *
 ******************************************************************************/
void FGmacPs_DMABLengthUpdate (FGmacPs *InstancePtr, s32 BLength)
{
    u32 Reg;
    u32 RegUpdateVal = 0;

    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid((BLength == FGMACPS_SINGLE_BURST) ||
    //				(BLength == FGMACPS_4BYTE_BURST) ||
    //				(BLength == FGMACPS_8BYTE_BURST) ||
    //				(BLength == FGMACPS_16BYTE_BURST));

    switch (BLength)
    {
    case FGMACPS_SINGLE_BURST:
        RegUpdateVal = FGMACPS_DMACR_SINGLE_AHB_BURST;
        break;

    case FGMACPS_4BYTE_BURST:
        RegUpdateVal = FGMACPS_DMACR_INCR4_AHB_BURST;
        break;

    case FGMACPS_8BYTE_BURST:
        RegUpdateVal = FGMACPS_DMACR_INCR8_AHB_BURST;
        break;

    case FGMACPS_16BYTE_BURST:
        RegUpdateVal = FGMACPS_DMACR_INCR16_AHB_BURST;
        break;
    }
    Reg = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                          FGMACPS_DMACR_OFFSET);

    Reg &= (u32)(~FGMACPS_DMACR_BLENGTH_MASK);
    Reg |= RegUpdateVal;
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_DMACR_OFFSET,
                     Reg);
}

/*****************************************************************************/
/**
 * Install an asynchronious handler function for the given HandlerType:
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param HandlerType indicates what interrupt handler type is.
 *        FGMACPS_HANDLER_DMASEND, FGMACPS_HANDLER_DMARECV and
 *        FGMACPS_HANDLER_ERROR.
 * @param FuncPointer is the pointer to the callback function
 * @param CallBackRef is the upper layer callback reference passed back when
 *        when the callback function is invoked.
 *
 * @return
 *
 * None.
 *
 * @note
 * There is no assert on the CallBackRef since the driver doesn't know what
 * it is.
 *
 *****************************************************************************/
LONG FGmacPs_SetHandler (FGmacPs *InstancePtr, u32 HandlerType,
                         void *FuncPointer, void *CallBackRef)
{
    LONG Status;
    // FGmacPs_AssertNonvoid(InstancePtr != NULL);
    // FGmacPs_AssertNonvoid(FuncPointer != NULL);
    // FGmacPs_AssertNonvoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    switch (HandlerType)
    {
    case FGMACPS_HANDLER_DMASEND:
        Status = (LONG)(FMSH_SUCCESS);
        InstancePtr->SendHandler = ((FGmacPs_Handler)(void *)FuncPointer);
        InstancePtr->SendRef = CallBackRef;
        break;
    case FGMACPS_HANDLER_DMARECV:
        Status = (LONG)(FMSH_SUCCESS);
        InstancePtr->RecvHandler = ((FGmacPs_Handler)(void *)FuncPointer);
        InstancePtr->RecvRef = CallBackRef;
        break;
    case FGMACPS_HANDLER_ERROR:
        Status = (LONG)(FMSH_SUCCESS);
        InstancePtr->ErrorHandler = ((FGmacPs_ErrHandler)(void *)FuncPointer);
        InstancePtr->ErrorRef = CallBackRef;
        break;
    default:
        Status = (LONG)(FGMACPS_INVALID_PARAM);
        break;
    }
    return Status;
}

/*****************************************************************************/
/**
 * Master interrupt handler for EMAC driver. This routine will query the
 * status of the device, bump statistics, and invoke user callbacks.
 *
 * This routine must be connected to an interrupt controller using OS/BSP
 * specific methods.
 *
 * @param FGmacPsPtr is a pointer to the FGMACPS instance that has caused the
 *        interrupt.
 *
 ******************************************************************************/
void FGmacPs_IntrHandler (void *FGmacPsPtr)
{
    u32 RegISR;
    u32 RegSR;
    u32 RegCtrl;
    u32 RegQ1ISR = 0U;
    FGmacPs *InstancePtr = (FGmacPs *)FGmacPsPtr;

    // FGmacPs_AssertVoid(InstancePtr != NULL);
    // FGmacPs_AssertVoid(InstancePtr->IsReady == (u32)COMPONENT_IS_READY);

    /* This ISR will try to handle as many interrupts as it can in a single
     * call. However, in most of the places where the user's error handler
     * is called, this ISR exits because it is expected that the user will
     * reset the device in nearly all instances.
     */
    RegISR = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_ISR_OFFSET);

    /* Read Transmit Q1 ISR */

    if (InstancePtr->Version > 2)
    {
        RegQ1ISR = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                   FGMACPS_INTQ1_STS_OFFSET);
    }

    /* Clear the interrupt status register */
    FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_ISR_OFFSET,
                     RegISR);

    /* Receive complete interrupt */
    if ((RegISR & FGMACPS_IXR_FRAMERX_MASK) != 0x00000000U)
    {
        /* Clear RX status register RX complete indication but preserve
         * error bits if there is any */
        FGmacPs_WriteReg(
            InstancePtr->Config.BaseAddress, FGMACPS_RXSR_OFFSET,
            ((u32)FGMACPS_RXSR_FRAMERX_MASK | (u32)FGMACPS_RXSR_BUFFNA_MASK));
        InstancePtr->RecvHandler(InstancePtr->RecvRef);
    }

    /* Transmit Q1 complete interrupt */
    if ((InstancePtr->Version > 2) &&
        ((RegQ1ISR & FGMACPS_INTQ1SR_TXCOMPL_MASK) != 0x00000000U))
    {
        /* Clear TX status register TX complete indication but preserve
         * error bits if there is any */
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_INTQ1_STS_OFFSET,
                         FGMACPS_INTQ1SR_TXCOMPL_MASK);
        FGmacPs_WriteReg(
            InstancePtr->Config.BaseAddress, FGMACPS_TXSR_OFFSET,
            ((u32)FGMACPS_TXSR_TXCOMPL_MASK | (u32)FGMACPS_TXSR_USEDREAD_MASK));
        InstancePtr->SendHandler(InstancePtr->SendRef);
    }

    /* Transmit complete interrupt */
    if ((RegISR & FGMACPS_IXR_TXCOMPL_MASK) != 0x00000000U)
    {
        /* Clear TX status register TX complete indication but preserve
         * error bits if there is any */
        FGmacPs_WriteReg(
            InstancePtr->Config.BaseAddress, FGMACPS_TXSR_OFFSET,
            ((u32)FGMACPS_TXSR_TXCOMPL_MASK | (u32)FGMACPS_TXSR_USEDREAD_MASK));
        InstancePtr->SendHandler(InstancePtr->SendRef);
    }

    /* Receive error conditions interrupt */
    if ((RegISR & FGMACPS_IXR_RX_ERR_MASK) != 0x00000000U)
    {
        /* Clear RX status register */
        RegSR = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                FGMACPS_RXSR_OFFSET);
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_RXSR_OFFSET,
                         RegSR);

        /* Fix for CR # 692702. Write to bit 18 of net_ctrl
         * register to flush a packet out of Rx SRAM upon
         * an error for receive buffer not available. */
        if ((RegISR & FGMACPS_IXR_RXUSED_MASK) != 0x00000000U)
        {
            RegCtrl = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                      FGMACPS_NWCTRL_OFFSET);
            RegCtrl |= (u32)FGMACPS_NWCTRL_FLUSH_DPRAM_MASK;
            FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                             FGMACPS_NWCTRL_OFFSET, RegCtrl);
        }

        if (RegSR != 0)
        {
            InstancePtr->ErrorHandler(InstancePtr->ErrorRef, FGMACPS_RECV,
                                      RegSR);
        }
    }

    /* When FGMACPS_IXR_TXCOMPL_MASK is flaged, FGMACPS_IXR_TXUSED_MASK
     * will be asserted the same time.
     * Have to distinguish this bit to handle the real error condition.
     */
    /* Transmit Q1 error conditions interrupt */
    if ((InstancePtr->Version > 2) &&
        ((RegQ1ISR & FGMACPS_INTQ1SR_TXERR_MASK) != 0x00000000U) &&
        ((RegQ1ISR & FGMACPS_INTQ1SR_TXCOMPL_MASK) != 0x00000000U))
    {
        /* Clear Interrupt Q1 status register */
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress,
                         FGMACPS_INTQ1_STS_OFFSET, RegQ1ISR);
        InstancePtr->ErrorHandler(InstancePtr->ErrorRef, FGMACPS_SEND,
                                  RegQ1ISR);
    }

    /* Transmit error conditions interrupt */
    if (((RegISR & FGMACPS_IXR_TX_ERR_MASK) != 0x00000000U) &&
        (!(RegISR & FGMACPS_IXR_TXCOMPL_MASK) != 0x00000000U))
    {
        /* Clear TX status register */
        RegSR = FGmacPs_ReadReg(InstancePtr->Config.BaseAddress,
                                FGMACPS_TXSR_OFFSET);
        FGmacPs_WriteReg(InstancePtr->Config.BaseAddress, FGMACPS_TXSR_OFFSET,
                         RegSR);
        InstancePtr->ErrorHandler(InstancePtr->ErrorRef, FGMACPS_SEND, RegSR);
    }
}
