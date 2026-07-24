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
 * @file fmsh_hpnfc_bbm.h
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This header file contains the bad block management functions (or macros).
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
#ifndef _FMSH_HPNFC_BBM_H_ /* prevent circular inclusions */
#define _FMSH_HPNFC_BBM_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************Include File*********************************/
#include "fmsh_hpnfc.h"

/******************************Constant Definition**************************/
#define NAND_BBT_SCAN_2ND_PAGE \
    (1) /**< Scan the second page for bad block information */
#define NAND_BBT_SIG_OFFSET (8)  /**< Bad Block Table signature offset */
#define NAND_BBT_VER_OFFSET (12) /**< Bad block Table version offset */
#define NAND_BBT_SIG_LEN    (4)  /**< Bad block Table signature length */
#define NAND_BBT_MAX_BLOCKS (4)  /**< Bad block Table max blocks */

#define NAND_BB_PATTERN_OFFSET_SMALL_PAGE \
    (5)                          /**< Bad block pattern offset in a page */
#define NAND_BB_PATTERN_LENGTH_SMALL_PAGE (1) /**< Bad block pattern length */
#define NAND_BB_PATTERN_OFFSET_LARGE_PAGE \
    (0) /**< Bad block pattern offset in a large page */
#define NAND_BB_PATTERN_LENGTH_LARGE_PAGE (2) /**< Bad block pattern length */
#define NAND_BB_PATTERN                   (0xFF) /**< Bad block pattern to search in a page */
/*
 * Block definitions for RAM based Bad Block Table (BBT)
 */
#define NAND_BLOCK_GOOD                   (0x3) /**< Block is good */
#define NAND_BLOCK_BAD                    (0x2) /**< Block is bad */
#define NAND_BLOCK_RESERVED               (0x1)
#define NAND_BLOCK_FACTORY_BAD            (0x0)

#define NAND_BBT_BLOCK_SHIFT              (2) /**< 1 byte represent 4 block in BBT */
#define NAND_BLOCK_TYPE_MASK              (0x03) /**< Block type mask */
#define NAND_BLOCK_SHIFT_MASK \
    (0x06) /**< Block shift mask for a Bad Block Table entry byte */

/******************************Type Definition******************************/

/******************************Macro (inline function) Definition***********/

/******************************Variable Definition**************************/

/******************************Function Prototype***************************/
/**
 * FNandPsu_InitBBT - Initialize BBT Descriptors
 *
 * This function initializes the Bad Block Table(BBT) descriptors with a
 * predefined pattern for searching Bad Block Table(BBT) in flash.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_InitBBT(FNandPsu_T* nfcPtr);

/**
 * FNandPsu_ScanBBT - Scan BBT in device
 *
 * This function reads the Bad Block Table(BBT) if present in flash. If not it
 * scans the flash for detecting factory marked bad blocks and creates a bad
 * block table and write the Bad Block Table(BBT) into the flash.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_ScanBBT(FNandPsu_T* nfcPtr);

/**
 * FNandPsu_CheckBlock - Check bad block marker in device
 *
 * This function reads the Bad Block marker in flash.
 *
 * @nfcPtr: The NAND controller
 * @block: block num in device to check
 *
 * Returns 0 if block is good, bad/reserved block otherwise.
 */
int FNandPsu_CheckBlock(FNandPsu_T* nfcPtr, u32 block);

/**
 * FNandPsu_MarkBlockBad - Set bad block marker in device
 *
 * This function set the Bad Block marker in flash.
 *
 * @nfcPtr: The NAND controller
 * @block: block num in device to check
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_MarkBlockBad(FNandPsu_T* nfcPtr, u32 block);

/**
 * FNandPsu_IsBlockBad - Check bad block marker in ram based BBT
 *
 * This function reads the BBT in ram and check if block is bad.
 *
 * @nfcPtr: The NAND controller
 * @block: block num in device to check
 *
 * Returns 0 if block is good, bad/reserved block otherwise.
 */
int FNandPsu_IsBlockBad(FNandPsu_T* nfcPtr, u32 block);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
