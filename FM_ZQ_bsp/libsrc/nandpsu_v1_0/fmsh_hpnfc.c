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
 * @file fmsh_hpnfc.c
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This source file contains functions that are used to operate hpnfc.
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
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_hpnfc.h"
#include "fmsh_hpnfc_flash.h"
#include "fmsh_hpnfc_hw.h"

static void stub_status_handler(void *callback_ref, u32 status,
                                unsigned int len);
static int nand_select_target(FNandPsu_T *nfcPtr, int cs);
static int nand_deselect_target(FNandPsu_T *nfcPtr);
static int nand_setup_iface(FNandPsu_T *nfcPtr,
                            struct nand_interface_config *iface);

static int nand_exec_op(FNandPsu_T *nfcPtr, struct nand_operation *ops);
static int nand_device_reset(FNandPsu_T *nfcPtr, int cs);
static int nand_set_feature(FNandPsu_T *nfcPtr, int cs, u8 feature, void *data);
static int nand_erase_block(FNandPsu_T *nfcPtr, int cs, u32 page, u32 len);
static int nand_write_page(FNandPsu_T *nfcPtr, int cs, unsigned int page,
                           unsigned int offset, void *buf, unsigned int len,
                           int raw);
static int nand_read_page(FNandPsu_T *nfcPtr, int cs, unsigned int page,
                          unsigned int offset, void *buf, unsigned int len,
                          int raw);
static int nand_copyback(FNandPsu_T *nfcPtr, int cs, u32 src_page, u32 dst_page,
                         u8 npages);

static struct nand_device device;

static struct hpnfc_usercfg usercfg_default = {
    .options = NAND_USE_RNB_LINE | NAND_ERASED_DET,
    .dma_type = NAND_MDMA,
    .dev_bbt_options = NAND_BBT_PERCHIP,
};

static struct nand_ctrl_ops nand_ops = {
    .setup_interface = nand_setup_iface,
    .exec_op = nand_exec_op,
    .reset = nand_device_reset,
    .set_feature = nand_set_feature,
    .erase = nand_erase_block,
    .read_page = nand_read_page,
    .write_page = nand_write_page,
    .copyback = nand_copyback,
};

static void stub_status_handler (void *callback_ref, u32 status,
                                 unsigned int len)
{
    (void)callback_ref;
    (void)status;
    (void)len;
}

static int nand_select_target (FNandPsu_T *nfcPtr, int cs)
{
    nfcPtr->cur_cs = cs;

    return 0;
}

static int nand_deselect_target (FNandPsu_T *nfcPtr)
{
    nfcPtr->cur_cs = -1;

    return 0;
}

int FNandPsu_CfgInitialize (FNandPsu_T *nfcPtr, FNandPsu_Config_T *cfgPtr)
{
    FMSH_ASSERT(nfcPtr != NULL);
    FMSH_ASSERT(cfgPtr != NULL);

    nfcPtr->config.device_id = cfgPtr->device_id;
    nfcPtr->config.base = cfgPtr->base;
    nfcPtr->config.data_base = cfgPtr->data_base;
    nfcPtr->config.io_width = cfgPtr->io_width;
    nfcPtr->config.clock_hz = cfgPtr->clock_hz;
    nfcPtr->config.board_delay = cfgPtr->board_delay;

    nfcPtr->flags = 0;
    nfcPtr->ctrl_rev = 0;
    nfcPtr->conf_done = 0;
    nfcPtr->skip_block_base = 0;

    nfcPtr->usercfg = &usercfg_default;
    nfcPtr->ctrl_ops = &nand_ops;
    nfcPtr->ecc_mode = ECC_NONE;

    nfcPtr->status_handler = stub_status_handler;
    nfcPtr->status_ref = 0;

    nfcPtr->cur_cs = -1;
    nfcPtr->cur_vol = 0;
    nfcPtr->device = &device;

    return FMSH_SUCCESS;
}

int FNandPsu_Reset (void)
{
    u32 value;
    int timeout_us;

    value = FMSH_ReadReg(0xff5e0000, 0x0238);
    FMSH_WriteReg(0xff5e0000, 0x0238, value | 0x2000000);
    FMSH_WriteReg(0xff5e0000, 0x0238, value | 0x4000000);
    delay_us(5);
    FMSH_WriteReg(0xff5e0000, 0x0238, value & ~0x4000000);
    FMSH_WriteReg(0xff5e0000, 0x0238, value & ~0x2000000);
    delay_us(5);

    timeout_us = 1000;
    while (1)
    {
        value = FMSH_ReadReg(0xff100000, NAND_R_CTRL_STATUS);
        if ((value & NAND_CTRL_STATUS_INIT_COMP) == NAND_CTRL_STATUS_INIT_COMP)
        {
            break;
        }

        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }

        delay_us(1);
        timeout_us -= 1;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_SelfTest (FNandPsu_T *nfcPtr)
{
    u32 reg = 0;

    FMSH_ASSERT(nfcPtr != NULL);

    /* Check Register */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG3, 0xaa559966);
    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_CMD_REG3);
    if (reg != 0xaa559966)
    {
        return FMSH_EIO;
    }

    return FMSH_SUCCESS;
}

void FNandPsu_SetStatusHandler (FNandPsu_T *nfcPtr, void *callBackRef,
                                FNandPsu_StatusHandler funcPtr)
{
    FMSH_ASSERT(nfcPtr != NULL);
    FMSH_ASSERT(funcPtr != NULL);

    nfcPtr->status_handler = funcPtr;
    nfcPtr->status_ref = callBackRef;
}

void FNandPsu_InterruptHandler (void *instancePtr)
{
    FNandPsu_T *nfcPtr = (FNandPsu_T *)instancePtr;
    u32 value;

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_INTR_STATUS);
    if (value)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_INTR_STATUS, value);
        nfcPtr->status.intr_status |= value;
        nfcPtr->status.irq_sync++;
    }

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_TRD_COMP_INTR_STATUS);
    if (value)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_COMP_INTR_STATUS, value);
        nfcPtr->status.trd_comp |= value;
        nfcPtr->status.trd_sync++;
    }

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_STATUS);
    if (value)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_STATUS, value);
        nfcPtr->status.trd_error |= value;
    }

    value = FMSH_ReadReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_STATUS);
    if (value)
    {
        FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_STATUS,
                      value);
        nfcPtr->status.trd_timeout |= value;
    }

    /* User Handler */
    nfcPtr->status_handler(nfcPtr, 0, 0);
}

int FNandPsu_WaitIrqComp (FNandPsu_T *nfcPtr, int sync, int timeout_us)
{
    while (1)
    {
        if (nfcPtr->status.irq_sync >= sync)
        {
            break;
        }

        delay_us(1);
        timeout_us--;
        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }
    }

    return FMSH_SUCCESS;
}

/**************************** Initialize *******************************/
int FNandPsu_Initialize (FNandPsu_T *nfcPtr)
{
    FNandPsu_Config_T *cfgPtr;

    FMSH_ASSERT(nfcPtr != NULL);

    cfgPtr = FNandPsu_LookupConfig(0);
    if (cfgPtr == NULL)
    {
        return FMSH_FAILURE;
    }

    return FNandPsu_CfgInitialize(nfcPtr, cfgPtr);
}

int FNandPsu_HwInit (FNandPsu_T *nfcPtr, FNandPsu_UserCfg_T *usercfg)
{
    int ret;
    u32 reg;
    int rbn;

    /* Set User Capability */
    if (usercfg)
    {
        nfcPtr->usercfg = usercfg;
    }
    else
    {
        nfcPtr->usercfg = &usercfg_default;
    }

    /* Wait for controller init complete */
    ret = FNandPsu_PollInitComp(nfcPtr, 1000);
    if (ret)
    {
        return ret;
    }

    /* get controller revision */
    reg = FMSH_ReadReg(nfcPtr->config.base, NAND_R_CTRL_VERSION);
    nfcPtr->ctrl_rev = reg & NAND_CTRL_VERSION_REV;

    /* Disable cache and multiplane. */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_MPL_CONFIG, 0x0);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CACHE_CONFIG, 0x0);

    /* set interrupts. */
    FNandPsu_EnableAllTrdIntr(nfcPtr);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_INTR_STATUS, 0xffffffff);
    FNandPsu_SetIntrStatusEnable(nfcPtr, 0, 0);

    /* Get HPNFC Capability */
    FNandPsu_GetHwCaps(nfcPtr);

    /* Set IO to 8bit during nand_detect */
    (void)FNandPsu_SetIOWidth16(nfcPtr, 0);

    /* Config controller before scan device*/
    if (nfcPtr->usercfg->options & NAND_USE_RNB_LINE)
    {
        rbn = 1;
    }
    else
    {
        rbn = 0;
    }
    FNandPsu_SetPolling(nfcPtr, 0x3e8, 0x1f4, rbn);

    return FMSH_SUCCESS;
}

