/******************************************************************************
 *
 * Copyright (C) 2009 - 2023 FMSH, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a FMSH device, or
 * (b) that interact with a FMSH device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file cache.c
 *
 * This file contains the cache routes for the processor
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------- -------- ---------------------------------------------------
 * 2.00 hzq      22/11/22 Initial version
 *
 * </pre>
 *
 * @note
 *
 * None.
 *
 ******************************************************************************/
#include <stdint.h>

#include "cortexr5/cortexr5.h"
#include "cortexr5/fmsh_cache.h"
#include "fmsh_pseudo_asm.h"

/****************************************************************************/
/**
 * @brief    Enable the Data cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_DCacheEnable (void)
{
    uint32_t reg;

    /* enable caches only if they are disabled */
    mfcp(CP15_SYS_CONTROL, reg);

    if ((reg & CP15_CONTROL_C_BIT) == 0x00000000U)
    {
        /* invalidate the Data cache */
        Fmsh_DCacheInvalidate();

        /* enable the Data cache */
        reg |= (CP15_CONTROL_C_BIT);

        mtcp(CP15_SYS_CONTROL, reg);
        dsb();
    }
}

/****************************************************************************/
/**
 * @brief    Disable the Data cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_DCacheDisable (void)
{
    uint32_t reg;

    /* clean and invalidate the Data cache */
    Fmsh_DCacheFlush();

    /* disable the Data cache */
    mfcp(CP15_SYS_CONTROL, reg);

    reg &= ~(CP15_CONTROL_C_BIT);

    mtcp(CP15_SYS_CONTROL, reg);

    dsb();
}

/****************************************************************************/
/**
 * @brief    Invalidate the entire Data cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_DCacheInvalidate (void)
{
    uint32_t currmask;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);
    /* Select L1 Data cache in CSSR */
    mtcp(CP15_CACHE_SIZE_SEL, 0);
    /*invalidate all D cache*/
    mtcp(CP15_INVAL_DC_ALL, 0);
    dsb();
    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief    Invalidate a Data cache line. If the byte specified by the
 *           address (adr) is cached by the data cache, the cacheline
 *           containing that byte is invalidated.If the cacheline is modified
 * 	        (dirty), the modified contents are lost and are NOT written
 *           to system memory before the line is invalidated.
 *
 *
 * @param	adr: 32bit address of the data to be flushed.
 *
 * @return	None.
 *
 * @note		The bottom 4 bits are set to 0, forced by architecture.
 *
 ****************************************************************************/
