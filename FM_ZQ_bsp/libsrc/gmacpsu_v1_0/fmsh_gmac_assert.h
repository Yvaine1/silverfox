/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_assert.h
 *
 * gmac driver
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 1_0   Danyang Wang  6/25/2023  First Release
 *</pre>
 ******************************************************************************/

#ifndef FMSH_GMAC_ASSERT_H /* prevent circular inclusions */
#define FMSH_GMAC_ASSERT_H /* by using protection macros */

#include "fmsh_common.h"
#include "fmsh_gmac_status.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

#define FGMACPS_ASSERT_NONE     0U
#define FGMACPS_ASSERT_OCCURRED 1U
#define XNULL                   NULL

extern u32 FGmacPs_AssertStatus;
extern s32 FGmacPs_AssertWait;
extern void FGmacPs_Assert(const char8 *File, s32 Line);
//void XNullHandler(void *NullParameter);

/**
 * This data type defines a callback to be invoked when an
 * assert occurs. The callback is invoked only when asserts are enabled
 */
typedef void (*FGmacPs_AssertCallback)(const char8 *File, s32 Line);

/***************** Macros (Inline Functions) Definitions *********************/

#ifndef NDEBUG

/*****************************************************************************/
/**
 * @brief    This assert macro is to be used for void functions. This in
 *           conjunction with the FGmacPs_AssertWait boolean can be used to
 *           accomodate tests so that asserts which fail allow execution to
 *           continue.
 *
 * @param    Expression: expression to be evaluated. If it evaluates to
 *           false, the assert occurs.
 *
 * @return   Returns void unless the FGmacPs_AssertWait variable is true, in
 *which case no return is made and an infinite loop is entered.
 *
 ******************************************************************************/
#define FGmacPs_AssertVoid(Expression)                      \
    {                                                       \
        if (Expression)                                     \
        {                                                   \
            FGmacPs_AssertStatus = FGMACPS_ASSERT_NONE;     \
        }                                                   \
        else                                                \
        {                                                   \
            FGmacPs_Assert(__FILE__, __LINE__);             \
            FGmacPs_AssertStatus = FGMACPS_ASSERT_OCCURRED; \
            return;                                         \
        }                                                   \
    }

/*****************************************************************************/
/**
 * @brief    This assert macro is to be used for functions that do return a
 *           value. This in conjunction with the FGmacPs_AssertWait boolean can
 *be used to accomodate tests so that asserts which fail allow execution to
 *continue.
 *
 * @param    Expression: expression to be evaluated. If it evaluates to false,
 *           the assert occurs.
 *
 * @return   Returns 0 unless the FGmacPs_AssertWait variable is true, in which
 * 	        case no return is made and an infinite loop is entered.
 *
 ******************************************************************************/
#define FGmacPs_AssertNonvoid(Expression)                   \
    {                                                       \
        if (Expression)                                     \
        {                                                   \
            FGmacPs_AssertStatus = FGMACPS_ASSERT_NONE;     \
        }                                                   \
        else                                                \
        {                                                   \
            FGmacPs_Assert(__FILE__, __LINE__);             \
            FGmacPs_AssertStatus = FGMACPS_ASSERT_OCCURRED; \
            return 0;                                       \
        }                                                   \
    }

/*****************************************************************************/
/**
 * @brief     Always assert. This assert macro is to be used for void functions.
 *            Use for instances where an assert should always occur.
 *
 * @return    Returns void unless the FGmacPs_AssertWait variable is true, in
 *which case no return is made and an infinite loop is entered.
 *
 ******************************************************************************/
#define FGmacPs_AssertVoidAlways()                      \
    {                                                   \
        FGmacPs_Assert(__FILE__, __LINE__);             \
        FGmacPs_AssertStatus = FGMACPS_ASSERT_OCCURRED; \
        return;                                         \
    }

/*****************************************************************************/
/**
 * @brief   Always assert. This assert macro is to be used for functions that
 *          do return a value. Use for instances where an assert should always
 *          occur.
 *
 * @return Returns void unless the FGmacPs_AssertWait variable is true, in which
 *	      case no return is made and an infinite loop is entered.
 *
 ******************************************************************************/
#define FGmacPs_AssertNonvoidAlways()                   \
    {                                                   \
        FGmacPs_Assert(__FILE__, __LINE__);             \
        FGmacPs_AssertStatus = FGMACPS_ASSERT_OCCURRED; \
        return 0;                                       \
    }

#else

#define FGmacPs_AssertVoid(Expression)
#define FGmacPs_AssertVoidAlways()
#define FGmacPs_AssertNonvoid(Expression)
#define FGmacPs_AssertNonvoidAlways()

#endif

/************************** Function Prototypes ******************************/

void FGmacPs_AssertSetCallback(FGmacPs_AssertCallback Routine);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
