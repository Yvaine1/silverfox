/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_can_private.h
 *
 * This file contains private constant & function define
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   wfb  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FMSH_CAN_PRIVATE_H_
#define _FMSH_CAN_PRIVATE_H_
#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/

#include "fmsh_can_common.h"

/************************** Constant Definitions *****************************/

/**
 * This macro is used to hardcode the APB data accesses,
 */
#define CAN_INP                  FMSH_CAN_IN32_32
#define CAN_OUTP                 FMSH_CAN_OUT32_32

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
// reg_grp1
// CFG_STAT register
#define bfoCAN_CFG_STAT_BUSOFF   0
#define bfwCAN_CFG_STAT_BUSOFF   1
#define bfoCAN_CFG_STAT_TACTIVE  1
#define bfwCAN_CFG_STAT_TACTIVE  1
#define bfoCAN_CFG_STAT_RACTIVE  2
#define bfwCAN_CFG_STAT_RACTIVE  1
#define bfoCAN_CFG_STAT_TSSS     3
#define bfwCAN_CFG_STAT_TSSS     1
#define bfoCAN_CFG_STAT_TPSS     4
#define bfwCAN_CFG_STAT_TPSS     1
#define bfoCAN_CFG_STAT_LBMI     5
#define bfwCAN_CFG_STAT_LBMI     1
#define bfoCAN_CFG_STAT_LBME     6
#define bfwCAN_CFG_STAT_LBME     1
#define bfoCAN_CFG_STAT_RESET    7
#define bfwCAN_CFG_STAT_RESET    1
// TCMD register
#define bfoCAN_TCMD_TSA          8
#define bfwCAN_TCMD_TSA          1
#define bfoCAN_TCMD_TSALL        9
#define bfwCAN_TCMD_TSALL        1
#define bfoCAN_TCMD_TSONE        10
#define bfwCAN_TCMD_TSONE        1
#define bfoCAN_TCMD_TPA          11
#define bfwCAN_TCMD_TPA          1
#define bfoCAN_TCMD_TPE          12
#define bfwCAN_TCMD_TPE          1
#define bfoCAN_TCMD_STBY         13
#define bfwCAN_TCMD_STBY         1
#define bfoCAN_TCMD_LOM          14
#define bfwCAN_TCMD_LOM          1
#define bfoCAN_TCMD_TBSEL        15
#define bfwCAN_TCMD_TBSEL        1
// TCTRL register
#define bfoCAN_TCTRL_TSSTAT      16
#define bfwCAN_TCTRL_TSSTAT      2
// 2 bit blank

#define bfoCAN_TCTRL_TTTBM       20
#define bfwCAN_TCTRL_TTTBM       1
#define bfoCAN_TCTRL_TSMODE      21
#define bfwCAN_TCTRL_TSMODE      1
#define bfoCAN_TCTRL_TSNEXT      22
#define bfwCAN_TCTRL_TSNEXT      1
#define bfoCAN_TCTRL_FD_ISO      23
#define bfwCAN_TCTRL_FD_ISO      1
// RCTRL register
#define bfoCAN_RCTRL_RSTAT       24
#define bfwCAN_RCTRL_RSTAT       2
// 1 bit blank

#define bfoCAN_RCTRL_RBALL       27
#define bfwCAN_RCTRL_RBALL       1
#define bfoCAN_RCTRL_RREL        28
#define bfwCAN_RCTRL_RREL        1
#define bfoCAN_RCTRL_ROV         29
#define bfwCAN_RCTRL_ROV         1
#define bfoCAN_RCTRL_ROM         30
#define bfwCAN_RCTRL_ROM         1
#define bfoCAN_RCTRL_SACK        31
#define bfwCAN_RCTRL_SACK        1

