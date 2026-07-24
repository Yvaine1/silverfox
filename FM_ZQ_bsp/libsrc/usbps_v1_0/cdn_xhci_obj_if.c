/**********************************************************************
 * Copyright (C) 2014-2021 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 **********************************************************************
 * WARNING: This file is auto-generated using api-generator utility.
 *          api-generator: 12.02.13bb8d5
 *          Do not edit it manually.
 **********************************************************************
 * XHCI driver for both host and device mode header file
 **********************************************************************/

#include "cdn_xhci_obj_if.h"

/* parasoft suppress item METRICS-41-3 "Number of blocks of comments per
 * statement" */

USBSSP_OBJ *USBSSP_GetInstance (void)
{
    static USBSSP_OBJ driver = {
        .transferData = USBSSP_TransferData,
        .transferData2 = USBSSP_TransferData2,
        .stopEndpoint = USBSSP_StopEndpoint,
        .resetEndpoint = USBSSP_ResetEndpoint,
        .resetDevice = USBSSP_ResetDevice,
        .isr = USBSSP_Isr,
        .SetMemRes = USBSSP_SetMemRes,
        .init = USBSSP_Init,
        .getDescriptor = USBSSP_GetDescriptor,
        .forceEvent = USBSSP_ForceEvent,
        .setAddress = USBSSP_SetAddress,
        .resetRootHubPort = USBSSP_ResetRootHubPort,
        .issueGenericCommand = USBSSP_IssueGenericCommand,
        .endpointSetFeature = USBSSP_EndpointSetFeature,
        .setConfiguration = USBSSP_SetConfiguration,
        .controlTransfer = USBSSP_ControlTransfer,
        .nBControlTransfer = USBSSP_NBControlTransfer,
        .controlTransferDev = USBSSP_ControlTransferDev,
        .noOpTest = USBSSP_NoOpTest,
        .calcFsLsEPIntrptInterval = USBSSP_CalcFsLsEPIntrptInterval,
        .enableSlot = USBSSP_EnableSlot,
        .disableSlot = USBSSP_DisableSlot,
        .enableEndpoint = USBSSP_EnableEndpoint,
        .disableEndpoint = USBSSP_DisableEndpoint,
        .GetEpState = USBSSP_GetEpState,
        .getMicroFrameIndex = USBSSP_GetMicroFrameIndex,
        .setEndpointExtraFlag = USBSSP_SetEndpointExtraFlag,
        .cleanEndpointExtraFlag = USBSSP_CleanEndpointExtraFlag,
        .getEndpointExtraFlag = USBSSP_GetEndpointExtraFlag,
        .setFrameID = USBSSP_SetFrameID,
        .addEventDataTRB = USBSSP_AddEventDataTRB,
        .forceHeader = USBSSP_ForceHeader,
        .setPortControlReg = USBSSP_SetPortControlReg,
        .getPortControlReg = USBSSP_GetPortControlReg,
        .EnableDDUSB = USBSSP_EnableDDUSB,
        .DisableDDUSB = USBSSP_DisableDDUSB,
        .SaveState = USBSSP_SaveState,
        .RestoreState = USBSSP_RestoreState,
    };

    return &driver;
}
