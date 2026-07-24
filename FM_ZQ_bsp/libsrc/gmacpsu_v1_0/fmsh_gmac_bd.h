/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_bd.h
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

#ifndef FGMACPS_BD_H /* prevent circular inclusions */
#define FGMACPS_BD_H /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include <string.h>

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
#ifdef __aarch64__
/* Minimum BD alignment */
#define FGMACPS_DMABD_MINIMUM_ALIGNMENT 64U
#define FGMACPS_BD_NUM_WORDS            4U
#else
/* Minimum BD alignment */
#define FGMACPS_DMABD_MINIMUM_ALIGNMENT 4U
#define FGMACPS_BD_NUM_WORDS            2U
#endif

/**
 * The FGmacPs_Bd is the type for buffer descriptors (BDs).
 */
typedef u32 FGmacPs_Bd[FGMACPS_BD_NUM_WORDS];

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * Zero out BD fields
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return Nothing
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdClear(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdClear(BdPtr) memset((BdPtr), 0, sizeof(FGmacPs_Bd))

/****************************************************************************/
/**
 *
 * Read the given Buffer Descriptor word.
 *
 * @param    BaseAddress is the base address of the BD to read
 * @param    Offset is the word offset to be read
 *
 * @return   The 32-bit value of the field
 *
 * @note
 * C-style signature:
 *    u32 FGmacPs_BdRead(UINTPTR BaseAddress, UINTPTR Offset)
 *
 *****************************************************************************/
#define FGmacPs_BdRead(BaseAddress, Offset) \
    (*(u32 *)((UINTPTR)((void *)(BaseAddress)) + (u32)(Offset)))

/****************************************************************************/
/**
 *
 * Write the given Buffer Descriptor word.
 *
 * @param    BaseAddress is the base address of the BD to write
 * @param    Offset is the word offset to be written
 * @param    Data is the 32-bit value to write to the field
 *
 * @return   None.
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdWrite(UINTPTR BaseAddress, UINTPTR Offset, UINTPTR Data)
 *
 *****************************************************************************/
#define FGmacPs_BdWrite(BaseAddress, Offset, Data) \
    (*(u32 *)((UINTPTR)(void *)(BaseAddress) + (u32)(Offset)) = (u32)(Data))

/*****************************************************************************/
/**
 * Set the BD's Address field (word 0).
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  Addr  is the value to write to BD's status field.
 *
 * @note :
 *
 * C-style signature:
 *    void FGmacPs_BdSetAddressTx(FGmacPs_Bd* BdPtr, UINTPTR Addr)
 *
 *****************************************************************************/
#if defined(__aarch64__) || defined(__arch64__)
#define FGmacPs_BdSetAddressTx(BdPtr, Addr)             \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_OFFSET,    \
                    (u32)((Addr) & ULONG64_LO_MASK));   \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_HI_OFFSET, \
                    (u32)(((Addr) & ULONG64_HI_MASK) >> 32U));
#else
#define FGmacPs_BdSetAddressTx(BdPtr, Addr) \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_OFFSET, (u32)(Addr))
#endif

/*****************************************************************************/
/**
 * Set the BD's Address field (word 0).
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  Addr  is the value to write to BD's status field.
 *
 * @note : Due to some bits are mixed within recevie BD's address field,
 *         read-modify-write is performed.
 *
 * C-style signature:
 *    void FGmacPs_BdSetAddressRx(FGmacPs_Bd* BdPtr, UINTPTR Addr)
 *
 *****************************************************************************/
#ifdef __aarch64__
#define FGmacPs_BdSetAddressRx(BdPtr, Addr)                             \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_OFFSET,                    \
                    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET) & \
                      ~FGMACPS_RXBUF_ADD_MASK) |                        \
                     ((u32)((Addr) & ULONG64_LO_MASK))));               \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_HI_OFFSET,                 \
                    (u32)(((Addr) & ULONG64_HI_MASK) >> 32U));
#else
#define FGmacPs_BdSetAddressRx(BdPtr, Addr)                             \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_OFFSET,                    \
                    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET) & \
                      ~FGMACPS_RXBUF_ADD_MASK) |                        \
                     (UINTPTR)(Addr)))
#endif

/*****************************************************************************/
/**
 * Set the BD's Status field (word 1).
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  Data  is the value to write to BD's status field.
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdSetStatus(FGmacPs_Bd* BdPtr, UINTPTR Data)
 *
 *****************************************************************************/
