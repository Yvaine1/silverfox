/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_GPIO_example.h
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
 * 0.01   zyh  08/29/2024  First Release
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_GPIO_EXAMPLE_H_
#define _FMSH_GPIO_EXAMPLE_H_

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u8 FGpioPs_input_example(void);
u8 FGpioPs_output_example(void);
u8 FGpioPs_example(u8 deviceId);
#endif /* #ifndef _FMSH_GPIO_EXAMPLE_H_ */
