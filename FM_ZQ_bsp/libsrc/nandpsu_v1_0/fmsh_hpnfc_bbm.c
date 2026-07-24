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
 * @file fmsh_hpnfc_bbm.c
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This source file contains bad block management functions.
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
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_hpnfc.h"
#include "fmsh_hpnfc_bbm.h"
#include "fmsh_hpnfc_flash.h"

/**
 * nand_create_bbt - Create RAM based BBT
 *
 * This function scans the NAND flash for factory marked bad blocks and creates
 * a RAM based Bad Block Table(BBT).
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
static int nand_create_bbt(FNandPsu_T *nfcPtr);

/**
 * nand_read_bbt - Search and read BBT in device
 *
 * This function searches the Bad Bloock Table(BBT) in flash and loads into the
 * memory based Bad Block Table(BBT).
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
static int nand_read_bbt(FNandPsu_T *nfcPtr);

/**
 * nand_search_bbt - Search BBT in device
 *
 * This function searches the Bad Bloock Table(BBT) in flash.
 *
 * @nfcPtr: The NAND controller
 * @desc: descriptor of BBT information
 *
 * Returns 0 on success, a negative error code otherwise.
 */
static int nand_search_bbt(FNandPsu_T *nfcPtr, FNandPsu_BbtDesc_T *desc);

/**
 * nand_write_bbt - Write BBT into device
 *
 * This function writes Bad Block Table(BBT)(Include bbInfo) from RAM to flash.
 * save at 1st (and 2nd page) page in block
 *
 * @nfcPtr: The NAND controller
 * @desc: descriptor of BBT information
 * @mirrorDesc: mirror descriptor of BBT information, it is used to avoid
 * overwrite mirror Bad Block Table(BBT).
 *
 * Returns 0 on success, a negative error code otherwise.
 */
static int nand_write_bbt(FNandPsu_T *nfcPtr, FNandPsu_BbtDesc_T *desc,
                           FNandPsu_BbtDesc_T *mirror_desc);

/**
 * nand_update_bbt - Update BBT in device
 *
 * This function updates the primary and mirror Bad Block Table(BBT) in the
 * flash.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
static int nand_update_bbt(FNandPsu_T *nfcPtr);

/**
 * nand_mark_bbt - Write BBT into device
 *
 * This function marks the block containing Bad Block Table as reserved
 * in RAM based BBT.
 *
 * @nfcPtr: The NAND controller
 * @desc: descriptor of BBT information
 *
 * Returns 0 on success, 1 if need update, a negative error code otherwise.
 */
static int nand_mark_bbt(FNandPsu_T *nfcPtr, FNandPsu_BbtDesc_T *desc);

static int nand_create_bbt (FNandPsu_T *nfcPtr)
{
    int ret;
    u32 block, nblocks;
    u32 block_offset;
    u32 block_shift;
    struct nand_device *device;

    device = CTRL_TO_NAND(nfcPtr);

    nblocks = device->model.ntargets *
              (0x1 << (device->model.target_shift - device->model.erase_shift));
    (void)memset(device->bb_info, 0xff, nblocks >> 2);
    /* scan all blocks for factory marked bad blocks */
    for (block = 0; block < nblocks; block++)
    {
        /* Block offset in Bad Block Table(BBT) entry */
        block_offset = block >> 2;
        /* Block shift value in the byte */
        block_shift = (block << 1) & 0x6;
        /* Search for the bad block pattern */
        ret = FNandPsu_CheckBlock(nfcPtr, block);
        if (ret)
        {
            /* Marking as bad block (bbInfo[]) */
            device->bb_info[block_offset] &= ((~(NAND_BLOCK_TYPE_MASK
                                                 << block_shift)) |
                                              (NAND_BLOCK_FACTORY_BAD
                                               << block_shift));
        }
    }

    return FMSH_SUCCESS;
}

