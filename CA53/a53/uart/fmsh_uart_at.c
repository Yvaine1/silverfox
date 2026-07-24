#include <stdlib.h>
#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_psu_parameters.h"
#include "fmsh_uart_lib.h"
#include "fmsh_uart_at.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "sys.h"
/************************** Constant Definitions *****************************/
#define UARTPS_BAUNDRAT       2000000
#define TEST_UART_RECEIVE_NUM 100
#define TEST_UART_SEND_NUM    10
#define UART_SEND_TIMEOUT     1000000

FUartPs_T g_UartDev;

extern SemaphoreHandle_t xSemaphore;
u8 g_UartTXBuffer[256] __attribute__ ((aligned (32)));
u8 g_UartRXBuffer[256] __attribute__ ((aligned (32)));
BOOL g_tx_flag = FALSE;
BOOL g_rx_flag = FALSE;
BOOL g_uart_at_rx_print_enable = FALSE;
int32_t data_count = 0;
at_message_t cmd_msg;

void fmsh_uart_at_set_rx_print_enable(BOOL enable)
{
    g_uart_at_rx_print_enable = enable ? TRUE : FALSE;
}

BOOL fmsh_uart_at_get_rx_print_enable(void)
{
    return g_uart_at_rx_print_enable;
}

void fmsh_uart_at_tx_callback (void* dev, int32_t eCode)
{
  g_tx_flag = TRUE;
}

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
void fmsh_uart_at_rx_callback (void* dev, int32_t eCode) 
{
    handle_rx_data(eCode);
    g_rx_flag = TRUE; 

}


void handle_rx_data(int32_t eCode)
{
    /*memcpy(&cmd_msg.data[data_count], g_UartRXBuffer, eCode);
    data_count = data_count + eCode;
    // fmsh_print("data_count = %d \r\n", data_count);
    cmd_msg.length = data_count;
    if(g_UartRXBuffer[eCode-2] == '\r' || g_UartRXBuffer[eCode-1] == '\n')
    {
        cmd_msg.data[data_count] = '\0';
        data_count = 0;
        xSemaphoreGive(xSemaphore);
    }
    */
   //call app fun
   if (eCode > 0 && eCode <= sizeof(g_UartRXBuffer)
       && fmsh_uart_at_get_rx_print_enable() == TRUE)
   {
       int32_t i;
       fmsh_print("l=%d\r\n", eCode);
       for (i = 0; i < eCode; i++)
       {
           fmsh_print("0x%02x ", g_UartRXBuffer[i]);
           if((i + 1) % 16 == 0)
            fmsh_print("\r\n");
       }
       fmsh_print(" \r\n");
   }
   SYS_ARCH_DECL_PROTECT(lev);
   SYS_ARCH_PROTECT(lev);
   notify_app_uart_rx_data(g_UartRXBuffer, eCode);
   SYS_ARCH_UNPROTECT(lev);
}

int send_uart(void *buffer, u32 length)
{
    return FUartPs_transmit (&g_UartDev, buffer, length, fmsh_uart_at_tx_callback);
}


void fmsh_uart_at_listener (void* dev, int32_t eCode)
{
    FUartPs_Param_T *param;
    FUartPs_Portmap_T *portmap = NULL;
    FUartPs_Instance_T *instance;
    uint32_t reg;
    int numChars = 0U;
    BOOL ptime, fifos =FALSE;
    int callbackArg = 0;
    FMSH_callback userCallback;

    //fmsh_print("Enter %s\r\n", __FUNCTION__);
    // process interrupt/event
    param = &(g_UartDev.comp_param);
    portmap = (FUartPs_Portmap_T *)g_UartDev.base_address;
    instance = &(g_UartDev.instance);

    userCallback = NULL;
    callbackArg = 0;


// fmsh_print("eCode = %x\r\n", eCode);
    reg = UART_INP(portmap->iir_fcr);
    fifos = (FMSH_BIT_GET(reg, UART_IIR_FIFO_STATUS) == 0 ? FALSE : TRUE);
    switch (eCode)
    {
    case Uart_event_line:
        // This event occurs for overrun/parity/framing errors and a break
        // interrupt. reading the line status register clears this interrupt
        reg = FUartPs_getLineStatus(dev);
        // print what error(s) occured
        if ((reg & Uart_line_oe) != 0)
        {
            fmsh_print("overrun ");
        }
        if ((reg & Uart_line_pe) != 0)
        {
            fmsh_print("parity ");
        }
        if ((reg & Uart_line_fe) != 0)
        {
            fmsh_print("framing ");
        }
        if ((reg & Uart_line_bi) != 0)
        {
            fmsh_print("break ");
        }
        fmsh_print("\r\n");
        break;
    case Uart_event_timeout:
    case Uart_event_data:
        // These event only occurs when data is received and no Rx
        // buffer has been set with uart_receive().
        memset(g_UartRXBuffer,0, sizeof(g_UartRXBuffer));
         if(param->fifo_stat == TRUE)
        {
            // If the FIFO status registers are available,
            // we can simply query how many characters are
            // in the Rx FIFO.
            numChars = FUartPs_getRxFifoLevel(&g_UartDev);
           //fmsh_print("numChars = %d\r\n", numChars);
        }
        FUartPs_receive(dev, g_UartRXBuffer, numChars,
                        fmsh_uart_at_rx_callback);
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
                FUartPs_disableIrq(&g_UartDev, Uart_irq_etbei);
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
            ptime = FUartPs_isPtimeEnabled(&g_UartDev);
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
                               FUartPs_getTxFifoLevel(&g_UartDev);
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
                    switch (FUartPs_getTxTrigger(&g_UartDev))
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
                uart_x_fifo_write_max(&g_UartDev, numChars);
                //UART_X_FIFO_WRITE(numChars);

                if ((ptime == TRUE) && (fifos == TRUE))
                {
                    // Send more bytes if the Tx FIFO is still
                    // not full.  Stops when LSR THRE bit (FIFO
                    // full) becomes Set (PTIME enabled!).
                    //UART_FIFO_WRITE();
                    uart_x_fifo_write(&g_UartDev);
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
    default:
        // TRACE_OUT(DEBUG_OUT,"[listener1] unrecognized error/event code:
        // %d\n", eCode);
        //FMSH_ASSERT(FALSE);
        break;
    }
}

u8 fmsh_uart_at_init (u8 id)
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
    //FUartPs_setParity(&g_UartDev, Uart_odd_parity);
    FUartPs_setStick(&g_UartDev, Uart_Stick_disable);
    FUartPs_setStopBits(&g_UartDev, Uart_one_stop_bit);
    FUartPs_setDataBits(&g_UartDev, Uart_eight_bits);
    ret = FUartPs_setBaudRate(&g_UartDev, UARTPS_BAUNDRAT);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }
#if 1
    /*enable fifo*/
    ret = FUartPs_enableFifos(&g_UartDev);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }
    
    FUartPs_setRxTrigger(&g_UartDev, Uart_fifo_half_full);
    FUartPs_setTxTrigger(&g_UartDev,Uart_half_full_fifo);
    
#endif

    /*set callback functions*/
    FUartPs_setListener(&g_UartDev, fmsh_uart_at_listener);

    /*initialize irq handler*/
    ret = FGicPs_Connect(&IntcInstance, UART0_INT_ID + id,
                         (FMSH_InterruptHandler)FUartPs_userIrqHandler, &g_UartDev);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }
   
    FGicPs_InterruptMaptoCpu(&IntcInstance, 1,UART0_INT_ID + id);
    FGicPs_Enable(&IntcInstance, UART0_INT_ID + id);



    return FMSH_SUCCESS;
}