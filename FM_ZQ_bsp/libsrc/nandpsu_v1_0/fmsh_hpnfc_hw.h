/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fmsh_hpnfc_hw.h
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This header file contains the hpnfc lowlevel operating functions (or
 *macros).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2023/02/16  First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_HPNFC_HW_H_ /* prevent circular inclusions */
#define _FMSH_HPNFC_HW_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************Include File*********************************/
#include "fmsh_hpnfc.h"

/******************************Constant Definition**************************/
/* Command and Status Registers */
#define NAND_R_CMD_REG0                              (0x0000)

#define NAND_R_CMD_REG1                              (0x0004)
#define NAND_CMD_BANK_SHIFT                          (24)

#define NAND_R_CMD_REG2                              (0x0008)
#define NAND_R_CMD_REG3                              (0x000C)
#define NAND_R_CMD_STATUS_PTR                        (0x0010)

#define NAND_R_CMD_STATUS                            (0x0014)
#define NAND_CMD_STATUS_COMP                         (0x8000)
#define NAND_CMD_STATUS_FAIL                         (0x4000)
#define NAND_R_CMD_STATUS_EXT                        (0x0018)
#define NAND_R_CMD_REG4                              (0x0020)
#define NAND_R_CMD_REG5                              (0x0024)
#define NAND_R_CMD_REG6                              (0x0028)
#define NAND_R_INTR_STATUS                           (0x0110)
#define NAND_INTR_ZQCL_ERR                           (0x800000)
#define NAND_INTR_SDMA_ERR                           (0x400000)
#define NAND_INTR_SDMA_TRIGG                         (0x200000)
#define NAND_INTR_CMD_IGNORED                        (0x100000)
#define NAND_INTR_DDMA_TERR                          (0x40000)
#define NAND_INTR_CDMA_TERR                          (0x20000)
#define NAND_INTR_CDMA_IDLE                          (0x10000)

#define NAND_R_INTR_ENABLE                           (0x0114)
#define NAND_INTR_ENABLE_INTR_EN                     (0x80000000)

#define NAND_R_CTRL_STATUS                           (0x0118)
#define NAND_CTRL_STATUS_INIT_COMP                   (0x200)
#define NAND_CTRL_STATUS_CTRL_BUSY                   (0x100)
#define NAND_CTRL_STATUS_MC_BUSY                     (0x8)
#define NAND_CTRL_STATUS_CMDENG_BUSY                 (0x4)
#define NAND_CTRL_STATUS_MDMA_BUSY                   (0x2)
#define NAND_CTRL_STATUS_SDMA_BUSY                   (0x1)

#define NAND_R_TRD_STATUS                            (0x0120)
#define NAND_R_TRD_ERROR_INTR_STATUS                 (0x0128)
#define NAND_R_TRD_ERROR_INTR_EN                     (0x0130)
#define NAND_R_TRD_COMP_INTR_STATUS                  (0x0138)
#define NAND_R_DMA_TARGET_ERROR_L                    (0x0140)
#define NAND_R_DMA_TARGET_ERROR_H                    (0x0144)
#define NAND_R_BOOT_STATUS                           (0x0148)
#define NAND_R_TRD_TIMEOUT_INTR_STATUS               (0x014C)
#define NAND_R_TRD_TIMEOUT_INTR_EN                   (0x0154)
#define NAND_R_ZQ_CAL_STAT                           (0x015C)

/* Config Registers */
#define NAND_R_TRANSFER_CONFIG_0                     (0x0400)
#define NAND_TRANSFER_CFG_0_OFFSET                   (0xffff0000)
#define NAND_TRANSFER_CFG_0_OFFSET_SHIFT             (16)
#define NAND_TRANSFER_CFG_0_SECTOR_CNT               (0xff)

#define NAND_R_TRANSFER_CONFIG_1                     (0x0404)
#define NAND_TRANSFER_CFG_1_LAST_SECTOR_SIZE         (0xffff0000)
#define NAND_TRANSFER_CFG_1_LAST_SECTOR_SIZE_SHIFT   (16)
#define NAND_TRANSFER_CFG_1_SECTOR_SIZE              (0xffff)

#define NAND_R_LONG_POLLING                          (0x0408)
#define NAND_R_SHORT_POLLING                         (0x040C)

#define NAND_R_RDST_CTRL_0                           (0x0410)
#define NAND_RDST_CTRL_0_RB_EN                       (0x1)