#define FGmacPs_BdSetStatus(BdPtr, Data)             \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET, \
                    FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) | (Data))

/*****************************************************************************/
/**
 * Retrieve the BD's Packet DMA transfer status word (word 1).
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return Status word
 *
 * @note
 * C-style signature:
 *    u32 FGmacPs_BdGetStatus(FGmacPs_Bd* BdPtr)
 *
 * Due to the BD bit layout differences in transmit and receive. User's
 * caution is required.
 *****************************************************************************/
#define FGmacPs_BdGetStatus(BdPtr) \
    FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET)

/*****************************************************************************/
/**
 * Get the address (bits 0..31) of the BD's buffer address (word 0)
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdGetBufAddr(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#if defined(__aarch64__) || defined(__arch64__)
#define FGmacPs_BdGetBufAddr(BdPtr)                    \
    (FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET) | \
     (FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_HI_OFFSET)) << 32U)
#else
#define FGmacPs_BdGetBufAddr(BdPtr) \
    (FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET))
#endif

/*****************************************************************************/
/**
 * Set transfer length in bytes for the given BD. The length must be set each
 * time a BD is submitted to hardware.
 *
 * @param  BdPtr is the BD pointer to operate on
 * @param  LenBytes is the number of bytes to transfer.
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdSetLength(FGmacPs_Bd* BdPtr, u32 LenBytes)
 *
 *****************************************************************************/
#define FGmacPs_BdSetLength(BdPtr, LenBytes)                            \
    FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,                    \
                    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
                      ~FGMACPS_TXBUF_LEN_MASK) |                        \
                     (LenBytes)))

/*****************************************************************************/
/**
 * Retrieve the BD length field.
 *
 * For Tx channels, the returned value is the same as that written with
 * FGmacPs_BdSetLength().
 *
 * For Rx channels, the returned value is the size of the received packet.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return Length field processed by hardware or set by
 *         FGmacPs_BdSetLength().
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdGetLength(FGmacPs_Bd* BdPtr)
 *    XEAMCPS_RXBUF_LEN_MASK is same as FGMACPS_TXBUF_LEN_MASK.
 *
 *****************************************************************************/
#define FGmacPs_BdGetLength(BdPtr) \
    (FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & FGMACPS_RXBUF_LEN_MASK)

/*****************************************************************************/
/**
 * Retrieve the RX frame size.
 *
 * The returned value is the size of the received packet.
 * This API supports jumbo frame sizes if enabled.
 *
 * @param  InstancePtr is the pointer to XEmacps instance
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return Length field processed by hardware or set by
 *         FGmacPs_BdSetLength().
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_GetRxFrameSize(FGmacPs* InstancePtr, FGmacPs_Bd* BdPtr)
 *    RxBufMask is dependent on whether jumbo is enabled or not.
 *
 *****************************************************************************/
#define FGmacPs_GetRxFrameSize(InstancePtr, BdPtr) \
    (FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & (InstancePtr)->RxBufMask)

/*****************************************************************************/
/**
 * Test whether the given BD has been marked as the last BD of a packet.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @return TRUE if BD represents the "Last" BD of a packet, FALSE otherwise
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsLast(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsLast(BdPtr)                         \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_EOF_MASK) != 0U                     \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Tell the DMA engine that the given transmit BD marks the end of the current
 * packet to be processed.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdSetLast(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdSetLast(BdPtr)                                       \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,                  \
                     FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) | \
                         FGMACPS_TXBUF_LAST_MASK))

/*****************************************************************************/
/**
 * Tell the DMA engine that the current packet does not end with the given
 * BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdClearLast(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdClearLast(BdPtr)                                     \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,                  \
                     FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
                         ~FGMACPS_TXBUF_LAST_MASK))

/*****************************************************************************/
/**
 * Set this bit to mark the last descriptor in the receive buffer descriptor
 * list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdSetRxWrap(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
/*#define FGmacPs_BdSetRxWrap(BdPtr)                                 \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_OFFSET,             \
    FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET) |             \
    FGMACPS_RXBUF_WRAP_MASK))
*/

/*****************************************************************************/
/**
 * Determine the wrap bit of the receive BD which indicates end of the
 * BD list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u8 FGmacPs_BdIsRxWrap(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxWrap(BdPtr)                       \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET) & \
      FGMACPS_RXBUF_WRAP_MASK) != 0U                    \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Sets this bit to mark the last descriptor in the transmit buffer
 * descriptor list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdSetTxWrap(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
/*#define FGmacPs_BdSetTxWrap(BdPtr)                                 \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,             \
    FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) |             \
    FGMACPS_TXBUF_WRAP_MASK))
*/

