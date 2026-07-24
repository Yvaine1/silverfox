;/*
; * FreeRTOS Kernel V10.5.1
; * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
; *
; * SPDX-License-Identifier: MIT
; *
; * Permission is hereby granted, free of charge, to any person obtaining a copy of
; * this software and associated documentation files (the "Software"), to deal in
; * the Software without restriction, including without limitation the rights to
; * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
; * the Software, and to permit persons to whom the Software is furnished to do so,
; * subject to the following conditions:
; *
; * The above copyright notice and this permission notice shall be included in all
; * copies or substantial portions of the Software.
; *
; * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
; * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
; * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
; * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
; * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
; *
; * https://www.FreeRTOS.org
; * https://github.com/FreeRTOS
; *
; */

    INCLUDE     portmacro.h
    INCLUDE     portASM.h
    
    EXTERN      vPortIRQHandler
    EXTERN      uxGetInterruptNesting
    EXTERN      uxGetYieldRequired
    EXTERN      vPortTaskSwitchContext

    PUBLIC      FreeRTOS_SWI_Handler
    PUBLIC      FreeRTOS_IRQ_Handler
    PUBLIC      FreeRTOS_TaskSwitch
    PUBLIC      vPortRestoreTaskContext

SYS_MODE        EQU     0x1f
SVC_MODE        EQU     0x13
IRQ_MODE        EQU     0x12

    SECTION .text:CODE:ROOT(2)
    ARM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SVC handler is used to yield a task.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FreeRTOS_SWI_Handler

	PRESERVE8

	; Save the context of the current task and select a new task to run.
	portSAVE_CONTEXT
	LDR R0, =vPortTaskSwitchContext
	BLX	R0
	portRESTORE_CONTEXT

FreeRTOS_TaskSwitch

	PRESERVE8

	; Save the context of the current task and select a new task to run.
	portSAVE_CONTEXT
	LDR R0, =vPortTaskSwitchContext
	BLX	R0
	portRESTORE_CONTEXT
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; vPortRestoreTaskContext is used to start the scheduler.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
vPortRestoreTaskContext
	; Switch to system mode
	CPS		#SYS_MODE
	portRESTORE_CONTEXT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PL390 GIC interrupt handler
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FreeRTOS_IRQ_Handler

	; Return to the interrupted instruction.
	SUB		lr, lr, #4

	; Push the return address and SPSR
	PUSH	{lr}
	MRS		lr, SPSR
	PUSH	{lr}

	; Change to supervisor mode to allow reentry.
	CPS		#SVC_MODE

	; Push used registers.
	PUSH	{r0-r7, r12}

	; Call the interrupt handler.
	LDR		r1, =vPortIRQHandler
	BLX		r1

    ; A context switch is never performed if the nesting count is not 0
	LDR		r1, =uxGetInterruptNesting
        BLX             r1
        CMP             r0, #0
        BNE             exit_without_switch

	; Did the interrupt request a context switch?  r1 holds the address of
	; ulPortYieldRequired and r0 the value of ulPortYieldRequired for future
	; use.
	LDR		r1, =uxGetYieldRequired
	BLX		r1
	CMP		r0, #0
	BEQ		exit_without_switch

switch_before_exit
	; Restore used registers, LR-irq and SPSR before saving the context
	; to the task stack.
	POP		{r0-r7, r12}
	CPS		#IRQ_MODE
	POP		{LR}
	MSR		SPSR_cxsf, LR
	POP		{LR}
    
	portSAVE_CONTEXT

	; Call the function that selects the new task to execute.
	; vTaskSwitchContext() if vTaskSwitchContext() uses LDRD or STRD
	; instructions, or 8 byte aligned stack allocated data.  LR does not need
	; saving as a new LR will be loaded by portRESTORE_CONTEXT anyway.
	LDR		r0, =vPortTaskSwitchContext
	BLX		r0

	; Restore the context of, and branch to, the task selected to execute next.
	portRESTORE_CONTEXT

exit_without_switch
	; No context switch.  Restore used registers, LR_irq and SPSR before
	; returning.
	POP		{r0-r7, r12}
	CPS		#IRQ_MODE
	POP		{LR}
	MSR		SPSR_cxsf, LR
	POP		{LR}
	MOVS	PC, LR

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SMP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    section .text:CODE:NOROOT(3)
    
    public SMP_TryLock
    
SMP_TryLock:
TryLock:
    LDREX   R1, [R0]
    CMP     R1, #1
    BEQ    LockAlready
    
    MOV     R2, #1
    STREX   R1, R2, [R0]
    CMP     R1, #0
    BNE    TryLock
    
LockAlready:
    MOV     R0, R1
    DMB     SY


    section .text:CODE:NOROOT(3)
    
    public SMP_Unlock
    
SMP_Unlock:
    DMB     SY

    MOV     R1, #0
    STR     R1, [R0]

    DSB     SY
    SEV         /* let everyone know */
    
	END
