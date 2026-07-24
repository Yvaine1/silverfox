/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_lib.c
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

#ifndef FGMACPS_HW_H /* prevent circular inclusions */
#define FGMACPS_HW_H /* by using protection macros */

/***************************** Include Files *********************************/

#include "fmsh_common.h"
#include "fmsh_psu_parameters.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/

#define FGMACPS_MAX_MAC_ADDR                                     \
    4U                         /**< Maxmum number of mac address \
                                    supported */
#define FGMACPS_MAX_TYPE_ID 4U /**< Maxmum number of type id supported */

#ifdef __aarch64__
#define FGMACPS_BD_ALIGNMENT                     \
    64U /**< Minimum buffer descriptor alignment \
            on the local bus */
#else

#define FGMACPS_BD_ALIGNMENT                    \
    4U /**< Minimum buffer descriptor alignment \
            on the local bus */
#endif
#define FGMACPS_RX_BUF_ALIGNMENT                \
    4U /**< Minimum buffer alignment when using \
            options that impose alignment       \
            restrictions on the buffer data on  \
            the local bus */

/** @name Direction identifiers
 *
 *  These are used by several functions and callbacks that need
 *  to specify whether an operation specifies a send or receive channel.
 * @{
 */
#define FGMACPS_SEND 1U /**< send direction */
#define FGMACPS_RECV 2U /**< receive direction */
/*@}*/

/**  @name MDC clock division
 *  currently supporting 8, 16, 32, 48, 64, 96, 128, 224.
 * @{
 */
typedef enum {
    MDC_DIV_8 = 0U,
    MDC_DIV_16,
    MDC_DIV_32,
    MDC_DIV_48,
    MDC_DIV_64,
    MDC_DIV_96,
    MDC_DIV_128,
    MDC_DIV_224
} FGmacPs_MdcDiv;

/*@}*/

#define FGMACPS_RX_BUF_SIZE                       \
    1536U /**< Specify the receive buffer size in \
               bytes, 64, 128, ... 10240 */
#define FGMACPS_RX_BUF_SIZE_JUMBO 10240U

#define FGMACPS_RX_BUF_UNIT                      \
    64U /**< Number of receive buffer bytes as a \
             unit, this is HW setup */

#define FGMACPS_MAX_RXBD           128U /**< Size of RX buffer descriptor queues */
#define FGMACPS_MAX_TXBD           128U /**< Size of TX buffer descriptor queues */

#define FGMACPS_MAX_HASH_BITS      64U /**< Maximum value for hash bits. 2**6 */

/* Register offset definitions. Unless otherwise noted, register access is
 * 32 bit. Names are self explained here.
 */

#define FGMACPS_NWCTRL_OFFSET      0x00000000U /**< Network Control reg */
#define FGMACPS_NWCFG_OFFSET       0x00000004U /**< Network Config reg */
#define FGMACPS_NWSR_OFFSET        0x00000008U /**< Network Status reg */

#define FGMACPS_DMACR_OFFSET       0x00000010U /**< DMA Control reg */
#define FGMACPS_TXSR_OFFSET        0x00000014U /**< TX Status reg */
#define FGMACPS_RXQBASE_OFFSET     0x00000018U /**< RX Q Base address reg */
#define FGMACPS_TXQBASE_OFFSET     0x0000001CU /**< TX Q Base address reg */
#define FGMACPS_RXSR_OFFSET        0x00000020U /**< RX Status reg */

#define FGMACPS_ISR_OFFSET         0x00000024U /**< Interrupt Status reg */
#define FGMACPS_IER_OFFSET         0x00000028U /**< Interrupt Enable reg */
#define FGMACPS_IDR_OFFSET         0x0000002CU /**< Interrupt Disable reg */
#define FGMACPS_IMR_OFFSET         0x00000030U /**< Interrupt Mask reg */

#define FGMACPS_PHYMNTNC_OFFSET    0x00000034U /**< Phy Maintaince reg */
#define FGMACPS_RXPAUSE_OFFSET     0x00000038U /**< RX Pause Time reg */
#define FGMACPS_TXPAUSE_OFFSET     0x0000003CU /**< TX Pause Time reg */

#define FGMACPS_JUMBOMAXLEN_OFFSET 0x00000048U /**< Jumbo max length reg */

#define FGMACPS_HASHL_OFFSET       0x00000080U /**< Hash Low address reg */
#define FGMACPS_HASHH_OFFSET       0x00000084U /**< Hash High address reg */

