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
 * @file fmsh_xspi_nand.c
 * @addtogroup qspipsu_v1_0
 * @{
 *
 * Contains implements the xspi nand flash interface functions.
 * See fmsh_xspi_nand.h for a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- ----------  -----------------------------------------------
 * 1.00  hzq 2024/2/22  First release
 *s
 * </pre>
 *
 ******************************************************************************/
#include <string.h>

#include "fmsh_xspi_hw.h"
#include "fmsh_xspi.h"
#include "fmsh_xspi_nand.h"
/*****************************************************************************/
/******* global seq for profile1 *******/
#define QSPINAND_GLB_SEQ0_CFG     (0x18020BB)
#define QSPINAND_GLB_SEQ1_CFG     (0x6000040)

/******* reset sequence for profile1 *******/
#define QSPINAND_RST_SEQ0_RST     (0xFF66)
#define QSPINAND_RST_SEQ1_RST     (0xD0669900)

/******* erase sequence for profile1 *******/
#define QSPINAND_ERS_SEQ0_SE      (0x30D8)
#define QSPINAND_ERS_SEQ1_SE      (0x12)

/******* program sequence for profile1 *******/
#define QSPINAND_PROG_SEQ0_PP     (0x00002002)
#define QSPINAND_PROG_SEQ1_PP     (0x00001000)

#define QSPINAND_PROG_SEQ0_QPP    (0x00302032)
#define QSPINAND_PROG_SEQ1_QPP    (0x00001001)

/******* read sequence for profile1 *******/
#define QSPINAND_READ_SEQ0_READ   (0x08002003)
#define QSPINAND_READ_SEQ1_READ   (0x00001300)

#define QSPINAND_READ_SEQ0_DOR    (0x0810203B)
#define QSPINAND_READ_SEQ1_DOR    (0x00001300)

#define QSPINAND_READ_SEQ0_QOR    (0x0830206B)
#define QSPINAND_READ_SEQ1_QOR    (0x00001300)

#define QSPINAND_READ_SEQ0_DIOR   (0x041120BB)
#define QSPINAND_READ_SEQ1_DIOR   (0x00001300)

#define QSPINAND_READ_SEQ0_QIOR   (0x023320EB)
#define QSPINAND_READ_SEQ1_QIOR   (0x00001300)

/******* wel sequence for profile1 *******/
#define QSPINAND_WE_SEQ0          (0x01000006)

/******* STATUS sequence for profile1 *******/
#define QSPINAND_STAST_SEQ0       (0x00000000)
#define QSPINAND_STAST_SEQ1       (0x40400040)
#define QSPINAND_STAST_SEQ2       (0x0F000F0F)
#define QSPINAND_STAST_SEQ3       (0x00000000)
#define QSPINAND_STAST_SEQ5       (0x53005220)  // 0x53005200 right //

#define QSPINAND_STAST_SEQ7       (0x000000C0)  // status address
#define QSPINAND_STAST_SEQ8       (0x000000C0)
#define QSPINAND_STAST_SEQ9       (0x000000C0)

#define QSPINAND_FMSH_STAST_SEQ10 (0x80003030)
#define QSPINAND_GD_STAST_SEQ10   (0x0)

/*****************************************************************************/
static __no_init struct qspi_nand qspi_nand_dev;

static struct qspi_nand_param nand_param_default = {
    .csda_ns = QSPINAND_CSDA_NS,
    .cseot_ns = QSPINAND_CSEOT_NS,
    .cssot_ns = QSPINAND_CSSOT_NS,

    .trst_max_ms = QSPINAND_TIMING_TRST_MS,
    .tpp_max_us = QSPINAND_TIMING_TPP_US,
    .tse_max_ms = QSPINAND_TIMING_TSE_MS,
};

static struct qspi_usercfg qspi_nand_cfg_default __attribute__((aligned(4))) = {
    .flags = 0,                 // QSPI_F_INTR_EN,
    .ers_mode = QSPI_ERS_SE,    // standard sector erase
    .prog_mode = QSPI_PROG_PP,  // x1 program
    .read_mode = QSPI_RD_READ,  // x1 read
};

int FQspiPsu_Nand_Init (FQspiPsu_T* qspiPtr, struct qspi_usercfg* usercfg)
{
    int ret;
    struct qspi_usercfg* cfg;
    u8 id[8];

    qspiPtr->type = QSPI_TYPE_NAND;

    // config qspi usercfg
    if (usercfg)
    {
        qspiPtr->usercfg = usercfg;
    }
    else
    {
        qspiPtr->usercfg = &qspi_nand_cfg_default;
    }
    cfg = qspiPtr->usercfg;

    FQspiPsu_InitHw(qspiPtr, cfg);

    // get maker & device size
    ret = FQspiPsu_Nand_ReadId(qspiPtr, id);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_GetFlashInfo(qspiPtr, id);
    if (ret)
    {
        return ret;
    }

    // set transfer mode
    ret = FQspiPsu_Nand_SetFlashMode(qspiPtr);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_Reset(qspiPtr);
    if (ret)
    {
        return ret;
    }

    // set baud rate & data capture delay
    FQspiPsu_Delay(qspiPtr, qspiPtr->config.sclk_hz, QSPINAND_CSDA_NS,
                   QSPINAND_CSEOT_NS, QSPINAND_CSSOT_NS);

    qspiPtr->priv = (void*)&qspi_nand_dev;

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_GetFlashInfo (FQspiPsu_T* qspiPtr, u8* id)
{
    qspi_nand_dev.flags = 0;
    qspi_nand_dev.param = &nand_param_default;

    // flash make
    switch (id[0])
    {
    case QSPI_MAKER_ID_MICRON:
    case QSPI_MAKER_ID_SPANSION:
    case QSPI_MAKER_ID_WINBOND:
    case QSPI_MAKER_ID_MACRONIX:
    case QSPI_MAKER_ID_ISSI:
    case QSPI_MAKER_ID_FMSH:
    case QSPI_MAKER_ID_GD:
        qspiPtr->maker = id[0];
        break;
    default:
        qspiPtr->maker = QSPI_MAKER_ID_UNKNOWN;
        break;
    }

    // device size(2Gb)

    qspiPtr->dev_size = 0x1 << 28;   // 2Gb
    qspiPtr->blk_size = 0x1 << 17;   // 128K as default
    qspiPtr->page_size = 0x1 << 11;  // 2K

    qspi_nand_dev.blk_shift = 17;
    qspi_nand_dev.page_shift = 11;
    qspi_nand_dev.oob_size = 64;

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_SetFlashMode (FQspiPsu_T* qspiPtr)
{
    u32 stast10;

    FQspiPsu_Nand_ChangeReadMode(qspiPtr, qspiPtr->usercfg->read_mode);

    FQspiPsu_Nand_ChangeProgMode(qspiPtr, qspiPtr->usercfg->prog_mode);

    FQspiPsu_Nand_ChangeErsMode(qspiPtr);

    // global seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_GLOBAL_SEQ_CFG,
                  QSPINAND_GLB_SEQ0_CFG);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_GLOBAL_SEQ_CFG_1,
                  QSPINAND_GLB_SEQ1_CFG);

    // direct access cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_RMP,
                  qspiPtr->config.data_base);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_RMP_1, 0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_CFG,
                  QSPI_DAC_BANK_NUM(qspiPtr->cur_cs) | QSPI_DAC_RMP_ADDR_EN);

    // rst seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_RST_SEQ_CFG_0,
                  QSPINAND_RST_SEQ0_RST);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_RST_SEQ_CFG_1,
                  QSPINAND_RST_SEQ1_RST);

    // we seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_WE_SEQ_CFG_0, QSPINAND_WE_SEQ0);

    // stat seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_0,
                  QSPINAND_STAST_SEQ0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_1,
                  QSPINAND_STAST_SEQ1);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_2,
                  QSPINAND_STAST_SEQ2);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_3,
                  QSPINAND_STAST_SEQ3);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_5,
                  QSPINAND_STAST_SEQ5);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_7,
                  QSPINAND_STAST_SEQ7);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_8,
                  QSPINAND_STAST_SEQ8);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_9,
                  QSPINAND_STAST_SEQ9);
    switch (qspiPtr->maker)
    {
    case QSPI_MAKER_ID_GD:
        stast10 = QSPINAND_GD_STAST_SEQ10;
        break;
    case QSPI_MAKER_ID_FMSH:
        stast10 = QSPINAND_FMSH_STAST_SEQ10;
        break;
    default:
        stast10 = 0;
        break;
    }
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_10,
                  stast10);  // QSPINAND_FMSH_STAST_SEQ10

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_ChangeReadMode (FQspiPsu_T* qspiPtr, int read_mode)
{
    u32 rd_seq0, rd_seq1;

    // set read
    switch (read_mode)
    {
    case QSPI_RD_READ:
        rd_seq0 = QSPINAND_READ_SEQ0_READ;
        rd_seq1 = QSPINAND_READ_SEQ1_READ;
        break;
    case QSPI_RD_DOR:
        rd_seq0 = QSPINAND_READ_SEQ0_DOR;
        rd_seq1 = QSPINAND_READ_SEQ1_DOR;
        break;
    case QSPI_RD_QOR:
        rd_seq0 = QSPINAND_READ_SEQ0_QOR;
        rd_seq1 = QSPINAND_READ_SEQ1_QOR;
        break;
    case QSPI_RD_DIOR:
        rd_seq0 = QSPINAND_READ_SEQ0_DIOR;
        rd_seq1 = QSPINAND_READ_SEQ1_DIOR | QSPI_READ_SEQ_MB_EN |
                  QSPI_READ_SEQ_DUMMY(4);
        break;
    case QSPI_RD_QIOR:
        rd_seq0 = QSPINAND_READ_SEQ0_QIOR;
        rd_seq1 = QSPINAND_READ_SEQ1_QIOR | QSPI_READ_SEQ_MB_EN |
                  QSPI_READ_SEQ_DUMMY(2);
        break;
    default:
        fmsh_print_err(
            "ERROR(QSPI):read cmd is not set correctly, use FAST READ cmd as "
            "default.\r\n");
        rd_seq0 = QSPINAND_READ_SEQ0_READ;
        rd_seq1 = QSPINAND_READ_SEQ1_READ;
        break;
    }

    // read seq cfg

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_READ_SEQ_CFG_1, rd_seq1);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_READ_SEQ_CFG_0, rd_seq0);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_ChangeProgMode (FQspiPsu_T* qspiPtr, int prog_mode)
{
    u32 prog_seq0, prog_seq1;

    // set prog
    switch (prog_mode)
    {
    case QSPI_PROG_PP:
        prog_seq0 = QSPINAND_PROG_SEQ0_PP;
        prog_seq1 = QSPINAND_PROG_SEQ1_PP;
        break;
    case QSPI_PROG_QPP:
        prog_seq0 = QSPINAND_PROG_SEQ0_QPP;
        prog_seq1 = QSPINAND_PROG_SEQ1_QPP;
        break;
    default:
        fmsh_print_err(
            "ERROR(QSPI):program cmd is not set correctly, use PP cmd as "
            "default.\r\n");
        prog_seq0 = QSPINAND_PROG_SEQ0_PP;
        prog_seq1 = QSPINAND_PROG_SEQ1_PP;
        break;
    }

    // prog seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PROG_SEQ_CFG_0, prog_seq0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PROG_SEQ_CFG_1, prog_seq1);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_ChangeErsMode (FQspiPsu_T* qspiPtr)
{
    // ers seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_ERS_SEQ_CFG_0,
                  QSPINAND_ERS_SEQ0_SE);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_ERS_SEQ_CFG_1,
                  qspi_nand_dev.blk_shift);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_ReadId (FQspiPsu_T* qspiPtr, void* id)
{
    int ret;
    struct qspi_cmd cmd;
    struct qspi_data data;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_DATA_GLUING;
    cmd.bank = 0;
    cmd.opcode = 0x9F;
    cmd.op_nios = 1;
    cmd.naddrs = 0;
    cmd.ndata = 0;

    data.flags = QSPI_DATA_F_DATA_IN;
    data.bank = 0;
    data.dummy = 8;
    data.data = id;
    data.ndata = 2;
    data.nios = 1;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, &data, 10);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_Erase (FQspiPsu_T* qspiPtr, u32 ra,
                         u32 page_cnt)  // ra = row address
{
    int ret;
    int timeout_ms;

    if (page_cnt == 0)
    {
        return FMSH_SUCCESS;
    }

    timeout_ms = QSPINAND_TIMING_TSE_MS * page_cnt;
    ret = FQspiPsu_PIO_EraseSectors(qspiPtr, ra, page_cnt, timeout_ms);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_DirectWrite (FQspiPsu_T* qspiPtr, u32 offs, u32 len,
                               u8* sendBuf)
{
    int ret;
    uintptr_t addr;

    ret = FQspiPsu_SetWorkMode(qspiPtr, QSPI_WORK_MODE_DIRECT);
    if (ret)
    {
        return ret;
    }

    // write request
    addr = qspiPtr->config.data_base + offs;
    memcpy((void*)addr, (void*)sendBuf, (size_t)len);

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, QSPINAND_TIMING_TPP_US);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_DirectRead (FQspiPsu_T* qspiPtr, u32 offs, u32 len,
                              u8* recvBuf)
{
    int ret;
    uintptr_t addr;

    ret = FQspiPsu_SetWorkMode(qspiPtr, QSPI_WORK_MODE_DIRECT);
    if (ret)
    {
        return ret;
    }

    // Read request
    addr = qspiPtr->config.data_base + offs;
    memcpy((void*)recvBuf, (void*)addr, (size_t)len);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_Write (FQspiPsu_T* qspiPtr, u32 ra, u32 ca, u32 len,
                         u8* sendBuf)
{
    int ret;
    u64 offs;
    u32 pages;

    offs = ((u64)ra << 12) | (u64)ca;

    pages = (len + qspiPtr->page_size - 1) / qspiPtr->page_size;

    ret = FQspiPsu_CDMA_Program(qspiPtr, offs, len, (u64)sendBuf,
                                QSPINAND_TIMING_TPP_US * pages);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_Read (FQspiPsu_T* qspiPtr, u32 ra, u32 ca, u32 len,
                        u8* recvBuf)
{
    int ret;
    u64 offs;
    u32 pages;

    offs = ((u64)ra << 12) | (u64)ca;

    pages = (len + qspiPtr->page_size - 1) / qspiPtr->page_size;

    ret = FQspiPsu_CDMA_Read(qspiPtr, offs, len, (u64)recvBuf,
                             QSPINAND_TIMING_TPP_US * pages);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_PIOWrite (FQspiPsu_T* qspiPtr, u32 ra, u32 ca, u32 len,
                            u8* sendBuf)
{
    int ret;
    u64 offs;
    u32 pages;

    offs = ((u64)ra << 12) | (u64)ca;

    pages = (len + qspiPtr->page_size - 1) / qspiPtr->page_size;

    ret = FQspiPsu_PIO_Program(qspiPtr, offs, len, (u64)sendBuf,
                               QSPINAND_TIMING_TPP_US * pages);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_PIORead (FQspiPsu_T* qspiPtr, u32 ra, u32 ca, u32 len,
                           u8* recvBuf)
{
    int ret;
    u64 offs;
    u32 pages;

    offs = ((u64)ra << 12) | (u64)ca;

    pages = (len + qspiPtr->page_size - 1) / qspiPtr->page_size;

    ret = FQspiPsu_PIO_Read(qspiPtr, offs, len, (u64)recvBuf,
                            QSPINAND_TIMING_TPP_US * pages);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_Reset (FQspiPsu_T* qspiPtr)
{
    int ret;

    ret = FQspiPsu_PIO_SoftReset(qspiPtr, QSPINAND_TIMING_TRST_MS * 1000);
    if (ret)
    {
        return ret;
    }

    delay_ms(QSPINAND_TIMING_TRST_MS);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_IsFlashQuad (FQspiPsu_T* qspiPtr) { return 0; }

int FQspiPsu_Nand_EnableQuad (FQspiPsu_T* qspiPtr)
{
    int ret = 0;
    u8 feature;

    ret = FQspiPsu_Nand_GetFeature(qspiPtr, QSPINAND_FEAT, &feature);
    if (ret)
    {
        return ret;
    }

    feature |= (1 << QSPINAND_FEAT_QE);

    ret = FQspiPsu_Nand_SetFeature(qspiPtr, QSPINAND_FEAT, feature);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_Lock (FQspiPsu_T* qspiPtr) { return 0; }

int FQspiPsu_Nand_Unlock (FQspiPsu_T* qspiPtr)
{
    int ret = 0;

    ret = FQspiPsu_Nand_SetFeature(qspiPtr, QSPINAND_PROT, 0);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_WaitForOIP (FQspiPsu_T* qspiPtr, int timeout_us)
{
    int ret;
    u8 status;

    while (1)
    {
        ret = FQspiPsu_Nand_GetFeature(qspiPtr, QSPINAND_SR, &status);
        if (ret)
        {
            if (timeout_us <= 0)
            {
                return FMSH_FAILURE;
            }
            delay_1us();
            timeout_us--;
        }
        else
        {
            if ((status & (1 << QSPINAND_SR_OIP)) == 0)
            {
                return FMSH_SUCCESS;
            }
        }
    }
}

int FQspiPsu_Nand_WaitForReady (FQspiPsu_T* qspiPtr, int timeout_us)
{
    int ret;

    ret = FQspiPsu_Nand_WaitForOIP(qspiPtr, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_WREN (FQspiPsu_T* qspiPtr)
{
    int ret;
    u8 status;
    int timeout_us = 1000000;

    ret = FQspiPsu_Nand_ExecCmd(qspiPtr, QSPINAND_CMD_WREN);
    if (ret)
    {
        return ret;
    }

    while (1)
    {
        ret = FQspiPsu_Nand_GetFeature(qspiPtr, QSPINAND_SR, &status);
        if (ret)
        {
            if (timeout_us <= 0)
            {
                return FMSH_FAILURE;
            }
            delay_1us();
            timeout_us--;
        }
        else
        {
            if (status & (1 << QSPINAND_SR_WEL))
            {
                return FMSH_SUCCESS;
            }
        }
    }
}

int FQspiPsu_Nand_WRDI (FQspiPsu_T* qspiPtr)
{
    int ret;
    u8 status;
    int timeout_us = 1000000;

    ret = FQspiPsu_Nand_ExecCmd(qspiPtr, QSPINAND_CMD_WRDI);
    if (ret)
    {
        return ret;
    }

    while (1)
    {
        ret = FQspiPsu_Nand_GetFeature(qspiPtr, QSPINAND_SR, &status);
        if (ret)
        {
            if (timeout_us <= 0)
            {
                return FMSH_FAILURE;
            }
            delay_1us();
            timeout_us--;
        }
        else
        {
            if ((status & (1 << QSPINAND_SR_WEL)) == 0)
            {
                return FMSH_SUCCESS;
            }
        }
    }
}

int FQspiPsu_Nand_SetFeature (FQspiPsu_T* qspiPtr, u8 addr, u8 value)
{
    int ret;
    struct qspi_cmd cmd;
    u8 data[2] = {0};
    int timeout_us = 10;

    data[0] = value;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_GENERAL;
    cmd.bank = 0;
    cmd.opcode = 0x1F;
    cmd.op_nios = 1;
    cmd.naddrs = 1;
    cmd.addr_l = addr;
    cmd.addr_nios = 1;
    cmd.ndata = 1;
    cmd.data = data;  // data[1] is send out first

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, 0, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_GetFeature (FQspiPsu_T* qspiPtr, u8 addr, u8* value)
{
    int ret;
    struct qspi_cmd cmd;
    struct qspi_data data;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_DATA_GLUING;
    cmd.bank = 0;
    cmd.opcode = 0x0F;
    cmd.op_nios = 1;
    cmd.naddrs = 1;
    cmd.addr_l = addr;
    cmd.addr_nios = 1;
    cmd.ndata = 0;

    data.flags = QSPI_DATA_F_DATA_IN;
    data.bank = 0;
    data.dummy = 8;
    data.data = value;
    data.ndata = 1;
    data.nios = 1;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, &data, 10);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_RD_CACHE (FQspiPsu_T* qspiPtr, u32 addr, u32 len, u8* value)
{
    int ret;
    struct qspi_cmd cmd;
    struct qspi_data data;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_DATA_GLUING;
    cmd.bank = 0;
    cmd.opcode = 0x03;
    cmd.op_nios = 1;
    cmd.naddrs = 2;
    cmd.addr_l = addr;
    cmd.addr_nios = 1;
    cmd.ndata = 0;

    data.flags = QSPI_DATA_F_DATA_IN;
    data.bank = 0;
    data.dummy = 8;
    data.data = value;
    data.ndata = len;
    data.nios = 1;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, &data, 1000);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_PAGE_RD (FQspiPsu_T* qspiPtr, u32 addr)
{
    int ret;
    struct qspi_cmd cmd;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_GENERAL;
    cmd.bank = 0;
    cmd.opcode = 0x13;
    cmd.op_nios = 1;
    cmd.naddrs = 3;
    cmd.addr_l = addr;
    cmd.addr_nios = 1;
    cmd.ndata = 0;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, 0, 1000);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_P_LOAD (FQspiPsu_T* qspiPtr, u32 addr, u32 len, u8* value)
{
    int ret;
    struct qspi_cmd cmd;
    struct qspi_data data;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_DATA_GLUING;
    cmd.bank = 0;
    cmd.opcode = 0x02;
    cmd.op_nios = 1;
    cmd.naddrs = 2;
    cmd.addr_l = addr;
    cmd.addr_nios = 1;
    cmd.ndata = 0;

    data.flags = 0;
    data.bank = 0;
    data.dummy = 0;
    data.data = value;
    data.ndata = len;
    data.nios = 1;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, &data, 1000);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_P_EXEC (FQspiPsu_T* qspiPtr, u32 addr)
{
    int ret;
    struct qspi_cmd cmd;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_GENERAL;
    cmd.bank = 0;
    cmd.opcode = 0x10;
    cmd.op_nios = 1;
    cmd.naddrs = 3;
    cmd.addr_l = addr;
    cmd.addr_nios = 1;
    cmd.ndata = 0;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, 0, 1000);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nand_WaitForReady(qspiPtr, 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nand_ExecCmd (FQspiPsu_T* qspiPtr, u8 opcode)
{
    int ret;
    struct qspi_cmd cmd;
    int timeout_us = 1000;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_GENERAL;
    cmd.bank = 0;
    cmd.opcode = opcode;
    cmd.op_nios = 1;
    cmd.naddrs = 0;
    cmd.ndata = 0;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, 0, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}
