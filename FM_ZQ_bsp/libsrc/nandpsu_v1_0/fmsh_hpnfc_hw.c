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
 * @file fmsh_hpnfc_hw.c
 * @addtogroup nandpsu_v1_0
 * @{
 *
 * This header file contains implements the interface functions of the
 * lowlevel operations.
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
#include <math.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_hpnfc.h"
#include "fmsh_hpnfc_flash.h"
#include "fmsh_hpnfc_hw.h"

/*****************************************************************************
 * This function is used to poll reg at addr to check if masked reg value
 * equals to cond.
 *
 * @param
 *       sleep_us: interval of polling
 *       timeout_us
 * @return
 *       FMSH_SUCCESS: success
 *       FMSH_ETIME: timeout
 * @note
 *
 ******************************************************************************/
static int polling (FNandPsu_T *nfcPtr, u32 addr, u32 mask, int cond,
                    int sleep_us, int timeout_us)
{
    volatile u32 value;

    while (1)
    {
        value = FMSH_ReadReg(nfcPtr->config.base, addr);
        if ((value & mask) == cond)
        {
            return FMSH_SUCCESS;
        }

        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }

        delay_us(sleep_us);
        timeout_us -= sleep_us;
    }
}

int FNandPsu_PollInitComp (FNandPsu_T *nfcPtr, int timeout_us)
{
    int ret;

    ret = polling(nfcPtr, NAND_R_CTRL_STATUS, NAND_CTRL_STATUS_INIT_COMP,
                  NAND_CTRL_STATUS_INIT_COMP, 10, timeout_us);

    return ret;
}

int FNandPsu_PollCtrlReady (FNandPsu_T *nfcPtr, int timeout_us)
{
    int ret;

    ret = polling(nfcPtr, NAND_R_CTRL_STATUS, NAND_CTRL_STATUS_CTRL_BUSY, 0, 10,
                  timeout_us);

    return ret;
}

int FNandPsu_PollTrdReady (FNandPsu_T *nfcPtr, int thread, int timeout_us)
{
    int ret;

    ret = polling(nfcPtr, NAND_R_TRD_STATUS, 0x1 << thread, 0, 10, timeout_us);

    return ret;
}

int FNandPsu_PollTrdComp (FNandPsu_T *nfcPtr, int thread, int timeout_us)
{
    u32 ret;

    ret = polling(nfcPtr, NAND_R_TRD_COMP_INTR_STATUS, 0x1 << thread,
                  0x1 << thread, 10, timeout_us);
    if (ret)
    {
        return ret;
    }

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_COMP_INTR_STATUS,
                  0x1 << thread);

    return FMSH_SUCCESS;
}

int FNandPsu_PollCmdComp (FNandPsu_T *nfcPtr, int thread, int timeout_us)
{
    int ret;
    u32 value;

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_STATUS_PTR, thread);

    ret = polling(nfcPtr, NAND_R_CMD_STATUS, NAND_CMD_STATUS_COMP,
                  NAND_CMD_STATUS_COMP, 10, timeout_us);
    if (ret)
    {
        return ret;
    }

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_CMD_STATUS);
    nfcPtr->cmd_status = value;
    if (value & NAND_CMD_STATUS_FAIL)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_PollSdmaTrigg (FNandPsu_T *qspiPtr, int timeout_us)
{
    int ret;

    ret = polling(qspiPtr, NAND_R_INTR_STATUS, NAND_INTR_SDMA_TRIGG,
                  NAND_INTR_SDMA_TRIGG, 10, timeout_us);
    if (ret == 0)
    {
        FMSH_WriteReg(qspiPtr->config.base, NAND_R_INTR_STATUS,
                      NAND_INTR_SDMA_TRIGG);
    }

    return ret;
}

u32 FNandPsu_GetCmdStatus (FNandPsu_T *nfcPtr, int thread)
{
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_STATUS_PTR, thread);
    return FMSH_ReadReg(nfcPtr->config.base, NAND_R_CMD_STATUS);
}

int FNandPsu_PollDevReady (FNandPsu_T *nfcPtr, int cs, int timeout_us)
{
    int ret;

    ret = polling(nfcPtr, NAND_R_RNB_SETTINGS, 0x1 << cs, 0x1 << cs, 10,
                  timeout_us);

    return ret;
}