#define FGMACPS_LADDR1L_OFFSET     0x00000088U /**< Specific1 addr low reg */
#define FGMACPS_LADDR1H_OFFSET     0x0000008CU /**< Specific1 addr high reg */
#define FGMACPS_LADDR2L_OFFSET     0x00000090U /**< Specific2 addr low reg */
#define FGMACPS_LADDR2H_OFFSET     0x00000094U /**< Specific2 addr high reg */
#define FGMACPS_LADDR3L_OFFSET     0x00000098U /**< Specific3 addr low reg */
#define FGMACPS_LADDR3H_OFFSET     0x0000009CU /**< Specific3 addr high reg */
#define FGMACPS_LADDR4L_OFFSET     0x000000A0U /**< Specific4 addr low reg */
#define FGMACPS_LADDR4H_OFFSET     0x000000A4U /**< Specific4 addr high reg */

#define FGMACPS_MATCH1_OFFSET      0x000000A8U /**< Type ID1 Match reg */
#define FGMACPS_MATCH2_OFFSET      0x000000ACU /**< Type ID2 Match reg */
#define FGMACPS_MATCH3_OFFSET      0x000000B0U /**< Type ID3 Match reg */
#define FGMACPS_MATCH4_OFFSET      0x000000B4U /**< Type ID4 Match reg */

#define FGMACPS_STRETCH_OFFSET     0x000000BCU /**< IPG Stretch reg */

#define FGMACPS_OCTTXL_OFFSET                                               \
    0x00000100U                                /**< Octects transmitted Low \
                                                    reg */
#define FGMACPS_OCTTXH_OFFSET                                                \
    0x00000104U                                /**< Octects transmitted High \
                                                    reg */

#define FGMACPS_TXCNT_OFFSET                                          \
    0x00000108U                                /**< Error-free Frmaes \
                                                    transmitted counter */
#define FGMACPS_TXBCCNT_OFFSET                                           \
    0x0000010CU                                /**< Error-free Broadcast \
                                                    Frames counter*/
#define FGMACPS_TXMCCNT_OFFSET                                           \
    0x00000110U                                /**< Error-free Multicast \
                                                    Frame counter */
#define FGMACPS_TXPAUSECNT_OFFSET                                            \
    0x00000114U                                /**< Pause Frames Transmitted \
                                                    Counter */
#define FGMACPS_TX64CNT_OFFSET                                                \
    0x00000118U                                /**< Error-free 64 byte Frames \
                                                    Transmitted counter */
#define FGMACPS_TX65CNT_OFFSET                                             \
    0x0000011CU                                /**< Error-free 65-127 byte \
                                                    Frames Transmitted     \
                                                    counter */
#define FGMACPS_TX128CNT_OFFSET                                             \
    0x00000120U                                /**< Error-free 128-255 byte \
                                                    Frames Transmitted      \
                                                    counter*/
#define FGMACPS_TX256CNT_OFFSET                                             \
    0x00000124U                                /**< Error-free 256-511 byte \
                                                    Frames transmitted      \
                                                    counter */
#define FGMACPS_TX512CNT_OFFSET                                              \
    0x00000128U                                /**< Error-free 512-1023 byte \
                                                    Frames transmitted       \
                                                    counter */
#define FGMACPS_TX1024CNT_OFFSET                                              \
    0x0000012CU                                /**< Error-free 1024-1518 byte \
                                                    Frames transmitted        \
                                                    counter */
#define FGMACPS_TX1519CNT_OFFSET                                           \
    0x00000130U                                /**< Error-free larger than \
                                                    1519 byte Frames       \
                                                    transmitted counter */
#define FGMACPS_TXURUNCNT_OFFSET                                       \
    0x00000134U                                /**< TX under run error \
                                                    counter */

#define FGMACPS_SNGLCOLLCNT_OFFSET                                         \
    0x00000138U                                /**< Single Collision Frame \
                                                    Counter */
#define FGMACPS_MULTICOLLCNT_OFFSET                                          \
    0x0000013CU                                /**< Multiple Collision Frame \
                                                    Counter */
#define FGMACPS_EXCESSCOLLCNT_OFFSET                                          \
    0x00000140U                                /**< Excessive Collision Frame \
                                                    Counter */
#define FGMACPS_LATECOLLCNT_OFFSET                                       \
    0x00000144U                                /**< Late Collision Frame \
                                                    Counter */
#define FGMACPS_TXDEFERCNT_OFFSET                                         \
    0x00000148U                                /**< Deferred Transmission \
                                                    Frame Counter */
#define FGMACPS_TXCSENSECNT_OFFSET                                         \
    0x0000014CU                                /**< Transmit Carrier Sense \
                                                    Error Counter */

#define FGMACPS_OCTRXL_OFFSET                                                 \
    0x00000150U                                /**< Octects Received register \
                                                    Low */
