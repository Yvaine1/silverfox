/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_uart_logout.c
 *
 * This file contains boot_main.h.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/28/2022  First Release.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/
#define UART_INIT_DONE 1
#define UART_UNINIT    0
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FUartPs_T g_UART;
static u32 s_uart_initflag = UART_UNINIT;
/************************** Function Prototypes ******************************/
/******************************************************************************
 *
 * This function is used to send data.
 *
 * @param    dev is a pointer to uart device
 * @param    UartBuffer is a pointer to send buffer.
 * @param    NumBytes is the number of data.
 *
 * @return	 None.
 *
 ******************************************************************************/
static void SendData (FUartPs_T *dev, char *UartBuffer, u32 NumBytes)
{
    u32 i, timeout = 0U;

    for (i = 0; i < NumBytes; i++)
    {
        FUartPs_write(dev, *UartBuffer);
        timeout = 0;
        while ((FUartPs_getLineStatus(dev) & Uart_line_thre) != Uart_line_thre)
        {
            timeout++;
            if (timeout > 0x80000)
            {
                break;
            }
        }
        UartBuffer++;
    }
}

/******************************************************************************
 *
 * This function is used to initialize g_UART.
 *
 * @param	 None.
 *
 * @return	 None.
 *
 ******************************************************************************/
u32 FmshFsbl_UartInit (void)
{
    u32 Status = FMSH_SUCCESS;
#ifdef STDOUT_BASEADDRESS
    /*Initialize UARTs and set baud rate*/
    FUartPs_Config *Config = NULL;

    /*Initialize UARTs and set baud rate*/
    Config = FUartPs_LookupConfig(STDOUT_BASEADDRESS == FPS_UART0_BASEADDR
                                      ? FPAR_UARTPS_0_DEVICE_ID
                                      : FPAR_UARTPS_1_DEVICE_ID);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }
    Status = FUartPs_init(&g_UART, Config);
    if (Status != FMSH_SUCCESS)
    {
        return Status;
    }

    FUartPs_setBaudRate(&g_UART, STDOUT_BASEADDRESS == FPS_UART0_BASEADDR
                                     ? FPAR_UARTPS_0_BAUDRATE
                                     : FPAR_UARTPS_1_BAUDRATE);
    FUartPs_setDataBits(&g_UART, Uart_eight_bits);
    FUartPs_setParity(&g_UART, Uart_no_parity);
    FUartPs_setStick(&g_UART, Uart_Stick_disable);
    FUartPs_setStopBits(&g_UART, Uart_one_stop_bit);

    FUartPs_read(&g_UART);
    FUartPs_enableFifos(&g_UART);
    FUartPs_setTxTrigger(&g_UART,Uart_half_full_fifo);
#endif
    s_uart_initflag = UART_INIT_DONE;
    return Status;
}

/******************************************************************************
 *
 * This function is used to print string.
 *
 * @param	 None.
 *
 * @return	 None.
 *
 ******************************************************************************/
u32 uart_printf (char *fmt, ...)
{
    u32 Status = FMSH_SUCCESS;
#ifdef STDOUT_BASEADDRESS
    if (s_uart_initflag == UART_INIT_DONE)
    {
        va_list ap;
        char string[256];
        va_start(ap, fmt);
        vsnprintf(string, 256, fmt, ap);
        SendData(&g_UART, string, strlen(string));
        va_end(ap);
    }
#endif
    return Status;
}

void fmsh_print (const char *ptr, ...)
{

}
/******************************************************************************
 *
 * This function is used to print array string.
 *
 * @param	 None.
 *
 * @return	 None.
 *
 ******************************************************************************/
void uart_printArray (const unsigned char Buf[], unsigned int Len)
{
#ifdef STDOUT_BASEADDRESS
    u32 Index = 0U;
    if (s_uart_initflag == UART_INIT_DONE)
    {
        for (Index = 0U; Index < Len; Index++)
        {
            UART_LOG_OUT(DEBUG_DETAILED, "%02lx ", Buf[Index]);
            if (((Index + 1) % 32 == 0) && (Index != 0))
            {
                UART_LOG_OUT(DEBUG_DETAILED, "\r\n");
            }
        }
        UART_LOG_OUT(DEBUG_DETAILED, "\r\n");
    }
#endif
}

/******************************************************************************
 *
 * This function is used to get error info.
 *
 * @param	 None.
 *
 * @return	 None.
 *
 ******************************************************************************/
char *FmshFsbl_GetErrorInfo (uint32_t key)
{
    char *err_msg = "";
    switch (key)
    {
    case FSBL_ERROR_UNAVAILABLE_CPU:
        err_msg = "Invalid cpu ProcessorID!\r\n";
        break;
    case FSBL_ERROR_PARTITION_SIGNATURE:
        err_msg = "Partition signature verify failed!\r\n";
        break;
    case FSBL_ERROR_INVALID_BOOT_MODE:
        err_msg = "Invalid boot mode!\r\n";
        break;
    case FSBL_ERROR_INVALID_ID:
        err_msg = "Invalid cpu ID!\r\n";
        break;
    case FSBL_ERROR_IMG_HEADER_CHECKSUM:
        err_msg = "Img header checksum error!\r\n";
        break;
    case FSBL_ERROR_PPK_HASH_MISMATCH:
        err_msg = "PPK hash mismatch!\r\n";
        break;
    case FSBL_ERROR_SPK_ID_MISMATCH:
        err_msg = "SPK ID mismatch!\r\n";
        break;
    case FSBL_ERROR_SPK_SIGNATURE:
        err_msg = "SPK signature verify failed!\r\n";
        break;
    case FSBL_ERROR_BOOT_HEADER_SIGNATURE:
        err_msg = "Boot header signature verify failed!\r\n";
        break;
    case FSBL_ERROR_PH_CHECKSUM:
        err_msg = "Partition Header checksum verify failed!\r\n";
        break;
    case FSBL_ERROR_XIP_AUTH_ENC_PRESENT:
        err_msg = "Encrypted Img runs in XIP mode!\r\n";
        break;
    case FSBL_ERROR_APU_XIP_EXCUTION_ADDRESS:
        err_msg = "XIP excution address is error!\r\n";
        break;
    case FSBL_ERROR_MISMATCH_PARTITION_LENGTH:
        err_msg = "Partition length mismatch!\r\n";
        break;
    case FSBL_ERROR_PARTITION_AUTHENTICATE:
        err_msg = "Partition authenticate failed!\r\n";
        break;
    case FSBL_ERROR_INVALID_EXCUTION_ADDRESS:
        err_msg = "Excution address is error!\r\n";
        break;
    case FSBL_ERROR_DECYPTION:
        err_msg = "Decyption failed!\r\n";
        break;
    case FSBL_ERROR_PARTITION_CHECKSUM:
        err_msg = "Partition checksum is error!\r\n";
        break;
    case FSBL_ERROR_SECURE_BOOT_FORCE:
        err_msg = "Boot force mismatch!\r\n";
        break;
    case FSBL_ERROR_PL_POWER:
        err_msg = "PL is power down!\r\n";
        break;
    case FSBL_ERROR_PL_CONFIG:
        err_msg = "Config PL is failed!\r\n";
        break;
    case FSBL_ERROR_DEVC_INIT:
        err_msg = "Devc init is failed!\r\n";
        break;
    default:
        err_msg = "Undefined error status";
        break;
    }
    return err_msg;
}
