/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_hw.h
 *
 * This file contains header bspconfig.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  12/28/2023  First Release.
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_HW_H_
#define _FMSH_HW_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/* IPI Base Address */
#define IPI_BASEADDR                           0XFF300000U

/* Register: IPI_PMU_0_TRIG */
#define IPI_PMU_0_TRIG                         ((IPI_BASEADDR) + 0X00030000U)
#define IPI_PMU_0_TRIG_PMU_0_MASK              0X00010000U

#define IPI_PMU_0_IER                          ((IPI_BASEADDR) + 0X00030018U)
#define IPI_PMU_0_IER_PMU_0_MASK               0X00010000U

/* AMS_PS_SYSMON Base Address */
#define AMS_PS_SYSMON_BASEADDR                 0XFFA50800U
#define FSBL_PS_SYSMON_CONFIGREG1              0XFFA50904U
#define FSBL_PS_SYSMON_CONFIGREG3              0XFFA5090CU
#define FSBL_PS_SYSMON_CFGREG1_ALRM_DISBL_MASK 0X0F0EU
#define FSBL_PS_SYSMON_CFGREG3_ALRM_DISBL_MASK 0X3FU

/* Register: AMS_PS_SYSMON_ANALOG_BUS */
#define AMS_PS_SYSMON_ANALOG_BUS               ((AMS_PS_SYSMON_BASEADDR) + 0X00000114U)

/* rpu */

/**
 * RPU Base Address
 */
#define RPU_BASEADDR                           0XFF9A0000U

/**
 * Register: RPU_RPU_GLBL_CNTL
 */
#define RPU_RPU_GLBL_CNTL                      ((RPU_BASEADDR) + 0X00000000U)
#define RPU_RPU_GLBL_CNTL_SLSPLIT_MASK         0X00000008U
#define RPU_RPU_GLBL_CNTL_TCM_COMB_MASK        0X00000040U
#define RPU_RPU_GLBL_CNTL_SLCLAMP_MASK         0X00000010U

/**
 * Register: RPU_RPU_0_CFG
 */
#define RPU_RPU_0_CFG                          ((RPU_BASEADDR) + 0X00000100U)
#define RPU_RPU_0_CFG_VINITHI_SHIFT            2U
#define RPU_RPU_0_CFG_VINITHI_MASK             0x00000004U
#define RPU_RPU_0_CFG_NCPUHALT_MASK            0X00000001U

/**
 * Register: RPU_RPU_1_CFG
 */
#define RPU_RPU_1_CFG                          ((RPU_BASEADDR) + 0X00000200U)
#define RPU_RPU_1_CFG_VINITHI_SHIFT            2U
#define RPU_RPU_1_CFG_VINITHI_MASK             0x00000004U
#define RPU_RPU_1_CFG_NCPUHALT_MASK            0X00000001U

/* csu */

/**
 * CSU Base Address
 */
#define SAC_BASEADDR                           0XFFCA0000U
#define SAC_CFG_REG                            ((SAC_BASEADDR) + 0X00000008U)
#define SAC_PROG_B_MASK                        (0X80000000U)
#define SAC_MULTIBOOT_EN_MASK                  (0x40000000U)
#define SAC_MULTIBOOT_EN_SHIFT                 30U
#define SAC_SECURE_MODE_MASK                   0x80
#define SAC_STATUS_REG                         ((SAC_BASEADDR) + 0X00000018U)
#define SAC_STATUS_PCFG_DONE_MASK              (0x00000200U)
#define SAC_MULTI_BOOT_REG                     ((SAC_BASEADDR) + 0X00000038U)
#define SAC_APU_EXCUTION_ADDR                  ((SAC_BASEADDR) + 0x000000D0U)
#define SAC_EFUSE_SEC_CTRL                     ((SAC_BASEADDR) + 0x254U)
#define SAC_EFUSE_SEC_CTRL_RSA_EN_MASK         0x00007fffU

#define SAC_EFUSE_SECURE_BOOT_EN               ((SAC_BASEADDR) + 0x32c)
#define SAC_SECURE_BOOT_EN_MASK                (0x7FFF0000U)