#define NAND_R_RDST_CTRL_1                           (0x0414)
#define NAND_R_LUN_STATUS_CMD                        (0x0418)
#define NAND_R_LUN_INTERLEAVED_CMD                   (0x041C)
#define NAND_R_LUN_ADDR_OFFSET                       (0x0420)

#define NAND_R_NF_DEV_LAYOUT                         (0x0424)
#define NAND_NF_DEV_LAYOUT_BLK_ADDR_IDX              (0xf8000000)
#define NAND_NF_DEV_LAYOUT_LN                        (0xf00000)
#define NAND_NF_DEV_LAYOUT_LN_SHIFT                  (20)
#define NAND_NF_DEV_LAYOUT_LUN_EN                    (0x10000)
#define NAND_NF_DEV_LAYOUT_PPB                       (0xffff)

#define NAND_R_ECC_CONFIG_0                          (0x0428)
#define NAND_ECC_CONFIG_0_CORR_STR                   (0x700)
#define NAND_ECC_CONFIG_0_CORR_STR_SHIFT             (8)
#define NAND_ECC_CONFIG_0_SCRAMBLER                  (0x10)
#define NAND_ECC_CONFIG_0_ERASE_DET                  (0x2)
#define NAND_ECC_CONFIG_0_ECC_EN                     (0x1)

#define NAND_R_ECC_CONFIG_1                          (0x042C)

#define NAND_R_DEVICE_CTRL                           (0x0430)
#define NAND_CTRL_TIMEOUT_EN                         (0x10)

#define NAND_R_MPL_CONFIG                            (0x0434)
#define NAND_MPL_CONFIG_PL_NUM                       (0x300)
#define NAND_MPL_CONFIG_PL_NUM_SHIFT                 (8)
#define NAND_MPL_CONFIG_MPL_WR_EN                    (0x2)
#define NAND_MPL_CONFIG_MPL_RD_EN                    (0x1)

#define NAND_R_CACHE_CONFIG                          (0x0438)
#define NAND_CACHE_CONFIG_WR_EN                      (0x2)
#define NAND_CACHE_CONFIG_RD_EN                      (0x1)

#define NAND_R_DMA_SETTINGS                          (0x043C)
#define NAND_R_SDMA_SIZE                             (0x0440)
#define NAND_R_SDMA_TRD_NUM                          (0x0444)
#define NAND_R_TIME_OUT                              (0x0448)
#define NAND_R_SDMA_ADDR0                            (0x044C)
#define NAND_R_SDMA_ADDR1                            (0x0450)
#define NAND_R_ZQ_CAL_CONFIG                         (0x0458)
#define NAND_R_ZQ_CAL_EN                             (0x045C)
#define NAND_R_LONG_CAL_PERIOD                       (0x0460)
#define NAND_R_SHORT_CAL_PERIOD                      (0x0464)
#define NAND_R_REMAP_CTRL                            (0x0480)
#define NAND_R_REMAP_MASK                            (0x0484)
#define NAND_R_REMAP_ACCESS                          (0x0488)
#define NAND_R_REMAP_LOG_ADDR                        (0x048C)
#define NAND_R_REMAP_PHYS_ADDR                       (0x0490)
#define NAND_R_REMAP_CONTROL_DATA_CTRL               (0x0494)

/*Controller and Device parameters */
#define NAND_R_CTRL_VERSION                          (0x0800)
#define NAND_CTRL_VERSION_REV                        (0xff)

#define NAND_R_CTRL_FEATURES_REG                     (0x0804)
#define NAND_CTRL_FEATURES_NF_16B_SUPP               (0x20000000)
#define NAND_CTRL_FEATURES_NVDDR23_SUPP              (0x10000000)
#define NAND_CTRL_FEATURES_NVDDR_SUPP                (0x8000000)
#define NAND_CTRL_FEATURES_ASYNC_SUPP                (0x4000000)
#define NAND_CTRL_FEATURES_NBANKS                    (0x3000000)
#define NAND_CTRL_FEATURES_NBANKS_SHIFT              (24)
#define NAND_CTRL_FEATURES_DMA_DATA_WIDTH64          (0x200000)
#define NAND_CTRL_FEATURES_DMA_ADDR_WIDTH64          (0x100000)
#define NAND_CTRL_FEATURES_ECC_AVAILABLE             (0x20000)
#define NAND_CTRL_FEATURES_BOOT_AVAILABLE            (0x10000)
#define NAND_CTRL_FEATURES_NTHREADS                  (0xf)

