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
 * @file fmsh_spi_hw.c
 * @addtogroup spips_v2_2
 * @{
 *
 * Contains implements the low level interface functions of the FSpiPs driver.
 * See fmsh_spips_hw.h for a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00  hzq 7/1/19
 * 		     First release
 * 1.10  hzq 11/26/20
 * 		     The FSpiPs_SetMst API has been increased to replace
 * 		     FSpiPs_Mst and FSpiPs_Slv API functions.
 * 		     The FSpiPs_SetEnable API has been increased to replace
 * 		     FSpiPs_Enable and FSpiPs_Disable API functions.
 * 		     The FSpiPs_SetSckMode API has been modified to update
 * 		     user configuration information.
 * 		     The FSpiPs_SetTMode API has been modified to update
 * 		     user configuration information.
 * 		     The FSpiPs_SetLoopback API has been modified to update
 * 		     user configuration information.
 * 		     The FSpiPs_SetDFS32 API has been modified to update
 * 		     user configuration information.
 * 		     The FSpiPs_SetDFNum API has been modified to update
 * 		     user configuration information.
 * 		     The FSpiPs_SetSckDv API has been modified to update
 * 		     user configuration information.
 * 2.00 hzq 2023/03/23
 *
 * </pre>
 *
 ******************************************************************************/
#include "fmsh_spi.h"
#include "fmsh_spi_hw.h"

void FSpiPs_SetMst (FSpiPs_T* spiPtr, int master)
{
    u32 value = 0;

    if (master)
    {
        value = 1;
    }

    FMSH_WriteReg(spiPtr->config.base, SPI_MSTR_OFFSET, value);
}

void FSpiPs_SetEnable (FSpiPs_T* spiPtr, int enable)
{
    u32 value = 0;

    if (enable)
    {
        value = 1;
    }

    FMSH_WriteReg(spiPtr->config.base, SPI_SSIENR_OFFSET, value);
}

int FSpiPs_SetSckMode (FSpiPs_T* spiPtr, u32 mode)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_CTRLR0_OFFSET);

    value &= ~(SPI_CTRL0_SCPH_MASK | SPI_CTRL0_SCPOL_MASK);
    value |= (mode << SPI_CTRL0_SCPH_SHIFT);

    FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR0_OFFSET, value);

    return FMSH_SUCCESS;
}

int FSpiPs_SetTMod (FSpiPs_T* spiPtr, u32 tmod)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_CTRLR0_OFFSET);

    value &= ~SPI_CTRL0_TMOD_MASK;
    value |= (tmod << SPI_CTRL0_TMOD_SHIFT);

    FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR0_OFFSET, value);

    spiPtr->transfer_mode = tmod;

    return FMSH_SUCCESS;
}

int FSpiPs_SetSlvOut (FSpiPs_T* spiPtr, int enable)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_CTRLR0_OFFSET);

    if (enable)
    {
        value &= ~SPI_CTRL0_SLVOE_MASK;
    }
    else
    {
        value |= SPI_CTRL0_SLVOE_MASK;
    }

    FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR0_OFFSET, value);

    return FMSH_SUCCESS;
}

int FSpiPs_SetLoopBack (FSpiPs_T* spiPtr, int enable)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_CTRLR0_OFFSET);

    if (enable)
    {
        value |= SPI_CTRL0_SRL_MASK;
    }
    else
    {
        value &= ~SPI_CTRL0_SRL_MASK;
    }

    FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR0_OFFSET, value);

    return FMSH_SUCCESS;
}

int FSpiPs_SetDFS32 (FSpiPs_T* spiPtr, int dfs32)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_CTRLR0_OFFSET);

    value &= ~SPI_CTRL0_DFS32_MASK;
    value |= ((dfs32 - 1) << SPI_CTRL0_DFS32_SHIFT);

    FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR0_OFFSET, value);

    spiPtr->usercfg->frame_size = dfs32;

    return FMSH_SUCCESS;
}

int FSpiPs_SetDFNum (FSpiPs_T* spiPtr, int dfnum)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR1_OFFSET, dfnum - 1);

    spiPtr->usercfg->frame_len = dfnum;

    return FMSH_SUCCESS;
}