/* crf_apb */

/**
 * CRF_APB Base Address
 */
#define CRF_APB_BASEADDR                       0XFD1A0000U

/**
 * Register: CRF_APB_RST_FPD_APU
 */
#define CRF_APB_RST_FPD_APU                    ((CRF_APB_BASEADDR) + 0X00000104U)
#define CRF_APB_RST_FPD_APU_ACPU0_RESET_MASK   (u32)0X00000001U
#define CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK  (u32)0X00000100U

/**
 * Register: CRF_APB_ACPU_CTRL
 */
#define CRF_APB_ACPU_CTRL                      ((CRF_APB_BASEADDR) + 0X00000060U)
#ifndef CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK
#define CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK 0X01000000U
#endif
#ifndef CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK
#define CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK 0X02000000U
#endif
#define CRF_APB_RST_FPD_APU_ACPU1_RESET_MASK       (u32)0X00000002U
#define CRF_APB_RST_FPD_APU_ACPU2_RESET_MASK       (u32)0X00000004U
#define CRF_APB_RST_FPD_APU_ACPU3_RESET_MASK       (u32)0X00000008U

#define CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK (u32)0X00002000U
#define CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK (u32)0X00001000U
#define CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK (u32)0X00000800U
#define CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK (u32)0X00000400U

/* crl_apb */

/**
 * CRL_APB Base Address
 */
#define CRL_APB_BASEADDR                           0XFF5E0000U

/* Register: CRL_APB_CPU_R5_CTRL */
#undef CRL_APB_CPU_R5_CTRL
#define CRL_APB_CPU_R5_CTRL ((CRL_APB_BASEADDR) + 0X00000090U)
#undef CRL_APB_CPU_R5_CTRL_CLKACT_MASK
#define CRL_APB_CPU_R5_CTRL_CLKACT_MASK            0X01000000U

/* Register: CRL_APB_BOOT_MODE_USER */
#define CRL_APB_BOOT_MODE_USER                     ((CRL_APB_BASEADDR) + 0X00000200U)
#define CRL_APB_BOOT_MODE_USER_BOOT_MODE_MASK      0X0000000FU
/* Register: CRL_APB_RESET_REASON */
#define CRL_APB_RESET_REASON                       ((CRL_APB_BASEADDR) + 0X00000220U)
#define CRL_APB_RESET_REASON_PMU_SYS_RESET_MASK    0X00000004U
#define CRL_APB_RESET_REASON_PSONLY_RESET_REQ_MASK 0x00000008U
/* Register: CRL_APB_RST_LPD_TOP */
#define CRL_APB_RST_LPD_TOP                        ((CRL_APB_BASEADDR) + 0X0000023CU)
#define CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK     (u32)0X00000001U
#define CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK    (u32)0X00000004U
#define CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK     (u32)0X00000002U
/**
 * Register: CRL_APB_RESET_CTRL
 */
#define CRL_APB_RESET_CTRL                         ((CRL_APB_BASEADDR) + 0X00000218U)
#define CRL_APB_RESET_CTRL_SOFT_RESET_MASK         0X00000010U

/* apu */

/**
 * APU Base Address
 */
#define APU_BASEADDR                               0XFD5C0000U

/**
 * Register: APU_CONFIG_0
 */
#define APU_CONFIG_0                               ((APU_BASEADDR) + 0X00000020U)
#define APU_CONFIG_0_VINITHI_MASK                  0x000000F0
#define APU_CONFIG_0_AA64N32_MASK_CPU0             (0x1U)
#define APU_CONFIG_0_AA64N32_MASK_CPU1             (0x2U)
#define APU_CONFIG_0_AA64N32_MASK_CPU2             (0x4U)
#define APU_CONFIG_0_AA64N32_MASK_CPU3             (0x8U)
#define APU_CONFIG_0_VINITHI_MASK_CPU0             (u32)(0x100U)
#define APU_CONFIG_0_VINITHI_MASK_CPU1             (u32)(0x200U)
#define APU_CONFIG_0_VINITHI_MASK_CPU2             (u32)(0x400U)
#define APU_CONFIG_0_VINITHI_MASK_CPU3             (u32)(0x800U)
#define APU_CONFIG_0_VINITHI_SHIFT_CPU0            (8U)
#define APU_CONFIG_0_VINITHI_SHIFT_CPU1            (9U)
#define APU_CONFIG_0_VINITHI_SHIFT_CPU2            (10U)
#define APU_CONFIG_0_VINITHI_SHIFT_CPU3            (11U)

