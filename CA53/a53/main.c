#include <stdlib.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "bspconfig.h"
#include "fmsh_common.h"
#include "platform.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include "FreeRTOSConfig.h"
#include "shell_port.h"
#include "fmsh_ipi_init.h"
#include "ring_buffer.h"
#include "fmsh_gmac_interface.h"
#include "nr_shm_oam.h"
#include "ant_ctrl.h"
#include "fmsh_fatfs_example.h"
#include "lwip/init.h"
#include "fmsh_gpio_public.h"


/*-----------------------------------------------------------*/
/*
 * The tasks as described in the comments at the top of this file.
 */

static void antCtrlTask(void *pvParameters);

extern void stack_main(void *para);
extern void LogAgentRecvFromStack(void *p_buf, uint32_t *msg_size, int16_t *rtc, uint16_t cell_id);
extern int shm_ipc_init_stack(int32_t cell_id);
extern void run_slot_isr(void);
extern void drop_to_el0_from_el1(void);
extern void drop_to_el1_from_el3(void);
extern void drop_to_el2_from_el3(void);

/* The rate at which data is sent to the queue, specified in milliseconds, and
converted to ticks using the portTICK_PERIOD_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS     (1000 / portTICK_PERIOD_MS)

/*-----------------------------------------------------------*/

static TaskHandle_t  xlog_task;
static TaskHandle_t  xwrtest_task;
/*-----------------------------------------------------------*/
void init_shared_memory ()
{
    Fmsh_DCacheEnable();
    Fmsh_ICacheEnable();

    //Fmsh_SetTlbAttributes(SHARED_MEM_BASE, NORM_NONCACHE | INNER_SHAREABLE);
    //Fmsh_DCacheFlush();
}

__attribute__((unused)) static void log_task (void *pvParameters)
{
    uint8_t  msg_buf[SIZE_SHORT_MESSAGE] = {0};
    uint8_t  msg_buf_out[SIZE_SHORT_MESSAGE] = {0};
    uint32_t msg_size = 0;
    uint16_t recv_ret = 0;
#if 1
#define     LOG_PRINT_ETH           1
#define     LOG_UDP_SEND_L1C_PORT   8889
#define     LOG_UDP_RECV_L1C_PORT   8888
#define     LOG_UDP_SEND_PS_PORT    9998
#define     LOG_UDP_RECV_PS_PORT    9999

    uint32_t log_size = 0;
    //uint8_t  *log_buf = (uint8_t *)malloc(SIZE_TINY_LOG_MESSAGE);
    struct udp_pcb *upcb_l1c;
    ip_addr_t serverIP;
    struct pbuf *p;
    
    IP4_ADDR(&serverIP, 10, 255, 0, 100);
    upcb_l1c = udp_new();
    if(upcb_l1c != NULL)
    {
        upcb_l1c->local_port = LOG_UDP_SEND_L1C_PORT;
    }

    /*struct udp_pcb *upcb_ps = udp_new();
    if(upcb_ps != NULL)
    {
        upcb_ps->local_port = LOG_UDP_SEND_PS_PORT;
    }*/
#endif
    for (;;)
    {
        for (uint8_t idx = 0; idx < IPC_LOG_QUEUE_NUMBER; idx++)
        {
        #if 1
            LogAgentRecvFromL1c(msg_buf, &msg_size, idx, &recv_ret, 0);
            if (IPC_SUCCESS == recv_ret)
            {
                #ifdef LOG_PRINT_ETH
                snprintf(msg_buf_out, SIZE_SHORT_MESSAGE, "%s recv %s\n", get_msg_id_name(idx), msg_buf);
                p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg_buf_out), PBUF_RAM);
                pbuf_take(p, msg_buf_out, strlen(msg_buf_out));
                udp_sendto(upcb_l1c, p, &serverIP, LOG_UDP_RECV_L1C_PORT);
                pbuf_free(p);
                #else
                fmsh_print("%s recv %s\r\n", get_msg_id_name(idx), msg_buf);
                #endif
            }
        #else
            LogAgentRecvTinyLogFromL1c(log_buf, &log_size, idx, &recv_ret, 0);
            if (IPC_SUCCESS == recv_ret)
            {
              p = pbuf_alloc(PBUF_TRANSPORT, log_size, PBUF_RAM);
              pbuf_take(p, log_buf, log_size);
              udp_sendto(upcb, p,&serverIP,LOG_UDP_RECV_PORT);       
              pbuf_free(p);
            }
        #endif
        }

        do
        {
            LogAgentRecvFromStack(msg_buf, &msg_size, &recv_ret, 0);
            if (IPC_SUCCESS == recv_ret)
            {
              #if 1
                fmsh_print("%s", msg_buf);
              #else
                p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg_buf), PBUF_RAM);
                pbuf_take(p, msg_buf, strlen(msg_buf));
                udp_sendto(upcb_ps, p, &serverIP, LOG_UDP_RECV_PS_PORT);
                pbuf_free(p);
              #endif
            }
        }while (IPC_SUCCESS == recv_ret);
        
        vTaskDelay(1);
    }
}

