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

#include "FreeRTOS.h"
#include "semphr.h"

#include "task.h"
#include "shell.h"

#include "fmsh_uart_lib.h"
#include "platform.h"

Shell shell;
char shellBuffer[512];

static SemaphoreHandle_t shellMutex;

extern FUartPs_T g_UART;

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
      while (FUartPs_isRxFifoEmpty(&g_UART) == TRUE) { vTaskDelay(100); }
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
    xSemaphoreTakeRecursive(shellMutex, portMAX_DELAY);
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
    xSemaphoreGiveRecursive(shellMutex);
    return 0;
}

/**
 * @brief 用户shell初始化
 *
 */
void userShellInit (void)
{
    shellMutex = xSemaphoreCreateMutex();

    shell.write = userShellWrite;
    shell.read = userShellRead;

    // shell.lock = userShellLock;
    // shell.unlock = userShellUnlock;
    shellInit(&shell, shellBuffer, 512);
#ifdef CORTEX_R5
  #if 2 == SHELL_CMD_MASTER
    if (xTaskCreate(shellTask, "shell", configMINIMAL_STACK_SIZE * 2, &shell, TASK_PRIORITY_0, NULL) != pdPASS)
    {
        fmsh_print_err("shell task creat failed\r\n");
    }
  #endif
#else
    if (xTaskCreateAffinitySet(shellTask, "shell", configMINIMAL_STACK_SIZE * 2, &shell, TASK_PRIORITY_1, AFFINITY_CORE0, NULL)!= pdPASS)
    {
        fmsh_print_err("shell task creat failed\r\n");
    }
#endif
}

/**
 * @brief 用户shell NoOs 写 no-op
 *
 * R5 shell 永远是被动接收方,不在 UART 上回显输入或写 shell 自身的 \r\n
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 返回 len,不实际写
 */
static short userShellWriteNoOp (char *data, unsigned short len)
{
    return len;
}

/**
 * @brief 用户shell NoOs 读 no-op
 *
 * R5 shell 不从 UART 读输入(字符全部由 IPI task 写入)
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 返回 0,不实际读
 */
static short userShellReadNoOp (char *data, unsigned short len)
{
    return 0;
}

/**
 * @brief 用户shellNoOs初始化
 *
 */
void userShellInitNoOs (void)
{
    shell.write = userShellWriteNoOp;
    shell.read = userShellReadNoOp;
    shellInit(&shell, shellBuffer, 512);
}
