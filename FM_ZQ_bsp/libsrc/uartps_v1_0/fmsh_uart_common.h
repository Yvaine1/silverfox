/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file
 *
 * This file contains
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

#ifndef _FUARTPS_COMMON_H_ /* prevent circular inclusions */
#define _FUARTPS_COMMON_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u16 DeviceId;           /**< Unique ID  of device */
    u32 BaseAddress;        /**< Base address of device (IPIF) */
    u32 InputClockHz;       /**< Input clock frequency */
    s32 ModemPinsConnected; /** Specifies whether modem pins are connected
                             *  to MIO or FMIO */
} FUartPs_Config;

/**
 *  These are the states used internally by the driver when managing
 *  interrupt-driven transfers.
 */
enum FUartPs_state {
    Uart_state_idle = 0,  // driver is idle
    Uart_state_tx,        // driver is transmitting data
    Uart_state_rx,        // driver is receiving data
    Uart_state_rx_req,    // user must call FUartPs_receive()
    Uart_state_tx_rx,     // driver is transceiving data
    Uart_state_tx_rx_req  // ditto as above; the user must call
                          // FUartPs_receive() to handle Rx transfer
};

/**
 *
 *  This is the data type used for specifying the Rx callback mode.  The
 *  default behaviour is to call a callback function when the associated
 *  Rx buffer is full.  In addition to this, the Uart_rx_timeout mode
 *  will invoke the callback function for every Uart_event_timeout
 *  interrupt which occurs, with the usual argument of the number of
 *  characters received so far in the Rx buffer.  Note that this is only
 *  relevant when using the Interrupt API.
 */
enum FMSH_callback_mode {
    Uart_buffer_full,  // callback called only when Rx buffer is full
    Uart_rx_timeout    // callback also called for character timeouts
};

/**
 *
 * DESCRIPTION
 *  This is the data type used for the UART transmitter FIFO empty
 *  trigger.
 * NOTES
 *  This data type relates to the following register bit field(s):
 *   - fcr/tx_empty_trigger
 * SEE ALSO
 *  FUartPs_setTxTrigger(), FUartPs_getTxTrigger()
 * SOURCE
 */
enum FUartPs_tx_trigger {
    Uart_empty_fifo = 0x0,
    Uart_two_chars_in_fifo = 0x1,
    Uart_quarter_full_fifo = 0x2,
    Uart_half_full_fifo = 0x3
};

/**************************** Type Definitions *******************************/

/**
 *
 * DESCRIPTION
 *  This structure comprises the uart harFMSHare parameters that affect
 *  the software driver.  This structure needs to be initialized with
 *  the correct values and be pointed to by the (FUartPs_pice).cc
 *  member of the relevant uart uart_pice structure.
 * SOURCE
 */
typedef struct FUartPs_Param {
    BOOL afce_mode;       // automatic flow control
    BOOL dma_extra;       // FMSH_ahb_dmac compatibility signals
    BOOL fifo_access;     // programmable FIFO access mode
    BOOL fifo_stat;       // additional FIFO status registers
    BOOL new_feat;        // new v3 features (shadow registers, etc.)
    BOOL shadow;          // include shadow registers
    BOOL sir_lp_mode;     // low-power IrDA SIR mode
    BOOL sir_mode;        // serial infrared
    BOOL thre_mode;       // programmable thre interrupts
    uint32_t fifo_depth;  // FIFO depth
} FUartPs_Param_T;

/**
 *
 *  This structure contains variables which relate to each individual
 *  uart instance.  Cumulatively, they can be thought of as the "state
 *  variables" for each distinct uart uart_pice.
 */
typedef struct FUartPs_instance {
    enum FUartPs_state state;              // uart driver state
    uint32_t value_in_fcr;                 // FCR value (write only reg)
    uint32_t ier_save;                     // saved IER value
    uint32_t lsr_save;                     // saved line status errors
    FMSH_callback listener;                // user event listener
    FMSH_callback txCallback;              // user Tx callback function
    FMSH_callback rxCallback;              // user Rx callback function
    enum FMSH_callback_mode callbackMode;  // Rx callback mode
    uint32_t *txBuffer;                    // pointer to user Tx buffer
    uint32_t txHold;                       // Tx holding register
    uint32_t txIdx;                        // Tx holding register index
    uint32_t txLength;                     // user Tx buffer length
    uint32_t txRemain;                     // chars left to send in buffer
    uint32_t *rxBuffer;                    // pointer to user Rx buffer
    uint32_t rxHold;                       // Rx holding register
    uint32_t rxIdx;                        // Rx holding register index
    uint32_t rxLength;                     // user Rx buffer length
    uint32_t rxRemain;                     // space left in buffer
    BOOL rxAlign;                          // is Rx buffer aligned?
    BOOL fifos_enabled;                    // are FIFOs enabled? (cached)
    enum FUartPs_tx_trigger txTrigger;     // saved user Tx trigger
} FUartPs_Instance_T;

/**
 * DESCRIPTION
 *  This is the primary structure used when dealing with all devices.
 *  It serves as a hardware abstraction layer for driver code and also
 *  allows this code to support more than one device of the same type
 *  simultaneously.  This structure needs to be initialized with
 *  meaningful values before a pointer to it is passed to a driver
 *  initialization function.
 * PARAMETERS
 *  name            name of device
 *  baseAddress     physical base address of device
 *  instance        device private data structure pointer
 *  compParam       pointer to structure containing device's
 *                  coreConsultant configuration parameters structure
 *  compVersion     device version identification number
 *  compType        device identification number
 ***/
typedef struct FUartPs {
    const char *name;
    u32 id;
    u32 base_address;
    u32 input_clock;
    FUartPs_Instance_T instance;
    FUartPs_Param_T comp_param;
    uint32_t comp_version;
    uint32_t comp_type;
} FUartPs_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
