// ------------------------------------------------------------
// Armv8-A Vector tables
//
// Copyright (c) 2014-2016 Arm Limited (or its affiliates). All rights reserved.
// Use, modification and redistribution of this file is subject to your possession of a
// valid End User License Agreement for the Arm Product of which these examples are part of 
// and your compliance with all applicable terms and conditions of such licence agreement.
// ------------------------------------------------------------

#include "armv8/bspconfig.h"

    section .text:CODE:NOROOT(3)    

    public irqFirstLevelHandler
    public fiqFirstLevelHandler
    public FPUContextBase
    public FPUContext
//    public el1_vectors
//    public el2_vectors
//    public el3_vectors

    extern IRQInterrupt
    extern FIQInterrupt
/*
 * FPUContextSize is the size of the array where floating point registers are
 * stored when required. The default size corresponds to the case when there is no
 * nested interrupt. If there are nested interrupts in application which are using
 * floating point operation, the size of FPUContextSize need to be increased as per
 * requirement
 */
FPUContextSize set 528

saveregister macro
	stp	X0,X1, [sp,#-0x10]!
	stp	X2,X3, [sp,#-0x10]!
	stp	X4,X5, [sp,#-0x10]!
	stp	X6,X7, [sp,#-0x10]!
	stp	X8,X9, [sp,#-0x10]!
	stp	X10,X11, [sp,#-0x10]!
	stp	X12,X13, [sp,#-0x10]!
	stp	X14,X15, [sp,#-0x10]!
	stp	X16,X17, [sp,#-0x10]!
	stp 	X18,X19, [sp,#-0x10]!
	stp 	X29,X30, [sp,#-0x10]!
    endm

restoreregister macro
	ldp 	X29,X30, [sp], #0x10
	ldp 	X18,X19, [sp], #0x10
	ldp	X16,X17, [sp], #0x10
	ldp	X14,X15, [sp], #0x10
	ldp	X12,X13, [sp], #0x10
	ldp	X10,X11, [sp], #0x10
	ldp	X8,X9, [sp], #0x10
	ldp	X6,X7, [sp], #0x10
	ldp	X4,X5, [sp], #0x10
	ldp	X2,X3, [sp], #0x10
	ldp	X0,X1, [sp], #0x10
    endm

savefloatregister macro
/* Load the floating point context array address from FPUContextBase */
	ldr	x1, =FPUContextBase
	ldr	x0, [x1]

/* Save all the floating point register to the array */
	stp	q0,q1, [x0], #0x20
	stp	q2,q3, [x0], #0x20
	stp	q4,q5, [x0], #0x20
	stp	q6,q7, [x0], #0x20
	stp	q8,q9, [x0], #0x20
	stp	q10,q11, [x0], #0x20
	stp	q12,q13, [x0], #0x20
	stp	q14,q15, [x0], #0x20
	stp	q16,q17, [x0], #0x20
	stp	q18,q19, [x0], #0x20
	stp	q20,q21, [x0], #0x20
	stp	q22,q23, [x0], #0x20
	stp	q24,q25, [x0], #0x20
	stp	q26,q27, [x0], #0x20
	stp	q28,q29, [x0], #0x20
	stp	q30,q31, [x0], #0x20
	mrs	x2, FPCR
	mrs	x3, FPSR
	stp	x2, x3, [x0], #0x10

/* Save current address of floating point context array to FPUContextBase */
	str	x0, [x1]
    endm

restorefloatregister macro
/* Restore the address of floating point context array from FPUContextBase */
	ldr	x1,=FPUContextBase
	ldr	x0, [x1]

/* Restore all the floating point register from the array */
    ldp	x2, x3, [x0,#-0x10]!
    msr	FPCR, x2
    msr	FPSR, x3
    ldp	q30,q31, [x0,#-0x20]!
    ldp	q28,q29, [x0,#-0x20]!
    ldp	q26,q27, [x0,#-0x20]!
    ldp	q24,q25, [x0,#-0x20]!
    ldp	q22,q23, [x0,#-0x20]!
    ldp	q20,q21, [x0,#-0x20]!
    ldp	q18,q19, [x0,#-0x20]!
    ldp	q16,q17, [x0,#-0x20]!
    ldp	q14,q15, [x0,#-0x20]!
    ldp	q12,q13, [x0,#-0x20]!
    ldp	q10,q11, [x0,#-0x20]!
    ldp	q8,q9, [x0,#-0x20]!
    ldp	q6,q7, [x0,#-0x20]!
    ldp	q4,q5, [x0,#-0x20]!
    ldp	q2,q3, [x0,#-0x20]!
    ldp	q0,q1, [x0,#-0x20]!

/* Save current address of floating point context array to FPUContextBase */
	str	x0, [x1]
    endm
    
//
// Current EL with SP0
//
el1_vectors:
#if EL1_LIVE
    section .c0sync1:CODE:ROOT(7)
c0sync1: B c0sync1

    section .c0irq1:CODE:ROOT(7)
c0irq1: B irqFirstLevelHandler

    section .c0fiq1:CODE:ROOT(7)
c0fiq1: B fiqFirstLevelHandler

    section .c0serr1:CODE:ROOT(7)
c0serr1: B c0serr1

//
// Current EL with SPx
//
     section .cxsync1:CODE:ROOT(7)
cxsync1: B cxsync1

     section .cxirq1:CODE:ROOT(7)
cxirq1: B irqFirstLevelHandler

     section .cxfiq1:CODE:ROOT(7)
cxfiq1: B fiqFirstLevelHandler

     section .cxserr1:CODE:ROOT(7)
cxserr1: B cxserr1

//
// Lower EL using AArch64
//
    section .l64sync1:CODE:ROOT(7)
l64sync1: B l64sync1

    section .l64irq1:CODE:ROOT(7)
l64irq1: B irqFirstLevelHandler

    section .l64fiq1:CODE:ROOT(7)
l64fiq1: B fiqFirstLevelHandler

    section .l64serr1:CODE:ROOT(7)
l64serr1: B l64serr1

//
// Lower EL using AArch32
//
    section .l32sync1:CODE:ROOT(7)
l32sync1: B l32sync1

    section .l32irq1:CODE:ROOT(7)
l32irq1: B irqFirstLevelHandler

    section .l32fiq1:CODE:ROOT(7)
l32fiq1: B fiqFirstLevelHandler

    section .l32serr1:CODE:ROOT(7)
l32serr1: B l32serr1
#endif // EL1_LIVE

//----------------------------------------------------------------
//
// Current EL with SP0
//
el2_vectors:
#if EL2_LIVE
    section .c0sync2:CODE:ROOT(7)
c0sync2: B c0sync2

    section .c0irq2:CODE:ROOT(7)
c0irq2: B irqFirstLevelHandler

    section .c0fiq2:CODE:ROOT(7)
c0fiq2: B fiqFirstLevelHandler

    section .c0serr2:CODE:ROOT(7)
c0serr2: B c0serr2

//
// Current EL with SPx
//
    section .cxsync2:CODE:ROOT(7)
cxsync2: B cxsync2

    section .cxirq2:CODE:ROOT(7)
cxirq2: B irqFirstLevelHandler

    section .cxfiq2:CODE:ROOT(7)
cxfiq2: B fiqFirstLevelHandler

    section .cxserr2:CODE:ROOT(7)
cxserr2: B cxserr2

//
// Lower EL using AArch64
//
    section .l64sync2:CODE:ROOT(7)
l64sync2: B l64sync2

    section .l64irq2:CODE:ROOT(7)
l64irq2: B irqFirstLevelHandler

    section .l64fiq2:CODE:ROOT(7)
l64fiq2: B fiqFirstLevelHandler

    section .l64serr2:CODE:ROOT(7)
l64serr2: B l64serr2

//
// Lower EL using AArch32
//
    section .l32sync2:CODE:ROOT(7)
l32sync2: B l32sync2

    section .l32irq2:CODE:ROOT(7)
l32irq2: B irqFirstLevelHandler

    section .l32fiq2:CODE:ROOT(7)
l32fiq2: B fiqFirstLevelHandler

    section .l32serr2:CODE:ROOT(7)
l32serr2: B l32serr2

#endif // EL2_LIVE
//----------------------------------------------------------------

//
// Current EL with SP0
//
el3_vectors:
    section .c0sync3:CODE:ROOT(7)
c0sync3: B c0sync3

    section .c0irq3:CODE:ROOT(7)
c0irq3: B irqFirstLevelHandler

    section .c0fiq3:CODE:ROOT(7)
c0fiq3: B fiqFirstLevelHandler

    section .c0serr3:CODE:ROOT(7)
c0serr3: B c0serr3

//
// Current EL with SPx
//
    section .cxsync3:CODE:ROOT(7)
cxsync3: B cxsync3

    section .cxirq3:CODE:ROOT(7)
cxirq3: B irqFirstLevelHandler

    section .cxfiq3:CODE:ROOT(7)
cxfiq3: B fiqFirstLevelHandler

    section .cxserr3:CODE:ROOT(7)
cxserr3: B cxserr3

//
// Lower EL using AArch64
//
    section .l64sync3:CODE:ROOT(7)
l64sync3: B l64sync3

    section .l64irq3:CODE:ROOT(7)
l64irq3: B irqFirstLevelHandler

    section .l64fiq3:CODE:ROOT(7)
l64fiq3: B fiqFirstLevelHandler

    section .l64serr3:CODE:ROOT(7)
l64serr3: B l64serr3

//
// Lower EL using AArch32
//
    section .l32sync3:CODE:ROOT(7)
l32sync3: B l32sync3

    section .l32irq3:CODE:ROOT(7)
l32irq3: B irqFirstLevelHandler

    section .l32fiq3:CODE:ROOT(7)
l32fiq3: B fiqFirstLevelHandler

    section .l32serr3:CODE:ROOT(7)
l32serr3: B l32serr3


    section .text:CODE:NOROOT:REORDER(3)
irqFirstLevelHandler:
    saveregister
    mrs	x4, currentEL
    cmp	x4, #0xc
    b.eq save_el3
    cmp	x4, #0x8
    b.eq save_el2
    
    /* el1 */
	mrs x0, CPACR_EL1
	mrs x1, ELR_EL1
	mrs	x2, SPSR_EL1    
    b save_r
    
save_el3:  /* el3 */
	mrs x0, CPTR_EL3
	mrs x1, ELR_EL3
	mrs	x2, SPSR_EL3
    b save_r
    
save_el2:  /* el2 */
	mrs x0, CPTR_EL2
	mrs x1, ELR_EL2
	mrs	x2, SPSR_EL2

save_r:
	stp	x0, x1, [sp,#-0x10]!
	str	x2, [sp,#-0x10]!
	isb

    savefloatregister
	bl	IRQInterrupt
	restorefloatregister

/* Restore the status of SPSR, ELR and CPTR from stack */
RestorePrevState:
	ldr	x2,[sp],0x10
	ldp	x0, x1, [sp],0x10
    
    mrs	x4, currentEL
    cmp	x4, #0xc
    b.eq restore_el3
    
    cmp	x4, #0x8
    b.eq restore_el2
    
    /* el1 */
    msr	CPACR_EL1, x0
	msr	ELR_EL1, x1
	msr	SPSR_EL1, x2
    b restore_r   
restore_el3:  /* el3 */
	msr	CPTR_EL3, x0
	msr	ELR_EL3, x1
	msr	SPSR_EL3, x2
    b restore_r
restore_el2:  /* el2 */
	msr	CPTR_EL2, x0
	msr	ELR_EL2, x1
	msr	SPSR_EL2, x2
restore_r:
    restoreregister
    ERET


fiqFirstLevelHandler:
    saveregister
    BL       FIQInterrupt
    restoreregister
    ERET

irqHandler:
fiqHandler:

    section .FPU_DATA:DATA:NOROOT:REORDER(4)
/* Array to store floating point registers */
FPUContext: 
    ds8 FPUContextSize
/* Stores address for floating point context array */
FPUContextBase:
    ds8 8

FPUStatus:
    ds8 1
    
    END