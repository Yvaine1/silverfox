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
 * @file fmsh_axidmapsu_hw.h
 * @addtogroup axidmapsu_v1_0
 * @{
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who   Date        Changes
 * ----- ---- --------   ---------------------------------------------
 * 1.00  whn 09/27/2024  First Release
 *
 *</pre>
 *
 ******************************************************************************/
#ifndef _FMSH_AXIDMAPSU_HW_H_
#define _FMSH_AXIDMAPSU_HW_H_

/***************************** Include Files *********************************/
#include "fmsh_parameters.h"

/************************** Constant Definitions *****************************/

/***** define the common register address *****/
#define AXIDMA_IDREG_OFFSET                         0x0
#define AXIDMA_COMPVERREG_OFFSET                    0x8
#define AXIDMA_CFGREG_OFFSET                        0x10
#define AXIDMA_CHENREG_L_OFFSET                     0x18
#define AXIDMA_CHENREG_H_OFFSET                     0x1C
#define AXIDMA_INTSTATUSREG_OFFSET                  0x30
#define AXIDMA_COMMONREG_INTCLEARREG_OFFSET         0x38
#define AXIDMA_COMMONREG_INTSTATUS_ENABLEREG_OFFSET 0x40
#define AXIDMA_COMMONREG_INTSIGNAL_ENABLEREG_OFFSET 0x48
#define AXIDMA_COMMONREG_INTSTATUSREG_OFFSET        0x50
#define AXIDMA_RESETREG_OFFSET                      0x58
#define AXIDMA_LOWPOWER_CFGREG_OFFSET               0x60

/***** AXIDMA_CFGREG *****/
#define AXIDMA_CFGREG_DMAC_EN_MASK BIT(0)
#define AXIDMA_CFGREG_INT_EN_MASK  BIT(1)

/***** AXIDMA_CHENREG, x = 0~7 *****/
#define AXIDMA_CHENREG_CHX_EN_MASK(i)       BIT(i)
#define AXIDMA_CHENREG_CHX_EN_WE_MASK(i)    (BIT(i) << 8)
#define AXIDMA_CHENREG_CHX_SUSP_MASK(i)     (BIT(i) << 16)
#define AXIDMA_CHENREG_CHX_SUSP_WE_MASK(i)  (BIT(i) << 24)
#define AXIDMA_CHENREG_CHX_ABORT_MASK(i)    (BIT(i) << 0)
#define AXIDMA_CHENREG_CHX_ABORT_WE_MASK(i) (BIT(i) << 8)

/***** AXIDMA_RESETREG *****/
#define AXIDMA_RST_MASK BIT(0)

/***** AXIDMA_LOWPOWER_CFGREG *****/
#define AXIDMA_LOWPOWER_CFGREG_GBL_CSLP_EN_MASK   BIT(0)
#define AXIDMA_LOWPOWER_CFGREG_CHNL_CSLP_EN_MASK  BIT(1)
#define AXIDMA_LOWPOWER_CFGREG_SBIUL_CSLP_EN_MASK BIT(2)
#define AXIDMA_LOWPOWER_CFGREG_MXIF_CSLP_EN_MASK  BIT(3)
#define AXIDMA_LOWPOWER_CFGREG_GLCH_LPDLY_MASK    (0xFF << 32)
#define AXIDMA_LOWPOWER_CFGREG_SBIU_LPDLY_MASK    (0xFF << 40)
#define AXIDMA_LOWPOWER_CFGREG_MXIF_LPDLY_MASK    (0xFF << 48)

/***** define the channel register address, i = 0~7 *****/
#define AXIDMA_CHX_OFFSET                      FPAR_AXIDMAPSU_CHX_OFFSET
#define AXIDMA_CHX_SAR_OFFSET                  0x00
#define AXIDMA_CHX_DAR_OFFSET                  0x08
#define AXIDMA_CHX_BLOCK_TS_OFFSET             0x10
#define AXIDMA_CHX_CTL_L_OFFSET                0x18
#define AXIDMA_CHX_CTL_H_OFFSET                0x1C
#define AXIDMA_CHX_CFG_L_OFFSET                0x20
#define AXIDMA_CHX_CFG_H_OFFSET                0x24
#define AXIDMA_CHX_LLP_OFFSET                  0x28
#define AXIDMA_CHX_STATUSREG_OFFSET            0x30
#define AXIDMA_CHX_SWHSSRCREG_OFFSET_OFFSET    0x38
#define AXIDMA_CHX_SWHSDSTREG_OFFSET_OFFSET    0x40
#define AXIDMA_CHX_BLK_TFR_RESUMEREQREG_OFFSET 0x48
#define AXIDMA_CHX_AXI_IDREG_OFFSET            0x50
#define AXIDMA_CHX_AXI_QOSREG_OFFSET           0x58
#define AXIDMA_CHX_SSTAT_OFFSET                0x60
#define AXIDMA_CHX_DSTAT_OFFSET                0x68
#define AXIDMA_CHX_SSTATAR_OFFSET              0x70
#define AXIDMA_CHX_DSTATAR_OFFSET              0x78
#define AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET  0x80
#define AXIDMA_CHX_INTSTATUS_OFFSET            0x88
#define AXIDMA_CHX_INTSIGNAL_ENABLEREG_OFFSET  0x90
#define AXIDMA_CHX_INTCLEARREG_OFFSET          0x98

/***** Channels registers bit mask *****/
#define AXIDMA_CHX_BLOCK_TS_MASK (0x3FFFFF)

#define AXIDMA_CHX_CTL_SINC_MASK                   BIT(4)
#define AXIDMA_CHX_CTL_DINC_MASK                   BIT(6)
#define AXIDMA_CHX_CTL_SRC_TR_WIDTH_MASK           (0x7 << 8)
#define AXIDMA_CHX_CTL_DST_TR_WIDTH_MASK           (0x7 << 11)
#define AXIDMA_CHX_CTL_SRC_MSIZE_MASK              (0xF << 14)
#define AXIDMA_CHX_CTL_DST_MSIZE_MASK              (0xF << 18)
#define AXIDMA_CHX_CTL_AR_CACHE_MASK               (0xF << 22)
#define AXIDMA_CHX_CTL_AW_CACHE_MASK               (0xF << 26)
#define AXIDMA_CHX_CTL_NONPOSTED_LASTWRITE_EN_MASK BIT(30)
#define AXIDMA_CHX_CTL_AR_PROT_MASK                (0x7 << 0)
#define AXIDMA_CHX_CTL_AW_PROT_MASK                (0x7 << 3)
#define AXIDMA_CHX_CTL_ARLEN_EN_MASK               BIT(6)
#define AXIDMA_CHX_CTL_ARLEN_MASK                  (0xFF << 7)
#define AXIDMA_CHX_CTL_AWLEN_EN_MASK               BIT(15)
#define AXIDMA_CHX_CTL_AWLEN_MASK                  (0xFF << 16)
#define AXIDMA_CHX_CTL_SRC_STAT_EN_MASK            BIT(24)
#define AXIDMA_CHX_CTL_DST_STAT_EN_MASK            BIT(25)
#define AXIDMA_CHX_CTL_IOC_BLKTFR_MASK             BIT(26)
#define AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_MASK  BIT(30)
#define AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_MASK BIT(31)

#define AXIDMA_CHX_CTL_SINC_SHIFT                   4
#define AXIDMA_CHX_CTL_DINC_SHIFT                   6
#define AXIDMA_CHX_CTL_SRC_TR_WIDTH_SHIFT           8
#define AXIDMA_CHX_CTL_DST_TR_WIDTH_SHIFT           11
#define AXIDMA_CHX_CTL_SRC_MSIZE_SHIFT              14
#define AXIDMA_CHX_CTL_DST_MSIZE_SHIFT              18
#define AXIDMA_CHX_CTL_AR_CACHE_SHIFT               22
#define AXIDMA_CHX_CTL_AW_CACHE_SHIFT               26
#define AXIDMA_CHX_CTL_NONPOSTED_LASTWRITE_EN_SHIFT 30

#define AXIDMA_CHX_CTL_AR_PROT_SHIFT                0
#define AXIDMA_CHX_CTL_AW_PROT_SHIFT                3
#define AXIDMA_CHX_CTL_ARLEN_EN_SHIFT               6
#define AXIDMA_CHX_CTL_ARLEN_SHIFT                  7
#define AXIDMA_CHX_CTL_AWLEN_EN_SHIFT               15
#define AXIDMA_CHX_CTL_AWLEN_SHIFT                  16
#define AXIDMA_CHX_CTL_SRC_STAT_EN_SHIFT            24
#define AXIDMA_CHX_CTL_DST_STAT_EN_SHIFT            25
#define AXIDMA_CHX_CTL_IOC_BLKTFR_SHIFT             26
#define AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_SHIFT  30
#define AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_SHIFT 31

#define AXIDMA_CHX_CFG_SRC_MULTBLK_TYPE_MASK (0x3 << 0)
#define AXIDMA_CHX_CFG_DST_MULTBLK_TYPE_MASK (0x3 << 2)
#define AXIDMA_CHX_CFG_TT_FC_MASK            (0x7 << 0)
#define AXIDMA_CHX_CFG_HG_SEL_SRC_MASK       BIT(3)
#define AXIDMA_CHX_CFG_HG_SEL_DST_MASK       BIT(4)
#define AXIDMA_CHX_CFG_SRC_HWHS_POL_MASK     BIT(5)
#define AXIDMA_CHX_CFG_DST_HWHS_POL_MASK     BIT(6)
#define AXIDMA_CHX_CFG_SRC_PER_MASK          (0x7 << 7)
#define AXIDMA_CHX_CFG_DST_PER_MASK          (0x7 << 12)
#define AXIDMA_CHX_CFG_CH_PRIOR_MASK         (0x7 << 17)
#define AXIDMA_CHX_CFG_LOCK_CH_MASK          BIT(20)
#define AXIDMA_CHX_CFG_LOCK_CH_L_MASK        BIT(21)
#define AXIDMA_CHX_CFG_SRC_OSR_LMT_MASK      (0xF << 23)
#define AXIDMA_CHX_CFG_DST_OSR_LMT_MASK      (0xF << 27)

#define AXIDMA_CHX_CFG_SRC_MULTBLK_TYPE_SHIFT 0
#define AXIDMA_CHX_CFG_DST_MULTBLK_TYPE_SHIFT 2
#define AXIDMA_CHX_CFG_TT_FC_SHIFT            0
#define AXIDMA_CHX_CFG_HG_SEL_SRC_SHIFT       3
#define AXIDMA_CHX_CFG_HG_SEL_DST_SHIFT       4
#define AXIDMA_CHX_CFG_SRC_HWHS_POL_SHIFT     5
#define AXIDMA_CHX_CFG_DST_HWHS_POL_SHIFT     6
#define AXIDMA_CHX_CFG_SRC_PER_SHIFT          7
#define AXIDMA_CHX_CFG_DST_PER_SHIFT          12
#define AXIDMA_CHX_CFG_CH_PRIOR_SHIFT         17
#define AXIDMA_CHX_CFG_LOCK_CH_SHIFT          20
#define AXIDMA_CHX_CFG_LOCK_CH_L_SHIFT        21
#define AXIDMA_CHX_CFG_SRC_OSR_LMT_SHIFT      23
#define AXIDMA_CHX_CFG_DST_OSR_LMT_SHIFT      27

#define AXIDMA_CHX_LLP_L_LMS_MASK   BIT(0)
#define AXIDMA_CHX_LLP_L_LOC_L_MASK 0xFFFFFFC0
#define AXIDMA_CHX_LLP_H_LOC_H_MASK 0xFFFFFFFF

#define AXIDMA_CHX_BLK_TFR_RESUMEREQ_MASK BIT(0)

#define AXIDMA_CHX_SWHS_REQ_MASK        BIT(0)
#define AXIDMA_CHX_SWHS_REQ_WE_MASK     BIT(1)
#define AXIDMA_CHX_SWHS_SGLREQ_MASK     BIT(2)
#define AXIDMA_CHX_SWHS_SGLREQ_WE_MASK  BIT(3)
#define AXIDMA_CHX_SWHS_LSTREQ_MASK     BIT(4)
#define AXIDMA_CHX_SWHS_LSTREQ_WE_MASK  BIT(5)
#define AXIDMA_CHX_SWHS_REQ_SHIFT       0
#define AXIDMA_CHX_SWHS_REQ_WE_SHIFT    1
#define AXIDMA_CHX_SWHS_SGLREQ_SHIFT    2
#define AXIDMA_CHX_SWHS_SGLREQ_WE_SHIFT 3
#define AXIDMA_CHX_SWHS_LST_SHIFT       4
#define AXIDMA_CHX_SWHS_LST_WE_SHIFT    5

#define AXIDMA_CHX_AXI_QOSREG_AWQOS_MASK 0x0F
#define AXIDMA_CHX_AXI_QOSREG_ARQOS_MASK 0xF0

#define AXIDMA_CHX_AXI_QOSREG_AWQOS_SHIFT 0
#define AXIDMA_CHX_AXI_QOSREG_ARQOS_SHIFT 4

/*****************************************************************************
 * AXI DMA common interrupts
 *
 * @AXIDMA_IRQ_CMN_NONE: Bitmask of no one interrupt
 * @AXIDMA_IRQ_CMN_SLVIF_DEC_ERR: Slave interface common register decode error
 * @AXIDMA_IRQ_CMN_SLVIF_WR2RO_ERR: Slave interface common register write to
 * read only error
 * @AXIDMA_IRQ_CMN_SLVIF_RD2WO_ERR: Slave interface common register read to
 * write only error
 * @AXIDMA_IRQ_CMN_SLVIF_WRONHOLD_ERR: Slave interface common register write on
 * hold error
 * @AXIDMA_IRQ_CMN_SLVIF_UNDEFINEDREG_DEC_ERR: Slave interface undefined
 * register decode error
 * @AXIDMA_IRQ_CMN_ALL_ERR: Bitmask of all interrupts
 *****************************************************************************/
#define AXIDMA_IRQ_CMN_NONE_MASK                       0
#define AXIDMA_IRQ_CMN_SLVIF_DEC_ERR_MASK              BIT(0)
#define AXIDMA_IRQ_CMN_SLVIF_WR2RO_ERR_MASK            BIT(1)
#define AXIDMA_IRQ_CMN_SLVIF_RD2WO_ERR_MASK            BIT(2)
#define AXIDMA_IRQ_CMN_SLVIF_WRONHOLD_ERR_MASK         BIT(3)
#define AXIDMA_IRQ_CMN_SLVIF_UNDEFINEDREG_DEC_ERR_MASK BIT(8)
#define AXIDMA_IRQ_CMN_ALL_ERR_MASK                    (BIT(8) | GENMASK(3, 0))

/*****************************************************************************
 * AXI DMA channel interrupts
 *
 * @AXIDMA_IRQ_CH_NONE: Bitmask of no one interrupt
 * @AXIDMA_IRQ_CH_BLOCK_TRF_DONE: Block transfer complete
 * @AXIDMA_IRQ_CH_DMA_TFR_DONE: Dma transfer complete
 * @AXIDMA_IRQ_CH_SRC_TRANSCOMP: Source transaction complete
 * @AXIDMA_IRQ_CH_DST_TRANSCOMP: Destination transaction complete
 * @AXIDMA_IRQ_CH_SRC_DEC_ERR: Source decode error
 * @AXIDMA_IRQ_CH_DST_DEC_ERR: Destination decode error
 * @AXIDMA_IRQ_CH_SRC_SLV_ERR: Source slave error
 * @AXIDMA_IRQ_CH_DST_SLV_ERR: Destination slave error
 * @AXIDMA_IRQ_CH_LLI_RD_DEC_ERR: LLI read decode error
 * @AXIDMA_IRQ_CH_LLI_WR_DEC_ERR: LLI write decode error
 * @AXIDMA_IRQ_CH_LLI_RD_SLV_ERR: LLI read slave error
 * @AXIDMA_IRQ_CH_LLI_WR_SLV_ERR: LLI write slave error
 * @AXIDMA_IRQ_CH_SHADOWREG_OR_LLI_INVALID_ERR: LLI invalid error or Shadow
 * register error
 * @AXIDMA_IRQ_CH_SLVIF_MULTIBLKTYPE_ERR: Slave Interface Multiblock type error
 * @AXIDMA_IRQ_CH_SLVIF_DEC_ERR: Slave Interface decode error
 * @AXIDMA_IRQ_CH_SLVIF_WR2RO_ERR: Slave Interface write to read only error
 * @AXIDMA_IRQ_CH_SLVIF_RD2RWO_ERR: Slave Interface read to write only error
 * @AXIDMA_IRQ_CH_SLVIF_WRONCHEN_ERR: Slave Interface write to channel error
 * @AXIDMA_IRQ_CH_SLVIF_SHADOWREG_WRON_VALID_ERR: Slave Interface shadow reg
 * error
 * @AXIDMA_IRQ_CH_SLVIF_WRONHOLD_ERR: Slave Interface hold error
 * @AXIDMA_IRQ_CH_LOCK_CLEARED: Lock Cleared Status
 * @AXIDMA_IRQ_CH_SRC_SUSPENDED: Source Suspended Status
 * @AXIDMA_IRQ_CH_SUSPENDED: Channel Suspended Status
 * @AXIDMA_IRQ_CH_DISABLED: Channel Disabled Status
 * @AXIDMA_IRQ_CH_ABORTED: Channel Aborted Status
 * @AXIDMA_IRQ_CH_ALL_ERR: Bitmask of all error interrupts
 * @AXIDMA_IRQ_CH_ALL_TRF: Bitmask of all transfer related interrupts
 * @AXIDMA_IRQ_CH_ALL: Bitmask of all interrupts
 *****************************************************************************/
#define AXIDMA_IRQ_CH_NONE_MASK                           0
#define AXIDMA_IRQ_CH_BLOCK_TRF_DONE_MASK                 BIT(0)
#define AXIDMA_IRQ_CH_DMA_TFR_DONE_MASK                   BIT(1)
#define AXIDMA_IRQ_CH_SRC_TRANSCOMP_MASK                  BIT(3)
#define AXIDMA_IRQ_CH_DST_TRANSCOMP_MASK                  BIT(4)
#define AXIDMA_IRQ_CH_SRC_DEC_ERR_MASK                    BIT(5)
#define AXIDMA_IRQ_CH_DST_DEC_ERR_MASK                    BIT(6)
#define AXIDMA_IRQ_CH_SRC_SLV_ERR_MASK                    BIT(7)
#define AXIDMA_IRQ_CH_DST_SLV_ERR_MASK                    BIT(8)
#define AXIDMA_IRQ_CH_LLI_RD_DEC_ERR_MASK                 BIT(9)
#define AXIDMA_IRQ_CH_LLI_WR_DEC_ERR_MASK                 BIT(10)
#define AXIDMA_IRQ_CH_LLI_RD_SLV_ERR_MASK                 BIT(11)
#define AXIDMA_IRQ_CH_LLI_WR_SLV_ERR_MASK                 BIT(12)
#define AXIDMA_IRQ_CH_SHADOWREG_OR_LLI_INVALID_ERR_MASK   BIT(13)
#define AXIDMA_IRQ_CH_SLVIF_MULTIBLKTYPE_ERR_MASK         BIT(14)
#define AXIDMA_IRQ_CH_SLVIF_DEC_ERR_MASK                  BIT(16)
#define AXIDMA_IRQ_CH_SLVIF_WR2RO_ERR_MASK                BIT(17)
#define AXIDMA_IRQ_CH_SLVIF_RD2RWO_ERR_MASK               BIT(18)
#define AXIDMA_IRQ_CH_SLVIF_WRONCHEN_ERR_MASK             BIT(19)
#define AXIDMA_IRQ_CH_SLVIF_SHADOWREG_WRON_VALID_ERR_MASK BIT(20)
#define AXIDMA_IRQ_CH_SLVIF_WRONHOLD_ERR_MASK             BIT(21)
#define AXIDMA_IRQ_CH_LOCK_CLEARED_MASK                   BIT(27)
#define AXIDMA_IRQ_CH_SRC_SUSPENDED_MASK                  BIT(28)
#define AXIDMA_IRQ_CH_SUSPENDED_MASK                      BIT(29)
#define AXIDMA_IRQ_CH_DISABLED_MASK                       BIT(30)
#define AXIDMA_IRQ_CH_ABORTED_MASK                        BIT(31)
#define AXIDMA_IRQ_CH_ALL_ERR_MASK                        (GENMASK(21, 16) | GENMASK(14, 5))
#define AXIDMA_IRQ_CH_ALL_TRF_MASK \
    (GENMASK(31, 27) | GENMASK(4, 3) | GENMASK(1, 0))
#define AXIDMA_IRQ_CH_ALL_MASK GENMASK(31, 0)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#endif /* #ifndef _FMSH_AXIDMAPSU_HW_H_ */