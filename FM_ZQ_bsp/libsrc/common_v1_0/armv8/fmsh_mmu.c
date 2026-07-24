/***************************** Include Files *********************************/

#include "armv8/cortexa53.h"
#include "armv8/fmsh_cache.h"
#include "armv8/fmsh_mmu.h"
#include "armv8/v8_mmu.h"
#include "armv8/v8_system.h"
#include "bspconfig.h"
#include "fmsh_common_bitops.h"
#include "fmsh_common_types.h"
#include "fmsh_pseudo_asm.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/

#define BLOCK_SIZE_2MB    0x200000U
#define BLOCK_SIZE_1GB    0x40000000U
#define ADDRESS_LIMIT_4GB 0x100000000UL

/************************** Variable Definitions *****************************/
/* EL3 MMU Table */
extern INTPTR MMUTableL0;
extern INTPTR MMUTableL1;
extern INTPTR MMUTableL2;

/* EL1 MMU Table */
extern INTPTR MMUTableEL1L0;
extern INTPTR MMUTableEL1L1;
extern INTPTR MMUTableEL1L2;

void Fmsh_MMUEnable (void)
{
    u64 table;
    u64 ttbr0, mair, tcr, sctlr;
    u64 ELx;

    table = (u64)&MMUTableL0;
    mfcp(currentEL, ELx);
    if (ELx == EL3_REG_VALUE)
    {
        mfcp(SCTLR_EL3, sctlr);
#if EL2_LIVE
    }
    else if (ELx == EL2_REG_VALUE)
    {
        mfcp(SCTLR_EL2, sctlr);
#endif
#if EL1_LIVE
    }
    else if (ELx == EL1_REG_VALUE)
    {
        mfcp(SCTLR_EL1, sctlr);
        table = (u64)&MMUTableEL1L0;
#endif
    }

    if (sctlr & CONTROL_MMU_BIT)
    {
        return;  // mmu already
    }
#if EL1_LIVE
    if (ELx == EL1_REG_VALUE)
    {
        /**********************************************
         * Set up TCR_EL1, the TCR_EL1 is special
         * Physical Address Size PS =  010 -> 40bits 1TB
         * Granual Size TG0 = 00 -> 4KB
         * size offset of the memory region T0SZ = 24 -> (region size 2^(64-24)
         *= 2^40)
         ***************************************************/
        tcr = (u64)0x280803518;
    }
    else
#endif
    {
        /**********************************************
         * Set up TCR_EL3
         * Physical Address Size PS =  010 -> 40bits 1TB
         * Granual Size TG0 = 00 -> 4KB
         * size offset of the memory region T0SZ = 24 -> (region size 2^(64-24)
         *= 2^40)
         ***************************************************/
        /*
           table = (u64)&MMUTableL0;
           tcr = (TCR_SIZE_1T << 16) + (TCR_GRANULE_4K << 14)
               + (TCR_SHARE_INNER << 12) + (TCR_RGN_WBWA << 10)
               + (TCR_RGN_WBWA << 8) + (24 << 0);
       */
        //    table = (u64)&MMUTableL0;
        tcr = 0x80823518;
    }
    //    table = (u64)&MMUTableL1;
    //    tcr = 0x80803520;
    ttbr0 = table;

    /**********************************************
     * Set up memory attributes
     * This equates to:
     * 0 = b01000100 = Normal, Inner/Outer Non-Cacheable
     * 1 = b11111111 = Normal, Inner/Outer WB/WA/RA
     * 2 = b00000000 = Device-nGnRnE
     * 3 = b00000100 = Device-nGnRE
     * 4 = b10111011 = Normal, Inner/Outer WT/WA/RA
     * 5 = b11001100 = Normal, Inner/Outer WB/WnA/RnA // for nEXTERRIRQ irq test
     **********************************************/
    mair = (u64)0x00CCBB0400FF44;

    sctlr |= BIT3;  // Enable SP alignment check
    sctlr |= BIT0;  // Enable MMU

    if (ELx == EL3_REG_VALUE)
    {
        mtcp(TTBR0_EL3, ttbr0);
        mtcp(MAIR_EL3, mair);
        mtcp(TCR_EL3, tcr);
        isb();

        mtcp(SCTLR_EL3, sctlr);
#if EL2_LIVE
    }
    else if (ELx == EL2_REG_VALUE)
    {
        mtcp(TTBR0_EL2, ttbr0);
        mtcp(MAIR_EL2, mair);
        mtcp(TCR_EL2, tcr);
        isb();

        mtcp(SCTLR_EL2, sctlr);
#endif
#if EL1_LIVE
    }
    else if (ELx == EL1_REG_VALUE)
    {
        mtcp(TTBR0_EL1, ttbr0);
        mtcp(MAIR_EL1, mair);
        mtcp(TCR_EL1, tcr);
        isb();

        mtcp(SCTLR_EL1, sctlr);
#endif
    }

    dsb();
    isb();
}

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/**
 * brief     It sets the memory attributes for a section, in the translation
 *           table. If the address (defined by Addr) is less than 4GB, the
 *           memory attribute(attrib) is set for a section of 2MB memory. If the
 *           address (defined by Addr) is greater than 4GB, the memory attribute
 *           (attrib) is set for a section of 1GB memory.
 *
 * @param    Addr: 64-bit address for which attributes are to be set.
 * @param    attrib: Attribute for the specified memory region. fmsh_mmu.h
 *           contains commonly used memory attributes definitions which can be
 *           utilized for this function.
 *
 * @return   None.
 *
 * @note     The MMU and D-cache need not be disabled before changing an
 *           translation table attribute.
 *
 ******************************************************************************/
