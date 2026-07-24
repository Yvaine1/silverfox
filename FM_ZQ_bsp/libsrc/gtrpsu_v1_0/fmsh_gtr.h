/************************************************************

5/11
5/16
5/28
6/5
********************************************************/
#ifndef _FMSH_GTRPSU_H_
#define _FMSH_GTRPSU_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <math.h>
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define PCIE                      1
#define USB                       2
#define SATA                      3
#define DP                        4
#define SGMII                     5
#define Ethernet                  6

// Base Address
#define GTR_SLCR                  0xFD3D0000
#define GTR_REG                   0xFD400000
//#define CRF_APB                   0xFD1A0000

// GTR_SLCR offset
#define ICM_CFG0_REG_ADDR         0X0
#define ICM_CFG1_REG_ADDR         0X4
#define GTR_CFG_REG_ADDR          0X8
#define PMA_CMN_CLK_REG_ADDR      0XC
#define PMA_XCVR_REG_ADDR         0X10
#define PMA_XCVR_ACK_REG_ADDR     0X1C
#define PIPE_LANE_RESET_N_IP_ADDR 0X24


#define PMA1_OFFSET \
    (0x2000 * 4)  
#define LANE1_OFFSET \
    (0x200 * 4)   
#define LANE2_OFFSET \
    (0x400 * 4)   
#define LANE3_OFFSET \
    (0x600 * 4)   

// 下列地址默认为pma0 lane0 地址。
// PMA0 GTR_REG PHY offset
#define PHY_PLL_CFG_ADDR                  (0xC00E * 4)
#define PHY_PIPE_CMN_CTRL2                (0xC001 * 4)
#define PHY_FULLRT_DIV_CFG                (0xD013 * 4)
#define PHY_PMA_PLL_RAW_CTRL              (0xE003 * 4)
#define PHY_PMA_CMN_CTRL1                 (0xE000 * 4)

// lane
// refclk
#define DRV_DIAG_LANE_FCM_EN_TO           (0x40c0 * 4)
#define DRV_DIAG_LANE_FCM_EN_MGN_TMR      (0x40c2 * 4)
#define RX_SDCAL0_INIT_TMR                (0x8044 * 4)
#define RX_SDCAL0_ITER_TMR                (0x8045 * 4)
#define RX_SDCAL1_INIT_TMR                (0x804c * 4)
#define RX_SDCAL1_ITER_TMR                (0x804d * 4)
#define TX_RCVDET_ST_TMR                  (0x4123 * 4)

// pcie
#define RX_REE_TAP1_CLIP_ADDR             (0x8171 * 4)  // lane寄存器
#define RX_REE_TAP2TON_CLIP_ADDR          (0x8172 * 4)  // lane寄存器
#define RX_REE_PEAK_UTHR_ADDR             (0x8142 * 4)  // lane寄存器
#define RX_CDRLF_CNFG_ADDR                (0x8080 * 4)  // lane寄存器
#define RX_CDRLF_CNFG2_ADDR               (0x8081 * 4)  // lane寄存器
#define RX_DIAG_ACYA_ADDR                 (0x81ff * 4)  // lane寄存器
#define RX_DIAG_DFE_AMP_TUNE_2_ADDR       (0x81e2 * 4)  // lane寄存器
#define RX_DIAG_DFE_AMP_TUNE_3_ADDR       (0x81e3 * 4)  // lane寄存器
#define RX_DIAG_REE_DAC_CTRL_ADDR         (0x81e4 * 4)  // lane寄存器
// USB
#define PHY_PIPE_USB3_GEN2_PRE_CFG0_ADDR  (0xc020 * 4)
#define PHY_PIPE_USB3_GEN2_POST_CFG0_ADDR (0xc022 * 4)
#define PHY_PIPE_USB3_GEN2_POST_CFG1_ADDR (0xc023 * 4)
#define CMN_CDIAG_CDB_PWRI_OVRD_ADDR      (0x41 * 4)    // common
#define CMN_CDIAG_XCVRC_PWRI_OVRD_ADDR    (0x47 * 4)    // common

