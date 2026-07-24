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
 * scsi.h
 * SCSI parser header file
 *
 *****************************************************************************/
#ifndef SCSI_H
#define SCSI_H

#include "cdn_stdint.h"


// SCSI Errors
#define ERR_SCSI_OK              0
#define ERR_SCSI_UNKNOWN_COMMAND 5
#define ERR_SCSI_DAMAGED_COMMAND 6
#define ERR_SCSI_INVALID_DATA_SIZE 7

// USB receiver/sender
extern uint32_t(*scsi_send_data)(void *, uint32_t, uint16_t);
extern uint32_t(*scsi_rec_data)(void *, uint32_t, uint16_t);

// keeps number of bytes required by device to transfer
extern uint32_t dev_num_of_bytes;

// keeps direction of data required to transfer: 1-IN, 0-OUT
extern uint8_t dev_data_dir;
//-------------------------------------------------------------------------------------------

// initializes SCSI device
void scsiInit(uint8_t* scsi_resp_buffer);

// executes SCSI command
uint16_t scsiExecCmd(uint8_t *Cmd, uint32_t *retBytes, uint16_t sid);

typedef struct __attribute__((__packed__)) {
    uint8_t *cmd_buff; // IN parameter points to SCSI command buffer
    uint8_t **allocated_buff; // IN/OUT parameter
    uint32_t transfer_length; // OUT parameter
    uint8_t data_direction; // OUT parameter
    uint8_t storage_active_flag; // OUT
    storage_param_t storage;
}
scsi_param_t;



//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
#endif // SCSI_H
//-------------------------------------------------------------------------------------------