static int nand_read_bbt (FNandPsu_T *nfcPtr)
{
    int ret;
    int update1 = 0, update2 = 0;
    int cs, ntargets, nblocks;
    struct nand_device *device;
    FNandPsu_BbtDesc_T *desc;
    FNandPsu_BbtDesc_T *mirror_desc;
    u8 *bb_info;
    u32 bb_info_len;
    u64 bb_info_addr;

    device = CTRL_TO_NAND(nfcPtr);
    desc = &(device->bbt_desc);
    mirror_desc = &(device->bbt_mirror_desc);
    bb_info = device->bb_info;

    /* Search the Bad Block Table(BBT) in flash & config desc */
    (void)nand_search_bbt(nfcPtr, desc);
    (void)nand_search_bbt(nfcPtr, mirror_desc);

    if (device->bbt_options & NAND_BBT_PERCHIP)
    {
        ntargets = device->model.ntargets;
        nblocks = 0x1 << (device->model.target_shift -
                          device->model.erase_shift);
        bb_info_len = nblocks >> 2;
    }
    else
    {
        ntargets = 1;
        nblocks = device->model.ntargets * (0x1 << (device->model.target_shift -
                                                    device->model.erase_shift));
        bb_info_len = nblocks >> 2;
    }

    for (cs = 0; cs < ntargets; cs++)
    {
        if( (desc->valid[cs]) && (mirror_desc->valid[cs]) )
        {
            /* Valid BBT & Mirror BBT found load newer bbt*/
            if (desc->version[cs] > mirror_desc->version[cs])
            {
                bb_info_addr = NAND_PAGE_TO_ADDR(desc->page_offset[cs], device);
                ret = FNandPsu_NoSkip_Read(nfcPtr, bb_info_addr, bb_info_len,
                                           bb_info, 0);
                if (ret)
                {
                    return ret;
                }
                /* updata mirror BBT, Write the BBT to Mirror BBT location in
                 * flash */
                mirror_desc->version[cs] = desc->version[cs];
                update2 = 1;
            }
            else if (desc->version[cs] < mirror_desc->version[cs])
            {
                bb_info_addr = NAND_PAGE_TO_ADDR(mirror_desc->page_offset[cs],
                                                 device);
                ret = FNandPsu_NoSkip_Read(nfcPtr, bb_info_addr, bb_info_len,
                                           bb_info, 0);
                if (ret)
                {
                    return ret;
                }
                /* updata BBT, Write the Mirror BBT to BBT location in flash */
                desc->version[cs] = mirror_desc->version[cs];
                update1 = 1;
            }
            else
            {
                bb_info_addr = NAND_PAGE_TO_ADDR(desc->page_offset[cs], device);
                /* Both are up-to-date */
                ret = FNandPsu_NoSkip_Read(nfcPtr, bb_info_addr, bb_info_len,
                                           bb_info, 0);
                if (ret)
                {
                    return ret;
                }
            }
        }
        else if (desc->valid[cs])
        {
            /* Valid Primary BBT found */
            bb_info_addr = NAND_PAGE_TO_ADDR(desc->page_offset[cs], device);
            ret = FNandPsu_NoSkip_Read(nfcPtr, bb_info_addr, bb_info_len,
                                       bb_info, 0);
            if (ret)
            {
                return ret;
            }
            /* Write the BBT to Mirror BBT location in flash */
            mirror_desc->version[cs] = desc->version[cs];
            update2 = 1;
        }
        else if (mirror_desc->valid[cs])
        {
            /* Valid Mirror BBT found */
            bb_info_addr = NAND_PAGE_TO_ADDR(mirror_desc->page_offset[cs],
                                             device);
            ret = FNandPsu_NoSkip_Read(nfcPtr, bb_info_addr, bb_info_len,
                                       bb_info, 0);
            if (ret)
            {
                return ret;
            }
            /* Write the Mirror BBT to BBT location in flash */
            desc->version[cs] = mirror_desc->version[cs];
            update1 = 1;
        }
        else
        {
            /* No Valid BBT found */
            return FMSH_FAILURE;
        }
        bb_info += bb_info_len;
    }

    if (update1)
    {
        ret = nand_write_bbt(nfcPtr, desc, mirror_desc);
        if (ret)
        {
            return ret;
        }
    }

    if (update2)
    {
        ret = nand_write_bbt(nfcPtr, mirror_desc, desc);
        if (ret)
        {
            return ret;
        }
    }

    return FMSH_SUCCESS;
}