#define TX_PSC_A0_ADDR                    (0x4100 * 4)  // lane寄存器
#define TX_PSC_A1_ADDR                    (0x4101 * 4)  // lane寄存器
#define TX_PSC_A2_ADDR                    (0x4102 * 4)  // lane寄存器
#define TX_PSC_A3_ADDR                    (0x4103 * 4)  // lane寄存器
#define RX_PSC_A0_ADDR                    (0x8000 * 4)  // lane寄存器
#define RX_PSC_A1_ADDR                    (0x8001 * 4)  // lane寄存器
#define RX_PSC_A2_ADDR                    (0x8002 * 4)  // lane寄存器
#define RX_PSC_A3_ADDR                    (0x8003 * 4)  // lane寄存器

#define TX_TXCC_CTRL_ADDR                 (0x4040 * 4)  // lane寄存器
#define TX_TXCC_CPOST_MULT_01_ADDR        (0x404d * 4)  // lane寄存器
#define RX_SIGDET_HL_FILT_TMR_ADDR        (0x8090 * 4)  // lane寄存器

#define RX_REE_GCSM1_CTRL_ADDR            (0x8108 * 4)  // lane寄存器
#define RX_REE_ATTEN_THR_ADDR             (0x8149 * 4)  // lane寄存器
#define RX_REE_SMGM_CTRL1_ADDR            (0x8177 * 4)  // lane寄存器
#define RX_REE_SMGM_CTRL2_ADDR            (0x8178 * 4)  // lane寄存器
#define XCVR_DIAG_PSC_OVRD_ADDR           (0x40eb * 4)  // lane寄存器
#define RX_REE_PEAK_LTHR_ADDR             (0x8143 * 4)  // lane寄存器

#define RX_DIAG_SIGDET_TUNE_ADDR          (0x81e8 * 4)  // lane寄存器
#define RX_DIAG_NQST_CTRL_ADDR            (0x81e5 * 4)  // lane寄存器
#define RX_DIAG_PI_CAP_ADDR               (0x81f5 * 4)  // lane寄存器
#define RX_DIAG_PI_RATE_ADDR              (0x81f4 * 4)  // lane寄存器
#define RX_CDRLF_CNFG3_ADDR               (0x8082 * 4)  // lane寄存器
// SATA
#define PHY_PIPE_CMN_CTRL2_ADDR           (0XC001 * 4)
#define TX_TXCC_CPOST_MULT_00_ADDR        (0X404C * 4)  // lane寄存器
#define TX_TXCC_MGNFS_MULT_000_ADDR       (0X4050 * 4)  // lane寄存器
#define RX_SIGDET_HL_DLY_TMR_ADDR         (0X8091 * 4)  // lane寄存器
#define RX_REE_GCSM1_EQENM_PH1_ADDR       (0X8109 * 4)  // lane寄存器
#define RX_REE_GCSM1_EQENM_PH2_ADDR       (0X810A * 4)  // lane寄存器
#define RX_DIAG_DFE_CTRL_ADDR             (0X81E0 * 4)  // lane寄存器
// DP
#define RX_PSC_CAL_ADDR                   (0x8006 * 4)  // lane寄存器
#define XCVR_DIAG_BIDI_CTRL_ADDR          (0x40ea * 4)  // lane寄存器
#define RX_REE_GCSM2_CTRL_ADDR            (0x8110 * 4)  // lane寄存器
#define RX_REE_PERGCSM_CTRL_ADDR          (0x8118 * 4)  // lane寄存器
#define DRV_DIAG_TX_DRV_ADDR              (0x40c6 * 4)  // lane寄存器
#define TX_DIAG_ACYA_ADDR                           (0x41E7 * 4)

// PMA0 GTR_REG PMA offset
#define XCVR_DIAG_HSCLK_DIV_ADDR          (0X40E7 * 4)  // lane寄存器
#define XCVR_DIAG_HSCLK_SEL_ADDR          (0X40E6 * 4)  // lane寄存器
#define XCVR_DIAG_PLLDRC_CTRL_ADDR        (0X40E5 * 4)  // lane寄存器

