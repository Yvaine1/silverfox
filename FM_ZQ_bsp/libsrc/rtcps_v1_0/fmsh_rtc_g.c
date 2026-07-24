/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_rtc_g.c
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
 * 0.01   tyf  04/24/2023  First Release
 *</pre>
 ******************************************************************************/

#include "fmsh_psu_parameters.h"
#include "fmsh_rtc_lib.h"

/*
 * The configuration table for devices
 */

FRtcPs_Config FRtcPs_ConfigTable[] = {
    {FPAR_RTCPS_DEVICE_ID, FPAR_RTCPS_BASEADDR}};
