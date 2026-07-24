/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  procise_GTR_golden.c
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
 *2.2    cen         31/5/2024
 *2.3    cen         7/6/2024
 *2.4    cen         16/6/2024       dp power
 *2.5    cen         28/6/2024       pcie refclk from pma0
 *2.6    cen         3/7/2024        power state reg check
 *2.7    cen         4/9/2024        only write
 *2.8    cen        17/2/2025       pcie refclk from pma0(pcie * 4)
 *</pre>
 ******************************************************************************/
#include <math.h>

#include "fmsh_common.h"  // common header for all drivers
#include "fmsh_gtr.h"
#include "fmsh_psu_parameters.h"

#define True     1
#define FBR      0
#define DR       1
#define D32      2

#define TIME_OUT 500000

static void mask_write (u32 base_addr, u32 offset, u32 mask, u32 value)
{
    u32 val = 0;
    val = FMSH_ReadReg(base_addr, offset);
    val &= (~mask);
    val += value;
    FMSH_WriteReg(base_addr, offset, val);
}
static int check_mac (u8 lane[4], u8 mac)
{
    for (int i = 0; i < 4; i++)
    {
        if (lane[i] == mac)
        {
            return FMSH_SUCCESS;
        }
    }
    return FMSH_FAILURE;
}
static void __GTR_Init (u8 lane[4], double refclk[2], u8 ssc_en[2],
                        GTR_Regval* struct1)
{
    /***********************************************************************************/

    // GTR SLCR

    FMSH_WriteReg(GTR_SLCR, ICM_CFG0_REG_ADDR, struct1->icm_cfg0_reg);
    FMSH_WriteReg(GTR_SLCR, ICM_CFG1_REG_ADDR, struct1->icm_cfg1_reg);
    FMSH_WriteReg(GTR_SLCR, GTR_CFG_REG_ADDR, struct1->gtr_cfg_reg);
    FMSH_WriteReg(GTR_SLCR, PMA_CMN_CLK_REG_ADDR, struct1->pma_cmn_clk_reg);
    FMSH_WriteReg(GTR_SLCR, PMA_XCVR_REG_ADDR, struct1->pma_xcvr_reg);

    // phy
    FMSH_WriteReg(
        GTR_REG, PHY_PLL_CFG_ADDR,
        struct1->phy_pll_cfg);  // refclk = 100M时无需配置该寄存器，默认值即可
    FMSH_WriteReg(GTR_REG, PHY_PMA_PLL_RAW_CTRL, struct1->phy_pma_pll_raw_ctrl);
    if (struct1->pcie == True)
    {
        if( ((lane[2] == 0) && (lane[3] == 0)) || ((lane[2] == PCIE) && (lane[3] == PCIE)))
        {
            FMSH_WriteReg(GTR_REG, PHY_PMA_CMN_CTRL1,
                          0x1 << 6);  // enable cmn_refclk_rcv_out_en pma0
        }
    }
    // cmn
    if (struct1->lane_icm_cfg[0] ||
        struct1->lane_icm_cfg[1])  // lane0 或 lane1启用，即pma1启用
    {
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_PADJ_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll0_cp_padj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_PADJ_M1_ADDR,
            struct1
                ->cmn0_pdiag_pll0_cp_padj_m1);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_CP_PADJ_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll1_cp_padj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_IADJ_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll0_cp_iadj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_IADJ_M1_ADDR,
            struct1
                ->cmn0_pdiag_pll0_cp_iadj_m1);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_CP_IADJ_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll1_cp_iadj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_FILT_PADJ_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll0_filt_padj_m0);  // refclk =
                                                  // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_FILT_PADJ_M1_ADDR,
            struct1
                ->cmn0_pdiag_pll0_filt_padj_m1);  // refclk =
                                                  // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_FILT_PADJ_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll1_filt_padj_m0);  // refclk =
                                                  // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBH_OVRD_M0_ADDR,
            struct1
                ->cmn0_pll0_dsm_fbh_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBL_OVRD_M0_ADDR,
            struct1
                ->cmn0_pll0_dsm_fbl_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBH_OVRD_M1_ADDR,
            struct1
                ->cmn0_pll0_dsm_fbh_ovrd_m1);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBL_OVRD_M1_ADDR,
            struct1
                ->cmn0_pll0_dsm_fbl_ovrd_m1);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL1_DSM_FBH_OVRD_M0_ADDR,
            struct1
                ->cmn0_pll1_dsm_fbh_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL1_DSM_FBL_OVRD_M0_ADDR,
            struct1
                ->cmn0_pll1_dsm_fbl_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(GTR_REG, CMN_TXPUCAL_TUNE_ADDR,
                      struct1->cmn0_txpucal_tune);
        FMSH_WriteReg(GTR_REG, CMN_TXPDCAL_TUNE_ADDR,
                      struct1->cmn0_txpdcal_tune);

        FMSH_WriteReg(GTR_REG, CMN_PLL0_VCOCAL_TCTRL_ADDR,
                      struct1->cmn0_pll0_vcocal_tctrl);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_VCOCAL_TCTRL_ADDR,
                      struct1->cmn0_pll1_vcocal_tctrl);

        FMSH_WriteReg(GTR_REG, CMN_PDIAG_PLL0_CTRL_M0_ADDR,
                      struct1->cmn_pdiag_pll_ctrl_m[0]);
        FMSH_WriteReg(GTR_REG, CMN_PDIAG_PLL0_CTRL_M1_ADDR,
                      struct1->cmn_pdiag_pll_ctrl_m[1]);
        FMSH_WriteReg(GTR_REG, CMN_PDIAG_PLL1_CTRL_M0_ADDR,
                      struct1->cmn_pdiag_pll_ctrl_m[2]);

        FMSH_WriteReg(GTR_REG, CMN_PLL0_DSM_DIAG_M0_ADDR,
                      struct1->cmn0_pll0_dsm_diag_m0);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_DSM_DIAG_M1_ADDR,
                      struct1->cmn0_pll0_dsm_diag_m1);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_DSM_DIAG_M0_ADDR,
                      struct1->cmn0_pll1_dsm_diag_m0);

        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CLK_SEL_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll0_clk_sel_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可。
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CLK_SEL_M1_ADDR,
            struct1
                ->cmn0_pdiag_pll0_clk_sel_m1);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_CLK_SEL_M0_ADDR,
            struct1
                ->cmn0_pdiag_pll1_clk_sel_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(GTR_REG, CMN_PLL0_INTDIV_M0,
                      struct1->cmn_pll_intdiv_m[0]);  // 加入计算公式 2024/5/14
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVH_M0,
                      struct1->cmn_pll_fracdivh_m[0]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_HIGH_THR_M0,
                      struct1->cmn_pll_high_thr_m[0]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_INTDIV_M1,
                      struct1->cmn_pll_intdiv_m[1]);  // 计算公式 2024/5/14
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVH_M1,
                      struct1->cmn_pll_fracdivh_m[1]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_HIGH_THR_M1,
                      struct1->cmn_pll_high_thr_m[1]);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_INTDIV_M0,
                      struct1->cmn_pll_intdiv_m[2]);  // 加入计算公式 2024/5/14
        FMSH_WriteReg(GTR_REG, CMN_PLL1_FRACDIVH_M0,
                      struct1->cmn_pll_fracdivh_m[2]);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_HIGH_THR_M0,
                      struct1->cmn_pll_high_thr_m[2]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVL_M0,
                      struct1->cmn_pll_fracdivl_m[0]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVL_M1,
                      struct1->cmn_pll_fracdivl_m[1]);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_FRACDIVL_M0,
                      struct1->cmn_pll_fracdivl_m[2]);
    }

    if (struct1->lane_icm_cfg[2] || struct1->lane_icm_cfg[3])
    {
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_PADJ_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_cp_padj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_PADJ_M1_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_cp_padj_m1);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_CP_PADJ_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll1_cp_padj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_IADJ_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_cp_iadj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CP_IADJ_M1_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_cp_iadj_m1);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_CP_IADJ_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll1_cp_iadj_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_FILT_PADJ_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_filt_padj_m0);  // refclk =
                                                  // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_FILT_PADJ_M1_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_filt_padj_m1);  // refclk =
                                                  // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_FILT_PADJ_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll1_filt_padj_m0);  // refclk =
                                                  // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBH_OVRD_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pll0_dsm_fbh_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBL_OVRD_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pll0_dsm_fbl_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBH_OVRD_M1_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pll0_dsm_fbh_ovrd_m1);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL0_DSM_FBL_OVRD_M1_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pll0_dsm_fbl_ovrd_m1);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL1_DSM_FBH_OVRD_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pll1_dsm_fbh_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PLL1_DSM_FBL_OVRD_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pll1_dsm_fbl_ovrd_m0);  // refclk =
                                               // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(GTR_REG, CMN_TXPUCAL_TUNE_ADDR + PMA1_OFFSET,
                      struct1->cmn1_txpucal_tune);
        FMSH_WriteReg(GTR_REG, CMN_TXPDCAL_TUNE_ADDR + PMA1_OFFSET,
                      struct1->cmn1_txpdcal_tune);

        FMSH_WriteReg(GTR_REG, CMN_PLL0_VCOCAL_TCTRL_ADDR + PMA1_OFFSET,
                      struct1->cmn1_pll0_vcocal_tctrl);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_VCOCAL_TCTRL_ADDR + PMA1_OFFSET,
                      struct1->cmn1_pll1_vcocal_tctrl);

        FMSH_WriteReg(GTR_REG, CMN_PDIAG_PLL0_CTRL_M0_ADDR + PMA1_OFFSET,
                      struct1->cmn_pdiag_pll_ctrl_m[3]);
        FMSH_WriteReg(GTR_REG, CMN_PDIAG_PLL0_CTRL_M1_ADDR + PMA1_OFFSET,
                      struct1->cmn_pdiag_pll_ctrl_m[4]);
        FMSH_WriteReg(GTR_REG, CMN_PDIAG_PLL1_CTRL_M0_ADDR + PMA1_OFFSET,
                      struct1->cmn_pdiag_pll_ctrl_m[5]);

        FMSH_WriteReg(GTR_REG, CMN_PLL0_DSM_DIAG_M0_ADDR + PMA1_OFFSET,
                      struct1->cmn1_pll0_dsm_diag_m0);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_DSM_DIAG_M1_ADDR + PMA1_OFFSET,
                      struct1->cmn1_pll0_dsm_diag_m1);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_DSM_DIAG_M0_ADDR + PMA1_OFFSET,
                      struct1->cmn1_pll1_dsm_diag_m0);

        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CLK_SEL_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_clk_sel_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可。该值与参考时钟频率
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL0_CLK_SEL_M1_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll0_clk_sel_m1);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可
        FMSH_WriteReg(
            GTR_REG, CMN_PDIAG_PLL1_CLK_SEL_M0_ADDR + PMA1_OFFSET,
            struct1
                ->cmn1_pdiag_pll1_clk_sel_m0);  // refclk =
                                                // 100M时无需配置该寄存器，默认值即可

        FMSH_WriteReg(GTR_REG, CMN_PLL0_INTDIV_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_intdiv_m[3]);  // 加入计算公式 2024/5/14
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVH_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_fracdivh_m[3]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_HIGH_THR_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_high_thr_m[3]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_INTDIV_M1 + PMA1_OFFSET,
                      struct1->cmn_pll_intdiv_m[4]);  // 计算公式 2024/5/14
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVH_M1 + PMA1_OFFSET,
                      struct1->cmn_pll_fracdivh_m[4]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_HIGH_THR_M1 + PMA1_OFFSET,
                      struct1->cmn_pll_high_thr_m[4]);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_INTDIV_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_intdiv_m[5]);  // 加入计算公式 2024/5/14
        FMSH_WriteReg(GTR_REG, CMN_PLL1_FRACDIVH_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_fracdivh_m[5]);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_HIGH_THR_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_high_thr_m[5]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVL_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_fracdivl_m[3]);
        FMSH_WriteReg(GTR_REG, CMN_PLL0_FRACDIVL_M1 + PMA1_OFFSET,
                      struct1->cmn_pll_fracdivl_m[4]);
        FMSH_WriteReg(GTR_REG, CMN_PLL1_FRACDIVL_M0 + PMA1_OFFSET,
                      struct1->cmn_pll_fracdivl_m[5]);
    }
    // 5/16
    // 参考时钟不为100或开启ssc
    for (int i = 0; i < 2; i++)
    {
        if (refclk[i] != 100)
        {
            FMSH_WriteReg(GTR_REG, CMN_SSM_BIAS_TMR + struct1->cmn_offset[i],
                          struct1->cmn_ssm_bias_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLLSM0_PLLPRE_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pllsm0_pllpre_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLLSM0_PLLLOCK_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pllsm0_plllock_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLLSM1_PLLPRE_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pllsm1_pllpre_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLLSM1_PLLLOCK_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pllsm1_plllock_tmr[i]);
            FMSH_WriteReg(GTR_REG, CMN_BGCAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_bgcal_init_tmr[i]);
            FMSH_WriteReg(GTR_REG, CMN_BGCAL_ITER_TMR + struct1->cmn_offset[i],
                          struct1->cmn_bgcal_iter_tmr[i]);
            FMSH_WriteReg(GTR_REG, CMN_IBCAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_ibcal_init_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_TXPUCAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_txpucal_init_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_TXPUCAL_ITER_TMR + struct1->cmn_offset[i],
                          struct1->cmn_txpucal_iter_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_TXPDCAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_txpdcal_init_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_TXPDCAL_ITER_TMR + struct1->cmn_offset[i],
                          struct1->cmn_txpdcal_iter_tmr[i]);
            FMSH_WriteReg(GTR_REG, CMN_RXCAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_rxcal_init_tmr[i]);
            FMSH_WriteReg(GTR_REG, CMN_RXCAL_ITER_TMR + struct1->cmn_offset[i],
                          struct1->cmn_rxcal_iter_tmr[i]);
            FMSH_WriteReg(GTR_REG, CMN_SD_CAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_sd_cal_init_tmr[i]);
            FMSH_WriteReg(GTR_REG, CMN_SD_CAL_ITER_TMR + struct1->cmn_offset[i],
                          struct1->cmn_sd_cal_iter_tmr[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_SD_CAL_REFTIM_START + struct1->cmn_offset[i],
                          struct1->cmn_sd_cal_reftim_start[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_SD_CAL_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_sd_cal_pllcnt_start[i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_VCOCAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_init_tmr[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_VCOCAL_INIT_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_init_tmr[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_VCOCAL_ITER_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_iter_tmr[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_VCOCAL_ITER_TMR + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_iter_tmr[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_VCOCAL_REFTIM_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_reftim_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_VCOCAL_REFTIM_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_reftim_start[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_VCOCAL_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_pllcnt_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_VCOCAL_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_pllcnt_start[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_LOCK_REFCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_refcnt_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_LOCK_REFCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_refcnt_start[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_LOCK_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_pllcnt_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_LOCK_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_pllcnt_start[2 * i + 1]);

            FMSH_WriteReg(GTR_REG,
                          DRV_DIAG_LANE_FCM_EN_TO + struct1->lane_offset[2 * i],
                          struct1->drv_diag_lane_fcm_en_to[i]);  // lane寄存器
            FMSH_WriteReg(
                GTR_REG,
                DRV_DIAG_LANE_FCM_EN_MGN_TMR + struct1->lane_offset[2 * i],
                struct1->drv_diag_lane_fcm_en_mgn_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL0_INIT_TMR + struct1->lane_offset[2 * i],
                          struct1->rx_sdcal0_init_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL0_ITER_TMR + struct1->lane_offset[2 * i],
                          struct1->rx_sdcal0_iter_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL1_INIT_TMR + struct1->lane_offset[2 * i],
                          struct1->rx_sdcal1_init_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL1_ITER_TMR + struct1->lane_offset[2 * i],
                          struct1->rx_sdcal1_iter_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          TX_RCVDET_ST_TMR + struct1->lane_offset[2 * i],
                          struct1->tx_rcvdet_st_tmr[i]);    // lane寄存器
            FMSH_WriteReg(
                GTR_REG,
                DRV_DIAG_LANE_FCM_EN_TO + struct1->lane_offset[2 * i + 1],
                struct1->drv_diag_lane_fcm_en_to[i]);  // lane寄存器
            FMSH_WriteReg(
                GTR_REG,
                DRV_DIAG_LANE_FCM_EN_MGN_TMR + struct1->lane_offset[2 * i + 1],
                struct1->drv_diag_lane_fcm_en_mgn_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL0_INIT_TMR + struct1->lane_offset[2 * i + 1],
                          struct1->rx_sdcal0_init_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL0_ITER_TMR + struct1->lane_offset[2 * i + 1],
                          struct1->rx_sdcal0_iter_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL1_INIT_TMR + struct1->lane_offset[2 * i + 1],
                          struct1->rx_sdcal1_init_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          RX_SDCAL1_ITER_TMR + struct1->lane_offset[2 * i + 1],
                          struct1->rx_sdcal1_iter_tmr[i]);  // lane寄存器
            FMSH_WriteReg(GTR_REG,
                          TX_RCVDET_ST_TMR + struct1->lane_offset[2 * i + 1],
                          struct1->tx_rcvdet_st_tmr[i]);    // lane寄存器
        }

        if (ssc_en[i] == 1)
        {
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_LOCK_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_pllcnt_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_LOCK_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_pllcnt_start[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_LOCK_REFCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_refcnt_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_LOCK_REFCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_refcnt_start[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_LOCK_PLLCNT_THR + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_pllcnt_thr[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_LOCK_PLLCNT_THR + struct1->cmn_offset[i],
                          struct1->cmn_pll_lock_pllcnt_thr[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_VCOCAL_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_pllcnt_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_VCOCAL_PLLCNT_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_pllcnt_start[2 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_VCOCAL_REFTIM_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_reftim_start[2 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_VCOCAL_REFTIM_START + struct1->cmn_offset[i],
                          struct1->cmn_pll_vcocal_reftim_start[2 * i + 1]);

            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL1_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl1_m[3 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL1_M1 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl1_m[3 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_SS_CTRL1_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl1_m[3 * i + 2]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL2_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl2_m[3 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL2_M1 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl2_m[3 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_SS_CTRL2_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl2_m[3 * i + 2]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL3_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl3_m[3 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL3_M1 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl3_m[3 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_SS_CTRL3_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl3_m[3 * i + 2]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL4_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl4_m[3 * i]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL0_SS_CTRL4_M1 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl4_m[3 * i + 1]);
            FMSH_WriteReg(GTR_REG,
                          CMN_PLL1_SS_CTRL4_M0 + struct1->cmn_offset[i],
                          struct1->cmn_pll_ss_ctrl4_m[3 * i + 2]);
        }
    }
    /******************************************************************************/

    /******************************************************************************/
    // PCIE
    if (struct1->pcie == True)
    {
        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == PCIE)
            {
                FMSH_WriteReg(GTR_REG, PHY_FULLRT_DIV_CFG + (i * 0x100 * 4),
                              0x111);

                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_DIV_ADDR + struct1->lane_offset[i],
                    0x1);  // refclk =
                           // 100M时无需配置该寄存器，默认值即可
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_SEL_ADDR + struct1->lane_offset[i],
                    0);    // refclk =
                           // 100M时无需配置该寄存器，默认值即可
                FMSH_WriteReg(
                    GTR_REG,
                    XCVR_DIAG_PLLDRC_CTRL_ADDR + struct1->lane_offset[i],
                    0x12);  // refclk =
                            // 100M时无需配置该寄存器，默认值即可

                FMSH_WriteReg(GTR_REG,
                              RX_REE_TAP1_CLIP_ADDR + struct1->lane_offset[i],
                              0x0019);
                FMSH_WriteReg(
                    GTR_REG, RX_REE_TAP2TON_CLIP_ADDR + struct1->lane_offset[i],
                    0x0019);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_PEAK_UTHR_ADDR + struct1->lane_offset[i],
                              0x0008);
                FMSH_WriteReg(GTR_REG,
                              RX_CDRLF_CNFG_ADDR + struct1->lane_offset[i],
                              0x018e);
                FMSH_WriteReg(GTR_REG,
                              RX_CDRLF_CNFG2_ADDR + struct1->lane_offset[i],
                              0x2e33);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_ACYA_ADDR + struct1->lane_offset[i],
                              0x0001);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_2_ADDR + struct1->lane_offset[i],
                    0x0c21);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_3_ADDR + struct1->lane_offset[i],
                    0x0002);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_REE_DAC_CTRL_ADDR + struct1->lane_offset[i],
                    0x0005);
            }
        }
    }

    /******************************************************************************/
    // USB
    if (struct1->usb == True)
    {
        // phy
        FMSH_WriteReg(GTR_REG, PHY_PIPE_USB3_GEN2_PRE_CFG0_ADDR, 0x0A0A);
        FMSH_WriteReg(GTR_REG, PHY_PIPE_USB3_GEN2_POST_CFG0_ADDR, 0x1000);
        FMSH_WriteReg(GTR_REG, PHY_PIPE_USB3_GEN2_POST_CFG1_ADDR, 0x0010);

        // cmn
        if( (lane[0] == USB) || (lane[1] == USB) )
        {
            FMSH_WriteReg(GTR_REG, CMN_CDIAG_CDB_PWRI_OVRD_ADDR, 0x8200);
            FMSH_WriteReg(GTR_REG, CMN_CDIAG_XCVRC_PWRI_OVRD_ADDR, 0x8200);
        }
        else if( (lane[2] == USB) || (lane[3]== USB) )
        {
            FMSH_WriteReg(GTR_REG, CMN_CDIAG_CDB_PWRI_OVRD_ADDR + PMA1_OFFSET,
                          0x8200);
            FMSH_WriteReg(GTR_REG, CMN_CDIAG_XCVRC_PWRI_OVRD_ADDR + PMA1_OFFSET,
                          0x8200);
        }
        else{
            ;/* no deal with */
        }
        
        // lane

        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == USB)
            {
                FMSH_WriteReg(GTR_REG,
                              PHY_FULLRT_DIV_CFG + struct1->lane_offset[i], 0);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_DIV_ADDR + struct1->lane_offset[i],
                    0x1);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_SEL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_hsclk_sel[i]);
                FMSH_WriteReg(
                    GTR_REG,
                    XCVR_DIAG_PLLDRC_CTRL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_plldrc_ctrl[i]);  //
                FMSH_WriteReg(GTR_REG, TX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x02FF);
                FMSH_WriteReg(GTR_REG, TX_PSC_A1_ADDR + struct1->lane_offset[i],
                              0x06AF);
                FMSH_WriteReg(GTR_REG, TX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x06AE);
                FMSH_WriteReg(GTR_REG, TX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x06AE);
                FMSH_WriteReg(GTR_REG, RX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x0D1D);
                FMSH_WriteReg(GTR_REG, RX_PSC_A1_ADDR + struct1->lane_offset[i],
                              0x0D1D);
                FMSH_WriteReg(GTR_REG, RX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x0D00);
                FMSH_WriteReg(GTR_REG, RX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x0500);
                FMSH_WriteReg(GTR_REG,
                              TX_TXCC_CTRL_ADDR + struct1->lane_offset[i],
                              0x2A82);
                FMSH_WriteReg(
                    GTR_REG,
                    TX_TXCC_CPOST_MULT_00_ADDR + struct1->lane_offset[i],
                    0x0014);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_SIGDET_HL_FILT_TMR_ADDR + struct1->lane_offset[i],
                    0x0013);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_GCSM1_CTRL_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_ATTEN_THR_ADDR + struct1->lane_offset[i],
                              0x0C02);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_SMGM_CTRL1_ADDR + struct1->lane_offset[i],
                              0x0330);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_SMGM_CTRL2_ADDR + struct1->lane_offset[i],
                              0x0300);
                FMSH_WriteReg(GTR_REG,
                              XCVR_DIAG_PSC_OVRD_ADDR + struct1->lane_offset[i],
                              0x0003);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_PEAK_UTHR_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_PEAK_LTHR_ADDR + struct1->lane_offset[i],
                              0x01F5);
                FMSH_WriteReg(
                    GTR_REG, RX_DIAG_SIGDET_TUNE_ADDR + struct1->lane_offset[i],
                    0x1004);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_NQST_CTRL_ADDR + struct1->lane_offset[i],
                              0x00F9);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_PI_CAP_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_PI_RATE_ADDR + struct1->lane_offset[i],
                              0x0031);
                FMSH_WriteReg(GTR_REG,
                              RX_CDRLF_CNFG3_ADDR + struct1->lane_offset[i],
                              0x0003);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_TAP1_CLIP_ADDR + struct1->lane_offset[i],
                              0x0019);
                FMSH_WriteReg(
                    GTR_REG, RX_REE_TAP2TON_CLIP_ADDR + struct1->lane_offset[i],
                    0x0019);

                FMSH_WriteReg(GTR_REG,
                              RX_CDRLF_CNFG_ADDR + struct1->lane_offset[i],
                              0x018C);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_ACYA_ADDR + struct1->lane_offset[i],
                              0x0001);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_2_ADDR + struct1->lane_offset[i],
                    0x0C01);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_3_ADDR + struct1->lane_offset[i],
                    0x0002);
            }
        }
    }
    /*********************************************************************************************/
    // SATA
    if (struct1->sata == True)
    {
        // phy
        FMSH_WriteReg(GTR_REG, PHY_PIPE_CMN_CTRL2, 0xBD41);

        // cmn

        // lane
        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == SATA)
            {
                // FMSH_WriteReg(GTR_REG, PHY_FULLRT_DIV_CFG +
                // struct1->lane_offset[i], 0);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_DIV_ADDR + struct1->lane_offset[i],
                    0x13);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_SEL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_hsclk_sel[i]);
                FMSH_WriteReg(
                    GTR_REG,
                    XCVR_DIAG_PLLDRC_CTRL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_plldrc_ctrl[i]);
                FMSH_WriteReg(GTR_REG, TX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x00fb);
                FMSH_WriteReg(GTR_REG, TX_PSC_A1_ADDR + struct1->lane_offset[i],
                              0x04bb);
                FMSH_WriteReg(GTR_REG, TX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x04aa);
                FMSH_WriteReg(GTR_REG, TX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x04aa);
                FMSH_WriteReg(GTR_REG, RX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x091d);
                FMSH_WriteReg(GTR_REG, RX_PSC_A1_ADDR + struct1->lane_offset[i],
                              0x091d);
                FMSH_WriteReg(GTR_REG, RX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x0900);
                FMSH_WriteReg(GTR_REG, RX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x0100);
                FMSH_WriteReg(GTR_REG,
                              TX_TXCC_CTRL_ADDR + struct1->lane_offset[i],
                              0x2aa0);
                FMSH_WriteReg(
                    GTR_REG,
                    TX_TXCC_CPOST_MULT_00_ADDR + struct1->lane_offset[i],
                    0x0000);
                FMSH_WriteReg(
                    GTR_REG,
                    TX_TXCC_MGNFS_MULT_000_ADDR + struct1->lane_offset[i],
                    0x0011);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_SIGDET_HL_DLY_TMR_ADDR + struct1->lane_offset[i],
                    0x0007);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_SIGDET_HL_FILT_TMR_ADDR + struct1->lane_offset[i],
                    0x0005);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_SMGM_CTRL1_ADDR + struct1->lane_offset[i],
                              0x0474);
                FMSH_WriteReg(GTR_REG,
                              XCVR_DIAG_PSC_OVRD_ADDR + struct1->lane_offset[i],
                              0x0004);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_REE_GCSM1_EQENM_PH1_ADDR + struct1->lane_offset[i],
                    0x03c7);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_REE_GCSM1_EQENM_PH2_ADDR + struct1->lane_offset[i],
                    0x01c7);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_DFE_CTRL_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_TAP1_CLIP_ADDR + struct1->lane_offset[i],
                              0x0019);
                FMSH_WriteReg(
                    GTR_REG, RX_REE_TAP2TON_CLIP_ADDR + struct1->lane_offset[i],
                    0x0019);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_NQST_CTRL_ADDR + struct1->lane_offset[i],
                              0x0988);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_2_ADDR + struct1->lane_offset[i],
                    0x0c01);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_3_ADDR + struct1->lane_offset[i],
                    0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_PI_CAP_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_PI_RATE_ADDR + struct1->lane_offset[i],
                              0x0310);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_ACYA_ADDR + struct1->lane_offset[i],
                              0x0001);
                FMSH_WriteReg(GTR_REG,
                              RX_CDRLF_CNFG_ADDR + struct1->lane_offset[i],
                              0x018c);
                FMSH_WriteReg(GTR_REG,
                              RX_CDRLF_CNFG3_ADDR + struct1->lane_offset[i],
                              0x0007);
            }
        }
    }
    /*********************************************************************************************/
    // DP
    if (struct1->dp == True)
    {
        int val = 0;
        // phy
        FMSH_WriteReg(GTR_REG, PHY_PMA_PLL_RAW_CTRL, 0x303);

        // power state
        val = FMSH_ReadReg(GTR_SLCR, PMA_XCVR_REG_ADDR);
        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == DP)
            {
                val &= ((~(0x1 << (i * 8))) & 0xffffffff);
                val += (0x4 << (i * 8));
            }
        }
        FMSH_WriteReg(GTR_SLCR, PMA_XCVR_REG_ADDR, val);  // a2 power down
        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == DP)
            {
                val -= (0x4 << (i * 8));
                val += (0x1 << (i * 8));
            }
        }
        FMSH_WriteReg(GTR_SLCR, PMA_XCVR_REG_ADDR, val);  // a0

        // cmn

        // lane
        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == DP)
            {
                // FMSH_WriteReg(GTR_REG, PHY_FULLRT_DIV_CFG +
                // struct1->lane_offset[i], 0);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_DIV_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_hsclk_div[i]);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_SEL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_hsclk_sel[i]);
                FMSH_WriteReg(
                    GTR_REG,
                    XCVR_DIAG_PLLDRC_CTRL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_plldrc_ctrl[i]);
                FMSH_WriteReg(GTR_REG, TX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x00FB);
                FMSH_WriteReg(GTR_REG, TX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x04AA);
                FMSH_WriteReg(GTR_REG, TX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x04AA);
                FMSH_WriteReg(GTR_REG, RX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG, RX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG, RX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(
                    GTR_REG, RX_PSC_CAL_ADDR + struct1->lane_offset[i], 0x0000);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_BIDI_CTRL_ADDR + struct1->lane_offset[i],
                    0x000F);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_GCSM2_CTRL_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_GCSM1_CTRL_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(
                    GTR_REG, RX_REE_PERGCSM_CTRL_ADDR + struct1->lane_offset[i],
                    0x0000);
                FMSH_WriteReg(GTR_REG,
                              TX_TXCC_CTRL_ADDR + struct1->lane_offset[i],
                              0x08A4);
                FMSH_WriteReg(GTR_REG,
                              DRV_DIAG_TX_DRV_ADDR + struct1->lane_offset[i],
                              0x0003);
                FMSH_WriteReg(
                    GTR_REG,
                    TX_TXCC_MGNFS_MULT_000_ADDR + struct1->lane_offset[i],
                    struct1->tx_txcc_mgnfs_mult_000);
                FMSH_WriteReg(
                    GTR_REG,
                    TX_TXCC_CPOST_MULT_00_ADDR + struct1->lane_offset[i],
                    struct1->tx_txcc_cpost_mult_00);
            }
        }
    }
    /*********************************************************************************************/
    // SGMII
    if (struct1->sgmii == True)
    {
        // lane
        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == SGMII)
            {
                FMSH_WriteReg(GTR_REG, PHY_FULLRT_DIV_CFG + i * (0x100 * 4),
                              0x2);  // 6/21
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_DIV_ADDR + struct1->lane_offset[i],
                    0x0103);
                FMSH_WriteReg(
                    GTR_REG, XCVR_DIAG_HSCLK_SEL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_hsclk_sel[i]);
                FMSH_WriteReg(
                    GTR_REG,
                    XCVR_DIAG_PLLDRC_CTRL_ADDR + struct1->lane_offset[i],
                    struct1->xcvr_diag_plldrc_ctrl[i]);
                FMSH_WriteReg(GTR_REG, TX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x00F3);
                FMSH_WriteReg(GTR_REG, TX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x04A2);
                FMSH_WriteReg(GTR_REG, TX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x04A2);
                FMSH_WriteReg(GTR_REG, RX_PSC_A0_ADDR + struct1->lane_offset[i],
                              0x091D);
                FMSH_WriteReg(GTR_REG, RX_PSC_A2_ADDR + struct1->lane_offset[i],
                              0x0900);
                FMSH_WriteReg(GTR_REG, RX_PSC_A3_ADDR + struct1->lane_offset[i],
                              0x0100);
                FMSH_WriteReg(
                    GTR_REG,
                    TX_TXCC_CPOST_MULT_00_ADDR + struct1->lane_offset[i],
                    0x0000);
                FMSH_WriteReg(GTR_REG,
                              DRV_DIAG_TX_DRV_ADDR + struct1->lane_offset[i],
                              0x00B3);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_REE_GCSM1_EQENM_PH1_ADDR + struct1->lane_offset[i],
                    0x03c7);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_REE_GCSM1_EQENM_PH2_ADDR + struct1->lane_offset[i],
                    0x01c7);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_DFE_CTRL_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_REE_TAP1_CLIP_ADDR + struct1->lane_offset[i],
                              0x0019);
                FMSH_WriteReg(
                    GTR_REG, RX_REE_TAP2TON_CLIP_ADDR + struct1->lane_offset[i],
                    0x0019);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_NQST_CTRL_ADDR + struct1->lane_offset[i],
                              0x0098);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_2_ADDR + struct1->lane_offset[i],
                    0x0c01);
                FMSH_WriteReg(
                    GTR_REG,
                    RX_DIAG_DFE_AMP_TUNE_3_ADDR + struct1->lane_offset[i],
                    0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_PI_CAP_ADDR + struct1->lane_offset[i],
                              0x0000);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_PI_RATE_ADDR + struct1->lane_offset[i],
                              0x0010);
                FMSH_WriteReg(GTR_REG,
                              RX_DIAG_ACYA_ADDR + struct1->lane_offset[i],
                              0x0001);
                FMSH_WriteReg(GTR_REG,
                              XCVR_DIAG_PSC_OVRD_ADDR + struct1->lane_offset[i],
                              0x0002);
                FMSH_WriteReg(GTR_REG,
                              RX_CDRLF_CNFG_ADDR + struct1->lane_offset[i],
                              0x018C);
            }
        }
    }
}
static int GTR_Init (u8 lane[4], double refclk[2], u8 ssc_en[2],
                     GTR_Regval* struct1)
{
    // 6/17
    u32 value = 1;  // 6/17
    //u32 temp = 0;
    u8 pma_mask = 0;
    u32 time_cnt = 0;
    /*****************************************************************/
    // only reset gtr
    mask_write(CRF_APB, RST_FPD_GTR_ADDR, 0xE00FF,
               0xFF);                           // apb_reset、phy0 ->1,assert
    mask_write(CRF_APB, RST_FPD_GTR_ADDR, 0xE0040,
               0x0);                            // apb_reset ->0,de-assert
    __GTR_Init(lane, refclk, ssc_en, struct1);  // init function
    mask_write(CRF_APB, RST_FPD_GTR_ADDR, 0xFF,
               0x0);  // phy reset ->0, lane reset, de-assert

    // check status
    value = FMSH_ReadReg(GTR_REG, 0xE000 * 4);  // PHY_PMA_CMN_CTRL1
    if( (lane[0] != 0) || (lane[1] != 0) )
    {
        pma_mask += 0x1;
    }  // pma0 cmn ready
    if( (lane[2] != 0) || (lane[3] != 0) )
    {
        pma_mask += 0x2;
    }  // pma1 cmn ready
    while ((value & pma_mask) != pma_mask)
    {
        value = FMSH_ReadReg(GTR_REG, 0xE000 * 4);  // PHY_PMA_CMN_CTRL1
        time_cnt++;
        if (time_cnt > TIME_OUT)
        {
            return FMSH_FAILURE;
        }
    }

    // if sata is used
    if (check_mac(lane, SATA) == 0)
    {
        mask_write(GTR_SLCR, PIPE_LANE_RESET_N_IP_ADDR, 0x3,
                   0);         // pipe_reset_n_sata
        mask_write(GTR_SLCR, GTR_CFG_REG_ADDR, 0x400000,
                   0x400000);  // initial done
    }

                               // change PLL clk power status
    if (check_mac(lane, SGMII) == 0)  // 6/17
    {
        value = FMSH_ReadReg(GTR_SLCR, PMA_XCVR_REG_ADDR);
        //temp = FMSH_ReadReg(GTR_SLCR, PMA_XCVR_ACK_REG_ADDR);
        /*
        while((temp != value))
        {
            temp = FMSH_ReadReg(GTR_SLCR,PMA_XCVR_ACK_REG_ADDR);
        }*/
        delay_us(1);

        for (int i = 0; i < 4; i++)
        {
            if (lane[i] == SGMII)
            {
                value |= (0x41 << (i * 8));
            }
        }
        FMSH_WriteReg(GTR_SLCR, PMA_XCVR_REG_ADDR, value);
    }

    return FMSH_SUCCESS;
}

/****************************************************************************/
int gtr_initial ()
{
    int ret;
    u8 lane[4];
    double refclk[2];
    u8 ssc_en[2];
    // extern typedef struct regval GTR_Regval;
    // extern GTR_Regval gtr_struct1;

    lane[0] = FPAR_GTRPSU_LANE0_PROTOCOL;
    lane[1] = FPAR_GTRPSU_LANE1_PROTOCOL;
    lane[2] = FPAR_GTRPSU_LANE2_PROTOCOL;
    lane[3] = FPAR_GTRPSU_LANE3_PROTOCOL;

    refclk[0] = (double)FPAR_GTRPSU_REFCLK0_FREQ_HZ;
    refclk[1] = (double)FPAR_GTRPSU_REFCLK1_FREQ_HZ;
    ssc_en[0] = FPAR_GTRPSU_PMA0_SSC_EN;
    ssc_en[1] = FPAR_GTRPSU_PMA1_SSC_EN;

    ret = GTR_Init(lane, refclk, ssc_en, &gtr_struct1);

    return ret;
}
