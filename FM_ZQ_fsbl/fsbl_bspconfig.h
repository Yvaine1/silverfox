#ifndef _BSPCONFIG_H_ /* prevent circular inclusions */
#define _BSPCONFIG_H_ /* by using protection macros */

#define CORTEX_A53
#define CONFIG_ARM_ERRATA_855873 (1)

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
 *
 *********************************************************************/
#define LOG_LEVEL                LOG_LEVEL_DEBUG

/*********************************************************************
 *
 *  ELx_LIVE should be set to 1 if corresponding level is used.
 *
 *********************************************************************/
#define EL3_LIVE                 (1)  // always live
#define EL2_LIVE                 (0)
#define EL1_LIVE                 (1)
#define EL0_LIVE                 (1)
#define SECURE_MODE              (0)  // only effective for EL1/EL0

/*********************************************************************
 *  DCACHE_ENABLE should be set to 1 if using dcache before enter main
 *********************************************************************/
#define DCACHE_ENABLE_EARLY      (1)

/*********************************************************************
 *  DCACHE_ENABLE should be set to 1 if using dcache
 *********************************************************************/
#define DCACHE_ENABLE            (0)

#define LWIP_CACHE_DISABLE

/*********************************************************************
 *
 *  PS_PREINITED should be set to 1 if ps is pre-inited
 *  in fsbl or ps_init.mac..., otherwise this value should keep 0
 *  USE_DDR is only valid if PS_PREINITED is 1, if ddr is not used,
 *  this value should keep 0
 *
 *********************************************************************/
#define PS_PREINITED (0)
#define USE_DDR      (1)

/*********************************************************************
 *  calculate DDR memory size
 *********************************************************************/
#ifndef DDR_SIZE_MB
#define DDR_SIZE_MB (2048)
#endif

#if (DDR_SIZE_MB <= 2048)
#define DDR_SIZE_MB_LOW  DDR_SIZE_MB
#define DDR_SIZE_MB_HIGH (0)
#else
#define DDR_SIZE_MB_LOW  (2048)
#define DDR_SIZE_MB_HIGH (DDR_SIZE_MB - DDR_SIZE_MB_LOW)
#endif

#endif /*end of __BSPCONFIG_H_*/