// reg_grp2
// RTIE register
#define bfoCAN_RTIE_TSFF         0
#define bfwCAN_RTIE_TSFF         1
#define bfoCAN_RTIE_EIE          1
#define bfwCAN_RTIE_EIE          1
#define bfoCAN_RTIE_TSIE         2
#define bfwCAN_RTIE_TSIE         1
#define bfoCAN_RTIE_TPIE         3
#define bfwCAN_RTIE_TPIE         1
#define bfoCAN_RTIE_RAFIE        4
#define bfwCAN_RTIE_RAFIE        1
#define bfoCAN_RTIE_RFIE         5
#define bfwCAN_RTIE_RFIE         1
#define bfoCAN_RTIE_ROIE         6
#define bfwCAN_RTIE_ROIE         1
#define bfoCAN_RTIE_RIE          7
#define bfwCAN_RTIE_RIE          1
// RTIF register
#define bfoCAN_RTIF_AIF          8
#define bfwCAN_RTIF_AIF          1
#define bfoCAN_RTIF_EIF          9
#define bfwCAN_RTIF_EIF          1
#define bfoCAN_RTIF_TSIF         10
#define bfwCAN_RTIF_TSIF         1
#define bfoCAN_RTIF_TPIF         11
#define bfwCAN_RTIF_TPIF         1
#define bfoCAN_RTIF_RAFIF        12
#define bfwCAN_RTIF_RAFIF        1
#define bfoCAN_RTIF_RFIF         13
#define bfwCAN_RTIF_RFIF         1
#define bfoCAN_RTIF_ROIF         14
#define bfwCAN_RTIF_ROIF         1
#define bfoCAN_RTIF_RIF          15
#define bfwCAN_RTIF_RIF          1
// ERRINT register
#define bfoCAN_ERRINT_BEIF       16
#define bfwCAN_ERRINT_BEIF       1
#define bfoCAN_ERRINT_BEIE       17
#define bfwCAN_ERRINT_BEIE       1
#define bfoCAN_ERRINT_ALIF       18
#define bfwCAN_ERRINT_ALIF       1
#define bfoCAN_ERRINT_ALIE       19
#define bfwCAN_ERRINT_ALIE       1
#define bfoCAN_ERRINT_EPIF       20
#define bfwCAN_ERRINT_EPIF       1
#define bfoCAN_ERRINT_EPIE       21
#define bfwCAN_ERRINT_EPIE       1
#define bfoCAN_ERRINT_EPASS      22
#define bfwCAN_ERRINT_EPASS      1
#define bfoCAN_ERRINT_EWARN      23
#define bfwCAN_ERRINT_EWARN      1
// LIMIT register
#define bfoCAN_LIMIT_EWL         24
#define bfwCAN_LIMIT_EWL         4
#define bfoCAN_LIMIT_AFWL        28
#define bfwCAN_LIMIT_AFWL        4

// reg_grp3
// S_Seg_1 register
#define bfoCAN_S_Seg_1           0
#define bfwCAN_S_Seg_1           8
// S_Seg_2 register
#define bfoCAN_S_Seg_2           8
#define bfwCAN_S_Seg_2           7
// S_SJW register
#define bfoCAN_S_SJW             16
#define bfwCAN_S_SJW             7
// S_PRESC register
#define bfoCAN_S_PRESC           24
#define bfwCAN_S_PRESC           8

// reg_grp4
// F_Seg_1 register
#define bfoCAN_F_Seg_1           0
#define bfwCAN_F_Seg_1           5
// F_Seg_2 register
#define bfoCAN_F_Seg_2           8
#define bfwCAN_F_Seg_2           4
// F_SJW register
#define bfoCAN_F_SJW             16
#define bfwCAN_F_SJW             4
// F_PRESC register
#define bfoCAN_F_PRESC           24
#define bfwCAN_F_PRESC           8

