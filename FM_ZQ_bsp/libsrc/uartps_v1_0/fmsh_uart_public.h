/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_timer.h
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

#ifndef _FMSH_UART_PUBLIC_H_ /* prevent circular inclusions */
#define _FMSH_UART_PUBLIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/

#include "fmsh_uart_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * DESCRIPTION
 *  This is a generic data type used for 1-bit wide bitfields which have
 *  a "set/clear" property.  This is used when modifying registers
 *  within a peripheral's memory map.
 */
enum UART_state { UART_err = -1, UART_clear = 0, UART_set = 1 };

#define UART_ENTER_CRITICAL_SECTION()             \
    do                                            \
    {                                             \
        uint32_t reg;                             \
        /* Don't change the value of PTIME */     \
        reg = instance->ier_save & ~Uart_irq_all; \
        /* Disable UART interrupts */             \
        UART_OUTP(reg, portmap->ier_dlh);       \
    } while (0)

#define UART_EXIT_CRITICAL_SECTION()                       \
    do                                                     \
    {                                                      \
        /* Restore UART interrupts */                      \
        UART_OUTP(instance->ier_save, portmap->ier_dlh); \
    } while (0)

/**
 * DESCRIPTION
 *  This is the data type used for specifying UART events.  An event is
 *  the occurrence of an interrupt which must be handled appropriately.
 *  One of these events is passed at a time to the user listener
 *  function to be processed.  The exception to this are the
 *  Uart_event_thre and Uart_event_timeout interrupts which are handled
 *  internally by the interrupt handler.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - iir/interrupt_id
 * SEE ALSO
 *  FUartPs_setListener(), FUartPs_getActiveIrq()
 * SOURCE
 */
enum FUartPs_event {
    Uart_event_modem = 0x0,   // CTR, DSR, RI or DCD status changed.
    Uart_event_none = 0x1,    // No event/interrupt pending.
    Uart_event_thre = 0x2,    // Transmit holding register empty or TX
                              // FIFO at or below trigger threshold.
    Uart_event_data = 0x4,    // Receive buffer register data
                              // available (non-FIFO mode) or RX FIFO
                              // trigger level reached.
    Uart_event_line = 0x6,    // Overrun/parity/framing error or break
                              // interrupt occurred.
    Uart_event_busy = 0x7,    // Attempt to write to the LCR[7] while
                              // FMSH_apb_uart was busy (DLAB).
    Uart_event_timeout = 0xc  // No characters in or out of the
                              // receiver FIFO during the last four
                              // character times and there is at least
                              // one character in it during this time.
};

/**
 * DESCRIPTION
 *  These are the bit definitions used for managing UART interrupts.
 *  The functionality of ETBEI and ERBFI alters when programmable THRE
 *  interrupt mode is active (FUartPs_enablePtime()).  See the
 *  FMSH_apb_uart databook for a detailed description.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - ier/erbfi
 *   - ier/etbei
 *   - ier/elsi
 *   - ier/edssi
 * SEE ALSO
 *  FUartPs_enableIrq(), FUartPs_disableIrq(), FUartPs_isIrqEnabled()
 * SOURCE
 */
enum FUartPs_irq {
    Uart_irq_erbfi = 0x01,  // receive data available
    Uart_irq_etbei = 0x02,  // transmitter holding register empty
    Uart_irq_elsi = 0x04,   // receiver line status
    Uart_irq_edssi = 0x08,  // modem status
    Uart_irq_all = 0x0f     // all interrupts
};

/**
 * DESCRIPTION
 *  These are the definitions used for reporting the line status
 *  including errors, if any.  Note that they represent bits of an 8-bit
 *  register and more than one can be active at any time.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - lsr/dr
 *   - lsr/oe
 *   - lsr/pe
 *   - lsr/fe
 *   - lsr/bi
 *   - lsr/thre
 *   - lsr/temt
 *   - lsr/rfe
 * SEE ALSO
 *  FUartPs_getLineStatus()
 * SOURCE
 */
enum FUartPs_line_status {
    Uart_line_dr = 0x01,    // data ready
    Uart_line_oe = 0x02,    // overrun error
    Uart_line_pe = 0x04,    // parity error
    Uart_line_fe = 0x08,    // framing error
    Uart_line_bi = 0x10,    // break interrupt
    Uart_line_thre = 0x20,  // transmit holding register empty
    Uart_line_temt = 0x40,  // transmitter empty
    Uart_line_rfe = 0x80    // receiver FIFO error
};

