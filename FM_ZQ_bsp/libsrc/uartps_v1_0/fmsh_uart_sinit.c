/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_uart_sint.c
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

#include "fmsh_psu_parameters.h"
#include "fmsh_uart_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern FUartPs_Config FUartPs_ConfigTable[];

/************************** Function Prototypes ******************************/

/****************************************************************************
 *
 *  This function look up the uart by device id.
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
FUartPs_Config *FUartPs_LookupConfig (u16 DeviceId)
{
    FUartPs_Config *CfgPtr = NULL;

    u32 Index = 0U;

    for (Index = 0; Index < FPAR_UARTPS_NUM_INSTANCES; Index++)
    {
        if (FUartPs_ConfigTable[Index].DeviceId == DeviceId)
        {
            CfgPtr = &FUartPs_ConfigTable[Index];
            break;
        }
    }

    return (FUartPs_Config *)CfgPtr;
}