#define FGMACPS_OCTRXH_OFFSET                                                 \
    0x00000154U                                /**< Octects Received register \
                                                    High */

#define FGMACPS_RXCNT_OFFSET                                          \
    0x00000158U                                /**< Error-free Frames \
                                                    Received Counter */
#define FGMACPS_RXBROADCNT_OFFSET                                        \
    0x0000015CU                                /**< Error-free Broadcast \
                                                    Frames Received Counter */
#define FGMACPS_RXMULTICNT_OFFSET                                        \
    0x00000160U                                /**< Error-free Multicast \
                                                    Frames Received Counter */
#define FGMACPS_RXPAUSECNT_OFFSET                                \
    0x00000164U                                /**< Pause Frames \
                                                    Received Counter */
#define FGMACPS_RX64CNT_OFFSET                                                \
    0x00000168U                                /**< Error-free 64 byte Frames \
                                                    Received Counter */
#define FGMACPS_RX65CNT_OFFSET                                             \
    0x0000016CU                                /**< Error-free 65-127 byte \
                                                    Frames Received Counter */
#define FGMACPS_RX128CNT_OFFSET                                             \
    0x00000170U                                /**< Error-free 128-255 byte \
                                                    Frames Received Counter */
#define FGMACPS_RX256CNT_OFFSET                                             \
    0x00000174U                                /**< Error-free 256-512 byte \
                                                    Frames Received Counter */
#define FGMACPS_RX512CNT_OFFSET                                              \
    0x00000178U                                /**< Error-free 512-1023 byte \
                                                    Frames Received Counter */
#define FGMACPS_RX1024CNT_OFFSET                                              \
    0x0000017CU                                /**< Error-free 1024-1518 byte \
                                                    Frames Received Counter */
#define FGMACPS_RX1519CNT_OFFSET                                             \
    0x00000180U                                /**< Error-free 1519-max byte \
                                                    Frames Received Counter */
#define FGMACPS_RXUNDRCNT_OFFSET                                              \
    0x00000184U                                /**< Undersize Frames Received \
                                                    Counter */
#define FGMACPS_RXOVRCNT_OFFSET                                              \
    0x00000188U                                /**< Oversize Frames Received \
                                                    Counter */
#define FGMACPS_RXJABCNT_OFFSET                                      \
    0x0000018CU                                /**< Jabbers Received \
                                                    Counter */
#define FGMACPS_RXFCSCNT_OFFSET                                          \
    0x00000190U                                /**< Frame Check Sequence \
                                                    Error Counter */
#define FGMACPS_RXLENGTHCNT_OFFSET                                     \
    0x00000194U                                /**< Length Field Error \
                                                    Counter */
#define FGMACPS_RXSYMBCNT_OFFSET  0x00000198U  /**< Symbol Error Counter */
#define FGMACPS_RXALIGNCNT_OFFSET 0x0000019CU  /**< Alignment Error Counter */
#define FGMACPS_RXRESERRCNT_OFFSET                                         \
    0x000001A0U                                /**< Receive Resource Error \
                                                    Counter */
#define FGMACPS_RXORCNT_OFFSET 0x000001A4U     /**< Receive Overrun Counter */
#define FGMACPS_RXIPCCNT_OFFSET                                              \
    0x000001A8U                                /**< IP header Checksum Error \
                                                    Counter */
#define FGMACPS_RXTCPCCNT_OFFSET                                       \
    0x000001ACU                                /**< TCP Checksum Error \
                                                    Counter */
#define FGMACPS_RXUDPCCNT_OFFSET                                       \
    0x000001B0U                                /**< UDP Checksum Error \
                                                    Counter */
#define FGMACPS_LAST_OFFSET                                                \
    0x000001B4U                                /**< Last statistic counter \
                            offset, for clearing */

#define FGMACPS_1588_SEC_OFFSET 0x000001D0U    /**< 1588 second counter */
#define FGMACPS_1588_NANOSEC_OFFSET                                         \
    0x000001D4U                                /**< 1588 nanosecond counter \
                                                */
#define FGMACPS_1588_ADJ_OFFSET                                     \
    0x000001D8U                                /**< 1588 nanosecond \
                            adjustment counter */
#define FGMACPS_1588_INC_OFFSET                                     \
    0x000001DCU                                /**< 1588 nanosecond \
                            increment counter */
#define FGMACPS_PTP_TXSEC_OFFSET                                             \
    0x000001E0U                                /**< 1588 PTP transmit second \
                            counter */
#define FGMACPS_PTP_TXNANOSEC_OFFSET                                  \
    0x000001E4U                                /**< 1588 PTP transmit \
                            nanosecond counter */
