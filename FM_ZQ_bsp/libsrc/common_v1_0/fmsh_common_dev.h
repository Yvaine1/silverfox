/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common_dev.h
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
 * 0.01   yl  12/20/2018  First Release
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_COMMON_DEV_H_
#define _FMSH_COMMON_DEV_H_

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*****************************************************************************
 * DESCRIPTION
 *  This is a generic data type used for 1-bit wide bitfields which have
 *  a "set/clear" property.  This is used when modifying registers
 *  within a peripheral's memory map.
 *
 *****************************************************************************/
enum FMSH_state { FMSH_err = -1, FMSH_clear = 0, FMSH_set = 1 };

/*****************************************************************************
 * DESCRIPTION
 *  This is a generic data type used for handling callback functions
 *  with each driver.
 *
 * @param
 *           pDev pointer to device handle.
 *           eCode event/error code.
 *
 * @note
 *  The usage of the eCode argument is typically negative for an error
 *  code and positive for an event code
 *
 *****************************************************************************/
typedef void (*FMSH_callback)(void *pDev, int32_t eCode);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FMSH_COMMON_DEV_H_ */
