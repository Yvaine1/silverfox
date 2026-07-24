/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_example.c
 *
 * gmac phyloop example
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 1_0   Danyang Wang  6/25/2024  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>

#include "fmsh_gmac_example.h"

// #include "fmsh_gmac_verify_lib.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"
#include "fmsh_psu_parameters.h"

// #include "fmsh_cache.h"
#include "fmsh_common.h"
#include "fmsh_gmac_mdio.h"
#include "marvell_88e1512.h"
#include "microchip_ksz9031RNX.h"

// EC2203N
#include "fmsh_gmac.h"
#include "fmsh_gmac_bd.h"
#include "fmsh_gmac_hw.h"

// GTR
// #include "gtr_reg.h"

/************************** Constant Definitions *****************************/

#define JUMBO_FRAME_SIZE 10240
#define FRAME_HDR_SIZE   18
#define RXBD_CNT         32 /* Number of RxBDs to use */
#define TXBD_CNT         32 /* Number of TxBDs to use */
char FGMACPS_MACADDR[] = {0x00, 0x0a, 0x35, 0x01, 0x02, 0x03};

/************************** Variable Definitions *****************************/
volatile s32 FramesRx;     /* Frames have been received */
volatile s32 FramesTx;     /* Frames have been sent */
volatile s32 DeviceErrors; /* Number of errors detected in the device */

__attribute__((
    section(".txframe"))) EthernetFrame TxFrame; /* Transmit buffer */
__attribute__((section(".rxframe"))) EthernetFrame RxFrame; /* Receive buffer */

u32 TxFrameLength;

u8 GTR_lane[4] = {5, 5, 5, 5};
u8 GTR_dp_speed = 0;
double GTR_refclk[2] = {100, 100};
u8 GTR_ssc_en[2] = {0, 0};

FGmacPs GmacPsInstance;

FGmacPs_PhyConfig PhyCfg = {
    .phy_device = PHY_KSZ9031RNX,  // PHY_KSZ9031RNX   PHY_88E1116R  PHY_88E1111
    .auto_detect_ad_en = 1,
    .phy_address = 0,
    .auto_nag_en = 1,
};

#if defined __aarch64__

u8 bd_space[0x10000];
#else

__attribute__((section(".bdspace"))) u8 bd_space[0x8000];
#endif

FGmacPs_Bd BdTxTerminate __attribute__((aligned(64)));

FGmacPs_Bd BdRxTerminate __attribute__((aligned(64)));
;

FGmacPs_Bd BdTemplate;

/************************* irq handler function ******************************/

/****************************************************************************/
/**
 *
 * This the Transmit handler callback function and will increment a shared
 * counter that can be shared by the main thread of operation.
 *
 * @param	Callback is the pointer to the instance of the GmacPs device.
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
void gmac_interrupt_handler (void)
{
    // printf("1\n");
    FGmacPs_IntrHandler(&GmacPsInstance);
}

/****************************************************************************/
/**
 *
 * This the Transmit handler callback function and will increment a shared
 * counter that can be shared by the main thread of operation.
 *
 * @param	Callback is the pointer to the instance of the GmacPs device.
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
static void FGmacPsSendHandler (void *Callback)
{
    FGmacPs *InstancePtr = (FGmacPs *)Callback;

    /*
     * Disable the transmit related interrupts
     */
    FGmacPs_IntDisable(InstancePtr,
                       (FGMACPS_IXR_TXCOMPL_MASK | FGMACPS_IXR_TX_ERR_MASK));
    FGmacPs_IntQ1Disable(InstancePtr, FGMACPS_INTQ1_IXR_ALL_MASK);
    /*
     * Increment the counter so that main thread knows something
     * happened.
     */
    FramesTx++;
}

/****************************************************************************/
/**
 *
 * This is the Receive handler callback function and will increment a shared
 * counter that can be shared by the main thread of operation.
 *
 * @param	Callback is a pointer to the instance of the GmacPs device.
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
static void FGmacPsRecvHandler (void *Callback)
{
    FGmacPs *InstancePtr = (FGmacPs *)Callback;

    /*
     * Disable the transmit related interrupts
     */
    FGmacPs_IntDisable(InstancePtr,
                       (FGMACPS_IXR_FRAMERX_MASK | FGMACPS_IXR_RX_ERR_MASK));
    /*
     * Increment the counter so that main thread knows something
     * happened.
     */
    FramesRx++;

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheInvalidateRange((UINTPTR)&RxFrame, sizeof(EthernetFrame));
    Fmsh_DCacheInvalidateRange((UINTPTR)RxBdSpacePtr, 64);
#endif
}