int FNandPsu_SetPageType (FNandPsu_T *nfcPtr, int type)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;
    struct nand_ecc *ecc;
    u32 offset, sector_cnt, sector_size, last_sector_size;
    u32 reg;

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);
    ecc = &(device->ecc);

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    switch (type)
    {
    case NAND_TT_PAGE_RAW:
        offset = 0;
        sector_cnt = 1;
        sector_size = 0;
        last_sector_size = model->pagesize + model->oobsize;
        break;
    case NAND_TT_MAIN_OOB:
        offset = 0;
        sector_cnt = ecc->steps;
        sector_size = ecc->size;
        last_sector_size = ecc->size + ecc->available_oob_size;
        break;
    case NAND_TT_OOB:
        offset = model->pagesize;
        sector_cnt = 1;
        sector_size = 0;
        last_sector_size = model->oobsize;
        break;
    default:
        break;
    }

    reg = sector_cnt & NAND_TRANSFER_CFG_0_SECTOR_CNT;
    reg |= (offset << NAND_TRANSFER_CFG_0_OFFSET_SHIFT) &
           NAND_TRANSFER_CFG_0_OFFSET;
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRANSFER_CONFIG_0, reg);

    reg = sector_size & NAND_TRANSFER_CFG_1_SECTOR_SIZE;
    reg |= (last_sector_size << NAND_TRANSFER_CFG_1_LAST_SECTOR_SIZE_SHIFT) &
           NAND_TRANSFER_CFG_1_LAST_SECTOR_SIZE;
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRANSFER_CONFIG_1, reg);

    return FMSH_SUCCESS;
}

int FNandPsu_EccConf (FNandPsu_T *nfcPtr, int enable)
{
    int ret;
    struct hpnfc_usercfg *usercfg = nfcPtr->usercfg;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_ecc *ecc = &(device->ecc);
    u32 reg, reg1 = 0;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    reg = (ecc->str_idx << NAND_ECC_CONFIG_0_CORR_STR_SHIFT) &
          NAND_ECC_CONFIG_0_CORR_STR;
    if (enable)
    {
        reg |= NAND_ECC_CONFIG_0_ECC_EN;
        reg1 = ecc->strength;
    }
    if (usercfg->options & NAND_ECC_SCRAMBLER)
    {
        reg |= NAND_ECC_CONFIG_0_SCRAMBLER;
    }
    if (usercfg->options & NAND_ERASED_DET)
    {
        reg |= NAND_ECC_CONFIG_0_ERASE_DET;
    }

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_ECC_CONFIG_0, reg);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_ECC_CONFIG_1, reg1);

    return FMSH_SUCCESS;
}

int FNandPsu_SetEccEnable (FNandPsu_T *nfcPtr, int enable)
{
    int ret;
    u32 reg, reg1;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_ecc *ecc = &(device->ecc);

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_ECC_CONFIG_0);
    if (enable)
    {
        reg |= NAND_ECC_CONFIG_0_ECC_EN;
        reg1 = ecc->strength;
    }
    else
    {
        reg &= ~NAND_ECC_CONFIG_0_ECC_EN;
        reg1 = 0;
    }

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_ECC_CONFIG_0, reg);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_ECC_CONFIG_1, reg1);

    return FMSH_SUCCESS;
}

int FNandPsu_SetSkipByte (FNandPsu_T *nfcPtr, u8 *marker, unsigned int offset,
                          unsigned int len)
{
    int ret;
    u32 reg, tmp = 0;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    reg = len & NAND_SKIP_CONF_SKIP_BYTES;
    if (marker)
    {
        tmp = marker[0] | ((u32)marker[1] << 8);
    }
    reg |= (tmp << NAND_SKIP_CONF_MARKER_SHIFT) & NAND_SKIP_CONF_MARKER;

    if (nfcPtr->usercfg->options & NAND_NO_SKIP_BYTE)
    {
        reg = 0;
        offset = 0;
    }

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_SKIP_CONF, reg);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_SKIP_OFFSET, offset);

    return FMSH_SUCCESS;
}

