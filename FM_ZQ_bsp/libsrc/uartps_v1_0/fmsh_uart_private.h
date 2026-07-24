/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fuartps_private.h
 *
 * This file contains ......
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   wfb  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_UART_PRIVATE_H_ /* prevent circular inclusions */
#define _FMSH_UART_PRIVATE_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/
#include "fmsh_uart_common.h"

/************************** Constant Definitions *****************************/

/* This macro is used to hardcode the APB data accesses */
#define UART_INP(p)                      (uint32_t) * ((uint32_t *)(&p))
#define UART_OUTP(v, p)                  *((volatile uint32_t *)(&p)) = (v)

/**
 * DESCRIPTION
 *  Used in conjunction with bitops.h to access register bitfields.
 *  They are defined as bit offset/mask tuples for each uart register
 *  bitfield.
 * NOTES
 *  bfo is the offset of the bitfield with respect to LSB;
 *  bfw is the width of the bitfield
 */
// interrupt enable register
#define bfoUART_IER_ERBFI                0  // received data available
#define bfwUART_IER_ERBFI                1
#define bfoUART_IER_ETBEI                1  // transmitter holding
#define bfwUART_IER_ETBEI                1  //  register
#define bfoUART_IER_ELSI                 2  // receiver line status
#define bfwUART_IER_ELSI                 1
#define bfoUART_IER_EDSSI                3  // modem status
#define bfwUART_IER_EDSSI                1
#define bfoUART_IER_PTIME                7  // programmable THRE
#define bfwUART_IER_PTIME                1  //  interrupt mode
// interrupt identity register
#define bfoUART_IIR_INTERRUPT_ID         0  // interrupt identity
#define bfwUART_IIR_INTERRUPT_ID         4
#define bfoUART_IIR_FIFO_STATUS          6  // FIFO status
#define bfwUART_IIR_FIFO_STATUS          2
// FIFO control register
#define bfoUART_FCR_FIFO_ENABLE          0  // FIFO enable
#define bfwUART_FCR_FIFO_ENABLE          1
#define bfoUART_FCR_RCVR_FIFO_RESET      1  // receiver FIFO reset
#define bfwUART_FCR_RCVR_FIFO_RESET      1
#define bfoUART_FCR_XMIT_FIFO_RESET      2  // transmitter FIFO reset
#define bfwUART_FCR_XMIT_FIFO_RESET      1
#define bfoUART_FCR_DMA_MODE             3  // dma mode
#define bfwUART_FCR_DMA_MODE             1
#define bfoUART_FCR_TX_EMPTY_TRIGGER     4  // transmitter empty trigger
#define bfwUART_FCR_TX_EMPTY_TRIGGER     2
#define bfoUART_FCR_RCVR_TRIGGER         6  // receiver trigger
#define bfwUART_FCR_RCVR_TRIGGER         2
// line control register
#define bfoUART_LCR_CLS                  0  // number of bits per
#define bfwUART_LCR_CLS                  2  //  character
#define bfoUART_LCR_STOP                 2  // stop bits
#define bfwUART_LCR_STOP                 1
#define bfoUART_LCR_PEN                  3  // parity enable
#define bfwUART_LCR_PEN                  1
#define bfoUART_LCR_EPS                  4  // parity select
#define bfwUART_LCR_EPS                  1
#define bfoUART_LCR_PARITY               3  // composite enable and
#define bfwUART_LCR_PARITY               2  //  select field
#define bfoUART_LCR_LINE                 0  // composite line
#define bfwUART_LCR_LINE                 5  //   control field
#define bfoUART_LCR_STICK_PARITY         5  // unsupported
#define bfwUART_LCR_STICK_PARITY         1  // unsupported
#define bfoUART_LCR_BREAK                6  // line break
#define bfwUART_LCR_BREAK                1
#define bfoUART_LCR_DLAB                 7  // divisor latch address bit
#define bfwUART_LCR_DLAB                 1
// modem control register
#define bfoUART_MCR_DTR                  0  // dtr
#define bfwUART_MCR_DTR                  1
#define bfoUART_MCR_RTS                  1  // rts
#define bfwUART_MCR_RTS                  1
#define bfoUART_MCR_OUT1                 2  // out1
#define bfwUART_MCR_OUT1                 1
#define bfoUART_MCR_OUT2                 3  // out2
#define bfwUART_MCR_OUT2                 1
#define bfoUART_MCR_LOOPBACK             4  // loopback
#define bfwUART_MCR_LOOPBACK             1
#define bfoUART_MCR_AFCE                 5  // automatic flow control
#define bfwUART_MCR_AFCE                 1
#define bfoUART_MCR_SIRE                 6  // serial infra-red
#define bfwUART_MCR_SIRE                 1
// line status register
#define bfoUART_LSR_DR                   0  // data ready
#define bfwUART_LSR_DR                   1
#define bfoUART_LSR_OE                   1  // overrun error
#define bfwUART_LSR_OE                   1
#define bfoUART_LSR_PE                   2  // parity error
#define bfwUART_LSR_PE                   1
#define bfoUART_LSR_FE                   3  // framing error
#define bfwUART_LSR_FE                   1
#define bfoUART_LSR_BI                   4  // break interrupt
#define bfwUART_LSR_BI                   1
#define bfoUART_LSR_THRE                 5  // transmitter holding
#define bfwUART_LSR_THRE                 1  //  register empty
#define bfoUART_LSR_TEMT                 6  // transmitter empty
#define bfwUART_LSR_TEMT                 1
#define bfoUART_LSR_RX_FIFO_ERROR        7  // receiver FIFO error
#define bfwUART_LSR_RX_FIFO_ERROR        1
// modem status register
#define bfoUART_MSR_DCTS                 0  // dcts
#define bfwUART_MSR_DCTS                 1
#define bfoUART_MSR_DDSR                 1  // ddsr
#define bfwUART_MSR_DDSR                 1
#define bfoUART_MSR_TERI                 2  // teri
#define bfwUART_MSR_TERI                 1
#define bfoUART_MSR_DDCD                 3  // ddcd
#define bfwUART_MSR_DDCD                 1
#define bfoUART_MSR_CTS                  4  // cts
#define bfwUART_MSR_CTS                  1
#define bfoUART_MSR_DSR                  5  // dsr
#define bfwUART_MSR_DSR                  1
#define bfoUART_MSR_RI                   6  // ri
#define bfwUART_MSR_RI                   1
#define bfoUART_MSR_DCD                  7  // dcd
#define bfwUART_MSR_DCD                  1
// uart status register
#define bfoUART_USR_BUSY                 0  // serial transfer is in
#define bfwUART_USR_BUSY                 1  //  progress
#define bfoUART_USR_TFNF                 1  // Tx FIFO not full
#define bfwUART_USR_TFNF                 1
#define bfoUART_USR_TFE                  2  // Tx FIFO empty
#define bfwUART_USR_TFE                  1
#define bfoUART_USR_RFNE                 3  // Rx FIFO not empty
#define bfwUART_USR_RFNE                 1
#define bfoUART_USR_RFF                  4  // Rx FIFO full
#define bfwUART_USR_RFF                  1
// transmit FIFO level register
#define bfoUART_TFL_LEVEL                0  // Tx FIFO data level
#define bfwUART_TFL_LEVEL                8
// receive FIFO level register
#define bfoUART_RFL_LEVEL                0  // Rx FIFO data level
#define bfwUART_RFL_LEVEL                8
// software reset register
#define bfoUART_SRR_UR                   0  // UART reset
#define bfwUART_SRR_UR                   1
#define bfoUART_SRR_RFR                  1  // Rx FIFO reset
#define bfwUART_SRR_RFR                  1
#define bfoUART_SRR_XFR                  2  // Tx FIFO reset
#define bfwUART_SRR_XFR                  1
// shadow request to send register
#define bfoUART_SRTS_RTS                 0  // shadow request to send
#define bfwUART_SRTS_RTS                 1
// shadow break control register
#define bfoUART_SBCR_BCR                 0  // shadow break control
#define bfwUART_SBCR_BCR                 1
// shadow FIFO enable register
#define bfoUART_SFE_FE                   0  // shadow FIFO enable
#define bfwUART_SFE_FE                   1
// shadow receiver trigger register
#define bfoUART_SRT_TRIGGER              0  // shadow Rx FIFO full
#define bfwUART_SRT_TRIGGER              2  //  trigger
// shadow transmitter empty trigger register
#define bfoUART_STET_TRIGGER             0  // shadow Tx FIFO empty
#define bfwUART_STET_TRIGGER             2  //  trigger
// halt transmitter register
#define bfoUART_HTX_HALT                 0  // halt transmitter
#define bfwUART_HTX_HALT                 1
// DMA software acknowledge register
#define bfoUART_DMASA_ACK                0  // DMA software acknowledge
#define bfwUART_DMASA_ACK                1
// uart component parameters
#define bfoUART_PARAM_DATA_WIDTH         0
#define bfwUART_PARAM_DATA_WIDTH         2
#define bfoUART_PARAM_AFCE_MODE          4
#define bfwUART_PARAM_AFCE_MODE          1
#define bfoUART_PARAM_THRE_MODE          5
#define bfwUART_PARAM_THRE_MODE          1
#define bfoUART_PARAM_SIR_MODE           6
#define bfwUART_PARAM_SIR_MODE           1
#define bfoUART_PARAM_SIR_LP_MODE        7
#define bfwUART_PARAM_SIR_LP_MODE        1
#define bfoUART_PARAM_NEW_FEAT           8
#define bfwUART_PARAM_NEW_FEAT           1
#define bfoUART_PARAM_FIFO_ACCESS        9
#define bfwUART_PARAM_FIFO_ACCESS        1
#define bfoUART_PARAM_FIFO_STAT          10
#define bfwUART_PARAM_FIFO_STAT          1
#define bfoUART_PARAM_SHADOW             11
#define bfwUART_PARAM_SHADOW             1
#define bfoUART_PARAM_ADD_ENCODED_PARAMS 12
#define bfwUART_PARAM_ADD_ENCODED_PARAMS 1
#define bfoUART_PARAM_DMA_EXTRA          13
#define bfwUART_PARAM_DMA_EXTRA          1
#define bfoUART_PARAM_FIFO_MODE          16
#define bfwUART_PARAM_FIFO_MODE          8

