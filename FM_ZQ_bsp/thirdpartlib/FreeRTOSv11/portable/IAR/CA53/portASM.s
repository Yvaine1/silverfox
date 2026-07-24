/*
 * FreeRTOS Kernel V10.4.3 LTS Patch 2
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * 1 tab == 4 spaces!
 */
    INCLUDE armv8\v8_mmu.h
    INCLUDE armv8\v8_system.h
    INCLUDE armv8\bspconfig.h
    
    INCLUDE portASM.h
    
	section .text:CODE:NOROOT(3)

	/* Variables and functions. */    
    extern _freertos_vector_table
	extern vPortIRQHandler
    extern uxGetInterruptNesting
    extern uxGetYieldRequired
    extern vPortTaskSwitchContext
    extern backtrace_abort
    
	public FreeRTOS_IRQ_Handler
	public FreeRTOS_SWI_Handler
	public vPortRestoreTaskContext

/******************************************************************************
 * FreeRTOS_SWI_Handler handler is used to perform a context switch.
 *****************************************************************************/           
    section .text:CODE:NOROOT(3)
    
FreeRTOS_SWI_Handler:

    portSAVE_CONTEXT labels_1
    
    /* Get ESR_ELx.EC */
#if defined( GUEST )
	MRS		X0, ESR_EL1
#else
	MRS		X0, ESR_EL3
#endif
	LSR		X1, X0, #26
#if defined( GUEST )
	CMP		X1, #0x15 	/* 0x15 = SVC instruction. */
#else
	CMP		X1, #0x17 	/* 0x17 = SMC instruction. */
#endif
	B.NE	FreeRTOS_Abort
    
    
    BL vPortTaskSwitchContext 
    portRESTORE_CONTEXT labelr_1
    
FreeRTOS_Abort:
	/* Full ESR is in X0, exception class code is in X1. */
	portRESTORE_CONTEXT_abort labelr_1_1
	BL backtrace_abort

/******************************************************************************
 * vPortRestoreTaskContext is used to start the scheduler.
 *****************************************************************************/
 
    section .text:CODE:NOROOT(3)
    
freertos_vector_base    set	    _freertos_vector_table
 
vPortRestoreTaskContext:
	/* Install the FreeRTOS interrupt handlers. */
	LDR		X1, =freertos_vector_base
#if defined( GUEST )
	MSR		VBAR_EL1, X1
#else
	MSR		VBAR_EL3, X1
#endif
	DSB		SY
	ISB		SY

	/* Start the first task. */
    portRESTORE_CONTEXT labelr_2


/******************************************************************************
 * FreeRTOS_IRQ_Handler handles IRQ entry and exit.
 *****************************************************************************/
    
    section .text:CODE:NOROOT(3)
    