int FNandPsu_DeviceConf (FNandPsu_T *nfcPtr, int enable)
{
    int ret;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_model *model = &(device->model);
    u32 ppb, blk_addr_idx;
    ;
    u32 reg;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    reg = (model->lun_num << NAND_NF_DEV_LAYOUT_LN_SHIFT) &
          NAND_NF_DEV_LAYOUT_LN;
    if (enable)
    {
        reg |= NAND_NF_DEV_LAYOUT_LUN_EN;
    }
    blk_addr_idx = model->erase_shift - model->page_shift;
    ppb = 0x1 << blk_addr_idx;
    reg |= (blk_addr_idx << 27) & NAND_NF_DEV_LAYOUT_BLK_ADDR_IDX;
    reg |= (ppb & NAND_NF_DEV_LAYOUT_PPB);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_NF_DEV_LAYOUT, reg);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_LUN_ADDR_OFFSET,
                  model->lun_shift);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_LUN_INTERLEAVED_CMD, 0);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_LUN_STATUS_CMD, 0);

    reg = NAND_CTRL_TIMEOUT_EN;
    switch (device->feat.row_cycles)
    {
    case 2:
        reg |= (0x1 << 7);
        break;
    case 3:
        reg |= (0x0 << 7);
        break;
    case 4:
        reg |= (0x2 << 7);
        break;
    default:
        return FMSH_FAILURE;
    }
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_DEVICE_CTRL, reg);

    return FMSH_SUCCESS;
}

int FNandPsu_MultiPlaneConf (FNandPsu_T *nfcPtr, int enable)
{
    int ret;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_feature *feat = &(device->feat);
    int pl_num;
    u32 reg;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    reg = 0;
    if (enable)
    {
        if ((feat->features & NAND_SUPPORT_MPL_WRITE) ||
            (feat->features & NAND_SUPPORT_MPL_READ))
        {
            pl_num = 1;  // 2 plane
        }
        else
        {
            pl_num = 0;  // plane
        }
        reg = (pl_num << NAND_MPL_CONFIG_PL_NUM_SHIFT) & NAND_MPL_CONFIG_PL_NUM;

        if (feat->features & NAND_SUPPORT_MPL_WRITE)
        {
            reg |= NAND_MPL_CONFIG_MPL_WR_EN;
        }
        if (feat->features & NAND_SUPPORT_MPL_READ)
        {
            reg |= NAND_MPL_CONFIG_MPL_RD_EN;
        }
    }
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_MPL_CONFIG, reg);

    return FMSH_SUCCESS;
}

int FNandPsu_CacheConf (FNandPsu_T *nfcPtr, int enable)
{
    int ret;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_feature *feat = &(device->feat);
    u32 reg;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    reg = 0;
    if (enable)
    {
        if (feat->opt_cmds & NAND_SUPPORT_CACHE_WRITE)
        {
            reg |= NAND_CACHE_CONFIG_WR_EN;
        }
        if (feat->opt_cmds & NAND_SUPPORT_CACHE_READ)
        {
            reg |= NAND_CACHE_CONFIG_RD_EN;
        }
    }
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CACHE_CONFIG, reg);

    return FMSH_SUCCESS;
}

int FNandPsu_SetPolling (FNandPsu_T *nfcPtr, u32 long_poll, u32 short_poll,
                         u8 rbn)
{
    int ret;
    u32 reg = 0;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    if (rbn)
    {
        reg = NAND_RDST_CTRL_0_RB_EN;
    }

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_RDST_CTRL_0, 0x40400000 | reg);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_RDST_CTRL_1, 0x41410000);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_LONG_POLLING, long_poll);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_SHORT_POLLING, short_poll);

    return FMSH_SUCCESS;
}

