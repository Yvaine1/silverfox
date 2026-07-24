///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_reg_hw.h
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

#ifndef IPROP_REGISTER_INTERFACE
#define IPROP_REGISTER_INTERFACE

#include "iprop_types.h"

#define XPAR_CPU_CORE_CLOCK_FREQ_HZ 20000000  // 20MHz

                                              // #define NIOS
// #include "little_endian.h"
// #include "xparameters.h" //2.18
#define USLEEP_CNT                  XPAR_CPU_CORE_CLOCK_FREQ_HZ / 1000000
#define SLEEP_CNT                   XPAR_CPU_CORE_CLOCK_FREQ_HZ
// #include "xil_io.h"  // 2.18

int sleep(unsigned int loops);

int usleep(unsigned int loops);

void swap_buf_le16(u16* buf, unsigned int buf_words);
void swap_buf_le32(u32* buf_ptr, u32 buf_words);
void swap_buf_leword(u32* buf_ptr, u32 buf_words);
/* Dynamic bus access functions */

u32 Iprop_RegRead32(u32* BaseAddr, u32 Offset);
void Iprop_RegWrite32(u32* BaseAddr, u32 Offset, u32 WriteData);

#define DEBUG_PRINT
#ifdef DEBUG_PRINT
// #define iprop_printf xil_printf
#define iprop_printf printf
#else
#define iprop_printf(...)
#endif
// #define iprop_printf xil_printf
// #else
// #endif

#endif
