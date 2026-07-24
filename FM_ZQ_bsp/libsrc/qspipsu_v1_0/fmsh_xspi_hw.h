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
 * @file fmsh_xqspi_hw.h
 * @addtogroup qspipsu_v1_0
 * @{
 *
 * This header file contains the identifiers and basic HW access driver
 * functions (or macros) that can be used to access the device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2022/12/22  First release
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef _FMSH_XSPI_HW_H_ /* prevent circular inclusions */
#define _FMSH_XSPI_HW_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_xspi.h"

/************************** Constant Definitions *****************************/
#define QSPI_R_CMD_REG0       (0x00)
#define QSPI_R_CMD_REG1       (0x04)
#define QSPI_R_CMD_REG2       (0x08)
#define QSPI_R_CMD_REG3       (0x0C)
#define QSPI_R_CMD_REG4       (0x10)
#define QSPI_R_CMD_REG5       (0x14)
#define QSPI_R_CMD_STATUS_PTR (0x40)
#define QSPI_R_CMD_STATUS     (0x44)
#define QSPI_R_CTRL_STATUS    (0x100)
#define QSPI_INIT_COMP        (0x1 << 16)
#define QSPI_INIT_FAIL        (0x1 << 15)
#define QSPI_CTRL_BUSY        (0x1 << 7)
#define QSPI_DISCOVERY_BUSY   (0x1 << 6)
#define QSPI_GCMD_ENG_MC_BUSY (0x1 << 4)
#define QSPI_GCMD_ENG_BUSY    (0x1 << 3)
#define QSPI_ACMD_ENG_BUSY    (0x1 << 2)
#define QSPI_MDMA_BUSY        (0x1 << 1)
#define QSPI_SDMA_BUSY        (0x1 << 0)

#define QSPI_R_TRD_STATUS     (0x104)
#define QSPI_BUSY_TRD(trd)    (0x1 << trd)

#define QSPI_R_INTR_STATUS    (0x110)
#define QSPI_R_INTR_ENABLE    (0x114)
#define QSPI_INTR_EN          (0x1U << 31)
#define QSPI_INTR_DIR_CMD_ERR (0x1 << 26)
#define QSPI_INTR_STIG_DONE   (0x1 << 23)
#define QSPI_INTR_SDMA_ERR    (0x1 << 22)
#define QSPI_INTR_SDMA_TRIGG  (0x1 << 21)
#define QSPI_INTR_CMD_IGNORED (0x1 << 20)
#define QSPI_INTR_DDMA_TERR   (0x1 << 18)
#define QSPI_INTR_CDMA_TERR   (0x1 << 17)
#define QSPI_INTR_CTRL_IDLE   (0x1 << 16)

#define QSPI_INTR_MASK_ALL                                                \
    (QSPI_INTR_DIR_CMD_ERR | QSPI_INTR_STIG_DONE | QSPI_INTR_SDMA_ERR |   \
     QSPI_INTR_SDMA_TRIGG | QSPI_INTR_CMD_IGNORED | QSPI_INTR_DDMA_TERR | \
     QSPI_INTR_CDMA_TERR | QSPI_INTR_CTRL_IDLE)
#define QSPI_INTR_MASK_STIG                                            \
    (QSPI_INTR_STIG_DONE | QSPI_INTR_SDMA_ERR | QSPI_INTR_SDMA_TRIGG | \
     QSPI_INTR_CTRL_IDLE)
#define QSPI_INTR_MASK_SDMA \
    (QSPI_INTR_SDMA_ERR | QSPI_INTR_SDMA_TRIGG | QSPI_INTR_CTRL_IDLE)

#define QSPI_R_TRD_COMP_INTR_STATUS  (0x120)
#define QSPI_R_TRD_ERROR_INTR_STATUS (0x130)
#define QSPI_R_TRD_ERROR_INTR_EN     (0x134)
#define QSPI_R_DMA_TARGET_ERROR_L    (0x150)
#define QSPI_R_DMA_TARGET_ERROR_H    (0x154)
#define QSPI_R_BOOT_STATUS           (0x158)

#define QSPI_R_LONG_POLLING          (0x208)
#define QSPI_R_SHORT_POLLING         (0x20C)
#define QSPI_R_CTRL_CONFIG           (0x230)
#define QSPI_WORK_MODE_MASK          (0x3 << 5)
#define QSPI_WORK_MODE(mode)         ((mode & 0x3) << 5)
#define QSPI_WORK_MODE_DIRECT        (0x0)
#define QSPI_WORK_MODE_STIG          (0x1)
#define QSPI_WORK_MODE_ACMD          (0x3)

#define QSPI_R_DMA_SETTING           (0x23C)
#define QSPI_R_SDMA_SIZE             (0x240)
#define QSPI_R_SDMA_TRD_INFO         (0x244)
#define QSPI_SDMA_DIR                (0x1 << 8)
#define QSPI_SDMA_DIR_READ           (0)
#define QSPI_SDMA_DIR_WRITE          (0x1 << 8)

#define QSPI_R_SDMA_ADDR0            (0x24C)
#define QSPI_R_SDMA_ADDR1            (0x250)
#define QSPI_R_DISCOVERY_CONTROL     (0x260)

#define QSPI_R_XIP_MODE_CFG          (0x388)
#define QSPI_DIS_MB_VAL(val)         ((val & 0xff) << 16)
#define QSPI_EN_MB_VAL(val)          ((val & 0xff) << 8)
#define QSPI_XIP_EN(bank)            (0x1 << bank)

#define QSPI_R_GLOBAL_SEQ_CFG        (0x390)
#define QSPI_SEQ_TYPE(type)          ((type & 0x3) << 23)
#define QSPI_SEQ_TYPE_P1             (0x0)
#define QSPI_SEQ_TYPE_P2HF           (0x1)
#define QSPI_SEQ_TYPE_P2HR           (0x2)
#define QSPI_SEQ_TYPE_SPINAND        (0x3)

#define QSPI_R_GLOBAL_SEQ_CFG_1      (0x394)
#define QSPI_R_DIRECT_ACCESS_CFG     (0x398)
#define QSPI_DAC_ADDR_MASK(mask)     ((mask & 0x1fff) << 16)
#define QSPI_DAC_RMP_ADDR_EN         (0x1 << 12)
#define QSPI_DAC_MB_XIP_DIS          (0x1 << 9)
#define QSPI_DAC_MB_XIP_EN           (0x1 << 8)
#define QSPI_DAC_BANK_NUM_MASK       (0x7)
#define QSPI_DAC_BANK_NUM(bank)      (bank & 0x7)

#define QSPI_R_DIRECT_ACCESS_RMP     (0x39C)
#define QSPI_R_DIRECT_ACCESS_RMP_1   (0x3A0)

#define QSPI_R_RST_SEQ_CFG_0         (0x400)
#define QSPI_R_RST_SEQ_CFG_1         (0x404)
#define QSPI_R_ERS_SEQ_CFG_0         (0x410)
#define QSPI_R_ERS_SEQ_CFG_1         (0x414)
#define QSPI_R_ERS_SEQ_CFG_2         (0x418)
#define QSPI_R_PROG_SEQ_CFG_0        (0x420)
#define QSPI_R_PROG_SEQ_CFG_1        (0x424)
#define QSPI_PROG_SEQ_CMD_EXT_EN     (0x1)

#define QSPI_R_PROG_SEQ_CFG_2        (0x428)
#define QSPI_R_READ_SEQ_CFG_0        (0x430)
#define QSPI_R_READ_SEQ_CFG_1        (0x434)
#define QSPI_READ_SEQ_MB_EN          (0x1U << 31)
#define QSPI_READ_SEQ_DUMMY(n)       ((n & 0x3f) << 24)
#define QSPI_READ_SEQ_CMD_EXT_EN     (0x1)

#define QSPI_R_READ_SEQ_CFG_2        (0x438)
#define QSPI_R_WE_SEQ_CFG_0          (0x440)
#define QSPI_R_STAT_SEQ_CFG_0        (0x450)
#define QSPI_R_STAT_SEQ_CFG_1        (0x454)
#define QSPI_R_STAT_SEQ_CFG_2        (0x458)
#define QSPI_R_STAT_SEQ_CFG_3        (0x45C)
#define QSPI_R_STAT_SEQ_CFG_4        (0x460)
#define QSPI_R_STAT_SEQ_CFG_5        (0x464)
#define QSPI_R_STAT_SEQ_CFG_7        (0x46C)
#define QSPI_R_STAT_SEQ_CFG_8        (0x470)
#define QSPI_R_STAT_SEQ_CFG_9        (0x474)
#define QSPI_R_STAT_SEQ_CFG_10       (0x478)

#define QSPI_R_CTRL_VERSION          (0xF00)
#define QSPI_R_CTRL_FEATURES         (0xF04)

#define QSPI_R_WP_SETTINGS           (0x1000)
#define QSPI_WP_ENABLE               (0x1 << 1)

#define QSPI_R_RESET_PIN_SETTINGS    (0x1004)
#define QSPI_R_CLOCK_MODE_SETTINGS   (0x1008)
#define QSPI_R_JEDEC_RST_TIMING      (0x100C)
#define QSPI_R_DEV_DELAY             (0x1010)
#define QSPI_R_RST_DISCOVERY         (0x1014)
#define QSPI_R_ACTIVE_MAX            (0x1018)
#define QSPI_R_HF_OFFSET             (0x1020)
#define QSPI_R_DLL_PHY_UPDATE_CNT    (0x1030)
#define QSPI_R_DLL_PHY_CTRL          (0x1034)
#define QSPI_DLL_RST_N               (0x1 << 24)

#define QSPI_R_PHY_DQ_TIMING         (0x2000)
#define QSPI_R_PHY_DQS_TIMING        (0x2004)
#define QSPI_R_PHY_GATE_LPBK_CTRL    (0x2008)
#define QSPI_R_PHY_DLL_MASTER_CTRL   (0x200C)
#define QSPI_R_PHY_DLL_SLAVE_CTRL    (0x2010)
#define QSPI_R_PHY_OBS_0             (0x2018)
#define QSPI_R_PHY_DLL_OBS_0         (0x201C)
#define QSPI_R_PHY_WR_DESKEW         (0x202C)
#define QSPI_R_PHY_WR_RD_DESKEW_CMD  (0x2030)
#define QSPI_R_PHY_RD_DESKEW         (0x203C)
#define QSPI_R_PHY_VERSION           (0x2070)
#define QSPI_R_PHY_FEATURE           (0x2074)

#define QSPI_R_PHY_CTRL              (0x2080)
#define QSPI_R_PHY_TSEL              (0x2084)

#define QSPI_R_PHY_TUNING            (0x2800)
#define QSPI_R_PHY_DQS_IDELAY        (0x2804)
#define QSPI_R_PHY_ODELAY            (0x2808)
#define QSPI_R_PHY_DQS_SHIFT_STAT    (0x280C)
#define QSPI_R_PHY_DQ_IDELAY_0_STAT  (0x2810)
#define QSPI_R_PHY_DQ_IDELAY_1_STAT  (0x2814)
#define QSPI_R_PHY_DQ_ODELAY_0_STAT  (0x2818)
#define QSPI_R_PHY_DQ_ODELAY_1_STAT  (0x281C)
#define QSPI_R_PHY_DQS_IDELAY_STAT   (0x2820)
#define QSPI_R_PHY_ODELAY_STAT       (0x2824)
#define QSPI_R_PHY_CMD_IODELAY_STAT  (0x2828)

/***** Command type *****/
#define QSPI_CT_ERASE                (0x1000)
#define QSPI_CT_ERASE_ALL            (0x1001)
#define QSPI_CT_RESET                (0x1100)
#define QSPI_CT_JEDEC_RESET          (0x1101)
#define QSPI_CT_PROG                 (0x2100)
#define QSPI_CT_READ                 (0x2200)

/***** Command flags *****/
#define QSPI_CF_MDMA                 (0x1 << 10)
#define QSPI_CF_CONT                 (0x1 << 9)
#define QSPI_CF_INT                  (0x1 << 8)
#define QSPI_CF_MB_XIP_DIS           (0x1 << 7)
#define QSPI_CF_MB_XIP_EN            (0x1 << 6)
#define QSPI_CF_XSPI_PTR_CONT        (0x1 << 5)
#define QSPI_CF_SYS_PTR_CONT         (0x1 << 4)

/***** Command status *****/
#define QSPI_STATUS_ECC_STAT         (0xff << 16)
#define QSPI_STATUS_COMPLETE         (0x1 << 15)
#define QSPI_STATUS_FAIL             (0x1 << 14)
#define QSPI_STATUS_ECC_CORR_ERROR   (0x1 << 5)
#define QSPI_STATUS_DEVICE_ERROR     (0x1 << 4)
#define QSPI_STATUS_DQS_ERROR        (0x1 << 3)
#define QSPI_STATUS_CRC_ERROR        (0x1 << 2)
#define QSPI_STATUS_BUS_ERROR        (0x1 << 1)
#define QSPI_STATUS_CMD_DESC_ERROR   (0x1 << 0)

#define QSPI_STATUS_STIG_ERR                                            \
    (QSPI_STATUS_FAIL | QSPI_STATUS_DQS_ERROR | QSPI_STATUS_CRC_ERROR | \
     QSPI_STATUS_BUS_ERROR | QSPI_STATUS_CMD_DESC_ERROR)
#define QSPI_STATUS_ADMA_ERR                            \
    (QSPI_STATUS_FAIL | QSPI_STATUS_ECC_CORR_ERROR |    \
     QSPI_STATUS_DEVICE_ERROR | QSPI_STATUS_DQS_ERROR | \
     QSPI_STATUS_CRC_ERROR | QSPI_STATUS_BUS_ERROR |    \
     QSPI_STATUS_CMD_DESC_ERROR)

/***** Interrupt mask *****/
#define QSPI_INTR_PIO_MASK      (0)
#define QSPI_INTR_CDMA_MASK     (0)

/***** Instruction Type *****/
#define QSPI_SEQ_P1_GENERAL     (0)
#define QSPI_SEQ_P1_DATA_GLUING (1)
#define QSPI_SEQ_CMD            (96)
#define QSPI_SEQ_DATA           (127)

#define QSPI_NAND2_DELAY        (15)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
int FQspiPsu_WaitCtrlIdle(FQspiPsu_T *qspiPtr);
int FQspiPsu_WaitSdmaTrigg(FQspiPsu_T *qspiPtr, int timeout_us);
int FQspiPsu_WaitTrdReady(FQspiPsu_T *qspiPtr, int timeout_us);
int FQspiPsu_WaitCmdComplete(FQspiPsu_T *qspiPtr, int timeout_us);
int FQspiPsu_CheckCmdStatus(FQspiPsu_T *qspiPtr, u32 *status);

int FQspiPsu_SetWorkMode(FQspiPsu_T *qspiPtr, int mode);

u32 FQspiPsu_IntrEnabled(FQspiPsu_T *qspiPtr);
void FQspiPsu_ClearIntr(FQspiPsu_T *qspiPtr, u32 mask);
void FQspiPsu_SetIntrMask(FQspiPsu_T *qspiPtr, u32 mask, int enable);

void FQspiPsu_SetClkMode(FQspiPsu_T *qspiPtr, int mode);
void FQspiPsu_DQ2Toggle(FQspiPsu_T *qspiPtr, int dq2_line, int enable);
void FQspiPsu_DQ3Toggle(FQspiPsu_T *qspiPtr, int dq3_line, int enable);

void FQspiPsu_SetXipEn(FQspiPsu_T *qspiPtr, int xip_en);

void FQspiPsu_SetDelays(FQspiPsu_T *qspiPtr, u8 csda, u8 cseot, u8 cssot);
void FQspiPsu_GetDelays(FQspiPsu_T *qspiPtr, u8 *csda, u8 *cseot, u8 *cssot);

int FQspiPsu_SetDma(FQspiPsu_T *qspiPtr, int word_size, int ote, int burst_sel,
                    int err_resp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
