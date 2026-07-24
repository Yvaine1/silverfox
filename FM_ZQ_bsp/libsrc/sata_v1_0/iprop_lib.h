
///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_lib.h
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

#ifndef _IPROP_LIB_
#define _IPROP_LIB_

#include "iprop_reg_hw.h"
#include "iprop_types.h"

u32 iprop_wait_reg(u32* reg_addr, u32 mask, u32 val, u32 interval_usec,
                   u32 timeout_usec);

/* all set */
u32 iprop_and_group_reg(u32 mask, u32* regs_array, u8 n_ports);

/* any set */
u32 iprop_or_group_reg(u32 mask, u32* regs_array, u8 n_ports);

u32 iprop_min_group_reg(u32 mask, u32* regs_array, u8 n_ports);

u32 iprop_max_group_reg(u32 mask, u32* regs_array, u8 n_ports);
/* all equal
 RETURNS:: 0 if one or more of the masked registers don't match the rest.
           1 if all the maxked registers match
*/
u32 iprop_equal_group_reg(u32 mask, u32* regs_array, u8 n_ports);

u32 iprop_endian_swap(u32 a);

int iprop_ceil_log_base2(u32 val);

unsigned int iprop_align_round_up(unsigned int addr, unsigned int alignment);

// Use for pre-processor.
// returns the first '1' set in the input val
#define IPR_GET_FIRST_SET(val) ((val) & (~(val) + 1))

// Use for pre-processor defines only. If variable, use the
// iprop_ceil_log_base2() function
#define IPR_CLB2(val)           \
    ((val) > 0x80000000)   ? 32 \
    : ((val) > 0x40000000) ? 31 \
    : ((val) > 0x20000000) ? 30 \
    : ((val) > 0x10000000) ? 29 \
    : ((val) > 0x08000000) ? 28 \
    : ((val) > 0x04000000) ? 27 \
    : ((val) > 0x02000000) ? 26 \
    : ((val) > 0x01000000) ? 25 \
    : ((val) > 0x00800000) ? 24 \
    : ((val) > 0x00400000) ? 23 \
    : ((val) > 0x00200000) ? 22 \
    : ((val) > 0x00100000) ? 21 \
    : ((val) > 0x00080000) ? 20 \
    : ((val) > 0x00040000) ? 19 \
    : ((val) > 0x00020000) ? 18 \
    : ((val) > 0x00010000) ? 17 \
    : ((val) > 0x00008000) ? 16 \
    : ((val) > 0x00004000) ? 15 \
    : ((val) > 0x00002000) ? 14 \
    : ((val) > 0x00001000) ? 13 \
    : ((val) > 0x00000800) ? 12 \
    : ((val) > 0x00000400) ? 11 \
    : ((val) > 0x00000200) ? 10 \
    : ((val) > 0x00000100) ? 9  \
    : ((val) > 0x00000080) ? 8  \
    : ((val) > 0x00000040) ? 7  \
    : ((val) > 0x00000020) ? 6  \
    : ((val) > 0x00000010) ? 5  \
    : ((val) > 0x00000008) ? 4  \
    : ((val) > 0x00000004) ? 3  \
    : ((val) > 0x00000002) ? 2  \
    : ((val) > 0x00000001) ? 1  \
                           : 0

enum {
    IPROP_STATUS_SUCCESS = 0,
    IPROP_STATUS_ERR = (1 << 31),
    IPROP_STATUS_TIMEOUT = (1 << 30),
    IPROP_STATUS_INVALID_ARGS = (1 << 29),
};

/********************************************************************
 * Move data from source to destination pointers with the processor
 *  Slow, but it works...
 * @ u16 * dest
 * @ u16 * source
 * Returns:
 *  0
 ********************************************************************/
u32 iprop_cp_sector(u16* dest, u16* source);

u8 iprop_id_gen_checksum(u8* data);

u8 iprop_oneHot2Dec(u32 oneHotData);

u32 iprop_get_lowest_cleared(u32 in);

u32 iprop_get_lowest_set(u32 in);

#endif  // define _IPROP_LIB_
