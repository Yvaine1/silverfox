/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fmsh_spi_flash.h
 * @addtogroup spips_v2_0
 * @{
 *
 * This header file contains the identifiers and basic spi flash driver
 * functions (or macros) that can be used to access the device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.10  hzq 11/26/20
 * 		     First release
 * 2.00 hzq 2023/03/23
 *
 * 2.01 hxq 2024/12/19
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef _FMSH_SPI_FLASH_H_ /* prevent circular inclusions */
#define _FMSH_SPI_FLASH_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************Include
 * File*********************************/

/**********************************Constant
 * Definition**************************/

/**********************************Type
 * Definition******************************/

/**********************************Macro (inline function)
 * Definition***********/

/**********************************Variable
 * Definition**************************/

/**********************************Function
 * Prototype***************************/
int FSpiPs_Nor_RDID(FSpiPs_T* spiPtr, u8* id);
int FSpiPs_Nor_WREN(FSpiPs_T* spiPtr);
int FSpiPs_Nor_WRDI(FSpiPs_T* spiPtr);
u8 FSpiPs_Nor_RDSR(FSpiPs_T* spiPtr);
int FSpiPs_Nor_CE (FSpiPs_T* spiPtr);
int FSpiPs_Nor_PP(FSpiPs_T* spiPtr, u32 address, u8* data, int len);
int FSpiPs_Nor_READ(FSpiPs_T* spiPtr, u32 address, u8* data, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
