/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_print.c
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "fmsh_psu_parameters.h"
#include "fmsh_uart_lib.h"

#ifdef STDOUT_IS_16550
#include "fuartplns550_l.h"
#define UART_BAUD 9600
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void SendData(FUartPs_T *dev, char *UartBuffer, u32 NumBytes);
void fmsh_print(const char *ptr, ...);

/************************** Variable Definitions *****************************/
FUartPs_T g_UART;

#define SLCR_UART_CRL_APB_BASEADDR (0xff5e0238)
#define SLCR_UART1_RESET           (1 << 2)
#define SLCR_UART0_RESET           (1 << 1)

void uart_slcrRstRelease (u32 id)
{
    u32 reg = 0;
    if (id == 0)
    {
        reg = FMSH_ReadReg(SLCR_UART_CRL_APB_BASEADDR, 0x0);
        reg &= (~SLCR_UART0_RESET);
        FMSH_WriteReg(SLCR_UART_CRL_APB_BASEADDR, 0x0, reg);
    }
    else if (id == 1)
    {
        reg = FMSH_ReadReg(SLCR_UART_CRL_APB_BASEADDR, 0x0);
        reg &= (~SLCR_UART1_RESET);
        FMSH_WriteReg(SLCR_UART_CRL_APB_BASEADDR, 0x0, reg);
    }
    return;
}

int init_uart ()
{
    int ret = FMSH_SUCCESS;

#ifdef STDOUT_IS_16550
    {
        FUartPlNs550_SetBaud(STDOUT_BASEADDR, FPAR_FUARTPLNS550_CLOCK_HZ,
                             UART_BAUD);
        FUartPlNs550_SetLineControlReg(STDOUT_BASEADDR, FUN_LCR_8_DATA_BITS);
    }
#endif /* Bootrom/BSP configures PS7/PSU UART to 115200 bps */

#ifdef STDOUT_BASEADDRESS
    {
        FUartPs_Config *Config = NULL;

        /*Initialize UARTs and set baud rate*/
        Config = FUartPs_LookupConfig(STDOUT_BASEADDRESS == FPS_UART0_BASEADDR
                                          ? FPAR_UARTPS_0_DEVICE_ID
                                          : FPAR_UARTPS_1_DEVICE_ID);
        if (Config == NULL)
        {
            return FMSH_FAILURE;
        }

        ret = FUartPs_init(&g_UART, Config);
        if (ret != FMSH_SUCCESS)
        {
            return ret;
        }

        FUartPs_setBaudRate(&g_UART, STDOUT_BASEADDRESS == FPS_UART0_BASEADDR
                                         ? FPAR_UARTPS_0_BAUDRATE
                                         : FPAR_UARTPS_1_BAUDRATE);
        /*line settings*/
        FUartPs_setLineControl(&g_UART, Uart_line_8n1);

        /*enable FIFOs*/
        FUartPs_enableFifos(&g_UART);
    }
#endif

    return ret;
}

void fmsh_print (const char *ptr, ...)
{
#ifdef STDOUT_BASEADDRESS
    {
        va_list ap;
        char string[256];

        va_start(ap, ptr);
        vsnprintf(string, 256, ptr, ap);
        SendData(&g_UART, string, strlen(string));
        va_end(ap);
    }
#else
    {
        PRINTF(ptr);
    }
#endif

    return;
}

static void SendData (FUartPs_T *dev, char *UartBuffer, u32 NumBytes)
{
    for (u16 i = 0; i < NumBytes; i++)
    {
        FUartPs_write(dev, *UartBuffer);
        while ((FUartPs_getLineStatus(dev) & Uart_line_thre) != Uart_line_thre);
        UartBuffer++;
    }
}

unsigned char uart_getc(void)
{
    uint8_t retval;
    //uart_printf("waiting input\n"); 
    //while((FUartPs_getLineStatus(&g_UART) & Uart_line_dr) != Uart_line_dr);
    retval = FUartPs_read(&g_UART);
    //uart_printf("read 0x%x\n", retval); 
    return retval;
}
