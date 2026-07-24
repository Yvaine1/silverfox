/******************************************************************************
 *
 * Copyright (C) 2014-2021 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 *
 ******************************************************************************
 * bot.h
 * BOT protocol header file
 *
 *****************************************************************************/
#ifndef BOT_H
#define BOT_H

#include "cdn_stdint.h"
#include "msc_config.h"

//-------------------------------------------------------------------------------------------
// Definitions
//-------------------------------------------------------------------------------------------
#define CBW_FLAG_DIR_MASK 0x80
#define CBW_FLAG_DIR_IN 0x80 // (to host)
#define CBW_FLAG_DIR_OUT 0x00 // (from host)

#define CBW_LUN_MASK 0x0F
#define CBW_LENGTH_MASK 0X1F

#define CSW_STRUCT_SZ 13

#define CBW_SIGNATURE 0x43425355
#define CSW_SIGNATURE 0x53425355
//-------------------------------------------------------------------------------------------
// Types definitions
//-------------------------------------------------------------------------------------------

typedef struct {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t bmCBWFlags;
    uint8_t bmCBWLun;
    uint8_t bmCBWLength;
    uint8_t cbwcb[16];
} __attribute__((packed)) cbw_t;

typedef struct {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t dCSWStatus;
} __attribute__((packed)) csw_t;

typedef enum {
    BOT_UNINIT_STATE = 0U,
    BOT_CBW_STATE = 1U,
    BOT_DATAIN_STATE = 2U,
    BOT_DATAOUT_STATE = 3U,
    BOT_CSW_STATE = 4U,
    BOT_ERROR_STATE = 5U
} bot_app_state_t;

#endif // BOT_H


