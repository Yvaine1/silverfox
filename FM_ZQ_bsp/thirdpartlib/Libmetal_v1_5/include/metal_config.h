/*
 * Copyright (c) 2015, FMSH Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	config.h
 * @brief	Generated configuration settings for libmetal.
 */

#ifndef __METAL_PLATFORM_CONFIG__H__
#define __METAL_PLATFORM_CONFIG__H__

#ifdef __cplusplus
extern "C" {
#endif

#define __BAREMETAL__
#define FMSH_PLATFORM
#define DEFAULT_LOGGER_ON

/* Configure System type (linux, generic, freertos, ...) */
#define SYSTEM_GENERIC
// #define SYSTEM_FREERTOS
/** System type (linux, generic, freertos, ...). */
#ifdef SYSTEM_FREERTOS
#define METAL_SYSTEM		"freertos"
#define METAL_SYSTEM_FREERTOS
#define METAL_SYS_DIR		 freertos
#elif defined(SYSTEM_GENERIC)
#define METAL_SYSTEM		"generic"
#define METAL_SYSTEM_GENERIC
#define METAL_SYS_DIR		 generic
#endif
/* End configure System type */
  

/* Configure Processor type (arm, aarch64, ...) */
#define PROCESSOR_ARM
// #define PROCESSOR_AARCH64
/** Processor type (arm, aarch64, ...). */
#ifdef PROCESSOR_ARM
#define METAL_PROCESSOR		"arm"
#define METAL_PROCESSOR_ARM
  
/** Machine type (fmzq_r5, fmzq_a53, ...). */
#define METAL_MACHINE		"fmzq_r5"
#define METAL_MACHINE_FMZQ_R5
  
#elif defined(PROCESSOR_AARCH64)
  
#define METAL_PROCESSOR		"aarch64"
#define METAL_PROCESSOR_AARCH64
  
/** Machine type (fmzq_r5, fmzq_a53, ...). */
#define METAL_MACHINE		"fmzq_a53"
#define METAL_MACHINE_FMZQ_A53
#endif
/* End configure System type */

#define HAVE_STDATOMIC_H
/* #undef HAVE_FUTEX_H */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_CONFIG__H__ */
