/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  startup.S
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
#include "bspconfig.h"

#include "cortexr5/cortexr5.h"

    module ?startup
    
    section IRQ_STACK:DATA:NOROOT(3)
    section FIQ_STACK:DATA:NOROOT(3)
    section SVC_STACK:DATA:NOROOT(3)
    section UND_STACK:DATA:NOROOT(3)
    section ABT_STACK:DATA:NOROOT(3)
    section CSTACK:DATA:NOROOT(3) 
    section HEAP:DATA:NOROOT(3) 

//----------------------------------------------------------------
// Reset Handler
//----------------------------------------------------------------
    
    section .boot:CODE:NOROOT(2)

RPU_GLBL_CNTL   DCD     0xFF9A0000
RPU_ERR_INJ     DCD	    0xFF9A0020
RPU_0_CFG       DCD     0xFF9A0100
RPU_1_CFG       DCD     0xFF9A0200
RST_LPD_DBG     DCD	    0xFF5E0240
BOOT_MODE_USER  DCD	    0xFF5E0200
//----------------------------------------------------------------
// Entry point for the Reset handler
//----------------------------------------------------------------
    public Start
    public Reset_Handler
    extern Fmsh_ICacheInvalidate
    extern Fmsh_DCacheInvalidate
    extern Fmsh_InitMPU
    extern ?main
    extern __Region_vector_start__
    extern __Region_atcm_start__
    extern __Region_btcm_start__
    extern __Size_vectors__

Start:    
Reset_Handler:
        MOV	    r0, #0
        MOV	    r1, #0
        MOV	    r2, #0
        MOV	    r3, #0
        MOV	    r4, #0
        MOV	    r5, #0
        MOV	    r6, #0
        MOV	    r7, #0
        MOV	    r8, #0
        MOV	    r9, #0
        MOV	    r10, #0
        MOV	    r11, #0
        MOV	    r12, #0

//----------------------------------------------------------------
// Initialize stack pointer and banked registers for various mode.
// Stacks must be 8 byte aligned.
//----------------------------------------------------------------
        MRS	    r0, cpsr 			        // get the current PSR
        MVN	    r1, #0x1f			        // set up the irq stack pointer
        AND	    r2, r1, r0
        ORR     r2, r2, #ARM_MODE_IRQ	    // IRQ mode
        MSR	    cpsr_c, r2
        LDR	    r13, =SFE(IRQ_STACK)		// IRQ stack pointer
        MOV 	r14,#0

        MRS	    r0, cpsr			        // get the current PSR
        MVN	    r1, #0x1f			        // set up the supervisor stack pointer
        AND	    r2, r1, r0
        orr	    r2, r2, #ARM_MODE_SVC		// supervisor mode
        MSR	    cpsr_c, r2
        LDR	    r13, =SFE(SVC_STACK)		// Supervisor stack pointer
        MOV 	r14,#0

        MRS	    r0, cpsr			        // get the current PSR
        MVN	    r1, #0x1f			        // set up the Abort  stack pointer
        AND	    r2, r1, r0
        ORR	    r2, r2, #ARM_MODE_ABT	    // Abort mode
        MSR	    cpsr_c, r2
        LDR	    r13, =SFE(ABT_STACK)		// Abort stack pointer
        MOV 	r14,#0

        MRS	    r0, cpsr			        // get the current PSR
        MVN	    r1, #0x1f			        // set up the FIQ stack pointer
        AND	    r2, r1, r0
        ORR	    r2, r2, #ARM_MODE_FIQ		// FIQ mode
        MSR	    cpsr_c, r2
        MOV 	r8, #0
        MOV 	r9, #0
        MOV 	r10, #0
        MOV 	r11, #0
        MOV 	r12, #0
        LDR	    r13, =SFE(FIQ_STACK)		// FIQ stack pointer
        MOV 	r14,#0

        MRS	    r0, cpsr			        // get the current PSR
        MVN	    r1, #0x1f			        // set up the Undefine stack pointer
        AND	    r2, r1, r0
        ORR	    r2, r2, #ARM_MODE_UND		// Undefine mode
        MSR	    cpsr_c, r2
        LDR	    r13, =SFE(UND_STACK)		// Undefine stack pointer
        MOV 	r14,#0

        MRS	    r0, cpsr			        // get the current PSR
        MVN	    r1, #0x1f			        // set up the system stack pointer
        AND	    r2, r1, r0
        ORR	    r2, r2, #(ARM_MODE_SYS | F_BIT | I_BIT)			// SYS mode and disable interrupt
        MSR	    cpsr_c, r2
        LDR	    r13, =SFE(CSTACK)		    // SYS stack pointer
        MOV 	r14,#0
        
