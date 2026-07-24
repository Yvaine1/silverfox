/*
 * Uncomment one of the following two lines, depending on the target,
 * if ps7/psu init source files are added in the source directory for
 * compiling example outside of IAR.
 */
#include "fmsh_common.h"
#include "fmsh_print.h"
#include "fmsh_psu_parameters.h"
#include "psu_init.h"
#include "gmac_init.h"
#include "platform.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "ff.h"
#include "shell_port.h"
#include "fmsh_gmac_interface.h"
#include "fmsh_gic.h"
#include "fmsh_rtc_mix.h"
#include "eeprom_api.h"
#include "uartns550.h"

SemaphoreHandle_t phy_mutex = NULL;
SemaphoreHandle_t printf_mutex = NULL;

int start_init(void)
{
    int ret = FMSH_SUCCESS;
    FATFS fs1; 

    ret = rtc_init();
    if (ret == 0) 
    fmsh_print("RTC init success\r\n");

    eeprom_i2c_init();

    if (emmc_mutex_init() != FMSH_SUCCESS) 
    {
        fmsh_print("Fail to get emmc lock\r\n");
        return FMSH_FAILURE;
    }

    ret = f_mount(&fs1, "0:", 1);
    if (ret != FR_OK)
    {
        fmsh_print("Fail to mount, errNum: %d\r\n", ret);
        return FMSH_FAILURE;
    }

    phy_mutex = xSemaphoreCreateMutex();
    configASSERT(phy_mutex);
    
    printf_mutex = xSemaphoreCreateMutex();
    configASSERT(printf_mutex);

    userShellInit();

    ret = tod_uart_init(UARTNS550_DEVICE_0_ID);
    if (ret != FMSH_SUCCESS) 
    {
        fmsh_print("Fail to init UartNs550\r\n");
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print("UartNs550 init success\r\n");
    }

    ret = pps_intr_init(PPS_INT);
    if (ret != FMSH_SUCCESS) 
    {
        fmsh_print("Fail to init pps intr\r\n");
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print("pps intr init success\r\n");
    }

    xTaskCreateAffinitySet(prv_init_task, "init", 2048, NULL,
                                TASK_PRIORITY_1, AFFINITY_CORE0, NULL);
#ifdef UARTNS550_EN
    xTaskCreateAffinitySet(uartns550_intr_handle, "uartns550 intr handle", 2048, NULL,
                                TASK_PRIORITY_1, AFFINITY_CORE0, NULL);
#endif

#ifdef PRINTF_TOD_TEST
    xTaskCreateAffinitySet(printf_tod_handler, "printf tod handler", 2048, NULL,
                                TASK_PRIORITY_1, AFFINITY_CORE0, NULL);
#endif

#ifdef API_TEST
    xTaskCreateAffinitySet(api_test_handler, "api test handler", 2048, NULL,
                                TASK_PRIORITY_1, AFFINITY_CORE0, NULL);
#endif

#if GMAC0_TEST_EXAMPLE 
    xTaskCreateAffinitySet(gmac_0_prv_test, "gmac 0 test", 2048, NULL,
                                TASK_PRIORITY_1, AFFINITY_CORE3, NULL);
#endif

#if GMAC2_TEST_EXAMPLE
    xTaskCreateAffinitySet(gmac_2_prv_test, "gmac 2 test", 2048, NULL,
                                    TASK_PRIORITY_1, AFFINITY_CORE3, NULL);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

void init_platform ()
{
    int ret = PSU_INIT_SUCCESS;

#if PS_PREINITED == 0
    ret = psu_init();
#endif
    init_uart();

    if (ret != PSU_INIT_SUCCESS)
    {
        fmsh_print("PS_INIT_FAIL!\n\r");
        while (1)
        {
        };
    }
    ret = FGicPs_CommonInit(&IntcInstance);
    if (ret != GIC_SUCCESS)
    {
        fmsh_print("GIC Setup Failed!\r\n");
    }
    else
    {
        fmsh_print("GIC Setup pass!\r\n");
    }
#if (USE_DDR == 1) && (PS_PREINITED == 0)
#if (DDR_SIZE >= 0x80000000)
    Fmsh_SetTlbAttributesRange(0x0, 0x7f000000, ATTR_MEM);
    Fmsh_SetTlbAttributesRange(0x7f000000, 0x1000000, ATTR_MEM_NC);
#else  /*DDR_SIZE*/
    Fmsh_SetTlbAttributesRange(0x0, DDR_SIZE - 0x1000000, ATTR_MEM);
    Fmsh_SetTlbAttributesRange(DDR_SIZE - 0x1000000, 0x1000000, ATTR_MEM_NC);
#endif /*DDR_SIZE*/
#endif /*(USE_DDR == 1) && (PS_PREINITED == 0)*/

    Fmsh_ICacheEnable();

#if (DCACHE_ENABLE == 1)
    Fmsh_DCacheEnable();
#endif

    start_init();
}

void cleanup_platform () {}
