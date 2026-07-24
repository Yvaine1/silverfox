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
 *  This source file contains the device operating functions that
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
#include "fmsh_common.h"
#include "fmsh_hpnfc.h"
#include "fmsh_hpnfc_bbm.h"
#include "fmsh_hpnfc_flash.h"

/****************************************************************************
 * nand_calculate_skip_length - Calculate actual length after skip bad blocks
 *
 * This function calculates actual length in device counting bad blocks.
 * It is used to calculate if device has enough space.
 *
 * @nfcPtr: The NAND controller
 * @addr: start address
 * @len: size of bytes needed
 *
 * Returns actual size of bytes, a negative error code otherwise.
 ***************************************************************************/
static u32 nand_calculate_skip_length(FNandPsu_T* nfcPtr, u64 addr, u32 len);

/****************************************************************************
 * nand_calculate_skip_badblocknum - Calculate bad blocks to skip.
 *
 * This function calculates bad blocks to skip.
 * It is used to keep "block" number of good blocks
 * before start operate nand.
 *
 * @nfcPtr: The NAND controller
 * @block: start block
 *
 * Returns block number to skip, a negative error code otherwise.
 ***************************************************************************/
static int nand_calculate_skip_badblocknum(FNandPsu_T* nfcPtr, u32 block);

static u32 nand_calculate_skip_length (FNandPsu_T* nfcPtr, u64 addr, u32 len)
{
    struct nand_device* device;
    u32 blocksize;
    u32 left, left2, block;
    u32 act_len = 0;

    device = CTRL_TO_NAND(nfcPtr);
    blocksize = device->model.blocksize;

    while (len > 0)
    {
        left = blocksize - (addr & (blocksize - 1));
        block = addr >> device->model.erase_shift;
        if (len > left)
        {
            left2 = left;
        }
        else
        {
            left2 = len;
        }
        /* Check if the block is bad */
        if (FNandPsu_IsBlockBad(nfcPtr, block) == 0)
        {
            /* good block */
            act_len += left2;
            len -= left2;
        }
        addr += left;
        if (addr >= device->model.device_size)
        {
            break;
        }
    }

    return act_len;
}

static int nand_calculate_skip_badblocknum (FNandPsu_T* nfcPtr, u32 block)
{
    u32 i, base;
    u16 skip_block = 0;

    base = nfcPtr->skip_block_base;

    for (i = base; i < block + skip_block; i++)
    {
        if (FNandPsu_IsBlockBad(nfcPtr, i) != 0)
        {
            skip_block++;
        }
    }
    return skip_block;
}

int FNandPsu_Skip_Erase (FNandPsu_T* nfcPtr, u64 addr, u32 len)
{
    int ret;
    struct nand_device* device;
    u32 blocksize, block;
    u32 act_len; /* bytes including bad blcok*/
    u16 skip_block_num = 0;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    /* Calculate number of skip blocks to ensure enough good block */
    skip_block_num = nand_calculate_skip_badblocknum(
        nfcPtr, addr >> device->model.erase_shift);
    addr += NAND_BLOCK_TO_ADDR(skip_block_num, device);

    /* Calculate the actual length including bad blocks */
    act_len = nand_calculate_skip_length(nfcPtr, addr, len);
    /* Check if the actual length cross flash size */
    if (act_len < len)
    {
        return FMSH_ENOSPC;
    }

    blocksize = device->model.blocksize;
    while (len > 0)
    {
        block = addr >> device->model.erase_shift;
        /* Check if the block is bad */
        if (FNandPsu_IsBlockBad(nfcPtr, block) != 0)
        {
            /* Move to next block */
            addr += blocksize;
            continue;
        }

        /* Erase the Nand flash block */
        ret = FNandPsu_NoSkip_Erase(nfcPtr, addr, blocksize);
        if (ret)
        {
            if (device->bbt_options & NAND_AUTO_MARKBAD)
            {
                ret = FNandPsu_MarkBlockBad(nfcPtr, block);
                if (ret)
                {
                    return ret;
                }
                addr += blocksize;
                continue;
            }
            else
            {
                return len;
            }
        }

        len -= blocksize;
        addr += blocksize;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_Skip_Read (FNandPsu_T* nfcPtr, u64 addr, u32 len, u8* buf,
                        u32 flags)
{
    int ret;
    struct nand_device* device;
    u32 blocksize, block, offset, left;
    u32 act_len; /* bytes including bad blcok*/
    u16 skip_block_num = 0;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    /* Calculate number of skip blocks to ensure enough good block */
    skip_block_num = nand_calculate_skip_badblocknum(
        nfcPtr, addr >> device->model.erase_shift);
    addr += NAND_BLOCK_TO_ADDR(skip_block_num, device);

    /* Calculate the actual length including bad blocks */
    act_len = nand_calculate_skip_length(nfcPtr, addr, len);
    /* Check if the actual length cross flash size */
    if (act_len < len)
    {
        return FMSH_ENOSPC;
    }

    blocksize = device->model.blocksize;
    while (len > 0)
    {
        /* bytes remaind in this block*/
        offset = addr & (blocksize - 1);
        left = blocksize - offset;
        block = addr >> device->model.erase_shift;
        /* Check if the block is bad */
        if (FNandPsu_IsBlockBad(nfcPtr, block) != 0)
        {
            /* Move to next block */
            addr += blocksize;
            continue;
        }

        if (len < left)
        {
            left = len;
        }
        /* Read from the NAND flash */
        ret = FNandPsu_NoSkip_Read(nfcPtr, addr, left, buf, flags);
        if (ret)
        {
            return len;
        }

        len -= left;
        addr += left;
        buf += left;
    }

    return 0;
}

int FNandPsu_Skip_Write (FNandPsu_T* nfcPtr, u64 addr, u32 len, u8* buf,
                         u32 flags)
{
    int ret;
    struct nand_device* device;
    u32 blocksize, block, offset, left;
    u32 act_len; /* bytes including bad blcok*/
    u16 skip_block_num = 0;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    /* Calculate number of skip blocks to ensure enough good block */
    skip_block_num = nand_calculate_skip_badblocknum(
        nfcPtr, addr >> device->model.erase_shift);
    addr += NAND_BLOCK_TO_ADDR(skip_block_num, device);

    /* Calculate the actual length including bad blocks */
    act_len = nand_calculate_skip_length(nfcPtr, addr, len);
    /* Check if the actual length cross flash size */
    if (act_len < len)
    {
        return FMSH_ENOSPC;
    }

    blocksize = device->model.blocksize;
    while (len > 0)
    {
        /* bytes remaind in this block*/
        offset = addr & (blocksize - 1);
        left = blocksize - offset;
        block = addr >> device->model.erase_shift;
        /* Check if the block is bad */
        if (FNandPsu_IsBlockBad(nfcPtr, block) != 0)
        {
            /* Move to next block */
            addr += blocksize;
            continue;
        }

        if (len < left)
        {
            left = len;
        }

        /* write to the NAND flash */
        ret = FNandPsu_NoSkip_Write(nfcPtr, addr, left, buf, flags);
        if (ret)
        {
            if (device->bbt_options & NAND_AUTO_MARKBAD)
            {
                ret = FNandPsu_MarkBlockBad(nfcPtr, block);
                if (ret)
                {
                    return ret;
                }
                addr += blocksize;
                continue;
            }
            else
            {
                return len;
            }
        }

        len -= left;
        addr += left;
        buf += left;
    }

    return FMSH_SUCCESS;
}