#define CMN_PDIAG_PLL0_CLK_SEL_M0_ADDR    (0X01A1 * 4)  // common
#define CMN_PDIAG_PLL0_CLK_SEL_M1_ADDR    (0X01B1 * 4)  // common
#define CMN_PDIAG_PLL1_CLK_SEL_M0_ADDR    (0X01C1 * 4)  // common
#define CMN_PDIAG_PLL0_CP_PADJ_M0_ADDR    (0X01A4 * 4)  // common
#define CMN_PDIAG_PLL0_CP_PADJ_M1_ADDR    (0X01B4 * 4)  // common
#define CMN_PDIAG_PLL1_CP_PADJ_M0_ADDR    (0X01C4 * 4)  // common
#define CMN_PDIAG_PLL0_CP_IADJ_M0_ADDR    (0X01A5 * 4)  // common
#define CMN_PDIAG_PLL0_CP_IADJ_M1_ADDR    (0X01B5 * 4)  // common
#define CMN_PDIAG_PLL1_CP_IADJ_M0_ADDR    (0X01C5 * 4)  // common
#define CMN_PDIAG_PLL0_FILT_PADJ_M0_ADDR  (0X01A6 * 4)  // common
#define CMN_PDIAG_PLL0_FILT_PADJ_M1_ADDR  (0X01B6 * 4)  // common
#define CMN_PDIAG_PLL1_FILT_PADJ_M0_ADDR  (0X01C6 * 4)  // common
#define CMN_PLL0_DSM_FBH_OVRD_M0_ADDR     (0x95 * 4)    // common
#define CMN_PLL0_DSM_FBL_OVRD_M0_ADDR     (0x96 * 4)    // common
#define CMN_PLL0_DSM_FBH_OVRD_M1_ADDR     (0xA5 * 4)    // common
#define CMN_PLL0_DSM_FBL_OVRD_M1_ADDR     (0xA6 * 4)    // common
#define CMN_PLL1_DSM_FBH_OVRD_M0_ADDR     (0xD5 * 4)    // common
#define CMN_PLL1_DSM_FBL_OVRD_M0_ADDR     (0xD6 * 4)    // common
#define CMN_PDIAG_PLL0_CTRL_M0_ADDR       (0X01A0 * 4)  // common
#define CMN_PDIAG_PLL0_CTRL_M1_ADDR       (0X01B0 * 4)  // common
#define CMN_PDIAG_PLL1_CTRL_M0_ADDR       (0X01C0 * 4)  // common
#define CMN_PLL0_VCOCAL_TCTRL_ADDR        (0x82 * 4)    // common
#define CMN_PLL1_VCOCAL_TCTRL_ADDR        (0xc2 * 4)    // common
#define CMN_TXPUCAL_TUNE_ADDR             (0x0103 * 4)
#define CMN_TXPDCAL_TUNE_ADDR             (0x010b * 4)
#define CMN_PLL0_DSM_DIAG_M0_ADDR         (0x0094 * 4)
#define CMN_PLL0_DSM_DIAG_M1_ADDR         (0x00A4 * 4)
#define CMN_PLL1_DSM_DIAG_M0_ADDR         (0x00D4 * 4)

#define CMN_PLL0_INTDIV_M0                (0x0090 * 4)
#define CMN_PLL0_INTDIV_M1                (0x00A0 * 4)
#define CMN_PLL1_INTDIV_M0                (0x00D0 * 4)
#define CMN_PLL0_FRACDIVH_M0              (0x0092 * 4)
#define CMN_PLL0_FRACDIVH_M1              (0x00A2 * 4)
#define CMN_PLL1_FRACDIVH_M0              (0x00D2 * 4)
#define CMN_PLL0_FRACDIVL_M0              (0x0091 * 4)
#define CMN_PLL0_FRACDIVL_M1              (0x00A1 * 4)
#define CMN_PLL1_FRACDIVL_M0              (0x00D1 * 4)
#define CMN_PLL0_HIGH_THR_M0              (0x0093 * 4)
#define CMN_PLL0_HIGH_THR_M1              (0x00A3 * 4)
#define CMN_PLL1_HIGH_THR_M0              (0x00D3 * 4)

