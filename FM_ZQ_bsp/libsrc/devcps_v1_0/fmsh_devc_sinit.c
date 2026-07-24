/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_devc_sint.c
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/27/2019  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_devc_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FDevcPs_Config FDevcPs_ConfigTable[];

/************************** Function Prototypes ******************************/

/****************************************************************************
 *
 *  This function look up the devc by device id.
 *
 * @param   DeviceId is device id.
 *
 * @return
 * - point to the corresponding device
 * - NULL invalid device id
 *
 * @note    none
 *
 ****************************************************************************/
FDevcPs_Config *FDevcPs_LookupConfig (u16 DeviceId)
{
    FDevcPs_Config *CfgPtr = NULL;

    u32 Index = 0U;

    for (Index = 0U; Index < FPAR_DEVCPS_NUM_INSTANCES; Index++)
    {
        if (FDevcPs_ConfigTable[Index].DeviceId == DeviceId)
        {
            CfgPtr = &FDevcPs_ConfigTable[Index];
            break;
        }
    }

    return (FDevcPs_Config *)CfgPtr;
}
