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

#ifndef FGMACPS_H /* prevent circular inclusions */
#define FGMACPS_H /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files ********************************/

#include "fmsh_common.h"
#include "fmsh_gmac_assert.h"
#include "fmsh_gmac_bd.h"
#include "fmsh_gmac_hw.h"
#include "fmsh_gmac_status.h"

/************************** Constant Definitions ****************************/

/*
 * Device information
 */
#define FGMACPS_DEVICE_NAME               "fmsh_gmac"
#define FGMACPS_DEVICE_DESC               "FMZQ PS 10/100/1000 MAC"

/** @name Configuration options
 *
 * Device configuration options. See the FGmacPs_SetOptions(),
 * FGmacPs_ClearOptions() and FGmacPs_GetOptions() for information on how to
 * use options.
 *
 * The default state of the options are noted and are what the device and
 * driver will be set to after calling FGmacPs_Reset() or
 * FGmacPs_Initialize().
 *
 * @{
 */

#define FGMACPS_PROMISC_OPTION            0x00000001U
/**< Accept all incoming packets.
 *   This option defaults to disabled (cleared) */

#define FGMACPS_FRAME1536_OPTION          0x00000002U
/**< Frame larger than 1516 support for Tx & Rx.
 *   This option defaults to disabled (cleared) */

#define FGMACPS_VLAN_OPTION               0x00000004U
/**< VLAN Rx & Tx frame support.
 *   This option defaults to disabled (cleared) */

#define FGMACPS_FLOW_CONTROL_OPTION       0x00000010U
/**< Enable recognition of flow control frames on Rx
 *   This option defaults to enabled (set) */

#define FGMACPS_FCS_STRIP_OPTION          0x00000020U
/**< Strip FCS and PAD from incoming frames. Note: PAD from VLAN frames is not
 *   stripped.
 *   This option defaults to enabled (set) */

#define FGMACPS_FCS_INSERT_OPTION         0x00000040U
/**< Generate FCS field and add PAD automatically for outgoing frames.
 *   This option defaults to disabled (cleared) */

#define FGMACPS_LENTYPE_ERR_OPTION        0x00000080U
/**< Enable Length/Type error checking for incoming frames. When this option is
 *   set, the MAC will filter frames that have a mismatched type/length field
 *   and if FGMACPS_REPORT_RXERR_OPTION is set, the user is notified when these
 *   types of frames are encountered. When this option is cleared, the MAC will
 *   allow these types of frames to be received.
 *
 *   This option defaults to disabled (cleared) */

#define FGMACPS_TRANSMITTER_ENABLE_OPTION 0x00000100U
/**< Enable the transmitter.
 *   This option defaults to enabled (set) */

#define FGMACPS_RECEIVER_ENABLE_OPTION    0x00000200U
/**< Enable the receiver
 *   This option defaults to enabled (set) */

#define FGMACPS_BROADCAST_OPTION          0x00000400U
/**< Allow reception of the broadcast address
 *   This option defaults to enabled (set) */

#define FGMACPS_MULTICAST_OPTION          0x00000800U
/**< Allows reception of multicast addresses programmed into hash
 *   This option defaults to disabled (clear) */

#define FGMACPS_RX_CHKSUM_ENABLE_OPTION   0x00001000U
/**< Enable the RX checksum offload
 *   This option defaults to enabled (set) */

#define FGMACPS_TX_CHKSUM_ENABLE_OPTION   0x00002000U
/**< Enable the TX checksum offload
 *   This option defaults to enabled (set) */

#define FGMACPS_JUMBO_ENABLE_OPTION       0x00004000U
#define FGMACPS_SGMII_ENABLE_OPTION       0x00008000U

