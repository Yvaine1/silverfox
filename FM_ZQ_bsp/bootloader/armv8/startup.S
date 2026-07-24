// ------------------------------------------------------------
// Armv8-A Single-core EL3 AArch64 Startup Code
//
// Basic Vectors, MMU, caches and GICv3 initialization
//
// Exits in EL1 AArch64
//
// Copyright (c) 2014-2019 Arm Limited (or its affiliates). All rights reserved.
// Use, modification and redistribution of this file is subject to your possession of a
// valid End User License Agreement for the Arm Product of which these examples are part of 
// and your compliance with all applicable terms and conditions of such licence agreement.
// ------------------------------------------------------------
#include "armv8/bspconfig.h"

#include "armv8/v8_mmu.h"
#include "armv8/v8_system.h"

    MODULE  ?startup
    
    section CSTACK:DATA:NOROOT(3)
    
    section .boot:CODE:NOROOT(3)
    
    public Start64
#if EL0_LIVE
    public drop_to_el0_from_el1
#endif
#if EL1_LIVE
    public drop_to_el1_from_el3
#endif
#if EL2_LIVE
    public drop_to_el2_from_el3
#endif

    extern el1_vectors
    extern el2_vectors
    extern el3_vectors

    extern EL0_main
    extern EL1_main
    extern EL2_main

    extern GetCPUID
    extern InvalidateUDCaches

    extern MMUTableL0
    extern MMUTableL1

    extern FPUContextBase
    extern FPUContext
    extern ?main
    
// ------------------------------------------------------------
Start64:
    mov      x0, #0
    mov      x1, #0
    mov      x2, #0
    mov      x3, #0
    mov      x4, #0
    mov      x5, #0
    mov      x6, #0
    mov      x7, #0
    mov      x8, #0
    mov      x9, #0
    mov      x10, #0
    mov      x11, #0
    mov      x12, #0
    mov      x13, #0
    mov      x14, #0
    mov      x15, #0
    mov      x16, #0
    mov      x17, #0
    mov      x18, #0
    mov      x19, #0
    mov      x20, #0
    mov      x21, #0
    mov      x22, #0
    mov      x23, #0
    mov      x24, #0
    mov      x25, #0
    mov      x26, #0
    mov      x27, #0
    mov      x28, #0
    mov      x29, #0
    mov      x30, #0
    // mrs      x0, MPIDR_EL1
    // and      x0, x0, #0xFF                        //Mask off to leave Aff0
    //sub      x0, x0, #0                         //for different a53 core gtc interrupt test
    cbz      x0, OKToRun                          //If core 0, run the primary init code
EndlessLoop0:
    wfi
    b        EndlessLoop0

OKToRun:
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
    
#ifdef __FSBL__
    /* Enable SError Exception for asynchronous abort */
    msr DAIFClr, #0x4
#else
    /* Enable SError Exception for asynchronous abort */
    msr DAIFClr, #0xf
#endif

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

    // configure stack
    ldr x0, =sfe(CSTACK)
    mov sp,  x0

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

    // Branch to C library init code
    b  ?main

#if EL1_LIVE
/* drop el1 code from el3 */
    //
    // Set up EL1 entry point and "dummy" exception return information,
    // then perform exception return to enter EL1
    //
drop_to_el1_from_el3:
    /* configure IRQ&FIQ do not routed to EL3 */
    mrs x1, SCR_EL3
    and  w1, w1, #~(1 << 2)   	//; Set FIQ bit (FIQs routed to EL3)
    and  w1, w1, #~(1 << 1)   	//; Set IRQ bit (IRQs routed to EL3)
#if SECURE_MODE
    and  w1, w1, #~(1 << 0)
#else
    orr  w1, w1, #(1 << 0)   	//; Set NS bit (EL1 NS)
#endif
    msr  SCR_EL3, x1

    /* drop configure */
    adr x1, el1_entry_aarch64
    msr ELR_EL3, x1
    mov x1, #(AARCH64_SPSR_EL1h | AARCH64_SPSR_F | AARCH64_SPSR_I | AARCH64_SPSR_A)
//  mov x1, #0x64
    msr SPSR_EL3, x1
    eret

// ------------------------------------------------------------
// EL1 - Common start-up code
// ------------------------------------------------------------
el1_entry_aarch64:
    //
    // Now we're in EL1, setup the application stack
    //
    ldr x0, =sfe(CSTACK)
    mov sp, x0

    //
    // Enable floating point
    //
    mov x1, #(0x3 << 20)
    msr cpacr_el1,x1