/**
 * DESCRIPTION
 *  These are the bit definitions used for reporting the modem status.
 *  Note that they represents bits of an 8-bit register and more than
 *  one can be active at any time.  The delta signals reflect a change
 *  in the relevant pin since the last read of the modem status
 *  register.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - msr/dcts
 *   - msr/ddsr
 *   - msr/teri
 *   - msr/ddcd
 *   - msr/cts
 *   - msr/dsr
 *   - msr/ri
 *   - msr/dcd
 * SEE ALSO
 *  FUartPs_getModemStatus()
 * SOURCE
 */
enum FUartPs_modem_status {
    Uart_modem_dcts = 0x01,  // delta clear to send
    Uart_modem_ddsr = 0x02,  // delta data set ready
    Uart_modem_teri = 0x04,  // ring indicator active->inactive
    Uart_modem_ddcd = 0x08,  // delta data carrier detect
    Uart_modem_cts = 0x10,   // clear to send
    Uart_modem_dsr = 0x20,   // data set ready
    Uart_modem_ri = 0x40,    // ring indicator
    Uart_modem_dcd = 0x80    // data carrier detect
};

/**
 * DESCRIPTION
 *  These are the bit definitions used for reporting the modem status.
 *  Note that they represents bits of an 4-bit register so one or more
 *  of them may be set/active at any time.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - mcr/dtr
 *   - mcr/rts
 *   - mcr/out1
 *   - mcr/out2
 * SEE ALSO
 *  FUartPs_setModemLine(), FUartPs_clearModemLine(),
 *  FUartPs_getModemLine()
 * SOURCE
 */
enum FUartPs_modem_line {
    Uart_modem_dtr = 0x1,      // data terminal ready
    Uart_modem_rts = 0x2,      // request to send differs since last
                               // read
    Uart_modem_out1 = 0x4,     // user-designated output one
    Uart_modem_out2 = 0x8,     // user-designated output two
    Uart_modem_line_all = 0xf  // all modem control lines
};

/**
 * DESCRIPTION
 *  This is the data type used for the UART receiver FIFO full trigger.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - fcr/rcvr_trigger
 * SEE ALSO
 *  FUartPs_setRxTrigger(), FUartPs_getRxTrigger()
 * SOURCE
 */
enum FUartPs_rx_trigger {
    Uart_one_char_in_fifo = 0x0,
    Uart_fifo_quarter_full = 0x1,
    Uart_fifo_half_full = 0x2,
    Uart_fifo_two_less_full = 0x3
};

/**
 * DESCRIPTION
 *  This is the data type used for the UART DMA mode.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - fcr/dma_mode
 * SEE ALSO
 *  FUartPs_setDmaMode(), FUartPs_getDmaMode()
 * SOURCE
 */
enum FUartPs_dma_mode { Uart_dma_single = 0x0, Uart_dma_multi = 0x1 };

/**
 * DESCRIPTION
 *  This is the data type used for specifying the number of stop bits.
 *  Note that if that number of bits per character is 5, the actual
 *  number of stop bits transmitted by FMSH_apb_uart will be either 1
 *  (Uart_one_stop_bit) or 1.5 (Uart_two_stop_bits).
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - lcr/stop
 * SEE ALSO
 *  FUartPs_getStopBits()
 * SOURCE
 */
enum FUartPs_stop_bits {
    Uart_one_stop_bit = 0x0,  // 1 stop bit
    Uart_two_stop_bits = 0x1  // 2 stop bits (or 1.5 stop bits if the
                              // number of bits per character is 5).
};

/**
 * DESCRIPTION
 *  This is the data type used for specifying the number of bits per
 *  character (Character Length Select).
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - lcr/cls
 * SEE ALSO
 *  FUartPs_getDataBits()
 * SOURCE
 */
enum FUartPs_cls {
    Uart_five_bits = 0x0,   // 5 bits per character
    Uart_six_bits = 0x1,    // 6 bits per character
    Uart_seven_bits = 0x2,  // 7 bits per character
    Uart_eight_bits = 0x3   // 8 bits per character
};

/**
 * DESCRIPTION
 *  This is the data type used for specifying UART parity.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - lcr/pen
 *   - lcr/eps
 * SEE ALSO
 *  FUartPs_getParity()
 * SOURCE
 */
enum FUartPs_parity {
    Uart_no_parity = 0x0,   // no parity
    Uart_odd_parity = 0x1,  // odd parity
    Uart_even_parity = 0x3  // even parity
};