//----------------------------------------------------------------
// Disable MPU and caches
//----------------------------------------------------------------
// Disable MPU and cache in case it was left enabled from an earlier run
// This does not need to be done from a cold reset

        MRC     p15, 0, r0, c1, c0, 0       // Read System Control Register
        BIC     r0, r0, #0x05               // Disable MPU (M bit) and data cache (C bit)
        BIC     r0, r0, #0x1000             // Disable instruction cache (I bit)
        DSB                                 // Ensure all previous loads/stores have completed
        MCR     p15, 0, r0, c1, c0, 0       // Write System Control Register
        ISB                                 // Ensure subsequent insts execute wrt new MPU settings

//----------------------------------------------------------------
// Disable Branch prediction, TCM ECC check
//----------------------------------------------------------------
// In the Cortex-R5, the Z-bit of the SCTLR does not control the program flow prediction.
// Some control bits in the ACTLR control the program flow and prefetch features instead.
// These are enabled by default, but are shown here for completeness.

        MRC     p15, 0, r0, c1, c0, 1       // Read ACTLR
        ORR     r0, r0, #(0x1 << 17)        // Enable RSDIS bit 17 to disable the return stack
        ORR     r0, r0, #(0x1 << 16)        // Clear BP bit 15 and set BP bit 16:
        BIC     r0, r0, #(0x1 << 15)        // Branch always not taken and history table updates disabled
        
        ORR     r0, r0, #(0x1 << 27)		// Enable B1TCM ECC check
        ORR     r0, r0, #(0x1 << 26)		// Enable B0TCM ECC check
        ORR     r0, r0, #(0x1 << 25)		// Enable ATCM ECC check
        BIC 	r0, r0, #(0x1 << 5)		// Generate abort on parity errors, with [5:3]=b'000
        BIC     r0, r0, #(0x1 << 4)
        BIC	    r0, r0, #(0x1 << 3)
        
        MCR     p15, 0, r0, c1, c0, 1       // Write ACTLR
        ISB

//----------------------------------------------------------------
// Cache invalidation
//----------------------------------------------------------------

        DSB                                 // Complete all outstanding explicit memory operations

        MOV     r0, #0
        MCR     p15, 0, r0, c7, c5, 0       // Invalidate entire instruction cache
        MCR     p15, 0, r0, c15, c5, 0      // Invalidate entire data cache

#if VFP_ENABLE == 1
//----------------------------------------------------------------
// Enable VFP
//----------------------------------------------------------------
// Enable access to VFP by enabling access to Coprocessors 10 and 11.
// Enables Full Access i.e. in both privileged and non privileged modes

        MRC     p15, 0, r0, c1, c0, 2      	// Read Coprocessor Access Control Register (CPACR)
        ORR     r0, r0, #(0xF << 20)       	// Enable access to CP 10 & 11
        MCR     p15, 0, r0, c1, c0, 2      	// Write Coprocessor Access Control Register (CPACR)
        ISB

// enable fpu access
        VMRS	r3, FPEXC
        ORR	    r1, r3, #(1<<30)
        VMSR	FPEXC, r1

// clear the floating point register
        MOV	    r1,#0
        VMOV	d0,r1,r1
	    VMOV	d1,r1,r1
	    VMOV	d2,r1,r1
	    VMOV	d3,r1,r1
	    VMOV	d4,r1,r1
	    VMOV	d5,r1,r1
	    VMOV	d6,r1,r1
	    VMOV	d7,r1,r1
	    VMOV	d8,r1,r1
	    VMOV	d9,r1,r1
	    VMOV	d10,r1,r1
	    VMOV	d11,r1,r1
	    VMOV	d12,r1,r1
	    VMOV	d13,r1,r1
	    VMOV	d14,r1,r1
        VMOV	d15,r1,r1
#endif /* VFP_ENABLE == 1 */

//----------------------------------------------------------------
// TCM Configuration
//----------------------------------------------------------------
// Cortex-R5 optionally provides two Tightly-Coupled Memory (TCM) blocks (ATCM and BTCM) for fast access to code or data.
// ATCM typically holds interrupt or exception code that must be accessed at high speed,
//    without any potential delay resulting from a cache miss.
// BTCM typically holds a block of data for intensive processing, such as audio or video data.
// In the Cortex-R5 processor, both ATCM and BTCM support both instruction and data accesses.

// The following illustrates basic TCM configuration, as the basis for exploration by the user

#if TCM_ENABLE == 1
        MRC     p15, 0, r0, c0, c0, 2       // Read TCM Type Register
        // r0 now contains ATCM & BTCM availability 
        
        MRC     p15, 0, r0, c9, c1, 1       // Read ATCM Region Register
        // r0 now contains ATCM size in bits [6:2]

        MCR     p15, 0, r0, c9, c1, 0       // Read BTCM Region Register
        // r0 now contains BTCM size in bits [6:2]

        LDR     r0, =__Region_atcm_start__  // Set ATCM base address
        ORR     r0, r0, #1                  // Enable it
        MCR     p15, 0, r0, c9, c1, 1       // Write ATCM Region Register

        LDR     r0, =__Region_btcm_start__  // Set BTCM base address
        ORR     r0, r0, #1                  // Enable it
        MCR     p15, 0, r0, c9, c1, 0       // Write BTCM Region Register
