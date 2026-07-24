
#include "fmsh_gmac_mutex.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "sys_arch.h"

int gmac_mutex_init(SemaphoreHandle_t *pxMutex)
{
    *pxMutex = xSemaphoreCreateMutex();

    if( *pxMutex == NULL )
    {
      return -1;
    }

    return 0;
}

BaseType_t gmac_mutex_Isr_lock(SemaphoreHandle_t *pxMutex)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  if( xSemaphoreTakeFromISR( *pxMutex, &xHigherPriorityTaskWoken ) ==  pdTRUE){
    if (xHigherPriorityTaskWoken == pdTRUE) {
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return pdTRUE;
  }
  else
  {
     return pdFALSE;
  }
}


BaseType_t gmac_mutex_Isr_unlock(SemaphoreHandle_t *pxMutex)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  if( xSemaphoreGiveFromISR( *pxMutex, &xHigherPriorityTaskWoken ) ==  pdTRUE){
    if (xHigherPriorityTaskWoken == pdTRUE) {
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return pdTRUE;
  }
  else
  {
     return pdFALSE;
  }
}