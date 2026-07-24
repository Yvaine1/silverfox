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
 * @file fmsh_mpu.c
 *
 * This file contains the cortex-r5 mpu functions for the processor
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------- -------- ---------------------------------------------------
 * 1.00  hzq     22/12/02 Initial version
 *
 * </pre>
 *
 * @note
 *
 * None.
 *
 ******************************************************************************/
#include <stdint.h>

#include "bspconfig.h"
#include "cortexr5/cortexr5.h"
#include "cortexr5/fmsh_cache.h"
#include "cortexr5/fmsh_mpu.h"
#include "fmsh_pseudo_asm.h"

#if (DDR_SIZE_MB >= 2048)
#define DDR_NCACHE_BASE (0x7f000000)
#else
#define DDR_NCACHE_BASE ((DDR_SIZE_MB - 16) << 20)
#endif
#define DDR_LOG_MESSAGE_BASE        0x01000000
struct mpu_region {
    unsigned int addr;
    unsigned int region_size;
    unsigned int attrib;
};

static const struct mpu_region default_regions[] = {
    /* 2G of DDR */
    {
        .addr = 0x00000000,
        .region_size = REGION_2G,
#if (USE_DDR == 1) && (PS_PREINITED == 1)
        .attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW,
#else
        .attrib = NO_ACCESS,
#endif
    },
    /* 128M of  LOG and shared memory non-cacheable,align with 16M */
    {
        .addr = DDR_LOG_MESSAGE_BASE,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    {
        .addr = DDR_LOG_MESSAGE_BASE+1*0x1000000,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    {
        .addr = DDR_LOG_MESSAGE_BASE+2*0x1000000,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    {
        .addr = DDR_LOG_MESSAGE_BASE+3*0x1000000,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    {
        .addr = DDR_LOG_MESSAGE_BASE+4*0x1000000,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    {
        .addr = DDR_LOG_MESSAGE_BASE+5*0x1000000,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    {
        .addr = DDR_LOG_MESSAGE_BASE+6*0x1000000,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    {
        .addr = DDR_LOG_MESSAGE_BASE+7*0x1000000,
        .region_size = REGION_16M,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },  
    /* 16 of DDR non-cacheable */
    {
        .addr = DDR_NCACHE_BASE,
        .region_size = REGION_16M,
        .attrib = NORM_NSHARED_NCACHE | PRIV_RW_USER_RW,
    },
    /* 256KB of TCM */
    {
        .addr = 0x00000000,
        .region_size = REGION_256K,
        .attrib = NORM_NSHARED_WB_WA | PRIV_RW_USER_RW,
    },
    /*
     * 1G of strongly ordered memory from 0x80000000 to 0xBFFFFFFF for PL.
     * 512 MB - LPD-PL interface
     * 256 MB - FPD-PL (HPM0) interface
     * 256 MB - FPD-PL (HPM1) interface
     */
    {
        .addr = 0x80000000,
        .region_size = REGION_1G,
        .attrib = STRONG_ORDERD_SHARED | PRIV_RW_USER_RW,
    },
    /* 512M of device memory from 0xC0000000 to 0xDFFFFFFF for QSPI */
    /* 256M of device memory from 0xE0000000 to 0xEFFFFFFF for PCIe Low */
    /* 16M of device memory from 0xF4000000 to 0xF5FFFFFF for DDR PHY Registers
     */
    /* 16M of device memory from 0xF8000000 to 0xF8FFFFFF for STM_CORESIGHT */
    /* 1M of device memory from 0xF9000000 to 0xF90FFFFF for RPU_A53_GIC */
    /* 16M of device memory from 0xFD000000 to 0xFDFFFFFF for FPS slaves */
    /* 16M of device memory from 0xFE000000 to 0xFEFFFFFF for Upper LPS slaves
     */
    /*
     * 16M of device memory from 0xFF000000 to 0xFFFFFFFF for Lower LPS slaves,
     * CSU, PMU, TCM, OCM
     */
    {
        .addr = 0xC0000000U,
        .region_size = REGION_1G,
        .attrib = DEVICE_NONSHARED | PRIV_RW_USER_RW,
    },
    /* 256K of OCM RAM from 0xFFFC0000 to 0xFFFFFFFF marked as normal memory */
    {
        .addr = 0xFFFC0000U,
        .region_size = REGION_256K,
        .attrib = NORM_SHARED_NCACHE | PRIV_RW_USER_RW,
    },
    /* A total of 6 MPU regions are allocated with another 10 being free for
       users */
};

/*****************************************************************************/
/**
 * @brief    Enable MPU for Cortex R5 processor. This function invalidates I
 *           cache and flush the D Caches, and then enables the MPU.
 *
 *
 * @param	None.
 * @return	None.
 *
 ******************************************************************************/
void Fmsh_EnableMPU (void)
{
    uint32_t reg;
    int dcache_status = 0, icache_status = 0;

    mfcp(CP15_SYS_CONTROL, reg);
    /* enable caches only if they are disabled */
    if ((reg & CP15_CONTROL_C_BIT) != 0x00000000U)
    {
        dcache_status = 1;
    }
    if ((reg & CP15_CONTROL_I_BIT) != 0x00000000U)
    {
        icache_status = 1;
    }

    if (dcache_status != 0)
    {
        Fmsh_DCacheDisable();
    }
    if (icache_status != 0)
    {
        Fmsh_ICacheDisable();
    }

    // disable
    mfcp(CP15_SYS_CONTROL, reg);
    reg |= (0x00000001U);
    dsb();
    mtcp(CP15_SYS_CONTROL, reg);
    isb();

    /* enable caches only if they are disabled in routine*/
    if (dcache_status != 0)
    {
        Fmsh_DCacheEnable();
    }
    if (icache_status != 0)
    {
        Fmsh_ICacheEnable();
    }
}

void Fmsh_DisableMPU (void)
{
    uint32_t reg;
    int dcache_status = 0, icache_status = 0;

    mfcp(CP15_SYS_CONTROL, reg);
    /* enable caches only if they are disabled */
    if ((reg & CP15_CONTROL_C_BIT) != 0x00000000U)
    {
        dcache_status = 1;
    }
    if ((reg & CP15_CONTROL_I_BIT) != 0x00000000U)
    {
        icache_status = 1;
    }

    if (dcache_status != 0)
    {
        Fmsh_DCacheDisable();
    }
    if (icache_status != 0)
    {
        Fmsh_ICacheDisable();
    }
    mtcp(CP15_INVAL_BRANCH_ARRAY, 0);

    // disable
    mfcp(CP15_SYS_CONTROL, reg);
    reg &= ~(0x00000001U);
    dsb();
    mtcp(CP15_SYS_CONTROL, reg);
    isb();

    /* enable caches only if they are disabled in routine*/
    if (dcache_status != 0)
    {
        Fmsh_DCacheEnable();
    }
    if (icache_status != 0)
    {
        Fmsh_ICacheEnable();
    }
}

/*****************************************************************************
 *
 * Disable all the MPU regions if any of them is enabled
 *
 * @param	None.
 *
 * @return	None.
 *
 *
 ******************************************************************************/
void Fmsh_DisableMPURegions (void)
{
    int i = 0;
    uint32_t value;

    for (i = 0; i < 16; i++)
    {
        mtcp(CP15_MPU_MEMORY_REG_NUMBER, i);
        mfcp(CP15_MPU_REG_SIZE_EN, value);
        value &= (~REGION_EN);
        dsb();
        mtcp(CP15_MPU_REG_SIZE_EN, value);
        isb();
    }
}

/*****************************************************************************
 *
 * Enable/Disable MPU region
 *
 * @param	None.
 *
 * @return	None.
 *
 *
 ******************************************************************************/
void Fmsh_EnableMPURegion (int region)
{
    uint32_t value;

    mtcp(CP15_MPU_MEMORY_REG_NUMBER, region);
    mfcp(CP15_MPU_REG_SIZE_EN, value);
    value |= (REGION_EN);
    dsb();
    mtcp(CP15_MPU_REG_SIZE_EN, value);
    isb();
}

void Fmsh_DisableMPURegion (int region)
{
    uint32_t value;

    mtcp(CP15_MPU_MEMORY_REG_NUMBER, region);
    mfcp(CP15_MPU_REG_SIZE_EN, value);
    value &= (~REGION_EN);
    dsb();
    mtcp(CP15_MPU_REG_SIZE_EN, value);
    isb();
}

/*****************************************************************************
 *
 * Enable/Disable MPU background region
 *
 * @param	None.
 *
 * @return	None.
 *
 *
 ******************************************************************************/
void Fmsh_EnableBackgroundRegion (void)
{
    uint32_t value;

    mfcp(CP15_SYS_CONTROL, value);
    value |= CP15_CONTROL_BR_BIT;
    dsb();
    mtcp(CP15_SYS_CONTROL, value);
    isb();
}

void Fmsh_DisableBackgroundRegion (void)
{
    uint32_t value;

    mfcp(CP15_SYS_CONTROL, value);
    value &= ~(CP15_CONTROL_BR_BIT);
    dsb();
    mtcp(CP15_SYS_CONTROL, value);
    isb();
}

/*****************************************************************************
 *
 * Set the memory attributes for a section of memory with starting address addr
 * of the region size defined by reg_size having attributes attrib of region
 *number reg_num
 *
 * @param	addr is the address for which attributes are to be set.
 * @param	attrib specifies the attributes for that memory region.
 * @param	reg_size specifies the size for that memory region.
 * @param	reg_num specifies the number for that memory region.
 * @return	None.
 *
 *
 ******************************************************************************/
void Fmsh_SetAttribute (uint32_t addr, uint32_t region_size, int region_num,
                        uint32_t attrib)
{
    uint32_t size = region_size;

    size = size << 1U;
    size |= REGION_EN;
    dsb();
    mtcp(CP15_MPU_MEMORY_REG_NUMBER, region_num);
    isb();
    mtcp(CP15_MPU_REG_BASEADDR, addr);      /* Set base address of a region */
    mtcp(CP15_MPU_REG_ACCESS_CTRL, attrib); /* Set the control attribute */
    mtcp(CP15_MPU_REG_SIZE_EN, size); /* set the region size and enable it*/
    dsb();
    isb(); /* synchronize context on this processor */
}

/*****************************************************************************
 *
 * Find and Set the memory attributes with starting address addr
 * of the region size defined by reg_size
 *
 * @param	addr is the address for which attributes are to be set.
 * @param	attrib specifies the attributes for that memory region.
 * @param	reg_size specifies the size for that memory region.
 * @return	no region return -1.
 *           success return region number
 *
 *
 ******************************************************************************/
int Fmsh_FindSetAttribute (uint32_t addr, uint32_t region_size, uint32_t attrib)
{
    uint32_t value;
    int region_num, max_region_num;

    mfcp(CP15_BUILD_OPTIONS2, value);
    value = (value >> 20U) & 0x3U;
    if (value == 0x2U)
    {
        max_region_num = 12;
    }
    else if (value == 0x3U)
    {
        max_region_num = 16;
    }
    else
    {
        max_region_num = 0;
    }

    region_num = sizeof(default_regions) / sizeof(struct mpu_region);

    if (region_num >= max_region_num)
    {
        return -1;
    }

    Fmsh_SetAttribute(addr, region_size, region_num, attrib);

    return region_num;
}

int Fmsh_InitMPU (void)
{
    int i;
    uint32_t addr, region_size, attrib;

    Fmsh_DisableMPURegions();

    for (i = 0; i < sizeof(default_regions) / sizeof(struct mpu_region); i++)
    {
        addr = default_regions[i].addr;
        region_size = default_regions[i].region_size;
        attrib = default_regions[i].attrib;
        Fmsh_SetAttribute(addr, region_size, i, attrib);
    }

    Fmsh_EnableMPU();

    return 0;
}
