/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_devc_g.c
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
 * 0.01   lq  07/01/2022  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_devc_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

FDevcPs_Config FDevcPs_ConfigTable[] = {{FPAR_DEVCPS_DEVICE_ID,
                                         FPAR_DEVCPS_BASEADDR,
                                         FPAR_DEVCPS_DEVC_CLK_FREQ_HZ}};

/************************** Function Prototypes ******************************/
