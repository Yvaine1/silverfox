/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common_io.h
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
#ifndef _FMSH_COMMON_IO_H_
#define _FMSH_COMMON_IO_H_

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/

#include "stdio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

#define FMSH_ReadReg(baseAddr, offSet) \
    (*((volatile u32 *)((uintptr_t)baseAddr + offSet)))
#define FMSH_WriteReg(baseAddr, offSet, data) \
    (*((volatile u32 *)((uintptr_t)baseAddr + offSet))) = ((u32)data)

#define FMSH_ReadReg64(baseAddr, offSet) \
    (*((volatile unsigned long long *)((intptr_t)baseAddr + offSet)))
#define FMSH_WriteReg64(baseAddr, offSet, data)             \
    (*((volatile unsigned long long *)((intptr_t)baseAddr + \
                                       offSet))) = (unsigned long long)(data)

/* the following macro performs an 8-bit read */
#define FMSH_IN8_8(p)           (uint8_t) * ((volatile uint8_t *)(p))
/* the following macro performs an 8-bit write */
#define FMSH_OUT8_8(v, p)       *((volatile uint8_t *)(p)) = (uint8_t)(v)

/* the following macro performs a 16-bit read */
#define FMSH_IN16_16(p)         (uint16_t) * ((volatile uint16_t *)(p))
/* the following macro performs a 16-bit write */
#define FMSH_OUT16_16(v, p)     *((volatile uint16_t *)(p)) = (uint16_t)(v)

/* the following macro performs a 32-bit reads */
#define FMSH_IN32_32(p)         (uint32_t) * ((uint32_t *)(p))
/* the following macro performs a 32-bit write */
#define FMSH_OUT32_32(v, p)     *((volatile uint32_t *)(p)) = (v)

/* the following macro performs a 32-bit reads for can ip*/
#define FMSH_CAN_IN32_32(p)     (uint32_t) * ((uint32_t *)(&p))
/* the following macro performs a 32-bit write for can ip*/
#define FMSH_CAN_OUT32_32(v, p) *((volatile uint32_t *)(&p)) = (v)

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n)        ((u32)(((n) >> 16) >> 16))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n)        ((u32)(n))

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FMSH_COMMON_IO_H_ */
