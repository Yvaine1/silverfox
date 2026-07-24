/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_can_example.h
*
* This file contains can lib
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   LQ  12/20/2018  First Release
*</pre> 
******************************************************************************/
#ifndef _FMSH_CAN_EXAMPLE_H_
#define _FMSH_CAN_EXAMPLE_H_

/************************** Constant Definitions *****************************/
// debug print
#define CAN_DEBUG_OUT                      1
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define CAN_TRACE_OUT(flag, ...) \
    do                            \
    {                             \
        if (flag)                 \
        {                         \
            printf(__VA_ARGS__);  \
        }                         \
    } while (0)
/************************** Function Prototypes ******************************/
u8 FCanPs_example(void);
u8 FCan0Ps_example(void);
#endif 
