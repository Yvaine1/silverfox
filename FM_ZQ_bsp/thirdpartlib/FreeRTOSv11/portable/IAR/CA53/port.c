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

/***************************** Include Files *********************************/
/* Standard includes. */
#include <stdlib.h>

/* IAR includes. */
#include <intrinsics.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"

#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_ttc_public.h"

/************************** Constant Definitions *****************************/
#ifndef configINTERRUPT_CONTROLLER_BASE_ADDRESS
#error configINTERRUPT_CONTROLLER_BASE_ADDRESS must be defined.  See https://www.FreeRTOS.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html
#endif

#ifndef configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET
#error configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET must be defined.  See https://www.FreeRTOS.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html
#endif

#ifndef configUNIQUE_INTERRUPT_PRIORITIES
#error configUNIQUE_INTERRUPT_PRIORITIES must be defined.  See https://www.FreeRTOS.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html
#endif

#ifndef configSETUP_TICK_INTERRUPT
#error configSETUP_TICK_INTERRUPT() must be defined.  See https://www.FreeRTOS.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html
#endif

#ifndef configMAX_API_CALL_INTERRUPT_PRIORITY
#error configMAX_API_CALL_INTERRUPT_PRIORITY must be defined.  See https://www.FreeRTOS.org/Using-FreeRTOS-on-Cortex-A-Embedded-Processors.html
#endif

#if configMAX_API_CALL_INTERRUPT_PRIORITY == 0
#error configMAX_API_CALL_INTERRUPT_PRIORITY must not be set to 0
#endif

#if configMAX_API_CALL_INTERRUPT_PRIORITY > configUNIQUE_INTERRUPT_PRIORITIES
#error configMAX_API_CALL_INTERRUPT_PRIORITY must be less than or equal to configUNIQUE_INTERRUPT_PRIORITIES as the lower the numeric priority value the higher the logical interrupt priority
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1
/* Check the configuration. */
#if (configMAX_PRIORITIES > 32)
#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
#endif
#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/* In case security extensions are implemented. */
#if configMAX_API_CALL_INTERRUPT_PRIORITY <= \
    (configUNIQUE_INTERRUPT_PRIORITIES / 2)
#error configMAX_API_CALL_INTERRUPT_PRIORITY must be greater than ( configUNIQUE_INTERRUPT_PRIORITIES / 2 )
#endif

/* A critical section is exited when the critical section nesting count reaches
this value. */
#define portNO_CRITICAL_NESTING       ((size_t)0)

/* Tasks are not created with a floating point context, but can be given a
floating point context after they have been created.  A variable is stored as
part of the tasks context that holds portNO_FLOATING_POINT_CONTEXT if the task
does not have an FPU context, or any other value if the task does have an FPU
context. */
#define portNO_FLOATING_POINT_CONTEXT ((StackType_t)0)

/* Used by portASSERT_IF_INTERRUPT_PRIORITY_INVALID() when ensuring the binary
point is zero. */
#define portBINARY_POINT_BITS         ((uint8_t)0x03)

/* In all GICs 255 can be written to the priority mask register to unmask all
(but the lowest) interrupt priority. */
#define portUNMASK_VALUE              (0xFFUL)

/* Constants required to setup the initial task context. */
#define portSP_ELx                    ((StackType_t)0x01)
#define portSP_EL0                    ((StackType_t)0x00)
#if defined(GUEST)
#define portEL1            ((StackType_t)0x04)
#define portINITIAL_PSTATE (portEL1 | portSP_EL0)
#else
#define portEL3            ((StackType_t)0x0c)
/* At the time of writing, the BSP only supports EL3. */
#define portINITIAL_PSTATE (portEL3 | portSP_EL0)
#endif

/* Masks all bits in the APSR other than the mode bits. */
#define portAPSR_MODE_BITS_MASK                (0x0C)

/* Hardware specifics used when sanity checking the configuration. */
#define portINTERRUPT_PRIORITY_REGISTER_OFFSET 0x10400UL
#define portMAX_8_BIT_VALUE                    ((uint8_t)0xff)
#define portBIT_0_SET                          ((uint8_t)0x01)

#define configTICK_USE_TIMER

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FGicPs IntcInstance;
#ifdef configTICK_USE_TIMER
static FTtcPs_T timer;
#endif

/* Saved as part of the task context.  If ullPortTaskHasFPUContext is non-zero
then floating point context must be saved and restored for the task. */
volatile UBaseType_t uxTaskHasFPUContext[configNUMBER_OF_CORES] = {pdFALSE};

/* Set to 1 to pend a context switch from an ISR. */
volatile UBaseType_t uxYieldRequired[configNUMBER_OF_CORES] = {pdFALSE};

/* Counts the interrupt nesting depth.  A context switch is only performed if
if the nesting depth is 0. */
volatile UBaseType_t uxInterruptNesting[configNUMBER_OF_CORES] = {pdFALSE};

