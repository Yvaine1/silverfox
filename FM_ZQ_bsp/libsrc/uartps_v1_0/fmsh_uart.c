/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_uart_lib.c
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

/***************************** Include Files *********************************/

#include <math.h>    // needed for memcpy()
#include <string.h>  // needed for memcpy()

#include "fmsh_uart_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 *
 *  This function initializes a uart. It disables all interrupts and
 *  resets the Tx and Rx FIFOs.  It also initializes the driver's
 *  internal data structures.
 *
 * @param   uartDev is uart handle.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_ENOSYS  -- hardware parameters for the device could not be
 *              automatically determined
 *
 * @note
 *  If the FMSH_apb_uart is independently reset, it must be re-initialized
 *  by calling this function, in order to keep the device and driver in
 *  sync.
 *
 ****************************************************************************/
int FUartPs_init (FUartPs_T *uartDev, FUartPs_Config *cfg)
{
    int retval = 0;

    uartDev->base_address = cfg->BaseAddress;
    uartDev->id = cfg->DeviceId;
    uartDev->input_clock = cfg->InputClockHz;

    // attempt to determine hardare parameters
    retval = FUartPs_autoCompParams(uartDev);

    // disable all uart interrupts
    FUartPs_disableIrq(uartDev, Uart_irq_all);

    // reset device FIFOs
    FUartPs_resetTxFifo(uartDev);
    FUartPs_resetRxFifo(uartDev);

    // initialize private variables
    FUartPs_resetInstance(uartDev);

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function returns whether the UART is busy (transmitting and/or
 *  receiving) or not.  If the UART busy bit is unsupported, an error
 *  code is returned.
 *
 * @param   uartDev is uart handle.
 *
 * @return
 *  TRUE        -- UART is busy
 *  FALSE       -- UART is not busy
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - usr/busy
 *
 ****************************************************************************/
BOOL FUartPs_isBusy (FUartPs_T *uartDev)
{
    BOOL retval=0;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->usr);
    if (FMSH_BIT_GET(reg, UART_USR_BUSY) == FMSH_set)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function performs a hardware reset on a FMSH_apb_uart device.
 *
 * @param   uartDev is uart handle.
 *
 * @return  None.
 *
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - srr/ur
 *  After resetting the uart, the device driver should be re-initialized
 *  by calling FUartPs_init().
 *
 ****************************************************************************/
void FUartPs_reset (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = 0;
    FMSH_BIT_SET(reg, UART_SRR_UR, FMSH_set);
    UART_OUTP(reg, portmap->srr);

    return;
}

/****************************************************************************/
/**
 *
 *  This funciton resets the transmitter FIFO.
 *
 * @param   uartDev is uart handle.
 *
 * @return  None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/xfifor
 *   - srr/xfr
 *
 ****************************************************************************/
void FUartPs_resetTxFifo (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // The FIFO reset bits are self-clearing.

    reg = 0;
    // FMSH_BIT_SET(reg, UART_SRR_XFR, FMSH_set);
    // UART_OUTP(reg, portmap->srr);
    FMSH_BIT_SET(reg, UART_FCR_XMIT_FIFO_RESET, FMSH_set);
    UART_OUTP(reg, portmap->iir_fcr);
}

/****************************************************************************/
/**
 *
 *  This function resets the receiver FIFO.
 *
 * @param   uartDev is uart handle.
 *
 * @return  None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/rfifor
 *   - srr/rfr
 *
 ****************************************************************************/
void FUartPs_resetRxFifo (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;
    reg = 0;
    FMSH_BIT_SET(reg, UART_SRR_RFR, FMSH_set);
    UART_OUTP(reg, portmap->srr);
}

/****************************************************************************/
/**
 *
 *  Sets the divisor value used when dividing down sclk, which
 *  determines the baud rate.  See the FMSH_apb_uart databook for more
 *  information.
 *
 * @param   uartDev is uart handle.
 * @param   divisor -- value to set clock divisor.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- uart is busy
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/dlab
 *   - dll/all bits
 *   - dlh/all bits
 *  A delay is required before using the UART after setting the baud
 *  rate.  The minimum delay needed is 8 clock cycles of the slowest
 *  FMSH_apb_uart clock.
 *
 ****************************************************************************/
int FUartPs_setClockDivisor (FUartPs_T *uartDev, uint16_t divisor,
                             uint16_t fdivisor)
{
    int retval = 0;
    uint32_t reg = 0U;
    uint8_t dll, dlh, dlf;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // the divisor latch access bit (DLAB) of the line
    //  control register (LCR) cannot be written while the UART is busy.
    //  This does not affect version 2.
    if (FUartPs_isBusy(uartDev) == TRUE)
    {
        retval = -FMSH_EBUSY;
    }
    else
    {
        retval = 0;
        // set DLAB to access DLL and DLH registers
        reg = UART_INP(portmap->lcr);
        FMSH_BIT_SET(reg, UART_LCR_DLAB, FMSH_set);
        UART_OUTP(reg, portmap->lcr);
        dll = divisor & 0x00ff;
        dlh = (divisor & 0xff00) >> 8;
        dlf = fdivisor;
        UART_OUTP(dll, portmap->rbr_thr_dll);
        UART_OUTP(dlh, portmap->ier_dlh);
        UART_OUTP(dlf, portmap->dlf);  // dlf
        // clear DLAB
        FMSH_BIT_CLEAR(reg, UART_LCR_DLAB);
        UART_OUTP(reg, portmap->lcr);
    }

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function returns the clock divisor value.  This is the value
 *  used to divide down sclk, thereby determining the baud rate.  See
 *  the FMSH_apb_uart databook for more information.
 *
 * @param   uartDev is uart handle.
 *
 * @return The clock divisor value.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/dlab
 *   - dll/all bits
 *   - dlh/all bits
 *
 *  This function should never be called when the FMSH_apb_uart is busy as
 *  to do so can result in data loss.  If the value of the divisor is
 *  required while a transfer is in progress, it should be saved to a
 *  local variable before the transfer begins.
 *
 ****************************************************************************/
uint16_t FUartPs_getClockDivisor (FUartPs_T *uartDev)
{
    uint8_t dll, dlh;
    uint16_t retval;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // set DLAB to access DLL and DLH registers
    reg = UART_INP(portmap->lcr);
    FMSH_BIT_SET(reg, UART_LCR_DLAB, FMSH_set);
    UART_OUTP(reg, portmap->lcr);
    dll = UART_INP(portmap->rbr_thr_dll);
    dlh = UART_INP(portmap->ier_dlh);
    // clear DLAB
    FMSH_BIT_CLEAR(reg, UART_LCR_DLAB);
    UART_OUTP(reg, portmap->lcr);

    retval = (dlh << 8) | dll;

    return retval;
}

/****************************************************************************/
/**
 *
 * This function set buad rate of uart device
 *
 * @param   uartDev is a pointer to the uart instance.
 * @param   baud_rate is the buad rate
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u8 FUartPs_setBaudRate (FUartPs_T *uartDev, u32 baudRate)
{
    int retval = 0;

    u32 divisor = 0U;
    float divisor_f = 1.0;
    float tmp = 1.0;

    tmp = 16 * baudRate;
    tmp = (float)(uartDev->input_clock / tmp);
    divisor = (u32)tmp;
    divisor_f = tmp - divisor;
    divisor_f = divisor_f * 16;
    retval = FUartPs_setClockDivisor(uartDev, divisor, (u16)round(divisor_f));
    if (retval != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/****************************************************************************/
/**
 *
 *  This function is used to set the parity and the number of data and
 *  stop bits.  The FUartPs_line_control defintions are used to specify
 *  this mode.  The line control settings should not be changed when the
 *  UART is busy.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  mode        -- line control settings
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- UART is busy
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/cls
 *   - lcr/stop
 *   - lcr/pen
 *   - lcr/eps
 *
 ****************************************************************************/
int32_t FUartPs_setLineControl (FUartPs_T *uartDev,
                                enum FUartPs_line_control setting)
{
    int32_t retval = 0U;
    uint8_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // Note: it does not matter if the UART is busy when setting the
    // line control register on FMSH_apb_uart v2.00.  For uart v2.00,
    // the isBusy function returns -FMSH_ENOSYS (not supported).  This is
    // necessary, however, for UART v3.00.
    if (FUartPs_isBusy(uartDev) == TRUE)
    {
        retval = -FMSH_EBUSY;
    }
    else
    {
        retval = 0;
        reg = UART_INP(portmap->lcr);
        // avoid bus write is possible
        if (FMSH_BIT_GET(reg, UART_LCR_LINE) != setting)
        {
            FMSH_BIT_SET(reg, UART_LCR_LINE, setting);
            UART_OUTP(reg, portmap->lcr);
        }
    }

    return retval;
}

/****************************************************************************/
/**
 *
 *  Returns the line control settings.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current line control settings.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/cls
 *   - lcr/stop
 *   - lcr/pen
 *   - lcr/eps
 *
 ****************************************************************************/
enum FUartPs_line_control FUartPs_getLineControl (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_line_control retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // get the value of the LCR, masking irrelevant bits
    reg = UART_INP(portmap->lcr);
    retval = (enum FUartPs_line_control)FMSH_BIT_GET(reg, UART_LCR_LINE);

    return retval;
}

/****************************************************************************/
/**
 *
 * Sets the number of bits per character (5/6/7/8).
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  cls         -- number of data bits per character
 *
 * @return None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/cls
 *
 ****************************************************************************/
void FUartPs_setDataBits (FUartPs_T *uartDev, enum FUartPs_cls cls)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->lcr);
    if (FMSH_BIT_GET(reg, UART_LCR_CLS) != cls)
    {
        FMSH_BIT_SET(reg, UART_LCR_CLS, cls);
        UART_OUTP(reg, portmap->lcr);
    }
}