// refclk&ssc
#define CMN_SSM_BIAS_TMR                  (0x22 * 4)
#define CMN_PLLSM0_PLLPRE_TMR             (0x2a * 4)
#define CMN_PLLSM0_PLLLOCK_TMR            (0x2c * 4)
#define CMN_PLLSM1_PLLPRE_TMR             (0x32 * 4)
#define CMN_PLLSM1_PLLLOCK_TMR            (0x34 * 4)
#define CMN_BGCAL_INIT_TMR                (0x64 * 4)
#define CMN_BGCAL_ITER_TMR                (0x65 * 4)
#define CMN_IBCAL_INIT_TMR                (0x74 * 4)
#define CMN_TXPUCAL_INIT_TMR              (0x104 * 4)
#define CMN_TXPUCAL_ITER_TMR              (0x105 * 4)
#define CMN_TXPDCAL_INIT_TMR              (0x10c * 4)
#define CMN_TXPDCAL_ITER_TMR              (0x10d * 4)
#define CMN_RXCAL_INIT_TMR                (0x114 * 4)
#define CMN_RXCAL_ITER_TMR                (0x115 * 4)
#define CMN_SD_CAL_INIT_TMR               (0x124 * 4)
#define CMN_SD_CAL_ITER_TMR               (0x125 * 4)
#define CMN_SD_CAL_REFTIM_START           (0x126 * 4)
#define CMN_SD_CAL_PLLCNT_START           (0x128 * 4)
#define CMN_PLL0_VCOCAL_INIT_TMR          (0x84 * 4)
#define CMN_PLL1_VCOCAL_INIT_TMR          (0xc4 * 4)
#define CMN_PLL0_VCOCAL_ITER_TMR          (0x85 * 4)
#define CMN_PLL1_VCOCAL_ITER_TMR          (0xc5 * 4)
#define CMN_PLL0_VCOCAL_REFTIM_START      (0x86 * 4)
#define CMN_PLL1_VCOCAL_REFTIM_START      (0xc6 * 4)
#define CMN_PLL0_VCOCAL_PLLCNT_START      (0x88 * 4)
#define CMN_PLL1_VCOCAL_PLLCNT_START      (0xc8 * 4)
#define CMN_PLL0_LOCK_REFCNT_START        (0x9c * 4)
#define CMN_PLL1_LOCK_REFCNT_START        (0xdc * 4)
#define CMN_PLL0_LOCK_PLLCNT_START        (0x9e * 4)
#define CMN_PLL1_LOCK_PLLCNT_START        (0xde * 4)
#define CMN_PLL0_LOCK_PLLCNT_THR          (0x9f * 4)
#define CMN_PLL1_LOCK_PLLCNT_THR          (0xdf * 4)

#define CMN_PLL0_SS_CTRL1_M0              (0x98 * 4)
#define CMN_PLL0_SS_CTRL1_M1              (0xa8 * 4)
#define CMN_PLL1_SS_CTRL1_M0              (0xd8 * 4)
#define CMN_PLL0_SS_CTRL2_M0              (0x99 * 4)
#define CMN_PLL0_SS_CTRL2_M1              (0xa9 * 4)
#define CMN_PLL1_SS_CTRL2_M0              (0xd9 * 4)
#define CMN_PLL0_SS_CTRL3_M0              (0x9a * 4)
#define CMN_PLL0_SS_CTRL3_M1              (0xaa * 4)
#define CMN_PLL1_SS_CTRL3_M0              (0xda * 4)
#define CMN_PLL0_SS_CTRL4_M0              (0x9b * 4)
#define CMN_PLL0_SS_CTRL4_M1              (0xab * 4)
#define CMN_PLL1_SS_CTRL4_M0              (0xdb * 4)

// CRF_APB offset
#define RST_FPD_GTR_ADDR                  0x010C
      

