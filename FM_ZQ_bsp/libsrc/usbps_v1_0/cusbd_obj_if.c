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
 * Layer interface for the Cadence USB device controller's family
 **********************************************************************/
#include <string.h>
#include "fmsh_common.h"
#include "cusbd_obj_if.h"
#include "fmsh_cache.h"
#include "fmsh_usb_data.h"

/* parasoft suppress item METRICS-41-3 "Number of blocks of comments per
 * statement" */
uint32_t CPS_ReadReg32 (volatile uint32_t* address) { return *address; }
void CPS_WriteReg32 (volatile uint32_t* address, uint32_t value)
{
    *address = value;
}
uint8_t CPS_UncachedRead8 (volatile uint8_t* address) { return *address; }
uint16_t CPS_UncachedRead16 (volatile uint16_t* address) { return *address; }
uint32_t CPS_UncachedRead32 (volatile uint32_t* address) { return *address; }
void CPS_UncachedWrite8 (volatile uint8_t* address, uint8_t value)
{
    *address = value;
}
void CPS_UncachedWrite16 (volatile uint16_t* address, uint16_t value)
{
    *address = value;
}
void CPS_UncachedWrite32 (volatile uint32_t* address, uint32_t value)
{
    *address = value;
}
void CPS_UncachedWrite64 (volatile uint64_t* address, uint64_t value)
{
    *address = value;
}
uint64_t CPS_UncachedRead64 (volatile uint64_t* address) { return *address; }
void CPS_BufferCopy (volatile uint8_t* dst, volatile const uint8_t* src,
                     uint32_t size)
{
    (void)memcpy((void*)dst, (void*)src, size);
}
void CPS_CacheInvalidate (void* address, size_t size, uintptr_t devInfo)
{
#if (DCACHE_ENABLE == 1) 
    Fmsh_DCacheInvalidateRange((uintptr_t)address, size);
#endif
}
void CPS_CacheFlush (void* address, size_t size, uintptr_t devInfo)
{
#ifdef CACHE_USB_DATA    
    Fmsh_DCacheInvalidateRange((uintptr_t)address, size);
    // Fmsh_DCacheFlush();
#endif
}
/*

CUSBD_OBJ *CUSBD_GetInstance(void)
{
   static CUSBD_OBJ driver =
   {
       .probe = CUSBD_Probe,
       .init = CUSBD_Init,
       .destroy = CUSBD_Destroy,
       .start = CUSBD_Start,
       .stop = CUSBD_Stop,
       .isr = CUSBD_Isr,
       .epEnable = CUSBD_EpEnable,
       .epDisable = CUSBD_EpDisable,
       .epSetHalt = CUSBD_EpSetHalt,
       .epSetWedge = CUSBD_EpSetWedge,
       .epFifoStatus = CUSBD_EpFifoStatus,
       .epFifoFlush = CUSBD_EpFifoFlush,
       .reqQueue = CUSBD_ReqQueue,
       .reqDequeue = CUSBD_ReqDequeue,
       .getDevInstance = CUSBD_GetDevInstance,
       .dGetFrame = CUSBD_DGetFrame,
       .dSetSelfpowered = CUSBD_DSetSelfpowered,
       .dClearSelfpowered = CUSBD_DClearSelfpowered,
       .dGetConfigParams = CUSBD_DGetConfigParams,
   };

   return &driver;
}
*/
