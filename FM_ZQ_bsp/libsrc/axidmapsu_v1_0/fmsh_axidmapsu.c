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
 * @file fmsh_axidmapsu.c
 * @addtogroup axidmapsu_v1_0
 * @{
 *
 * Contains implements the interface functions of the FAxidmaPsu driver.
 * See fmsh_axidmapsu.h for a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---- --------   ---------------------------------------------
 * 1.00  whn  09/03/2024  First Release
 *
 * </pre>
 *
 ******************************************************************************/
#include "fmsh_axidmapsu.h"

extern FAxidmaPsu_Config_T FAxidmaPsu_ConfigTable[];
/*****************************************************************************
 * This function looks up the device configuration based on the unique device
 * ID. The table FAxidmaPsu_ConfigTable contains the configuration info for each
 * device in the system.
 *
 * @param    deviceId contains the ID of the device for which the device
 *               configuration pointer is to be returned.
 *
 * @return   - A pointer to the configuration found.
 *           - NULL if the specified device ID was not found.
 *
 * @note     None.
 *
 ******************************************************************************/
FAxidmaPsu_Config_T *FAxidmaPsu_LookupConfig(u16 deviceId)
{
    int index;
    FAxidmaPsu_Config_T *cfgPtr = NULL;

    for (index = 0; index < FPAR_AXIDMAPSU_NUM_INSTANCES; index++)
    {
        if (FAxidmaPsu_ConfigTable[index].device_id == deviceId)
        {
            cfgPtr = &FAxidmaPsu_ConfigTable[index];
            break;
        }
    }
    return cfgPtr;
}

/*****************************************************************************
 * This function initializes a specific FAxidmaPsu_T device/instance. This
 * function must be called prior to use the device to read or write any data.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    configPtr points to the FAxidmaPsu_Config_T configuration structure.
 *
 * @return   - FMSH_SUCCESS if successful.
 *           - FMSH_FAILURE if fail.
 *
 * @note     The user needs to first call the FAxidmaPsu_LookupConfig() API
 *	     which returns the Configuration structure pointer which is
 *	     passed as a parameter to the FAxidmaPsu_Initialize() API.
 *
 ******************************************************************************/
int FAxidmaPsu_Initialize(FAxidmaPsu_T *InstancePtr,
                         FAxidmaPsu_Config_T *configPtr)
{
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(configPtr != NULL);

    (void)memset(InstancePtr, 0, sizeof(FAxidmaPsu_T));
    /* set default value */
    InstancePtr->ch_num_max = configPtr->ch_num_max;
    InstancePtr->base_address = configPtr->base_address;
    FAxidmaPsu_HwInit(InstancePtr);

    return FMSH_SUCCESS;
}

/*****************************************************************************
 * This function is used to read 32 bit data of one AXIDMA register.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    offset is the offset value of the target register.
 *
 * @return   The 32 bit data of the target register .
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_ReadReg(FAxidmaPsu_T *InstancePtr, int offset)
{
    return FMSH_ReadReg(InstancePtr->base_address, offset);
}

/*****************************************************************************
 * This function is used to read 64 bit data of one register.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    offset is the offset value of the target register.
 *
 * @return   The 64 bit data of the target register .
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_ReadReg64(FAxidmaPsu_T *InstancePtr, int offset)
{
    return FMSH_ReadReg64(InstancePtr->base_address, offset);
}

/*****************************************************************************
 * This function is used to read 32 bit data of one AXIDMA channel register.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    offset is the offset value of the target register.
 *
 * @return   The 32 bit data of the target channel register .
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_ReadRegChn(FAxidmaPsu_Chn_T *ChannelPtr, int offset)
{
    return FMSH_ReadReg(ChannelPtr->axidma->base_address,
                        (AXIDMA_CHX_OFFSET * (ChannelPtr->id + 1)) + offset);
}

/*****************************************************************************
 * This function is used to read 64 bit data of one AXIDMA channel register.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    offset is the offset value of the target register.
 *
 * @return   The 64 bit data of the target channel register .
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_ReadRegChn64(FAxidmaPsu_Chn_T *ChannelPtr, int offset)
{
    return FMSH_ReadReg64(ChannelPtr->axidma->base_address,
                          (AXIDMA_CHX_OFFSET * (ChannelPtr->id + 1)) + offset);
}

/*****************************************************************************
 * This function is used to write 32 bit data into one AXIDMA register.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    offset is the offset value of the target register.
 * @param    v is the write data.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_WriteReg(FAxidmaPsu_T *InstancePtr, int offset, u32 v)
{
    FMSH_WriteReg(InstancePtr->base_address, offset, v);
}

/*****************************************************************************
 * This function is used to write 64 bit data into one register.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    offset is the offset value of the target register.
 * @param    v is the write data.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_WriteReg64(FAxidmaPsu_T *InstancePtr, int offset, u64 v)
{
    FMSH_WriteReg64(InstancePtr->base_address, offset, v);
}

/*****************************************************************************
 * This function is used to write 32 bit data into one AXIDMA channel register.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    offset is the offset value of the target register.
 * @param    v is the write data.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_WriteRegChn(FAxidmaPsu_Chn_T *ChannelPtr, int offset, u32 v)
{
    FMSH_WriteReg(ChannelPtr->axidma->base_address,
                  (AXIDMA_CHX_OFFSET * (ChannelPtr->id + 1)) + offset, v);
}

/*****************************************************************************
 * This function is used to write 64 bit data into one AXIDMA channel register.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    offset is the offset value of the target register.
 * @param    v is the write data.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_WriteRegChn64(FAxidmaPsu_Chn_T *ChannelPtr, int offset, u64 v)
{
    FMSH_WriteReg64(ChannelPtr->axidma->base_address,
                    (AXIDMA_CHX_OFFSET * (ChannelPtr->id + 1)) + offset, v);
}

/*****************************************************************************
 * This function is used to reset dma.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 *
 * @return   - FMSH_SUCCESS if successful.
 *           - FMSH_FAILURE if fail.
 *
 * @note     None.
 *
 ******************************************************************************/
int FAxidmaPsu_Reset(FAxidmaPsu_T *InstancePtr)
{
    u32 val, count = 20;

    FAxidmaPsu_WriteReg(InstancePtr, AXIDMA_RESETREG_OFFSET, AXIDMA_RST_MASK);
    while (1)
    {
        delay_us(20);
        val = FAxidmaPsu_ReadReg(InstancePtr, AXIDMA_RESETREG_OFFSET);
        if ((val & AXIDMA_RST_MASK) == 0) return FMSH_SUCCESS;

        if (count-- == 0) break;
    }

    return FMSH_FAILURE;
}

/*****************************************************************************
 * This function is used to disable dma.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_DisableDma(FAxidmaPsu_T *InstancePtr)
{
    u32 val = FAxidmaPsu_ReadReg(InstancePtr, AXIDMA_CFGREG_OFFSET);
    val &= ~AXIDMA_CFGREG_DMAC_EN_MASK;
    FAxidmaPsu_WriteReg(InstancePtr, AXIDMA_CFGREG_OFFSET, val);
}

/*****************************************************************************
 * This function is used to enable dma.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_EnableDma(FAxidmaPsu_T *InstancePtr)
{
    u32 val = FAxidmaPsu_ReadReg(InstancePtr, AXIDMA_CFGREG_OFFSET);
    val |= AXIDMA_CFGREG_DMAC_EN_MASK;
    FAxidmaPsu_WriteReg(InstancePtr, AXIDMA_CFGREG_OFFSET, val);
}

/*****************************************************************************
 * This function is used to globally disable the interrupt generation.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_DisableIrqGlobal(FAxidmaPsu_T *InstancePtr)
{
    u32 val = FAxidmaPsu_ReadReg(InstancePtr, AXIDMA_CFGREG_OFFSET);
    val &= ~AXIDMA_CFGREG_INT_EN_MASK;
    FAxidmaPsu_WriteReg(InstancePtr, AXIDMA_CFGREG_OFFSET, val);
}

/*****************************************************************************
 * This function is used to globally enable the interrupt generation.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_EnableIrqGlobal(FAxidmaPsu_T *InstancePtr)
{
    u32 val = FAxidmaPsu_ReadReg(InstancePtr, AXIDMA_CFGREG_OFFSET);
    val |= AXIDMA_CFGREG_INT_EN_MASK;
    FAxidmaPsu_WriteReg(InstancePtr, AXIDMA_CFGREG_OFFSET, val);
}

/*****************************************************************************
 * This function is used to disable the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_DisableChn(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val = FAxidmaPsu_ReadReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET);
    val &= (~(AXIDMA_CHENREG_CHX_EN_MASK(ChannelPtr->id)));
    val |= AXIDMA_CHENREG_CHX_EN_WE_MASK(ChannelPtr->id);
    FAxidmaPsu_WriteReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET, val);
}

/*****************************************************************************
 * This function is used to enable the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_EnableChn(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val = FAxidmaPsu_ReadReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET);
    val |= AXIDMA_CHENREG_CHX_EN_MASK(ChannelPtr->id) |
           AXIDMA_CHENREG_CHX_EN_WE_MASK(ChannelPtr->id);
    FAxidmaPsu_WriteReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET, val);
}

/*****************************************************************************
 * This function is used to suspend the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SuspendChn(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val = FAxidmaPsu_ReadReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET);
    val |= (AXIDMA_CHENREG_CHX_SUSP_MASK(ChannelPtr->id) |
           AXIDMA_CHENREG_CHX_SUSP_WE_MASK(ChannelPtr->id));
    FAxidmaPsu_WriteReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET, val);
}

/*****************************************************************************
 * This function is used to resume the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_ResumeChn(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val = FAxidmaPsu_ReadReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET);
    val &= (~(AXIDMA_CHENREG_CHX_SUSP_MASK(ChannelPtr->id)));
    val |= AXIDMA_CHENREG_CHX_SUSP_WE_MASK(ChannelPtr->id);

    FAxidmaPsu_WriteReg(ChannelPtr->axidma, AXIDMA_CHENREG_L_OFFSET, val);
}

/*****************************************************************************
 * This function is used to abort the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_AbortChn(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val = FAxidmaPsu_ReadReg(ChannelPtr->axidma, AXIDMA_CHENREG_H_OFFSET);
    val |= AXIDMA_CHENREG_CHX_ABORT_MASK(ChannelPtr->id) |
           AXIDMA_CHENREG_CHX_ABORT_WE_MASK(ChannelPtr->id);
    FAxidmaPsu_WriteReg(ChannelPtr->axidma, AXIDMA_CHENREG_H_OFFSET, val);
}

/*****************************************************************************
 * This function is used to clear the interrupt status of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    irq_mask is write mask to clear specific irq status bit.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_ClearChnIrq(FAxidmaPsu_Chn_T *ChannelPtr, u32 irq_mask)
{
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_INTCLEARREG_OFFSET, irq_mask);
}

/*****************************************************************************
 * This function is used to clear the common interrupt status of the AXIDMA.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    irq_mask is write mask to clear specific irq status bit.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_ClearCmnIrq(FAxidmaPsu_T *InstancePtr, u32 irq_mask)
{
    FAxidmaPsu_WriteReg(InstancePtr, AXIDMA_COMMONREG_INTCLEARREG_OFFSET,
                       irq_mask);
}

/*****************************************************************************
 * This function is used to read the interrupt status of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of the channel's interrupt status.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_GetChnIrq(FAxidmaPsu_Chn_T *ChannelPtr)
{
    return FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_INTSTATUS_OFFSET);
}

/*****************************************************************************
 * This function is used to read the common interrupt status of the AXIDMA.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 *
 * @return   The value of the common interrupt status.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_GetCmnIrq(FAxidmaPsu_T *InstancePtr)
{
    return FAxidmaPsu_ReadReg(InstancePtr, AXIDMA_COMMONREG_INTSTATUSREG_OFFSET);
}

/*****************************************************************************
 * This function is used to enable transfer related interrupt of the target
 * channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_EnableChnTrfIrq(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val =
        FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET);
    val &= ~AXIDMA_IRQ_CH_ALL_TRF_MASK;
    val |= AXIDMA_IRQ_CH_ALL_TRF_MASK;
    val |= AXIDMA_IRQ_CH_SHADOWREG_OR_LLI_INVALID_ERR_MASK;

    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET,
                          val);
}

/*****************************************************************************
 * This function is used to enable the interrupt of the target channel by a
 * mask.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    irq_mask is write mask to enable specific irq.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_EnableChnIrq(FAxidmaPsu_Chn_T *ChannelPtr, u32 irq_mask)
{
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET,
                          irq_mask);
}

/*****************************************************************************
 * This function is used to disable all interrupt of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_DisableChnIrqAll(FAxidmaPsu_Chn_T *ChannelPtr)
{
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET,
                          0x0);
}

/*****************************************************************************
 * This function is used to disable the interrupt of the target channel by a
 * mask.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    irq_mask is the mask for disabled interrupt.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_DisableChnIrq(FAxidmaPsu_Chn_T *ChannelPtr, u32 irq_mask)
{
    u32 val;

    val =
        FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET);
    val &= ~irq_mask;
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_INTSTATUS_ENABLEREG_OFFSET,
                          val);
}

/*****************************************************************************
 * This function is used to set SAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    sar is the source address of dma transfer.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetSar(FAxidmaPsu_Chn_T *ChannelPtr, u64 sar)
{
    ChannelPtr->chn_config.sar = sar;
    FAxidmaPsu_WriteRegChn64(ChannelPtr, AXIDMA_CHX_SAR_OFFSET, sar);
}

/*****************************************************************************
 * This function is used to read SAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of SAR register.
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_GetSar(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u64 val = FAxidmaPsu_ReadReg64(ChannelPtr->axidma, AXIDMA_CHX_SAR_OFFSET);
    ChannelPtr->chn_config.sar = val;

    return val;
}

/*****************************************************************************
 * This function is used to set DAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    dar is the destination address of dma transfer.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetDar(FAxidmaPsu_Chn_T *ChannelPtr, u64 dar)
{
    ChannelPtr->chn_config.dar = dar;
    FAxidmaPsu_WriteRegChn64(ChannelPtr, AXIDMA_CHX_DAR_OFFSET, dar);
}

/*****************************************************************************
 * This function is used to read DAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of AXIDMA_CHX_DAR register.
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_GetDar(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u64 val = FAxidmaPsu_ReadReg64(ChannelPtr->axidma, AXIDMA_CHX_DAR_OFFSET);
    ChannelPtr->chn_config.dar = val;

    return val;
}

/*****************************************************************************
 * This function is used to config BLOCK_TS register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    block_ts is the transfer number of one block.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetBlockTs(FAxidmaPsu_Chn_T *ChannelPtr, u32 block_ts)
{
    ChannelPtr->chn_config.block_ts = block_ts - 1;
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_BLOCK_TS_OFFSET, block_ts - 1);
}

/*****************************************************************************
 * This function is used to get the value of BLOCK_TS register of the
 * target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of AXIDMA_CHX_BLOCK_TS register.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_GetBlkTs(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_BLOCK_TS_OFFSET);
    val &= AXIDMA_CHX_BLOCK_TS_MASK;
    ChannelPtr->chn_config.block_ts = val;

    return (val + 1);
}

/*****************************************************************************
 * This function is used to set CTL register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    ctl_ptr is a pointer to the FAxidmaPsu_Ctl_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetCtl(FAxidmaPsu_Chn_T *ChannelPtr, FAxidmaPsu_Ctl_T *ctl_ptr)
{
    FAxidmaPsu_Ctl_T *ch_ctl_ptr = &(ChannelPtr->chn_config.ctl);
    (void)memcpy(ch_ctl_ptr, ctl_ptr, sizeof(FAxidmaPsu_Ctl_T));

    u64 val = FAxidmaPsu_GenCtl(ch_ctl_ptr);
    FAxidmaPsu_WriteRegChn64(ChannelPtr, AXIDMA_CHX_CTL_L_OFFSET, val);
}

/*****************************************************************************
 * This function is used to set CFG register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    cfg_ptr is a pointer to the FAxidmaPsu_Cfg_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetCfg(FAxidmaPsu_Chn_T *ChannelPtr, FAxidmaPsu_Cfg_T *cfg_ptr)
{
    FAxidmaPsu_Cfg_T *ch_cfg_ptr = &(ChannelPtr->chn_config.cfg);
    (void)memcpy(ch_cfg_ptr, cfg_ptr, sizeof(FAxidmaPsu_Cfg_T));

    u64 val = FAxidmaPsu_GenCfg(ch_cfg_ptr);
    FAxidmaPsu_WriteRegChn64(ChannelPtr, AXIDMA_CHX_CFG_L_OFFSET, val);
}

/*****************************************************************************
 * This function is used to set LLP register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    llp is the address of the first linked list node.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetLlp(FAxidmaPsu_Chn_T *ChannelPtr, u64 llp)
{
    ChannelPtr->chn_config.llp = llp;
    FAxidmaPsu_WriteRegChn64(ChannelPtr, AXIDMA_CHX_LLP_OFFSET, llp);
}

/*****************************************************************************
 * This function is used to get the value of the LLP register of the target
 * channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of the LLP register.
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_GetLlp(FAxidmaPsu_Chn_T *ChannelPtr)
{
    ChannelPtr->chn_config.llp =
        FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_LLP_OFFSET);
    return ChannelPtr->chn_config.llp;
}

/*****************************************************************************
 * This function is used to config AXI QOS register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    awqos is the config awqos value.
 * @param    arqos is the config arqos value.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetQos(FAxidmaPsu_Chn_T *ChannelPtr, u8 awqos, u8 arqos)
{
    u32 val;

    ChannelPtr->chn_config.awqos = awqos;
    ChannelPtr->chn_config.arqos = arqos;

    val = ((ChannelPtr->chn_config.awqos << AXIDMA_CHX_AXI_QOSREG_AWQOS_SHIFT) &
           AXIDMA_CHX_AXI_QOSREG_AWQOS_MASK) |
          ((ChannelPtr->chn_config.arqos << AXIDMA_CHX_AXI_QOSREG_ARQOS_SHIFT) &
           AXIDMA_CHX_AXI_QOSREG_ARQOS_MASK);
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_AXI_QOSREG_OFFSET, val);
}

/*****************************************************************************
 * This function is used to get AXI QOS register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_GetQos(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_AXI_QOSREG_OFFSET);

    ChannelPtr->chn_config.awqos = (val & AXIDMA_CHX_AXI_QOSREG_AWQOS_MASK) >>
                                   AXIDMA_CHX_AXI_QOSREG_AWQOS_SHIFT;
    ChannelPtr->chn_config.arqos = (val & AXIDMA_CHX_AXI_QOSREG_ARQOS_MASK) >>
                                   AXIDMA_CHX_AXI_QOSREG_ARQOS_SHIFT;
}

/*****************************************************************************
 * This function is used to config SSTATAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    sstatar is the config value.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetSstatar(FAxidmaPsu_Chn_T *ChannelPtr, u64 sstatar)
{
    ChannelPtr->chn_config.sstatar = sstatar;
    FAxidmaPsu_WriteRegChn64(ChannelPtr, AXIDMA_CHX_SSTATAR_OFFSET, sstatar);
}

/*****************************************************************************
 * This function is used to read SSTATAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of SSTATAR register.
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_GetSstatar(FAxidmaPsu_Chn_T *ChannelPtr)
{
    ChannelPtr->chn_config.sstatar =
        FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_SSTATAR_OFFSET);
    return ChannelPtr->chn_config.sstatar;
}

/*****************************************************************************
 * This function is used to config DSTATAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    dstatar is the config value.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetDstatar(FAxidmaPsu_Chn_T *ChannelPtr, u64 dstatar)
{
    ChannelPtr->chn_config.dstatar = dstatar;
    FAxidmaPsu_WriteRegChn64(ChannelPtr, AXIDMA_CHX_DSTATAR_OFFSET, dstatar);
}

/*****************************************************************************
 * This function is used to read DSTATAR register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of DSTATAR register.
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_GetDstatar(FAxidmaPsu_Chn_T *ChannelPtr)
{
    ChannelPtr->chn_config.sstatar =
        FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_DSTATAR_OFFSET);
    return ChannelPtr->chn_config.dstatar;
}

/*****************************************************************************
 * This function is used to read SSTAT register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of SSTAT register.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_GetSstat(FAxidmaPsu_Chn_T *ChannelPtr)
{
    ChannelPtr->chn_config.sstat =
        FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_SSTAT_OFFSET);
    return ChannelPtr->chn_config.sstat;
}

/*****************************************************************************
 * This function is used to read DSTAT register of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of DSTAT register.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_GetDstat(FAxidmaPsu_Chn_T *ChannelPtr)
{
    ChannelPtr->chn_config.dstat =
        FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_DSTAT_OFFSET);
    return ChannelPtr->chn_config.dstat;
}

/*****************************************************************************
 * This function is used to read the CMPLTD_BLK_TFR_SIZE of the STATUS register
 * of the target channel, which indicates the total number of data of width
 * CHx_CTL.SRC_TR_WIDTH transferred for the previous block transfer.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   The value of completed block transfer size.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_GetCompletedSize(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_STATUSREG_OFFSET);
    val = (val & AXIDMA_CHX_BLOCK_TS_MASK);

    return val;
}

/*****************************************************************************
 * This function is used to set the next block as the last block.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetLastBlk(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_CTL_H_OFFSET);
    val |= AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_MASK;
    val |= AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_MASK;

    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_CTL_H_OFFSET, val);
}

/*****************************************************************************
 * This function is used to set the next block as the non-last block.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetNonlastBlk(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_CTL_H_OFFSET);
    val &= (~AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_MASK);
    val |= AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_MASK;

    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_CTL_H_OFFSET, val);
}

/*****************************************************************************
 * This function is used to set the AXIDMA_CHX_BLK_TFR_RESUMEREQ register of the
 * target channel, which requests to resume the suspend tranfer druing LLI or
 * shadow register multiblock tranfer.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SetBlkResumeReq(FAxidmaPsu_Chn_T *ChannelPtr)
{
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_BLK_TFR_RESUMEREQREG_OFFSET,
                          AXIDMA_CHX_BLK_TFR_RESUMEREQ_MASK);
}

/*****************************************************************************
 * This function is used to config the software source handshaking register of
 * the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    val is the config value.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SendSwhsSrcReq(FAxidmaPsu_Chn_T *ChannelPtr, u32 val)
{
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_SWHSSRCREG_OFFSET_OFFSET, val);
}

/*****************************************************************************
 * This function is used to config the software destination handshaking register
 * of the target channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    val is the config value.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_SendSwhsDstReq(FAxidmaPsu_Chn_T *ChannelPtr, u32 val)
{
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_SWHSDSTREG_OFFSET_OFFSET, val);
}

/*****************************************************************************
 * This function is used to clear the MulblkType in CFG register of the target
 * channel.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_ClrBlktype(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 val;

    val = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_CFG_L_OFFSET);
    val &= ~(AXIDMA_CHX_CFG_SRC_MULTBLK_TYPE_MASK |
             AXIDMA_CHX_CFG_DST_MULTBLK_TYPE_MASK);
    FAxidmaPsu_WriteRegChn(ChannelPtr, AXIDMA_CHX_CFG_L_OFFSET, val);
}

/*****************************************************************************
 * This function is a default common error interrupt handler.
 *
 * @param    CallBackRef is a pointer to the FAxidmaPsu_T instance.
 * @param    Mask is a error code.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
static void FAxidmaPsu_StubHandler(void *CallBackRef, int Mask) {}

/*****************************************************************************
 * This function is used to initialize the AXIDMA instance. It will disable dma
 * and the global interrupt, and the channels' too.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_HwInit(FAxidmaPsu_T *InstancePtr)
{
    int i;

    FAxidmaPsu_DisableIrqGlobal(InstancePtr);
    InstancePtr->cmn_err_handler =
        (FAxidmaPsu_Handler)((void *)FAxidmaPsu_StubHandler);
    FAxidmaPsu_DisableDma(InstancePtr);
    for (i = 0; i < InstancePtr->ch_num_max; i++)
    {
        FAxidmaPsu_Chn_T *ChannelPtr = &(InstancePtr->chn[i]);
        ChannelPtr->axidma = InstancePtr;
        ChannelPtr->id = i;
        (void)memset(&ChannelPtr->chn_config, 0, sizeof(FAxidmaPsu_ChnConfig_T));
        ChannelPtr->chn_err_handler =
            (FAxidmaPsu_Handler)((void *)FAxidmaPsu_StubHandler);
        ChannelPtr->trf_done_handler =
            (FAxidmaPsu_Handler)((void *)FAxidmaPsu_StubHandler);
        FAxidmaPsu_DisableChnIrqAll(&InstancePtr->chn[i]);
        FAxidmaPsu_DisableChn(&InstancePtr->chn[i]);
    }
}

/*****************************************************************************
 * This function is used to config the channel's CFG/SAR/DAR/BLOCK_TS/CTL.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    chn_config_ptr is a pointer to the FAxidmaPsu_ChnConfig_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_ChnConfig(FAxidmaPsu_Chn_T *ChannelPtr,
                         FAxidmaPsu_ChnConfig_T *chn_config_ptr)
{
    // set CFG register
    FAxidmaPsu_SetCfg(ChannelPtr, &(chn_config_ptr->cfg));

    if ((chn_config_ptr->cfg.src_mltblk_type == axidma_linked_list) &&
        (chn_config_ptr->cfg.dst_mltblk_type == axidma_linked_list))
    {
        // llp need to be configured
        FAxidmaPsu_SetLlp(ChannelPtr, chn_config_ptr->llp);
    }
    else if ((chn_config_ptr->cfg.src_mltblk_type != axidma_linked_list) &&
             (chn_config_ptr->cfg.dst_mltblk_type != axidma_linked_list))
    {
        // sar, dar, block_ts, ctl need to be configured
        FAxidmaPsu_SetSar(ChannelPtr, chn_config_ptr->sar);
        FAxidmaPsu_SetDar(ChannelPtr, chn_config_ptr->dar);
        FAxidmaPsu_SetBlockTs(ChannelPtr, chn_config_ptr->block_ts);
        FAxidmaPsu_SetCtl(ChannelPtr, &(chn_config_ptr->ctl));
    }
    else
    {
        // (sar, dar, block_ts, ctl) + llp
        FAxidmaPsu_SetSar(ChannelPtr, chn_config_ptr->sar);
        FAxidmaPsu_SetDar(ChannelPtr, chn_config_ptr->dar);
        FAxidmaPsu_SetBlockTs(ChannelPtr, chn_config_ptr->block_ts);
        FAxidmaPsu_SetCtl(ChannelPtr, &(chn_config_ptr->ctl));
        FAxidmaPsu_SetLlp(ChannelPtr, chn_config_ptr->llp);
    }
}

/*****************************************************************************
 * This function is used to get the value of the channel's CFG/SAR/DAR/BLOCK_TS/
 * CTL/LLP/SSTAT/DSTAT/SSTATAR/DSTATAR.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_GetChnConfig(FAxidmaPsu_Chn_T *ChannelPtr)
{
    u32 reg;
    FAxidmaPsu_ChnConfig_T *chn_config;

    chn_config = &ChannelPtr->chn_config;

    FAxidmaPsu_Cfg_T *ch_cfg_ptr;
    ch_cfg_ptr = &chn_config->cfg;

    FAxidmaPsu_Ctl_T *ch_ctl_ptr;
    ch_ctl_ptr = &chn_config->ctl;

    // read CFG_L register
    reg = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_CFG_L_OFFSET);
    ch_cfg_ptr->src_mltblk_type =
        (FAxidmaPsu_MltblkType_E)(reg & AXIDMA_CHX_CFG_SRC_MULTBLK_TYPE_MASK);
    ch_cfg_ptr->dst_mltblk_type =
        (FAxidmaPsu_MltblkType_E)((reg & AXIDMA_CHX_CFG_DST_MULTBLK_TYPE_MASK) >>
                                 AXIDMA_CHX_CFG_DST_MULTBLK_TYPE_SHIFT);

    // read CFG_H register
    reg = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_CFG_H_OFFSET);
    ch_cfg_ptr->tt_fc =
        (FAxidmaPsu_TransFlow_E)(reg & AXIDMA_CHX_CFG_TT_FC_MASK);
    ch_cfg_ptr->hs_sel_src =
        (FAxidmaPsu_HsSelect_E)((reg & AXIDMA_CHX_CFG_HG_SEL_SRC_MASK) >>
                               AXIDMA_CHX_CFG_HG_SEL_SRC_SHIFT);
    ch_cfg_ptr->hs_sel_dst =
        (FAxidmaPsu_HsSelect_E)((reg & AXIDMA_CHX_CFG_HG_SEL_DST_MASK) >>
                               AXIDMA_CHX_CFG_HG_SEL_DST_SHIFT);
    ch_cfg_ptr->src_per =
        (FAxidmaPsu_HsIf_E)((reg & AXIDMA_CHX_CFG_SRC_PER_MASK) >>
                           AXIDMA_CHX_CFG_SRC_PER_SHIFT);
    ch_cfg_ptr->dst_per =
        (FAxidmaPsu_HsIf_E)((reg & AXIDMA_CHX_CFG_DST_PER_MASK) >>
                           AXIDMA_CHX_CFG_DST_PER_SHIFT);
    ch_cfg_ptr->ch_prior =
        (FAxidmaPsu_ChanPrior_E)((reg & AXIDMA_CHX_CFG_CH_PRIOR_MASK) >>
                                AXIDMA_CHX_CFG_CH_PRIOR_SHIFT);

    // read CTL_L register
    reg = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_CTL_L_OFFSET);
    ch_ctl_ptr->sinc =
        (FAxidmaPsu_BurstType_E)((reg & AXIDMA_CHX_CTL_SINC_MASK) >>
                                AXIDMA_CHX_CTL_SINC_SHIFT);
    ch_ctl_ptr->dinc =
        (FAxidmaPsu_BurstType_E)((reg & AXIDMA_CHX_CTL_DINC_MASK) >>
                                AXIDMA_CHX_CTL_DINC_SHIFT);
    ch_ctl_ptr->src_tr_width =
        (FAxidmaPsu_BurstWidth_E)((reg & AXIDMA_CHX_CTL_SRC_TR_WIDTH_MASK) >>
                                 AXIDMA_CHX_CTL_SRC_TR_WIDTH_SHIFT);
    ch_ctl_ptr->dst_tr_width =
        (FAxidmaPsu_BurstWidth_E)((reg & AXIDMA_CHX_CTL_DST_TR_WIDTH_MASK) >>
                                 AXIDMA_CHX_CTL_DST_TR_WIDTH_SHIFT);
    ch_ctl_ptr->src_msize =
        (FAxidmaPsu_BurstLen_E)((reg & AXIDMA_CHX_CTL_SRC_MSIZE_MASK) >>
                               AXIDMA_CHX_CTL_SRC_MSIZE_SHIFT);
    ch_ctl_ptr->dst_msize =
        (FAxidmaPsu_BurstLen_E)((reg & AXIDMA_CHX_CTL_DST_MSIZE_MASK) >>
                               AXIDMA_CHX_CTL_DST_MSIZE_SHIFT);
    ch_ctl_ptr->ar_cache = (u8)((reg & AXIDMA_CHX_CTL_AR_CACHE_MASK) >>
                                AXIDMA_CHX_CTL_AR_CACHE_SHIFT);
    ch_ctl_ptr->aw_cache = (u8)((reg & AXIDMA_CHX_CTL_AW_CACHE_MASK) >>
                                AXIDMA_CHX_CTL_AW_CACHE_SHIFT);
    ch_ctl_ptr->nonposted_lastwrite_en =
        ((reg & AXIDMA_CHX_CTL_NONPOSTED_LASTWRITE_EN_MASK) >>
         AXIDMA_CHX_CTL_NONPOSTED_LASTWRITE_EN_SHIFT);

    // read CTL_H register
    reg = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_CTL_H_OFFSET);
    ch_ctl_ptr->ar_prot = (u8)(reg & AXIDMA_CHX_CTL_AR_PROT_MASK);
    ch_ctl_ptr->aw_prot = (u8)((reg & AXIDMA_CHX_CTL_AW_PROT_MASK) >>
                               AXIDMA_CHX_CTL_AW_PROT_SHIFT);
    ch_ctl_ptr->arlen_en =
        ((reg & AXIDMA_CHX_CTL_ARLEN_EN_MASK) >> AXIDMA_CHX_CTL_ARLEN_EN_SHIFT);
    ch_ctl_ptr->arlen =
        (u8)((reg & AXIDMA_CHX_CTL_ARLEN_MASK) >> AXIDMA_CHX_CTL_ARLEN_SHIFT);
    ch_ctl_ptr->awlen_en =
        ((reg & AXIDMA_CHX_CTL_AWLEN_EN_MASK) >> AXIDMA_CHX_CTL_AWLEN_EN_SHIFT);
    ch_ctl_ptr->awlen =
        (u8)((reg & AXIDMA_CHX_CTL_AWLEN_MASK) >> AXIDMA_CHX_CTL_AWLEN_SHIFT);
    ch_ctl_ptr->src_stat_en = ((reg & AXIDMA_CHX_CTL_SRC_STAT_EN_MASK) >>
                               AXIDMA_CHX_CTL_SRC_STAT_EN_SHIFT);
    ch_ctl_ptr->dst_stat_en = ((reg & AXIDMA_CHX_CTL_DST_STAT_EN_MASK) >>
                               AXIDMA_CHX_CTL_DST_STAT_EN_SHIFT);
    ch_ctl_ptr->ioc_blktfr = ((reg & AXIDMA_CHX_CTL_IOC_BLKTFR_MASK) >>
                              AXIDMA_CHX_CTL_IOC_BLKTFR_SHIFT);
    ch_ctl_ptr->shadowreg_or_lli_last =
        ((reg & AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_MASK) >>
         AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_SHIFT);
    ch_ctl_ptr->shadowreg_or_lli_valid =
        ((reg & AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_MASK) >>
         AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_SHIFT);

    // read SAR/DAR register
    chn_config->sar = FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_SAR_OFFSET);
    chn_config->dar = FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_DAR_OFFSET);

    // read BLOCK_TS register
    reg = FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_BLOCK_TS_OFFSET);
    chn_config->block_ts = (reg & AXIDMA_CHX_BLOCK_TS_MASK) + 1;

    // read LLP register
    chn_config->llp = FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_LLP_OFFSET);

    // read the SSTAT/DSTAT SSTATAR/DSTATAR registers
    chn_config->sstat =
        FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_SSTAT_OFFSET);
    chn_config->dstat =
        FAxidmaPsu_ReadRegChn(ChannelPtr, AXIDMA_CHX_DSTAT_OFFSET);

    chn_config->sstatar =
        FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_SSTATAR_OFFSET);
    chn_config->dstatar =
        FAxidmaPsu_ReadRegChn64(ChannelPtr, AXIDMA_CHX_DSTATAR_OFFSET);
}

/*****************************************************************************
 * This function is used to get a free channel.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T pointer.
 *
 * @return   - FMSH_SUCCESS if get a free channel pointer.
 *           - FMSH_FAILURE if don't get a free channel pointer.
 *
 * @note     None.
 *
 ******************************************************************************/
int FAxidmaPsu_GetFreeChannel(FAxidmaPsu_T *InstancePtr,
                             FAxidmaPsu_Chn_T **ChannelPtr)
{
    u32 reg = 0;
    reg = FAxidmaPsu_ReadReg(InstancePtr, AXIDMA_CHENREG_L_OFFSET);

    for (int channel_index = 0; channel_index < InstancePtr->ch_num_max;
         channel_index++)
    {
        if (!(reg & AXIDMA_CHENREG_CHX_EN_MASK(channel_index)))
        {
            *ChannelPtr = &(InstancePtr->chn[channel_index]);
            return FMSH_SUCCESS;
        }
    }

    return FMSH_FAILURE;
}

/*****************************************************************************
 * This function is used to generate register CTL config.
 *
 * @param    ctl_ptr is a pointer to the FAxidmaPsu_Ctl_T instance.
 *
 * @return   CTL config value.
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_GenCtl(FAxidmaPsu_Ctl_T *ctl_ptr)
{
    u64 val_l = 0x0;
    /* set address increment */
    val_l |=
        (ctl_ptr->sinc << AXIDMA_CHX_CTL_SINC_SHIFT) & AXIDMA_CHX_CTL_SINC_MASK;
    val_l |=
        (ctl_ptr->dinc << AXIDMA_CHX_CTL_DINC_SHIFT) & AXIDMA_CHX_CTL_DINC_MASK;

    /* set transfer width */
    val_l |= (ctl_ptr->src_tr_width << AXIDMA_CHX_CTL_SRC_TR_WIDTH_SHIFT) &
             AXIDMA_CHX_CTL_SRC_TR_WIDTH_MASK;
    val_l |= (ctl_ptr->dst_tr_width << AXIDMA_CHX_CTL_DST_TR_WIDTH_SHIFT) &
             AXIDMA_CHX_CTL_DST_TR_WIDTH_MASK;

    /* set burst length */
    val_l |= (ctl_ptr->src_msize << AXIDMA_CHX_CTL_SRC_MSIZE_SHIFT) &
             AXIDMA_CHX_CTL_SRC_MSIZE_MASK;
    val_l |= (ctl_ptr->dst_msize << AXIDMA_CHX_CTL_DST_MSIZE_SHIFT) &
             AXIDMA_CHX_CTL_DST_MSIZE_MASK;

    /* set axcache */
    val_l |= (ctl_ptr->ar_cache << AXIDMA_CHX_CTL_AR_CACHE_SHIFT) &
             AXIDMA_CHX_CTL_AR_CACHE_MASK;
    val_l |= (ctl_ptr->aw_cache << AXIDMA_CHX_CTL_AW_CACHE_SHIFT) &
             AXIDMA_CHX_CTL_AW_CACHE_MASK;

    u64 val_h = 0x0;
    
    /* set axprot */
    val_h |= (ctl_ptr->ar_prot << AXIDMA_CHX_CTL_AR_PROT_SHIFT) &
             AXIDMA_CHX_CTL_AR_PROT_MASK;
    val_h |= (ctl_ptr->aw_prot << AXIDMA_CHX_CTL_AW_PROT_SHIFT) &
             AXIDMA_CHX_CTL_AW_PROT_MASK;
    
    /* burst len enable */
    val_h |= (ctl_ptr->arlen_en << AXIDMA_CHX_CTL_ARLEN_EN_SHIFT) &
             AXIDMA_CHX_CTL_ARLEN_EN_MASK;
    val_h |= (ctl_ptr->arlen << AXIDMA_CHX_CTL_ARLEN_SHIFT) &
             AXIDMA_CHX_CTL_ARLEN_MASK;
    val_h |= (ctl_ptr->awlen_en << AXIDMA_CHX_CTL_AWLEN_EN_SHIFT) &
             AXIDMA_CHX_CTL_AWLEN_EN_MASK;
    val_h |= (ctl_ptr->awlen << AXIDMA_CHX_CTL_AWLEN_SHIFT) &
             AXIDMA_CHX_CTL_AWLEN_MASK;

    /* set status enable */
    val_h |= (ctl_ptr->src_stat_en << AXIDMA_CHX_CTL_SRC_STAT_EN_SHIFT) &
             AXIDMA_CHX_CTL_SRC_STAT_EN_MASK;
    val_h |= (ctl_ptr->dst_stat_en << AXIDMA_CHX_CTL_DST_STAT_EN_SHIFT) &
             AXIDMA_CHX_CTL_DST_STAT_EN_MASK;

    /* set shadow blk done interrupt */
    val_h |= (ctl_ptr->ioc_blktfr << AXIDMA_CHX_CTL_IOC_BLKTFR_SHIFT) &
             AXIDMA_CHX_CTL_IOC_BLKTFR_MASK;

    /* set last block */
    val_h |= (ctl_ptr->shadowreg_or_lli_last
              << AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_SHIFT) &
             AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_LAST_MASK;

    /* set shadow/lli valid */
    val_h |= (ctl_ptr->shadowreg_or_lli_valid
              << AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_SHIFT) &
             AXIDMA_CHX_CTL_SHADOWREG_OR_LLI_VALID_MASK;

    return ((val_h << 32) | val_l);
}

