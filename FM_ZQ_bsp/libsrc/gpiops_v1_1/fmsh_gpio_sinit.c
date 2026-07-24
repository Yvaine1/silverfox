/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gpio_sint.c
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
 * 0.01   yhs  08/30/2019  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_gpio_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FGpioPs_Config FGpioPs_ConfigTable[];

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
FGpioPs_Config *FGpioPs_LookupConfig (u16 DeviceId)
{
    FGpioPs_Config *CfgPtr = NULL;

    int i;

    for (i = 0; i < FPAR_GPIOPS_NUM_INSTANCES; i++)
    {
        if (FGpioPs_ConfigTable[i].DeviceId == DeviceId)
        {
            CfgPtr = &FGpioPs_ConfigTable[i];
            break;
        }
    }
    return CfgPtr;
}
