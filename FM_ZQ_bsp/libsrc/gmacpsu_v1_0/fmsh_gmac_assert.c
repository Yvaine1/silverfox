/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_bd.c
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

/***************************** Include Files *********************************/

#include "fmsh_gmac_assert.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**
 * This variable allows testing to be done easier with asserts. An assert
 * sets this variable such that a driver can evaluate this variable
 * to determine if an assert occurred.
 */
u32 FGmacPs_AssertStatus;

/**
 * This variable allows the assert functionality to be changed for testing
 * such that it does not wait infinitely. Use the debugger to disable the
 * waiting during testing of asserts.
 */
s32 FGmacPs_AssertWait = 1;

/* The callback function to be invoked when an assert is taken */
static FGmacPs_AssertCallback FGmacPs_AssertCallbackRoutine = NULL;

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 *
 * @brief    Implement assert. Currently, it calls a user-defined callback
 *           function if one has been set.  Then, it potentially enters an
 *           infinite loop depending on the value of the FGmacPs_AssertWait
 *           variable.
 *
 * @param    file: filename of the source
 * @param    line: linenumber within File
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
void FGmacPs_Assert (const char8 *File, s32 Line)
{
    /* if the callback has been set then invoke it */
    if (FGmacPs_AssertCallbackRoutine != 0)
    {
        (*FGmacPs_AssertCallbackRoutine)(File, Line);
    }

    /* if specified, wait indefinitely such that the assert will show up
     * in testing
     */
    while (FGmacPs_AssertWait != 0)
    {
    }
}

/*****************************************************************************/
/**
 *
 * @brief    Set up a callback function to be invoked when an assert occurs.
 *           If a callback is already installed, then it will be replaced.
 *
 * @param    routine: callback to be invoked when an assert is taken
 *
 * @return   None.
 *
 * @note     This function has no effect if NDEBUG is set
 *
 ******************************************************************************/
void FGmacPs_AssertSetCallback (FGmacPs_AssertCallback Routine)
{
    FGmacPs_AssertCallbackRoutine = Routine;
}

/*****************************************************************************/
/**
 *
 * @brief    Null handler function. This follows the XInterruptHandler
 *           signature for interrupt handlers. It can be used to assign a null
 *           handler (a stub) to an interrupt controller vector table.
 *
 * @param    NullParameter: arbitrary void pointer and not used.
 *
 * @return   None.
 *
 * @note     None.
 *
 ******************************************************************************/
//void XNullHandler (void *NullParameter) { (void)NullParameter; }