/*****************************************************************************
 * This function is used to generate register CFG config.
 *
 * @param    cfg_ptr is a pointer to the FAxidmaPsu_Cfg_T instance.
 *
 * @return   CFG config value.
 *
 * @note     None.
 *
 ******************************************************************************/
u64 FAxidmaPsu_GenCfg(FAxidmaPsu_Cfg_T *cfg_ptr)
{
    u64 val_l, val_h;

    /* set multi_blk type */
    val_l = 0x0;
    val_l |=
        (cfg_ptr->src_mltblk_type << AXIDMA_CHX_CFG_SRC_MULTBLK_TYPE_SHIFT) &
        AXIDMA_CHX_CFG_SRC_MULTBLK_TYPE_MASK;
    val_l |=
        (cfg_ptr->dst_mltblk_type << AXIDMA_CHX_CFG_DST_MULTBLK_TYPE_SHIFT) &
        AXIDMA_CHX_CFG_DST_MULTBLK_TYPE_MASK;

    val_h = 0x0;
    /* set flow control */
    val_h |= (cfg_ptr->tt_fc << AXIDMA_CHX_CFG_TT_FC_SHIFT) &
             AXIDMA_CHX_CFG_TT_FC_MASK;
    /* set handshaking */
    val_h |= (cfg_ptr->hs_sel_src << AXIDMA_CHX_CFG_HG_SEL_SRC_SHIFT) &
             AXIDMA_CHX_CFG_HG_SEL_SRC_MASK;
    val_h |= (cfg_ptr->hs_sel_dst << AXIDMA_CHX_CFG_HG_SEL_DST_SHIFT) &
             AXIDMA_CHX_CFG_HG_SEL_DST_MASK;
    /* set handshaking interface */
    val_h |= (cfg_ptr->src_per << AXIDMA_CHX_CFG_SRC_PER_SHIFT) &
             AXIDMA_CHX_CFG_SRC_PER_MASK;
    val_h |= (cfg_ptr->dst_per << AXIDMA_CHX_CFG_DST_PER_SHIFT) &
             AXIDMA_CHX_CFG_DST_PER_MASK;
    /* set channel priority */
    val_h |= (cfg_ptr->ch_prior << AXIDMA_CHX_CFG_CH_PRIOR_SHIFT) &
             AXIDMA_CHX_CFG_CH_PRIOR_MASK;

    return ((val_h << 32) | val_l);
}

