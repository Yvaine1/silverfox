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
 * @file fmsh_hpnfc_flash.h
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This source file contains the device operating functions.
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
#include <stdlib.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_hpnfc_bbm.h"
#include "fmsh_hpnfc_flash.h"
#include "fmsh_hpnfc_hw.h"

/*
 *  Only used for bootrom,
 *  If BOOT_SEARCH is defined, only part of device will be scaned
 *  to get bad block table whose size is defined by BOOT_SEARCH_SIZE_SHIFT.
 */
// #define BOOT_SEARCH
#define BOOT_SEARCH_SIZE_SHIFT (27)

#define ONFI_PARAM_PAGES       (3)
#define ONFI_CRC_BASE          (0x4F4E)

static struct nand_interface_config sdr_interface_config[] = {
    {
        .type = NAND_SDR_IFACE,
        .timings.mode = 0,
        .timings.sdr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 70,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 40,
                .tWHR_min = 120,
                .tWB_max = 200,
                .tWW_min = 100,
                .tRC_min = 100,
                .tREA_max = 40,
                .tREH_min = 30,
                .tRHW_min = 200,
                .tRHZ_max = 200,
                .tRP_min = 50,
                .tWC_min = 100,
                .tWH_min = 30,
                .tWP_min = 50,
            },
    },

    {
        .type = NAND_SDR_IFACE,
        .timings.mode = 1,
        .timings.sdr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 35,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 50,
                .tREA_max = 30,
                .tREH_min = 15,
                .tRHW_min = 100,
                .tRHZ_max = 100,
                .tRP_min = 25,
                .tWC_min = 45,
                .tWH_min = 15,
                .tWP_min = 25,
            },
    },

    {
        .type = NAND_SDR_IFACE,
        .timings.mode = 2,
        .timings.sdr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 25,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 35,
                .tREA_max = 25,
                .tREH_min = 15,
                .tRHW_min = 100,
                .tRHZ_max = 100,
                .tRP_min = 17,
                .tWC_min = 35,
                .tWH_min = 15,
                .tWP_min = 17,
            },
    },

    {
        .type = NAND_SDR_IFACE,
        .timings.mode = 3,
        .timings.sdr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 25,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 30,
                .tREA_max = 20,
                .tREH_min = 10,
                .tRHW_min = 100,
                .tRHZ_max = 100,
                .tRP_min = 15,
                .tWC_min = 30,
                .tWH_min = 10,
                .tWP_min = 15,
            },
    },

    {
        .type = NAND_SDR_IFACE,
        .timings.mode = 4,
        .timings.sdr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 20,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 25,
                .tREA_max = 20,
                .tREH_min = 10,
                .tRHW_min = 100,
                .tRHZ_max = 100,
                .tRP_min = 12,
                .tWC_min = 25,
                .tWH_min = 10,
                .tWP_min = 12,
            },
    },

    {
        .type = NAND_SDR_IFACE,
        .timings.mode = 5,
        .timings.sdr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 15,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 20,
                .tREA_max = 16,
                .tREH_min = 7,
                .tRHW_min = 100,
                .tRHZ_max = 100,
                .tRP_min = 10,
                .tWC_min = 20,
                .tWH_min = 7,
                .tWP_min = 10,
            },
    },
};

static struct nand_interface_config nvddr_interface_config[] = {
    {
        .type = NAND_NVDDR_IFACE,
        .timings.mode = 0,
        .timings.nvddr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 35,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tCK_avg = 50,
                .tCAD_min = 45,
                .tDQSCK_max = 25,
                .tDQSCK_min = 3,
                .tRHW_min = 100,
                .tWRCK_min = 20,
                .tDQSD_max = 18,
                .tDQSHZ_max = 20,
            },
    },

    {
        .type = NAND_NVDDR_IFACE,
        .timings.mode = 1,
        .timings.nvddr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 25,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tCK_avg = 30,
                .tCAD_min = 45,
                .tDQSCK_max = 25,
                .tDQSCK_min = 3,
                .tRHW_min = 100,
                .tWRCK_min = 20,
                .tDQSD_max = 18,
                .tDQSHZ_max = 20,
            },
    },

    {
        .type = NAND_NVDDR_IFACE,
        .timings.mode = 2,
        .timings.nvddr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 15,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tCK_avg = 20,
                .tCAD_min = 45,
                .tDQSCK_max = 25,
                .tDQSCK_min = 3,
                .tRHW_min = 100,
                .tWRCK_min = 20,
                .tDQSD_max = 18,
                .tDQSHZ_max = 20,
            },
    },

    {
        .type = NAND_NVDDR_IFACE,
        .timings.mode = 3,
        .timings.nvddr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 15,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tCK_avg = 15,
                .tCAD_min = 45,
                .tDQSCK_max = 25,
                .tDQSCK_min = 3,
                .tRHW_min = 100,
                .tWRCK_min = 20,
                .tDQSD_max = 18,
                .tDQSHZ_max = 20,
            },
    },

    {
        .type = NAND_NVDDR_IFACE,
        .timings.mode = 4,
        .timings.nvddr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 15,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tCK_avg = 12,
                .tCAD_min = 45,
                .tDQSCK_max = 25,
                .tDQSCK_min = 3,
                .tRHW_min = 100,
                .tWRCK_min = 20,
                .tDQSD_max = 18,
                .tDQSHZ_max = 20,
            },
    },

    {
        .type = NAND_NVDDR_IFACE,
        .timings.mode = 5,
        .timings.nvddr =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 15,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tCK_avg = 10,
                .tCAD_min = 45,
                .tDQSCK_max = 25,
                .tDQSCK_min = 3,
                .tRHW_min = 100,
                .tWRCK_min = 20,
                .tDQSD_max = 18,
                .tDQSHZ_max = 20,
            },
    },
};