static int nand_search_bbt (FNandPsu_T *nfcPtr, FNandPsu_BbtDesc_T *desc)
{
    int ret;
    struct nand_device *device;
    u32 cs, block, page, offset;
    u32 ntargets, nblocks;
    u32 startblock;
    u8 buf[16];

    device = CTRL_TO_NAND(nfcPtr);

    if (device->bbt_options & NAND_BBT_PERCHIP)
    {
        ntargets = device->model.ntargets;
        nblocks = 0x1 << (device->model.target_shift -
                          device->model.erase_shift);
    }
    else
    {
        ntargets = 1;
        nblocks = device->model.ntargets * (0x1 << (device->model.target_shift -
                                                    device->model.erase_shift));
    }
    startblock = nblocks - 1;

    for (cs = 0; cs < ntargets; cs++)
    {
        /* Read the last 4 blocks for Bad Block Table(BBT) signature */
        for (block = 0; block < desc->max_blocks; block++)
        {
            /* Check bad block marker. if block is bad, skip this block */
            ret = FNandPsu_CheckBlock(nfcPtr, startblock - block);
            if (ret)
            {
                continue;
            }

            page = NAND_BLOCK_TO_PAGE(startblock - block, device);
            ret = FNandPsu_NoSkip_ReadOob(nfcPtr, page, sizeof(buf), buf, 0);
            if (ret)
            {
                continue;
            }

            /* Check the Bad Block Table(BBT) signature */
            for (offset = 0; offset < desc->sig_length; offset++)
            {
                if (buf[desc->sig_offset + offset] != desc->signature[offset])
                {
                    break;
                }
            }
            if (offset >= desc->sig_length)
            {
                /* Bad Block Table(BBT) found */
                fmsh_print_info("BBT found at 0x%012llx.\r\n",
                                NAND_PAGE_TO_ADDR(page, device));
                desc->page_offset[cs] = page;
                desc->version[cs] = buf[desc->ver_offset];
                desc->valid[cs] = 1;
                break;
            }
        }
        startblock += nblocks;
    }

    return FMSH_SUCCESS;
}

static int nand_write_bbt (FNandPsu_T *nfcPtr, FNandPsu_BbtDesc_T *desc,
                           FNandPsu_BbtDesc_T *mirror_desc)
{
    int ret;
    struct nand_device *device;
    int cs, ntargets, nblocks, i;
    u32 startblock, block; /* block which write bad block info to */
    u32 block_offset;
    u32 block_shift;
    u8 block_type;
    u32 bb_info_len; /* number of bad block info bytes */
    u64 bb_info_addr;
    u8 *bb_info, *bbinfo_tmp;
    u8 spare[16];

    device = CTRL_TO_NAND(nfcPtr);
    bb_info = device->bb_info;
    bbinfo_tmp = bb_info;

    if (device->bbt_options & NAND_BBT_WRITE)
    {
        if (device->bbt_options & NAND_BBT_PERCHIP)
        {
            ntargets = device->model.ntargets;
            nblocks = 0x1 << (device->model.target_shift -
                              device->model.erase_shift);
            bb_info_len = nblocks >> 2;
            startblock = nblocks - 1;
        }
        else
        {
            ntargets = 1;
            nblocks = device->model.ntargets *
                      (0x1 << (device->model.target_shift -
                               device->model.erase_shift));
            bb_info_len = nblocks >> 2;
            startblock = nblocks - 1;
        }

        for (cs = 0; cs < ntargets; cs++)
        {
            /* Find a good block to write the Bad Block Table(BBT) */
            if (!desc->valid[cs])
            {
                for (i = 0; i < desc->max_blocks; i++)
                {
                    block = (startblock - i);
                    /* one byte contain 4 bad block info*/
                    block_offset = block >> NAND_BBT_BLOCK_SHIFT;
                    /* each bad block info occupy 2 bits, offset0,2,4,6 */
                    block_shift = (block << 1) & 0x6;
                    block_type = (device->bb_info[block_offset] >>
                                  block_shift) &
                                 NAND_BLOCK_TYPE_MASK;
                    switch (block_type)
                    {
                    case NAND_BLOCK_BAD:
                    case NAND_BLOCK_FACTORY_BAD:
                        continue;
                    default: /* Good Block */
                        break;
                    }
                    /* good block found */
                    desc->page_offset[cs] = NAND_BLOCK_TO_PAGE(block, device);
                    if (desc->page_offset[cs] != mirror_desc->page_offset[cs])
                    {
                        /* Free block found */
                        desc->valid[cs] = 1;
                        break;
                    }
                }

                /* Block not found for writing Bad Block Table(BBT) */
                if (i >= desc->max_blocks)
                {
                    fmsh_print_err("No valid block fount to write BBT.\r\n");
                    return FMSH_FAILURE;
                }
            }

            bb_info_addr = NAND_PAGE_TO_ADDR(desc->page_offset[cs], device);
            /* erase block before markbad */
            ret = FNandPsu_NoSkip_Erase(nfcPtr, bb_info_addr,
                                        device->model.blocksize);
            if (ret)
            {
                return ret;
            }
            /* Write the signature and version in the spare data area */
            (void)memset(spare, 0xff, 16);
            (void)memcpy(spare + desc->sig_offset, &desc->signature[0],
                   desc->sig_length);
            (void)memcpy(spare + desc->ver_offset, &desc->version[cs], 1);
            /* Write the BBT to page offset */
            (void)FNandPsu_SetOobBuf(nfcPtr, spare, 16);
            ret = FNandPsu_NoSkip_Write(nfcPtr, bb_info_addr, bb_info_len,
                                        bbinfo_tmp, NAND_OP_OOBREQ);
            if (ret)
            {
                fmsh_print_err("Failed to write BBT.\r\n");
                return ret;
            }

            startblock += nblocks;
            bbinfo_tmp += bb_info_len;
        }
    }

    return FMSH_SUCCESS;
}

