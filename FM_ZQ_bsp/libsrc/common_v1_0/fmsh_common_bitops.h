/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common_bitops.h
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
 *
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_COMMON_BITOPS_H_
#define _FMSH_COMMON_BITOPS_H_

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/
#include <limits.h>

#include "fmsh_common_types.h"

/************************** Constant Definitions *****************************/

/* Constant definitions for various bits of a 32-bit word. */
#define BIT0  0x00000001U
#define BIT1  0x00000002U
#define BIT2  0x00000004U
#define BIT3  0x00000008U
#define BIT4  0x00000010U
#define BIT5  0x00000020U
#define BIT6  0x00000040U
#define BIT7  0x00000080U
#define BIT8  0x00000100U
#define BIT9  0x00000200U
#define BIT10 0x00000400U
#define BIT11 0x00000800U
#define BIT12 0x00001000U
#define BIT13 0x00002000U
#define BIT14 0x00004000U
#define BIT15 0x00008000U
#define BIT16 0x00010000U
#define BIT17 0x00020000U
#define BIT18 0x00040000U
#define BIT19 0x00080000U
#define BIT20 0x00100000U
#define BIT21 0x00200000U
#define BIT22 0x00400000U
#define BIT23 0x00800000U
#define BIT24 0x01000000U
#define BIT25 0x02000000U
#define BIT26 0x04000000U
#define BIT27 0x08000000U
#define BIT28 0x10000000U
#define BIT29 0x20000000U
#define BIT30 0x40000000U
#define BIT31 0x80000000U

#if (defined __aarch64__) || (defined __arch64__)

#define BIT32            0x0000000100000000ULL
#define BIT33            0x0000000200000000ULL
#define BIT34            0x0000000400000000ULL
#define BIT35            0x0000000800000000ULL
#define BIT36            0x0000001000000000ULL
#define BIT37            0x0000002000000000ULL
#define BIT38            0x0000004000000000ULL
#define BIT39            0x0000008000000000ULL
#define BIT40            0x0000010000000000ULL
#define BIT41            0x0000020000000000ULL
#define BIT42            0x0000040000000000ULL
#define BIT43            0x0000080000000000ULL
#define BIT44            0x0000100000000000ULL
#define BIT45            0x0000200000000000ULL
#define BIT46            0x0000400000000000ULL
#define BIT47            0x0000800000000000ULL
#define BIT48            0x0001000000000000ULL
#define BIT49            0x0002000000000000ULL
#define BIT50            0x0004000000000000ULL
#define BIT51            0x0008000000000000ULL
#define BIT52            0x0010000000000000ULL
#define BIT53            0x0020000000000000ULL
#define BIT54            0x0040000000000000ULL
#define BIT55            0x0080000000000000ULL
#define BIT56            0x0100000000000000ULL
#define BIT57            0x0200000000000000ULL
#define BIT58            0x0400000000000000ULL
#define BIT59            0x0800000000000000ULL
#define BIT60            0x1000000000000000ULL
#define BIT61            0x2000000000000000ULL
#define BIT62            0x4000000000000000ULL
#define BIT63            0x8000000000000000ULL

#define BITS_ALL         0xFFFFFFFFFFFFFFFFULL
#define UPPER_32_BITS(n) ((n) >> 32)
#define BITS_PER_LONG    64

#else

#define BITS_ALL         0xFFFFFFFFU
#define UPPER_32_BITS(n) 0U
#define BITS_PER_LONG    32

#endif

#define LOWER_32_BITS(n)        ((u32)(n))

#define BIT(n)                  (1ULL << (n))

/**************************** Type Definitions *******************************/

/*****************************************************************************
 * DESCRIPTION
 *  Returns the width of the specified bit-field.
 * ARGUMENTS
 *  __bfws      a width/shift pair
 *
 *****************************************************************************/
#define FMSH_BIT_WIDTH(__bfws)  ((uint32_t)(bfw##__bfws))

/*****************************************************************************
 * DESCRIPTION
 *  Returns the offset of the specified bit-field.
 * ARGUMENTS
 *  __bfws      a width/shift pair
 *
 *****************************************************************************/
#define FMSH_BIT_OFFSET(__bfws) fls(__bfws)