#define FGMACPS_DEFAULT_OPTIONS                                          \
    ((u32)FGMACPS_FLOW_CONTROL_OPTION | (u32)FGMACPS_FCS_INSERT_OPTION | \
     (u32)FGMACPS_FCS_STRIP_OPTION | (u32)FGMACPS_BROADCAST_OPTION |     \
     (u32)FGMACPS_LENTYPE_ERR_OPTION |                                   \
     (u32)FGMACPS_TRANSMITTER_ENABLE_OPTION |                            \
     (u32)FGMACPS_RECEIVER_ENABLE_OPTION |                               \
     (u32)FGMACPS_RX_CHKSUM_ENABLE_OPTION |                              \
     (u32)FGMACPS_TX_CHKSUM_ENABLE_OPTION)

/**< Default options set when device is initialized or reset */
/*@}*/

/** @name Callback identifiers
 *
 * These constants are used as parameters to FGmacPs_SetHandler()
 * @{
 */
#define FGMACPS_HANDLER_DMASEND 1U
#define FGMACPS_HANDLER_DMARECV 2U
#define FGMACPS_HANDLER_ERROR   3U
/*@}*/

/* Constants to determine the configuration of the hardware device. They are
 * used to allow the driver to verify it can operate with the hardware.
 */
#define FGMACPS_MDIO_DIV_DFT    MDC_DIV_32 /**< Default MDIO clock divisor */

/* The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define FGMACPS_MAC_ADDR_SIZE   6U     /* size of Ethernet header */

#define FGMACPS_MTU             1500U  /* max MTU size of Ethernet frame */
#define FGMACPS_MTU_JUMBO       10240U /* max MTU size of jumbo frame */
#define FGMACPS_HDR_SIZE        14U    /* size of Ethernet header */
#define FGMACPS_HDR_VLAN_SIZE   18U    /* size of Ethernet header with VLAN */
#define FGMACPS_TRL_SIZE        4U     /* size of Ethernet trailer (FCS) */
#define FGMACPS_MAX_FRAME_SIZE \
    (FGMACPS_MTU + FGMACPS_HDR_SIZE + FGMACPS_TRL_SIZE)
#define FGMACPS_MAX_VLAN_FRAME_SIZE \
    (FGMACPS_MTU + FGMACPS_HDR_SIZE + FGMACPS_HDR_VLAN_SIZE + FGMACPS_TRL_SIZE)
#define FGMACPS_MAX_VLAN_FRAME_SIZE_JUMBO                           \
    (FGMACPS_MTU_JUMBO + FGMACPS_HDR_SIZE + FGMACPS_HDR_VLAN_SIZE + \
     FGMACPS_TRL_SIZE)

/* DMACR Bust length hash defines */

#define FGMACPS_SINGLE_BURST 0x00000001
#define FGMACPS_4BYTE_BURST  0x00000004
#define FGMACPS_8BYTE_BURST  0x00000008
#define FGMACPS_16BYTE_BURST 0x00000010

/**************************** Type Definitions ******************************/
/** @name Typedefs for callback functions
 *
 * These callbacks are invoked in interrupt context.
 * @{
 */
/**
 * Callback invoked when frame(s) have been sent or received in interrupt
 * driven DMA mode. To set the send callback, invoke FGmacPs_SetHandler().
 *
 * @param CallBackRef is user data assigned when the callback was set.
 *
 * @note
 * See fmsh_gmac_hw.h for bitmasks definitions and the device hardware spec for
 * further information on their meaning.
 *
 */
typedef void (*FGmacPs_Handler)(void *CallBackRef);

/**
 * Callback when an asynchronous error occurs. To set this callback, invoke
 * FGmacPs_SetHandler() with FGMACPS_HANDLER_ERROR in the HandlerType
 * paramter.
 *
 * @param CallBackRef is user data assigned when the callback was set.
 * @param Direction defines either receive or transmit error(s) has occurred.
 * @param ErrorWord definition varies with Direction
 *
 */
typedef void (*FGmacPs_ErrHandler)(void *CallBackRef, u8 Direction,
                                   u32 ErrorWord);

/*@}*/

/* interface type */
typedef enum _FGmacPs_PathSel {
    gmac_path_gmii = 0,
    gmac_path_rgmii = 1,
    gmac_path_sgmii = 2
} FGmacPs_ITF_Type;

