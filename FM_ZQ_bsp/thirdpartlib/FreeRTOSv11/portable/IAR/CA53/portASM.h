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

    EXTERN vPortSaveTopStack
    EXTERN vPortRestoreTopStack
    EXTERN uxPortGetCriticalNesting
    EXTERN uxPortSetCriticalNesting
    EXTERN uxPortGetFPUContext
    EXTERN uxPortSetFPUContext
        
portSAVE_CONTEXT macro label
	/* Switch to use the EL0 stack pointer. */
	MSR 	SPSEL, #0
    
	/* Save the entire context. */
	STP 	X0, X1, [SP, #-0x10]!
	STP 	X2, X3, [SP, #-0x10]!
	STP 	X4, X5, [SP, #-0x10]!
	STP 	X6, X7, [SP, #-0x10]!
	STP 	X8, X9, [SP, #-0x10]!
	STP 	X10, X11, [SP, #-0x10]!
	STP 	X12, X13, [SP, #-0x10]!
	STP 	X14, X15, [SP, #-0x10]!
	STP 	X16, X17, [SP, #-0x10]!
	STP 	X18, X19, [SP, #-0x10]!
	STP 	X20, X21, [SP, #-0x10]!
	STP 	X22, X23, [SP, #-0x10]!
	STP 	X24, X25, [SP, #-0x10]!
	STP 	X26, X27, [SP, #-0x10]!
	STP 	X28, X29, [SP, #-0x10]!
	STP 	X30, XZR, [SP, #-0x10]!
    
	/* Save the SPSR. */
#if defined( GUEST )
	MRS		X3, SPSR_EL1
	MRS		X2, ELR_EL1
#else
	MRS		X3, SPSR_EL3
	MRS		X2, ELR_EL3
#endif
	STP 	X2, X3, [SP, #-0x10]!
    
    /* X3 save CriticalNestingContext */
    BL      uxPortGetCriticalNesting
    MOV     X3, X0
    
    /* X2 save FPUContext */
    BL      uxPortGetFPUContext
    MOV     X2, X0
    
    /* Save the FPU context, if any (32 128-bit registers). */
    CMP     X2, #0
    B.EQ    label
    
	/* Save the FPU context, if any (32 128-bit registers). */
	STP		Q0, Q1, [SP,#-0x20]!
	STP		Q2, Q3, [SP,#-0x20]!
	STP		Q4, Q5, [SP,#-0x20]!
	STP		Q6, Q7, [SP,#-0x20]!
	STP		Q8, Q9, [SP,#-0x20]!
	STP		Q10, Q11, [SP,#-0x20]!
	STP		Q12, Q13, [SP,#-0x20]!
	STP		Q14, Q15, [SP,#-0x20]!
	STP		Q16, Q17, [SP,#-0x20]!
	STP		Q18, Q19, [SP,#-0x20]!
	STP		Q20, Q21, [SP,#-0x20]!
	STP		Q22, Q23, [SP,#-0x20]!
	STP		Q24, Q25, [SP,#-0x20]!
	STP		Q26, Q27, [SP,#-0x20]!
	STP		Q28, Q29, [SP,#-0x20]!
	STP		Q30, Q31, [SP,#-0x20]!

label:
	/* Store the critical nesting count and FPU context indicator. */
	STP 	X2, X3, [SP, #-0x10]!
    
    /* Move SP into X0 for saving. */
    MOV		X0, SP
    /* Switch to use the ELx stack pointer. */
    MSR 	SPSEL, #1
    BL      vPortSaveTopStack

	endm

portRESTORE_CONTEXT macro label

	/* Set the SP to point to the stack of the task being restored. */
    BL      vPortRestoreTopStack
    /* Switch to use the EL0 stack pointer. */
    MSR 	SPSEL, #0
	MOV		SP, X0

    /* Critical nesting and FPU context. */
    LDP 	X2, X3, [SP], #0x10  
    
    /* Restore the task's critical nesting count. */
    MOV     X0, X3
    BL      uxPortSetCriticalNesting
    
    /* Restore the FPU context indicator. */
    MOV     X0, X2
    BL      uxPortSetFPUContext
    CMP     X0, #0
    B.EQ    label
    
	/* Restore the FPU context, if any. */
	LDP		Q30, Q31, [SP], #0x20
	LDP		Q28, Q29, [SP], #0x20
	LDP		Q26, Q27, [SP], #0x20
	LDP		Q24, Q25, [SP], #0x20
	LDP		Q22, Q23, [SP], #0x20
	LDP		Q20, Q21, [SP], #0x20
	LDP		Q18, Q19, [SP], #0x20
	LDP		Q16, Q17, [SP], #0x20
	LDP		Q14, Q15, [SP], #0x20
	LDP		Q12, Q13, [SP], #0x20
	LDP		Q10, Q11, [SP], #0x20
	LDP		Q8, Q9, [SP], #0x20
	LDP		Q6, Q7, [SP], #0x20
	LDP		Q4, Q5, [SP], #0x20
	LDP		Q2, Q3, [SP], #0x20
	LDP		Q0, Q1, [SP], #0x20

label:
    /* SPSR and ELR. */
	LDP 	X2, X3, [SP], #0x10  
#if defined( GUEST )
	MSR		SPSR_EL1, X3
	MSR		ELR_EL1, X2
#else
	MSR		SPSR_EL3, X3
	MSR		ELR_EL3, X2
#endif

	LDP 	X30, XZR, [SP], #0x10
	LDP 	X28, X29, [SP], #0x10
	LDP 	X26, X27, [SP], #0x10
	LDP 	X24, X25, [SP], #0x10
	LDP 	X22, X23, [SP], #0x10
	LDP 	X20, X21, [SP], #0x10
	LDP 	X18, X19, [SP], #0x10
	LDP 	X16, X17, [SP], #0x10
	LDP 	X14, X15, [SP], #0x10
	LDP 	X12, X13, [SP], #0x10
	LDP 	X10, X11, [SP], #0x10
	LDP 	X8, X9, [SP], #0x10
	LDP 	X6, X7, [SP], #0x10
	LDP 	X4, X5, [SP], #0x10
	LDP 	X2, X3, [SP], #0x10
	LDP 	X0, X1, [SP], #0x10

	/* Switch to use the ELx stack pointer.  _RB_ Might not be required. */
	MSR 	SPSEL, #1

	ERET

    endm

portRESTORE_CONTEXT_abort macro label

	/* Set the SP to point to the stack of the task being restored. */
    BL      vPortRestoreTopStack
    /* Switch to use the EL0 stack pointer. */
    MSR 	SPSEL, #0
	MOV		SP, X0

    /* Critical nesting and FPU context. */
    LDP 	X2, X3, [SP], #0x10  
    
    /* Restore the task's critical nesting count. */
    MOV     X0, X3
    BL      uxPortSetCriticalNesting
    
    /* Restore the FPU context indicator. */
    MOV     X0, X2
    BL      uxPortSetFPUContext
    CMP     X0, #0
    B.EQ    label
    
	/* Restore the FPU context, if any. */
	LDP		Q30, Q31, [SP], #0x20
	LDP		Q28, Q29, [SP], #0x20
	LDP		Q26, Q27, [SP], #0x20
	LDP		Q24, Q25, [SP], #0x20
	LDP		Q22, Q23, [SP], #0x20
	LDP		Q20, Q21, [SP], #0x20
	LDP		Q18, Q19, [SP], #0x20
	LDP		Q16, Q17, [SP], #0x20
	LDP		Q14, Q15, [SP], #0x20
	LDP		Q12, Q13, [SP], #0x20
	LDP		Q10, Q11, [SP], #0x20
	LDP		Q8, Q9, [SP], #0x20
	LDP		Q6, Q7, [SP], #0x20
	LDP		Q4, Q5, [SP], #0x20
	LDP		Q2, Q3, [SP], #0x20
	LDP		Q0, Q1, [SP], #0x20

label:
    /* SPSR and ELR. */
	LDP 	X2, X3, [SP], #0x10  
#if defined( GUEST )
	MSR		SPSR_EL1, X3
	MSR		ELR_EL1, X2
#else
	MSR		SPSR_EL3, X3
	MSR		ELR_EL3, X2
#endif

	LDP 	X30, XZR, [SP], #0x10
	LDP 	X28, X29, [SP], #0x10
	LDP 	X26, X27, [SP], #0x10
	LDP 	X24, X25, [SP], #0x10
	LDP 	X22, X23, [SP], #0x10
	LDP 	X20, X21, [SP], #0x10
	LDP 	X18, X19, [SP], #0x10
	LDP 	X16, X17, [SP], #0x10
	LDP 	X14, X15, [SP], #0x10
	LDP 	X12, X13, [SP], #0x10
	LDP 	X10, X11, [SP], #0x10
	LDP 	X8, X9, [SP], #0x10
	LDP 	X6, X7, [SP], #0x10
	LDP 	X4, X5, [SP], #0x10
	LDP 	X2, X3, [SP], #0x10
	LDP 	X0, X1, [SP], #0x10

	/* Switch to use the ELx stack pointer.  _RB_ Might not be required. */
	MSR 	SPSEL, #1

    endm