/**************************** Type Definitions *******************************/

/**
 *
 * DESCRIPTION
 *  This is the structure used for accessing the uart register memory
 *  map.
 * EXAMPLE
 *  struct  *portmap;
 *  portmap = (struct  *) FMSH_APB_UART_BASE;
 *  lineStatus = INP(portmap->lsr);
 * SOURCE
 */
typedef struct FUartPs_portmap {
    volatile uint32_t rbr_thr_dll;  // receive buffer register
                                    // receive buffer register
                                    // divisor latch low          (0x00)
    volatile uint32_t ier_dlh;      // interrupt enable register
                                    // divisor latch high         (0x04)
    volatile uint32_t iir_fcr;      // interrupt identity register
                                    // FIFO control register      (0x08)
    volatile uint32_t lcr;          // line control register      (0x0c)
    volatile uint32_t mcr;          // modem control register     (0x10)
    volatile uint32_t lsr;          // line status register       (0x14)
    volatile uint32_t msr;          // modem status register      (0x18)
    volatile uint32_t scr;          // scratch register           (0x1c)
    volatile uint32_t rsv1[4];      // reserved              (0x20-0x2c)
    // This is deliberately not marked as volatile as the address needs
    // to be used with memcpy, which won't accept a volatile pointer.
    uint32_t srbr_sthr[16];           // shadow receive buffer and  (0x30-
                                      // transmit holding registers  0x6c)
    volatile uint32_t rsv2[3];        //(0x70-0x78)
    volatile uint32_t usr;            // uart status register       (0x7c)
    volatile uint32_t tfl;            // transmit FIFO level        (0x80)
    volatile uint32_t rfl;            // receive FIFO level         (0x84)
    volatile uint32_t srr;            // software reset register    (0x88)
    volatile uint32_t srts;           // shadow request to send     (0x8c)
    volatile uint32_t sbcr;           // shadow break control       (0x90)
    volatile uint32_t rsv3;           //         (0x94)
    volatile uint32_t sfe;            // shadow FIFO enable         (0x98)
    volatile uint32_t srt;            // shadow receiver trigger    (0x9c)
    volatile uint32_t stet;           // shadow transmitter trigger (0xa0)
    volatile uint32_t htx;            // halt Tx                    (0xa4)
    volatile uint32_t dmasa;          // dma software acknowledge   (0xa8)
    volatile uint32_t reserved2[5];   // reserved              (0xac-0xbc)
    volatile uint32_t dlf;            // 0xc0
    volatile uint32_t reserved3[12];  //  (0xc4-0xf0)
    volatile uint32_t comp_param_1;   // component parameters       (0xf4)
    volatile uint32_t comp_version;   // component version          (0xf8)
    volatile uint32_t comp_type;      // component type             (0xfc)
} FUartPs_Portmap_T;