int FNandPsu_HwInitr (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_model *model = &(device->model);
    struct nand_ecc *ecc = &(device->ecc);
    struct hpnfc_bchcaps bch;
    int i, strength, bytes, total;
    u8 enable;

    /* Config ecc layout */
    if (nfcPtr->usercfg->options & NAND_ECC_ONDIE)
    {
        nfcPtr->ecc_mode = ECC_ONDIE;
        // enable ondie ecc
    }
    else
    {
        nfcPtr->ecc_mode = ECC_HW;
        FNandPsu_GetBchCaps(nfcPtr, &bch);
        ecc->size = bch.sector_size[1];
        ecc->steps = model->pagesize / ecc->size;
        /* Select ECC strength as stronger as possible */
        ecc->str_idx = -1;
        for (i = 0; i < 7; i++)
        {
            switch (i)
            {
            case 0:
                strength = 8;
                bytes = 16;
                break;
            case 1:
                strength = 16;
                bytes = 30;
                break;
            case 2:
                strength = 32;
                bytes = 60;
                break;
            case 3:
                strength = 64;
                bytes = 120;
                break;
            case 4:
                strength = 72;
                bytes = 136;
                break;
            case 5:
                strength = 96;
                bytes = 180;
                break;
            case 6:
                strength = 130;
                bytes = 242;
                break;
            default:
                return FMSH_FAILURE;
            }
            total = bytes * ecc->steps;
            /* reserve at least  16B in oob for user data and bbt*/
            if (total > (model->oobsize - 16))
            {
                break;
            }

            ecc->str_idx = i;
            ecc->strength = strength;
            ecc->bytes = bytes;
        }
        if (ecc->str_idx == -1)
        {
            return FMSH_FAILURE;
        }
        ecc->total = ecc->bytes * ecc->steps;
        /* ECC available oob is protected by ecc (exclude ecc data and bbm)*/
        ecc->available_oob_size = model->oobsize - ecc->total;
        if (ecc->available_oob_size > bch.meta_size)
        {
            ecc->available_oob_size = bch.meta_size;
        }
        if ((ecc->available_oob_size + 2 + ecc->total) > model->oobsize)
        {
            ecc->available_oob_size -= 4;  // reserved for bb pattern
        }

        ret = FNandPsu_EccConf(nfcPtr, 1);
        if (ret)
        {
            return ret;
        }
    }

    /* Config multi-LUN */
    if (device->feat.features & NAND_SUPPORT_MULTILUN)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }
    ret = FNandPsu_DeviceConf(nfcPtr, enable);
    if (ret)
    {
        return ret;
    }

    /* After set conf_done mark, only limited config
     * can be done.
     */
    nfcPtr->conf_done = 1;

    return 0;
}

/**************************** Timing *****************************************/
static int nand_setup_sdr_iface (FNandPsu_T *nfcPtr,
                                 struct nand_interface_config *iface,
                                 struct hpnfc_timings *regs)
{
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_timings *timings = &(iface->timings);
    float sdr_clk_period, board_delay;
    int cycle;  // nand2_delay;
    int extended_wr_mode, extended_rd_mode;
    int rd_del_sel, phony_dqs_timing;
    int trp_cnt = 0, treh_cnt = 0, twp_cnt = 0, twh_cnt = 0;

    fmsh_print_dbg("SDR Timming mode\t %d\r\n", timings->mode);

    (void)memset(regs, 0, sizeof(struct hpnfc_timings));

    board_delay = nfcPtr->config.board_delay;
    // nand2_delay = NAND_PHY_NAND2DELAY;
    sdr_clk_period = (float)1000000000 / nfcPtr->config.clock_hz;

    /* common settings */
    regs->common_settings = NAND_MODE_SDR;
    if (device->model.io_width == 16)
    {
        regs->common_settings |= NAND_COMMON_SETTINGS_DEV_16BITS;
    }

    /* toggle timing0 */
    regs->toggle_timings0 = 0;

    /* toggle timing1 */
    regs->toggle_timings1 = 0;

    /* read timing */
    if (timings->sdr.tRP_min <= (timings->sdr.tREA_max + board_delay))
    {
        if ((sdr_clk_period / 2) > (timings->sdr.tREA_max + board_delay))
        {
            extended_rd_mode = 0;
            rd_del_sel = 3;
            phony_dqs_timing = 0;
            regs->dll_phy_dqs_timings = 0x00110004;
        }
        else
        {
            extended_rd_mode = 1;
            trp_cnt = (int)ceil((timings->sdr.tREA_max + board_delay) /
                                sdr_clk_period);
            if (trp_cnt <= 28)
            {
                rd_del_sel = trp_cnt + 3;
                phony_dqs_timing = trp_cnt - 1;
            }
            else
            {
                fmsh_print_err("ERROR: rd_del_sel overflow\r\n");
            }
            regs->dll_phy_dqs_timings = 0x00100004;
        }
    }
    else
    {
        if ((sdr_clk_period / 2) > timings->sdr.tRP_min)
        {
            extended_rd_mode = 0;
            rd_del_sel = 3;
            phony_dqs_timing = 0;
            regs->dll_phy_dqs_timings = 0x00110004;
        }
        else
        {
            extended_rd_mode = 1;
            /* ? 
            float re_tmp;
            re_tmp = (timings->sdr.tRP_min > 25) ? 25 : timings->sdr.tRP_min;
            trp_cnt = (int)ceil(re_tmp / sdr_clk_period);*/
            trp_cnt = (int)ceil(timings->sdr.tRP_min / sdr_clk_period);
            rd_del_sel = trp_cnt + 3;
            phony_dqs_timing = trp_cnt - 1;
            regs->dll_phy_dqs_timings = 0x00100004;
        }
    }

    /* phy ctrl */
    regs->dll_phy_ctrl = NAND_PHY_CTRL_SDR_DQS_VALUE;
    regs->dll_phy_ctrl |= (phony_dqs_timing & 0x3f) << 4;

    regs->dll_phy_tsel = 0x00000000;

    regs->dll_phy_dq_timings = 0x2;

    regs->dll_phy_gate_lpbk_ctrl = (rd_del_sel & 0xf) << 19;

    regs->dll_phy_dll_master_ctrl = 0x00800000;

    regs->dll_phy_dll_slave_ctrl = 0;

    /***************************************************************/
    // async toggle timings
    u32 treh, twh;

    twp_cnt = (int)ceil(timings->sdr.tWP_min / sdr_clk_period);
    if (twp_cnt > 0xff)
    {
        twp_cnt = 0xff;
    }
    regs->async_toggle_timings |= twp_cnt;

    twh = (u32)(timings->sdr.tWC_min - timings->sdr.tWP_min);
    if (timings->sdr.tWH_min > twh)
    {
        twh = (u32)timings->sdr.tWH_min;
    }
    twh_cnt = (int)ceil(twh / sdr_clk_period);
    if (twh_cnt > 0xff)
    {
        twh_cnt = 0xff;
    }
    regs->async_toggle_timings |= twh_cnt << 8;

    trp_cnt = (int)ceil(timings->sdr.tRP_min / sdr_clk_period);
    if (trp_cnt > 0xff)
    {
        trp_cnt = 0xff;
    }
    regs->async_toggle_timings |= trp_cnt << 16;

    treh = (u32)(timings->sdr.tRC_min - timings->sdr.tRP_min);
    if (timings->sdr.tREH_min > treh)
    {
        treh = (u32)timings->sdr.tREH_min;
    }
    treh_cnt = (int)ceil(treh / sdr_clk_period);
    if (treh_cnt > 0xff)
    {
        treh_cnt = 0xff;
    }
    regs->async_toggle_timings |= treh_cnt << 24;

    /* sync timings */
    regs->sync_timings = 0;