/**
 * Register: APU_RVBARADDR0L
 */
#define APU_RVBARADDR0L                            ((APU_BASEADDR) + 0X00000040U)

/**
 * Register: APU_RVBARADDR0H
 */
#define APU_RVBARADDR0H                            ((APU_BASEADDR) + 0X00000044U)

/**
 * Register: APU_RVBARADDR1L
 */
#define APU_RVBARADDR1L                            ((APU_BASEADDR) + 0X00000048U)

/**
 * Register: APU_RVBARADDR1H
 */
#define APU_RVBARADDR1H                            ((APU_BASEADDR) + 0X0000004CU)

/**
 * Register: APU_RVBARADDR2L
 */
#define APU_RVBARADDR2L                            ((APU_BASEADDR) + 0X00000050U)

/**
 * Register: APU_RVBARADDR2H
 */
#define APU_RVBARADDR2H                            ((APU_BASEADDR) + 0X00000054U)

/**
 * Register: APU_RVBARADDR3L
 */
#define APU_RVBARADDR3L                            ((APU_BASEADDR) + 0X00000058U)

/**
 * Register: APU_RVBARADDR3H
 */
#define APU_RVBARADDR3H                            ((APU_BASEADDR) + 0X0000005CU)

/* pmu_global */

/**
 * PMU_GLOBAL Base Address
 */
#define PMU_GLOBAL_BASEADDR                        0XFFD80000U

#define PMU_GLOBAL_GLOBAL_CNTRL                    ((PMU_GLOBAL_BASEADDR) + 0X00000000U)
#define PMU_GLOBAL_GLOBAL_CNTRL_MB_SLEEP_MASK      0X00010000U
#define PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK 0X00000010U

#define PMU_GLOBAL_PS_CNTRL                        ((PMU_GLOBAL_BASEADDR) + 0x4U)
#define PMU_GLOBAL_PS_CNTRL_PROG_ENABLE_MASK       0x2U
#define PMU_GLOBAL_PS_CNTRL_PROG_GATE_MASK         0x1U

#define FSBL_EXEC_COMPLETED                        (0x1U)
#define PMU_GLOBAL_GLOB_GEN_STORAGE1               ((PMU_GLOBAL_BASEADDR) + 0X34U)
#define PMU_GLOBAL_GLOB_GEN_STORAGE2               ((PMU_GLOBAL_BASEADDR) + 0X38U)
#define PMU_GLOBAL_GLOB_GEN_STORAGE4               ((PMU_GLOBAL_BASEADDR) + 0X40U)
#define PMU_GLOBAL_GLOB_GEN_STORAGE5               ((PMU_GLOBAL_BASEADDR) + 0x44U)
#define PMU_GLOBAL_GLOB_GEN_STORAGE6               ((PMU_GLOBAL_BASEADDR) + 0X48U)
/**
 * Register: PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4
 */
#define PMU_GLOBAL_PERS_GLOB_GEN_STORAGE4          ((PMU_GLOBAL_BASEADDR) + 0X00000060U)

/**
 * Register: PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5
 */
#define PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5          ((PMU_GLOBAL_BASEADDR) + 0X00000064U)

/*
 * Register: PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7
 */
