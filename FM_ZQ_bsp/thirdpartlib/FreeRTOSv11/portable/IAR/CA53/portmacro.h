/*
 * FreeRTOS Kernel V10.5.1
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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
 * https://github.com/FreeRTOS
 *
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

/* IAR includes. */
#ifdef __ICCARM__

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include <intrinsics.h>

#include "fmsh_gic.h"
#include "FreeRTOSConfig.h"
/************************** Constant Definitions *****************************/

/*-----------------------------------------------------------
* Port specific definitions.

* The settings in this file configure FreeRTOS correctly for the given hardware
* and compiler.
*
* These settings should not be altered.
*-----------------------------------------------------------
*/

/* Type definitions. */
#define portCHAR              char
#define portFLOAT             float
#define portDOUBLE            double
#define portLONG              long
#define portSHORT             short
#define portSTACK_TYPE        size_t
#define portBASE_TYPE         long
#define portPOINTER_SIZE_TYPE uint64_t

typedef portSTACK_TYPE StackType_t;
typedef portBASE_TYPE BaseType_t;
typedef uint64_t UBaseType_t;

#if (configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_16_BITS)
typedef uint16_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffff
#elif (configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_32_BITS)
typedef uint32_t TickType_t;
#define portMAX_DELAY           (TickType_t)0xffffffffUL
/* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
    not need to be guarded with a critical section. */
#define portTICK_TYPE_IS_ATOMIC 1
#elif (configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_64_BITS)
typedef uint64_t TickType_t;
#define portMAX_DELAY           (TickType_t)0xffffffffffffffffULL
/* 64-bit tick type on a 64-bit architecture, so reads of the tick count do
    not need to be guarded with a critical section. */
#define portTICK_TYPE_IS_ATOMIC 1
#else
#error configTICK_TYPE_WIDTH_IN_BITS set to unsupported tick type width.
#endif

/*-----------------------------------------------------------*/
/* Hardware specifics. */
#define portSTACK_GROWTH   (-1)
#define portBYTE_ALIGNMENT (16)
#define portTICK_PERIOD_MS ((TickType_t)1000 / configTICK_RATE_HZ)

/*-----------------------------------------------------------*/
/* Task utilities. */
#if defined(GUEST)
#define portYIELD() __asm volatile("SVC 0" ::: "memory")
#else
#define portYIELD() __asm volatile("SMC 0" ::: "memory")
#endif

void portYIELD_FROM_ISR(UBaseType_t xSwitchRequired);

/* The port can maintain the critical nesting count in TCB or maintain the
 * critical nesting count in the port. */
#define portCRITICAL_NESTING_IN_TCB 1

#define portENTER_CRITICAL          vTaskEnterCritical
#define portEXIT_CRITICAL           vTaskExitCritical

#if (configNUMBER_OF_CORES > 1)

/* vTaskEnterCriticalFromISR and vTaskExitCriticalFromISR should be used in the
 * implementation of portENTER/EXIT_CRITICAL_FROM_ISR if the number of cores is
 * more than 1 in the system. */
#define portENTER_CRITICAL_FROM_ISR vTaskEnterCriticalFromISR
#define portEXIT_CRITICAL_FROM_ISR  vTaskExitCriticalFromISR

/* Return the core ID on which the code is running. */
#define portGET_CORE_ID()           uxPortGetCoreId()
UBaseType_t uxPortGetCoreId();

/* Request the core ID x to yield. */
#define portYIELD_CORE(x) vPortYieldCore(x)
void vPortYieldCore(BaseType_t xOtherCoreID);

#define ISR_LOCK                (0u)
#define TASK_LOCK               (1u)
#define portRTOS_LOCK_COUNT     (2u)

/* Acquire the TASK lock. TASK lock is a recursive lock.
 * It should be able to be locked by the same core multiple times. */
#define portGET_TASK_LOCK()     vPortRecursiveLock(TASK_LOCK, pdTRUE)

/* Release the TASK lock. If a TASK lock is locked by the same core multiple
 * times, it should be released as many times as it is locked. */
#define portRELEASE_TASK_LOCK() vPortRecursiveLock(TASK_LOCK, pdFALSE)

/* Acquire the ISR lock. ISR lock is a recursive lock.
 * It should be able to be locked by the same core multiple times. */
#define portGET_ISR_LOCK()      vPortRecursiveLock(ISR_LOCK, pdTRUE)

/* Release the ISR lock. If a ISR lock is locked by the same core multiple
 * times, \ it should be released as many times as it is locked. */
#define portRELEASE_ISR_LOCK()  vPortRecursiveLock(ISR_LOCK, pdFALSE)
void vPortRecursiveLock(uint32_t ulLockNum, BaseType_t uxAcquire);

