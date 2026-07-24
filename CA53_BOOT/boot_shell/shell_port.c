/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief
 * @version 0.1
 * @date 2019-02-22
 *
 * @copyright (c) 2019 Letter
 *
 */
#include <stdlib.h>

#include "shell.h"

#include "fmsh_uart_lib.h"

Shell shell;
char shellBuffer[512];

extern FUartPs_T g_UART;

void * pvPortMalloc(size_t size)
{
    void* point;
    
    point = malloc(size);
    
    return point;
}

void pvPortFree(void* point)
{   
    free(point);
}

/**
 * @brief 用户shell写
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 实际写入的数据长度
 */
short userShellWrite (char *data, unsigned short len)
{
    unsigned short i;

    for (i = 0; i < len; i++)
    {
        FUartPs_write(&g_UART, *data);
        while ((FUartPs_getLineStatus(&g_UART) & Uart_line_thre) !=
               Uart_line_thre);
        data++;
    }

    return i;
}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 实际读取到
 */


short userShellRead (char *data, unsigned short len)
{
    unsigned short i;
    u8 value;

    for (i = 0; i < len; i++)
    {
        //while (FUartPs_isRxFifoEmpty(&g_UART) == TRUE);
        if (FUartPs_isRxFifoEmpty(&g_UART) == TRUE)
          break;
        value = FUartPs_read(&g_UART);
        *data = value;
        data++;
    }
    return i;
}

/**
 * @brief 用户shell上锁
 *
 * @param shell shell
 *
 * @return int 0
 */
int userShellLock (Shell *shell)
{
    return 0;
}

/**
 * @brief 用户shell解锁
 *
 * @param shell shell
 *
 * @return int 0
 */
int userShellUnlock (Shell *shell)
{
    return 0;
}

/**
 * @brief 用户shell初始化
 *
 */
void userShellInit (void)
{
    shell.write = userShellWrite;
    shell.read = userShellRead;
    // shell.lock = userShellLock;
    // shell.unlock = userShellUnlock;
    shellInit(&shell, shellBuffer, 512);
    while(1){
        shellTask(&shell);
    }
}
