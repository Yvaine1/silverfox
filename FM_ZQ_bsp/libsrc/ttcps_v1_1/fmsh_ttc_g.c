/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_ttc_g.c
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

#include "fmsh_gic_hw.h"
#include "fmsh_psu_parameters.h"
#include "fmsh_ttc_lib.h"

/*
 * The configuration table for devices
 */

FTtcPs_Config FTtcPs_ConfigTable[] = {
    {FPAR_TTCPS_0_DEVICE_ID,
     FPAR_TTCPS_0_BASEADDR,
     {TTC0_1_INT_ID, TTC0_2_INT_ID, TTC0_3_INT_ID}},
    {FPAR_TTCPS_1_DEVICE_ID,
     FPAR_TTCPS_1_BASEADDR,
     {TTC1_1_INT_ID, TTC1_2_INT_ID, TTC1_3_INT_ID}},
    {FPAR_TTCPS_2_DEVICE_ID,
     FPAR_TTCPS_2_BASEADDR,
     {TTC2_1_INT_ID, TTC2_2_INT_ID, TTC2_3_INT_ID}},
    {FPAR_TTCPS_3_DEVICE_ID,
     FPAR_TTCPS_3_BASEADDR,
     {TTC3_1_INT_ID, TTC3_2_INT_ID, TTC3_3_INT_ID}}};
