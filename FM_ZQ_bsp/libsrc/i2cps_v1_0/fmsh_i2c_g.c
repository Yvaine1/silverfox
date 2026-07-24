/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_i2c_g.c
 *
 * This file contains global variable which use to config.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   wfb  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_i2c_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FI2cPs_Config FI2cPs_ConfigTable[] = {
    {FPAR_I2CPS_0_DEVICE_ID, FPAR_I2CPS_0_BASEADDR,
     FPAR_I2CPS_0_I2C_CLK_FREQ_HZ},
    {FPAR_I2CPS_1_DEVICE_ID, FPAR_I2CPS_1_BASEADDR,
     FPAR_I2CPS_1_I2C_CLK_FREQ_HZ}};
/************************** Function Prototypes ******************************/