/****************************************************************************/
/**
 *
 * This is the Error handler callback function and this function increments
 * the error counter so that the main thread knows the number of errors.
 *
 * @param	Callback is the callback function for the driver. This
 *		parameter is not used in this example.
 * @param	Direction is passed in from the driver specifying which
 *		direction error has occurred.
 * @param	ErrorWord is the status register value passed in.
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
static void FGmacPsErrorHandler (void *Callback, u8 Direction, u32 ErrorWord)
{
#if GMAC_DEBUG_RESET_ON_ERR
    FGmacPs *InstancePtr = (FGmacPs *)Callback;
#endif
    /*
     * Increment the counter so that main thread knows something
     * happened. Reset the device and reallocate resources ...
     */
    DeviceErrors++;

    switch (Direction)
    {
    case FGMACPS_RECV:
        if (ErrorWord & FGMACPS_RXSR_HRESPNOK_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Receive DMA error");
        }
        if (ErrorWord & FGMACPS_RXSR_RXOVR_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Receive over run");
        }
        if (ErrorWord & FGMACPS_RXSR_BUFFNA_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Receive buffer not available");
        }
        break;
    case FGMACPS_SEND:
        if (ErrorWord & FGMACPS_TXSR_HRESPNOK_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit DMA error");
        }
        if (ErrorWord & FGMACPS_TXSR_URUN_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit under run");
        }
        if (ErrorWord & FGMACPS_TXSR_BUFEXH_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer exhausted");
        }
        if (ErrorWord & FGMACPS_TXSR_RXOVR_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit retry excessed limits");
        }
        if (ErrorWord & FGMACPS_TXSR_FRAMERX_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit collision");
        }
        if (ErrorWord & FGMACPS_TXSR_USEDREAD_MASK)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer not available");
        }
        break;
    }
    /*
     * Bypassing the reset functionality as the default tx status for q0 is
     * USED BIT READ. so, the first interrupt will be tx used bit and it resets
     * the core always.
     */
#if GMAC_DEBUG_RESET_ON_ERR
    GmacPsResetDevice(InstancePtr);
#endif
}

/*************************** initial function ********************************/

/*****************************************************************************/
/**
 * Initialize a specific FGmacPs instance/driver.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/

int fmsh_gmac_verify_device_initial (FGmacPs *InstancePtr)
{
    int Status;
    FGmacPs_Config *ConfigPtr;

    ConfigPtr = FGmacPs_LookupConfig(GMAC_SELECT_ID);

    Status = FGmacPs_CfgInitialize(InstancePtr, ConfigPtr,
                                   ConfigPtr->BaseAddress);

    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error in cfg initialize");
        return FMSH_FAILURE;
    }

    /* Enable jumbo frames for zynqmp */
    FGmacPs_SetOptions(InstancePtr, FGMACPS_JUMBO_ENABLE_OPTION);
    // FGmacPsClkSetup(InstancePtr, GmacPsIntrId);

    /*
     * Set the MAC address
     */
    Status = FGmacPs_SetMacAddress(InstancePtr, FGMACPS_MACADDR, 1);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error setting MAC address");
        return FMSH_FAILURE;
    }
    /*
     * Setup callbacks
     */
    Status = FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMASEND,
                                (void *)FGmacPsSendHandler, InstancePtr);
    Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMARECV,
                                 (void *)FGmacPsRecvHandler, InstancePtr);
    Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_ERROR,
                                 (void *)FGmacPsErrorHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error assigning handlers");
        return FMSH_FAILURE;
    }

    return Status;
}

/*****************************************************************************/
/**
 * Initialize phy for a specific FGmacPs instance.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/

int fmsh_gmac_verify_phy_initial (FGmacPs *InstancePtr,
                                  FGmacPs_PhyConfig *PhyCfgPtr)
{
    int speed = speed_1000;
    u16 PhyAddr = 0;
    int Status;

    FGmacPs_SetMdioDivisor(InstancePtr, MDC_DIV_224);
    delay_ms(1000);
    /* detect phy */
    if (PhyCfgPtr->auto_detect_ad_en == 1)
    {
        PhyAddr = FGmacPs_PHYDetect(InstancePtr);
        PhyCfgPtr->phy_address = PhyAddr;
    }
    PhyCfgPtr->speed = InstancePtr->Config.Speed;

    /* set phy address & phy device */
    switch (InstancePtr->Config.BaseAddress)
    {
    case FPS_GMAC0_BASEADDR:
    {
        PhyCfgPtr->phy_address = 0;
        PhyCfgPtr->phy_device = PHY_88E1512;
        break;
    }
    case FPS_GMAC1_BASEADDR:
    {
        PhyCfgPtr->phy_address = 1;
        PhyCfgPtr->phy_device = PHY_88E1512;
        break;
    }
    case FPS_GMAC2_BASEADDR:
    {
        PhyCfgPtr->phy_address = 4;
        PhyCfgPtr->phy_device = PHY_YT8521;
        break;
    }
    case FPS_GMAC3_BASEADDR:
    {
        PhyCfgPtr->phy_address = 6;
        PhyCfgPtr->phy_device = PHY_YT8521;
        break;
    }
    }

    /* operate phy Init */
    Status = FGmacPs_PHYInit(InstancePtr, PhyCfgPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "PHY init fail\r\n");
    }
    else
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "PHY init success, Address = %x, type = %d\r\n",
                       PhyCfgPtr->phy_address, PhyCfgPtr->phy_device);
    }

    /* get phy operating speed if autoneg is on */
    if (PhyCfgPtr->auto_nag_en == 1)
    {
        speed = PhyCfgPtr->speed;
    }
    else
    {
        speed = PhyCfgPtr->speed;
    }

    /* set operating speed */
    FGmacPs_SetOperatingSpeed(InstancePtr, speed);

    return FMSH_SUCCESS;
}

