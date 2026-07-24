/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common.h
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
#ifndef _FMSH_COMMON_H_
#define _FMSH_COMMON_H_

/***************************** Include Files *********************************/

#include <inttypes.h>            // defined-width data types
#include <stdarg.h>              // variable-length argument support
#include <stddef.h>              // standard definitions

#include "fmsh_common_bitops.h"  // bit-manipulation macros
#include "fmsh_common_dbc.h"     // assertion macros
#include "fmsh_common_delay.h"   // device info
#include "fmsh_common_dev.h"     // assertion macros
#include "fmsh_common_io.h"      // low-level I/O
#include "fmsh_common_list.h"    // linked list macros
#include "fmsh_common_status.h"  // status codes
#include "fmsh_common_types.h"   // custom data type definitions
#include "fmsh_pseudo_asm.h"

#if defined(CORTEX_A53)

#include "armv8/cortexa53.h"
#include "armv8/fmsh_cache.h"
#include "armv8/fmsh_mmu.h"

#elif defined(CORTEX_R5)

#include "cortexr5/cortexr5.h"
#include "cortexr5/fmsh_cache.h"
#include "cortexr5/fmsh_mpu.h"
#include "cortexr5/fmsh_pmu.h"

#endif

/************************** Constant Definitions *****************************/
#define ROUND_UP(a, b)     ((a + b - 1) & ~(b - 1))
#define ALIGN_UP(x, align) ((x + align - 1) & ~(align - 1))

#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size) \
    ALLOC_ALIGN_BUFFER(type, name, size, DCACHE_LINE_SIZE)

#define ALLOC_ALIGN_BUFFER(type, name, size, align)                    \
    char __##name[ROUND_UP(sizeof(type) * size, align) + (align - 1)]; \
    type *name = (type *)ALIGN_UP((uintptr_t)__##name, align);

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void fmsh_print(const char *ptr, ...);
void fmsh_print_info(const char *ptr, ...);
void fmsh_print_dbg(const char *ptr, ...);
void fmsh_print_warning(const char *ptr, ...);
void fmsh_print_err(const char *ptr, ...);
void fmsh_print_fatal(const char *ptr, ...);
int fls(int x);
u32 be_to_cpu32(u32 be);
u32 be_to_cpu16(u16 be);
u32 convert_to_align(void *data, int size);
#endif /* FMSH_COMMON_H */