/*****************************************************************************/
/**
 * Determine the wrap bit of the transmit BD which indicates end of the
 * BD list.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    u8 FGmacPs_BdGetTxWrap(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsTxWrap(BdPtr)                       \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_TXBUF_WRAP_MASK) != 0U                    \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/*
 * Must clear this bit to enable the MAC to write data to the receive
 * buffer. Hardware sets this bit once it has successfully written a frame to
 * memory. Once set, software has to clear the bit before the buffer can be
 * used again. This macro clear the new bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdClearRxNew(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdClearRxNew(BdPtr)                                    \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_ADDR_OFFSET,                  \
                     FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET) & \
                         ~FGMACPS_RXBUF_NEW_MASK))

/*****************************************************************************/
/**
 * Determine the new bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsRxNew(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxNew(BdPtr)                        \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_ADDR_OFFSET) & \
      FGMACPS_RXBUF_NEW_MASK) != 0U                     \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Software sets this bit to disable the buffer to be read by the hardware.
 * Hardware sets this bit for the first buffer of a frame once it has been
 * successfully transmitted. This macro sets this bit of transmit BD to avoid
 * confusion.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdSetTxUsed(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdSetTxUsed(BdPtr)                                     \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,                  \
                     FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) | \
                         FGMACPS_TXBUF_USED_MASK))

/*****************************************************************************/
/**
 * Software clears this bit to enable the buffer to be read by the hardware.
 * Hardware sets this bit for the first buffer of a frame once it has been
 * successfully transmitted. This macro clears this bit of transmit BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    void FGmacPs_BdClearTxUsed(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdClearTxUsed(BdPtr)                                   \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,                  \
                     FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
                         ~FGMACPS_TXBUF_USED_MASK))

/*****************************************************************************/
/**
 * Determine the used bit of the transmit BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsTxUsed(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsTxUsed(BdPtr)                       \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_TXBUF_USED_MASK) != 0U                    \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine if a frame fails to be transmitted due to too many retries.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsTxRetry(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsTxRetry(BdPtr)                      \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_TXBUF_RETRY_MASK) != 0U                   \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine if a frame fails to be transmitted due to data can not be
 * feteched in time or buffers are exhausted.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsTxUrun(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsTxUrun(BdPtr)                       \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_TXBUF_URUN_MASK) != 0U                    \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine if a frame fails to be transmitted due to buffer is exhausted
 * mid-frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsTxExh(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsTxExh(BdPtr)                        \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_TXBUF_EXH_MASK) != 0U                     \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Sets this bit, no CRC will be appended to the current frame. This control
 * bit must be set for the first buffer in a frame and will be ignored for
 * the subsequent buffers of a frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * This bit must be clear when using the transmit checksum generation offload,
 * otherwise checksum generation and substitution will not occur.
 *
 * C-style signature:
 *    UINTPTR FGmacPs_BdSetTxNoCRC(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdSetTxNoCRC(BdPtr)                                    \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,                  \
                     FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) | \
                         FGMACPS_TXBUF_NOCRC_MASK))

/*****************************************************************************/
/**
 * Clear this bit, CRC will be appended to the current frame. This control
 * bit must be set for the first buffer in a frame and will be ignored for
 * the subsequent buffers of a frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * This bit must be clear when using the transmit checksum generation offload,
 * otherwise checksum generation and substitution will not occur.
 *
 * C-style signature:
 *    UINTPTR FGmacPs_BdClearTxNoCRC(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdClearTxNoCRC(BdPtr)                                  \
    (FGmacPs_BdWrite((BdPtr), FGMACPS_BD_STAT_OFFSET,                  \
                     FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
                         ~FGMACPS_TXBUF_NOCRC_MASK))

/*****************************************************************************/
/**
 * Determine the broadcast bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsRxBcast(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxBcast(BdPtr)                      \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_BCAST_MASK) != 0U                   \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine the multicast hash bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsRxMultiHash(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxMultiHash(BdPtr)                  \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_MULTIHASH_MASK) != 0U               \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine the unicast hash bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsRxUniHash(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxUniHash(BdPtr)                    \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_UNIHASH_MASK) != 0U                 \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine if the received frame is a VLAN Tagged frame.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsRxVlan(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxVlan(BdPtr)                       \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_VLAN_MASK) != 0U                    \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine if the received frame has Type ID of 8100h and null VLAN
 * identifier(Priority tag).
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsRxPri(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxPri(BdPtr)                        \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_PRI_MASK) != 0U                     \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine if the received frame's Concatenation Format Indicator (CFI) of
 * the frames VLANTCI field was set.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdIsRxCFI(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxCFI(BdPtr)                        \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_CFI_MASK) != 0U                     \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine the End Of Frame (EOF) bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdGetRxEOF(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxEOF(BdPtr)                        \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_EOF_MASK) != 0U                     \
         ? TRUE                                         \
         : FALSE)

/*****************************************************************************/
/**
 * Determine the Start Of Frame (SOF) bit of the receive BD.
 *
 * @param  BdPtr is the BD pointer to operate on
 *
 * @note
 * C-style signature:
 *    UINTPTR FGmacPs_BdGetRxSOF(FGmacPs_Bd* BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdIsRxSOF(BdPtr)                        \
    ((FGmacPs_BdRead((BdPtr), FGMACPS_BD_STAT_OFFSET) & \
      FGMACPS_RXBUF_SOF_MASK) != 0U                     \
         ? TRUE                                         \
         : FALSE)

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif                   /* end of protection macro */