// reg_grp5
// EALCAP register
#define bfoCAN_EALCAP_ALC        0
#define bfwCAN_EALCAP_ALC        5
#define bfoCAN_EALCAP_KOER       5
#define bfwCAN_EALCAP_KOER       3
// TDC register
#define bfoCAN_TDC_SSPOFF        8
#define bfwCAN_TDC_SSPOFF        7
#define bfoCAN_TDC_TDCEN         15
#define bfwCAN_TDC_TDCEN         1
// RECNT register
#define bfoCAN_RECNT             16
#define bfwCAN_RECNT             8
// TECNT register
#define bfoCAN_TECNT             24
#define bfwCAN_TECNT             8

// reg_grp6
// ACFCTRL register
#define bfoCAN_ACFCTRL_ACFADR    0
#define bfwCAN_ACFCTRL_ACFADR    4
// 1 bit blank

#define bfoCAN_ACFCTRL_SELMASK   5
#define bfwCAN_ACFCTRL_SELMASK   1
// 1 bit blank

// TIMECFG register
#define bfoCAN_TIMECFG_TIMEEN    8
#define bfwCAN_TIMECFG_TIMEEN    1
#define bfoCAN_TIMECFG_TIMEPOS   9
#define bfwCAN_TIMECFG_TIMEPOS   1
// 6 bit blank

// ACF_EN_0 register
#define bfoCAN_ACF_EN_0          16
#define bfwCAN_ACF_EN_0          8
// ACF_EN_1 register
#define bfoCAN_ACF_EN_1          24
#define bfwCAN_ACF_EN_1          8

// ACF0~3
// ACF_0 register
#define bfoCAN_ACF_0             0
#define bfwCAN_ACF_0             8
// ACF_1 register
#define bfoCAN_ACF_1             8
#define bfwCAN_ACF_1             8
// ACF_2 register
#define bfoCAN_ACF_2             16
#define bfwCAN_ACF_2             8
// ACF_3 register
#define bfoCAN_ACF_3_COM         24  // acode or amask
#define bfwCAN_ACF_3_COM         5
#define bfoCAN_ACF_3_AIDE        29
#define bfwCAN_ACF_3_AIDE        1
#define bfoCAN_ACF_3_AIDEE       30
#define bfwCAN_ACF_3_AIDEE       1
// 1 bit blank

// reg_grp7
// VER_0 register
#define bfoCAN_VER_0             0
#define bfwCAN_VER_0             8
// VER_1 register
#define bfoCAN_VER_1             8
#define bfwCAN_VER_1             8
// TBSLOT register
#define bfoCAN_TBSLOT_TBPTR      16
#define bfwCAN_TBSLOT_TBPTR      6
#define bfoCAN_TBSLOT_TBF        22
#define bfwCAN_TBSLOT_TBF        1
#define bfoCAN_TBSLOT_TBE        23
#define bfwCAN_TBSLOT_TBE        1
// TTCFG register
#define bfoCAN_TTCFG_TTEN        24
#define bfwCAN_TTCFG_TTEN        1
#define bfoCAN_TTCFG_T_PRESC     25
#define bfwCAN_TTCFG_T_PRESC     2
#define bfoCAN_TTCFG_TTIF        27
#define bfwCAN_TTCFG_TTIF        1
#define bfoCAN_TTCFG_TTIE        28
#define bfwCAN_TTCFG_TTIE        1
#define bfoCAN_TTCFG_TEIF        29
#define bfwCAN_TTCFG_TEIF        1
#define bfoCAN_TTCFG_WTIF        30
#define bfwCAN_TTCFG_WTIF        1
#define bfoCAN_TTCFG_WTIE        31
#define bfwCAN_TTCFG_WTIE        1

// REF_MSG0~3
// REF_MSG0 register
// #define bfoCAN_REF_MSG_0 0
// #define bfwCAN_REF_MSG_0 8
// REF_MSG1 register
// #define bfoCAN_REF_MSG_1 8
// #define bfwCAN_REF_MSG_1 8
// REF_MSG2 register
// #define bfoCAN_REF_MSG_2 16
// #define bfwCAN_REF_MSG_2 8
// REF_MSG3 register
// REF_MSG register
#define bfoCAN_REF_MSG_REF_ID    0
#define bfwCAN_REF_MSG_REF_ID    29
// 2 bit blank

