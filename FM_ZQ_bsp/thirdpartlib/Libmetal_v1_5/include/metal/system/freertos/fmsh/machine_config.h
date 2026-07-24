/*
 * Copyright (c) 2024, FMSH Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/fmsh/machine_config.h
 * @brief	freertos fmsh system primitives for libmetal.
 */


#ifndef __METAL_MACHINE_CONFIG__H__
#define __METAL_MACHINE_CONFIG__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>
#include <metal/config.h>

#define FMSH_MAXIRQS 192

static inline void WriteReg32(uint32_t addr, uint32_t offset, uint32_t value)
{
	uint32_t *paddr = (uint32_t *)((uintptr_t)addr + offset);
    *paddr = value;
}

#ifdef METAL_MACHINE_FMZQ_A53
#define FPS_GIC_BASEADDR			(0xF9000000)
#define FPAR_SCUGIC_CPU_BASEADDR	(FPS_GIC_BASEADDR + 0x00020000U)
#define FPAR_SCUGIC_DIST_BASEADDR	(FPS_GIC_BASEADDR + 0x00010000U)

#define REG_CPSR_IRQ_ENABLE        0x80U
#define REG_CPSR_FIQ_ENABLE        0x40U

#define MACHINE_EXCEPTION_ALL 	(REG_CPSR_IRQ_ENABLE | REG_CPSR_FIQ_ENABLE)
#define FMSH_Metal_ExceptionEnable()  asm("msr daifclr, #2")
#define FMSH_Metal_ExceptionDisable() asm("msr daifset, #2")

/* pseudo assembler instructions */
#define metal_mfcpsr()    ({uint64_t rval = 0U; \
						asm volatile("mrs %0,  DAIF" : "=r" (rval)); \
						rval; \
						})

extern void Fmsh_DCacheInvalidate(void);
extern void Fmsh_DCacheInvalidateRange(uintptr_t addr, uint32_t size);
extern void Fmsh_DCacheFlush(void);
extern void Fmsh_SetTlbAttributesRange(uintptr_t addr, uint64_t size, uint64_t attrib);

#define Machine_DCacheInvalidate() 				Fmsh_DCacheInvalidate()
#define Machine_DCacheInvalidateRange(adr, len)	Fmsh_DCacheInvalidateRange(adr, len)
#define Machine_DCacheFlush()	 				Fmsh_DCacheInvalidate()
#define Machine_DCacheFlushRange(adr, len)		Fmsh_DCacheInvalidateRange(adr, len)
#define Machine_MemMap(pa, size, flags)			({Fmsh_SetTlbAttributesRange(pa, size, flags); \
													(void *)pa; })

#elif defined(METAL_MACHINE_FMZQ_R5)

#define FPS_GIC_BASEADDR			(0xF9000000)
#define FPAR_SCUGIC_CPU_BASEADDR	(FPS_GIC_BASEADDR + 0x00020000U)
#define FPAR_SCUGIC_DIST_BASEADDR	(FPS_GIC_BASEADDR + 0x00010000U)

#define REG_CPSR_IRQ_ENABLE        0x80U
#define REG_CPSR_FIQ_ENABLE        0x40U

#define MACHINE_EXCEPTION_ALL 	(REG_CPSR_IRQ_ENABLE | REG_CPSR_FIQ_ENABLE)

#define FMSH_Metal_ExceptionEnable()   asm("cpsie i\n")
#define FMSH_Metal_ExceptionDisable()  asm("cpsid i\n")

/* pseudo assembler instructions */
#define metal_mfcpsr()    ({uint32_t rval = 0U; \
              __asm volatile ("mrs %0, cpsr" : "=r" (rval));\
              rval;\
             })

extern void Fmsh_DCacheInvalidate(void);
extern void Fmsh_DCacheInvalidateRange(uintptr_t addr, uint32_t size);
extern void Fmsh_DCacheFlush(void);
extern int Fmsh_FindSetAttribute(uint32_t addr, uint32_t size, uint32_t attrib);

#define Machine_DCacheInvalidate() 				Fmsh_DCacheInvalidate()
#define Machine_DCacheInvalidateRange(adr, len)	Fmsh_DCacheInvalidateRange((uint32_t)adr, (uint32_t)len)
#define Machine_DCacheFlush()	 				Fmsh_DCacheInvalidate()
#define Machine_DCacheFlushRange(adr, len)		Fmsh_DCacheInvalidateRange((uint32_t)adr, (uint32_t)len)
/*
#define Machine_MemMap(pa, size, flags)			({Fmsh_FindSetAttribute((uint32_t)pa, (uint32_t)(log2(size) - 1), (uint32_t)flags); \
													(void *)pa; })
*/

#define Machine_MemMap(pa, size, flags)	((void *)pa)

#elif defined(FMQL) /* end defined(METAL_MACHINE_FMZQ_R5) */

#define MACHINE_EXCEPTION_ALL
#define FMSH_Metal_ExceptionEnable()
#define FMSH_Metal_ExceptionDisable()
// #define metal_mfcpsr() 

#define Machine_DCacheInvalidate()
#define Machine_DCacheInvalidateRange(adr, len)
#define Machine_DCacheFlush()
#define Machine_DCacheFlushRange(adr, len)
#define Machine_MemMap(pa, size, flags)

#endif /* end defined(FMZQ_R5) */

/* GIC configure */
#define FGicPs_ENABLE_OFFSET	0x00000100U  /**< Enable Set Register */
#define FGicPs_DISABLE_OFFSET	0x00000180U  /**< Enable Clear Register */

#define Metal_FGicPs_EnableIntr(DistBaseAddress, Int_Id)                      \
	WriteReg32((DistBaseAddress),                                  \
			FGicPs_ENABLE_OFFSET + (((Int_Id) / 32U) * 4U), \
			(0x00000001U << ((Int_Id) % 32U)))

#define Metal_FGicPs_DisableIntr(DistBaseAddress, Int_Id)                  \
    WriteReg32((DistBaseAddress),                               \
                    FGicPs_DISABLE_OFFSET + (((Int_Id) / 32U) * 4U), \
                    (0x00000001U << ((Int_Id) % 32U)))

#if 0
extern void vPortEnableInterrupt(uint8_t ucInterruptID);
extern void vPortDisableInterrupt(uint8_t ucInterruptID);

/* libmetal interface */
#define Machine_EnableIntr(vector)	vPortEnableInterrupt((uint8_t)vector)
#define Machine_DisableIntr(vector)	vPortDisableInterrupt((uint8_t)vector)
#endif

#define Machine_EnableIntr(vector)	Metal_FGicPs_EnableIntr(FPAR_SCUGIC_DIST_BASEADDR, vector)
#define Machine_DisableIntr(vector)	Metal_FGicPs_DisableIntr(FPAR_SCUGIC_DIST_BASEADDR, vector)

#define Machine_ExceptionEnableMask(mask)		FMSH_Metal_ExceptionEnable()
#define Machine_ExceptionDisableMask(mask)		FMSH_Metal_ExceptionDisable()

#ifdef __cplusplus
}
#endif

#endif /* __METAL_MACHINE_CONFIG__H__ */