#ifndef FGMACPS_BDRING_H /* prevent curcular inclusions */
#define FGMACPS_BDRING_H /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/**************************** Type Definitions *******************************/

/** This is an internal structure used to maintain the DMA list */
typedef struct {
    UINTPTR PhysBaseAddr; /**< Physical address of 1st BD in list */
    UINTPTR BaseBdAddr;   /**< Virtual address of 1st BD in list */
    UINTPTR HighBdAddr;   /**< Virtual address of last BD in the list */
    u32 Length;           /**< Total size of ring in bytes */
    u32 RunState;         /**< Flag to indicate DMA is started */
    u32 Separation;       /**< Number of bytes between the starting address
                                       of adjacent BDs */
    FGmacPs_Bd *FreeHead;
    /**< First BD in the free group */
    FGmacPs_Bd *PreHead; /**< First BD in the pre-work group */
    FGmacPs_Bd *HwHead;  /**< First BD in the work group */
    FGmacPs_Bd *HwTail;  /**< Last BD in the work group */
    FGmacPs_Bd *PostHead;
    /**< First BD in the post-work group */
    FGmacPs_Bd *BdaRestart;
    /**< BDA to load when channel is started */

    volatile u32 HwCnt; /**< Number of BDs in work group */
    u32 PreCnt;         /**< Number of BDs in pre-work group */
    u32 FreeCnt;        /**< Number of allocatable BDs in the free group */
    u32 PostCnt;        /**< Number of BDs in post-work group */
    u32 AllCnt;         /**< Total Number of BDs for channel */
} FGmacPs_BdRing;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * Use this macro at initialization time to determine how many BDs will fit
 * in a BD list within the given memory constraints.
 *
 * The results of this macro can be provided to FGmacPs_BdRingCreate().
 *
 * @param Alignment specifies what byte alignment the BDs must fall on and
 *        must be a power of 2 to get an accurate calculation (32, 64, 128,...)
 * @param Bytes is the number of bytes to be used to store BDs.
 *
 * @return Number of BDs that can fit in the given memory area
 *
 * @note
 * C-style signature:
 *    u32 FGmacPs_BdRingCntCalc(u32 Alignment, u32 Bytes)
 *
 ******************************************************************************/
#define FGmacPs_BdRingCntCalc(Alignment, Bytes) \
    (u32)((Bytes) / (sizeof(FGmacPs_Bd)))

/*****************************************************************************/
/**
 * Use this macro at initialization time to determine how many bytes of memory
 * is required to contain a given number of BDs at a given alignment.
 *
 * @param Alignment specifies what byte alignment the BDs must fall on. This
 *        parameter must be a power of 2 to get an accurate calculation (32, 64,
 *        128,...)
 * @param NumBd is the number of BDs to calculate memory size requirements for
 *
 * @return The number of bytes of memory required to create a BD list with the
 *         given memory constraints.
 *
 * @note
 * C-style signature:
 *    u32 FGmacPs_BdRingMemCalc(u32 Alignment, u32 NumBd)
 *
 ******************************************************************************/
#define FGmacPs_BdRingMemCalc(Alignment, NumBd) \
    (u32)(sizeof(FGmacPs_Bd) * (NumBd))

