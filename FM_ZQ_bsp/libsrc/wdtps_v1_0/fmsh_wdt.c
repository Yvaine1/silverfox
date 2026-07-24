/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_wdt.c
 *
 * This file contains all private & pbulic functions of wdt.
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

#include "fmsh_wdt_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/******************************************************************************
 *
 *
 * This function initializes watchdog timer.
 *
 * @param    wdt_wdtDev is WDT(watchDog timer) handle.
 *
 * @return   TRUE if successful, otherwise do not support.
 *
 * @note     None.
 *
 ******************************************************************************/

BOOL FWdtPs_init (FWdtPs_T *dev, FWdtPs_Config *cfg)
{
    dev->base_address = (void *)(cfg->BaseAddr);
    dev->comp_version = FWdtPs_compVersion(dev);
    dev->comp_type = FWdtPs_compType(dev);

    return TRUE;
}

/******************************************************************************
 *
 *
 * This function set WDT enable
 *
 * @param    dev is a pointer to WDT device
 * @param    state: FMSH_clear/FMSH_err/FMSH_set.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FWdtPs_setWDT_EN (FWdtPs_T *dev, enum FMSH_state state)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    reg = WDT_IN32P(portmap->cr);
    if (FMSH_BIT_GET(reg, WDT_EN) != state)
    {
        FMSH_BIT_SET(reg, WDT_EN, state);
        // for test
        // FMSH_BIT_CLEAR(reg, RMOD);
        WDT_OUT32P(reg, portmap->cr);
    }

    return 0;
}

/******************************************************************************
 *
 *
 * This function set WDT Response mode
 *
 * @param    dev is a pointer to WDT device
 * @param    state: FMSH_clear/FMSH_err/FMSH_set.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FWdtPs_setRMOD (FWdtPs_T *dev, enum FMSH_state state)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    reg = WDT_IN32P(portmap->cr);
    if (FMSH_BIT_GET(reg, RMOD) != state)
    {
        FMSH_BIT_SET(reg, RMOD, state);
        WDT_OUT32P(reg, portmap->cr);
    }

    return 0;
}

/******************************************************************************
 *
 *
 * This function set WDT Reset Pulse Lenth
 *
 * @param    dev is a pointer to WDT device
 * @param    value is enum prl_value value _2_pclk/....../_256_pclk
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FWdtPs_setRPL (FWdtPs_T *dev, enum prl_value value)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;
    reg = WDT_IN32P(portmap->cr);

    if (FMSH_BIT_GET(reg, RPL) != value)
    {
        FMSH_BIT_SET(reg, RPL, value);
        WDT_OUT32P(reg, portmap->cr);
    }

    return 0;
}

/******************************************************************************
 *
 *
 * This function set TOP value, as WDT restart counter value
 *
 * @param    dev is a pointer to WDT device
 * @param    value is TOP value
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FWdtPs_setTOP (FWdtPs_T *dev, u32 value)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    if (value > 15)
    {
        return 1;
    }

    reg = WDT_IN32P(portmap->torr);
    if (FMSH_BIT_GET(reg, TOP) != value)
    {
        FMSH_BIT_SET(reg, TOP, value);
        WDT_OUT32P(reg, portmap->torr);
    }

    return 0;
}

/******************************************************************************
 *
 *
 * This function set TOP_INIT value, as WDT fisrt counter value
 *
 * @param    dev is a pointer to WDT device
 * @param    value is TOP_INIT value
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FWdtPs_setTOP_INIT (FWdtPs_T *dev, u32 value)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    if (value > 15)
    {
        return 1;
    }

    reg = WDT_IN32P(portmap->torr);
    if (FMSH_BIT_GET(reg, TOP_INIT) != value)
    {
        FMSH_BIT_SET(reg, TOP_INIT, value);
        WDT_OUT32P(reg, portmap->torr);
    }

    return 0;
}

/******************************************************************************
 *
 *
 * This function get current counter value.
 *
 * @param    dev is a pointer to WDT device.
 *
 * @return   current counter value.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FWdtPs_getCurrentCounterValue (FWdtPs_T *dev)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    reg = WDT_IN32P(portmap->ccvr);

    return reg;
}

/******************************************************************************
 *
 *
 * This function restart WDT.
 *
 * @param    dev is a pointer to WDT device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FWdtPs_restart (FWdtPs_T *dev)
{
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;
    WDT_OUT32P(0x76, portmap->ccr);

    return 0;
}

/******************************************************************************
 *
 *
 * This function get interrupt status.
 *
 * @param    dev is a pointer to WDT device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FWdtPs_getInterruptStatus (FWdtPs_T *dev)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    reg = WDT_IN32P(portmap->stat);

    return reg;
}

/******************************************************************************
 *
 *
 * This function clear interrupt status, read eio clear interrupt.
 *
 * @param    dev is a pointer to WDT device.
 *
 * @return   return is unsed.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FWdtPs_clearInterruptStatus (FWdtPs_T *dev)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    FWDTPS_COMMON_ASSERT(dev);

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    reg = WDT_IN32P(portmap->eio);

    return reg;
}

/******************************************************************************
 *
 *
 * This function get componet Version ID.
 *
 * @param    dev is a pointer to WDT device.
 *
 * @return   componet Version ID.
 *
 * @note     None.
 *
 ******************************************************************************/
static u32 FWdtPs_compVersion (FWdtPs_T *dev)
{
    u32 reg = 0;
    FWdtPs_Portmap_T *portmap;

    portmap = (FWdtPs_Portmap_T *)dev->base_address;

    reg = WDT_IN32P(portmap->comp_version);

    return reg;
}

/******************************************************************************
 *
 *
 * This function get componet Type
 *
 * @param    dev is a pointer to WDT device.
 *
 * @return   componet Type.
 *
 * @note     None.
 *
 ******************************************************************************/
static u32 FWdtPs_compType (FWdtPs_T *dev)
{
    u32 reg = 0;

    reg = FMSH_ReadReg((uintptr_t)(dev->base_address), 0xfc);

    return reg;
}
/******************************************************************************
 *
 *
 * This function get wdt reset
 *
 * @param    dev is a pointer to WDT device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FWdtPs_reset (FWdtPs_T *dev)
{
    u32 reg = 0;

    if (dev->base_address == (void *)FPAR_LPDWDTPS_BASEADDR)
    {
        reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x23C);
        FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x23C, reg | 0x100000);
        FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x23C, reg & ~0x100000);
    }
    else if (dev->base_address == (void *)FPAR_FPDWDTPS_BASEADDR)
    {
        reg = FMSH_ReadReg(FPS_CRF_APB_BASEADDR, 0x100);
        FMSH_WriteReg(FPS_CRF_APB_BASEADDR, 0x100, reg | 0x20000);
        FMSH_WriteReg(FPS_CRF_APB_BASEADDR, 0x100, reg & ~0x20000);
    }
    else if (dev->base_address == (void *)FPAR_CSUWDTPS_BASEADDR)
    {
        reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
        FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg | 0x8000);
        FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg & ~0x8000);
    }
    else{
        ;/* no deal with */
    }

    return 0;
}
