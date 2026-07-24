/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_wdt_g.c
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

#include "fmsh_psu_parameters.h"
#include "fmsh_wdt_lib.h"

/*
 * The configuration table for devices
 */

FWdtPs_Config FWdtPs_ConfigTable[] = {
    {FPAR_LPDWDTPS_DEVICE_ID, FPAR_LPDWDTPS_BASEADDR},
    {FPAR_FPDWDTPS_DEVICE_ID, FPAR_FPDWDTPS_BASEADDR},
    {FPAR_CSUWDTPS_DEVICE_ID, FPAR_CSUWDTPS_BASEADDR}};