#if 0
    //
    // Invalidate caches and TLBs for all stage 1
    // translations used at EL1
    //
    // Cortex-A processors automatically invalidate their caches on reset
    // (unless suppressed with the DBGL1RSTDISABLE or L2RSTDISABLE pins).
    // It is therefore not necessary for software to invalidate the caches 
    // on startup, however, this is done here in case of a warm reset.
    bl  InvalidateUDCaches
    tlbi VMALLE1
    tlbi 	ALLE1
    ic      IALLU                  	//; Invalidate I cache to PoU
    bl 	InvalidateUDCaches
    dsb	 sy
    isb
#endif
    //
    /* Enable SError Exception for asynchronous abort */
	msr DAIFClr, #0xf
    // Branch to C code
    b  EL1_main
#endif // end EL1_LIVE

#if EL2_LIVE
/* drop el2 code form el3 */
    //
    // Set up EL2 entry point and "dummy" exception return information,
    // then perform exception return to enter EL2
    //
drop_to_el2_from_el3:
    /* configure IRQ&FIQ do not routed to EL3 */
    mrs x1, SCR_EL3
    and  w1, w1, #~(1 << 2)   	//; Set FIQ bit (FIQs routed to EL3)
    and  w1, w1, #~(1 << 1)   	//; Set IRQ bit (IRQs routed to EL3)
    msr  SCR_EL3, x1

    /* drop configure */
    adr x1, el2_entry_aarch64
    msr ELR_EL3, x1
    mov x1, #(AARCH64_SPSR_EL2h | AARCH64_SPSR_F | AARCH64_SPSR_I | AARCH64_SPSR_A)
//  mov x1, #0x64
    msr SPSR_EL3, x1
    eret


// ------------------------------------------------------------
// EL2 - Common start-up code
// ------------------------------------------------------------
el2_entry_aarch64:
    //
    // Now we're in EL2, setup the application stack
    //
    ldr x0, =sfe(CSTACK)
    mov sp, x0

    //
    // Enable floating point
    //
     mov x1, #(0x3 << 20)
     msr cptr_el2,x1
     
    /* Enable SError Exception for asynchronous abort */
	msr DAIFClr, #0xf
    // Branch to C code
    b  EL2_main
#endif // end EL2_LIVE

#if EL0_LIVE
/* drop el0 code from el1 */
drop_to_el0_from_el1:
    /* drop configure */
    adr x1, el0_entry_aarch64
    msr ELR_EL1, x1
    mov x1, #(AARCH64_SPSR_EL0t | AARCH64_SPSR_F | AARCH64_SPSR_I | AARCH64_SPSR_A)
//  mov x1, #0x64
    msr SPSR_EL1, x1
    eret


// ------------------------------------------------------------
// EL1 - Common start-up code
// ------------------------------------------------------------
el0_entry_aarch64:
    //
    // Now we're in EL0, setup the application stack
    //
    ldr x0, =sfe(CSTACK)
    mov sp, x0

    // Branch to C code
    b  EL0_main
#endif // end EL0_LIVE
  END

// ------------------------------------------------------------

// AArch64 Arm C library startup add-in:

// The Arm Architecture Reference Manual for Armv8-A states:
//
//     Instruction accesses to Non-cacheable Normal memory can be held in instruction caches.
//     Correspondingly, the sequence for ensuring that modifications to instructions are available
//     for execution must include invalidation of the modified locations from the instruction cache,
//     even if the instructions are held in Normal Non-cacheable memory.
//     This includes cases where the instruction cache is disabled.
//
// To invalidate the AArch64 instruction cache after scatter-loading and before initialization of the stack and heap,
// it is necessary for the user to:
//
// * Implement instruction cache invalidation code in _platform_pre_stackheap_init.
// * Ensure all code on the path from the program entry up to and including _platform_pre_stackheap_init is located in a root region.
#if 0
    public _platform_pre_stackheap_init
_platform_pre_stackheap_init:
    dsb ish     // ensure all previous stores have completed before invalidating
    ic  ialluis // I cache invalidate all inner shareable to PoU (which includes secondary cores)
    dsb ish     // ensure completion on inner shareable domain   (which includes secondary cores)
    isb

    // Scatter-loading is complete, so enable the caches here, so that the C-library's mutex initialization later will work
    mrs x1, SCTLR_EL1
    orr x1, x1, #SCTLR_ELx_C
    orr x1, x1, #SCTLR_ELx_I
    msr SCTLR_EL1, x1
    isb

    ret
#endif
