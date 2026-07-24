///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_types.h
// PROJECT : Generic
// KEYWORDS:
// LANGUAGE: C
// INTELLIPROP AUTHOR  : ehanke
// CREATED : 8/25/2014
//
// DESCRIPTION:
//
// TESTS USED/CREATED:
//
// REVISION HISTORY: Rev1.0
// Date     Person      Description
// -------- ----------- -------------------------------------------------------
//
// CURRENT ISSUES: none.
//
// REMAINING WORK:
//
//
// This media contains an authorized copy or copies of material owned by
// Intelliprop Inc.  This ownership notice and any
// other notices included in machine readable copies must be reproduced on all
// authorized copies.
//
// This is confidential and unpublished property of Intelliprop Inc.
//
// All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __IPROP_TYPES_H__
#define __IPROP_TYPES_H__
#include "stdint.h"

/* this is a special 64bit data type that is 8-byte aligned */
#define aligned_u64  u64 __attribute__((aligned(8)))
#define aligned_be64 __be64 __attribute__((aligned(8)))
#define aligned_le64 __le64 __attribute__((aligned(8)))

// #ifdef __CHECKER__
// #define __bitwise__ __attribute__((bitwise))
// #else
// #define __bitwise__
// #endif
// #ifdef __CHECK_ENDIAN__
// #define __bitwise __bitwise__
// #else
#define __bitwise
// #endif

typedef uint8_t u8;    ///< Data type of 1 byte.
typedef uint16_t u16;  ///< Data type of 2 bytes.
typedef uint32_t u32;  ///< Data type of 4 bytes.
typedef uint64_t u64;  ///< Data type of 8 bytes.

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned long __u32;
typedef unsigned long long __u64;

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
//
typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;
/*
 * aligned_u64 should be used in defining kernel<->userspace ABIs to avoid
 * common 32/64-bit compat problems.
 * 64-bit values align to 4-byte boundaries on x86_32 (and possibly other
 * architectures) and to 8-byte boundaries on 64-bit architetures.  The new
 * aligned_64 type enforces 8-byte alignment so that structs containing
 * aligned_64 values have the same alignment on 32-bit and 64-bit architectures.
 * No conversions are necessary between 32-bit user-space and a 64-bit kernel.
 */

#define __aligned_u64  u64 __attribute__((aligned(8)))
#define __aligned_be64 __be64 __attribute__((aligned(8)))
#define __aligned_le64 __le64 __attribute__((aligned(8)))

#endif /* __IPROP_TYPES_H__ */
