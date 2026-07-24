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
 * @file fmsh_axidmapsu.h
 * @addtogroup axidmapsu_v1_0
 * @{
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver  Who   Date        Changes
 * ---- ---- --------   ---------------------------------------------
 * 1.00  whn 07/18/2024  First Release
 *
 *</pre>
 *
 ******************************************************************************/
#ifndef _FMSH_AXIDMAPSU_H_
#define _FMSH_AXIDMAPSU_H_

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include <string.h>
#include "fmsh_axidmapsu_hw.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
/*****************************************************************************
 * DESCRIPTION
 *  This typedef contains configuration information for the device.
 *
 *****************************************************************************/
typedef struct
{
    u16 device_id;
    int ch_num_max;   /**< Maximum channels */
    u32 base_address; /**< Base address of device*/
} FAxidmaPsu_Config_T;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used to select the source and/or the
 *  destination for a specific DMA channel when using some
 *  of the driver's API functions.
 *  This data type is used by many of the API functions in the driver.
 *
 *****************************************************************************/
typedef enum
{
    axidma_src = 0x1,
    axidma_dst = 0x2,
    axidma_src_dst = 0x3
} FAxidmaPsu_SrcDstSelect_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used to select a software or hardware interface
 *  when using the specified driver API functions to access the
 *  handshaking interface on a specified DMA channel.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s) / bit field(s): (x = channel number)
 *    - CFGx.HS_SEL_SRC, CFGx.HS_SEL_DST
 *
 *****************************************************************************/
typedef enum
{
    axidma_hs_hardware = 0x0,
    axidma_hs_software = 0x1
} FAxidmaPsu_HsSelect_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the transfer flow device
 *  (memory or peripheral device) and for setting the flow control
 *  device for the DMA transfer when using the specified driver
 *  API functions.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s) / bit field(s): (x = channel number)
 *    - CTLx.TT_FC
 *
 *****************************************************************************/
typedef enum
{
    axidma_mem2mem_dma = 0x0,    /* mem to mem - DMAC   flow ctlr */
    axidma_mem2prf_dma = 0x1,    /* mem to prf - DMAC   flow ctlr */
    axidma_prf2mem_dma = 0x2,    /* prf to mem - DMAC   flow ctlr */
    axidma_prf2prf_dma = 0x3,    /* prf to prf - DMAC   flow ctlr */
    axidma_prf2mem_prf = 0x4,    /* prf to mem - periph flow ctlr */
    axidma_prf2prf_srcprf = 0x5, /* prf to prf - source flow ctlr */
    axidma_mem2prf_prf = 0x6,    /* mem to prf - periph flow ctlr */
    axidma_prf2prf_dstprf = 0x7  /* prf to prf - dest   flow ctlr */
} FAxidmaPsu_TransFlow_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the polarity level for the
 *  source and/or destination on a DMA channel's handshaking interface
 *  when using the specified driver API functions.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s) / bit-field(s): (x = channel number)
 *    - CFGx.SRC_HS_POL, CFGx.DST_HS_POL
 *
 *****************************************************************************/
typedef enum
{
    axidma_active_high = 0x0,
    axidma_active_low = 0x1
} FAxidmaPsu_PolarLevel_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used to select which of the software handshake request
 *  registers are accessed within the DMA Controller when using the
 *  specified driver API functions.
 *
 *****************************************************************************/
typedef enum
{
    axidma_request = 0x1,        /* ReqSrcReq/ReqDstReq */
    axidma_single_request = 0x2, /* SglReqSrcReq/SglReqDstReq */
    axidma_last_request = 0x4    /* LstReqSrcReq/LstReqDstReq */
} FAxidmaPsu_SwReq_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the priority level of a DMA
 *  channel when using the specified driver API functions.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s)/bit field(s): (x = channel number)
 *    - CFGx.CH_PRIOR
 *
 *****************************************************************************/
typedef enum
{
    axidma_priority_0 = 0x0,
    axidma_priority_1 = 0x1,
    axidma_priority_2 = 0x2,
    axidma_priority_3 = 0x3,
    axidma_priority_4 = 0x4,
    axidma_priority_5 = 0x5,
    axidma_priority_6 = 0x6,
    axidma_priority_7 = 0x7
} FAxidmaPsu_ChanPrior_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the address increment
 *  type for the source and/or destination on a DMA channel when using
 *  the specified driver API functions.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s) / bit-field(s): (x = channel number)
 *    - CTLx.SINC, CTLx.DINC
 *
 *****************************************************************************/
