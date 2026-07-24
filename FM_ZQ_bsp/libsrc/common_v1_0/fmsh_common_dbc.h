/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common_dbc.h
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
#ifndef _FMSH_COMMON_DBC_H_
#define _FMSH_COMMON_DBC_H_

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/
#include "bspconfig.h"

/************************** Constant Definitions *****************************/
#define LOG_OUT           1

#define LOG_LEVEL_DEBUG   0
#define LOG_LEVEL_INFO    1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR   3
#define LOG_LEVEL_FATAL   4

/* FMSH_NASSERT macro disables all contract validations
 * (assertions, preconditions, postconditions, and invariants).
 */
#ifndef FMSH_NASSERT

// callback invoked in case of assertion failure
extern void onAssert__(const char *file, unsigned line);

#define FMSH_ASSERT(test_) \
    if (test_)             \
    {                      \
    }                      \
    else onAssert__(__FILE__, __LINE__)

#define FMSH_REQUIRE(test_)   FMSH_ASSERT(test_)
#define FMSH_ENSURE(test_)    FMSH_ASSERT(test_)
#define FMSH_INVARIANT(test_) FMSH_ASSERT(test_)
#define FMSH_ALLEGE(test_)    FMSH_ASSERT(test_)

#else  // FMSH_NASSERT

#define FMSH_ASSERT(test_)
#define FMSH_REQUIRE(test_)
#define FMSH_ENSURE(test_)
#define FMSH_INVARIANT(test_)
#define FMSH_ALLEGE(test_) \
    if (test_)             \
    {                      \
    }                      \
    else

#endif  // FMSH_NASSERT

#define PRINTF(fmt, ...)            \
    do                              \
    {                               \
        printf(fmt, ##__VA_ARGS__); \
    } while (0)

#define TRACE_OUT(flag, fmt, ...)       \
    do                                  \
    {                                   \
        if (flag)                       \
        {                               \
            printf(fmt, ##__VA_ARGS__); \
        }                               \
    } while (0)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FMSH_COMMON_DBC_H_ */
