/*
* FreeRTOS Kernel V10.5.1
* Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#if( configMAX_PRIORITIES > 32 )
#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
#endif
#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/* In case security extensions are implemented. */
#if configMAX_API_CALL_INTERRUPT_PRIORITY <= ( configUNIQUE_INTERRUPT_PRIORITIES / 2 )
#error configMAX_API_CALL_INTERRUPT_PRIORITY must be greater than ( configUNIQUE_INTERRUPT_PRIORITIES / 2 )
#endif

/* A critical section is exited when the critical section nesting count reaches
this value. */
#define portNO_CRITICAL_NESTING			( ( uint32_t ) 0 )

/* Tasks are not created with a floating point context, but can be given a
floating point context after they have been created.  A variable is stored as
part of the tasks context that holds portNO_FLOATING_POINT_CONTEXT if the task
does not have an FPU context, or any other value if the task does have an FPU
context. */
#define portNO_FLOATING_POINT_CONTEXT	( ( StackType_t ) 0 )

/* Used by portASSERT_IF_INTERRUPT_PRIORITY_INVALID() when ensuring the binary
point is zero. */
#define portBINARY_POINT_BITS			( ( uint8_t ) 0x03 )

/* In all GICs 255 can be written to the priority mask register to unmask all
(but the lowest) interrupt priority. */
#define portUNMASK_VALUE				( 0xFFUL )

/* Constants required to setup the initial task context. */
#define portINITIAL_SPSR				( ( StackType_t ) 0x1f ) /* System mode, ARM mode, interrupts enabled. */
#define portTHUMB_MODE_BIT				( ( StackType_t ) 0x20 )
#define portTHUMB_MODE_ADDRESS			( 0x01UL )

/* Masks all bits in the APSR other than the mode bits. */
#define portAPSR_MODE_BITS_MASK			( 0x1F )

/* The value of the mode bits in the APSR when the CPU is executing in user
mode. */
#define portAPSR_USER_MODE				( 0x10 )

/***************** Macros (Inline Functions) Definitions *********************/  


/************************** Variable Definitions *****************************/ 
extern FGicPs IntcInstance;
static FTtcPs_T timer;

/* Saved as part of the task context.  If ullPortTaskHasFPUContext is non-zero
then floating point context must be saved and restored for the task. */
volatile UBaseType_t uxTaskHasFPUContext[configNUMBER_OF_CORES] = { pdFALSE };

/* Set to 1 to pend a context switch from an ISR. */
volatile UBaseType_t uxYieldRequired[configNUMBER_OF_CORES] = { pdFALSE };

/* Counts the interrupt nesting depth.  A context switch is only performed if
if the nesting depth is 0. */
volatile UBaseType_t uxInterruptNesting[configNUMBER_OF_CORES] = { pdFALSE };

/* Counts the critical nesting depth. */
volatile UBaseType_t uxCriticalNesting[configNUMBER_OF_CORES] = { pdFALSE };

/* Used in the ASM code. */
__attribute__(( used )) const uint64_t ullICCEOIR = portICCEOIR_END_OF_INTERRUPT_REGISTER_ADDRESS;
__attribute__(( used )) const uint64_t ullICCIAR = portICCIAR_INTERRUPT_ACKNOWLEDGE_REGISTER_ADDRESS;
__attribute__(( used )) const uint64_t ullICCPMR = portICCPMR_PRIORITY_MASK_REGISTER_ADDRESS;
__attribute__(( used )) const uint64_t ullMaxAPIPriorityMask = ( configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );

/************************** Function Prototypes ******************************/
/*
* Starts the first task executing.  This function is necessarily written in
* assembly code so is implemented in portASM.s.
*/
extern void vPortRestoreTaskContext( void );

/*
* Used to catch tasks that attempt to return from their implementing function.
*/
static void prvTaskExitError( void );

void vPortStartSMPScheduler();