#ifdef __cplusplus
}
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/**
 *  These are the common preconditions which must be met for all driver
 *  functions.  Primarily, they check that a function has been passed
 *  a legitimate FUartPs_pice structure.
 */
#define UART_COMMON_ASSERT(p)                    \
    do                                           \
    {                                            \
        FMSH_ASSERT(p != NULL);                  \
        FMSH_ASSERT(p->base_address != NULL);    \
        FMSH_ASSERT(p->comp_type == 0x44570110); \
    } while (0)

/*****************************************************************************/
/**
 *
 *  This macro reads as much data from the Rx FIFO as possible until its
 *  or the destination buffer is full.  Writes to the destination buffer
 *  are in word-size memory accesses for speed and bus efficieny.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
#define UART_FIFO_READ()                                                    \
    do                                                                      \
    {                                                                       \
        volatile uint32_t reg;                                              \
        if (uartDev->instance.rxRemain == 0){ break;    }                   \
        reg = UART_INP(portmap->lsr);                                       \
        uartDev->instance.lsr_save = (reg & (Uart_line_oe | Uart_line_pe |  \
                                             Uart_line_fe | Uart_line_bi)); \
        /* While the Rx FIFO is not empty. */                               \
        while (FMSH_BIT_GET(reg, UART_LSR_DR) == FMSH_set)                  \
        {                                                                   \
            uartDev->instance.rxHold >>= 8;                                 \
            uartDev->instance.rxHold |= (UART_INP(portmap->rbr_thr_dll)   \
                                         << 24);                            \
            if (--uartDev->instance.rxIdx == 0)                             \
            {                                                               \
                *(uartDev->instance.rxBuffer++) = uartDev->instance.rxHold; \
                uartDev->instance.rxIdx = 4;                                \
            }                                                               \
            if (--uartDev->instance.rxRemain == 0){ break;  }               \
            reg = UART_INP(portmap->lsr);                                   \
            /* Save any line status errors that may have occurred */        \
            uartDev->instance.lsr_save = (reg &                             \
                                          (Uart_line_oe | Uart_line_pe |    \
                                           Uart_line_fe | Uart_line_bi));   \
        }                                                                   \
    } while (0)