#define NAND_R_MANUFACTRURE_ID                       (0x0808)
#define NAND_MANUFACTRURE_ID_DID                     (0xff0000)
#define NAND_MANUFACTRURE_ID_DID_SHIFT               (16)
#define NAND_MANUFACTRURE_ID_MID                     (0xff)

#define NAND_R_NF_DEVICE_AREAS                       (0x080C)
#define NAND_NF_DEVICE_AREAS_SPARE_SIZE              (0xffff0000)
#define NAND_NF_DEVICE_AREAS_SPARE_SIZE_SHIFT        (16)
#define NAND_NF_DEVICE_AREAS_MAIN_SIZE               (0xffff)

#define NAND_R_DEVICE_PARAMS_0                       (0x0810)
#define NAND_DEVICE_PARAMS_0_LUN_NUM                 (0xff)

#define NAND_R_DEVICE_PARAMS_1                       (0x0814)
#define NAND_R_DEVICE_FEATURES                       (0x0818)
#define NAND_R_DEVICE_BLOCKS_PER_LUN                 (0x081C)
#define NAND_R_DEVICE_REVISION                       (0x0820)
#define NAND_R_ONFI_TIMING_MODES_0                   (0x0824)
#define NAND_R_ONFI_TIMING_MODES_1                   (0x0828)
#define NAND_R_ONFI_ITERLV_OP_ATTR                   (0x082C)
#define NAND_R_ONFI_SYNC_OPT_0                       (0x0830)
#define NAND_R_ONFI_SYNC_OPT_1                       (0x0834)
#define NAND_R_BCH_CFG_0                             (0x0838)
#define NAND_R_BCH_CFG_1                             (0x083C)
#define NAND_R_BCH_CFG_2                             (0x0840)
#define NAND_R_BCH_CFG_3                             (0x0844)

/* Protect Mechanism Registers */
#define NAND_R_PROT_CTRL_0                           (0x0900)
#define NAND_R_PROT_DOWN_0                           (0x0904)
#define NAND_R_PROT_UP_0                             (0x0908)
#define NAND_R_PROT_CTRL_1                           (0x0910)
#define NAND_R_PROT_DOWN_1                           (0x0914)
#define NAND_R_PROT_UP_1                             (0x0918)

/* Minicontroller Registers */
#define NAND_R_WP_SETTINGS                           (0x1000)
#define NAND_R_RNB_SETTINGS                          (0x1004)
#define NAND_R_COMMON_SETTINGS                       (0x1008)
#define NAND_COMMON_SETTINGS_WR_WARMUP               (0xf00000)
#define NAND_COMMON_SETTINGS_WR_WARMUP_SHIFT         (20)
#define NAND_COMMON_SETTINGS_RD_WARMUP               (0xf0000)
#define NAND_COMMON_SETTINGS_RD_WARMUP_SHIFT         (16)
#define NAND_COMMON_SETTINGS_DEV_16BITS              (0x100)
#define NAND_COMMON_SETTINGS_OPR_MODE                (0x3)
#define NAND_MODE_SDR                                (0x0)
#define NAND_MODE_NVDDR                              (0x1)
#define NAND_MODE_NVDDR23                            (0x2)

#define NAND_R_SKIP_CONF                             (0x100c)
#define NAND_SKIP_CONF_MARKER                        (0xffff0000)
#define NAND_SKIP_CONF_MARKER_SHIFT                  (16)
#define NAND_SKIP_CONF_SKIP_BYTES                    (0xff)

#define NAND_R_SKIP_OFFSET                           (0x1010)
#define NAND_R_TOGGLE_TIMINGS_0                      (0x1014)
#define NAND_R_TOGGLE_TIMINGS_1                      (0x1018)
#define NAND_R_ASYNC_TOGGLE_TIMINGS                  (0x101C)
#define NAND_R_SYNC_TIMINGS                          (0x1020)
#define NAND_R_TIMINGS0                              (0x1024)
#define NAND_R_TIMINGS1                              (0x1028)
#define NAND_R_TIMINGS2                              (0x102C)
#define NAND_R_DLL_PHY_UPDATE_CNT                    (0x1030)

