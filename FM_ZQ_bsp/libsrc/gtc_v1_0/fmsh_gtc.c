/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gtc.c
 *
 * This file contains all private & pbulic functions of gtc.
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

#include "fmsh_gtc_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/******************************************************************************
 *
 *
 * This function is used to init GTC
 *
 * @param    dev  is a pointer to the instance of GTC.
 * @param    addr is baseaddr of GTC.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FGtcPs_init (FGtcPs_T *dev, FGtcPs_Config *cfg)
{
    dev->base_address = (void *)(cfg->BaseAddr);

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to enable GTC
 *
 * @param    dev  is a pointer to the instance of GTC.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FGtcPs_enableCounter (FGtcPs_T *dev)
{
    FGtcPs_portmap_T *portmap;
    u32 val;

    portmap = (FGtcPs_portmap_T *)dev->base_address;

    val = GTC_IN32P(portmap->CNTCR);
    GTC_OUT32P(val | 0x1, portmap->CNTCR);

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to disenable GTC
 *
 * @param    dev  is a pointer to the instance of GTC.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FGtcPs_disableCounter (FGtcPs_T *dev)
{
    FGtcPs_portmap_T *portmap;
    u32 val;

    portmap = (FGtcPs_portmap_T *)dev->base_address;

    val = GTC_IN32P(portmap->CNTCR);
    GTC_OUT32P(val & (~0x1), portmap->CNTCR);

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to halt GTC
 *
 * @param    dev  is a pointer to the instance of GTC.
 * @param    val = 1 halt, val = 0 donnot halt
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FGtcPs_haltCounter (FGtcPs_T *dev, u8 val)
{
    FGtcPs_portmap_T *portmap;
    u32 rd;

    portmap = (FGtcPs_portmap_T *)dev->base_address;

    rd = GTC_IN32P(portmap->CNTCR);
    rd = (val == 1) ? (rd | 0x2) : (rd & (~0x2));

    GTC_OUT32P(rd, portmap->CNTCR);

    return 0;
}

/******************************************************************************
 *
 *
 * This function is used to get GTC is stop?
 *
 * @param    dev  is a pointer to the instance of GTC.
 *
 * @return   0 if no stop, 1 stop.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FGtcPs_isstop (FGtcPs_T *dev)
{
    FGtcPs_portmap_T *portmap;
    u32 ret;

    portmap = (FGtcPs_portmap_T *)dev->base_address;

    ret = GTC_IN32P(portmap->CNTSR);
    ret = (ret >> 1) & 0x1;

    return ret;
}

/******************************************************************************
 *
 *
 * This function is used to get GTC counter high
 *
 * @param    dev  is a pointer to the instance of GTC.
 *
 * @return   counter high value
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FGtcPs_getConuterH (FGtcPs_T *dev)
{
    FGtcPs_portmap_T *portmap;
    u32 ret;

    portmap = (FGtcPs_portmap_T *)dev->base_address;

    ret = GTC_IN32P(portmap->CNTCV_H);

    return ret;
}

/******************************************************************************
 *
 *
 * This function is used to get GTC counter low
 *
 * @param    dev  is a pointer to the instance of GTC.
 *
 * @return   counter low value
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FGtcPs_getConuterL (FGtcPs_T *dev)
{
    FGtcPs_portmap_T *portmap;
    u32 ret;

    portmap = (FGtcPs_portmap_T *)dev->base_address;

    ret = GTC_IN32P(portmap->CNTCV_L);

    return ret;
}

/******************************************************************************
 *
 *
 * This function is used to get GTC counter high
 *
 * @param    dev  is a pointer to the instance of GTC.
 *
 * @return   counter low value
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FGtcPs_ns_getConuterH (FGtcPs_T *dev)
{
    FGtcPs_ns_portmap_T *portmap;
    u32 ret;

    portmap = (FGtcPs_ns_portmap_T *)dev->base_address;

    ret = GTC_IN32P(portmap->CNTCV_H);

    return ret;
}

/******************************************************************************
 *
 *
 * This function is used to get GTC counter low
 *
 * @param    dev  is a pointer to the instance of GTC.
 *
 * @return   counter low value
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FGtcPs_ns_getConuterL (FGtcPs_T *dev)
{
    FGtcPs_ns_portmap_T *portmap;
    u32 ret;

    portmap = (FGtcPs_ns_portmap_T *)dev->base_address;

    ret = GTC_IN32P(portmap->CNTCV_L);

    return ret;
}

/******************************************************************************
 *
 *
 * This function is used to get GTC reset
 *
 * @param
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FGtcPs_reset (void)
{
    u32 reg;
    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg | 0x100000);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg & ~0x100000);
    return 0;
}