typedef struct regval {
     u8 pcie;
    u8 usb ;
    u8 sata;
    u8 dp;
    u8 sgmii ;
    u8 ethernet ;
    u16 pllclk[6] ; 
    u8 DSM_fractional[6] ;
    u8 pll_div[6];
    u16 cmn_offset[2] ;
    u16 lane_offset[4] ;
    u8 cmn0_pll1_mac ; 
    u8 cmn1_pll1_mac ;  
    u8 fb_div[6];   

    // phy reg
    u8 phy_l_mode[4];
    u8 lane_icm_cfg[4];
    u32 icm_cfg0_reg ;
    u32 icm_cfg1_reg ;
    u8 phy_bfr_en ;
    u8 phy_link_cfg_ln_mode[4];
    u32 gtr_cfg_reg ;
    u8 num_standard ;
    u32 pma_xcvr_reg ;
    u32 pma_cmn_clk_reg ;
    u8 phy_pll_cfg ;
    u16 phy_pma_pll_raw_ctrl ;
    // cmn reg
    u16 cmn0_pll0_dsm_diag_m0 ;
    u16 cmn0_pll0_dsm_diag_m1 ;
    u16 cmn0_pll1_dsm_diag_m0 ;
    u16 cmn1_pll0_dsm_diag_m0 ;
    u16 cmn1_pll0_dsm_diag_m1 ;
    u16 cmn1_pll1_dsm_diag_m0 ;

    u16 cmn0_pdiag_pll0_clk_sel_m0 ;  // rst val 0x601
    u16 cmn0_pdiag_pll0_clk_sel_m1 ;  // rst val 0x400
    u16 cmn0_pdiag_pll1_clk_sel_m0 ;  // rst val 0x400
    u16 cmn1_pdiag_pll0_clk_sel_m0 ;
    u16 cmn1_pdiag_pll0_clk_sel_m1 ;
    u16 cmn1_pdiag_pll1_clk_sel_m0 ;

    u16 cmn0_pdiag_pll0_cp_padj_m0 ;
    u16 cmn0_pdiag_pll0_cp_padj_m1 ;
    u16 cmn0_pdiag_pll1_cp_padj_m0 ;
    u16 cmn1_pdiag_pll0_cp_padj_m0 ;
    u16 cmn1_pdiag_pll0_cp_padj_m1 ;
    u16 cmn1_pdiag_pll1_cp_padj_m0 ;

    u16 cmn0_txpucal_tune ;  // 单协议下为默认值，多协议下需要配置为0x007f
    u16 cmn0_txpdcal_tune ;  // 单协议下为默认值，多协议下需要配置为0x007f
    u16 cmn1_txpucal_tune ;  // 单协议下为默认值，多协议下需要配置为0x007f
    u16 cmn1_txpdcal_tune ;  // 单协议下为默认值，多协议下需要配置为0x007f

    u16 cmn0_pll0_vcocal_tctrl ;
    u16 cmn0_pll1_vcocal_tctrl ;
    u16 cmn1_pll0_vcocal_tctrl ;
    u16 cmn1_pll1_vcocal_tctrl ;
    u16 cmn_pll_vcocal_init_tmr[4] ;
    u16 cmn_pll_vcocal_iter_tmr[4] ;
    u16 cmn_pll_vcocal_reftim_start[4] ;
    u16 cmn_pll_vcocal_pllcnt_start[4] ;
    
    u16 cmn_pdiag_pll_ctrl_m[6] ;

    u16 cmn0_pdiag_pll0_cp_iadj_m0 ;
    u16 cmn0_pdiag_pll0_cp_iadj_m1 ;
    u16 cmn0_pdiag_pll1_cp_iadj_m0 ;
    u16 cmn1_pdiag_pll0_cp_iadj_m0 ;
    u16 cmn1_pdiag_pll0_cp_iadj_m1 ;
    u16 cmn1_pdiag_pll1_cp_iadj_m0 ;

    u16 cmn0_pdiag_pll0_filt_padj_m0 ;
    u16 cmn0_pdiag_pll0_filt_padj_m1 ;
    u16 cmn0_pdiag_pll1_filt_padj_m0 ;
    u16 cmn1_pdiag_pll0_filt_padj_m0 ;
    u16 cmn1_pdiag_pll0_filt_padj_m1 ;
    u16 cmn1_pdiag_pll1_filt_padj_m0 ;

    u16 cmn0_pll0_dsm_fbh_ovrd_m0;
    u16 cmn0_pll0_dsm_fbh_ovrd_m1;
    u16 cmn0_pll1_dsm_fbh_ovrd_m0;
    u16 cmn1_pll0_dsm_fbh_ovrd_m0;
    u16 cmn1_pll0_dsm_fbh_ovrd_m1;
    u16 cmn1_pll1_dsm_fbh_ovrd_m0;

    u16 cmn0_pll0_dsm_fbl_ovrd_m0 ;
    u16 cmn0_pll0_dsm_fbl_ovrd_m1 ;
    u16 cmn0_pll1_dsm_fbl_ovrd_m0 ;
    u16 cmn1_pll0_dsm_fbl_ovrd_m0 ;
    u16 cmn1_pll0_dsm_fbl_ovrd_m1 ;
    u16 cmn1_pll1_dsm_fbl_ovrd_m0 ;

    u16 cmn_pll_intdiv_m[6] ; 
    u16 cmn_pll_fracdivh_m[6] ;         
    u16 cmn_pll_fracdivl_m[6] ;
    u16 cmn_pll_high_thr_m[6] ; 

    // different refclk
    u16 cmn_ssm_bias_tmr[2];

    u16 cmn_pllsm0_pllpre_tmr[2] ;
    u16 cmn_pllsm0_plllock_tmr[2];

    u16 cmn_pllsm1_pllpre_tmr[2] ;
    u16 cmn_pllsm1_plllock_tmr[2] ;

    u16 cmn_bgcal_init_tmr[2];
    u16 cmn_bgcal_iter_tmr[2];

    u16 cmn_ibcal_init_tmr[2] ;

    u16 cmn_txpucal_init_tmr[2] ;
    u16 cmn_txpucal_iter_tmr[2] ;
    u16 cmn_txpdcal_init_tmr[2] ;
    u16 cmn_txpdcal_iter_tmr[2] ;
    u16 cmn_rxcal_init_tmr[2];
    u16 cmn_rxcal_iter_tmr[2];

    u16 cmn_sd_cal_init_tmr[2] ;
    u16 cmn_sd_cal_iter_tmr[2] ;
    u16 cmn_sd_cal_reftim_start[2];
    u16 cmn_sd_cal_pllcnt_start[2] ;

    u16 drv_diag_lane_fcm_en_to[4] ;
    u16 drv_diag_lane_fcm_en_mgn_tmr[4];

    u16 rx_sdcal0_init_tmr[4] ;
    u16 rx_sdcal0_iter_tmr[4] ;
    u16 rx_sdcal1_init_tmr[4] ;
    u16 rx_sdcal1_iter_tmr[4] ;

    u16 tx_rcvdet_st_tmr[4] ;

    u16 cmn_pll_lock_refcnt_start[4] ; 
    u16 cmn_pll_lock_pllcnt_start[4] ;
    u16 cmn_pll_lock_pllcnt_thr[4] ;

    // ssc
    u16 cmn_pll_ss_ctrl1_m[6] ;
    u16 cmn_pll_ss_ctrl2_m[6] ;
    u16 cmn_pll_ss_ctrl3_m[6] ;
    u16 cmn_pll_ss_ctrl4_m[6] ;
    
    //mac
    u16 xcvr_diag_hsclk_div[4] ;
    u16 xcvr_diag_hsclk_sel[4] ;
    u16 xcvr_diag_plldrc_ctrl[4]; 
    
    u8 tx_txcc_mgnfs_mult_000;
    u8 tx_txcc_cpost_mult_00;

}GTR_Regval;


extern GTR_Regval gtr_struct1;

/************************** Function Prototypes ******************************/
int gtr_initial(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
