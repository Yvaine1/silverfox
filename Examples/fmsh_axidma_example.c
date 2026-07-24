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
 * @file fmsh_axidma_example.c
 * @{
 *
 * Contains example of the FAxidmaPsu driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ---  --------   -----------------------------------------------
 * 1.00  whn  2025/02/24  First Release.
 * 		         
 * </pre>
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "fmsh_axidma_example.h"

/************************** Constant Definitions *****************************/

typedef struct
{
    uintptr_t sar;
    uintptr_t dar;
    u32 src_init_data;
    u32 dst_init_data;
    u32 block_ts;
} axidma_transfer_test_t;

/* for single block transfer */
#define TESTDATA1 0x12345678

/* for multiple block transfer */
#define BLOCK_TS0 64
#define BLOCK_TS1 128
#define BLOCK_TS2 256
u32 src_buf0[BLOCK_TS0 * 4] __attribute__((aligned(128)));
u32 dst_buf0[BLOCK_TS0 * 4] __attribute__((aligned(128)));
u32 src_buf1[BLOCK_TS1 * 4] __attribute__((aligned(128)));
u32 dst_buf1[BLOCK_TS1 * 4] __attribute__((aligned(128)));
u32 src_buf2[BLOCK_TS2 * 4] __attribute__((aligned(128)));
u32 dst_buf2[BLOCK_TS2 * 4] __attribute__((aligned(128)));

/* prepare data */
axidma_transfer_test_t axidma_transfer[3] = {
    {.sar = (uintptr_t)src_buf0,
     .dar = (uintptr_t)dst_buf0,
     .src_init_data = 0x12345678,
     .dst_init_data = 0x9abcdef1,
     .block_ts = BLOCK_TS0},
    {.sar = (uintptr_t)src_buf1,
     .dar = (uintptr_t)dst_buf1,
     .src_init_data = 0x13579bdf,
     .dst_init_data = 0x33445566,
     .block_ts = BLOCK_TS1},
    {.sar = (uintptr_t)src_buf2,
     .dar = (uintptr_t)dst_buf2,
     .src_init_data = 0x5a5a5a5a,
     .dst_init_data = 0xa5a5a5a5,
     .block_ts = BLOCK_TS2},
};


axidma_transfer_test_t axidma_reload_transfer[3] = {
    {.sar = (uintptr_t)src_buf0,
     .dar = (uintptr_t)dst_buf0,
     .src_init_data = 0x12345678,
     .dst_init_data = 0x9abcdef1,
     .block_ts = BLOCK_TS0},
    {.sar = (uintptr_t)src_buf0,
     .dar = (uintptr_t)dst_buf0,
     .src_init_data = 0x13579bdf,
     .dst_init_data = 0x33445566,
     .block_ts = BLOCK_TS0},
    {.sar = (uintptr_t)src_buf0,
     .dar = (uintptr_t)dst_buf0,
     .src_init_data = 0x5a5a5a5a,
     .dst_init_data = 0xa5a5a5a5,
     .block_ts = BLOCK_TS0},
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
static int axidma_transfer_done = 0;
static int axidma_issued_blocks = 0;
static int axidma_transfered_blocks = 0;
static int axidma_num_blocks = 3;
static int axidma_reload_check_status = 0;

#define AXIDMA_SGLBLK_EXAMPLE 0
#define AXIDMA_LLI_EXAMPLE    1
#define AXIDMA_RELOAD_EXAMPLE 2
#define AXIDMA_SHAREG_EXAMPLE 3

u32 axidma_example = 0;

FAxidmaPsu_T g_AxiDma;

FAxidmaPsu_Lli_T lli_entry[3];

/******************************************************************************
 * @description: this function is for initializing the transfer blocks.
 *
 *
 ******************************************************************************/
static void axidma_memory_prepare(axidma_transfer_test_t *transfer,
                                  u32 block_num)
{
    for (int i = 0; i < block_num; i++)
    {
        axidma_transfer_test_t *curr_transfer_ptr = transfer + i;
        for (int j = 0; j < (curr_transfer_ptr->block_ts) * 4; j++)
        {
            *((u32 *)curr_transfer_ptr->sar + j) =
                curr_transfer_ptr->src_init_data + j;
            *((u32 *)curr_transfer_ptr->dar + j) =
                curr_transfer_ptr->dst_init_data + j;
        }
    }

    Fmsh_DCacheFlush();
    dsb();
}

/******************************************************************************
 * @description: this function is for checking the transfer results.
 *
 *
 ******************************************************************************/
static int axidma_memory_compare(axidma_transfer_test_t *transfer,
                                 u32 block_num)
{
    for (int i = 0; i < block_num; i++)
    {
        axidma_transfer_test_t *curr_transfer_ptr = transfer + i;
        for (int j = 0; j < (curr_transfer_ptr->block_ts) * 4; j++)
        {
            if (*((u32 *)curr_transfer_ptr->dar + j) !=
                curr_transfer_ptr->src_init_data + j)
            {
                return FMSH_FAILURE;
            }
        }
    }

    return FMSH_SUCCESS;
}

/******************************************************************************
 * @description: this function is an example of dma interrupt register in gic.
 *
 *
 ******************************************************************************/
int fmsh_axidma_setup_interrupt_system(FAxidmaPsu_T *InstancePtr)
{
    int Status = FMSH_FAILURE;

    u32 cmn_intr_id;
    u32 chn0_intr_id;
    u32 chn1_intr_id;
    u32 chn2_intr_id;
    u32 chn3_intr_id;
    u32 chn4_intr_id;
    u32 chn5_intr_id;
    u32 chn6_intr_id;
    u32 chn7_intr_id;
    
    if(InstancePtr->base_address == FPAR_AXIDMAPSU_0_BASEADDR)
    {
        cmn_intr_id = LPD_DMA_CMN_INT_ID; 
        chn0_intr_id = LPD_DMA_CH0_INT_ID;
        chn1_intr_id = LPD_DMA_CH1_INT_ID;
        chn2_intr_id = LPD_DMA_CH2_INT_ID;
        chn3_intr_id = LPD_DMA_CH3_INT_ID;
        chn4_intr_id = LPD_DMA_CH4_INT_ID;
        chn5_intr_id = LPD_DMA_CH5_INT_ID;
        chn6_intr_id = LPD_DMA_CH6_INT_ID;
        chn7_intr_id = LPD_DMA_CH7_INT_ID;
    }
    else if(InstancePtr->base_address == FPAR_AXIDMAPSU_1_BASEADDR)
    {
        cmn_intr_id = FPD_DMA_CMN_INT_ID;
        chn0_intr_id = FPD_DMA_CH0_INT_ID;
        chn1_intr_id = FPD_DMA_CH1_INT_ID;
        chn2_intr_id = FPD_DMA_CH2_INT_ID;
        chn3_intr_id = FPD_DMA_CH3_INT_ID;
        chn4_intr_id = FPD_DMA_CH4_INT_ID;
        chn5_intr_id = FPD_DMA_CH5_INT_ID;
        chn6_intr_id = FPD_DMA_CH6_INT_ID;
        chn7_intr_id = FPD_DMA_CH7_INT_ID;    
    }
    
    Status = FGicPs_registerInt(&IntcInstance, chn0_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[0]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, chn1_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[1]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, chn2_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[2]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, chn3_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[3]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, chn4_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[4]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, chn5_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[5]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, chn6_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[6]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, chn7_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_ChnIrqHandler,
                                &(InstancePtr->chn[7]));
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    Status = FGicPs_registerInt(&IntcInstance, cmn_intr_id,
                                (FMSH_InterruptHandler)FAxidmaPsu_CmnIrqHandler,
                                InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    return FMSH_SUCCESS;
}

/******************************************************************************
 * @description: this function is an example of user callback for dma common
 *               error interrupt.
 *
 *
 ******************************************************************************/
void fmsh_axidma_cmn_err_handler(void *CallBackRef, int status)
{
    if (status & AXIDMA_IRQ_CMN_SLVIF_DEC_ERR_MASK)
    {
        fmsh_print("Axidma cmn int slvif dec err.\r\n");
    }
    if (status & AXIDMA_IRQ_CMN_SLVIF_WR2RO_ERR_MASK)
    {
        fmsh_print("Axidma cmn int slvif wr2ro err.\r\n");
    }
    if (status & AXIDMA_IRQ_CMN_SLVIF_RD2WO_ERR_MASK)
    {
        fmsh_print("Axidma cmn int slvif rd2wo err.\r\n");
    }
    if (status & AXIDMA_IRQ_CMN_SLVIF_WRONHOLD_ERR_MASK)
    {
        fmsh_print("Axidma cmn int slvif wronhold err.\r\n");
    }
    if (status & AXIDMA_IRQ_CMN_SLVIF_UNDEFINEDREG_DEC_ERR_MASK)
    {
        fmsh_print("Axidma cmn int slvif undef dec err.\r\n");
    }
}

/******************************************************************************
 * @description: this function is an example of user callback for dma channel
 *               error interrupt. And it demonstrates how to configure the
 *               shadow register in shadow tranfers mode.
 *
 *
 ******************************************************************************/
void fmsh_axidma_chn_err_handler(void *CallBackRef, u32 mask)
{
    FAxidmaPsu_Chn_T *ChannelPtr = (FAxidmaPsu_Chn_T *)CallBackRef;
    if (mask & AXIDMA_IRQ_CH_SHADOWREG_OR_LLI_INVALID_ERR_MASK)
    {
        if (axidma_example == AXIDMA_SHAREG_EXAMPLE)
        {
            /* this example does not change ctl during write to shadow reg,
             * except the valid bit in cfg.
             */
            FAxidmaPsu_SetSar(ChannelPtr,
                             axidma_transfer[axidma_issued_blocks].sar);
            FAxidmaPsu_SetDar(ChannelPtr,
                             axidma_transfer[axidma_issued_blocks].dar);
            FAxidmaPsu_SetBlockTs(
                ChannelPtr, axidma_transfer[axidma_issued_blocks].block_ts);
            /* config next block type */
            if (axidma_issued_blocks == axidma_num_blocks - 1)
            {
                /* last block */
                FAxidmaPsu_SetLastBlk(ChannelPtr);
            }
            else
            {
                FAxidmaPsu_SetNonlastBlk(ChannelPtr);
            }
            /* resume the transfer */
            axidma_issued_blocks++;
            FAxidmaPsu_SetBlkResumeReq(ChannelPtr);
        }
    }
}

/******************************************************************************
 * @description: this function is an example of user callback for dma channel
 *               transfer interrupt. And for reloaad mode, the data will be
 *rewritten to validate the mode.
 *
 *
 ******************************************************************************/
void fmsh_axidma_trf_done_handler(void *CallBackRef, int mask)
{
    FAxidmaPsu_Chn_T *ChannelPtr = (FAxidmaPsu_Chn_T *)CallBackRef;
    if (mask & AXIDMA_IRQ_CH_BLOCK_TRF_DONE_MASK)
    {
        fmsh_print("Block done interrupt!\r\n");
        axidma_transfered_blocks++;
        if (axidma_example == AXIDMA_RELOAD_EXAMPLE)
        {
            if (axidma_transfered_blocks < axidma_num_blocks)
            {
                /* compare data & prepare data again */
                Fmsh_DCacheFlush();
                axidma_reload_check_status = axidma_memory_compare(
                    &axidma_reload_transfer[axidma_transfered_blocks - 1], 1);
                if (axidma_reload_check_status != FMSH_SUCCESS)
                {
                    fmsh_print("ERROR: CH[%d] compare data Failed!\r\n",
                               ChannelPtr->id);
                }

                axidma_memory_prepare(
                    &axidma_reload_transfer[axidma_transfered_blocks], 1);
            }
            else
            {
                /* This will stop reload immediately */
                FAxidmaPsu_ClrBlktype(ChannelPtr);
            }
        }
    }
    if (mask & AXIDMA_IRQ_CH_DMA_TFR_DONE_MASK)
    {
        fmsh_print("Dma done interrupt!\r\n");
        axidma_transfer_done = 1;
    }
    if (mask & AXIDMA_IRQ_CH_SRC_TRANSCOMP_MASK)
    {
        fmsh_print("Src trans done interrupt!\r\n");
    }
    if (mask & AXIDMA_IRQ_CH_DST_TRANSCOMP_MASK)
    {
        fmsh_print("Dst trans done interrupt!\r\n");
    }
}

/******************************************************************************
 * @description: this function is an example of single tranfer using axidma.
 *
 *
 ******************************************************************************/
static int fmsh_axidma_sglblk_example(FAxidmaPsu_T *InstancePtr)
{
    int status;
    FAxidmaPsu_Chn_T *ChannelPtr;
    axidma_transfer_done = 0;
    axidma_example = AXIDMA_SGLBLK_EXAMPLE;

    /* Transfer data init */
    axidma_memory_prepare(axidma_transfer, 1);
    
    Fmsh_DCacheFlush();

    /* Enable axidma device */
    FAxidmaPsu_EnableDma(InstancePtr);

    /* Get one idle channel */
    status = FAxidmaPsu_GetFreeChannel(InstancePtr, &ChannelPtr);
    if (status != FMSH_SUCCESS)
    {
        fmsh_print("No idle channel\r\n");
        return FMSH_FAILURE;
    }

    /* Set interrupt call back has been set */
    FAxidmaPsu_SetCallBackCmn(InstancePtr, FAXIDMAPS_CMNHANDLER_ERROR,
                             (void *)fmsh_axidma_cmn_err_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_DONE,
                             (void *)fmsh_axidma_trf_done_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_ERROR,
                             (void *)fmsh_axidma_chn_err_handler);

    /* Config channel with user's need */
    FAxidmaPsu_ChnConfig_T chn_config;
    memset(&chn_config, 0, sizeof(FAxidmaPsu_ChnConfig_T));
    chn_config.sar = (u64)src_buf0;
    chn_config.dar = (u64)dst_buf0;
    chn_config.block_ts = BLOCK_TS0;
    chn_config.cfg.src_mltblk_type = axidma_contiguous;
    chn_config.cfg.dst_mltblk_type = axidma_contiguous;
    chn_config.ctl.src_tr_width = axidma_trans_width_128;
    chn_config.ctl.dst_tr_width = axidma_trans_width_128;
    chn_config.ctl.ioc_blktfr = 1;
    chn_config.ctl.shadowreg_or_lli_valid = 1;
    chn_config.ctl.shadowreg_or_lli_last = 1;
    FAxidmaPsu_ChnConfig(ChannelPtr, &chn_config);

    /* Enable channel interrupt and start axidma transfer */
    FAxidmaPsu_EnableIrqGlobal(InstancePtr);
    FAxidmaPsu_EnableChnTrfIrq(ChannelPtr);
    FAxidmaPsu_EnableChn(ChannelPtr);

    /* Wait for tranfer finishing */
    while (!axidma_transfer_done)
    {
    }

    fmsh_print("Axidma transfer done: %d\n", axidma_transfer_done);

    Fmsh_DCacheFlush();
    /* Memory check */
    status = axidma_memory_compare(axidma_transfer, 1);
    if (status != FMSH_SUCCESS)
    {
        fmsh_print("ERROR: CH[%d] compare data Failed!\r\n", ChannelPtr->id);
    }
    

    FAxidmaPsu_DisableChnIrqAll(ChannelPtr);
    FAxidmaPsu_DisableIrqGlobal(InstancePtr);

    return status;
}

/******************************************************************************
 * @description: this function is an example of LLI transfer using axidma.
 *
 *
 ******************************************************************************/
static int fmsh_axidma_lli_example(FAxidmaPsu_T *InstancePtr)
{
    int status = FMSH_SUCCESS;
    FAxidmaPsu_Chn_T *ChannelPtr;

    axidma_transfer_done = 0;
    axidma_transfered_blocks = 0;
    axidma_example = AXIDMA_LLI_EXAMPLE;

    /* Transfer data init */
    axidma_memory_prepare(axidma_transfer, 3);

    /* Enable axidma device */
    FAxidmaPsu_EnableDma(InstancePtr);

    /* Get one idle channel */
    status = FAxidmaPsu_GetFreeChannel(InstancePtr, &ChannelPtr);
    if (status != FMSH_SUCCESS)
    {
        return status;
    }

    /* Set interrupt call back has been set */
    FAxidmaPsu_SetCallBackCmn(InstancePtr, FAXIDMAPS_CMNHANDLER_ERROR,
                             (void *)fmsh_axidma_cmn_err_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_DONE,
                             (void *)fmsh_axidma_trf_done_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_ERROR,
                             (void *)fmsh_axidma_chn_err_handler);

    /* Config channel */
    FAxidmaPsu_ChnConfig_T chn_config;
    memset(&chn_config, 0, sizeof(FAxidmaPsu_ChnConfig_T));
    chn_config.cfg.src_mltblk_type = axidma_linked_list;
    chn_config.cfg.dst_mltblk_type = axidma_linked_list;
    chn_config.cfg.tt_fc = axidma_mem2mem_dma;
    chn_config.llp = (u64)lli_entry;

    /* Prepare lli */
    chn_config.ctl.src_tr_width = axidma_trans_width_128;
    chn_config.ctl.dst_tr_width = axidma_trans_width_128;
    chn_config.ctl.ioc_blktfr = 1;
    chn_config.ctl.shadowreg_or_lli_last = 0;
    chn_config.ctl.shadowreg_or_lli_valid = 1;
    u64 ctl_reg = FAxidmaPsu_GenCtl(&chn_config.ctl);
    fmsh_axidma_create_lli_entry(&lli_entry[0], &lli_entry[1], (u64)src_buf0,
                                 (u64)dst_buf0, BLOCK_TS0, ctl_reg);
    fmsh_axidma_create_lli_entry(&lli_entry[1], &lli_entry[2], (u64)src_buf1,
                                 (u64)dst_buf1, BLOCK_TS1, ctl_reg);
    chn_config.ctl.shadowreg_or_lli_last = 1;
    ctl_reg = FAxidmaPsu_GenCtl(&chn_config.ctl);
    fmsh_axidma_create_lli_entry(&lli_entry[2], &lli_entry[0], (u64)src_buf2,
                                 (u64)dst_buf2, BLOCK_TS2, ctl_reg);

    FAxidmaPsu_ChnConfig(ChannelPtr, &chn_config);

    /* Make the linked list ready */
    Fmsh_DCacheFlush();
    dsb();

    /* Enable channel interrupt and start axidma transfer */
    FAxidmaPsu_EnableIrqGlobal(InstancePtr);
    FAxidmaPsu_EnableChnTrfIrq(ChannelPtr);
    FAxidmaPsu_EnableChn(ChannelPtr);

    while (!axidma_transfer_done)
    {
    }
    fmsh_print("Axidma transfer done: %d\n", axidma_transfer_done);

    Fmsh_DCacheFlush();
    /* Memory check */
    status = axidma_memory_compare(axidma_transfer, 3);
    if (status != FMSH_SUCCESS)
    {
        fmsh_print("ERROR: CH[%d] compare data Failed!\r\n", ChannelPtr->id);
    }

    FAxidmaPsu_DisableChnIrqAll(ChannelPtr);
    FAxidmaPsu_DisableIrqGlobal(InstancePtr);

    return status;
}

/******************************************************************************
 * @description: this function is an example of auto-reload transfer using
 * axidma.
 *
 *
 ******************************************************************************/
static int fmsh_axidma_reload_example(FAxidmaPsu_T *InstancePtr)
{
    int status;
    axidma_transfer_done = 0;
    axidma_transfered_blocks = 0;
    axidma_example = AXIDMA_RELOAD_EXAMPLE;
    axidma_reload_check_status = FMSH_SUCCESS;
      
    FAxidmaPsu_Chn_T *ChannelPtr;

    axidma_num_blocks = 3;
    /* Transfer data init */
    axidma_memory_prepare(axidma_reload_transfer, 1);
    Fmsh_DCacheFlush();

    /* Enable axidma device */
    FAxidmaPsu_EnableDma(InstancePtr);

    /* Get one idle channel */
    status = FAxidmaPsu_GetFreeChannel(InstancePtr, &ChannelPtr);
    if (status != FMSH_SUCCESS)
    {
        fmsh_print("No idle channel\r\n");
        return status;
    }

    /* Set interrupt call back has been set */
    FAxidmaPsu_SetCallBackCmn(InstancePtr, FAXIDMAPS_CMNHANDLER_ERROR,
                             (void *)fmsh_axidma_cmn_err_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_DONE,
                             (void *)fmsh_axidma_trf_done_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_ERROR,
                             (void *)fmsh_axidma_chn_err_handler);

    /* Config channel */
    FAxidmaPsu_ChnConfig_T chn_config;
    memset(&chn_config, 0, sizeof(FAxidmaPsu_ChnConfig_T));
    chn_config.cfg.src_mltblk_type = axidma_reload;
    chn_config.cfg.dst_mltblk_type = axidma_reload;
    chn_config.sar = (u64)src_buf0;
    chn_config.dar = (u64)dst_buf0;
    chn_config.block_ts = BLOCK_TS0;
    chn_config.ctl.src_tr_width = axidma_trans_width_128;
    chn_config.ctl.dst_tr_width = axidma_trans_width_128;
    chn_config.ctl.ioc_blktfr = 1;
    chn_config.ctl.shadowreg_or_lli_valid = 1;
    chn_config.ctl.shadowreg_or_lli_last = 0;

    /* Config channel */
    FAxidmaPsu_ChnConfig(ChannelPtr, &chn_config);

    /* Enable channel interrupt and start axidma transfer */
    FAxidmaPsu_EnableIrqGlobal(InstancePtr);
    FAxidmaPsu_EnableChnTrfIrq(ChannelPtr);
    FAxidmaPsu_EnableChn(ChannelPtr);

    while (!axidma_transfer_done)
    {
    }
    fmsh_print("Axidma transfer done: %d\n", axidma_transfer_done);

    Fmsh_DCacheFlush();
    /* Memory check */
    status = axidma_memory_compare(&axidma_reload_transfer[axidma_num_blocks-1], 
                                   1);
    if ((status != FMSH_SUCCESS) && (axidma_reload_check_status != FMSH_SUCCESS))
    {
        fmsh_print("ERROR: CH[%d] compare data Failed!\r\n", ChannelPtr->id);
    }

    FAxidmaPsu_DisableChnIrqAll(ChannelPtr);
    FAxidmaPsu_DisableIrqGlobal(InstancePtr);

    return (status|axidma_reload_check_status);
}

/******************************************************************************
 * @description: this function is an example of shadow register transfer using
 * axidma.
 *
 *
 ******************************************************************************/
static int fmsh_axidma_shadowreg_example(FAxidmaPsu_T *InstancePtr)
{
    int status;
    axidma_transfer_done = 0;
    axidma_issued_blocks = 0;
    axidma_example = AXIDMA_SHAREG_EXAMPLE;

    FAxidmaPsu_Chn_T *ChannelPtr;

    /* Prepare memory */
    axidma_memory_prepare(axidma_transfer, 3);

    /* Enable axidma device */
    FAxidmaPsu_EnableDma(InstancePtr);
    FAxidmaPsu_EnableIrqGlobal(InstancePtr);

    /* Get one idle channel */
    status = FAxidmaPsu_GetFreeChannel(InstancePtr, &ChannelPtr);
    if (status != FMSH_SUCCESS)
    {
        fmsh_print("No idle channel\r\n");
        return status;
    }

    /* Set interrupt callback */
    FAxidmaPsu_SetCallBackCmn(InstancePtr, FAXIDMAPS_CMNHANDLER_ERROR,
                             (void *)fmsh_axidma_cmn_err_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_DONE,
                             (void *)fmsh_axidma_trf_done_handler);
    FAxidmaPsu_SetCallBackChn(ChannelPtr, FAXIDMAPS_CHNHANDLER_ERROR,
                             (void *)fmsh_axidma_chn_err_handler);

    /* Config channel */
    FAxidmaPsu_ChnConfig_T chn_config;
    memset(&chn_config, 0, sizeof(FAxidmaPsu_ChnConfig_T));
    chn_config.sar = axidma_transfer[0].sar;
    chn_config.dar = axidma_transfer[0].dar;
    chn_config.block_ts = axidma_transfer[0].block_ts;
    chn_config.cfg.src_mltblk_type = axidma_shadow_register;
    chn_config.cfg.dst_mltblk_type = axidma_shadow_register;
    chn_config.ctl.src_tr_width = axidma_trans_width_128;
    chn_config.ctl.dst_tr_width = axidma_trans_width_128;
    chn_config.ctl.ioc_blktfr = 1;
    chn_config.ctl.shadowreg_or_lli_valid = 1;
    chn_config.ctl.shadowreg_or_lli_last = 0;

    /* Config channel */
    FAxidmaPsu_ChnConfig(ChannelPtr, &chn_config);

    /* Enable channel interrupt*/
    FAxidmaPsu_EnableChnTrfIrq(ChannelPtr);

    /* Start transfer */
    axidma_issued_blocks = 1;
    FAxidmaPsu_EnableChn(ChannelPtr);

    while (!axidma_transfer_done)
    {
    }
    fmsh_print("Axidma transfer done: %i\r\n", axidma_transfer_done);

    /* Memory check */
    Fmsh_DCacheFlush();
    status = axidma_memory_compare(axidma_transfer, 3);
    if (status != FMSH_SUCCESS)
    {
        fmsh_print("ERROR: CH[%i] compare data Failed!\r\n", ChannelPtr->id);
    }

    FAxidmaPsu_DisableChnIrqAll(ChannelPtr);
    FAxidmaPsu_DisableIrqGlobal(InstancePtr);

    return status;
}

/******************************************************************************
 * @description: this function is an example of axidma peripheral test.
 *               
 * @param deviceId sets the tested axidma. 0 is LPD AXIDMA, 1 is FPD AXIDMA.
 *
 ******************************************************************************/
u32 FAxidmaPsu_example(u16 deviceId)
{
    u32 status;
    u32 test_status = FMSH_SUCCESS;
    
    /* Initialize the driver */
    FAxidmaPsu_Config_T configT;
    FAxidmaPsu_Config_T *configPtr = &configT;
    configPtr = FAxidmaPsu_LookupConfig(deviceId);
    FAxidmaPsu_Initialize(&g_AxiDma, configPtr);

    /* Setup the interrupt */
    status = fmsh_axidma_setup_interrupt_system(&g_AxiDma);
    if (status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    fmsh_print("Begin to run axidma_%d test example.\r\n", deviceId);
    fmsh_print("/ -------\r\n");

    /* Single block */
    status = fmsh_axidma_sglblk_example(&g_AxiDma);
    if (status != FMSH_SUCCESS)
        fmsh_print("Run axidma single block test failed.\r\n");
    else
        fmsh_print("Run axidma single block test pass.\r\n");
    test_status |= status;
    
    /* LLI tranfer */
    status = fmsh_axidma_lli_example(&g_AxiDma);
    if (status != FMSH_SUCCESS)
        fmsh_print("Run axidma multi block(LLI mode) test failed.\r\n");
    else
        fmsh_print("Run axidma multi block(LLI mode) test pass.\r\n");
    test_status |= status;
    
    /* Auto-reload tranfer */
    status = fmsh_axidma_reload_example(&g_AxiDma);
    if (status != FMSH_SUCCESS)
        fmsh_print("Run axidma multi block(Reload mode) test failed.\r\n");
    else
        fmsh_print("Run axidma multi block(Reload mode) test pass.\r\n");
    test_status |= status;
    
    /* Shadow register tranfer */
    status = fmsh_axidma_shadowreg_example(&g_AxiDma);
    if (status != FMSH_SUCCESS)
        fmsh_print("Run axidma multi block(Shadow mode) test failed.\r\n");
    else
        fmsh_print("Run axidma multi block(Shadow mode) test pass.\r\n");
    test_status |= status;
    
    if (test_status == FMSH_SUCCESS)
    {
        fmsh_print("Run axidma test example success.\r\n");
        fmsh_print("/ -------\r\n");
    }
    else{
        fmsh_print("Run axidma test example fail, please check.\r\n");
        fmsh_print("/ -------\r\n");
    }

    return test_status;
}