/* speed */
typedef enum _FGmacPs_Speed {
    speed_10 = 10,
    speed_100 = 100,
    speed_1000 = 1000
} FGmacPs_Speed;

/**
 * This typedef contains configuration information for a device.
 */
typedef struct {
    u16 DeviceId;        /**< Unique ID  of device */
    UINTPTR BaseAddress; /**< Physical base address of IPIF registers */
    FGmacPs_Speed Speed; /**< gmac speed */
    FGmacPs_ITF_Type InterFaceType; /**< gmac interface type */
    u8 IsCacheCoherent;             /**< Applicable only to A53 in EL1 mode;
                                     * describes whether Cache Coherent or not */
} FGmacPs_Config;

/**
 * The FGmacPs driver instance data. The user is required to allocate a
 * structure of this type for every FGmacPs device in the system. A pointer
 * to a structure of this type is then passed to the driver API functions.
 */
typedef struct FGmacPs_Instance {
    FGmacPs_Config Config;   /* Hardware configuration */
    u32 IsStarted;           /* Device is currently started */
    u32 IsReady;             /* Device is initialized and ready */
    u32 Options;             /* Current options word */

    FGmacPs_BdRing TxBdRing; /* Transmit BD ring */
    FGmacPs_BdRing RxBdRing; /* Receive BD ring */

    FGmacPs_Handler SendHandler;
    FGmacPs_Handler RecvHandler;
    void *SendRef;
    void *RecvRef;

    FGmacPs_ErrHandler ErrorHandler;
    void *ErrorRef;
    u32 Version;
    u32 RxBufMask;
    u32 MaxMtuSize;
    u32 MaxFrameSize;
    u32 MaxVlanFrameSize;

} FGmacPs;

/* phy */
typedef struct FGmacPs_Phyconfig_Instance {
    u8 phy_device;
    u8 auto_detect_ad_en;
    u8 phy_address;
    u8 link_up;
    u8 auto_nag_en;
    FGmacPs_Speed speed;
    u8 is_duplex;
    u8 is_fixlink;
    FGmacPs_ITF_Type interface;

    u8 (*phy_op_init)(FGmacPs *InstancePtr);
    u8 (*phy_op_cfg)(FGmacPs *InstancePtr);
    u8 (*phy_op_reset)(FGmacPs *InstancePtr);
    u8 (*phy_op_get_status)(FGmacPs *InstancePtr);
    u8 (*phy_op_reg_dump)(FGmacPs *InstancePtr);

} FGmacPs_PhyConfig;

/***************** Macros (Inline Functions) Definitions ********************/

/****************************************************************************/
/**
 * Retrieve the Tx ring object. This object can be used in the various Ring
 * API functions.
 *
 * @param  InstancePtr is the DMA channel to operate on.
 *
 * @return TxBdRing attribute
 *
 * @note
 * C-style signature:
 *    FGmacPs_BdRing FGmacPs_GetTxRing(FGmacPs *InstancePtr)
 *
 *****************************************************************************/
#define FGmacPs_GetTxRing(InstancePtr) ((InstancePtr)->TxBdRing)

/****************************************************************************/
/**
 * Retrieve the Rx ring object. This object can be used in the various Ring
 * API functions.
 *
 * @param  InstancePtr is the DMA channel to operate on.
 *
 * @return RxBdRing attribute
 *
 * @note
 * C-style signature:
 *    FGmacPs_BdRing FGmacPs_GetRxRing(FGmacPs *InstancePtr)
 *
 *****************************************************************************/
#define FGmacPs_GetRxRing(InstancePtr) ((InstancePtr)->RxBdRing)