/* Interrupt number to interrupt a core for task yield */
#define YIELD_CORE_INTERRUPT_NO (1U)

#endif /* if ( configNUMBER_OF_CORES == 1 ) */

#define portUNMASK_VALUE (0xFFUL)
#define portICCPMR_PRIORITY_MASK \
    ((uint32_t)(configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT))

/*-----------------------------------------------------------
 * Critical section control
 *----------------------------------------------------------*/
#define portCPU_DISABLE_INTERRUPTS()                    \
    {                                                   \
        __asm volatile("MSR DAIFSET, #2" ::: "memory"); \
        __asm volatile("DSB SY");                       \
        __asm volatile("ISB SY");                       \
    }

#define portCPU_ENABLE_INTERRUPTS()                     \
    {                                                   \
        __asm volatile("MSR DAIFCLR, #2" ::: "memory"); \
        __asm volatile("DSB SY");                       \
        __asm volatile("ISB SY");                       \
    }

#define portCLEAR_ENABLE_INTERRUPT_MASK()                     \
    {                                                         \
        portCPU_DISABLE_INTERRUPTS();                         \
        portICCPMR_PRIORITY_MASK_REGISTER = portUNMASK_VALUE; \
        __asm volatile(                                       \
            "DSB SY     \n"                                   \
            "ISB SY     \n");                                 \
        portCPU_ENABLE_INTERRUPTS();                          \
    }

#define portDISABLE_INTERRUPTS()               portCPU_DISABLE_INTERRUPTS()
#define portENABLE_INTERRUPTS()                portCLEAR_ENABLE_INTERRUPT_MASK()
#define portSET_INTERRUPT_MASK()               uxPortSetInterruptMask()
#define portCLEAR_INTERRUPT_MASK(pmr)          vPortClearInterruptMask(pmr)
#define portSET_INTERRUPT_MASK_FROM_ISR()      uxPortSetInterruptMask()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(pmr) vPortClearInterruptMask(pmr)

UBaseType_t uxPortSetInterruptMask(void);
void vPortClearInterruptMask(UBaseType_t ulPreviousMask);

/*-----------------------------------------------------------*/
/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not required for this port but included in case common demo code that uses these
macros is used. */
#define portTASK_FUNCTION_PROTO(vFunction, pvParameters) \
    void vFunction(void *pvParameters)
#define portTASK_FUNCTION(vFunction, pvParameters) \
    void vFunction(void *pvParameters)

/* Any task that uses the floating point unit MUST call vPortTaskUsesFPU()
before any floating point instructions are executed. */
#define portTASK_USES_FLOATING_POINT() vPortTaskUsesFPU()
void vPortTaskUsesFPU(void);

/* Prototype of the FreeRTOS tick handler.  This must be installed as the
handler for whichever peripheral is used to generate the RTOS tick. */
#define configTIMER_ID               FPAR_TTCPS_0_DEVICE_ID
#define configTIMER_NO               (1)
#define configTIMER_INTERRUPT_ID     (68)
#define configSETUP_TICK_INTERRUPT() vPortSetupTickInterrupt()
#define configCLEAR_TICK_INTERRUPT() vPortClearTickInterrupt()

void vPortEnableInterrupt(uint8_t ucInterruptID);
void vPortDisableInterrupt(uint8_t ucInterruptID);
void vPortClearTickInterrupt(void);
void vPortSetupTickInterrupt(void);
void vPortTickHandler(void);
void vPortIPIHandler(void);

/* Interrupt controller access addresses. */
#define configINTERRUPT_CONTROLLER_DEVICE_ID            (0)
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS         (0xf9000000)
#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET (0x20000)
#define configINTERRUPT_CONTROLLER_DIST_OFFSET          (0x10000)
#define configUNIQUE_INTERRUPT_PRIORITIES               (32)

#define portICCPMR_PRIORITY_MASK_OFFSET                 (0x04)
#define portICCIAR_INTERRUPT_ACKNOWLEDGE_OFFSET         (0x0C)
#define portICCEOIR_END_OF_INTERRUPT_OFFSET             (0x10)
#define portICCBPR_BINARY_POINT_OFFSET                  (0x08)
#define portICCRPR_RUNNING_PRIORITY_OFFSET              (0x14)

#define portINTERRUPT_CONTROLLER_CPU_INTERFACE_ADDRESS \
    (configINTERRUPT_CONTROLLER_BASE_ADDRESS +         \
     configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET)
#define portICCPMR_PRIORITY_MASK_REGISTER                                     \
    (*((volatile uint32_t *)(portINTERRUPT_CONTROLLER_CPU_INTERFACE_ADDRESS + \
                             portICCPMR_PRIORITY_MASK_OFFSET)))