FreeRTOS_IRQ_Handler:
	/* Save volatile registers. */
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
	STP 	X29, X30, [SP, #-0x10]!
    
	/* Save the SPSR and ELR. */
#if defined( GUEST )
	MRS		X3, SPSR_EL1
	MRS		X2, ELR_EL1
#else
	MRS		X3, SPSR_EL3
	MRS		X2, ELR_EL3
#endif
	STP 	X2, X3, [SP, #-0x10]!
    
	/* Call the C handler. */
	BL vPortIRQHandler
    
    /* Has interrupt nesting unwound? */
    BL      uxGetInterruptNesting
	CMP		X0, #0
	B.NE	Exit_IRQ_No_Context_Switch

	/* Is a context switch required? */
	BL      uxGetYieldRequired
	CMP		X0, #0
	B.EQ	Exit_IRQ_No_Context_Switch
    
	/* Restore volatile registers. */
	LDP 	X2, X3, [SP], #0x10
#if defined( GUEST )
	MSR		SPSR_EL1, X3
	MSR		ELR_EL1, X2
#else
	MSR		SPSR_EL3, X3 /*_RB_ Assumes started in EL3. */
	MSR		ELR_EL3, X2
#endif

	LDP 	X29, X30, [SP], #0x10
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
    
	/* Save the context of the current task and select a new task to run. */
    portSAVE_CONTEXT labels_3
	BL vPortTaskSwitchContext
    portRESTORE_CONTEXT labelr_3
    
Exit_IRQ_No_Context_Switch:

	/* Restore volatile registers. */
	LDP 	X2, X3, [SP], #0x10
#if defined( GUEST )
	MSR		SPSR_EL1, X3
	MSR		ELR_EL1, X2
#else
	MSR		SPSR_EL3, X3 /*_RB_ Assumes started in EL3. */
	MSR		ELR_EL3, X2
#endif

	LDP 	X29, X30, [SP], #0x10
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
    
    ERET

    section .text:CODE:NOROOT(3)
    
    public SMP_TryLock
    
SMP_TryLock:
TryLock:
    LDXR    W1, [X0]
    CMP     W1, #1
    B.EQ    LockAlready

    MOV     W2, #1
    STXR    W1, W2, [X0]
    CMP     W1, #0
    B.NE    TryLock
    
LockAlready:
    MOV     W0, W1
    DMB     SY
    RET

    section .text:CODE:NOROOT(3)
    
    public SMP_Unlock
    
SMP_Unlock:
    DMB     SY

    MOV     W1, #0
    STR     W1, [X0]

    SEV         /* let everyone know */
    DSB SY
    ISB SY
    RET
        
/******************************************************************************
 * vSecondaryStart do secondary start kernel
 *****************************************************************************/
    section CSTACK:DATA:NOROOT(3)
    section CSTACK1:DATA:NOROOT(3)
    section CSTACK2:DATA:NOROOT(3)
    section CSTACK3:DATA:NOROOT(3)
    
    section .text:CODE:NOROOT(3)
    
    extern el1_vectors
    extern el2_vectors
    extern el3_vectors
    extern vSecondaryStart
    
    extern GetCPUID
    extern InvalidateUDCaches
    
    extern MMUTableL0
    extern MMUTableL1
    extern MMUTable1L0
    
    extern FPUContextBase
    extern FPUContext
    
    public vSecondaryEntry
    
vSecondaryEntry:
    //
    // program the VBARs
    //
    ldr x1, =el1_vectors
    msr VBAR_EL1, x1

    ldr x1, =el2_vectors
    msr VBAR_EL2, x1

    ldr x1, =el3_vectors
    msr VBAR_EL3, x1

    mov      x0, #0
    msr      CPTR_EL3, x0
    isb

    /* Configure SCR_EL3 */
    mov      w1, #0              	//; Initial value of register is unknown
    orr      w1, w1, #(1 << 11)  	//; Set ST bit (Secure EL1 can access CNTPS_TVAL_EL1, CNTPS_CTL_EL1 & CNTPS_CVAL_EL1)
    orr      w1, w1, #(1 << 10)  	//; Set RW bit (EL1 is AArch64, as this is the Secure world)
    orr      w1, w1, #(1 << 3)   	//; Set EA bit (SError routed to EL3)
    // orr      w1, w1, #(1 << 0)   	//; Set NS bit (EL1 NS)
    orr      w1, w1, #(1 << 2)   	//; Set FIQ bit (FIQs routed to EL3)
    orr      w1, w1, #(1 << 1)   	//; Set IRQ bit (IRQs routed to EL3)
    msr      SCR_EL3, x1
    isb

    /* Enable SError Exception for asynchronous abort */
    msr DAIFSet, #0x3
    msr DAIFClr, #0xc
    //
    // no traps or VM modifications from the Hypervisor, EL1 is AArch64
    //
    mov x2, #HCR_EL2_RW
    msr HCR_EL2, x2

    //
    // VMID is still significant, even when virtualisation is not
    // being used, so ensure VTTBR_EL2 is properly initialised
    //
    msr VTTBR_EL2, xzr
    
    //
    // VMPIDR_EL2 holds the value of the Virtualization Multiprocessor ID. This is the value returned by Non-secure EL1 reads of MPIDR_EL1.
    //  VPIDR_EL2 holds the value of the Virtualization Processor ID. This is the value returned by Non-secure EL1 reads of MIDR_EL1.
    // Both of these registers are architecturally UNKNOWN at reset, and so they must be set to the correct value
    // (even if EL2/virtualization is not being used), otherwise non-secure EL1 reads of MPIDR_EL1/MIDR_EL1 will return garbage values.
    // This guarantees that any future reads of MPIDR_EL1 and MIDR_EL1 from Non-secure EL1 will return the correct value.
    //
    mrs x0, MPIDR_EL1
    msr VMPIDR_EL2, x0
    mrs x0, MIDR_EL1
    msr VPIDR_EL2, x0

    // extract the core number from MPIDR_EL1 and store it in
    // x19 (defined by the AAPCS as callee-saved), so we can re-use
    // the number later
    //
    bl GetCPUID
    mov x19, x0

    //
    // neither EL3 nor EL2 trap floating point or accesses to CPACR
    //
    msr CPTR_EL3, xzr
    msr CPTR_EL2, xzr
    
    //
    // SCTLR_ELx may come out of reset with UNKNOWN values so we will
    // set the fields to 0 except, possibly, the endianess field(s).
    // Note that setting SCTLR_EL2 or the EL0 related fields of SCTLR_EL1
    // is not strictly needed, since we're never in EL2 or EL0
    //
#ifdef __ARM_BIG_ENDIAN
    mov x0, #(SCTLR_ELx_EE | SCTLR_EL1_E0E)
#else
    mov x0, #0
#endif
    msr SCTLR_EL3, x0
    msr SCTLR_EL2, x0
    msr SCTLR_EL1, x0


    //
    // configure CPUECTLR_EL1
    //
    // These bits are IMP DEF, so need to different for different
    // processors
    //
    // SMPEN - bit 6 - Enables the processor to receive cache
    //                 and TLB maintenance operations
    //
    // Note: For Cortex-A57/53 SMPEN should be set before enabling
    //       the caches and MMU, or performing any cache and TLB
    //       maintenance operations.
    //
    //       This register has a defined reset value, so we use a
    //       read-modify-write sequence to set SMPEN
    //
    mrs x0, S3_1_c15_c2_1  // Read EL1 CPU Extended Control Register
    orr x0, x0, #(1 << 6)  // Set the SMPEN bit
    msr S3_1_c15_c2_1, x0  // Write EL1 CPU Extended Control Register

#if CONFIG_ARM_ERRATA_855873
    mrs x0, S3_1_C15_C2_0
    /*
     *  Set ENDCCASCI bit in CPUACTLR_EL1 register, to execute data
     *  cache clean operations as data cache clean and invalidate
     *
     */
    orr     x0, x0, #(1 << 44)      //; Set ENDCCASCI bit
    msr S3_1_C15_C2_0, x0       //CPUACTLR_EL1
#endif
    isb

    tlbi    ALLE3
    ic      IALLU                   //; Invalidate I cache to PoU
    bl      InvalidateUDCaches
    dsb     sy
    isb

    /**********************************************
    * Set up memory attributes
    * This equates to:
    * 0 = b01000100 = Normal, Inner/Outer Non-Cacheable
    * 1 = b11111111 = Normal, Inner/Outer WB/WA/RA
    * 2 = b00000000 = Device-nGnRnE
    * 3 = b00000100 = Device-nGnRE
    * 4 = b10111011 = Normal, Inner/Outer WT/WA/RA
    * 5 = b11001100 = Normal, Inner/Outer WB/WnA/RnA // for nEXTERRIRQ irq test
    **********************************************/
    ldr      x1, =0x0000CCBB0400FF44
    msr      MAIR_EL3, x1
    
    /**********************************************
    * Set up TCR_EL3
    * Physical Address Size PS =  010 -> 40bits 1TB
    * Granual Size TG0 = 00 -> 4KB
    * size offset of the memory region T0SZ = 24 -> (region size 2^(64-24) = 2^40)
    ***************************************************/
    ldr      x1,=0x80823518
    msr      TCR_EL3, x1
    isb

#if DCACHE_ENABLE_EARLY
    ldr      x1, =MMUTableL0       // Get address of level 0 for MMUTableL0
    msr      TTBR0_EL3, x1         // Set MMUTableL0
#endif

    /* Configure SCTLR_EL3 */
    mrs      x1, SCTLR_EL3
    orr      x1, x1, #(1 << 12)     //Enable I cache
    orr      x1, x1, #(1 << 3)      //Enable SP alignment check
#if DCACHE_ENABLE_EARLY
    orr      x1, x1, #(1 << 2)      //Enable caches
    orr      x1, x1, #(1 << 0)      //Enable MMU
#endif
    msr      SCTLR_EL3, x1
    dsb      sy
    isb
    
    //secondary stack
core1_stack:
    cmp x19, #0x1
    b.ne core2_stack
    ldr x0, =sfe(CSTACK1)
    mov sp,  x0
    b stack_end
core2_stack:
    cmp x19, #0x2
    b.ne core3_stack
    ldr x0, =sfe(CSTACK2)
    mov sp,  x0
    b stack_end
core3_stack:
    ldr x0, =sfe(CSTACK3)
    mov sp,  x0
stack_end:

    mov x1, #(0x3 <<20)
    msr cpacr_el1,x1

    mrs x1,cptr_el3
    and x1,x1,#~(1<<10)
    msr cptr_el3,x1

    isb
    
    // configure floating context
    ldr	x1, =FPUContextBase
    ldr	x0, =FPUContext
    /* Save the address of floating point context array to FPUContextBase */
    str	x0, [x1]
    
    BL vSecondaryStart

    END