///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_reg_hw.c
// PROJECT :
// KEYWORDS:
// LANGUAGE: C
// INTELLIPROP AUTHOR  : ehanke
// CREATED : 8/16/2010
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
#include "iprop_reg_hw.h"

// Assume a nop can occure in 1 cpu cycle.
int sleep (unsigned int loops)
{
    unsigned int ii, jj;
    for (ii = 0; ii < loops; ii++)
    {
        for (jj = 0; jj < SLEEP_CNT; jj++)
        {
            asm("nop");
        }
    }
    return 0;
}

// Assume a nop can occure in 1 cpu cycle.
int usleep (unsigned int loops)
{
    unsigned int ii, jj;
    for (ii = 0; ii < loops; ii++)
    {
        for (jj = 0; jj < USLEEP_CNT; jj++)
        {
            asm("nop");
        }
    }
    return 0;
}

u32 Iprop_RegRead32 (u32 *BaseAddr, u32 Offset)
{
    u32 temp;
    temp = (u32)BaseAddr + Offset;
    return *(volatile int *)temp;
}
void Iprop_RegWrite32 (u32 *BaseAddr, u32 Offset, u32 WriteData)
{
    u32 temp;
    temp = (u32)BaseAddr + Offset;
    *(volatile int *)temp = WriteData;
    return;
}

// volatile int Iprop_RegRead32 ( u32 BaseAddr, u32 Offset)
//{
// volatile int * ptr = (BaseAddr + Offset);
// return *ptr;
//}
//
// void Iprop_RegWrite32 ( u32 BaseAddr, u32 Offset, u32 WriteData)
//{
// volatile int * ptr = (BaseAddr + Offset);
//*ptr = WriteData;
// return;
//}

/**
 *  swap_buf_le16 - swap halves of 16-bit words in place
 *  @buf:  Buffer to swap
 *  @buf_words:  Number of 16-bit words in buffer.
 *
 *  Swap halves of 16-bit words if needed to convert from
 *  little-endian byte order to native cpu byte order, or
 *  vice-versa.
 *
 *  LOCKING:
 *  Inherited from caller.
 */
void swap_buf_le16 (u16 *buf, unsigned int buf_words)
{
    // #ifdef __BIG_ENDIAN
    // unsigned int i;
    //
    // for (i = 0; i < buf_words; i++)
    // buf[i] = __le16_to_cpu(buf[i]);
    // #endif /* __BIG_ENDIAN */
    return;
}

void swap_buf_le32 (u32 *buf_ptr, u32 buf_words)
{
    // #ifdef __BIG_ENDIAN
    // u32 ii;
    // u32 tmp_swp;
    // for(ii=0;ii<buf_words;ii++){
    // tmp_swp = ((buf_ptr[ii] & 0xFF000000) >> 24) |
    //((buf_ptr[ii] & 0x00FF0000) >>  8) |
    //((buf_ptr[ii] & 0x0000FF00) <<  8) |
    //((buf_ptr[ii] & 0x000000FF) << 24) ;
    // buf_ptr[ii] = tmp_swp;
    //}
    // #endif /* __BIG_ENDIAN*/
    return;
}

void swap_buf_leword (u32 *buf_ptr, u32 buf_words)
{
    // #ifdef __BIG_ENDIAN
    // u32 ii;
    // u32 tmp_swp;
    // for(ii=0;ii<buf_words;ii++){
    // tmp_swp = ((buf_ptr[ii] & 0xFFFF0000) >> 16) |
    //((buf_ptr[ii] & 0x0000FFFF) << 16) ;
    // buf_ptr[ii] = tmp_swp;
    //}
    // #endif /* __BIG_ENDIAN*/
    return;
}
