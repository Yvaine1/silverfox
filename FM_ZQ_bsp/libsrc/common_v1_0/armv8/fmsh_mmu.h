#ifndef XIL_MMU_H
#define XIL_MMU_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/***************************** Include Files *********************************/
#include "fmsh_common_types.h"  // custom data type definitions

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/
/* Memory attribute type */
#define ATTR_MEM    (NORM_WB_CACHE)                 /* normal cacheale memory */
#define ATTR_DEVICE (DEVICE_MEMORY | EXECUTE_NEVER) /* device memory */
#define ATTR_MEM_NC \
    (NORM_NONCACHE | OUTER_SHAREABLE) /* normal non-cacheale memory */

#define ATTR_MEM_NS \
    (NORM_WB_CACHE | NON_SECURE)      /* non-security normal cacheale memory */
#define ATTR_DEVICE_NS               \
    (DEVICE_MEMORY | EXECUTE_NEVER | \
     NON_SECURE)                      /* non-security device memory */
#define ATTR_MEM_NC_NS                 \
    (NORM_NONCACHE | OUTER_SHAREABLE | \
     NON_SECURE) /* non-security normal non-cacheale memory */

/* Memory type */
#define NORM_NONCACHE   0x401UL /* Normal Non-cacheable*/
#define STRONG_ORDERED  0x409UL /* Strongly ordered (Device-nGnRnE)*/
#define DEVICE_MEMORY   0x40DUL /* Device memory (Device-nGnRE)*/
#define RESERVED        0x0UL   /* reserved memory*/

/* Normal write-through cacheable inner shareable*/
// #define NORM_WT_CACHE 0x711UL
#define NORM_WT_CACHE   (0x611)  // outer-shareable

/* Normal write back cacheable inner-shareable */
// #define NORM_WB_CACHE 0x705UL
#define NORM_WB_CACHE   0x605UL  // outer-shareable

/*
 * shareability attribute only applicable to
 * normal cacheable memory
 */
#define INNER_SHAREABLE (0x3 << 8)
#define OUTER_SHAREABLE (0x2 << 8)
#define NON_SHAREABLE   (~(0x3 << 8))

/* Execution type */
#define EXECUTE_NEVER   ((0x1 << 53) | (0x1 << 54))

/* Security type */
#define NON_SECURE      (0x1 << 5)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void Fmsh_MMUEnable(void);
void Fmsh_SetTlbAttributes(UINTPTR Addr, u64 attrib);

/*****************************************************************************/
/**
 * brief     It sets the memory attributes for a section, in the translation
 *           table. If the address (defined by Addr) is less than 4GB, the
 *           memory attribute(attrib) is set for a section of 2MB memory. If the
 *           address (defined by Addr) is greater than 4GB, the memory attribute
 *           (attrib) is set for a section of 1GB memory.
 *
 * @param    Addr: 64-bit address for which attributes are to be set.
 * @param    Size: Range size, extended to 2M Byte alignment.
 * @param    attrib: Attribute for the specified memory region. fmsh_mmu.h
 *           contains commonly used memory attributes definitions which can be
 *           utilized for this function.
 *
 * @return   None.
 *
 * @note     The MMU and D-cache need not be disabled before changing an
 *           translation table attribute.
 *
 ******************************************************************************/
void Fmsh_SetTlbAttributesRange(UINTPTR Addr, u64 Size, u64 attrib);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIL_MMU_H */
