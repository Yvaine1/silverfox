/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_pmu.c
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
 * 0.01   lq  02/23/2024  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

void FmshFsbl_MarkUsedRPUCores (BootPs *BootInstance, u32 PartitionNum)
{
    u32 DestCpu, RegValue;

    DestCpu = FmshFsbl_GetDestinationCpu(
        &BootInstance->ImageHeader.PartitionHeader[PartitionNum]);

    RegValue = Fmsh_In32(FSBL_R5_USAGE_STATUS_REG);

    /*
     * Check if any RPU core is used. If it is used set particular bit of
     * that core to indicate PMU that it is used and it is not need to
     * power down.
     */
    switch (DestCpu)
    {
    case IH_PH_ATTRB_DEST_CPU_R5_0:
    case IH_PH_ATTRB_DEST_CPU_R5_L:
        Fmsh_Out32(FSBL_R5_USAGE_STATUS_REG, RegValue | FSBL_R5_0_STATUS_MASK);
        break;
    case IH_PH_ATTRB_DEST_CPU_R5_1:
        Fmsh_Out32(FSBL_R5_USAGE_STATUS_REG, RegValue | FSBL_R5_1_STATUS_MASK);
        break;
    case IH_PH_ATTRB_DEST_CPU_NONE:
        if ((BootInstance->ProcessorID == IH_PH_ATTRB_DEST_CPU_R5_0) ||
            (BootInstance->ProcessorID == IH_PH_ATTRB_DEST_CPU_R5_L))
        {
            Fmsh_Out32(FSBL_R5_USAGE_STATUS_REG,
                       RegValue | FSBL_R5_0_STATUS_MASK);
        }
        break;
     default:
       break;
    }
}