int FSpiPs_SetSlave (FSpiPs_T* spiPtr, u32 cs)
{
    u32 value;

    if (spiPtr->usercfg->flags & SPI_F_DECODE_CS)
    {
        value = 0x1 << cs;
    }
    else
    {
        value = cs;
    }

    FMSH_WriteReg(spiPtr->config.base, SPI_SER_OFFSET, value);

    return FMSH_SUCCESS;
}

int FSpiPs_SetSckDv (FSpiPs_T* spiPtr, int sckdv)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_BAUDR_OFFSET, sckdv);

    spiPtr->usercfg->baudrate = sckdv;

    return FMSH_SUCCESS;
}

int FSpiPs_SetTxEmptyLvl (FSpiPs_T* spiPtr, int tlvl)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_TXFTLR_OFFSET, tlvl);

    return FMSH_SUCCESS;
}

u32 FSpiPs_GetTxLevel (FSpiPs_T* spiPtr)
{
    return FMSH_ReadReg(spiPtr->config.base, SPI_TXFLR_OFFSET);
}

int FSpiPs_SetRxFullLvl (FSpiPs_T* spiPtr, int tlvl)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_RXFTLR_OFFSET, tlvl);

    return FMSH_SUCCESS;
}

u32 FSpiPs_GetRxLevel (FSpiPs_T* spiPtr)
{
    return FMSH_ReadReg(spiPtr->config.base, SPI_RXFLR_OFFSET);
}

void FSpiPs_EnableIntr (FSpiPs_T* spiPtr, u32 mask)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_IMR_OFFSET);

    value |= mask;

    FMSH_WriteReg(spiPtr->config.base, SPI_IMR_OFFSET, value);
}

void FSpiPs_DisableIntr (FSpiPs_T* spiPtr, u32 mask)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_IMR_OFFSET);

    value &= ~mask;

    FMSH_WriteReg(spiPtr->config.base, SPI_IMR_OFFSET, value);
}

void FSpiPs_ClearIntrStatus (FSpiPs_T* spiPtr)
{
    (void)FMSH_ReadReg(spiPtr->config.base, SPI_ICR_OFFSET);
}

void FSpiPs_SetDMATLvl (FSpiPs_T* spiPtr, int tlvl)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_DMATDLR_OFFSET, tlvl);
}

void FSpiPs_SetDMARLvl (FSpiPs_T* spiPtr, int tlvl)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_DMARDLR_OFFSET, tlvl);
}

void FSpiPs_EnableDMATx (FSpiPs_T* spiPtr)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_DMACR_OFFSET, 0x2);
}

void FSpiPs_EnableDMARx (FSpiPs_T* spiPtr)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_DMACR_OFFSET, 0x1);
}

void FSpiPs_DisableDMA (FSpiPs_T* spiPtr)
{
    FMSH_WriteReg(spiPtr->config.base, SPI_DMACR_OFFSET, 0);
}

u32 FSpiPs_GetStatus (FSpiPs_T* spiPtr)
{
    u32 value;

    value = FMSH_ReadReg(spiPtr->config.base, SPI_SR_OFFSET);

    return value;
}

u32 FSpiPs_Recv (FSpiPs_T* spiPtr)
{
    u32 value;
    /*
    u32 count = 0;
    u8 status;

    status = FMSH_ReadReg(spi->config.base, SPI_SR_OFFSET);
    while((status & SPI_SR_RFNE) == 0)  // loop if RX fifo empty
    {
        delay_us(1);
        count++;
        if(count > 10000)
        {
            break;
        }
        status = FMSH_ReadReg(spi->config.base, SPI_SR_OFFSET);
    }*/

    value = FMSH_ReadReg(spiPtr->config.base, SPI_DR_OFFSET);

    return value;
}

void FSpiPs_Send (FSpiPs_T* spiPtr, u32 data)
{
    /*
    u32 count = 0;
    u8 status;

    status = FMSH_ReadReg(spi->config.base, SPI_SR_OFFSET);
    while((status & SPI_SR_TFNF) == 0)   // loop if TX fifo full
    {
        delay_us(1);
        count++;
        if(count > 10000)
        {
            break;
        }
        status = FMSH_ReadReg(spi->config.base, SPI_SR_OFFSET);
    }*/

    FMSH_WriteReg(spiPtr->config.base, SPI_DR_OFFSET, data);
}
