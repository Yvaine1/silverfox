
#ifndef FMSH_GMAC_MUTEX_H
#define FMSH_GMAC_MUTEX_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "sys_arch.h"


int gmac_mutex_init(SemaphoreHandle_t *pxMutex);
BaseType_t gmac_mutex_Isr_lock(SemaphoreHandle_t *pxMutex);
BaseType_t gmac_mutex_Isr_unlock(SemaphoreHandle_t *pxMutex);







#endif
