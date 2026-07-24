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

#include "shell_port.h"
#include "fmsh_ipi_init.h"
#include "fmsh_gic_hw.h"
#include "l1c_r50.h"
#include "nr_shm_oam.h"
#include "fmsh_gpio_public.h"
#include "dac4651_main.h"
#define FREERTOS_SHELL

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

// #define XREG_CP15_PERF_CYCLE_COUNTER		"p15, 0, %0,  c9, c13, 0"
// mfcp(XREG_CP15_PERF_CYCLE_COUNTER, val);
/*-----------------------------------------------------------*/
/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask(void *pvParameters);
static void prvQueueSendTask(void *pvParameters);

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

static TaskHandle_t  xshell_task;
static TaskHandle_t  r50_timecritical_msg;
static TaskHandle_t  r50_timecritical_main;

extern int shm_ipc_init(int32_t cell_id, int32_t mem_reset);
extern void userShellInitNoOs(void);
extern volatile u32 RecvFlagA53;

/*-----------------------------------------------------------*/

void init_shared_memory ()
{
    Fmsh_DCacheEnable();
    Fmsh_ICacheEnable();
}

int freertos_demo (void)
{
    /* Create the queue. */
//    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(unsigned long));
//    if (xQueue != NULL)
//    {
        /* Start the two tasks as described in the comments at the top of this
        file. */
//        xTaskCreate(prvQueueReceiveTask, "Rx", 1024, NULL,
//                    mainQUEUE_RECEIVE_TASK_PRIORITY, NULL);
//        xTaskCreate(prvQueueSendTask, "TX", 1024, NULL,
//                    mainQUEUE_SEND_TASK_PRIORITY, NULL);
  
        xTaskCreate(prvRecvMessage, "rpu0 recv/send message", 2048, NULL,
                    mainQUEUE_SEND_TASK_PRIORITY, NULL);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
//    }

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
int freertos_shell (void)
{
    //userShellInit();
    //xTaskCreate(ushell_task, "shell_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY, &xshell_task);
    
  
    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

FGpioPs_T gpio0;
void pl_int_test(void *data)
{
    // FGpioPs_writeBit(&gpio0, Gpio_low, Gpio_bit_16);
    l1c_nr_timecritical_main();
    // FGpioPs_writeBit(&gpio0, Gpio_high, Gpio_bit_16);
}
void pdsch_data_notifier_interrupt(void *data)
{
    //pdsch_data_notifier();
}
void freertos_r50(void)
{
    // xTaskCreate(l1c_nr_timecritical_msg_monitor, "r50_timecritical_msg", configMINIMAL_STACK_SIZE * 16, NULL, tskIDLE_PRIORITY, &r50_timecritical_msg);
    // xTaskCreate(l1c_nr_timecritical_main_interrupt, "r50_timecritical_main", configMINIMAL_STACK_SIZE * 16, NULL, tskIDLE_PRIORITY, &r50_timecritical_main);
    fmsh_print("r50 msg monitor task!\r\n");
    // FGpioPs_bank_init(FPAR_GPIOPS_0_DEVICE_ID, &gpio0);  /*gpio bank0, mio 0-25*/
    // FGpioPs_setBitDirection(&gpio0, Gpio_bit_16, Gpio_output);
    // FGpioPs_writeBit(&gpio0, Gpio_high, Gpio_bit_16);
    //* Slot interrupt
#if 1
    int status = FGicPs_Connect(&IntcInstance, PL2_INT_ID,
                            (FMSH_InterruptHandler)pl_int_test, 0);
    FGicPs_InterruptMaptoCpu(&IntcInstance, 0x1, PL2_INT_ID);
    FGicPs_SetPriorityTriggerType(&IntcInstance, PL2_INT_ID, 0x0, 0x3);
    FGicPs_Enable(&IntcInstance, PL2_INT_ID);
    fmsh_print("r50 slot interrupt cb!\r\n");
    /*
    status = FGicPs_Connect(&IntcInstance, PL3_INT_ID,
                            (FMSH_InterruptHandler)pdsch_data_notifier_interrupt, 0);
    FGicPs_InterruptMaptoCpu(&IntcInstance, 0x1, PL3_INT_ID);
    FGicPs_SetPriorityTriggerType(&IntcInstance, PL3_INT_ID, 0x8, 0x3);
    FGicPs_Enable(&IntcInstance, PL3_INT_ID);
    fmsh_print("r50 pdsch interrupt cb!\r\n");
    */
#endif
}


/*****************************************************************************/
extern void rfdc_main(void);

int main (void)
{
    init_platform();

    init_shared_memory();

    shm_ipc_init(0, 1);

    psu_ps_pl_isolation_removal_data();
    psu_ps_pl_reset_config_data();
    FGicPs_CommonInit(&IntcInstance);

    cx4e04_init();
    eeprom_i2c_init();
    dac4651_i2c_init();
    set_dac4651_by_eeprom();
    metal_rfdc_init();
    freertos_r50();
    userShellInitNoOs();
    ipi_a53_2_r50_init();
#ifdef FREERTOS_SHELL
    // freertos_shell();
#else
    ipi_demo();
    freertos_demo();
#endif
    char recv_buf[SHELL_WORK_BUFFER_SIZE] = {0};
    while (1)
    {
        if (RecvFlagA53)
        {
            RecvFlagA53 = 0;
            uint32_t msg_size = sizeof(recv_buf);
            int16_t ret = 0;
            L1cRecvFromOam(recv_buf, &msg_size, MSGQ_A53_R50_SHMA_SHELL_CMD_REQ, &ret, 0);
            if (IPC_SUCCESS == ret)
            {
                for (uint32_t i = 0; i < msg_size; i++)
                {
                    shellHandler(&shell, recv_buf[i]);
                }
            }
        }
    }

    return 0;
}
