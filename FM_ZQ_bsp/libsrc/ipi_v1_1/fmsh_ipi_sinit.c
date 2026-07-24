
/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/

/**
 *
 * @file ipi_sinit.c
 * @{
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_ipi.h"

/************************** Variable Definitions *****************************/
extern IpiPsu_Config IpiPsu_ConfigTable[];

/*****************************************************************************/

/**
 *
 * Looks up the device configuration based on the unique device ID. A table
 * contains the configuration info for each device in the system.
 *
 * @param	DeviceId contains the ID of the device to look up the
 *			configuration for.
 *
 * @return	A pointer to the configuration found or NULL if the specified
 *			device ID was not found. See ipi.h for the definition of
 *			IpiPsu_Config.
 *
 * @note		None.
 *
 ******************************************************************************/
IpiPsu_Config *IpiPsu_LookupConfig (u32 DeviceId)
{
    IpiPsu_Config *CfgPtr = NULL;
    u32 Index;

    for (Index = 0U; Index < PAR_IPIPSU_NUM_INSTANCES; Index++)
    {
        if (IpiPsu_ConfigTable[Index].DeviceId == DeviceId)
        {
            CfgPtr = &IpiPsu_ConfigTable[Index];
            break;
        }
    }

    return (IpiPsu_Config *)CfgPtr;
}
/** @} */