void Fmsh_SetTlbAttributes (UINTPTR Addr, u64 attrib)
{
    u64 *ptr;
    u64 section;
    u64 block_size;
    u64 ELx;
    u64 *p_tlbl2;

    p_tlbl2 = (u64 *)&MMUTableL2;
    mfcp(currentEL, ELx);
#if EL1_LIVE
    if (ELx == EL1_REG_VALUE)
    {
        p_tlbl2 = (u64 *)&MMUTableEL1L2;
    }
#endif
    /* if region is less than 4GB MMUTable level 2 need to be modified */
    //    if(Addr < ADDRESS_LIMIT_4GB){
    /* block size is 2MB for addressed < 4GB*/
    block_size = BLOCK_SIZE_2MB;
    section = Addr / block_size;
    ptr = p_tlbl2 + section;
    //    }
    /* if region is greater than 4GB MMUTable level 1 need to be modified */
    //    else{
    //        /* block size is 1GB for addressed > 4GB */
    //        block_size = BLOCK_SIZE_1GB;
    //        section = Addr / block_size;
    //        ptr = &MMUTableL1 + section;
    //    }
    *ptr = (Addr & (~(block_size - 1))) | attrib;

    Fmsh_DCacheFlush();

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

    dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */
}

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/**
 * brief     It sets the memory attributes for a section, in the translation
 *           table. If the address (defined by Addr) is less than 4GB, the
 *           memory attribute(attrib) is set for a section of 2MB memory. If the
 *           address (defined by Addr) is greater than 4GB, the memory attribute
 *           (attrib) is set for a section of 1GB memory.
 *
 * @param    Addr: 64-bit address for which attributes are to be set.
 * @param    Size: Range size, extended to 2M Byte alignment.
 * @param    attrib: Attribute for the specified memory region. fmsh_mmu.h
 *           contains commonly used memory attributes definitions which can be
 *           utilized for this function.
 *
 * @return   None.
 *
 * @note     The MMU and D-cache need not be disabled before changing an
 *           translation table attribute.
 *
 ******************************************************************************/
void Fmsh_SetTlbAttributesRange (UINTPTR Addr, u64 Size, u64 attrib)
{
    u64 *ptr;
    u64 section;
    u64 block_size;
    u64 end_addr;
    u64 ELx;
    u64 *p_tlbl2;

    p_tlbl2 = (u64 *)&MMUTableL2;
    mfcp(currentEL, ELx);
#if EL1_LIVE
    if (ELx == EL1_REG_VALUE)
    {
        p_tlbl2 = (u64 *)&MMUTableEL1L2;
    }
#endif
    /* block size is 2MB for addressed < 4GB*/
    block_size = BLOCK_SIZE_2MB;
    section = Addr / block_size;
    ptr = p_tlbl2 + section;
    end_addr = Addr + Size;

    do
    {
        *ptr = (Addr & (~(block_size - 1))) | attrib;
        Addr += block_size;
        ptr++;
    } while (Addr < end_addr);

    Fmsh_DCacheFlush();

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

    dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */
}
