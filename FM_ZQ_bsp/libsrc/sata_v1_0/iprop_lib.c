
///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_lib.c
// PROJECT :
// KEYWORDS:
// LANGUAGE: C
// INTELLIPROP AUTHOR  : ehanke
// CREATED : 2/4/13
//
// DESCRIPTION:
//
// TESTS USED/CREATED:
//
// REVISION HISTORY: Rev1.0
// Date     Person      Description
// -------- ----------- -------------------------------------------------------
//
// CURRENT ISSUES: none.
//
// REMAINING WORK:
//
//
// This media contains an authorized copy or copies of material owned by
// Intelliprop Inc.  This ownership notice and any
// other notices included in machine readable copies must be reproduced on all
// authorized copies.
//
// This is confidential and unpublished property of Intelliprop Inc.
//
// All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include "iprop_lib.h"

u32 iprop_get_lowest_set (u32 in) { return (in & (~in + 1)); }

u32 iprop_get_lowest_cleared (u32 in) { return (~in & (in + 1)); }

u8 iprop_oneHot2Dec (u32 oneHotData)
{
    u32 r;
    if (oneHotData & (oneHotData - 1))
    {
        return 0;
    }

    r = ((0xffff0000 & oneHotData) != 0) ? 0x10 : 0;
    r |= ((0xff00ff00 & oneHotData) != 0) ? 0x08 : 0;
    r |= ((0xf0f0f0f0 & oneHotData) != 0) ? 0x04 : 0;
    r |= ((0xcccccccc & oneHotData) != 0) ? 0x02 : 0;
    r |= ((0xaaaaaaaa & oneHotData) != 0) ? 0x01 : 0;

    return r;
}

// Returns register value when register & mask are not equal to value, or if
// timeout condition occurs
u32 iprop_wait_reg (u32* reg_addr, u32 mask, u32 val, u32 interval_usec,
                    u32 timeout_usec)
{
    u32 ii = 0;
    u32 rc = 0;
    for (ii = 0; ii < (timeout_usec / interval_usec); ii++)
    {
        rc = Iprop_RegRead32(reg_addr, 0);
        if ((rc & mask) != val)
        {
            return rc;
        }
        // branch slot pad...
        Iprop_RegRead32(reg_addr, 0);
        usleep(interval_usec);
    }
    iprop_printf("%s:: wait reg timout. ending register value == %08X\n\r",
                 __func__, rc);
    return rc;
}

/* all set */
u32 iprop_and_group_reg (u32 mask, u32* regs_array, u8 n_ports)
{
    u8 ii;
    for (ii = 0; ii < n_ports; ii++)
    {
        mask &= regs_array[ii];
    }
    return mask;
}

/* any set */
u32 iprop_or_group_reg (u32 mask, u32* regs_array, u8 n_ports)
{
    u8 ii;
    u32 rc = 0;
    for (ii = 0; ii < n_ports; ii++)
    {
        rc |= regs_array[ii] & mask;
    }
    return rc;
}

u32 iprop_min_group_reg (u32 mask, u32* regs_array, u8 n_ports)
{
    u8 ii;
    u32 rc = 0xffffffff;
    for (ii = 0; ii < n_ports; ii++)
    {
        rc = (rc > (regs_array[ii] & mask)) ? (regs_array[ii] & mask) : rc;
    }
    return rc;
}

u32 iprop_max_group_reg (u32 mask, u32* regs_array, u8 n_ports)
{
    u8 ii;
    u32 rc = 0x0;
    for (ii = 0; ii < n_ports; ii++)
    {
        rc = (rc < (regs_array[ii] & mask)) ? (regs_array[ii] & mask) : rc;
    }
    return rc;
}
/* all equal
 RETURNS:: 0 if one or more of the masked registers don't match the rest.
           1 if all the maxked registers match
*/
u32 iprop_equal_group_reg (u32 mask, u32* regs_array, u8 n_ports)
{
    u8 ii;
    u32 rc = regs_array[0] & mask;
    for (ii = 0; ii < n_ports; ii++)
    {
        if (rc != (regs_array[ii] & mask))
        {
            return 0;
        }
    }
    return 1;
}

u32 iprop_endian_swap (u32 a)
{
    u8 tmp_0 = (a >> 0) & 0xFF;
    u8 tmp_1 = (a >> 8) & 0xFF;
    u8 tmp_2 = (a >> 16) & 0xFF;
    u8 tmp_3 = (a >> 24) & 0xFF;

    return ((tmp_0 << 24) | (tmp_1 << 16) | (tmp_2 << 8) | (tmp_3));
}

u32 iprop_cp_sector (u16* dest, u16* source)
{
    u16 ii;
    for (ii = 0; ii < 256; ii++)
    {
        dest[ii] = source[ii];
    }
    return IPROP_STATUS_SUCCESS;
}

int iprop_ceil_log_base2 (u32 val)
{
    u32 ii = 0;
    while (val)
    {
        val >>= 1;
        ii++;
    }
    return ii;
}

unsigned int iprop_align_round_up (unsigned int addr, unsigned int alignment)
{
    if ((addr & (alignment - 1)) == 0)
    {
        return addr;  // Already aligned!
    }

    return (addr + alignment) & ~(alignment - 1);
}

u8 iprop_id_gen_checksum (u8* data)
{
    u16 ii;
    unsigned char sum = 0;

    for (ii = 0; ii < 512; ii++)
    {
        sum += data[ii];
    }
    sum = ~sum;
    sum++;
    return sum;  // return 2's comp of sum.
}