static struct nand_interface_config nvddr23_interface_config[] = {
    {
        .type = NAND_NVDDR23_IFACE,
        .timings.mode = 0,
        .timings.nvddr23 =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 40,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 30,
            },
    },

    {
        .type = NAND_NVDDR23_IFACE,
        .timings.mode = 1,
        .timings.nvddr23 =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 40,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 25,
            },
    },

    {
        .type = NAND_NVDDR23_IFACE,
        .timings.mode = 2,
        .timings.nvddr23 =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 40,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 15,
            },
    },

    {
        .type = NAND_NVDDR23_IFACE,
        .timings.mode = 3,
        .timings.nvddr23 =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 40,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 12,
            },
    },

    {
        .type = NAND_NVDDR23_IFACE,
        .timings.mode = 4,
        .timings.nvddr23 =
            {
                .tADL_min = 400,
                .tCEH_min = 20,
                .tCS_min = 40,
                .tFEAT_max = 1000,
                .tITC_max = 1000,
                .tRR_min = 20,
                .tWHR_min = 80,
                .tWB_max = 100,
                .tWW_min = 100,
                .tRC_min = 10,
            },
    },
};

static int nand_cleanup(FNandPsu_T *nfcPtr);
static int nand_extid_detect(FNandPsu_T *nfcPtr, int cs);
static u16 nand_onfi_crc16(u16 crc, const u8 *p, unsigned int len);
static int nand_onfi_detect(FNandPsu_T *nfcPtr, int cs);

/************************** Nand Interface************************************/
int FNandPsu_ChooseBestIface (FNandPsu_T *nfcPtr, u8 mask,
                              struct nand_interface_config *ext_iface)
{
    struct nand_device *device;
    struct nand_interface_config iface;
    int ntimings, i, idx;
    float clk_period;

    device = CTRL_TO_NAND(nfcPtr);

    if (ext_iface)
    {
        device->best_iface = ext_iface;
        return 0;
    }

    clk_period = (float)1000000000 / nfcPtr->config.clock_hz;

    // NV-DDR2/3
    if ((device->feat.features & NAND_SUPPORT_NVDDR3) && (mask & 0x8))
    {
        iface.type = NAND_NVDDR23_IFACE;
        iface.timings.mode = fls(device->feat.nvddr3_mode);

        ntimings = sizeof(nvddr23_interface_config) / sizeof(iface);
        for (i = 0; i < ntimings; i++)
        {
            if (nvddr23_interface_config[i].timings.mode <= iface.timings.mode)
            {
                if (clk_period >=
                    nvddr23_interface_config[i].timings.nvddr23.tRC_min)
                {
                    device->best_iface = &nvddr23_interface_config[i];
                    return 0;
                }
            }
        }
    }

    if ((device->feat.features & NAND_SUPPORT_NVDDR2) && (mask & 0x4))
    {
        iface.type = NAND_NVDDR23_IFACE;
        iface.timings.mode = fls(device->feat.nvddr2_mode);

        ntimings = sizeof(nvddr23_interface_config) / sizeof(iface);
        for (i = 0; i < ntimings; i++)
        {
            if (nvddr23_interface_config[i].timings.mode <= iface.timings.mode)
            {
                if (clk_period >=
                    nvddr23_interface_config[i].timings.nvddr23.tRC_min)
                {
                    device->best_iface = &nvddr23_interface_config[i];
                    return 0;
                }
            }
        }
    }

    /* NV-DDR mode */
    if ((device->feat.features & NAND_SUPPORT_NVDDR) && (mask & 0x2))
    {
        iface.type = NAND_NVDDR_IFACE;
        iface.timings.mode = fls(device->feat.nvddr_mode);

        ntimings = sizeof(nvddr_interface_config) / sizeof(iface);
        for (i = 0; i < ntimings; i++)
        {
            if (nvddr_interface_config[i].timings.mode <= iface.timings.mode)
            {
                if (nvddr_interface_config[i].timings.nvddr23.tRC_min <=
                    clk_period)
                {
                    device->best_iface = &nvddr_interface_config[i];
                    return 0;
                }
            }
        }
    }

    /* SDR mode is always supported */
    iface.type = NAND_SDR_IFACE;
    iface.timings.mode = fls(device->feat.sdr_mode);

    idx = -1;
    ntimings = sizeof(sdr_interface_config) / sizeof(iface);
    for (i = 0; i < ntimings; i++)
    {
        if (sdr_interface_config[i].timings.mode <= iface.timings.mode)
        {
            idx = i;
        }
    }
    if (idx != -1)
    {
        device->best_iface = &sdr_interface_config[idx];
    }

    return 0;
}

int FNandPsu_ResetInterface (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->setup_interface) )
    {
        return FMSH_ENOSYS;
    }

#if (NAND_CONFIG_IFACE_SUPP == 0x8)
    device->cur_iface = &nvddr23_interface_config[0];
    ret = ctrl_ops->setup_interface(nfcPtr, device->cur_iface);
#else
    /* reset controller to sdr mode before nand_reset_op */
    device->cur_iface = &sdr_interface_config[0];
    ret = ctrl_ops->setup_interface(nfcPtr, device->cur_iface);
#endif

    return ret;
}

int FNandPsu_SetupInterface (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;
    u8 dataout[4] = {0}, datain[4] = {0};
    int i;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->setup_interface) )
    {
        return FMSH_ENOSYS;
    }

    if (!device->best_iface)
    {
        return FMSH_SUCCESS;
    }

    /* Change the mode on the flash side */
    dataout[0] = device->best_iface->timings.mode & 0xf;
    dataout[0] |= (device->best_iface->type << 4);

    /* Setup best interface for all die*/
    for (i = 0; i < device->model.ntargets; i++)
    {
        ret = FNandPsu_SetFeature_Op(nfcPtr, i, 0x01, dataout);
        if (ret)
        {
            goto err_reset;
        }
    }

    /* Change the mode on the controller side */
    ret = ctrl_ops->setup_interface(nfcPtr, device->best_iface);
    if (ret)
    {
        goto err_reset;
    }

    /* Check if change timing mode is accepted by chip */
    for (i = 0; i < device->model.ntargets; i++)
    {
        ret = FNandPsu_GetFeature_Op(nfcPtr, i, 0x01, datain);
        if (ret || (datain[0] != dataout[0]))
        {
            goto err_reset;
        }
    }

    device->cur_iface = device->best_iface;
    return FMSH_SUCCESS;