/* Counts the critical nesting depth. */
volatile UBaseType_t uxCriticalNesting[configNUMBER_OF_CORES] = {pdFALSE};

/* Used in the ASM code. */
__attribute__((used))
const uint64_t ullICCEOIR = portICCEOIR_END_OF_INTERRUPT_REGISTER_ADDRESS;
__attribute__((used))
const uint64_t ullICCIAR = portICCIAR_INTERRUPT_ACKNOWLEDGE_REGISTER_ADDRESS;
__attribute__((used))
const uint64_t ullICCPMR = portICCPMR_PRIORITY_MASK_REGISTER_ADDRESS;
__attribute__((used))
const uint64_t ullMaxAPIPriorityMask = (configMAX_API_CALL_INTERRUPT_PRIORITY
                                        << portPRIORITY_SHIFT);

/************************** Function Prototypes ******************************/
/*
 * Starts the first task executing.  This function is necessarily written in
 * assembly code so is implemented in portASM.s.
 */
extern void vPortRestoreTaskContext(void);

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError(void);

void vPortStartSMPScheduler();

/*****************************************************************************/

StackType_t *pxPortInitialiseStack (StackType_t *pxTopOfStack,
                                    TaskFunction_t pxCode, void *pvParameters)
{
    /* Setup the initial stack of the task.  The stack is set exactly as
    expected by the portRESTORE_CONTEXT() macro. */

    /* First all the general purpose registers. */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0101010101010101ULL; /* R1 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)pvParameters;          /* R0 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0303030303030303ULL; /* R3 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0202020202020202ULL; /* R2 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0505050505050505ULL; /* R5 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0404040404040404ULL; /* R4 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0707070707070707ULL; /* R7 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0606060606060606ULL; /* R6 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0909090909090909ULL; /* R9 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x0808080808080808ULL; /* R8 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1111111111111111ULL; /* R11 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1010101010101010ULL; /* R10 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1313131313131313ULL; /* R13 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1212121212121212ULL; /* R12 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1515151515151515ULL; /* R15 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1414141414141414ULL; /* R14 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1717171717171717ULL; /* R17 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1616161616161616ULL; /* R16 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1919191919191919ULL; /* R19 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x1818181818181818ULL; /* R18 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2121212121212121ULL; /* R21 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2020202020202020ULL; /* R20 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2323232323232323ULL; /* R23 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2222222222222222ULL; /* R22 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2525252525252525ULL; /* R25 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2424242424242424ULL; /* R24 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2727272727272727ULL; /* R27 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2626262626262626ULL; /* R26 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2929292929292929ULL; /* R29 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x2828282828282828ULL; /* R28 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x00; /* XZR - has no effect, used so there are
                                          an even number of registers. */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x00; /* R30 - procedure call link register. */
    pxTopOfStack--;

    *pxTopOfStack = portINITIAL_PSTATE;
    pxTopOfStack--;

    *pxTopOfStack = (StackType_t)pxCode; /* Exception return address. */
    pxTopOfStack--;

    /* The task will start with a critical nesting count of 0 as interrupts are
    enabled. */
    *pxTopOfStack = portNO_CRITICAL_NESTING;
    pxTopOfStack--;

    /* The task will start without a floating point context.  A task that uses
    the floating point hardware must call vPortTaskUsesFPU() before executing
    any floating point instructions. */
    *pxTopOfStack = portNO_FLOATING_POINT_CONTEXT;

    return pxTopOfStack;
}

BaseType_t xPortStartScheduler (void)
{
    uint64_t ulAPSR;

    /* At the time of writing, the BSP only supports EL3. */
    __asm volatile("MRS %0, CurrentEL" : "=r"(ulAPSR));
    ulAPSR &= portAPSR_MODE_BITS_MASK;

#if defined(GUEST)
#warning Building for execution as a guest under XEN. THIS IS NOT A FULLY TESTED PATH.
    configASSERT(ulAPSR == portEL1);
    if (ulAPSR == portEL1)
#else
    configASSERT(ulAPSR == portEL3);
    if (ulAPSR == portEL3)
#endif
    {
#if (configNUMBER_OF_CORES == 1)
        {
            /* Start the timer that generates the tick ISR. */
            configSETUP_TICK_INTERRUPT();
            portCLEAR_INTERRUPT_MASK(portUNMASK_VALUE);
            /* Start the first task executing. */
            vPortRestoreTaskContext();
        }
#else
        {
            vPortStartSMPScheduler();
        }
#endif
    }

    return 0;
}

void vPortEndScheduler (void) {}

__attribute__((unused)) static void prvTaskExitError (void)
{
    /* A function that implements a task must not exit or attempt to return to
    its caller as there is nothing to return to.  If a task wants to exit it
    should instead call vTaskDelete( NULL ).

    Artificially force an assert() to be triggered if configASSERT() is
    defined, then stop here so application writers can catch the error. */
    portDISABLE_INTERRUPTS();
    for (;;);
}

