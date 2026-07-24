/******************************************************************************
 *
 * Copyright (C) 2009 - 2015 FMSH, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a FMSH device, or
 * (b) that interact with a FMSH device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file FMSH_exception.h
 *
 * This header file contains specific exception related APIs.
 * For exception related functions that can be used across all FMSH supported
 * processors, please use FMSH_exception.h.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who      Date     Changes
 * ----- -------- -------- -----------------------------------------------
 * 1.00a ecm/sdm  11/04/09 First release
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef _FMSH_EXCEPTION_H_ /* prevent circular inclusions */
#define _FMSH_EXCEPTION_H_ /* by using protection macros */

/***************************** Include Files ********************************/

#include "fmsh_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions ****************************/

#define FMSH_EXCEPTION_ID_FIRST              0U
#define FMSH_EXCEPTION_ID_RESET              0U
#define FMSH_EXCEPTION_ID_UNDEFINED_INT      1U
#define FMSH_EXCEPTION_ID_SWI_INT            2U
#define FMSH_EXCEPTION_ID_PREFETCH_ABORT_INT 3U
#define FMSH_EXCEPTION_ID_DATA_ABORT_INT     4U
#define FMSH_EXCEPTION_ID_IRQ_INT            5U
#define FMSH_EXCEPTION_ID_FIQ_INT            6U
#define FMSH_EXCEPTION_ID_LAST               6U

/*
 * FMSH_EXCEPTION_ID_INT is defined for all FMSH processors.
 */
#define FMSH_EXCEPTION_ID_INT                FMSH_EXCEPTION_ID_IRQ_INT

/**************************** Type Definitions ******************************/

/**
 * This typedef is the exception handler function.
 */
typedef void (*FMSH_ExceptionHandler)(void *data);
typedef void (*FMSH_InterruptHandler)(void *data);

/***************** Macros (Inline Functions) Definitions ********************/

/****************************************************************************/
/**
 * Enable Exceptions.
 *
 * @param	Mask for exceptions to be enabled.
 *
 * @return	None.
 *
 * @note		If bit is 0, exception is enabled.
 *		C-Style signature: void FMSH_ExceptionEnableMask(Mask)
 *
 ******************************************************************************/

/****************************************************************************/
/**
 * Enable the IRQ exception.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/

/****************************************************************************/
/**
 * Disable Exceptions.
 *
 * @param	Mask for exceptions to be enabled.
 *
 * @return	None.
 *
 * @note		If bit is 1, exception is disabled.
 *		C-Style signature: FMSH_ExceptionDisableMask(Mask)
 *
 ******************************************************************************/
#define FMSH_ExceptionEnable()      asm("cpsie i")
#define FMSH_FastInterruptEnable()  asm("cpsie f")

/****************************************************************************/
/**
 * Disable the IRQ exception.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
#define FMSH_ExceptionDisable()     asm("cpsid i")
#define FMSH_FastInterruptDisable() asm("cpsid f")

/****************************************************************************/
/**
 * Enable nested interrupts by clearing the I and F bits it CPSR
 *
 * @return   None.
 *
 * @note     This macro is supposed to be used from interrupt handlers. In the
 *			interrupt handler the interrupts are disabled by default (I and F
 *			are 1). To allow nesting of interrupts, this macro should be
 *			used. It clears the I and F bits by changing the ARM mode to
 *			system mode. Once these bits are cleared and provided the
 *			preemption of interrupt conditions are met in the GIC, nesting of
 *			interrupts will start happening.
 *			Caution: This macro must be used with caution. Before calling this
 *			macro, the user must ensure that the source of the current IRQ
 *			is appropriately cleared. Otherwise, as soon as we clear the I and
 *			F bits, there can be an infinite loop of interrupts with an
 *			eventual crash (all the stack space getting consumed).
 ******************************************************************************/
#define FMSH_EnableNestedInterrupts()              \
    __asm__ __volatile__("mrs     lr, spsr");      \
    __asm__ __volatile__("stmfd   sp!, {lr}");     \
    __asm__ __volatile__("msr     cpsr_c, #0x1F"); \
    __asm__ __volatile__("stmfd   sp!, {lr}");

/****************************************************************************/
/**
 * Disable the nested interrupts by setting the I and F bits.
 *
 * @return   None.
 *
 * @note     This macro is meant to be called in the interrupt service routines.
 *			This macro cannot be used independently. It can only be used when
 *			nesting of interrupts have been enabled by using the macro
 *			FMSH_EnableNestedInterrupts(). In a typical flow, the user first
 *			calls the FMSH_EnableNestedInterrupts in the ISR at the appropriate
 *			point. The user then must call this macro before exiting the
 *interrupt service routine. This macro puts the ARM back in IRQ/FIQ mode and
 *			hence sets back the I and F bits.
 ******************************************************************************/
#define FMSH_DisableNestedInterrupts()             \
    __asm__ __volatile__("ldmfd   sp!, {lr}");     \
    __asm__ __volatile__("msr     cpsr_c, #0x92"); \
    __asm__ __volatile__("ldmfd   sp!, {lr}");     \
    __asm__ __volatile__("msr     spsr_cxsf, lr");

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

extern void FMSH_ExceptionRegisterHandler(u32 Exception_id,
                                          FMSH_ExceptionHandler Handler,
                                          void *Data);
extern void FMSH_ExceptionRemoveHandler(u32 Exception_id);
extern void FMSH_ExceptionInit(void);
extern void FMSH_DataAbortHandler(void *CallBackRef);
extern void FMSH_PrefetchAbortHandler(void *CallBackRef);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FMSH_EXCEPTION_H */