#define FGMACPS_PTP_RXSEC_OFFSET                                            \
    0x000001E8U                                /**< 1588 PTP receive second \
                            counter */
#define FGMACPS_PTP_RXNANOSEC_OFFSET                                 \
    0x000001ECU                                /**< 1588 PTP receive \
                            nanosecond counter */
#define FGMACPS_PTPP_TXSEC_OFFSET                                          \
    0x000001F0U                                /**< 1588 PTP peer transmit \
                            second counter */
#define FGMACPS_PTPP_TXNANOSEC_OFFSET                                      \
    0x000001F4U                                /**< 1588 PTP peer transmit \
                           nanosecond counter */
#define FGMACPS_PTPP_RXSEC_OFFSET                                         \
    0x000001F8U                                /**< 1588 PTP peer receive \
                            second counter */
#define FGMACPS_PTPP_RXNANOSEC_OFFSET                                     \
    0x000001FCU                                /**< 1588 PTP peer receive \
                           nanosecond counter */

#define FGMACPS_INTQ1_STS_OFFSET                                        \
    0x00000400U                                /**< Interrupt Q1 Status \
                          reg */
#define FGMACPS_TXQ1BASE_OFFSET                                        \
    0x00000440U                                /**< TX Q1 Base address \
                          reg */
#define FGMACPS_RXQ1BASE_OFFSET                                        \
    0x00000480U                                /**< RX Q1 Base address \
                          reg */
#define FGMACPS_MSBBUF_TXQBASE_OFFSET                                    \
    0x000004C8U                                /**< MSB Buffer TX Q Base \
                        reg */
#define FGMACPS_MSBBUF_RXQBASE_OFFSET                                    \
    0x000004D4U                                /**< MSB Buffer RX Q Base \
                        reg */
#define FGMACPS_INTQ1_IER_OFFSET                                        \
    0x00000600U                                /**< Interrupt Q1 Enable \
                          reg */
#define FGMACPS_INTQ1_IDR_OFFSET                                         \
    0x00000620U                                /**< Interrupt Q1 Disable \
                          reg */
#define FGMACPS_INTQ1_IMR_OFFSET                                      \
    0x00000640U                                /**< Interrupt Q1 Mask \
                          reg */

/* Define some bit positions for registers. */

/** @name network control register bit definitions
 * @{
 */
#define FGMACPS_NWCTRL_FLUSH_DPRAM_MASK                                  \
    0x00040000U                                 /**< Flush a packet from \
                        Rx SRAM */
#define FGMACPS_NWCTRL_ZEROPAUSETX_MASK                                    \
    0x00000800U                                 /**< Transmit zero quantum \
                                                     pause frame */
#define FGMACPS_NWCTRL_PAUSETX_MASK 0x00000800U /**< Transmit pause frame */
#define FGMACPS_NWCTRL_HALTTX_MASK                                     \
    0x00000400U                                 /**< Halt transmission \
                                                     after current frame */
#define FGMACPS_NWCTRL_STARTTX_MASK 0x00000200U /**< Start tx (tx_go) */

#define FGMACPS_NWCTRL_STATWEN_MASK                                    \
    0x00000080U                                 /**< Enable writing to \
                                                     stat counters */
#define FGMACPS_NWCTRL_STATINC_MASK                                      \
    0x00000040U                                 /**< Increment statistic \
                                                     registers */
#define FGMACPS_NWCTRL_STATCLR_MASK                                  \
    0x00000020U                                 /**< Clear statistic \
                                                     registers */
#define FGMACPS_NWCTRL_MDEN_MASK   0x00000010U  /**< Enable MDIO port */
#define FGMACPS_NWCTRL_TXEN_MASK   0x00000008U  /**< Enable transmit */
#define FGMACPS_NWCTRL_RXEN_MASK   0x00000004U  /**< Enable receive */
#define FGMACPS_NWCTRL_LOOPEN_MASK 0x00000002U  /**< local loopback */
/*@}*/

/** @name network configuration register bit definitions
 * @{
 */
#define FGMACPS_NWCFG_BADPREAMBEN_MASK                                      \
    0x20000000U                                   /**< disable rejection of \
                                                       non-standard preamble */
#define FGMACPS_NWCFG_IPDSTRETCH_MASK 0x10000000U /**< enable transmit IPG */
#define FGMACPS_NWCFG_SGMIIEN_MASK    0x08000000U /**< SGMII Enable */
#define FGMACPS_NWCFG_FCSIGNORE_MASK                                        \
    0x04000000U                                   /**< disable rejection of \
                                                       FCS error */
