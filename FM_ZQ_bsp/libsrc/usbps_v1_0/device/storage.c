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
 * storage.c
 * storage device driver
 *
 *****************************************************************************/
#include <string.h>
#include "storage.h"

uint8_t *StorageBase = NULL;
uintptr_t StorageBasePhyAddr = 0;


//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------

uint32_t(*storage_send_data)(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid);
uint32_t(*storage_rec_data)(uint32_t start_sec, uint32_t num_of_sec, uint16_t sid);

static uint8_t initialized = 0;

//------------------------------------------------------------------------------
// Device initialization
//------------------------------------------------------------------------------

uint16_t devInit(void)  {

    initialized = 1;

    // No errors at initialization process
    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Device deinitialization
//------------------------------------------------------------------------------

uint16_t devDeinit(void)  {
    initialized = 0;

    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Resuming a device
//------------------------------------------------------------------------------

uint16_t devPowerUp(void)  {
    // No errors at suspend process
    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Suspending a device
//------------------------------------------------------------------------------

uint16_t devPowerDown(void)  {
    // No errors at resume process
    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Returns a maximum Logical Unit Number for this device
//      MaxLUN - Pointer to variable to obtain a Maximum LUN value
//------------------------------------------------------------------------------

uint16_t devGetMaxLun(uint8_t *maxLun)  {
    // Maximum LUN is a total LUN number - 1
    *maxLun = LUN_COUNT - 1;

    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Returns informations about device capacities for specified LUN
//      LUN - Logical Unit Number
//      SectorCount - Pointer to variable for a total number of sectors
//      BytesPerSector - Pointer to variable for a size of the single sector
//      (in bytes)
//------------------------------------------------------------------------------

uint16_t devGetCapacities(uint8_t lun, uint32_t *sectorCount,
        uint16_t *bytesPerSector)  {
    // Check parameters
    if (lun >= LUN_COUNT)
        return ERR_INVALID_LUN;

    // Device capacities
    *sectorCount = SECTOR_COUNT;
    *bytesPerSector = SECTOR_SIZE;

    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Read a few sectors from the device
//      LUN - Logical Unit Number
//      *Buffer - Pointer to buffer for data
//      StartSector - Starting sector of reading data
//      SectorCount - Number of sectors to read data
//------------------------------------------------------------------------------

uint16_t devReadSector(uint8_t lun, uint32_t startSector, uint32_t sectorCount,
        uint32_t * BytesRead, uint16_t sid)  {

    // Check parameters
    if (lun >= LUN_COUNT)
        return ERR_INVALID_LUN;
    
    *BytesRead += storage_send_data(startSector, sectorCount, sid);

    return ERR_NO_ERRORS;

}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Writes a few sectors to the device
//      LUN - Logical Unit Number
//      *Buffer - Pointer to buffer with data to write
//      StartSector - Starting sector of writing data
//      SectorCount - Number of sectors to write data
//------------------------------------------------------------------------------

uint16_t devWriteSector(uint8_t lun, uint32_t startSector, uint32_t sectorCount,
        uint32_t * bytesWritten, uint16_t sid)  {

    // Check parameters
    if (lun >= LUN_COUNT)
        return ERR_INVALID_LUN;

    if (startSector + sectorCount > SECTOR_COUNT)
        return ERR_INVALID_SECTOR;

    *bytesWritten += storage_rec_data(startSector, sectorCount, sid);

    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Verifies a data written on the device
//      LUN - Logical Unit Number
//      *Buffer - Pointer to buffer with data to compare or NULL for no compare
//      StartSector - Starting sector
//      SectorCount - Number of sectors
//------------------------------------------------------------------------------

uint16_t devVerifySector(uint8_t lun, uint32_t startSector,
        uint32_t sectorCount)  {


    // Check parameters
    if (lun >= LUN_COUNT)
        return ERR_INVALID_LUN;

    if (startSector + sectorCount > SECTOR_COUNT)
        return ERR_INVALID_SECTOR;

    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Formates a coresponding unit
//      LUN - Logical Unit Number
//      Interleave - An Interleave (0 as default)
//------------------------------------------------------------------------------

uint16_t devFormatUnit(uint8_t lun, uint16_t interleave)  {
    interleave = interleave;

    // Check parameters
    if (lun >= LUN_COUNT)
        return ERR_INVALID_LUN;

    return ERR_NO_ERRORS;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Returns a device ready state
//      LUN - Logical Unit Number
//------------------------------------------------------------------------------

uint16_t devTestUnitReady(uint8_t lun)  {
    lun = lun;

    // Return state of the device
    return initialized ? ERR_NO_ERRORS : ERR_NOT_READY;
}
//------------------------------------------------------------------------------