err_reset:
    (void)FNandPsu_ResetInterface(nfcPtr);
    for (i = 0; i < device->model.ntargets; i++)
    {
        (void)FNandPsu_Reset_Op(nfcPtr, i);
    }

    return ret;
}

/************************** Nand Operation************************************/
int FNandPsu_Reset_Op (FNandPsu_T *nfcPtr, int cs)
{
    struct nand_ctrl_ops *ctrl_ops;

    FMSH_ASSERT(nfcPtr != NULL);

    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->reset) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->reset)
    {
        return ctrl_ops->reset(nfcPtr, cs);
    }

    return FMSH_SUCCESS;
}

int FNandPsu_ReadId_Op (FNandPsu_T *nfcPtr, int cs, u8 addr, void *buf,
                        unsigned int len)
{
    int ret;
    int i;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;
    u8 *id = buf;
    u8 ddrbuf[16];

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->exec_op) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->exec_op)
    {
        struct nand_instr instr[2];
        struct nand_operation ops;
        struct nand_interface_config *interface = device->cur_iface;

        instr[0].instr_type = RDID_INSTR;
        instr[0].ctx.addr.naddrs = 1;
        instr[0].ctx.addr.addrs = &addr;

        instr[1].instr_type = DATAIN_INSTR;
        instr[1].ctx.data.len = len;
        instr[1].ctx.data.buf = id;
        instr[1].ctx.data.force_8bit = 1;
        instr[1].ctx.data.raw = 1;
        instr[1].ctx.data.options = 0;
        instr[1].delay_ns = 0;

        /* READ_ID data bytes are received twice in NV-DDR mode */
        if (len && NAND_IS_DDR_IFACE(interface))
        {
            instr[1].ctx.data.len = len << 1;
            instr[1].ctx.data.buf = ddrbuf;
        }

        ops.cs = cs;
        ops.instr = &instr[0];
        ops.ninstr = 2;

        if (!len)
        {
            ops.ninstr--;
        }

        ret = ctrl_ops->exec_op(nfcPtr, &ops);
        if (!ret && len && NAND_IS_DDR_IFACE(interface))
        {
            for (i = 0; i < len; i++)
            {
                id[i] = ddrbuf[i << 1];
            }
        }
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_ParamPage_Op (FNandPsu_T *nfcPtr, int cs, u8 page, void *buf,
                           unsigned int len)
{
    int ret;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->exec_op) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->exec_op)
    {
        struct nand_instr instr[3];
        struct nand_operation ops;
        struct nand_interface_config *interface = device->cur_iface;

        instr[0].instr_type = READ_PARAPAGE_INSTR;
        instr[0].ctx.addr.naddrs = 1;
        instr[0].ctx.addr.addrs = &page;

        instr[1].instr_type = WAITRDY_INSTR;
        instr[1].ctx.waitrdy.timeout_us = device->feat.tR_max;
        instr[1].delay_ns = (unsigned int)NAND_TIMING_NS(interface, tRR_min);

        instr[2].instr_type = DATAIN_INSTR;
        instr[2].ctx.data.len = len;
        instr[2].ctx.data.buf = buf;
        instr[2].ctx.data.force_8bit = 1;
        instr[2].ctx.data.raw = 1;
        instr[2].ctx.data.options = 0;
        instr[2].delay_ns = 0;

        ops.cs = cs;
        ops.instr = &instr[0];
        ops.ninstr = 3;

        ret = ctrl_ops->exec_op(nfcPtr, &ops);
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_SetFeature_Op (FNandPsu_T *nfcPtr, int cs, u8 feature, void *data)
{
    struct nand_ctrl_ops *ctrl_ops;

    FMSH_ASSERT(nfcPtr != NULL);

    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->set_feature) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->set_feature)
    {
        return ctrl_ops->set_feature(nfcPtr, cs, feature, data);
    }

    return FMSH_SUCCESS;
}