/****************************************************************************/
/**
 *
 * Returns the number of bits per character setting for data transfers.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current number of data bits setting.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/cls
 *
 ****************************************************************************/
enum FUartPs_cls FUartPs_getDataBits (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_cls retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->lcr);
    retval = (enum FUartPs_cls)FMSH_BIT_GET(reg, UART_LCR_CLS);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Sets the number of stop bits (1/1.5/2).
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  stop        -- number of stop bits
 *
 * @return None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/stop
 *
 ****************************************************************************/
void FUartPs_setStopBits (FUartPs_T *uartDev, enum FUartPs_stop_bits stop)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->lcr);
    if (FMSH_BIT_GET(reg, UART_LCR_STOP) != stop)
    {
        FMSH_BIT_SET(reg, UART_LCR_STOP, stop);
        UART_OUTP(reg, portmap->lcr);
    }
}

/****************************************************************************/
/**
 *
 *  Returns the number of stop bits setting for data transfers.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current number of stop bits setting.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/stop
 *
 ****************************************************************************/
enum FUartPs_stop_bits FUartPs_getStopBits (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_stop_bits retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->lcr);
    retval = (enum FUartPs_stop_bits)FMSH_BIT_GET(reg, UART_LCR_STOP);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Sets the parity mode (none/odd/even).
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  parity      -- parity to set
 *
 * @return None
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/pen
 *   - lcr/eps
 *
 ****************************************************************************/
void FUartPs_setParity (FUartPs_T *uartDev, enum FUartPs_parity parity)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->lcr);
    if (FMSH_BIT_GET(reg, UART_LCR_PARITY) != parity)
    {
        FMSH_BIT_SET(reg, UART_LCR_PARITY, parity);
        UART_OUTP(reg, portmap->lcr);
    }
}
/****************************************************************************/
/**
 *
 *  Sets the Stick Parity (none/odd/even).
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  stick           -- Stick parity to set
 *
 * @return None
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/pen
 *   - lcr/eps
 *
 ****************************************************************************/
void FUartPs_setStick (FUartPs_T *uartDev, enum FUartPs_stick stick)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->lcr);
    if (FMSH_BIT_GET(reg, UART_LCR_STICK_PARITY) != stick)
    {
        FMSH_BIT_SET(reg, UART_LCR_STICK_PARITY, stick);
        UART_OUTP(reg, portmap->lcr);
    }
}

/****************************************************************************/
/**
 *
 *  Returns the parity setting for data transfers.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current parity setting.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/pen
 *   - lcr/eps
 *
 ****************************************************************************/
enum FUartPs_parity FUartPs_getParity (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_parity retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->lcr);
    retval = (enum FUartPs_parity)FMSH_BIT_GET(reg, UART_LCR_PARITY);

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function enables receive and transmit FIFOs, if they are
 *  available.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  0           -- if successful
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/fifoe
 *   - sfe/fe
 *
 ****************************************************************************/
int FUartPs_enableFifos (FUartPs_T *uartDev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // if FIFOs are available
    retval = 0;
    // The status of the FIFOs is cached here as reading the IIR
    // register directly with FUartPs_areFifosEnabled() can
    // inadvertently clear any pending THRE interrupt.
    uartDev->instance.fifos_enabled = TRUE;
    // update stored FCR value
    FMSH_BIT_SET(uartDev->instance.value_in_fcr, UART_FCR_FIFO_ENABLE,
                 FMSH_set);
    // if shadow registers are available

    reg = 0;
    FMSH_BIT_SET(reg, UART_SFE_FE, FMSH_set);
    UART_OUTP(reg, portmap->sfe);

    return retval;
}

/****************************************************************************/
/**
 *
 * This function disables receive and transmit FIFOs.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  0           -- if successful
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/fifoe
 *   - sfe/fe
 *
 ****************************************************************************/
int FUartPs_disableFifos (FUartPs_T *uartDev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    retval = 0;
    // The status of the FIFOs is cached here as reading the IIR
    // register directly with FUartPs_areFifosEnabled() can
    // inadvertently clear any pending THRE interrupt.
    uartDev->instance.fifos_enabled = FALSE;
    // update stored FCR value
    FMSH_BIT_CLEAR(uartDev->instance.value_in_fcr, UART_FCR_FIFO_ENABLE);
    // if shadow registers are available
    reg = 0;
    FMSH_BIT_SET(reg, UART_SFE_FE, FMSH_clear);
    UART_OUTP(reg, portmap->sfe);

    return retval;
}

/****************************************************************************/
/**
 *
 * Returns whether the FIFOs and enabled or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- FIFOs are enabled
 *  FALSE       -- FIFOs are disabled or not available
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - iir/fifose
 *   - sfe/fe
 *  warning
 *  This function can indirectly clear a THRE interrupt.  When the
 *  SHADOW registers are not available, it reads the IIR register in
 *  order to determine if FIFOs are enabled or not.  Reading the IIR
 *  register clears any pending THRE interrupt.
 *
 ****************************************************************************/
BOOL FUartPs_areFifosEnabled (FUartPs_T *uartDev)
{
    BOOL retval = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->sfe);
    if (FMSH_BIT_GET(reg, UART_SFE_FE) == FMSH_clear)
    {
        retval = FALSE;
    }
    else
    {
        retval = TRUE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 * Returns whether the transmitter FIFO is full or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- Tx FIFO is full
 *  FALSE       -- Tx FIFO is not full
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *  This function is affected by the FIFO_STAT hardware parameter.
 *   - usr/tfnf
 *
 ****************************************************************************/
BOOL FUartPs_isTxFifoFull (FUartPs_T *uartDev)
{
    BOOL retval = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->usr);
    if (FMSH_BIT_GET(reg, UART_USR_TFNF) == FMSH_clear)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 * Returns whether the transmitter FIFO is empty or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- Tx FIFO is empty
 *  FALSE       -- Tx FIFO is not empty
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - usr/tfe
 *
 *  This function is affected by the FIFO_STAT hardware parameter.
 *
 ****************************************************************************/
BOOL FUartPs_isTxFifoEmpty (FUartPs_T *uartDev)
{
    BOOL retval  = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->usr);
    if (FMSH_BIT_GET(reg, UART_USR_TFE) == 1)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 * Returns whether the receiver FIFO is full or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- Rx FIFO is full
 *  FALSE       -- Rx FIFO is not full
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - usr/rff
 *
 *  This function is affected by the FIFO_STAT hardware parameter.
 *
 ****************************************************************************/
BOOL FUartPs_isRxFifoFull (FUartPs_T *uartDev)
{
    BOOL retval  = FALSE;
    uint32_t reg = 0U;

    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->usr);
    if (FMSH_BIT_GET(reg, UART_USR_RFF) == 1)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 * This function returns whether the receiver FIFO is empty or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- Rx FIFO is empty
 *  FALSE       -- Rx FIFO is not empty
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - usr/rfne
 *
 *  This function is affected by the FIFO_STAT hardware parameter.
 *
 ****************************************************************************/
BOOL FUartPs_isRxFifoEmpty (FUartPs_T *uartDev)
{
    BOOL retval  = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->usr);
    // RFNE bit clear == Rx FIFO empty
    if (FMSH_BIT_GET(reg, UART_USR_RFNE) == FMSH_clear)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function returns the number of characters currently present in
 *  the Tx FIFO.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  -FMSH_ENOSYS  -- function not supported
 *  Otherwise number of characters currently in the Tx FIFO is returned.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - tfl/all bits
 *
 *  This function is affected by the FIFO_STAT hardware parameter.
 *
 ****************************************************************************/
int FUartPs_getTxFifoLevel (FUartPs_T *uartDev)
{
    int retval  = 0;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->tfl);
    retval = FMSH_BIT_GET(reg, UART_TFL_LEVEL);

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function returns the number of characters currently present in
 *  the Rx FIFO.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  -FMSH_ENOSYS  -- function not supported
 *  Otherwise number of characters currently in the Rx FIFO is returned.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - rfl/all bits
 *
 *  This function is affected by the FIFO_STAT hardware parameter.
 *
 ****************************************************************************/
int FUartPs_getRxFifoLevel (FUartPs_T *uartDev)
{
    int retval  = 0;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->rfl);
    retval = FMSH_BIT_GET(reg, UART_RFL_LEVEL);

    return retval;
}

/****************************************************************************/
/**
 *
 * Returns how many bytes deep the transmitter and receiver FIFOs are.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  FIFO depth in bytes (64)
 *
 * @note
 *  This function is affected by the FIFO_MODE hardware parameter.
 *
 ****************************************************************************/