typedef enum axidma_burst_type
{
    axidma_burst_increment = 0x0,
    axidma_burst_fixed = 0x1
} FAxidmaPsu_BurstType_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the transfer width for the
 *  source and/or destination on a DMA channel when using the specified
 *  driver API functions.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s) / bit field(s): (x = channel number)
 *    - CTLx.SRC_TR_WIDTH, CTLx.DST_TR_WIDTH
 *
 *****************************************************************************/
typedef enum axidma_transfer_width
{
    axidma_trans_width_8 = 0x0,
    axidma_trans_width_16 = 0x1,
    axidma_trans_width_32 = 0x2,
    axidma_trans_width_64 = 0x3,
    axidma_trans_width_128 = 0x4
} FAxidmaPsu_BurstWidth_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the burst transfer length
 *  on the source and/or destination of a DMA channel when using the
 *  specified driver API functions.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s) / bit field(s): (x = channel number)
 *    - CTLx.SRC_MSIZE, CTLx.DEST_MSIZE
 *
 *****************************************************************************/
typedef enum axidma_burst_trans_length
{
    axidma_msize_1 = 0x0,
    axidma_msize_4 = 0x1,
    axidma_msize_8 = 0x2,
    axidma_msize_16 = 0x3,
} FAxidmaPsu_BurstLen_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used to select the multi block type for src/dst
 *  on the DMA Controller when using the specified driver API
 *  functions.
 *
 *****************************************************************************/
typedef enum
{
    axidma_contiguous = 0x0,
    axidma_reload = 0x1,
    axidma_shadow_register = 0x2,
    axidma_linked_list = 0x3,
} FAxidmaPsu_MltblkType_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the handshaking interface
 *  number for the source and/or destination on a DMA channel when
 *  using the specified driver API functions.
 * NOTES
 *  This data type relates directly to the following DMA Controller
 *  register(s) / bit-field(s): (x = channel number)
 *    - CFGx.DEST_PER, CFGx.SRC_PER
 *
 *****************************************************************************/
typedef enum axidma_handshake_interface
{
    axidma_hs_if0 = 0x0,
    axidma_hs_if1 = 0x1,
    axidma_hs_if2 = 0x2,
    axidma_hs_if3 = 0x3,
    axidma_hs_if4 = 0x4,
    axidma_hs_if5 = 0x5,
    axidma_hs_if6 = 0x6,
    axidma_hs_if7 = 0x7,
} FAxidmaPsu_HsIf_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting the channel lock level when
 *  using the specified driver API functions.
 *
 *****************************************************************************/
typedef enum axidma_channel_lock_level
{
    axidma_over_dma_transfer = 0x0,
    axidma_over_block_transfer = 0x1
} FAxidmaPsu_LockLevel_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for indicating if the block is the last one.
 *
 *****************************************************************************/
typedef enum axidma_shadow_lli_last_block
{
    axidma_nonlastblock = 0x0,
    axidma_lastblock = 0x1
} FAxidmaPsu_LastBlock_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for creating linked list item when running
 *  LLI multiblock transfers.
 *
 *****************************************************************************/
typedef struct
{
    u64 sar;
    u64 dar;
    u32 block_ts;
    u32 reserved;
    u64 llp;
    u64 ctl;
    u32 sstat;
    u32 dstat;
    u64 llp_status;
    u64 reserved2;
} FAxidmaPsu_Lli_T __attribute__((aligned(64)));

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for configuring the CTL register of one channel.
 *
 *****************************************************************************/
typedef struct
{
    FAxidmaPsu_BurstType_E sinc;
    FAxidmaPsu_BurstType_E dinc;
    FAxidmaPsu_BurstWidth_E src_tr_width;
    FAxidmaPsu_BurstWidth_E dst_tr_width;
    FAxidmaPsu_BurstLen_E src_msize;
    FAxidmaPsu_BurstLen_E dst_msize;
    u8 ar_cache;
    u8 aw_cache;
    u8 nonposted_lastwrite_en;
    u8 ar_prot;
    u8 aw_prot;
    u8 arlen_en;
    u8 arlen;
    u8 awlen_en;
    u8 awlen;
    u8 src_stat_en;
    u8 dst_stat_en;
    u8 ioc_blktfr;
    u8 shadowreg_or_lli_last;
    u8 shadowreg_or_lli_valid;
} FAxidmaPsu_Ctl_T;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for configuring the CFG register of one channel.
 *
 *****************************************************************************/