static int nand_mark_bbt (FNandPsu_T *nfcPtr, FNandPsu_BbtDesc_T *desc)
{
    struct nand_device *device;
    int cs, ntargets, nblocks, i;
    u32 startblock, block;
    u32 block_offset;
    u32 block_shift;
    u8 old_val, new_val;
    int update_bbt = 0;

    device = CTRL_TO_NAND(nfcPtr);

    if (device->bbt_options & NAND_BBT_PERCHIP)
    {
        ntargets = device->model.ntargets;
        nblocks = 0x1 << (device->model.target_shift -
                          device->model.erase_shift);
        startblock = nblocks - 1;
    }
    else
    {
        ntargets = 1;
        nblocks = device->model.ntargets * (0x1 << (device->model.target_shift -
                                                    device->model.erase_shift));
        startblock = nblocks - 1;
    }

    for (cs = 0; cs < ntargets; cs++)
    {
        /* Mark the last 4 blocks as Reserved */
        for (i = 0; i < desc->max_blocks; i++, block++)
        {
            block = startblock - i;
            block_offset = block >> NAND_BBT_BLOCK_SHIFT;
            block_shift = (block << 1) & 0x6;
            old_val = device->bb_info[block_offset];
            new_val = old_val & ((~(NAND_BLOCK_TYPE_MASK << block_shift)) |
                                 (NAND_BLOCK_RESERVED << block_shift));
            device->bb_info[block_offset] = new_val;

            if (old_val != new_val)
            {
                update_bbt = 1;
            }
        }
        startblock += nblocks;
    }

    return update_bbt;
}

static int nand_update_bbt (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device;
    int cs, ntargets;
    u8 version;

    device = CTRL_TO_NAND(nfcPtr);

    if (device->bbt_options & NAND_BBT_PERCHIP)
    {
        ntargets = device->model.ntargets;
    }
    else
    {
        ntargets = 1;
    }
    /* Update the version number */
    for (cs = 0; cs < ntargets; cs++)
    {
        version = device->bbt_desc.version[cs];
        device->bbt_desc.version[cs] = (version + 1) % 256;
        version = device->bbt_mirror_desc.version[cs];
        device->bbt_mirror_desc.version[cs] = (version + 1) % 256;
    }

    /* Update the primary Bad Block Table(BBT) in flash */
    ret = nand_write_bbt(nfcPtr, &device->bbt_desc, &device->bbt_mirror_desc);
    if (ret)
    {
        return ret;
    }
    /* Update the mirrored Bad Block Table(BBT) in flash */
    ret = nand_write_bbt(nfcPtr, &device->bbt_mirror_desc, &device->bbt_desc);
    if (ret)
    {
        return ret;
    }

    return 0;
}

