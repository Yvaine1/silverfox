/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gtc_private.h
 *
 * This file contains private constant & function define of gtc
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   tyf  04/24/2023  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_RTC_PRIVATE_H_ /* prevent circular inclusions */
#define _FMSH_RTC_PRIVATE_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_rtc_common.h"

/************************** Constant Definitions *****************************/

/* This macro is used to hardcode the APB data accesses */
#define RTC_IN32P                 FMSH_CAN_IN32_32
#define RTC_OUT32P                FMSH_CAN_OUT32_32

/**
 * DESCRIPTION
 *  Used in conjunction with fmsh_common_bitops.h to access register
 *  bitfields.  They are defined as bit offset/mask pairs for each gpio
 *  register bitfield.
 * NOTES
 *  bfo is the offset of the bitfield with respect to LSB;
 *  bfw is the width of the bitfield
 */
// register bit operation
// arm adopts small terminal mode by default
// SET_TIME_WRITE(READ)
#define bfoCALIB_RW_Max_Tick      0
#define bfwCALIB_RW_Max_Tick      16
#define bfoCALIB_RW_Fraction_Data 16
#define bfwCALIB_RW_Fraction_Data 4
#define bfoCALIB_RW_Fraction_En   20
#define bfwCALIB_RW_Fraction_En   1
// RTC_INT_STATUS\RTC_INT_MASK\RTC_INT_EN\RTC_INT_DIS
#define bfoRTC_INT_Seconds        0
#define bfwRTC_INT_Seconds        1
#define bfoRTC_INT_Alarm          1
#define bfwRTC_INT_Alarm          1
// CONTROL
#define bfoCONTROL_SLVERR_Enable  0
#define bfwCONTROL_SLVERR_Enable  1
#define bfoCONTROL_Osc_Cntrl      24
#define bfwCONTROL_Osc_Cntrl      4
#define bfoCONTROL_Battery_Enable 31
#define bfwCONTROL_Battery_Enable 1
/**************************** Type Definitions *******************************/

/* This is the structure used for accessing the gpio memory map. */
typedef struct FRtcPs_portmap {
    volatile u32 SET_TIME_WRITE;       // 0x00
    volatile u32 SET_TIME_READ;        // 0x04
    volatile u32 CALIB_WRITE;          // 0x08
    volatile u32 CALIB_READ;           // 0x0c
    volatile u32 CURRENT_TIME;         // 0x10
    volatile u32 RESERVE1;             // 0x14
    volatile u32 ALARM;                // 0x18
    volatile u32 RESERVE2;             // 0x1c
    volatile u32 RTC_INT_STATUS;       // 0x20
    volatile u32 RTC_INT_MASK;         // 0x24
    volatile u32 RTC_INT_EN;           // 0x28
    volatile u32 RTC_INT_DIS;          // 0x2c
    volatile u32 ADDR_ERROR;           // 0x30
    volatile u32 ADDR_ERROR_INT_MASK;  // 0x34
    volatile u32 ADDR_ERROR_INT_EN;    // 0x38
    volatile u32 ADDR_ERROR_INT_DIS;   // 0x3c
    volatile u32 CONTROL;              // 0x40
    volatile u32 RESERVE3;             // 0x44
    volatile u32 RESERVE4;             // 0x48
    volatile u32 RESERVE5;             // 0x4c
    volatile u32 SAFETY_CHK;           // 0x50

} FRtcPs_portmap_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
