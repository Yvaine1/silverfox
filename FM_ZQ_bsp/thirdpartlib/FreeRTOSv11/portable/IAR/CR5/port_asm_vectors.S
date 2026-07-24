/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  vector.S
*
* This file contains 
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 2.00  hzq 22/11/18    first release
*
*</pre>
******************************************************************************/

    MODULE  ?freertos_vectors

    section .freertos_vectors:CODE:NOROOT(2)
    
    ARM

    public FreeRTOS_vectors
    extern Reset_Handler
    extern FreeRTOS_IRQ_Handler
    extern FreeRTOS_SWI_Handler
    extern g_RegCtx
    
//----------------------------------------------------------------
// Exception Vector Table
//----------------------------------------------------------------
// Note: LDR PC instructions are used here, though branch (B) instructions
// could also be used, unless the exception handlers are >32MB away.
FreeRTOS_vectors:
        LDR PC, Reset_Addr
        LDR PC, Undefined_Addr
        LDR PC, SVC_Addr
        LDR PC, Prefetch_Addr
        LDR PC, Abort_Addr
        B .                              // Reserved vector
        LDR PC, IRQ_Addr
        LDR PC, FIQ_Addr

Reset_Addr:         DCD     Reset_Handler
Undefined_Addr:     DCD     Undefined_Handler
SVC_Addr:           DCD     FreeRTOS_SWI_Handler
Prefetch_Addr:      DCD     Prefetch_Handler
Abort_Addr:         DCD     Abort_Handler
IRQ_Addr:           DCD     FreeRTOS_IRQ_Handler
FIQ_Addr:           DCD     FIQ_Handler

//----------------------------------------------------------------
// Exception Handlers
//----------------------------------------------------------------   
    section .text:CODE:NOROOT(2)
    
    extern FIQInterrupt
    extern DataAbortInterrupt
    extern PrefetchAbortInterrupt
    extern UndefinedException
    
    extern DataAbortAddr
    extern PrefetchAbortAddr
    extern UndefinedExceptionAddr

Undefined_Handler:
        STMDB   sp!, {r4}
        LDR     r4, =g_RegCtx

        STR     r0, [r4, #0]           ; r[0]
        STR     r1, [r4, #4]           ; r[1]
        STR     r2, [r4, #8]           ; r[2]
        STR     r3, [r4, #12]          ; r[3]
        STR     r5, [r4, #20]          ; r[5]
        STR     r6, [r4, #24]          ; r[6]
        STR     r7, [r4, #28]          ; r[7]
        STR     r8, [r4, #32]          ; r[8]
        STR     r9, [r4, #36]          ; r[9]
        STR     r10, [r4, #40]         ; r[10]
        STR     r11, [r4, #44]         ; r[11]
        STR     r12, [r4, #48]         ; r[12]

        LDR     r0, [sp]               ; r0 = 原始 R4
        STR     r0, [r4, #16]          ; 存 r[4]
        ADD     sp, sp, #4             ; 释放栈空间

        STR     lr, [r4, #68]

        MRS     r5, SPSR               ; r5 = SPSR
        STR     r5, [r4, #60]          ; 存 spsr 字段
        MRS     r2, CPSR
        STR     r2, [r4, #64]          ; 存 cpsr 字段

        MOV     r11, #0
        B       Save_Origin_SP_LR

Prefetch_Handler:
        STMDB   sp!, {r4}
        LDR     r4, =g_RegCtx

        STR     r0, [r4, #0]           ; r[0]
        STR     r1, [r4, #4]           ; r[1]
        STR     r2, [r4, #8]           ; r[2]
        STR     r3, [r4, #12]          ; r[3]
        STR     r5, [r4, #20]          ; r[5]
        STR     r6, [r4, #24]          ; r[6]
        STR     r7, [r4, #28]          ; r[7]
        STR     r8, [r4, #32]          ; r[8]
        STR     r9, [r4, #36]          ; r[9]
        STR     r10, [r4, #40]         ; r[10]
        STR     r11, [r4, #44]         ; r[11]
        STR     r12, [r4, #48]         ; r[12]

        LDR     r0, [sp]               ; r0 = 原始 R4
        STR     r0, [r4, #16]          ; 存 r[4]
        ADD     sp, sp, #4             ; 释放栈空间

        STR     lr, [r4, #68]

        MRS     r5, SPSR               ; r5 = SPSR
        STR     r5, [r4, #60]          ; 存 spsr 字段
        MRS     r2, CPSR
        STR     r2, [r4, #64]          ; 存 cpsr 字段

        MOV     r11, #1
        B       Save_Origin_SP_LR 
        
Abort_Handler:
        STMDB   sp!, {r4}
        LDR     r4, =g_RegCtx

        STR     r0, [r4, #0]           ; r[0]
        STR     r1, [r4, #4]           ; r[1]
        STR     r2, [r4, #8]           ; r[2]
        STR     r3, [r4, #12]          ; r[3]
        STR     r5, [r4, #20]          ; r[5]
        STR     r6, [r4, #24]          ; r[6]
        STR     r7, [r4, #28]          ; r[7]
        STR     r8, [r4, #32]          ; r[8]
        STR     r9, [r4, #36]          ; r[9]
        STR     r10, [r4, #40]         ; r[10]
        STR     r11, [r4, #44]         ; r[11]
        STR     r12, [r4, #48]         ; r[12]

        LDR     r0, [sp]               ; r0 = 原始 R4
        STR     r0, [r4, #16]          ; 存 r[4]
        ADD     sp, sp, #4             ; 释放栈空间

        STR     lr, [r4, #68]

        MRS     r5, SPSR               ; r5 = SPSR
        STR     r5, [r4, #60]          ; 存 spsr 字段
        MRS     r2, CPSR
        STR     r2, [r4, #64]          ; 存 cpsr 字段

        MOV     r11, #2
        B       Save_Origin_SP_LR

Save_Origin_SP_LR:
        AND     r1, r5, #0x1F
        BIC     r3, r2, #0x1F          ; 清除模式位
        CMP     r1, #0x10              ; USR模式
        CMPNE   r1, #0x1F              ; 非USR模式
        ORREQ   r3, r3, #0x1F
        ORRNE   r3, r3, r1             ; 置为原始模式
        MSR     CPSR_c, r3
        ISB
        STR     lr, [r4, #52]          ; 存原始 LR（此时 lr = R14_sys）
        STR     sp, [r4, #56]          ; 存原始 SP（此时 sp = R13_sys）
        MSR     CPSR_c, r2             ; 切回异常模式
        ISB
        B       Backtrace_Entry

Backtrace_Entry:
        MOV     r0, r4
        CMP     r11, #0
        BEQ     UndefinedException
        CMP     r11, #1
        BEQ     PrefetchAbortInterrupt
        CMP     r11, #2
        BEQ     DataAbortInterrupt

FIQ_Handler:
        STMDB	sp!, {r0-r3,r12,lr}		// state save from compiled code
        BL	FIQInterrupt			// FIQ vector
        LDMIA	sp!, {r0-r3,r12,lr}		// state restore from compiled code
        SUBS	pc, lr, #4			    // adjust return

        END
