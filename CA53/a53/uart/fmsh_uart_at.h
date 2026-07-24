#ifndef _FMSH_UART_AT_H_
#define _FMSH_UART_AT_H_

#include "fmsh_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct{
  char data[256];
  size_t length;
} at_message_t;


void fmsh_uart_at_tx_callback (void* dev, int32_t eCode);
void fmsh_uart_at_rx_callback (void* dev, int32_t eCode);
void fmsh_uart_at_set_rx_print_enable(BOOL enable);
BOOL fmsh_uart_at_get_rx_print_enable(void);
u8 fmsh_uart_at_init (u8 id);
void handle_rx_data(int32_t eCode);
int send_uart(void *buffer, u32 length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif