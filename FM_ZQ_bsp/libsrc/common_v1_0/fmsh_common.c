/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common.c
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
 * 0.01   yl  12/20/2018  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
__weak void fmsh_print (const char *ptr, ...)
{
    if (LOG_OUT)
    {
        PRINTF(ptr);
    }
}

void fmsh_print_dbg (const char *ptr, ...)
{
    if (LOG_LEVEL <= LOG_LEVEL_DEBUG)
    {
        fmsh_print(ptr);
    }
}

void fmsh_print_info (const char *ptr, ...)
{
    if (LOG_LEVEL <= LOG_LEVEL_INFO)
    {
        fmsh_print(ptr);
    }
}

void fmsh_print_warning (const char *ptr, ...)
{
    if (LOG_LEVEL <= LOG_LEVEL_WARNING)
    {
        fmsh_print(ptr);
    }
}

void fmsh_print_err (const char *ptr, ...)
{
    if (LOG_LEVEL <= LOG_LEVEL_ERROR)
    {
        fmsh_print(ptr);
    }
}

void fmsh_print_fatal (const char *ptr, ...)
{
    if (LOG_LEVEL <= LOG_LEVEL_FATAL)
    {
        fmsh_print(ptr);
    }
}

#ifndef FMSH_NASSERT
void onAssert__ (const char *file, unsigned line)
{
    fmsh_print("[ERR]: Error Report: %s, line: %u\r\n", file, line);
    abort();
}
#endif

/*bit_no 1-32
 *set the 0 to bit_no bits 1
 */
u32 mask_generate (u32 bit_no)
{
    u32 i, temp = 0;
    for (i = 0; i < bit_no; i++)
    {
        temp |= (0x01 << i);
    }
    return temp;
}

int fls (int x)
{
    int r = 32;

    if (!x)
    {
        return 0;
    }
    if (!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }
    return r - 1;
}

u32 be_to_cpu32 (u32 be)
{
    u32 le;

    le = ((be & 0xff) << 24) | ((be & 0xff00) << 8) | ((be & 0xff0000) >> 8) |
         ((be & 0xff000000) >> 24);

    return le;
}

u32 be_to_cpu16 (u16 be)
{
    u32 le;

    le = ((be & 0xff) << 8) | ((be & 0xff00) >> 8);

    return le;
}

u32 convert_to_align (void *data, int size)
{
    int i;
    u8 *ptr;
    u32 value;

    ptr = (u8 *)data;
    value = 0;
    for (i = 0; i < size; i++)
    {
        value |= *ptr << (i * 8);
        ptr++;
    }

    return value;
}