uint32_t FUartPs_getFifoDepth (FUartPs_T *uartDev)
{
    uint32_t retval;

    UART_COMMON_ASSERT(uartDev);

    retval = uartDev->comp_param.fifo_depth;

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function enables Programmable Threshold (THRE) Interrupt Mode
 *  (PTIME).  This mode enables triggering of interrupts for different
 *  levels of the Tx/Rx FIFOs.  Enabling PTIME also changes the
 *  functionality of the lsr/thre bit (LSR[5]) to indicate that the Tx
 *  FIFO is full.  See the FMSH_apb_uart databook for more information.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return -none
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - ier/ptime
 *
 *  This function is affected by the THRE_MODE hardware parameter.
 *  warning
 *  FIFOs must also be enabled in order to use the Programmable THRE
 *  Interrupt Mode.
 *
 ****************************************************************************/
void FUartPs_enablePtime (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->ier_dlh);
    // avoid bus write if possible
    if (FMSH_BIT_GET(reg, UART_IER_PTIME) != FMSH_set)
    {
        FMSH_BIT_SET(reg, UART_IER_PTIME, FMSH_set);
        // save IER value
        uartDev->instance.ier_save = reg;
        // enable PTIME
        UART_OUTP(reg, portmap->ier_dlh);
    }

    return;
}

/****************************************************************************/
/**
 *
 *  This function disables Programmable Threshold (THRE) Interrupt Mode
 *  (PTIME).  When PTIME is disabled, the functionality of the lsr/thre
 *  bit (LSR[5]) is normal, indicating that the Tx FIFO/THR is empty.
 *  See the FMSH_apb_uart databook for more information.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return    -none
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - ier/ptime
 *
 *  This function is affected by the THRE_MODE hardware parameter.
 *
 ****************************************************************************/
void FUartPs_disablePtime (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;

    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->ier_dlh);
    // avoid bus write if possible
    if (FMSH_BIT_GET(reg, UART_IER_PTIME) != FMSH_clear)
    {
        FMSH_BIT_SET(reg, UART_IER_PTIME, FMSH_clear);
        // save IER value
        uartDev->instance.ier_save = reg;
        // disable PTIME
        UART_OUTP(reg, portmap->ier_dlh);
    }

    return;
}

/****************************************************************************/
/**
 *
 *  This function returns whether Programmable Threshold (THRE)
 *  Interrupt Mode (PTIME) is enabled or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- PTIME is enabled
 *  FALSE       -- PTIME is disabled
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - ier/ptime
 *
 ****************************************************************************/
BOOL FUartPs_isPtimeEnabled (FUartPs_T *uartDev)
{
    BOOL retval = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->ier_dlh);
    if (FMSH_BIT_GET(reg, UART_IER_PTIME) == FMSH_set)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 *  Sets the break control bit to 'state'.  When enabled, it causes a
 *  break signal to be generated, by holding the sout line low, until
 *  the break bit is subsequently cleared (with this function).
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  state       -- Set or Clear
 *
 * @return None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/break
 *   - sbcr/bcr
 *
 *  This function is affected by the SHADOW hardware parameter.
 *
 ****************************************************************************/
void FUartPs_setBreak (FUartPs_T *uartDev, enum FMSH_state state)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    FMSH_ASSERT((state == FMSH_clear) || (state == FMSH_set));

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = 0;
    FMSH_BIT_SET(reg, UART_SBCR_BCR, state);
    UART_OUTP(state, portmap->sbcr);
}

/****************************************************************************/
/**
 *
 * Returns the state of the break control bit.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current break bit state.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lcr/break
 *   - sbcr/bcr
 *
 *  This function is affected by the SHADOW hardware parameter.
 *
 ****************************************************************************/
enum FMSH_state FUartPs_getBreak (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FMSH_state retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->sbcr);
    retval = (enum FMSH_state)FMSH_BIT_GET(reg, UART_SBCR_BCR);

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function is used to Set specific modem lines.  The lines
 *  argument comprises of one or more bitwise OR'ed FUartPs_modem_line
 *  enumerated values.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  lines       -- modem line(s) to Set
 *
 * @return None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/dtr
 *   - mcr/rts
 *   - mcr/out1
 *   - mcr/out2
 *
 ****************************************************************************/
void FUartPs_setModemLine (FUartPs_T *uartDev, enum FUartPs_modem_line lines)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    reg |= lines;
    UART_OUTP(reg, portmap->mcr);
}

/****************************************************************************/
/**
 *
 *  This function is used to Clear specific modem lines.  The lines
 *  argument comprises of one or more bitwise OR'ed FUartPs_modem_line
 *  enumerated values.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  lines       -- modem line(s) to Clear
 *
 * @return None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/dtr
 *   - mcr/rts
 *   - mcr/out1
 *   - mcr/out2
 *
 ****************************************************************************/
void FUartPs_clearModemLine (FUartPs_T *uartDev, enum FUartPs_modem_line lines)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    reg &= ~lines;
    UART_OUTP(reg, portmap->mcr);
}

/****************************************************************************/
/**
 *
 *  This function returns the state of the modem control lines.  The
 *  FUartPs_modem_line enumerated values are used with this function's
 *  return value to determine the current state of the modem lines.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current value on the modem line control settings.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/dtr
 *   - mcr/rts
 *   - mcr/out1
 *   - mcr/out2
 *
 ****************************************************************************/
