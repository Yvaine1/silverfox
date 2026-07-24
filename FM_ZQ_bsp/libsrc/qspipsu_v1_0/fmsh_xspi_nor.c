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
 * @file fmsh_xspi_nor.c
 * @addtogroup qspipsu_v1_0
 * @{
 *
 * Contains implements the xspi nor flash interface functions.
 * See fmsh_xspi_nor.h for a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- ----------  -----------------------------------------------
 * 1.00  hzq 2022/12/28  First release
 *s
 * </pre>
 *
 ******************************************************************************/
#include <string.h>

#include "fmsh_xspi.h"
#include "fmsh_xspi_hw.h"
#include "fmsh_xspi_nor.h"

/*****************************************************************************/
/******* global seq for profile1 *******/
#define QSPINOR_GLB_SEQ0_CFG       (0x208f)
#define QSPINOR_GLB_SEQ1_CFG       (0x0)

/******* reset sequence for profile1 *******/
#define QSPINOR_RST_SEQ0_RST       (0x00019966)
#define QSPINOR_RST_SEQ1_RST       (0x00669900)

#define QSPINOR_RST_SEQ0_RST_2     (0x0001F0FF)
#define QSPINOR_RST_SEQ1_RST_2     (0x00E00000)

/******* erase sequence for profile1 *******/
#define QSPINOR_ERS_SEQ0_SE        (0x002730D8)
#define QSPINOR_ERS_SEQ0_P4E       (0x00DF3020)
#define QSPINOR_ERS_SEQ0_4SE       (0x002340DC)
#define QSPINOR_ERS_SEQ0_4P4E      (0x00DE4021)

#define QSPINOR_ERS_SEQ2_BE        (0x003800C7)

/******* program sequence for profile1 *******/
#define QSPINOR_PROG_SEQ0_PP       (0x00003002)
#define QSPINOR_PROG_SEQ1_PP       (0x0000FD00)

#define QSPINOR_PROG_SEQ0_QPP      (0x00203032)
#define QSPINOR_PROG_SEQ1_QPP      (0x0000CD00)

#define QSPINOR_PROG_SEQ0_4PP      (0x00004012)
#define QSPINOR_PROG_SEQ1_4PP      (0x0000ED00)

#define QSPINOR_PROG_SEQ0_4QPP     (0x00204034)
#define QSPINOR_PROG_SEQ1_4QPP     (0x0000CB00)

/******* read sequence for profile1 *******/
#define QSPINOR_READ_SEQ0_READ     (0x00003003)
#define QSPINOR_READ_SEQ1_READ     (0x0000FC00)

#define QSPINOR_READ_SEQ0_FR       (0x0800300B)
#define QSPINOR_READ_SEQ1_FR       (0x0000F400)

#define QSPINOR_READ_SEQ0_DOR      (0x0810303B)
#define QSPINOR_READ_SEQ1_DOR      (0x0000CC00)

#define QSPINOR_READ_SEQ0_QOR      (0x0820306B)
#define QSPINOR_READ_SEQ1_QOR      (0x00009400)

#define QSPINOR_READ_SEQ0_DIOR     (0x001130BB)
#define QSPINOR_READ_SEQ1_DIOR     (0x00004400)

#define QSPINOR_READ_SEQ0_QIOR     (0x002230EB)
#define QSPINOR_READ_SEQ1_QIOR     (0x00001400)

#define QSPINOR_READ_SEQ0_DDRFR    (0x0088300D)
#define QSPINOR_READ_SEQ1_DDRFR    (0x0000F200)

#define QSPINOR_READ_SEQ0_DDRDIOR  (0x009930BD)
#define QSPINOR_READ_SEQ1_DDRDIOR  (0x00004200)

#define QSPINOR_READ_SEQ0_DDRQIOR  (0x00AA30ED)
#define QSPINOR_READ_SEQ1_DDRQIOR  (0x00001200)

#define QSPINOR_READ_SEQ0_4READ    (0x00004013)
#define QSPINOR_READ_SEQ1_4READ    (0x0000EC00)

#define QSPINOR_READ_SEQ0_4FR      (0x0800400C)
#define QSPINOR_READ_SEQ1_4FR      (0x0000F300)

#define QSPINOR_READ_SEQ0_4DOR     (0x0810403C)
#define QSPINOR_READ_SEQ1_4DOR     (0x0000C300)

#define QSPINOR_READ_SEQ0_4QOR     (0x0820406C)
#define QSPINOR_READ_SEQ1_4QOR     (0x00009300)

#define QSPINOR_READ_SEQ0_4DIOR    (0x001140BC)
#define QSPINOR_READ_SEQ1_4DIOR    (0x00004300)

#define QSPINOR_READ_SEQ0_4QIOR    (0x002240EC)
#define QSPINOR_READ_SEQ1_4QIOR    (0x00001300)

#define QSPINOR_READ_SEQ0_4DDRFR   (0x0088400E)
#define QSPINOR_READ_SEQ1_4DDRFR   (0x0000F100)

#define QSPINOR_READ_SEQ0_4DDRDIOR (0x009940BE)
#define QSPINOR_READ_SEQ1_4DDRDIOR (0x00004100)

#define QSPINOR_READ_SEQ0_4DDRQIOR (0x00AA40EE)
#define QSPINOR_READ_SEQ1_4DDRQIOR (0x00001100)

/******* wel sequence for profile1 *******/
#define QSPINOR_WE_SEQ0            (0x01F90006)

/******* STATUS sequence for profile1 *******/
#define QSPINOR_STAST_SEQ0         (0x00000000)
#define QSPINOR_STAST_SEQ1         (0x00000000)
#define QSPINOR_STAST_SEQ2         (0x05000505)
#define QSPINOR_STAST_SEQ3         (0xFA00FAFA)
#define QSPINOR_STAST_SEQ5         (0x00000040)

#define QSPINOR_STAST_SEQ2_2       (0x70007070)
#define QSPINOR_STAST_SEQ3_2       (0x8F008F8F)
#define QSPINOR_STAST_SEQ5_2       (0x00000057)

/*****************************************************************************/
static __no_init struct qspi_nor qspi_nor_dev;

static struct qspi_nor_param nor_param_default = {
    .csda_ns = QSPINOR_CSDA_NS,
    .cseot_ns = QSPINOR_CSEOT_NS,
    .cssot_ns = QSPINOR_CSSOT_NS,

    .trst_max_ms = QSPINOR_TIMING_TRST_MS,
    .tw_max_ms = QSPINOR_TIMING_TW_MS,
    .tpp_max_us = QSPINOR_TIMING_TPP_US,
    .tse_max_ms = QSPINOR_TIMING_TSE_MS,
    .tbe_max_s = QSPINOR_TIMING_TBE_S,
};

static struct qspi_usercfg qspi_nor_cfg_default __attribute__((aligned(4))) = {
    .flags = 0,                 // QSPI_F_INTR_EN,
    .ers_mode = QSPI_ERS_SE,    // standard sector erase
    .prog_mode = QSPI_PROG_PP,  // x1 program
    .read_mode = QSPI_RD_QOR,   // x4 read
};

int FQspiPsu_Nor_Init (FQspiPsu_T* qspiPtr, struct qspi_usercfg* usercfg)
{
    int ret;
    struct qspi_usercfg* cfg;
    u8 id[8];

    qspiPtr->type = QSPI_TYPE_NOR;

    // config qspi usercfg
    if (usercfg)
    {
        qspiPtr->usercfg = usercfg;
    }
    else
    {
        qspiPtr->usercfg = &qspi_nor_cfg_default;
    }
    cfg = qspiPtr->usercfg;

    (void)FQspiPsu_InitHw(qspiPtr, cfg);

    // get maker & device size
    ret = FQspiPsu_Nor_ReadId(qspiPtr, id);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nor_GetFlashInfo(qspiPtr, id);
    if (ret)
    {
        return ret;
    }

    // set transfer mode
    ret = FQspiPsu_Nor_SetFlashMode(qspiPtr);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nor_Reset(qspiPtr);
    if (ret)
    {
        return ret;
    }

    // set baud rate & data capture delay
    (void)FQspiPsu_Delay(qspiPtr, qspiPtr->config.sclk_hz, QSPINOR_CSDA_NS,
                   QSPINOR_CSEOT_NS, QSPINOR_CSSOT_NS);

    qspiPtr->priv = (void*)&qspi_nor_dev;

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_GetFlashInfo (FQspiPsu_T* qspiPtr, u8* id)
{
    qspi_nor_dev.flags = 0;
    qspi_nor_dev.param = &nor_param_default;

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

    // device size
    switch (id[2])
    {
    case QSPINOR_SIZE_ID_32KB:
        qspiPtr->dev_size = 0x1 << 15;
        break;
    case QSPINOR_SIZE_ID_64KB:
        qspiPtr->dev_size = 0x1 << 16;
        break;
    case QSPINOR_SIZE_ID_128KB:
        qspiPtr->dev_size = 0x1 << 17;
        break;
    case QSPINOR_SIZE_ID_256KB:
        qspiPtr->dev_size = 0x1 << 18;
        break;
    case QSPINOR_SIZE_ID_512KB:
        qspiPtr->dev_size = 0x1 << 19;
        break;
    case QSPINOR_SIZE_ID_1MB:
        qspiPtr->dev_size = 0x1 << 20;
        break;
    case QSPINOR_SIZE_ID_2MB:
        qspiPtr->dev_size = 0x1 << 21;
        break;
    case QSPINOR_SIZE_ID_4MB:
        qspiPtr->dev_size = 0x1 << 22;
        break;
    case QSPINOR_SIZE_ID_8MB:
        qspiPtr->dev_size = 0x1 << 23;
        break;
    case QSPINOR_SIZE_ID_16MB:
        qspiPtr->dev_size = 0x1 << 24;
        break;
    case QSPINOR_SIZE_ID_32MB:
        qspiPtr->dev_size = 0x1 << 25;
        break;
    case QSPINOR_SIZE_ID_64MB:
    case QSPINOR_SIZE_ID_64MB_TYPE2:
        qspiPtr->dev_size = 0x1 << 26;
        break;
    case QSPINOR_SIZE_ID_128MB:
    case QSPINOR_SIZE_ID_128MB_TYPE2:
        qspiPtr->dev_size = 0x1 << 27;
        break;
    case QSPINOR_SIZE_ID_256MB:
    case QSPINOR_SIZE_ID_256MB_TYPE2:
        qspiPtr->dev_size = 0x1 << 28;
        break;
    default:
        qspiPtr->dev_size = QSPI_UNKNOWN_SIZE;
        break;
    }

    qspiPtr->blk_size = 0x1 << 16;  // 64K as default
    qspi_nor_dev.blk_shift = 16;
    qspiPtr->page_size = 0x1 << 8;  // 256

    switch (id[0])
    {
    case QSPI_MAKER_ID_MICRON:
        if (qspiPtr->dev_size > 0x2000000)
        {
            qspi_nor_dev.flags |= QSPINOR_F_FSR;
        }
        break;
    case QSPI_MAKER_ID_SPANSION:
        if (id[4] == 0x0)
        {
            qspiPtr->blk_size = 0x1 << 18;  // 256KB
            qspi_nor_dev.blk_shift = 18;
        }
        break;
     default:
        break;
    }

    if (qspiPtr->usercfg->naddrs == 0)
    {
        if (qspiPtr->dev_size > 0x1000000)
        {
            qspiPtr->usercfg->naddrs = 4;
        }
        else
        {
            qspiPtr->usercfg->naddrs = 3;
        }
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_SetFlashMode (FQspiPsu_T* qspiPtr)
{
    u32 prog_seq0, prog_seq1;
    u32 ers_seq0;
    u32 mb;

    (void)FQspiPsu_Nor_ChangeReadMode(qspiPtr, qspiPtr->usercfg->read_mode);

    if (qspiPtr->usercfg->naddrs == 4)
    {
        // set program
        switch (qspiPtr->usercfg->prog_mode)
        {
        case QSPI_PROG_PP:
            prog_seq0 = QSPINOR_PROG_SEQ0_4PP;
            prog_seq1 = QSPINOR_PROG_SEQ1_4PP;
            break;
        case QSPI_PROG_QPP:
            prog_seq0 = QSPINOR_PROG_SEQ0_4QPP;
            prog_seq1 = QSPINOR_PROG_SEQ1_4QPP;
            break;
        default:
            fmsh_print_err(
                "ERROR(QSPI):program cmd is not set correctly, use 4PP cmd as "
                "default.\r\n");
            prog_seq0 = QSPINOR_PROG_SEQ0_4PP;
            prog_seq1 = QSPINOR_PROG_SEQ1_4PP;
            break;
        }
        // set erase
        switch (qspiPtr->usercfg->ers_mode)
        {
        case QSPI_ERS_SE:
            ers_seq0 = QSPINOR_ERS_SEQ0_4SE;
            break;
        case QSPI_ERS_P4E:
            qspiPtr->blk_size = 0x1 << 12;
            qspi_nor_dev.blk_shift = 12;
            ers_seq0 = QSPINOR_ERS_SEQ0_4P4E;
            break;
        default:
            fmsh_print_err(
                "ERROR(QSPI):erase cmd is not set correctly, use 4SE cmd as "
                "default.\r\n");
            ers_seq0 = QSPINOR_ERS_SEQ0_4SE;
            break;
        }
    }
    // else 3B address mode
    else
    {
        // set program
        switch (qspiPtr->usercfg->prog_mode)
        {
        case QSPI_PROG_PP:
            prog_seq0 = QSPINOR_PROG_SEQ0_PP;
            prog_seq1 = QSPINOR_PROG_SEQ1_PP;
            break;
        case QSPI_PROG_QPP:
            prog_seq0 = QSPINOR_PROG_SEQ0_QPP;
            prog_seq1 = QSPINOR_PROG_SEQ1_QPP;
            break;
        default:
            fmsh_print_err(
                "ERROR(QSPI):program cmd is not set correctly, use PP cmd as "
                "default.\r\n");
            prog_seq0 = QSPINOR_PROG_SEQ0_PP;
            prog_seq1 = QSPINOR_PROG_SEQ1_PP;
            break;
        }
        // set erase
        switch (qspiPtr->usercfg->ers_mode)
        {
        case QSPI_ERS_SE:
            ers_seq0 = QSPINOR_ERS_SEQ0_SE;
            break;
        case QSPI_ERS_P4E:
            qspiPtr->blk_size = 0x1 << 12;
            qspi_nor_dev.blk_shift = 12;
            ers_seq0 = QSPINOR_ERS_SEQ0_P4E;
            break;
        default:
            fmsh_print_err(
                "ERROR(QSPI):erase cmd is not set correctly, use SE cmd as "
                "default.\r\n");
            ers_seq0 = QSPINOR_ERS_SEQ0_SE;
            break;
        }
    }
    // ers seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_ERS_SEQ_CFG_0, ers_seq0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_ERS_SEQ_CFG_1,
                  qspi_nor_dev.blk_shift);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_ERS_SEQ_CFG_2,
                  QSPINOR_ERS_SEQ2_BE);
    // prog seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PROG_SEQ_CFG_0, prog_seq0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PROG_SEQ_CFG_1, prog_seq1);

    // set mode bits
    switch (qspiPtr->maker)
    {
    case QSPI_MAKER_ID_SPANSION:
    case QSPI_MAKER_ID_ISSI:
    case QSPI_MAKER_ID_MACRONIX:
        mb = QSPI_DIS_MB_VAL(0xFF) | QSPI_EN_MB_VAL(0xA5);
        break;
    case QSPI_MAKER_ID_WINBOND:
    case QSPI_MAKER_ID_FMSH:
    case QSPI_MAKER_ID_GD:
        mb = QSPI_DIS_MB_VAL(0xFF) | QSPI_EN_MB_VAL(0x20);
        break;
    case QSPI_MAKER_ID_MICRON:
    default:
        mb = QSPI_DIS_MB_VAL(0xFF) | QSPI_EN_MB_VAL(0x00);
        break;
    }
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_XIP_MODE_CFG, mb);

    // global seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_GLOBAL_SEQ_CFG,
                  QSPINOR_GLB_SEQ0_CFG);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_GLOBAL_SEQ_CFG_1,
                  QSPINOR_GLB_SEQ1_CFG);
    // direct access cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_RMP,
                  qspiPtr->config.data_base);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_RMP_1, 0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_CFG,
                  QSPI_DAC_BANK_NUM(qspiPtr->cur_cs) | QSPI_DAC_RMP_ADDR_EN);
    // rst seq cfg
    if (qspiPtr->maker == QSPI_MAKER_ID_SPANSION)
    {
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_RST_SEQ_CFG_0,
                      QSPINOR_RST_SEQ0_RST_2);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_RST_SEQ_CFG_1,
                      QSPINOR_RST_SEQ1_RST_2);
    }
    else
    {
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_RST_SEQ_CFG_0,
                      QSPINOR_RST_SEQ0_RST);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_RST_SEQ_CFG_1,
                      QSPINOR_RST_SEQ1_RST);
    }
    // we seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_WE_SEQ_CFG_0, QSPINOR_WE_SEQ0);
    // stat seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_0,
                  QSPINOR_STAST_SEQ0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_1,
                  QSPINOR_STAST_SEQ1);
    if (qspi_nor_dev.flags & QSPINOR_F_FSR)
    {
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_2,
                      QSPINOR_STAST_SEQ2_2);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_3,
                      QSPINOR_STAST_SEQ3_2);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_5,
                      QSPINOR_STAST_SEQ5_2);
    }
    else
    {
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_2,
                      QSPINOR_STAST_SEQ2);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_3,
                      QSPINOR_STAST_SEQ3);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_STAT_SEQ_CFG_5,
                      QSPINOR_STAST_SEQ5);
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_ChangeReadMode (FQspiPsu_T* qspiPtr, int read_mode)
{
    u32 rd_seq0, rd_seq1;

    if (qspiPtr->usercfg->naddrs == 4)
    {
        // set read
        switch (read_mode)
        {
        case QSPI_RD_READ:
            rd_seq0 = QSPINOR_READ_SEQ0_4READ;
            rd_seq1 = QSPINOR_READ_SEQ1_4READ;
            break;
        case QSPI_RD_FR:
            rd_seq0 = QSPINOR_READ_SEQ0_4FR;
            rd_seq1 = QSPINOR_READ_SEQ1_4FR;
            break;
        case QSPI_RD_DOR:
            rd_seq0 = QSPINOR_READ_SEQ0_4DOR;
            rd_seq1 = QSPINOR_READ_SEQ1_4DOR;
            break;
        case QSPI_RD_QOR:
            rd_seq0 = QSPINOR_READ_SEQ0_4QOR;
            rd_seq1 = QSPINOR_READ_SEQ1_4QOR;
            break;
        case QSPI_RD_DIOR:
            if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
            {
                rd_seq0 = QSPINOR_READ_SEQ0_4DIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_4DIOR | QSPI_READ_SEQ_MB_EN |
                          QSPI_READ_SEQ_DUMMY(4);
            }
            else
            {
                rd_seq0 = QSPINOR_READ_SEQ0_4DIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_4DIOR | QSPI_READ_SEQ_MB_EN;
            }
            break;
        case QSPI_RD_QIOR:
            if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
            {
                rd_seq0 = QSPINOR_READ_SEQ0_4QIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_4QIOR | QSPI_READ_SEQ_MB_EN |
                          QSPI_READ_SEQ_DUMMY(8);
            }
            else
            {
                rd_seq0 = QSPINOR_READ_SEQ0_4QIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_4QIOR | QSPI_READ_SEQ_MB_EN |
                          QSPI_READ_SEQ_DUMMY(4);
            }
            break;
        default:
            fmsh_print_err(
                "ERROR(QSPI):read cmd is not set correctly, use FAST READ cmd "
                "as default.\r\n");
            rd_seq0 = QSPINOR_READ_SEQ0_4FR;
            rd_seq1 = QSPINOR_READ_SEQ1_4FR;
            break;
        }
    }
    // else 3B address mode
    else
    {
        // set read
        switch (read_mode)
        {
        case QSPI_RD_READ:
            rd_seq0 = QSPINOR_READ_SEQ0_READ;
            rd_seq1 = QSPINOR_READ_SEQ1_READ;
            break;
        case QSPI_RD_FR:
            rd_seq0 = QSPINOR_READ_SEQ0_FR;
            rd_seq1 = QSPINOR_READ_SEQ1_FR;
            break;
        case QSPI_RD_DOR:
            rd_seq0 = QSPINOR_READ_SEQ0_DOR;
            rd_seq1 = QSPINOR_READ_SEQ1_DOR;
            break;
        case QSPI_RD_QOR:
            rd_seq0 = QSPINOR_READ_SEQ0_QOR;
            rd_seq1 = QSPINOR_READ_SEQ1_QOR;
            break;
        case QSPI_RD_DIOR:
            if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
            {
                rd_seq0 = QSPINOR_READ_SEQ0_DIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_DIOR | QSPI_READ_SEQ_MB_EN |
                          QSPI_READ_SEQ_DUMMY(4);
            }
            else
            {
                rd_seq0 = QSPINOR_READ_SEQ0_DIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_DIOR | QSPI_READ_SEQ_MB_EN;
            }
            break;
        case QSPI_RD_QIOR:
            if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
            {
                rd_seq0 = QSPINOR_READ_SEQ0_QIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_QIOR | QSPI_READ_SEQ_MB_EN |
                          QSPI_READ_SEQ_DUMMY(8);
            }
            else
            {
                rd_seq0 = QSPINOR_READ_SEQ0_QIOR;
                rd_seq1 = QSPINOR_READ_SEQ1_QIOR | QSPI_READ_SEQ_MB_EN |
                          QSPI_READ_SEQ_DUMMY(4);
            }
            break;
        default:
            fmsh_print_err(
                "ERROR(QSPI):read cmd is not set correctly, use FAST READ cmd "
                "as default.\r\n");
            rd_seq0 = QSPINOR_READ_SEQ0_FR;
            rd_seq1 = QSPINOR_READ_SEQ1_FR;
            break;
        }
    }

    // read seq cfg
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_READ_SEQ_CFG_0, rd_seq0);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_READ_SEQ_CFG_1, rd_seq1);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_ReadId (FQspiPsu_T* qspiPtr, void* id)
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
    data.dummy = 0;
    data.data = id;
    data.ndata = 8;
    data.nios = 1;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, &data, 10);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_Erase (FQspiPsu_T* qspiPtr, u32 offs, u32 len)
{
    int ret;
    u32 addr_st, addr_ed;
    int nblks;
    int timeout_ms;

    if (len == 0)
    {
        return FMSH_SUCCESS;
    }

    addr_st = offs & ~(qspiPtr->blk_size - 1);
    addr_ed = (offs + len + qspiPtr->blk_size - 1) & ~(qspiPtr->blk_size - 1);
    nblks = (addr_ed - addr_st) / qspiPtr->blk_size;

    timeout_ms = QSPINOR_TIMING_TSE_MS * nblks;
    ret = FQspiPsu_PIO_EraseSectors(qspiPtr, addr_st, nblks, timeout_ms);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_Write (FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* sendBuf)
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
    (void)memcpy((void*)addr, (void*)sendBuf, (size_t)len);

    ret = FQspiPsu_Nor_WaitForReady(qspiPtr, QSPINOR_TIMING_TPP_US);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_Read (FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* recvBuf)
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
    (void)memcpy((void*)recvBuf, (void*)addr, (size_t)len);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_FastWrite (FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* sendBuf)
{
    int ret;
    u32 pages;

    pages = (len + qspiPtr->page_size - 1) / qspiPtr->page_size;

    ret = FQspiPsu_CDMA_Program(qspiPtr, offs, len, (u64)sendBuf,
                                QSPINOR_TIMING_TPP_US * pages);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_FastRead (FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* recvBuf)
{
    int ret;
    u32 pages;

    pages = (len + qspiPtr->page_size - 1) / qspiPtr->page_size;

    ret = FQspiPsu_CDMA_Read(qspiPtr, offs, len, (u64)recvBuf,
                             QSPINOR_TIMING_TPP_US * pages);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_Reset (FQspiPsu_T* qspiPtr)
{
    int ret;

    ret = FQspiPsu_PIO_SoftReset(qspiPtr, QSPINOR_TIMING_TRST_MS * 1000);
    if (ret)
    {
        return ret;
    }

    delay_ms(QSPINOR_TIMING_TRST_MS);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_IsFlashQuad (FQspiPsu_T* qspiPtr)
{
    u8 value;
    int qe = 0;

    switch (qspiPtr->maker)
    {
    case QSPI_MAKER_ID_SPANSION:
        (void)FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_SPANSION_RDCR, &value,
                            1);  // 0x35
        if (value & QSPINOR_SPANSION_CR_QE)
        {
            qe = 1;
        }
        break;

    case QSPI_MAKER_ID_MICRON:
        qe = 1;
        break;

    case QSPI_MAKER_ID_WINBOND:
    case QSPI_MAKER_ID_GD:
        (void)FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_WINBOND_RDSR2, &value,
                            1);  // 0x35
        if (value & QSPINOR_WINBOND_SR2_QE)
        {
            qe = 1;
        }
        break;

    case QSPI_MAKER_ID_MACRONIX:
        (void)FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_RDSR1, &value, 1);  // 0x05
        if (value & QSPINOR_MACRONIX_SR1_QE)
        {
            qe = 1;
        }
        break;

    case QSPI_MAKER_ID_ISSI:
        (void)FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_RDSR1, &value, 1);  // 0x05
        if (value & QSPINOR_ISSI_SR1_QE)
        {
            qe = 1;
        }
        break;

    case QSPI_MAKER_ID_FMSH:
        (void)FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_FMSH_RDSR2, &value,
                            1);  // 0x35
        if (value & QSPINOR_FMSH_SR2_QE)
        {
            qe = 1;
        }
        break;
     default:
          break;
    }

    return qe;
}