/*****************************************************************************/

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    /* Setup the initial stack of the task.  The stack is set exactly as
    expected by the portRESTORE_CONTEXT() macro.
    
    The fist real value on the stack is the status register, which is set for
    system mode, with interrupts enabled.  A few NULLs are added first to ensure
    GDB does not try decoding a non-existent return address. */
    *pxTopOfStack = NULL;
    pxTopOfStack--;
    *pxTopOfStack = NULL;
    pxTopOfStack--;
    *pxTopOfStack = NULL;
    pxTopOfStack--;
    
    *pxTopOfStack = ( StackType_t ) portINITIAL_SPSR;
    if( ( ( uint32_t ) pxCode & portTHUMB_MODE_ADDRESS ) != 0x00UL )
    {
        /* The task will start in THUMB mode. */
        *pxTopOfStack |= portTHUMB_MODE_BIT;
    }
    pxTopOfStack--;
    
    /* Next the return address, which in this case is the start of the task. */
    *pxTopOfStack = ( StackType_t ) pxCode;
    pxTopOfStack--;
    
    /* Next all the registers other than the stack pointer. */
    *pxTopOfStack = ( StackType_t ) prvTaskExitError;	/* R14 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x12121212;	/* R12 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x11111111;	/* R11 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x10101010;	/* R10 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x09090909;	/* R9 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x08080808;	/* R8 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x07070707;	/* R7 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x06060606;	/* R6 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x05050505;	/* R5 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x04040404;	/* R4 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x03030303;	/* R3 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x02020202;	/* R2 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x01010101;	/* R1 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pvParameters; /* R0 */
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

BaseType_t xPortStartScheduler( void )
{
    uint32_t ulAPSR;
    
    /* Only continue if the CPU is not in User mode.  The CPU must be in a
    Privileged mode for the scheduler to start. */
    __asm volatile ( "MRS %0, APSR" : "=r" ( ulAPSR ) );
    ulAPSR &= portAPSR_MODE_BITS_MASK;
    
    configASSERT( ulAPSR != portAPSR_USER_MODE );
    
    if( ulAPSR != portAPSR_USER_MODE )
    {
        /* Only continue if the binary point value is set to its lowest possible
        setting.  See the comments in vPortValidateInterruptPriority() below for
        more information. */
        configASSERT( ( portICCBPR_BINARY_POINT_REGISTER & portBINARY_POINT_BITS ) <= portMAX_BINARY_POINT_VALUE );
        
        if( ( portICCBPR_BINARY_POINT_REGISTER & portBINARY_POINT_BITS ) <= portMAX_BINARY_POINT_VALUE )
        {
#if ( configNUMBER_OF_CORES == 1 )
            {
                /* Start the timer that generates the tick ISR. */
                configSETUP_TICK_INTERRUPT();
                portCLEAR_INTERRUPT_MASK( portUNMASK_VALUE );
                /* Start the first task executing. */
                vPortRestoreTaskContext();
            }
#else
            {
                vPortStartSMPScheduler();
            }
#endif
        }
    }
    
    /* Will only get here if vTaskStartScheduler() was called with the CPU in
    a non-privileged mode or the binary point register was not set to its lowest
    possible value. */
    return 0;
}

void vPortEndScheduler( void )
{

}

static void prvTaskExitError( void )
{
    /* A function that implements a task must not exit or attempt to return to
    its caller as there is nothing to return to.  If a task wants to exit it
    should instead call vTaskDelete( NULL ).
    
    Artificially force an assert() to be triggered if configASSERT() is
    defined, then stop here so application writers can catch the error. */
    portDISABLE_INTERRUPTS();
    for( ;; );
}

void vPortSaveTopStack(StackType_t *topOfStack)
{
    TaskHandle_t tcb;
    
    tcb = xTaskGetCurrentTaskHandle();
    
    *(StackType_t**)tcb = topOfStack;
}

StackType_t *vPortRestoreTopStack()
{
    TaskHandle_t tcb;
    
    tcb = xTaskGetCurrentTaskHandle();
    
    return *(StackType_t**)tcb;
}

void vPortTaskUsesFPU( void )
{
    uint32_t ulInitialFPSCR = 0;
    
    #if ( configNUMBER_OF_CORES == 1 )
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
    
    /* Initialise the floating point status register. */
    __asm( "FMXR        FPSCR, %0" :: "r" (ulInitialFPSCR) );
}

UBaseType_t uxPortGetFPUContext( void )
{
    #if ( configNUMBER_OF_CORES == 1 )
    {
        return uxTaskHasFPUContext[0];
    }
    #else
    {
        return uxTaskHasFPUContext[portGET_CORE_ID()];
    }
    #endif
}