enum FUartPs_modem_line FUartPs_getModemLine (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_modem_line retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    retval = (enum FUartPs_modem_line)(reg & Uart_modem_line_all);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Enables loopback mode.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current value on the modem line control settings.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/loopback
 *
 ****************************************************************************/
void FUartPs_enableLoopback (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    if (FMSH_BIT_GET(reg, UART_MCR_LOOPBACK) != FMSH_set)
    {
        FMSH_BIT_SET(reg, UART_MCR_LOOPBACK, FMSH_set);
        UART_OUTP(reg, portmap->mcr);
    }
}

/****************************************************************************/
/**
 *
 *  Disables loopback mode.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/loopback
 *
 ****************************************************************************/
void FUartPs_disableLoopback (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    if (FMSH_BIT_GET(reg, UART_MCR_LOOPBACK) != FMSH_clear)
    {
        FMSH_BIT_SET(reg, UART_MCR_LOOPBACK, FMSH_clear);
        UART_OUTP(reg, portmap->mcr);
    }
}

/****************************************************************************/
/**
 *
 *  Returns whether loopback mode is enabled or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- loopback mode is enabled
 *  FALSE       -- loopback mode is disabled
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/loopback
 *
 ****************************************************************************/
BOOL FUartPs_isLoopbackEnabled (FUartPs_T *uartDev)
{
    BOOL retval = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    retval = (BOOL)FMSH_BIT_GET(reg, UART_MCR_LOOPBACK);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Enables Automatic Flow Control mode.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  0           -- if successful
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/afce
 *
 *  This function is affected by the AFCE_MODE hardware parameter.
 *
 ****************************************************************************/
void FUartPs_enableAfc (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    FMSH_BIT_SET(reg, UART_MCR_AFCE, FMSH_set);
    UART_OUTP(reg, portmap->mcr);

    return;
}

/****************************************************************************/
/**
 *
 *  Disables Automatic Flow Control mode.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  0           -- if successful
 *  -FMSH_ENOSYS  -- function not supported
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/afce
 *
 *  This function is affected by the AFCE_MODE hardware parameter.
 *
 ****************************************************************************/
void FUartPs_disableAfc (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    FMSH_BIT_CLEAR(reg, UART_MCR_AFCE);
    UART_OUTP(reg, portmap->mcr);

    return;
}

/****************************************************************************/
/**
 *
 *  Returns whether Automatic Flow Control mode is enabled or not.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- Automatic Flow Control is enabled
 *  FALSE       -- Automatic Flow Control is disabled
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - mcr/afce
 *
 *  This function is affected by the AFCE_MODE hardware parameter.
 *
 ****************************************************************************/
BOOL FUartPs_isAfcEnabled (FUartPs_T *uartDev)
{
    BOOL retval = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->mcr);
    retval = FMSH_BIT_GET(reg, UART_MCR_AFCE);

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function returns the current line status register value.  This
 *  value is used in conjunction with the FUartPs_line_status enumerated
 *  values to determine the current line status.  See the FMSH_apb_uart
 *  databook for more information about the line status register.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 * The current line status register value.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - lsr/all bits
 *
 *  When using the Interrupt API and the FIFO status registers are not
 *  available, the line status register is read to determine when the
 *  Tx/Rx FIFOs are full/empty.  As this can inadvertently reset an
 *  error bit, any active error bits are temporarily saved.  These bits
 *  are OR'ed with the return value of this function when it is called.
 *
 *  This is to ensure that no errors are missed.  Also note that this
 *  means that reported errors are not necessarily associated with the
 *  character at the top of the Rx FIFO, when using the Interrupt API.
 *
 ****************************************************************************/
enum FUartPs_line_status FUartPs_getLineStatus (FUartPs_T *uartDev)
{
    uint8_t reg = 0U;
    enum FUartPs_line_status retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // read line status register
    reg = UART_INP(portmap->lsr);
    // include saved line status errors
    retval = (enum FUartPs_line_status)(reg | uartDev->instance.lsr_save);
    // reset saved errors
    uartDev->instance.lsr_save = 0;

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function returns the current modem status register value.  This
 *  value is used in conjunction with the FUartPs_modem_status
 *  enumerated values to determine the current modem status.  See the
 *  FMSH_apb_uart databook for more information about the modem status
 *  register.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current modem status register value.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - msr/all bits
 *
 ****************************************************************************/
enum FUartPs_modem_status FUartPs_getModemStatus (FUartPs_T *uartDev)
{
    uint8_t reg = 0U;
    enum FUartPs_modem_status retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->msr);
    retval = (enum FUartPs_modem_status)reg;

    return retval;
}

/****************************************************************************/
/**
 *
 *  Sets the value of the scratchpad register.  This register has no
 *  functional use and is available to a programmer to use at their own
 *  discretion.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  byte        -- value to set
 *
 * @return
 *  None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - scr/all bits
 *
 ****************************************************************************/
void FUartPs_setScratchpad (FUartPs_T *uartDev, uint8_t value)
{
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    UART_OUTP(value, portmap->scr);
}

/****************************************************************************/
/**
 *
 *  Returns the value of the scratchpad register.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current scratchpad register value.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - scr/all bits
 *
 ****************************************************************************/
uint8_t FUartPs_getScratchpad (FUartPs_T *uartDev)
{
    uint8_t retval = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    retval = UART_INP(portmap->scr);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Reads a single character from the receiver FIFO and returns it.
 *  This function does not check if there is data in the Rx FIFO
 *  beforehand, which is a user responsibility.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The character read from the Rx FIFO.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - rbr/all bits
 *
 ****************************************************************************/
uint8_t FUartPs_read (FUartPs_T *uartDev)
{
    uint8_t retval = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    retval = UART_INP(portmap->rbr_thr_dll);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Writes a single character to the transmitter FIFO.  This function
 *  does not check if there is space in the Tx FIFO beforehand, which is
 *  a user responsibility.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  character   -- character to write
 *
 * @return
 *  None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - thr/all bits
 *
 ****************************************************************************/
void FUartPs_write (FUartPs_T *uartDev, uint8_t character)
{
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    UART_OUTP(character, portmap->rbr_thr_dll);
}

/****************************************************************************/
/**
 *
 *  This function reads characters from the Rx FIFO, using burst
 *  transactions on the AHB bus.  This function does not check if there
 *  is enough valid data in the Rx FIFO beforehand, which is a user
 *  responsibility.  The length argument should never exceed the FIFO
 *  depth.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  buffer      -- buffer to which data is stored
 *  length      -- number of characters to read
 *
 * @return
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - srbr/all bits
 *
 *  This function is affected by the SIR_MODE hardware parameter.
 *
 *  The ANSI C memcpy() function is used to read from the shadow receive
 *  buffer registers in bursts of four characters.  When compiled using
 *  the ARM compiler, this function is replaced with efficient LDM/STM
 *  assembler instructions, thus improving AHB bus usage.
 *
 ****************************************************************************/
void FUartPs_burstRead (FUartPs_T *uartDev, uint8_t *buffer, uint32_t length)
{
    int i, index=0;
    uint32_t tmpbuf[4];
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    i = 0;
    // if there are four or more bytes to be read
    while (((int)length - (i + 4)) >= 4)
    {
        // burst read from the shadow receive buffer registers
        (void)memcpy(tmpbuf, &(portmap->srbr_sthr[0]), sizeof(uint32_t) * 4);
        // copy 4 bytes from temporary word buffer to user buffer
        for (index = 0; index < 4; i++, index++)
        {
            buffer[i] = tmpbuf[index];
        }
    }
    // read any remaining characters
    while (i < length)
    {
        buffer[i++] = (uint8_t)UART_INP(portmap->rbr_thr_dll);
    }

    return;
}

/****************************************************************************/
/**
 *
 *  This function writes a characters to the Tx FIFO, using burst
 *  transfers on the AHB bus.  This function does not check if there is
 *  sufficient spave available in the Tx FIFO beforehand, which is a
 *  user responsibility.  The length argument should never exceed the
 *  FIFO depth.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  buffer      -- buffer from which to send data
 *  length      -- number of characters to write
 *
 * @return
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - sthr/all bits
 *
 *  This function is affected by the SIR_MODE hardware parameter.
 *  The ANSI C memcpy() function to write to the shadow transmit hold
 *  registers in bursts of four characters.  When compiled using the ARM
 *  compiler, this function is replaced with efficient LDM/STM assembler
 *  instructions, thus improving AHB bus usage.
 *
 ****************************************************************************/
void FUartPs_burstWrite (FUartPs_T *uartDev, uint8_t *buffer, uint32_t length)
{
    int i, index=0;
    uint32_t tmpbuf[4];
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    i = 0;
    while (((int)length - i) >= 4)
    {
        // copy 4 bytes from user buffer to temporary word buffer
        for (index = 0; index < 4; i++, index++)
        {
            tmpbuf[index] = buffer[i];
        }
        // burst write to the shadow transmit hold registers
        (void)memcpy(&(portmap->srbr_sthr[0]), tmpbuf, sizeof(uint32_t) * 4);
    }
    // write any remaining characters
    while (i < length)
    {
        UART_OUTP(buffer[i++], portmap->rbr_thr_dll);
    }

    return;
}

/****************************************************************************/
/**
 *
 *  Enables specified interrupt(s).
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  interrupts  -- interrupt(s) to enable
 *
 * @return
 * None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - ier/all bits
 *
 ****************************************************************************/
void FUartPs_enableIrq (FUartPs_T *uartDev, enum FUartPs_irq interrupts)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // get current interrupt enable settings
    reg = UART_INP(portmap->ier_dlh);
    // set interrupts to be enabled
    reg |= interrupts;
    // update copy of IER value (used when avoiding shared data issues)
    uartDev->instance.ier_save = reg;
    // write new IER value
    UART_OUTP(reg, portmap->ier_dlh);
}

/****************************************************************************/
/**
 *
 *  Disables specified interrupt(s).
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  interrupts  -- interrupt(s) to disable
 *
 * @return
 * None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - ier/all bits
 *
 ****************************************************************************/
void FUartPs_disableIrq (FUartPs_T *uartDev, enum FUartPs_irq interrupts)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // get current interrupt enable settings
    reg = UART_INP(portmap->ier_dlh);
    // mask interrupts to be disabled
    reg = (uint32_t)reg & ~interrupts;
    // update copy of IER value (used when avoiding shared data issues)
    uartDev->instance.ier_save = reg;
    // write new IER value
    UART_OUTP(reg, portmap->ier_dlh);
}

/****************************************************************************/
/**
 *
 *  Returns whether the specified interrupt is enabled or not.  Only one
 *  interrupt may be specifed per invocation of this function.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  interrupt   -- interrupt to check
 *
 * @return
 *  TRUE        -- interrupt is enabled
 *  FALSE       -- interrupt is disabled
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - ier/all bits
 *
 ****************************************************************************/
BOOL FUartPs_isIrqEnabled (FUartPs_T *uartDev, enum FUartPs_irq interrupt)
{
    BOOL retval = FALSE;
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);
    FMSH_ASSERT((interrupt == Uart_irq_erbfi) ||
                (interrupt == Uart_irq_etbei) || (interrupt == Uart_irq_elsi) ||
                (interrupt == Uart_irq_edssi));

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // get current interrupt enable settings
    reg = UART_INP(portmap->ier_dlh);
    // specified interrupt enabled?
    if ((reg & interrupt) != 0)
    {
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    return retval;
}

/****************************************************************************/
/**
 *
 *  Returns the current interrupt mask.  For each bitfield, a value of
 *  '0' indicates that an interrupt is masked while a value of '1'
 *  indicates that an interrupt is enabled.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *       value of ier register
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - ier/all bits
 *
 ****************************************************************************/
uint8_t FUartPs_getIrqMask (FUartPs_T *uartDev)
{
    uint8_t retval = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // get current interrupt enable settings
    retval = UART_INP(portmap->ier_dlh);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Returns the event identification number of the highest priority
 *  interrupt that is active.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  The current highest priority active interrupt.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - iir/iid
 *  warnings
 *  This function can indirectly clear a THRE interrupt.  Reading the
 *  IIR register clears any pending THRE interrupt.
 *
 ****************************************************************************/
enum FUartPs_event FUartPs_getActiveIrq (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_event retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // get current interrupt ID
    reg = UART_INP(portmap->iir_fcr);
    retval = (enum FUartPs_event)FMSH_BIT_GET(reg, UART_IIR_INTERRUPT_ID);

    return retval;
}

/****************************************************************************/
/**
 *
 *  Sets the trigger level of the transmitter FIFO empty interrupt.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  trigger       -- level at which to set trigger
 *
 * @return
 *  None.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/tet
 *   - stet/trigger
 *
 ****************************************************************************/
void FUartPs_setTxTrigger (FUartPs_T *uartDev, enum FUartPs_tx_trigger trigger)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // save user-specified Tx trigger
    uartDev->instance.txTrigger = trigger;

    // update stored FCR value
    FMSH_BIT_SET(uartDev->instance.value_in_fcr, UART_FCR_TX_EMPTY_TRIGGER,
                 trigger);
    // set transmitter empty trigger

    reg = 0;
    FMSH_BIT_SET(reg, UART_STET_TRIGGER, trigger);
    UART_OUTP(reg, portmap->stet);
}

/****************************************************************************/
/**
 *
 *  Gets the trigger level of the transmitter FIFO empty interrupt.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *
 * @return
 *  transmitter trigger level
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/tet
 *   - stet/trigger
 *
 ****************************************************************************/
enum FUartPs_tx_trigger FUartPs_getTxTrigger (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_tx_trigger retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    reg = UART_INP(portmap->stet);
    retval = (enum FUartPs_tx_trigger)FMSH_BIT_GET(reg, UART_STET_TRIGGER);
    return retval;
}

/****************************************************************************/
/**
 *
 *  Sets the trigger level for the receiver FIFO full interrupt.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  trigger       -- level at which to set trigger
 *
 * @return
 *  None
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/rt
 *   - srt/trigger
 *
 ****************************************************************************/
void FUartPs_setRxTrigger (FUartPs_T *uartDev, enum FUartPs_rx_trigger trigger)
{
    uint32_t reg = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // update stored FCR value
    FMSH_BIT_SET(uartDev->instance.value_in_fcr, UART_FCR_RCVR_TRIGGER,
                 trigger);
    // set receiver trigger
    reg = 0;
    FMSH_BIT_SET(reg, UART_SRT_TRIGGER, trigger);
    UART_OUTP(reg, portmap->srt);
}

/****************************************************************************/
/**
 *
 *  Gets the trigger level of the receiver FIFO full interrupt.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  level       -- level at which to set trigger
 *
 * @return
 *  The receiver empty trigger level.
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - fcr/rt
 *   - srt/trigger
 *
 ****************************************************************************/
enum FUartPs_rx_trigger FUartPs_getRxTrigger (FUartPs_T *uartDev)
{
    uint32_t reg = 0U;
    enum FUartPs_rx_trigger retval;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // get receiver trigger
    reg = UART_INP(portmap->srt);
    retval = (enum FUartPs_rx_trigger)FMSH_BIT_GET(reg, UART_SRT_TRIGGER);

    return retval;
}

/****************************************************************************/
/**
 *
 *  This function is used to set a user listener callback.  The listener
 *  function is responsible for handling all events/interrupts that are
 *  not handled internally by the Driver Kit.  This encompasses events
 *  such as errors or receiving data when there is no user Rx buffer set
 *  up.  In this respect, it can be considered as a way of extending the
 *  default interrupt handler.
 *
 * @param
 *  uartDev         -- FMSH_apb_uart handle
 *  listener    -- user listener function
 *
 * @return
 *  None.
 *
 * @note
 *  This function enables the following interrupts:
 *  Uart_irq_erbfi
 *  Uart_irq_elsi
 *
 *  When the FUartPs_userIrqHandler()
 *  interrupt  handler is being used, this function is used to set the
 *  user listener function that is called by it.
 *
 ****************************************************************************/
void FUartPs_setListener (FUartPs_T *uartDev, FMSH_callback userFunction)
{
    UART_COMMON_ASSERT(uartDev);
    FMSH_ASSERT(userFunction != NULL);

    // set user listener function
    uartDev->instance.listener = userFunction;

    // enable interrupts/events which should be handled by the user
    // listener function
    FUartPs_enableIrq(uartDev,
                      (enum FUartPs_irq)(Uart_irq_elsi | Uart_irq_erbfi));
}

/****************************************************************************/
/**
 *
 *  This function identifies the current highest priority active
 *  interrupt, if any, and forwards it to the user-specified listener
 *  function for processing.  This allows a user absolute control over
 *  how each UART interrupt is processed.
 *
 *  None of the other Interrupt API functions can be used with this
 *  interrupt handler.  This is because they are symbiotic with the
 *  FUartPs_irqHandler() interrupt handler.  All Command and Status API
 *  functions, however, can be used within the user listener function.
 *  This is in contrast to FUartPs_irqHandler(), where FUartPs_read(),
 *  FUartPs_write(), FUartPs_burstRead() and FUartPs_burstWrite() cannot
 *  be used within the user listener function.
 *
 * @param
 *  uartDev      -- FMSH_apb_uart handle
 *
 * @return
 *  TRUE        -- an interrupt was processed
 *  FALSE       -- no interrupt was processed
 *  -FMSH_EIO     -- unrecognized interrupt ID was read
 *
 * @note
 *  Accesses the following FMSH_apb_uart register(s)/bit field(s):
 *   - iir/all bits
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the UART in a poll-driven manner.
 * warnings
 *  The user listener function is run in interrupt context and, as such,
 *  care must be taken to ensure that any data shared between it and
 *  normal code is adequately protected from corruption.  Depending on
 *  the target platform, spinlocks, mutexes or semaphores may be used to
 *  achieve this.
 *
 ****************************************************************************/
int FUartPs_userIrqHandler (FUartPs_T *uartDev)
{
    int retval  = 0;
    int callbackArg = 0;
    uint32_t reg = 0U;
    enum FUartPs_event event;
    FUartPs_Portmap_T *portmap = NULL;
    FMSH_callback userCallback;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // A listener function _must_ be setup before using interrupts.
    FMSH_ASSERT(uartDev->instance.listener != NULL);

    userCallback = NULL;
    callbackArg = 0;

    // what caused this interrupt
    reg = UART_INP(portmap->iir_fcr);
    event = (enum FUartPs_event)(reg & 0xf);
    // assume an interrupt will be processed
    retval = TRUE;

    // process event
    switch (event)
    {
    case Uart_event_none:
        // no interrupt has occurred
        retval = FALSE;
        break;
    case Uart_event_modem:
        userCallback = uartDev->instance.listener;
        callbackArg = Uart_event_modem;
        break;
    case Uart_event_thre:
        userCallback = uartDev->instance.listener;
        callbackArg = Uart_event_thre;
        break;
    case Uart_event_timeout:
        userCallback = uartDev->instance.listener;
        callbackArg = Uart_event_timeout;
        break;
    case Uart_event_data:
        userCallback = uartDev->instance.listener;
        callbackArg = Uart_event_data;
        break;
    case Uart_event_line:
        userCallback = uartDev->instance.listener;
        callbackArg = Uart_event_line;
        break;
    case Uart_event_busy:
        userCallback = uartDev->instance.listener;
        callbackArg = Uart_event_busy;
        break;
    default:
        // If we've reached this point, the value read from the
        // iir_fcr register is unrecognized.
        retval = -FMSH_EIO;
        break;
    }

    // If an interrupt has occurred, pass it to the user listener
    // function.
    if (userCallback != NULL)
    {
        userCallback(uartDev, callbackArg);
    }

    return retval;
}
/****************************************************************************
 *
 * This function is used to handler irq from uart
 *
 * @param   uartDev is a pointer to the uart instance.
 *
 * @return
 *  TRUE        -- an interrupt was processed
 *  FALSE       -- no interrupt was processed
 *  -FMSH_EIO     -- unrecognized interrupt ID was read
 *
 * @note     None.
 *
 ****************************************************************************/
int FUartPs_irqHandler (FUartPs_T *uartDev)
{
    int retval = 0;
    int callbackArg = 0;
    BOOL ptime, fifos =FALSE;
    uint32_t numChars = 0U;
    uint32_t chars_to_align = 0U;
    uint8_t *tmp = NULL;
    uint32_t reg = 0U;
    enum FUartPs_event event;
    FUartPs_Param_T *param;
    FUartPs_Portmap_T *portmap = NULL;
    FUartPs_Instance_T *instance;
    FMSH_callback userCallback;

    UART_COMMON_ASSERT(uartDev);

    param = &(uartDev->comp_param);
    portmap = (FUartPs_Portmap_T *)uartDev->base_address;
    instance = &(uartDev->instance);

    // A listener function _must_ be setup before using interrupts.
    FMSH_REQUIRE(instance->listener != NULL);

    // Assume an interrupt will be processed.  This will be set to false
    // if there is no active interrupt found.
    retval = TRUE;
    userCallback = NULL;
    callbackArg = 0;

    // what caused this interrupt
    reg = UART_INP(portmap->iir_fcr);
    event = (enum FUartPs_event)(reg & 0xf);
    // are FIFOs enabled?
    fifos = (FMSH_BIT_GET(reg, UART_IIR_FIFO_STATUS) == 0 ? FALSE : TRUE);
    // process event
    switch (event)
    {
    case Uart_event_none:
        // no pending interrupt
        retval = FALSE;
        break;
    case Uart_event_modem:
        userCallback = instance->listener;
        callbackArg = Uart_event_modem;
        break;
    case Uart_event_thre:
        // The Tx empty interrupt should never be enabled when using
        // DMA with hardware handshaking.
        if (instance->txRemain == 0)
        {
            switch (instance->state)
            {
            case Uart_state_tx:
            case Uart_state_tx_rx:
                // disable interrupt
                FUartPs_disableIrq(uartDev, Uart_irq_etbei);
                // restore user Tx trigger
                FMSH_BIT_SET(instance->value_in_fcr, UART_FCR_TX_EMPTY_TRIGGER,
                             instance->txTrigger);
                UART_OUTP(instance->value_in_fcr, portmap->iir_fcr);
                // inform user of end of transfer
                userCallback = instance->txCallback;
                // pass callback the number of bytes sent
                callbackArg = instance->txLength;
                // update state
                if (instance->state == Uart_state_tx_rx)
                {
                    instance->state = Uart_state_rx;
                }
                else
                {
                    instance->state = Uart_state_idle;
                }
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
                break;
            default:
                // We should not get this interrupt in any other
                // state.
                FMSH_ASSERT(FALSE);
                break;
            }
        }
        else
        {
            // Is PTIME enabled?
            ptime = FUartPs_isPtimeEnabled(uartDev);
            switch (instance->state)
            {
            case Uart_state_tx:
            case Uart_state_tx_rx:
                if (fifos == FALSE)
                {
                    // Can only write one character if FIFOs are
                    // disabled.
                    numChars = 1;
                }
                else if (param->fifo_stat == TRUE)
                {
                    // If the FIFO status registers are
                    // available, we can query how many
                    // characters are already in the Tx FIFO.
                    numChars = param->fifo_depth -
                               FUartPs_getTxFifoLevel(uartDev);
                }
                else if (ptime == FALSE)
                {
                    // If PTIME is disabled when a
                    // Uart_event_thre interrupt occurs, the Tx
                    // FIFO is completely empty.
                    numChars = param->fifo_depth;
                }
                else
                {
                    // How many characters we can write to the
                    // Tx FIFO depends on the trigger which
                    // caused this interrupt.
                    switch (FUartPs_getTxTrigger(uartDev))
                    {
                    case Uart_empty_fifo:
                        numChars = param->fifo_depth;
                        break;
                    case Uart_two_chars_in_fifo:
                        numChars = (param->fifo_depth - 2);
                        break;
                    case Uart_quarter_full_fifo:
                        numChars = (param->fifo_depth * 3 / 4);
                        break;
                    case Uart_half_full_fifo:
                        numChars = (param->fifo_depth / 2);
                        break;
                    default:
                        FMSH_ASSERT(FALSE);
                        break;
                    }
                }
                // Write maximum number of bytes to the Tx
                // FIFO with no risk of overflow.
                UART_X_FIFO_WRITE(numChars);

                if ((ptime == TRUE) && (fifos == TRUE))
                {
                    // Send more bytes if the Tx FIFO is still
                    // not full.  Stops when LSR THRE bit (FIFO
                    // full) becomes Set (PTIME enabled!).
                    UART_FIFO_WRITE();
                }
                break;
            default:
                // We should not get this interrupt in any other
                // state.
                FMSH_ASSERT(FALSE);
                break;
            }
            // Ensure we get an interrupt when the last byte has
            // been sent.  The user callback function will be called
            // on the next thre interrupt.
            if (instance->txRemain == 0)
            {
                FMSH_BIT_SET(instance->value_in_fcr, UART_FCR_TX_EMPTY_TRIGGER,
                             Uart_empty_fifo);
                UART_OUTP(instance->value_in_fcr, portmap->iir_fcr);
            }
        }
        break;
    case Uart_event_timeout:
    case Uart_event_data:
        // The Rx full interrupt should never be enabled when using
        // a DMA with hardware handshaking.
        if ((instance->state == Uart_state_rx) ||
            (instance->state == Uart_state_tx_rx))
        {
            // Does the Rx buffer need to be 32-bit word-aligned?
            // Need to do this here as UART_[X_]FIFO_READ() works
            // efficiently on word writes from rxHold
            chars_to_align = 0;
            if (instance->rxAlign == FALSE)
            {
                tmp = (uint8_t *)instance->rxBuffer;
                // need to store any errors that may occur so the
                // user sees them on a subsequent call to
                // dw_uart_getLineStatus().
                reg = UART_INP(portmap->lsr);
                // save any line status errors
                instance->lsr_save = (reg & (Uart_line_oe | Uart_line_pe |
                                             Uart_line_fe | Uart_line_bi));
                while ((((uintptr_t)tmp) & 0x3) && (instance->rxRemain > 0) &&
                       ((reg & Uart_line_dr) == Uart_line_dr))
                {
                    *tmp++ = UART_INP(portmap->rbr_thr_dll);
                    instance->rxRemain--;
                    // Record the number of characters read used to
                    // align the buffer pointer.
                    chars_to_align++;
                    // read the line status register
                    reg = UART_INP(portmap->lsr);
                    // save any line status errors
                    instance->lsr_save = (reg & (Uart_line_oe | Uart_line_pe |
                                                 Uart_line_fe | Uart_line_bi));
                }
                instance->rxBuffer = (uint32_t *)tmp;
                if (((uintptr_t)tmp & 0x3) == 0)
                {
                    instance->rxAlign = TRUE;
                }
            }

            // number of characters to read from the RBR/Rx FIFO
            numChars = 0;
            // The following code only gets executed when the Rx
            // buffer is 32-bit word-aligned.
            if (instance->rxAlign == TRUE)
            {
                if (fifos == FALSE)
                {
                    // There is only one character available if
                    // FIFOs are disabled.
                    if (chars_to_align == 0)
                    {
                        numChars = 1;
                    }
                }
                else if (param->fifo_stat == TRUE)
                {
                    // If the FIFO status registers are available,
                    // we can simply query how many characters are
                    // in the Rx FIFO.
                    numChars = FUartPs_getRxFifoLevel(uartDev);
                }
                else if ((event == Uart_event_data) &&
                         (FUartPs_isPtimeEnabled(uartDev) == TRUE))
                {
                    // If this interrupt was caused by received data
                    // reaching the Rx FIFO full trigger level.
                    switch (FUartPs_getRxTrigger(uartDev))
                    {
                    case Uart_one_char_in_fifo:
                        numChars = 1;
                        break;
                    case Uart_fifo_quarter_full:
                        numChars = param->fifo_depth / 4;
                        break;
                    case Uart_fifo_half_full:
                        numChars = param->fifo_depth / 2;
                        break;
                    case Uart_fifo_two_less_full:
                        numChars = param->fifo_depth - 2;
                        break;
                    default:
                        FMSH_ASSERT(FALSE);
                        break;
                    }
                    // Subtract any bytes already read when
                    // aligning the Rx buffer.
                    if (numChars > chars_to_align)
                    {
                        numChars -= chars_to_align;
                    }
                }
                else{
                    ;/* no deal with */
                }
                // Read maximum known number of characters
                // from the Rx FIFO without underflowing it.
                UART_X_FIFO_READ(numChars);
                // Read any remaining bytes in the Rx FIFO (until
                // LSR Data Ready bit is _clear).
                UART_FIFO_READ();
            }  // instance->rxAlign == true

            // end of rxBuffer?
            if (instance->rxRemain == 0)
            {
                // flush rxHold variable to user Rx buffer
                FUartPs_flushRxHold(uartDev);
                // inform user of end of transfer
                userCallback = instance->rxCallback;
                // pass callback the number of bytes received
                callbackArg = instance->rxLength;
                // update state
                if (instance->state == Uart_state_tx_rx)
                {
                    instance->state = Uart_state_tx;
                }
                else
                {
                    instance->state = Uart_state_idle;
                }
                instance->rxBuffer = NULL;
                instance->rxCallback = NULL;
            }  // rxRemain == 0
            else if ((event == Uart_event_timeout) &&
                     (instance->callbackMode == Uart_rx_timeout))
            {
                // Call the associated callback function if this
                // interrupt was caused by a character timeout and
                // the callback mode has been set to
                // Uart_rx_timeout.
                userCallback = instance->rxCallback;
                callbackArg = instance->rxLength - instance->rxRemain;
            }  // rxRemain == 0
            else{
                ;/* no deal with */
            }
        }  // Uart_state_rx
        else
        {
            // update state
            switch (instance->state)
            {
            case Uart_state_tx:
                instance->state = Uart_state_tx_rx_req;
                break;
            case Uart_state_idle:
                instance->state = Uart_state_rx_req;
                break;
            default:
                FMSH_ASSERT(FALSE);
                break;
            }
            userCallback = instance->listener;
            callbackArg = event;
        }
        break;
    case Uart_event_line:
        // inform user callback function
        userCallback = instance->listener;
        callbackArg = Uart_event_line;
        break;
    case Uart_event_busy:
        userCallback = instance->listener;
        callbackArg = Uart_event_busy;
        break;
    default:
        // If we've reached this point, the value read from the
        // iir_fcr register is unrecognized.
        retval = -FMSH_EIO;
        break;
    }

    // If required, call the user listener or callback function
    if (userCallback != NULL)
    {
        userCallback(uartDev, callbackArg);
    }

    // The driver should never be in one of these states when the
    // interrupt handler is finished.  If it is, the user listener
    // function has not correctly dealt with an event/interrupt.
    FMSH_REQUIRE(instance->state != Uart_state_rx_req);
    FMSH_REQUIRE(instance->state != Uart_state_tx_rx_req);

    return retval;
}
/******************************************************************************
 *
 *  This function is used to transmit date.
 *
 * @param
 *  uartDev      -- FMSH_apb_uart handle
 *  buffer   --
 *
 * @return
 *  0        --
 *  -FMSH_EBUSY     --
 *
 * @note
 *
 ******************************************************************************/
int FUartPs_transmit (FUartPs_T *uartDev, void *buffer, uint32_t length,
                      FMSH_callback userFunction)
{
    int retval = 0;
    uint32_t numChars = 0U;
    uint8_t *tmp;
    FUartPs_Param_T *param;
    FUartPs_Portmap_T *portmap = NULL;
    FUartPs_Instance_T *instance;

    // FMSH_REQUIREMENTS(uartDev);
    FMSH_REQUIRE(buffer != NULL);
    FMSH_REQUIRE(length != 0);

    param = &(uartDev->comp_param);
    portmap = (FUartPs_Portmap_T *)uartDev->base_address;
    instance = &(uartDev->instance);

    // disable UART interrupts
    UART_ENTER_CRITICAL_SECTION();

    if ((instance->state == Uart_state_tx) ||
        (instance->state == Uart_state_tx_rx))
    {
        retval = -FMSH_EBUSY;
    }
    else
    {
        retval = 0;

        // update state
        switch (instance->state)
        {
        case Uart_state_rx:
            instance->state = Uart_state_tx_rx;
            break;
        case Uart_state_idle:
            instance->state = Uart_state_tx;
            break;
        default:
            FMSH_ASSERT(FALSE);
            break;
        }

        // reset number of characters sent in current/last transfer
        instance->txLength = length;
        instance->txRemain = length;
        instance->txCallback = userFunction;
        instance->txHold = 0;
        instance->txIdx = 0;

        // check for non word-aligned buffer as UART[_X]_FIFO_WRITE()
        // works efficiently on word reads from txHold
        tmp = (uint8_t *)buffer;
        while (((uintptr_t)tmp) & 0x3)
        {
            instance->txHold |= ((*tmp++ & 0xff) << (instance->txIdx * 8));
            instance->txIdx++;
        }
        instance->txBuffer = (uint32_t *)tmp;

        if (instance->fifos_enabled == FALSE)
        {
            UART_FIFO_WRITE();
        }
        else if (FUartPs_isTxFifoEmpty(uartDev) == TRUE)
        {
            numChars = param->fifo_depth;
            UART_X_FIFO_WRITE(numChars);
        }
        else if (param->fifo_stat == TRUE)
        {
            numChars = param->fifo_depth - FUartPs_getTxFifoLevel(uartDev);
            UART_X_FIFO_WRITE(numChars);
        }
        else
        {
            UART_FIFO_WRITE();
        }

        // make sure THR/Tx FIFO empty interrupt is enabled
        instance->ier_save |= Uart_irq_etbei;
    }

    // restore UART interrupts
    UART_EXIT_CRITICAL_SECTION();

    return retval;
}

/******************************************************************************
 *
 *  This function is used to receive date.
 *
 * @param
 *  uartDev      -- FMSH_apb_uart handle
 *  buffer   -- save address
 *  length   -- number of perparing to receive
 *  userFunction   -- callback function
 *
 * @return
 *  0        --
 *  -FMSH_EBUSY     --
 *
 * @note
 *
 ******************************************************************************/
int FUartPs_receive (FUartPs_T *uartDev, void *buffer, uint32_t length,
                     FMSH_callback userFunction)
{
    int retval  = 0;
    uint32_t numChars = 0U;
    FUartPs_Param_T *param;
    FUartPs_Portmap_T *portmap = NULL;
    FUartPs_Instance_T *instance;
    FMSH_callback userCallback;

    FMSH_REQUIRE(buffer != NULL);
    FMSH_REQUIRE(length != 0);

    param = &(uartDev->comp_param);
    portmap = (FUartPs_Portmap_T *)uartDev->base_address;
    instance = &(uartDev->instance);

    // disable UART interrupts
    UART_ENTER_CRITICAL_SECTION();

    if ((instance->state == Uart_state_rx) ||
        (instance->state == Uart_state_tx_rx))
    {
        retval = -FMSH_EBUSY;
    }
    else
    {
        retval = 0;

        switch (instance->state)
        {
        case Uart_state_tx:
        case Uart_state_tx_rx_req:
            instance->state = Uart_state_tx_rx;
            break;
        case Uart_state_idle:
        case Uart_state_rx_req:
            instance->state = Uart_state_rx;
            break;
        default:
            FMSH_ASSERT(FALSE);
            break;
        }

        // check for buffer alignment
        if (((uintptr_t)buffer & 0x3) == 0)
        {
            instance->rxAlign = TRUE;
        }
        else
        {
            instance->rxAlign = FALSE;
        }

        // update Rx buffer
        instance->rxBuffer = buffer;
        // reset number of characters received for current transfer
        instance->rxLength = length;
        instance->rxRemain = length;
        instance->rxCallback = userFunction;
        instance->rxHold = 0;
        instance->rxIdx = 4;

        // if the Rx buffer is 32-bit word-aligned
        if (instance->rxAlign == TRUE)
        {
            if (instance->fifos_enabled == FALSE)
            {
                UART_FIFO_READ();
            }
            else if (param->fifo_stat == TRUE)
            {
                numChars = FUartPs_getRxFifoLevel(uartDev);
                UART_X_FIFO_READ(numChars);
            }
            else
            {
                UART_FIFO_READ();
            }
        }

        // if we've already filled the user Rx buffer
        if (instance->rxRemain == 0)
        {
            // flush any byte in the hold register
            FUartPs_flushRxHold(uartDev);
            // update state
            switch (instance->state)
            {
            case Uart_state_tx_rx:
                instance->state = Uart_state_tx;
                break;
            case Uart_state_rx:
                instance->state = Uart_state_idle;
                break;
            default:
                FMSH_ASSERT(FALSE);
                break;
            }
            userCallback = instance->rxCallback;
            instance->rxBuffer = NULL;
            instance->rxCallback = NULL;
            // call callback functoin
            if (userCallback != NULL)
            {
                (userCallback)(uartDev, instance->rxLength);
            }
        }
        else
        {
            // ensure receive is underway
            instance->ier_save |= Uart_irq_erbfi;
        }
    }

    // restore UART interrupts
    UART_EXIT_CRITICAL_SECTION();

    return retval;
}

/**********************************************************************/
/***                    PRIVATE FUNCTIONS                           ***/
/**********************************************************************/

/****************************************************************************/
/**
 *
 *  Initializes the uart instance structure members.
 *
 * @param
 *  uartDev      -- FMSH_apb_uart handle
 *
 * @return
 * None
 *
 * @note
 *  Interrupt *must* be disabled before calling this function to avoid
 *  shared data issues.
 *
 ****************************************************************************/
void FUartPs_resetInstance (FUartPs_T *uartDev)
{
    UART_COMMON_ASSERT(uartDev);

    // initialize instance variables
    uartDev->instance.state = Uart_state_idle;
    uartDev->instance.value_in_fcr = 0;
    uartDev->instance.lsr_save = 0;
    uartDev->instance.ier_save = 0;
    uartDev->instance.txCallback = NULL;
    uartDev->instance.rxCallback = NULL;
    uartDev->instance.callbackMode = Uart_buffer_full;
    uartDev->instance.listener = NULL;
    uartDev->instance.txBuffer = NULL;
    uartDev->instance.txHold = 0;
    uartDev->instance.txIdx = 0;
    uartDev->instance.txLength = 0;
    uartDev->instance.txRemain = 0;
    uartDev->instance.rxBuffer = NULL;
    uartDev->instance.rxHold = 0;
    uartDev->instance.rxIdx = 0;
    uartDev->instance.rxLength = 0;
    uartDev->instance.rxRemain = 0;
    uartDev->instance.rxAlign = FALSE;
    uartDev->instance.fifos_enabled = FUartPs_areFifosEnabled(uartDev);
    uartDev->instance.txTrigger = FUartPs_getTxTrigger(uartDev);
}

/****************************************************************************/
/**
 *
 *  Flushes any characters present in the Rx FIFO to the user Rx buffer.
 *
 * @param
 *  uartDev      -- FMSH_apb_uart handle
 *
 * @return
 *  0           -- if successful
 *  -EPERM      -- no Rx transfer is currently in progress
 *
 * @note
 *  Interrupt *must* be disabled before calling this function to avoid
 *  shared data issues.
 *
 ****************************************************************************/
int FUartPs_flushRxFifo (FUartPs_T *uartDev)
{
    int retval  = 0;
    uint32_t numChars = 0U;
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // if a user Rx buffer is available
    if (uartDev->instance.rxBuffer != NULL)
    {
        retval = 0;

        if (uartDev->instance.fifos_enabled == FALSE)
        {
            UART_FIFO_READ();
        }
        else if (uartDev->comp_param.fifo_stat == TRUE)
        {
            numChars = FUartPs_getRxFifoLevel(uartDev);
            UART_X_FIFO_READ(numChars);
        }
        else
        {
            UART_FIFO_READ();
        }

        // flush the Rx 'hold' variable
        FUartPs_flushRxHold(uartDev);
    }
    else
    {
        retval = -FMSH_EPERM;  // permission denied (no Rx buffer set)
    }

    return retval;
}

/****************************************************************************/
/**
 *
 *  Flushes any characters present in the uartDev->instance.rxHold variable to
 *  the user Rx buffer.
 *
 * @param
 *  uartDev      -- FMSH_apb_uart handle
 *
 * @return
 * None
 *
 * @note
 *  Interrupt *must* be disabled before calling this function to avoid
 *  shared data issues.
 *
 ****************************************************************************/
void FUartPs_flushRxHold (FUartPs_T *uartDev)
{
    int i = 0;
    uint8_t *tmp;
    uint32_t rxMask, c =0U;

    UART_COMMON_ASSERT(uartDev);

    FMSH_ASSERT(uartDev->instance.rxIdx <= 4);
    // if there is data in the rxHold variable
    if (uartDev->instance.rxIdx != 4)
    {
        // Flush read buffer of data if last read in
        // UART_[X_]FIFO_READ was not 4 complete bytes
        // if there is greater than 4 bytes remaining in the Rx buffer
        if (uartDev->instance.rxRemain >= 4)
        {
            // safe to perform 32-bit writes
            rxMask = ((uint32_t)(0xffffffff) >> (8 * uartDev->instance.rxIdx));
            c = rxMask &
                (uartDev->instance.rxHold >> (8 * uartDev->instance.rxIdx));
            *uartDev->instance.rxBuffer = (*uartDev->instance.rxBuffer &
                                           ~rxMask) |
                                          c;
        }
        else
        {
            // Need to write each byte individually to avoid writing
            // past the end of the Rx buffer.
            // tmp = next free location in Rx buffer
            tmp = (uint8_t *)uartDev->instance.rxBuffer;
            // shift rxHold so that lsB contains valid data
            c = uartDev->instance.rxHold >> (8 * uartDev->instance.rxIdx);
            // write out valid character to Rx buffer
            for (i = (4 - uartDev->instance.rxIdx); i > 0; i--)
            {
                *tmp++ = (uint8_t)(c & 0xff);
                c >>= 8;
            }
        }
    }
}

/****************************************************************************/
/**
 *
 *  This function attempts to automatically discover the component
 *  hardware parameters, if possible.  If this is not supported, an
 *  error code is returned.
 *
 * @param
 *  uartDev      -- FMSH_apb_uart handle
 *
 * @return
 *  0           if successful
 *  -ENOSYS     -- auto-configuration is not supported
 *
 * @note
 *  This function is affected by the ADD_ENCODED_PARAMS hardware
 *  parameter.
 *  Interrupt *must* be disabled before calling this function to avoid
 *  shared data issues.
 *
 ****************************************************************************/
int FUartPs_autoCompParams (FUartPs_T *uartDev)
{
    int retval  = 0;
    uint32_t comp_param;
    FUartPs_Portmap_T *portmap = NULL;

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;

    // assume component parameters register is not available
    retval = -FMSH_ENOSYS;
    // only version 3.00 and greater support component identification
    // registers
    if (UART_INP(portmap->comp_type) == 0x44570110)
    {
        uartDev->comp_version = UART_INP(portmap->comp_version);
        uartDev->comp_type = UART_INP(portmap->comp_type);
        comp_param = UART_INP(portmap->comp_param_1);
        if (comp_param != 0x0)
        {
            retval = 0;
            uartDev->comp_param.afce_mode = FMSH_BIT_GET(comp_param,
                                                         UART_PARAM_AFCE_MODE);
            uartDev->comp_param.fifo_access = FMSH_BIT_GET(
                comp_param, UART_PARAM_FIFO_ACCESS);
            uartDev->comp_param.fifo_depth = FMSH_BIT_GET(comp_param,
                                                          UART_PARAM_FIFO_MODE);
            uartDev->comp_param.fifo_depth *= 16;
            uartDev->comp_param.fifo_stat = FMSH_BIT_GET(comp_param,
                                                         UART_PARAM_FIFO_STAT);
            uartDev->comp_param.new_feat = FMSH_BIT_GET(comp_param,
                                                        UART_PARAM_NEW_FEAT);
            uartDev->comp_param.shadow = FMSH_BIT_GET(comp_param,
                                                      UART_PARAM_SHADOW);
            uartDev->comp_param.sir_lp_mode = FMSH_BIT_GET(
                comp_param, UART_PARAM_SIR_LP_MODE);
            uartDev->comp_param.sir_mode = FMSH_BIT_GET(comp_param,
                                                        UART_PARAM_SIR_MODE);
            uartDev->comp_param.thre_mode = FMSH_BIT_GET(comp_param,
                                                         UART_PARAM_THRE_MODE);
            uartDev->comp_param.dma_extra = FMSH_BIT_GET(comp_param,
                                                         UART_PARAM_DMA_EXTRA);
        }
    }

    return retval;
}


void uart_x_fifo_write_max(FUartPs_T *uartDev, int MAX)
{
    FUartPs_Portmap_T *portmap = NULL;

    UART_COMMON_ASSERT(uartDev);

    portmap = (FUartPs_Portmap_T *)uartDev->base_address;
    
    do                                                                      
    {                                                                       
        int i;                                                              
        if (uartDev->instance.txRemain == 0){break;}                        
        for (i = 0; i < (MAX); i++)                                         
        {                                                                   
            if (uartDev->instance.txIdx == 0)                               
            {                                                               
                uartDev->instance.txHold = *(uartDev->instance.txBuffer++); 
                uartDev->instance.txIdx = 4;                                
            }                                                               
            UART_OUTP((uartDev->instance.txHold & 0xff),                  
                        portmap->rbr_thr_dll);                              
            uartDev->instance.txHold >>= 8;                                 
            uartDev->instance.txIdx--;                                      
            if (--uartDev->instance.txRemain == 0){break;}                  
        }                                                                   
    } while (0);
}                                       

void uart_x_fifo_write(FUartPs_T *uartDev)
{
    FUartPs_Portmap_T *portmap = NULL;
    FUartPs_Param_T *param;
    UART_COMMON_ASSERT(uartDev);

    param = &(uartDev->comp_param);
    portmap = (FUartPs_Portmap_T *)uartDev->base_address;
    do                                                                        
    {                                                                         
        volatile uint32_t reg;                                                
        if (uartDev->instance.txRemain == 0){break;}                          
        reg = UART_INP(portmap->lsr);                                         
        /* Save any line status errors that may have occurred */              
        uartDev->instance.lsr_save = (reg & (Uart_line_oe | Uart_line_pe |    
                                             Uart_line_fe | Uart_line_bi));   
        /* Fill up the FIFO until the THRE bit is Set (Tx FIFO full) */       
        if ((FUartPs_isPtimeEnabled(uartDev) == TRUE) &&                      
            (uartDev->instance.fifos_enabled == TRUE))                        
        {                                                                     
            /* While the Tx FIFO is not full (PTIME enabled). */              
            while (FMSH_BIT_GET(reg, UART_LSR_THRE) == FMSH_clear)            
            {                                                                 
                if (uartDev->instance.txIdx == 0)                             
                {                                                             
                    uartDev->instance.txHold = *(                             
                        uartDev->instance.txBuffer++);                        
                    uartDev->instance.txIdx = 4;                              
                }                                                             
                UART_OUTP((uartDev->instance.txHold & 0xff),                
                            portmap->rbr_thr_dll);                            
                uartDev->instance.txHold >>= 8;                               
                uartDev->instance.txIdx--;                                    
                if (--uartDev->instance.txRemain == 0){ break;}               
                reg = UART_INP(portmap->lsr);                                 
                /* Save any line status errors that may have occurred */      
                uartDev->instance.lsr_save = (reg &                           
                                              (Uart_line_oe | Uart_line_pe |  
                                               Uart_line_fe | Uart_line_bi)); 
            }                                                                 
        }                                                                     
        /* Fill up the FIFO if the THRE bit is Set (Tx FIFO empty) */         
        else if (uartDev->instance.fifos_enabled == TRUE)                     
        {                                                                     
           if (FMSH_BIT_GET(reg, UART_LSR_THRE) == FMSH_set){                 
                UART_X_FIFO_WRITE(param->fifo_depth);}                        
        }                                                                     
        /* Can only write one character if FIFOs are disabled */              
        else if (FMSH_BIT_GET(reg, UART_LSR_THRE) == FMSH_set)                
        {                                                                     
            if (uartDev->instance.txIdx == 0)                                 
            {                                                                 
                uartDev->instance.txHold = *(uartDev->instance.txBuffer++);   
                uartDev->instance.txIdx = 4;                                  
            }                                                                 
            UART_OUTP((uartDev->instance.txHold & 0xff),                    
                        portmap->rbr_thr_dll);                                
            uartDev->instance.txHold >>= 8;                                   
            uartDev->instance.txIdx--;                                        
            uartDev->instance.txRemain--;                                     
        }                                                                     
        /* This clause should never be reached */                             
        else                                                                  
        {                                                                     
            FMSH_ASSERT(FALSE);                                               
        }                                                                     
    } while (0);
}
