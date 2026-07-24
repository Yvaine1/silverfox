/******************************************************************************
*
* Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
* Copyright (C) 2014 - 2020 Xilinx, Inc. All rights reserved.
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
* http://www.FreeRTOS.org
* http://aws.amazon.com/freertos
*
* 1 tab == 4 spaces!
*
******************************************************************************/
/*****************************************************************************/
/**
* @file asm_vectors.s
*
* This file contains the initial vector table for the Cortex A53 processor
* Currently NEON registers are not saved on stack if interrupt is taken.
* It will be implemented.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- ---------------------------------------------------
* 5.00	pkp	5/21/14 Initial version
* </pre>
*
* @note
*
* None.
*
******************************************************************************/
#define GICv2

    section .freertos_start:CODE:ROOT(3)

    public  _vector_table

    extern  Start64
    
    extern  FreeRTOS_SWI_Handler
    extern  FreeRTOS_IRQ_Handler
    extern  FreeRTOS_FIQ_Handler

_vector_table:
	b	Start64

/******************************************************************************
* Vector table to use when FreeRTOS is running.
*****************************************************************************/

_freertos_vector_table:
//.org (FREERTOS_VBAR)
    section .freertos_t0sync:CODE:ROOT(7)
    b	FreeRTOS_SWI_Handler

//.org (FREERTOS_VBAR + 0x80)
    section .freertos_t0irq:CODE:ROOT(7)
    b	FreeRTOS_IRQ_Handler

//.org (FREERTOS_VBAR + 0x100)
    section .freertos_t0fiq:CODE:ROOT(7)
#if defined(GICv3)
    b	FreeRTOS_IRQ_Handler
#else
    b	.
#endif

//.org (FREERTOS_VBAR + 0x180)
    section .freertos_t0serr:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x200)
    section .freertos_t1sync:CODE:ROOT(7)
    b	FreeRTOS_SWI_Handler

//.org (FREERTOS_VBAR + 0x280)
    section .freertos_t1irq:CODE:ROOT(7)
    b	FreeRTOS_IRQ_Handler

//.org (FREERTOS_VBAR + 0x300)
    section .freertos_t1fiq:CODE:ROOT(7)
    b	FreeRTOS_IRQ_Handler

//.org (FREERTOS_VBAR + 0x380)
    section .freertos_t1serr:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x400)
    section .freertos_t2sync:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x480)
    section .freertos_t2irq:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x500)
    section .freertos_t2fiq:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x580)
    section .freertos_t2serr:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x600)
    section .freertos_t3sync:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x680)
    section .freertos_t3irq:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x700)
    section .freertos_t3fiq:CODE:ROOT(7)
    b	.

//.org (FREERTOS_VBAR + 0x780)
    section .freertos_t3serr:CODE:ROOT(7)
    b	.
    
    END