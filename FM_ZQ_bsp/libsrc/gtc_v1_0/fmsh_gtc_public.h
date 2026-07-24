/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gtc_pbulic.h
 *
 * This file contains public constant & function define of gtc
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

#ifndef _FMSH_GTC_PUBLIC_H_ /* prevent circular inclusions */
#define _FMSH_GTC_PUBLIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_gtc_common.h"
#include "fmsh_psu_parameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
    u16 DeviceId; /**< Unique ID of device */
    u32 BaseAddr; /**< Base address of the device */
} FGtcPs_Config;

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
FGtcPs_Config *FGtcPs_LookupConfig(u16 DeviceId);
u8 FGtcPs_reset(void);
u8 FGtcPs_init(FGtcPs_T *dev, FGtcPs_Config *cfg);
u8 FGtcPs_enableCounter(FGtcPs_T *dev);
u8 FGtcPs_disableCounter(FGtcPs_T *dev);
u8 FGtcPs_haltCounter(FGtcPs_T *dev, u8 val);
u8 FGtcPs_isstop(FGtcPs_T *dev);
u32 FGtcPs_getConuterH(FGtcPs_T *dev);
u32 FGtcPs_getConuterL(FGtcPs_T *dev);
u32 FGtcPs_ns_getConuterH(FGtcPs_T *dev);
u32 FGtcPs_ns_getConuterL(FGtcPs_T *dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
