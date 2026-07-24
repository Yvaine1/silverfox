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
 * uasp.h
 * uasp device header file
 *
 *****************************************************************************/

#ifndef _UASP_H
#define _UASP_H

#include "scsi.h"                    // SCSI layer

//--- UASP IU ID---------------------------------------------------------
#define COMMAND_IU  0x01
#define SENSE_IU 0x03
#define RESPONSE_IU 0x04
#define TASK_MANAGEMENT_IU 0x05
#define READ_READY_IU 0x06
#define WRITE_READY_IU 0x07

//---Response code field----------------------------------------------
#define TASK_MANAGEMENT_FUNCTION_COMPLETE 0x00
#define INVALID_INFORMATION_UNIT 0x02
#define TASK_MANAGEMENT_FUNCTION_NOT_SUPPORTED 0x04
#define TASK_MANAGEMENT_FUNCTION_FAILED 0x05
#define TASK_MANAGEMENT_FUNCTION_SUCCEEDED 0x08
#define INCORRECT_LOGICAL_UNIT_NUMBER 0x09
#define OVERLAPPED_TAG_ATTEMPTED 0x0A

/*
 *  SCSI Architecture Model (SAM) Status codes. Taken from SAM-3 draft
 *  T10/1561-D Revision 4 Draft dated 7th November 2002.
 */
#define SAM_STAT_GOOD            0x00
#define SAM_STAT_CHECK_CONDITION 0x02
#define SAM_STAT_CONDITION_MET   0x04
#define SAM_STAT_BUSY            0x08
#define SAM_STAT_INTERMEDIATE    0x10
#define SAM_STAT_INTERMEDIATE_CONDITION_MET 0x14
#define SAM_STAT_RESERVATION_CONFLICT 0x18
#define SAM_STAT_COMMAND_TERMINATED 0x22	/* obsolete in SAM-3 */
#define SAM_STAT_TASK_SET_FULL   0x28
#define SAM_STAT_ACA_ACTIVE      0x30
#define SAM_STAT_TASK_ABORTED    0x40

//---Task management function------------------------------------
#define ABORT_TASK 0x01
#define ABORT_TASK_SET 0x02
#define CLEAR_TASK_SET  0x04
#define LOGICAL_UNIT_RESET 0x08
#define I_T_NEXUS_RESET 0x10
#define CLEAR_ACA 0x40
#define QUERY_TASK 0x80
#define QUERY_TASK_SET 0x81
#define QUERY_ASYNCHRONOUS_EVENT 0x82

#define NUM_STREAMS_LG2    3 /* => num_streams = 2^3 = 8*/
#define CMD_QUE_DEEP 16 /* this means that 1 <= SID <= 8 */

typedef struct __attribute__((__packed__)) {
    uint8_t iu_id;
    uint8_t reserved_0;
    uint16_t tag;
}
iu_header_struct;

typedef struct __attribute__((__packed__)) {
    iu_header_struct iu_header;
    uint8_t priority_attribute;
    uint8_t reserved_0;
    uint8_t additonal_cdb_len;
    uint8_t reserved_1;
    uint8_t logical_unit_number[8];
    uint8_t cdb[16];
}
command_iu_struct;

typedef struct __attribute__((__packed__)) {
    iu_header_struct iu_header;
    uint16_t status_qualifier;
    uint8_t status;
    uint8_t reserved[7];
    uint16_t length;
    uint8_t sense_data[18];
}
sense_iu_struct;

typedef struct __attribute__((__packed__)) {
    iu_header_struct iu_header;
    uint16_t additional_resp_info;
    uint16_t response_code;
}
response_iu_struct;

typedef struct __attribute__((__packed__)) {
    iu_header_struct iu_header;
    uint8_t task_management_function;
    uint8_t reserved_0;
    uint16_t tag_task_to_managed;
    uint16_t logical_unit_number;
}
task_management_iu_struct;

typedef enum {
    UASP_CMD_STATE_INIT = 0,
    UASP_CMD_STATE_ACTIVE = 1
} UASP_CMD_STATE_T;

typedef enum {
    SCSI_OBJ_STATE_INACTIVE = 0,
    SCSI_OBJ_STATE_COMMAND = 1,
    SCSI_OBJ_STATE_DATA = 2,
    SCSI_OBJ_STATE_STATUS = 3
} SCSI_OBJ_STATE_T;

typedef struct {
    sense_iu_struct *hw_buff;
    CUSBD_Req commandReq; // command request is not per SID
    CUSBD_Req reqIn; // data xfer in request for SID
    CUSBD_Req reqOut; // data xfer in request for SID    
    CUSBD_Req status;
    CUSBD_Req response;    
    uint16_t sid;
    SCSI_OBJ_STATE_T eState;
} uasp_transfer_t;

// Access this array using (SID - 1)
extern uasp_transfer_t uasp_command_container[CMD_QUE_DEEP];

#endif /* _UASP_H */