/*****************************************************************************
 * This function is used to create a lli entry of a linked list.
 *
 * @param    lli_ptr is a pointer to the FAxidmaPsu_Lli_T instance.
 * @param    next_lli_ptr is a pointer to the FAxidmaPsu_Lli_T instance.
 * @param    sar is the source address of the target node.
 * @param    dar is the destination address of the target node.
 * @param    block_ts is the transfer number of the target node.
 * @param    ctl is the CTL register config of the target node.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void fmsh_axidma_create_lli_entry(FAxidmaPsu_Lli_T *lli_ptr,
                                  FAxidmaPsu_Lli_T *next_lli_ptr, u64 sar,
                                  u64 dar, u32 block_ts, u64 ctl)
{
    lli_ptr->sar = sar;
    lli_ptr->dar = dar;
    lli_ptr->block_ts = block_ts - 1;
    lli_ptr->ctl = ctl;
    lli_ptr->llp = (u64)next_lli_ptr;
}

/*****************************************************************************
 * This function is used to deal with the channel interrupt.
 *
 * @param    CallBackRef is a pointer to the channel.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_ChnIrqHandler(void *CallBackRef)
{
    u32 status = 0;
    FAxidmaPsu_Chn_T *ChannelPtr = (FAxidmaPsu_Chn_T *)CallBackRef;

    status = FAxidmaPsu_GetChnIrq(ChannelPtr);

    if (status & AXIDMA_IRQ_CH_ALL_TRF_MASK)
    {
        ChannelPtr->trf_done_handler(ChannelPtr,
                                     status & AXIDMA_IRQ_CH_ALL_TRF_MASK);
        FAxidmaPsu_ClearChnIrq(ChannelPtr, status & AXIDMA_IRQ_CH_ALL_TRF_MASK);	
    }

    if (status & AXIDMA_IRQ_CH_ALL_ERR_MASK)
    {
        FAxidmaPsu_ClearChnIrq(ChannelPtr, status & AXIDMA_IRQ_CH_ALL_ERR_MASK);
        ChannelPtr->chn_err_handler(ChannelPtr,
                                    status & AXIDMA_IRQ_CH_ALL_ERR_MASK);
    }
}

/*****************************************************************************
 * This function is used to deal with the common interrupt.
 *
 * @param    CallBackRef is a pointer to the axidma device.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FAxidmaPsu_CmnIrqHandler(void *CallBackRef)
{
    FAxidmaPsu_T *InstancePtr = (FAxidmaPsu_T *)CallBackRef;

    u32 status = FAxidmaPsu_GetCmnIrq(InstancePtr);

    // ERR INTERRUPT
    if (status & AXIDMA_IRQ_CMN_ALL_ERR_MASK)
    {
        InstancePtr->cmn_err_handler(InstancePtr,
                                     status & AXIDMA_IRQ_CMN_ALL_ERR_MASK);
        FAxidmaPsu_ClearCmnIrq(InstancePtr, status & AXIDMA_IRQ_CMN_ALL_ERR_MASK);
    }
}

/*****************************************************************************
 * This function is used to set callback functions for interrupt.
 *
 * @param    ChannelPtr is a pointer to the FAxidmaPsu_Chn_T instance.
 * @param    HandlerType is the handler functional type.
 * @param    CallBackFunc is a callback function for channel interrupt.
 *
 * @return   - FMSH_SUCCESS if set callback function successfully.
 *           - FMSH_FAILURE if fail to set callback function.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_SetCallBackChn(FAxidmaPsu_Chn_T *ChannelPtr,
                             FAxidmaPsu_ChnHandler_E HandlerType,
                             void *CallBackFunc)
{
    /* Verify arguments. */
    FMSH_ASSERT(ChannelPtr != NULL);
    FMSH_ASSERT(CallBackFunc != NULL);
    FMSH_ASSERT((HandlerType == FAXIDMAPS_CHNHANDLER_DONE) ||
                (HandlerType == FAXIDMAPS_CHNHANDLER_ERROR));

    /*
     * Calls the respective callback function corresponding to
     * the handler type
     */
    if (HandlerType == FAXIDMAPS_CHNHANDLER_DONE)
    {
        ChannelPtr->trf_done_handler =
            (FAxidmaPsu_Handler)((void *)CallBackFunc);
    }
    else if (HandlerType == FAXIDMAPS_CHNHANDLER_ERROR)
    {
        ChannelPtr->chn_err_handler = (FAxidmaPsu_Handler)((void *)CallBackFunc);
    }
    else
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/*****************************************************************************
 * This function is used to set callback functions for interrupt.
 *
 * @param    InstancePtr is a pointer to the FAxidmaPsu_T instance.
 * @param    HandlerType is the handler functional type.
 * @param    CallBackFunc is a callback function for common interrupt.
 *
 * @return   - FMSH_SUCCESS if set callback function successfully.
 *           - FMSH_FAILURE if fail to set callback function.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FAxidmaPsu_SetCallBackCmn(FAxidmaPsu_T *InstancePtr,
                             FAxidmaPsu_CmnHandler_E HandlerType,
                             void *CallBackFunc)
{
    /* Verify arguments. */
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(CallBackFunc != NULL);
    FMSH_ASSERT(HandlerType == FAXIDMAPS_CMNHANDLER_ERROR);

    /*
     * Calls the respective callback function corresponding to
     * the handler type
     */
    if (HandlerType == FAXIDMAPS_CMNHANDLER_ERROR)
    {
        InstancePtr->cmn_err_handler =
            (FAxidmaPsu_Handler)((void *)CallBackFunc);
    }
    else
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}