void vPortSaveTopStack (StackType_t *topOfStack)
{
    TaskHandle_t tcb;

    tcb = xTaskGetCurrentTaskHandleForCore(portGET_CORE_ID());
    /* The pxCurrentTCBs use the function without from_isr */
    /* tcb = pxCurrentTCBs[portGET_CORE_ID()]; */

    *(StackType_t **)tcb = topOfStack;
}

StackType_t *vPortRestoreTopStack ()
{
    TaskHandle_t tcb;

    tcb = xTaskGetCurrentTaskHandleForCore(portGET_CORE_ID());

    return *(StackType_t **)tcb;
}

void vPortTaskUsesFPU (void)
{
#if (configNUMBER_OF_CORES == 1)
    {
        /* A task is registering the fact that it needs an FPU context.  Set the
        FPU flag (which is saved as part of the task context). */
        uxTaskHasFPUContext[0] = pdTRUE;

        /* Consider initialising the FPSR here - but probably not necessary in
        AArch64. */
    }
#else
    {
        uxTaskHasFPUContext[portGET_CORE_ID()] = pdTRUE;
    }
#endif
}

UBaseType_t uxPortGetFPUContext (void)
{
#if (configNUMBER_OF_CORES == 1)
    {
        return uxTaskHasFPUContext[0];
    }
#else
    {
        return uxTaskHasFPUContext[portGET_CORE_ID()];
    }
#endif
}

UBaseType_t uxPortSetFPUContext (UBaseType_t uxValue)
{
#if (configNUMBER_OF_CORES == 1)
    {
        uxTaskHasFPUContext[0] = uxValue;
    }
#else
    {
        uxTaskHasFPUContext[portGET_CORE_ID()] = uxValue;
    }
#endif

    return uxValue;
}

UBaseType_t uxGetInterruptNesting ()
{
#if (configNUMBER_OF_CORES == 1)
    {
        return uxInterruptNesting[0];
    }
#else
    {
        return uxInterruptNesting[portGET_CORE_ID()];
    }
#endif
}

UBaseType_t uxPortGetCriticalNesting (void)
{
#if (configNUMBER_OF_CORES == 1)
    {
        return uxCriticalNesting[0];
    }
#else
    {
        return uxCriticalNesting[portGET_CORE_ID()];
    }
#endif
}

UBaseType_t uxPortSetCriticalNesting (UBaseType_t uxValue)
{
#if (configNUMBER_OF_CORES == 1)
    {
        uxCriticalNesting[0] = uxValue;
    }
#else
    {
        uxCriticalNesting[portGET_CORE_ID()] = uxValue;
    }
#endif

    return uxValue;
}

void portYIELD_FROM_ISR (UBaseType_t xSwitchRequired)
{
    if (xSwitchRequired != pdFALSE)
    {
#if (configNUMBER_OF_CORES == 1)
        uxYieldRequired[0] = pdTRUE;
#else
        uxYieldRequired[portGET_CORE_ID()] = pdTRUE;
#endif
    }

    return;
}

UBaseType_t uxGetYieldRequired ()
{
    UBaseType_t uxReturn;

#if (configNUMBER_OF_CORES == 1)
    {
        uxReturn = uxYieldRequired[0];
        uxYieldRequired[0] = 0;
    }
#else
    {
        UBaseType_t uxCoreID;

        uxCoreID = portGET_CORE_ID();

        uxReturn = uxYieldRequired[uxCoreID];
        uxYieldRequired[portGET_CORE_ID()] = 0;
    }
#endif

    return uxReturn;
}

void vPortTaskSwitchContext ()
{
#if (configNUMBER_OF_CORES == 1)
    {
        vTaskSwitchContext();
    }
#else  /* #elif ( configNUMBER_OF_CORES > 1 ) */
    {
        vTaskSwitchContext(portGET_CORE_ID());
    }
#endif /* end #if ( configNUMBER_OF_CORES == 1 ) */
}

// UBaseType_t uxPortSetInterruptMask( void )
//{
//     UBaseType_t ulReturn;
//
//	/* Interrupt in the CPU must be turned off while the ICCPMR is being
//	updated. */
//	portDISABLE_INTERRUPTS();
//
//     ulReturn = portICCPMR_PRIORITY_MASK_REGISTER;
//	portICCPMR_PRIORITY_MASK_REGISTER = ( uint32_t ) (
// configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );
//	__asm volatile ("dsb sy		\n"
//					"isb sy		\n" ::: "memory" );
//
//	portENABLE_INTERRUPTS();
//
//	return ulReturn;
// }

// void vPortClearInterruptMask( UBaseType_t ulPreviousMask )
//{
//	/* Interrupt in the CPU must be turned off while the ICCPMR is being
//	updated. */
//	portDISABLE_INTERRUPTS();
//
//	portICCPMR_PRIORITY_MASK_REGISTER = ulPreviousMask;
//	__asm volatile ("dsb sy		\n"
//					"isb sy		\n" ::: "memory" );
//
//	portENABLE_INTERRUPTS();
// }

