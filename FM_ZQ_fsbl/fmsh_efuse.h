/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_efuse.h
 *
 * This file contains header fmsh_common.h & fmsh_types.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   xxx  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_EFUSE_H_
#define _FMSH_EFUSE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* Register PPK0_0 */
#define EFUSE_PPK0_START             (FPS_CSU_BASEADDR + 0x1000U)

/* Register PPK1_0 */
#define EFUSE_PPK1_START             (FPS_CSU_BASEADDR + 0x1028U)

/* Register SPK ID */
#define EFUSE_SPKID                  (FPS_CSU_BASEADDR + 0x258U)

#define EFUSE_XADC_ALRM_INTERRUPT_EN (FPS_CSU_BASEADDR + 0x338U)

#define APU_ROM_DIS_ROW_NUMBER       74

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

u32 FmshFsbl_EfuseReadData(u32 bRowAddr);
u32 FmshFsbl_FindOneInNumber(u32 Data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