#define FGMACPS_NWCFG_HDRXEN_MASK 0x02000000U     /**< RX half duplex */
#define FGMACPS_NWCFG_RXCHKSUMEN_MASK                                     \
    0x01000000U                                   /**< enable RX checksum \
                                                       offload */
#define FGMACPS_NWCFG_PAUSECOPYDI_MASK                                   \
    0x00800000U                                   /**< Do not copy pause \
                                                       Frames to memory */
#define FGMACPS_NWCFG_DWIDTH_64_MASK 0x00200000U  /**< 64 bit Data bus width */
#define FGMACPS_NWCFG_MDC_SHIFT_MASK 18U          /**< shift bits for MDC */
#define FGMACPS_NWCFG_MDCCLKDIV_MASK 0x001C0000U  /**< MDC Mask PCLK divisor */
#define FGMACPS_NWCFG_FCSREM_MASK                                       \
    0x00020000U                                   /**< Discard FCS from \
                                                       received frames */
#define FGMACPS_NWCFG_LENERRDSCRD_MASK 0x00010000U
/**< RX length error discard */
#define FGMACPS_NWCFG_RXOFFS_MASK      0x0000C000U /**< RX buffer offset */
#define FGMACPS_NWCFG_PAUSEEN_MASK     0x00002000U /**< Enable pause RX */
#define FGMACPS_NWCFG_RETRYTESTEN_MASK 0x00001000U /**< Retry test */
#define FGMACPS_NWCFG_XTADDMACHEN_MASK 0x00000200U
/**< External address match enable */
#define FGMACPS_NWCFG_PCSSEL_MASK      0x00000800U /**< PCS Select */
#define FGMACPS_NWCFG_1000_MASK        0x00000400U /**< 1000 Mbps */
#define FGMACPS_NWCFG_1536RXEN_MASK                                      \
    0x00000100U                                    /**< Enable 1536 byte \
                                                        frames reception */
#define FGMACPS_NWCFG_UCASTHASHEN_MASK                                       \
    0x00000080U                                    /**< Receive unicast hash \
                                                        frames */
#define FGMACPS_NWCFG_MCASTHASHEN_MASK                                         \
    0x00000040U                                    /**< Receive multicast hash \
                                                        frames */
#define FGMACPS_NWCFG_BCASTDI_MASK                                     \
    0x00000020U                                    /**< Do not receive \
                                                        broadcast frames */
#define FGMACPS_NWCFG_COPYALLEN_MASK 0x00000010U   /**< Copy all frames */
#define FGMACPS_NWCFG_JUMBO_MASK     0x00000008U   /**< Jumbo frames */
#define FGMACPS_NWCFG_NVLANDISC_MASK                                      \
    0x00000004U                                    /**< Receive only VLAN \
                                                        frames */
#define FGMACPS_NWCFG_FDEN_MASK    0x00000002U     /**< full duplex */
#define FGMACPS_NWCFG_100_MASK     0x00000001U     /**< 100 Mbps */
#define FGMACPS_NWCFG_RESET_MASK   0x00080000U     /**< reset value */
/*@}*/

/** @name network status register bit definitaions
 * @{
 */
#define FGMACPS_NWSR_MDIOIDLE_MASK 0x00000004U /**< PHY management idle */
#define FGMACPS_NWSR_MDIO_MASK     0x00000002U /**< Status of mdio_in */
/*@}*/

/** @name MAC address register word 1 mask
 * @{
 */
#define FGMACPS_LADDR_MACH_MASK          \
    0x0000FFFFU /**< Address bits[47:32] \
                   bit[31:0] are in BOTTOM */
/*@}*/

/** @name DMA control register bit definitions
 * @{
 */
#define FGMACPS_DMACR_ADDR_WIDTH_64 0x40000000U /**< 64 bit address bus */
#define FGMACPS_DMACR_TXEXTEND_MASK 0x20000000U /**< Tx Extended desc mode */
#define FGMACPS_DMACR_RXEXTEND_MASK 0x10000000U /**< Rx Extended desc mode */
#define FGMACPS_DMACR_RXBUF_MASK                                            \
    0x00FF0000U                                 /**< Mask bit for RX buffer \
                                                size */
#define FGMACPS_DMACR_RXBUF_SHIFT                                            \
    16U                                         /**< Shift bit for RX buffer \
                                                    size */
#define FGMACPS_DMACR_TCPCKSUM_MASK                                    \
    0x00000800U                                 /**< enable/disable TX \
                                                    checksum offload */