/*****************************************************************************/
/**
 *
 *  This macro is similar to UART_FIFO_READ, except it read 'MAX'
 *  characters from the Rx FIFO, as long as rxRemain > 0.  This
 *  implementation is much more efficient because the line status
 *  register is not polled after each FIFO read in order to determine if
 *  there is still valid data available.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
#define UART_X_FIFO_READ(MAX)                                               \
    do                                                                      \
    {                                                                       \
        int i;                                                              \
        if (uartDev->instance.rxRemain == 0){ break;}                       \
        for (i = 0; i < (MAX); i++)                                         \
        {                                                                   \
            uartDev->instance.rxHold >>= 8;                                 \
            uartDev->instance.rxHold |= (UART_INP(portmap->rbr_thr_dll)   \
                                         << 24);                            \
            if (--uartDev->instance.rxIdx == 0)                             \
            {                                                               \
                *(uartDev->instance.rxBuffer++) = uartDev->instance.rxHold; \
                uartDev->instance.rxIdx = 4;                                \
            }                                                               \
            if (--uartDev->instance.rxRemain == 0){ break;}                 \
        }                                                                   \
    } while (0)

/*****************************************************************************/
/**
 *
 *  This macro writes as much of the current user Tx buffer data to the
 *  Tx FIFO until it fills or there is no more data to send.  Reads
 *  words at a time to improve speed and bus efficiency.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
#define UART_FIFO_WRITE()                                                     \
    do                                                                        \
    {                                                                         \
        volatile uint32_t reg;                                                \
        if (uartDev->instance.txRemain == 0){break;}                          \
        reg = UART_INP(portmap->lsr);                                         \
        /* Save any line status errors that may have occurred */              \
        uartDev->instance.lsr_save = (reg & (Uart_line_oe | Uart_line_pe |    \
                                             Uart_line_fe | Uart_line_bi));   \
        /* Fill up the FIFO until the THRE bit is Set (Tx FIFO full) */       \
        if ((FUartPs_isPtimeEnabled(uartDev) == TRUE) &&                      \
            (uartDev->instance.fifos_enabled == TRUE))                        \
        {                                                                     \
            /* While the Tx FIFO is not full (PTIME enabled). */              \
            while (FMSH_BIT_GET(reg, UART_LSR_THRE) == FMSH_clear)            \
            {                                                                 \
                if (uartDev->instance.txIdx == 0)                             \
                {                                                             \
                    uartDev->instance.txHold = *(                             \
                        uartDev->instance.txBuffer++);                        \
                    uartDev->instance.txIdx = 4;                              \
                }                                                             \
                UART_OUTP((uartDev->instance.txHold & 0xff),                \
                            portmap->rbr_thr_dll);                            \
                uartDev->instance.txHold >>= 8;                               \
                uartDev->instance.txIdx--;                                    \
                if (--uartDev->instance.txRemain == 0){ break;}               \
                reg = UART_INP(portmap->lsr);                                 \
                /* Save any line status errors that may have occurred */      \
                uartDev->instance.lsr_save = (reg &                           \
                                              (Uart_line_oe | Uart_line_pe |  \
                                               Uart_line_fe | Uart_line_bi)); \
            }                                                                 \
        }                                                                     \
        /* Fill up the FIFO if the THRE bit is Set (Tx FIFO empty) */         \
        else if (uartDev->instance.fifos_enabled == TRUE)                     \
        {                                                                     \
           if (FMSH_BIT_GET(reg, UART_LSR_THRE) == FMSH_set){                 \
                UART_X_FIFO_WRITE(param->fifo_depth);}                        \
        }                                                                     \
        /* Can only write one character if FIFOs are disabled */              \
        else if (FMSH_BIT_GET(reg, UART_LSR_THRE) == FMSH_set)                \
        {                                                                     \
            if (uartDev->instance.txIdx == 0)                                 \
            {                                                                 \
                uartDev->instance.txHold = *(uartDev->instance.txBuffer++);   \
                uartDev->instance.txIdx = 4;                                  \
            }                                                                 \
            UART_OUTP((uartDev->instance.txHold & 0xff),                    \
                        portmap->rbr_thr_dll);                                \
            uartDev->instance.txHold >>= 8;                                   \
            uartDev->instance.txIdx--;                                        \
            uartDev->instance.txRemain--;                                     \
        }                                                                     \
        /* This clause should never be reached */                             \
        else                                                                  \
        {                                                                     \
            FMSH_ASSERT(FALSE);                                               \
        }                                                                     \
    } while (0)

