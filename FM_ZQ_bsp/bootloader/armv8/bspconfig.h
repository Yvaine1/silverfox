#ifndef _BSPCONFIG_H_ /* prevent circular inclusions */
#define _BSPCONFIG_H_ /* by using protection macros */

#include "psu_init.h"

#define CORTEX_A53
#define CONFIG_ARM_ERRATA_855873 (1)

/*********************************************************************
 *
 *  Suppress warning
 *
 *********************************************************************/
#pragma diag_suppress=Pe188
#pragma diag_suppress=Pa082
     
/*********************************************************************
 *
 *  USE_NO_SEMIHOST should be used if dont want to print on Terminator IO
 *  reference file "iar_retarget.c"
 *
 *********************************************************************/
#define USE_NO_SEMIHOST          (0)

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
#define LOG_LEVEL                LOG_LEVEL_INFO

/*********************************************************************
 *
 *  ELx_LIVE should be set to 1 if corresponding level is used.
 *
 *********************************************************************/
#define EL3_LIVE                 (1)  // always live
#define EL2_LIVE                 (0)
#define EL1_LIVE                 (0)
#define EL0_LIVE                 (0)
#define SECURE_MODE              (0)  // only effective for EL1/EL0

/*********************************************************************
 *  DCACHE_ENABLE should be set to 1 if using dcache
 *  LWIP_CACHE_DISABLE is only used in LWIP example
 *  DCACHE_ENABLE_EARLY only valid when DCACHE_ENABLE is set to 1,
 *  and dcache and mmu are enabled before main
 *********************************************************************/
#define DCACHE_ENABLE            (1)

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
 *
 *********************************************************************/
#ifndef __FSBL__
#ifndef PS_PREINITED
#define PS_PREINITED (1)
#endif
#endif

#ifndef USE_DDR
#define USE_DDR (0)
#endif

/*********************************************************************
 *  calculate DDR memory size
 *********************************************************************/
#ifndef DDR_SIZE_MB
#define DDR_SIZE_MB (DDR_SIZE >> 20)
#endif

#if (DDR_SIZE_MB <= 2048)
#define DDR_SIZE_MB_LOW  DDR_SIZE_MB
#define DDR_SIZE_MB_HIGH (0)
#else
#define DDR_SIZE_MB_LOW  (2048)
#define DDR_SIZE_MB_HIGH (DDR_SIZE_MB - DDR_SIZE_MB_LOW)
#endif

#endif /*end of __BSPCONFIG_H_*/
