/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common_status.h
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
 *
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_COMMON_STATUS_H_
#define _FMSH_COMMON_STATUS_H_

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

#define FMSH_SUCCESS 0UL
#define FMSH_FAILURE 1UL

#define FMSH_EPERM   1   // operation not permitted
#define FMSH_EIO     5   // I/O error
#define FMSH_ENXIO   6   // no such device or address
#define FMSH_ENOMEM  12  // out of memory
#define FMSH_EACCES  13  // permission denied
#define FMSH_EBUSY   16  // device or resource busy
#define FMSH_ENODEV  19  // no such device
#define FMSH_EINVAL  22  // invalid argument
#define FMSH_ENOSPC  28  // no space left on device
#define FMSH_ENOSYS  38  // function not implemented/supported
#define FMSH_ECHRNG  44  // channel number out of range
#define FMSH_ENODATA 61  // no data available
#define FMSH_ETIME   62  // timer expired
#define FMSH_EPROTO  71  // protocol error

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#endif /* #ifndef _FMSH_COMMON_STATUS_H_ */
