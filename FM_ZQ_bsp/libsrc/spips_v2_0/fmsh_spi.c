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
 * @file fmsh_spi.c
 * @addtogroup spips_v2_0
 * @{
 *
 * Contains implements the interface functions of the FSpiPs driver.
 * See fmsh_spips.h for a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00  hzq 7/1/19
 * 		     First release
 * 1.10  hzq 11/26/20
 * 		     The string.h header has benn increased.
 * 		     The FSpiPs_InterruptHandler API has been modified to increase
 * 		     data transfer operation.
 * 		     The spi_dma_default has been increased which is a struct of
 * 		     dma related parameters.
 * 		     The spi_caps_default has been increased which is a struct of
 * 		     spi user configuration.
 * 		     The FSpiPs_Initialize API has been increased to initialize
 * 		     FSpiPs with default configuration.
 * 		     The FSpiPs_InitHw API has been increased to initialize
 * 		     FSpiPs with user defined configuration.
 * 		     The FSpiPs_Transfer API has been increased to transfer data.
 * 		     The FSpiPs_PolledTransfer API has been used to transfer data
 *            using poll.
 * 1.12 hzq 2022/03/04
 * 			 The FSpiPs_InterruptHandler API has been modified to support
 * 			 user handler completely
 * 2.00 hzq 2023/03/23
 *
 * </pre>
 *
 ******************************************************************************/
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_spi.h"
#include "fmsh_spi_hw.h"

static FSpiPs_UserCfg_T spi_usercfg_default = {
    .flags = SPI_F_MASTER,
    .tx_empty_lvl = 10,
    .rx_full_lvl = 0,
    .cpol = 0,
    .cpha = 0,
    .frame_size = 8,
    .frame_len = 256,
    .baudrate = 10,
    .sample_delay = 0,
};

static FSpiPs_Dma_T spi_dma_default = {
    .type = 0,
    .tx_if = 2,
    .rx_if = 3,
    .io = SPI_DR_OFFSET,
};

static FSpiPs_Msg_T ring_msg[2];
static u8 ring_cnt = 0;

static void StubStatusHandler (void* callBackRef, u32 statusEvent,
                               u32 byteCount)
{
    (void)callBackRef;
    (void)statusEvent;
    (void)byteCount;
}

int FSpiPs_CfgInitialize (FSpiPs_T* spiPtr, FSpiPs_Config_T* configPtr)
{
    FMSH_ASSERT(spiPtr != NULL);
    FMSH_ASSERT(configPtr != NULL);

    (void)memset(spiPtr, 0, sizeof(FSpiPs_T));

    // Set default value
    spiPtr->config.device_id = configPtr->device_id;
    spiPtr->config.base = configPtr->base;

    spiPtr->version = 0x0200;

    spiPtr->usercfg = &spi_usercfg_default;
    spiPtr->dma = &spi_dma_default;

    spiPtr->status_handler = StubStatusHandler;
    spiPtr->status_ref = NULL;

    spiPtr->priv = NULL;

    return FMSH_SUCCESS;
}

void FSpiPs_Reset (FSpiPs_T* spiPtr)
{
    u32 value;

    value = FMSH_ReadReg(0xff5e0000, 0x238);
    if (spiPtr->config.device_id == 0)
    {
        FMSH_WriteReg(0xff5e0000, 0x238, value | (0x1 << 3));
    }
    else if (spiPtr->config.device_id == 1)
    {
        FMSH_WriteReg(0xff5e0000, 0x238, value | (0x1 << 4));
    }
    else{
        ;/* no deal with */
    }
    
    delay_us(10);
    FMSH_WriteReg(0xff5e0000, 0x238, value);
}

int FSpiPs_SelfTest (FSpiPs_T* spiPtr)
{
    u32 value;

    FMSH_WriteReg(spiPtr->config.base, SPI_TXFTLR_OFFSET, 0xa);
    value = FMSH_ReadReg(spiPtr->config.base, SPI_TXFTLR_OFFSET);
    if (value != 0xa)
    {
        return FMSH_FAILURE;
    }

    FMSH_WriteReg(spiPtr->config.base, SPI_TXFTLR_OFFSET, 0x0);
    
    return FMSH_SUCCESS;
}

void FSpiPs_SetStatusHandler (FSpiPs_T* spiPtr, void* callBackRef,
                              FSpiPs_StatusHandler funcPtr)
{
    FMSH_ASSERT(spiPtr != NULL);
    FMSH_ASSERT(funcPtr != NULL);

    spiPtr->status_handler = funcPtr;
    spiPtr->status_ref = callBackRef;
}

void FSpiPs_InterruptHandler (void* instancePtr)
{
    FSpiPs_T* spiPtr = (FSpiPs_T*)instancePtr;
    FSpiPs_UserCfg_T* usercfg;
    u32 int_status;
    u32 rxw, txw;
    volatile int cnt;

    FMSH_ASSERT(instancePtr != NULL);

    if (!spiPtr->is_busy)
    {
        return;
    }

    /* Get & Clear interrupt status */
    int_status = FMSH_ReadReg(spiPtr->config.base, SPI_ISR_OFFSET);
    (void)FMSH_ReadReg(spiPtr->config.base, SPI_ICR_OFFSET);

    usercfg = spiPtr->usercfg;
    /* Multi-Master Fault */
    if (int_status & SPI_INTR_MSTIS_MASK)
    {
        spiPtr->status_handler(spiPtr, SPI_INTR_MSTIS_MASK, 0);
    }

    /* Check for overflow and underflow errors */
    if (int_status & SPI_INTR_RXOIS_MASK)
    {
        spiPtr->status_handler(spiPtr, SPI_INTR_RXOIS_MASK, 0);
    }

    if (int_status & SPI_INTR_RXUIS_MASK)
    {
        spiPtr->status_handler(spiPtr, SPI_INTR_RXUIS_MASK, 0);
    }

    if (int_status & SPI_INTR_TXOIS_MASK)
    {
        spiPtr->status_handler(spiPtr, SPI_INTR_TXOIS_MASK, 0);
    }

    /* Tx Empty */
    if (int_status & SPI_INTR_TXEIS_MASK)
    {
        if ((usercfg->flags & SPI_F_USR_INTRHANDLE) == 0)
        {
            int index;
            FSpiPs_Msg_T* msg;

            msg = spiPtr->msg;
            index = spiPtr->cur_msg;

            if (index < spiPtr->msg_num)
            {
                if ((msg[index].tx_bytes != 0) || (msg[index].rx_bytes != 0))
                {
                    cnt = FMSH_ReadReg(spiPtr->config.base, SPI_RXFLR_OFFSET);
                    while( (cnt > 0) && (msg[index].rx_bytes != 0) )
                    {
                        rxw = FSpiPs_Recv(spiPtr);
                        if (usercfg->frame_size == 8)
                        {
                            *(u8*)(msg[index].rx_buf) = (u8)rxw;
                        }
                        else if (usercfg->frame_size == 16)
                        {
                            *(u16*)(msg[index].rx_buf) = (u16)rxw;
                        }
                        else if (usercfg->frame_size == 32)
                        {
                            *(u32*)(msg[index].rx_buf) = (u32)rxw;
                        }
                        else{
                            ;/* no deal with */
                        }
                        
                        if (!(msg[index].flags & SPI_F_RX_ADDR_NOCHANGE))
                        {
                            msg[index].rx_buf += usercfg->frame_size >> 3;
                        }
                        msg[index].rx_bytes -= usercfg->frame_size >> 3;
                        cnt--;
                    }  // while end

                    cnt = SPI_FIFO_DEPTH - usercfg->tx_empty_lvl;
                    while( (cnt > 0) && (msg[index].tx_bytes != 0) )
                    {
                        txw = *(u32*)(msg[index].tx_buf);
                        FSpiPs_Send(spiPtr, txw);
                        if (!(msg[index].flags & SPI_F_TX_ADDR_NOCHANGE))
                        {
                            msg[index].tx_buf += usercfg->frame_size >> 3;
                        }
                        msg[index].tx_bytes -= usercfg->frame_size >> 3;
                        cnt--;
                    }  // while end
                }
                else
                {
                    spiPtr->cur_msg++;

                }  // if((msg[index].tx_bytes != 0) || (msg[index].rx_bytes !=
                   // 0)) end

            }  // if(index < spiPtr->msg_num) end

            else if (!(FSpiPs_GetStatus(spiPtr) & SPI_SR_BUSY))
            {
                /* Transfer complete, diable interrupt & spi*/
                FMSH_WriteReg(spiPtr->config.base, SPI_IMR_OFFSET, 0);

                FSpiPs_SetEnable(spiPtr, 0);

                if (usercfg->flags & SPI_F_GPIO_CS)
                {
                    FSpiPs_SlaveManually(spiPtr, 1);
                }

                spiPtr->is_busy = 0;

                /* inform upper layer*/
                spiPtr->status_handler(spiPtr, SPI_TRANSFER_DONE, 0);
            }
            else{
                ;/* no deal with */
            }
        }
        /*upper layer has something else to do*/
        spiPtr->status_handler(spiPtr, SPI_INTR_TXEIS_MASK, 0);
    }

    /* Rx FIFO Full */
    if (int_status & SPI_INTR_RXFIS_MASK)
    {
f1:
        if ((usercfg->flags & SPI_F_USR_INTRHANDLE) == 0)
        {
            int index;
            FSpiPs_Msg_T* msg;

            msg = spiPtr->msg;
            index = spiPtr->cur_msg;

            if (index < spiPtr->msg_num)
            {
                while ((FSpiPs_GetStatus(spiPtr) & SPI_SR_RFNE) &&
                       (msg[index].rx_bytes != 0) )
                {
                    rxw = FSpiPs_Recv(spiPtr);
                    if (usercfg->frame_size == 8)
                    {
                        *(u8*)(msg[index].rx_buf) = (u8)rxw;
                    }
                    else if (usercfg->frame_size == 16)
                    {
                        *(u16*)(msg[index].rx_buf) = (u16)rxw;
                    }
                    else if (usercfg->frame_size == 32)
                    {
                        *(u32*)(msg[index].rx_buf) = (u32)rxw;
                    }
                    else{
                        ;/* no deal with */
                    }

                    if (!(msg[index].flags & SPI_F_RX_ADDR_NOCHANGE))
                    {
                        msg[index].rx_buf += usercfg->frame_size >> 3;
                    }
                    msg[index].rx_bytes -= usercfg->frame_size >> 3;
                    cnt--;
                }  // while end
                if (msg[index].rx_bytes == 0)
                {
                    spiPtr->cur_msg++;
                    goto f1;
                }
            }
            else
            {
                /* Transfer complete, diable interrupt & spi*/
                FMSH_WriteReg(spiPtr->config.base, SPI_IMR_OFFSET, 0);

                FSpiPs_SetEnable(spiPtr, 0);

                if (usercfg->flags & SPI_F_GPIO_CS)
                {
                    FSpiPs_SlaveManually(spiPtr, 1);
                }

                spiPtr->is_busy = 0;

                /* inform upper layer*/
                spiPtr->status_handler(spiPtr, SPI_TRANSFER_DONE, 0);
            }
        }
        /*upper layer has something else to do*/
        spiPtr->status_handler(spiPtr, SPI_INTR_RXFIS_MASK, 0);
    }
}

int FSpiPs_Initialize (FSpiPs_T* spiPtr, u16 device_id)
{
    FSpiPs_Config_T* cfgPtr;

    FMSH_ASSERT(spiPtr != NULL);

    // get config info table from parameter
    cfgPtr = FSpiPs_LookupConfig(device_id);
    if (cfgPtr == NULL)
    {
        return FMSH_FAILURE;
    }
    // initialize controller struct
    return FSpiPs_CfgInitialize(spiPtr, cfgPtr);
}

int FSpiPs_HwInit (FSpiPs_T* spiPtr, FSpiPs_UserCfg_T* usercfg)
{
    u32 value;
    FSpiPs_UserCfg_T* cfg;

    FMSH_ASSERT(spiPtr != NULL);

    // Check whether there is another transfer in progress. Not thread-safe
    if (usercfg)
    {
        spiPtr->usercfg = usercfg;
    }

    cfg = spiPtr->usercfg;

    // reset hardware
    FSpiPs_Reset(spiPtr);

    // Disable spi
    FSpiPs_SetEnable(spiPtr, 0);

    // config spi as master or slave
    if (cfg->flags & SPI_F_MASTER)
    {
        FSpiPs_SetMst(spiPtr, 1);
    }
    else
    {
        FSpiPs_SetMst(spiPtr, 0);
    }

    value = 0;
    // config loop(none or internal or slcr)
    if (cfg->flags & SPI_F_LPBK)
    {
        value |= 0x1 << SPI_CTRL0_SRL_SHIFT;
    }
    // config spi cpol
    if (cfg->cpol)
    {
        value |= 0x1 << SPI_CTRL0_SCPOL_SHIFT;
    }
    // config spi cpha
    if (cfg->cpha)
    {
        value |= 0x1 << SPI_CTRL0_SCPH_SHIFT;
    }
    value |= (cfg->frame_size - 1) << SPI_CTRL0_DFS32_SHIFT;
    FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR0_OFFSET, value);

    // config baudrate & frame length
    if (cfg->flags & SPI_F_MASTER)
    {
        FMSH_WriteReg(spiPtr->config.base, SPI_BAUDR_OFFSET, cfg->baudrate);
        FMSH_WriteReg(spiPtr->config.base, SPI_CTRLR1_OFFSET,
                      cfg->frame_len - 1);
    }
    // config rx sample delay
    FMSH_WriteReg(spiPtr->config.base, SPI_RX_SAMPLE_OFFSET, cfg->sample_delay);

    // Config IMR
    FMSH_WriteReg(spiPtr->config.base, SPI_IMR_OFFSET, 0);

    // Config Tx/Rx Threshold
    FMSH_WriteReg(spiPtr->config.base, SPI_TXFTLR_OFFSET, cfg->tx_empty_lvl);
    FMSH_WriteReg(spiPtr->config.base, SPI_RXFTLR_OFFSET, cfg->rx_full_lvl);

    if (cfg->flags & SPI_F_DMA_EN)
    {
        FMSH_WriteReg(spiPtr->config.base, SPI_DMATDLR_OFFSET,
                      cfg->tx_empty_lvl);
        FMSH_WriteReg(spiPtr->config.base, SPI_DMARDLR_OFFSET,
                      cfg->rx_full_lvl);
    }

    // config slave selsct
    if (cfg->flags & SPI_F_MASTER)
    {
        if (cfg->flags & SPI_F_DECODE_CS)
        {
            FMSH_WriteReg(spiPtr->config.base, SPI_SER_OFFSET,
                          0x1 << spiPtr->cur_cs);
        }
        else
        {
            FMSH_WriteReg(spiPtr->config.base, SPI_SER_OFFSET, spiPtr->cur_cs);
        }
    }

    return FMSH_SUCCESS;
}