#define PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7          ((PMU_GLOBAL_BASEADDR) + 0X0000006CU)
/* Register: PMU_GLOBAL_PWR_STATE */
#define PMU_GLOBAL_PWR_STATE                       (0X00000100U)
#define PMU_GLOBAL_PWR_STATE_PL_MASK               0X00800000U
#define PMU_GLOBAL_PWR_STATE_FP_MASK               0X00400000U
#define PMU_GLOBAL_PWR_STATE_USB1_MASK             0X00200000U
#define PMU_GLOBAL_PWR_STATE_USB0_MASK             0X00100000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK3_MASK        0X00080000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK2_MASK        0X00040000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK1_MASK        0X00020000U
#define PMU_GLOBAL_PWR_STATE_OCM_BANK0_MASK        0X00010000U
#define PMU_GLOBAL_PWR_STATE_TCM1B_MASK            (u32)0X00008000U
#define PMU_GLOBAL_PWR_STATE_TCM1A_MASK            (u32)0X00004000U
#define PMU_GLOBAL_PWR_STATE_TCM0B_MASK            (u32)0X00002000U
#define PMU_GLOBAL_PWR_STATE_TCM0A_MASK            (u32)0X00001000U
#define PMU_GLOBAL_PWR_STATE_R5_1_MASK             (u32)0X00000800U
#define PMU_GLOBAL_PWR_STATE_R5_0_MASK             (u32)0X00000400U
#define PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK         0X00000080U
#define PMU_GLOBAL_PWR_STATE_PP1_MASK              0X00000020U
#define PMU_GLOBAL_PWR_STATE_PP0_MASK              0X00000010U
#define PMU_GLOBAL_PWR_STATE_ACPU3_MASK            0X00000008U
#define PMU_GLOBAL_PWR_STATE_ACPU2_MASK            0X00000004U
#define PMU_GLOBAL_PWR_STATE_ACPU1_MASK            0X00000002U
#define PMU_GLOBAL_PWR_STATE_ACPU0_MASK            0X00000001U

/* Register: PMU_GLOBAL_REQ_PWRUP_STATUS */
#define PMU_GLOBAL_REQ_PWRUP_STATUS                ((PMU_GLOBAL_BASEADDR) + 0X00000110U)
#undef PMU_GLOBAL_REQ_PWRUP_STATUS_PL_SHIFT
#define PMU_GLOBAL_REQ_PWRUP_STATUS_PL_SHIFT 23U
#undef PMU_GLOBAL_REQ_PWRUP_STATUS_PL_MASK
#define PMU_GLOBAL_REQ_PWRUP_STATUS_PL_MASK 0X00800000U

/* Register: PMU_GLOBAL_REQ_PWRUP_INT_EN */
#define PMU_GLOBAL_REQ_PWRUP_INT_EN         ((PMU_GLOBAL_BASEADDR) + 0X00000118U)
#undef PMU_GLOBAL_REQ_PWRUP_INT_EN_PL_MASK
#define PMU_GLOBAL_REQ_PWRUP_INT_EN_PL_MASK 0X00800000U

/* Register: PMU_GLOBAL_REQ_PWRUP_TRIG */
#define PMU_GLOBAL_REQ_PWRUP_TRIG           ((PMU_GLOBAL_BASEADDR) + 0X00000120U)
#undef PMU_GLOBAL_REQ_PWRUP_TRIG_PL_MASK
#define PMU_GLOBAL_REQ_PWRUP_TRIG_PL_MASK 0X00800000U

/*
 * For DDR status PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7 is used.
 */
#define FSBL_DDR_STATUS_REGISTER_OFFSET		(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE7)
#define DDRC_INIT_FLAG_MASK		            0x00000010U
#define DDR_STATUS_FLAG_MASK		        0x00000004U
    
    
/**
 * ARM Processor defines
 */
#define FSBL_CLUSTER_ID_MASK              (0x00000F00U)
#define FSBL_A53_PROCESSOR                (0x00000000U)
#define FSBL_R5_PROCESSOR                 (0x00000100U)

/**
 * TCM address for R5
 */
#define FSBL_R5_TCM_START_ADDRESS         (u32)(0x0U)

#define FSBL_R50_HIGH_ATCM_START_ADDRESS  (0xFFE00000U)
#define FSBL_R50_HIGH_BTCM_START_ADDRESS  (0xFFE20000U)
#define FSBL_R51_HIGH_ATCM_START_ADDRESS  (0xFFE90000U)
#define FSBL_R51_HIGH_BTCM_START_ADDRESS  (0xFFEB0000U)

#define FSBL_R5_TCM_BANK_LENGTH           (0x10000U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
