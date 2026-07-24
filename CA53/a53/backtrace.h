#ifndef _BACKTRACE_H_
#define _BACKTRACE_H_


#ifdef __cplusplus
extern "C"
{
#endif
#include "FreeRTOS.h"
#include "task.h"
void backtrace(void);
void backtrace_abort(void);
void backtrace_overflow(TaskHandle_t xTask);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /*_BACKTRACE_H_*/