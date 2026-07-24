/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_watchdog.c
 *
 * This file contains "boot_main.h".
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/28/2022  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"
#include "fmsh_wdt_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FWdtPs_T g_WDT;
/************************** Function Prototypes ******************************/
/******************************************************************************
 *
 * This function is used to initialize g_WDT.
 *
 * @param	 None.
 *
 * @return	 None.
 *
 ******************************************************************************/
void FmshFsbl_WdtInit (void) 
{ 
    FWdtPs_Config *cfg = NULL;
#ifdef CORTEX_A53
    cfg = FWdtPs_LookupConfig(FPAR_FPDWDTPS_DEVICE_ID);
#else
    cfg = FWdtPs_LookupConfig(FPAR_LPDWDTPS_DEVICE_ID);
#endif   
    FWdtPs_init(&g_WDT, cfg);
    cfg = NULL;
    (void)FWdtPs_setRMOD(&g_WDT, FMSH_clear);
    (void)FWdtPs_setRPL(&g_WDT, _128_pclk);
    (void)FWdtPs_setTOP_INIT(&g_WDT, FSBL_WDT_TOP);
    (void)FWdtPs_setTOP(&g_WDT,FSBL_WDT_TOP);
    (void)FWdtPs_setWDT_EN(&g_WDT, FMSH_set);
    (void)FWdtPs_restart(&g_WDT);
    return; 
}

/******************************************************************************
 *
 * This function is used to close g_WDT.
 *
 * @param	 None.
 *
 * @return	 None.
 *
 ******************************************************************************/
void FmshFsbl_WdtClose (void) 
{ 
    u32 reg=0;
#ifdef CORTEX_A53
    reg = FMSH_ReadReg(FPS_CRF_APB_BASEADDR, 0x100);
    FMSH_WriteReg(FPS_CRF_APB_BASEADDR, 0x100, reg | 0x20000);
#else
    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x23C);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x23C, reg | 0x100000);
#endif    
    return; 
}
