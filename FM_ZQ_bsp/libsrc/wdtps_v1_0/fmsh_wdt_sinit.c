/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_wdt_sint.c
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
 * 0.01   mtl  08/27/2019  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_wdt_lib.h"
// #include "fmsh_ps_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FWdtPs_Config FWdtPs_ConfigTable[];

/************************** Function Prototypes ******************************/

/****************************************************************************/
/**
 *
 * Looks up the device configuration based on the unique device ID. The table
 * contains the configuration info for each device in the system.
 *
 * @param DeviceId contains the ID of the device
 *
 * @return
 *
 * A pointer to the configuration structure or NULL if the specified device
 * is not in the system.
 *
 * @note
 *
 * None.
 *
 ******************************************************************************/
FWdtPs_Config *FWdtPs_LookupConfig (u16 DeviceId)
{
    FWdtPs_Config *CfgPtr = NULL;

    int i;

    for (i = 0; i < FPAR_WDTPS_NUM_INSTANCES; i++)
    {
        if (FWdtPs_ConfigTable[i].DeviceId == DeviceId)
        {
            CfgPtr = &FWdtPs_ConfigTable[i];
            break;
        }
    }

    return CfgPtr;
}