int FSpiPs_SlaveManually (FSpiPs_T* spiPtr, int select_n) { return 0; }

static int FSpiPs_DmaSetup (FSpiPs_T* spiPtr)
{
    // not implemented
    return FMSH_SUCCESS;
}

static int FSpiPs_DmaCleanup (FSpiPs_T* spiPtr)
{
    int timeout_us = 1000000;
    /* wait for data in tx fifo send */
    while (FSpiPs_GetStatus(spiPtr) & SPI_SR_BUSY)
    {
        if (timeout_us <= 0)
        {
            break;
        }
        delay_us(1);
        timeout_us--;
    }

    /* disable spi */
    FSpiPs_SetEnable(spiPtr, 0);

    if (spiPtr->usercfg->flags & SPI_F_GPIO_CS)
    {
        FSpiPs_SlaveManually(spiPtr, 1);
    }

    /* Clear the busy flag */
    spiPtr->is_busy = 0;

    return FMSH_SUCCESS;
}

static int FSpiPs_DmaTransfer (FSpiPs_T* spiPtr)
{
    u32* status_ref;

    status_ref = (u32*)spiPtr->status_ref;

    spiPtr->status_handler(spiPtr, SPI_DMA_TRANSFER, 0);
    if (*status_ref)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int FSpiPs_TransferMsg (FSpiPs_T* spiPtr, FSpiPs_Msg_T* msg, int msg_num)
{
    int ret;
    FSpiPs_UserCfg_T* usercfg;
    int index;
    u32 int_mask = 0;

    FMSH_ASSERT(spiPtr != NULL);
    FMSH_ASSERT(msg != NULL);
    FMSH_ASSERT(msg_num > 0);

    /* Check whether there is another transfer in progress. Not thread-safe */
    if (spiPtr->is_busy)
    {
        ret = FMSH_EBUSY;
        goto fail;
    }
    else
    {
        spiPtr->msg = msg;
        spiPtr->msg_num = msg_num;
        spiPtr->cur_msg = 0;

        usercfg = spiPtr->usercfg;

        index = 0;
        while (index < msg_num)
        {
            switch (spiPtr->transfer_mode)
            {
            case SPI_TRANSFER_STATE:
                if( (msg[index].tx_buf == NULL) || (msg[index].rx_buf == NULL) )
                {
                    return FMSH_EINVAL;
                }
                msg[index].tx_bytes = msg[index].total_bytes;
                msg[index].rx_bytes = msg[index].total_bytes;
                break;
            case SPI_TRANSMIT_ONLY_STATE:
                if (msg[index].tx_buf == NULL)
                {
                    return FMSH_EINVAL;
                }
                if (msg[index].tx_bytes == 0)
                {
                    msg[index].tx_bytes = msg[index].total_bytes;
                }
                msg[index].rx_bytes = 0;
                break;
            case SPI_RECEIVE_ONLY_STATE:
                if (msg[index].rx_buf == NULL)
                {
                    return FMSH_EINVAL;
                }
                if (msg[index].rx_bytes == 0)
                {
                    msg[index].rx_bytes = msg[index].total_bytes;
                }
                break;
            case SPI_EEPROM_STATE:
                if( (msg[index].tx_buf == NULL) || (msg[index].rx_buf == NULL) )
                {
                    return FMSH_EINVAL;
                }
                msg[index].tx_bytes = 3;
                msg[index].rx_bytes = msg[index].total_bytes;
                break;
            default:
                return FMSH_EINVAL;
            }

            index++;
        }

        /* disable spi */
        FSpiPs_SetEnable(spiPtr, 0);

        if (msg[0].total_bytes >= 0x10000)
        {
            FSpiPs_SetDFNum(spiPtr, 0xffff);
        }
        else
        {
            FSpiPs_SetDFNum(spiPtr, msg[0].total_bytes);
        }

        /* clear all interrupts */
        FSpiPs_DisableIntr(spiPtr, SPI_INTR_ALL);
        FSpiPs_ClearIntrStatus(spiPtr);

        if ((msg[0].flags & SPI_F_USE_POLL) == 0)
        {
            /* config dma & interrupts */
            if (usercfg->flags & SPI_F_DMA_EN)
            {
                if (msg[0].tx_buf != NULL)
                {
                    FSpiPs_EnableDMATx(spiPtr);
                }
                if (msg[0].rx_buf != NULL)
                {
                    FSpiPs_EnableDMARx(spiPtr);
                }
            }
            else if (usercfg->flags & SPI_F_INTR_EN)
            {
                int_mask |= SPI_INTR_TXEIS_MASK | SPI_INTR_RXFIS_MASK |
                            SPI_INTR_RXOIS_MASK | SPI_INTR_RXUIS_MASK |
                            SPI_INTR_TXOIS_MASK | SPI_INTR_MSTIS_MASK;
                if (msg[0].tx_bytes == 0)
                {
                    int_mask &= ~SPI_INTR_TXEIS_MASK;
                }

                FSpiPs_EnableIntr(spiPtr, int_mask);
            }
            else{
                ;/* no deal with */
            }
        }

        /* Set the busy flag, cleared when transfer is done */
        spiPtr->is_busy = 1;

        if (usercfg->flags & SPI_F_MASTER)
        {
            FSpiPs_SetSlave(spiPtr, spiPtr->cur_cs);
        }

        if (usercfg->flags & SPI_F_GPIO_CS)
        {
            FSpiPs_SlaveManually(spiPtr, 0);
        }

        /* enable spi to start transfer */
        FSpiPs_SetEnable(spiPtr, 1);

        /* start transfer */
        if ((msg[0].flags & SPI_F_USE_POLL) == 0)
        {
            if (usercfg->flags & SPI_F_DMA_EN)
            {
                FSpiPs_DmaSetup(spiPtr);
                while (spiPtr->cur_msg < spiPtr->msg_num)
                {
                    ret = FSpiPs_DmaTransfer(spiPtr);
                    if (ret)
                    {
                        break;
                    }
                }
                FSpiPs_DmaCleanup(spiPtr);
                if (ret)
                {
                    return FMSH_FAILURE;
                }
            }
        }

        if (msg[0].tx_bytes == 0)
        {
            /* Write one dummy data word to Tx FIFO */
            FSpiPs_Send(spiPtr, 0x0);
        }

        /* poll transfer */
        if ((msg[0].flags & SPI_F_USE_POLL) ||
            (((usercfg->flags & SPI_F_DMA_EN) == 0) &&
             ((usercfg->flags & SPI_F_INTR_EN) == 0)))
        {
            volatile int cnt;
            u32 txLvl;
            u32 rxw, txw;

            index = 0;
            while (index < msg_num)
            {
                /* polling tx fifo level until transfer complete */
                while( (msg[index].tx_bytes != 0) || (msg[index].rx_bytes != 0) )
                {
                    txLvl = FMSH_ReadReg(spiPtr->config.base, SPI_TXFLR_OFFSET);
                    if (txLvl <= usercfg->tx_empty_lvl)
                    {
                        /* read rx fifo first, get number of data in fifo */
                        cnt = FMSH_ReadReg(spiPtr->config.base,
                                           SPI_RXFLR_OFFSET);
                        while( (cnt > 0) && (msg[index].rx_bytes != 0) )
                        {
                            rxw = FSpiPs_Recv(spiPtr);
                            if (usercfg->frame_size == 8)
                            {
                                *(u8*)(msg[index].rx_buf) = (u8)rxw;
                            }
                            else if (usercfg->frame_size == 16)
                            {
                                *(u16*)(msg[index].rx_buf) = (u16)rxw;
                            }
                            else if (usercfg->frame_size == 32)
                            {
                                *(u32*)(msg[index].rx_buf) = (u32)rxw;
                            }
                            else{
                                ;/* no deal with */
                            }

                            if (!(msg[index].flags & SPI_F_RX_ADDR_NOCHANGE))
                            {
                                msg[index].rx_buf += usercfg->frame_size >> 3;
                            }
                            msg[index].rx_bytes -= usercfg->frame_size >> 3;
                            cnt--;
                        }
                        /* calculate the min number of data written to tx fifo
                         */
                        cnt = SPI_FIFO_DEPTH - usercfg->tx_empty_lvl;
                        while( (cnt > 0) && (msg[index].tx_bytes != 0) )
                        {
                            if (usercfg->frame_size == 8)
                            {
                                txw = *(u8*)(msg[index].tx_buf);
                            }
                            else if (usercfg->frame_size == 16)
                            {
                                txw = *(u16*)(msg[index].tx_buf);
                            }
                            else if (usercfg->frame_size == 32)
                            {
                                txw = *(u32*)(msg[index].tx_buf);
                            }
                            else{
                                ;/* no deal with */
                            }
                            FSpiPs_Send(spiPtr, txw);
                            if (!(msg[index].flags & SPI_F_TX_ADDR_NOCHANGE))
                            {
                                msg[index].tx_buf += usercfg->frame_size >> 3;
                            }
                            msg[index].tx_bytes -= usercfg->frame_size >> 3;
                            cnt--;
                        }
                    }
                } /* end transfer */

                index++;
            }
            
            delay_us(1);

            /* wait for data in tx fifo send */
            int timeout_us = 1000000;
            while (FSpiPs_GetStatus(spiPtr) & SPI_SR_BUSY)
            {
                if (timeout_us <= 0)
                {
                    break;
                }
                delay_us(1);
                timeout_us--;
            }

            /* disable spi */
            FSpiPs_SetEnable(spiPtr, 0);

            if (usercfg->flags & SPI_F_GPIO_CS)
            {
                FSpiPs_SlaveManually(spiPtr, 1);
            }

            /* Clear the busy flag */
            spiPtr->is_busy = 0;

        } /* end poll transfer*/
    }

    return FMSH_SUCCESS;

fail:
    return ret;
}

int FSpiPs_Transfer (FSpiPs_T* spiPtr, void* send_buf, void* recv_buf, int len)
{
    int ret;
    FSpiPs_Msg_T* msg = &ring_msg[ring_cnt];

    (void)memset(msg, 0, sizeof(FSpiPs_Msg_T));
    ring_cnt = (ring_cnt + 1) & 0x1;

    msg->flags = 0;
    msg->tx_buf = (u8*)send_buf;
    msg->rx_buf = (u8*)recv_buf;
    msg->total_bytes = len;

    ret = FSpiPs_TransferMsg(spiPtr, msg, 1);

    return ret;
}

int FSpiPs_PolledTransfer (FSpiPs_T* spiPtr, void* send_buf, void* recv_buf,
                           u32 len)
{
    int ret;
    FSpiPs_Msg_T* msg = &ring_msg[ring_cnt];
    ;

    (void)memset(msg, 0, sizeof(FSpiPs_Msg_T));
    ring_cnt = (ring_cnt + 1) & 0x1;

    msg->flags = SPI_F_USE_POLL;
    msg->tx_buf = (u8*)send_buf;
    msg->rx_buf = (u8*)recv_buf;
    msg->total_bytes = len;

    ret = FSpiPs_TransferMsg(spiPtr, msg, 1);

    return ret;
}