int FQspiPsu_Nor_EnableQuad (FQspiPsu_T* qspiPtr)
{
    int ret = 0;
    u8 sr;

    // config qspi flash registers (quad_en)
    if (qspiPtr->maker == QSPI_MAKER_ID_SPANSION)
    {
        u8 cr;
        u8 value[2];

        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x35, &cr, 1);
        if ((cr & QSPINOR_SPANSION_CR_QE) == 0)
        {
            cr |= (u8)QSPINOR_SPANSION_CR_QE;
            value[0] = cr;
            value[1] = sr;
            ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, value, 2);
        }
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_MACRONIX)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if ((sr & QSPINOR_MACRONIX_SR1_QE) == 0)
        {
            sr |= (u8)QSPINOR_MACRONIX_SR1_QE;
            ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
        }
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_ISSI)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if ((sr & QSPINOR_ISSI_SR1_QE) == 0)
        {
            sr |= (u8)QSPINOR_ISSI_SR1_QE;
            ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
        }
    }

    else if (qspiPtr->maker == QSPI_MAKER_ID_WINBOND)
    {
        u8 sr2;
        u8 value[2];

        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x35, &sr2, 1);
        if ((sr2 & QSPINOR_WINBOND_SR2_QE) == 0)
        {
            sr2 |= (u8)QSPINOR_WINBOND_SR2_QE;
            value[0] = sr2;
            value[1] = sr;
            ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, value, 2);
        }
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_FMSH)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x35, &sr, 1);
        if ((sr & QSPINOR_FMSH_SR2_QE) == 0)
        {
            sr |= (u8)QSPINOR_FMSH_SR2_QE;
            ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x31, &sr, 1);
        }
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_GD)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x35, &sr, 1);
        if ((sr & QSPINOR_GD_SR2_QE) == 0)
        {
            sr |= (u8)QSPINOR_GD_SR2_QE;
            ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x31, &sr, 1);
        }
    }
    else
    {
        // do nothing
        ret = 0;
    }

    return ret;
}

