/******************************************************************************
*
* Copyright (C) 2015-2018 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/

/*****************************************************************************/
/**
 * @file pm_clock.h
 *
 * PM Definitions of clocks - for fmshpm internal purposes only
 *****************************************************************************/

#ifndef PM_CLOCKS_H_
#define PM_CLOCKS_H_

#include "fmsh_common_types.h"
#include "pm_status.h"
#include "pm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

FPmStatus FPm_GetClockParentBySelect(const enum FPmClock clockId,
				   const u32 select,
				   enum FPmClock* const parentId);

FPmStatus FPm_GetSelectByClockParent(const enum FPmClock clockId,
				   const enum FPmClock parentId,
				   u32* const select);

u8 FPm_GetClockDivType(const enum FPmClock clock);

u8 FPm_MapDivider(const enum FPmClock clock,
		  const u32 div,
		  u32* const div0,
		  u32* const div1);

#ifdef __cplusplus
}
#endif

#endif /* PM_CLOCKS_H_ */
