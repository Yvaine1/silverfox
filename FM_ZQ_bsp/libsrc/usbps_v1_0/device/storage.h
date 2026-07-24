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
 * storage.h
 * storage device header file
 *
 *****************************************************************************/

#ifndef Storage_H
#define Storage_H

#include "cdn_stdint.h"

#define SECTOR_SIZE 512
#define SECTOR_COUNT (1024 * 2 * 16)
#define LUN_COUNT 1

// All errors
#define ERR_NO_ERRORS 0

// Device specific errors
#define ERR_INVALID_LUN 1
#define ERR_INVALID_SECTOR 2
#define ERR_NOT_READY 3
#define ERR_VERIFY_ERROR 4

// USB receiver/sender
extern uint32_t(*storage_send_data)(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid);
extern uint32_t(*storage_rec_data)(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid);

//extern uint8_t *MemBuffer;
//------------------------------------------------------------------------------
// Driver interface
//------------------------------------------------------------------------------
uint16_t devInit(void);
uint16_t devDeinit(void);
uint16_t devPowerUp(void);
uint16_t devPowerDown(void);
uint16_t devGetMaxLun(uint8_t *MaxLUN);
uint16_t devGetCapacities(uint8_t LUN, uint32_t *SectorCount,
        uint16_t *BytesPerSector);
uint16_t devReadSector(uint8_t LUN, uint32_t StartSector, uint32_t SectorCount,
        uint32_t * BytesRead, uint16_t sid);
uint16_t devWriteSector(uint8_t LUN, uint32_t StartSector,
        uint32_t SectorCount, uint32_t * BytesWritten, uint16_t sid);
uint16_t devVerifySector(uint8_t LUN, uint32_t StartSector,
        uint32_t SectorCount);
uint16_t devFormatUnit(uint8_t LUN, uint16_t Interleave);
uint16_t devTestUnitReady(uint8_t LUN);

typedef struct __attribute__((__packed__)) {
    uint8_t **allocated_buff; // out parameter
    uint8_t lun;
    uint32_t start_sector;
    uint32_t sector_count;
}
storage_param_t;


//------------------------------------------------------------------------------
#endif // Storage_H
//------------------------------------------------------------------------------