    /* timings0 */
    cycle = (int)ceil(timings->sdr.tADL_min / sdr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 24;

    cycle = (int)ceil(device->feat.tCCS_min / sdr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 16;

    cycle = (int)ceil(timings->sdr.tWHR_min / sdr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 8;

    cycle = (int)ceil(timings->sdr.tRHW_min / sdr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle;

    /* timings1 */
    cycle = (int)ceil(timings->sdr.tRHZ_max / sdr_clk_period) + 1;
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings1 |= (u32)cycle << 24;

    cycle = (int)ceil((timings->sdr.tWB_max + board_delay) / sdr_clk_period) +
            2;
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings1 |= (u32)cycle << 16;

    cycle = (int)ceil(50 / sdr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings1 |= (u32)cycle;  // tVDLY

    /* timings2 */
    cycle = (int)ceil(timings->sdr.tFEAT_max / sdr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle << 16;

    cycle = (int)ceil(timings->sdr.tCEH_min / sdr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle << 8;

    cycle = (int)ceil(timings->sdr.tCS_min / sdr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle;

    /* mini dll phy ctrl */
    if (ceil(sdr_clk_period / 2) >= ceil(timings->sdr.tWP_min))
    {
        extended_wr_mode = 0;
    }
    else
    {
        extended_wr_mode = 1;
    }

    regs->mini_dll_phy_ctrl = 0x0707;
    if (extended_wr_mode)
    {
        regs->mini_dll_phy_ctrl |= NAND_DLL_PHY_CTRL_EXTENDED_WR_MODE;
    }
    if (extended_rd_mode)
    {
        regs->mini_dll_phy_ctrl |= NAND_DLL_PHY_CTRL_EXTENDED_RD_MODE;
    }

    /***************************************************************/
    fmsh_print_dbg("common_settings\t 0x%x\r\n", regs->common_settings);
    fmsh_print_dbg("timings0\t 0x%x\r\n", regs->timings0);
    fmsh_print_dbg("timings1\t 0x%x\r\n", regs->timings1);
    fmsh_print_dbg("timings2\t 0x%x\r\n", regs->timings2);
    fmsh_print_dbg("async_toggle_timings\t 0x%x\r\n",
                   regs->async_toggle_timings);
    fmsh_print_dbg("sync_timings\t 0x%x\r\n", regs->sync_timings);
    fmsh_print_dbg("toggle_timings0\t 0x%x\r\n", regs->toggle_timings0);
    fmsh_print_dbg("toggle_timings1\t 0x%x\r\n", regs->toggle_timings1);
    fmsh_print_dbg("mini_dll_phy_ctrl\t 0x%x\r\n", regs->mini_dll_phy_ctrl);
    fmsh_print_dbg("dll_phy_dq_timings\t 0x%x\r\n", regs->dll_phy_dq_timings);
    fmsh_print_dbg("dll_phy_dqs_timings\t 0x%x\r\n", regs->dll_phy_dqs_timings);
    fmsh_print_dbg("dll_phy_gate_lpbk_ctrl\t 0x%x\r\n",
                   regs->dll_phy_gate_lpbk_ctrl);
    fmsh_print_dbg("dll_phy_dll_master_ctrl\t 0x%x\r\n",
                   regs->dll_phy_dll_master_ctrl);
    fmsh_print_dbg("dll_phy_dll_slave_ctrl\t 0x%x\r\n",
                   regs->dll_phy_dll_slave_ctrl);
    fmsh_print_dbg("dll_phy_tsel\t 0x%x\r\n", regs->dll_phy_tsel);
    fmsh_print_dbg("dll_phy_ctrl\t 0x%x\r\n", regs->dll_phy_ctrl);

    return 0;
}

static int nand_setup_nvddr_iface (FNandPsu_T *nfcPtr,
                                   struct nand_interface_config *iface,
                                   struct hpnfc_timings *regs)
{
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_timings *timings = &(iface->timings);
    float ddr_clk_period, board_delay;
    int cycle, nand2_delay;
    float dqs_delay;
    int gate_open_delay, rd_del_sel, gate_close_delay, half_cycle_mode;
    int use_dll_action, number_of_element;
    int oe_start, oe_end, oe_end_dqsd;

    fmsh_print_dbg("NV-DDR Timming mode\t %d\r\n", timings->mode);

    (void)memset(regs, 0, sizeof(struct hpnfc_timings));

    board_delay = nfcPtr->config.board_delay;
    nand2_delay = NAND_PHY_NAND2DELAY;
    ddr_clk_period = (float)1000000000 / nfcPtr->config.clock_hz;

    /* common settings */
    regs->common_settings = NAND_MODE_NVDDR;

    /* toggle timing0 */
    regs->toggle_timings0 = 0x00000000;

    /* toggle timing1 */
    regs->toggle_timings1 = 0x00000002;  // tWPST=1.5tCK required

    /* async toggle timings */
    regs->async_toggle_timings = 0;

    /* sync timings */
    // tCKWR = Roundup([tDQSCK_max + tCK] / tCK)
    cycle = (int)ceil((timings->nvddr.tDQSCK_max + ddr_clk_period) /
                      ddr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->sync_timings |= (u32)cycle << 16;

    cycle = (int)ceil(timings->nvddr.tWRCK_min / ddr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->sync_timings |= (u32)cycle << 8;

    cycle = (int)ceil(timings->nvddr.tCAD_min / ddr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->sync_timings |= (u32)cycle;

    // timings0
    cycle = (int)ceil(timings->nvddr.tADL_min / ddr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 24;

    cycle = (int)ceil(device->feat.tCCS_min / ddr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 16;

    cycle = (int)ceil(timings->nvddr.tWHR_min / ddr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 8;

    cycle = (int)ceil(timings->nvddr.tRHW_min / ddr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle;

    // timings1
    cycle = (int)ceil(timings->nvddr.tWB_max / ddr_clk_period) + 2;
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings1 |= (u32)cycle << 16;

    cycle = (int)ceil(50 / ddr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings1 |= (u32)cycle;

    // timings2
    cycle = (int)ceil(timings->nvddr.tFEAT_max / ddr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle << 16;

    cycle = (int)ceil(timings->nvddr.tCEH_min / ddr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle << 8;

    cycle = (int)ceil(timings->nvddr.tCS_min / ddr_clk_period);
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle;

    /* twp and extended write mode */
    regs->mini_dll_phy_ctrl = 0x00000707;

    /***************************************************************/
    /* flash device captures RE# at poseedge of clk */
    dqs_delay = board_delay + timings->nvddr.tDQSD_max + (ddr_clk_period / 2);
    gate_open_delay = (int)(dqs_delay / ddr_clk_period);
    if (gate_open_delay < 1)
    {
        gate_open_delay = 1;
    }
    else if (gate_open_delay > 15)
    {
        gate_open_delay = 15;
        fmsh_print_err("ERROR: gate_open_delay overflow in ddr mode\r\n");
    }
    else
    {
        ; /* no deal with */
    }

    rd_del_sel = (int)ceil((board_delay + timings->nvddr.tDQSCK_max) /
                           ddr_clk_period);
    if (rd_del_sel <= 29)
    {
        rd_del_sel = rd_del_sel + 2;
    }
    else
    {
        fmsh_print_err("ERROR: rd_del_sel overflow\r\n");
    }

    gate_close_delay = (int)ceil(timings->nvddr.tDQSCK_max / ddr_clk_period);

    regs->dll_phy_gate_lpbk_ctrl = (rd_del_sel << 19) |
                                   (gate_close_delay << 4) | gate_open_delay;

    /***************************************************************/
    if ((ddr_clk_period * 1000) > (int)(115 * 2 * nand2_delay))
    {
        half_cycle_mode = 1;
    }
    else
    {
        half_cycle_mode = 0;
    }

    if ((ddr_clk_period * 1000 / 2) < (int)(115 * 2 * nand2_delay))
    {
        use_dll_action = 1;
    }
    else
    {
        use_dll_action = 0;
    }

    number_of_element = (int)ceil((ddr_clk_period * 1000 / 4) /
                                  (2 * nand2_delay));
    if (number_of_element > 255)
    {
        number_of_element = 255;  // 63, modeified for 10M
    }

    if (use_dll_action)
    {
        regs->dll_phy_dll_master_ctrl = (half_cycle_mode << 24) | 0x140004;
        regs->dll_phy_dll_slave_ctrl = 0x2424;
    }
    else
    {
        regs->dll_phy_dll_master_ctrl = 0x00800000;
        regs->dll_phy_dll_slave_ctrl = (number_of_element << 8) |
                                       number_of_element;
    }

    /***************************************************************/
    oe_start = 0;
    oe_end = 4;
    oe_end_dqsd = (int)(timings->nvddr.tDQSD_max / (ddr_clk_period / 2)) + 4;
    oe_end = (int)((oe_end + oe_end_dqsd) / 2);
    if (oe_end > 7)
    {
        oe_end = 7;
    }
    /*
    if((int)(timings->nvddr.tDQSHZ_max/(ddr_clk_period/2)) ==
       (int)ceil(timings->nvddr.tDQSHZ_max/(ddr_clk_period/2)))
        oe_start = (int)ceil(timings->nvddr.tDQSHZ_max/(ddr_clk_period/2)) + 1;
    else {
        oe_start = (int)ceil(timings->nvddr.tDQSHZ_max/(ddr_clk_period/2));
    }
    if(oe_start > 7) {
        oe_start = 7;
    }*/

    regs->dll_phy_dq_timings = (oe_start << 12) | (oe_end << 8) |
                               (oe_start << 4) | oe_end;
    regs->dll_phy_dqs_timings = (oe_start << 12) | (oe_end << 8) |
                                (oe_start << 4) | oe_end;

    /***************************************************************/
    regs->dll_phy_ctrl = 0x00004000;
    regs->dll_phy_tsel = 0x00000000;

    /***************************************************************/
    fmsh_print_dbg("common_settings\t 0x%x\r\n", regs->common_settings);
    fmsh_print_dbg("timings0\t 0x%x\r\n", regs->timings0);
    fmsh_print_dbg("timings1\t 0x%x\r\n", regs->timings1);
    fmsh_print_dbg("timings2\t 0x%x\r\n", regs->timings2);
    fmsh_print_dbg("async_toggle_timings\t 0x%x\r\n",
                   regs->async_toggle_timings);
    fmsh_print_dbg("sync_timings\t 0x%x\r\n", regs->sync_timings);
    fmsh_print_dbg("toggle_timings0\t 0x%x\r\n", regs->toggle_timings0);
    fmsh_print_dbg("toggle_timings1\t 0x%x\r\n", regs->toggle_timings1);
    fmsh_print_dbg("mini_dll_phy_ctrl\t 0x%x\r\n", regs->mini_dll_phy_ctrl);
    fmsh_print_dbg("dll_phy_dq_timings\t 0x%x\r\n", regs->dll_phy_dq_timings);
    fmsh_print_dbg("dll_phy_dqs_timings\t 0x%x\r\n", regs->dll_phy_dqs_timings);
    fmsh_print_dbg("dll_phy_gate_lpbk_ctrl\t 0x%x\r\n",
                   regs->dll_phy_gate_lpbk_ctrl);
    fmsh_print_dbg("dll_phy_dll_master_ctrl\t 0x%x\r\n",
                   regs->dll_phy_dll_master_ctrl);
    fmsh_print_dbg("dll_phy_dll_slave_ctrl\t 0x%x\r\n",
                   regs->dll_phy_dll_slave_ctrl);
    fmsh_print_dbg("dll_phy_tsel\t 0x%x\r\n", regs->dll_phy_tsel);
    fmsh_print_dbg("dll_phy_ctrl\t 0x%x\r\n", regs->dll_phy_ctrl);

    return 0;
}

static int nand_setup_nvddr23_iface (FNandPsu_T *nfcPtr,
                                     struct nand_interface_config *iface,
                                     struct hpnfc_timings *regs)
{
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_timings *timings = &(iface->timings);
    float ddr_clk_period, board_delay;
    int cycle, nand2_delay;
    float temp;
    int gate_open_delay, rd_del_sel, half_cycle_mode;
    int use_dll_action, number_of_element;
    int oe_start, oe_end, oe_end_dqsd;
    int oe_end_data, oe_end_dqsd_data;
    int extended_wr_mode;

    fmsh_print_dbg("NV-DDR2/3 Timming mode\t %d\r\n", timings->mode);

    (void)memset(regs, 0, sizeof(struct hpnfc_timings));

    board_delay = nfcPtr->config.board_delay;
    nand2_delay = NAND_PHY_NAND2DELAY;
    ddr_clk_period = (float)1000000000 / nfcPtr->config.clock_hz;

    /* common settings */
    regs->common_settings = NAND_MODE_NVDDR23;

    /* toggle timing0 */
    cycle = (int)(100 / ddr_clk_period);     // tCR
    regs->toggle_timings0 |= (cycle & 0x3f) << 24;

    cycle = (int)(25 / ddr_clk_period);      // tPRE
    regs->toggle_timings0 |= (cycle & 0x3f) << 16;

    cycle = (int)(30 / ddr_clk_period);      // tCDQSS
    regs->toggle_timings0 |= (cycle & 0x3f) << 8;

    cycle = (int)(25 / ddr_clk_period) + 1;  // tPSTH
    regs->toggle_timings0 |= (cycle & 0x3f);

    /* toggle timing1 */
    cycle = (int)(100 / ddr_clk_period) + 1;  // tCDQSH
    regs->toggle_timings1 |= (cycle & 0x7f) << 24;

    temp = 3 + (timings->nvddr23.tRC_min / 2);
    cycle = (int)(temp / ddr_clk_period) + 1;
    regs->toggle_timings1 |= (cycle & 0x3f) < 8;

    cycle = (int)ceil(6.5 / ddr_clk_period) + 1;  // tWPST
    regs->toggle_timings1 |= (cycle & 0x3f);

    /* async toggle timings */
    temp = timings->nvddr23.tRC_min * 0.55;
    cycle = (int)ceil(temp / ddr_clk_period);  // tRH
    regs->async_toggle_timings |= (cycle & 0x1f) << 24;

    temp = timings->nvddr23.tRC_min * 0.55;
    cycle = (int)ceil(temp / ddr_clk_period);  // tRP
    regs->async_toggle_timings |= (cycle & 0x1f) << 16;

    cycle = (int)ceil(11 / ddr_clk_period);    // tWH
    regs->async_toggle_timings |= (cycle & 0x1f) << 8;

    cycle = (int)ceil(11 / ddr_clk_period);    // tWP
    regs->async_toggle_timings |= (cycle & 0x1f);

    /* sync timings */
    regs->sync_timings = 0;

    // timings0
    cycle = (int)(timings->nvddr23.tADL_min / ddr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 24;

    cycle = (int)(device->feat.tCCS_min / ddr_clk_period);
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 16;

    cycle = (int)(80 / ddr_clk_period);  // tWHR
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 8;

    cycle = (int)ceil(100 / ddr_clk_period);  // tRHW
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle;

    // timings1
    cycle = (int)ceil(100 / ddr_clk_period) + 2;  // tWB
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings0 |= (u32)cycle << 16;

    cycle = (int)ceil(50 / ddr_clk_period);  // tVDLY
    if (cycle > 0xff)
    {
        cycle = 0xff;
    }
    regs->timings1 |= (u32)cycle;

    // timings2
    cycle = (int)ceil(1000 / ddr_clk_period);  // tFEAT
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle << 16;

    cycle = (int)ceil(20 / ddr_clk_period);  // tCEH
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle << 8;

    cycle = (int)ceil(40 / ddr_clk_period);  // tCS2
    if (cycle > 0x3f)
    {
        cycle = 0x3f;
    }
    regs->timings2 |= (u32)cycle;

    /* twp and extended write mode */
    if (ceil(ddr_clk_period / 2) >= ceil(11))
    {
        extended_wr_mode = 0;
    }
    else
    {
        extended_wr_mode = 1;
    }

    regs->mini_dll_phy_ctrl = 0x00000707;
    if (extended_wr_mode)
    {
        regs->mini_dll_phy_ctrl |= (0x1 << 17);
    }

    /***************************************************************/
    /* flash device captures RE# at poseedge of clk */
    gate_open_delay = (int)((board_delay + 18) / ddr_clk_period);  // tDQSD
    if (gate_open_delay > 15)
    {
        gate_open_delay = 15;
        fmsh_print_err("ERROR: gate_open_delay overflow in ddr mode\r\n");
    }

    rd_del_sel = (int)ceil((board_delay + 25) / ddr_clk_period);  // tDQSRE
    if (rd_del_sel <= 29)
    {
        rd_del_sel = rd_del_sel + 2;
    }
    else
    {
        fmsh_print_err("ERROR: rd_del_sel overflow\r\n");
    }

    regs->dll_phy_gate_lpbk_ctrl = (rd_del_sel << 19) | gate_open_delay;

    /***************************************************************/
    if ((ddr_clk_period * 1000) > (int)(115 * 2 * nand2_delay))
    {
        half_cycle_mode = 1;
    }
    else
    {
        half_cycle_mode = 0;
    }

    if ((ddr_clk_period * 1000 / 2) < (int)(115 * 2 * nand2_delay))
    {
        use_dll_action = 1;
    }
    else
    {
        use_dll_action = 0;
    }

    number_of_element = (int)ceil((ddr_clk_period * 1000 / 4) /
                                  (2 * nand2_delay));
    if (number_of_element > 255)
    {
        number_of_element = 255;
    }

    if (use_dll_action)
    {
        regs->dll_phy_dll_master_ctrl = (half_cycle_mode << 24) | 0x140004;
        regs->dll_phy_dll_slave_ctrl = 0x1f1f;
    }
    else
    {
        regs->dll_phy_dll_master_ctrl = 0x00800000;
        regs->dll_phy_dll_slave_ctrl = (number_of_element << 8) |
                                       number_of_element;
    }

    /***************************************************************/
    oe_start = 0;

    oe_end = (int)ceil(5 / (ddr_clk_period / 2)) + 2;
    oe_end_dqsd = (int)(18 / (ddr_clk_period / 2)) + 4;
    oe_end = (int)((oe_end + oe_end_dqsd) / 2);
    if (oe_end > 7)
    {
        oe_end = 7;
    }

    oe_end_data = (int)ceil(5 / (ddr_clk_period / 2));
    oe_end_dqsd_data = (int)(18 / (ddr_clk_period / 2));
    oe_end_data = (int)((oe_end_data + oe_end_dqsd_data) / 2);
    if (oe_end_data > 7)
    {
        oe_end_data = 7;
    }

    regs->dll_phy_dq_timings = (oe_start << 12) | (oe_end_data << 8) |
                               (oe_start << 4) | oe_end_data;
    regs->dll_phy_dqs_timings = (oe_start << 12) | (oe_end << 8) |
                                (oe_start << 4) | oe_end;

    /***************************************************************/
    regs->dll_phy_ctrl = 0x00004000;
    regs->dll_phy_tsel = 0x00000000;

    /***************************************************************/
    fmsh_print_dbg("common_settings\t 0x%x\r\n", regs->common_settings);
    fmsh_print_dbg("timings0\t 0x%x\r\n", regs->timings0);
    fmsh_print_dbg("timings1\t 0x%x\r\n", regs->timings1);
    fmsh_print_dbg("timings2\t 0x%x\r\n", regs->timings2);
    fmsh_print_dbg("async_toggle_timings\t 0x%x\r\n",
                   regs->async_toggle_timings);
    fmsh_print_dbg("sync_timings\t 0x%x\r\n", regs->sync_timings);
    fmsh_print_dbg("toggle_timings0\t 0x%x\r\n", regs->toggle_timings0);
    fmsh_print_dbg("toggle_timings1\t 0x%x\r\n", regs->toggle_timings1);
    fmsh_print_dbg("mini_dll_phy_ctrl\t 0x%x\r\n", regs->mini_dll_phy_ctrl);
    fmsh_print_dbg("dll_phy_dq_timings\t 0x%x\r\n", regs->dll_phy_dq_timings);
    fmsh_print_dbg("dll_phy_dqs_timings\t 0x%x\r\n", regs->dll_phy_dqs_timings);
    fmsh_print_dbg("dll_phy_gate_lpbk_ctrl\t 0x%x\r\n",
                   regs->dll_phy_gate_lpbk_ctrl);
    fmsh_print_dbg("dll_phy_dll_master_ctrl\t 0x%x\r\n",
                   regs->dll_phy_dll_master_ctrl);
    fmsh_print_dbg("dll_phy_dll_slave_ctrl\t 0x%x\r\n",
                   regs->dll_phy_dll_slave_ctrl);
    fmsh_print_dbg("dll_phy_tsel\t 0x%x\r\n", regs->dll_phy_tsel);
    fmsh_print_dbg("dll_phy_ctrl\t 0x%x\r\n", regs->dll_phy_ctrl);

    return 0;
}

static int nand_setup_iface (FNandPsu_T *nfcPtr,
                             struct nand_interface_config *iface)
{
    int ret;
    struct hpnfc_timings regs;

    (void)memset(&regs, 0, sizeof(regs));

    switch (iface->type)
    {
    case NAND_SDR_IFACE:
        ret = nand_setup_sdr_iface(nfcPtr, iface, &regs);
        break;
    case NAND_NVDDR_IFACE:
        ret = nand_setup_nvddr_iface(nfcPtr, iface, &regs);
        break;
    case NAND_NVDDR23_IFACE:
        ret = nand_setup_nvddr23_iface(nfcPtr, iface, &regs);
        break;
    default:
        ret = FMSH_EINVAL;
        break;
    }
    if (ret)
    {
        return ret;
    }

    /* Assert dll phy rstn */
    ret = FNandPsu_SetTimings(nfcPtr, &regs);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

/**************************** nand_ctrl_ops *******************************/
static int nand_device_reset (FNandPsu_T *nfcPtr, int cs)
{
    int ret;

    nand_select_target(nfcPtr, cs);

    ret = FNandPsu_PIO_Reset(nfcPtr, 0, 0, NAND_ASYNC_RST);

    nand_deselect_target(nfcPtr);

    return ret;
}

static int nand_set_feature (FNandPsu_T *nfcPtr, int cs, u8 feature, void *data)
{
    int ret;

    nand_select_target(nfcPtr, cs);

    ret = FNandPsu_PIO_SetFeature(nfcPtr, 0, feature, *(u32 *)data);

    nand_deselect_target(nfcPtr);

    return ret;
}

static int nand_erase_block (FNandPsu_T *nfcPtr, int cs, u32 page, u32 len)
{
    int ret;
    u8 nblocks;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);

    nand_select_target(nfcPtr, cs);

    nblocks = len / device->model.blocksize;
    ret = FNandPsu_PIO_Erase(nfcPtr, 0, page, nblocks);

    nand_deselect_target(nfcPtr);

    return ret;
}

static int nand_write_page (FNandPsu_T *nfcPtr, int cs, unsigned int page,
                            unsigned int offset, void *buf, unsigned int len,
                            int raw)
{
    int ret;
    u8 npages;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);

    nand_select_target(nfcPtr, cs);
    if (raw || (nfcPtr->ecc_mode == ECC_NONE) ||
        (nfcPtr->ecc_mode == ECC_ONDIE))
    {
        /* Config page parameter */
        FNandPsu_SetPageType(nfcPtr, NAND_TT_PAGE_RAW);
        FNandPsu_SetSkipByte(nfcPtr, 0, 0, 0);
        /* Config Ecc */
        FNandPsu_SetEccEnable(nfcPtr, 0);
    }
    else
    {
        /* Config page parameter */
        FNandPsu_SetPageType(nfcPtr, NAND_TT_MAIN_OOB);
        FNandPsu_SetSkipByte(nfcPtr, device->bb_pattern.pattern,
                             device->model.pagesize + device->bb_pattern.offset,
                             device->bb_pattern.length);
        /* Config Ecc */
        FNandPsu_SetEccEnable(nfcPtr, 1);
    }
    npages = len / device->model.pagesize;
    ret = FNandPsu_CDMA_WritePage(nfcPtr, 0, page, buf, npages);

    nand_deselect_target(nfcPtr);

    return ret;
}

static int nand_read_page (FNandPsu_T *nfcPtr, int cs, unsigned int page,
                           unsigned int offset, void *buf, unsigned int len,
                           int raw)
{
    int ret;
    u8 npages;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);

    nand_select_target(nfcPtr, cs);
    if (raw || (nfcPtr->ecc_mode == ECC_NONE) ||
        (nfcPtr->ecc_mode == ECC_ONDIE))
    {
        /* Config page parameter */
        FNandPsu_SetPageType(nfcPtr, NAND_TT_PAGE_RAW);
        FNandPsu_SetSkipByte(nfcPtr, 0, 0, 0);
        /* Config Ecc */
        FNandPsu_SetEccEnable(nfcPtr, 0);
    }
    else
    {
        /* Config page parameter */
        FNandPsu_SetPageType(nfcPtr, NAND_TT_MAIN_OOB);
        FNandPsu_SetSkipByte(nfcPtr, device->bb_pattern.pattern,
                             device->model.pagesize + device->bb_pattern.offset,
                             device->bb_pattern.length);
        /* Config Ecc */
        FNandPsu_SetEccEnable(nfcPtr, 1);
    }
    npages = len / device->model.pagesize;
    ret = FNandPsu_CDMA_ReadPage(nfcPtr, 0, page, buf, npages);

    nand_deselect_target(nfcPtr);

    return ret;
}

static int nand_copyback (FNandPsu_T *nfcPtr, int cs, u32 src_page,
                          u32 dst_page, u8 npages)
{
    int ret;

    nand_select_target(nfcPtr, cs);

    ret = FNandPsu_CDMA_CopyBack(nfcPtr, 0, src_page, dst_page, npages);

    nand_deselect_target(nfcPtr);

    return ret;
}

/******************************************************************************
 * SDMA transfer
 *
 *****************************************************************************/
static int wait_sdma_trigg (FNandPsu_T *nfcPtr, u64 *sdma_addr, u32 *sdma_size,
                            u32 *sdma_trd_num, int timeout_us)
{
    int ret;

    ret = FNandPsu_PollSdmaTrigg(nfcPtr, timeout_us);
    if (ret)
    {
        return ret;
    }

    if (sdma_size)
    {
        *sdma_size = FMSH_ReadReg(nfcPtr->config.base, NAND_R_SDMA_SIZE);
    }
    if (sdma_trd_num)
    {
        *sdma_trd_num = FMSH_ReadReg(nfcPtr->config.base, NAND_R_SDMA_TRD_NUM);
    }
    if (sdma_addr)
    {
        *sdma_addr = nfcPtr->config.data_base;
    }

    return FMSH_SUCCESS;
}

static int nand_sdma_finish (FNandPsu_T *nfcPtr) { return 0; }

static unsigned long long read_slave_dma64 (u32 addr)
{
#ifdef __aarch64__
    unsigned long long data;
    data = *(unsigned long long *)addr;
    return data;
#else
    u32 data_l, data_h;
    __asm volatile("ldrexd %[Rn], %[Rm], [%[Rd]] \n"
                   : [Rn] "=r"(data_l), [Rm] "=r"(data_h)
                   : [Rd] "r"(addr)
                   : "cc");
    return ((u64)data_h << 32) | data_l;
#endif
}

static int write_slave_dma64 (u32 addr, unsigned long long data)
{
#ifdef __aarch64__
    *(unsigned long long *)addr = data;
    return 0;
#else
    fmsh_print_err(
        "Write operation using sdma slave is not support for __aarch32__\r\n");
    return 1;
#endif
}
static int nand_sdma_transfer (FNandPsu_T *nfcPtr, void *buf, u32 len,
                               enum hpnfc_tsf_dir dir)
{
    int ret, i;
    u64 sdma_addr;
    u32 sdma_size, sdma_trd;
    u64 temp_buf[1] = {0};

    /* Wait for sdma ready */
    ret = wait_sdma_trigg(nfcPtr, &sdma_addr, &sdma_size, &sdma_trd, 100000);
    if (ret)
    {
        return ret;
    }

    if (len > sdma_size)
    {
        len = sdma_size;
    }

    /* Transfer data */
    if (nfcPtr->usercfg->dma_type == NAND_SDMA)
    {
        nfcPtr->dma.status = 0;
        nfcPtr->dma.len = len;
        nfcPtr->dma.dir = dir;

        if (nfcPtr->dma.dir == READ_FROM_DEVICE)
        {
            nfcPtr->dma.dst = (void *)(uintptr_t)sdma_addr;
            nfcPtr->dma.dst = buf;
            ret = nfcPtr->dma.handler(nfcPtr);
        }
        else
        {
            nfcPtr->dma.dst = buf;
            nfcPtr->dma.dst = (void *)(uintptr_t)sdma_addr;
            ret = nfcPtr->dma.handler(nfcPtr);
            ;
        }

        if (ret)
        {
            return ret;
        }

        ret = nand_sdma_finish(nfcPtr);
        if (ret)
        {
            return ret;
        }
    }
    else
    {
        int len_in_dwords = len / 8;
        int left_dwords = sdma_size / 8 - len_in_dwords;
        u64 *buffer = (u64 *)buf;

        /* read alingment data */
        if (dir == READ_FROM_DEVICE)
        {
            for (i = 0; i < len_in_dwords; i++)
            {
                *buffer++ = read_slave_dma64(nfcPtr->config.data_base);
            }
            /* read rest data from slave DMA interface if any */
            for (i = 0; i < left_dwords; i++)
            {
                temp_buf[0] = read_slave_dma64(nfcPtr->config.data_base);
            }
            (void)memcpy(buffer, temp_buf, len - (len_in_dwords << 3));
        }
        else if (dir == WRITE_TO_DEVICE)
        {
            for (i = 0; i < len_in_dwords; i++)
            {
                write_slave_dma64(nfcPtr->config.data_base, *buffer++);
            }
            /* write rest data from slave DMA interface if any */
            (void)memcpy(temp_buf, buffer, len - (len_in_dwords << 3));
            for (i = 0; i < left_dwords; i++)
            {
                write_slave_dma64(nfcPtr->config.data_base, temp_buf[0]);
            }
        }
        else
        {
            ; /* no deal with */
        }
    }

    return 0;
}

/******************************************************************************
 * PIO work mode
 * Dedicated command registers are used to trigger operation
 * in selected execution thread of the Command Engine.
 *
 ******************************************************************************/
int FNandPsu_PIO_Send (FNandPsu_T *nfcPtr, int thread, u32 *cmd)
{
    int ret;

    /* Wait for thread ready */
    ret = FNandPsu_PollTrdReady(nfcPtr, thread, 100);
    if (ret)
    {
        return ret;
    }

    /* Clear trd status */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_COMP_INTR_STATUS,
                  0x1 << thread);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_STATUS,
                  0x1 << thread);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_STATUS,
                  0x1 << thread);

    /* Send PIO command */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG1, cmd[1]);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG2, cmd[2]);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG3, cmd[3]);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG4, cmd[4]);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG0, cmd[0]);

    return FMSH_SUCCESS;
}

int FNandPsu_PIO_SendAndWait (FNandPsu_T *nfcPtr, int thread, u32 *cmd,
                              int timeout_us)
{
    int ret;

    /* Send PIO command */
    FNandPsu_PIO_Send(nfcPtr, thread, cmd);

    /* Wait for trd comp */
    ret = FNandPsu_PollTrdComp(nfcPtr, thread, timeout_us);
    if (ret)
    {
        return ret;
    }

    // read status
    ret = FNandPsu_PollCmdComp(nfcPtr, thread, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_PIO_Reset (FNandPsu_T *nfcPtr, int vol, u32 page, int type)
{
    int ret;
    int trd;
    u32 cmd[7] = {0};

    FMSH_ASSERT(nfcPtr != NULL);

    trd = nfcPtr->cur_cs;

    cmd[0] = NAND_CMD_LAYOUT_PIO | NAND_CMD_INT;
    cmd[0] |= NAND_CMD_CT_RST | type;
    cmd[0] |= NAND_CMD_TRD_NUM(trd);
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            cmd[0] |= NAND_CMD_VOL_ID(vol);
            nfcPtr->cur_vol = vol;
        }
    }
    cmd[1] = page;
    cmd[4] = nfcPtr->cur_cs << 24;

    ret = FNandPsu_PIO_SendAndWait(nfcPtr, trd, cmd, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_PIO_SetFeature (FNandPsu_T *nfcPtr, int vol, u8 feature, u32 data)
{
    int ret;
    int trd;
    u32 cmd[7] = {0};

    FMSH_ASSERT(nfcPtr != NULL);

    trd = nfcPtr->cur_cs;
    cmd[0] = NAND_CMD_LAYOUT_PIO | NAND_CMD_INT;
    cmd[0] |= NAND_CMD_CT_SETFEAT;
    cmd[0] |= NAND_CMD_TRD_NUM(trd);
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            cmd[0] |= NAND_CMD_VOL_ID(vol);
            nfcPtr->cur_vol = vol;
        }
    }
    cmd[1] = feature;
    cmd[2] = data;
    cmd[4] = nfcPtr->cur_cs << 24;

    ret = FNandPsu_PIO_SendAndWait(nfcPtr, trd, cmd, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_PIO_Erase (FNandPsu_T *nfcPtr, int vol, u32 page, u8 nblocks)
{
    int ret;
    int trd;
    u32 cmd[7] = {0};

    FMSH_ASSERT(nfcPtr != NULL);

    trd = nfcPtr->cur_cs;
    cmd[0] = NAND_CMD_LAYOUT_PIO | NAND_CMD_INT;
    cmd[0] |= NAND_CMD_CT_ERASE;
    if (nblocks > 0)
    {
        cmd[0] |= (nblocks - 1);
    }
    cmd[0] |= NAND_CMD_TRD_NUM(trd);
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            cmd[0] |= NAND_CMD_VOL_ID(vol);
            nfcPtr->cur_vol = vol;
        }
    }
    cmd[1] = page;
    cmd[4] = nfcPtr->cur_cs << 24;

    ret = FNandPsu_PIO_SendAndWait(nfcPtr, trd, cmd, 10000 * nblocks);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_PIO_WritePage (FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                            u8 npages)
{
    int ret;
    int trd;
    u32 cmd[7] = {0};
    u32 len;
    struct nand_device *device;

    FMSH_ASSERT(nfcPtr != NULL);

#if (DCACHE_ENABLE == 1)
    len = npages *
          (nfcPtr->device->model.pagesize + nfcPtr->device->model.oobsize);
    Fmsh_DCacheFlushRange((uintptr_t)buf, len);
#endif

    device = CTRL_TO_NAND(nfcPtr);
    len = device->model.pagesize * npages;
    trd = nfcPtr->cur_cs;
    cmd[0] = NAND_CMD_LAYOUT_PIO | NAND_CMD_INT;
    cmd[0] |= NAND_CMD_CT_WR;
    if (npages > 0)
    {
        cmd[0] |= (npages - 1);
    }
    cmd[0] |= NAND_CMD_TRD_NUM(trd);
    if (nfcPtr->usercfg->dma_type == NAND_MDMA)
    {
        cmd[0] |= NAND_CMD_MDMA;
    }
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            cmd[0] |= NAND_CMD_VOL_ID(vol);
            nfcPtr->cur_vol = vol;
        }
    }
    cmd[1] = page;
    cmd[2] = (unsigned long long)buf & 0xffffffff;
    cmd[3] = ((unsigned long long)buf >> 32) & 0xffffffff;
    cmd[4] = nfcPtr->cur_cs << 24;

    if (nfcPtr->usercfg->dma_type == NAND_MDMA)
    {
        ret = FNandPsu_PIO_SendAndWait(nfcPtr, trd, cmd, 4000 * npages);
        if (ret)
        {
            return ret;
        }
    }
    else
    {
        ret = FNandPsu_PIO_Send(nfcPtr, trd, cmd);
        if (ret)
        {
            return ret;
        }
        /* Transfer data */
        len = npages *
              (nfcPtr->device->model.pagesize + nfcPtr->device->model.oobsize);
        ret = nand_sdma_transfer(nfcPtr, buf, len, WRITE_TO_DEVICE);
        if (ret)
        {
            return ret;
        }
        /* Wait for trd comp */
        ret = FNandPsu_PollTrdComp(nfcPtr, trd, 4000 * npages);
        if (ret)
        {
            return ret;
        }
        // read status
        ret = FNandPsu_PollCmdComp(nfcPtr, trd, 4000 * npages);
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_PIO_ReadPage (FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                           u8 npages)
{
    int ret;
    int trd;
    u32 cmd[7] = {0};
    u32 len;
    struct nand_device *device;

    FMSH_ASSERT(nfcPtr != NULL);

#if (DCACHE_ENABLE == 1)
    len = npages *
          (nfcPtr->device->model.pagesize + nfcPtr->device->model.oobsize);
    Fmsh_DCacheInvalidateRange((uintptr_t)buf, len);
#endif

    device = CTRL_TO_NAND(nfcPtr);
    len = device->model.pagesize * npages;
    trd = nfcPtr->cur_cs;
    cmd[0] = NAND_CMD_LAYOUT_PIO | NAND_CMD_INT;
    cmd[0] |= NAND_CMD_CT_RD;
    if (npages > 0)
    {
        cmd[0] |= (npages - 1);
    }
    cmd[0] |= NAND_CMD_TRD_NUM(trd);
    if (nfcPtr->usercfg->dma_type == NAND_MDMA)
    {
        cmd[0] |= NAND_CMD_MDMA;
    }
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            cmd[0] |= NAND_CMD_VOL_ID(vol);
            nfcPtr->cur_vol = vol;
        }
    }
    cmd[1] = page;
    cmd[2] = (unsigned long long)buf & 0xffffffff;
    cmd[3] = ((unsigned long long)buf >> 32) & 0xffffffff;
    cmd[4] = nfcPtr->cur_cs << 24;

    if (nfcPtr->usercfg->dma_type == NAND_MDMA)
    {
        ret = FNandPsu_PIO_SendAndWait(nfcPtr, trd, cmd, 4000 * npages);
        if (ret)
        {
            return ret;
        }
    }
    else
    {
        /* Send PIO command */
        FNandPsu_PIO_Send(nfcPtr, trd, cmd);
        /* Transfer data */
        len = npages *
              (nfcPtr->device->model.pagesize + nfcPtr->device->model.oobsize);
        ret = nand_sdma_transfer(nfcPtr, buf, len, READ_FROM_DEVICE);
        if (ret)
        {
            return ret;
        }
        /* Wait for trd comp */
        ret = FNandPsu_PollTrdComp(nfcPtr, trd, 4000 * npages);
        if (ret)
        {
            return ret;
        }

        // read status
        ret = FNandPsu_PollCmdComp(nfcPtr, trd, 4000 * npages);
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_PIO_CopyBack (FNandPsu_T *nfcPtr, int vol, u32 src_page,
                           u32 dst_page, u8 npages)
{
    int ret;
    int trd;
    u32 cmd[7] = {0};

    FMSH_ASSERT(nfcPtr != NULL);

    trd = nfcPtr->cur_cs;

    cmd[0] = NAND_CMD_LAYOUT_PIO | NAND_CMD_INT;
    cmd[0] |= NAND_CMD_CT_CB;
    if (npages > 0)
    {
        cmd[0] |= (npages - 1);
    }
    cmd[0] |= NAND_CMD_TRD_NUM(trd);
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            cmd[0] |= NAND_CMD_VOL_ID(vol);
            nfcPtr->cur_vol = vol;
        }
    }
    cmd[1] = src_page;
    cmd[2] = dst_page;
    cmd[4] = nfcPtr->cur_cs << 24;

    ret = FNandPsu_PIO_SendAndWait(nfcPtr, nfcPtr->cur_cs, cmd, 1000 * npages);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

/******************************************************************************
 * CDMA work mode
 * Uses descriptor-based command processing functionality
 * of the Command Engine module
 *
 ******************************************************************************/
int FNandPsu_CDMA_Send (FNandPsu_T *nfcPtr, int thread,
                        struct hpnfc_cdma_desc *desc)
{
    int ret;

    /* Wait for thread ready */
    ret = FNandPsu_PollTrdReady(nfcPtr, nfcPtr->cur_cs, 100);
    if (ret)
    {
        return ret;
    }

    /* Clear trd status */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_COMP_INTR_STATUS,
                  0x1 << thread);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_STATUS,
                  0x1 << thread);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_STATUS,
                  0x1 << thread);

#if (DCACHE_ENABLE == 1)
    Fmsh_DCacheFlushRange((uintptr_t)desc, sizeof(struct hpnfc_cdma_desc));
#endif

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG2,
                  (unsigned long long)desc & 0xffffffff);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG3, 0);

    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG0,
                  NAND_CMD_TRD_NUM(thread));

    return FMSH_SUCCESS;
}

int FNandPsu_CDMA_SendAndWait (FNandPsu_T *nfcPtr, int thread,
                               struct hpnfc_cdma_desc *desc, int timeout_us)
{
    int ret;

    /* Send CDMA */
    ret = FNandPsu_CDMA_Send(nfcPtr, thread, desc);
    if (ret)
    {
        return ret;
    }

    /* Wait for trd comp */
    ret = FNandPsu_PollTrdComp(nfcPtr, thread, timeout_us);
    if (ret)
    {
        return ret;
    }

    // read status
    ret = FNandPsu_PollCmdComp(nfcPtr, thread, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_CDMA_Reset (FNandPsu_T *nfcPtr, int vol, u32 page, int type)
{
    int ret;
    int trd;
    struct hpnfc_cdma_desc desc = {0};

    FMSH_ASSERT(nfcPtr != NULL);

    trd = nfcPtr->cur_cs;

    desc.command_type = NAND_CMD_CT_RST | type;
    desc.command_flags = NAND_DESC_CFLAGS_INT;
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            desc.command_flags |= (vol & 0xf) << 4;
            nfcPtr->cur_vol = vol;
        }
    }
    desc.flash_pointer = page;
    desc.bank = nfcPtr->cur_cs;

    ret = FNandPsu_CDMA_SendAndWait(nfcPtr, trd, &desc, 1000);
    if (ret)
    {
        return ret;
    }

    ret = FNandPsu_PollDevReady(nfcPtr, nfcPtr->cur_cs, 10000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_CDMA_Erase (FNandPsu_T *nfcPtr, int vol, u32 page, u8 nblocks)
{
    int ret;
    int trd;
    struct hpnfc_cdma_desc desc = {0};

    FMSH_ASSERT(nfcPtr != NULL);

    trd = nfcPtr->cur_cs;

    // page should align to block
    desc.command_type = NAND_CMD_CT_ERASE;
    if (nblocks > 0)
    {
        desc.command_type |= (nblocks - 1);
    }
    desc.command_flags = NAND_DESC_CFLAGS_INT;
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            desc.command_flags |= (vol & 0xf) << 4;
            nfcPtr->cur_vol = vol;
        }
    }
    desc.flash_pointer = page;
    desc.bank = nfcPtr->cur_cs;

    ret = FNandPsu_CDMA_SendAndWait(nfcPtr, trd, &desc, 10000 * nblocks);
    if (ret)
    {
        return ret;
    }

    ret = FNandPsu_PollDevReady(nfcPtr, nfcPtr->cur_cs, 10000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

static int FNandPsu_CDMA_Transfer (FNandPsu_T *nfcPtr, int vol, u32 page,
                                   void *buf, u8 npages, enum hpnfc_tsf_dir dir)
{
    int ret;
    int trd;
    struct hpnfc_cdma_desc desc = {0};

    trd = nfcPtr->cur_cs;

    if (dir == READ_FROM_DEVICE)
    {
        desc.command_type = NAND_CMD_CT_RD;
    }
    else
    {
        desc.command_type = NAND_CMD_CT_WR;
    }
    if (npages > 0)
    {
        desc.command_type |= (npages - 1);
    }
    desc.command_flags = NAND_DESC_CFLAGS_INT;
    if (nfcPtr->usercfg->dma_type == NAND_MDMA)
    {
        desc.command_flags |= NAND_DESC_CFLAGS_MDMA;
    }
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            desc.command_flags |= (vol & 0xf) << 4;
            nfcPtr->cur_vol = vol;
        }
    }
    desc.flash_pointer = page;
    desc.bank = nfcPtr->cur_cs;
    desc.memory_pointer = (u64)buf;

    if (nfcPtr->usercfg->dma_type == NAND_MDMA)
    {
        ret = FNandPsu_CDMA_SendAndWait(nfcPtr, trd, &desc, 4000 * npages);
        if (ret)
        {
            return ret;
        }
    }
    else
    {
        ret = FNandPsu_CDMA_Send(nfcPtr, trd, &desc);
        if (ret)
        {
            return ret;
        }
        u32 len = npages * (nfcPtr->device->model.pagesize +
                            nfcPtr->device->model.oobsize);
        ret = nand_sdma_transfer(nfcPtr, buf, len, dir);
        if (ret)
        {
            return ret;
        }

        /* Wait for trd comp */
        ret = FNandPsu_PollTrdComp(nfcPtr, trd, 4000 * npages);
        if (ret)
        {
            return ret;
        }

        // read status
        ret = FNandPsu_PollCmdComp(nfcPtr, trd, 4000 * npages);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

int FNandPsu_CDMA_WritePage (FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                             u8 npages)
{
    int ret;

    FMSH_ASSERT(nfcPtr != NULL);

#if (DCACHE_ENABLE == 1)
    u32 len = npages *
              (nfcPtr->device->model.pagesize + nfcPtr->device->model.oobsize);
    Fmsh_DCacheFlushRange((uintptr_t)buf, len);
#endif

    ret = FNandPsu_CDMA_Transfer(nfcPtr, vol, page, buf, npages,
                                 WRITE_TO_DEVICE);
    if (ret)
    {
        return ret;
    }

    return 0;
}

int FNandPsu_CDMA_ReadPage (FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                            u8 npages)
{
    int ret;

    FMSH_ASSERT(nfcPtr != NULL);

#if (DCACHE_ENABLE == 1)
    u32 len = npages *
              (nfcPtr->device->model.pagesize + nfcPtr->device->model.oobsize);
    Fmsh_DCacheInvalidateRange((uintptr_t)buf, len);
#endif

    ret = FNandPsu_CDMA_Transfer(nfcPtr, vol, page, buf, npages,
                                 READ_FROM_DEVICE);
    if (ret)
    {
        return ret;
    }

    return 0;
}

int FNandPsu_CDMA_CopyBack (FNandPsu_T *nfcPtr, int vol, u32 src_page,
                            u32 dst_page, u8 npages)
{
    int ret;
    int trd;
    struct hpnfc_cdma_desc desc = {0};

    FMSH_ASSERT(nfcPtr != NULL);

    trd = nfcPtr->cur_cs;

    desc.command_type = NAND_CMD_CT_CB;
    if (npages > 0)
    {
        desc.command_type |= (npages - 1);
    }
    desc.command_flags = NAND_DESC_CFLAGS_INT;
    if (nfcPtr->usercfg->options & NAND_USE_VOLUME)
    {
        if (vol != nfcPtr->cur_vol)
        {
            desc.command_flags |= (vol & 0xf) << 4;
            nfcPtr->cur_vol = vol;
        }
    }
    desc.flash_pointer = src_page;
    desc.bank = nfcPtr->cur_cs;
    desc.memory_pointer = dst_page;

    ret = FNandPsu_CDMA_SendAndWait(nfcPtr, trd, &desc, 1000 * npages);
    if (ret)
    {
        return ret;
    }

    return 0;
}

/******************************************************************************
 * Generic work mode
 * Allows to bypass the Command Engine module and allows to
 * send low level commands directly to the MiniController unit
 *
 *****************************************************************************/
int FNandPsu_Generic_CmdSend (FNandPsu_T *nfcPtr, int thread, u64 mini_ctrl_cmd)
{
    int ret;
    u32 regs[4] = {0};

    /* Wait for thread ready */
    ret = FNandPsu_PollTrdReady(nfcPtr, thread, 100);
    if (ret)
    {
        return ret;
    }

    /* Clear trd status */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_COMP_INTR_STATUS,
                  0x1 << thread);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_ERROR_INTR_STATUS,
                  0x1 << thread);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_TRD_TIMEOUT_INTR_STATUS,
                  0x1 << thread);

    regs[0] = NAND_CMD_LAYOUT_GENERIC | NAND_CMD_INT;
    regs[0] |= NAND_CMD_TRD_NUM(thread);
    mini_ctrl_cmd |= (nfcPtr->cur_cs & 0xf) << 8;
    regs[2] = mini_ctrl_cmd & 0xffffffff;
    regs[3] = mini_ctrl_cmd >> 32;

    /* Send Generic command */
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG3, regs[3]);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG2, regs[2]);
    FMSH_WriteReg(nfcPtr->config.base, NAND_R_CMD_REG0, regs[0]);