/*****************************************************************************/
/**
 * Initialize a specific FGmacPs Gic instance.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/
int fmsh_gmac_verify_gic_setup (FGmacPs *InstancePtr)
{
    u32 int_id = 0x59U;
    // u32 Status;
    switch (InstancePtr->Config.DeviceId)
    {
    case FPAR_GMACPS_0_DEVICE_ID:
        int_id = 0x59U;
        break;
    case FPAR_GMACPS_1_DEVICE_ID:
        int_id = 0x5BU;
        break;
    case FPAR_GMACPS_2_DEVICE_ID:
        int_id = 0x5DU;
        break;
    case FPAR_GMACPS_3_DEVICE_ID:
        int_id = 0x5FU;
        break;
    default:
        int_id = 0x59U;
        break;
    }

    // Status = FGicPs_SetupInterruptSystem(&IntcInstance);
    // if(Status != GIC_SUCCESS)
    //   GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "FGicPs_SetupInterruptSystem fail\r\n");

    FGicPs_Connect(&IntcInstance, int_id,
                   (FMSH_InterruptHandler)gmac_interrupt_handler, 0);
    FMSH_ExceptionRegisterHandler(
        FMSH_EXCEPTION_ID_FIQ_INT,
        (FMSH_ExceptionHandler)FGicPs_InterruptHandler_FIQ, &IntcInstance);
    FGicPs_Enable(&IntcInstance, int_id);

    return 0;
}

/*****************************************************************************/
/**
 * Stop a specific FGmacPs Gic instance.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/
int fmsh_gmac_verify_gic_stop (FGmacPs *InstancePtr)
{
    u32 int_id;

    switch (InstancePtr->Config.DeviceId)
    {
    case FPAR_GMACPS_0_DEVICE_ID:
        int_id = 0x59U;
    case FPAR_GMACPS_1_DEVICE_ID:
        int_id = 0x5BU;
    case FPAR_GMACPS_2_DEVICE_ID:
        int_id = 0x5DU;
    case FPAR_GMACPS_3_DEVICE_ID:
        int_id = 0x5FU;
    }

    FGicPs_Disconnect(&IntcInstance, int_id);

    return 0;
}

/***************************** frame function ********************************/

/****************************************************************************/
/**
 *
 * Set the MAC addresses in the frame.
 *
 * @param    FramePtr is the pointer to the frame.
 * @param    DestAddr is the Destination MAC address.
 *
 * @return   None.
 *
 * @note     None.
 *
 *****************************************************************************/
void FGmacPs_FrameFormatMAC (EthernetFrame *FramePtr, char *DestAddr)
{
    char *Frame = (char *)FramePtr;
    char *SourceAddress = FGMACPS_MACADDR;
    s32 Index;
    /* Destination address */
    for (Index = 0; Index < FGMACPS_MAC_ADDR_SIZE; Index++)
    {
        *Frame++ = *DestAddr++;
    }

    /* Source address */
    for (Index = 0; Index < FGMACPS_MAC_ADDR_SIZE; Index++)
    {
        *Frame++ = *SourceAddress++;
    }
}

/****************************************************************************/
/**
 *
 * Set the frame type for the specified frame.
 *
 * @param    FramePtr is the pointer to the frame.
 * @param    FrameType is the Type to set in frame.
 *
 * @return   None.
 *
 * @note     None.
 *
 *****************************************************************************/
void FGmacPs_FrameFormatType (EthernetFrame *FramePtr, u16 FrameType)
{
    char *Frame = (char *)FramePtr;

    /*
     * Increment to type field
     */
    Frame = Frame + 12;
    /*
     * Do endian swap from little to big-endian.
     */
    FrameType = FGmacPsEndianSwap16(FrameType);
    /*
     * Set the type
     */
    *(u16 *)Frame = FrameType;
}

/****************************************************************************/
/**
 * This function places a pattern in the payload section of a frame. The pattern
 * is a  8 bit incrementing series of numbers starting with 0.
 * Once the pattern reaches 256, then the pattern changes to a 16 bit
 * incrementing pattern:
 * <pre>
 *   0, 1, 2, ... 254, 255, 00, 00, 00, 01, 00, 02, ...
 * </pre>
 *
 * @param    FramePtr is a pointer to the frame to change.
 * @param    PayloadSize is the number of bytes in the payload that will be set.
 *
 * @return   None.
 *
 * @note     None.
 *
 *****************************************************************************/
