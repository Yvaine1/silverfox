/******************************************************************************
*
* Copyright (C) 2015-2016 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/

/*****************************************************************************/
/**
 * @file pm_callbacks.h
 *
 * Callbacks implementation - for fmshpm internal purposes only
 *****************************************************************************/

#ifndef FMSHPM_CALLBACKS_H_
#define FMSHPM_CALLBACKS_H_

#include "fmsh_common_types.h"
#include "pm_status.h"
#include "pm_defs.h"
#include "pm_api_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

FPmStatus FPm_NotifierAdd(FPm_Notifier* const notifier);

FPmStatus FPm_NotifierRemove(FPm_Notifier* const notifier);

void FPm_NotifierProcessEvent(const enum FPmNodeId node,
			      const enum FPmNotifyEvent event,
			      const u32 oppoint);

#ifdef __cplusplus
}
#endif

#endif /* FMSHPM_CALLBACKS_H_ */