    return FMSH_SUCCESS;
}

int FNandPsu_Generic_CmdSendAndWait (FNandPsu_T *nfcPtr, int thread,
                                     u64 mini_ctrl_cmd, int timeout_us)
{
    int ret;

    ret = FNandPsu_Generic_CmdSend(nfcPtr, thread, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }

    /* Wait for trd comp */
    ret = FNandPsu_PollTrdComp(nfcPtr, thread, timeout_us);
    if (ret)
    {
        return ret;
    }

    // read status
    ret = FNandPsu_PollCmdComp(nfcPtr, thread, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

static int generic_instr_cmd (FNandPsu_T *nfcPtr, struct nand_instr *instrs)
{
    int ret;
    u64 mini_ctrl_cmd = 0;

    if (instrs->delay_ns > 0)
    {
        mini_ctrl_cmd |= NAND_GCMD_LAYOUT_TWB;
    }

    mini_ctrl_cmd |= NAND_GCMD_LAY_INSTR_CMD;
    mini_ctrl_cmd |= NAND_GCMD_LAYOUT_BANK_NUM(nfcPtr->cur_cs);

    mini_ctrl_cmd |= instrs->ctx.cmd.opcode << 16;

    ret = FNandPsu_Generic_CmdSend(nfcPtr, nfcPtr->cur_cs, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }
    return FMSH_SUCCESS;
}

static int generic_instr_addr (FNandPsu_T *nfcPtr, struct nand_instr *instrs)
{
    int ret, i;
    u64 mini_ctrl_cmd = 0, addr = 0;
    u32 naddrs;
    u8 *paddr;

    naddrs = instrs->ctx.addr.naddrs;
    paddr = instrs->ctx.addr.addrs;

    if (instrs->delay_ns > 0)
    {
        mini_ctrl_cmd |= NAND_GCMD_LAYOUT_TWB;
    }

    mini_ctrl_cmd |= NAND_GCMD_LAY_INSTR_ADDR;
    mini_ctrl_cmd |= NAND_GCMD_LAYOUT_BANK_NUM(nfcPtr->cur_cs);

    for (i = 0; i < naddrs; i++)
    {
        addr = addr << 8;
        addr |= paddr[i];
    }
    mini_ctrl_cmd |= addr << 16;
    mini_ctrl_cmd |= NAND_GCMD_LAY_ADDR_NBYTES(naddrs - 1);

    ret = FNandPsu_Generic_CmdSend(nfcPtr, nfcPtr->cur_cs, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

static int generic_instr_data (FNandPsu_T *nfcPtr, struct nand_instr *instrs,
                               enum hpnfc_tsf_dir dir)
{
    int ret;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_model *model = &(device->model);
    struct nand_ecc *ecc = &(device->ecc);
    u64 mini_ctrl_cmd = 0;
    u8 force_8bit, raw;
    u32 sector_cnt, sector_size, last_sector_size;
    ;

    raw = instrs->ctx.data.raw;
    force_8bit = instrs->ctx.data.force_8bit;

    if (instrs->delay_ns > 0)
    {
        mini_ctrl_cmd |= NAND_GCMD_LAYOUT_TWB;
    }

    mini_ctrl_cmd |= NAND_GCMD_LAY_INSTR_DATA;
    mini_ctrl_cmd |= NAND_GCMD_LAYOUT_BANK_NUM(nfcPtr->cur_cs);

    mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_DIR(dir);
    if (raw)
    {
        sector_cnt = 1;
        sector_size = 0;
        last_sector_size = instrs->ctx.data.len;
    }
    else
    {
        sector_cnt = ecc->steps;
        sector_size = ecc->size;
        last_sector_size = ecc->size + ecc->available_oob_size;
        mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_ECC_EN;
    }
    mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_SECT_SIZE(sector_size);
    mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_SECT_CNT(sector_cnt);
    mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_LAST_SECT_SIZE(last_sector_size);
    mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_CORR_CAP(ecc->str_idx);

    if (instrs->ctx.data.options & NAND_ECC_SCRAMBLER)
    {
        mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_SCRAMBLER_EN;
    }
    if (instrs->ctx.data.options & NAND_ERASED_DET)
    {
        mini_ctrl_cmd |= NAND_GCMD_LAY_DATA_PAGEDET_EN;
    }

    if (force_8bit && (model->io_width == 16))
    {
        (void)FNandPsu_SetIOWidth16(nfcPtr, 0);
    }

    ret = FNandPsu_Generic_CmdSend(nfcPtr, nfcPtr->cur_cs, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }

    /* transfer data */
    ret = nand_sdma_transfer(nfcPtr, instrs->ctx.data.buf, instrs->ctx.data.len,
                             dir);
    if (ret)
    {
        return ret;
    }

    if (force_8bit && (model->io_width == 16))
    {
        (void)FNandPsu_SetIOWidth16(nfcPtr, 1);
    }

    return FMSH_SUCCESS;
}

static int generic_instr_waitrdy (FNandPsu_T *nfcPtr, struct nand_instr *instrs)
{
    int ret;
    u32 timeout_us;

    timeout_us = instrs->ctx.waitrdy.timeout_us;

    ret = FNandPsu_PollDevReady(nfcPtr, nfcPtr->cur_cs, timeout_us);

    return ret;
}

static int generic_instr_getfeature (FNandPsu_T *nfcPtr,
                                     struct nand_instr *instrs)
{
    int ret;
    u64 mini_ctrl_cmd = 0;
    u8 *paddr;

    paddr = instrs->ctx.addr.addrs;

    mini_ctrl_cmd |= GET_FEAT_INSTR;
    mini_ctrl_cmd |= NAND_GCMD_LAYOUT_BANK_NUM(nfcPtr->cur_cs);

    mini_ctrl_cmd |= paddr[0] << 16;

    ret = FNandPsu_Generic_CmdSend(nfcPtr, nfcPtr->cur_cs, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

static int generic_instr_rdid (FNandPsu_T *nfcPtr, struct nand_instr *instrs)
{
    int ret;
    u64 mini_ctrl_cmd = 0;
    u8 *paddr;

    paddr = instrs->ctx.addr.addrs;

    mini_ctrl_cmd |= RDID_INSTR;
    mini_ctrl_cmd |= NAND_GCMD_LAYOUT_BANK_NUM(nfcPtr->cur_cs);

    mini_ctrl_cmd |= paddr[0] << 16;

    ret = FNandPsu_Generic_CmdSend(nfcPtr, nfcPtr->cur_cs, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

static int generic_instr_rdparapage (FNandPsu_T *nfcPtr,
                                     struct nand_instr *instrs)
{
    int ret;
    u64 mini_ctrl_cmd = 0;
    u8 *paddr;

    paddr = instrs->ctx.addr.addrs;

    mini_ctrl_cmd |= READ_PARAPAGE_INSTR;
    mini_ctrl_cmd |= NAND_GCMD_LAYOUT_BANK_NUM(nfcPtr->cur_cs);

    mini_ctrl_cmd |= paddr[0] << 16;

    ret = FNandPsu_Generic_CmdSend(nfcPtr, nfcPtr->cur_cs, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

static int generic_instr_rdpage (FNandPsu_T *nfcPtr, struct nand_instr *instrs)
{
    int ret;
    u64 mini_ctrl_cmd = 0;
    u8 *paddr;
    unsigned int i, naddrs;

    paddr = instrs->ctx.addr.addrs;
    naddrs = instrs->ctx.addr.naddrs;

    mini_ctrl_cmd |= 0x3;
    mini_ctrl_cmd |= NAND_GCMD_LAYOUT_BANK_NUM(nfcPtr->cur_cs);

    mini_ctrl_cmd |= NAND_GCMD_LAY_ADDR_NBYTES(naddrs);
    for (i = 0; i < naddrs; i++)
    {
        mini_ctrl_cmd |= (u64)paddr[i] << (16 + i * 8);
    }

    ret = FNandPsu_Generic_CmdSend(nfcPtr, nfcPtr->cur_cs, mini_ctrl_cmd);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

static int nand_exec_op (FNandPsu_T *nfcPtr, struct nand_operation *ops)
{
    int ret;
    int i, ninstr;
    struct nand_instr *instrs;

    nand_select_target(nfcPtr, ops->cs);

    ninstr = ops->ninstr;
    instrs = ops->instr;

    for (i = 0; i < ninstr; i++)
    {
        switch (instrs[i].instr_type)
        {
        // generic instr sequences
        case CMD_INSTR:
            ret = generic_instr_cmd(nfcPtr, &instrs[i]);
            break;
        case ADDR_INSTR:
            ret = generic_instr_addr(nfcPtr, &instrs[i]);
            break;
        case DATAIN_INSTR:
            ret = generic_instr_data(nfcPtr, &instrs[i], READ_FROM_DEVICE);
            break;
        case DATAOUT_INSTR:
            ret = generic_instr_data(nfcPtr, &instrs[i], WRITE_TO_DEVICE);
            break;
        case WAITRDY_INSTR:
            ret = generic_instr_waitrdy(nfcPtr, &instrs[i]);
            break;
        // specified instr sequences
        case GET_FEAT_INSTR:
            ret = generic_instr_getfeature(nfcPtr, &instrs[i]);
            break;
        case RDID_INSTR:
            ret = generic_instr_rdid(nfcPtr, &instrs[i]);
            break;
        case READ_PARAPAGE_INSTR:
            ret = generic_instr_rdparapage(nfcPtr, &instrs[i]);
            break;
        case READ_INSTR:
            ret = generic_instr_rdpage(nfcPtr, &instrs[i]);
            break;

        default:
            ret = FMSH_EINVAL;
            break;
        }
        if (ret)
        {
            nand_deselect_target(nfcPtr);
            return ret;
        }
    }

    // Wait for trd comp
    ret = FNandPsu_PollTrdComp(nfcPtr, nfcPtr->cur_cs, 100);
    if (ret == 0)
    {
        ret = FNandPsu_PollCmdComp(nfcPtr, nfcPtr->cur_cs, 100);
    }

    nand_deselect_target(nfcPtr);

    return ret;
}

int FNandPsu_SetOobBuf (FNandPsu_T *nfcPtr, u8 *buf, unsigned int len)
{
    struct nand_device *device;

    FMSH_ASSERT(nfcPtr != NULL);
    FMSH_ASSERT(buf != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    device->oob_buf = buf;
    device->ooblen = len;

    return FMSH_SUCCESS;
}