#define FGMACPS_DMACR_TXSIZE_MASK      0x00000400U /**< TX buffer memory size */
#define FGMACPS_DMACR_RXSIZE_MASK      0x00000300U /**< RX buffer memory size */
#define FGMACPS_DMACR_ENDIAN_MASK      0x00000080U /**< endian configuration */
#define FGMACPS_DMACR_BLENGTH_MASK     0x0000001FU /**< buffer burst length */
#define FGMACPS_DMACR_SINGLE_AHB_BURST 0x00000001U /**< single AHB bursts */
#define FGMACPS_DMACR_INCR4_AHB_BURST  0x00000004U /**< 4 bytes AHB bursts */
#define FGMACPS_DMACR_INCR8_AHB_BURST  0x00000008U /**< 8 bytes AHB bursts */
#define FGMACPS_DMACR_INCR16_AHB_BURST 0x00000010U /**< 16 bytes AHB bursts */
/*@}*/

/** @name transmit status register bit definitions
 * @{
 */
#define FGMACPS_TXSR_HRESPNOK_MASK     0x00000100U /**< Transmit hresp not OK */
#define FGMACPS_TXSR_URUN_MASK         0x00000040U /**< Transmit underrun */
#define FGMACPS_TXSR_TXCOMPL_MASK      0x00000020U /**< Transmit completed OK */
#define FGMACPS_TXSR_BUFEXH_MASK                                             \
    0x00000010U                                /**< Transmit buffs exhausted \
                                                    mid frame */
#define FGMACPS_TXSR_TXGO_MASK     0x00000008U /**< Status of go flag */
#define FGMACPS_TXSR_RXOVR_MASK    0x00000004U /**< Retry limit exceeded */
#define FGMACPS_TXSR_FRAMERX_MASK  0x00000002U /**< Collision tx frame */
#define FGMACPS_TXSR_USEDREAD_MASK 0x00000001U /**< TX buffer used bit set */

#define FGMACPS_TXSR_ERROR_MASK                                      \
    ((u32)FGMACPS_TXSR_HRESPNOK_MASK | (u32)FGMACPS_TXSR_URUN_MASK | \
     (u32)FGMACPS_TXSR_BUFEXH_MASK | (u32)FGMACPS_TXSR_RXOVR_MASK |  \
     (u32)FGMACPS_TXSR_FRAMERX_MASK | (u32)FGMACPS_TXSR_USEDREAD_MASK)
/*@}*/

/**
 * @name receive status register bit definitions
 * @{
 */
#define FGMACPS_RXSR_HRESPNOK_MASK 0x00000008U /**< Receive hresp not OK */
#define FGMACPS_RXSR_RXOVR_MASK    0x00000004U /**< Receive overrun */
#define FGMACPS_RXSR_FRAMERX_MASK  0x00000002U /**< Frame received OK */
#define FGMACPS_RXSR_BUFFNA_MASK   0x00000001U /**< RX buffer used bit set */

#define FGMACPS_RXSR_ERROR_MASK                                       \
    ((u32)FGMACPS_RXSR_HRESPNOK_MASK | (u32)FGMACPS_RXSR_RXOVR_MASK | \
     (u32)FGMACPS_RXSR_BUFFNA_MASK)
/*@}*/

/**
 * @name Interrupt Q1 status register bit definitions
 * @{
 */
#define FGMACPS_INTQ1SR_TXCOMPL_MASK 0x00000080U /**< Transmit completed OK */
#define FGMACPS_INTQ1SR_TXERR_MASK   0x00000040U /**< Transmit AMBA Error */

#define FGMACPS_INTQ1_IXR_ALL_MASK \
    ((u32)FGMACPS_INTQ1SR_TXCOMPL_MASK | (u32)FGMACPS_INTQ1SR_TXERR_MASK)

/*@}*/

/**
 * @name interrupts bit definitions
 * Bits definitions are same in FGMACPS_ISR_OFFSET,
 * FGMACPS_IER_OFFSET, FGMACPS_IDR_OFFSET, and FGMACPS_IMR_OFFSET
 * @{
 */
#define FGMACPS_IXR_PTPPSTX_MASK  0x02000000U   /**< PTP Pdelay_resp TXed */
#define FGMACPS_IXR_PTPPDRTX_MASK 0x01000000U   /**< PTP Pdelay_req TXed */
#define FGMACPS_IXR_PTPPSRX_MASK  0x00800000U   /**< PTP Pdelay_resp RXed */
#define FGMACPS_IXR_PTPPDRRX_MASK 0x00400000U   /**< PTP Pdelay_req RXed */

