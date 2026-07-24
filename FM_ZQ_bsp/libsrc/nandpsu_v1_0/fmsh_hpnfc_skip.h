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
 * @file fmsh_hpnfc_skip.h
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This header file contains the device operating functions (or macros) that
 *  skip bad blocks.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2023/02/16  First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_HPNFC_SKIP_H_ /* prevent circular inclusions */
#define _FMSH_HPNFC_SKIP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************Include File*********************************/
#include "fmsh_hpnfc.h"

/******************************Constant Definition**************************/

/******************************Type Definition******************************/

/******************************Macro (inline function) Definition***********/

/******************************Variable Definition**************************/

/******************************Function Prototype***************************/
/****************************************************************************
 * FNandPsu_Skip_Erase - Erase Nand skip bad blocks
 *
 * This function skips bad blocks and erase enough blocks.
 * Enough good blocks (equal to address) must be kept before
 * start address to erase.
 *
 * @nfcPtr: The NAND controller
 * @addr: start address to erase (must align to blocksize)
 * @len: size of bytes to erase (must align to blocksize)
 *
 * Returns 0 on success, a negative error code otherwise.
 ***************************************************************************/
int FNandPsu_Skip_Erase(FNandPsu_T *nfcPtr, u64 addr, u32 len);

/****************************************************************************
 * FNandPsu_Skip_Write - Write Nand skip bad blocks
 *
 * This function skips bad blocks and write data to nand from memory.
 * Enough good blocks (equal to address) must be kept before
 * start address to write.
 * FNandPsu_SetOobBuf must be used if use NAND_OP_OOBREQ flag
 *
 * @nfcPtr: The NAND controller
 * @addr: nand device address
 * @len: bytes to write
 * @buf: data buffer in memory
 * @flags: can be set to the value below
 *      NAND_OP_RAW - do not use ecc
 *      NAND_OP_OOBREQ - write data from device->oob_buf
 *          with device->ooblen size to oob area in flash
 *
 * Returns 0 on success, a negative error code otherwise.
 ***************************************************************************/
int FNandPsu_Skip_Write(FNandPsu_T *nfcPtr, u64 addr, u32 len, u8 *buf,
                        u32 flags);

/****************************************************************************
 * FNandPsu_Skip_Read - Read Nand skip bad blocks
 *
 * This function skips bad blocks and read data to memory from nand.
 * Enough good blocks (equal to address) must be kept before
 * start address to write.
 * FNandPsu_SetOobBuf must be used if use NAND_OP_OOBREQ flag
 *
 * @nfcPtr: The NAND controller
 * @addr: nand device address
 * @len: bytes to read
 * @buf: data buffer in memory
 * @flags: can be set to the value below
 *      NAND_OP_RAW - do not use ecc
 *      NAND_OP_OOBREQ - read data to device->oob_buf
 *          with device->ooblen size from oob area in flash
 *
 * Returns 0 on success, a negative error code otherwise.
 *****************************************************************************/
int FNandPsu_Skip_Read(FNandPsu_T *nfcPtr, u64 addr, u32 len, u8 *buf,
                       u32 flags);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