int FQspiPsu_Nor_Lock (FQspiPsu_T* qspiPtr)
{
    int ret;
    u8 sr;

    if (qspiPtr->maker == QSPI_MAKER_ID_SPANSION)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        sr |= QSPINOR_SPANSION_SR1_BP;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        sr |= QSPINOR_MICRON_SR1_BP;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_ISSI)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        sr |= QSPINOR_ISSI_SR1_BP;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_MACRONIX)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        sr |= QSPINOR_ISSI_SR1_BP;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }
    else
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        sr |= 0x1c;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }

    return ret;
}

int FQspiPsu_Nor_Unlock (FQspiPsu_T* qspiPtr)
{
    int ret;
    u8 sr;

    if (qspiPtr->maker == QSPI_MAKER_ID_SPANSION)
    {
        sr = 0;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_ISSI)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        sr &= QSPINOR_ISSI_SR1_QE;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }
    else if (qspiPtr->maker == QSPI_MAKER_ID_MACRONIX)
    {
        (void)FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        sr &= QSPINOR_ISSI_SR1_QE;
        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }
    else
    {
        sr = 0;

        ret = FQspiPsu_Nor_SetNVReg(qspiPtr, 0x01, &sr, 1);
    }

    return ret;
}

int FQspiPsu_Nor_GetBankReg (FQspiPsu_T* qspiPtr, u8* value)
{
    int ret;
    u8 reg;

    // not support bank select
    if (qspiPtr->dev_size <= 0x1000000)
    {
        ret = FMSH_SUCCESS;
        reg = 0;
        goto done;
    }

    if (qspiPtr->maker == QSPI_MAKER_ID_SPANSION)
    {
        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x16, &reg, 1);
    }
    else
    {
        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0xC8, &reg, 1);
    }