void at_queues_init(void) {
    int ret = fmsh_uart_at_init(0); 
}

static void antCtrlTask (void *pvParameters)
{
    TickType_t xNextWakeTime;

    xNextWakeTime = xTaskGetTickCount();

    // init ant_ctrl
    ant_ctrl_init_func();

    for (;;)
    {
        vTaskDelayUntil(&xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS);
        //ant_ctrl_send_all_check_msg();
    }
}

FGpioPs_T gpio0;
uint32_t int1_cnt = 0;
SemaphoreHandle_t int_sem = NULL;
void pl_int1_handle(void *data)
{
  #if 0 //test code
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(int_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE)
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  #else
    run_slot_isr();
  #endif
}

int PL_Int1_Init(void)
{
    int_sem = xSemaphoreCreateBinary();
    int status;
    status = FGicPs_Connect(&IntcInstance, PL1_INT_ID,
                            (FMSH_InterruptHandler)pl_int1_handle, 0);
    if(status != GIC_SUCCESS)
        return status;
    FGicPs_InterruptMaptoCpu(&IntcInstance, GICMAP_CPUID2, PL1_INT_ID);
    FGicPs_SetPriorityTriggerType(&IntcInstance, 
                                  PL1_INT_ID, 
                                  (configMAX_API_CALL_INTERRUPT_PRIORITY +0) << portPRIORITY_SHIFT,
                                  0x3);
    FGicPs_Enable(&IntcInstance, PL1_INT_ID);
    return status;
}

__attribute__((unused)) static void int_work (void *pvParameters)
{
    //PL_Int1_Init();
    while (1)
    {
      if( xSemaphoreTake( int_sem, portMAX_DELAY ) == pdTRUE )
      {
        int1_cnt ++;
      }
    }
}

__attribute__((unused)) static void int_print (void *pvParameters)
{
  while (1)
  {
     vTaskDelay(1000);
     fmsh_print("int1_cnt = %d\r\n", int1_cnt);
  }
}

void app_init(void)
{
    init_shared_memory();

    ipi_a53_2_r5_init();

    shm_ipc_init_oam(0, 0);

    shm_ipc_init_stack(0);

    //udp_igmp_init();

    at_queues_init();

    __iar_Initlocks();

    PL_Int1_Init();

    //chaojunt, todo: comment it temperary
    xTaskCreateAffinitySet(antCtrlTask, "ant_ctrl_test", 1024 * 1024, NULL, TASK_PRIORITY_1, AFFINITY_CORE0, NULL);

    xTaskCreateAffinitySet(log_task, "log_task", configMINIMAL_STACK_SIZE * 1024, NULL, TASK_PRIORITY_1, AFFINITY_CORE0, &xlog_task);

    xTaskCreateAffinitySet(stack_main, "ue_stack", 1024 * 1024, NULL, TASK_PRIORITY_1, AFFINITY_CORE0, NULL);

    //xTaskCreateAffinitySet(int_work, "int_work", 1024 * 1024, NULL, TASK_PRIORITY_3, AFFINITY_CORE2, NULL);
    //xTaskCreateAffinitySet(int_print, "int_print", 1024 * 1024, NULL, TASK_PRIORITY_2, AFFINITY_CORE2, NULL);
}

int EL3_main ()
{
    init_platform();

#if EL2_LIVE
    drop_to_el2_from_el3();
#endif

#if EL1_LIVE
    drop_to_el1_from_el3();
#endif
}

int EL2_main ()
{
    // do something at EL2

    return 0;
}

int EL1_main ()
{
    // do something at EL1

#if EL0_LIVE
    drop_to_el0_from_el1();
#endif

    return 0;
}

int EL0_main ()
{
    // do something at EL0

    return 0;
}

int main (void)
{
    EL3_main();

    return 0;
}