void Fmsh_DCacheInvalidateLine (uintptr_t adr)
{
    uint32_t currmask;
    const uint32_t cacheline = DCACHE_LINE_SIZE;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);
    /* Select L1 Data cache in CSSR */
    mtcp(CP15_CACHE_SIZE_SEL, 0);
    mtcp(CP15_INVAL_DC_LINE_MVA_POC, adr & (~(cacheline - 1)));
    /* Wait for invalidate to complete */
    dsb();

    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief    Invalidate the Data cache for the given address range.
 *           If the bytes specified by the address (adr) are cached by the
 *           Data cache,the cacheline containing that byte is invalidated.
 *           If the cacheline is modified (dirty), the modified contents are
 *           lost and are NOT written to system memory before the line is
 *           invalidated.
 *
 * @param	adr: 32bit start address of the range to be invalidated.
 * @param	len: Length of range to be invalidated in bytes.
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_DCacheInvalidateRange (uintptr_t adr, uint32_t len)
{
    const uint32_t cacheline = DCACHE_LINE_SIZE;
    uint32_t end;
    uint32_t tempadr = adr;
    uint32_t tempend;
    uint32_t currmask;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    if (len != 0U)
    {
        end = tempadr + len;
        tempend = end;
        /* Select L1 Data cache in CSSR */
        mtcp(CP15_CACHE_SIZE_SEL, 0U);

        if ((tempadr & (cacheline - 1U)) != 0U)
        {
            tempadr &= (~(cacheline - 1U));

            Fmsh_DCacheFlushLine(tempadr);
        }
        if ((tempend & (cacheline - 1U)) != 0U)
        {
            tempend &= (~(cacheline - 1U));

            Fmsh_DCacheFlushLine(tempend);
        }

        while (tempadr < tempend)
        {
            /* Invalidate Data cache line */
            mtcp(CP15_INVAL_DC_LINE_MVA_POC, tempadr);

            tempadr += cacheline;
        }
    }

    dsb();
    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief    Flush the entire Data cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_DCacheFlush (void)
{
    register uint32_t CsidReg, C7Reg;
    uint32_t CacheSize, LineSize, NumWays;
    uint32_t Way, WayIndex, Set, SetIndex, NumSet;
    uint32_t currmask;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    /* Select cache level 0 and D cache in CSSR */
    mtcp(CP15_CACHE_SIZE_SEL, 0);

    mfcp(CP15_CACHE_SIZE_ID, CsidReg);
    /* Determine Cache Size */

    CacheSize = (CsidReg >> 13U) & 0x000001FFU;
    CacheSize += 0x00000001U;
    CacheSize *= (uint32_t)128; /* to get number of bytes */

    /* Number of Ways */
    NumWays = (CsidReg & 0x000003ffU) >> 3U;
    NumWays += 0x00000001U;

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    NumSet = CacheSize / NumWays;
    NumSet /= (0x00000001U << LineSize);

    Way = 0U;
    Set = 0U;

    /* Invalidate all the cachelines */
    for (WayIndex = 0U; WayIndex < NumWays; WayIndex++)
    {
        for (SetIndex = 0U; SetIndex < NumSet; SetIndex++)
        {
            C7Reg = Way | Set;
            /* Flush by Set/Way */
            mtcp(CP15_CLEAN_INVAL_DC_LINE_SW, C7Reg);

            Set += (0x00000001U << LineSize);
        }
        Set = 0U;
        Way += 0x40000000U;
    }

    /* Wait for flush to complete */
    dsb();
    mtcpsr(currmask);

    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief   Flush a Data cache line. If the byte specified by the address (adr)
 *          is cached by the Data cache, the cacheline containing that byte is
 *          invalidated.	If the cacheline is modified (dirty), the entire
 *          contents of the cacheline are written to system memory before the
 *          line is invalidated.
 *
 * @param   adr: 32bit address of the data to be flushed.
 *
 * @return	None.
 *
 * @note		The bottom 4 bits are set to 0, forced by architecture.
 *
 ****************************************************************************/
void Fmsh_DCacheFlushLine (uintptr_t adr)
{
    uint32_t currmask;
    const uint32_t cacheline = DCACHE_LINE_SIZE;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    mtcp(CP15_CACHE_SIZE_SEL, 0);

    mtcp(CP15_CLEAN_INVAL_DC_LINE_MVA_POC, adr & (~(cacheline - 1)));

    /* Wait for flush to complete */
    dsb();
    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief    Flush the Data cache for the given address range.
 *           If the bytes specified by the address (adr) are cached by the
 *           Data cache, the cacheline containing those bytes is invalidated.If
 *           the cacheline is modified (dirty), the written to system memory
 *           before the lines are invalidated.
 *
 * @param	adr: 32bit start address of the range to be flushed.
 * @param	len: Length of the range to be flushed in bytes
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_DCacheFlushRange (uintptr_t adr, uint32_t len)
{
    uint32_t LocalAddr = adr;
    const uint32_t cacheline = DCACHE_LINE_SIZE;
    uint32_t end;
    uint32_t currmask;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    if (len != 0x00000000U)
    {
        /* Back the starting address up to the start of a cache line
         * perform cache operations until adr+len
         */
        end = LocalAddr + len;
        LocalAddr &= ~(cacheline - 1U);

        while (LocalAddr < end)
        {
            /* Flush Data cache line */
            mtcp(CP15_CLEAN_INVAL_DC_LINE_MVA_POC, LocalAddr);

            LocalAddr += cacheline;
        }
    }
    dsb();
    mtcpsr(currmask);
}
/****************************************************************************/
/**
 * @brief    Store a Data cache line. If the byte specified by the address
 *           (adr) is cached by the Data cache and the cacheline is modified
 *           (dirty), the entire contents of the cacheline are written to
 *           system memory.After the store completes, the cacheline is marked
 *           as unmodified (not dirty).
 *
 * @param	adr: 32bit address of the data to be stored
 *
 * @return	None.
 *
 * @note		The bottom 4 bits are set to 0, forced by architecture.
 *
 ****************************************************************************/
void Fmsh_DCacheStoreLine (uintptr_t adr)
{
    uint32_t currmask;
    const uint32_t cacheline = ICACHE_LINE_SIZE;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    mtcp(CP15_CACHE_SIZE_SEL, 0);
    mtcp(CP15_CLEAN_DC_LINE_MVA_POC, adr & (~(cacheline - 1)));

    /* Wait for store to complete */
    dsb();
    isb();

    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief    Enable the instruction cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_ICacheEnable (void)
{
    register uint32_t CtrlReg;

    /* enable caches only if they are disabled */
    mfcp(CP15_SYS_CONTROL, CtrlReg);
    if ((CtrlReg & CP15_CONTROL_I_BIT) == 0x00000000U)
    {
        /* invalidate the instruction cache */
        mtcp(CP15_INVAL_IC_POU, 0);

        /* enable the instruction cache */
        CtrlReg |= (CP15_CONTROL_I_BIT);

        mtcp(CP15_SYS_CONTROL, CtrlReg);
        dsb();
        isb();
    }
}

/****************************************************************************/
/**
 * @brief    Disable the instruction cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_ICacheDisable (void)
{
    register uint32_t CtrlReg;

    dsb();

    /* invalidate the instruction cache */
    mtcp(CP15_INVAL_IC_POU, 0);

    /* disable the instruction cache */
    mfcp(CP15_SYS_CONTROL, CtrlReg);

    CtrlReg &= ~(CP15_CONTROL_I_BIT);

    mtcp(CP15_SYS_CONTROL, CtrlReg);
    dsb();
    isb();
}

/****************************************************************************/
/**
 * @brief    Invalidate the entire instruction cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_ICacheInvalidate (void)
{
    uint32_t currmask;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    mtcp(CP15_CACHE_SIZE_SEL, 1);

    /* invalidate the instruction cache */
    mtcp(CP15_INVAL_IC_POU, 0);

    /* Wait for invalidate to complete */
    dsb();
    isb();
    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief    Invalidate an instruction cache line.If the instruction specified
 *           by the address is cached by the instruction cache, the
 *           cacheline containing that instruction is invalidated.
 *
 * @param	adr: 32bit address of the instruction to be invalidated.
 *
 * @return	None.
 *
 * @note		The bottom 4 bits are set to 0, forced by architecture.
 *
 ****************************************************************************/
void Fmsh_ICacheInvalidateLine (uintptr_t adr)
{
    uint32_t currmask;
    const uint32_t cacheline = ICACHE_LINE_SIZE;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    mtcp(CP15_CACHE_SIZE_SEL, 1);
    mtcp(CP15_INVAL_IC_LINE_MVA_POU, adr & (~(cacheline - 1)));

    /* Wait for invalidate to complete */
    dsb();
    isb();
    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief    Invalidate the instruction cache for the given address range.
 *           If the bytes specified by the address (adr) are cached by the
 *           Data cache, the cacheline containing that byte is invalidated.
 *           If the cachelineis modified (dirty), the modified contents are
 *           lost  and are NOT written to system memory before the line is
 *           invalidated.
 *
 * @param	adr: 32bit start address of the range to be invalidated.
 * @param	len: Length of the range to be invalidated in bytes.
 *
 * @return	None.
 *
 ****************************************************************************/
void Fmsh_ICacheInvalidateRange (uintptr_t adr, uint32_t len)
{
    uint32_t LocalAddr = adr;
    const uint32_t cacheline = ICACHE_LINE_SIZE;
    uint32_t end;
    uint32_t currmask;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);
    if (len != 0x00000000U)
    {
        /* Back the starting address up to the start of a cache line
         * perform cache operations until adr+len
         */
        end = LocalAddr + len;
        LocalAddr = LocalAddr & ~(cacheline - 1U);

        /* Select cache L0 I-cache in CSSR */
        mtcp(CP15_CACHE_SIZE_SEL, 1U);

        while (LocalAddr < end)
        {
            /* Invalidate L1 I-cache line */
            mtcp(CP15_INVAL_IC_LINE_MVA_POU, LocalAddr);

            LocalAddr += cacheline;
        }
    }

    /* Wait for invalidate to complete */
    dsb();
    isb();
    mtcpsr(currmask);
}