#endif /* TCM_ENABLE */

//----------------------------------------------------------------
// MPU Configuration
//----------------------------------------------------------------
#if (MPU_ENABLE == 1)
        BL Fmsh_InitMPU
#endif /* MPU_ENABLE == 1 */
//----------------------------------------------------------------
// Enable Branch prediction
//----------------------------------------------------------------
// In the Cortex-R5, the Z-bit of the SCTLR does not control the program flow prediction. 
// Some control bits in the ACTLR control the program flow and prefetch features instead.
// These are enabled by default, but are shown here for completeness.

        MRC     p15, 0, r0, c1, c0, 1       // Read ACTLR
        BIC     r0, r0, #(0x1 << 17)        // Clear RSDIS bit 17 to enable return stack
        BIC     r0, r0, #(0x1 << 16)        // Clear BP bit 15 and BP bit 16:
        BIC     r0, r0, #(0x1 << 15)        // Normal operation, BP is taken from the global history table.
        ORR	    r0, r0, #(0x1 << 14)        // Disable DBWR for errata 780125
        MCR     p15, 0, r0, c1, c0, 1       // Write ACTLR
        ISB

//----------------------------------------------------------------
// Enable Cache
//----------------------------------------------------------------   
#if (DCACHE_ENABLE_EARLY == 1)
        // invalidate icache
        BL      Fmsh_ICacheInvalidate
        // invalidate dcache
        BL      Fmsh_DCacheInvalidate
        // enable cache
        MRC 	p15,0,r1,c1,c0,0
        LDR	    r0, =0x1004
        ORR 	r1,r1,r0
        DSB
        MCR	    p15,0,r1,c1,c0,0		    // Enable cache
        ISB					                // isb	flush prefetch buffer
#endif


//----------------------------------------------------------------
// Copy vector table to TCM/LOVEC
//---------------------------------------------------------------- 
//       LDR     r0, =__Region_vector_start__    // Դ��ַ
//       LDR     r1, =__Region_atcm_start__           // Ŀ���ַ
//       LDR     r2, =__Size_vectors__             
//       LSR     r2, r2, #2                              
//
//loop
//        LDR     r3, [r0], #4       
//        STR     r3, [r1], #4       
//        SUBS    r2, r2, #1         
//       BNE     loop               

//----------------------------------------------------------------
// Set vector table in TCM/LOVEC
//----------------------------------------------------------------   
#ifndef VEC_TABLE_IN_OCM
        MRC	    p15, 0, r0, c1, c0, 0
        MVN	    r1, #0x2000
        AND	    r0, r0, r1
        MCR	    p15, 0, r0, c1, c0, 0
// Clear VINITHI to enable LOVEC on reset
#if FPAR_CPU_ID == 0
        LDR	    r0, =RPU_0_CFG
#else
        LDR	    r0, =RPU_1_CFG
#endif
        LDR	    r1, [r0]
        BIC	    r1, r1, #(0x1 << 2)
        STR	    r1, [r0]
#endif /* #ifndef VEC_TABLE_IN_OCM */

//----------------------------------------------------------------
// Enable asynchronous abort exception
//---------------------------------------------------------------- 
        
        MRS	r0, cpsr
        BIC	r0, r0, #0x100
        MSR	cpsr_xsf, r0

//----------------------------------------------------------------
// Clear cp15 regs with unknown reset values
//---------------------------------------------------------------- 
        MOV r0, #0x0
        MCR	p15, 0, r0, c5, c0, 0	; DFSR
        MCR	p15, 0, r0, c5, c0, 1	; IFSR
        MCR	p15, 0, r0, c6, c0, 0	; DFAR
        MCR	p15, 0, r0, c6, c0, 2	; IFAR
        MCR	p15, 0, r0, c9, c13, 2	; PMXEVCNTR
        MCR	p15, 0, r0, c13, c0, 2	; TPIDRURW
        MCR	p15, 0, r0, c13, c0, 3	; TPIDRURO

//----------------------------------------------------------------
// Reset and start Cycle Counter
//---------------------------------------------------------------- 
#ifdef PMU_ACCESS_USER
        MOV r2, #0x1
        MCR p15, 0, r2, c9, c14, 0
#endif
        MOV	r2, #0x80000000		; clear overflow
        MCR	p15, 0, r2, c9, c12, 3
        MOV	r2, #0x5		; 0xd D, C, E
        MCR	p15, 0, r2, c9, c12, 0
        MOV	r2, #0x80000000		; enable cycle counter
        MCR	p15, 0, r2, c9, c12, 1

        LDR     r0, =?main
        BLX     r0

Ldone:
        b       Ldone                       // Paranoia: we should never get here

 end