#define bfoCAN_REF_MSG_3_REF_IDE 31
#define bfwCAN_REF_MSG_3_REF_IDE 1

// reg_grp8
// IRIG_CFG_0 register
#define bfoCAN_TRIG_CFG_0_TTPTR  0
#define bfwCAN_TRIG_CFG_0_TTPTR  6
// 2 bit blank

// IRIG_CFG_0 register
#define bfoCAN_TRIG_CFG_1_TTYPE  8
#define bfwCAN_TRIG_CFG_1_TTYPE  3
// 1 bit blank

#define bfoCAN_TRIG_CFG_1_TEW    12
#define bfwCAN_TRIG_CFG_1_TEW    4
// TT_TRIG_0 register
// #define bfoCAN_TT_TRIG_0 16
// #define bfwCAN_TT_TRIG_0 8
// TT_TRIG_1 register
// #define bfoCAN_TT_TRIG_1 24
// #define bfwCAN_TT_TRIG_1 8
// TT_TRIG register
#define bfoCAN_TT_TRIG           16
#define bfwCAN_TT_TRIG           16

// TT_WTRIG
// TT_WTRIG_0 register
#define bfoCAN_TT_WTRIG_0        0
#define bfwCAN_TT_WTRIG_0        8
// TT_WTRIG_1 register
#define bfoCAN_TT_WTRIG_1        8
#define bfwCAN_TT_WTRIG_1        8
// 16 bit blank

/**************************** Type Definitions *******************************/

/**
 * DESCRIPTION
 *  This is the structure used for accessing the gpio memory map.
 */
typedef struct FCanPs_Portmap {
    volatile u32 rbuf[18];     //(0x00~0x47)
    volatile u32 rts[2];       //(0x48~0x4f)
    volatile u32 tbuf[18];     //(0x50~0x97)
    volatile u32 tts[2];       //(0x98~0x9f)
    volatile u32 reg_grp1[1];  // 0xa0:CFG_STAT 0xa1:TCMD 0xa2:TCTRL 0xa3:RCTRL
    volatile u32 reg_grp2[1];  // 0xa4:RTIE	0xa5:RTIF 0xa6:ERRINT 0xa7:LIMIT
    volatile u32 reg_grp3[1];  // 0xa8:S_Seg_1 0xa9:S_Seg_2 0xaa:S_SJW
                               // 0xab:S_PRESC
    volatile u32 reg_grp4[1];  // 0xac:F_Seg_1 0xad:S_Seg_2 0xae:F_SJW
                               // 0xaf:F_PRESC
    volatile u32 reg_grp5[1];  // 0xb0:EALCAP 0xb1:TDC 0xb2:RECNT 0xb3:TECNT
    volatile u32 reg_grp6[1];  // 0xb4:ACFCTRL 0xb5:TIMECFG 0xb6:ACF_EN_0
                               // 0xb7:ACF_EN_1
    volatile u32 acf[1];       // 0xb8~0xbb
    volatile u32 reg_grp7[1];  // 0xbc~0xbd:VERSION 0xbe:TBSLOT 0xbf:TTCFG
    volatile u32 ref_msg[1];   // 0xc0~0xc3
    volatile u32 reg_grp8[1];  // 0xc4~0xc5:TRIG_CFG 0xc6~0xc7:TT_TRIG
    volatile u32 tt_wtrig[1];  // 0xc8~0xc9:tt_wtrig 0xca~0xcb:reserved

} FCanPs_Portmap_T;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static int mcan_set_bittiming(FCanPs_T *dev);
static int can_calc_bittiming(FCanPs_T *dev, struct can_bittiming *bt,
                              const struct can_bittiming_const *btc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