done:
    if (value)
    {
        *value = reg;
    }

    return ret;
}

int FQspiPsu_Nor_SetSegment (FQspiPsu_T* qspiPtr, u8 high_addr)
{
    int ret;
    u8 ext_addr, ext_addr_rb;

    switch (qspiPtr->maker)
    {
    case QSPI_MAKER_ID_SPANSION:
    case QSPI_MAKER_ID_ISSI:
        /* bit7 is extadd id 0 to enable 3B address */
        ext_addr = high_addr & 0x7F;
        ret = FQspiPsu_Nor_SetReg(qspiPtr, 0x17, &ext_addr, 1);
        if (ret)
        {
            return ret;
        }
        /* readback & verify */
        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x16, &ext_addr_rb, 1);
        if (ret)
        {
            return ret;
        }
        if (ext_addr_rb != ext_addr)
        {
            return FMSH_FAILURE;
        }
        break;
    case QSPI_MAKER_ID_MICRON:
    case QSPI_MAKER_ID_MACRONIX:
    case QSPI_MAKER_ID_WINBOND:
    case QSPI_MAKER_ID_FMSH:
    case QSPI_MAKER_ID_GD:
        ext_addr = high_addr;
        ret = FQspiPsu_Nor_SetReg(qspiPtr, 0xc5, &ext_addr, 1);
        if (ret)
        {
            return ret;
        }
        /* readback & verify */
        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0xc8, &ext_addr_rb, 1);
        if (ret)
        {
            return ret;
        }
        if (ext_addr_rb != ext_addr)
        {
            return FMSH_FAILURE;
        }
        break;
    default:
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_Enter4B (FQspiPsu_T* qspiPtr)
{
    int ret;

    ret = FQspiPsu_Nor_WREN(qspiPtr);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nor_ExecCmd(qspiPtr, 0xb7);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_Exit4B (FQspiPsu_T* qspiPtr)
{
    int ret;

    ret = FQspiPsu_Nor_WREN(qspiPtr);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nor_ExecCmd(qspiPtr, 0xe9);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_EnterXIP (FQspiPsu_T* qspiPtr, int mode)
{
    int ret;

    ret = FQspiPsu_SetWorkMode(qspiPtr, QSPI_WORK_MODE_DIRECT);
    if (ret)
    {
        return ret;
    }

    if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
    {
        u8 vcr;
        (void)FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_MICRON_RDVCR, &vcr, 1);
        vcr &= 0xf7;
        (void)FQspiPsu_Nor_SetReg(qspiPtr, QSPINOR_CMD_MICRON_WRVCR, &vcr, 1);
    }

    (void)FQspiPsu_Nor_ChangeReadMode(qspiPtr, mode);

    (void)FQspiPsu_SetXipEn(qspiPtr, 1);

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_ExitXIP (FQspiPsu_T* qspiPtr, int mode)
{
    (void)FQspiPsu_SetXipEn(qspiPtr, 0);
    (void)FQspiPsu_Nor_ChangeReadMode(qspiPtr, mode);

    if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
    {
        u8 vcr;
        (void)FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_MICRON_RDVCR, &vcr, 1);
        vcr |= 0x8;
        (void)FQspiPsu_Nor_SetReg(qspiPtr, QSPINOR_CMD_MICRON_WRVCR, &vcr, 1);
    }

    return 0;
}

int FQspiPsu_Nor_WaitForWIP (FQspiPsu_T* qspiPtr, int timeout_us)
{
    int ret;
    u8 value;

    while (1)
    {
        ret = FQspiPsu_Nor_GetStatus1(qspiPtr, &value);
        if (ret)
        {
            goto delay;
        }

        if ((value & QSPINOR_SR1_BUSY) == 0)
        {
            ret = FMSH_SUCCESS;
            goto end;
        }
delay:
        if (timeout_us <= 0)
        {
            ret = FMSH_ETIME;
            goto end;
        }
        delay_1us();
        timeout_us--;
    }
end:
    return ret;
}

static int FQspiPsu_Nor_WaitForFSR (FQspiPsu_T* qspiPtr, int timeout_us)
{
    int ret;
    u8 value;
    int det = 2;

    while (1)
    {
        ret = FQspiPsu_Nor_GetFlagStatus(qspiPtr, &value);
        if (ret)
        {
            goto delay;
        }

        if ((value & 0x80) == 0x80)
        {
            det--;
            if (det == 0)
            {
                ret = FMSH_SUCCESS;
                goto end;
            }
        }
delay:
        if (timeout_us <= 0)
        {
            ret = FMSH_ETIME;
            goto end;
        }
        delay_1us();
        timeout_us--;
    }
end:
    return ret;
}

int FQspiPsu_Nor_WaitForReady (FQspiPsu_T* qspiPtr, int timeout_us)
{
    int ret;

    // Poll Status Register1
    ret = FQspiPsu_Nor_WaitForWIP(qspiPtr, timeout_us);
    if (ret)
    {
        return ret;
    }

    // Poll Flag Status Register
    if (qspi_nor_dev.flags & QSPINOR_F_FSR)
    {
        ret = FQspiPsu_Nor_WaitForFSR(qspiPtr, timeout_us);
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_WREN (FQspiPsu_T* qspiPtr)
{
    int ret;
    u8 status;
    int timeout_us = 1000000;

    ret = FQspiPsu_Nor_ExecCmd(qspiPtr, QSPINOR_CMD_WREN);
    if (ret)
    {
        return ret;
    }

    while (1)
    {
        ret = FQspiPsu_Nor_GetStatus1(qspiPtr, &status);
        if (ret)
        {
            goto delay;
        }
        if (status & QSPINOR_SR1_WEL)
        {
            return FMSH_SUCCESS;
        }
delay:
        if (timeout_us <= 0)
        {
            return FMSH_FAILURE;
        }
        delay_1us();
        timeout_us--;
    }
}

int FQspiPsu_Nor_WRDI (FQspiPsu_T* qspiPtr)
{
    int ret;
    u8 status;
    int timeout_us = 1000000;

    ret = FQspiPsu_Nor_ExecCmd(qspiPtr, QSPINOR_CMD_WRDI);
    if (ret)
    {
        return ret;
    }

    while (1)
    {
        ret = FQspiPsu_Nor_GetStatus1(qspiPtr, &status);
        if (ret)
        {
            goto delay;
        }
        if ((status & QSPINOR_SR1_WEL) == 0)
        {
            return FMSH_SUCCESS;
        }
delay:
        if (timeout_us <= 0)
        {
            return FMSH_FAILURE;
        }
        delay_1us();
        timeout_us--;
    }
}

int FQspiPsu_Nor_GetStatus1 (FQspiPsu_T* qspiPtr, u8* status)
{
    u8 value;
    int ret;

    ret = FQspiPsu_Nor_GetReg(qspiPtr, QSPINOR_CMD_RDSR1, &value, 1);
    if (ret)
    {
        return ret;
    }

    if (status)
    {
        *status = value;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_GetFlagStatus (FQspiPsu_T* qspiPtr, u8* status)
{
    u8 value;
    int ret;

    ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x70, &value, 1);
    if (ret)
    {
        return ret;
    }

    if (status)
    {
        *status = value;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_ClearFlagStatus (FQspiPsu_T* qspiPtr)
{
    int ret;

    ret = FQspiPsu_Nor_ExecCmd(qspiPtr, 0x50);
    return ret;
}

int FQspiPsu_Nor_SetNVReg (FQspiPsu_T* qspiPtr, u8 opcode, u8* value, u8 len)
{
    int ret;
    struct qspi_cmd cmd;
    u8 data[2] = {0};
    int timeout_us = 10;

    ret = FQspiPsu_Nor_WREN(qspiPtr);
    if (ret)
    {
        return ret;
    }

    if (value)
    {
        data[0] = value[0];
        if (len > 1)
        {
            data[1] = value[1];
        }
    }

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_GENERAL;
    cmd.bank = 0;
    cmd.opcode = opcode;
    cmd.op_nios = 1;
    cmd.naddrs = 0;
    cmd.ndata = len;
    cmd.data = data;  // data[1] is send out first

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, 0, timeout_us);
    if (ret)
    {
        return ret;
    }

    ret = FQspiPsu_Nor_WaitForReady(qspiPtr, QSPINOR_TIMING_TW_MS * 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_SetReg (FQspiPsu_T* qspiPtr, u8 opcode, u8* value, u8 len)
{
    int ret;
    struct qspi_cmd cmd;
    u8 data[2] = {0};
    int timeout_us = 10;

    if (value)
    {
        data[0] = value[0];
        if (len > 1)
        {
            data[1] = value[1];
        }
    }

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_GENERAL;
    cmd.bank = 0;
    cmd.opcode = opcode;
    cmd.op_nios = 1;
    cmd.naddrs = 0;
    cmd.ndata = len;
    cmd.data = data;  // data[1] is send out first

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, 0, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_GetReg (FQspiPsu_T* qspiPtr, u8 opcode, u8* value, u8 len)
{
    int ret;
    struct qspi_cmd cmd;
    struct qspi_data data;

    cmd.flags = 0;
    cmd.inst_type = QSPI_SEQ_P1_DATA_GLUING;
    cmd.bank = 0;
    cmd.opcode = opcode;
    cmd.op_nios = 1;
    cmd.naddrs = 0;
    cmd.ndata = 0;

    data.flags = QSPI_DATA_F_DATA_IN | QSPI_DATA_F_CMD_FIFO;
    data.bank = 0;
    data.dummy = 0;
    data.data = 0;
    data.ndata = len;
    data.nios = 1;

    ret = FQspiPsu_Stig_Exec(qspiPtr, &cmd, &data, 10);
    if (ret)
    {
        return ret;
    }

    if (value)
    {
        value[0] = (cmd.status >> 16) & 0xff;
        if (len > 1)
        {
            value[1] = (cmd.status >> 24) & 0xff;
        }
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_Nor_ExecCmd (FQspiPsu_T* qspiPtr, u8 opcode)
{
    int ret;
    struct qspi_cmd cmd;
    int timeout_us = 10;

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