/* The I bit in the DAIF bits. */
#define portDAIF_I                    (0x80)

/* The Mask Flag for InterruptMask */
#define portINTERRUPT_MASK_FLAG_BIT   (1 << 8)
#define portINTERRUPT_MASK_FLAG_VALUE (1)

void vPortClearInterruptMask (UBaseType_t uxNewMaskValue)
{
    portCPU_DISABLE_INTERRUPTS();

    if (!(uxNewMaskValue & portINTERRUPT_MASK_FLAG_VALUE))
    {
        portICCPMR_PRIORITY_MASK_REGISTER = portUNMASK_VALUE;
        __asm volatile(
            "DSB SY     \n"
            "ISB SY     \n");
    }

    if (!(uxNewMaskValue & portINTERRUPT_MASK_FLAG_BIT))
    {
        portCPU_ENABLE_INTERRUPTS();
    }
}

UBaseType_t uxPortSetInterruptMask (void)
{
    uint64_t ullMaskBits;
    uint32_t ulReturn = 0;

    __asm volatile("MRS %0, DAIF" : "=r"(ullMaskBits)::"memory");

    if ((ullMaskBits & portDAIF_I))
    {
        /* Interrupts mask bit DAIF_I were already masked. */
        ulReturn |= portINTERRUPT_MASK_FLAG_BIT;
    }

    /* Interrupt in the CPU must be turned off while the ICCPMR is being
     * updated. */
    portCPU_DISABLE_INTERRUPTS();

    if (portICCPMR_PRIORITY_MASK_REGISTER == portICCPMR_PRIORITY_MASK)
    {
        /* Interrupts mask were already masked. */
        ulReturn |= portINTERRUPT_MASK_FLAG_VALUE;
    }
    else
    {
        portICCPMR_PRIORITY_MASK_REGISTER =
            (uint32_t)(portICCPMR_PRIORITY_MASK);
        __asm volatile(
            "dsb sy     \n"
            "isb sy     \n" ::
                : "memory");
    }

    portCPU_ENABLE_INTERRUPTS();

    return ulReturn;
}
void vPortIRQHandler (void)
{
    extern const FGicPs_Config FGicPs_ConfigTable[];
    static const FGicPs_VectorTableEntry
        *pxVectorTable = FGicPs_ConfigTable[GIC_DEVICE_ID].HandlerTable;
    uint32_t ulInterruptID;
    const FGicPs_VectorTableEntry *pxVectorEntry;
    uint32_t ulICCIAR;
    UBaseType_t ulPreviousMask;

#if (configNUMBER_OF_CORES == 1)
    {
        uxInterruptNesting[0]++;
    }
#else
    {
        uxInterruptNesting[portGET_CORE_ID()]++;
    }
#endif

    /* The ID of the interrupt is obtained by bitwise anding the ICCIAR value
    with 0x3FF. */
    ulICCIAR = *(
        (volatile uint32_t *)portICCIAR_INTERRUPT_ACKNOWLEDGE_REGISTER_ADDRESS);
    ulInterruptID = ulICCIAR & 0x3FFUL;

    configASSERT(ulInterruptID < FGicPs_MAX_NUM_INTR_INPUTS);

    /* Only interrupts priority < 18 are allowed nesting after set mask */
    ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();

    /* Write ICCEOIR, clear it for interrupt nesting */
   // *((volatile uint32_t *)
   //       portICCEOIR_END_OF_INTERRUPT_REGISTER_ADDRESS) = ulICCIAR;

    /* All interrupts are allowed nesting now */

    /* Call the function installed in the array of installed handler functions.
     */
    pxVectorEntry = &(pxVectorTable[ulInterruptID]);
    pxVectorEntry->Handler(pxVectorEntry->CallBackRef);

    portCLEAR_INTERRUPT_MASK_FROM_ISR(ulPreviousMask);
/* All interrupts are disabled now */

/* Interrupts should not be enabled this point. */
#if (configASSERT_DEFINED == 1)
    {
        uint64_t ullMaskBits;

        __asm volatile("mrs %0, daif" : "=r"(ullMaskBits)::"memory");
        configASSERT((ullMaskBits & portDAIF_I) != 0);
    }
#endif /* configASSERT_DEFINED */

    *( ( volatile uint32_t * )portICCEOIR_END_OF_INTERRUPT_REGISTER_ADDRESS)
    = ulICCIAR;

#if (configNUMBER_OF_CORES == 1)
    {
        uxInterruptNesting[0]--;
    }
#else
    {
        uxInterruptNesting[portGET_CORE_ID()]--;
    }
#endif
}

