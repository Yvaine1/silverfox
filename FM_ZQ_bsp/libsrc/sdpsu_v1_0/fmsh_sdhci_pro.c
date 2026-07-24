#if !NO_OS

#include "FreeRTOS.h"
#include "semphr.h"
#include  "fmsh_sdhci_pro.h"

static SemaphoreHandle_t xEmmcMutex = NULL;



int emmc_mutex_init(void)
{
    xEmmcMutex = xSemaphoreCreateMutex();
    if (xEmmcMutex == NULL) 
    {
        fmsh_print("Failed to create eMMC mutex\r\n");
        return FMSH_FAILURE;
    }
    return FMSH_SUCCESS;
}


int emmc_mutex_delete(void)
{
    if (xEmmcMutex == NULL)
    {
        return FMSH_FAILURE;
    }
    vSemaphoreDelete(xEmmcMutex);

    return FMSH_SUCCESS;
}


int emmc_lock(uint32_t timeout_ms)
{
    if (xEmmcMutex == NULL)
    {
        return FMSH_FAILURE;
    }
    
    if (xSemaphoreTake(xEmmcMutex, pdMS_TO_TICKS(timeout_ms)))
    {
        // fmsh_print("%s, lock eMMC\r\n", __func__);
        return FMSH_SUCCESS;
    }
    
    fmsh_print("Failed to acquire eMMC lock, timeout\r\n");
    return FMSH_FAILURE;
}


void emmc_unlock(void)
{
    if (xEmmcMutex != NULL) 
    {
        xSemaphoreGive(xEmmcMutex);
        // fmsh_print("%s, unlock eMMC\r\n", __func__);
    }
}

#endif /* !NO_OS */