/****************************************************************************/
/**
 *
 * Enable interrupts specified in <i>Mask</i>. The corresponding interrupt for
 * each bit set to 1 in <i>Mask</i>, will be enabled.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Mask contains a bit mask of interrupts to enable. The mask can
 *        be formed using a set of bitwise or'd values.
 *
 * @note
 * The state of the transmitter and receiver are not modified by this function.
 * C-style signature
 *     void FGmacPs_IntEnable(FGmacPs *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define FGmacPs_IntEnable(InstancePtr, Mask)                                \
    FGmacPs_WriteReg((InstancePtr)->Config.BaseAddress, FGMACPS_IER_OFFSET, \
                     ((Mask) & FGMACPS_IXR_ALL_MASK));

/****************************************************************************/
/**
 *
 * Disable interrupts specified in <i>Mask</i>. The corresponding interrupt for
 * each bit set to 1 in <i>Mask</i>, will be enabled.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Mask contains a bit mask of interrupts to disable. The mask can
 *        be formed using a set of bitwise or'd values.
 *
 * @note
 * The state of the transmitter and receiver are not modified by this function.
 * C-style signature
 *     void FGmacPs_IntDisable(FGmacPs *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define FGmacPs_IntDisable(InstancePtr, Mask)                               \
    FGmacPs_WriteReg((InstancePtr)->Config.BaseAddress, FGMACPS_IDR_OFFSET, \
                     ((Mask) & FGMACPS_IXR_ALL_MASK));

/****************************************************************************/
/**
 *
 * Enable interrupts specified in <i>Mask</i>. The corresponding interrupt for
 * each bit set to 1 in <i>Mask</i>, will be enabled.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Mask contains a bit mask of interrupts to enable. The mask can
 *        be formed using a set of bitwise or'd values.
 *
 * @note
 * The state of the transmitter and receiver are not modified by this function.
 * C-style signature
 *     void FGmacPs_IntQ1Enable(FGmacPs *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define FGmacPs_IntQ1Enable(InstancePtr, Mask)          \
    FGmacPs_WriteReg((InstancePtr)->Config.BaseAddress, \
                     FGMACPS_INTQ1_IER_OFFSET,          \
                     ((Mask) & FGMACPS_INTQ1_IXR_ALL_MASK));

/****************************************************************************/
/**
 *
 * Disable interrupts specified in <i>Mask</i>. The corresponding interrupt for
 * each bit set to 1 in <i>Mask</i>, will be enabled.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Mask contains a bit mask of interrupts to disable. The mask can
 *        be formed using a set of bitwise or'd values.
 *
 * @note
 * The state of the transmitter and receiver are not modified by this function.
 * C-style signature
 *     void FGmacPs_IntDisable(FGmacPs *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define FGmacPs_IntQ1Disable(InstancePtr, Mask)         \
    FGmacPs_WriteReg((InstancePtr)->Config.BaseAddress, \
                     FGMACPS_INTQ1_IDR_OFFSET,          \
                     ((Mask) & FGMACPS_INTQ1_IXR_ALL_MASK));

/****************************************************************************/
/**
 *
 * This macro triggers trasmit circuit to send data currently in TX buffer(s).
 *
 * @param InstancePtr is a pointer to the FGmacPs instance to be worked on.
 *
 * @return
 *
 * @note
 *
 * Signature: void FGmacPs_Transmit(FGmacPs *InstancePtr)
 *
 *****************************************************************************/
#define FGmacPs_Transmit(InstancePtr)                                          \
    FGmacPs_WriteReg((InstancePtr)->Config.BaseAddress, FGMACPS_NWCTRL_OFFSET, \
                     (FGmacPs_ReadReg((InstancePtr)->Config.BaseAddress,       \
                                      FGMACPS_NWCTRL_OFFSET) |                 \
                      FGMACPS_NWCTRL_STARTTX_MASK))

/****************************************************************************/
/**
 *
 * This macro determines if the device is configured with checksum offloading
 * on the receive channel
 *
 * @param InstancePtr is a pointer to the FGmacPs instance to be worked on.
 *
 * @return
 *
 * Boolean TRUE if the device is configured with checksum offloading, or
 * FALSE otherwise.
 *
 * @note
 *
 * Signature: u32 FGmacPs_IsRxCsum(FGmacPs *InstancePtr)
 *
 *****************************************************************************/