void FGmacPs_FrameSetPayloadData (EthernetFrame *FramePtr, u32 PayloadSize)
{
    u32 BytesLeft = PayloadSize;
    u8 *Frame;
    u16 Counter = 0;

    /*
     * Set the frame pointer to the start of the payload area
     */
    Frame = (u8 *)FramePtr + FGMACPS_HDR_SIZE;

    /*
     * Insert 8 bit incrementing pattern
     */
    while (BytesLeft && (Counter < 256))
    {
        *Frame++ = (u8)Counter++;
        BytesLeft--;
    }

    /*
     * Switch to 16 bit incrementing pattern
     */
    while (BytesLeft)
    {
        *Frame++ = (u8)(Counter >> 8); /* high */
        BytesLeft--;

        if (!BytesLeft)
        {
            break;
        }

        *Frame++ = (u8)Counter++; /* low */
        BytesLeft--;
    }
}

/****************************************************************************/
/**
 * This function verifies the frame data against a CheckFrame.
 *
 * Validation occurs by comparing the ActualFrame to the header of the
 * CheckFrame. If the headers match, then the payload of ActualFrame is
 * verified for the same pattern Util_FrameSetPayloadData() generates.
 *
 * @param    CheckFrame is a pointer to a frame containing the 14 byte header
 *           that should be present in the ActualFrame parameter.
 * @param    ActualFrame is a pointer to a frame to validate.
 *
 * @return   FMSH_SUCCESS if successful, else FMSH_FAILURE.
 *
 * @note     None.
 *****************************************************************************/
LONG fmsh_gmac_verify_FrameVerify (EthernetFrame *CheckFrame,
                                   EthernetFrame *ActualFrame)
{
    char *CheckPtr = (char *)CheckFrame;
    char *ActualPtr = (char *)ActualFrame;
    u16 BytesLeft;
    u16 Counter;
    u32 Index;

    /*
     * Compare the headers
     */
    for (Index = 0; Index < FGMACPS_HDR_SIZE; Index++)
    {
        if (CheckPtr[Index] != ActualPtr[Index])
        {
            return FMSH_FAILURE;
        }
    }

    /*
     * Get the length of the payload
     */
    BytesLeft = *(u16 *)&ActualPtr[12];
    /*
     * Do endian swap from big back to little-endian.
     */
    BytesLeft = FGmacPsEndianSwap16(BytesLeft);
    /*
     * Validate the payload
     */
    Counter = 0;
    ActualPtr = &ActualPtr[14];

    /*
     * Check 8 bit incrementing pattern
     */
    while (BytesLeft && (Counter < 256))
    {
        if (*ActualPtr++ != (char)Counter++)
        {
            return FMSH_FAILURE;
        }
        BytesLeft--;
    }

    /*
     * Check 16 bit incrementing pattern
     */
    while (BytesLeft)
    {
        if (*ActualPtr++ != (char)(Counter >> 8))
        { /* high */
            return FMSH_FAILURE;
        }

        BytesLeft--;

        if (!BytesLeft)
        {
            break;
        }

        if (*ActualPtr++ != (char)Counter++)
        { /* low */
            return FMSH_FAILURE;
        }

        BytesLeft--;
    }

    return FMSH_SUCCESS;
}

/****************************************************************************/
/**
 * This function sets all bytes of a frame to 0.
 *
 * @param    FramePtr is a pointer to the frame itself.
 *
 * @return   None.
 *
 * @note     None.
 *
 *****************************************************************************/
void fmsh_gmac_verify_FrameMemClear (EthernetFrame *FramePtr)
{
    u32 *Data32Ptr = (u32 *)FramePtr;
    u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);

    /* frame should be an integral number of words */
    while (WordsLeft--)
    {
        *Data32Ptr++ = 0xDEADBEEF;
    }
}

/***************************** test function *********************************/