int FNandPsu_SetTimings (FNandPsu_T *nfcPtr, struct hpnfc_timings *timings)
{
    int ret;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    /* common settings */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_COMMON_SETTINGS,
                  timings->common_settings);

    /* controller timing registers */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TOGGLE_TIMINGS_0,
                  timings->toggle_timings0);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TOGGLE_TIMINGS_1,
                  timings->toggle_timings1);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_ASYNC_TOGGLE_TIMINGS,
                  timings->async_toggle_timings);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_SYNC_TIMINGS,
                  timings->sync_timings);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TIMINGS0, timings->timings0);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TIMINGS1, timings->timings1);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TIMINGS2, timings->timings2);

    /* Assert PHY rstn */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_DLL_PHY_CTRL, 0);

    /* nand clk can be changed here(not implemented) */

    /* phy registers */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_PHY_DQ_TIMING_REG,
                  timings->dll_phy_dq_timings);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_PHY_DQS_TIMING_REG,
                  timings->dll_phy_dqs_timings);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_PHY_GATE_LPBK_CTRL_REG,
                  timings->dll_phy_gate_lpbk_ctrl);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_PHY_DLL_MASTER_CTRL_REG,
                  timings->dll_phy_dll_master_ctrl);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_PHY_DLL_SLAVE_CTRL_REG,
                  timings->dll_phy_dll_slave_ctrl);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_PHY_CTRL, timings->dll_phy_ctrl);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_PHY_TSEL, timings->dll_phy_tsel);

    /* De-assert PHY rstn */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_DLL_PHY_CTRL,
                  timings->mini_dll_phy_ctrl | NAND_DLL_PHY_CTRL_DLL_RST_N);
    ret = polling(nfcPtr, NAND_R_DLL_PHY_CTRL, 0x1 << 26, 0x1 << 26, 10, 1000);

    return ret;
}

int FNandPsu_SetIOWidth16 (FNandPsu_T *nfcPtr, int width16)
{
    int ret;
    u32 reg;

    /* Wait for ctrl ready */
    ret = FNandPsu_PollCtrlReady(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_COMMON_SETTINGS);
    if (width16)
    {
        reg |= NAND_COMMON_SETTINGS_DEV_16BITS;
    }
    else
    {
        reg &= ~NAND_COMMON_SETTINGS_DEV_16BITS;
    }
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_COMMON_SETTINGS, reg);

    return 0;
}

int FNandPsu_EnableAllTrdIntr (FNandPsu_T *nfcPtr)
{
    /* enable and clear trd_error_status */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_STATUS,
                  0xffffffff);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_EN, 0xffffffff);

    /* enable and clear trd_timeout_status */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_STATUS,
                  0xffffffff);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_EN, 0xffffffff);

    return FMSH_SUCCESS;
}

int FNandPsu_SetIntrStatusEnable (FNandPsu_T *nfcPtr, u32 mask, u8 enable)
{
    /* enable intr_status */
    if (enable)
    {
        mask |= NAND_INTR_ENABLE_INTR_EN;
    }
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_INTR_ENABLE, mask);

    return FMSH_SUCCESS;
}

int FNandPsu_ClearAllIntr (FNandPsu_T *nfcPtr)
{
    /* clear intr_status */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_INTR_STATUS, 0xffffffff);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_COMP_INTR_STATUS, 0xffffffff);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_STATUS,
                  0xffffffff);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_STATUS,
                  0xffffffff);

    return FMSH_SUCCESS;
}

int FNandPsu_InitIntrStatus (FNandPsu_T *nfcPtr)
{
    (void)FNandPsu_ClearAllIntr(nfcPtr);
    (void)memset(&(nfcPtr->status), 0, sizeof(nfcPtr->status));

    return FMSH_SUCCESS;
}

int FNandPsu_GetHwCaps (FNandPsu_T *nfcPtr)
{
    u32 reg, tmp;

    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_CTRL_FEATURES_REG);

    tmp = (reg & NAND_CTRL_FEATURES_NBANKS) >> NAND_CTRL_FEATURES_NBANKS_SHIFT;
    nfcPtr->hwcaps.max_banks = 0x1 << tmp;

    if (reg & NAND_CTRL_FEATURES_DMA_DATA_WIDTH64)
    {
        nfcPtr->hwcaps.data_dma_width = 8;
    }
    else
    {
        nfcPtr->hwcaps.data_dma_width = 4;
    }

    if (reg & (NAND_CTRL_FEATURES_NVDDR_SUPP | NAND_CTRL_FEATURES_NVDDR23_SUPP))
    {
        nfcPtr->hwcaps.is_phy_type_dll = 1;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_GetBchCaps (FNandPsu_T *nfcPtr, struct hpnfc_bchcaps *bch)
{
    u32 reg;

    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_BCH_CFG_0);
    bch->corr_str[0] = (u8)reg;
    bch->corr_str[1] = (u8)(reg >> 8);
    bch->corr_str[2] = (u8)(reg >> 16);
    bch->corr_str[3] = (u8)(reg >> 24);
    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_BCH_CFG_1);
    bch->corr_str[4] = (u8)reg;
    bch->corr_str[5] = (u8)(reg >> 8);
    bch->corr_str[6] = (u8)(reg >> 16);
    bch->corr_str[7] = (u8)(reg >> 24);
    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_BCH_CFG_2);
    bch->sector_size[0] = (u16)reg;
    bch->sector_size[1] = (u16)(reg >> 16);
    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_BCH_CFG_3);
    bch->meta_size = (u8)(reg >> 16);

    return FMSH_SUCCESS;
}