enum FUartPs_stick {
    Uart_Stick_disable = 0x0,  // no parity
    Uart_Stick_enable = 0x1    // odd parity
};

/**
 * DESCRIPTION
 *  This is the data type used for manipulation of the UART line control
 *  settings.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - lcr/cls
 *   - lcr/stop
 *   - lcr/pen
 *   - lcr/eps
 * SEE ALSO
 *  FUartPs_getLineControl(), FUartPs_setLineControl()
 * SOURCE
 */
enum FUartPs_line_control {
    Uart_line_5n1 = 0x00,    // 5 data bits, no parity, 1 stop bit
    Uart_line_5n1_5 = 0x04,  // 5 data bits, no parity, 1.5 stop bits
    Uart_line_5e1 = 0x18,    // 5 data bits, even parity, 1 stop bit
    Uart_line_5e1_5 = 0x1c,  // 5 data bits, even parity, 1.5 stop bits
    Uart_line_5o1 = 0x08,    // 5 data bits, odd parity, 1 stop bit
    Uart_line_5o1_5 = 0x0c,  // 5 data bits, odd parity, 1.5 stop bits
    Uart_line_6n1 = 0x01,    // 6 data bits, no parity, 1 stop bit
    Uart_line_6n2 = 0x05,    // 6 data bits, no parity, 2 stop bits
    Uart_line_6e1 = 0x19,    // 6 data bits, even parity, 1 stop bit
    Uart_line_6e2 = 0x1d,    // 6 data bits, even parity, 2 stop bits
    Uart_line_6o1 = 0x09,    // 6 data bits, odd parity, 1 stop bit
    Uart_line_6o2 = 0x0d,    // 6 data bits, odd parity, 2 stop bits
    Uart_line_7n1 = 0x02,    // 7 data bits, no parity, 1 stop bit
    Uart_line_7n2 = 0x06,    // 7 data bits, no parity, 2 stop bits
    Uart_line_7e1 = 0x1a,    // 7 data bits, even parity, 1 stop bit
    Uart_line_7e2 = 0x1e,    // 7 data bits, even parity, 2 stop bits
    Uart_line_7o1 = 0x0a,    // 7 data bits, odd parity, 1 stop bit
    Uart_line_7o2 = 0x0e,    // 7 data bits, odd parity, 2 stop bits
    Uart_line_8n1 = 0x03,    // 8 data bits, no parity, 1 stop bit
    Uart_line_8n2 = 0x07,    // 8 data bits, no parity, 2 stop bits
    Uart_line_8e1 = 0x1b,    // 8 data bits, even parity, 1 stop bit
    Uart_line_8e2 = 0x1f,    // 8 data bits, even parity, 2 stop bits
    Uart_line_8o1 = 0x0b,    // 8 data bits, odd parity, 1 stop bit
    Uart_line_8o2 = 0x0f     // 8 data bits, odd parity, 2 stop bits
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
FUartPs_Config *FUartPs_LookupConfig(u16 DeviceId);
int FUartPs_init(FUartPs_T *uartDev, FUartPs_Config *cfg);
BOOL FUartPs_isBusy(FUartPs_T *uartDev);

void FUartPs_reset(FUartPs_T *uartDev);
void FUartPs_resetTxFifo(FUartPs_T *uartDev);
void FUartPs_resetRxFifo(FUartPs_T *uartDev);

int FUartPs_setClockDivisor(FUartPs_T *uartDev, uint16_t divisor,
                            uint16_t fdivisor);
uint16_t FUartPs_getClockDivisor(FUartPs_T *uartDev);
int FUartPs_setLineControl(FUartPs_T *uartDev,
                                enum FUartPs_line_control setting);
u8 FUartPs_setBaudRate(FUartPs_T *uartDev, u32 baudRate);

enum FUartPs_line_control FUartPs_getLineControl(FUartPs_T *uartDev);
void FUartPs_setDataBits(FUartPs_T *uartDev, enum FUartPs_cls cls);
enum FUartPs_cls FUartPs_getDataBits(FUartPs_T *uartDev);
void FUartPs_setStopBits(FUartPs_T *uartDev, enum FUartPs_stop_bits stop);
enum FUartPs_stop_bits FUartPs_getStopBits(FUartPs_T *uartDev);
void FUartPs_setParity(FUartPs_T *uartDev, enum FUartPs_parity parity);
void FUartPs_setStick(FUartPs_T *uartDev, enum FUartPs_stick stick);
enum FUartPs_parity FUartPs_getParity(FUartPs_T *uartDev);

int FUartPs_enableFifos(FUartPs_T *uartDev);
int FUartPs_disableFifos(FUartPs_T *uartDev);
BOOL FUartPs_areFifosEnabled(FUartPs_T *uartDev);
BOOL FUartPs_isTxFifoFull(FUartPs_T *uartDev);
BOOL FUartPs_isTxFifoEmpty(FUartPs_T *uartDev);
BOOL FUartPs_isRxFifoFull(FUartPs_T *uartDev);
BOOL FUartPs_isRxFifoEmpty(FUartPs_T *uartDev);
int FUartPs_getTxFifoLevel(FUartPs_T *uartDev);
int FUartPs_getRxFifoLevel(FUartPs_T *uartDev);
uint32_t FUartPs_getFifoDepth(FUartPs_T *uartDev);

void FUartPs_enablePtime(FUartPs_T *uartDev);
void FUartPs_disablePtime(FUartPs_T *uartDev);
BOOL FUartPs_isPtimeEnabled(FUartPs_T *uartDev);
void FUartPs_setBreak(FUartPs_T *uartDev, enum FMSH_state state);
enum FMSH_state FUartPs_getBreak(FUartPs_T *uartDev);
void FUartPs_setModemLine(FUartPs_T *uartDev, enum FUartPs_modem_line lines);
void FUartPs_clearModemLine(FUartPs_T *uartDev, enum FUartPs_modem_line lines);
enum FUartPs_modem_line FUartPs_getModemLine(FUartPs_T *uartDev);

void FUartPs_enableLoopback(FUartPs_T *uartDev);
void FUartPs_disableLoopback(FUartPs_T *uartDev);
BOOL FUartPs_isLoopbackEnabled(FUartPs_T *uartDev);

void FUartPs_enableAfc(FUartPs_T *uartDev);
void FUartPs_disableAfc(FUartPs_T *uartDev);
BOOL FUartPs_isAfcEnabled(FUartPs_T *uartDev);

enum FUartPs_line_status FUartPs_getLineStatus(FUartPs_T *uartDev);
enum FUartPs_modem_status FUartPs_getModemStatus(FUartPs_T *uartDev);
void FUartPs_setScratchpad(FUartPs_T *uartDev, uint8_t value);
uint8_t FUartPs_getScratchpad(FUartPs_T *uartDev);

uint8_t FUartPs_read(FUartPs_T *uartDev);
void FUartPs_write(FUartPs_T *uartDev, uint8_t character);

void FUartPs_burstRead(FUartPs_T *uartDev, uint8_t *buffer, uint32_t length);
void FUartPs_burstWrite(FUartPs_T *uartDev, uint8_t *buffer, uint32_t length);

void FUartPs_enableIrq(FUartPs_T *uartDev, enum FUartPs_irq interrupts);
void FUartPs_disableIrq(FUartPs_T *uartDev, enum FUartPs_irq interrupts);
BOOL FUartPs_isIrqEnabled(FUartPs_T *uartDev, enum FUartPs_irq interrupt);
uint8_t FUartPs_getIrqMask(FUartPs_T *uartDev);
enum FUartPs_event FUartPs_getActiveIrq(FUartPs_T *uartDev);
void FUartPs_setTxTrigger(FUartPs_T *uartDev, enum FUartPs_tx_trigger trigger);
enum FUartPs_tx_trigger FUartPs_getTxTrigger(FUartPs_T *uartDev);
void FUartPs_setRxTrigger(FUartPs_T *uartDev, enum FUartPs_rx_trigger trigger);
enum FUartPs_rx_trigger FUartPs_getRxTrigger(FUartPs_T *uartDev);

void FUartPs_setListener(FUartPs_T *uartDev, FMSH_callback userFunction);
int FUartPs_userIrqHandler(FUartPs_T *uartDev);

int FUartPs_irqHandler(FUartPs_T *uartDev);
int FUartPs_transmit(FUartPs_T *uartDev, void *buffer, uint32_t length,
                     FMSH_callback userFunction);
int FUartPs_receive(FUartPs_T *uartDev, void *buffer, uint32_t length,
                    FMSH_callback userFunction);

void FUartPs_resetInstance(FUartPs_T *uartDev);
void uart_x_fifo_write(FUartPs_T *uartDev);
void uart_x_fifo_write_max(FUartPs_T *uartDev, int max);
#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
