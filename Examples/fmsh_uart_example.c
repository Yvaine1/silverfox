/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_uart_example.c
 *
 * This file contains a example of uart.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  11/11/2019  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_gic.h"
#include "fmsh_psu_parameters.h"
#include "fmsh_uart_example.h"

/************************** Constant Definitions *****************************/
#define UARTPS_BAUNDRAT       115200
#define TEST_UART_RECEIVE_NUM 100
#define TEST_UART_SEND_NUM    100
#define UART_SEND_TIMEOUT     1000000
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FUartPs_T g_UartDev;

u8 g_UartTXBuffer[256];
u8 g_UartRXBuffer[256];
BOOL g_tx_flag = FALSE;
BOOL g_rx_flag = FALSE;

/************************** Function Prototypes ******************************/
void FUartPsu_Reset (FUartPs_T* dev)
{
    u32 value = 0;
    if (dev->id == FPAR_UARTPS_0_DEVICE_ID)
    {
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value |= (1 << 1);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
        delay_us(5);
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value &= ~(1 << 1);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
    }
    else if (dev->id == FPAR_UARTPS_1_DEVICE_ID)
    {
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value |= (1 << 2);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
        delay_us(5);
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value &= ~(1 << 2);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
    }
    else
    {
        ;
    }
}
/****************************************************************************
 *
 * This function is called when the data of FUartPs_transmit() is all
 * written to the  TX FIFO
 *
 * @param
 *  dev         -- FUartPs_T*
 *  eCode       -- number of the tx data
 *
 * @return None.
 *
 * @note     None.
 *
 ****************************************************************************/
void fmsh_uart_tx_callback (void* dev, int32_t eCode) { g_tx_flag = TRUE; }

/****************************************************************************
 *
 * This function is called when the data of FUartPs_receive() is all
 * written to the  RX FIFO
 *
 * @param
 *  dev         -- FUartPs_T*
 *  eCode       -- number of the rx data
 *
 * @return None.
 *
 * @note     None.
 *
 ****************************************************************************/
void fmsh_uart_rx_callback (void* dev, int32_t eCode) { g_rx_flag = TRUE; }

/****************************************************************************
 *
 * This function is called when an event/interrupt occurs which the
 * low-level driver doesn't normally handle. It instead passes these
 * events to an upper software layer (i.e. this listener function).
 *
 * @param
 * dev is a pointer to the uart instance.
 * eCode is event code.
 *
 * @return   None.
 *
 * @note     None.
 *
 ****************************************************************************/
void fmsh_uart_listener (void* dev, int32_t eCode)
{
    uint32_t reg;

    // process interrupt/event
    switch (eCode)
    {
    case Uart_event_line:
        // This event occurs for overrun/parity/framing errors and a break
        // interrupt. reading the line status register clears this interrupt
        reg = FUartPs_getLineStatus(dev);
        // print what error(s) occured
        if ((reg & Uart_line_oe) != 0)
        {
            TRACE_OUT(1, "overrun ");
        }
        if ((reg & Uart_line_pe) != 0)
        {
            TRACE_OUT(1, "parity ");
        }
        if ((reg & Uart_line_fe) != 0)
        {
            TRACE_OUT(1, "framing ");
        }
        if ((reg & Uart_line_bi) != 0)
        {
            TRACE_OUT(1, "break ");
        }
        TRACE_OUT(1, "\n");
        break;
    case Uart_event_timeout:
        // There has not been any data read from or written to the
        // Rx FIFO within the last four character times and there is
        // at least one character in the Rx FIFO.
    case Uart_event_data:
        // These event only occurs when data is received and no Rx
        // buffer has been set with uart_receive().
        FUartPs_receive(dev, g_UartRXBuffer, TEST_UART_RECEIVE_NUM,
                        fmsh_uart_rx_callback);
        break;
    case Uart_event_modem:
        // This event occurs when there is a change in the status of
        // the modem lines.
        // reading the modem status register clears this interrupt
        reg = FUartPs_getModemStatus(dev);
        break;
    case Uart_event_busy:
        // The line control register should never be written while
        // the UART is busy.
        // TRACE_OUT(DEBUG_OUT,"[listener1] write to LCR while uart is
        // busy!!!\n");
        break;
    default:
        // TRACE_OUT(DEBUG_OUT,"[listener1] unrecognized error/event code:
        // %d\n", eCode);
        FMSH_ASSERT(FALSE);
        break;
    }
}

/****************************************************************************
 *
 * @description
 *    This function is called to initialize the uart0 & uart1.
 *
 * @param none.
 *
 * @return 0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u8 fmsh_uart_init (u8 id)
{
    u8 ret = FMSH_SUCCESS;
    FUartPs_Config* Config = NULL;

    Config = FUartPs_LookupConfig(id);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }
    ret = FUartPs_init(&g_UartDev, Config);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }

    FUartPs_resetInstance(&g_UartDev);

    /*param for transfer*/
    FUartPs_setParity(&g_UartDev, Uart_no_parity);
    FUartPs_setStick(&g_UartDev, Uart_Stick_disable);
    FUartPs_setStopBits(&g_UartDev, Uart_one_stop_bit);
    FUartPs_setDataBits(&g_UartDev, Uart_eight_bits);
    ret = FUartPs_setBaudRate(&g_UartDev, UARTPS_BAUNDRAT);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }

    /*enable fifo*/
    ret = FUartPs_enableFifos(&g_UartDev);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }

    /*initialize irq handler*/
    ret = FGicPs_Connect(&IntcInstance, UART0_INT_ID,
                         (FMSH_InterruptHandler)FUartPs_irqHandler, &g_UartDev);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }
    FGicPs_Enable(&IntcInstance, UART0_INT_ID);

    /*set callback functions*/
    FUartPs_setListener(&g_UartDev, fmsh_uart_listener);

    return FMSH_SUCCESS;
}

/****************************************************************************
 *
 * @description
 *    Used to test UARTs' fuction.
 *
 * @param none.
 *
 * @return none.
 *
 * @note     none.
 *
 ****************************************************************************/
u8 FUartPs_example (u8 id)
{
    long timeout;
    u8 i = 0;

    for (i = 0; i < TEST_UART_SEND_NUM; i++)
    {
        g_UartTXBuffer[i] = i;
    }

    fmsh_uart_init(id);
    FUartPs_enableLoopback(&g_UartDev);
    FUartPs_transmit(&g_UartDev, g_UartTXBuffer, TEST_UART_SEND_NUM,
                     fmsh_uart_tx_callback);

    timeout = UART_SEND_TIMEOUT;
    while (timeout-- > 0)
    {
        if (g_tx_flag)
        {
            g_tx_flag = FALSE;
            break;
        }
    }

    while (!g_rx_flag);
    g_rx_flag = FALSE;

    for (i = 0; i < TEST_UART_SEND_NUM; i++)
    {
        if (g_UartTXBuffer[i] != g_UartRXBuffer[i])
        {
            return FMSH_FAILURE;
        }
    }

    return FMSH_SUCCESS;
}
