/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  bspconfig.h
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
 * 2.00  hzq 22/11/18    first release
 *
 *</pre>
 ******************************************************************************/
#ifndef _BSPCONFIG_H_ /* prevent circular inclusions */
#define _BSPCONFIG_H_ /* by using protection macros */

#include "psu_init.h"

#ifndef CORTEX_R5
#define CORTEX_R5
#endif

/*********************************************************************
 *
 *  Suppress warning
 *
 *********************************************************************/
#pragma diag_suppress=Pe1053
#pragma diag_suppress=Pa082

/*********************************************************************
 *
 *  FMSH_NASSERT should be used if ASSERT is not desired
 *
 *********************************************************************/
// #define FMSH_NASSERT

/*********************************************************************
 *
 *  USE_NO_SEMIHOST should be used if print on Terminator IO is not desired
 *
 *********************************************************************/
// #define USE_NO_SEMIHOST

/*********************************************************************
 *
 *  Any message whose level is higher or equal to LOG_LEVEL is allowed
 *  to print.
 *  Following items are acceptable.
 *
 *  LOG_LEVEL_DEBUG
 *  LOG_LEVEL_INFO
 *  LOG_LEVEL_WARNING
 *  LOG_LEVEL_ERROR
 *  LOG_LEVEL_FATAL
 *
 *********************************************************************/
#define LOG_LEVEL     LOG_LEVEL_INFO

/*********************************************************************
 *
 *  DCACHE_ENABLE should be set to 1 if using dcache
 *  LWIP_CACHE_DISABLE is only used in LWIP example
 *  DCACHE_ENABLE_EARLY only valid when DCACHE_ENABLE is set to 1,
 *  and dcache and mmu are enabled before main
 *********************************************************************/
#define DCACHE_ENABLE (1)

#define LWIP_CACHE_DISABLE

#if (DCACHE_ENABLE == 1)
#define DCACHE_ENABLE_EARLY (1)
#endif

/*********************************************************************
 *
 *  PS_PREINITED should be set to 1 if ps is pre-inited
 *  in fsbl or ps_init.mac..., otherwise this value should keep 0
 *  USE_DDR is only valid if PS_PREINITED is 1, if ddr is not used,
 *  this value should keep 0
 *********************************************************************/
#ifndef __FSBL__
#ifndef PS_PREINITED
#define PS_PREINITED (1)
#endif
#endif

#ifndef USE_DDR
#define USE_DDR (0)
#endif

#ifndef DDR_SIZE_MB
#define DDR_SIZE_MB (DDR_SIZE >> 20)
#endif

/*********************************************************************
 *
 *  Cortex-R5 Configuration
 *
 *********************************************************************/

// #define VEC_TABLE_IN_OCM
#define PMU_ACCESS_USER
// #define CYCLECNT_GEN_DELAY
#define VFP_ENABLE (1)
#define TCM_ENABLE (1)
#define MPU_ENABLE (1)

#endif /*end of _BSPCONFIG_H_*/
