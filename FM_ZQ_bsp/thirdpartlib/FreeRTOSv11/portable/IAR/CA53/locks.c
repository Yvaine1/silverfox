#include <Dlib_Threads.h>
#include "FreeRTOS.h"
#include "semphr.h"

void __iar_system_Mtxinit(__iar_Rmtx *m)
{
  if (m != NULL) {
    *m = xSemaphoreCreateRecursiveMutex();
    configASSERT(*m != NULL);
  }
}

void __iar_system_Mtxdst(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    vSemaphoreDelete(*m);
    *m = NULL;
  }
}

void __iar_system_Mtxlock(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    xSemaphoreTakeRecursive(*m, portMAX_DELAY);
  }
}

void __iar_system_Mtxunlock(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    xSemaphoreGiveRecursive(*m);
  }
}

void __iar_file_Mtxinit(__iar_Rmtx *m)
{
  if (m != NULL) {
    *m = xSemaphoreCreateRecursiveMutex();
    configASSERT(*m != NULL);
  }
}

void __iar_file_Mtxdst(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    vSemaphoreDelete(*m);
    *m = NULL;
  }
}

void __iar_file_Mtxlock(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    xSemaphoreTakeRecursive(*m, portMAX_DELAY);
  }
}

void __iar_file_Mtxunlock(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    xSemaphoreGiveRecursive(*m);
  }
}

void __iar_Initdynamiclock(__iar_Rmtx *m)
{
  if (m != NULL) {
    *m = xSemaphoreCreateRecursiveMutex();
    configASSERT(*m != NULL);
  }
}

void __iar_Dstdynamiclock(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    vSemaphoreDelete(*m);
    *m = NULL;
  }
}

void __iar_Lockdynamiclock(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    xSemaphoreTakeRecursive(*m, portMAX_DELAY);
  }
}

void __iar_Unlockdynamiclock(__iar_Rmtx *m)
{
  if (m != NULL && *m != NULL) {
    xSemaphoreGiveRecursive(*m);
  }
}

static __attribute__((aligned(8))) char tls_area[4][256] = {"core0", "core1", "core2", "core3"};

void *__aeabi_read_tp(void)
{
  uint32_t core_id = portGET_CORE_ID();
  return &tls_area[core_id][0];
}