/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://aws.amazon.com/freertos
 *
 */
#include <stdlib.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "fmsh_common.h"
#include "platform.h"
#include "psu_init.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "fmsh_gic_hw.h"
#include "shell_port.h"
#include "l1c_r51.h"
#include "dg_tod_utc.h"
#include "fmsh_ipi_init.h"
#include "semphr.h"
#include "nr_shm_oam.h"

// #define FREERTOS_SHELL

/* Priorities used by the various different standard demo tasks. */
#define mainCHECK_TASK_PRIORITY         (configMAX_PRIORITIES - 1)
#define mainQUEUE_POLL_PRIORITY         (tskIDLE_PRIORITY + 1)
#define mainSEM_TEST_PRIORITY           (tskIDLE_PRIORITY + 1)
#define mainBLOCK_Q_PRIORITY            (tskIDLE_PRIORITY + 2)
#define mainCREATOR_TASK_PRIORITY       (tskIDLE_PRIORITY + 3)
#define mainFLASH_TASK_PRIORITY         (tskIDLE_PRIORITY + 1)
#define mainINTEGER_TASK_PRIORITY       (tskIDLE_PRIORITY)
#define mainGEN_QUEUE_TASK_PRIORITY     (tskIDLE_PRIORITY)
#define mainCOM_TEST_PRIORITY           (tskIDLE_PRIORITY + 2)

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY    (tskIDLE_PRIORITY + 1)

/* The rate at which data is sent to the queue, specified in milliseconds, and
converted to ticks using the portTICK_PERIOD_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS     (1000 / portTICK_PERIOD_MS)

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH                (1)

/* A zero block time. */
#define mainDONT_BLOCK                  (0UL)
/*-----------------------------------------------------------*/
/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask(void *pvParameters);
static void prvQueueSendTask(void *pvParameters);

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;
static TaskHandle_t  r51_hls_simulate;
static TaskHandle_t  r51_high_layer;
static TaskHandle_t  r51_ul_dl;
static TaskHandle_t  shell_ipi;

extern int shm_ipc_init(int32_t cell_id, int32_t mem_reset);

SemaphoreHandle_t xShellIpiSem = NULL;
void init_shared_memory ()
{
    Fmsh_DCacheEnable();
    Fmsh_ICacheEnable();
}

/*-----------------------------------------------------------*/

int freertos_demo (void)
{
    /* Create the queue. */
    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(unsigned long));
    if (xQueue != NULL)
    {
        /* Start the two tasks as described in the comments at the top of this
        file. */
        xTaskCreate(prvQueueReceiveTask, "Rx", 1024, NULL,
                    mainQUEUE_RECEIVE_TASK_PRIORITY, NULL);
        xTaskCreate(prvQueueSendTask, "TX", 1024, NULL,
                    mainQUEUE_SEND_TASK_PRIORITY, NULL);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

static void prvQueueSendTask (void *pvParameters)
{
    TickType_t xNextWakeTime;
    unsigned long ulValueToSend = 100UL;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for (;;)
    {
        /* Place this task in the blocked state until it is time to run again.
        The block time is specified in ticks, the constant used converts ticks
        to ms.  While in the Blocked state this task will not consume any CPU
        time. */
        vTaskDelayUntil(&xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS);

        /* Send to the queue - causing the queue receive task to unblock and
        toggle an LED.  0 is used as the block time so the sending operation
        will not block - it shouldn't need to block as the queue should always
        be empty at this point in the code. */
        ulValueToSend = rand() % 100;
        // fmsh_print("Send Task: value is %d\r\n", ulValueToSend);
        xQueueSend(xQueue, &ulValueToSend, 0);
    }
}

static void prvQueueReceiveTask (void *pvParameters)
{
    unsigned long ulReceivedValue;
    unsigned long times = 0;
    for (;;)
    {
        ulReceivedValue = 0;
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h. */
        xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

        /*  To get here something must have been received from the queue, but
        is it the expected value?  If it is, toggle the LED. */
        // fmsh_print("Receive Task[%d times]: value is %d\r\n", times++,
        //            ulReceivedValue);
    }
}

/*****************************************************************************/
static void prvShellFromIpiTask(void *pvParameters)
{
    char recv_buf[SHELL_WORK_BUFFER_SIZE] = {0};
    uint32_t msg_size = sizeof(recv_buf);
    int16_t ret = 0;
    
    ipi_a53_2_r51_init();
    for (;;)
    {
        /* Block until ISR gives the semaphore */
        if (xSemaphoreTake(xShellIpiSem, portMAX_DELAY) == pdTRUE)
        {
            L1cRecvFromOam(recv_buf, &msg_size, MSGQ_A53_R51_SHMA_SHELL_CMD_REQ, &ret, 0);
            if (IPC_SUCCESS == ret)
            {
                for (uint32_t i = 0; i < msg_size; i++)
                {
                    shellHandler(&shell, recv_buf[i]);
                }
            }
        }
    }
}

int freertos_shell (void)
{
    userShellInitNoOs();
    xShellIpiSem = xSemaphoreCreateBinary();
    configASSERT(xShellIpiSem);
    xTaskCreate(prvShellFromIpiTask, "shell_ipi", configMINIMAL_STACK_SIZE * 2, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, &shell_ipi);

    vTaskStartScheduler();

    for (;;);
    return 0;
}

#ifdef R51_UTC_TEST
static void r51_get_utc_time_handle(void *pvParameters)
{
    while (1)
    {
        r51_get_utc_time();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#endif

void freertos_r51(void)
{
#if 0
    xTaskCreate(l1c_nr_leo_r50_msg_monitor, "r51_ul_dl", configMINIMAL_STACK_SIZE *4, NULL, tskIDLE_PRIORITY, &r51_ul_dl);
    xTaskCreate(l1c_nr_leo_high_layer_msg_monitor, "r51_high_layer", configMINIMAL_STACK_SIZE * 16, NULL, tskIDLE_PRIORITY, &r51_high_layer);
    //xTaskCreate(hls_simulate_function, "r51_hls_simulate", configMINIMAL_STACK_SIZE * 16, NULL, tskIDLE_PRIORITY, &r51_hls_simulate);
#else
    xTaskCreate(l1c_nr_leo_r50_msg_monitor, "r51_ul_dl", configMINIMAL_STACK_SIZE *20, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, &r51_ul_dl);
#ifdef R51_UTC_TEST
    xTaskCreate(r51_get_utc_time_handle, "r51 get utc time", configMINIMAL_STACK_SIZE *20, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL);
#endif    
#endif
}

int test_cmd (int a, int b)
{
    fmsh_print("test cmd echo a = %d, b = %d \r\n", a, b);

    return 0;
}

u32 FGicPs_CommonInit (FGicPs *InstancePtr)
{
    u32 Status;

    Status = FGicPs_SetupInterruptSystem(InstancePtr);
    if (Status != GIC_SUCCESS)
    {
        return GIC_FAILURE;
    }

    FMSH_ExceptionRegisterHandler(
        FMSH_EXCEPTION_ID_IRQ_INT,
        (FMSH_ExceptionHandler)FGicPs_InterruptHandler_IRQ, InstancePtr);

    return Status;
}
/*****************************************************************************/
int main (void)
{
    init_platform();
    init_shared_memory();
    shm_ipc_init(0, 0);
    freertos_r51();
    freertos_shell();
    /*
#ifdef FREERTOS_SHELL
    freertos_shell();
#else
    freertos_demo();
#endif
    */
    return 0;
}
