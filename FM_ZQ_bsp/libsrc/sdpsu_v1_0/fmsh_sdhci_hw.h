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
 * @file fmsh_sdhci_hw.h
 * @addtogroup sdpsu_v1_0
 * @{
 *
 * This header file contains the identifiers and basic HW access driver
 * functions (or  macros) that can be used to access the device. Other driver
 * functions are defined in fmsh_sdpsu.h.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ---    -------- -----------------------------------------------
 * 1.00  hzq  22/10/31 Initial release
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef FMSH_SDHCI_HW_H_
#define FMSH_SDHCI_HW_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_sdhci.h"

/************************** Constant Definitions *****************************/
#define SDHCI_HOST_VERSION        (0x06020002)

/***** Host registers (specific to Cadence) *****/
#define SDHCI_HRS_BASE            (0x0000U)
#define SDHCI_SRS_BASE            (0x0200U)
#define SDHCI_CQRS_BASE           (0x0400U)
#define SDHCI_PHY_REG_BASE        (0x2000U)

/***** Host registers (detail) *****/
#define SDHCI_HRS00               (0x000U)
#define SDHCI_HRS00_SOFTRST       0x1

#define SDHCI_HRS01               (0x004U)
#define SDHCI_DP_MS(freq, x)      ((uint32_t)(x * (float)freq / 1000))

#define SDHCI_HRS02               (0x008U)

#define SDHCI_HRS03               (0x00CU)

#define SDHCI_HRS04               (0x010U)

#define SDHCI_HRS05               (0x014U)

#define SDHCI_HRS06               (0x018U)
#define SDHCI_HRS06_EMM_SD        0x0U
#define SDHCI_HRS06_EMM_LEGACY    0x1U
#define SDHCI_HRS06_EMM_SDR       0x2U
#define SDHCI_HRS06_EMM_DDR       0x3U
#define SDHCI_HRS06_EMM_HS200     0x4U
#define SDHCI_HRS06_EMM_HS400     0x5U
#define SDHCI_HRS06_EMM_HS400ES   0x6U

#define SDHCI_HRS07               (0x01CU)

#define SDHCI_HRS08               (0x020U)
#define SDHCI_HRS08_PHYDLL_UPDACK 0x2U
#define SDHCI_HRS08_PHYDLL_UPDREQ 0x1U

#define SDHCI_HRS09               (0x024U)

#define SDHCI_HRS10               (0x028U)

#define SDHCI_HRS11               (0x02CU)

#define SDHCI_HRS12               (0x030U)
#define SDHCI_HRS12_PHYDATOF      0x8U
#define SDHCI_HRS12_PHYDATUR      0x4U
#define SDHCI_HRS12_PHYCMDOF      0x2U
#define SDHCI_HRS12_PHYCMDUR      0x1U

#define SDHCI_HRS13               (0x034U)

#define SDHCI_HRS14               (0x038U)

#define SDHCI_HRS16               (0x040U)

#define SDHCI_HRS29               (0x074U)

#define SDHCI_HRS30               (0x078U)

#define SDHCI_HRS31               (0x07CU)

#define SDHCI_HRS32               (0x080U)

#define SDHCI_HRS33               (0x084U)

#define SDHCI_HRS34               (0x088U)

#define SDHCI_HRS36               (0x090U)

#define SDHCI_HRS40               (0x0A0U)

#define SDHCI_HRS41               (0x0A4U)

#define SDHCI_HRS42               (0x0A8U)

#define SDHCI_HRS43               (0x0ACU)

/***** SDHCI registers *****/
// SDMA System Address/Argument2/32-bit Block Count
#define SDHCI_SRS00               (0x200U)
// Block Size/Block Count
#define SDHCI_SRS01               (0x204U)
#define SDHCI_SRS01_TBS           (0xfffU)
#define SDHCI_SRS01_SDMABB        (0x3U << 12)
#define SDHCI_SRS01_SDMABB_4K     (0x0U << 12)
#define SDHCI_SRS01_SDMABB_8K     (0x1U << 12)
#define SDHCI_SRS01_SDMABB_16K    (0x2U << 12)
#define SDHCI_SRS01_SDMABB_32K    (0x3U << 12)
#define SDHCI_SRS01_SDMABB_64K    (0x4U << 12)
#define SDHCI_SRS01_SDMABB_128K   (0x5U << 12)
#define SDHCI_SRS01_SDMABB_256K   (0x6U << 12)
#define SDHCI_SRS01_SDMABB_512K   (0x7U << 12)
#define SDHCI_SRS01_BCCT          (0xffffU << 16)
// Argument1
#define SDHCI_SRS02               (0x208U)
// Command/Transfer Mode
#define SDHCI_SRS03               (0x20CU)
#define SDHCI_SRS03_DMAE          (0x1U)
#define SDHCI_SRS03_BCE           (0x1U << 1)
#define SDHCI_SRS03_ACE           (0x3U << 2)
#define SDHCI_ACE_NONE            (0x0U << 2)
#define SDHCI_ACE_CMD12           (0x1U << 2)
#define SDHCI_ACE_CMD23           (0x1U << 3)
#define SDHCI_ACE_AUTO            (0x3U << 2)
#define SDHCI_SRS03_DTDS          (0x1U << 4)
#define SDHCI_DTDS_READ           (0x1U << 4)
#define SDHCI_SRS03_MSBS          (0x1U << 5)
#define SDHCI_MSBS_MULTI          (0x1U << 5)
#define SDHCI_SRS03_RECT          (0x1U << 6)
#define SDHCI_SRS03_RECE          (0x1U << 7)
#define SDHCI_SRS03_RID           (0x1U << 8)
#define SDHCI_SRS03_RTS           (0x3U << 16)
#define SDHCI_RTS_NONE            (0x0U << 16)
#define SDHCI_RTS_LONG            (0x1U << 16)
#define SDHCI_RTS_SHORT           (0x2U << 16)
#define SDHCI_RTS_SHORT_BUSY      (0x3U << 16)
#define SDHCI_SRS03_SCF           (0x1U << 18)
#define SDHCI_SRS03_CRCCE         (0x1U << 19)
#define SDHCI_SRS03_CICE          (0x1U << 20)
#define SDHCI_SRS03_DPS           (0x1U << 21)
#define SDHCI_SRS03_CT            (0x3U << 22)
#define SDHCI_CT_NORMAL           (0x0U)
#define SDHCI_CT_SUSPEND          (0x1U << 22)
#define SDHCI_CT_RESUME           (0x2U << 22)
#define SDHCI_CT_ABORT            (0x3U << 22)
#define SDHCI_SRS03_CIDX          (0x3fU << 24)
// Response0
#define SDHCI_SRS04               (0x210U)
// Response1
#define SDHCI_SRS05               (0x214U)
// Response2
#define SDHCI_SRS06               (0x218U)
// Response3
#define SDHCI_SRS07               (0x21CU)
// Data Buffer
#define SDHCI_SRS08               (0x220U)
// Present State
#define SDHCI_SRS09               (0x224U)
#define SDHCI_SRS09_CICMD         (0x1U)
#define SDHCI_SRS09_CIDAT         (0x1U << 1)
#define SDHCI_SRS09_DLA           (0x1U << 2)
#define SDHCI_SRS09_DATSL2        (0xfU << 4)
#define SDHCI_DATSL_4             (0x1U << 4)
#define SDHCI_DATSL_5             (0x1U << 5)
#define SDHCI_DATSL_6             (0x1U << 6)
#define SDHCI_DATSL_7             (0x1U << 7)
#define SDHCI_SRS09_WTA           (0x1U << 8)
#define SDHCI_SRS09_RTA           (0x1U << 9)
#define SDHCI_SRS09_BWE           (0x1U << 10)
#define SDHCI_SRS09_BRE           (0x1U << 11)
#define SDHCI_SRS09_CI            (0x1U << 16)
#define SDHCI_SRS09_CSS           (0x1U << 17)
#define SDHCI_SRS09_CDSL          (0x1U << 18)
#define SDHCI_SRS09_WPSL          (0x1U << 19)
#define SDHCI_SRS09_DATSL1        (0xfU << 20)
#define SDHCI_DATSL_0             (0x1U << 20)
#define SDHCI_DATSL_1             (0x1U << 21)
#define SDHCI_DATSL_2             (0x1U << 22)
#define SDHCI_DATSL_3             (0x1U << 23)
#define SDHCI_SRS09_CMDSL         (0x1U << 24)
#define SDHCI_SRS09_LVSIRSLT      (0x1U << 26)
// Host Control1(General, Power, Blcok-Gap, WakeUp)
#define SDHCI_SRS10               (0x228U)
#define SDHCI_SRS10_LEDC          (0x1U)
#define SDHCI_SRS10_DTW           (0x1U << 1)
#define SDHCI_SRS10_HSE           (0x1U << 2)
#define SDHCI_SRS10_DMASEL        (0x3U << 3)
#define SDHCI_DMASEL_SDMA         (0x0U << 3)
#define SDHCI_DMASEL_ADMA2        (0x2U << 3)
#define SDHCI_DMASEL_ADMA3        (0x3U << 3)
#define SDHCI_SRS10_EDTW          (0x1U << 5)
#define SDHCI_SRS10_WIDTH         (SDHCI_SRS10_EDTW | SDHCI_SRS10_DTW)
#define SDHCI_WIDTH_1             (0x0U << 1)
#define SDHCI_WIDTH_4             (0x1U << 1)
#define SDHCI_WIDTH_8             (0x1U << 5)
#define SDHCI_SRS10_CDTL          (0x1U << 6)
#define SDHCI_SRS10_CDSS          (0x1U << 7)
#define SDHCI_SRS10_BP            (0x1U << 8)
#define SDHCI_SRS10_BVS           (0x7U << 9)
#define SDHCI_BVS_180             (0x5U << 9)
#define SDHCI_BVS_300             (0x6U << 9)
#define SDHCI_BVS_330             (0x7U << 9)
// Host Control2(Clock, Timeout, Reset)
#define SDHCI_SRS11               (0x22CU)
#define SDHCI_SRS11_CLOCK         (0xFFFFU)
#define SDHCI_CLOCK_ICE           (0x1U)
#define SDHCI_CLOCK_ICS           (0x1U << 1)
#define SDHCI_CLOCK_SDCE          (0x1U << 2)
#define SDHCI_CLOCK_SDCFSH        (0x3U << 6)
#define SDHCI_CLOCK_ISDCFSL       (0xFFU << 8)
#define SDHCI_SRS11_TIMEOUT       (0xFU << 16)
#define SDHCI_TIMEOUT_DTCV        (0xFU << 16)
#define SDHCI_SRS11_RESET         (0xFU << 16)
#define SDHCI_RESET_SRFA          (0x1U << 24)
#define SDHCI_RESET_SRCMD         (0x1U << 25)
#define SDHCI_RESET_SRDAT         (0x1U << 26)
// Error/Normal INterrupt Status
#define SDHCI_SRS12               (0x230U)
#define SDHCI_SRS12_INT           (0xffffU)
#define SDHCI_INT_CC              (0x1U)
#define SDHCI_INT_TC              (0x1U << 1)
#define SDHCI_INT_BGE             (0x1U << 2)
#define SDHCI_INT_DMAINT          (0x1U << 3)
#define SDHCI_INT_BWR             (0x1U << 4)
#define SDHCI_INT_BRR             (0x1U << 5)
#define SDHCI_INT_CIN             (0x1U << 6)
#define SDHCI_INT_CR              (0x1U << 7)
#define SDHCI_INT_CINT            (0x1U << 8)
#define SDHCI_INT_FXE             (0x1U << 13)
#define SDHCI_INT_CQINT           (0x1U << 14)
#define SDHCI_INT_EINT            (0x1U << 15)
#define SDHCI_SRS12_ERROR         (0xffffU << 16)
#define SDHCI_ERR_ECT             (0x1U << 16)
#define SDHCI_ERR_ECCRC           (0x1U << 17)
#define SDHCI_ERR_ECEB            (0x1U << 18)
#define SDHCI_ERR_ECI             (0x1U << 19)
#define SDHCI_ERR_CMDLINE \
    (SDHCI_ERR_ECT | SDHCI_ERR_ECCRC | SDHCI_ERR_ECEB | SDHCI_ERR_ECI)
#define SDHCI_ERR_EDT                 (0x1U << 20)
#define SDHCI_ERR_EDCRC               (0x1U << 21)
#define SDHCI_ERR_EDEB                (0x1U << 22)
#define SDHCI_ERR_DATALINE            (SDHCI_ERR_EDT | SDHCI_ERR_EDCRC | SDHCI_ERR_EDEB)
#define SDHCI_ERR_ECL                 (0x1U << 23)
#define SDHCI_ERR_EAC                 (0x1U << 24)
#define SDHCI_ERR_EADMA               (0x1U << 25)
#define SDHCI_ERR_ERSP                (0x1U << 27)
#define SDHCI_INT_ALL_MASK            (SDHCI_SRS12_INT | SDHCI_SRS12_ERROR)
// Error/Normal Status Enable
#define SDHCI_SRS13                   (0x234U)
// Error/Normal Signal Enable
#define SDHCI_SRS14                   (0x238U)
// Host Controller2/Auto CMD Error Status
#define SDHCI_SRS15                   (0x23CU)
#define SDHCI_SRS15_ACNE              (0x1U)
#define SDHCI_SRS15_ACTE              (0x1U << 1)
#define SDHCI_SRS15_ACCE              (0x1U << 2)
#define SDHCI_SRS15_ACEBE             (0x1U << 3)
#define SDHCI_SRS15_ACIE              (0x1U << 4)
#define SDHCI_SRS15_ACRE              (0x1U << 5)
#define SDHCI_SRS15_ANIACE            (0x1U << 7)
#define SDHCI_SRS15_UMS               (0x7U << 16)
#define SDHCI_UMS_SDR12               (0x0U << 16)
#define SDHCI_UMS_SDR25               (0x1U << 16)
#define SDHCI_UMS_SDR50               (0x2U << 16)
#define SDHCI_UMS_SDR104              (0x3U << 16)
#define SDHCI_UMS_DDR50               (0x4U << 16)
#define SDHCI_SRS15_V18SE             (0x1U << 19)
#define SDHCI_SRS15_DSS               (0x3U << 20)
#define SDHCI_SRS15_EXTNG             (0x1U << 22)
#define SDHCI_SRS15_SCS               (0x1U << 23)
#define SDHCI_SRS15_LVSIEXEC          (0x1U << 24)
#define SDHCI_SRS15_ADMA2LM           (0x1U << 26)
#define SDHCI_SRS15_CMD23E            (0x1U << 27)
#define SDHCI_SRS15_HV4E              (0x1U << 28)
#define SDHCI_SRS15_A64B              (0x1U << 29)
#define SDHCI_SRS15_PVE               (0x1U << 31)
// Capabilities1
#define SDHCI_SRS16                   (0x240)
// Capabilities2
#define SDHCI_SRS17                   (0x244)
// Capabilities3
#define SDHCI_SRS18                   (0x248U)
// Capabilities4
#define SDHCI_SRS19                   (0x24CU)
// Force Event
#define SDHCI_SRS20                   (0x250U)
#define SDHCI_FORCE_ACNE              (0x1U)
#define SDHCI_FORCE_ACTE              (0x1U << 1)
#define SDHCI_FORCE_ACCE              (0x1U << 2)
#define SDHCI_FORCE_ACEBE             (0x1U << 3)
#define SDHCI_FORCE_ACIE              (0x1U << 4)
#define SDHCI_FORCE_CNIACE            (0x1U << 7)
#define SDHCI_FORCE_ECT               (0x1U << 16)
#define SDHCI_FORCE_ECCRC             (0x1U << 17)
#define SDHCI_FORCE_ECEB              (0x1U << 18)
#define SDHCI_FORCE_ECI               (0x1U << 19)
#define SDHCI_FORCE_EDT               (0x1U << 20)
#define SDHCI_FORCE_EDCRC             (0x1U << 21)
#define SDHCI_FORCE_EDEB              (0x1U << 22)
#define SDHCI_FORCE_ECL               (0x1U << 23)
#define SDHCI_FORCE_EAC               (0x1U << 24)
#define SDHCI_FORCE_EADMA             (0x1U << 25)
#define SDHCI_FORCE_ETUNE             (0x1U << 26)
#define SDHCI_FORCE_ERESP             (0x1U << 27)
// ADMA Error Status
#define SDHCI_SRS21                   (0x254U)
// ADMA/SDMA System Address1
#define SDHCI_SRS22                   (0x258U)
// ADMA/SDMA System Address2
#define SDHCI_SRS23                   (0x25CU)
// Preset Value(Default Speed)
#define SDHCI_SRS24                   (0x260U)
// Preset Value(High Speed and SDR12)
#define SDHCI_SRS25                   (0x264U)
// Preset Value(SDR25 and SDR50)
#define SDHCI_SRS26                   (0x268U)
// Preset Value(SDR104 and DDR50)
#define SDHCI_SRS27                   (0x26CU)
// ADMA3 ID Address1
#define SDHCI_SRS30                   (0x278U)
// ADMA3 ID Address2
#define SDHCI_SRS31                   (0x27CU)
// Host Controller Version/Slot Interrupt Status
#define SDHCI_CRS63                   (0x2FCU)

/***** PHY registers *****/
#define SDHCI_PHY_DQ_TIMING           (0x2000U)
#define SDHCI_PHY_DQS_TIMING          (0x2004U)
#define SDHCI_PHY_GATE_LPBK_CTRL      (0x2008U)
#define SDHCI_PHY_DLL_MASTER_CTRL     (0x200CU)
#define SDHCI_PHY_DLL_SLAVE_CTRL      (0x2010U)
#define SDHCI_PHY_WR_DESKEW_PD_CTRL_0 (0x2034U)
#define SDHCI_PHY_CTRL                (0x2080U)

/***** Descriptor table *****/
#define SDHCI_ADMA_ATTR_VAL           (0x1)
#define SDHCI_ADMA_ATTR_END           (0x2)
#define SDHCI_ADMA_ATTR_INT           (0x4)
#define SDHCI_ADMA_ATTR_TRANS         (0x20)
#define SDHCI_ADMA_ATTR_LINK          (0x30)
#define SDHCI_ADMA_MAX_LEN            (65535)

#define DRIVER_TYPEB                  (0)
#define DRIVER_TYPEA                  (1)
#define DRIVER_TYPEC                  (2)
#define DRIVER_TYPED                  (3)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static inline void FSdPsu_Host_WriteBuf (FSdPsu_T *sdPtr, uint32_t data)
{
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS08, data);
}

static inline uint32_t FSdPsu_Host_ReadBuf (FSdPsu_T *sdPtr)
{
    return FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS08);
}

/**********************************Variable
 * Definition**************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* FMSH_SDPSU_HW_H__ */
