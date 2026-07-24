/*
 * Copyright (c) 2024, Machine Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	generic/fmsh/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <metal/sys.h>
#include <metal/system/generic/fmsh/sys.h>

void sys_irq_restore_enable(unsigned int flags)
{
	(void)flags;
	Machine_ExceptionEnableMask(~flags);
}

unsigned int sys_irq_save_disable(void)
{
	unsigned int state = metal_mfcpsr() & MACHINE_EXCEPTION_ALL;

	if (state != MACHINE_EXCEPTION_ALL)
		Machine_ExceptionDisableMask(MACHINE_EXCEPTION_ALL);

	return state;
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr && !len)
		Machine_DCacheFlush();
	else
		Machine_DCacheFlushRange((intptr_t)addr, len);
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr && !len)
		Machine_DCacheInvalidate();
	else
		Machine_DCacheInvalidateRange((intptr_t)addr, len);
}

/**
 * @brief poll function until some event happens
 */
void metal_weak metal_generic_default_poll(void)
{
	metal_asm volatile("wfi");
}

void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
	void *__attribute__((unused)) physaddr;

	physaddr = Machine_MemMap(pa, size, flags);
	metal_assert(physaddr == (void *)pa);

	return va;
}
