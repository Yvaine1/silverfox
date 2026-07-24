/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
*
* @file  fmsh_cci_hw.h
*
* This file contains
*
* @note		CCI driver for FMZQ MPSoC.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   zyh  03/29/2024  First Release

*</pre>
******************************************************************************/

#ifndef FMSH_CCI_HW_H /* prevent circular inclusions */
#define FMSH_CCI_HW_H /* by using protection macros */

/**
 * Define IO functions
 */
#include "fmsh_common_io.h"
#include "fmsh_psu_parameters.h"
#define FCciPs_ReadReg  FMSH_ReadReg

#define FCciPs_WriteReg FMSH_WriteReg

#define FCciPs_MaskWrite(BaseAddress, RegOffset, Data, Mask) \
    FCciPs_WriteReg(                                         \
        (BaseAddress), (RegOffset),                          \
        ((FCciPs_ReadReg(BaseAddress, RegOffset) & ~(Mask)) | Data))

/**
 * Register: ctrl_ovr
 */
#define FCCI_CTRL_OVR_OFFSET          0x00000000U
#define FCCI_CTRL_OVR_SNOOP_MASK      0x1
/**
 * Register: secr_acc
 */
#define FCCI_SECR_ACC_OFFSET          0x00000008U

/**
 * Register: status
 */
#define FCCI_STATUS_OFFSET            0x0000000CU
#define FCCI_STATUS_PENDING_MASK      0x00103
// TODO

/**
 * Register: pmu_ctrl
 */
#define FCCI_PMU_CTRL_OFFSET          0x00000100U

#define FCCI_PMU_CTRL_CEN_SHIFT       0
#define FCCI_PMU_CTRL_CEN_MASK        0x1
#define FCCI_PMU_CTRL_CEN_ENABLE      0x1
#define FCCI_PMU_CTRL_CEN_DISABLE     0x0

#define FCCI_PMU_CTRL_RST_SHIFT       1
#define FCCI_PMU_CTRL_RST_MASK        0x2
#define FCCI_PMU_CTRL_RST_RESET       0x1

#define FCCI_DBG_CTRL_OFFSET          0x00000104U
#define FCCI_DBG_CTRL_ENABLE          1
#define FCCI_DBG_CTRL_DISABLE         0
/*                 Slave interface registers                */

/**
 * Register: snoop_ctrl
 */
#define FCCI_SNOOP_CTRL_OFFSET(x)     (0x00001000U + 0x01000 * x)

// ACE features
#define FCCI_SNOOP_CTRL_DVM_ENABLE    (1 << 1)
#define FCCI_SNOOP_CTRL_SNOOP_ENABLE  (1 << 0)
#define FCCI_SNOOP_CTRL_DVM_DISABLE   (0 << 1)
#define FCCI_SNOOP_CTRL_SNOOP_DISABLE (0 << 0)

/*
 * Register: share_ovr
 */
#define FCCI_SHARE_OVR_OFFSET(x)      (0x00001004U + 0x01000 * x)

/*
 * Register: arqos_ovr
 */
#define FCCI_ARQOS_OVR_OFFSET(x)      (0x00001100U + 0x01000 * x)

/*
 * Register: awqos_ovr
 */
#define FCCI_AWQOS_OVR_OFFSET(x)      (0x00001104U + 0x01000 * x)

/*
 * Register: qos_max_ot
 */
#define FCCI_QOS_MAX_OT_OFFSET(x)     (0x00001110U + 0x01000 * x)

/*              Performance counter registers                */

/*
 * Register: evnt_sel
 */
#define FCCI_EVNT_SEL_OFFSET(x)       (0x00010000U + 0x10000 * x)

/*
 * Register: ecnt_data
 */
#define FCCI_ECNT_DATA_OFFSET(x)      (0x00010004U + 0x10000 * x)

/*
 * Register: ecnt_ctrl
 */
#define FCCI_ECNT_CTRL_OFFSET(x)      (0x00010008U + 0x10000 * x)
#define FCCI_ECNT_CTRL_COUNT_ENABLE   1
#define FCCI_ECNT_CTRL_COUNT_DISABLE  0
/**
 * Register: ecnt_clr_ovfl
 */
#define FCCI_ECNT_CLR_OVFL_OFFSET(x)  (0x0001000CU + 0x10000 * x)

#define FCCI_SLAVE_MONITOR_OFFSET(x)  (0x90000U + 0x4 * x)
#define FCCI_MASTER_MONITOR_OFFSET(x) (0x90100U + 0x4 * x)

#define CCI_SLCR_BASEADDR             0xFD5E0000
#define CCI_SLCR_ISR                  0x10
#define CCI_SLCR_IMR                  0x14
#define CCI_SLCR_IER                  0x18
#define CCI_SLCR_IDR                  0x1C
#define CCI_SLCR_MISC_DEBUG           0x40

#endif /* end of protection macro */