typedef struct
{
    FAxidmaPsu_MltblkType_E src_mltblk_type;
    FAxidmaPsu_MltblkType_E dst_mltblk_type;
    FAxidmaPsu_TransFlow_E tt_fc;
    FAxidmaPsu_HsSelect_E hs_sel_src;
    FAxidmaPsu_HsSelect_E hs_sel_dst;
    FAxidmaPsu_HsIf_E src_per;
    FAxidmaPsu_HsIf_E dst_per;
    FAxidmaPsu_ChanPrior_E ch_prior;
} FAxidmaPsu_Cfg_T;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for configing DMA channel when using the
 *  specified driver API functions.
 *
 *****************************************************************************/
typedef struct
{
    u64 sar;
    u64 dar;
    u32 block_ts;

    FAxidmaPsu_Ctl_T ctl;
    FAxidmaPsu_Cfg_T cfg;

    u64 llp;

    u32 sstat; // read only
    u32 dstat; // read only
    u64 sstatar;
    u64 dstatar;

    u8 awqos;
    u8 arqos;
} FAxidmaPsu_ChnConfig_T;

/******************************************************************************/
/**
 * Callback type for all error interrupts.
 *
 * @param 	CallBackRef is a callback reference passed in by the upper layer
 *		when setting the callback functions, and passed back to the
 *		upper layer when the callback is invoked.
 * @param	Mask is a bit mask indicating the cause of the error.
 ****************************************************************************/
typedef void (*FAxidmaPsu_Handler)(void *CallBackRef, u32 Mask);

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for managing a specific DMA channel when
 *  using the specified driver API functions.
 *
 *****************************************************************************/
typedef struct
{
    struct axi_dma_device *axidma;
    u8 id;
    FAxidmaPsu_ChnConfig_T chn_config;
    FAxidmaPsu_Handler chn_err_handler;
    FAxidmaPsu_Handler trf_done_handler;
} FAxidmaPsu_Chn_T;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for managing a specific DMA instance when
 *  using the specified driver API functions.
 *
 *****************************************************************************/
typedef struct axi_dma_device
{
    u32 base_address;
    FAxidmaPsu_Chn_T chn[FPAR_AXIDMAPSU_NUM_CHANNEL];
    int ch_num_max;
    FAxidmaPsu_Handler cmn_err_handler;
} FAxidmaPsu_T;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting channel interrupt handler type.
 *
 *****************************************************************************/
typedef enum
{
    FAXIDMAPS_CHNHANDLER_DONE,
    FAXIDMAPS_CHNHANDLER_ERROR,
} FAxidmaPsu_ChnHandler_E;

/*****************************************************************************
 * DESCRIPTION
 *  This data type is used for selecting common interrupt handler type.
 *
 *****************************************************************************/
typedef enum
{
    FAXIDMAPS_CMNHANDLER_ERROR,
} FAxidmaPsu_CmnHandler_E;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
FAxidmaPsu_Config_T *FAxidmaPsu_LookupConfig(u16 deviceId);
int FAxidmaPsu_Initialize(FAxidmaPsu_T *InstancePtr,
                         FAxidmaPsu_Config_T *configPtr);

u32 FAxidmaPsu_ReadReg(FAxidmaPsu_T *InstancePtr, int offset);
u64 FAxidmaPsu_ReadReg64(FAxidmaPsu_T *InstancePtr, int offset);
u32 FAxidmaPsu_ReadRegChn(FAxidmaPsu_Chn_T *ChannelPtr, int offset);
u64 FAxidmaPsu_ReadRegChn64(FAxidmaPsu_Chn_T *ChannelPtr, int offset);
void FAxidmaPsu_WriteReg(FAxidmaPsu_T *InstancePtr, int offset, u32 v);
void FAxidmaPsu_WriteReg64(FAxidmaPsu_T *InstancePtr, int offset, u64 v);
void FAxidmaPsu_WriteRegChn(FAxidmaPsu_Chn_T *ChannelPtr, int offset, u32 v);
void FAxidmaPsu_WriteRegChn64(FAxidmaPsu_Chn_T *ChannelPtr, int offset, u64 v);

int FAxidmaPsu_Reset(FAxidmaPsu_T *InstancePtr);

void FAxidmaPsu_DisableDma(FAxidmaPsu_T *InstancePtr);
void FAxidmaPsu_EnableDma(FAxidmaPsu_T *InstancePtr);
void FAxidmaPsu_DisableIrqGlobal(FAxidmaPsu_T *InstancePtr);
void FAxidmaPsu_EnableIrqGlobal(FAxidmaPsu_T *InstancePtr);