#define portICCIAR_INTERRUPT_ACKNOWLEDGE_REGISTER_ADDRESS \
    (portINTERRUPT_CONTROLLER_CPU_INTERFACE_ADDRESS +     \
     portICCIAR_INTERRUPT_ACKNOWLEDGE_OFFSET)
#define portICCEOIR_END_OF_INTERRUPT_REGISTER_ADDRESS \
    (portINTERRUPT_CONTROLLER_CPU_INTERFACE_ADDRESS + \
     portICCEOIR_END_OF_INTERRUPT_OFFSET)
#define portICCPMR_PRIORITY_MASK_REGISTER_ADDRESS     \
    (portINTERRUPT_CONTROLLER_CPU_INTERFACE_ADDRESS + \
     portICCPMR_PRIORITY_MASK_OFFSET)
#define portICCBPR_BINARY_POINT_REGISTER                        \
    (*((const volatile uint32_t                                 \
            *)(portINTERRUPT_CONTROLLER_CPU_INTERFACE_ADDRESS + \
               portICCBPR_BINARY_POINT_OFFSET)))
#define portICCRPR_RUNNING_PRIORITY_REGISTER                    \
    (*((const volatile uint32_t                                 \
            *)(portINTERRUPT_CONTROLLER_CPU_INTERFACE_ADDRESS + \
               portICCRPR_RUNNING_PRIORITY_OFFSET)))

#define portLOWEST_INTERRUPT_PRIORITY \
    (((uint32_t)configUNIQUE_INTERRUPT_PRIORITIES) - 1UL)
#define portLOWEST_USABLE_INTERRUPT_PRIORITY \
    (portLOWEST_INTERRUPT_PRIORITY - 1UL)
/* The number of bits to shift for an interrupt priority is dependent on the
number of bits implemented by the interrupt controller. */
#if configUNIQUE_INTERRUPT_PRIORITIES == 16
#define portPRIORITY_SHIFT         4
#define portMAX_BINARY_POINT_VALUE 3
#elif configUNIQUE_INTERRUPT_PRIORITIES == 32
#define portPRIORITY_SHIFT         3
#define portMAX_BINARY_POINT_VALUE 2
#elif configUNIQUE_INTERRUPT_PRIORITIES == 64
#define portPRIORITY_SHIFT         2
#define portMAX_BINARY_POINT_VALUE 1
#elif configUNIQUE_INTERRUPT_PRIORITIES == 128
#define portPRIORITY_SHIFT         1
#define portMAX_BINARY_POINT_VALUE 0
#elif configUNIQUE_INTERRUPT_PRIORITIES == 256
#define portPRIORITY_SHIFT         0
#define portMAX_BINARY_POINT_VALUE 0
#else
#error Invalid configUNIQUE_INTERRUPT_PRIORITIES setting.  configUNIQUE_INTERRUPT_PRIORITIES must be set to the number of unique priorities implemented by the target hardware
#endif

/* MAC address configuration. */
#define configMAC_ADDR0 0x00
#define configMAC_ADDR1 0x12
#define configMAC_ADDR2 0x13
#define configMAC_ADDR3 0x10
#define configMAC_ADDR4 0x15
#define configMAC_ADDR5 0x11

/* IP address configuration. */
#define configIP_ADDR0  192
#define configIP_ADDR1  168
#define configIP_ADDR2  0
#define configIP_ADDR3  200

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* Architecture specific optimisations. */
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

/* Store/clear the ready priorities in a bit map. */
#define portRECORD_READY_PRIORITY(uxPriority, uxReadyPriorities) \
    (uxReadyPriorities) |= (1UL << (uxPriority))
#define portRESET_READY_PRIORITY(uxPriority, uxReadyPriorities) \
    (uxReadyPriorities) &= ~(1UL << (uxPriority))
#define portGET_HIGHEST_PRIORITY(uxTopPriority, uxReadyPriorities) \
    uxTopPriority = (31 - __clz(uxReadyPriorities))

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

#ifdef configASSERT
void vPortValidateInterruptPriority(void);
#define portASSERT_IF_INTERRUPT_PRIORITY_INVALID() \
    vPortValidateInterruptPriority()
#endif /* configASSERT */

#define portNOP()            __asm volatile("NOP")
#define portINLINE           __inline
#define portMEMORY_BARRIER() __asm volatile("" ::: "memory")

/************************** Function Prototypes ******************************/
void vApplicationAssert(const char *pcFileName, uint32_t ulLine)
    __attribute__((weak));
void vApplicationTickHook(void) __attribute__((weak));
void vApplicationIdleHook(void) __attribute__((weak));
void vApplicationMallocFailedHook(void) __attribute((weak));

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* __ICCARM__ */

#endif /* PORTMACRO_H */
