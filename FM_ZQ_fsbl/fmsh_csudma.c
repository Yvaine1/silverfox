/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_csudma.c
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
 * 0.01   lq  08/28/2022  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/*******************************************************************************
 *
 * This function is used to initialize csu-dma.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
void FmshFsbl_CsuInitiateDma (u32 Source, u32 Dest, u32 SrcWordLength,
                              u32 DestWordLength)
{
#if DCACHE_ENABLE == 1
    Fmsh_DCacheInvalidateRange(Source, SrcWordLength * 4U);
    Fmsh_DCacheInvalidateRange(Dest, DestWordLength * 4U);
#endif
    WriteReg(FPS_CSU_BASEADDR + SAC_DMA_SRC_ADDR_OFFSET, Source);

    WriteReg(FPS_CSU_BASEADDR + SAC_DMA_DEST_ADDR_OFFSET, Dest);

    WriteReg(FPS_CSU_BASEADDR + SAC_DMA_SRC_LEN_OFFSET, SrcWordLength);

    WriteReg(FPS_CSU_BASEADDR + SAC_DMA_DEST_LEN_OFFSET, DestWordLength);
}

/*******************************************************************************
 *
 * This function will be in busy while loop until the data transfer is
 * completed.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_CsuDmaPollDone (u32 MaskValue, u32 MaxCount)
{
    u32 Count = MaxCount;
    u32 IntrStsReg = 0U;

    // poll for the DMA done
    IntrStsReg = ReadReg(FPS_CSU_BASEADDR + SAC_INT_STS_OFFSET);

    while ((IntrStsReg & MaskValue) != MaskValue)
    {
        IntrStsReg = ReadReg(FPS_CSU_BASEADDR + SAC_INT_STS_OFFSET);

        Count -= 1U;

        if (IntrStsReg & IXR_ERROR_FLAGS_MASK)
        {
            return FMSH_FAILURE;
        }

        if (!Count)
        {
            return FMSH_FAILURE;
        }
        delay_1us();
    }
    WriteReg(FPS_CSU_BASEADDR + SAC_INT_STS_OFFSET, IntrStsReg & MaskValue);
    return FMSH_SUCCESS;
}
