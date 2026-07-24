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
 * @file fmsh_spi_eeprom.c
 * @addtogroup spips_v2_0
 * @{
 *
 * Contains implements the spi eeprom flash interface functions of
 * the FSpiPs driver.
 * See fmsh_spips_eeprom.h for a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.10  hzq 11/26/20
 * 		     First release
 * 2.00 hzq 2023/03/23
 *
 * </pre>
 *
 ******************************************************************************/
#include "fmsh_common.h"
#include "fmsh_spi.h"
#include "fmsh_spi_eeprom.h"
#include "fmsh_spi_hw.h"

int FSpiPs_ReadUid (FSpiPs_T* spiPtr, u8* uid)
{
    u8 tx_buffer[3];

    tx_buffer[0] = 0x83;
    tx_buffer[1] = 0x02;
    tx_buffer[2] = 0x00;

    FSpiPs_SetTMod(spiPtr, SPI_EEPROM_STATE);

    FSpiPs_PolledTransfer(spiPtr, &tx_buffer[0], &uid[0], 16);

    return FMSH_SUCCESS;
}

int FSpiPs_ReadLockStatus (FSpiPs_T* spiPtr)
{
    u8 tx_buffer[3];
    u8 data;

    tx_buffer[0] = 0x83;
    tx_buffer[1] = 0x4;
    tx_buffer[2] = 0x0;

    FSpiPs_SetTMod(spiPtr, SPI_EEPROM_STATE);

    FSpiPs_PolledTransfer(spiPtr, &tx_buffer[0], &data, 1);

    if ((data & 0x02) == 0x02)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int FSpiPs_WREN (FSpiPs_T* spiPtr)
{
    int timeout_us = 5000;
    u8 tx_buffer[1];
    u8 status;

    tx_buffer[0] = 0x06;

    FSpiPs_SetTMod(spiPtr, SPI_TRANSMIT_ONLY_STATE);

    FSpiPs_PolledTransfer(spiPtr, &tx_buffer[0], NULL, 1);

    while (1)
    {
        status = FSpiPs_ReadStatus(spiPtr);
        if ((status & 0x2) == 0x2)
        {
            break;
        }

        delay_1us();
        timeout_us--;

        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }
    }
    return FMSH_SUCCESS;
}

int FSpiPs_WRDI (FSpiPs_T* spiPtr)
{
    int timeout_us = 5000;
    u8 tx_buffer[1];
    u8 status;

    tx_buffer[0] = 0x04;

    FSpiPs_SetTMod(spiPtr, SPI_TRANSMIT_ONLY_STATE);

    FSpiPs_PolledTransfer(spiPtr, &tx_buffer[0], NULL, 1);

    while (1)
    {
        status = FSpiPs_ReadStatus(spiPtr);
        if ((status & 0x2) == 0x0)
        {
            break;
        }

        delay_1us();
        timeout_us--;

        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }
    }
    return FMSH_SUCCESS;
}

u8 FSpiPs_ReadStatus (FSpiPs_T* spiPtr)
{
    u8 tx_buffer[3];
    u8 status;

    tx_buffer[0] = 0x05;
    tx_buffer[1] = 0x00;
    tx_buffer[2] = 0x00;

    FSpiPs_SetTMod(spiPtr, SPI_EEPROM_STATE);

    FSpiPs_PolledTransfer(spiPtr, &tx_buffer[0], &status, 1);

    return status;
}

int FSpiPs_WriteArray (FSpiPs_T* spiPtr, u16 address, u8* data, int len)
{
    int ret;
    int timeout_us = 5000;
    u8 status;

    ret = FSpiPs_WREN(spiPtr);
    if (ret)
    {
        return FMSH_EIO;
    }

    data[0] = 0x02;
    data[1] = (address >> 8) & 0xff;
    data[2] = (address & 0xff);

    FSpiPs_SetTMod(spiPtr, SPI_TRANSMIT_ONLY_STATE);

    FSpiPs_PolledTransfer(spiPtr, &data[0], NULL, len + 3);

    while (1)
    {
        status = FSpiPs_ReadStatus(spiPtr);
        if ((status & 0x1) == 0x0)
        {
            break;
        }

        delay_1us();
        timeout_us--;

        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }
    }

    return FMSH_SUCCESS;
}

int FSpiPs_ReadArray (FSpiPs_T* spiPtr, u16 address, u8* data, int len)
{
    u8 tx_buffer[3];

    tx_buffer[0] = 0x03;
    tx_buffer[1] = (address >> 8) & 0xff;
    tx_buffer[2] = (address & 0xff);

    FSpiPs_SetTMod(spiPtr, SPI_EEPROM_STATE);

    FSpiPs_PolledTransfer(spiPtr, &tx_buffer[0], &data[0], len);

    return FMSH_SUCCESS;
}
