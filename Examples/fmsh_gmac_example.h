/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_verify.h
 *
 * gmac driver
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 1_0   Danyang Wang  6/25/2023  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_gmac.h"
#include "fmsh_psu_parameters.h"

/**************************** test item define *******************************/

// example select

#define GMAC_DEBUG_ITEM_PHYLOOP_EXAMPLE     1
#define GMAC_DEBUG_ITEM_GTR_PHYLOOP_EXAMPLE 0

// test times
#define FMSH_GMAC_REG_TEST_NUM              16
#define FMSH_GMAC_MDIO_TEST_NUM             8

/*************************** test related define *****************************/
// gmac test loop times
#define GMAC_DEBUG_TIMES                    10
// whether reset gmac in err handler or not
#define GMAC_DEBUG_RESET_ON_ERR             0
// GMAC baseaddr and device id set
#define GMAC_SELECT_BASEADDR                0xFF0B0000U
#define GMAC_SELECT_ID                      FPAR_GMACPS_0_DEVICE_ID
// debug print
#define GMAC_DEBUG_OUT                      1

/*************************** frame related const *****************************/

#define EthernetFrameSize                   FGMACPS_MAX_VLAN_FRAME_SIZE_JUMBO
typedef char EthernetFrame[EthernetFrameSize];

/***************** Macros (Inline Functions) Definitions *********************/
#define GMAC_TRACE_OUT(flag, ...) \
    do                            \
    {                             \
        if (flag)                 \
        {                         \
            printf(__VA_ARGS__);  \
        }                         \
    } while (0)

/*********************** Return Code Definitions ****************************/

/************************** lib function define ******************************/

/****************************************************************************/
/**
 *
 * @brief        Perform a 16-bit endian converion.
 *
 * @param	Data: 16 bit value to be converted.
 *
 * @return	16 bit Data with converted endianess.
 *
 *****************************************************************************/
int FGmacPsEndianSwap16 (u16 Data)
{
    return (u16)(((Data & 0xFF00U) >> 8U) | ((Data & 0x00FFU) << 8U));
}

/************************* functions Definitions *****************************/

int FGmacpsu_example();