UBaseType_t uxPortSetFPUContext( UBaseType_t uxValue )
{
    #if ( configNUMBER_OF_CORES == 1 )
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

UBaseType_t uxGetInterruptNesting()
{
    #if ( configNUMBER_OF_CORES == 1 )
    {
        return uxInterruptNesting[0];
    }
    #else
    {
        return uxInterruptNesting[portGET_CORE_ID()];
    }
    #endif
}

UBaseType_t uxPortGetCriticalNesting( void )
{
    #if ( configNUMBER_OF_CORES == 1 )
    {
        return uxCriticalNesting[0];
    }
    #else
    {
        return uxCriticalNesting[portGET_CORE_ID()];
    }
    #endif
}

UBaseType_t uxPortSetCriticalNesting( UBaseType_t uxValue )
{
    #if ( configNUMBER_OF_CORES == 1 )
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

void portYIELD_FROM_ISR( UBaseType_t xSwitchRequired )
{											    
	if( xSwitchRequired != pdFALSE )
	{
#if ( configNUMBER_OF_CORES == 1 )
		uxYieldRequired[0] = pdTRUE;
#else
        uxYieldRequired[portGET_CORE_ID()] = pdTRUE;     
#endif
	}
    
    return;
}

UBaseType_t uxGetYieldRequired()
{
    UBaseType_t uxReturn;
        
    #if ( configNUMBER_OF_CORES == 1 )
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

void vPortTaskSwitchContext()
{
    #if ( configNUMBER_OF_CORES == 1 )
    {
        vTaskSwitchContext();
    }
    #else /* #elif ( configNUMBER_OF_CORES > 1 ) */
    {
        vTaskSwitchContext( portGET_CORE_ID() );
    }
    #endif /* end #if ( configNUMBER_OF_CORES == 1 ) */
}

UBaseType_t uxPortSetInterruptMask( void )
{
    UBaseType_t ulReturn;
    
    __asm volatile ("push {lr}");
    
	/* Interrupt in the CPU must be turned off while the ICCPMR is being
	updated. */
    portDISABLE_INTERRUPTS();
    
    ulReturn = portICCPMR_PRIORITY_MASK_REGISTER;
    portICCPMR_PRIORITY_MASK_REGISTER = ( uint32_t ) ( configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT );
    __asm volatile ("DSB		\n"
                    "ISB		\n"::: "memory");
    
    portENABLE_INTERRUPTS();
    
    __asm volatile ("pop {lr}");
    
    return ulReturn;
}

void vPortClearInterruptMask( UBaseType_t ulPreviousMask )
{
    __asm volatile ("push {lr}");
    
    /* Interrupt in the CPU must be turned off while the ICCPMR is being
	updated. */
	portDISABLE_INTERRUPTS();
    
    portICCPMR_PRIORITY_MASK_REGISTER = ulPreviousMask;
    __asm volatile ("DSB		\n"
                    "ISB		\n"::: "memory");
    
    portENABLE_INTERRUPTS();
    
    __asm volatile ("pop {lr}");
}

void vPortIRQHandler( void )
{  
    extern const FGicPs_Config FGicPs_ConfigTable[];
    static const FGicPs_VectorTableEntry *pxVectorTable = FGicPs_ConfigTable[ GIC_DEVICE_ID ].HandlerTable;
    uint32_t ulInterruptID;
    const FGicPs_VectorTableEntry *pxVectorEntry;
    uint32_t ulICCIAR; 
    UBaseType_t ulPreviousMask;
    
    #if ( configNUMBER_OF_CORES == 1 )
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
    ulICCIAR = *( ( volatile uint32_t * )portICCIAR_INTERRUPT_ACKNOWLEDGE_REGISTER_ADDRESS);
	ulInterruptID = ulICCIAR & 0x3FFUL;
	if( ulInterruptID < FGicPs_MAX_NUM_INTR_INPUTS )
	{
        /* Only interrupts priority < 18 are allowed nesting after set mask */
        ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
        
		/* Call the function installed in the array of installed handler functions. */
		pxVectorEntry = &( pxVectorTable[ ulInterruptID ] );
		pxVectorEntry->Handler( pxVectorEntry->CallBackRef );
        
        portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );
        /* All interrupts are allowed nesting now */
	}
    *( ( volatile uint32_t * )portICCEOIR_END_OF_INTERRUPT_REGISTER_ADDRESS) = ulICCIAR;
    
    #if ( configNUMBER_OF_CORES == 1 )
    {  
        uxInterruptNesting[0]--;
    }
    #else 
    {
        uxInterruptNesting[portGET_CORE_ID()]--;
    }
    #endif
}

void vPortTickHandler( void )
{        
    //UBaseType_t ulPreviousMask;

    #if ( configNUMBER_OF_CORES == 1 )
    {
        //ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
        
        configCLEAR_TICK_INTERRUPT();
            
        if( xTaskIncrementTick() != pdFALSE )
        {
            uxYieldRequired[0] = pdTRUE;
        }
        
        //portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );
    }
    #else
    {
        //ulPreviousMask = taskENTER_CRITICAL_FROM_ISR();
        
        configCLEAR_TICK_INTERRUPT();
        
        if(portGET_CORE_ID() == configTICK_CORE) 
        {        
            if( xTaskIncrementTick() != pdFALSE )
            {
                uxYieldRequired[portGET_CORE_ID()] = pdTRUE;
            }
        }
        
        //taskEXIT_CRITICAL_FROM_ISR( ulPreviousMask );
    }
    #endif /* if ( configNUMBER_OF_CORES == 1 ) */   
}

#if( configASSERT_DEFINED == 1 )

void vPortValidateInterruptPriority( void )
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
    configASSERT( portICCRPR_RUNNING_PRIORITY_REGISTER >= ( uint32_t ) ( configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT ) );
    
    /* Priority grouping:  The interrupt controller (GIC) allows the bits
    that define each interrupt's priority to be split between bits that
    define the interrupt's pre-emption priority bits and bits that define
    the interrupt's sub-priority.  For simplicity all bits must be defined
    to be pre-emption priority bits.  The following assertion will fail if
    this is not the case (if some bits represent a sub-priority).
    
    The priority grouping is configured by the GIC's binary point register
    (ICCBPR).  Writting 0 to ICCBPR will ensure it is set to its lowest
    possible value (which may be above 0). */
    configASSERT( ( portICCBPR_BINARY_POINT_REGISTER & portBINARY_POINT_BITS ) <= portMAX_BINARY_POINT_VALUE );
}

#endif /* configASSERT_DEFINED */

#if ( configNUMBER_OF_CORES > 1 )

UBaseType_t uxPortGetCoreId()
{
    BaseType_t mpidr;
    
    mfcp(CP15_MULTI_PROC_AFFINITY, mpidr); 
    
    return mpidr & 0xff;
}

static void vPortInterruptCore(UBaseType_t xOtherCoreID) 
{
    FGicPs_SoftwareIntr(&IntcInstance, YIELD_CORE_INTERRUPT_NO, 0x1<<xOtherCoreID);
}

void vPortIPIHandler( void )
{     
    //UBaseType_t ulPreviousMask;
    
    //ulPreviousMask = taskENTER_CRITICAL_FROM_ISR();
    
    uxYieldRequired[portGET_CORE_ID()] = pdTRUE;
    
    //taskEXIT_CRITICAL_FROM_ISR( ulPreviousMask );
}

void vPortYieldCore(BaseType_t xOtherCoreID)
{
	if ( xOtherCoreID < configNUMBER_OF_CORES )
	{
        vPortInterruptCore(xOtherCoreID);
    }
}

#endif /* end #if ( configNUMBER_OF_CORES > 1 ) */

/*-----------------------------------------------------------*/
static int32_t prvInitialiseInterruptController( void )
{
    int32_t status;
    FGicPs_Config *gicConfig;
    u32 value;
    
    /* Initialize the interrupt controller driver. */
    gicConfig = FGicPs_LookupConfig( GIC_DEVICE_ID );
    status = FGicPs_CfgInitialize( &IntcInstance, gicConfig, gicConfig->CpuBaseAddress);
    if(status)
        return 1;
    
    /* Connect the interrupt controller interrupt handler to the hardware
    interrupt handling logic in the ARM processor. */
    FMSH_ExceptionRegisterHandler(FMSH_EXCEPTION_ID_IRQ_INT,
                                  (FMSH_ExceptionHandler)FGicPs_InterruptHandler_IRQ,
                                  &IntcInstance); 
    
    value = *(u32*)0xf9020000;
    value |= 0xa0;
    *(u32*)0xf9020000 = value;
    
    return 0;
}

static int32_t prvInitialiseTimer( void )
{
    FTtcPs_Config *pxTimerConfig;
    
    /* Initialize triple-timer driver */ 
    pxTimerConfig = FTtcPs_LookupConfig( configTIMER_ID );
    if( pxTimerConfig == NULL ) {
        fmsh_print( "In %s: Timer Cfg initialization failed.\r\n", __func__ );
        return 1;
    }    
    FTtcPs_init(&timer, pxTimerConfig);
    
    /* Stop timer */
    FTtcPs_setTimerEnble(&timer, configTIMER_NO, FMSH_clear);
    
    FTtcPs_setTimerMode(&timer, configTIMER_NO, user_DefinedCount_mode);
    FTtcPs_setTIMERPWM(&timer, configTIMER_NO, FMSH_clear); 
    FTtcPs_TimerNLoadCount(&timer, configTIMER_NO, TTC0_FREQ / configTICK_RATE_HZ);
    FTtcPs_setTimerInterruptMask(&timer, configTIMER_NO, FMSH_clear); 
    
    /* Start Timer */
    FTtcPs_setTimerEnble(&timer, configTIMER_NO, FMSH_set); 
    
    return 0;
}

static int prvInstallTickInterrupt()
{
    const uint8_t ucLevel = 1;
    
    FGicPs_Connect(&IntcInstance, configTIMER_INTERRUPT_ID, 
                   ( FMSH_InterruptHandler )vPortTickHandler, 
                   ( void * ) 0);
#if FPAR_CPU_ID == 0
    FGicPs_InterruptMaptoCpu(&IntcInstance,0x1,configTIMER_INTERRUPT_ID);
#else 
    FGicPs_InterruptMaptoCpu(&IntcInstance,0x2,configTIMER_INTERRUPT_ID);    
#endif
    
    /* The priority must be the lowest possible. */
    FGicPs_SetPriorityTriggerType( &IntcInstance, configTIMER_INTERRUPT_ID, portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT, ucLevel );
    
    return 0;
}

static int prvEnableTickInperrupt()
{
    FGicPs_Enable(&IntcInstance, configTIMER_INTERRUPT_ID );
    
    return 0;
}

void vPortSetupTickInterrupt( void )
{   
    prvInitialiseInterruptController();
    prvInitialiseTimer();
    
    prvInstallTickInterrupt();
    
    configCLEAR_TICK_INTERRUPT();
    
    prvEnableTickInperrupt();   
}

void vPortClearTickInterrupt( void )
{
    FTtcPs_ClearTimerNInterrupt( &timer, configTIMER_NO);
}

/*-----------------------------------------------------------*/
#if (configNUMBER_OF_CORES > 1)

static uint32_t core_status[configNUMBER_OF_CORES] = { 0 };

extern void vSecondaryEntry();

void vSecondaryStart()
{
    Fmsh_ICacheEnable();
    
#if PSOC_DCACHE_ENABLE==1
    Fmsh_DCacheEnable();
#endif        

    FGicPs_CPUWriteReg(&IntcInstance, FGicPs_CPU_PRIOR_OFFSET, 0xF0U);
    FGicPs_CPUWriteReg(&IntcInstance, FGicPs_CONTROL_OFFSET, 0x03U);
    
    /* Initialize interrupt */
    FGicPs_Connect(&IntcInstance, YIELD_CORE_INTERRUPT_NO, 
                   ( FMSH_InterruptHandler )vPortIPIHandler, 
                   ( void * ) 0);

    FGicPs_SetPriorityTriggerType( &IntcInstance, YIELD_CORE_INTERRUPT_NO, portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT, 1 );
    FGicPs_Enable(&IntcInstance, YIELD_CORE_INTERRUPT_NO );
    
    configCLEAR_TICK_INTERRUPT();
    FGicPs_Enable(&IntcInstance, configTIMER_INTERRUPT_ID );
    
    portCLEAR_INTERRUPT_MASK( portUNMASK_VALUE );
    
    /* wait for secondary core ready */
    core_status[portGET_CORE_ID()] = 1;
    
    /* Start first task */
    vPortRestoreTaskContext();
}

void vStartCore(UBaseType_t xCoreID) 
{
    uint32_t value;
        
    //initialize
    FMSH_WriteReg(SLCR_REG_BASE, 0x008, 0xDF0D767BU);
    
    //hold core por reset
    value = FMSH_ReadReg(0xff5e0000, 0x23c);
    value |= (0x1 << xCoreID);
    FMSH_WriteReg(0xff5e0000, 0x23c, value);
    
    //assign core start address(RVBAR)
    if(xCoreID == 0x1U) {
        
    }
    else {
        while(1) { }
    }
    
    //release core por reset
    value = FMSH_ReadReg(0xff5e0000, 0x23c);
    value &= ~(0x1 << xCoreID);
    FMSH_WriteReg(0xff5e0000, 0x23c, value);
}

void vPortStartSMPScheduler()
{
    int i;
    
     /* Start the timer that generates the tick ISR. */
	configSETUP_TICK_INTERRUPT();
    
    /* Start secondary cores */
    for(i=0; i<configNUMBER_OF_CORES; i++) 
    {
        if(i != configTICK_CORE)
        {
            vStartCore( i );
            /* wait for secondary core ready */
            while(core_status[i] == 0);
        }
    }
    
    /* Enable interrupt */
    portCLEAR_INTERRUPT_MASK( portUNMASK_VALUE );
    /* Start the first task executing. */
    vPortRestoreTaskContext();
}

/*-----------------------------------------------------------
 * Critical section locks
 *----------------------------------------------------------*/
uint32_t Get_32(volatile uint32_t* x);
void Set_32(volatile uint32_t* x, uint32_t value);

/* Which core owns the lock */
volatile uint32_t ucOwnedByCore[ configNUMBER_OF_CORES ];
/* Lock count a core owns */
volatile uint32_t ucRecursionCountByLock[ portRTOS_LOCK_COUNT ];

/* Index 0 is used for ISR lock and Index 1 is used for task lock */
uint32_t GateWord[ portRTOS_LOCK_COUNT ];
extern int32_t SMP_TryLock(uint32_t* gateWord);
extern void SMP_Unlock(uint32_t* gateWord);

/* Read 32b value shared between cores */
uint32_t Get_32(volatile uint32_t* x)
{
    __asm volatile("dsb sy");
    return *x;
}

/* Write 32b value shared between cores */
void Set_32(volatile uint32_t* x, uint32_t value)
{
    *x = value;
    __asm  volatile("dsb sy");
}

void vPortRecursiveLock(uint32_t ulLockNum, BaseType_t uxAcquire)
{
    UBaseType_t uxCoreId = portGET_CORE_ID();
    uint32_t ulLockBit = 1u << ulLockNum;

    /* Lock acquire */
    if (uxAcquire) {

        /* Check if spinlock is available */
        /* If spinlock is not available check if the core owns the lock */
        /* If the core owns the lock wait increment the lock count by the core */
        /* If core does not own the lock wait for the spinlock */
        if( SMP_TryLock( &GateWord[ulLockNum] ) != 0)
        {
            /* Check if the core owns the spinlock */
            if( Get_32(&ucOwnedByCore[uxCoreId]) & ulLockBit )
            {
                configASSERT( Get_32(&ucRecursionCountByLock[ulLockNum]) != 255u);
                Set_32(&ucRecursionCountByLock[ulLockNum], (Get_32(&ucRecursionCountByLock[ulLockNum])+1));
                return;
            }

            /* Preload the gate word into the cache */
            uint32_t dummy = GateWord[ulLockNum];
            dummy++;

            /* Wait for spinlock */
            while( SMP_TryLock(&GateWord[ulLockNum]) != 0);
        }

         /* Add barrier to ensure lock is taken before we proceed */
        __asm volatile("dmb sy"::: "memory");

        /* Assert the lock count is 0 when the spinlock is free and is acquired */
        configASSERT(Get_32(&ucRecursionCountByLock[ulLockNum]) == 0);

        /* Set lock count as 1 */
        Set_32(&ucRecursionCountByLock[ulLockNum], 1);
        /* Set ucOwnedByCore */
        Set_32(&ucOwnedByCore[uxCoreId], (Get_32(&ucOwnedByCore[uxCoreId]) | ulLockBit));
    }
    /* Lock release */
    else
    {
        /* Assert the lock is not free already */
        configASSERT( (Get_32(&ucOwnedByCore[uxCoreId]) & ulLockBit) != 0 );
        configASSERT( Get_32(&ucRecursionCountByLock[ulLockNum]) != 0 );

        /* Reduce ucRecursionCountByLock by 1 */
        Set_32(&ucRecursionCountByLock[ulLockNum], (Get_32(&ucRecursionCountByLock[ulLockNum]) - 1) );

        if( !Get_32(&ucRecursionCountByLock[ulLockNum]) )
        {
            Set_32(&ucOwnedByCore[uxCoreId], (Get_32(&ucOwnedByCore[uxCoreId]) & ~ulLockBit));
            SMP_Unlock(&GateWord[ulLockNum]);
            /* Add barrier to ensure lock is taken before we proceed */
            __asm volatile ("dmb sy"::: "memory");
        }
    }
}

#endif //if (configNUMBER_OF_CORES > 1)

/*-----------------------------------------------------------*/

/* This version of vApplicationAssert() is declared as a weak symbol to allow it
to be overridden by a version implemented within the application that is using
this BSP. */
void vApplicationAssert( const char *pcFileName, uint32_t ulLine )
{
    volatile uint32_t ul = 0;
    volatile const char *pcLocalFileName = pcFileName; /* To prevent pcFileName being optimized away. */
    volatile uint32_t ulLocalLine = ulLine; /* To prevent ulLine being optimized away. */
    
    /* Prevent compile warnings about the following two variables being set but
    not referenced.  They are intended for viewing in the debugger. */
    ( void ) pcLocalFileName;
    ( void ) ulLocalLine;
    
    fmsh_print( "Assert failed in file %s, line %lu\r\n", pcLocalFileName, ulLocalLine );
    
    /* If this function is entered then a call to configASSERT() failed in the
    FreeRTOS code because of a fatal error.  The pcFileName and ulLine
    parameters hold the file name and line number in that file of the assert
    that failed.  Additionally, if using the debugger, the function call stack
    can be viewed to find which line failed its configASSERT() test.  Finally,
    the debugger can be used to set ul to a non-zero value, then step out of
    this function to find where the assert function was entered. */
    taskENTER_CRITICAL();
    {
        while( ul == 0 )
        {
            __asm volatile( "NOP" );
        }
    }
    taskEXIT_CRITICAL();
}

/* This default tick hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationTickHook( void )
{
    
}

/* This default idle hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationIdleHook( void )
{
    volatile size_t xFreeStackSpace;
    
    /* This function is called on each cycle of the idle task.  In this case it
    does nothing useful, other than report the amount of FreeRTOS heap that
    remains unallocated. */
    xFreeStackSpace = xPortGetFreeHeapSize();
    
    if( xFreeStackSpace > 100 )
    {
        /* By now, the kernel has allocated everything it is going to, so
        if there is a lot of heap remaining unallocated then
        the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
        reduced accordingly. */
    }
}

/* This default malloc failed hook does nothing and is declared as a weak symbol
to allow the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationMallocFailedHook( void )
{
    fmsh_print( "vApplicationMallocFailedHook() called\n" );
}

/* This default stack overflow hook will stop the application for executing.  It
is declared as a weak symbol to allow the application writer to override this
default by providing their own implementation in the application code. */
extern void backtrace_overflow(TaskHandle_t xTask);
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    /* Attempt to prevent the handle and name of the task that overflowed its stack
    from being optimised away because they are not used. */
    volatile TaskHandle_t xOverflowingTaskHandle = xTask;
    volatile char *pcOverflowingTaskName = pcTaskName;
    
    //( void ) xOverflowingTaskHandle;
    //( void ) pcOverflowingTaskName;
    portDISABLE_INTERRUPTS();
    fmsh_print( "R5 HALT: Task %s overflowed its stack.", pcOverflowingTaskName );
    backtrace_overflow(xTask);
    for( ;; );
}