int FNandPsu_InitBBT (FNandPsu_T *nfcPtr)
{
    struct nand_device *device;
    int i;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    /* Initialize primary Bad Block Table(BBT) */
    for (i = 0; i < NAND_MAX_CHIPS; i++)
    {
        device->bbt_desc.page_offset[i] = 0;
        device->bbt_desc.version[i] = 0;
        device->bbt_desc.valid[i] = 0;
    }
    device->bbt_desc.sig_offset = NAND_BBT_SIG_OFFSET;
    device->bbt_desc.ver_offset = NAND_BBT_VER_OFFSET;
    device->bbt_desc.sig_length = NAND_BBT_SIG_LEN;
    device->bbt_desc.max_blocks = NAND_BBT_MAX_BLOCKS;
    (void)strcpy(&device->bbt_desc.signature[0], "Bbt0");

    /* Initialize mirror Bad Block Table(BBT) */
    for (i = 0; i < NAND_MAX_CHIPS; i++)
    {
        device->bbt_mirror_desc.page_offset[i] = 0;
        device->bbt_mirror_desc.version[i] = 0;
        device->bbt_mirror_desc.valid[i] = 0;
    }
    device->bbt_mirror_desc.sig_offset = NAND_BBT_SIG_OFFSET;
    device->bbt_mirror_desc.ver_offset = NAND_BBT_VER_OFFSET;
    device->bbt_mirror_desc.sig_length = NAND_BBT_SIG_LEN;
    device->bbt_mirror_desc.max_blocks = NAND_BBT_MAX_BLOCKS;
    (void)strcpy(&device->bbt_mirror_desc.signature[0], "1tbB");

    /* Initialize Bad block search pattern structure */
    if (device->model.pagesize > 512)
    {
        device->bb_pattern.options = NAND_BBT_SCAN_2ND_PAGE;
        device->bb_pattern.offset = NAND_BB_PATTERN_OFFSET_LARGE_PAGE;
        device->bb_pattern.length = NAND_BB_PATTERN_LENGTH_LARGE_PAGE;
    }
    else
    {
        device->bb_pattern.options = 0;
        device->bb_pattern.offset = NAND_BB_PATTERN_OFFSET_SMALL_PAGE;
        device->bb_pattern.length = NAND_BB_PATTERN_LENGTH_SMALL_PAGE;
    }
    for (i = 0; i < 2; i++)
    {
        device->bb_pattern.pattern[i] = NAND_BB_PATTERN;
    }

    return FMSH_SUCCESS;
}

int FNandPsu_ScanBBT (FNandPsu_T *nfcPtr)
{
    int ret;
    int update1, update2;
    struct nand_device *device;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    /* Try to search and read BBT in device */
    if (device->bbt_options & NAND_USE_FLASH_BBT)
    {
        ret = nand_read_bbt(nfcPtr);
    }
    else
    {
        ret = FMSH_FAILURE;
    }

    if (ret)
    {
        /* Create memory based Bad Block Table(BBT) */
        fmsh_print_dbg("No bbt found, scan entire chip to create bbt.\r\n");
        (void)nand_create_bbt(nfcPtr);
        /* Mark the blocks containing Bad Block Table(BBT) as Reserved */
        update1 = nand_mark_bbt(nfcPtr, &device->bbt_desc);
        update2 = nand_mark_bbt(nfcPtr, &device->bbt_mirror_desc);
        if ((update1 == 1) || (update2 == 1))
        {
            ret = nand_update_bbt(nfcPtr);
            if (ret)
            {
                return ret;
            }
        }
    }

    return FMSH_SUCCESS;
}

