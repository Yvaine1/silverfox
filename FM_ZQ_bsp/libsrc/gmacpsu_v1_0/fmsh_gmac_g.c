/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_lib.c
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

#include "fmsh_gmac.h"
#include "fmsh_psu_parameters.h"

/*
 * The configuration table for devices
 */

FGmacPs_Config FGmacPs_ConfigTable[FPAR_GMACPS_NUM_INSTANCES] = {
    {FPAR_GMACPS_0_DEVICE_ID, FPAR_GMACPS_0_BASEADDR, FPAR_GMACPS_0_SPEED,
     FPAR_GMACPS_0_INTERFACE, FPAR_GMACPS_0_CACHE_COHERENT},
    {FPAR_GMACPS_1_DEVICE_ID, FPAR_GMACPS_1_BASEADDR, FPAR_GMACPS_1_SPEED,
     FPAR_GMACPS_1_INTERFACE, FPAR_GMACPS_1_CACHE_COHERENT},
    {FPAR_GMACPS_2_DEVICE_ID, FPAR_GMACPS_2_BASEADDR, FPAR_GMACPS_2_SPEED,
     FPAR_GMACPS_2_INTERFACE, FPAR_GMACPS_2_CACHE_COHERENT},
    {FPAR_GMACPS_3_DEVICE_ID, FPAR_GMACPS_3_BASEADDR, FPAR_GMACPS_3_SPEED,
     FPAR_GMACPS_3_INTERFACE, FPAR_GMACPS_3_CACHE_COHERENT}};