/*****************************************************************************/
/**
 *
 *  This macro is similar to UART_FIFO_WRITE except it writes 'MAX'
 *  characters to the Tx FIFO, as long as txRemain > 0.  This
 *  implementation is much more efficient as it does not check the line
 *  status register after every write to determine if there is space
 *  available in the Tx FIFO or not.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
#define UART_X_FIFO_WRITE(MAX)                                              \
    do                                                                      \
    {                                                                       \
        int i;                                                              \
        if (uartDev->instance.txRemain == 0){break;}                        \
        for (i = 0; i < (MAX); i++)                                         \
        {                                                                   \
            if (uartDev->instance.txIdx == 0)                               \
            {                                                               \
                uartDev->instance.txHold = *(uartDev->instance.txBuffer++); \
                uartDev->instance.txIdx = 4;                                \
            }                                                               \
            UART_OUTP((uartDev->instance.txHold & 0xff),                  \
                        portmap->rbr_thr_dll);                              \
            uartDev->instance.txHold >>= 8;                                 \
            uartDev->instance.txIdx--;                                      \
            if (--uartDev->instance.txRemain == 0){break;}                  \
        }                                                                   \
    } while (0)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

int FUartPs_flushRxFifo(FUartPs_T *uartDev);
void FUartPs_flushRxHold(FUartPs_T *uartDev);
int FUartPs_autoCompParams(FUartPs_T *uartDev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