#define NAND_R_DLL_PHY_CTRL                          (0x1034)
#define NAND_DLL_PHY_CTRL_DLL_RST_N                  (0x1000000)
#define NAND_DLL_PHY_CTRL_EXTENDED_WR_MODE           (0x20000)
#define NAND_DLL_PHY_CTRL_EXTENDED_RD_MODE           (0x10000)
#define NAND_DLL_PHY_CTRL_RESYNC_HIWAIT_CNT          (0xf00)
#define NAND_DLL_PHY_CTRL_RESYNC_HIWAIT_CNT_SHIFT    (8)
#define NAND_DLL_PHY_CTRL_RESYNC_IDLE_CNT            (0xff)

/* DLL PHY Registers */
#define NAND_R_PHY_DQ_TIMING_REG                     (0x2000)

#define NAND_R_PHY_DQS_TIMING_REG                    (0x2004)
#define NAND_PHY_DQS_TIMING_USE_PHONY_DQS            (0x100000)
#define NAND_PHY_DQS_TIMING_DQS_PHONY_DQS_SEL        (0x10000)
#define NAND_PHY_DQS_TIMING_DQS_SEL_TSEL_START       (0xF000)
#define NAND_PHY_DQS_TIMING_DQS_SEL_TSEL_END         (0xF00)
#define NAND_PHY_DQS_TIMING_DQS_SEL_OE_START         (0xF0)
#define NAND_PHY_DQS_TIMING_DQS_SEL_OE_END           (0xF)

#define NAND_R_PHY_GATE_LPBK_CTRL_REG                (0x2008)
#define NAND_PHY_GATE_LPBK_CTRL_RD_DEL_SEL           (0xF80000)
#define NAND_PHY_GATE_LPBK_CTRL_RD_DEL_SEL_SHIFT     (19)

#define NAND_R_PHY_DLL_MASTER_CTRL_REG               (0x200C)
#define NAND_PHY_DLL_MASTER_PARAM_DLL_LOCK_NUM       (0x70000)
#define NAND_PHY_DLL_MASTER_PARAM_DLL_LOCK_NUM_SHIFT (0x16)
#define NAND_PHY_DLL_MASTER_PARAM_DLL_STAT_POINT     (0x7F)
#define NAND_PHY_DLL_MASTER_CTRL_BYPASS_MODE         (0x1U << 23)

#define NAND_R_PHY_DLL_SLAVE_CTRL_REG                (0x2010)
#define NAND_PHY_DLL_SLAVE_CLK_WR_DELAY              (0x7F00)
#define NAND_PHY_DLL_SLAVE_CLK_WR_DELAY_SHIFT        (8)
#define NAND_PHY_DLL_SLAVE_RD_DQS_DELAY              (0x7F)

#define NAND_R_PHY_OBS_REG_0                         (0x2014)

#define NAND_R_PHY_DLL_OBS_REG_0                     (0x2018)
#define NAND_PHY_DLL_OBS_DLL_LOCK_VALUE              (0x7F00)
#define NAND_PHY_DLL_OBS_DLL_LOCK_VALUE_SHIFT        (8)
#define NAND_PHY_DLL_OBS_DLL_UNLOCK_CNT              (0xF8)
#define NAND_PHY_DLL_OBS_DLL_UNLOCK_CNT_SHIFT        (3)
#define NAND_PHY_DLL_OBS_DLL_LOCKED_MODE             (0x6)
#define NAND_PHY_DLL_OBS_DLL_LOCKED_MODE_SHIFT       (1)
#define NAND_PHY_DLL_OBS_DLL_LOCK                    (0x1)

#define NAND_R_PHY_DLL_OBS_REG_1                     (0x201C)

/* Control Timing block Register */
#define NAND_R_PHY_CTRL                              (0x2080)
#define NAND_PHY_CTRL_SDR_DQS_VALUE                  (0x4000)
#define NAND_PHY_CTRL_PHONY_DQS_TIMING               (0x1F0)

#define NAND_R_PHY_TSEL                              (0x2084)
#define NAND_R_PHY_GPIO_CTR                          (0x2088)
#define NAND_R_PHY_GPIO_STATUS                       (0x208C)

/* Descriptor cmd flags */
#define NAND_DESC_CFLAGS_MDMA                        (0x400)
#define NAND_DESC_CFLAGS_CONT                        (0x200)
#define NAND_DESC_CFLAGS_INT                         (0x100)
#define NAND_DESC_CFLAGS_VOLUME                      (0xF0)
#define NAND_DESC_CFLAGS_VOLUME_SHIFT                (4)

/* Command layout */
#define NAND_CMD_LAYOUT_CDMA                         (0x0U << 30)
#define NAND_CMD_LAYOUT_PIO                          (0x1U << 30)
#define NAND_CMD_LAYOUT_GENERIC                      (0x3U << 30)

