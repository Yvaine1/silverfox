/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_cache.c
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
 * 0.01   ll  07/12/2021  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <stdint.h>

#include "armv8/cortexa53.h"
#include "armv8/fmsh_cache.h"
#include "armv8/fmsh_mmu.h"
#include "armv8/v8_system.h"
#include "bspconfig.h"
#include "fmsh_common_bitops.h"
#include "fmsh_pseudo_asm.h"

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
 * @brief	Enable the Data cache.
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
    uint64_t value = 0;
    uint64_t ELx;
    BOOL invalid = 0;

    mfcp(currentEL, ELx);
    if (ELx == EL3_REG_VALUE)
    {
        mfcp(SCTLR_EL3, value);
        invalid = 1;
#if EL2_LIVE
    }
    else if (ELx == EL2_REG_VALUE)
    {
        mfcp(SCTLR_EL2, value);
#endif
#if EL1_LIVE
    }
    else if (ELx == EL1_REG_VALUE)
    {
        mfcp(SCTLR_EL1, value);
#endif
    }

    if ((value & CONTROL_MMU_BIT) == 0x00000000U)
    {
        Fmsh_MMUEnable();
    }

    /* enable caches only if they are disabled */
    if ((value & CONTROL_DCACHE_BIT) == 0X00000000U)
    {
        if (invalid)
        {
            /* invalidate the Data cache */
            Fmsh_DCacheInvalidate();
        }

        value |= CONTROL_DCACHE_BIT;
        if (ELx == EL3_REG_VALUE)
        {
            /* enable the Data cache for el3*/
            mtcp(SCTLR_EL3, value);
#if EL2_LIVE
        }
        else if (ELx == EL2_REG_VALUE)
        {
            /* enable the Data cache for el1*/
            mtcp(SCTLR_EL2, value);
#endif
#if EL1_LIVE
        }
        else if (ELx == EL1_REG_VALUE)
        {
            /* enable the Data cache for el1*/
            mtcp(SCTLR_EL1, value);
#endif
        }

        dsb();
    }
}

/****************************************************************************/
/**
 * @brief	Disable the Data cache.
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
    uint64_t CsidReg;
    uint64_t C7Reg;
    uint32_t LineSize;
    uint32_t NumWays;
    uint32_t Way;
    uint32_t WayIndex;
    uint32_t WayAdjust;
    uint32_t Set;
    uint32_t SetIndex;
    uint32_t NumSet;
    uint32_t CacheLevel;
    uint64_t value;
    uint64_t ELx;

    mfcp(currentEL, ELx);
    if (ELx == EL3_REG_VALUE)
    {
        mfcp(SCTLR_EL3, value);
        value &= ~BIT2;
        mtcp(SCTLR_EL3, value);
#if EL2_LIVE
    }
    else if (ELx == EL2_REG_VALUE)
    {
        mfcp(SCTLR_EL2, value);
        value &= ~BIT2;
        mtcp(SCTLR_EL2, value);
#endif
#if EL1_LIVE
    }
    else if (ELx == EL1_REG_VALUE)
    {
        mfcp(SCTLR_EL1, value);
        value &= ~BIT2;
        mtcp(SCTLR_EL1, value);
#endif
    }
    dsb();

    /* Number of level of cache*/
    CacheLevel = 0U;
    /* Select cache level 0 and D cache in CSSR */
    mtcp(CSSELR_EL1, CacheLevel);
    isb();

    mfcp(CCSIDR_EL1, CsidReg);

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    /* Number of Ways */
    NumWays = (CsidReg & 0x00001FFFU) >> 3U;
    NumWays += 0x00000001U;

    /*Number of Set*/
    NumSet = (CsidReg >> 13U) & 0x00007FFFU;
    NumSet += 0x00000001U;

    WayAdjust = (uint32_t)clz_c(NumWays);
    WayAdjust -= (uint32_t)0x0000001FU;

    Way = 0U;
    Set = 0U;

    /* Flush all the cachelines */
    for (WayIndex = 0U; WayIndex < NumWays; WayIndex++)
    {
        for (SetIndex = 0U; SetIndex < NumSet; SetIndex++)
        {
            C7Reg = Way | Set | CacheLevel;
            dc(CISW, C7Reg);
            Set += (0x00000001U << LineSize);
        }
        Set = 0U;
        Way += (0x00000001U << WayAdjust);
    }

    /* Wait for Flush to complete */
    dsb();

    /* Select cache level 1 and D cache in CSSR */
    CacheLevel += (0x00000001U << 1U);
    mtcp(CSSELR_EL1, CacheLevel);
    isb();

    mfcp(CCSIDR_EL1, CsidReg);

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    /* Number of Ways */
    NumWays = (CsidReg & 0x00001FFFU) >> 3U;
    NumWays += 0x00000001U;

    /* Number of Sets */
    NumSet = (CsidReg >> 13U) & 0x00007FFFU;
    NumSet += 0x00000001U;

    WayAdjust = (uint32_t)clz_c(NumWays);
    WayAdjust -= (uint32_t)0x0000001FU;

    Way = 0U;
    Set = 0U;

    /* Flush all the cachelines */
    for (WayIndex = 0U; WayIndex < NumWays; WayIndex++)
    {
        for (SetIndex = 0U; SetIndex < NumSet; SetIndex++)
        {
            C7Reg = Way | Set | CacheLevel;
            dc(CISW, C7Reg);
            Set += (0x00000001U << LineSize);
        }
        Set = 0U;
        Way += (0x00000001U << WayAdjust);
    }
    /* Wait for Flush to complete */
    dsb();

    if (ELx == EL3_REG_VALUE)
    {
        tlbi(ALLE3);
#if EL2_LIVE
    }
    else if (ELx == EL2_REG_VALUE)
    {
        tlbi(ALLE2);
#endif
#if EL1_LIVE
    }
    else if (ELx == EL1_REG_VALUE)
    {
        tlbi(VMALLE1);
#endif
    }
    dsb();
    isb();
}