int FNandPsu_CheckBlock (FNandPsu_T *nfcPtr, u32 block)
{
    int ret;
    struct nand_device *device;
    u32 num_pages;
    u32 page_index; /* Page index which stores bad block pattern */
    u32 length;     /* Pattern length index */
    u8 spare[8];
    int raw = 1;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    if (nfcPtr->usercfg->options & NAND_BBM_WITHECC)
    {
        raw = 0;
    }

    if (block >= (device->model.device_size >> device->model.erase_shift))
    {
        return FMSH_EINVAL;
    }

    /* Number of pages to search for bad block pattern */
    if (device->bb_pattern.options & NAND_BBT_SCAN_2ND_PAGE)
    {
        num_pages = 2;
    }
    else
    {
        num_pages = 1;
    }

    /* Search for the bad block pattern */
    for (page_index = 0; page_index < num_pages; page_index++)
    {
        ret = FNandPsu_NoSkip_ReadOob(
            nfcPtr, NAND_BLOCK_TO_PAGE(block, device) + page_index, 8, spare,
            raw);
        if (ret)
        {
            return ret;
        }

        /* Read the spare bytes to check for bad block pattern */
        for (length = 0; length < device->bb_pattern.length; length++)
        {
            if (spare[device->bb_pattern.offset + length] !=
                device->bb_pattern.pattern[length])
            {
                /* Bad block found, return error to marking as bad block */
                fmsh_print_warning("Bad block %d at 0x%012llx\r\n", block,
                                   NAND_BLOCK_TO_ADDR(block, device));
                return 1;
            }
        }
    }
    return 0;
}

/**
 * This function marks a block as bad in the RAM based Bad Block
 * Table(nfc->bbInfo[]). It also updates the Bad Block Table(BBT) in the flash.
 */
int FNandPsu_MarkBlockBad (FNandPsu_T *nfcPtr, u32 block)
{
    int ret;
    struct nand_device *device;
    u8 spare[8];
    u8 num_pages, page_index;
    u32 block_offset; /* byte offset in bbt contain bad block info */
    u32 block_shift;  /* shift in byte save bad block info */
    u8 data;          /* contain bad block info for this block */
    u8 old_val, new_val;
    int raw = 1;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    if (nfcPtr->usercfg->options & NAND_BBM_WITHECC)
    {
        raw = 0;
    }

    if (block >= (device->model.device_size >> device->model.erase_shift))
    {
        return FMSH_EINVAL;
    }

    /* erase block before markbad */
    FNandPsu_NoSkip_Erase(nfcPtr, NAND_BLOCK_TO_ADDR(block, device),
                          device->model.blocksize);

    /* write bad block pattern to block spare */
    (void)memset(spare, 0xff, 8);
    (void)memset(&spare[device->bb_pattern.offset], 0x00, device->bb_pattern.length);
    if (device->bb_pattern.options & NAND_BBT_SCAN_2ND_PAGE)
    {
        num_pages = 2;
    }
    else
    {
        num_pages = 1;
    }
    for (page_index = 0; page_index < num_pages; page_index++)
    {
        FNandPsu_NoSkip_WriteOob(nfcPtr,
                                 NAND_BLOCK_TO_PAGE(block, device) + page_index,
                                 8, spare, raw);
    }
    /* one byte contain 4 bad block info*/
    block_offset = block >> NAND_BBT_BLOCK_SHIFT;
    data = device->bb_info[block_offset];
    /* each bad block info occupy 2 bits, offset0,2,4,6 */
    block_shift = (block << 1) & 0x6;
    /* Mark the block as bad in the RAM based Bad Block Table */
    old_val = data;
    data &= ((~(NAND_BLOCK_TYPE_MASK << block_shift)) |
             (NAND_BLOCK_BAD << block_shift));
    new_val = data;
    device->bb_info[block_offset] = data;

    /* Update the Bad Block Table(BBT) in flash */
    if (old_val != new_val)
    {
        ret = nand_update_bbt(nfcPtr);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

int FNandPsu_IsBlockBad (FNandPsu_T *nfcPtr, u32 block)
{
    struct nand_device *device;
    u8 data;
    u32 block_offset;
    u32 block_shift;
    u8 block_type;

    FMSH_ASSERT(nfcPtr != NULL);

    device = CTRL_TO_NAND(nfcPtr);

    if (block >= (device->model.device_size >> device->model.erase_shift))
    {
        return FMSH_EINVAL;
    }

    block_offset = block >> NAND_BBT_BLOCK_SHIFT;
    block_shift = (block << 1) & NAND_BLOCK_SHIFT_MASK;
    data = device->bb_info[block_offset];
    block_type = (data >> block_shift) & NAND_BLOCK_TYPE_MASK;

    if (block_type != NAND_BLOCK_GOOD)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
