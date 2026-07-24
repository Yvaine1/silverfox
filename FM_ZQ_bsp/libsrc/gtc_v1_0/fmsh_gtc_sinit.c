/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gtc_sint.c
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

#include "fmsh_gtc_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FGtcPs_Config FGtcPs_ConfigTable[];

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
FGtcPs_Config *FGtcPs_LookupConfig (u16 DeviceId)
{
    FGtcPs_Config *CfgPtr = NULL;

    int i;

    for (i = 0; i < FPAR_GTCPS_NUM_INSTANCES; i++)
    {
        if (FGtcPs_ConfigTable[i].DeviceId == DeviceId)
        {
            CfgPtr = &FGtcPs_ConfigTable[i];
            break;
        }
    }

    return CfgPtr;
}