/*****************************************************************************
 * DESCRIPTION
 *  Returns a mask with the bits to be addressed set and all others cleared.
 * ARGUMENTS
 *  __bfws      a width/shift pair
 *
 *****************************************************************************/
#define FMSH_BIT_MASK(__bfws)                                        \
    ((uint32_t)(((bfw##__bfws) == 32) ? 0xFFFFFFFF                   \
                                      : ((1U << (bfw##__bfws)) - 1)) \
     << (bfo##__bfws))

/*****************************************************************************
 * DESCRIPTION
 *  Clear the specified bits.
 * ARGUMENTS
 *  __datum     the word of data to be modified
 *  __bfws      a width/shift pair
 *
 *****************************************************************************/
#define FMSH_BIT_CLEAR(__datum, __bfws) \
    ((__datum) = ((uint32_t)(__datum) & ~FMSH_BIT_MASK(__bfws)))

/*****************************************************************************
 * DESCRIPTION
 *  Returns the relevant bits masked from the data word, still at their
 *  original offset.
 * ARGUMENTS
 *  __datum     the word of data to be accessed
 *  __bfws      a width/shift pair
 *
 *****************************************************************************/
#define FMSH_BIT_GET_UNSHIFTED(__datum, __bfws) \
    ((uint32_t)((__datum) & FMSH_BIT_MASK(__bfws)))

/*****************************************************************************
 * DESCRIPTION
 *  Returns the relevant bits masked from the data word shifted to bit
 *  zero (i.e. access the specifed bits from a word of data as an
 *  integer value).
 * ARGUMENTS
 *  __datum     the word of data to be accessed
 *  __bfws      a width/shift pair
 *
 *****************************************************************************/
#define FMSH_BIT_GET(__datum, __bfws) \
    ((uint32_t)(((__datum) & FMSH_BIT_MASK(__bfws)) >> (bfo##__bfws)))

/*****************************************************************************
 * DESCRIPTION
 *  Place the specified value into the specified bits of a word of data
 *  (first the data is read, and the non-specified bits are re-written).
 * ARGUMENTS
 *  __datum     the word of data to be accessed
 *  __bfws      a width/shift pair
 *  __val       the data value to be shifted into the specified bits
 *
 *****************************************************************************/
#define FMSH_BIT_SET(__datum, __bfws, __val)                      \
    ((__datum) = ((uint32_t)(__datum) & ~FMSH_BIT_MASK(__bfws)) | \
                 ((__val << (bfo##__bfws)) & FMSH_BIT_MASK(__bfws)))

/*****************************************************************************
 * DESCRIPTION
 *  Place the specified value into the specified bits of a word of data
 *  without reading first - for sensitive interrupt type registers
 * ARGUMENTS
 *  __datum     the word of data to be accessed
 *  __bfws      a width/shift pair
 *  __val       the data value to be shifted into the specified bits
 *
 *****************************************************************************/
#define FMSH_BIT_SET_NOREAD(__datum, __bfws, __val)       \
    ((uint32_t)((__datum) = (((__val) << (bfo##__bfws)) & \
                             FMSH_BIT_MASK(__bfws))))

/*****************************************************************************
 * DESCRIPTION
 *  Shift the specified value into the desired bits.
 * ARGUMENTS
 *  __bfws      a width/shift pair
 *  __val       the data value to be shifted into the specified bits
 *
 *****************************************************************************/
#define FMSH_BIT_BUILD(__bfws, __val) \
    ((uint32_t)(((__val) << (bfo##__bfws)) & FMSH_BIT_MASK(__bfws)))

#ifdef __cplusplus
}
#endif

/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h. For example
 * GENMASK_ULL(39, 21) gives us the 64bit vector 0x000000ffffe00000.
 */
#define GENMASK(h, l) \
    (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

//#define GENMASK_32(h, l)                                                \
//	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (32 - 1 - (h))))

#define GENMASK_32 GENMASK
#define GENMASK_64 GENMASK
//#define GENMASK_64(h, l)                                                \
//	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (64 - 1 - (h))))

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 mask_generate(u32 bit_no);

#endif /* #ifndef _FMSH_COMMON_BITOPS_H_ */