int FNandPsu_GetRecord (FNandPsu_T *nfcPtr, int cs, int idx, u32 *logic,
                        u32 *phy)
{
    int ret;
    u32 value;

    value = 0x2;  // read single record
    value |= (cs & 0xf) << 8;
    value |= (idx & 0x3ff) << 16;

    // trig access
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS, value | 0x1);

    // wait for clear access bit
    ret = polling(nfcPtr, NAND_R_REMAP_ACCESS, 0x1, 0x0, 10, 100000);
    if (ret)
    {
        return ret;
    }

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS);
    if (value & 0x80)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS, 0x80);
        return FMSH_EIO;
    }

    if (logic)
    {
        *logic = FMSH_ReadReg(nfcPtr->config.base, NAND_R_REMAP_LOG_ADDR);
    }
    if (phy)
    {
        *phy = FMSH_ReadReg(nfcPtr->config.base, NAND_R_REMAP_PHYS_ADDR);
    }

    return FMSH_SUCCESS;
}

int FNandPsu_AddRecord (FNandPsu_T *nfcPtr, int cs, u32 phy, u32 logic,
                        u32 *idx)
{
    int ret;
    u32 value;

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_LOG_ADDR, logic);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_PHYS_ADDR, phy);

    value = 0x0;  // write single record
    value |= (cs & 0xf) << 8;

    // trig access
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS, value | 0x1);

    // wait for clear access bit
    ret = polling(nfcPtr, NAND_R_REMAP_ACCESS, 0x1, 0x0, 10, 100000);
    if (ret)
    {
        return ret;
    }

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS);
    if (value & 0x80)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS, 0x80);
        return FMSH_EIO;
    }

    if (idx)
    {
        value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_REMAP_CTRL);
        *idx = (value >> 16) & 0x7f;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_ClearRecord (FNandPsu_T *nfcPtr)
{
    int ret;
    u32 value;

    // trig access
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS, 0x5);

    // wait for clear access bit
    ret = polling(nfcPtr, NAND_R_REMAP_ACCESS, 0x1, 0x0, 10, 100000);
    if (ret)
    {
        return ret;
    }

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS);
    if (value & 0x80)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_ACCESS, 0x80);
        return FMSH_EIO;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_SetRmpMask (FNandPsu_T *nfcPtr, u32 mask)
{
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_MASK, mask);

    return FMSH_SUCCESS;
}

int FNandPsu_SetRmpEnable (FNandPsu_T *nfcPtr, int enable)
{
    if (enable)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_CTRL, 0x1);
    }
    else
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_REMAP_CTRL, 0x0);
    }

    return FMSH_SUCCESS;
}

int FNandPsu_SetProtEnable (FNandPsu_T *nfcPtr, int cs, int num, u32 range_low,
                            u32 range_high, int enable)
{
    u32 ctrl = 0;

    if (enable)
    {
        ctrl = 0x1 << cs;
    }

    if (num == 0)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_PROT_CTRL_0, ctrl);
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_PROT_DOWN_0, range_low);
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_PROT_UP_0, range_high + 1);
    }
    else if (num == 1)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_PROT_CTRL_1, ctrl);
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_PROT_DOWN_1, range_low);
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_PROT_UP_1, range_high + 1);
    }
    else{
        ;/* no deal with */
    }
    
    return FMSH_SUCCESS;
}