void vPortTickHandler (void)
{
    UBaseType_t ulPreviousMask;
#if (configNUMBER_OF_CORES == 1)
    {
        ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();

        configCLEAR_TICK_INTERRUPT();

        if (xTaskIncrementTick() != pdFALSE)
        {
            uxYieldRequired[0] = pdTRUE;
        }

        portCLEAR_INTERRUPT_MASK_FROM_ISR(ulPreviousMask);
    }
#else
    {
        ulPreviousMask = taskENTER_CRITICAL_FROM_ISR();

        configCLEAR_TICK_INTERRUPT();

        if (portGET_CORE_ID() == configTICK_CORE)
        {
            if (xTaskIncrementTick() != pdFALSE)
            {
                uxYieldRequired[portGET_CORE_ID()] = pdTRUE;
            }
        }

        taskEXIT_CRITICAL_FROM_ISR(ulPreviousMask);
    }
#endif /* if ( configNUMBER_OF_CORES == 1 ) */
}

#if (configASSERT_DEFINED == 1)

void vPortValidateInterruptPriority (void)
{
    /* The following assertion will fail if a service routine (ISR) for
    an interrupt that has been assigned a priority above
    configMAX_SYSCALL_INTERRUPT_PRIORITY calls an ISR safe FreeRTOS API
    function.  ISR safe FreeRTOS API functions must *only* be called
    from interrupts that have been assigned a priority at or below
    configMAX_SYSCALL_INTERRUPT_PRIORITY.

    Numerically low interrupt priority numbers represent logically high
    interrupt priorities, therefore the priority of the interrupt must
    be set to a value equal to or numerically *higher* than
    configMAX_SYSCALL_INTERRUPT_PRIORITY.

    FreeRTOS maintains separate thread and ISR API functions to ensure
    interrupt entry is as fast and simple as possible.

    The following links provide detailed information:
    https://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html
    https://www.FreeRTOS.org/FAQHelp.html */
    configASSERT(portICCRPR_RUNNING_PRIORITY_REGISTER >=
                 (uint32_t)(configMAX_API_CALL_INTERRUPT_PRIORITY
                            << portPRIORITY_SHIFT));

    /* Priority grouping:  The interrupt controller (GIC) allows the bits
    that define each interrupt's priority to be split between bits that
    define the interrupt's pre-emption priority bits and bits that define
    the interrupt's sub-priority.  For simplicity all bits must be defined
    to be pre-emption priority bits.  The following assertion will fail if
    this is not the case (if some bits represent a sub-priority).

    The priority grouping is configured by the GIC's binary point register
    (ICCBPR).  Writting 0 to ICCBPR will ensure it is set to its lowest
    possible value (which may be above 0). */
    configASSERT((portICCBPR_BINARY_POINT_REGISTER & portBINARY_POINT_BITS) <=
                 portMAX_BINARY_POINT_VALUE);
}

#endif /* configASSERT_DEFINED */

#if (configNUMBER_OF_CORES > 1)

UBaseType_t uxPortGetCoreId ()
{
    BaseType_t mpidr;

    __asm volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr));

    return mpidr & 0xff;
}

static void vPortInterruptCore (UBaseType_t xOtherCoreID)
{
    FGicPs_SoftwareIntr(&IntcInstance, YIELD_CORE_INTERRUPT_NO,
                        0x1 << xOtherCoreID);
}

void vPortIPIHandler (void)
{
    UBaseType_t ulPreviousMask;

    ulPreviousMask = taskENTER_CRITICAL_FROM_ISR();

    uxYieldRequired[portGET_CORE_ID()] = pdTRUE;

    taskEXIT_CRITICAL_FROM_ISR(ulPreviousMask);
}

void vPortYieldCore (BaseType_t xOtherCoreID)
{
    if (xOtherCoreID < configNUMBER_OF_CORES)
    {
        vPortInterruptCore(xOtherCoreID);
    }
}

#endif /* end #if ( configNUMBER_OF_CORES > 1 ) */

/*-----------------------------------------------------------*/
static int32_t prvInitialiseInterruptController (void)
{
    int32_t status;
    FGicPs_Config *gicConfig;
    // u32 value;

    /* Initialize the interrupt controller driver. */
    gicConfig = FGicPs_LookupConfig(GIC_DEVICE_ID);
    status = FGicPs_CfgInitialize(&IntcInstance, gicConfig,
                                  gicConfig->CpuBaseAddress);
    if (status)
    {
        return 1;
    }

    /* Connect the interrupt controller interrupt handler to the hardware
    interrupt handling logic in the ARM processor. */
    FMSH_ExceptionRegisterHandler(
        FMSH_EXCEPTION_ID_IRQ_INT,
        (FMSH_ExceptionHandler)FGicPs_InterruptHandler_IRQ, &IntcInstance);

    /* Set default interrupt priority */
    portICCPMR_PRIORITY_MASK_REGISTER = portUNMASK_VALUE;

    return 0;
}

