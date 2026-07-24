/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_uart_logout.h
 *
 * This file contains header fmsh_uart_common.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   xx  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_UART_LOGOUT_H_
#define _FMSH_UART_LOGOUT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_uart_common.h"
#include "fmsh_uart_lib.h"
#include "fmsh_uart_public.h"
#include "string.h"

/************************** Constant Definitions *****************************/
  

#ifdef __FSBL__
  #define UART_LOG_OUT(flag, ...)       \
      do                                \
      {                                 \
          if (flag)                     \
          {                             \
           (void)uart_printf(__VA_ARGS__); \
          }                             \
      } while (0)
#else
  #define UART_LOG_OUT(flag, ...)       \
      do                                \
      {                                 \
          if (flag)                     \
          {                             \
           (void)fmsh_print(__VA_ARGS__); \
          }                             \
      } while (0)
#endif
        
#define PRINT_ARRAY(flag, ...)            \
    do                                    \
    {                                     \
        if (flag)                         \
        {                                 \
            uart_printArray(__VA_ARGS__); \
        }                                 \
    } while (0)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FUartPs_T g_UART;
/************************** Function Prototypes ******************************/
u32 FmshFsbl_UartInit(void);
u32 uart_printf(char *fmt, ...);
void uart_printArray(const unsigned char Buf[], unsigned int Len);
char *FmshFsbl_GetErrorInfo(uint32_t key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