void FAxidmaPsu_DisableChn(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_EnableChn(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SuspendChn(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_ResumeChn(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_AbortChn(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_ClearChnIrq(FAxidmaPsu_Chn_T *ChannelPtr, u32 irq_mask);
void FAxidmaPsu_ClearCmnIrq(FAxidmaPsu_T *InstancePtr, u32 irq_mask);
u32 FAxidmaPsu_GetChnIrq(FAxidmaPsu_Chn_T *ChannelPtr);
u32 FAxidmaPsu_GetCmnIrq(FAxidmaPsu_T *InstancePtr);
void FAxidmaPsu_EnableChnTrfIrq(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_EnableChnIrq(FAxidmaPsu_Chn_T *ChannelPtr, u32 irq_mask);
void FAxidmaPsu_DisableChnIrqAll(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_DisableChnIrq(FAxidmaPsu_Chn_T *ChannelPtr, u32 irq_mask);

void FAxidmaPsu_SetSar(FAxidmaPsu_Chn_T *ChannelPtr, u64 sar);
u64 FAxidmaPsu_GetSar(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SetDar(FAxidmaPsu_Chn_T *ChannelPtr, u64 dar);
u64 FAxidmaPsu_GetDar(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SetBlockTs(FAxidmaPsu_Chn_T *ChannelPtr, u32 block_ts);
u32 FAxidmaPsu_GetBlkTs(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SetCtl(FAxidmaPsu_Chn_T *ChannelPtr, FAxidmaPsu_Ctl_T *ctl_ptr);
void FAxidmaPsu_SetCfg(FAxidmaPsu_Chn_T *ChannelPtr, FAxidmaPsu_Cfg_T *cfg_ptr);
void FAxidmaPsu_SetLlp(FAxidmaPsu_Chn_T *ChannelPtr, u64 llp);
u64 FAxidmaPsu_GetLlp(FAxidmaPsu_Chn_T *ChannelPtr);

void FAxidmaPsu_SetQos(FAxidmaPsu_Chn_T *ChannelPtr, u8 awqos, u8 arqos);
void FAxidmaPsu_GetQos(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SetSstatar(FAxidmaPsu_Chn_T *ChannelPtr, u64 sstatar);
u64 FAxidmaPsu_GetSstatar(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SetDstatar(FAxidmaPsu_Chn_T *ChannelPtr, u64 dstatar);
u64 FAxidmaPsu_GetDstatar(FAxidmaPsu_Chn_T *ChannelPtr);
u32 FAxidmaPsu_GetSstat(FAxidmaPsu_Chn_T *ChannelPtr);
u32 FAxidmaPsu_GetDstat(FAxidmaPsu_Chn_T *ChannelPtr);
u32 FAxidmaPsu_GetCompletedSize(FAxidmaPsu_Chn_T *ChannelPtr);

void FAxidmaPsu_SetLastBlk(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SetNonlastBlk(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SetBlkResumeReq(FAxidmaPsu_Chn_T *ChannelPtr);
void FAxidmaPsu_SendSwhsSrcReq(FAxidmaPsu_Chn_T *ChannelPtr, u32 val);
void FAxidmaPsu_SendSwhsDstReq(FAxidmaPsu_Chn_T *ChannelPtr, u32 val);
void FAxidmaPsu_ClrBlktype(FAxidmaPsu_Chn_T *ChannelPtr);

void FAxidmaPsu_HwInit(FAxidmaPsu_T *InstancePtr);
void FAxidmaPsu_ChnConfig(FAxidmaPsu_Chn_T *ChannelPtr,
                         FAxidmaPsu_ChnConfig_T *chn_config_ptr);
void FAxidmaPsu_GetChnConfig(FAxidmaPsu_Chn_T *ChannelPtr);

int FAxidmaPsu_GetFreeChannel(FAxidmaPsu_T *InstancePtr,
                             FAxidmaPsu_Chn_T **ChannelPtr);
u64 FAxidmaPsu_GenCtl(FAxidmaPsu_Ctl_T *ctl_ptr);
u64 FAxidmaPsu_GenCfg(FAxidmaPsu_Cfg_T *cfg_ptr);
void fmsh_axidma_create_lli_entry(FAxidmaPsu_Lli_T *lli_ptr,
                                  FAxidmaPsu_Lli_T *next_lli_ptr, u64 sar,
                                  u64 dar, u32 block_ts, u64 ctl);

void FAxidmaPsu_ChnIrqHandler(void *CallBackRef);
void FAxidmaPsu_CmnIrqHandler(void *CallBackRef);

u32 FAxidmaPsu_SetCallBackChn(FAxidmaPsu_Chn_T *ChannelPtr,
                             FAxidmaPsu_ChnHandler_E HandlerType,
                             void *CallBackFunc);
u32 FAxidmaPsu_SetCallBackCmn(FAxidmaPsu_T *InstancePtr,
                             FAxidmaPsu_CmnHandler_E HandlerType,
                             void *CallBackFunc);

#endif