#ifdef configTICK_USE_TIMER
static int32_t prvInitialiseTimer (void)
{
    FTtcPs_Config *pxTimerConfig;

    /* Initialize triple-timer driver */
    pxTimerConfig = FTtcPs_LookupConfig(configTIMER_ID);
    if (pxTimerConfig == NULL)
    {
        fmsh_print("In %s: Timer Cfg initialization failed.\r\n", __func__);
        return 1;
    }
    FTtcPs_init(&timer, pxTimerConfig);

    /* Stop timer */
    FTtcPs_setTimerEnble(&timer, configTIMER_NO, FMSH_clear);

    FTtcPs_setTimerMode(&timer, configTIMER_NO, user_DefinedCount_mode);
    FTtcPs_setTIMERPWM(&timer, configTIMER_NO, FMSH_clear);
    FTtcPs_TimerNLoadCount(&timer, configTIMER_NO,
                           TTC0_FREQ / configTICK_RATE_HZ);
    FTtcPs_setTimerInterruptMask(&timer, configTIMER_NO, FMSH_clear);

    /* Start Timer */
    FTtcPs_setTimerEnble(&timer, configTIMER_NO, FMSH_set);

    return 0;
}
#endif

static int prvInstallTickInterrupt ()
{
#ifdef configTICK_USE_TIMER
    {
        const uint8_t ucLevel = 1;

        FGicPs_Connect(&IntcInstance, configTIMER_INTERRUPT_ID,
                       (FMSH_InterruptHandler)vPortTickHandler, (void *)0);

        /* The priority must be the lowest possible. */
        FGicPs_SetPriorityTriggerType(
            &IntcInstance, configTIMER_INTERRUPT_ID,
            portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT,
            ucLevel);
    }
#else
    {
        const uint8_t ucLevel = 1;

        FGicPs_Connect(&IntcInstance, SCU_TMR_INT_ID,
                       (FMSH_InterruptHandler)vPortTickHandler, (void *)0);

        /* The priority must be the lowest possible. */
        FGicPs_SetPriorityTriggerType(
            &IntcInstance, SCU_TMR_INT_ID,
            portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT,
            ucLevel);
    }
#endif

    return 0;
}

static int prvEnableTickInperrupt ()
{
#ifdef configTICK_USE_TIMER
    {
        FGicPs_Enable(&IntcInstance, configTIMER_INTERRUPT_ID);
    }
#else
    {
        u32 value = 0x1;

        FGicPs_Enable(&IntcInstance, SCU_TMR_INT_ID);

        __asm volatile("MSR CNTPS_CTL_EL1, %0" ::"r"(value));
    }
#endif

    return 0;
}

void vPortSetupTickInterrupt (void)
{
    prvInitialiseInterruptController();

#ifdef configTICK_USE_TIMER
    {
        prvInitialiseTimer();
    }
#else
    {
        global_timer_enable();
    }
#endif

    prvInstallTickInterrupt();

    configCLEAR_TICK_INTERRUPT();

    prvEnableTickInperrupt();
}

void vPortClearTickInterrupt (void)
{
#ifdef configTICK_USE_TIMER
    {
        FTtcPs_ClearTimerNInterrupt(&timer, configTIMER_NO);
    }
#else
    {
        uint64_t value, value1;

        __asm volatile("MRS %0, CNTPS_CTL_EL1" : "=r"(value));

        /* mask interrupt */
        value |= 0x2;
        __asm volatile("MSR CNTPS_CTL_EL1, %0" ::"r"(value));

        /* reset counter value */
#if (configGENERATE_RUN_TIME_STATS == 1)
        value1 = GTC_FREQ / (configTICK_RATE_HZ * 10);
#else
        value1 = GTC_FREQ / (configTICK_RATE_HZ);
#endif
        __asm volatile("MSR CNTPS_TVAL_EL1, %0" ::"r"(value1));

        /* unmask interrupt */
        value &= ~0x2;
        __asm volatile("MSR CNTPS_CTL_EL1, %0" ::"r"(value));

        __asm volatile("DSB SY");
        __asm volatile("ISB SY");
    }
#endif
}

/*-----------------------------------------------------------*/
#if (configNUMBER_OF_CORES > 1)

static uint64_t core_status[configNUMBER_OF_CORES] = {0};

extern void vSecondaryEntry();

void vSecondaryStart ()
{
    Fmsh_ICacheEnable();

#if PSOC_DCACHE_ENABLE == 1
    Fmsh_DCacheEnable();
#endif

    portCPU_DISABLE_INTERRUPTS();
    FGicPs_CPUWriteReg(&IntcInstance, FGicPs_CPU_PRIOR_OFFSET, 0xF8U);
    FGicPs_CPUWriteReg(&IntcInstance, FGicPs_CONTROL_OFFSET, 0xa3);  // 0x03U);

    /* Initialize interrupt */
    FGicPs_Connect(&IntcInstance, YIELD_CORE_INTERRUPT_NO,
                   (FMSH_InterruptHandler)vPortIPIHandler, (void *)0);
    FGicPs_SetPriorityTriggerType(
        &IntcInstance, YIELD_CORE_INTERRUPT_NO,
        portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT, 1);
    FGicPs_Enable(&IntcInstance, YIELD_CORE_INTERRUPT_NO);

    // configCLEAR_TICK_INTERRUPT();

    // prvEnableTickInperrupt();

    /* Set default interrupt priority */
    portICCPMR_PRIORITY_MASK_REGISTER = portUNMASK_VALUE;

    /* wait for secondary core ready */
    core_status[portGET_CORE_ID()] = 1;

    /* Start first task */
    vPortRestoreTaskContext();

    /* Interrupt must enable after first task ready, it may receive an IPI at
     * any time */
    portENABLE_INTERRUPTS();

    while (1);
}

