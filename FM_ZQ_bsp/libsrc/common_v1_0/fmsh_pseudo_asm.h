#ifndef _FMSH_PSEUDO_ASM_H_ /* prevent circular inclusions */
#define _FMSH_PSEUDO_ASM_H_ /* by using protection macros */

/***************************** Include Files ********************************/
#include <stdint.h>

#include "bspconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

#if defined(__aarch64__) || defined(__arch64__)
#include "v8_system.h"
/*
#define mfcpsr()   \
    ({ \
        uint64_t __val; \
        __asm volatile("mrs %0  ,DAIF": "=r" (__val)); \
        __val; \
    })*/
#define mfcpsr(v)    __asm volatile("mrs %0, DAIF" : "=r"(v))
#define mtcpsr(v)    __asm volatile("msr DAIF, %0" : : "r"(v))

#define cpsiei()     //__asm volatile("cpsie	i\n")
#define cpsidi()     //__asm volatile("cpsid	i\n")

#define cpsief()     //__asm volatile("cpsie	f\n")
#define cpsidf()     //__asm volatile("cpsid	f\n")

/* memory synchronization operations */

/* Instruction Synchronization Barrier */
#define isb()        __asm volatile("isb sy")

/* Data Synchronization Barrier */
#define dsb()        __asm volatile("dsb sy")

/* Data Memory Barrier */
#define dmb()        __asm volatile("dmb sy")

/* CP15 operations */
/*
#define mfcp(reg)     \
    ({					\
        uint64_t __val;	\
        __asm volatile("mrs %0, " #reg : "=r" (__val)); \
        __val;							\
    })*/
#define mfcp(reg, v) __asm volatile("mrs %0, " #reg : "=r"(v));
#define mtcp(reg, v) __asm volatile("msr " #reg ",%0" : : "r"(v))

/* cache maintence */
#define dc(reg, v)   __asm volatile("dc " #reg ",%0" : : "r"(v))
#define ic(reg, v)   __asm volatile("ic " #reg ",%0" : : "r"(v))
#define icall(reg)   __asm volatile("ic " #reg)
#define tlbi(reg)    __asm volatile("tlbi " #reg)
#define at(reg, val) __asm volatile("at " #reg ",%0" : : "r"(val))

// #define tlbi_el3() asm volatile("tlbi ALLE3")
// #define tlbi_el2() asm volatile("tlbi ALLE2")
// #define tlbi_el1() asm volatile("tlbi VMALLE1")

/* Count leading zeroes (clz) */
#define clz_c(arg)                                                 \
    ({                                                             \
        uint64_t __rval;                                           \
        uint64_t __val = (uint64_t)(arg);                          \
        __asm volatile("clz  %0, %1" : "=r"(__rval) : "r"(__val)); \
        __rval;                                                    \
    })

/****************************************************************************/
#else /* aarch32 */

#define mtcpsr(v)   __asm volatile("msr	cpsr_cf, %0\n" : : "r"(v))
#define mfcpsr(v)   __asm volatile("mrs	%0, cpsr\n" : "=r"(v))

#define cpsiei()    __asm volatile("cpsie	i\n")
#define cpsidi()    __asm volatile("cpsid	i\n")

#define cpsief()    __asm volatile("cpsie	f\n")
#define cpsidf()    __asm volatile("cpsid	f\n")

/* memory synchronization operations */

/* Instruction Synchronization Barrier */
#define isb()       __asm volatile("isb \n")

/* Data Synchronization Barrier */
#define dsb()       __asm volatile("dsb \n")

/* Data Memory Barrier */
#define dmb()       __asm volatile("dmb \n")

/* CP15 operations */
#define mtcp(rn, v) __asm volatile("mcr " rn "\n" : : "r"(v))
#define mfcp(rn, v) __asm volatile("mrc " rn "\n" : "=r"(v))

#endif

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

#define is_secure()                                 \
    ({                                              \
        uint64_t __val;                             \
        mfcp(currentEL, __val);                     \
        (__val == EL3_REG_VALUE) ? 1 : SECURE_MODE; \
    })  // 0xC -- EL3

#define is_el3()                          \
    ({                                    \
        uint64_t __val;                   \
        mfcp(currentEL, __val);           \
        (__val == EL3_REG_VALUE) ? 1 : 0; \
    })  // 0xC -- EL3

#define is_privilege()            \
    ({                            \
        uint64_t __val;           \
        mfcp(currentEL, __val);   \
        (__val != EL0_REG_VALUE); \
    })  // 0 -- EL0

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FMSH_PSEUDO_ASM_H */