#define FGMACPS_IXR_PTPSTX_MASK   0x00200000U   /**< PTP Sync TXed */
#define FGMACPS_IXR_PTPDRTX_MASK  0x00100000U   /**< PTP Delay_req TXed */
#define FGMACPS_IXR_PTPSRX_MASK   0x00080000U   /**< PTP Sync RXed */
#define FGMACPS_IXR_PTPDRRX_MASK  0x00040000U   /**< PTP Delay_req RXed */

#define FGMACPS_IXR_PAUSETX_MASK  0x00004000U   /**< Pause frame transmitted */
#define FGMACPS_IXR_PAUSEZERO_MASK                                          \
    0x00002000U                                 /**< Pause time has reached \
                                                     zero */
#define FGMACPS_IXR_PAUSENZERO_MASK 0x00001000U /**< Pause frame received */
#define FGMACPS_IXR_HRESPNOK_MASK   0x00000800U /**< hresp not ok */
#define FGMACPS_IXR_RXOVR_MASK      0x00000400U /**< Receive overrun occurred */
#define FGMACPS_IXR_TXCOMPL_MASK    0x00000080U /**< Frame transmitted ok */
#define FGMACPS_IXR_TXEXH_MASK                                                \
    0x00000040U                                 /**< Transmit err occurred or \
                                                     no buffers*/
#define FGMACPS_IXR_RETRY_MASK   0x00000020U    /**< Retry limit exceeded */
#define FGMACPS_IXR_URUN_MASK    0x00000010U    /**< Transmit underrun */
#define FGMACPS_IXR_TXUSED_MASK  0x00000008U    /**< Tx buffer used bit read */
#define FGMACPS_IXR_RXUSED_MASK  0x00000004U    /**< Rx buffer used bit read */
#define FGMACPS_IXR_FRAMERX_MASK 0x00000002U    /**< Frame received ok */
#define FGMACPS_IXR_MGMNT_MASK   0x00000001U    /**< PHY management complete */
#define FGMACPS_IXR_ALL_MASK     0x00007FFFU    /**< Everything! */

#define FGMACPS_IXR_TX_ERR_MASK                                  \
    ((u32)FGMACPS_IXR_TXEXH_MASK | (u32)FGMACPS_IXR_RETRY_MASK | \
     (u32)FGMACPS_IXR_URUN_MASK)

#define FGMACPS_IXR_RX_ERR_MASK                                      \
    ((u32)FGMACPS_IXR_HRESPNOK_MASK | (u32)FGMACPS_IXR_RXUSED_MASK | \
     (u32)FGMACPS_IXR_RXOVR_MASK)

/*@}*/

/** @name PHY Maintenance bit definitions
 * @{
 */
#define FGMACPS_PHYMNTNC_OP_MASK       0x40020000U /**< operation mask bits */
#define FGMACPS_PHYMNTNC_OP_R_MASK     0x20000000U /**< read operation */
#define FGMACPS_PHYMNTNC_OP_W_MASK     0x10000000U /**< write operation */
#define FGMACPS_PHYMNTNC_ADDR_MASK     0x0F800000U /**< Address bits */
#define FGMACPS_PHYMNTNC_REG_MASK      0x007C0000U /**< register bits */
#define FGMACPS_PHYMNTNC_DATA_MASK     0x00000FFFU /**< data bits */
#define FGMACPS_PHYMNTNC_PHAD_SHFT_MSK 23U         /**< Shift bits for PHYAD */
#define FGMACPS_PHYMNTNC_PREG_SHFT_MSK 18U         /**< Shift bits for PHREG */
/*@}*/

/* Transmit buffer descriptor status words offset
 * @{
 */
#define FGMACPS_BD_ADDR_OFFSET         0x00000000U /**< word 0/addr of BDs */
#define FGMACPS_BD_STAT_OFFSET         0x00000004U /**< word 1/status of BDs */
#define FGMACPS_BD_ADDR_HI_OFFSET      0x00000008U /**< word 2/addr of BDs */

/*
 * @}
 */

/* Transmit buffer descriptor status words bit positions.
 * Transmit buffer descriptor consists of two 32-bit registers,
 * the first - word0 contains a 32-bit address pointing to the location of
 * the transmit data.
 * The following register - word1, consists of various information to control
 * the FGmacPs transmit process.  After transmit, this is updated with status
 * information, whether the frame was transmitted OK or why it had failed.
 * @{
 */
