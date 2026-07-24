/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gpio_g.c
 *
 *
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

#include "fmsh_gpio_lib.h"
#include "fmsh_psu_parameters.h"

/*
 * The configuration table for devices
 */

FGpioPs_Config FGpioPs_ConfigTable[] = {
    {FPAR_GPIOPS_0_DEVICE_ID, FPAR_GPIOPS_0_BASEADDR, FPAR_GPIOPS_0_MAXPIN},
    {FPAR_GPIOPS_1_DEVICE_ID, FPAR_GPIOPS_1_BASEADDR, FPAR_GPIOPS_1_MAXPIN},
    {FPAR_GPIOPS_2_DEVICE_ID, FPAR_GPIOPS_2_BASEADDR, FPAR_GPIOPS_2_MAXPIN

    },
    {FPAR_GPIOPS_3_DEVICE_ID, FPAR_GPIOPS_3_BASEADDR, FPAR_GPIOPS_3_MAXPIN},
    {FPAR_GPIOPS_4_DEVICE_ID, FPAR_GPIOPS_4_BASEADDR, FPAR_GPIOPS_4_MAXPIN},
    {FPAR_GPIOPS_5_DEVICE_ID, FPAR_GPIOPS_5_BASEADDR, FPAR_GPIOPS_5_MAXPIN}};
