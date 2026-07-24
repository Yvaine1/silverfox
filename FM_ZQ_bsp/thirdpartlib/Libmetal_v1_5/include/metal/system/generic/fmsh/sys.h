/*
 * Copyright (c) 2024, FMSH Inc. and Contributors. All rights reserved.
 * Copyright (C) 2023 - 2024, Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/fmsh/sys.h
 * @brief	generic fmsh system primitives for libmetal.
 */

#ifndef __METAL_GENERIC_SYS__H__
#error "Include metal/sys.h instead of metal/generic/fmzq_r5/sys.h"
#endif

#include "machine_config.h"
#include <metal/assert.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <stdint.h>
#include <metal/compiler.h>
#include <metal/utilities.h>

#ifndef __METAL_GENERIC_FMSH_SYS__H__
#define __METAL_GENERIC_FMSH_SYS__H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief	metal_fmsh_irq_isr
 *
 * FMSH interrupt ISR can be registered to the FMSH 
 * IRQ controller driver.
 *
 * @param[in] arg input argument, interrupt vector id.
 */
void metal_fmsh_irq_isr(void *arg);

/**
 * @brief	metal_fmsh_irq_int
 *
 * @return 0 for success, or negative value for failure
 */
int metal_fmsh_irq_init(void);

static inline void sys_irq_enable(unsigned int vector)
{
	Machine_EnableIntr(vector);
}

static inline void sys_irq_disable(unsigned int vector)
{
	Machine_DisableIntr(vector);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_GENERIC_SYS__H__ */
