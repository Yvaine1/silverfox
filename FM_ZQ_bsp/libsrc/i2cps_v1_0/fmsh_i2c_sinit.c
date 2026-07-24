/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_i2c_sint.c
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

#include "fmsh_i2c_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FI2cPs_Config FI2cPs_ConfigTable[];

/************************** Function Prototypes ******************************/

/****************************************************************************
 *
 *  This function look up the i2c by device id.
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
FI2cPs_Config *FI2cPs_LookupConfig (u16 DeviceId)
{
    FI2cPs_Config *CfgPtr = NULL;

    u32 Index = 0U;

    for (Index = 0; Index < FPAR_I2CPS_NUM_INSTANCES; Index++)
    {
        if (FI2cPs_ConfigTable[Index].DeviceId == DeviceId)
        {
            CfgPtr = &FI2cPs_ConfigTable[Index];
            break;
        }
    }

    return (FI2cPs_Config *)CfgPtr;
}
