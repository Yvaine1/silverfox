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

    EXTERN      uxPortGetFPUContext
    EXTERN      uxPortSetFPUContext
    EXTERN      uxPortGetCriticalNesting
    EXTERN      uxPortSetCriticalNesting  
    EXTERN      vPortSaveTopStack
    EXTERN      vPortRestoreTopStack

portSAVE_CONTEXT MACRO

    /* Save the LR and SPSR onto the system mode stack before switching to
    system mode to save the remaining system mode registers. */
    SRSDB       sp!, #SYS_MODE
    CPS         #SYS_MODE  
    PUSH        {R0-R12, R14}

    /* Push the critical nesting count. */
    LDR         R1, =uxPortGetCriticalNesting
    BLX         R1
    PUSH        {R0}

    /* Does the task have a floating point context that needs saving?  If
    uxTaskHasFPUContext is 0 then no. */
    LDR         R1, =uxPortGetFPUContext
    BLX         R1
    CMP         R0, #0

    /* Save the floating point context, if any. */
    VMRSNE      R1,  FPSCR
    VPUSHNE     {D0-D15}
#if configFPU_D32 == 1
    VPUSHNE     {D16-D31}
#endif /* configFPU_D32 */
    PUSHNE      {R1}

    /* Save uxTaskHasFPUContext */
    PUSH        {R0}

    MOV         R0, SP
    LDR	        R1, =vPortSaveTopStack
    BLX         R1

    ENDM

; /**********************************************************************/

portRESTORE_CONTEXT MACRO

    /* Set the SP to point to the stack of the task being restored. */
    LDR	        R1, =vPortRestoreTopStack
    BLX         R1
    MOV	        SP, R0

    /* Is there a floating point context to restore?  If the restored
    ulPortTaskHasFPUContext is zero then no. */
    POP	        {R0}
    LDR	        R1, =uxPortSetFPUContext
    BLX         R1
    CMP	        R0, #0

    /* Restore the floating point context, if any. */
    POPNE       {R1}
#if configFPU_D32 == 1
    VPOPNE      {D16-D31}
#endif /* configFPU_D32 */
    VPOPNE      {D0-D15}
    VMSRNE      FPSCR, R1

    /* Restore the critical section nesting depth. */
    POP	        {R0}
    LDR         R1, =uxPortSetCriticalNesting
    BLX         R1

    /* Restore all system mode registers other than the SP (which is already
    being used). */
    POP	        {R0-R12, R14}

    /* Return to the task code, loading CPSR on the way. */
    RFEIA       sp!

    ENDM