int FNandPsu_GetFeature_Op (FNandPsu_T *nfcPtr, int cs, u8 feature, void *data)
{
    int ret;
    int i;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;
    u8 *buf = data;
    u8 ddrbuf[8];

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;

    if (!ctrl_ops || !ctrl_ops->exec_op)
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->exec_op)
    {
        struct nand_instr instr[4];
        struct nand_operation ops;
        struct nand_interface_config *interface = device->cur_iface;

        instr[0].instr_type = GET_FEAT_INSTR;
        instr[0].ctx.addr.naddrs = 1;
        instr[0].ctx.addr.addrs = &feature;

        instr[1].instr_type = WAITRDY_INSTR;
        instr[1].ctx.waitrdy.timeout_us = (unsigned int)NAND_TIMING_US(
            interface, tFEAT_max);
        instr[1].delay_ns = (unsigned int)NAND_TIMING_NS(interface, tRR_min);

        instr[2].instr_type = DATAIN_INSTR;
        instr[2].ctx.data.len = 4;
        instr[2].ctx.data.buf = buf;
        instr[2].ctx.data.force_8bit = 1;
        instr[2].ctx.data.raw = 1;
        instr[2].ctx.data.options = 0;
        instr[2].delay_ns = 0;

        if (NAND_IS_DDR_IFACE(interface))
        {
            instr[2].ctx.data.len = 8;
            instr[2].ctx.data.buf = ddrbuf;
        }

        ops.cs = cs;
        ops.instr = &instr[0];
        ops.ninstr = 3;

        ret = ctrl_ops->exec_op(nfcPtr, &ops);
        if (NAND_IS_DDR_IFACE(interface))
        {
            for (i = 0; i < 4; i++)
            {
                buf[i] = ddrbuf[i << 1];
            }
        }
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_Status_Op (FNandPsu_T *nfcPtr, int cs, u8 *status)
{
    int ret;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;
    u8 ddrbuf[2];

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->exec_op) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->exec_op)
    {
        struct nand_instr instr[2];
        struct nand_operation ops;
        struct nand_interface_config *interface = device->cur_iface;

        instr[0].instr_type = CMD_INSTR;
        instr[0].ctx.cmd.opcode = NAND_CMD_STATUS;
        instr[0].delay_ns = (unsigned int)NAND_TIMING_NS(interface, tADL_min);

        instr[1].instr_type = DATAIN_INSTR;
        instr[1].ctx.data.len = 1;
        instr[1].ctx.data.buf = &status[0];
        instr[1].ctx.data.force_8bit = 1;
        instr[1].ctx.data.raw = 1;
        instr[1].ctx.data.options = 0;
        instr[1].delay_ns = 0;

        if (NAND_IS_DDR_IFACE(interface))
        {
            instr[1].ctx.data.len = 2;
            instr[1].ctx.data.buf = ddrbuf;
        }

        ops.cs = cs;
        ops.instr = &instr[0];
        ops.ninstr = 2;

        ret = ctrl_ops->exec_op(nfcPtr, &ops);
        if (NAND_IS_DDR_IFACE(interface))
        {
            *status = ddrbuf[0];
        }
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_Erase_Op (FNandPsu_T *nfcPtr, int cs, unsigned int eraseblock)
{
    int ret;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;
    struct nand_model *model;
    unsigned int page;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;
    model = &(device->model);
    page = NAND_BLOCK_TO_PAGE(eraseblock, device);

    if( (!ctrl_ops) || (!ctrl_ops->erase) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->erase)
    {
        ret = ctrl_ops->erase(nfcPtr, cs, page, model->blocksize);
        if (ret)
        {
            fmsh_print_err(
                "BlockErase operation failed at page: 0x%x, CMD_STATUS is "
                "0x%x.\r\n",
                page, nfcPtr->cmd_status);
            nfcPtr->fault_page = page;
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_WritePage_Op (FNandPsu_T *nfcPtr, int cs, unsigned int page,
                           unsigned int offset, void *buf, unsigned int len,
                           u8 raw)
{
    int ret;
    struct nand_ctrl_ops *ctrl_ops;

    FMSH_ASSERT(nfcPtr != NULL);

    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->write_page) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->write_page)
    {
        ret = ctrl_ops->write_page(nfcPtr, cs, page, offset, buf, len, raw);
        if (ret)
        {
            fmsh_print_err(
                "PageWrite operation failed at page: 0x%x, CMD_STATUS is "
                "0x%x.\r\n",
                page, nfcPtr->cmd_status);
            nfcPtr->fault_page = page;
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_ReadPage_Op (FNandPsu_T *nfcPtr, int cs, unsigned int page,
                          unsigned int offset, void *buf, unsigned int len,
                          u8 raw)
{
    int ret;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;
    struct nand_model *model;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;
    model = &(device->model);

    if( (!ctrl_ops) || ( (!ctrl_ops->read_page) && (!ctrl_ops->exec_op) ) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->read_page)
    {
        ret = ctrl_ops->read_page(nfcPtr, cs, page, offset, buf, len, raw);
        if (ret)
        {
            fmsh_print_err(
                "PageRead operation failed at page: 0x%x, CMD_STATUS is "
                "0x%x.\r\n",
                page, nfcPtr->cmd_status);
            nfcPtr->fault_page = page;
            return ret;
        }
    }
    else if (ctrl_ops->exec_op)
    {
        struct nand_instr instr[3];
        struct nand_operation ops;
        struct nand_interface_config *interface = device->cur_iface;

        if (model->io_width == 16)
        {
            offset = offset >> 1;
        }
        u8 addrs[6] = {offset,    offset >> 8, page,
                       page >> 8, page >> 16,  page >> 24};
        instr[0].instr_type = READ_INSTR;
        instr[0].ctx.addr.naddrs = device->feat.row_cycles + 2;
        instr[0].ctx.addr.addrs = &addrs[0];
        instr[0].delay_ns = 0;

        instr[1].instr_type = WAITRDY_INSTR;
        instr[1].ctx.waitrdy.timeout_us = device->feat.tR_max;
        instr[1].delay_ns = (unsigned int)NAND_TIMING_NS(interface, tRR_min);

        instr[2].instr_type = DATAIN_INSTR;
        instr[2].ctx.data.len = model->pagesize + model->oobsize;
        instr[2].ctx.data.buf = buf;
        instr[2].ctx.data.force_8bit = 0;
        instr[2].ctx.data.raw = raw;
        instr[2].ctx.data.options = 0;
        if (nfcPtr->usercfg->options & NAND_ECC_SCRAMBLER)
        {
            instr[2].ctx.data.options |= NAND_ECC_SCRAMBLER;
        }
        if (nfcPtr->usercfg->options & NAND_ERASED_DET)
        {
            instr[2].ctx.data.options |= NAND_ERASED_DET;
        }
        instr[2].delay_ns = 0;

        ops.cs = cs;
        ops.instr = &instr[0];
        ops.ninstr = 3;

        ret = ctrl_ops->exec_op(nfcPtr, &ops);
        if (ret)
        {
            fmsh_print_err(
                "PageRead operation failed at page: 0x%x, CMD_STATUS is "
                "0x%x.\r\n",
                page, nfcPtr->cmd_status);
            nfcPtr->fault_page = page;
            return ret;
        }
    }
    else{
        ;/* no deal with */
    }
    return FMSH_SUCCESS;
}

int FNandPsu_ChangeReadColumn_Op (FNandPsu_T *nfcPtr, int cs,
                                  unsigned int offset, void *buf,
                                  unsigned int len, int force_8bit)
{
    int ret;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    ctrl_ops = nfcPtr->ctrl_ops;

    if( (!ctrl_ops) || (!ctrl_ops->exec_op) )
    {
        return FMSH_ENOSYS;
    }

    if (ctrl_ops->exec_op)
    {
        struct nand_instr instr[5];
        struct nand_operation ops;

        instr[0].instr_type = CMD_INSTR;
        instr[0].ctx.cmd.opcode = NAND_CMD_READ2;
        instr[0].delay_ns = 0;

        if (nfcPtr->config.io_width == 16)
        {
            offset = offset >> 1;
        }
        u8 addrs[2] = {offset >> 8, offset};
        instr[1].instr_type = ADDR_INSTR;
        instr[1].ctx.addr.naddrs = 2;
        instr[1].ctx.addr.addrs = &addrs[0];
        instr[1].delay_ns = 0;

        instr[2].instr_type = CMD_INSTR;
        instr[2].ctx.cmd.opcode = NAND_CMD_RNDOUTSTART;
        instr[2].delay_ns = device->feat.tCCS_min;

        instr[3].instr_type = DATAIN_INSTR;
        instr[3].ctx.data.len = len;
        instr[3].ctx.data.buf = buf;
        instr[3].ctx.data.force_8bit = force_8bit;
        instr[3].ctx.data.raw = 1;
        instr[3].ctx.data.options = 0;
        instr[3].delay_ns = 0;

        ops.cs = cs;
        ops.instr = &instr[0];
        ops.ninstr = 4;

        ret = ctrl_ops->exec_op(nfcPtr, &ops);
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

/************************ Nand Rdwr ******************************************/
int FNandPsu_TranslateFlashAddress (FNandPsu_T *nfcPtr, u64 addr, u32 *cs,
                                    u32 *page, u32 *offset)
{
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_model *model = &(device->model);

    FMSH_ASSERT(nfcPtr != NULL);

    if (cs)
    {
        *cs = addr >> model->target_shift;
        addr &= ((u64)0x1 << model->target_shift) - 1;
    }
    if (page)
    {
        *page = addr >> model->page_shift;
    }
    if (offset)
    {
        *offset = addr & (model->pagesize - 1);
    }

    return FMSH_SUCCESS;
}

int FNandPsu_NoSkip_Erase (FNandPsu_T *nfcPtr, u64 addr, int len)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;
    unsigned int cs, page;
    unsigned int block;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    /* Remaining blocks to erase */
    while (len > 0)
    {
        /* calculate address */
        (void)FNandPsu_TranslateFlashAddress(nfcPtr, addr, &cs, &page, 0);
        block = NAND_PAGE_TO_BLOCK(page, device);
        /* erase nand flash */
        ret = FNandPsu_Erase_Op(nfcPtr, cs, block);
        if (ret)
        {
            return len;
        }
        addr += model->blocksize;
        len -= model->blocksize;
    }

    return 0;
}

int FNandPsu_NoSkip_Write (FNandPsu_T *nfcPtr, u64 addr, int len, u8 *buf,
                           u32 flags)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;
    unsigned int cs, page, offset, left, ooblen;
    int oob_required = 0, raw = 0;

    FMSH_ASSERT(nfcPtr != NULL);
    FMSH_ASSERT(buf != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    if (flags & NAND_OP_RAW)
    {
        raw = 1;
    }
    if (flags & NAND_OP_OOBREQ)
    {
        oob_required = 1;
    }

    while (len > 0)
    {
        /* calculate address */
        (void)FNandPsu_TranslateFlashAddress(nfcPtr, addr, &cs, &page, &offset);
        /* length of bytes to send to nandflash */
        left = model->pagesize - offset;
        if (len < left)
        {
            left = len;
        }
        ooblen = (device->ooblen > model->oobsize) ? model->oobsize
                                                   : device->ooblen;
        /* init & fill data buffer */
        if (left != model->pagesize)
        {
            (void)memset(device->data_buf, 0xff, model->pagesize);
        }
        (void)memcpy(device->data_buf, buf, left);
        (void)memset(device->oob_poi, 0xff, model->oobsize);
        if (oob_required && device->oob_buf)
        {
            (void)memcpy(device->oob_poi, device->oob_buf, ooblen);
        }

        /* write to nand flash */
        ret = FNandPsu_WritePage_Op(nfcPtr, cs, page, 0, device->data_buf,
                                    model->pagesize, raw);
        if (ret)
        {
            return len;
        }
        buf += left;
        addr += left;
        len -= left;

        if (oob_required)
        {
            device->oob_buf += ooblen;
            device->ooblen -= ooblen;
        }
    }

    return 0;
}

int FNandPsu_NoSkip_Read (FNandPsu_T *nfcPtr, u64 addr, int len, u8 *buf,
                          u32 flags)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;
    unsigned int cs, page, offset, left, ooblen;
    int oob_required = 0, raw = 0;

    FMSH_ASSERT(nfcPtr != NULL);
    FMSH_ASSERT(buf != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    if (flags & NAND_OP_RAW)
    {
        raw = 1;
    }
    if (flags & NAND_OP_OOBREQ)
    {
        oob_required = 1;
    }

    while (len > 0)
    {
        /* calculate address */
        (void)FNandPsu_TranslateFlashAddress(nfcPtr, addr, &cs, &page, &offset);
        /* length of bytes to be send to nandflash */
        left = model->pagesize - offset;
        if (len < left)
        {
            left = len;
        }
        ooblen = (device->ooblen > model->oobsize) ? model->oobsize
                                                   : device->ooblen;
        /* read from nand flash */
        ret = FNandPsu_ReadPage_Op(nfcPtr, cs, page, 0, device->data_buf,
                                   model->pagesize, raw);
        if (ret)
        {
            return len;
        }
        /* fill data buffer */
        (void)memcpy(buf, device->data_buf + offset, left);
        if (oob_required && device->oob_buf)
        {
            (void)memcpy(device->oob_buf, device->oob_poi, ooblen);
        }

        buf += left;
        addr += left;
        len -= left;

        if (oob_required)
        {
            device->oob_buf += ooblen;
            device->ooblen -= ooblen;
        }
    }

    return 0;
}

int FNandPsu_NoSkip_WriteOob (FNandPsu_T *nfcPtr, u32 page, int len, u8 *buf,
                              int raw)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;
    u64 addr;
    int ooblen, cs;

    FMSH_ASSERT(nfcPtr != NULL);
    FMSH_ASSERT(buf != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    addr = NAND_PAGE_TO_ADDR(page, device);

    /* Set oob_buf and ooblen */
    (void)FNandPsu_SetOobBuf(nfcPtr, buf, len);

    while (device->ooblen > 0)
    {
        /* calculate address */
        (void)FNandPsu_TranslateFlashAddress(nfcPtr, addr, (u32 *)&cs, &page, 0);
        /* length of bytes to be send to nandflash */
        ooblen = (device->ooblen > model->oobsize) ? model->oobsize
                                                   : device->ooblen;
        /* init & fill data buffer */
        (void)memset(device->data_buf, 0xff, model->pagesize + model->oobsize);
        (void)memcpy(device->oob_poi, device->oob_buf, ooblen);
        /* write to nand flash */
        ret = FNandPsu_WritePage_Op(nfcPtr, cs, page, 0, device->data_buf,
                                    model->pagesize, raw);
        if (ret)
        {
            return device->ooblen;
        }
        device->oob_buf += ooblen;
        device->ooblen -= ooblen;
        addr += model->pagesize;
    }

    return 0;
}

int FNandPsu_NoSkip_ReadOob (FNandPsu_T *nfcPtr, u32 page, int len, u8 *buf,
                             int raw)
{
    int ret;
    struct nand_device *device = CTRL_TO_NAND(nfcPtr);
    struct nand_model *model = &(device->model);
    u64 addr;
    int ooblen, cs;

    FMSH_ASSERT(nfcPtr != NULL);
    FMSH_ASSERT(buf != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    addr = NAND_PAGE_TO_ADDR(page, device);

    /* Set oob_buf and ooblen */
    (void)FNandPsu_SetOobBuf(nfcPtr, buf, len);

    while (device->ooblen > 0)
    {
        /* calculate address */
        (void)FNandPsu_TranslateFlashAddress(nfcPtr, addr, (u32 *)&cs, &page, 0);
        ooblen = (device->ooblen > model->oobsize) ? model->oobsize
                                                   : device->ooblen;
        /* read from nand flash */
        ret = FNandPsu_ReadPage_Op(nfcPtr, cs, page, 0, device->data_buf,
                                   model->pagesize, raw);
        if (ret)
        {
            return device->ooblen;
        }
        (void)memcpy(device->oob_buf, device->oob_poi, ooblen);
        device->oob_buf += ooblen;
        device->ooblen -= ooblen;
        addr += model->pagesize;
    }

    return 0;
}

/********************* Nand Initialize ***************************************/
static int nand_cleanup (FNandPsu_T *nfcPtr)
{
    struct nand_device *device;

    device = CTRL_TO_NAND(nfcPtr);

    if (device->data_buf)
    {
        free(device->data_buf);
        device->data_buf = 0;
    }

    if (device->bb_info)
    {
        free(device->bb_info);
        device->bb_info = 0;
    }

    return FMSH_SUCCESS;
}

static int nand_extid_detect (FNandPsu_T *nfcPtr, int cs)
{
    struct nand_device *device;
    struct nand_model *model;
    struct nand_feature *feat;
    u8 id3, id4, id5;
    int lun_num_shift, plane_num_shift, plane_shift;

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);
    feat = &(device->feat);

    id3 = model->ids[0];
    id4 = model->ids[1];
    id5 = model->ids[2];

    /* LUN number */
    switch (id3 & 0x03)
    {
    case 0x00:
        model->lun_num = 1;
        break;
    case 0x01:
        model->lun_num = 2;
        break;
    case 0x02:
        model->lun_num = 4;
        break;
    case 0x03:
        model->lun_num = 8;
        break;
    default:
        break;
    }
    lun_num_shift = id3 & 0x03;
    /* Cell Type */
    switch (id3 & 0x0c)
    {
    case 0x00:
        feat->bits_per_cell = 1;
        break;
    case 0x04:
        feat->bits_per_cell = 2;
        break;
    case 0x08:
        feat->bits_per_cell = 3;
        break;
    case 0x0c:
        feat->bits_per_cell = 4;
        break;
    default:
        break;
    }
    // Page Size
    switch (id4 & 0x03)
    {
    case 0x00:
        model->pagesize = 1024;
        model->page_shift = 10;
        break;
    case 0x01:
        model->pagesize = 2048;
        model->page_shift = 11;
        break;
    case 0x02:
        model->pagesize = 4096;
        model->page_shift = 12;
        break;
    case 0x03:
        model->pagesize = 8192;
        model->page_shift = 13;
        break;
     default:   
        break;
    }
    // OOB Size
    switch (id4 & 0x04)
    {
    case 0x00:
        model->oobsize = model->pagesize >> 6;
        break;
    case 0x04:
        model->oobsize = model->pagesize >> 5;
        break;
    default:
        break;
    }
    // Block Size
    switch (id4 & 0x30)
    {
    case 0x00:
        model->blocksize = 0x10000;
        model->erase_shift = 16;
        break;
    case 0x10:
        model->blocksize = 0x20000;
        model->erase_shift = 17;
        break;
    case 0x20:
        model->blocksize = 0x40000;
        model->erase_shift = 18;
        break;
    case 0x30:
        model->blocksize = 0x80000;
        model->erase_shift = 19;
        break;
    default:
        break;
    }
    // IO width
    switch (id4 & 0x40)
    {
    case 0x00:
        model->io_width = 8;
        break;
    case 0x40:
        model->io_width = 16;
        break;
    default:
        break;
    }
    // Plane number
    plane_num_shift = (id5 & 0x0c) >> 2;
    // Plane size
    switch (id5 & 0x70)
    {
    case 0x00:
        plane_shift = 23;
        break;
    case 0x10:
        plane_shift = 24;
        break;
    case 0x20:
        plane_shift = 25;
        break;
    case 0x30:
        plane_shift = 26;
        break;
    case 0x40:
        plane_shift = 27;
        break;
    case 0x50:
        plane_shift = 28;
        break;
    case 0x60:
        plane_shift = 29;
        break;
    case 0x70:
        plane_shift = 30;
        break;
    default:
        break;
    }

    model->subpagesize = 0;
    model->suboobsize = 0;
    model->target_shift = plane_shift + plane_num_shift;
    model->lun_shift = model->target_shift - lun_num_shift;
    model->block_num = 0x1 << (model->target_shift - model->erase_shift);
    model->page_num = 0x1 << (model->erase_shift - model->page_shift);

    feat->revisions = 0;
    feat->features = 0;
    if (model->io_width == 16)
    {
        feat->features |= NAND_SUPPORT_WIDTH16;
    }
    feat->opt_cmds = 0;
    feat->adv_cmds = 0;
    feat->sdr_mode = 0x1f;
    feat->nvddr_mode = 0;
    feat->nvddr2_mode = 0;
    feat->nvddr3_mode = 0;
    device->feat.tRST_max = NAND_TRST_MAX;
    device->feat.tBRES_max = NAND_TBERS_MAX;
    device->feat.tPROG_max = NAND_TPROG_MAX;
    device->feat.tR_max = NAND_TR_MAX;
    device->feat.tCCS_min = NAND_TCCS_MIN;
    feat->wr_warmup = 0;
    feat->rd_warmup = 0;
    feat->ecc_corr_str = 0;
    feat->row_cycles = 3;

    return FMSH_SUCCESS;
}

static u16 nand_onfi_crc16 (u16 crc, const u8 *p, unsigned int len)
{
    int i;
    while (len--)
    {
        crc ^= *p++ << 8;
        for (i = 0; i < 8; i++)
        {
            crc = (crc << 1) ^ ((crc & 0x8000) ? 0x8005 : 0);
        }
    }

    return crc;
}

static int nand_onfi_detect (FNandPsu_T *nfcPtr, int cs)
{
    int ret;
    int i;
    struct nand_device *device;
    struct nand_model *model;
    struct nand_feature *feat;
    char id[4];
    struct nand_parapage pbuf;
    u32 offset;
    u16 crc;

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);
    feat = &(device->feat);

    /* Set Default value */
    feat->tR_max = NAND_TR_MAX;
    feat->tCCS_min = NAND_TCCS_MIN;

    /* Read ONFI id */
    ret = FNandPsu_ReadId_Op(nfcPtr, cs, 0x20, id, 4);
    /* return <0 if onfi not supported */
    if (ret || strncmp(id, "ONFI", 4))
    {
        return FMSH_EIO;
    }

    /* Read parameter page and check crc */
    for (i = 0; i < ONFI_PARAM_PAGES; i++)
    {
        if (i == 0)
        {
            ret = FNandPsu_ParamPage_Op(nfcPtr, cs, 0, &pbuf, sizeof(pbuf));
        }
        else
        {
            offset = i * sizeof(pbuf);
            ret = FNandPsu_ChangeReadColumn_Op(nfcPtr, cs, offset, &pbuf,
                                               sizeof(pbuf), 1);
        }
        if (ret)
        {
            return ret;
        }

        crc = nand_onfi_crc16(ONFI_CRC_BASE, (u8 *)&pbuf, 254);
        if (crc == pbuf.Crc)
        {
            break;
        }
    }
    /* No valid parameter page found */
    if (i == ONFI_PARAM_PAGES)
    {
        return FMSH_EIO;
    }

    device->options |= NAND_SUPPORT_ONFI;
    /* Set ONFI parameters */
    feat->revisions = pbuf.Revision;
    feat->features = pbuf.Features;
    feat->opt_cmds = pbuf.OptionalCmds;
    feat->adv_cmds = pbuf.AdvancedCmds;
    feat->sdr_mode = pbuf.TimingMode;
    feat->nvddr_mode = pbuf.SynTimingMode;
    feat->nvddr2_mode = ((u16)pbuf.Nvddr2TimingMode2 << 8) |
                        pbuf.Nvddr2TimingMode1;
    feat->nvddr3_mode = pbuf.Nvddr3TimingMode;
    feat->tBRES_max = pbuf.TBers;
    feat->tPROG_max = pbuf.TProg;
    feat->tR_max = pbuf.TR;
    feat->tCCS_min = pbuf.TCcs;
    feat->wr_warmup = pbuf.Nvddr3Warmup & 0xf;
    feat->rd_warmup = (pbuf.Nvddr3Warmup >> 4) & 0xf;
    feat->ecc_corr_str = pbuf.EccBits;
    feat->bits_per_cell = pbuf.BitsPerCell;
    feat->row_cycles = pbuf.AddrCycles & 0xf;

    feat->tRST_max = NAND_TRST_MAX;

    /* Parse parameter page */
    model->pagesize = pbuf.BytesPerPage;
    model->oobsize = pbuf.SpareBytesPerPage;
    model->subpagesize = pbuf.BytesPerPartialPage;
    model->suboobsize = pbuf.SpareBytesPerPartialPage;
    model->blocksize = pbuf.PagesPerBlock * pbuf.BytesPerPage;
    model->page_num = pbuf.PagesPerBlock;
    model->block_num = pbuf.BlocksPerLun;
    model->lun_num = pbuf.NumLuns;
    model->page_shift = fls(model->pagesize);
    model->erase_shift = fls(model->blocksize);
    model->lun_shift = model->erase_shift + fls(model->block_num);
    model->target_shift = model->lun_shift + fls(model->lun_num);

    if (device->options & NAND_BUSWIDTH_AUTO)
    {
        if (feat->features & NAND_SUPPORT_WIDTH16)
        {
            model->io_width = 16;
        }
        else
        {
            model->io_width = 8;
        }
    }
    else
    {
        model->io_width = nfcPtr->config.io_width;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_Detect (FNandPsu_T *nfcPtr, int cs)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;
    u8 id[8];
    u8 maf_id, dev_id;

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    /* Reset chip after power-up */
    (void)FNandPsu_ResetInterface(nfcPtr);
    ret = FNandPsu_Reset_Op(nfcPtr, cs);
    if (ret)
    {
        return ret;
    }

    /* Send the command for reading device ID */
    ret = FNandPsu_ReadId_Op(nfcPtr, cs, 0x00, id, 2);
    if (ret)
    {
        return ret;
    }
    maf_id = id[0];
    dev_id = id[1];

    /*
     * Try again to make sure, as some systems the bus-hold or other
     * interface concerns can cause random data which looks like a
     * possibly credible NAND flash to appear. If the two results do
     * not match, ignore the device completely.
     */
    ret = FNandPsu_ReadId_Op(nfcPtr, cs, 0x00, id, sizeof(id));
    if (ret)
    {
        return ret;
    }
    if ((id[0] != maf_id) || (id[1] != dev_id))
    {
        return FMSH_ENXIO;
    }

    model->manufacture = id[0];
    model->device_id = id[1];
    model->ids[0] = id[2];
    model->ids[1] = id[3];
    model->ids[2] = id[4];
    model->ids[3] = id[5];
    /* Check if the chip is ONFI compliant */
    ret = nand_onfi_detect(nfcPtr, cs);
    if (ret == 0)
    {
        return FMSH_SUCCESS;
    }

    ret = nand_extid_detect(nfcPtr, cs);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_Scan (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    /* Init nand_device */
    (void)memset(device, 0, sizeof(struct nand_device));
    device->ctrl = nfcPtr;
    device->options = 0;
    device->bbt_options = nfcPtr->usercfg->dev_bbt_options;

    model->ntargets = 1;

    /* Get nand info from first chip */
    ret = FNandPsu_Detect(nfcPtr, 0);
    if (ret)
    {
        return ret;
    }

#ifdef BOOT_SEARCH
    /* In boot code, only part of device is used */
    model->ntargets = 1;
    model->device_size = (unsigned long long)0x1 << model->target_shift;
#else
    int i;
    /* Check Chip Array */
    (void)FNandPsu_ResetInterface(nfcPtr);
    for (i = 0; i < NAND_MAX_CHIPS; i++)
    {
        u8 id[2];
        ret = FNandPsu_Reset_Op(nfcPtr, i);
        if (ret)
        {
            break;
        }
        ret = FNandPsu_ReadId_Op(nfcPtr, i, 0x00, id, 2);
        if (ret)
        {
            break;
        }
        if( (id[0] != model->manufacture) || (id[1] != model->device_id) )
        {
            break;
        }
    }
    model->ntargets = i;
    model->device_size = i * ((unsigned long long)0x1 << model->target_shift);
#endif /* BOOT_SEARCH */

    /* Allocate buffer */
    device->data_buf = (u8 *)malloc(model->pagesize + model->oobsize);
    if (!device->data_buf)
    {
        fmsh_print_err("device->data_buf malloc failed, no enough space!\r\n");
        nand_cleanup(nfcPtr);
        return FMSH_ENOMEM;
    }
    device->oob_poi = device->data_buf + model->pagesize;

    return FMSH_SUCCESS;
}

int FNandPsu_ScanTail (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device;
    struct nand_model *model;
    u32 nblocks;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);
    model = &(device->model);

    FNandPsu_ChooseBestIface(nfcPtr, NAND_CONFIG_IFACE_SUPP, 0);
    FNandPsu_SetupInterface(nfcPtr);

    /* Create BBT */
    FNandPsu_InitBBT(nfcPtr);
    nblocks = model->device_size >> model->erase_shift;
    device->bb_info = (u8 *)malloc(nblocks >> 2);
    if (!device->bb_info)
    {
        fmsh_print_err("device->bb_info malloc failed, no enough space!\r\n");
        nand_cleanup(nfcPtr);
        return FMSH_ENOMEM;
    }

    /* return if do not need BBT in memory */
    if (device->bbt_options & NAND_SKIP_BBTSCAN)
    {
        u32 nblocks = device->model.device_size >> device->model.erase_shift;
        (void)memset(device->bb_info, 0xff, nblocks >> 2);
        return FMSH_SUCCESS;
    }

    ret = FNandPsu_ScanBBT(nfcPtr);
    if (ret)
    {
        fmsh_print_err("FNandPsu_ScanBBT failed!\r\n");
        nand_cleanup(nfcPtr);
        return ret;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_Device_Init (FNandPsu_T *nfcPtr, FNandPsu_UserCfg_T *usercfg)
{
    int ret;

    FNandPsu_HwInit(nfcPtr, usercfg);

    /* Detect devices */
    ret = FNandPsu_Scan(nfcPtr);
    if (ret)
    {
        return ret;
    }

    /* Complete controller initialize using detected devices information */
    ret = FNandPsu_HwInitr(nfcPtr);
    if (ret)
    {
        return ret;
    }

    /* fix: this code is added to support some devices */
    if ((nfcPtr->device->model.manufacture == 0xEC) &&
        ((nfcPtr->device->model.device_id == 0xD3) ||
         (nfcPtr->device->model.device_id == 0xD5) ||
         (nfcPtr->device->model.device_id == 0xDC)))
    {
        nfcPtr->usercfg->options |= NAND_BBM_WITHECC | NAND_NO_SKIP_BYTE;
        nfcPtr->ctrl_ops->read_page = 0;
    }

    /* Complete devices initialization and create bbt */
    ret = FNandPsu_ScanTail(nfcPtr);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}