/****************************************************************************/
/**
 * @brief	Invalidate the Data cache. The contents present in the cache are
 * 			cleaned and invalidated.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		In Cortex-A53, functionality to simply invalid the cachelines
 *  			is not present. Such operations are a problem for an environment
 * 			that supports virtualisation. It would allow one OS to invalidate
 * 			a line belonging to another OS. This could lead to the other OS
 * 			crashing because of the loss of essential data. Hence, such
 * 			operations are promoted to clean and invalidate which avoids such
 *			corruption.
 *
 ****************************************************************************/
void Fmsh_DCacheInvalidate (void)
{
    uint64_t currmask;
    register uint64_t CsidReg, C7Reg;
    uint32_t LineSize, NumWays;
    uint32_t Way, WayIndex, WayAdjust, Set, SetIndex, NumSet, CacheLevel;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    /* Number of level of cache*/
    CacheLevel = 0U;

    /* Select cache level 0 and D cache in CSSR */
    mtcp(CSSELR_EL1, CacheLevel);
    isb();

    mfcp(CCSIDR_EL1, CsidReg);

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    /* Number of Ways */
    NumWays = (CsidReg & 0x00001FFFU) >> 3U;
    NumWays += 0X00000001U;

    /*Number of Set*/
    NumSet = (CsidReg >> 13U) & 0x00007FFFU;
    NumSet += 0X00000001U;

    WayAdjust = (uint32_t)clz_c(NumWays);
    WayAdjust -= (uint32_t)0x0000001FU;

    Way = 0U;
    Set = 0U;

    /* Invalidate all the cachelines */
    for (WayIndex = 0U; WayIndex < NumWays; WayIndex++)
    {
        for (SetIndex = 0U; SetIndex < NumSet; SetIndex++)
        {
            C7Reg = Way | Set | CacheLevel;
            dc(ISW, C7Reg);
            Set += (0x00000001U << LineSize);
        }
        Set = 0U;
        Way += (0x00000001U << WayAdjust);
    }

    /* Wait for invalidate to complete */
    dsb();

    /* Select cache level 1 and D cache in CSSR */
    CacheLevel += (0x00000001U << 1U);
    mtcp(CSSELR_EL1, CacheLevel);
    isb();

    mfcp(CCSIDR_EL1, CsidReg);

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    /* Number of Ways */
    NumWays = (CsidReg & 0x00001FFFU) >> 3U;
    NumWays += 0x00000001U;

    /* Number of Sets */
    NumSet = (CsidReg >> 13U) & 0x00007FFFU;
    NumSet += 0x00000001U;

    WayAdjust = (uint32_t)clz_c(NumWays);
    WayAdjust -= (uint32_t)0x0000001FU;

    Way = 0U;
    Set = 0U;

    /* Invalidate all the cachelines */
    for (WayIndex = 0U; WayIndex < NumWays; WayIndex++)
    {
        for (SetIndex = 0U; SetIndex < NumSet; SetIndex++)
        {
            C7Reg = Way | Set | CacheLevel;
            dc(ISW, C7Reg);
            Set += (0x00000001U << LineSize);
        }
        Set = 0U;
        Way += (0x00000001U << WayAdjust);
    }
    /* Wait for invalidate to complete */
    dsb();

    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief	Invalidate a Data cache line. The cacheline is cleaned and
 *			invalidated.
 *
 * @param	adr: 64bit address of the data to be flushed.
 *
 * @return	None.
 *
 * @note		In Cortex-A53, functionality to simply invalid the cachelines
 *  			is not present. Such operations are a problem for an environment
 * 			that supports virtualisation. It would allow one OS to invalidate
 * 			a line belonging to another OS. This could lead to the other OS
 * 			crashing because of the loss of essential data. Hence, such
 * 			operations are promoted to clean and invalidate which avoids such
 *			corruption.
 *
 ****************************************************************************/
void Fmsh_DCacheInvalidateLine (uintptr_t adr)
{
    uint64_t currmask;
    const uint32_t cacheline = DCACHE_LINE_SIZE;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    /* Select cache level 0 and D cache in CSSR */
    mtcp(CSSELR_EL1, 0x0);

    dc(CIVAC, adr & (~(cacheline - 1)));

    /* Wait for invalidate to complete */
    dsb();

    /* Select cache level 1 and D cache in CSSR */
    mtcp(CSSELR_EL1, 0x2);

    dc(IVAC, adr & (~(cacheline - 1)));

    /* Wait for invalidate to complete */
    dsb();

    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief	Invalidate the Data cache for the given address range.
 * 			The cachelines present in the adderss range are
 *			invalidated
 *
 * @param	adr: 64bit start address of the range to be invalidated.
 * @param	len: Length of the range to be invalidated in bytes.
 *
 * @return	None.
 *
 * @note		In Cortex-A53, functionality to simply invalid the cachelines
 *  			is not present. Such operations are a problem for an environment
 * 			that supports virtualisation. It would allow one OS to invalidate
 * 			a line belonging to another OS. This could lead to the other OS
 * 			crashing because of the loss of essential data. Hence, such
 * 			operations are promoted to clean and invalidate which avoids such
 *			corruption.
 *
 ****************************************************************************/
void Fmsh_DCacheInvalidateRange (uintptr_t adr, uint32_t len)
{
    uint64_t currmask;
    const uint64_t cacheline = DCACHE_LINE_SIZE;
    uintptr_t end;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    end = adr + len;
    adr = adr & (~(cacheline - 1));

    if (len != 0U)
    {
        while (adr < end)
        {
            dc(CIVAC, adr);
            adr += cacheline;
        }
    }
    /* Wait for invalidate to complete */
    dsb();
    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief	Flush the Data cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_DCacheFlush (void)
{
    uint64_t currmask;
    register uint64_t CsidReg, C7Reg;
    uint32_t LineSize, NumWays;
    uint32_t Way, WayIndex, WayAdjust, Set, SetIndex, NumSet, CacheLevel;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    /* Number of level of cache*/
    CacheLevel = 0U;
    /* Select cache level 0 and D cache in CSSR */
    mtcp(CSSELR_EL1, CacheLevel);
    isb();

    mfcp(CCSIDR_EL1, CsidReg);

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    /* Number of Ways */
    NumWays = (CsidReg & 0x00001FFFU) >> 3U;
    NumWays += 0x00000001U;

    /*Number of Set*/
    NumSet = (CsidReg >> 13U) & 0x00007FFFU;
    NumSet += 0x00000001U;

    WayAdjust = (uint32_t)clz_c(NumWays);
    WayAdjust -= (uint32_t)0x0000001FU;

    Way = 0U;
    Set = 0U;

    /* Flush all the cachelines */
    for (WayIndex = 0U; WayIndex < NumWays; WayIndex++)
    {
        for (SetIndex = 0U; SetIndex < NumSet; SetIndex++)
        {
            C7Reg = Way | Set | CacheLevel;
            dc(CISW, C7Reg);
            Set += (0x00000001U << LineSize);
        }
        Set = 0U;
        Way += (0x00000001U << WayAdjust);
    }

    /* Wait for Flush to complete */
    dsb();

    /* Select cache level 1 and D cache in CSSR */
    CacheLevel += (0x00000001U << 1U);
    mtcp(CSSELR_EL1, CacheLevel);
    isb();

    mfcp(CCSIDR_EL1, CsidReg);

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    /* Number of Ways */
    NumWays = (CsidReg & 0x00001FFFU) >> 3U;
    NumWays += 0x00000001U;

    /* Number of Sets */
    NumSet = (CsidReg >> 13U) & 0x00007FFFU;
    NumSet += 0x00000001U;

    WayAdjust = (uint32_t)clz_c(NumWays);
    WayAdjust -= (uint32_t)0x0000001FU;

    Way = 0U;
    Set = 0U;

    /* Flush all the cachelines */
    for (WayIndex = 0U; WayIndex < NumWays; WayIndex++)
    {
        for (SetIndex = 0U; SetIndex < NumSet; SetIndex++)
        {
            C7Reg = Way | Set | CacheLevel;
            dc(CISW, C7Reg);
            Set += (0x00000001U << LineSize);
        }
        Set = 0U;
        Way += (0x00000001U << WayAdjust);
    }
    /* Wait for Flush to complete */
    dsb();

    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief	Flush a Data cache line. If the byte specified by the address (adr)
 * 			is cached by the Data cache, the cacheline containing that byte is
 *			invalidated. If the cacheline is modified (dirty), the entire
 *			contents of the cacheline are written to system memory before the
 * 			line is invalidated.
 *
 * @param	adr: 64bit address of the data to be flushed.
 *
 * @return	None.
 *
 * @note		The bottom 6 bits are set to 0, forced by architecture.
 *
 ****************************************************************************/
void Fmsh_DCacheFlushLine (uintptr_t adr)
{
    uint64_t currmask;
    const uint32_t cacheline = DCACHE_LINE_SIZE;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    /* Select cache level 0 and D cache in CSSR */
    mtcp(CSSELR_EL1, 0x0);

    dc(CIVAC, adr & (~(cacheline - 1)));
    /* Wait for flush to complete */
    dsb();

    /* Select cache level 1 and D cache in CSSR */
    mtcp(CSSELR_EL1, 0x2);

    dc(CIVAC, adr & (~(cacheline - 1)));

    /* Wait for flush to complete */
    dsb();

    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief	Enable the instruction cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_ICacheEnable (void)
{
    uint64_t value;
    uint64_t ELx;

    mfcp(currentEL, ELx);
    if (ELx == EL3_REG_VALUE)
    {
        mfcp(SCTLR_EL3, value);
#if EL2_LIVE
    }
    else if (ELx == EL2_REG_VALUE)
    {
        mfcp(SCTLR_EL2, value);
#endif
#if EL1_LIVE
    }
    else if (ELx == EL1_REG_VALUE)
    {
        mfcp(SCTLR_EL1, value);
#endif
    }
    else
    {
        value = 0U;
    }

    /* enable caches only if they are disabled */
    if ((value & CONTROL_ICACHE_BIT) == 0x00000000U)
    {
        /* invalidate the instruction cache */
        Fmsh_ICacheInvalidate();

        value |= CONTROL_ICACHE_BIT;
        if (ELx == EL3_REG_VALUE)
        {
            mtcp(SCTLR_EL3, value);
#if EL2_LIVE
        }
        else if (ELx == EL2_REG_VALUE)
        {
            mtcp(SCTLR_EL2, value);
#endif
#if EL1_LIVE
        }
        else if (ELx == EL1_REG_VALUE)
        {
            mtcp(SCTLR_EL1, value);
#endif
        }

        isb();
    }
}

/****************************************************************************/
/**
 * @brief	Disable the instruction cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_ICacheDisable (void)
{
    uint64_t value = 0;
    uint64_t ELx;

    mfcp(currentEL, ELx);
    if (ELx == EL3_REG_VALUE)
    {
        mfcp(SCTLR_EL3, value);
#if EL2_LIVE
    }
    else if (ELx == EL2_REG_VALUE)
    {
        mfcp(SCTLR_EL2, value);
#endif
#if EL1_LIVE
    }
    else if (ELx == EL1_REG_VALUE)
    {
        mfcp(SCTLR_EL1, value);
#endif
    }
    else
    {
        value = 0U;
    }

    /* disable caches only if they are enabled */
    if (value & CONTROL_ICACHE_BIT)
    {
        /* invalidate the instruction cache */
        Fmsh_ICacheInvalidate();

        value &= ~(CONTROL_ICACHE_BIT);

        if (ELx == EL3_REG_VALUE)
        {
            mtcp(SCTLR_EL3, value);
#if EL2_LIVE
        }
        else if (ELx == EL2_REG_VALUE)
        {
            mtcp(SCTLR_EL2, value);
#endif
#if EL1_LIVE
        }
        else if (ELx == EL1_REG_VALUE)
        {
            mtcp(SCTLR_EL1, value);
#endif
        }

        isb();
    }
}

/****************************************************************************/
/**
 * @brief	Invalidate the entire instruction cache.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_ICacheInvalidate (void)
{
    uint64_t value;

    mfcpsr(value);
    mtcpsr(value | I_BIT | F_BIT);

    /* select ICache */
    mtcp(CSSELR_EL1, 0x1);
    /* invalidate the instruction cache */
    icall(IALLU);
    /* Wait for invalidate to complete */
    dsb();

    mtcpsr(value);
}

/****************************************************************************/
/**
 * @brief	Invalidate an instruction cache line. If the instruction specified
 *			by the parameter adr is cached by the instruction cache, the
 *			cacheline containing that instruction is invalidated.
 *
 * @param	adr: 64bit address of the instruction to be invalidated.
 *
 * @return	None.
 *
 * @note		The bottom 6 bits are set to 0, forced by architecture.
 *
 ****************************************************************************/
void Fmsh_ICacheInvalidateLine (uintptr_t adr)
{
    uint64_t value;
    const uint32_t cacheline = ICACHE_LINE_SIZE;

    mfcpsr(value);
    mtcpsr(value | I_BIT | F_BIT);

    mtcp(CSSELR_EL1, 0x1);
    /*Invalidate I Cache line*/
    ic(IVAU, adr & (~(cacheline - 1)));
    /* Wait for invalidate to complete */
    dsb();

    mtcpsr(value);
}

/****************************************************************************/
/**
 * @brief	Invalidate the instruction cache for the given address range.
 * 			If the instructions specified by the address range are cached by
 * 			the instrunction cache, the cachelines containing those
 *			instructions are invalidated.
 *
 * @param	adr: 64bit start address of the range to be invalidated.
 * @param	len: Length of the range to be invalidated in bytes.
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void Fmsh_ICacheInvalidateRange (uintptr_t adr, uint32_t len)
{
    uint64_t currmask;
    const uint32_t cacheline = ICACHE_LINE_SIZE;
    intptr_t end;
    intptr_t tempadr = adr;
    intptr_t tempend;

    mfcpsr(currmask);
    mtcpsr(currmask | I_BIT | F_BIT);

    if (len != 0x00000000U)
    {
        end = tempadr + len;
        tempend = end;
        tempadr &= ~(cacheline - 0x00000001U);

        /* Select cache Level 0 I-cache in CSSR */
        mtcp(CSSELR_EL1, 0x1);
        while (tempadr < tempend)
        {
            /*Invalidate I Cache line*/
            ic(IVAU, adr & (~(cacheline - 1)));
            tempadr += cacheline;
        }
    }
    /* Wait for invalidate to complete */
    dsb();
    mtcpsr(currmask);
}

/****************************************************************************/
/**
 * @brief	Configure the maximum number of outstanding data prefetches
 *               allowed in L1 cache.
 *
 * @param	num: maximum number of outstanding data prefetches allowed,
 *                    valid values are 0-7.
 *
 * @return	None.
 *
 * @note		This function is implemented only for EL3 privilege level.
 *
 *****************************************************************************/
void Fmsh_ConfigureL1Prefetch (uint8_t num)
{
    uint64_t value = 0;
    uint64_t ELx;

    mfcp(currentEL, ELx);
    if (ELx == EL3_REG_VALUE)
    {
        mfcp(S3_1_C15_C2_0, value);
        value &= ~(L1_DATA_PREFETCH_CONTROL_MASK);
        value |= (num << L1_DATA_PREFETCH_CONTROL_SHIFT);
        mtcp(S3_1_C15_C2_0, value);
    }
}

uint32_t Fmsh_GetCacheLineSize ()
{
    uint64_t CsidReg;
    uint32_t LineSize;
    uint32_t CacheLevel;

    /* Number of level of cache*/
    CacheLevel = 0U;
    /* Select cache level 0 and D cache in CSSR */
    mtcp(CSSELR_EL1, CacheLevel);
    isb();

    mfcp(CCSIDR_EL1, CsidReg);

    /* Get the cacheline size, way size, index size from csidr */
    LineSize = (CsidReg & 0x00000007U) + 0x00000004U;

    return (0x1 << LineSize);
}