/*****************************************************************************/
/**
 * Gmac phy loopback test.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/
int fmsh_gmac_phyloop_example (FGmacPs *InstancePtr,
                               FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u32 PayloadSize = 1000;
    u32 NumRxBuf = 0;
    u32 RxFrLen;
    FGmacPs_Bd *BdTxPtr;
    FGmacPs_Bd *BdRxPtr;

    GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                   "Entering into fmsh gmac verify phyloop_test\r\n");

    /*
     * Set up phy into loop mode
     */

    Status = FGmacPs_PHYloopback(InstancePtr, PhyCfgPtr);

    /*
     * Clear variables shared with callbacks
     */
    FramesRx = 0;
    FramesTx = 0;
    DeviceErrors = 0;
    PayloadSize = (JUMBO_FRAME_SIZE - FRAME_HDR_SIZE);
    /*
     * Calculate the frame length (not including FCS)
     */
    TxFrameLength = FGMACPS_HDR_SIZE + PayloadSize;

    /*
     * Setup packet to be transmitted
     */
    FGmacPs_FrameFormatMAC(&TxFrame, FGMACPS_MACADDR);
    FGmacPs_FrameFormatType(&TxFrame, PayloadSize);
    FGmacPs_FrameSetPayloadData(&TxFrame, PayloadSize);

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)&TxFrame, sizeof(EthernetFrame));
#endif

    /*
     * Clear out receive packet memory area
     */
    fmsh_gmac_verify_FrameMemClear(&RxFrame);

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)&RxFrame, sizeof(EthernetFrame));
#endif

    /*
     * Allocate RxBDs since we do not know how many BDs will be used
     * in advance, use RXBD_CNT here.
     */
    Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetRxRing(InstancePtr)), 1,
                                 &BdRxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error allocating RxBD");
        return FMSH_FAILURE;
    }

    /*
     * Setup the BD. The FGmacPs_BdRingClone() call will mark the
     * "wrap" field for last RxBD. Setup buffer address to associated
     * BD.
     */

    FGmacPs_BdSetAddressRx(BdRxPtr, (UINTPTR)&RxFrame);

    /*
     * Enqueue to HW
     */
    Status = FGmacPs_BdRingToHw(&(FGmacPs_GetRxRing(InstancePtr)), 1, BdRxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error committing RxBD to HW");
        return FMSH_FAILURE;
    }
    /*
     * Though the max BD size is 16 bytes for extended desc mode, performing
     * cache flush for 64 bytes. which is equal to the cache line size.
     */

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)BdRxPtr, 64);
#endif

    /*
     * Allocate, setup, and enqueue 1 TxBDs. The first BD will
     * describe the first 32 bytes of TxFrame and the rest of BDs
     * will describe the rest of the frame.
     *
     * The function below will allocate 1 adjacent BDs with BdTxPtr
     * being set as the lead BD.
     */
    Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetTxRing(InstancePtr)), 1,
                                 &BdTxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error allocating TxBD");
        return FMSH_FAILURE;
    }

    /*
     * Setup first TxBD
     */
    FGmacPs_BdSetAddressTx(BdTxPtr, (UINTPTR)&TxFrame);
    FGmacPs_BdSetLength(BdTxPtr, TxFrameLength);
    FGmacPs_BdClearTxUsed(BdTxPtr);
    FGmacPs_BdSetLast(BdTxPtr);

    /*
     * Enqueue to HW
     */
    Status = FGmacPs_BdRingToHw(&(FGmacPs_GetTxRing(InstancePtr)), 1, BdTxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error committing TxBD to HW");
        return FMSH_FAILURE;
    }

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)BdTxPtr, 64);
#endif

    /*
     * Set the Queue pointers
     */
    FGmacPs_SetQueuePtr(InstancePtr, InstancePtr->RxBdRing.BaseBdAddr, 0,
                        FGMACPS_RECV);
    FGmacPs_SetQueuePtr(InstancePtr, InstancePtr->TxBdRing.BaseBdAddr, 1,
                        FGMACPS_SEND);

    /*
     * Start the device
     */
    FGmacPs_Start(InstancePtr);

    /* Start transmit */
    FGmacPs_Transmit(InstancePtr);

    /*
     * Wait for transmission to complete
     */
    while (!FramesTx);

    /*
     * Now that the frame has been sent, post process our TxBDs.
     * Since we have only submitted 1 to hardware, then there should
     * be only 1 ready for post processing.
     */
    if (FGmacPs_BdRingFromHwTx(&(FGmacPs_GetTxRing(InstancePtr)), 1,
                               &BdTxPtr) == 0)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "TxBDs were not ready for post processing");
        return FMSH_FAILURE;
    }

    /*
     * Examine the TxBDs.
     *
     * There isn't much to do. The only thing to check would be DMA
     * exception bits. But this would also be caught in the error
     * handler. So we just return these BDs to the free list.
     */

    Status = FGmacPs_BdRingFree(&(FGmacPs_GetTxRing(InstancePtr)), 1, BdTxPtr);

    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error freeing up TxBDs");
        return FMSH_FAILURE;
    }

    /*
     * Wait for Rx indication
     */
    while (!FramesRx);

    /*
     * Now that the frame has been received, post process our RxBD.
     * Since we have submitted to hardware, then there should be only 1
     * ready for post processing.
     */
    NumRxBuf = FGmacPs_BdRingFromHwRx(&(FGmacPs_GetRxRing(InstancePtr)), 1,
                                      &BdRxPtr);
    if (0 == NumRxBuf)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "RxBD was not ready for post processing");
        return FMSH_FAILURE;
    }

    /*
     * There is no device status to check. If there was a DMA error,
     * it should have been reported to the error handler. Check the
     * receive lengthi against the transmitted length, then verify
     * the data.
     */

    RxFrLen = FGmacPs_GetRxFrameSize(InstancePtr, BdRxPtr);

    if (RxFrLen != TxFrameLength)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Length mismatch");
        return FMSH_FAILURE;
    }
    if (fmsh_gmac_verify_FrameVerify(&TxFrame, &RxFrame) != 0)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Data mismatch");
        return FMSH_FAILURE;
    }

    /*
     * Return the RxBD back to the channel for later allocation. Free
     * the exact number we just post processed.
     */
    Status = FGmacPs_BdRingFree(&(FGmacPs_GetRxRing(InstancePtr)), NumRxBuf,
                                BdRxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error freeing up RxBDs");
        return FMSH_FAILURE;
    }

    return Status;
}

