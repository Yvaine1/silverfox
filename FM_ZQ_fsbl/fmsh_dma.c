/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_uart_logout.c
 *
 * This file contains boot_main.h.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/28/2022  First Release.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/**
**dma: 0(LPD DMA), 1(FPD DMA);
**channel: 0-7;
**src_addr: source address for dma transfer;
**dst_addr: destination address for dma transfer;
**tr_num: transfer data number(the unit is trans width);
**src_incr: axidma_burst_increment(src addr increases) , axidma_burst_fixed(src
*addr keep during transfer); *dst_incr: axidma_burst_increment(dst addr
*increases) , axidma_burst_fixed(dst addr keep during transfer);
**/
int FmshFsbl_InitMem (u64 SrcAddr, u64 DestAddr, u32 LengthBytes)
{
    u32 DMA_baseAddr;
    u32 val;
    int errorCode = FMSH_SUCCESS;
    int intr = 0;
    u32 channel = 0;
    u32 tr_num = LengthBytes / 16;
    u32 reg=0;
    u32 timeout = 1000;
    FAxidmaPsu_BurstType_E src_incr = axidma_burst_fixed;
    FAxidmaPsu_BurstType_E dst_incr = axidma_burst_increment;

        DMA_baseAddr = FPS_LPD_DMA_BASEADDR;
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x1C, 0);   // release reset
    reg=FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg|(1<<17)); 
    reg&=~(1<<17);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg);  // release reset


    FMSH_WriteReg(DMA_baseAddr, AXIDMA_CFGREG_OFFSET, 0);  // disable dma
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET +
                      (channel + 1) * AXIDMA_CHX_OFFSET,
                  0);  // disable channel irq
    FMSH_WriteReg(DMA_baseAddr, AXIDMA_CHENREG_L_OFFSET, 0);  // disable channel
    FMSH_WriteReg(DMA_baseAddr, AXIDMA_CFGREG_OFFSET, 0x3);   // enable dma

    // cfg
    val = 0;
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_CFG_L_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET,
                  val);
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_CFG_H_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET,
                  val);
    // sar
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_SAR_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET,
                  lower_32_bits(SrcAddr));
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_SAR_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET + 4,
                  upper_32_bits(SrcAddr));
    // dar
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_DAR_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET,
                  lower_32_bits(DestAddr));
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_DAR_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET + 4,
                  upper_32_bits(DestAddr));
    // blk ts
    FMSH_WriteReg(
        DMA_baseAddr,
        AXIDMA_CHX_BLOCK_TS_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET,
        tr_num - 1);
    FMSH_WriteReg(
        DMA_baseAddr,
        AXIDMA_CHX_BLOCK_TS_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET + 0x4,
        0);
    // ctl
    val = 0;
    val |= (src_incr << 4) | (dst_incr << 6);
    val |= (axidma_trans_width_128 << 8) | (axidma_trans_width_128 << 11);
    val |= (axidma_msize_16 << 14) | (axidma_msize_16 << 18);
    val |= (0x0 << 22) | (0x0 << 26);  // axcache
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_CTL_L_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET,
                  val);
    val |= (0x0 << 6) | (0x8 << 7) | (0x0 << 15) | (0x8 << 16);
    val |= (0x1 << 26) | (0x1 << 30) | (0x1 << 31);
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_CTL_H_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET,
                  val);
    // irq
    val = GENMASK(6, 0) | BIT(13);
    FMSH_WriteReg(DMA_baseAddr,
                  AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET +
                      (channel + 1) * AXIDMA_CHX_OFFSET,
                  val);
    // enable
    val = AXIDMA_CHENREG_CHX_EN_MASK(channel) |
          AXIDMA_CHENREG_CHX_EN_WE_MASK(channel);
    FMSH_WriteReg(DMA_baseAddr, AXIDMA_CHENREG_L_OFFSET, val);

    while (1)
    {
        intr = FMSH_ReadReg(
            DMA_baseAddr,
            AXIDMA_CHX_INTSTATUS_OFFSET + (channel + 1) * AXIDMA_CHX_OFFSET);

        if (intr & BIT(1))
        {
            FMSH_WriteReg(DMA_baseAddr,
                          AXIDMA_CHX_INTCLEARREG_OFFSET +
                              (channel + 1) * AXIDMA_CHX_OFFSET,
                          BIT(1));
            break;
        }
        if (intr & BIT(0))
        {
            FMSH_WriteReg(DMA_baseAddr,
                          AXIDMA_CHX_INTCLEARREG_OFFSET +
                              (channel + 1) * AXIDMA_CHX_OFFSET,
                          BIT(0));
        }
    }

    // wait enable self clr
    timeout = 100;
    while (FMSH_ReadReg(DMA_baseAddr, AXIDMA_CHENREG_L_OFFSET) &
           AXIDMA_CHENREG_CHX_EN_MASK(channel))
    {
        if (timeout-- == 0)
        {
            return -1;
        }
        delay_us(10);
    }

    return errorCode;
}
