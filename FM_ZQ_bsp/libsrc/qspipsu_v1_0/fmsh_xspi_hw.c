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
 * @file fmsh_xspi_hw.c
 * @addtogroup qspipsu_v1_0
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
#include "fmsh_xspi_hw.h"

/*****************************************************************************
 * These functions are about status
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
static int polling (FQspiPsu_T *qspiPtr, u32 addr, u32 mask, int cond,
                    int sleep_us, int timeout_us)
{
    u32 value;

    while (1)
    {
        value = FMSH_ReadReg(qspiPtr->config.base, addr);
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

int FQspiPsu_WaitCtrlIdle (FQspiPsu_T *qspiPtr)
{
    int ret;

    ret = polling(qspiPtr, QSPI_R_CTRL_STATUS, QSPI_CTRL_BUSY, 0, 100, 5000);
    return ret;
}

int FQspiPsu_WaitSdmaTrigg (FQspiPsu_T *qspiPtr, int timeout_us)
{
    int ret;

    ret = polling(qspiPtr, QSPI_R_INTR_STATUS, QSPI_INTR_SDMA_TRIGG,
                  QSPI_INTR_SDMA_TRIGG, 1, timeout_us);
    if (ret == 0)
    {
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_INTR_STATUS,
                      QSPI_INTR_SDMA_TRIGG);
    }

    return ret;
}

int FQspiPsu_WaitTrdReady (FQspiPsu_T *qspiPtr, int timeout_us)
{
    int ret;
    u32 trd = qspiPtr->cur_cs;

    ret = polling(qspiPtr, QSPI_R_TRD_STATUS, QSPI_BUSY_TRD(trd), 0, 1,
                  timeout_us);

    return ret;
}

int FQspiPsu_WaitCmdComplete (FQspiPsu_T *qspiPtr, int timeout_us)
{
    u32 value;

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_STATUS_PTR, qspiPtr->cur_cs);

    while (1)
    {
        value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_CMD_STATUS);
        if (value & QSPI_STATUS_FAIL)
        {
            return FMSH_FAILURE;
        }
        if (value & QSPI_STATUS_COMPLETE)
        {
            return FMSH_SUCCESS;
        }

        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }
        timeout_us--;
        delay_us(1);
    }
}

int FQspiPsu_CheckCmdStatus (FQspiPsu_T *qspiPtr, u32 *status)
{
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_STATUS_PTR, qspiPtr->cur_cs);
    if (status)
    {
        *status = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_CMD_STATUS);
    }

    return FMSH_SUCCESS;
}

/*****************************************************************************
 * This function config work mode
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_SetWorkMode (FQspiPsu_T *qspiPtr, int mode)
{
    int ret;
    u32 value;

    // wait for controller idle
    ret = FQspiPsu_WaitCtrlIdle(qspiPtr);
    if (ret)
    {
        fmsh_print_err(
            "fail to change to work mode %d, qspi controller is not idle!\r\n",
            mode);
        return ret;
    }

    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_CTRL_CONFIG);
    if ((value & QSPI_WORK_MODE_MASK) != QSPI_WORK_MODE(mode))
    {
        // switch work mode
        value &= ~QSPI_WORK_MODE_MASK;
        value |= QSPI_WORK_MODE(mode);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CTRL_CONFIG, value);
    }

    return FMSH_SUCCESS;
}

/*****************************************************************************
 * This function config interrupt
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
u32 FQspiPsu_IntrEnabled (FQspiPsu_T *qspiPtr)
{
    u32 value;

    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_INTR_ENABLE);

    return value;
}

void FQspiPsu_ClearIntr (FQspiPsu_T *qspiPtr, u32 mask)
{
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_INTR_STATUS, mask);
}

void FQspiPsu_SetIntrMask (FQspiPsu_T *qspiPtr, u32 mask, int enable)
{
    u32 value;

    value = mask;
    if (enable)
    {
        value |= QSPI_INTR_EN;
    }
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_INTR_ENABLE, value);
}

/*****************************************************************************
 * This function config spi clk mode
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FQspiPsu_SetClkMode (FQspiPsu_T *qspiPtr, int mode)
{
    u32 value = 0;

    if (mode == 3)
    {
        value = 1;
    }

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CLOCK_MODE_SETTINGS, value);
}

/*****************************************************************************
 * This function config write protect pin(dq2)
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FQspiPsu_DQ2Toggle (FQspiPsu_T *qspiPtr, int dq2_line, int enable)
{
    u32 value;

    value = 0;
    if (dq2_line)
    {
        value |= 0x1;
    }
    if (enable)
    {
        value |= 0x2;
    }

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_WP_SETTINGS, value);
}

/*****************************************************************************
 * This function config reset pin(dq3)
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FQspiPsu_DQ3Toggle (FQspiPsu_T *qspiPtr, int dq3_line, int enable)
{
    u32 value;

    value = 0x10;
    if (dq3_line)
    {
        value |= 0x1;
    }
    if (enable)
    {
        value |= 0x2;
    }

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_RESET_PIN_SETTINGS, value);
}

/*****************************************************************************
 * This function config xip
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FQspiPsu_SetXipEn (FQspiPsu_T *qspiPtr, int xip_en)
{
    u32 value;

    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_CFG);
    value &= ~(QSPI_DAC_BANK_NUM_MASK | QSPI_DAC_MB_XIP_EN |
               QSPI_DAC_MB_XIP_DIS);
    value |= QSPI_DAC_BANK_NUM(qspiPtr->cur_cs);
    if (xip_en)
    {
        value |= QSPI_DAC_MB_XIP_EN;
    }
    else
    {
        value |= QSPI_DAC_MB_XIP_DIS;
    }
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DIRECT_ACCESS_CFG, value);
}

/*****************************************************************************
 * This function config timing & delay
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FQspiPsu_SetDelays (FQspiPsu_T *qspiPtr, u8 csda, u8 cseot, u8 cssot)
{
    u32 value;

    value = ((u32)csda << 24) | ((u32)cseot << 8) | cssot;

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DEV_DELAY, value);
}

void FQspiPsu_GetDelays (FQspiPsu_T *qspiPtr, u8 *csda, u8 *cseot, u8 *cssot)
{
    u32 value;
    u8 csda_int, cseot_int, cssot_int;

    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_DEV_DELAY);
    csda_int = (value >> 24) & 0xff;
    cseot_int = (value >> 8) & 0xff;
    cssot_int = value & 0xff;

    if (csda)
    {
        *csda = csda_int;
    }
    if (cseot)
    {
        *cseot = cseot_int;
    }
    if (cssot)
    {
        *cssot = cssot_int;
    }
}

/*****************************************************************************
 * This function config phy
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FQspiPsu_ResetDll (FQspiPsu_T *qspiPtr, int signal)
{
    u32 value;

    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_DLL_PHY_CTRL);
    if (signal)
    {
        value |= QSPI_DLL_RST_N;
    }
    else
    {
        value &= ~QSPI_DLL_RST_N;
    }
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DLL_PHY_CTRL, value);
}

int FQspiPsu_SetDma (FQspiPsu_T *qspiPtr, int word_size, int ote, int burst_sel,
                     int err_resp)
{
    u32 value;

    // value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_DMA_SETTING);
    value = ((word_size & 0x3) << 18) | ((err_resp & 0x1) << 17) |
            ((ote & 0x1) << 16) | (burst_sel & 0xff);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DMA_SETTING, value);

    return FMSH_SUCCESS;
}