#define FGMACPS_TXBUF_USED_MASK        0x80000000U /**< Used bit. */
#define FGMACPS_TXBUF_WRAP_MASK        0x40000000U /**< Wrap bit, last descriptor */
#define FGMACPS_TXBUF_RETRY_MASK       0x20000000U /**< Retry limit exceeded */
#define FGMACPS_TXBUF_URUN_MASK        0x10000000U /**< Transmit underrun occurred */
#define FGMACPS_TXBUF_EXH_MASK         0x08000000U /**< Buffers exhausted */
#define FGMACPS_TXBUF_TCP_MASK         0x04000000U /**< Late collision. */
#define FGMACPS_TXBUF_NOCRC_MASK       0x00010000U /**< No CRC */
#define FGMACPS_TXBUF_LAST_MASK        0x00008000U /**< Last buffer */
#define FGMACPS_TXBUF_LEN_MASK         0x00003FFFU /**< Mask for length field */
/*
 * @}
 */

/* Receive buffer descriptor status words bit positions.
 * Receive buffer descriptor consists of two 32-bit registers,
 * the first - word0 contains a 32-bit word aligned address pointing to the
 * address of the buffer. The lower two bits make up the wrap bit indicating
 * the last descriptor and the ownership bit to indicate it has been used by
 * the FGmacPs.
 * The following register - word1, contains status information regarding why
 * the frame was received (the filter match condition) as well as other
 * useful info.
 * @{
 */
#define FGMACPS_RXBUF_BCAST_MASK       0x80000000U /**< Broadcast frame */
#define FGMACPS_RXBUF_MULTIHASH_MASK                                           \
    0x40000000U                                    /**< Multicast hashed frame \
                                                    */
#define FGMACPS_RXBUF_UNIHASH_MASK 0x20000000U     /**< Unicast hashed frame */
#define FGMACPS_RXBUF_EXH_MASK     0x08000000U     /**< buffer exhausted */
#define FGMACPS_RXBUF_AMATCH_MASK                                        \
    0x06000000U                                    /**< Specific address \
                                                        matched */
#define FGMACPS_RXBUF_IDFOUND_MASK   0x01000000U   /**< Type ID matched */
#define FGMACPS_RXBUF_IDMATCH_MASK   0x00C00000U   /**< ID matched mask */
#define FGMACPS_RXBUF_VLAN_MASK      0x00200000U   /**< VLAN tagged */
#define FGMACPS_RXBUF_PRI_MASK       0x00100000U   /**< Priority tagged */
#define FGMACPS_RXBUF_VPRI_MASK      0x000E0000U   /**< Vlan priority */
#define FGMACPS_RXBUF_CFI_MASK       0x00010000U   /**< CFI frame */
#define FGMACPS_RXBUF_EOF_MASK       0x00008000U   /**< End of frame. */
#define FGMACPS_RXBUF_SOF_MASK       0x00004000U   /**< Start of frame. */
#define FGMACPS_RXBUF_LEN_MASK       0x00001FFFU   /**< Mask for length field */
#define FGMACPS_RXBUF_LEN_JUMBO_MASK 0x00003FFFU   /**< Mask for jumbo length */

#define FGMACPS_RXBUF_WRAP_MASK      0x00000002U   /**< Wrap bit, last BD */
#define FGMACPS_RXBUF_NEW_MASK       0x00000001U   /**< Used bit.. */
#define FGMACPS_RXBUF_ADD_MASK       0xFFFFFFFCU   /**< Mask for address */
/*
 * @}
 */

/*
 * Define appropriate I/O access method to memory mapped I/O or other
 * interface if necessary.
 */

#define FGmacPs_In32                 FMSH_IN32_32
#define FGmacPs_Out32                FMSH_OUT32_32

/****************************************************************************/
/**
 *
 * Read the given register.
 *
 * @param    BaseAddress is the base address of the device
 * @param    RegOffset is the register offset to be read
 *
 * @return   The 32-bit value of the register
 *
 * @note
 * C-style signature:
 *    u32 FGmacPs_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 *****************************************************************************/
#define FGmacPs_ReadReg(BaseAddress, RegOffset) \
    FGmacPs_In32((BaseAddress) + (u32)(RegOffset))

/****************************************************************************/
/**
 *
 * Write the given register.
 *
 * @param    BaseAddress is the base address of the device
 * @param    RegOffset is the register offset to be written
 * @param    Data is the 32-bit value to write to the register
 *
 * @return   None.
 *
 * @note
 * C-style signature:
 *    void FGmacPs_WriteReg(u32 BaseAddress, u32 RegOffset,
 *         u32 Data)
 *
 *****************************************************************************/
#define FGmacPs_WriteReg(BaseAddress, RegOffset, Data) \
    FGmacPs_Out32((u32)(Data), (BaseAddress) + (u32)(RegOffset))

/************************** Function Prototypes *****************************/
/*
 * Perform reset operation to the gmacps interface
 */
void FGmacPs_ResetHw(u32 BaseAddr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
