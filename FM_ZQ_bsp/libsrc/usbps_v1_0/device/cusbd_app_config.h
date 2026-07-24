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
 * cusbd_app_config.h
 * test bulk transfer mode
 *
 *
 *****************************************************************************/

#ifndef CUSBD_APP_CONFIG_H
#define CUSBD_APP_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include "cdn_xhci_if.h"
#include "cdn_xhci_structs_if.h"

#define TEST_INDEX 2

#define DATA_XFER_BUFFER_COUNT 4

typedef void (*CUSBDSS_LoopbackApp_CfgCb)(uint32_t state);
typedef void (*CUSBDSS_LoopbackApp_InXferCb)(uint8_t *buffer, uint32_t size);
typedef void (*CUSBDSS_LoopbackApp_OutXferCb)(uint8_t *buffer, uint32_t size);

typedef struct {
    uint8_t forcedUsbMode;
    CUSBDSS_LoopbackApp_CfgCb cfgCb;
    CUSBDSS_LoopbackApp_InXferCb inXferCb;
    CUSBDSS_LoopbackApp_OutXferCb outXferCb;

    /* buffer used by the device to get setup packet from host */
    uint8_t *ep0RespBuffer;
    uintptr_t ep0RespBufferPhyAddr;

    /* EP0 buffer for sending descriptors */
    uint8_t *ep0Buffer;
    uintptr_t ep0BufferPhyAddr;

    USBSSP_DriverConfigT driverConfig;
    
    /***Loopback app specific config */
    /* Data transfer buffer */
    uint8_t *dataXferBuffer[DATA_XFER_BUFFER_COUNT];
    uintptr_t dataXferBufferPhyAddr[DATA_XFER_BUFFER_COUNT];
    uint32_t dataXferBufferSize;


    /*** BOT app specific config */
    uint8_t *mscCmdBuffer;
    uintptr_t mscCmdBufferPhyAddr;
    uint32_t mscCmdBufferSize;
    
    uint8_t *mscRespBuffer;
    uintptr_t mscRespBufferPhyAddr;
    uint32_t mscRespBufferSize;

    uint8_t *scsiRespBuffer;
    uintptr_t scsiRespBufferPhyAddr;
    uint32_t scsiRespBufferSize;
    
    /* data buffer */
    uint8_t *dataBuffer;
    uintptr_t dataBufferPhyAddr;
    uint32_t dataBufferSize;

    /*Hid buffer */
    uint8_t *hidBuffer;
    uintptr_t hidBufferPhyAddr;

    /* hid report buffer*/
    uint8_t *hidReportBuffer;
    uintptr_t hidReportBufferPhyAddr;
    

} CUSBD_App_Config;

#if (TEST_INDEX == 1)
void bulk_LBAppIsr(void* deviceID);
int bulk_LBAppInit(CUSBD_App_Config *appConfig);
void bulk_LBAppStop();

#elif (TEST_INDEX == 2)
void bot_app_Isr(void* deviceID);
int bot_app_Init();
void bot_app_Stop();

#elif (TEST_INDEX == 3)
void uasp_app_Isr(void* deviceID);
int uasp_app_Init(CUSBD_App_Config *appConfig);
void uasp_app_Stop();

#elif (TEST_INDEX == 4)
void HIDAppIsr(void* deviceID);
int HIDAppInit(CUSBD_App_Config  *appConfig);

#else
#error "TEST_INDEX not defined"
#endif

#endif