/*****************************************************************************/
/**
 * Gmac phy loopback test.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/
int fmsh_gmac_gtr_phyloop_example (FGmacPs *InstancePtr,
                                   FGmacPs_PhyConfig *PhyCfgPtr)
{
    LONG Status;
    u32 PayloadSize = 1000;
    u32 NumRxBuf = 0;
    u32 RxFrLen;
    FGmacPs_Bd *BdTxPtr;
    FGmacPs_Bd *BdRxPtr;

    GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                   "Entering into fmsh gmac verify phyloop_test\r\n");

    // configure sgmii mode for phy
    // mv88e1512_sgmii_setup_simple(InstancePtr, PhyCfgPtr);

    // configure sgmii mode for gmac
    FGmacPs_SetOperatingSpeed(InstancePtr, speed_1000);
    FGmacPs_SetOptions(InstancePtr, FGMACPS_SGMII_ENABLE_OPTION);

    /*
     * Set up phy into loop mode
     */

    Status = FGmacPs_PHYloopback(InstancePtr, PhyCfgPtr);

    /*
     * Clear variables shared with callbacks
     */
    FramesRx = 0;
    FramesTx = 0;
    DeviceErrors = 0;
    PayloadSize = 600;  // rand()%1400 + 60 - FRAME_HDR_SIZE;//(JUMBO_FRAME_SIZE
                        // - FRAME_HDR_SIZE);
    /*
     * Calculate the frame length (not including FCS)
     */
    TxFrameLength = FGMACPS_HDR_SIZE + PayloadSize;

    /*
     * Setup packet to be transmitted
     */
    FGmacPs_FrameFormatMAC(&TxFrame, FGMACPS_MACADDR);
    FGmacPs_FrameFormatType(&TxFrame, PayloadSize);
    FGmacPs_FrameSetPayloadData(&TxFrame, PayloadSize);

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)&TxFrame, sizeof(EthernetFrame));
#endif

    /*
     * Clear out receive packet memory area
     */
    fmsh_gmac_verify_FrameMemClear(&RxFrame);

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)&RxFrame, sizeof(EthernetFrame));
#endif

    /*
     * Allocate RxBDs since we do not know how many BDs will be used
     * in advance, use RXBD_CNT here.
     */
    Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetRxRing(InstancePtr)), 1,
                                 &BdRxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error allocating RxBD");
        return FMSH_FAILURE;
    }

    /*
     * Setup the BD. The FGmacPs_BdRingClone() call will mark the
     * "wrap" field for last RxBD. Setup buffer address to associated
     * BD.
     */

    FGmacPs_BdSetAddressRx(BdRxPtr, (UINTPTR)&RxFrame);

    /*
     * Enqueue to HW
     */
    Status = FGmacPs_BdRingToHw(&(FGmacPs_GetRxRing(InstancePtr)), 1, BdRxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error committing RxBD to HW");
        return FMSH_FAILURE;
    }
    /*
     * Though the max BD size is 16 bytes for extended desc mode, performing
     * cache flush for 64 bytes. which is equal to the cache line size.
     */

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)BdRxPtr, 64);
#endif

    /*
     * Allocate, setup, and enqueue 1 TxBDs. The first BD will
     * describe the first 32 bytes of TxFrame and the rest of BDs
     * will describe the rest of the frame.
     *
     * The function below will allocate 1 adjacent BDs with BdTxPtr
     * being set as the lead BD.
     */
    Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetTxRing(InstancePtr)), 1,
                                 &BdTxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error allocating TxBD");
        return FMSH_FAILURE;
    }

    /*
     * Setup first TxBD
     */
    FGmacPs_BdSetAddressTx(BdTxPtr, (UINTPTR)&TxFrame);
    FGmacPs_BdSetLength(BdTxPtr, TxFrameLength);
    FGmacPs_BdClearTxUsed(BdTxPtr);
    FGmacPs_BdSetLast(BdTxPtr);

    /*
     * Enqueue to HW
     */
    Status = FGmacPs_BdRingToHw(&(FGmacPs_GetTxRing(InstancePtr)), 1, BdTxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error committing TxBD to HW");
        return FMSH_FAILURE;
    }

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)BdTxPtr, 64);
#endif

    /*
     * Set the Queue pointers
     */
    FGmacPs_SetQueuePtr(InstancePtr, InstancePtr->RxBdRing.BaseBdAddr, 0,
                        FGMACPS_RECV);
    FGmacPs_SetQueuePtr(InstancePtr, InstancePtr->TxBdRing.BaseBdAddr, 1,
                        FGMACPS_SEND);

    /*
     * Start the device
     */
    FGmacPs_Start(InstancePtr);

    /* Start transmit */
    FGmacPs_Transmit(InstancePtr);

    /*
     * Wait for transmission to complete
     */
    while (!FramesTx);

    /*
     * Now that the frame has been sent, post process our TxBDs.
     * Since we have only submitted 1 to hardware, then there should
     * be only 1 ready for post processing.
     */
    if (FGmacPs_BdRingFromHwTx(&(FGmacPs_GetTxRing(InstancePtr)), 1,
                               &BdTxPtr) == 0)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "TxBDs were not ready for post processing");
        return FMSH_FAILURE;
    }

    /*
     * Examine the TxBDs.
     *
     * There isn't much to do. The only thing to check would be DMA
     * exception bits. But this would also be caught in the error
     * handler. So we just return these BDs to the free list.
     */

    Status = FGmacPs_BdRingFree(&(FGmacPs_GetTxRing(InstancePtr)), 1, BdTxPtr);

    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error freeing up TxBDs");
        return FMSH_FAILURE;
    }

    /*
     * Wait for Rx indication
     */
    while (!FramesRx);

    /*
     * Now that the frame has been received, post process our RxBD.
     * Since we have submitted to hardware, then there should be only 1
     * ready for post processing.
     */
    NumRxBuf = FGmacPs_BdRingFromHwRx(&(FGmacPs_GetRxRing(InstancePtr)), 1,
                                      &BdRxPtr);
    if (0 == NumRxBuf)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "RxBD was not ready for post processing");
        return FMSH_FAILURE;
    }

    /*
     * There is no device status to check. If there was a DMA error,
     * it should have been reported to the error handler. Check the
     * receive lengthi against the transmitted length, then verify
     * the data.
     */

    RxFrLen = FGmacPs_GetRxFrameSize(InstancePtr, BdRxPtr);

    if (RxFrLen != TxFrameLength)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Length mismatch");
        return FMSH_FAILURE;
    }
    if (fmsh_gmac_verify_FrameVerify(&TxFrame, &RxFrame) != 0)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Data mismatch");
        return FMSH_FAILURE;
    }

    GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "GMAC PHY LOOP Data check pass! \r\n");

    /*
     * Return the RxBD back to the channel for later allocation. Free
     * the exact number we just post processed.
     */
    Status = FGmacPs_BdRingFree(&(FGmacPs_GetRxRing(InstancePtr)), NumRxBuf,
                                BdRxPtr);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error freeing up RxBDs");
        return FMSH_FAILURE;
    }

    return Status;
}