void vStartCore (UBaseType_t xCoreID)
{
    uint32_t value;
    uint64_t addr;

    // hold core por reset
    value = FMSH_ReadReg(0xfd1a0000, 0x104);
    value |= ((0x1 << xCoreID) | (0x400 << xCoreID));
    FMSH_WriteReg(0xfd1a0000, 0x104, value);

    // assign core start address(RVBAR)
    addr = (uint64_t)vSecondaryEntry;
    if (xCoreID == 0x1U)
    {
        FMSH_WriteReg(0xfd5c0000, 0x48, addr & 0xffffffff);
        FMSH_WriteReg(0xfd5c0000, 0x4c, (addr >> 32) & 0xff);
    }
    else if (xCoreID == 0x2U)
    {
        FMSH_WriteReg(0xfd5c0000, 0x50, addr & 0xffffffff);
        FMSH_WriteReg(0xfd5c0000, 0x54, (addr >> 32) & 0xff);
    }
    else if (xCoreID == 0x3U)
    {
        FMSH_WriteReg(0xfd5c0000, 0x58, addr & 0xffffffff);
        FMSH_WriteReg(0xfd5c0000, 0x5c, (addr >> 32) & 0xff);
    }

    // release core por reset
    value = FMSH_ReadReg(0xfd1a0000, 0x104);
    value &= ~((0x1 << xCoreID) | (0x400 << xCoreID));
    FMSH_WriteReg(0xfd1a0000, 0x104, value);
}

void vPortStartSMPScheduler ()
{
    int i;

    /* Start the timer that generates the tick ISR. */
    configSETUP_TICK_INTERRUPT();

    /* Start secondary cores */
    for (i = 0; i < configNUMBER_OF_CORES; i++)
    {
        if (i != configTICK_CORE)
        {
            vStartCore(i);
            /* wait for secondary core ready */
            while (core_status[i] == 0);
        }
    }

    FGicPs_SetPriorityTriggerType(
        &IntcInstance, YIELD_CORE_INTERRUPT_NO,
        portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT, 1);

    /* Enable interrupt */
    configCLEAR_TICK_INTERRUPT();

    /* Start the first task executing. */
    vPortRestoreTaskContext();
}

/*-----------------------------------------------------------
 * Critical section locks
 *----------------------------------------------------------*/
uint64_t Get_64(volatile uint64_t *x);
void Set_64(volatile uint64_t *x, uint64_t value);

/* Which core owns the lock */
volatile uint64_t ucOwnedByCore[configNUMBER_OF_CORES];
/* Lock count a core owns */
volatile uint64_t ucRecursionCountByLock[portRTOS_LOCK_COUNT];

/* Index 0 is used for ISR lock and Index 1 is used for task lock */
uint32_t GateWord[portRTOS_LOCK_COUNT];
extern int32_t SMP_TryLock(uint32_t *gateWord);
extern void SMP_Unlock(uint32_t *gateWord);

/* Read 64b value shared between cores */
uint64_t Get_64 (volatile uint64_t *x)
{
    __asm volatile("dsb sy");
    return *x;
}

/* Write 64b value shared between cores */
void Set_64 (volatile uint64_t *x, uint64_t value)
{
    *x = value;
    __asm volatile("dsb sy");
}