/****************************************************************************/
/**
 * Return the total number of BDs allocated by this channel with
 * FGmacPs_BdRingCreate().
 *
 * @param  RingPtr is the DMA channel to operate on.
 *
 * @return The total number of BDs allocated for this channel.
 *
 * @note
 * C-style signature:
 *    u32 FGmacPs_BdRingGetCnt(FGmacPs_BdRing* RingPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdRingGetCnt(RingPtr)     ((RingPtr)->AllCnt)

/****************************************************************************/
/**
 * Return the number of BDs allocatable with FGmacPs_BdRingAlloc() for pre-
 * processing.
 *
 * @param  RingPtr is the DMA channel to operate on.
 *
 * @return The number of BDs currently allocatable.
 *
 * @note
 * C-style signature:
 *    u32 FGmacPs_BdRingGetFreeCnt(FGmacPs_BdRing* RingPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdRingGetFreeCnt(RingPtr) ((RingPtr)->FreeCnt)

/****************************************************************************/
/**
 * Return the next BD from BdPtr in a list.
 *
 * @param  RingPtr is the DMA channel to operate on.
 * @param  BdPtr is the BD to operate on.
 *
 * @return The next BD in the list relative to the BdPtr parameter.
 *
 * @note
 * C-style signature:
 *    FGmacPs_Bd *FGmacPs_BdRingNext(FGmacPs_BdRing* RingPtr,
 *                                      FGmacPs_Bd *BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdRingNext(RingPtr, BdPtr)                 \
    (((UINTPTR)((void *)(BdPtr)) >= (RingPtr)->HighBdAddr) \
         ? (FGmacPs_Bd *)((void *)(RingPtr)->BaseBdAddr)   \
         : (FGmacPs_Bd *)((UINTPTR)((void *)(BdPtr)) + (RingPtr)->Separation))

/****************************************************************************/
/**
 * Return the previous BD from BdPtr in the list.
 *
 * @param  RingPtr is the DMA channel to operate on.
 * @param  BdPtr is the BD to operate on
 *
 * @return The previous BD in the list relative to the BdPtr parameter.
 *
 * @note
 * C-style signature:
 *    FGmacPs_Bd *FGmacPs_BdRingPrev(FGmacPs_BdRing* RingPtr,
 *                                      FGmacPs_Bd *BdPtr)
 *
 *****************************************************************************/
#define FGmacPs_BdRingPrev(RingPtr, BdPtr)       \
    (((UINTPTR)(BdPtr) <= (RingPtr)->BaseBdAddr) \
         ? (FGmacPs_Bd *)(RingPtr)->HighBdAddr   \
         : (FGmacPs_Bd *)((UINTPTR)(BdPtr) - (RingPtr)->Separation))

/************************** Function Prototypes ******************************/

/*
 * Scatter gather DMA related functions in fmsh_gmac_bdring.c
 */
LONG FGmacPs_BdRingCreate(FGmacPs_BdRing *RingPtr, UINTPTR PhysAddr,
                          UINTPTR VirtAddr, u32 Alignment, u32 BdCount);
LONG FGmacPs_BdRingClone(FGmacPs_BdRing *RingPtr, FGmacPs_Bd *SrcBdPtr,
                         u8 Direction);
LONG FGmacPs_BdRingAlloc(FGmacPs_BdRing *RingPtr, u32 NumBd,
                         FGmacPs_Bd **BdSetPtr);
LONG FGmacPs_BdRingUnAlloc(FGmacPs_BdRing *RingPtr, u32 NumBd,
                           FGmacPs_Bd *BdSetPtr);
LONG FGmacPs_BdRingToHw(FGmacPs_BdRing *RingPtr, u32 NumBd,
                        FGmacPs_Bd *BdSetPtr);
LONG FGmacPs_BdRingFree(FGmacPs_BdRing *RingPtr, u32 NumBd,
                        FGmacPs_Bd *BdSetPtr);
u32 FGmacPs_BdRingFromHwTx(FGmacPs_BdRing *RingPtr, u32 BdLimit,
                           FGmacPs_Bd **BdSetPtr);
u32 FGmacPs_BdRingFromHwRx(FGmacPs_BdRing *RingPtr, u32 BdLimit,
                           FGmacPs_Bd **BdSetPtr);
LONG FGmacPs_BdRingCheck(FGmacPs_BdRing *RingPtr, u8 Direction);

void FGmacPs_BdRingPtrReset(FGmacPs_BdRing *RingPtr, void *virtaddrloc);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macros */