#define NAND_CMD_CT_SETFEAT                          (0x0100U)
#define NAND_CMD_CT_ERASE                            (0x1000U)
#define NAND_CMD_CT_RST                              (0x1100U)
#define NAND_CMD_CT_CB                               (0x1200U)
#define NAND_CMD_CT_WR                               (0x2100U)
#define NAND_CMD_CT_RD                               (0x2200U)
#define NAND_CMD_CT_NOP                              (0xFFFFU)

#define NAND_CMD_VOL_ID(x)                           ((x & 0xf) << 16)
#define NAND_CMD_INT                                 (0x1U << 20)
#define NAND_CMD_MDMA                                (0x1U << 21)
#define NAND_CMD_TRD_NUM(x)                          ((x & 0x7) << 24)

/* Generic Work mode cmd layout */
#define NAND_GCMD_LAYOUT_TWB                         (0x1U << 6)
#define NAND_GCMD_LAYOUT_BANK_NUM(x)                 ((x & 0x7) << 8)
#define NAND_GCMD_LAYOUT_CE_HOLD                     (0x1 << 15)

/* Generic Work mode instr type */
#define NAND_GCMD_LAY_INSTR_CMD                      (0)
#define NAND_GCMD_LAY_INSTR_ADDR                     (1)
#define NAND_GCMD_LAY_INSTR_DATA                     (2)

#define NAND_GCMD_LAY_ADDR_NBYTES(x)                 ((x & 0x7) << 11)
#define NAND_GCMD_LAY_DATA_DIR(x)                    ((x & 0x1) << 11)
#define NAND_GCMD_LAY_DATA_ECC_EN                    (0x1 << 12)
#define NAND_GCMD_LAY_DATA_SCRAMBLER_EN              (0x1 << 13)
#define NAND_GCMD_LAY_DATA_PAGEDET_EN                (0x1 << 14)
#define NAND_GCMD_LAY_DATA_SECT_SIZE(x)              (((u64)x & 0xffff) << 16)
#define NAND_GCMD_LAY_DATA_SECT_CNT(x)               (((u64)x & 0xff) << 32)
#define NAND_GCMD_LAY_DATA_LAST_SECT_SIZE(x)         (((u64)x & 0xffff) << 40)
#define NAND_GCMD_LAY_DATA_CORR_CAP(x)               (((u64)x & 0x7) << 56)

/******************************Type Definition******************************/

/******************************Macro (inline function) Definition***********/

/******************************Variable Definition**************************/

