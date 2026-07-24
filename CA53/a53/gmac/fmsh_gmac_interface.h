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
//#pragma once

#ifndef FMSH_GMAC_INTERFACE_H
#define FMSH_GMAC_INTERFACE_H

#include "cc.h"
#include "fmsh_gmac.h"
#include "fmsh_psu_parameters.h"
#include "fmsh_gmac_bd.h"
#include "sys_arch.h"
#include "fmsh_gmac_mutex.h"
/**************************** test item define *******************************/

// example select

#define GMAC0_TEST_EXAMPLE               0
#define GMAC2_TEST_EXAMPLE               0

// test times
#define FMSH_GMAC_REG_TEST_NUM              16
#define FMSH_GMAC_MDIO_TEST_NUM             8

/*************************** test related define *****************************/

// whether reset gmac in err handler or not
#define GMAC_DEBUG_RESET_ON_ERR             0
// debug print
#define GMAC_DEBUG_OUT                      1
  
#define EthernetFrameSize                   0x680  // 1664
#define ETH_MAX_FRAME_SIZE_FOR_APP          1620   // mmj_hdr:6, eth_hdr:14, ip_mtu:1600
typedef char EthernetFrame[EthernetFrameSize];

        
/***************** Macros (Inline Functions) Definitions *********************/
extern SemaphoreHandle_t  printf_mutex;
#define GMAC_TRACE_OUT(flag, ...) \
    do {                           \
        if (flag){                  \
          if (xSemaphoreTake(printf_mutex,portMAX_DELAY) == pdTRUE){\
          fmsh_print(__VA_ARGS__);  \
            xSemaphoreGive(printf_mutex);}\
          }                         \
    } while (0)

#define GMAC_ISR_TRACE_OUT(flag,...) \
    do {                           \
        if (flag){                  \
          if (gmac_mutex_Isr_lock(&printf_mutex) == pdTRUE){\
          fmsh_print(__VA_ARGS__);   \
            gmac_mutex_Isr_unlock(&printf_mutex);}\
          }                         \
    } while (0)
            
/************************* functions Definitions *****************************/

int fmsh_gmac_initial_config(u8 GMAC_ID, const u8* mac_addr);

#if GMAC0_TEST_EXAMPLE
    int fmsh_gmac_0_tx_send_example();
    void gmac_0_prv_test(void *pvParameters);
#endif

#if GMAC2_TEST_EXAMPLE
    void gmac_2_prv_test(void *pvParameters);
#endif

#endif
