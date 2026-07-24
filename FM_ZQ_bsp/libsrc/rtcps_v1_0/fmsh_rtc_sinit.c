/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_rtc_sint.c
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
 * 0.01   tyf  04/23/2023  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_psu_parameters.h"
#include "fmsh_rtc_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FRtcPs_Config FRtcPs_ConfigTable[];

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
FRtcPs_Config *FRtcPs_LookupConfig (u16 DeviceId)
{
    FRtcPs_Config *CfgPtr = NULL;

    int i;

    for (i = 0; i < FPAR_RTCPS_NUM_INSTANCES; i++)
    {
        if (FRtcPs_ConfigTable[i].DeviceId == DeviceId)
        {
            CfgPtr = &FRtcPs_ConfigTable[i];
            break;
        }
    }

    return CfgPtr;
}