/*****************************************************************************/
/**
 * Gmac tests entry.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 *
 * @return
 * - FMSH_SUCCESS if initialization was successful
 *
 ******************************************************************************/
int fmsh_gmac_example ()
{
    int Status = FMSH_SUCCESS;
    FGmacPs_Config *GmacPsConfigPtr;
    FGmacPs *GmacPsInstancePtr;
    FGmacPs_PhyConfig *PhyCfgPtr;

    u8 *RxBdSpacePtr;
    u8 *TxBdSpacePtr;

    GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Entering into fmsh gmac example \r\n");

    GmacPsInstancePtr = &GmacPsInstance;
    GmacPsConfigPtr = &GmacPsInstancePtr->Config;
    PhyCfgPtr = &PhyCfg;

    /*************************************/
    /* Setup device for first-time usage */
    /*************************************/
    /*
     *  Initialize instance. Should be configured for DMA
     *  This example calls _CfgInitialize instead of _Initialize due to
     *  retiring _Initialize. So in _CfgInitialize we use
     *  XPAR_(IP)_BASEADDRESS to make sure it is not virtual address.
     */
    fmsh_gmac_verify_device_initial(GmacPsInstancePtr);

    /*
     * Setup the PL isolation if necessary
     */
    u32 REQ_ISO_INT_EN;
    u32 REQ_ISO_INT_TRIG;
    if (GmacPsInstancePtr->Config.InterFaceType == gmac_path_gmii)
    {
        // enable pmu service
        REQ_ISO_INT_EN = FGmacPs_ReadReg(0xFFD80000U, 0x0318U);
        REQ_ISO_INT_EN |= 0x4000U;
        Status = FGmacPs_WriteReg(0xFFD80000U, 0x0318U, REQ_ISO_INT_EN);

        // trigger pmu service
        delay_ms(10);
        REQ_ISO_INT_TRIG = 0x4000U;
        Status = FGmacPs_WriteReg(0xFFD80000U, 0x0320U, REQ_ISO_INT_TRIG);
    }

    fmsh_gmac_verify_phy_initial(GmacPsInstancePtr, PhyCfgPtr);

    /* Allocate Rx and Tx BD space each */
    RxBdSpacePtr = &(bd_space[0]);
    TxBdSpacePtr = &(bd_space[0x4000]);

    /*
     * Setup RxBD space.
     *
     * We have already defined a properly aligned area of memory to store
     * RxBDs at the beginning of this source code file so just pass its
     * address into the function. No MMU is being used so the physical
     * and virtual addresses are the same.
     *
     * Setup a BD template for the Rx channel. This template will be
     * copied to every RxBD. We will not have to explicitly set these
     * again.
     */
    FGmacPs_BdClear(&BdTemplate);

    /*
     * Create the RxBD ring
     */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                  (UINTPTR)RxBdSpacePtr, (UINTPTR)RxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, RXBD_CNT);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "Error setting up RxBD space, BdRingCreate");
        return FMSH_FAILURE;
    }

    Status = FGmacPs_BdRingClone(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                 &BdTemplate, FGMACPS_RECV);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "Error setting up RxBD space, BdRingClone");
        return FMSH_FAILURE;
    }

    /*
     * Setup TxBD space.
     *
     * Like RxBD space, we have already defined a properly aligned area
     * of memory to use.
     *
     * Also like the RxBD space, we create a template. Notice we don't
     * set the "last" attribute. The example will be overriding this
     * attribute so it does no good to set it up here.
     */
    FGmacPs_BdClear(&BdTemplate);
    FGmacPs_BdSetStatus(&BdTemplate, FGMACPS_TXBUF_USED_MASK);

    /*
     * Create the TxBD ring
     */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                  (UINTPTR)TxBdSpacePtr, (UINTPTR)TxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, TXBD_CNT);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "Error setting up TxBD space, BdRingCreate");
        return FMSH_FAILURE;
    }
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                 &BdTemplate, FGMACPS_SEND);
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "Error setting up TxBD space, BdRingClone");
        return FMSH_FAILURE;
    }

    /*
     * This version of GEM supports priority queuing and the current
     * dirver is using tx priority queue 1 and normal rx queue for
     * packet transmit and receive. The below code ensure that the
     * other queue pointers are parked to known state for avoiding
     * the controller to malfunction by fetching the descriptors
     * from these queues.
     */
    FGmacPs_BdClear(&BdRxTerminate);
    FGmacPs_BdSetAddressRx(&BdRxTerminate,
                           (FGMACPS_RXBUF_NEW_MASK | FGMACPS_RXBUF_WRAP_MASK));

    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_RXQ1BASE_OFFSET,
                     (UINTPTR)&BdRxTerminate);
    FGmacPs_BdClear(&BdTxTerminate);
    FGmacPs_BdSetStatus(&BdTxTerminate,
                        (FGMACPS_TXBUF_USED_MASK | FGMACPS_TXBUF_WRAP_MASK));
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_TXQBASE_OFFSET,
                     (UINTPTR)&BdTxTerminate);