#define FGmacPs_IsRxCsum(InstancePtr)                    \
    ((FGmacPs_ReadReg((InstancePtr)->Config.BaseAddress, \
                      FGMACPS_NWCFG_OFFSET) &            \
      FGMACPS_NWCFG_RXCHKSUMEN_MASK) != 0U               \
         ? TRUE                                          \
         : FALSE)

/****************************************************************************/
/**
 *
 * This macro determines if the device is configured with checksum offloading
 * on the transmit channel
 *
 * @param InstancePtr is a pointer to the FGmacPs instance to be worked on.
 *
 * @return
 *
 * Boolean TRUE if the device is configured with checksum offloading, or
 * FALSE otherwise.
 *
 * @note
 *
 * Signature: u32 FGmacPs_IsTxCsum(FGmacPs *InstancePtr)
 *
 *****************************************************************************/
#define FGmacPs_IsTxCsum(InstancePtr)                    \
    ((FGmacPs_ReadReg((InstancePtr)->Config.BaseAddress, \
                      FGMACPS_DMACR_OFFSET) &            \
      FGMACPS_DMACR_TCPCKSUM_MASK) != 0U                 \
         ? TRUE                                          \
         : FALSE)

/************************** Function Prototypes *****************************/

/*
 * Initialization functions in fmsh_gmac.c
 */
LONG FGmacPs_CfgInitialize(FGmacPs *InstancePtr, FGmacPs_Config *CfgPtr,
                           UINTPTR EffectiveAddress);
void FGmacPs_Start(FGmacPs *InstancePtr);
void FGmacPs_Stop(FGmacPs *InstancePtr);
void FGmacPs_Reset(FGmacPs *InstancePtr);
void FGmacPs_SetQueuePtr(FGmacPs *InstancePtr, UINTPTR QPtr, u8 QueueNum,
                         u16 Direction);

/*
 * Lookup configuration in fmsh_gmac_sinit.c
 */
FGmacPs_Config *FGmacPs_LookupConfig(u16 DeviceId);

/*
 * Interrupt-related functions in fmsh_gmac_intr.c
 * DMA only and FIFO is not supported. This DMA does not support coalescing.
 */
LONG FGmacPs_SetHandler(FGmacPs *InstancePtr, u32 HandlerType,
                        void *FuncPointer, void *CallBackRef);
void FGmacPs_IntrHandler(void *FGmacPsPtr);

/*
 * MAC configuration/control functions in FGmacPs_control.c
 */
LONG FGmacPs_SetOptions(FGmacPs *InstancePtr, u32 Options);
LONG FGmacPs_ClearOptions(FGmacPs *InstancePtr, u32 Options);
u32 FGmacPs_GetOptions(FGmacPs *InstancePtr);

LONG FGmacPs_SetMacAddress(FGmacPs *InstancePtr, void *AddressPtr, u8 Index);
LONG FGmacPs_DeleteHash(FGmacPs *InstancePtr, void *AddressPtr);
void FGmacPs_GetMacAddress(FGmacPs *InstancePtr, void *AddressPtr, u8 Index);

LONG FGmacPs_SetHash(FGmacPs *InstancePtr, void *AddressPtr);
void FGmacPs_ClearHash(FGmacPs *InstancePtr);
void FGmacPs_GetHash(FGmacPs *InstancePtr, void *AddressPtr);

void FGmacPs_SetOperatingSpeed(FGmacPs *InstancePtr, u16 Speed);
u16 FGmacPs_GetOperatingSpeed(FGmacPs *InstancePtr);

LONG FGmacPs_SetTypeIdCheck(FGmacPs *InstancePtr, u32 Id_Check, u8 Index);

LONG FGmacPs_SendPausePacket(FGmacPs *InstancePtr);
void FGmacPs_DMABLengthUpdate(FGmacPs *InstancePtr, s32 BLength);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
