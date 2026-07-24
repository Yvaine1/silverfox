/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_bd.c
 *
 * gmac driver
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 1_0   Danyang Wang  6/25/2023  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_gmac.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*************************** Variable Definitions *****************************/
extern FGmacPs_Config FGmacPs_ConfigTable[FPAR_GMACPS_NUM_INSTANCES];

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * Lookup the device configuration based on the unique device ID.  The table
 * contains the configuration info for each device in the system.
 *
 * @param DeviceId is the unique device ID of the device being looked up.
 *
 * @return
 * A pointer to the configuration table entry corresponding to the given
 * device ID, or NULL if no match is found.
 *
 ******************************************************************************/
FGmacPs_Config *FGmacPs_LookupConfig (u16 DeviceId)
{
    FGmacPs_Config *CfgPtr = NULL;
    u32 i;

    for (i = 0U; i < (u32)FPAR_GMACPS_NUM_INSTANCES; i++)
    {
        if (FGmacPs_ConfigTable[i].DeviceId == DeviceId)
        {
            CfgPtr = &FGmacPs_ConfigTable[i];
            break;
        }
    }

    return (FGmacPs_Config *)(CfgPtr);
}