#ifdef PSU_CACHE_ENABLE_GMAC
    Fmsh_DCacheFlushRange((UINTPTR)(&BdTxTerminate), 64);
#endif

    /*
     * Setup the interrupt controller and enable interrupts
     */
    Status = fmsh_gmac_verify_gic_setup(GmacPsInstancePtr);

    /*
     * Run the corresponding example
     */

#if GMAC_DEBUG_ITEM_PHYLOOP_EXAMPLE
    Status = fmsh_gmac_phyloop_example(GmacPsInstancePtr, PhyCfgPtr);
    if (Status == FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "fmsh gmac verify phy loop example sucess!\r\n");
    }
    else
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "fmsh gmac verify phy loop example fail!\r\n");
    }
#endif

#if GMAC_DEBUG_ITEM_GTR_PHYLOOP_EXAMPLE
    Status = fmsh_gmac_gtr_phyloop_example(GmacPsInstancePtr, PhyCfgPtr);
    if (Status == FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "fmsh gmac verify gtr phy loop example sucess!\r\n");
    }
    else
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                       "fmsh gmac verify gtr phy loop example fail!\r\n");
    }
#endif

    /*
     * Disable the interrupts for the GmacPs device
     */
    fmsh_gmac_verify_gic_stop(GmacPsInstancePtr);

    /*
     * Stop the device
     */
    FGmacPs_Stop(GmacPsInstancePtr);

    return 0;
}

/***************************** main function *********************************/

int FGmacpsu_example (void)
{
    int Status = FMSH_SUCCESS;

    Status = fmsh_gmac_example();
    if (Status == FMSH_SUCCESS)
    {
        fmsh_print("fmsh gmac example sucess!\r\n");
    }
    else
    {
        fmsh_print("fmsh gmac example fail!\r\n");
    }

    return Status;
}