/******************************Function Prototype***************************/
/*****************************************************************************
 * This function is used to poll ctrl_status.init_comp bit, 1 is valid.
 *
 * @param
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_PollInitComp(FNandPsu_T *nfcPtr, int timeout_us);

/*****************************************************************************
 * This function is used to poll ctrl_status.ctrl_busy bit, 0 is valid.
 *
 * @param
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_PollCtrlReady(FNandPsu_T *nfcPtr, int timeout_us);

/*****************************************************************************
 * This function is used to poll trd_status.trd_busy bit, 0 is valid.
 *
 * @param
 *       thread: thread number to poll
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_PollTrdReady(FNandPsu_T *nfcPtr, int thread, int timeout_us);

/*****************************************************************************
 * This function is used to poll trd_comp_intr_status.trdx_comp bit, 1 is valid.
 *
 * @param
 *       thread: thread number to poll
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_PollTrdComp(FNandPsu_T *nfcPtr, int thread, int timeout_us);

/*****************************************************************************
 * This function is used to poll cmd_status.comp bit, 1 is valid.
 *
 * @param
 *       thread: thread number to poll
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_PollCmdComp(FNandPsu_T *nfcPtr, int thread, int timeout_us);

/*****************************************************************************
 * This function read cmd_status reg with specified thread.
 *
 * @param
 *       thread: thread number to poll
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
u32 FNandPsu_GetCmdStatus(FNandPsu_T *nfcPtr, int thread);

/*****************************************************************************
 * This function is used to poll rbn settings.Rbn bit, 1 is valid.
 *
 * @param
 *       thread: thread number to poll
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_PollDevReady(FNandPsu_T *nfcPtr, int cs, int timeout_us);

/*****************************************************************************
 * This function is used to select type for read/write by setting transfer_cfg_0
 * and transfer_cfg_1 reg. 3 types can  be used.
 *
 * @param
 *       type: NAND_TT_PAGE_RAW - data & oob(raw)
 *             NAND_TT_MAIN_OOB - data & oob(ecc)
 *             NAND_TT_OOB - oob only(raw)
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_SetPageType(FNandPsu_T *nfcPtr, int type);

/*****************************************************************************
 * This function is used to configure ecc durineg initialization.
 * ecc_config_0 and ecc_config_1 are configured.
 *
 * @param
 *       enable: ecc enable
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_EccConf(FNandPsu_T *nfcPtr, int enable);

/*****************************************************************************
 * This function is used to enable/disable ecc, other configurations
 * are not changed
 *
 * @param
 *       enable: ecc enable
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_SetEccEnable(FNandPsu_T *nfcPtr, int enable);

/*****************************************************************************
 * This function is used to configure skip for bbt.
 * skip_bytes_conf and skip_bytes_offset are configured.
 *
 * @param
 *
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_SetSkipByte(FNandPsu_T *nfcPtr, u8 *marker, unsigned int offset,
                         unsigned int len);

/*****************************************************************************
 * This function is used to configure device & lun operation durineg
 *initialization. nf_dev_layout, lun_addr_offset, lun_status_cmd and
 *lun_interleaved_cmd reg. lun_interleaved_cmd are configured.
 *
 * @param
 *       enable: multilun enable
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_DeviceConf(FNandPsu_T *nfcPtr, int enable);

/*****************************************************************************
 * This function is used to configure multiplane operation
 * durineg initialization.
 * multiplane_config is configured.
 *
 * @param
 *
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_MultiPlaneConf(FNandPsu_T *nfcPtr, int enable);

/*****************************************************************************
 * This function is used to configure cache operation
 * durineg initialization.
 *cache_config reg is configured.
 *
 * @param
 *
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
int FNandPsu_CacheConf(FNandPsu_T *nfcPtr, int enable);

int FNandPsu_SetPolling(FNandPsu_T *nfcPtr, u32 long_poll, u32 short_poll,
                        u8 rbn);

int FNandPsu_SetTimings(FNandPsu_T *nfcPtr, struct hpnfc_timings *timings);

int FNandPsu_PollSdmaTrigg(FNandPsu_T *qspiPtr, int timeout_us);

int FNandPsu_EnableAllTrdIntr(FNandPsu_T *nfcPtr);

/****************************************************************************
 * FNandPsu_SetIntrEnable  - Config interrupt mask and enable interrupt
 *
 * This function is used to set interrupt mask and enable interrupts.
 * Controller has few registers related to interrupts, such as intr_status,
 * trd_error, trd_comp, trd_timeout.
 *
 *
 * Returns 0 on success, a negative error code otherwise.
 ***************************************************************************/
int FNandPsu_SetIntrStatusEnable(FNandPsu_T *nfcPtr, u32 mask, u8 enable);
int FNandPsu_ClearAllIntr(FNandPsu_T *nfcPtr);
int FNandPsu_InitIntrStatus(FNandPsu_T *nfcPtr);

/*****************************************************************************
 * This functions is used for setting IO width
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FNandPsu_SetIOWidth16(FNandPsu_T *nfcPtr, int width16);

/*****************************************************************************
 * This functions are used for getting hardware capabilities
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FNandPsu_GetHwCaps(FNandPsu_T *nfcPtr);
int FNandPsu_GetBchCaps(FNandPsu_T *nfcPtr, struct hpnfc_bchcaps *bch);

/*****************************************************************************
 * This function is used for ZQ calibrate
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FNandPsu_ZQCalConf(FNandPsu_T *nfcPtr, u32 short_cal, u32 long_cal);

/*****************************************************************************
 * This functions are used for remap
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FNandPsu_GetRecord(FNandPsu_T *nfcPtr, int cs, int idx, u32 *logic,
                       u32 *phy);
int FNandPsu_AddRecord(FNandPsu_T *nfcPtr, int cs, u32 phy, u32 logic,
                       u32 *idx);
int FNandPsu_ClearRecord(FNandPsu_T *nfcPtr);
int FNandPsu_SetRmpMask(FNandPsu_T *nfcPtr, u32 mask);
int FNandPsu_SetRmpEnable(FNandPsu_T *nfcPtr, int enable);

/*****************************************************************************
 * This functions are used for protect
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FNandPsu_SetProtEnable(FNandPsu_T *nfcPtr, int cs, int num, u32 range_low,
                           u32 range_high, int enable);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