void vPortRecursiveLock (uint32_t ulLockNum, BaseType_t uxAcquire)
{
    UBaseType_t uxCoreId = portGET_CORE_ID();
    uint32_t ulLockBit = 1u << ulLockNum;

    /* Validate the core ID and lock number. */
    configASSERT(uxCoreId < configNUMBER_OF_CORES);
    configASSERT(ulLockNum < portRTOS_LOCK_COUNT);
    /* Lock acquire */
    if (uxAcquire)
    {
        /* Check if spinlock is available */
        /* If spinlock is not available check if the core owns the lock */
        /* If the core owns the lock wait increment the lock count by the core
         */
        /* If core does not own the lock wait for the spinlock */
        if (SMP_TryLock(&GateWord[ulLockNum]) != 0)
        {
            /* Check if the core owns the spinlock */
            if (Get_64(&ucOwnedByCore[uxCoreId]) & ulLockBit)
            {
                configASSERT(Get_64(&ucRecursionCountByLock[ulLockNum]) !=
                             255u);
                Set_64(&ucRecursionCountByLock[ulLockNum],
                       (Get_64(&ucRecursionCountByLock[ulLockNum]) + 1));
                return;
            }

            /* Preload the gate word into the cache */
            uint32_t dummy = GateWord[ulLockNum];
            dummy++;

            /* Wait for spinlock */
            __asm volatile("sevl");
            __asm volatile("wfe");
            while (SMP_TryLock(&GateWord[ulLockNum]) != 0)
            {
                __asm volatile("wfe");
            }
        }

        /* Add barrier to ensure lock is taken before we proceed */
        __asm volatile("dmb sy" ::: "memory");

        /* Assert the lock count is 0 when the spinlock is free and is acquired
         */
        configASSERT(Get_64(&ucRecursionCountByLock[ulLockNum]) == 0);

        /* Set lock count as 1 */
        Set_64(&ucRecursionCountByLock[ulLockNum], 1);
        /* Set ucOwnedByCore */
        Set_64(&ucOwnedByCore[uxCoreId],
               (Get_64(&ucOwnedByCore[uxCoreId]) | ulLockBit));
    }
    /* Lock release */
    else
    {
        /* Assert the lock is not free already */
        configASSERT((Get_64(&ucOwnedByCore[uxCoreId]) & ulLockBit) != 0);
        configASSERT(Get_64(&ucRecursionCountByLock[ulLockNum]) != 0);

        /* Reduce ucRecursionCountByLock by 1 */
        Set_64(&ucRecursionCountByLock[ulLockNum],
               (Get_64(&ucRecursionCountByLock[ulLockNum]) - 1));

        if (!Get_64(&ucRecursionCountByLock[ulLockNum]))
        {
            Set_64(&ucOwnedByCore[uxCoreId],
                   (Get_64(&ucOwnedByCore[uxCoreId]) & ~ulLockBit));
            SMP_Unlock(&GateWord[ulLockNum]);
            /* Add barrier to ensure lock is taken before we proceed */
            __asm volatile("dmb sy" ::: "memory");
        }
    }
}

#endif  // if (configNUMBER_OF_CORES > 1)

/*****************************************************************************/
/* This version of vApplicationAssert() is declared as a weak symbol to allow it
to be overridden by a version implemented within the application that is using
this BSP. */
void vApplicationAssert (const char *pcFileName, uint32_t ulLine)
{
    volatile uint32_t ul = 0;
    const volatile char
        *pcLocalFileName = pcFileName; /* To prevent pcFileName being optimized
                                          away. */
    volatile uint32_t
        ulLocalLine = ulLine; /* To prevent ulLine being optimized away. */

    /* Prevent compile warnings about the following two variables being set but
    not referenced.  They are intended for viewing in the debugger. */
    (void)pcLocalFileName;
    (void)ulLocalLine;

    fmsh_print("Assert failed in file %s, line %lu\r\n", pcLocalFileName,
               ulLocalLine);

    /* If this function is entered then a call to configASSERT() failed in the
    FreeRTOS code because of a fatal error.  The pcFileName and ulLine
    parameters hold the file name and line number in that file of the assert
    that failed.  Additionally, if using the debugger, the function call stack
    can be viewed to find which line failed its configASSERT() test.  Finally,
    the debugger can be used to set ul to a non-zero value, then step out of
    this function to find where the assert function was entered. */
    taskENTER_CRITICAL();
    {
        while (ul == 0)
        {
            asm volatile("NOP");
        }
    }
    taskEXIT_CRITICAL();
}

/* This default tick hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationTickHook (void) {}

/* This default idle hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationIdleHook (void)
{
    volatile size_t xFreeStackSpace;

    /* This function is called on each cycle of the idle task.  In this case it
    does nothing useful, other than report the amount of FreeRTOS heap that
    remains unallocated. */
    xFreeStackSpace = xPortGetFreeHeapSize();

    if (xFreeStackSpace > 100)
    {
        /* By now, the kernel has allocated everything it is going to, so
        if there is a lot of heap remaining unallocated then
        the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
        reduced accordingly. */
    }

    /* Sleep */
    asm volatile("wfi");
}

/* This default malloc failed hook does nothing and is declared as a weak symbol
to allow the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationMallocFailedHook (void)
{
    fmsh_print("vApplicationMallocFailedHook() called\r\n");
}

/* This default stack overflow hook will stop the application for executing.  It
is declared as a weak symbol to allow the application writer to override this
default by providing their own implementation in the application code. */
extern void backtrace_overflow(TaskHandle_t xTask);
void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName)
{
    /* Attempt to prevent the handle and name of the task that overflowed its
    stack from being optimised away because they are not used. */
    volatile TaskHandle_t xOverflowingTaskHandle = xTask;
    volatile char *pcOverflowingTaskName = pcTaskName;

    //(void)xOverflowingTaskHandle;
    //(void)pcOverflowingTaskName;

    portDISABLE_INTERRUPTS();
    fmsh_print( "A53 HALT: Task %s overflowed its stack.\r\n", pcOverflowingTaskName );
    backtrace_overflow(xTask);

    for (;;);
}
