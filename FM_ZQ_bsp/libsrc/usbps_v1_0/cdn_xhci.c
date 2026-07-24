/******************************************************************************
 *
 * Copyright (C) 2014-2020 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 *
 ******************************************************************************
 * cdn_xhci.c
 * Host/Device Super Speed Plus controller driver,
 *
 * XHCI driver for both host and device mode
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <stdarg.h>

#include "cdn_xhci_if.h"
#include "cdn_xhci_structs_if.h"
#include "cusbd_structs_if.h"
#include "cusb_ch9_if.h"
#include "cusb_ch9_structs_if.h"

#include "trb.h"
#include "cdn_log.h"  // DEBUG macros
#include "cps_drv.h"
#include "usbssp_regs.h"

#include "cdn_xhci_sanity.h"
#include "fmsh_common.h"

#include "fmsh_usb_data.h"
void CPS_MemoryBarrier ()
{
    __asm volatile("dsb sy");  //__asm__ __volatile__("dsb sy": : : "memory")
}

void CPS_DelayUs (u32 delay) { delay_us(delay); }
#if 0
#define QUEUE_TEST
#endif

#if defined USBSSP_DEMO_TB
void SetMemResCallback(USBSSP_DriverResourcesT *res);
#endif

#pragma diag_suppress = Pa039
/* parasoft-begin-suppress MISRA2012-DIR-4_9-4 "Do not define function-like
 * macro, DRV-5628" */
#if (defined XHCI_DISP_TRB_PROC)
extern void XHCI_DISP_TRB_PROC(const USBSSP_RingElementT *trb,
                               const char *prefix_str);
#else
#define XHCI_DISP_TRB_PROC(trb, prefix_str) \
    do                                      \
    {                                       \
    } while (0)
#endif

#if (defined XHCI_DISP_DEV_SETUP_REQ)
extern void XHCI_DISP_DEV_SETUP_REQ(CH9_UsbSetup *setup);
#else
#define XHCI_DISP_DEV_SETUP_REQ(setup) \
    do                                 \
    {                                  \
    } while (0)
#endif
/* parasoft-end-suppress MISRA2012-DIR-4_9-4 */

/**
 * Device mode XHCI Extended config 3xPORT CAP ID
 */
#define USBSSP_D_XEC_CFG_3XPORT_CAP_ID 0xC0

/*
 * These bits are Read Only (RO) and should be saved and written to the
 * registers: 0 (connect status) and  10:13 (port speed).
 * These bits are also sticky - meaning they're in the AUX well and they aren't
 * changed by a hot and warm.
 */
#define USBSSP_DEV_USB23_PORT_RO \
    (USBSSP__PORTSC1USB3__CCS_MASK | USBSSP__PORTSC1USB3__PORTSPEED_MASK)

/*
 * These bits are RW; writing a 0 clears the bit, writing a 1 sets the bit:
 * bits 5:8 (link state), 25:26  ("wake on" enable state)
 */
#define USBSSP_DEV_USB23_PORT_RWS                                    \
    (USBSSP__PORTSC1USB3__PLS_MASK | USBSSP__PORTSC1USB3__WCE_MASK | \
     USBSSP__PORTSC1USB3__WDE_MASK)

#define USBSSP_DEV_PORTSC_CHANGE_BITS                                \
    (USBSSP__PORTSC1USB3__CSC_MASK | USBSSP__PORTSC1USB3__PEC_MASK | \
     USBSSP__PORTSC1USB3__WRC_MASK | USBSSP__PORTSC1USB3__OCC_MASK | \
     USBSSP__PORTSC1USB3__PRC_MASK | USBSSP__PORTSC1USB3__PLC_MASK | \
     USBSSP__PORTSC1USB3__CEC_MASK)

#ifdef USE_VIRTUAL_MEM
/**
 * Convert Virtual pointer to Physical address
 * @param ptr Virtual pointer
 * @return Physical address OR NULL if v-ptr can't be mapped to phy-address
 */
extern uintptr_t CPS_GetPhyAddrOfVPtr(const void *ptr);

/**
 * Convert Physical address to virtual pointer
 * @param phyaddr Physical address
 * @return V-Ptr corresponding to phyaddr, NULL if mapping doesn't exist
 */

extern uint8_t *CPS_GetVPtrFromPhyAddr(uintptr_t phyaddr);
#endif

static void setU1timeout(const USBSSP_DriverResourcesT *res, uint8_t t);
static void setU2timeout(const USBSSP_DriverResourcesT *res, uint8_t t);

#ifdef DEBUG
/*
 * Define global variables used by cdn_log.h for debug logging
 */
uint32_t g_dbg_enable_log = 0x000000010;
uint32_t g_dbg_log_lvl = DBG_INFLOOP;
uint32_t g_dbg_log_cnt;
uint32_t g_dbg_state;

#endif

/*local functions*/
static void setAddress(USBSSP_DriverResourcesT *res, uint8_t bsrVal);
static uint32_t configureEndpoints(USBSSP_DriverResourcesT *res,
                                   const uint8_t *conf, uint16_t length);

/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline USBSSP_EpContexEpState getEndpointStatus (
    const USBSSP_DriverResourcesT *res, uint32_t epIndex)
{
    // returns endpoint state
    USBSSP_EpContexEpState ret;

    // check if enIndex is within correct index range
    if (epIndex < USBSSP_EP0_CONT_OFFSET)
    {
        // display error
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "<%d> Endpoint context Index error < %d\n", res->instanceNo,
                USBSSP_EP0_CONT_OFFSET);
        ret = USBSSP_EP_CONTEXT_EP_STATE_ERROR;
    }
    else
    {
        // keeps integer value returned from endpoint context
        uint32_t hwValue = 0U;

        // get state of default endpoint
        if (epIndex == USBSSP_EP0_CONT_OFFSET)
        {
            hwValue = (le32ToCpu(
                           res->xhciMemRes->outputContext->ep0Context[0]) &
                       (uint32_t)USBSSP_EP_CONTEXT_STATE_MASK);
        }
        else
        {  // get status of no default endpoint
            // calculate index of endpoint in epContext array
            uint32_t epBase = epIndex - USBSSP_EP_CONT_OFFSET;
            // read endpoint state from output context updated by controller
            hwValue = (le32ToCpu(res->xhciMemRes->outputContext
                                     ->epContext[epBase][0]) &
                       (uint32_t)USBSSP_EP_CONTEXT_STATE_MASK);
        }
        // transcode integer values to enumerated values - MISRA requirement
        switch (hwValue)
        {
            // endpoint state disabled
        case 0:
            ret = USBSSP_EP_CONTEXT_EP_STATE_DISABLED;
            break;

            // // endpoint state running
        case 1:
            ret = USBSSP_EP_CONTEXT_EP_STATE_RUNNING;
            break;

            // endpoint state halted
        case 2:
            ret = USBSSP_EP_CONTEXT_EP_STATE_HALTED;
            break;

            // endpoint state stopped
        case 3:
            ret = USBSSP_EP_CONTEXT_EP_STATE_STOPPED;
            break;

            // endpoint state error
        default:
            ret = USBSSP_EP_CONTEXT_EP_STATE_ERROR;
            break;
        }
    }

    return (ret);
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Auxiliary function, returns actual speed field from endpoint context, note
 * that software speed enum values may differ from speed values coded in XHCI
 * spec.
 * @param speed actual speed kept in res->actualSpeed
 * @return speed value of speed given in values as XHCI specification states in
 *                endpoint context structure
 */

static uint32_t getSlotSpeed (CH9_UsbSpeed speed)
{
    uint32_t slotSpeed;

    // translate CH9_UsbSpeed value to integer values according to XHCI spec
    switch (speed)
    {
        // low speed
    case CH9_USB_SPEED_LOW:
        slotSpeed = 2U;
        break;
        // full speed
    case CH9_USB_SPEED_FULL:
        slotSpeed = 1U;
        break;
        // high speed
    case CH9_USB_SPEED_HIGH:
        slotSpeed = 3U;
        break;
        // super speed
    case CH9_USB_SPEED_SUPER:
        slotSpeed = 4U;
        break;
        // super speed plus
    case CH9_USB_SPEED_SUPER_PLUS:
        slotSpeed = 5U;
        break;
    default:
        slotSpeed = 0U;
        break;
    }

    return (slotSpeed);
}

/**
 * Constructs a uint16_t value from uint8_t pointer
 *
 * @param dataPtr Pointer to uint8 buffer
 * @return uint16_t value constructed from uint8 buffer
 */
static inline uint16_t getU16ValFromU8Ptr (const uint8_t *dataPtr)
{
    // Constructs a uint32_t value from uint8_t pointer
#ifdef CPU_BIG_ENDIAN
    uint16_t value = (uint16_t)dataPtr[1];
    uint16_t byte0 = (uint16_t)dataPtr[0] << 8U;

    value += byte0;
    return value;
#else
    uint16_t value = (uint16_t)dataPtr[0];
    uint16_t byte1 = (uint16_t)dataPtr[1] << 8U;

    value += byte1;
    return value;
#endif
}

static inline uint64_t getU64ValFromU32Ptr (const uint32_t *dataPtr)
{
    // Constructs a uint64_t value from uint32_t pointer
#ifdef CPU_BIG_ENDIAN
    uint64_t value = (uint64_t)dataPtr[1];
    uint64_t byte0 = (uint64_t)dataPtr[0] << 32U;

    value += byte0;
    return value;
#else
    uint64_t value = (uint64_t)dataPtr[0];
    uint64_t byte1 = (uint64_t)dataPtr[1] << 32U;

    value += byte1;
    return value;
#endif
}

static uint32_t checkStructAlign (const char *name, uintptr_t startPhysAddr,
                                  size_t byteSize, size_t alignBytes,
                                  size_t pageBytes)
{
    // calculate end of physical address
    uintptr_t endPhysAddr = (startPhysAddr + (uintptr_t)byteSize) -
                            (uintptr_t)1U;
    uint32_t result = 0U;

    if (name != NULL)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "Structure \"%s\", %d bytes @[0x%p...0x%p]\n", name,
                (uint64_t)byteSize, startPhysAddr, endPhysAddr);
    }
    // check alignment violation
    if ((alignBytes > 1U) &&
        ((startPhysAddr & ((uintptr_t)alignBytes - 1U)) != 0U))
    {
        result |= 1U;
    }

    // check page size violation
    if (pageBytes > 1U)
    {
        uintptr_t page_no_bit_mask = ~(uintptr_t)(pageBytes - 1U);

        if ((startPhysAddr & page_no_bit_mask) !=
            (endPhysAddr & page_no_bit_mask))
        {
            result |= 2U;
        }
    }

    // display error codes
    if ((result & 1U) != 0U)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "[ERROR] \"%s\" not aligned to %d bytes\n", name, alignBytes);
    }

    if ((result & 2U) != 0U)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "[ERROR] \"%s\" cannot fit single %d byte page\n", name,
                pageBytes);
    }

    return result;
}

/**
 * Wrapper for 64bit read register function
 * @param address pointer to 64 bit register
 * @return 64bit value read from given address
 */
static inline uint64_t xhciRead64 (volatile uint64_t *address)
{
    uint64_t value;

    value = le64ToCpu(CPS_UncachedRead64(address));
    return (value);
}

/**
 * Wrapper for 64bit write register function
 * @param address pointer to 64 bit register
 * @param value 64bit value to write to given address
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline void xhciWrite64 (volatile uint64_t *address, uint64_t value)
{
    CPS_UncachedWrite64(address, cpuToLe64(value));
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Wrapper for CPS register access function
 * @param pointer to 32bit register
 * @param value vale to written at given pointer
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline void xhciWrite32 (volatile uint32_t *address, uint32_t value)
{
    CPS_REG_WRITE(address, cpuToLe32(value));
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Wrapper for CPS register read function
 * @param address pointer to 32 bit register
 * @return 32bit value at given address
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline uint32_t xhciRead32 (volatile uint32_t *address)
{
    uint32_t value;

    // value = CPS_REG_READ(address);
    value = (uint32_t) * ((uint32_t *)address);
    return (value);
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Function writes DRBL register
 * @param res driver resources
 * @param slotID - TODO obsolete should be removed, slotID is a internal field
 * of SSP resources
 * @param dbValue - DRBL register value
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static void USBSSP_WriteDoorbell (USBSSP_DriverResourcesT *res, uint8_t slotID,
                                  uint32_t dbValue)
{
    xhciEpRingFlush();
    
    CPS_MemoryBarrier();

    xhciWrite32(&res->regs.xhciDoorbell[slotID], dbValue);
}
/* parasoft-end-suppress METRICS-36-3 */

/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
/**
 * Wrapper for DRBL register
 * @param res driver resources
 */
static void hostCmdDoorbell (USBSSP_DriverResourcesT *res)
{
    USBSSP_WriteDoorbell(res, 0, 0);
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Functions check if driver declared register width is equal to platform
 * settings
 * @return CDN_EOK is driver and platform are aligned, CDN_EINVAL elsewhere
 */
uint32_t checkAddrWidth (void)
{
    uint32_t ret = CDN_EOK;

    // check width: 32 or 64 bit platform
    if (sizeof(void *) == (uint32_t)8U)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Platform 64bit\n", 0);
#ifndef PLATFORM_64_BIT
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Platform width mismatch: platform 64 bit, driver 32 bit\n", 0);
        ret = CDN_EINVAL;
#endif
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Platform 32bit\n", 0);
#ifdef PLATFORM_64_BIT
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Platform width mismatch: platform 32 bit, driver 64 bit\n", 0);
        ret = CDN_EINVAL;
#endif
    }
    return ret;
}

/**
 * Function check if endian type of driver setting is correct
 * @return CDN_EOK if endian is OK, CDN_EINVAL elsewhere
 */
uint32_t checkEndianness (void)
{
    uint32_t ret = CDN_EOK;

    // write pattern to memory
    uint32_t value32b = 0x87654321U;

    // check byte from memory
    if ((*(uint8_t *)(&value32b)) == 0x21U)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Platform endian: LITTLE\n", 0);
        // check endian mismatch
#ifdef CPU_BIG_ENDIAN
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Endian mismatch: platform little, driver big\n", 0);
        ret = CDN_EINVAL;
#endif
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Platform endian: BIG\n", 0);
        // check endian mismatch
#ifndef CPU_BIG_ENDIAN
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Endian mismatch: platform big, driver little\n", 0);
        ret = CDN_EINVAL;
#endif
    }
    return ret;
}

/* These functions intentionally violate MISRA C rules, to allow pointer
 * casts and/or manipulations required for driver operation. */

/**
 * Function sets 64 value at address given in addr parameter
 * @param addrL pointer to uint32_t word low
 * @param addrH pointer to uint32_t word high
 * @param value 64-bit dword value to write
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline void set64Value (uint32_t *addrL, uint32_t *addrH, uint64_t value)
{
    *addrL = (uint32_t)(value & UINT32_MAX);
    *addrH = (uint32_t)((value >> 32) & UINT32_MAX);
}
/* parasoft-end-suppress METRICS-36-3 */

#ifdef DEBUG
/**
 * Function returns 64bit integer value of C void pointer
 * @param ptr pointer to void type
 * @return 64bit integer of C pointer to void
 */
static inline uint64_t get64PhyAddrOfVoidPtr (const void *ptr)
{
#ifdef USE_VIRTUAL_MEM
    return CPS_GetPhyAddrOfVPtr(ptr);
#else
    return ((uint64_t)(uintptr_t)ptr);
#endif
}
#endif

/**
 * Function returns 64bit integer value of C pointer to 32bit
 * @param ptr pointer to 8 bit type
 * @return 64bit integer of C pointer to 8 bit
 */
/* parasoft-begin-suppress MISRA2012-RULE-11_4 "const uint8_t* converted to
 * unsigned long, DRV-4283" */
static inline uint64_t get64PhyAddrOf8ptr (const uint8_t *ptr)
{
#ifdef USE_VIRTUAL_MEM
    return CPS_GetPhyAddrOfVPtr((void *)ptr);
#else
    // return ((uint64_t) (uintptr_t) ptr);
    return ((uint64_t)ptr);
#endif
}
/* parasoft-end-suppress MISRA2012-RULE-11_4 */

/**
 * Function returns 64bit integer value of C pointer to 32bit
 * @param ptr pointer to 32bit type
 * @return 64bit integer of C pointer to 32bit
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
/* parasoft-begin-suppress MISRA2012-RULE-11_4 "const uint32_t* converted to
 * unsigned long, DRV-4284" */
static inline uint64_t get64PhyAddrOf32ptr (const uint32_t *ptr)
{
#ifdef USE_VIRTUAL_MEM
    return CPS_GetPhyAddrOfVPtr((void *)ptr);
#else
    return ((uint64_t)(uintptr_t)ptr);
#endif
}
/* parasoft-end-suppress MISRA2012-RULE-11_4 */
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Function returns 64bit integer value of C pointer to 64bit
 * @param ptr pointer to 64bit type
 * @return 64bit integer of C pointer to 64bit
 */
/* parasoft-begin-suppress MISRA2012-RULE-11_4 "const uint64_t* converted to
 * unsigned long, DRV-5629" */
static inline uint64_t get64PhyAddrOf64ptr (const uint64_t *ptr)
{
#ifdef USE_VIRTUAL_MEM
    return CPS_GetPhyAddrOfVPtr((void *)ptr);
#else
    return ((uint64_t)(uintptr_t)ptr);
#endif
}
/* parasoft-end-suppress MISRA2012-RULE-11_4 */

/**
 * Function returns 64bit integer value of C pointer to type uintptr
 * @param ptr pointer to 32bit type
 * @return 64bit integer of C pointer to type uintptr
 */
/* parasoft-begin-suppress MISRA2012-RULE-11_4 "unsigned long converted to
 * USBSSP_RingElementT*, DRV-5630" */
static inline USBSSP_RingElementT *getRingPtrFromPhyAddr (uintptr_t phyaddr)
{
#ifdef USE_VIRTUAL_MEM
    return (USBSSP_RingElementT *)CPS_GetVPtrFromPhyAddr(phyaddr);
#else
    return (USBSSP_RingElementT *)(phyaddr);
#endif
}
/* parasoft-end-suppress MISRA2012-RULE-11_4 */

//---------------------------------------------------------------------------

/**
 * Function displays endpoints context
 * @param ctx pointer to input context
 * @param epOffset offset in input context where endpoint context starts
 */
/* parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter inputContext/epOffset
 * not used in function displayInputContexEp, DRV-5631" */
static void displayInputContexEp (const USBSSP_InputContexT *inputContext,
                                  uint32_t epOffset)
{
    uint32_t j;
    uint32_t k;
#ifdef DEBUG
    uint32_t i = epOffset;
    const USBSSP_InputContexT *ctx = inputContext;
#endif
    // display default endpoint context
    for (j = 0; j < USBSSP_CONTEXT_WIDTH; j++)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                "  (+0x%03x/EP0_CTX+%2d      ) 0x%08X\n", 4 * i++, j,
                le32ToCpu(ctx->ep0Context[j]));
    }

    // display all endpoints context
    for (j = 0; j < USBSSP_MAX_EP_CONTEXT_NUM; j++)
    {
        for (k = 0; k < USBSSP_CONTEXT_WIDTH; k++)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                    "  (+0x%03x/EP%02d_%s_CTX+%2d ) 0x%08X\n", 4 * i++,
                    (j >> 1) + 1, ((j & 1) ? "IN " : "OUT"), k,
                    le32ToCpu(ctx->epContext[j][k]));
        }
    }
}
/* parasoft-end-suppress MISRA2012-RULE-2_7-4 */

static void displayInputContex (const USBSSP_InputContexT *ctx)
{
    uint32_t i;
    uint32_t j;

    // check if context is not NULL
    if (ctx != NULL)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_HIVERB,
            "INPUT_CONTEXT @(log 0x%016X..0x%016X phys. 0x%016X..0x%016X):\n",
            get64PhyAddrOfVoidPtr((const void *)ctx),
            get64PhyAddrOfVoidPtr((const void *)ctx) +
                sizeof(USBSSP_InputContexT) - 1,
            get64PhyAddrOfVoidPtr(ctx),
            get64PhyAddrOfVoidPtr(ctx) + sizeof(USBSSP_InputContexT) - 1);

        i = 0;
        // display input control context
        for (j = 0; j < USBSSP_CONTEXT_WIDTH; j++)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                    "  (+0x%03x/IN_CTRL_CTX+%2d  ) 0x%08X\n", 4 * i++, j,
                    le32ToCpu(ctx->inputControlContext[j]));
        }
        // display slot context
        for (j = 0; j < USBSSP_CONTEXT_WIDTH; j++)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                    "  (+0x%03x/SLOT_CTX+%2d     ) 0x%08X\n", 4 * i++, j,
                    le32ToCpu(ctx->slot[j]));
        }
        displayInputContexEp(ctx, i);
    }
}

/**
 * Update event pointer
 * @param[in] res driver resources
 */
static void updateEventPtr (USBSSP_DriverResourcesT *res)
{
    // get address of last element in event ring
    USBSSP_RingElementT
        *poolEnd = &res->xhciMemRes->eventPool[USBSSP_EVENT_QUEUE_SIZE - 1U];

    CPS_CacheFlush((void *)res->eventPtr, sizeof(USBSSP_RingElementT), 0);
    ++res->eventPtr;

    if (res->eventPtr > poolEnd)
    {
        res->eventPtr = res->xhciMemRes->eventPool;
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> Change toggle bit\n",
                res->instanceNo);
        if (res->eventToogleBit == 1U)
        {
            res->eventToogleBit = 0U;
        }
        else
        {
            res->eventToogleBit = 1U;
        }
    }
}

/**
 * Function updates res->devConfigFlag
 * @param res driver resource
 * @param newCfgFlag flag to be set as actual
 */
static void setDevConfigFlag (USBSSP_DriverResourcesT *res, uint8_t newCfgFlag)
{
    uint8_t prev_cfg_flag = res->devConfigFlag;

    if (newCfgFlag != prev_cfg_flag)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Configured status changed from %d to %d\n",
                res->instanceNo, prev_cfg_flag, newCfgFlag);

        res->devConfigFlag = newCfgFlag;
    }
}

/**
 * Insert Link TRB
 * @param[in] queue: pointer to producer queue
 * @param linkTrbChainFlag Chain flag if using Link TRB
 */
static void insertLinkTRB (USBSSP_ProducerQueueT *queue,
                           uint32_t linkTrbChainFlag)
{
    uint32_t linkTrbFlags = (USBSSP_TRB_LINK << USBSSP_TRB_TYPE_POS) |
                            USBSSP_TRB_LNK_TGLE_CYC_MSK |
                            ((uint32_t)(queue->toogleBit)) | linkTrbChainFlag;

    /* set DWORD0 and DWORD1 */
    set64Value(&queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 1U].dword0,
               &queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 1U].dword1,
               cpuToLe64(get64PhyAddrOf32ptr(&queue->ring[0].dword0)));
    /* set DWORD2 */
    queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 1U].dword2 = cpuToLe32(0);
    /* set flags in DWORD3 */
    queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 1U].dword3 = cpuToLe32(
        linkTrbFlags);

    CPS_CacheFlush((void *)&queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 1U],
                   sizeof(USBSSP_RingElementT), 0);

    XHCI_DISP_TRB_PROC(queue->enqueuePtr, "LINK.");

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "LinkTRB contextIndex:%d linkTRBAddr:0x%16x LinkAddr:0x%16x "
            "dword3:0x%08x\n",
            queue->contextIndex,
            get64PhyAddrOf32ptr(
                &queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 1U].dword0),
            cpuToLe64(get64PhyAddrOf32ptr(&queue->ring[0].dword0)),
            queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 1U].dword3);
}
/**
 * Update queue
 * @param[in] queue pointer to producer queue
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
/* parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter debugString not used
 * in function updateQueuePtr, DRV-5631" */
static void updateQueuePtr (USBSSP_ProducerQueueT *queue,
                            uint32_t linkTrbChainFlag, const char *debugString)
{
    USBSSP_RingElementT *oldPtr = queue->enqueuePtr;
    // calculate where ring ends - the last element is a LINK TRB
    USBSSP_RingElementT
        *poolEnd = &queue->ring[(USBSSP_PRODUCER_QUEUE_SIZE - 2U)];

    XHCI_DISP_TRB_PROC(queue->enqueuePtr, debugString);

    ++queue->enqueuePtr;

    /* check if enqueuePtr exceeded ring pool and turn back to origin if yes */
    if (queue->enqueuePtr > poolEnd)
    {
        queue->enqueuePtr = queue->ring;

        /* update TRB when queue is not full */
        if (queue->enqueuePtr != queue->dequeuePtr)
        {
            /* insert LINK TRB */
            insertLinkTRB(queue, linkTrbChainFlag);
            /* toggle the cycle bit */
            queue->toogleBit ^= 1U;
        }
    }

    // check if queue full and if yes do nothing
    if (queue->enqueuePtr == queue->dequeuePtr)
    {
        queue->enqueuePtr = oldPtr;
        vDbgMsg(USBSSP_DBG_DRV, DBG_WARN, "QUEUE FULL!\n", 0);
    }
}
/* parasoft-end-suppress MISRA2012-RULE-2_7-4 */
/* parasoft-end-suppress METRICS-36-3 */

static USBSSP_RingElementT *getPrevTrb (const USBSSP_ProducerQueueT *queue,
                                        USBSSP_RingElementT *actualTrb)
{
    USBSSP_RingElementT *prevTrb = actualTrb;
    --prevTrb;

    // check if TD overturn on ring
    if (prevTrb < queue->ring)
    {
        prevTrb = &queue->ring[USBSSP_PRODUCER_QUEUE_SIZE - 2U];
        // if cycle bit has not been changed return error
        if (getToogleBit(prevTrb) == getToogleBit(actualTrb))
        {
            prevTrb = NULL;
        }
    }
    return prevTrb;
}

/**
 * Function updates res->actualSpeed value
 * @param res driver resources
 */
static void setSpeed (USBSSP_DriverResourcesT *res)
{
    uint32_t portStatus;
    uint8_t portSpeed;

    // get protocol speed ID
    portStatus = xhciRead32(
        &res->regs.xhciPortControl[res->actualPort - 1U].portsc);
    portSpeed = (uint8_t)CPS_FLD_READ(USBSSP__PORTSC1USB3, PORTSPEED,
                                      portStatus);

    /* according to Table 157: Default USB Speed ID Mapping */
    switch (portSpeed)
    {
        // full speed
    case 1U:
        res->actualSpeed = CH9_USB_SPEED_FULL;
        break;

        // low speed
    case 2U:
        res->actualSpeed = CH9_USB_SPEED_LOW;
        break;

        // high speed
    case 3U:
        res->actualSpeed = CH9_USB_SPEED_HIGH;
        break;

        // super speed
    case 4U:
        res->actualSpeed = CH9_USB_SPEED_SUPER;
        break;

        // super speed plus
    case 5U:
        res->actualSpeed = CH9_USB_SPEED_SUPER_PLUS;
        break;

        // speed unknown
    default:
        res->actualSpeed = CH9_USB_SPEED_UNKNOWN;
        break;
    }

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> actual speed: %d\n", res->instanceNo,
            res->actualSpeed);
}

/**
 * Get Endpoint state
 * @param res Pointer to the XHCI resource structure
 * @param epIndex EP-index as per device context array (XHCI 6.2.1)
 * @return
 */
uint32_t USBSSP_GetEpState (const USBSSP_DriverResourcesT *res,
                            uint32_t epIndex)
{
    uint32_t ret = (uint32_t)(USBSSP_EP_CONTEXT_EP_STATE_ERROR);

    if ((res == NULL) || (epIndex < USBSSP_EP0_CONT_OFFSET) ||
        (epIndex >= (USBSSP_MAX_EP_CONTEXT_NUM + USBSSP_EP_CONT_OFFSET)))
    {
        ret = CDN_EINVAL;
    }
    else
    {
        if (epIndex == USBSSP_EP0_CONT_OFFSET)
        {
            ret = (le32ToCpu(res->xhciMemRes->outputContext->ep0Context[0]) &
                   (uint32_t)USBSSP_EP_CONTEXT_STATE_MASK);
        }
        else
        {
            // get status of sw endpoint
            // calculate index of endpoint in epContext array
            uint32_t epBase = epIndex - USBSSP_EP_CONT_OFFSET;
            // read endpoint state from output context updated by controller
            ret = (le32ToCpu(
                       res->xhciMemRes->outputContext->epContext[epBase][0]) &
                   (uint32_t)USBSSP_EP_CONTEXT_STATE_MASK);
        }
    }
    return ret;
}

/**
 * Function wait for register value
 * @param reg address of checked register
 * @param mask bitmap of relevant bits taken for checking
 * @param waitFor value of which we wait for
 * @param timeout timeout value
 * @return CDN_EOK on success or CDN_ETIMEDOUT on timeout
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static uint32_t waitForReg (volatile uint32_t *reg, uint32_t mask,
                            uint32_t waitFor, uint32_t timeout)
{
    uint32_t counter = timeout;
    uint32_t ret = CDN_EOK;

    while ((xhciRead32(reg) & mask) != waitFor)
    {
        // break loop when timeout value is 0
        if (counter == 0U)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "%s() timeout after %d\n",
                    __func__, timeout);
            ret = CDN_ETIMEDOUT;
            break;
        }
        counter--;
        CPS_DelayUs(1);
    }
    return ret;
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Waits for 64bit register value
 * @param reg address of checked 64bit register
 * @param mask 64 bits bitmap of relevant bits taken for checking
 * @param waitFor value of which we wait for
 * @param timeout timeout value
 * @return CDN_EOK on success or CDN_ETIMEDOUT on timeout
 */
static uint32_t waitForReg64 (volatile uint64_t *reg, uint64_t mask,
                              uint64_t waitFor, uint32_t timeout)
{
    uint32_t counter = timeout;
    uint32_t ret = CDN_EOK;

    while ((xhciRead64(reg) & mask) != waitFor)
    {
        // break loop when timeout value is 0
        if (counter == 0U)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "%s() timeout after %d\n",
                    __func__, timeout);
            ret = CDN_ETIMEDOUT;
            break;
        }
        counter--;
        CPS_DelayUs(1);
    }
    return ret;
}

/**
 * Wait for default endpoint ready for next operation, function must not be used
 * in interrupt context
 * @param res driver resources
 * @param timeout timeout value
 * @return CDN_EOK on success or CDN_ETIMEDOUT on timeout, CDN_EIO on disconnect
 * event
 */
static uint32_t waitForResponse (const USBSSP_DriverResourcesT *res,
                                 uint32_t timeout)
{
    uint32_t counter = timeout;
    uint32_t ret = CDN_EOK;

    while (res->ep0.isRunningFlag != 0U)
    {
        CPS_DelayUs(1);
        // break loop when timeout value is 0
        if (counter == 0U)
        {
            ret = CDN_ETIMEDOUT;
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> %s() timeout after %d\n",
                    res->instanceNo, __func__, timeout);
        }
        else if (res->connected == 0U)
        {  // break loop if device is disconnected
            ret = CDN_EIO;
        }
        else
        {
            counter--;
        }
        if (ret != CDN_EOK)
        {
            break;
        }
    }
    return ret;
}

/**
 *  Function returns when device is in addressed state, function must not be
 * called from interrupt context! Function used in host mode
 * @param res driver resources
 * @param timeout TODO timeout parameter should be removed
 * @return CDN_EOK when device is in addressed state, CDN_ETIMEDOUT on timeout
 * event
 */
static uint32_t waitUntilAddressState (const USBSSP_DriverResourcesT *res,
                                       uint32_t timeout)
{
    uint32_t counter = timeout;
    uint32_t ret = CDN_EOK;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "connected(%d) devAddress(%d) - start\n",
            res->connected, res->devAddress);

    // device must be connected on port
    while (res->connected == 0U)
    {
        CPS_DelayUs(1);
    }

    while (res->devAddress == 0U)
    {
        CPS_DelayUs(1);
        // break loop when timeout value is 0
        if (counter == 0U)
        {
            ret = CDN_ETIMEDOUT;
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> %s() timeout after %d\n",
                    res->instanceNo, __func__, timeout);
        }
        else if (res->connected == 0U)
        {  // break loop when device is disconnected
            ret = CDN_EIO;
        }
        else
        {
            counter--;
        }
        if (ret != CDN_EOK)
        {
            break;
        }
    }
    return ret;
}

/**
 * Function returns on endpoint's stopped or disabled state or on timeout
 * @param res driver resources
 * @param timeout value of operation timeout
 * @return CDN_EOK on success, ETIMEOUT when timeout generated return
 */
static uint32_t waitUntilEpStoppedDisabled (const USBSSP_DriverResourcesT *res,
                                            uint32_t timeout)
{
    uint32_t counter = timeout;
    uint32_t ret = CDN_ETIMEDOUT;
    uint32_t flags = 0U;

    // do constant checking endpoint state, and break loop only if all endpoints
    // are in stopped or disabled state, or break loop if timeout counter zeroed
    do
    {
        uint8_t epIndex;
        flags = 0U;
        for (epIndex = 1U; epIndex <= 31U; epIndex++)
        {
            USBSSP_EpContexEpState epState = getEndpointStatus(res, epIndex);
            if ((epState != USBSSP_EP_CONTEXT_EP_STATE_DISABLED) &&
                (epState != USBSSP_EP_CONTEXT_EP_STATE_STOPPED) &&
                (epState != USBSSP_EP_CONTEXT_EP_STATE_ERROR))
            {
                flags |= ((uint32_t)1 << epIndex);
            }
        }
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "Waiting to stop endpoints Flags = 0x%X\n", flags);

        counter--;

        CPS_DelayUs(1);
    } while ((counter > 0U) && (flags != 0U));

    // check the reason which loop has ended its work and return suitable status
    if (flags == 0U)
    {
        ret = CDN_EOK;
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Error: Timed out waiting to stop endpoints Flags = 0x%X\n",
                flags);
    }
    return ret;
}

/**
 * Function checks whether the specified ep is enabled for data transfer
 * @param res driver resources
 * @param[in] epIndex index of endpoint according to xhci spec e.g for ep1out
              epIndex=2, for ep1in epIndex=3, for ep2out epIndex=4 end so on
 * @return  CDN_EOK on success
 *          CDN_EIO on not connected error
 *          CDN_ENOTSUP if EP not configured for transfer
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static uint32_t checkEpXferEnabled (const USBSSP_DriverResourcesT *res,
                                    uint8_t epIndex)
{
    uint32_t ret = CDN_EOK;

    /* operation not permitted on disconnected device */
    if ((res->devAddress == 0U) || (res->connected == 0U))
    {
        ret = CDN_EIO;
    }
    else if (epIndex > 1U)
    {
        /* check endpoint descriptor */
        if (res->ep[epIndex].epDesc[0] != CH9_USB_DS_ENDPOINT)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Error: Endpoint index %d not initialized correctly\n",
                    res->instanceNo, epIndex);
            ret = CDN_ENOTSUP;
        }
        else if (res->devConfigFlag == 0U)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Error: Endpoint index %d NOT configured \n",
                    res->instanceNo, epIndex);
            ret = CDN_ENOTSUP;
        }
        else
        {
            /* Else required by MISRA*/
        }
    }
    else
    {
        /* For EP0 check if last transfer is complete
         * since Ep0 supports only control transfer */
        if (res->ep0.isRunningFlag == 1U)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Error: EP0 is already running \n", res->instanceNo);
            ret = CDN_ENOTSUP;
        }
    }

    return ret;
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Function aborts currently executed command if any
 * @param res driver resources
 */
static void abortCurrentCommand (const USBSSP_DriverResourcesT *res)
{
    uint64_t crcr = xhciRead64(&res->regs.xhciOperational->crcr);

    // check if any command is pending
    if ((crcr & ((uint64_t)USBSSP__CRCR_LO__CRR_MASK)) > 0U)
    {
        // read CRCR register value and set CA (command abort) bit
        crcr |= (uint64_t)USBSSP__CRCR_LO__CA_MASK;
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> CRCR:%016X\n", res->instanceNo,
                crcr);
        xhciWrite64(&res->regs.xhciOperational->crcr, crcr);

        // wait until CRR (Command Ring Running) is not active
        do
        {
            crcr = xhciRead64(&res->regs.xhciOperational->crcr);
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Command abort\n",
                    res->instanceNo);
            CPS_DelayUs(1);
        } while ((crcr & ((uint64_t)USBSSP__CRCR_LO__CRR_MASK)) > 0U);
    }
}

/**
 * issue enable slot command
 * @param res driver resources
 * @return CDN_EOK on success, CDN_EINVAL on critical error
 */
static uint32_t issueEnableSlotCommand (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "actualdeviceSlot: %d\n",
            res->actualdeviceSlot);

    // enable slot if no actual device slot is active.
    if (res->actualdeviceSlot == 0U)
    {
        ret = USBSSP_EnableSlot(res);
        if (ret == (uint32_t)CDN_EINVAL)
        {
            DbgMsg(
                USBSSP_DBG_DRV, DBG_CRIT,
                "Critical error! wrong value in one of function parameters\n");
        }
    }
    return ret;
}

/**
 * issue disable slot command
 * @param res driver resources
 * @return CDN_EOK on success, CDN_EINVAL on critical error
 */
static uint32_t issueDisableSlotCommand (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    // disable slot if any device slot is active.
    if (res->actualdeviceSlot > 0U)
    {
        ret = USBSSP_DisableSlot(res);
        if (ret == (uint32_t)CDN_EINVAL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Critical error! wrong value in one of function "
                    "parameters\n",
                    res->instanceNo);
        }
    }
    return ret;
}

/**
 * handle connect status change for connect case
 * @param res driver resources
 * @param portStatus port status
 */
void connectStatusChangeConnect (USBSSP_DriverResourcesT *res,
                                 uint32_t portStatus)
{
    uint32_t ret;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Device connected on port: %d\n",
            res->instanceNo, res->actualPort);

    res->connected = 1U;
    // call connect callback of cusbd module
    if (res->deviceModeFlag > 0U)
    {
        if (res->cusbdCallbacks->connect != NULL)
        {
            res->cusbdCallbacks->connect(res->privateData);
        }
    }

    // check if port enabled
    if ((portStatus & USBSSP__PORTSC1USB3__PED_MASK) > 0U)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Port %d ENABLED!\n",
                res->instanceNo, res->actualPort);
        ret = issueEnableSlotCommand(res);
    }
    else
    {
        // port is disabled
        uint32_t actualSpeed = CPS_FLD_READ(USBSSP__PORTSC1USB3, PORTSPEED,
                                            portStatus);

        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Port %d DISABLED!\n",
                res->instanceNo, res->actualPort);
        // do reset for USB20
        if (actualSpeed < (uint32_t)CH9_USB_SPEED_SUPER)
        {
            ret = USBSSP_ResetRootHubPort(res);
            if (ret == (uint32_t)CDN_EINVAL)
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                        "<%d> Critical error! wrong value in one of function "
                        "parameters\n",
                        res->instanceNo);
            }
            ret = issueEnableSlotCommand(res);
        }
    }
}

/**
 * handle connect status change for disconnect case
 * @param res driver resources
 * @param portStatus port status
 */
static void connectStatusChangeDisconnect (USBSSP_DriverResourcesT *res)
{
    // device is disconnected
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Device disconnected\n",
            res->instanceNo);

    // set U1 nd U2 to default value
    setU1timeout(res, 0);
    setU2timeout(res, 0);

    // call disconnect callback of cusbd module
    if (res->deviceModeFlag > 0U)
    {
        if (res->cusbdCallbacks->disconnect != NULL)
        {
            res->cusbdCallbacks->disconnect(res->privateData);
            res->devDisconnectCBPendingFlag = 0U;
        }
    }

    // abort current command
    abortCurrentCommand(res);
    res->connected = 0U;

    // issue disable slot command
    (void)issueDisableSlotCommand(res);
}

/**
 * Function handles connect status change event
 * @param res driver resources
 * @param portStatus port status read from portsc register
 */
static void connectStatusChange (USBSSP_DriverResourcesT *res,
                                 uint32_t portStatus)
{
    // check CCS bit, check if device is connected
    if ((portStatus & USBSSP__PORTSC1USB3__CCS_MASK) != 0U)
    {
        connectStatusChangeConnect(res, portStatus);
    }
    else
    {
        connectStatusChangeDisconnect(res);
    }
}

/**
 * Get Pointer to the Extended capability register
 * @param res Driver resources
 * @param capId Capability ID
 * @return Pointer to the first register of the specified capability ID
 */
static uint32_t *getExtCapRegPtr (const USBSSP_DriverResourcesT *const res,
                                  uint8_t capId)
{
    uint32_t extCapIdx = 0;
    const USBSSP_ExtCapSetT *extCapRegs = &res->regs.xhciExtCaps;
    uint32_t *regPtr = NULL;

    for (extCapIdx = 0; extCapIdx < extCapRegs->extCapsCount; extCapIdx++)
    {
        if (extCapRegs->extCaps[extCapIdx].capId == capId)
        {
            regPtr = extCapRegs->extCaps[extCapIdx].firstCapSfrPtr;
        }
    }

    return regPtr;
}

/**
 * Initialization of SSPDriverResourcesT object: Set up port control
 * registers 2.
 *
 * @param[in] res driver resources
 */
static void setPortControl2Registers (USBSSP_DriverResourcesT *res)
{
    uint32_t *extCap3xPortRegs = getExtCapRegPtr(
        res, USBSSP_D_XEC_CFG_3XPORT_CAP_ID);

    if (extCap3xPortRegs != NULL)
    {
        uint32_t *extCap3xPortMdReg = &extCap3xPortRegs[54];
        uint32_t extCap3xPortMdRegVal = xhciRead32(extCap3xPortMdReg);

        // set MODE 2 register
        extCap3xPortMdRegVal = CPS_FLD_CLEAR(USBSSP__XEC_CFG_3XPORT_MODE_2,
                                             CFG_U1_PIPE_CLK_GATE_EN,
                                             extCap3xPortMdRegVal);
        extCap3xPortMdRegVal = CPS_FLD_CLEAR(USBSSP__XEC_CFG_3XPORT_MODE_2,
                                             CFG_U2_PIPE_CLK_GATE_EN,
                                             extCap3xPortMdRegVal);
        xhciWrite32(extCap3xPortMdReg, extCap3xPortMdRegVal);
    }
}

/**
 * Function handle power mode
 * @param res
 * @param portStatus
 */
static void handleLPM (USBSSP_DriverResourcesT *res)
{
    // set U1 and U2 timeouts to default value
    setU1timeout(res, 0);
    setU2timeout(res, 0);

    if (res->deviceModeFlag > 0U)
    {
        uint32_t portpmsc = CPS_FLD_WRITE(USBSSP__PORTPMSC1USB2, L1S, 0, 2U);
        xhciWrite32(&res->regs.xhciPortControl[USBSSP_DEV_MODE_2_PORT].portpmsc,
                    portpmsc);
    }
}

/**
 * Function handles port reset change
 * @param res driver resources
 * @param portStatus port status read from portsc register
 */
void portResetChange (USBSSP_DriverResourcesT *res, uint32_t portStatus)
{
    uint32_t ret = CDN_EOK;
    uint32_t delayDisconnectCB = 0U;
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Port reset change\n",
            res->instanceNo);

    handleLPM(res);

    if (res->deviceModeFlag > 0U)
    {
        setPortControl2Registers(res);
    }

    // check if port enabled
    if ((portStatus & USBSSP__PORTSC1USB3__PED_MASK) != 0U)
    {
        // check if EnableSlot command is not already pending
        if (res->enableSlotInProgress == 0U)
        {
            if (res->actualdeviceSlot != 0U)
            {
                delayDisconnectCB = 1U;
                setSpeed(res);
                ret = USBSSP_ResetDevice(res);
                if (ret != CDN_EOK)
                {
                    vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                            "<%d> Could not reset controller\n",
                            res->instanceNo);
                }
            }
            else
            {
                // Slot not enabled yet
                ret = issueEnableSlotCommand(res);
            }
        }
    }

    // call disconnect callback of CUSBD module
    if ((res->deviceModeFlag > 0U) && (res->cusbdCallbacks->disconnect != NULL))
    {
        if (delayDisconnectCB == 0U)
        {
            res->cusbdCallbacks->disconnect(res->privateData);
        }
        else
        {
            res->devDisconnectCBPendingFlag = 1U;
        }
    }
}

static void portLinkStateChange (USBSSP_DriverResourcesT *res,
                                 uint32_t portStatus)
{
    // TODO no enum values generated for PLS field!
    if (CPS_FLD_READ(USBSSP__PORTSC1USB3, PLS, portStatus) == 15U /*RESUME*/)
    {
        uint32_t portsc;
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Device resumed from low power mode\n", res->instanceNo);

        // PLS=U0 LWS=1
        portsc = xhciRead32(
            &res->regs.xhciPortControl[res->actualPort - 1U].portsc);
        portsc = CPS_FLD_WRITE(USBSSP__PORTSC1USB3, PLS, portsc,
                               0U); /*U0 state*/
        portsc = CPS_FLD_SET(USBSSP__PORTSC1USB3, LWS, portsc);
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> portsc: %08X\n", res->instanceNo,
                portsc & ~(USBSSP__PORTSC1USB3__PED_MASK));
        xhciWrite32(&res->regs.xhciPortControl[res->actualPort - 1U].portsc,
                    portsc & ~(USBSSP__PORTSC1USB3__PED_MASK));
    }
}

/**
 * Port change detection. Function handles all changes on port
 * @param[in] res driver resources
 */
static void portChangeDetect (USBSSP_DriverResourcesT *res)
{
    uint32_t portStatus;
    uint32_t maskAllChangeBits = USBSSP__PORTSC1USB3__CSC_MASK |
                                 USBSSP__PORTSC1USB3__PEC_MASK |
                                 USBSSP__PORTSC1USB3__WRC_MASK |
                                 USBSSP__PORTSC1USB3__PRC_MASK |
                                 USBSSP__PORTSC1USB3__PLC_MASK |
                                 USBSSP__PORTSC1USB3__CEC_MASK |
                                 USBSSP__PORTSC1USB3__OCC_MASK;

    res->actualPort = getPortId(res->eventPtr);
    portStatus = xhciRead32(
        &res->regs.xhciPortControl[res->actualPort - 1U].portsc);

    vDbgMsg(
        USBSSP_DBG_DRV, DBG_FYI,
        "<%d> Port ID: %d, PORTSC: 0x%08X PORTPMSC: 0x%08X\n", res->instanceNo,
        res->actualPort, portStatus,
        xhciRead32(&res->regs.xhciPortControl[res->actualPort - 1U].portpmsc));

    do
    {
        // clear all interrupts
        xhciWrite32(&res->regs.xhciPortControl[res->actualPort - 1U].portsc,
                    portStatus & ~(USBSSP__PORTSC1USB3__PED_MASK));

        if (res->usbsspCallbacks.preportChangeDetectCallback != NULL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                    "<%d> Call user defined Port Status Change Event pre "
                    "callback\n",
                    res->instanceNo);
            res->usbsspCallbacks.preportChangeDetectCallback(res, portStatus,
                                                             res->actualPort);
        }

        // handle connect status change
        if ((portStatus & USBSSP__PORTSC1USB3__CSC_MASK) != 0U)
        {
            connectStatusChange(res, portStatus);
        }

        // handle port reset change IF port is connected
        if (((portStatus & USBSSP__PORTSC1USB3__PRC_MASK) != 0U) &&
            ((portStatus & USBSSP__PORTSC1USB3__CCS_MASK) != 0U))
        {
            portResetChange(res, portStatus);
        }

        // handle port link state change
        if ((portStatus & USBSSP__PORTSC1USB3__PLC_MASK) != 0U)
        {
            portLinkStateChange(res, portStatus);
        }

        // update portStatus - in meantime new change may have happen
        portStatus = xhciRead32(
            &res->regs.xhciPortControl[res->actualPort - 1U].portsc);

    } while ((portStatus & maskAllChangeBits) > 0U);
}

/**
 * Enable DDUSB. This function needs to be called with xHC stopped
 * @param[in] res Driver resources
 * @param[in] ddusbTxRegs Pointer to array of 4 ddusb_tx_valid registers
 * @param[in] ddusbRxRegs Pointer to array of 4 ddusb_rx_valid registers
 * @param[in] ddusbEpIntIdx Interrupter index for DDUSB enabled Endpoints
 */
uint32_t USBSSP_EnableDDUSB (USBSSP_DriverResourcesT *res,
                             const uint32_t *ddusbTxRegs,
                             const uint32_t *ddusbRxRegs,
                             uint32_t ddusbEpIntIdx)
{
    uint32_t ret = CDN_EOK;
    ret = USBSSP_EnableDDUSBSF(res, ddusbTxRegs, ddusbRxRegs);

    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        USBSSP_SfrT *regs = &res->regs;
        // check whether we have pointer to the DDUSB registers
        if (regs->ddusbConfig == NULL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> DDUSB Not configured ddusbConfig-Ptr:(0x%08X)\n",
                    res->instanceNo, regs->ddusbConfig);
            ret = CDN_EINVAL;
        }
        else
        {
            uint32_t txRegIdx;
            uint32_t rxRegIdx;

            // program the ddusb TX registers
            for (txRegIdx = 0; txRegIdx < USBSSP_DDUSB_TX_CTRL_REG_COUNT;
                 txRegIdx++)
            {
                uint32_t txEpNum;         // denotes Tx Ep Num [1,15]
                uint32_t txEpContextIdx;  // denotes context idx of TxEP {2, 4,
                                          // 6, .. 30}
                txEpNum = (ddusbTxRegs[txRegIdx] >>
                           USBSSP_DDUSB_CTRL_REG_EP_POS) &
                          USBSSP_DDUSB_CTRL_REG_EP_MASK;
                if (txEpNum != 0U)
                {
                    txEpContextIdx = (txEpNum << 1U);
                    res->ep[txEpContextIdx].interrupterIdx = ddusbEpIntIdx;
                    xhciWrite32(&regs->ddusbConfig->ddusbTx[txRegIdx],
                                ddusbTxRegs[txRegIdx]);
                }
            }

            // program the Rx registers
            for (rxRegIdx = 0; rxRegIdx < USBSSP_DDUSB_RX_CTRL_REG_COUNT;
                 rxRegIdx++)
            {
                uint32_t rxEpNum;         // denotes Rx Ep Num [1,15]
                uint32_t rxEpContextIdx;  // denotes context idx of RxEP {3, 5,
                                          // 7, .. 31}
                rxEpNum = (ddusbRxRegs[rxRegIdx] >>
                           USBSSP_DDUSB_CTRL_REG_EP_POS) &
                          USBSSP_DDUSB_CTRL_REG_EP_MASK;
                if (rxEpNum != 0U)
                {
                    rxEpContextIdx = (rxEpNum << 1U) | 1U;
                    res->ep[rxEpContextIdx].interrupterIdx = ddusbEpIntIdx;
                    xhciWrite32(&regs->ddusbConfig->ddusbRx[rxRegIdx],
                                ddusbRxRegs[rxRegIdx]);
                }
            }

            // Enable DDUSB
            xhciWrite32(&regs->ddusbConfig->ddusbEnable, (uint32_t)1U);
        }
    }
    return ret;
}

/**
 * Disable DDUSB. This function needs to be called with xHC stopped
 * @param[in] res Driver resources
 * @param[in] epIntIdx Restored Interrupter index for DDUSB enabled Endpoints
 */
uint32_t USBSSP_DisableDDUSB (USBSSP_DriverResourcesT *res, uint32_t epIntIdx)
{
    uint32_t ret = CDN_EOK;
    ret = USBSSP_DisableDDUSBSF(res);

    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        USBSSP_SfrT *regs = &res->regs;
        if (regs->ddusbConfig == NULL)
        {
            // check whether we have pointer to the DDUSB registers
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> DDUSB Not configured ddusbConfig-Ptr:(0x%08X) "
                    "eventRingAddr[0]:\n(0x%08X)",
                    res->instanceNo, regs->ddusbConfig,
                    regs->ddusbEventRegs.eventRingAddr[0]);
            ret = CDN_EINVAL;
        }
        else
        {
            uint32_t txRegIdx;
            uint32_t rxRegIdx;
            USBSSP_DDUSBConfigT *ddusbCfg = regs->ddusbConfig;

            // reset the ddusb TX registers and corresponding ep-context
            for (txRegIdx = 0; txRegIdx < USBSSP_DDUSB_TX_CTRL_REG_COUNT;
                 txRegIdx++)
            {
                uint32_t txEpContextIdx;  // denotes context idx of TxEP {2, 4,
                                          // 6, .. 30}
                uint32_t txRegValue = xhciRead32(&ddusbCfg->ddusbTx[txRegIdx]);
                // denotes Tx Ep Num [1,15]
                uint32_t txEpNum = (txRegValue >>
                                    USBSSP_DDUSB_CTRL_REG_EP_POS) &
                                   USBSSP_DDUSB_CTRL_REG_EP_MASK;

                if (txEpNum != 0U)
                {
                    txEpContextIdx = (txEpNum << 1U);
                    res->ep[txEpContextIdx].interrupterIdx = epIntIdx;
                    xhciWrite32(&ddusbCfg->ddusbTx[txRegIdx], 0U);
                }
            }

            // RX registers immediately follows the TX registers
            for (rxRegIdx = 0; rxRegIdx < USBSSP_DDUSB_RX_CTRL_REG_COUNT;
                 rxRegIdx++)
            {
                uint32_t rxEpContextIdx;  // denotes context idx of RxEP {3, 5,
                                          // 7, .. 31}
                uint32_t rxRegValue = xhciRead32(&ddusbCfg->ddusbRx[rxRegIdx]);
                // denotes Rx Ep Num [1,15]
                uint32_t rxEpNum = (rxRegValue >>
                                    USBSSP_DDUSB_CTRL_REG_EP_POS) &
                                   USBSSP_DDUSB_CTRL_REG_EP_MASK;

                if (rxEpNum != 0U)
                {
                    rxEpContextIdx = (rxEpNum << 1U) | 1U;
                    res->ep[rxEpContextIdx].interrupterIdx = epIntIdx;
                    xhciWrite32(&ddusbCfg->ddusbRx[rxRegIdx], 0U);
                }
            }

            // Disable DDUSB
            xhciWrite32(&ddusbCfg->ddusbEnable, (uint32_t)0U);
        }
    }
    return ret;
}

/**
 * function sets max packet size for default endpoint
 * @param res driver resources
 * @return max packet size for default endpoint
 */
static CH9_UsbEP0Max setMaxPacketSizeEp0 (const USBSSP_DriverResourcesT *res)
{
    CH9_UsbEP0Max epMaxPacketSize;

    switch (res->actualSpeed)
    {
        // low speed
    case CH9_USB_SPEED_LOW:
        epMaxPacketSize = CH9_USB_EP0_MAX_LOW;
        break;

        // full speed
    case CH9_USB_SPEED_FULL:
        epMaxPacketSize = CH9_USB_EP0_MAX_FULL;
        break;

        // high speed
    case CH9_USB_SPEED_HIGH:
        epMaxPacketSize = CH9_USB_EP0_MAX_HIGH;
        break;

        // super, super speed plus
    case CH9_USB_SPEED_SUPER:
    case CH9_USB_SPEED_SUPER_PLUS:
        epMaxPacketSize = CH9_USB_EP0_MAX_SUPER;
        break;

        // unknown speed
    default:
        epMaxPacketSize = CH9_USB_EP0_MAX_UNKNOWN;
        break;
    }

    return epMaxPacketSize;
}

/**
 * Function builds SET ADDRESS command TRB
 * @param res driver resources
 * @param bsrVal BSR bit value
 */
static void setAddressCommandTrb (USBSSP_DriverResourcesT *res, uint8_t bsrVal)
{
    // two first DWORDs are pointer to input context
    set64Value(
        &res->commandQ.enqueuePtr->dword0, &res->commandQ.enqueuePtr->dword1,
        cpuToLe64(get64PhyAddrOf32ptr(res->inputContext->inputControlContext)));

    res->commandQ.enqueuePtr->dword2 = 0;

    // DWORD3 is a TRB code and required flags
    res->commandQ.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)res->actualdeviceSlot << USBSSP_SLOT_ID_POS) |
        (uint32_t)(USBSSP_TRB_ADDR_DEV_CMD << USBSSP_TRB_TYPE_POS) |
        ((uint32_t)bsrVal << USBSSP_BSR_POS) |
        (uint32_t)res->commandQ.toogleBit);

    CPS_CacheFlush((void *)res->inputContext,
                   sizeof(res->inputContext->inputControlContext) +
                       sizeof(res->inputContext->slot) +
                       sizeof(res->inputContext->ep0Context),
                   0);
}

/**
 * set the slot context
 * @param res driver resource
 */
static void setSlotContext (USBSSP_DriverResourcesT *res)
{
    CH9_UsbEP0Max epMaxPacketSize;
    uint32_t slotSpeed = 0U;

    // set EP0 max packet size
    epMaxPacketSize = setMaxPacketSizeEp0(res);

    // CH9_UsbSpeed coding is different than XHCI one
    slotSpeed = getSlotSpeed(res->actualSpeed);

    // 6.2.2 set slot context entries and speed
    res->inputContext->slot[0] = cpuToLe32(
        (uint32_t)(1UL << USBSSP_SLOT_CXT_CXT_ENT_POS) |
        (uint32_t)(slotSpeed << USBSSP_SLOT_CONTEXT_SPEED_POS));

    res->inputContext->slot[1] = cpuToLe32((uint32_t)res->actualPort
                                           << USBSSP_SLOT_CXT_PORT_NUM_POS);

    // set USB device address
    res->inputContext->slot[3] = cpuToLe32((uint32_t)res->devAddress);

    // set default endpoint context
    res->inputContext->ep0Context[1] = cpuToLe32(
        ((uint32_t)USBSSP_EP_CXT_EPTYP_CTL_BI
         << USBSSP_EP_CONTEXT_EP_TYPE_POS) |
        ((uint32_t)epMaxPacketSize << USBSSP_EP_CXT_MAX_PKT_SZ_POS) |
        (USBSSP_EP_CONTEXT_3ERR
         << USBSSP_EP_CONTEXT_CERR_POS));  // endpoint 0 control type, max
                                           // packet, error count

    set64Value(&res->inputContext->ep0Context[2],
               &res->inputContext->ep0Context[3],
               cpuToLe64(get64PhyAddrOf32ptr(&res->ep0.enqueuePtr->dword0) |
                         res->ep0.toogleBit));

    res->ep0.contextIndex = 1;

    res->inputContext->ep0Context[4] = cpuToLe32(
        ((uint32_t)(USBSSP_EP_CXT_EP_CTL_AVGTRB_LEN
                    << USBSSP_EP_CXT_EP_AVGTRBLEN_POS)));  // endpoint 0 control
                                                           // type - average trb
                                                           // length must be set
                                                           // to 8.
}

/**
 * Set address. Function executes set address request on connected device
 * @param[in] res driver resources
 */
static void setAddress (USBSSP_DriverResourcesT *res, uint8_t bsrVal)
{
    // If slot is not enabled (SLOT_ENABLED not completed) - ending
    if (res->actualdeviceSlot != 0U)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "setAddress(): actual port: %d, BSR: %d\n", res->actualPort,
                bsrVal);
        setSpeed(res);

        (void)memset((void *)res->inputContext, 0, sizeof(USBSSP_InputContexT));

        // On a set address also reserve the memory for the output context
        (void)memset((void *)res->xhciMemRes->outputContext, 0,
                     sizeof(USBSSP_OutputContexT));

        // set input control context: A0 and A1, all Dx should be 0
        res->inputContext->inputControlContext[0] = 0;  // Dx = 0
        res->inputContext->inputControlContext[1] = cpuToLe32(
            3);  // A0/A1=1/1, all other Ax = 0

        /* set slot context */
        setSlotContext(res);

        // send command to XHCI
        setAddressCommandTrb(res, bsrVal);

        if (res->usbsspCallbacks.inputContextCallback != NULL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                    "<%d> Calling inputContextCallback()\n", res->instanceNo);
            res->usbsspCallbacks.inputContextCallback(res);
        }
        (void)memcpy(&res->inputContextCopy, res->inputContext,
                     sizeof(USBSSP_InputContexT));

        updateQueuePtr(&res->commandQ, 0U, "CMD.SET_ADDRESS.");
        //printf("setaddress\n");
        hostCmdDoorbell(res);
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "setAddress() Warning: res->actualdeviceSlot: %d\n",
                res->actualdeviceSlot);
    }
}

/**
 * No Operation test. Function used for testing purposes
 * @param[in] res driver resources, internal function
 */
static void noOpTest (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> NOOP test\n", res->instanceNo);
    res->commandQ.enqueuePtr->dword0 = 0;
    res->commandQ.enqueuePtr->dword1 = 0;
    res->commandQ.enqueuePtr->dword2 = 0;
    res->commandQ.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((USBSSP_TRB_NO_OP_COMMAND << USBSSP_TRB_TYPE_POS) |
                   (uint32_t)res->commandQ.toogleBit));
    // update queue pointer
    updateQueuePtr(&res->commandQ, 0U, "CMD.NO.OP.TEST.");
    hostCmdDoorbell(res);
}

/**
 * No Operation test. Function used for testing purposes: NO_OP_COMMAND is send
 * to SSP controller. When event ring receives NO_OP_COMMAND complete it calls
 * complete callback
 *
 * @param[in] res driver resources
 * @complete complete callback pointer
 */
uint32_t USBSSP_NoOpTest (USBSSP_DriverResourcesT *res,
                          USBSSP_NopComplete complete)
{
    // check if res is not NULL
    uint32_t ret = USBSSP_NoOpTestSF(res);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        // set complete callback
        res->nopComplete = complete;
        noOpTest(res);
    }

    return ret;
}

/**
 * Function creates TRB for setup request - only host mode
 * @param res driver resources
 * @param setup pointer to setup packet
 * @param dataLength data length
 * @param dir data direction flag
 */
static void nbControlTransferSetup (USBSSP_DriverResourcesT *res,
                                    const CH9_UsbSetup *setup,
                                    uint16_t dataLength, uint8_t dir)
{
    uint32_t trt = 0;

    // setup TRB
    res->ep0.enqueuePtr->dword0 = cpuToLe32(
        ((uint32_t)le16ToCpu(setup->wValue) << USBSSP_TRB_WVALUE_POS) |
        ((uint32_t)setup->bRequest << USBSSP_TRB_BREQUEST_POS) |
        (uint32_t)setup->bmRequestType);

    res->ep0.enqueuePtr->dword1 = cpuToLe32(
        ((uint32_t)le16ToCpu(setup->wLength) << USBSSP_TRB_WLENGTH_POS) |
        (uint32_t)le16ToCpu(setup->wIndex));

    res->ep0.enqueuePtr->dword2 = cpuToLe32(
        (uint32_t)8U);  // setup is always 8 bytes length

    if (dataLength > 0U)
    {
        if (dir == 0U)
        {
            trt = USBSSP_TRB_SETUP_TRT_OUT_DATA << USBSSP_TRB_SETUP_TRT_POS;
        }
        else
        {
            trt = USBSSP_TRB_SETUP_TRT_IN_DATA << USBSSP_TRB_SETUP_TRT_POS;
        }
    }

    res->ep0.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((USBSSP_TRB_SETUP_STAGE << USBSSP_TRB_TYPE_POS) |
                   USBSSP_TRB_NORMAL_IDT_MASK | (uint32_t)res->ep0.toogleBit) |
        trt);
    updateQueuePtr(&res->ep0, 0U, "EP0.CTRL.XFER.SETUP.");
}

/**
 * function handles data phase of setup request in host mode
 * @param res driver resources
 * @param pdata pointer to data
 * @param dataLength data length
 * @param dir data direction flag
 */
void nbControlTransferData (USBSSP_DriverResourcesT *res, const uint8_t *pdata,
                            uint16_t dataLength, uint8_t dir)
{
    uint32_t iocFlag = (res->deviceModeFlag == 0U) ? USBSSP_TRB_NORMAL_IOC_MASK
                                                   : 0U;
    // set data buffer address
    set64Value(&res->ep0.enqueuePtr->dword0, &res->ep0.enqueuePtr->dword1,
               cpuToLe64(get64PhyAddrOf8ptr(pdata)));

    // set data length
    res->ep0.enqueuePtr->dword2 = cpuToLe32((uint32_t)dataLength);

    // set flags
    res->ep0.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)((uint32_t)dir << USBSSP_TRANSFER_DIR_POS) |
                   (USBSSP_TRB_DATA_STAGE << USBSSP_TRB_TYPE_POS) | iocFlag |
                   (uint32_t)res->ep0.toogleBit));

    updateQueuePtr(&res->ep0, 0U, "EP0.CTRL.XFER.DATA.");
}

/**
 * Enqueues non-blocking control transfer request
 * @param[in] res driver resources
 * @param[in] setup keeps setup packet
 * @param[in] pdata pointer for data to send/receive
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static void enqueueNBControlTransfer (USBSSP_DriverResourcesT *res,
                                      const CH9_UsbSetup *setup,
                                      const uint8_t *pdata)
{
    uint8_t dir;
    uint16_t dataLength;
    uint8_t dataInFlag = 0U;

    /* see spec in 4.11.2.2 */
    dir = setup->bmRequestType & CH9_USB_EP_DIR_IN;
    /* check data phase direction: set 1 for IN and 0 for OUT */
    dir = (dir > 0U) ? 1U : 0U;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "<%d> START NO BLOCKING CONTROL TRANSFER\n", res->instanceNo);
    dataLength = le16ToCpu(setup->wLength);

    /* handle setup stage without IOC */
    nbControlTransferSetup(res, setup, dataLength, dir);

    /* data TRB when exists */
    if (dataLength > 0U)
    {
        /* handle data stage without IOC */
        nbControlTransferData(res, pdata, dataLength, dir);
        if (dir > 0U)
        {
            dataInFlag = 1U;
        }
    }

    dir = (dataInFlag > 0U) ? 0U : 1U;

    /* status TRB - This is the only TRB with IOC */
    res->ep0.enqueuePtr->dword0 = 0;
    res->ep0.enqueuePtr->dword1 = 0;
    res->ep0.enqueuePtr->dword2 = 0;
    res->ep0.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)((uint32_t)dir << USBSSP_TRANSFER_DIR_POS) |
                   (USBSSP_TRB_STATUS_STAGE << USBSSP_TRB_TYPE_POS) |
                   USBSSP_TRB_NORMAL_IOC_MASK | (uint32_t)res->ep0.toogleBit));

    updateQueuePtr(&res->ep0, 0U, "EP0.CTRL.STATUS_STAGE.");
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> DRBL: Ring doorbell on EP0\n",
            res->instanceNo);
    USBSSP_WriteDoorbell(res, res->actualdeviceSlot, res->ep0.contextIndex);
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * Control transfer. Function executes control transfer. Information about
 * transfer like: data direction, data length, wIndex, wValue etc. are passed in
 * 'setup' parameter. No blocking version, result is returned to callback
 * function
 *
 * @param[in] res driver resources
 * @param[in] setup keeps setup packet
 * @param[in] pdata pointer for data to send/receive
 *
 */
uint32_t USBSSP_NBControlTransfer (USBSSP_DriverResourcesT *res,
                                   const CH9_UsbSetup *setup,
                                   const uint8_t *pdata,
                                   USBSSP_Complete complete)
{
    uint32_t ret = USBSSP_NBControlTransferSF(res, setup);
    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        /* check if transfers are enabled on this endpoint */
        ret = checkEpXferEnabled(res, 1);
    }
    if (ret == CDN_EOK)
    {
        res->ep0.complete = complete;
        res->ep0.isRunningFlag = 1;
        enqueueNBControlTransfer(res, setup, pdata);
    }
    return ret;
}

/**
 * Control transfer. Function executes control transfer. Information about
 * transfer like: data direction, data length, wIndex, wValue etc. are passed in
 * 'setup' parameter.
 *
 * @param[in] res driver resources
 * @param[in] setup keeps setup packet
 * @param[in] pdata pointer for data to send/receive
 *
 * @return CDN_EOK on success
 * @return complete_code XHCI transfer complete status code*
 */
uint32_t USBSSP_ControlTransfer (USBSSP_DriverResourcesT *res,
                                 const CH9_UsbSetup *setup,
                                 const uint8_t *pdata)
{
    uint32_t ret = USBSSP_ControlTransferSF(res, setup);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        // wait until device in addressed state
        ret = waitUntilAddressState(res, USBSSP_DEFAULT_TIMEOUT);
    }

    if (ret == CDN_EOK)
    {
        ret = USBSSP_NBControlTransfer(res, setup, pdata, NULL);
        if (ret == (uint32_t)CDN_EINVAL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Critical error! wrong value in one of function "
                    "parameters\n",
                    res->instanceNo);
        }

        // wait for response
        ret = waitForResponse(res, USBSSP_DEFAULT_TIMEOUT);
    }
    if (ret == CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> CONTROL TRANSFER completed.\n",
                res->instanceNo);

        // check result and translate from XHCI to Cadence error code
        ret = getCompletionCode(res->ep0.completePtr);
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Completion Code: %d\n",
                res->instanceNo, ret);
        if (ret == (uint32_t)USBSSP_TRB_COMPLETE_SUCCESS)
        {
            ret = CDN_EOK;
        }
    }

    return ret;
}

/**
 * Function handles data phase of device setup request
 * @param res driver resources
 * @param pdata pointer to memory where data is/will be stored
 * @param length data length
 * @param dirFlag data direction flag
 * @return CDN_EOK if success, error code elsewhere
 */
static uint32_t controlXferDevDataPhase (USBSSP_DriverResourcesT *res,
                                         const uint8_t *pdata, uint32_t length,
                                         uint8_t dirFlag)
{
    uint32_t ret = CDN_EOK;
    uint32_t speedId = (res->actualSpeed > CH9_USB_SPEED_HIGH)
                           ? USBSSP_TRB_SPEED_ID_3
                           : USBSSP_TRB_SPEED_ID_2;
    uint64_t dataAddrPhy = cpuToLe64(get64PhyAddrOf8ptr(pdata));

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "<%d> Control Xfer Data phase - device mode, pdata: %p, length: "
            "%d, dir: %d\n",
            res->instanceNo, dataAddrPhy, length, dirFlag);
    // update default endpoint state
    res->ep0State = USBSSP_EP0_DATA_PHASE;
    // set data buffer in TRB
    set64Value(&res->ep0.enqueuePtr->dword0, &res->ep0.enqueuePtr->dword1,
               dataAddrPhy);

    // set data length in TRB
    res->ep0.enqueuePtr->dword2 = cpuToLe32((uint32_t)length);

    // set flags in TRB
    res->ep0.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)((uint32_t)dirFlag << USBSSP_TRANSFER_DIR_POS) |
                   USBSSP_TRB_NORMAL_IOC_MASK |
                   (USBSSP_TRB_DATA_STAGE << USBSSP_TRB_TYPE_POS) |
                   (uint32_t)res->ep0.toogleBit) |
        (uint32_t)((uint32_t)res->setupID << USBSSP_TRB_SETUPID_POS) | speedId);

    updateQueuePtr(&res->ep0, 0U, "EP0.CTRL.DATA_STAGE.");

    return ret;
}

/**
 * Handles Status Phase of control Xfer
 * @param res driver resources
 * @param statusResp Indicates status response 1: ACK 0: Stall
 */
static void controlXferDevStatusPhase (USBSSP_DriverResourcesT *res,
                                       uint32_t statusResp)
{
    uint32_t speedId = (res->actualSpeed > CH9_USB_SPEED_HIGH)
                           ? USBSSP_TRB_SPEED_ID_3
                           : USBSSP_TRB_SPEED_ID_2;

    vDbgMsg(
        USBSSP_DBG_DRV, DBG_FYI,
        "<%d> Control Xfer Status phase - device mode, statusResp: 0x%08X\n",
        (statusResp << USBSSP_TRB_STS_STG_STAT_POS));

    // create status TRB
    res->ep0State = USBSSP_EP0_STATUS_PHASE;
    res->ep0.enqueuePtr->dword0 = 0;
    res->ep0.enqueuePtr->dword1 = 0;
    res->ep0.enqueuePtr->dword2 = 0;
    res->ep0.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((USBSSP_TRB_STATUS_STAGE << USBSSP_TRB_TYPE_POS) |
                   USBSSP_TRB_NORMAL_IOC_MASK | (uint32_t)res->ep0.toogleBit) |
        (statusResp << USBSSP_TRB_STS_STG_STAT_POS) |
        (uint32_t)((uint32_t)res->setupID << USBSSP_TRB_SETUPID_POS) | speedId);

    updateQueuePtr(&res->ep0, 0U, "EP0.CTRL.STATUS.");
}

/**
 * Control transfer in device mode. Function used in response to setup event
 *
 * @param[in] res driver resources
 * @param[in] pdata pointer for data to send/receive
 * @param[in] length data length
 *
 * @return CDN_EOK on success
 * @return complete_code XHCI transfer complete status code*
 */
uint32_t USBSSP_ControlTransferDev (USBSSP_DriverResourcesT *res,
                                    const uint8_t *pdata, uint32_t length,
                                    uint8_t dirFlag)
{
    // check input parameter correctness
    uint32_t ret = USBSSP_ControlTransferDevSF(res);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        // check if data phase exist
        if (length > 0U)
        {
            ret = controlXferDevDataPhase(res, pdata, length, dirFlag);
            CPS_CacheInvalidate((void*)pdata,length,0);
        }
        else
        {
            // send status TRB
            controlXferDevStatusPhase(res, USBSSP_TRB_STS_STG_STAT_ACK);
        }

        res->ep0.isRunningFlag = 1;
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> DRBL: Ring doorbell on EP0\n",
                res->instanceNo);

        // depending on data transfer direction write 0 or 1 to DRBL for ep0
        // this quirk applies for device mode only
        if ((length > 0U) && (dirFlag == 0U))
        {
            USBSSP_WriteDoorbell(res, res->actualdeviceSlot, 0U);
        }
        else
        {
            USBSSP_WriteDoorbell(res, res->actualdeviceSlot,
                                 res->ep0.contextIndex);
        }
        CPS_MemoryBarrier();
    }

    return ret;
}

/**
 * Handling of hub request
 * @param res driver resources
 * @param descType descriptor type
 * @param setup pointer to setup request
 */
static void getDescriptorHubReq (uint8_t descType, uint8_t *setup)
{
    // Hub requests are of Type Class.
    if (descType == CH9_USB_DT_USB2_HUB)
    {
        setup[0] = (uint8_t)(CH9_USB_DIR_DEVICE_TO_HOST |
                             CH9_USB_REQ_TYPE_CLASS |
                             CH9_USB_REQ_RECIPIENT_DEVICE);
        // length in little endian on bytes: 6 and 7
        setup[6] = CH9_USB_DS_USB2_HUB;
        setup[7] = 0x0U;
    }
    else if (descType == CH9_USB_DT_USB3_HUB)
    {
        setup[0] = (uint8_t)(CH9_USB_DIR_DEVICE_TO_HOST |
                             CH9_USB_REQ_TYPE_CLASS |
                             CH9_USB_REQ_RECIPIENT_DEVICE);
        // length in little endian on bytes: 6 and 7
        setup[6] = CH9_USB_DS_USB3_HUB;
        setup[7] = 0x0U;
    }
    else
    {
        setup[0] = (uint8_t)(CH9_USB_DIR_DEVICE_TO_HOST |
                             CH9_USB_REQ_TYPE_STANDARD |
                             CH9_USB_REQ_RECIPIENT_DEVICE);
    }
}

/**
 *
 * @param setupData Pointer to the uint8_t * buffer having setup data
 * @param setup pointer to the output struct CH9_UsbSetup
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static void constructCH9setup (const uint8_t *setupData, CH9_UsbSetup *ch9setup)
{
    // map the bytes to the struct values
    ch9setup->bmRequestType = setupData[0];
    ch9setup->bRequest = setupData[1];
    ch9setup->wValue = getU16ValFromU8Ptr(&setupData[2]);
    ch9setup->wIndex = getU16ValFromU8Ptr(&setupData[4]);
    ch9setup->wLength = getU16ValFromU8Ptr(&setupData[6]);
}
/* parasoft-end-suppress METRICS-36-3 */

/**
 * This function is called in ISR context to process the final get description
 * control transfer completion
 * @param res Pointer to driver private data
 * @param status Status of the transfer
 * @param eventPtr Pointer to the completed event
 */
static void xhciGetDescXferComplete (USBSSP_DriverResourcesT *res,
                                     uint32_t status,
                                     const USBSSP_RingElementT *eventPtr,
                                     uint8_t *buffer, uint32_t actualLength)
{
    uint32_t retStatus = CDN_EOK;
    if (status == CDN_EOK)
    {
        /* check result and translate from XHCI to Cadence error code */
        retStatus = getCompletionCode(eventPtr);
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Completion Code: %d\n",
                res->instanceNo, retStatus);
        if (retStatus == (uint32_t)USBSSP_TRB_COMPLETE_SUCCESS)
        {
            retStatus = CDN_EOK;
        }
    }
    else
    {
        retStatus = status;
    }

    /* this is the final stage of the aggregated transfer */
    if (res->ep0.aggregatedComplete != NULL)
    {
        USBSSP_Complete complete = res->ep0.aggregatedComplete;
        /* Clear the complete pointer since the callback
         * could queue in another blocking transfer */
        res->ep0.aggregatedComplete = NULL;
        complete(res, retStatus, eventPtr, buffer, actualLength);
    }
}

/**
 * This function is called in ISR context to process the final get description
 * control transfer completion
 * @param res Pointer to driver private data
 * @param status Status of the transfer
 * @param eventPtr Pointer to the completed event
 */
static void xhciGetShortCfgDescComplete (USBSSP_DriverResourcesT *res,
                                         uint32_t status,
                                         const USBSSP_RingElementT *eventPtr,
                                         uint8_t *buffer, uint32_t actualLength)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "<%d> Get short configuration status(%d)\n", res->instanceNo,
            status);

    /* check if the request was success */
    if (status == CDN_EOK)
    {
        CH9_UsbSetup ch9setup;
        uint16_t confLength;
        uint8_t byte_l, byte_h;
        uint8_t setup[] = {0x00U, (uint8_t)CH9_USB_REQ_GET_DESCRIPTOR,
                           0x00U, 0x00U,
                           0x00U, 0x00U,
                           0x00U, 0x02U};

        /* replace descriptor type with required one */
        setup[3] = (uint8_t)CH9_USB_DT_CONFIGURATION;
        setup[0] = (uint8_t)(CH9_USB_DIR_DEVICE_TO_HOST |
                             CH9_USB_REQ_TYPE_STANDARD |
                             CH9_USB_REQ_RECIPIENT_DEVICE);

        byte_l = res->ep0Buff[2];
        byte_h = res->ep0Buff[3];

        confLength = ((uint16_t)byte_h << 8) | (uint16_t)byte_l;

        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Whole configuration length: %d\n", res->instanceNo,
                confLength);

        /* write length in LE order */
        setup[6] = (uint8_t)confLength;
        setup[7] = (uint8_t)(confLength >> 8);
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Get long configuration\n",
                res->instanceNo);

        res->ep0.isRunningFlag = 1U;
        res->ep0.complete = &xhciGetDescXferComplete;
        constructCH9setup(&setup[0], &ch9setup);

        enqueueNBControlTransfer(res, &ch9setup, res->ep0Buff);
    }
    else if (res->ep0.aggregatedComplete != NULL)
    {
        USBSSP_Complete complete = res->ep0.aggregatedComplete;
        /* Clear the complete pointer since the callback
         * could queue in another blocking transfer */
        res->ep0.aggregatedComplete = NULL;
        complete(res, status, eventPtr, buffer, actualLength);
    }
    else
    {
        /* Do nothing if error and no-aggregate callback is registered */
    }
}

/**
 * Function gets short configuration
 * @param res driver resources
 * @param setup pointer to setup request
 * @return CDN_EOK if success, error code elsewhere
 */
static uint32_t getDescriptorShortConf (USBSSP_DriverResourcesT *res,
                                        uint8_t *setup)
{
    uint32_t ret = CDN_EOK;
    CH9_UsbSetup ch9setup;

    /* length in little endian on bytes: 6 and 7 */
    setup[6] = CH9_USB_DS_CONFIGURATION;
    setup[7] = 0;

    constructCH9setup(&setup[0], &ch9setup);
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Get short configuration\n",
            res->instanceNo);
    enqueueNBControlTransfer(res, &ch9setup, res->ep0Buff);

    return ret;
}

/**
 * Get descriptor. Function gets descriptor from connected device, used in host
 * mode and stores it in internal res->ep0Buff buffer. Maximal descriptor length
 * is limited to 255. Function is blocking type and must not be called from
 * interrupt context.
 *
 * @param[in] res driver resources
 * @param[in] descType type of descriptor to get (CH9_USB_DT_DEVICE,
 * CH9_USB_DT_CONFIGURATION,...)
 *
 * @return CDN_EOK on success
 * @return complete_code XHCI transfer complete status code
 */
uint32_t USBSSP_GetDescriptor (USBSSP_DriverResourcesT *res, uint8_t descType,
                               USBSSP_Complete complete)
{
    uint32_t ret = USBSSP_GetDescriptorSF(res);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        /* check if transfers are enabled on this endpoint */
        ret = checkEpXferEnabled(res, 1);
    }

    if (ret == CDN_EOK)
    {
        // GET configuration descriptor template with maximal length = 255
        uint8_t setup[] = {0x00U, (uint8_t)CH9_USB_REQ_GET_DESCRIPTOR,
                           0x00U, 0x00U,
                           0x00U, 0x00U,
                           0x00U, 0x02U};

        // replace descriptor type with required one
        setup[3] = descType;

        // handle hub requests
        getDescriptorHubReq(descType, setup);

        /* configuration descriptor should be handled in two steps: */
        /* 1. get short configuration = 9 bytes */
        /* 2. get long configuration with length encoded in short configuration
         */
        res->ep0.isRunningFlag = 1U;
        res->ep0.aggregatedComplete = complete;

        if (descType == CH9_USB_DT_CONFIGURATION)
        {
            /* handle short configuration */
            res->ep0.complete = &xhciGetShortCfgDescComplete;
            ret = getDescriptorShortConf(res, setup);
        }
        else
        {
            CH9_UsbSetup ch9setup;
            constructCH9setup(&setup[0], &ch9setup);
            res->ep0.complete = &xhciGetDescXferComplete;
            enqueueNBControlTransfer(res, &ch9setup, res->ep0Buff);
        }
    }
#if 0
                USBSSP_Isr((USBSSP_DriverResourcesT *)res);
#endif
    return ret;
}

/**
 * USBSSP_ControlIn/OutTest.
 * @param[in] res Driver resources
 * @param[in] data Pointer for data to send/receive
 * @return CDN_EOK on success
 */
uint32_t USBSSP_ControlInTest (USBSSP_DriverResourcesT *res, uint8_t *data)
{
    CH9_UsbSetup ch9setup;

    uint8_t setup[] = {0x80U, (uint8_t)CH9_USB_REQ_GET_DESCRIPTOR,
                       0x00U, CH9_USB_DT_STRING,
                       0x00U, 0x00U,
                       0x00U, 0x02U};

    constructCH9setup(&setup[0], &ch9setup);
    enqueueNBControlTransfer(res, &ch9setup, data);
    return 0;
}

uint32_t USBSSP_ControlOutTest (USBSSP_DriverResourcesT *res, uint8_t *data)
{
    CH9_UsbSetup ch9setup;

    uint8_t setup[] = {CH9_USB_DIR_HOST_TO_DEVICE | CH9_USB_REQ_TYPE_STANDARD |
                           CH9_USB_REQ_RECIPIENT_ENDPOINT,
                       CH9_USB_REQ_SET_FEATURE,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x08,
                       0x00};

    constructCH9setup(&setup[0], &ch9setup);
    enqueueNBControlTransfer(res, &ch9setup, data);
    return 0;
}

/*
 * Sets feature in device mode
 */
static void endpointSetFeatureDev (USBSSP_DriverResourcesT *res,
                                   uint8_t epIndex, uint8_t feature)
{
    // handle in device mode
    /*    uint32_t expectedCommand; */

    if (feature > 0U)
    {
        // set stall
        res->commandQ.enqueuePtr->dword0 = 0;
        res->commandQ.enqueuePtr->dword1 = 1;
        res->commandQ.enqueuePtr->dword2 = 2;
        res->commandQ.enqueuePtr->dword3 = cpuToLe32(
            (uint32_t)((uint32_t)res->actualdeviceSlot << USBSSP_SLOT_ID_POS) |
            (uint32_t)((uint32_t)epIndex << USBSSP_ENDPOINT_POS) |
            (uint32_t)(USBSSP_TRB_HALT_ENDP_CMD << USBSSP_TRB_TYPE_POS) |
            res->commandQ.toogleBit);
        updateQueuePtr(&res->commandQ, 0U, "CMD.HALT_ENDP.");
        res->commandQ.isRunningFlag = 1;
        hostCmdDoorbell(res);
    }
    else
    {
        // clear stall
        /*        expectedCommand = USBSSP_TRB_RESET_EP_CMD; */
        (void)USBSSP_ResetEndpoint(res, epIndex);
        // TODO return correct error number
        // ret = ....?
    }
}

/*
 * Sets feature in host mode
 */
static uint32_t endpointSetFeatureHost (USBSSP_DriverResourcesT *res,
                                        uint8_t epIndex, uint8_t feature)
{
    uint8_t epAddress;
    uint32_t ret;
    CH9_UsbSetup ch9setup;

    // GET configuration descriptor pattern
    uint8_t setup[] = {CH9_USB_DIR_HOST_TO_DEVICE | CH9_USB_REQ_TYPE_STANDARD |
                           CH9_USB_REQ_RECIPIENT_ENDPOINT,
                       CH9_USB_REQ_CLEAR_FEATURE,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00};

    // calculate endpoint address from ep_index
    epAddress = (epIndex / 2U) |
                (((epIndex % 2U) != 0U) ? CH9_USB_EP_DIR_IN : 0U);
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Endpoint address: %02X\n",
            res->instanceNo, epAddress);

    // replace endpoint selector with required one
    setup[4] = epAddress;

    // replace clear feature request with set feature request
    if (feature > 0U)
    {
        setup[1] = CH9_USB_REQ_SET_FEATURE;
    }

    constructCH9setup(&setup[0], &ch9setup);
    ret = USBSSP_ControlTransfer(res, &ch9setup, res->ep0Buff);
    // TODO: What if ret != CDN_EOK?

    // check if any data queued to transfer
    if (feature == 0U)
    {
        USBSSP_EpContexEpState endpointState = getEndpointStatus(res, epIndex);
        // put endpoint to run state
        if (endpointState == USBSSP_EP_CONTEXT_EP_STATE_STOPPED)
        {
            USBSSP_WriteDoorbell(res, res->actualdeviceSlot,
                                 res->ep[epIndex].contextIndex);
        }
    }
    return ret;
}

/**
 * Set feature on device's endpoint. Functions sends setup requested to device
 * with set/cleared endpoint feature
 *
 * @param[in] res driver resources
 * @param[in] epIndex index of endpoint to set/clear feature on
 * @param[in] feature when 1 sets stall, when 0 clears stall
 *
 * @return CDN_EOK on success
 * @return complete_code XHCI transfer complete status code
 */
uint32_t USBSSP_EndpointSetFeature (USBSSP_DriverResourcesT *res,
                                    uint8_t epIndex, uint8_t feature)
{
    // check parameter correctness
    uint32_t ret = USBSSP_EndpointSetFeatureSF(res, epIndex);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        if (feature > 0U)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Set Feature on ep:%d\n",
                    res->instanceNo, epIndex);
        }
        else
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Clear Feature on ep:%d\n",
                    res->instanceNo, epIndex);
        }

        // call handle feature
        if (res->deviceModeFlag == 0U)
        {
            ret = endpointSetFeatureHost(res, epIndex, feature);
        }
        else
        {
            endpointSetFeatureDev(res, epIndex, feature);
        }
    }

    return ret;
}

/**
 * Enqueue Setup TRB for SET_CONFIGURATION setup request
 * @param res driver resources
 * @param configValue onfiguration value
 */
static void enqueueSetCfgSetupTRB (USBSSP_DriverResourcesT *res,
                                   uint32_t configValue)
{
    /* clear trb */
    (void)memset((void *)res->ep0.enqueuePtr, 0, sizeof(USBSSP_RingElementT));

    /* setup TRB */
    res->ep0.enqueuePtr->dword0 = cpuToLe32(
        ((uint32_t)configValue << USBSSP_TRB_WVALUE_POS) |
        ((uint32_t)CH9_USB_DS_CONFIGURATION << USBSSP_TRB_BREQUEST_POS) |
        (uint8_t)(CH9_USB_DIR_HOST_TO_DEVICE | CH9_USB_REQ_TYPE_STANDARD |
                  CH9_USB_REQ_RECIPIENT_DEVICE));

    res->ep0.enqueuePtr->dword1 = 0;

    res->ep0.enqueuePtr->dword2 = cpuToLe32((uint32_t)(8U)); /* TRB length */

    res->ep0.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((USBSSP_TRB_SETUP_STAGE << USBSSP_TRB_TYPE_POS) |
                   USBSSP_TRB_NORMAL_IDT_MASK | (uint32_t)res->ep0.toogleBit));
}

/**
 * Function issues SET_CONFIGURATION setup request to connected device
 * @param res driver resources
 * @param configValue configuration value
 */
static void setConfigurationSetupReq (USBSSP_DriverResourcesT *res)
{
    uint32_t configValue = (uint32_t)res->configValue;
    USBSSP_ProducerQueueT *ep0 = &res->ep0;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Setting device config: %d\n",
            res->instanceNo, configValue);

    ep0->lastXferActualLength = 0U;
    ep0->lastXferBufferPhyAddr = 0U;

    /* Enqueue Setup TRB for SET_CONFIGURATION setup request */
    enqueueSetCfgSetupTRB(res, configValue);
    updateQueuePtr(ep0, 0U, "EP0.SET_CFG.SETUP.");

    // status TRB
    ep0->enqueuePtr->dword0 = 0;
    ep0->enqueuePtr->dword1 = 0;
    ep0->enqueuePtr->dword2 = 0;
    ep0->enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)(USBSSP_TRB_STATUS_STAGE << USBSSP_TRB_TYPE_POS) |
        (uint32_t)USBSSP_TRB_NORMAL_IOC_MASK | (uint32_t)res->ep0.toogleBit |
        ((uint32_t)1 << USBSSP_TRANSFER_DIR_POS));
    updateQueuePtr(ep0, 0U, "EP0.SET_CFG.DATA.");

    res->ep0.isRunningFlag = 1;
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> DRBL: Ring doorbell on EP0\n",
            res->instanceNo);
    USBSSP_WriteDoorbell(res, res->actualdeviceSlot, res->ep0.contextIndex);

    return;
}

/**
 * Set configuration. Function configures SSP controller as well as device
 * connected to this SSP controller. Function must not be called from interrupt
 * context.
 *
 * @param[in] res driver resources
 * @param[in] configValue USB device's configuration selector
 *
 * @return CDN_EOK on success
 * @return complete_code XHCI transfer complete status code
 */
uint32_t USBSSP_SetConfiguration (USBSSP_DriverResourcesT *res,
                                  uint32_t configValue, uint8_t *epCfgBuffer,
                                  uint16_t epCfgBufferLen,
                                  USBSSP_Complete complete)
{
    // check the parameters
    uint32_t ret = USBSSP_SetConfigurationSF(res, epCfgBuffer);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else if (checkEpXferEnabled(res, 1U) != CDN_EOK)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "<%d> Critical error! EP0 not ready \n", res->instanceNo);
        ret = CDN_EIO;
    }
    else
    {
        uint8_t *buffer = epCfgBuffer;
        uint16_t length = epCfgBufferLen;
        if (buffer == NULL)
        {
            buffer = res->ep0Buff;
            length = ((uint16_t)buffer[3] << 8) | (uint16_t)buffer[2];
        }

        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> buffer:0x%X length:%d\n",
                res->instanceNo, buffer, length);

        res->ep0.aggregatedComplete = NULL;
        res->ep0.complete = complete;
        res->configValue = (uint16_t)configValue;

        // configure endpoints
        ret = configureEndpoints(res, buffer, length);
    }

    return ret;
}

/**
 * Calculate full/low speed endpoint interval based on bInterval
 * See xHCI spec Section 6.2.3.6 for more details.
 * @param[in] bInterval
 * @return valid endpoint context interval value
 */
uint8_t USBSSP_CalcFsLsEPIntrptInterval (uint8_t bInterval)
{
    uint8_t interval = 2U;
    uint8_t bitOffset;  // register with '1' circulating
    uint8_t res1;  // used for finding the most significant 1 from left to right
    uint8_t
        res2 = 0U;  // used for finding the first 1 looking from right to left

    if (bInterval > 0U)
    {
        // find the oldest bit
        bitOffset = 0x80U;
        do
        {
            res1 = bInterval & bitOffset;
            bitOffset >>= 1U;
        } while ((res1 == 0U) && (bitOffset > 0U));

        // calculate context interrupt value
        bitOffset = 0x01;
        do
        {
            res2 = res1 & bitOffset;
            ++interval;
            bitOffset <<= 1U;
        } while (res2 == 0U);
    }
    else
    {
        interval = 0U;  // what to return if bInterval is zero?
    }

    return interval;
}

#ifdef DEBUG

/**
 * returns epType string
 * @param epType given in XHCI spec convention
 * @return string with type description
 */
static char *epTypeStr (uint8_t epType)
{
    char *retStr;

    char *stringArray[] = {"NULL",    "ISOCH-OUT", "BULK-OUT", "INTERRUPT-OUT",
                           "CONTROL", "ISOCH-IN",  "BULK-IN",  "INTERRUPT-IN"};
    if (epType < 8)
    {
        retStr = stringArray[epType];
    }
    else
    {
        retStr = "UNKNOWN Endpoint type";
    }
    return (retStr);
}
#endif

/**
 * Function sets MULT field in endpoint context structure
 * @param epObj
 */
static void setMult (USBSSP_ProducerQueueT *epObj)
{
    uint8_t mult = 0U;
    USBSSP_DriverResourcesT *res = epObj->parent;
    uint32_t lec;
    CH9_UsbSpeed actualSpeed = res->actualSpeed;
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;

    lec = CPS_FLD_READ(USBSSP__HCCPARAMS2, LEC,
                       (res->qaRegs.xHCCaps.hccparams2));

    // this filed is different from only if Large ESIT Payload is not supported
    if (lec == 0U)
    {
        if ((actualSpeed >= CH9_USB_SPEED_SUPER) &&
            (epDescType == CH9_USB_EP_ISOCHRONOUS))
        {
            // check if isochronous endpoint companion descriptor does not
            // exists
            if ((epObj->epDesc[10] & 0x80U) != 0x80U)
            {
                mult = epObj->epDesc[10] & 0x03U;
            }
        }
    }
    if (mult > 0U)
    {
        epObj->hwContext[0] |= (uint32_t)mult << USBSSP_EP_CONTEXT_MULT_POS;
    }
}

/**
 * Function initializes stream object
 * @param epObj endpoint object
 * @param stream stream object
 * @param iter iterator equal to (stream ID - 1)
 */
static void initStreamObj (const USBSSP_ProducerQueueT *epObj,
                           USBSSP_ProducerQueueT *stream, uint32_t sid)
{
    USBSSP_DriverResourcesT *res = epObj->parent;

    // initialize ring of stream
    stream->ring = (USBSSP_RingElementT *)(&(
        *res->xhciMemRes->streamRing)[epObj->contextIndex - 2U][sid][0]);
    stream->enqueuePtr = stream->ring;
    stream->dequeuePtr = stream->ring;
    stream->contextIndex = epObj->contextIndex;  // All child stream objects
                                                 // have parent's (endpoint)
                                                 // contextIndex
    stream->toogleBit = 1U;
    stream->actualSID = (uint16_t)sid + 1U;
    stream->eventSID = stream->actualSID;
    stream->interrupterIdx = 0U;  // Set interrupter to default
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> Memory allocated for epIndex: %02X, streamID (%d) ring: %p\n",
            res->instanceNo, stream->contextIndex, stream->actualSID,
            (void *)stream->ring);

    // update stream context - The parent TRB ring acts as Primary Stream Array
    // (PSA)
    set64Value(
        &epObj->ring[stream->actualSID].dword0,
        &epObj->ring[stream->actualSID].dword1,
        cpuToLe64(get64PhyAddrOf32ptr(&stream->enqueuePtr->dword0) | 0x02UL |
                  stream->toogleBit)  // SCT = 1, PRIMARY string, transfer ring,
                                      // spec 6.2.4.1
    );
}

/**
 * Function calculates max streams
 * @param res driver resources
 * @param epObj endpoint object
 */
static uint8_t calcMaxPsStreams (const USBSSP_DriverResourcesT *res,
                                 const USBSSP_ProducerQueueT *epObj)
{
    uint8_t hwMaxPStreams = (uint8_t)CPS_FLD_READ(
        USBSSP__HCCPARAMS1, MAXPSASIZE, res->qaRegs.xHCCaps.hccparams1);
    uint8_t maxPStreams = 0;
    // maxPStreams should be set to minimal o three factors:
    // MAX_STREMS_PER_EP, hccparams1, companion descriptor
    // first check if driver allows to use full hardware stream number and limit
    // if NO
    hwMaxPStreams = (hwMaxPStreams > USBSSP_MAX_STREMS_PER_EP)
                        ? USBSSP_MAX_STREMS_PER_EP
                        : hwMaxPStreams;

    // Then check if descriptor companion streams number exceeds hardware number
    // and limit if Yes
    maxPStreams = epObj->epDesc[10] & 0x1FU;
    maxPStreams = (maxPStreams > hwMaxPStreams) ? hwMaxPStreams : maxPStreams;
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> MAX_PSA_SIZE: %d\n",
            res->instanceNo, hwMaxPStreams);
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> maxPStreams: %d\n",
            res->instanceNo, maxPStreams);

    return maxPStreams;
}

/**
 * Function sets max streams
 * @param epObj endpoint object
 */
static void setMaxPStreams (USBSSP_ProducerQueueT *epObj)
{
    USBSSP_DriverResourcesT *res = epObj->parent;
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;

    if (epDescType == CH9_USB_EP_BULK)
    {
        uint32_t streamId;
        uint8_t maxPStreams = calcMaxPsStreams(res, epObj);

        // ------------------- initialize streams ------------------
        if (maxPStreams > 0U)
        {
            if ((epObj->contextIndex - 2U) < USBSSP_MAX_EP_NUM_STRM_EN)
            {
                epObj->streamCount = USBSSP_STREAM_ARRAY_SIZE;
                for (streamId = 0; streamId < USBSSP_STREAM_ARRAY_SIZE;
                     streamId++)
                {
                    USBSSP_ProducerQueueT *stream;
                    USBSSP_ProducerQueueT(
                        *streamObj)[USBSSP_MAX_EP_NUM_STRM_EN]
                                   [USBSSP_STREAM_ARRAY_SIZE] =
                                       res->xhciMemRes->streamMemoryPool;
                    // get reference to single stream object within stream
                    // container
                    epObj->stream[streamId] = &(
                        (*streamObj)[epObj->contextIndex - 2U][streamId]);
                    stream = (USBSSP_ProducerQueueT *)epObj->stream[streamId];

                    vDbgMsg(
                        USBSSP_DBG_DRV, DBG_HIVERB,
                        "<%d> Memory allocated for stream (%d) object: %p\n",
                        res->instanceNo, streamId + 1, (void *)stream);
                    initStreamObj(epObj, stream, streamId);
                }
                epObj->hwContext[0] |= ((uint32_t)maxPStreams
                                        << USBSSP_EP_CXT_PMAXSTREAMS_POS) |
                                       USBSSP_EP_CONTEXT_LSA_MASK;
            }
            else
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                        "<%d> maxPStreams(%d) for unsupported endpoint (%d)\n",
                        epObj->parent->instanceNo, maxPStreams,
                        (epObj->contextIndex));
            }
        }
    }
}

/**
 * Function sets interval field in endpoint context
 * @param epObj endpoint object
 */
static void setInterval (USBSSP_ProducerQueueT *epObj)
{
    uint8_t interval = epObj->epDesc[6];
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;

    // Convert interval to endpoint valid value (Table 65, section 6.2.3.6 of
    // the xHCI Spec).
    switch (epObj->parent->actualSpeed)
    {
    case CH9_USB_SPEED_LOW:
        if (epDescType == CH9_USB_EP_INTERRUPT)
        {
            // Table 65 - LS Interrupt - covert interval (1-255) to (3-10).
            interval = USBSSP_CalcFsLsEPIntrptInterval(interval);
        }
        // If none of the above leave interval unchanged as it should be zero
        // already.
        break;

    case CH9_USB_SPEED_FULL:
        // Table 65 - FS Isoch. - convert interval (1-16) to (3-18) i.e.
        // increment by 2.
        if (epDescType == CH9_USB_EP_ISOCHRONOUS)
        {
            interval = interval + 2U;
        }
        else
        {
            if (epDescType == CH9_USB_EP_INTERRUPT)
            {
                // Table 65 - FS Interrupt - covert interval (1-255) to (3-10).
                interval = USBSSP_CalcFsLsEPIntrptInterval(interval);
            }
        }
        // If none of the above leave interval unchanged.
        break;

    default:
        // Table 65 - SS or HS (Interrupt/Isoch) - convert interval (1-16) to
        // (0-15) i.e. decrement by 1. Bulk value will be left unchanged
        if (epDescType != CH9_USB_EP_BULK)
        {
            if (interval > 0U)
            {
                interval = interval - 1U;
            }
        }
        break;
    }
    if (interval > 0U)
    {
        epObj->hwContext[0] |= (uint32_t)interval
                               << USBSSP_EP_CONTEXT_INTERVAL_POS;
    }
}

/**
 * Sets Max ESIT payload for SS mode
 * @param epObj endpoint object
 * @param bytesPerInterval pointer to bytesPerInterval variable
 */
static void setMaxESITPayloadSS (const USBSSP_ProducerQueueT *epObj,
                                 uint32_t *bytesPerInterval)
{
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;
    const uint8_t *desc = epObj->epDesc;

    if (epDescType == CH9_USB_EP_ISOCHRONOUS)
    {
        // check if iso endpoint companion descriptor exists
        if ((desc[10] & 0x80U) == 0x80U)
        {
            /* we should get the SSP Isochronous Endpoint Companion Descriptor
             */
            if ((desc[13] == CH9_USB_DS_SSP_ISO_EP_COMPANION) &&
                (desc[14] == CH9_USB_DT_SSP_ISO_EP_COMPANION))
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                        "<%d> SSP Isochronous Endpoint companion found-\n",
                        epObj->parent->instanceNo);

                *bytesPerInterval = ((uint32_t)desc[20] << 24U) |
                                    ((uint32_t)desc[19] << 16U) |
                                    ((uint32_t)desc[18] << 8U) |
                                    (uint32_t)desc[17];
            }
            else
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                        "<%d> ERROR: Missing SSP Isochronous Endpoint "
                        "Companion Descriptor\n",
                        epObj->parent->instanceNo);
            }
        }
        else
        {
            *bytesPerInterval = ((uint32_t)desc[12] << 8) |
                                ((uint32_t)desc[11]);
        }
    }
    else if (epDescType == CH9_USB_EP_INTERRUPT)
    {
        // bytes 5 (11 when added endpoint descriptor ) and 6
        //(12 when added endpoint descriptor) of companion descriptor is
        // wBytesPerInterval value
        *bytesPerInterval = ((uint32_t)desc[12] << 8) | ((uint32_t)desc[11]);
    }
    else
    {
        // required by MISRA
    }
}

/**
 * calculate bytes per interval in term of LEC parameter
 * @param epObj endpoint object
 * @param bytesPerInterval pointer to bytesPerInterval variable
 */
static void setMaxESITPayLEC (const USBSSP_ProducerQueueT *epObj,
                              uint32_t *bytesPerInterval)
{
    // read LEC parameter from capabilities
    uint32_t hccparams2 = epObj->parent->qaRegs.xHCCaps.hccparams2;
    uint32_t lec;

    lec = CPS_FLD_READ(USBSSP__HCCPARAMS2, LEC, hccparams2);
    // update bytesPerInterval field depending on LED field
    if ((lec == 1U) && (*bytesPerInterval > (64U * 1024U)))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                "<%d> WARNING: LEC(%d) bytesPerInterval(%d) > 64KB, Patching "
                "to 64KB\n",
                epObj->parent->instanceNo, lec, *bytesPerInterval);
        *bytesPerInterval = 0x10000U;
    }
    else if ((lec == 0U) && (*bytesPerInterval > (48U * 1024U)))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                "<%d> WARNING: LEC(%d) bytesPerInterval(%d) > 48KB, Patching "
                "to 48KB\n",
                epObj->parent->instanceNo, lec, *bytesPerInterval);
        *bytesPerInterval = 0xC000U;
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> bytesPerInterval = 0x%X\n",
                epObj->parent->instanceNo, *bytesPerInterval);
    }
}
/**
 * Used for handling USB SPEED LOW
 * @param epMaxPacketSize
 * @param epObj
 * @return uint32_t number of bytes per interval
 */
/* parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter epObj not used in
 * function handleSpeedLow", DRV-3800 */

static uint32_t handleSpeedLow (const USBSSP_ProducerQueueT *epObj,
                                uint16_t epMaxPacketSize)
{
    if (epMaxPacketSize > 8U)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "<%d> WARNING: epMaxPacketSize (%d) > 8\n",
                epObj->parent->instanceNo, epMaxPacketSize);
    }
    uint32_t bytesPerInterval = epMaxPacketSize;
    return bytesPerInterval;
}
/* parasoft-end-suppress MISRA2012-RULE-2_7-4 */

/**
 * Used for handling USB SPEED FULL
 * @param epMaxPacketSize
 * @param epDescType
 * @return uint32_t number of bytes per interval
 */
static uint32_t handleSpeedFull (uint8_t epDescType, uint16_t epMaxPacketSize)
{
    uint32_t bytesPerInterval = 0U;
    if ((epDescType == CH9_USB_EP_ISOCHRONOUS) ||
        (epDescType == CH9_USB_EP_INTERRUPT))
    {
        bytesPerInterval = epMaxPacketSize;
    }
    return bytesPerInterval;
}

/**
 * Used for handling USB SPEED HIGH
 * @param epMaxPacketSize
 * @param epDescType
 * @param burst_value
 * @return uint32_t number of bytes per interval
 */
static uint32_t handleSpeedHigh (uint8_t epDescType, uint16_t epMaxPacketSize,
                                 uint8_t burst_value)
{
    uint32_t bytesPerInterval = 0U;
    if ((epDescType == CH9_USB_EP_ISOCHRONOUS) ||
        (epDescType == CH9_USB_EP_INTERRUPT))
    {
        bytesPerInterval = (uint32_t)epMaxPacketSize *
                           ((uint32_t)burst_value + 1U);
    }
    return bytesPerInterval;
}

/**
 * Used for handling USB SPEED SUPER
 * @param epDescType
 * @param desc
 * @return uint32_t number of bytes per interval
 */
static uint32_t handleSpeedSuper (uint8_t epDescType, const uint8_t *desc)
{
    uint32_t bytesPerInterval = 0U;
    if ((epDescType == CH9_USB_EP_ISOCHRONOUS) ||
        (epDescType == CH9_USB_EP_INTERRUPT))
    {
        // bytes 5 (11 when added endpoint descriptor ) and 6
        //(12 when added endpoint descriptor) of companion descriptor is
        // wBytesPerInterval value
        bytesPerInterval = ((uint32_t)desc[12] << 8) | ((uint32_t)desc[11]);
    }
    return bytesPerInterval;
}

/**
 * Function sets max ESIT payload field in endpoint context
 * @param epObj endpoint object
 */
static void setMaxESITPayload (USBSSP_ProducerQueueT *epObj)
{
    uint32_t bytesPerInterval = 0U;
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;
    uint8_t *desc = epObj->epDesc;
    uint16_t epMaxPacketSize = (((uint16_t)desc[5] & 0x7U) << 8) |
                               (uint16_t)desc[4];
    uint8_t burst_value = (desc[5] & 0x18U) >> 3U;

    // calculate bytesPerInterval depending on different operating speed
    switch (epObj->parent->actualSpeed)
    {
        // for low speed
    case CH9_USB_SPEED_LOW:
        bytesPerInterval = handleSpeedLow(epObj, epMaxPacketSize);
        break;

        // full speed
    case CH9_USB_SPEED_FULL:
        bytesPerInterval = handleSpeedFull(epDescType, epMaxPacketSize);
        break;

        // high speed
    case CH9_USB_SPEED_HIGH:
        bytesPerInterval = handleSpeedHigh(epDescType, epMaxPacketSize,
                                           burst_value);
        break;

        // super speed
    case CH9_USB_SPEED_SUPER:
        bytesPerInterval = handleSpeedSuper(epDescType, desc);
        break;

        // super speed plus
    case CH9_USB_SPEED_SUPER_PLUS:
        setMaxESITPayloadSS(epObj, &bytesPerInterval);
        break;

    default:
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Incorrect speed value: %d!\n",
                epObj->parent->instanceNo, epObj->parent->actualSpeed);
        break;
    }

    setMaxESITPayLEC(epObj, &bytesPerInterval);

    // sets bytesPerInterval in endpoint context
    if (bytesPerInterval > 0U)
    {
        epObj->hwContext[0] |= ((bytesPerInterval >> 16U)
                                << USBSSP_EP_CXT_MAXESITPLD_HI_POS);
        epObj->hwContext[4] |= ((bytesPerInterval & 0xFFFFU)
                                << USBSSP_EP_CXT_MAXESITPLD_LO_POS);
    }
}

/**
 * Function sets CErr field in endpoint context structure
 * @param epObj endpoint object
 */
static void setCErr (USBSSP_ProducerQueueT *epObj)
{
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;
    uint32_t cerr = 0U;

    // cerr field should be set to tree only for bulk and interrupt endpoint
    if (epDescType != CH9_USB_EP_ISOCHRONOUS)
    {
        cerr = USBSSP_EP_CONTEXT_3ERR;
    }
    if (cerr > 0U)
    {
        cerr <<= USBSSP_EP_CONTEXT_CERR_POS;
        epObj->hwContext[1] |= cerr;
    }
}

/**
 * sets endpoint type field in endpoint context
 * @param epObj
 */
static void setEPType (USBSSP_ProducerQueueT *epObj)
{
    // this value reflects endpoint context bytes order
    uint32_t epType = 0U;

    // in endpoint descriptor, type is done on third byte, mask it with two less
    // significant bits
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;

    if (epDescType == 0U)
    {
        epDescType = 4U;  // Control - Bidirectional
    }
    else if ((epObj->epDesc[2] & CH9_USB_EP_DIR_IN) != 0U)
    {
        // check if endpoint in or out
        // address is kept on second byte of endpoint descriptor
        // according to XHCI endpoint type coding convention, for IN extra 1 or
        // fourth position must be added for this direction
        epDescType |= USBSSP_EP_CXT_EP_DIR_IN;
    }
    else
    {
        // MISRA: do nothing for out descriptors
    }

    epType = ((uint32_t)epDescType << USBSSP_EP_CONTEXT_EP_TYPE_POS) &
             USBSSP_EP_CONTEXT_EP_TYPE_MASK;

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> epType: %d\n",
            epObj->parent->instanceNo, epType >> USBSSP_EP_CONTEXT_EP_TYPE_POS);

    epObj->hwContext[1] |= epType;
}

/**
 * Function sets max burst value for SS and SSP mode
 * @param desc endpoint descriptor
 * @param maxBurstSize max burst value
 */
static void setMaxBurstSizeSS (const uint8_t *desc, uint8_t *maxBurstSize)
{
    // in endpoint descriptor, type is done on third byte, mask it with two less
    // significant bits
    uint8_t epDescType = desc[3] & 0x03U;

    // check if super speed endpoint companion available
    if ((desc[7] == CH9_USB_DS_SS_USB_EP_COMPANION) &&
        (desc[8] == CH9_USB_DT_SS_USB_EP_COMPANION))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                "SuperSpeed Endpoint companion found-\n", 0);
        *maxBurstSize = desc[9];

        if (epDescType == CH9_USB_EP_INTERRUPT)
        {
            if (*maxBurstSize > 2U)
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                        "WARNING: burst value for interrupt endpoint > 2\n", 0);
                vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                        "WARNING: Limit burst value to 2\n", 0);
                *maxBurstSize = 2;
            }
        }
    }
}

/**
 * Function sets max burst value for HS mode
 * @param desc endpoint descriptor
 * @param maxBurstSize max burst value
 */
static void setMaxBurstSizeHS (const uint8_t *desc, uint8_t *maxBurstSize)
{
    uint8_t epDescType = desc[3] & 0x03U;

    // calculate burst value from wMaxPacketSize field for HS speed & periodic
    // endpoints
    if ((epDescType == CH9_USB_EP_INTERRUPT) ||
        (epDescType == CH9_USB_EP_ISOCHRONOUS))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                "Periodic endpoint found, setting burst value-\n", 0);
        *maxBurstSize = (desc[5] & 0x18U) >> 3U;

        if (*maxBurstSize > 2U)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                    "WARNING: burst value for interrupt endpoint > 2\n", 0);
            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                    "WARNING: Limit burst value to 2\n", 0);
            *maxBurstSize = 2U;
        }
    }
}

/**
 * Function sets max burst size field in endpoint context
 * @param epObj endpoint object
 */
static void setMaxBurstSize (USBSSP_ProducerQueueT *epObj)
{
    uint8_t maxBurstSize = 0U;
    uint8_t *desc = epObj->epDesc;

    if (epObj->parent->actualSpeed >= CH9_USB_SPEED_SUPER)
    {
        setMaxBurstSizeSS(desc, &maxBurstSize);
    }
    else if (epObj->parent->actualSpeed == CH9_USB_SPEED_HIGH)
    {
        setMaxBurstSizeHS(desc, &maxBurstSize);
    }
    else
    {
        // required for MISRA
    }
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> bMaxBurst: %d\n",
            epObj->parent->instanceNo, maxBurstSize);
    if (maxBurstSize > 0U)
    {
        epObj->hwContext[1] |= ((uint32_t)maxBurstSize
                                << USBSSP_EP_CXT_MAX_BURST_SZ_POS) &
                               USBSSP_EP_CXT_MAX_BURST_SZ_MASK;
    }
}

/**
 * Function sets max packet size field in endpoint context
 * @param epObj endpoint object
 */
static void setMaxPacketSize (USBSSP_ProducerQueueT *epObj)
{
    uint8_t *desc = epObj->epDesc;

    // calculate max packet size from bytes: 4 and 5 written in Little endian
    uint16_t epMaxPacketSize = (((uint16_t)desc[5] & 0x7U) << 8) |
                               (uint16_t)desc[4];
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> epMaxPacketSize: %d\n",
            epObj->parent->instanceNo, epMaxPacketSize);

    epObj->hwContext[1] |= (uint32_t)epMaxPacketSize
                           << USBSSP_EP_CXT_MAX_PKT_SZ_POS;
}

/**
 * Function return dequeue cycle state bit from endpoint context
 * @param epObj endpoint object
 * @return dequeue cycle state bit
 */
static uint32_t getDCS (const USBSSP_ProducerQueueT *epObj)
{
    uint32_t dcs;
    uint32_t maxPStreams;

    // get actual value of maxPStreams
    maxPStreams = (epObj->hwContext[0] >> USBSSP_EP_CXT_PMAXSTREAMS_POS) &
                  0x1FU;

    if (maxPStreams > 0U)
    {
        dcs = 0;
    }
    else
    {
        dcs = epObj->toogleBit;
    }

    return dcs;
}

/**
 * Function sets TR dequeue pointer field in endpoint context
 */
static void setTRDequeuPointer (USBSSP_ProducerQueueT *epObj)
{
    set64Value(&epObj->hwContext[2], &epObj->hwContext[3],
               cpuToLe64(get64PhyAddrOf32ptr(&epObj->enqueuePtr->dword0) |
                         getDCS(epObj)));
    epObj->dequeuePtr = epObj->enqueuePtr;
}

/**
 * Function sets average TRB length in endpoint context
 * @param epObj endpoint object
 */
static void setAverageTRBLength (USBSSP_ProducerQueueT *epObj)
{
    uint8_t epDescType = epObj->epDesc[3] & 0x03U;
    uint32_t averageTRBLength = 0U;

    // check endpoint transfer type
    switch (epDescType)
    {
    case CH9_USB_EP_ISOCHRONOUS:
        averageTRBLength = USBSSP_EP_CXT_EP_ISO_AVGTRB_LEN;
        break;
    case CH9_USB_EP_INTERRUPT:
        averageTRBLength = USBSSP_EP_CXT_EP_INT_AVGTRB_LEN;
        break;
    case CH9_USB_EP_BULK:
        averageTRBLength = USBSSP_EP_CXT_EP_BLK_AVGTRB_LEN;
        break;
    default:  // Assume control EP
        averageTRBLength = USBSSP_EP_CXT_EP_CTL_AVGTRB_LEN;
        break;
    }
    averageTRBLength <<= USBSSP_EP_CXT_EP_AVGTRBLEN_POS;
    epObj->hwContext[4] |= averageTRBLength;
}
//----------------------------------------

/**
 * Function stores descriptors in endpoint object
 * @param res driver resources
 * @param epObj endpoint object
 * @param desc endpoint descriptor
 * @return CDN_EOK if for correct descriptor, error code elsewhere
 */
static uint32_t storeEpDesc (const USBSSP_DriverResourcesT *res,
                             USBSSP_ProducerQueueT *epObj, const uint8_t *desc)
{
    uint32_t ret = CDN_EOK;
    uint8_t epDescType = desc[3] & 0x03U;  // get endpoint attributes

    // store descriptor endpoint in endpoint object, will be used by upper
    // layers
    (void)memcpy(epObj->epDesc, desc, CH9_USB_DS_ENDPOINT);
    if (res->actualSpeed >= CH9_USB_SPEED_SUPER)
    {
        // first check if companion descriptor exists aligned in memory to
        // endpoint descriptor and return error if doesn't
        if (desc[CH9_USB_DS_ENDPOINT] != CH9_USB_DS_SS_USB_EP_COMPANION)
        {
            ret = CDN_EPERM;
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                    "<%d> Endpoint companion descriptor does not exist for SSx "
                    "device\n",
                    res->instanceNo);
        }
        else
        {
            (void)memcpy(&epObj->epDesc[CH9_USB_DS_ENDPOINT],
                         &desc[CH9_USB_DS_ENDPOINT],
                         CH9_USB_DS_SS_USB_EP_COMPANION);
        }
    }
    if ((res->actualSpeed == CH9_USB_SPEED_SUPER_PLUS) && (ret == CDN_EOK) &&
        (epDescType == CH9_USB_EP_ISOCHRONOUS))
    {
        // check if isochronous endpoint companion descriptor exists in memory
        uint8_t descOffset = CH9_USB_DS_ENDPOINT +
                             CH9_USB_DS_SS_USB_EP_COMPANION;
        if (desc[descOffset] != CH9_USB_DS_SSP_ISO_EP_COMPANION)
        {
            // device mode should support iso endpoint companion descriptor
            // for host mode it may happen that VIP doesn't support it
            if (res->deviceModeFlag == 1U)
            {
                ret = CDN_EPERM;
            }
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Isochronous endpoint companion descriptor does not "
                    "exist for SSP device\n",
                    res->instanceNo);
        }
        else
        {
            (void)memcpy(&epObj->epDesc[descOffset], &desc[descOffset],
                         CH9_USB_DS_SSP_ISO_EP_COMPANION);
        }
    }
    return ret;
}

/**
 * Set endpoint context
 * @param[in] epObj endpoint object
 */
static void setEPContext (USBSSP_ProducerQueueT *epObj)
{
    // setup context
    setEPType(epObj);
    setMult(epObj);
    setMaxPStreams(epObj);
    setInterval(epObj);
    setMaxESITPayload(epObj);
    setCErr(epObj);
    setMaxBurstSize(epObj);
}

/**
 * Set endpoint parameters
 * @param[in] epObj endpoint object
 */
static void setEPParams (USBSSP_ProducerQueueT *epObj)
{
    // setup context
    setEPContext(epObj);
    setMaxPacketSize(epObj);
    setTRDequeuPointer(epObj);
    setAverageTRBLength(epObj);
    epObj->first_prime_det = 0U;
}

/**
 * Function builds configure endpoint command TRB
 * @param res driver resources
 */
static void issueConfigEpCmd (USBSSP_DriverResourcesT *res)
{
    // set input context pointer in TRB
    set64Value(
        &res->commandQ.enqueuePtr->dword0, &res->commandQ.enqueuePtr->dword1,
        cpuToLe64(get64PhyAddrOf32ptr(res->inputContext->inputControlContext)));
    // set device slot, cycle bit, TRB type
    res->commandQ.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)((uint32_t)res->actualdeviceSlot
                              << USBSSP_SLOT_ID_POS) |
                   (USBSSP_TRB_CONF_EP_CMD << USBSSP_TRB_TYPE_POS) |
                   (uint32_t)res->commandQ.toogleBit));
    updateQueuePtr(&res->commandQ, 0U, "CMD.CONF_EP_CMD.");
    res->commandQ.isRunningFlag = 1;
    hostCmdDoorbell(res);
}

/**
 * Call setInterface callback
 * @param[in] res driver resources
 * @param[in] contextIndex context index
 * @param[in] configEpCmd configure ep command
 */
static void USBSSP_SetInterface (USBSSP_DriverResourcesT *res,
                                 uint8_t contextIndex,
                                 USBSSP_SetInterfaceState configEpCmd)
{
    if ((res->usbsspCallbacks.setInterfaceCallback != NULL) &&
        (contextIndex < 32U))
    {
        res->usbsspCallbacks.setInterfaceCallback(
            res, &configEpCmd, cpuToLe32((uint32_t)(1UL << contextIndex)));

        if (configEpCmd == USBSSP_EP_CONFIGURE)
        {
            uint32_t addMask;

            issueConfigEpCmd(res);

            addMask = le32ToCpu(res->inputContext->inputControlContext[1]);
            res->enabledEndpsMask |= addMask;
            res->enabledEndpsMask &= 0xFFFFFFFCU;  // Slot context and EP0 - not
                                                   // considered
        }
    }
}

/**
 * Configure and enable single endpoint
 * This function is called only for non EP0 endpoints
 * @param[in] res driver resources
 * @param[in] desc endpoint descriptor
 */
uint32_t USBSSP_EnableEndpoint (USBSSP_DriverResourcesT *res,
                                const uint8_t *desc)
{
    uint32_t ret = CDN_EOK;  // returned value

    uint8_t epIn;
    uint8_t epIndex;
    USBSSP_ProducerQueueT *epObj;

    ret = USBSSP_EnableEndpointSF(res, desc);
    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        uint8_t epAddress = desc[2];  // get endpoint address from descriptor
        uint8_t epDescType = desc[3] & 0x03U;  // get endpoint attributes

        // calculate endpoint direction
        if (epDescType == CH9_USB_EP_CONTROL)
        {
            epIn = 1U;  // For control endpoint flag should be set
        }
        else
        {
            epIn = ((epAddress & CH9_USB_EP_DIR_IN) > 0U) ? (uint8_t)1U
                                                          : (uint8_t)0U;
        }

        // calculate endpoint index
        epIndex = (uint8_t)((((epAddress & 0x7FU) - 1U) * 2U) +
                            ((epIn > 0U) ? 1U : 0U));

        // get endpoint object from endpoint container
        epObj = &res->ep[epIndex + USBSSP_EP_CONT_OFFSET];

        // store context index
        epObj->contextIndex = epIndex + USBSSP_EP_CONT_OFFSET;

        // set endpoint object's hardware context
        epObj->hwContext = res->inputContext->epContext[epIndex];

        // set endpoint parent
        epObj->parent = res;

        // set interrupter to default interrupter(0)
        epObj->interrupterIdx = 0U;

        ret = storeEpDesc(res, epObj, desc);

        if (ret == CDN_EOK)
        {
            setEPParams(epObj);

            // update input control context, checking 32 is required by Misra
            if (epObj->contextIndex < 32U)
            {
                uint32_t leAddMask = cpuToLe32(
                    (uint32_t)(1UL << epObj->contextIndex));
                res->inputContext->inputControlContext[1] |= leAddMask;

                // If endpoint was previously registered to be dropped
                // (Dx==1)_clearing Dx flag
                if ((res->inputContext->inputControlContext[0] & leAddMask) !=
                    0U)
                {
                    res->inputContext->inputControlContext[0] &= ~leAddMask;
                }
            }

            // update slot context when required
            if (epObj->contextIndex > res->contextEntries)
            {
                res->contextEntries = epObj->contextIndex;
            }

            res->inputContext->slot[0] = cpuToLe32(
                ((uint32_t)res->contextEntries << USBSSP_SLOT_CXT_CXT_ENT_POS) |
                ((uint32_t)getSlotSpeed(res->actualSpeed)
                 << USBSSP_SLOT_CONTEXT_SPEED_POS));  // 6.2.2 set slot context
                                                      // entries and speed
            if (res->usbsspCallbacks.inputContextCallback != NULL)
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                        "<%d> Calling inputContextCallback()\n",
                        res->instanceNo);
                res->usbsspCallbacks.inputContextCallback(res);
            }

            USBSSP_SetInterfaceState configEpCmd = USBSSP_EP_ENABLE;
            USBSSP_SetInterface(res, epObj->contextIndex, configEpCmd);
        }
    }
    return (ret);
}

/**
 * Disables single endpoint (before issuing CONFIGURE_ENDPOINT command)
 * @param[in] res driver resources
 * @param[in] epAddress Endpoint address
 */
uint32_t USBSSP_DisableEndpoint (USBSSP_DriverResourcesT *res,
                                 uint8_t epAddress)
{
    // check if res is not NULL
    uint32_t ret = USBSSP_DisableEndpointSF(res);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        uint8_t epIn;
        uint8_t epBase;
        uint8_t contextEntry;
        uint32_t leDropMask = 0U;
        USBSSP_EpContexEpState epState;

        // calculate context index from endpoint address
        epIn = (uint8_t)(((epAddress & CH9_USB_EP_DIR_IN) > 0U) ? 1U : 0U);
        epBase = (uint8_t)((((epAddress & 0x7FU) - 1U) * 2U) +
                           ((epIn > 0U) ? 1U : 0U));
        contextEntry = epBase + USBSSP_EP_CONT_OFFSET;

        epState = getEndpointStatus(res, contextEntry);
        if (epState == USBSSP_EP_CONTEXT_EP_STATE_RUNNING)
        {
            // enqueue stop endpoint command
            ret = USBSSP_StopEndpoint(res, (uint8_t)contextEntry);
        }
        else
        {
            USBSSP_SetInterfaceState configEpCmd = USBSSP_EP_DISABLE;
            USBSSP_SetInterface(res, contextEntry, configEpCmd);
        }

        if (epState != USBSSP_EP_CONTEXT_EP_STATE_DISABLED)
        {
            if (contextEntry < 32U)
            {
                leDropMask = cpuToLe32((uint32_t)(1UL << contextEntry));
            }

            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                    "<%d> ep_address: %02X (EP%d_%s) leDropMask:0x%08X\n",
                    res->instanceNo, epAddress, (epAddress & 0xF),
                    (epIn ? "IN" : "OUT"), leDropMask);

            // Setting Dx flag ...
            res->inputContext->inputControlContext[0] |= leDropMask;

            // ... and clearing Ax flag, if set
            if ((res->inputContext->inputControlContext[1] & leDropMask) !=
                (uint32_t)0U)
            {
                res->inputContext->inputControlContext[1] &= ~leDropMask;
            }
            if (res->usbsspCallbacks.inputContextCallback != NULL)
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                        "<%d> Calling inputContextCallback()\n",
                        res->instanceNo);
                res->usbsspCallbacks.inputContextCallback(res);
            }
            (void)memcpy(&res->inputContextCopy, res->inputContext,
                         sizeof(USBSSP_InputContexT));
        }
    }

    return ret;
}

/**
 * Issue generic command to SSP controller
 * @param[in] res Driver resources
 * @param[in] dword0 word 0 of command
 * @param[in] dword1 word 1 of command
 * @param[in] dword2 word 2 of command
 * @param[in] dword3 word 3 of command
 */
uint32_t USBSSP_IssueGenericCommand (USBSSP_DriverResourcesT *res,
                                     uint32_t dword0, uint32_t dword1,
                                     uint32_t dword2, uint32_t dword3)
{
    // check if res parameter is not NULL
    uint32_t ret = USBSSP_IssueGenericCommandSF(res);
    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        // fill four DWORDs of TRB
        res->commandQ.enqueuePtr->dword0 = cpuToLe32(dword0);
        res->commandQ.enqueuePtr->dword1 = cpuToLe32(dword1);
        res->commandQ.enqueuePtr->dword2 = cpuToLe32(dword2);
        res->commandQ.enqueuePtr->dword3 = cpuToLe32(dword3);
        // set cycle bit to correct value
        res->commandQ.enqueuePtr->dword3 &= 0xFFFFFFFEU;
        res->commandQ.enqueuePtr->dword3 |= (uint32_t)res->commandQ.toogleBit;

        updateQueuePtr(&res->commandQ, 0U, "CMD.GENERIC.");
        res->commandQ.isRunningFlag = 1;
        hostCmdDoorbell(res);
    }
    return ret;
}

/**
 * Configure end enable all endpoints
 * @param[in] res driver resources
 * @param[in] conf configuration descriptor
 */
static uint32_t configureEndpoints (USBSSP_DriverResourcesT *res,
                                    const uint8_t *conf, uint16_t length)
{
    uint32_t i = 0;
    uint32_t ret = CDN_EOK;

    // Ax flags (within Input Control Context) need to be: A0 = 1, A1 = 0, Dx
    // should be 0
    res->inputContext->inputControlContext[0] = 0;  // Dx = 0
    res->inputContext->inputControlContext[1] = cpuToLe32(
        1);                                         // A0 = 1, all other Ax = 0
    if (res->usbsspCallbacks.inputContextCallback != NULL)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Calling inputContextCallback()\n", res->instanceNo);
        res->usbsspCallbacks.inputContextCallback(res);
    }
    (void)memcpy(&res->inputContextCopy, res->inputContext,
                 sizeof(USBSSP_InputContexT));
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> configureEndpoints: %02X %d\n",
            res->instanceNo, conf[0], length);

    while ((i < length) && (ret == CDN_EOK))
    {
        /*descriptor type has offset 1 in descriptor*/
        if (conf[i + 1U] == CH9_USB_DT_ENDPOINT)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> ---Endpoint found---\n",
                    res->instanceNo);
            ret = USBSSP_EnableEndpoint(res, &conf[i]);
        }
        i += conf[i];
    }

    if (ret == CDN_EOK)
    {
        issueConfigEpCmd(res);
    }
    return ret;
}

static void handleStandardRequestSetAdd (USBSSP_DriverResourcesT *res,
                                         uint8_t devAdd)
{
    // data stage for this request will be sent in SET ADDRESS completion
    // see USBSSP_TRB_ADDRESS_DEVICE_COMMAND section in handleXhciCommad()

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> SET_ADDRESS: %d\n", res->instanceNo,
            devAdd);
    res->devAddress = devAdd;

    if ((res->actualdeviceSlot > 0U) && (res->enableSlotInProgress == 0U))
    {
        // send command to SSP controller
        setAddress(res, 0U);
    }
    else if (res->actualdeviceSlot == 0U)
    {
        // For some reasons like disconnect/reset we are unable to issue
        // set address command to SSP controller now
        res->ep0State = USBSSP_EP0_UNCONNECTED;
        res->ep0.isRunningFlag = 1U;
    }
    else
    {
        // required by MISRA
    }
}

static void handleStandardRequestSetConfPre (USBSSP_DriverResourcesT *res)
{
    // Before CONFIGURE_ENDPOINT command is issued - Dx/Ax flags
    // (within Input Context) should be cleared (all except A0, that
    // need to be set); they are set individually by CUSBD_EpEnable /
    // CUSBD_EpDisable
    res->inputContext->inputControlContext[0] = 0;  // Dx = 0
    res->inputContext->inputControlContext[1] = cpuToLe32(
        1);                                         // A0 = 1, rest Ax = 0
    if (res->usbsspCallbacks.inputContextCallback != NULL)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Calling inputContextCallback()\n", res->instanceNo);
        res->usbsspCallbacks.inputContextCallback(res);
    }
    // make software clone of input context
    (void)memcpy(&res->inputContextCopy, res->inputContext,
                 sizeof(USBSSP_InputContexT));
}

static void handleDeviceStandardReqPre (USBSSP_DriverResourcesT *res,
                                        const CH9_UsbSetup *const setup)
{
    if (setup->bRequest == CH9_USB_REQ_SET_ADDRESS)
    {
        // handle SET ADDRESS request
        handleStandardRequestSetAdd(res, (uint8_t)setup->wValue);
    }
    else if (setup->bRequest == CH9_USB_REQ_SET_CONFIGURATION)
    {
        // handle set configuration before calling callback
        handleStandardRequestSetConfPre(res);
    }
    else if (setup->bRequest == CH9_USB_REQ_CLEAR_FEATURE)
    {
        if (setup->wValue == CH9_USB_FS_U1_ENABLE)
        {
            setU1timeout(res, 0U);
        }
        else if (setup->wValue == CH9_USB_FS_U2_ENABLE)
        {
            setU2timeout(res, 0U);
        }
        else
        {
            // required by MISRA
        }
    }
    else if (setup->bRequest == CH9_USB_REQ_SET_FEATURE)
    {
        if (setup->wValue == CH9_USB_FS_U1_ENABLE)
        {
            setU1timeout(res, 1U);
        }
        else if (setup->wValue == CH9_USB_FS_U2_ENABLE)
        {
            setU2timeout(res, 1U);
        }
        else
        {
            // required by MISRA
        }
    }
    else
    {
        // required by MISRA
    }
}

/**
 * Send status stage
 * @param res driver resources
 * @param setup setup request
 */
static uint32_t handleSetupStatusStage (USBSSP_DriverResourcesT *res,
                                        const CH9_UsbSetup *setup)
{
    uint32_t ret = CDN_EOK;

    if (setup->wLength == 0U)
    {
        // send status stage for requests without data stage
        ret = USBSSP_ControlTransferDev(res, NULL, 0, 0);
    }

    return ret;
}

/**
 * Handles device setup request
 * @param res driver resources
 * @param setup setup request
 */
static uint32_t handleDeviceSetupRequest (USBSSP_DriverResourcesT *res,
                                          CH9_UsbSetup *setup)
{
    uint32_t ret;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> setup->bRequest %d\n",
            res->instanceNo, setup->bRequest);

    if (((setup->bmRequestType & CH9_USB_REQ_TYPE_MASK) ==
         CH9_USB_REQ_TYPE_STANDARD) &&
        ((setup->bmRequestType & CH9_REQ_RECIPIENT_MASK) ==
         CH9_USB_REQ_RECIPIENT_DEVICE))
    {
        handleDeviceStandardReqPre(res, setup);
    }

    // send request to higher layer and check result
    if ((res->cusbdCallbacks->setup != NULL) && (res->privateData != NULL))
    {
        ret = res->cusbdCallbacks->setup(res->privateData, setup);
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "<%d> ERROR: No callback defined for CUSBD module !\n",
                res->instanceNo);
        ret = CDN_EPERM;
    }

    // post user application handler
    if (ret == CDN_EOK)
    {
        switch (setup->bRequest)
        {
        case CH9_USB_REQ_SET_ADDRESS:
            break;

        case CH9_USB_REQ_SET_CONFIGURATION:
        {
            uint32_t addMask;
            uint32_t dropMask;

            displayInputContex(res->inputContext);

            // we configure endpoints in post handler only when higher
            // layer returned CDN_EOK for current setup
            issueConfigEpCmd(res);

            addMask = le32ToCpu(res->inputContext->inputControlContext[1]);
            dropMask = le32ToCpu(res->inputContext->inputControlContext[0]);

            res->enabledEndpsMask |= addMask;
            res->enabledEndpsMask &= ~dropMask;
            res->enabledEndpsMask &= 0xFFFFFFFCU;  // Slot context and EP0 - not
                                                   // considered
            break;
        }

        case CH9_USB_REQ_SET_INTERFACE:
            if (res->usbsspCallbacks.setInterfaceCallback == NULL)
            {
                ret = handleSetupStatusStage(res, setup);
            }
            break;

        default:
            ret = handleSetupStatusStage(res, setup);
            break;
        }
    }
    return (ret);
}

static void processSetupDevMode (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    CH9_UsbSetup *setup = &res->devSetupReq;

    res->ep0State = USBSSP_EP0_SETUP_PHASE;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "<%d> bmRequestType: %02X, bRequest: %02X, wValue: %04X, wIndex: "
            "%04X, wLength: %04X\n",
            res->instanceNo, setup->bmRequestType, setup->bRequest,
            setup->wValue, setup->wIndex, setup->wLength);

    ret = handleDeviceSetupRequest(res, setup);

    // set stall if request is not supported
    if (ret != CDN_EOK)
    {
        if (setup->wLength != 0U)
        {
            // STALL EP0 if control transfer has data phase
            (void)USBSSP_EndpointSetFeature(res, 1, 1);
            res->ep0State = USBSSP_EP0_HALT_PENDING;
        }
        else
        {
            // send STALL response in status phase
            controlXferDevStatusPhase(res, 0U);
            USBSSP_WriteDoorbell(res, res->actualdeviceSlot,
                                 res->ep0.contextIndex);
            res->ep0.isRunningFlag = 1;
        }
    }
}

/**
 * Displaying of setup request. This is internal driver function, called only
 * when SSP controller works in device mode.
 *
 * @param[in] res driver resources
 */
static void displaySetupRequest (USBSSP_DriverResourcesT *res)
{
    CH9_UsbSetup *setup = &res->devSetupReq;

    res->setupID = getSetupId(res->eventPtr);
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Received SETUP packet ID: %d\n",
            res->instanceNo, res->setupID);

    setup->bRequest = getSetupBrequest(res->eventPtr);
    setup->bmRequestType = getSetupBmRequestType(res->eventPtr);
    setup->wValue = cpuToLe16(getwValue(res->eventPtr));
    setup->wIndex = cpuToLe16(getwIndex(res->eventPtr));
    setup->wLength = cpuToLe16(getwLength(res->eventPtr));

    // display setup request
    XHCI_DISP_DEV_SETUP_REQ(setup);
}

/**
 * Handling of setup request. This is internal driver function, called only when
 * SSP controller works in device mode. Function parses incoming setup and
 * replies to USB host accordingly.
 *
 * @param[in] res driver resources
 */
static void handleSetupDeviceMode (USBSSP_DriverResourcesT *res)
{
    displaySetupRequest(res);

    if ((res->ep0State == USBSSP_EP0_HALT_PENDING) ||
        (res->ep0State == USBSSP_EP0_HALT_SETUP_PENDING))
    {
        // if EP0 is being halted - mark setup as pending
        res->ep0State = USBSSP_EP0_HALT_SETUP_PENDING;
    }
    else if (res->ep0State == USBSSP_EP0_HALTED)
    {
        // if EP0 is halted - reset EP0 to transition out from halt state
        (void)USBSSP_ResetEndpoint(res, 1U);
        res->ep0State = USBSSP_EP0_SETUP_PENDING;
    }
    else if (res->ep0State == USBSSP_EP0_SETUP_PENDING)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "<%d> New setup received while USBSSP_EP0_SETUP_PENDING\n",
                res->instanceNo);
    }
    else
    {
        if ((res->ep0State == USBSSP_EP0_SETUP_PHASE) ||
            (res->ep0State == USBSSP_EP0_DATA_PHASE))
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> setup request is being handled already\n",
                    res->instanceNo);
        }
        // handle setup request
        processSetupDevMode(res);
    }
}

/**
 * Functions creates event data TRB
 * @param ep endpoint object
 * @param eventDataLo event data low DWORD
 * @param eventDataHi event data high DWORD
 * @param flags extra flags
 */
static void addEventDataTRB (USBSSP_ProducerQueueT *ep, uint32_t eventDataLo,
                             uint32_t eventDataHi, uint32_t flags)
{
    // create data event TRB
    ep->enqueuePtr->dword0 = cpuToLe32(eventDataLo);
    ep->enqueuePtr->dword1 = cpuToLe32(eventDataHi);
    ep->enqueuePtr->dword2 = cpuToLe32(0);
    ep->enqueuePtr->dword3 = cpuToLe32(
            ((USBSSP_TRB_EVENT_DATA << USBSSP_TRB_TYPE_POS) | flags |
                   (uint32_t)ep->toogleBit));
    updateQueuePtr(ep, 0U, "EP.EVENT_DATA_TRB.");
}

/**
 * Add event data TRB to transfer ring
 * @param res driver resources
 * @param epIndex endpoint index
 * @param eventDataLo event data low DWORD
 * @param eventDataHi event data high DWORD
 * @param flags extra flags for TRB
 * @return
 */
uint32_t USBSSP_AddEventDataTRB (USBSSP_DriverResourcesT *res, uint8_t epIndex,
                                 uint32_t eventDataLo, uint32_t eventDataHi,
                                 uint32_t flags)
{
    // check if res input parameter is not NULL
    uint32_t ret = USBSSP_AddEventDataTRBSF(res);

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        // get endpoint object
        USBSSP_ProducerQueueT *ep = (epIndex == USBSSP_EP0_CONTEXT_OFFSET)
                                        ? &res->ep0
                                        : &res->ep[epIndex];

        // set event data TRB
        ep->enqueuePtr->dword0 = cpuToLe32(eventDataLo);
        ep->enqueuePtr->dword1 = cpuToLe32(eventDataHi);
        ep->enqueuePtr->dword2 = cpuToLe32(0);
        ep->enqueuePtr->dword3 = cpuToLe32(
            (uint32_t)((USBSSP_TRB_EVENT_DATA << USBSSP_TRB_TYPE_POS) | flags |
                       (uint32_t)ep->toogleBit));

        updateQueuePtr(ep, 0U, "EP.EVENT_DATA.");
    }

    return ret;
}

/**
 * Function returns max packet size for selected endpoint
 * @param epObj endpoint object
 * @return max packet size value
 */
static uint16_t getMaxPacketSize (const USBSSP_ProducerQueueT *epObj)
{
    // get max packet size value from endpoint context
    uint16_t maxPacketSize = (uint16_t)(epObj->hwContext[1] >>
                                        USBSSP_EP_CXT_MAX_PKT_SZ_POS);
    return (maxPacketSize);
}

/**
 * auxiliary structure type used for variables traversing between functions when
 * TD is being created
 */
typedef struct {
    uint8_t isLastBuffer;  // used for signaling last buffer in scatter/gather
                           // transfer
    uint8_t isFirstPage;   // used for signaling first memory page
    uint8_t isLastPage;    // used for signaling last memory page
    uint8_t isFirstTrb;    // used for signaling first TRB
    uint8_t isLastTrb;     // used for signaling last TRB
    uint32_t trbTransferLengthSum;  // used for TDSize calculation
    uint32_t packetTransfered;      // used for TDSize calculation
    uint32_t tdPacketCount;         // used for TDSize calculation
    uint16_t epMaxPacketSize;       // used for TDSize calculation
    uint8_t epIndex;                // keeps endpoint context index
    // --- used in USBSSP_CreateTD ---------------------
    uintptr_t pageStart;   // used for page number calculation
    uintptr_t pageEnd;     // used for page number calculation
    uintptr_t numOfPages;  // keeps number of memory page used for TD
    uintptr_t dataPtr;
} USBSSP_TDCreateT;

/**
 * Calculate TD Size value of TRB field
 * @param singleTrbLength data length
 * @param tdParams pointer to extra parameters (used internally)
 * @return value of TD size (0-31)
 */
static uint32_t calculateTdSize (uint32_t singleTrbLength,
                                 USBSSP_TDCreateT *tdParams)
{
    uint32_t tdSize;

    // calculate tdSize
    tdParams->trbTransferLengthSum += singleTrbLength;

    if (tdParams->epMaxPacketSize > 0U)
    {
        // round down packetTransfered
        tdParams->packetTransfered = tdParams->trbTransferLengthSum /
                                     tdParams->epMaxPacketSize;
    }

    // set tdSize to zero for the last TRB in TD
    if ((tdParams->isLastBuffer == 1U) && (tdParams->isLastTrb == 1U) &&
        (tdParams->isLastPage == 1U))
    {
        tdSize = 0U;
    }
    else
    {
        tdSize = ((tdParams->tdPacketCount - tdParams->packetTransfered) > 31U)
                     ? 31U
                     : (tdParams->tdPacketCount - tdParams->packetTransfered);
    }
    return tdSize;
}

/**
 * Function gets BurstCount for an endpoint
 * BurstCount is similar to MULT, i.e.
 * burstCount = (Max # of packets) / (actual burst size)
 * @param epObj
 */
static uint32_t getBurstCount (const USBSSP_ProducerQueueT *ep,
                               const USBSSP_TDCreateT *tdParams)
{
    // get burst size from EP context
    uint32_t burstSize = (ep->hwContext[1] & USBSSP_EP_CXT_MAX_BURST_SZ_MASK) >>
                         USBSSP_EP_CXT_MAX_BURST_SZ_POS;
    uint32_t burstCount = (tdParams->tdPacketCount + burstSize + 1U) /
                          (burstSize + 1U);

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "getBurstCount: %d\n", (burstCount - 1U));
    return (burstCount - 1U);
}

/**
 * Function updates TRB for isochronous endpoint
 * Note that in a TD, only first TRB will be marked ISO
 * @param ep endpoint object
 */
static void updateForIsoTrb (const USBSSP_DriverResourcesT *res,
                             USBSSP_ProducerQueueT *ep,
                             const USBSSP_TDCreateT *tdParams)
{
    uint32_t ete = res->qaRegs.xHCCaps.hcsparams2 &
                   0x100U;  // Missing USBSSP__HCSPARAMS2__ETE_MASK
    uint32_t burstCount = getBurstCount(ep, tdParams);

    // get TRB's last DWORD
    uint32_t tempDword3 = le32ToCpu(ep->enqueuePtr->dword3);

    // set TRB type as isochronous
    uint32_t trbType = USBSSP_TRB_ISOCH;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "hcsparams2(0x%X): ete(0x%X)\n",
            res->qaRegs.xHCCaps.hcsparams2, ete);

    // check if frameID is valid
    if ((ep->frameID & 0x8000U) > 0U)
    {
        // reset frameID
        tempDword3 &= ~((uint32_t)0x7FFU << USBSSP_TRB_ISOCH_FRAME_ID_POS);
        tempDword3 &= ~((uint32_t)1 << USBSSP_TRB_ISOCH_SIA_POS);
        // set frameID
        tempDword3 |= ((ep->frameID & 0x7FFU) << USBSSP_TRB_ISOCH_FRAME_ID_POS);
    }
    else
    {
        // send frame at once when frameID = 0;
        tempDword3 |= ((uint32_t)1 << USBSSP_TRB_ISOCH_SIA_POS);
    }

    // clear TRB field
    tempDword3 &= ~(trbType << USBSSP_TRB_TYPE_POS);

    // set ISO TRB field
    tempDword3 |= (trbType << USBSSP_TRB_TYPE_POS);

    // clear TBC_TBSTs field
    tempDword3 &= ~(USBSSP_TRB_TBC_TBSTS_MASK);

    if (ete == 0U)
    {
        tempDword3 |= (burstCount & 0x3U) << USBSSP_TRB_TBC_TBSTS_POS;
    }
    else
    {
        uint32_t tempDword2 = le32ToCpu(ep->enqueuePtr->dword2);

        // Clear TDSIZE_TBC
        tempDword2 &= ~(USBSSP_TRB_TDSIZE_TBC_MASK);

        tempDword2 |= (burstCount & 0x1FU) << USBSSP_TRB_TDSIZE_TBC_POS;

        ep->enqueuePtr->dword2 = cpuToLe32(tempDword2);
    }
    // update TRB
    ep->enqueuePtr->dword3 = cpuToLe32(tempDword3);
}

static void updateEpObjToStream (USBSSP_ProducerQueueT **epObj)
{
    USBSSP_ProducerQueueT *ep = *epObj;

    // if stream used, switch endpoint object to stream object
    if (ep->actualSID > 0U)
    {
        // get data size from endpoint object
        uint32_t size = ep->numOfBytes;
        *epObj = ep->stream[ep->actualSID - 1U];
        ep->stream[ep->actualSID - 1U]->numOfBytes = size;
        ep->stream[ep->actualSID - 1U]->req_pending = USBSSP_REQUEST_PENDING;
    }
}

/**
 * Create single TRB in transfer ring
 * @param res driver resources
 * @param dataPtr address of data
 * @param singleTrbLength length of this data chunk
 * @param tdParams pointer to extra parameters (used internally)
 */
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter res with const
 * specifier, DRV-3806" */
static void createSingleTrb (USBSSP_DriverResourcesT *res, uintptr_t dataPtr,
                             uint32_t singleTrbLength,
                             USBSSP_TDCreateT *tdParams)
{
    // default transfer TRB as normal
    uint32_t trbType = USBSSP_TRB_NORMAL;
    uint32_t linkTrbChainFlag = USBSSP_TRB_NORMAL_CH_MASK;

    // get endpoint object
    USBSSP_ProducerQueueT *ep = &res->ep[tdParams->epIndex];

    // set by default CHAIN flag
    uint32_t flags = USBSSP_TRB_NORMAL_ISP_MASK | USBSSP_TRB_NORMAL_CH_MASK;

    // calculate TD size
    uint32_t tdSize = calculateTdSize(singleTrbLength, tdParams);

    // switch to stream object if stream used
    updateEpObjToStream(&ep);

    ep->req_pending = USBSSP_REQUEST_PENDING;

    if ((tdParams->isLastTrb == 1U) && (tdParams->isLastPage == 1U))
    {
        if ((ep->extraFlags & (uint8_t)USBSSP_EXTRAFLAGSENUMT_FORCELINKTRB) ==
            0U)
        {
            if (ep->actualSID == 0U)
            {
                flags = USBSSP_TRB_NORMAL_IOC_MASK;
                linkTrbChainFlag = 0;
            }
            else
            {
                flags = USBSSP_TRB_NORMAL_ENT_MASK | USBSSP_TRB_NORMAL_CH_MASK;
            }
        }
    }

    // Create TRB
    set64Value(&ep->enqueuePtr->dword0, &ep->enqueuePtr->dword1,
               cpuToLe64((uint64_t)(dataPtr)));

    ep->enqueuePtr->dword2 = cpuToLe32(
        (ep->interrupterIdx << USBSSP_TRB_INTR_TRGT_POS) |
        (singleTrbLength & 0x0001FFFFU) | (tdSize << 17));

    ep->enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((trbType << USBSSP_TRB_TYPE_POS) | flags |
                   (uint32_t)ep->toogleBit));

    // update TRB for iso transfer only in first TRB of TD
    if ((ep->epDesc[3] == CH9_USB_EP_ISOCHRONOUS) &&
        (tdParams->isFirstPage == 1U) && (tdParams->isFirstTrb == 1U))
    {
        updateForIsoTrb(res, ep, tdParams);
    }

    /* remember first TRB in TD */
    if ((tdParams->isFirstPage == 1U) && (tdParams->isFirstTrb == 1U))
    {
        ep->firstQueuedTRB = ep->enqueuePtr;
    }

    // remember last TRB in TD
    ep->lastQueuedTRB = ep->enqueuePtr;

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "address: 0x%X, length: %d, tdSize: %d, flags: %08X, "
            "trbTransferLengthSum: %d, packetTransfered: %d\n",
            dataPtr, singleTrbLength, tdSize, flags,
            tdParams->trbTransferLengthSum, tdParams->packetTransfered);
    updateQueuePtr(ep, linkTrbChainFlag, "EP.DATA.XFER.");
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */

static uint32_t calculateSingleTrbLength (uint32_t numOfTrb,
                                          uint32_t dataLength,
                                          uintptr_t buffStart)
{
    uint32_t singleTrbLength;

    // if there is only one TRB size equals to dataLength for this page
    if (numOfTrb == 1U)
    {
        singleTrbLength = dataLength;
    }
    else
    {
        singleTrbLength = (uint32_t)(USBSSP_TRB_MAX_TRANSFER_LENGTH -
                                     (buffStart %
                                      USBSSP_TRB_MAX_TRANSFER_LENGTH));
    }

    return singleTrbLength;
}

/**
 * Function creates TRBs for single memory page
 * @param res driver resources
 * @param dataPtr pointer to memory where actual data pointer is stored
 * @param dataLength data length
 * @param tdParams pointer to extra parameters (used internally)
 */
static void trbSinglePage (USBSSP_DriverResourcesT *res, uintptr_t *dataPtr,
                           uint32_t dataLength, USBSSP_TDCreateT *tdParams)
{
    uint32_t trbIndex;               // used as enumerator in for loop
    uintptr_t buffStart = *dataPtr;  // keeps original address of data start
    uintptr_t endAddress = buffStart + (uintptr_t)dataLength;
    tdParams->isLastTrb = 0U;

    // calculate number of TRBs
    uint32_t numOfTrb = dataLength / USBSSP_TRB_MAX_TRANSFER_LENGTH;

    // round up number of packets
    if ((dataLength % USBSSP_TRB_MAX_TRANSFER_LENGTH) > 0U)
    {
        ++numOfTrb;
    }

    // for data length = 0
    if (dataLength == 0U)
    {
        numOfTrb = 1U;
    }

    for (trbIndex = 0U; trbIndex < numOfTrb; trbIndex++)
    {
        // reset flag
        tdParams->isFirstTrb = 0U;

        // calculate date length of single TRB
        uint32_t singleTrbLength;

        // for first TRB
        if (trbIndex == 0U)
        {
            // mark first TRB
            tdParams->isFirstTrb = 1U;
            singleTrbLength = calculateSingleTrbLength(numOfTrb, dataLength,
                                                       buffStart);
        }
        else if (trbIndex == (numOfTrb - 1U))
        {
            // for last TRB
            singleTrbLength = (uint32_t)(endAddress - (*dataPtr));
        }
        else
        {
            // elsewhere
            singleTrbLength = USBSSP_TRB_MAX_TRANSFER_LENGTH;
        }

        // check if TRB is last
        if (trbIndex == (numOfTrb - 1U))
        {
            tdParams->isLastTrb = 1U;
        }

        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "TRB(%d):\n", trbIndex);

        // create single TRB
        createSingleTrb(res, *dataPtr, singleTrbLength, tdParams);

        // move data pointer
        (*dataPtr) += singleTrbLength;
    }
}

/**
 * calculate number of bytes for first memory page
 * @param pageStart address of first page
 * @param pageEnd address of last page
 * @param buff pointer to transfered data
 * @param size size of data buffer
 * @return number of buffer for first memory page
 */
static uint32_t calcFirstPageNumBytes (uintptr_t pageStart, uintptr_t pageEnd,
                                       uintptr_t buff, uint32_t size)
{
    uint32_t numOfBytes;
    if (pageStart == pageEnd)
    {
        // whole TD is located on the same page
        numOfBytes = size;
    }
    else
    {
        // TD exceed single page
        numOfBytes = (uint32_t)(USBSSP_SYSTEM_MEMORY_PAGE_SIZE -
                                (buff % USBSSP_SYSTEM_MEMORY_PAGE_SIZE));
    }
    return numOfBytes;
}

/**
 * calculate number of bytes for last memory page
 * @param buff pointer to transfered data
 * @param size size of data buffer
 * @return number of buffer for last memory page
 */
static uint32_t calcLastPageNumBytes (uintptr_t buff, uint32_t size)
{
    uint32_t numOfBytes = (uint32_t)((buff + (uintptr_t)size) %
                                     USBSSP_SYSTEM_MEMORY_PAGE_SIZE);
    return numOfBytes;
}

static void createTdAllMemPagesIndex (USBSSP_TDCreateT *tdParams,
                                      const uintptr_t buff, const uint32_t size)
{
    // calculate number of memory pages used for this TD
    tdParams->pageStart = buff / USBSSP_SYSTEM_MEMORY_PAGE_SIZE;

    // check if size is greater than zero
    if (size > 0U)
    {
        tdParams->pageEnd = ((buff + (uintptr_t)size) - 1U) /
                            USBSSP_SYSTEM_MEMORY_PAGE_SIZE;
    }
    else
    {
        tdParams->pageEnd = tdParams->pageStart;
    }
    tdParams->numOfPages = (tdParams->pageEnd - tdParams->pageStart) + 1U;
}

static void createTdAllMemPages (USBSSP_DriverResourcesT *res,
                                 USBSSP_TDCreateT *tdParams,
                                 const uintptr_t buff, const uint32_t size)
{
    uintptr_t pageIndex;  // page enumerator

    // calculate first, last pages
    createTdAllMemPagesIndex(tdParams, buff, size);

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> pageStart: 0x%X, pageEnd: 0x%X, numOfpages: %d "
            "tdPacketCount: %d\n",
            res->instanceNo, tdParams->pageStart, tdParams->pageEnd,
            tdParams->numOfPages, tdParams->tdPacketCount);

    // create TRBs for every memory page
    for (pageIndex = 0; pageIndex < tdParams->numOfPages; pageIndex++)
    {
        uint32_t numOfBytes;
        tdParams->isFirstPage = 0U;

        // for first page
        if (pageIndex == 0U)
        {
            tdParams->isFirstPage = 1U;
            numOfBytes = calcFirstPageNumBytes(tdParams->pageStart,
                                               tdParams->pageEnd, buff, size);
            // for last page
        }
        else if (pageIndex == ((uintptr_t)tdParams->numOfPages - 1U))
        {
            numOfBytes = calcLastPageNumBytes(buff, size);
        }
        else
        {
            // for middle pages
            numOfBytes = (uint32_t)USBSSP_SYSTEM_MEMORY_PAGE_SIZE;
        }

        // check if page is last page in TD
        if (pageIndex == ((uintptr_t)tdParams->numOfPages - 1U))
        {
            tdParams->isLastPage = 1U;
        }

        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> page(%d)\n", res->instanceNo,
                pageIndex);

        // create single page TRBS
        trbSinglePage(res, &tdParams->dataPtr, numOfBytes, tdParams);
    }
}

/**
 * Function creates transfer descriptor in TRB ring
 * @param res driver resources
 * @param index endpoint index in device context
 * @param buff data user buffer
 * @param size data length of user buffer
 */
static void USBSSP_CreateTD (USBSSP_DriverResourcesT *res, const uint8_t index,
                             const uintptr_t buff, const uint32_t size,
                             USBSSP_TDCreateT *tdInputParams)
{
    USBSSP_TDCreateT *tdParams;
    USBSSP_TDCreateT tdParamsAlloc;
    // get endpoint object
    USBSSP_ProducerQueueT *ep = &res->ep[index];

    // check if tdParams is from external function
    if (tdInputParams != NULL)
    {
        tdParams = tdInputParams;
    }
    else
    {
        // create own tdParams object
        tdParams = &tdParamsAlloc;

        // set extra variables used for tdSize calculation
        tdParams->packetTransfered = 0U;
        tdParams->trbTransferLengthSum = 0U;
        tdParams->isLastPage = 0U;
        tdParams->epIndex = index;
        tdParams->isLastBuffer = 1U;  // only one user buffer is sent

        // get max packet size
        tdParams->epMaxPacketSize = getMaxPacketSize(&res->ep[index]);

        // calculate number of USB packets
        // first check if maxPacketSize > 0
        if (tdParams->epMaxPacketSize == 0U)
        {
            tdParams->tdPacketCount = 1U;
        }
        else
        {
            tdParams->tdPacketCount = (size / tdParams->epMaxPacketSize);

            // round up tdPacketCount
            if ((size % tdParams->epMaxPacketSize) > 0U)
            {
                ++tdParams->tdPacketCount;
            }
        }
    }
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> buff: %p, size: %d\n",
            res->instanceNo, buff, size);
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> Endpoint(index %d) maxPacketSize: %d\n", res->instanceNo,
            index, tdParams->epMaxPacketSize);
    tdParams->dataPtr = buff;
    createTdAllMemPages(res, tdParams, buff, size);

    // if stream used add event data TRB at the end of whole TD
    if (ep->actualSID > 0U)
    {
        uint16_t actualSID = ep->actualSID;
        ep = (USBSSP_ProducerQueueT *)ep->stream[ep->actualSID - 1U];
        addEventDataTRB(ep, (uint32_t)index, (uint32_t)actualSID,
                        USBSSP_TRB_NORMAL_IOC_MASK);
    }
}

static uint32_t USBSSP_TransferDataDRBL (USBSSP_DriverResourcesT *res,
                                         USBSSP_ProducerQueueT *ep)
{
    uint32_t ret = CDN_EOK;

    // get endpoint context index
    uint8_t epIndex = ep->contextIndex;

    // get endpoint state
    USBSSP_EpContexEpState endpointState = getEndpointStatus(res, epIndex);

    // handle not stalled endpoint
    if (endpointState != USBSSP_EP_CONTEXT_EP_STATE_HALTED)
    {
        uint32_t drblReg = ((uint32_t)ep->actualSID) << 16U;
        drblReg |= (uint32_t)epIndex;

        // send clear feature to endpoint in host mode if endpoint halted
        if ((res->deviceModeFlag == 0U) && (ep->isDisabledFlag != 0U))
        {
            ret = USBSSP_EndpointSetFeature(res, epIndex, 0);
        }

        if (ep->streamCount != 0U)
        {
            if (ep->first_prime_det == 0U)
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Wait for PRIME (%d)\n",
                        res->instanceNo, epIndex);
            }
            else
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> DRBL(%d)\n",
                        res->instanceNo, epIndex);
                USBSSP_WriteDoorbell(res, res->actualdeviceSlot, drblReg);
                ep->drbls_count++;
            }
        }
        else
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> DRBL(%d)\n", res->instanceNo,
                    epIndex);
            USBSSP_WriteDoorbell(res, res->actualdeviceSlot, drblReg);
        }
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> ENDPOINT %d is in not RUNNING state, can not issue "
                "DOORBELL - current status: %d\n",
                res->instanceNo, epIndex, endpointState);
        ep->req_pending = USBSSP_REQUEST_HALTED;
        ret = CDN_EPERM;
    }
    return ret;
}

/**
 * Transfer data on given endpoint. This function is non-blocking type. The XHCI
 * operation result should be checked in complete callback function.
 *
 * @param[in] res driver resources
 * @param[in] ep_index index of endpoint according to xhci spec e.g for ep1out
              ep_index=2, for ep1in ep_index=3, for ep2out ep_index=4 end so on
 * @param[in] buff buffer for data to send or to receive
 * @param[in] size size of data in bytes
 * @param[in] complete pointer to complete callback function
 * @param[in] data pointer to function which will be returned in callback in
 input
 *             parameter, can be set to NULL when no extra parameter used
 *
 * @return CDN_EINVAL if selected endpoint index is out of available range
 * @return CDN_EOK if selected endpoint is within available endpoint range
 */
uint32_t USBSSP_TransferData (USBSSP_DriverResourcesT *res, uint8_t epIndex,
                              const uintptr_t buff, uint32_t size,
                              USBSSP_Complete complete)
{
    // check parameters correctness
    uint32_t ret = (uint32_t)USBSSP_TransferDataSF(res, epIndex);

    if (ret != CDN_EOK)
    {
        DbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        if (ret == CDN_EOK)
        {
            /* check if transfers are enabled on this endpoint */
            ret = checkEpXferEnabled(res, epIndex);
        }

        if (ret == CDN_EOK)
        {
            // get endpoint object
            USBSSP_ProducerQueueT *ep = &res->ep[epIndex];

            ep->complete = complete;
            ep->numOfBytes = size;
            ep->isRunningFlag = 1;

            // create TRB's chain
            USBSSP_CreateTD(res, epIndex, buff, size, NULL);

            // set DRBL only on last packet in chain
            if ((ep->extraFlags & (uint8_t)USBSSP_EXTRAFLAGSENUMT_NODORBELL) ==
                0U)
            {
                ret = USBSSP_TransferDataDRBL(res, ep);
            }
        }
    }
    return ret;
}

/**
 * calculate number of USB packets in whole transfer descriptor
 * @param tdParams
 * @param wholeSGLength
 */
static void calcTdPackNum (USBSSP_TDCreateT *tdParams, uint32_t wholeSGLength)
{
    // calculate number of USB packets
    // first check if maxPacketSize > 0

    if (tdParams->epMaxPacketSize == 0U)
    {
        tdParams->tdPacketCount = 1U;
    }
    else
    {
        tdParams->tdPacketCount = (wholeSGLength / tdParams->epMaxPacketSize);

        // round up tdPacketCount
        if ((wholeSGLength % tdParams->epMaxPacketSize) > 0U)
        {
            ++tdParams->tdPacketCount;
        }
    }
}

/**
 * calculate data sum of all scatter/gather elements
 * @param sizeVec size vector
 * @param sgElementsSum number of scatter/gather elements
 * @return sum of data transfered in scatter/gather transfer
 */
static uint32_t calcSGDataLength (const uint32_t *sizeVec,
                                  uint32_t sgElementsSum)
{
    uint32_t i;
    uint32_t wholeSGLength = 0U;

    // sum of all elements in size vector
    for (i = 0; i < sgElementsSum; i++)
    {
        wholeSGLength += sizeVec[i];
    }

    return wholeSGLength;
}

/**
 * Scatter/gather transfer function
 * @param res driver resources
 * @param epIndex endpoint index
 * @param buffVec buffer for user data buffers
 * @param sizeVec buffer for user data buffer sizes
 * @param sgElementsSum number of user buffers
 * @param complete completion callback
 * @return CDN_EOK if success, error code elsewhere
 */
uint32_t USBSSP_TransferData2 (USBSSP_DriverResourcesT *res, uint8_t epIndex,
                               USBSSP_param paramT, USBSSP_Complete complete)
{
    // check input parameters correctness
    uint32_t ret = (uint32_t)USBSSP_TransferData2SF(res, epIndex);
    if (ret != CDN_EOK)
    {
        DbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        USBSSP_ProducerQueueT *ep = &res->ep[epIndex];  // get endpoint object
        uint32_t i;

        if (ret == CDN_EOK)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Transfer data on ep: %d\n",
                    res->instanceNo, epIndex);
            /* check if transfers are enabled on this endpoint */
            ret = checkEpXferEnabled(res, epIndex);
        }
        if (ret == CDN_EOK)
        {
            const uintptr_t *buffVec = paramT.buffVec;
            const uint32_t *sizeVec = paramT.sizeVec;
            uint32_t sgElementsSum = paramT.sgElementsSum;
            uint32_t wholeSGLength;
            USBSSP_TDCreateT tdParams;

            // set extra variables used for tdSize calculation
            tdParams.packetTransfered = 0U;
            tdParams.trbTransferLengthSum = 0U;
            tdParams.isLastPage = 0U;
            tdParams.epIndex = epIndex;
            tdParams.isLastBuffer = 0U;

            // get max packet size
            tdParams.epMaxPacketSize = getMaxPacketSize(&res->ep[epIndex]);

            // calculate of whole length
            wholeSGLength = calcSGDataLength(sizeVec, sgElementsSum);

            // calculate number of packets
            calcTdPackNum(&tdParams, wholeSGLength);

            uint32_t drblReg = ((uint32_t)ep->actualSID) << 16;
            drblReg |= (uint32_t)epIndex;

            // set complete callback
            ep->complete = complete;
            ep->numOfBytes = wholeSGLength;  // ?

            // set extra flags
            ep->extraFlags = (uint8_t)USBSSP_EXTRAFLAGSENUMT_FORCELINKTRB;

            // build TD with USBSSP_TransferData function with blocked DRBL
            for (i = 0; i < (sgElementsSum - 1U); i++)
            {
                USBSSP_CreateTD(res, epIndex, buffVec[i], sizeVec[i],
                                &tdParams);
            }

            // put last TRB with IOC enabled: clear all extra flags
            ep->extraFlags = ~((uint8_t)USBSSP_EXTRAFLAGSENUMT_FORCELINKTRB);
            tdParams.isLastBuffer = 1U;
            USBSSP_CreateTD(res, epIndex, buffVec[i], sizeVec[i], &tdParams);
            USBSSP_WriteDoorbell(res, res->actualdeviceSlot, drblReg);
            ep->isRunningFlag = 1;
            // start DRBL
        }
    }
    return ret;
}

/**
 * Reset port.
 * @param[in] res driver resources
 * @param[in] portIndex index of port to reset
 */
uint32_t USBSSP_ResetRootHubPort (const USBSSP_DriverResourcesT *res)
{
    // Check if parameters are valid.
    uint32_t ret = USBSSP_ResetRootHubPortSF(res);
    if (ret != CDN_EOK)
    {
        DbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        uint8_t portIndex = res->actualPort;
        uint32_t reg = xhciRead32(
            &res->regs.xhciPortControl[portIndex - 1U].portsc);
        reg |= USBSSP__PORTSC1USB2__PR_MASK;

        xhciWrite32(&res->regs.xhciPortControl[portIndex - 1U].portsc, reg);

        // in host mode we need to pool PED bit of PORTSC
        if (res->deviceModeFlag == 0U)
        {
            uint32_t ped;

            // pool PED until set to 1
            do
            {
                ped = xhciRead32(
                          &res->regs.xhciPortControl[res->actualPort - 1U]
                               .portsc) &
                      USBSSP__PORTSC1USB3__PED_MASK;
                CPS_DelayUs(1);
            } while (ped == 0U);
        }
    }
    return ret;
}

/**
 * Enqueues the Stop endpoint command in the command queue
 *
 * @param[in] res driver resources
 * @param[in] endpoint index of endpoint to stop
 */
static void enqueueStopEndpointCmd (USBSSP_DriverResourcesT *res,
                                    uint8_t endpoint)
{
    res->commandQ.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)res->actualdeviceSlot << USBSSP_SLOT_ID_POS) |
        (uint32_t)((uint32_t)endpoint << USBSSP_ENDPOINT_POS) |
        (uint32_t)(USBSSP_TRB_STOP_EP_CMD << USBSSP_TRB_TYPE_POS) |
        res->commandQ.toogleBit);

    updateQueuePtr(&res->commandQ, 0U, "CMD.STOP_EP.");
}

/**
 * Stop endpoint. Function sends STOP_ENDPOINT_COMMAND command to SSP controller
 *
 * @param[in] res driver resources
 * @param[in] endpoint index of endpoint to stop
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_StopEndpoint (USBSSP_DriverResourcesT *res, uint8_t endpoint)
{
    // check input parameters
    uint32_t ret = USBSSP_StopEndpointSF(res, endpoint);

    // return CDN_EINVAL if parameters are not correct
    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        // enqueue stop endpoint command
        enqueueStopEndpointCmd(res, endpoint);
        hostCmdDoorbell(res);
    }

    return ret;
}

/**
 * Set dequeue pointer. Function sends SET_TR_DEQUEUE_POINTER_COMMAND to SSP
 * controller
 *
 * @param[in] res driver resources
 * @param[in] endpoint index to set dequeue pointer to
 */
static void USBSSP_SetDequeuePointer (USBSSP_DriverResourcesT *res,
                                      uint8_t epIndex, uint64_t newDequeuePtr)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "<%d> SET dequeue pointer of endpoint(%d) to 0x%08x%08x\n",
            res->instanceNo, epIndex,
            (uint32_t)((newDequeuePtr & 0xFFFFFFFF00000000ULL) >> 32),
            (uint32_t)((newDequeuePtr & 0xFFFFFFFFULL)));

    if (epIndex == USBSSP_EP0_CONTEXT_OFFSET)
    {
        set64Value(
            &res->commandQ.enqueuePtr->dword0,
            &res->commandQ.enqueuePtr->dword1,
            cpuToLe64(newDequeuePtr | res->ep0.toogleBit)
            // cpuToLe64(get64PhyAddrOf32ptr(&res->ep0.enqueuePtr->dword0)
            // | res->ep0.toogleBit)
        );
    }
    else
    {
        set64Value(&res->commandQ.enqueuePtr->dword0,
                   &res->commandQ.enqueuePtr->dword1,
                   // cpuToLe64(get64PhyAddrOf32ptr(&
                   // res->ep[endpoint].enqueuePtr->dword0) |
                   // res->ep[endpoint].toogleBit)
                   cpuToLe64(newDequeuePtr | res->ep[epIndex].toogleBit));
    }

    res->commandQ.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)res->actualdeviceSlot << USBSSP_SLOT_ID_POS) |
        (uint32_t)((uint32_t)epIndex << USBSSP_ENDPOINT_POS) |
        (uint32_t)(USBSSP_TRB_SET_TR_DQ_PTR_CMD << USBSSP_TRB_TYPE_POS) |
        res->commandQ.toogleBit);

    updateQueuePtr(&res->commandQ, 0U, "CMD.SET_TR_DQ_PTR.");
    hostCmdDoorbell(res);
}

/**
 * Enqueues RESET_ENDPOINT_COMMAND in the command queue. Doesn't ring doorbell
 *
 * @param[in] res driver resources
 * @param[in] endpoint index of endpoint to reset
 */
static void enqueueResetEndpointCmd (USBSSP_DriverResourcesT *res,
                                     uint8_t endpoint)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> reset endpoint %d\n",
            res->instanceNo, endpoint);
    res->commandQ.enqueuePtr->dword3 = cpuToLe32(
        (uint32_t)((uint32_t)res->actualdeviceSlot << USBSSP_SLOT_ID_POS) |
        (uint32_t)((uint32_t)endpoint << USBSSP_ENDPOINT_POS) |
        (uint32_t)(USBSSP_TRB_RESET_EP_CMD << USBSSP_TRB_TYPE_POS) |
        res->commandQ.toogleBit
        //| 0x200
    );
    updateQueuePtr(&res->commandQ, 0U, "CMD.RESET_EP.");
}

/**
 * Reset of endpoint. Function sends RESET_ENDPOINT_COMMAND to SSP controller
 *
 * @param[in] res driver resources
 * @param[in] endpoint index of endpoint to reset
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_ResetEndpoint (USBSSP_DriverResourcesT *res, uint8_t endpoint)
{
    // check input parameters
    uint32_t ret = USBSSP_ResetEndpointSF(res, endpoint);

    // if input parameters are not correct return CDN_EINVAL error
    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }

    if (ret == CDN_EOK)
    {
        // enqueue reset endpoint command
        enqueueResetEndpointCmd(res, endpoint);
        hostCmdDoorbell(res);
    }

    return ret;
}

/**
 * Reset of connected device. Function sends RESET_DEVICE_COMMAND to SSP
 * controller in order to issue reset state on USB bus.
 *
 * @param[in] res driver resources
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if selected endpoint is within available endpoint range
 */
uint32_t USBSSP_ResetDevice (USBSSP_DriverResourcesT *res)
{
    // check if input parameter is not NULL
    uint32_t ret = USBSSP_ResetDeviceSF(res);

    // return error code when input parameter is NULL
    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        // reset device address
        res->devAddress = 0;
        res->ep0.isRunningFlag = 0;

        // issue reset device command to controller
        res->commandQ.enqueuePtr->dword3 = cpuToLe32((
            uint32_t)((uint32_t)((uint32_t)res->actualdeviceSlot
                                 << USBSSP_SLOT_ID_POS) |
                      (USBSSP_TRB_RESET_DEVICE_COMMAND << USBSSP_TRB_TYPE_POS) |
                      (uint32_t)res->commandQ.toogleBit));

        updateQueuePtr(&res->commandQ, 0U, "CMD.RESET_DEVICE.");
        hostCmdDoorbell(res);
    }

    return ret;
}

/**
 * Force header command.
 *
 * @param[in] res driver resources
 */
uint32_t USBSSP_ForceHeader (USBSSP_DriverResourcesT *res,
                             const USBSSP_dword word,
                             USBSSP_ForceHeaderComplete complete)
{
    // check if pointer to resources is not NULL
    uint32_t ret = USBSSP_ForceHeaderSF(res);
    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        uint32_t dword0 = word.dword0;
        uint32_t dword1 = word.dword1;
        uint32_t dword2 = word.dword2;
        uint32_t dword3 = word.dword3;
        // fill TRB with FORCE HEADER command
        res->commandQ.enqueuePtr->dword0 = cpuToLe32(dword0);
        res->commandQ.enqueuePtr->dword1 = cpuToLe32(dword1);
        res->commandQ.enqueuePtr->dword2 = cpuToLe32(dword2);
        res->commandQ.enqueuePtr->dword3 = cpuToLe32(dword3);
        res->commandQ.enqueuePtr->dword3 &= 0xFFFFFFFEU;
        res->commandQ.enqueuePtr->dword3 |= (uint32_t)res->commandQ.toogleBit;

        // set internal force header complete callback to this function caller's
        // callback
        res->forceHeaderComplete = complete;
        updateQueuePtr(&res->commandQ, 0U, "CMD.FORCE_HEADER.");
        hostCmdDoorbell(res);
    }
    return ret;
}

/**
 * Function enables slot on connected device
 * @param[in] res driver resources
 */
uint32_t USBSSP_EnableSlot (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = USBSSP_EnableSlotSF(res);
    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else if (res->enableSlotInProgress == 0U)
    {
        res->commandQ.enqueuePtr->dword3 = cpuToLe32(
            (uint32_t)((USBSSP_TRB_ENABLE_SLOT_COMMAND << USBSSP_TRB_TYPE_POS) |
                       (uint32_t)res->commandQ.toogleBit));
        updateQueuePtr(&res->commandQ, 0U, "CMD.ENABLE_SLOT.");
        hostCmdDoorbell(res);
        res->enableSlotInProgress = 1U;
    }
    else
    {
        /*
         * All 'if ... else if' constructs shall be terminated with an 'else'
         * statement (MISRA2012-RULE-15_7-3)
         */
    }
    return ret;
}

/**
 * Function disables slot on enabled device
 * @param[in] res driver resources
 */
uint32_t USBSSP_DisableSlot (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = USBSSP_DisableSlotSF(res);
    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        uint8_t epIndex;
        USBSSP_EpContexEpState epState;

        // first execute stop endpoint command on all running endpoints
        for (epIndex = 1U; epIndex < USBSSP_EP_CONT_MAX; epIndex++)
        {
            epState = getEndpointStatus(res, epIndex);
            if (epState != USBSSP_EP_CONTEXT_EP_STATE_RUNNING)
            {
                continue;
            }
            else
            {
                uint32_t retValue;
                retValue = USBSSP_StopEndpoint(res, epIndex);
                if (retValue != CDN_EOK)
                {
                    vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                            "<%d> Stop endpoint(%d) command error\n",
                            res->instanceNo, epIndex);
                }
            }  // else
        }  // for

        res->commandQ.enqueuePtr->dword3 = cpuToLe32((
            uint32_t)((uint32_t)((uint32_t)res->actualdeviceSlot
                                 << USBSSP_SLOT_ID_POS) |
                      (USBSSP_TRB_DISABLE_SLOT_COMMAND << USBSSP_TRB_TYPE_POS) |
                      res->commandQ.toogleBit));
        updateQueuePtr(&res->commandQ, 0U, "CMD.DISABLE_SLOT.");
        res->actualPort = 0;
        res->actualSpeed = CH9_USB_SPEED_UNKNOWN;
        res->actualdeviceSlot = 0;
        res->devAddress = 0;
        res->ep0.isRunningFlag = 0;
        setDevConfigFlag(res, 0);
        hostCmdDoorbell(res);
    }
    return ret;
}

/**
 * Function to send a force event command.
 * @param[in] res Driver resources
 * @param[in] vf_id vf_id ID of VF whose Event Ring will receive event
 * @param[in] vf_int_target vf_int_target ID of the interrupter whose Event Ring
 * will receive event
 * @param[in] eventPtr eventPtr pointer to event that will be sent
 */
uint32_t USBSSP_ForceEvent (USBSSP_DriverResourcesT *res, uint32_t vf_id,
                            uint32_t vf_int_target,
                            const USBSSP_RingElementT *eventPtr)
{
    // check if input pointer parameters are not NULL
    uint32_t ret = USBSSP_ForceEventSF(res, eventPtr);
    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        // get pointer to actual event TRB
        uint64_t trb_addr = get64PhyAddrOf32ptr(&eventPtr->dword0);

        res->commandQ.isRunningFlag = 1;
        // fill TRBs of Force Event command
        res->commandQ.enqueuePtr->dword0 = cpuToLe32(
            (uint32_t)(trb_addr & 0xFFFFFFFFULL));
        res->commandQ.enqueuePtr->dword1 = cpuToLe32(
            (uint32_t)((trb_addr >> 32) & 0xFFFFFFFFULL));
        res->commandQ.enqueuePtr->dword2 = cpuToLe32(
            (uint32_t)(vf_int_target << USBSSP_TRB_FRCEVT_VFINTTGT_POS));
        res->commandQ.enqueuePtr->dword3 = cpuToLe32(
            (uint32_t)((vf_id << USBSSP_TRB_FORCEEV_VF_ID_POS) |
                       (USBSSP_TRB_FORCE_EVENT_COMMAND << USBSSP_TRB_TYPE_POS) |
                       res->commandQ.toogleBit));
        updateQueuePtr(&res->commandQ, 0U, "CMD.FORCE_EVENT.");
        hostCmdDoorbell(res);
    }
    return ret;
}

/**
 * Completion handler for Enable Slot command
 * @param res driver resources
 */
static void cmdCmplEnableSlot (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> USBSSP_TRB_ENABLE_SLOT_COMMAND\n",
            res->instanceNo);
    res->actualdeviceSlot = getSlotId(res->eventPtr);
    if (res->actualdeviceSlot > res->maxDeviceSlot)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "<%d> commadCompletionEnableSlot: actualdeviceSlot (%d) "
                "greater than max slots (%d)\n",
                res->instanceNo, res->actualdeviceSlot, res->maxDeviceSlot);
    }
    res->enableSlotInProgress = 0U;
    res->contextEntries = 1;  // A0 and A1 enabled
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> Actual dev. slotID: %d\n",
            res->instanceNo, res->actualdeviceSlot);
    if (res->deviceModeFlag == 0U)
    {
        setAddress(res, 0);
    }
    else
    {
        // check if SET_ADDRESS setup request already handled
        if (res->ep0.isRunningFlag == 1U)
        {
            setAddress(res, 0U);
        }
        else
        {
            setAddress(res, 1U);
        }
    }
}

/**
 * Handle address device command completion
 * @param res driver resources
 */
static void cmdCmplAddressDev (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> USBSSP_TRB_ADDRESS_DEVICE_COMMAND\n", res->instanceNo);

    if (res->deviceModeFlag == 0U)
    {
        res->devAddress = 1;
    }
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> Device in addressed state\n",
            res->instanceNo);
    if ((res->deviceModeFlag == 1U) && (res->devAddress > 0U))
    {
        // status TRB
        (void)USBSSP_ControlTransferDev(res, NULL, 0, 0);
    }
}

/**
 * Handle configure endpoint command completion
 * @param res driver resources
 */
static void cmdCmplConfigureEp (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> USBSSP_TRB_CONFIGURE_ENDPOINT_COMMAND\n", res->instanceNo);

    setDevConfigFlag(res, 1);

    if (res->deviceModeFlag == 1U)
    {
        if (res->actualSpeed > CH9_USB_SPEED_HIGH)
        {
            // enable U1 and U2 for ss and ssp speed
            setU1timeout(res, 1);
            setU2timeout(res, 1);
        }
        else
        {
            uint32_t usb2_portpmsc_val = USBSSP__PORTPMSC1USB2__HLE_MASK;

            /* define CDNSP_DEFAULT_BESL 5 */
            usb2_portpmsc_val = CPS_FLD_WRITE(USBSSP__PORTPMSC1USB2, BESL,
                                              usb2_portpmsc_val, 0U);
            usb2_portpmsc_val = CPS_FLD_WRITE(USBSSP__PORTPMSC1USB2, HLE,
                                              usb2_portpmsc_val, 1U);
            usb2_portpmsc_val = CPS_FLD_WRITE(USBSSP__PORTPMSC1USB2, L1S,
                                              usb2_portpmsc_val, 2U);
            // configure USB2 LPM
            xhciWrite32(
                &res->regs.xhciPortControl[USBSSP_DEV_MODE_2_PORT].portpmsc,
                usb2_portpmsc_val);
        }
        // send status stage for SET_CONFIG request
        (void)USBSSP_ControlTransferDev(res, NULL, 0, 0);

        if (res->cusbdCallbacks->configured != NULL)
        {
            res->cusbdCallbacks->configured(res->privateData,
                                            res->enabledEndpsMask);
        }
    }
    else
    {
        // issue set configuration to device.
        setConfigurationSetupReq(res);
    }
}

/**
 * Handle reset endpoint command completion
 * @param res driver resources
 */
static void cmdCmplResetEp0 (USBSSP_DriverResourcesT *res)
{
    if (res->deviceModeFlag == 0U)
    {
        USBSSP_RingElementT *dequeuePtr = NULL;
        // get trb type
        uint32_t trbType = getTrbType(res->ep0.dequeuePtr);

        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Issue SetDequePointer for ep %d\n", res->instanceNo, 1U);
        // handle set dequeue pointer
        switch (trbType)
        {
        case USBSSP_TRB_SETUP_STAGE:
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Stalled on SETUP stage\n",
                    res->instanceNo);
            dequeuePtr = &res->ep0.dequeuePtr[3];
            break;
        case USBSSP_TRB_DATA_STAGE:
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Stalled on DATA stage\n",
                    res->instanceNo);
            dequeuePtr = &res->ep0.dequeuePtr[2];
            break;
        case USBSSP_TRB_STATUS_STAGE:
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Stalled on STATUS stage\n",
                    res->instanceNo);
            dequeuePtr = &res->ep0.dequeuePtr[1];
            break;
        default:
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Control transfer TRB mismatch\n", res->instanceNo);
            break;
        }

        // update dequeue pointer
        res->ep0.dequeuePtr = dequeuePtr;
        if (dequeuePtr == NULL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> Control transfer TRB mismatch, dequeuePtr=NULL\n",
                    res->instanceNo);
        }
        else
        {
            USBSSP_SetDequeuePointer(
                res, 1U, le64ToCpu(get64PhyAddrOf32ptr(&dequeuePtr->dword0)));
        }
    }
    else
    {
        /* in device mode we wouldn't queue in TRBs after Stall */
        /* handle setup request */
        if (res->ep0State == USBSSP_EP0_SETUP_PENDING)
        {
            processSetupDevMode(res);
        }
    }
}

/**
 * Handle reset endpoint command completion
 * @param res driver resources
 */
static void cmdCmplResetEp (USBSSP_DriverResourcesT *res)
{
    uint8_t epIndex = getEndpoint(res->commandQ.dequeuePtr);
    USBSSP_EpContexEpState endpointState = getEndpointStatus(res, epIndex);

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> USBSSP_TRB_RESET_ENDPOINT_COMMAND completed on ep %d\n",
            res->instanceNo, epIndex);
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Endpoint(%d) status: %d\n",
            res->instanceNo, epIndex, endpointState);

    // set dequeue pointer command allowed only for stopped endpoint
    if (endpointState == USBSSP_EP_CONTEXT_EP_STATE_STOPPED)
    {
        // handle default endpoint
        if (epIndex == USBSSP_EP0_CONTEXT_OFFSET)
        {
            cmdCmplResetEp0(res);
        }
        else
        {
            // handle no default endpoint
            // get endpoint object
            USBSSP_ProducerQueueT *ep = &res->ep[epIndex];
            USBSSP_ReqState reqPendingFlag;

            if ((ep->streamCount != 0U) && (ep->actualSID != 0U))
            {
                reqPendingFlag = ep->stream[ep->actualSID - 1U]->req_pending;
            }
            else
            {
                reqPendingFlag = ep->req_pending;
            }

            if (ep->isDisabledFlag > 0U)
            {
                uint32_t result = USBSSP_EndpointSetFeature(res, epIndex, 0);
                if (result != CDN_EOK)
                {
                    vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                            "<%d> Clear stall on endpoint %d failed!\n",
                            res->instanceNo, epIndex);
                }
            }
            else if (res->deviceModeFlag != 0U)
            {
                // ring doorbell to transition to Running state
                if (reqPendingFlag != USBSSP_REQUEST_COMPLETE)
                {
                    USBSSP_WriteDoorbell(
                        res, res->actualdeviceSlot,
                        ((((uint32_t)ep->actualSID) << 16) | epIndex));
                }
            }
            else
            {
                /*
                 * All 'if ... else if' constructs shall be terminated with an
                 * 'else' statement (MISRA2012-RULE-15_7-3)
                 */
            }
        }
    }
}

/**
 * Handle set transfer dequeue pointer command completion
 * @param res driver resources
 */
static void cmdCmplSetTrDePtr (USBSSP_DriverResourcesT *res)
{
    uint8_t epIndex = getEndpoint(res->commandQ.dequeuePtr);
    USBSSP_EpContexEpState endpointState = getEndpointStatus(res, epIndex);

    vDbgMsg(
        USBSSP_DBG_DRV, DBG_HIVERB,
        "<%d> USBSSP_TRB_SET_TR_DEQUEUE_POINTER_COMMAND completed on ep %d\n",
        res->instanceNo, epIndex);
    if (epIndex == USBSSP_EP0_CONTEXT_OFFSET)
    {
        // res->ep0.dequeuePtr = res->ep0.enqueuePtr;
        USBSSP_WriteDoorbell(res, res->actualdeviceSlot, epIndex);
    }
    else
    {
        if (epIndex > USBSSP_EP0_CONTEXT_OFFSET)
        {
            // res->ep[epIndex].dequeuePtr = res->ep[epIndex].enqueuePtr;
            if (endpointState == USBSSP_EP_CONTEXT_EP_STATE_STOPPED)
            {
                if (res->ep[epIndex].isRunningFlag != 0U)
                {
                    // ring doorbell to put endpoint in running state
                    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                            "<%d> DRBL: Ring doorbell on ep_index: %d\n",
                            res->instanceNo, epIndex);
                    USBSSP_WriteDoorbell(res, res->actualdeviceSlot, epIndex);
                }
            }
        }
    }
}

/**
 * Handle reset device command completion
 * @param res driver resources
 */
static void cmdCmplResetDevice (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "<%d> Reset device completed. Device address: %d\n",
            res->instanceNo, res->devAddress);

    // for host mode
    if (res->deviceModeFlag == 0U)
    {
        if (res->devAddress == 0U)
        {
            setAddress(res, 0U);
        }
    }  // for device mode
    else
    {
        if (res->devDisconnectCBPendingFlag != 0U)
        {
            res->devDisconnectCBPendingFlag = 0U;
            res->cusbdCallbacks->disconnect(res->privateData);
        }
        if ((res->devAddress > 0U) && (res->actualSpeed > CH9_USB_SPEED_HIGH))
        {
            setAddress(res, 0U);
        }
        else
        {
        }
    }
}

/**
 * Checks for the presence of set in array and returns the index
 * @param set
 * @param function array
 * @return
 */
static uint32_t remap (uint32_t command, const uint32_t *array, uint32_t size)
{
    uint32_t i, res = 0;
    for (i = 0; i < size; i++)
    {
        if (array[i] == command)
        {
            res = i;
            break;
        }
        else
        {
            // Returns the size(default case)
            res = size;
        }
    }
    return res;
}

/**
 * handleNoOp
 * @param res  driver resources
 */
static void handleNoOp (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> USBSSP_TRB_NO_OP_COMMAND\n",
            res->instanceNo);
    if (res->nopComplete != NULL)
    {
        res->nopComplete(res);
    }
}

/**
 * handleForceHeader
 * @param res  driver resources
 */
static void cmdCmplForceHeader (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Force header completed.\n",
            res->instanceNo);
    if (res->forceHeaderComplete != NULL)
    {
        res->forceHeaderComplete(res);
    }
}

/**
 * handleDisableSlot
 * @param res driver resources
 */
// parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter res not used in
// function handleDisableSlot", DRV-5631
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter res with const
 * specifier, DRV-3806" */
static void cmdCmplDisableSlot (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Disable slot completed.\n",
            res->instanceNo);
    return;
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */
// parasoft-end-suppress MISRA2012-RULE-2_7-4

/**
 * handleHaltEndpoint
 * @param res driver resources
 */
static void cmdCmplHaltEndpoint (USBSSP_DriverResourcesT *res)
{
    uint8_t epIndex = getEndpoint(res->commandQ.dequeuePtr);

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Halt endpoint(%d) completed.\n",
            res->instanceNo, getEndpoint(res->commandQ.dequeuePtr));
    if ((res->deviceModeFlag != 0U) && (epIndex == 1U))
    {
        if (res->ep0State == USBSSP_EP0_HALT_PENDING)
        {
            res->ep0State = USBSSP_EP0_HALTED;
            // stay in HALT state till a new setup request arrives.
        }
        else if (res->ep0State == USBSSP_EP0_HALT_SETUP_PENDING)
        {
            // enqueue reset ep0 command - before handling setup request
            enqueueResetEndpointCmd(res, 1U);
            hostCmdDoorbell(res);
            res->ep0State = USBSSP_EP0_SETUP_PENDING;
        }
        else
        {
            // required by MISRA
        }
    }

    if ((res->usbsspCallbacks.setInterfaceCallback != NULL) && (epIndex < 32U))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Call user defined interface callback\n", res->instanceNo);
        USBSSP_SetInterfaceState configEpCmd = USBSSP_EP_DISABLE;
        res->usbsspCallbacks.setInterfaceCallback(
            res, &configEpCmd, cpuToLe32((uint32_t)(1UL << epIndex)));
    }
    return;
}

/**
 * handleHaltStopEndpoint
 * @param res driver resources
 */
// parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter res not used in
// function handleHaltStopEndpoint", DRV-5631
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter res with const
 * specifier, DRV-3806" */
static void handleHaltStopEndpoint (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Stop endpoint(%d) completed.\n",
            res->instanceNo, getEndpoint(res->commandQ.dequeuePtr));

    if ((res->usbsspCallbacks.setInterfaceCallback != NULL) &&
        ((getEndpoint(res->commandQ.dequeuePtr)) < 32U))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Call user defined interface callback\n", res->instanceNo);

        uint32_t dropMask;
        dropMask = cpuToLe32(
            (uint32_t)(1UL << (getEndpoint(res->commandQ.dequeuePtr))));
        res->enabledEndpsMask &= ~dropMask;
        res->enabledEndpsMask &= 0xFFFFFFFCU;  // Slot context and EP0 - not
                                               // considered

        USBSSP_SetInterfaceState configEpCmd = USBSSP_EP_DISABLE;
        res->usbsspCallbacks.setInterfaceCallback(res, &configEpCmd, dropMask);
    }

    return;
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */
// parasoft-end-suppress MISRA2012-RULE-2_7-4

/**
 * handleForceEvent
 * @param res driver resources
 */
// parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter res not used in
// function handleForceEvent", DRV-5631
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter res with const
 * specifier, DRV-3806" */
static void handleForceEvent (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Force event completed.\n",
            res->instanceNo);
    return;
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */
// parasoft-end-suppress MISRA2012-RULE-2_7-4

/**
 * handleDefault
 * @param res driver resources
 */
// parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter res not used in
// function handleDefault", DRV-5631
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter res with const
 * specifier, DRV-3806" */
static void handleDefault (USBSSP_DriverResourcesT *res)
{
    vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Unknown/not supported cmd ...\n",
            res->instanceNo);
    return;
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */
// parasoft-end-suppress MISRA2012-RULE-2_7-4

/**
 * Handle successfully completed command
 * @param res driver resources
 * @param command command code
 */
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter res with const
 * specifier, DRV-3806" */
static void commandCompletionSuccess (USBSSP_DriverResourcesT *res,
                                      uint32_t command)
{
    uint32_t commands[] = {
        USBSSP_TRB_ENABLE_SLOT_COMMAND,  USBSSP_TRB_ADDR_DEV_CMD,
        USBSSP_TRB_CONF_EP_CMD,          USBSSP_TRB_NO_OP_COMMAND,
        USBSSP_TRB_RESET_EP_CMD,         USBSSP_TRB_SET_TR_DQ_PTR_CMD,
        USBSSP_TRB_RESET_DEVICE_COMMAND, USBSSP_TRB_DISABLE_SLOT_COMMAND,
        USBSSP_TRB_FORCE_HEADER_COMMAND, USBSSP_TRB_HALT_ENDP_CMD,
        USBSSP_TRB_STOP_EP_CMD,          USBSSP_TRB_FORCE_EVENT_COMMAND};
    uint32_t ch = remap(command, commands,
                        (uint32_t)(sizeof(commands) / sizeof(uint32_t)));
    static void (*function_ptr[])(USBSSP_DriverResourcesT *) = {
        cmdCmplEnableSlot,   cmdCmplAddressDev,      cmdCmplConfigureEp,
        handleNoOp,          cmdCmplResetEp,         cmdCmplSetTrDePtr,
        cmdCmplResetDevice,  cmdCmplDisableSlot,     cmdCmplForceHeader,
        cmdCmplHaltEndpoint, handleHaltStopEndpoint, handleForceEvent,
        handleDefault};
    (*function_ptr[ch])(res);
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */

/**
 * Function handles command completion
 * @param res driver resources
 */
static void handleXhciCommadCompletion (USBSSP_DriverResourcesT *res)
{
    uint32_t command;
    uint32_t completionCode = getCompletionCode(res->eventPtr);
    uint64_t dequeuePtrPhyAddr = getU64ValFromU32Ptr(&res->eventPtr->dword0);

    res->commandQ.dequeuePtr = getRingPtrFromPhyAddr(
        (uintptr_t)dequeuePtrPhyAddr);

    command = getTrbType(res->commandQ.dequeuePtr);
    res->commandQ.isRunningFlag = 0;
    res->commandQ.completePtr = res->eventPtr;
    res->commandQ.completionCode = (uint8_t)completionCode;

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> USBSSP_TRB_COMMAND_COMPLETION_EVENT (cmd@%p, type=0x%02x):\n",
            res->instanceNo, (uintptr_t)(res->commandQ.dequeuePtr), command);

    // check if completion is successful
    if (completionCode == USBSSP_TRB_COMPLETE_SUCCESS)
    {
        commandCompletionSuccess(res, command);
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Command %d failed, code: %d\n",
                res->instanceNo, command, completionCode);
    }
}

/**
 *
 * @param ep
 * @param dequeueAddr
 * @return
 */
static int32_t getStreamIdFromDequeuePtr (const USBSSP_ProducerQueueT *ep,
                                          uintptr_t dequeueAddr)
{
    int32_t streamId = -1;
    int32_t streamIdx = 0;
    /* Get pointer to the last valid TRB for this endpoint */
    USBSSP_RingElementT *poolEnd = &ep->ring[(USBSSP_PRODUCER_QUEUE_SIZE - 1U)];
    uintptr_t poolStartAddr = (uintptr_t)get64PhyAddrOf32ptr(
        &ep->ring[0].dword0);
    uintptr_t poolEndAddr = (uintptr_t)get64PhyAddrOf32ptr(&poolEnd->dword0);

    if ((dequeueAddr >= poolStartAddr) && (dequeueAddr <= poolEndAddr))
    {
        /* set streamId to '0' to indicate default endpoint */
        streamId = 0;
    }
    else
    {
        for (streamIdx = 0; streamIdx < (int32_t)ep->streamCount; streamIdx++)
        {
            const USBSSP_ProducerQueueT *steamContext = ep->stream[streamIdx];
            poolEnd = &steamContext->ring[(USBSSP_PRODUCER_QUEUE_SIZE - 1U)];
            poolStartAddr = (uintptr_t)get64PhyAddrOf32ptr(
                &steamContext->ring[0].dword0);
            poolEndAddr = (uintptr_t)get64PhyAddrOf32ptr(&poolEnd->dword0);
            if ((dequeueAddr >= poolStartAddr) && (dequeueAddr <= poolEndAddr))
            {
                streamId = streamIdx + 1;
                break;
            }
        }
    }

    return streamId;
}

/**
 * Function sets some transfer object parameters
 * @param res driver resources
 * @param transferObj transfer object
 */
static void setTransferObjInterParams (USBSSP_DriverResourcesT *res,
                                       USBSSP_ProducerQueueT *transferObj)
{
    // get endpoint ID
    uint8_t endpointId = getEndpoint(res->eventPtr);

    // store eventPtr as complete pTR
    transferObj->completePtr = res->eventPtr;

    // transfer is stoped
    transferObj->isRunningFlag = 0U;

    res->lastEpIntIndex = endpointId;
}

/* parasoft-begin-suppress METRICS-39-3 "The value of VOCF metric for a function
 * should not be higher than 4, DRV-4790" */
static USBSSP_ProducerQueueT *getTransferObj (USBSSP_DriverResourcesT *res)
{
    USBSSP_ProducerQueueT *transferObj;
    USBSSP_RingElementT *eventTrbPtr = res->eventPtr;
    uint8_t endpointId = getEndpoint(eventTrbPtr);
    uintptr_t dequeueAddr = getU64ValFromU32Ptr(&eventTrbPtr->dword0);
    USBSSP_RingElementT *dequeuePtr = (USBSSP_RingElementT *)
        getRingPtrFromPhyAddr(dequeueAddr);

    // handle EP0
    if (endpointId == USBSSP_EP0_CONTEXT_OFFSET)
    {
        transferObj = &res->ep0;
        transferObj->dequeuePtr = dequeuePtr;
    }
    else
    {
        // check if event triggered by Event Data TRB
        if (getEDbit(res->eventPtr) > 0U)
        {
            uint32_t actualSID = res->eventPtr->dword1;

            // update actualSID field in parent endpoint
            res->ep[endpointId].eventSID = (uint16_t)actualSID;

            // get the stream object corresponding to this event data trb
            transferObj = res->ep[endpointId].stream[actualSID - 1U];

            // calculate dequeue pointer ???
            transferObj->dequeuePtr = transferObj->lastQueuedTRB;
            // update actualSID field in parent endpoint
            res->ep[endpointId].actualSID = transferObj->actualSID;
            vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                    "epObj: %p, epIndex: %d,  actualSID: %d\n",
                    (void *)transferObj, transferObj->contextIndex,
                    transferObj->actualSID);
            // use parent's complete to notify higher
            // layers about stream transfer complete
            transferObj->complete = res->ep[endpointId].complete;
        }
        else
        {
            int32_t streamId = getStreamIdFromDequeuePtr(&res->ep[endpointId],
                                                         dequeueAddr);
            if (streamId < 1)
            {
                // no stream endpoint
                transferObj = &res->ep[endpointId];
                transferObj->dequeuePtr = dequeuePtr;
            }
            else
            {
                // update actualSID field in parent endpoint
                res->ep[endpointId].eventSID = (uint16_t)streamId;

                // get the stream object corresponding to this event data trb
                transferObj = res->ep[endpointId].stream[streamId - 1];

                // calculate dequeue pointer ???
                transferObj->dequeuePtr = transferObj->lastQueuedTRB;

                vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                        "epObj: %p, epIndex: %d,  actualSID: %d\n",
                        (void *)transferObj, transferObj->contextIndex,
                        transferObj->actualSID);
                // use parent's complete to notify higher
                // layers about stream transfer complete
                transferObj->complete = res->ep[endpointId].complete;
            }
        }
    }
    setTransferObjInterParams(res, transferObj);
    transferObj->completionCode = (uint8_t)getCompletionCode(eventTrbPtr);
    return transferObj;
}
/* parasoft-end-suppress METRICS-39-3 */

/**
 * Send status stage for setup with data phase
 * @param res driver resources
 * @param transferObj pointer to endpoint transfer event
 */
static void sendEP0StatusStage (USBSSP_DriverResourcesT *res,
                                const USBSSP_ProducerQueueT *transferObj)
{
    if ((res->deviceModeFlag == 1U) &&
        (transferObj->contextIndex == USBSSP_EP0_CONTEXT_OFFSET))
    {
        // make sure that interrupt is from data stage
        if (res->ep0State == USBSSP_EP0_DATA_PHASE)
        {
            // and send status stage
            (void)USBSSP_ControlTransferDev(res, NULL, 0, 0);
        }
    }
}

/**
 * Handle completion code for no default endpoint
 * @param res driver resources
 * @param transferObj pointer to endpoint transfer event
 */
/* parasoft-begin-suppress METRICS-18-3 "Follow the Cyclomatic Complexity limit
 * of 10, DRV-4789" */
static uint32_t handleEpCompletion (const USBSSP_DriverResourcesT *res,
                                    USBSSP_ProducerQueueT *transferObj)
{
    // transfered data length sum
    uint32_t userDataLenSum = 0;

    // really transferred number of bytes
    uint32_t realDataLenSum = 0;

    // get number of residue bytes
    uint32_t numOfResidue = getTrEvtTrbTransLen(res->eventPtr);

    // used in for-each loop in TD parsing
    USBSSP_RingElementT *ringIterator = transferObj->dequeuePtr;

    uint32_t trbType = (ringIterator == NULL) ? (uint32_t)0
                                              : getTrbType(ringIterator);

    uint64_t bufferPhyAddr = 0;

    // in current version is not full support for streams implemented, we
    // assume that stream has always completed
    if (transferObj->actualSID > 0U)
    {
        numOfResidue = 0U;
    }

    if ((ringIterator != NULL) &&
        ((trbType == USBSSP_TRB_NORMAL) || (trbType == USBSSP_TRB_DATA_STAGE) ||
         (trbType == USBSSP_TRB_ISOCH)))
    {
        uint8_t chainBit;
        do
        {
            uint32_t userTrbLength;
            uint32_t realTrbLength;

            // recreate user data length
            userDataLenSum += getTrTrbTransLen(ringIterator);

            // get length of actual TRB
            userTrbLength = getTrTrbTransLen(ringIterator);

            // calculate real TRB length
            realTrbLength = userTrbLength - numOfResidue;

            // check if short packet has been received
            if (realTrbLength < userTrbLength)
            {
                realDataLenSum = 0U;
            }

            // calculate number of bytes really transferred
            realDataLenSum += realTrbLength;

            // display TRB
            XHCI_DISP_TRB_PROC(ringIterator, "COMPLETE TR RING ");

            bufferPhyAddr = getU64ValFromU32Ptr(&ringIterator->dword0);

            // get previous TRB and exit if NULL
            ringIterator = getPrevTrb(transferObj, ringIterator);
            if (ringIterator == NULL)
            {
                break;
            }
            // check chain bit
            chainBit = getTrTrbChain(ringIterator);
        } while (chainBit > 0U);

        // recreate number of bytes for transfer
        transferObj->numOfBytes = userDataLenSum;

        // store really transfered bytes per transfer
        transferObj->numOfResidue = userDataLenSum - realDataLenSum;

        transferObj->lastXferActualLength = userDataLenSum - numOfResidue;
        transferObj->lastXferBufferPhyAddr = bufferPhyAddr;
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> numOfBytes: %d, realDataLenSum: %d, numOfResidue: %d\n",
                res->instanceNo, transferObj->numOfBytes, realDataLenSum,
                transferObj->numOfResidue);
    }
    return trbType;
}
/* parasoft-end-suppress METRICS-18-3 */

/**
 * Handling transfer completion status cases
 * @param res driver resources
 * @param transferObj transfer objectsetd
 * @return 1 when user callback needs to be called, 0 elsewhere
 */
static uint8_t handleCompleteStatus (USBSSP_DriverResourcesT *res,
                                     USBSSP_ProducerQueueT *transferObj)
{
    uint8_t ret = 0U;
    uint8_t epIndex = transferObj->contextIndex;

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> contextIndex: %02X (EP%d_%s)  Completion code: %d\n",
            res->instanceNo, transferObj->contextIndex,
            (transferObj->contextIndex >> 1U),
            ((transferObj->contextIndex & 1U) ? "IN" : "OUT"),
            transferObj->completionCode);

    // handle different completion codes
    switch (transferObj->completionCode)
    {
        // for success
    case USBSSP_TRB_COMPLETE_SUCCESS:
        ret = 1U;
        break;

        // when endpoint stalled
    case USBSSP_TRB_COMPLETE_STALL_ERROR:
        (void)USBSSP_ResetEndpoint(res, epIndex);
        transferObj->isDisabledFlag = 1U;
        break;

        // when short packet received
    case USBSSP_TRB_CMPL_SHORT_PKT:

        if (transferObj->ignoreShortPacket == 0U)
        {
            ret = 1U;
        }
        else
        {
            ret = 0U;
            if (getTrTrbChain(transferObj->dequeuePtr) == 0U)
            {
                transferObj->ignoreShortPacket = 0;
            }
        }

        if (getTrTrbChain(transferObj->dequeuePtr) > 0U)
        {
            transferObj->ignoreShortPacket = 1U;
        }
        break;

        // when missed service error
    case USBSSP_TRB_CMPL_MISSED_SRV_ER:
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> MISSED_SERVICE_ERROR Endpoint(%d)\n", res->instanceNo,
                epIndex);
        // since we always set IOC for the last TRB of the td, only call
        // completion for the TRB with IOC
        if (getTrTrbChain(transferObj->dequeuePtr) == 0U)
        {
            ret = 1U;
        }
        break;

    case USBSSP_TRB_CMPL_RING_UNDERRUN:  /* shared */
    case USBSSP_TRB_CMPL_RING_OVERRUN:
    case USBSSP_TRB_CMPL_NO_PNG_RSP_ER:
        ret = 1U;
        break;

    default:
        // do nothing by default
        break;
    }
    return ret;
}

/**
 * data transfer completion callback handler
 * @param res driver resources
 */
static void handleTransferCallback (USBSSP_DriverResourcesT *res,
                                    const USBSSP_RingElementT *const eventPtr)
{
    uint32_t trbType = 0;

    // get transfer object, it may be default endpoint, endpoint or stream
    // object
    USBSSP_ProducerQueueT *transferObj = getTransferObj(res);

    trbType = handleEpCompletion(res, transferObj);

    /* In host mode, we queue the data and status TRBs back to back */
    if (res->deviceModeFlag != 0U)
    {
        /* In device mode, we wait for the data st */
        if (trbType == USBSSP_TRB_STATUS_STAGE)
        {
            // call user complete callback
            if (transferObj->complete != NULL)
            {
                transferObj->complete(res, CDN_EOK, eventPtr, NULL, 0U);
            }
        }
        else
        {
            if (transferObj->complete != NULL)
            {
                transferObj->complete(res, CDN_EOK, eventPtr,
                                      (uint8_t *)getRingPtrFromPhyAddr(
                                          transferObj->lastXferBufferPhyAddr),
                                      transferObj->lastXferActualLength);

                switch (transferObj->req_pending)
                {
                case USBSSP_REQUEST_PENDING:
                    transferObj->req_pending = USBSSP_REQUEST_COMPLETE;
                    break;

                case USBSSP_REQUEST_HALTED:
                    transferObj->req_pending = USBSSP_REQUEST_PENDING;
                    break;

                default:
                    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                            "<%d> No pending request\n", res->instanceNo);
                    break;
                }
            }
            else
            {
                /*
                 * All 'if ... else if' constructs shall be terminated with an
                 * 'else' statement (MISRA2012-RULE-15_7-3)
                 */
            }

            sendEP0StatusStage(res, transferObj);
        }
    }
    else
    {
        // host mode
        if (trbType == USBSSP_TRB_DATA_STAGE)
        {
            // wait for
        }
        else if (transferObj->complete != NULL)
        {
            transferObj->complete(res, CDN_EOK, eventPtr,
                                  (uint8_t *)getRingPtrFromPhyAddr(
                                      transferObj->lastXferBufferPhyAddr),
                                  transferObj->lastXferActualLength);
        }
        else
        {
            /*
             * All 'if ... else if' constructs shall be terminated with an
             * 'else' statement (MISRA2012-RULE-15_7-3)
             */
        }
    }
}

/**
 * rings doorbell for active streams
 * @param res driver resources
 * @param eventPtr event pointer
 */
static void ringDBForActiveStreams (USBSSP_DriverResourcesT *res,
                                    const USBSSP_RingElementT *const eventPtr)
{
    uint32_t cur_stream;
    USBSSP_ProducerQueueT *stream;
    uint8_t endpointId = getEndpoint(eventPtr);

    for (cur_stream = 1U; cur_stream < USBSSP_STREAM_ARRAY_SIZE; cur_stream++)
    {
        stream = res->ep[endpointId].stream[cur_stream - 1U];
        if (((stream->stream_active == 1U) ||
             (stream->stream_rejected == 0U)) &&
            (stream->req_pending != USBSSP_REQUEST_COMPLETE) &&
            (res->ep[endpointId].drbls_count < 2U))
        {
            uint32_t drblReg = cur_stream << 16U;
            drblReg |= (uint32_t)endpointId;

            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> DRBL(%d) cur_stream (%d) \n",
                    res->instanceNo, endpointId, cur_stream);
            // ring doorbell for active streams if there is a pending request
            USBSSP_WriteDoorbell(res, res->actualdeviceSlot, drblReg);
            res->ep[endpointId].drbls_count++;
        }
    }
}

/**
 * Handles tx nrdy event
 * @param res driver resources
 * @param eventPtr event pointer
 */
static void handleTxNRDYEvent (USBSSP_DriverResourcesT *res,
                               const USBSSP_RingElementT *const eventPtr)
{
    uint32_t host_sid;
    uint32_t dev_sid;
    uint32_t cur_stream;
    USBSSP_ProducerQueueT *stream;
    uint8_t endpointId = getEndpoint(eventPtr);

    dev_sid = (eventPtr->dword0 & 0x01FFU);
    host_sid = (eventPtr->dword2 & 0x01FFU) | 0xFF00U;

    if (res->ep[endpointId].streamCount > 0U)
    {
        // check if stream is prime acknowledged
        if (host_sid == 0xFFFEU)
        {
            res->ep[endpointId].first_prime_det = 1U;
            for (cur_stream = 1U; cur_stream < USBSSP_STREAM_ARRAY_SIZE;
                 cur_stream++)
            {
                stream = res->ep[endpointId].stream[cur_stream - 1U];
                stream->stream_active = 1U;
                stream->stream_rejected = 0U;
            }
        }

        // check if stream is rejected
        if ((host_sid == 0xFFFFU) && (dev_sid < USBSSP_STREAM_ARRAY_SIZE))
        {
            stream = res->ep[endpointId].stream[dev_sid - 1U];
            stream->stream_active = 0U;
            stream->stream_rejected = 1U;
            stream->req_pending = USBSSP_REQUEST_PENDING;
            res->ep[endpointId].drbls_count--;
        }
        // ring doorbell for active streams
        ringDBForActiveStreams(res, eventPtr);
    }
}

/**
 * data transfer completion handler
 * @param res driver resources
 */
static void handleTransferEvent (USBSSP_DriverResourcesT *res,
                                 const USBSSP_RingElementT *const eventPtr)
{
    uint8_t callbackFlag = 0U;

    // get transfer object, it may be default endpoint, endpoint or stream
    // object
    USBSSP_ProducerQueueT *transferObj = getTransferObj(res);

    // check transfer status
    // handle stall etc.
    callbackFlag = handleCompleteStatus(res, transferObj);

    if ((transferObj->completionCode == USBSSP_TRB_COMPLETE_SUCCESS) &&
        (transferObj->actualSID > 0U))
    {
        res->ep[transferObj->contextIndex].drbls_count--;
    }

    if (callbackFlag == 1U)
    {
        handleTransferCallback(res, eventPtr);
    }
}

static uint32_t checkIfSlotIdCorrect (const USBSSP_RingElementT *trb)
{
    uint32_t ret = CDN_EOK;
    uint32_t trbType = getTrbType(trb);

    // check if slotId is within correct range
    // slot id is valid only for TRB's checked below
    if ((trbType == USBSSP_TRB_TRANSFER_EVENT) ||
        (trbType == USBSSP_TRB_CMD_CMPL_EVT) ||
        (trbType == USBSSP_TRB_BNDWTH_RQ_EVT) ||
        (trbType == USBSSP_TRB_DOORBELL_EVENT) ||
        (trbType == USBSSP_TRB_DEV_NOTIFCN_EVT))
    {
        uint32_t value = (uint32_t)getSlotId(trb);

        // check if slot id is within correct range
        if (value > USBSSP_MAX_DEVICE_SLOT_NUM)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "Slot ID(%d) exceeded USBSSP_MAX_DEVICE_SLOT_NUM\n", value);
            ret = CDN_EINVAL;
        }
    }
    return ret;
}

/**
 * Validate whether a dequeue pointer is correct. Works only for contiguous
 * queues
 *
 * @param queue The queue corresponding to this pointer.
 * @param dequeuePtr De-queue pointer to validate.
 * @return CDN_EOK if dequeue-pointer lies in the given queue.
 */
static uint32_t validateDequeuePtr (const USBSSP_ProducerQueueT *queue,
                                    uintptr_t dequeueAddr)
{
    uint32_t ret = CDN_EOK;
    USBSSP_RingElementT
        *poolEnd = &queue->ring[(USBSSP_PRODUCER_QUEUE_SIZE - 1U)];
    uintptr_t poolStartAddr = (uintptr_t)get64PhyAddrOf32ptr(
        &queue->ring[0].dword0);
    uintptr_t poolEndAddr = (uintptr_t)get64PhyAddrOf32ptr(&poolEnd->dword0);

    if ((dequeueAddr < poolStartAddr) || (dequeueAddr > poolEndAddr))
    {
        ret = CDN_EINVAL;
    }
    return ret;
}

/**
 * Check whether a transfer event TRB is a valid TRB
 * @param res Pointer to driver resources
 * @param trb Pointer to the TRB
 * @return CDN_EOK on success
 */
static uint32_t checkTranferEventTRB (const USBSSP_DriverResourcesT *res,
                                      const USBSSP_RingElementT *trb)
{
    uint32_t ret = CDN_EOK;
    uint8_t endpointId = getEndpoint(trb);

    // parasoft-begin-suppress MISRA2012-RULE-11_3-2 "uint32_t* converted to
    // uintptr_t*, DRV-5633"
    uintptr_t dequeueAddr = leXToCpu(*((const uintptr_t *)&trb->dword0));
    // parasoft-end-suppress MISRA2012-RULE-11_3-2

    if (endpointId == USBSSP_EP0_CONTEXT_OFFSET)
    {
        ret = validateDequeuePtr(&res->ep0, dequeueAddr);
    }
    else
    {
        uint32_t completionCode = getCompletionCode(
            trb);  // returns code within [0:255]
        // check if event NOT triggered by stream
        if ((getEDbit(res->eventPtr) == 0U) &&
            (completionCode != USBSSP_TRB_CMPL_RING_UNDERRUN) &&
            (completionCode != USBSSP_TRB_CMPL_RING_OVERRUN) &&
            (completionCode != USBSSP_TRB_CMPL_VF_EVTRNGFL_ER) &&
            (completionCode != USBSSP_TRB_CMPL_STOP_LEN_INV))
        {
            ret = validateDequeuePtr(&res->ep[endpointId], dequeueAddr);
        }
    }

    return ret;
}

/**
 * Check whether a Command Completion event TRB is a valid TRB
 * @param res Pointer to driver resources
 * @param trb Pointer to the TRB
 * @return CDN_EOK on success
 */
static uint32_t checkCmdCmplEvtTRB (const USBSSP_DriverResourcesT *res,
                                    const USBSSP_RingElementT *trb)
{
    // parasoft-begin-suppress MISRA2012-RULE-11_3-2 "uint32_t* converted to
    // uintptr_t*, DRV-5633"
    uintptr_t dequeueAddr = leXToCpu(*((const uintptr_t *)&trb->dword0));
    // parasoft-end-suppress MISRA2012-RULE-11_3-2

    uint32_t ret = validateDequeuePtr(&res->commandQ, dequeueAddr);

    return ret;
}

/**
 * Check whether the Event TRB is valid
 *
 * @param res Pointer to driver resources
 * @param trb Pointer to the Event TRB
 * @return CDN_EOK if valid Event TRB
 */
static uint32_t checkEventTrb (const USBSSP_DriverResourcesT *res,
                               const USBSSP_RingElementT *trb)
{
    uint32_t trbType;
    uint32_t value;
    uint32_t result = CDN_EOK;

    // check if TRB type field in within correct value
    trbType = getTrbType(trb);
    if (trbType > USBSSP_TRB_VF_SEC_VIOLN_EVT)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "INCORRECT TRB Type value: %d\n",
                trbType);
        result = CDN_EINVAL;
    }

    if (result == CDN_EOK)
    {
        // check if completion is within correct range
        value = getCompletionCode(trb);  // returns code within [0:255]
        if ((value > USBSSP_TRB_CMPL_SPLT_TRNSCN_ER) &&
            (value < USBSSP_TRB_CMPL_CDNSDEF_ERCODES))
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "INCORRECT COMPLETION value: %d\n", value);
            result = CDN_EINVAL;
        }
    }

    if (result == CDN_EOK)
    {
        result = checkIfSlotIdCorrect(trb);
    }

    if (result == CDN_EOK)
    {
        if (trbType == USBSSP_TRB_TRANSFER_EVENT)
        {
            result = checkTranferEventTRB(res, trb);
        }
        else if (trbType == USBSSP_TRB_CMD_CMPL_EVT)
        {
            result = checkCmdCmplEvtTRB(res, trb);
        }
        else
        {
            /*
             * All 'if ... else if' constructs shall be terminated with an
             * 'else' statement (MISRA2012-RULE-15_7-3)
             */
        }
    }
    return result;
}

/**
 *
 * @param res
 * @param trbType
 */
static void switchFunction (USBSSP_DriverResourcesT *res,
                            const USBSSP_RingElementT *const eventPtr,
                            uint32_t trbType)
{
    // check event type
    switch (trbType)
    {
    case USBSSP_TRB_PORT_ST_CHG_EVT:
#ifdef QUEUE_TEST
        noOpTest(res);
#else
        // handle events on port
        portChangeDetect(res);
#endif
        break;

        // handle command completion
    case USBSSP_TRB_CMD_CMPL_EVT:
        handleXhciCommadCompletion(res);
        break;

        // handle transfer event
    case USBSSP_TRB_TRANSFER_EVENT:
        handleTransferEvent(res, eventPtr);
        break;

        // for device only
    case USBSSP_TRB_SETUP_STAGE:
        if (res->deviceModeFlag != 0U)
        {
            handleSetupDeviceMode(res);
        }
        else
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "<%d> SSP operates in HOST mode, setup IRQ must not be "
                    "generated for that mode.\n",
                    res->instanceNo);
        }
        break;

        // handle host controller event
    case USBSSP_TRB_HOST_CTRL_EVT:
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Host Controller ERROR:\n",
                res->instanceNo);
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Completion Code: %d\n",
                res->instanceNo,
                le32ToCpu(res->eventPtr->dword2) >> USBSSP_COMPLETION_CODE_POS);
        break;

        // not ready notification
    case USBSSP_TRB_NRDY_EVT:

        // vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> NRDY on endpoint index %d\n",
        // res->instanceNo, getEndpoint(res->eventPtr)); printf( "<%d> NRDY on
        // endpoint index %d\n", res->instanceNo, getEndpoint(res->eventPtr));
        handleTxNRDYEvent(res, eventPtr);
        break;

    default:
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> TRB... %d\n", res->instanceNo,
                trbType);
        break;
    }
    return;
}

/**
 * Handling of callbacks
 *
 * @param[in] res driver resources
 */
static void handleCallbacks (USBSSP_DriverResourcesT *res)
{
    uint8_t retFunction = 0;
    uint32_t trbType;

    do
    {
        // call generic callback if defined by user - used for diagnostic
        // purposes
        if (res->usbsspCallbacks.genericCallback != NULL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                    "<%d> Call user defined generic pre callback\n",
                    res->instanceNo);
            retFunction = res->usbsspCallbacks.genericCallback(res,
                                                               res->eventPtr);
        }

        // If non-zero return from genericCallback skip to next iteration.
        if (retFunction != 0U)
        {
            updateEventPtr(res);
            continue;
        }

        XHCI_DISP_TRB_PROC(res->eventPtr, "EVENT.");

        trbType = getTrbType(res->eventPtr);
        switchFunction(res, res->eventPtr, trbType);

        if (res->usbsspCallbacks.postCallback != NULL)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                    "<%d> Call user defined generic post callback\n",
                    res->instanceNo);
            res->usbsspCallbacks.postCallback(res, res->eventPtr);
        }

        updateEventPtr(res);

        // check if any pending event still on event ring and handle it if yes
    } while (getToogleBit(res->eventPtr) == res->eventToogleBit);
}

/**
 *
 * @param res
 * @param xhciInterrupter
 */
static void isrClearUSBSTSIntEvent (const USBSSP_DriverResourcesT *res)
{
    /* Get the ISR status word*/
    uint32_t *usbstsRegPtr = &res->regs.xhciOperational->usbsts;

    uint32_t reg = xhciRead32(usbstsRegPtr);
    reg &= ~(USBSSP__USBSTS__SRE_MASK | USBSSP__USBSTS__PCD_MASK |
             USBSSP__USBSTS__HSE_MASK);
    reg |= USBSSP__USBSTS__EINT_MASK;

    xhciWrite32(usbstsRegPtr, reg);
}

/**
 * Process ISR event and update event read pointer
 * @param res driver resources
 */
static void isrProcessEvent (USBSSP_DriverResourcesT *res)
{
    uint8_t retFunction = 0;

    // check babble interrupt
    if (getToogleBit(res->eventPtr) != res->eventToogleBit)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Babble interrupt!\n",
                res->instanceNo);
        retFunction = 1;
    }
    if (retFunction == 0U)
    {
        if (checkEventTrb(res, res->eventPtr) != CDN_EOK)
        {
            retFunction = 1;
            updateEventPtr(res);
        }
    }
    if (retFunction == 0U)
    {
        handleCallbacks(res);
    }
}

/**
 * Handling of SSP controller interrupt. Function is called from SSP interrupt
 * context.
 *
 * @param[in] res driver resources
 */
uint32_t USBSSP_Isr (USBSSP_DriverResourcesT *res)
{
    static uint32_t
        irqNum;  // used for debug purposes only, counts number of irqs
    uint32_t ret = USBSSP_IsrSF(res);
    if (ret == CDN_EOK)
    {
        ++irqNum;

        vDbgMsg(USBSSP_DBG_DRV, DBG_INFLOOP, "<%d> IRQ number: %d\n",
                res->instanceNo, irqNum);

        isrClearUSBSTSIntEvent(res);

        // Enable the Interrupter by writing a '1' to the Interrupt Pending (IP)
        xhciWrite32(&res->regs.xhciInterrupter[0].iman,
                    USBSSP__IMAN0__IE_MASK | USBSSP__IMAN0__IP_MASK);

        isrProcessEvent(res);

        // Program the Interrupter Event Ring Dequeue Pointer (ERDP)
        xhciWrite64(&res->regs.xhciInterrupter[0].erdp,
                    get64PhyAddrOf32ptr(&res->eventPtr->dword0) |
                        USBSSP__ERDP0_LO__EHB_MASK);
    }
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> IRQ EXIT\n", res->instanceNo);
    return ret;
}

/**
 * Reads the capability registers.
 * Should be called after regs->xhciCapability is initialized.
 *
 * @param[in] xHCCapsAddr pointer to the xHC capability registers
 * @param[in] qaRegs pointer to the structure for storing quick Access registers
 */
static void initReadCapRegs (USBSSP_CapabilityT *xHCCapsAddr,
                             USBSSP_QuickAccessRegs *qaRegs)
{
    // get pointer to the structure for storing capability register values
    USBSSP_CapabilityT *xHCCapsRegValues = &qaRegs->xHCCaps;

    xHCCapsRegValues->hcsparams1 = xhciRead32(&xHCCapsAddr->hcsparams1);
    xHCCapsRegValues->hcsparams2 = xhciRead32(&xHCCapsAddr->hcsparams2);
    xHCCapsRegValues->hcsparams3 = xhciRead32(&xHCCapsAddr->hcsparams3);

    xHCCapsRegValues->dboff = xhciRead32(&xHCCapsAddr->dboff);
    xHCCapsRegValues->rtsoff = xhciRead32(&xHCCapsAddr->rtsoff);

    xHCCapsRegValues->hccparams1 = xhciRead32(&xHCCapsAddr->hccparams1);
    xHCCapsRegValues->hccparams2 = xhciRead32(&xHCCapsAddr->hccparams2);
}

/**
 * Sets internal pointers to extended capabilities register in driver resources
 * @param res driver resources
 * @param base memory address where controller is mapped
 */
static void setSwRegsDrblExCap (USBSSP_DriverResourcesT *res, uintptr_t base)
{
    USBSSP_SfrT *regs = &res->regs;
    USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;
    uint32_t temp;
    uint32_t *extCapsBaseAddr;

    //  ----------- set doorbell register --------------------------------------
    // parasoft-begin-suppress MISRA2012-RULE-11_4-4 "uintptr_t converted to
    // uint32_t*, DRV-5634"
    regs->xhciDoorbell = (uint32_t *)((xHCCaps->dboff) + base);
    // parasoft-end-suppress MISRA2012-RULE-11_4-4
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> XHCI doorbell address: %p\n",
            res->instanceNo, regs->xhciDoorbell);

    // ---------- set extended capabilities registers -------------
    temp = CPS_FLD_READ(USBSSP__HCCPARAMS1, XECP, xHCCaps->hccparams1);
    if (temp > 0U)
    {
        temp <<= 2;  // offset is given in DWORDs so multiply by 4
        // parasoft-begin-suppress MISRA2012-RULE-11_4-4 "uintptr_t converted to
        // uint32_t*, DRV-5634"
        extCapsBaseAddr = (uint32_t *)(temp + base);
        // parasoft-end-suppress MISRA2012-RULE-11_4-4
        regs->xhciExtCaps.extCapsBaseAddr = extCapsBaseAddr;
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> XHCI extended capabilities base address: %p\n",
                res->instanceNo, regs->xhciExtCaps.extCapsBaseAddr);
    }
    else
    {
        regs->xhciExtCaps.extCapsBaseAddr = NULL;
    }
}

/**
 * Sets internal pointers to register in driver resources
 * @param res driver resources
 * @param base memory address where controller is mapped
 */
static void setSwRegs (USBSSP_DriverResourcesT *res, uintptr_t base)
{
    USBSSP_SfrT *regs = &res->regs;
    USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;
    uint32_t temp;

    // ------------- set capability register address ----------------------

    // set base address
    // parasoft-begin-suppress MISRA2012-RULE-11_4-4 "uintptr_t converted to
    // USBSSP_CapabilityT*, DRV-5634"
    regs->xhciCapability = (USBSSP_CapabilityT *)base;
    // parasoft-end-suppress MISRA2012-RULE-11_4-4
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> XHCI capability address: %p\n",
            res->instanceNo, &regs->xhciCapability->caplength_hciver);

    // set operability offset
    xHCCaps->caplength_hciver = xhciRead32(
        &regs->xhciCapability->caplength_hciver);

    // read the xHCI capability registers and keep a local copy
    initReadCapRegs(regs->xhciCapability, &res->qaRegs);

    // ------------- set operational register --------------------------------

    temp = CPS_FLD_READ(USBSSP__HCIVERSION_CAPLENGTH, CAPLENGTH,
                        (xHCCaps->caplength_hciver));
    if (temp > 0U)
    {
        // parasoft-begin-suppress MISRA2012-RULE-11_4-4 "uint8_t* converted to
        // USBSSP_OperationalT*, DRV-5634"
        regs->xhciOperational = (USBSSP_OperationalT *)(base + temp);
        // parasoft-end-suppress MISRA2012-RULE-11_4-4
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> XHCI operational address: %p\n",
                res->instanceNo, &regs->xhciOperational->usbcmd);
    }
    // ------------- set runtime register --------------------------------------

    // set runtime offset
    // parasoft-begin-suppress MISRA2012-RULE-11_4-4 "uint32_t* converted to
    // USBSSP_RuntimeT*, DRV-5634"
    regs->xhciRuntime = (USBSSP_RuntimeT *)((xHCCaps->rtsoff) + base);
    // parasoft-end-suppress MISRA2012-RULE-11_4-4
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> XHCI runtime address: %p\n",
            res->instanceNo, &regs->xhciRuntime->mfindex);

    // ------------ set port control registers ---------------------------------
    regs->xhciPortControl = &regs->xhciOperational->portControl;

    // ------------- set interrupters registers --------------------------------
    regs->xhciInterrupter = &regs->xhciRuntime->interrupters;

    // set extended capabilities pointer
    setSwRegsDrblExCap(res, base);
}

/**
 * Displays controller feature: interrupters
 * @param res driver resources
 */
static void initSfrObjDispInterrupter (const USBSSP_DriverResourcesT *res)
{
    const USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;
#ifdef DEBUG
    const USBSSP_SfrT *regs = &res->regs;
#endif
    uint8_t numOfInterrupters;
    uint8_t i;

    // ------------- interrupters ----------------------------------------------
    numOfInterrupters = (uint8_t)CPS_FLD_READ(USBSSP__HCSPARAMS1, MAXINTRS,
                                              xHCCaps->hcsparams1);
    // display addresses of all interrupters
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Number of interrupters: %d\n",
            res->instanceNo, numOfInterrupters);
    for (i = 0U; i < numOfInterrupters; i++)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> interrupter[%d] address: %p\n",
                res->instanceNo, i, &regs->xhciInterrupter[i].iman);
    }
}

/**
 * Displays controller feature: ports
 * @param res driver resources
 */
static void initSfrObjDispPorts (const USBSSP_DriverResourcesT *const res)
{
    const USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;
#ifdef DEBUG
    const USBSSP_SfrT *regs = &res->regs;
#endif
    uint8_t numOfPorts;
    uint8_t i;

    // --------------- port features -------------------------------------------
    // check number of ports
    numOfPorts = (uint8_t)CPS_FLD_READ(USBSSP__HCSPARAMS1, MAXPORTS,
                                       xHCCaps->hcsparams1);
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Number of ports: %d\n",
            res->instanceNo, numOfPorts);
    // print addresses for all endpoints
    for (i = 0U; i < numOfPorts; i++)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> XHCI port_control[%d] address: %p\n", res->instanceNo, i,
                &regs->xhciPortControl[i].portsc);
    }
}

/**
 * Displays controller feature: device slot number
 * @param res driver resources
 */
static void initSfrObjDispDevSlotNum (USBSSP_DriverResourcesT *res)
{
    USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;
    uint32_t maxDeviceSlot;

    // --------------- device slots --------------------------------------------
    maxDeviceSlot = CPS_FLD_READ(USBSSP__HCSPARAMS1, MAXSLOTS,
                                 xHCCaps->hcsparams1);
    if (maxDeviceSlot > USBSSP_MAX_DEVICE_SLOT_NUM)
    {
        res->maxDeviceSlot = USBSSP_MAX_DEVICE_SLOT_NUM;
        vDbgMsg(USBSSP_DBG_DRV, DBG_WARN,
                "<%d> Number of hw slots (%d) greater than %d\n",
                res->instanceNo, maxDeviceSlot, USBSSP_MAX_DEVICE_SLOT_NUM);
    }
    else
    {
        res->maxDeviceSlot = maxDeviceSlot;
    }
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Number of device slots: %d\n",
            res->instanceNo, res->maxDeviceSlot);
}

/**
 * Displays controller feature: extended capabilities
 * @param res driver resources
 */
static void initSfrObjDispExtCap (USBSSP_DriverResourcesT *res)
{
    USBSSP_SfrT *regs = &res->regs;
    uint8_t i;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "<%d> ExtCap base address: 0x%p, USBLEGSUP=0x%08X / "
            "USBLEGCTLS=0x%08X\n",
            res->instanceNo, (uintptr_t)regs->xhciExtCaps.extCapsBaseAddr,
            regs->xhciExtCaps.usbLegSup, regs->xhciExtCaps.usbLegCtlSts);

    for (i = 0U; i < regs->xhciExtCaps.extCapsCount; i++)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Ext. cap. %d @(%p): firstCapPtr:%p, DWORD[0]=0x%08X, "
                "capID=%d\n",
                res->instanceNo, i, (uintptr_t)&regs->xhciExtCaps.extCaps[i],
                regs->xhciExtCaps.extCaps[i].firstCapSfrPtr,
                regs->xhciExtCaps.extCaps[i].firstDwordVal,
                regs->xhciExtCaps.extCaps[i].capId);
    }
}

/**
 * initialize extension capabilities
 * @param res driver resources
 */
static void initSfrObjExtCap (USBSSP_DriverResourcesT *res)
{
    uint8_t numOfExtCaps;
    uint8_t nextCapPtr;
    uint32_t *extCapsBaseAddr;
    uint32_t *extCapSfrIter;
    USBSSP_ExtCapElemT *extCapElemIter;
    USBSSP_SfrT *regs = &res->regs;

    // Reading USBLEGSUP and USBLEGCTLSTS

    extCapsBaseAddr = regs->xhciExtCaps.extCapsBaseAddr;
    regs->xhciExtCaps.usbLegSup = xhciRead32(extCapsBaseAddr);
    regs->xhciExtCaps.usbLegCtlSts = xhciRead32(&extCapsBaseAddr[1]);

    numOfExtCaps = 0;

    extCapElemIter = &(regs->xhciExtCaps.extCaps[0]);
    nextCapPtr = (uint8_t)CPS_FLD_READ(
        USBSSP__XEC_USBLEGSUP, NEXT_XHCI_XCAP_PTR, regs->xhciExtCaps.usbLegSup);
    extCapSfrIter = &extCapsBaseAddr[nextCapPtr];

    while ((numOfExtCaps < USBSSP_MAX_EXT_CAPS_COUNT) && (nextCapPtr > 0U))
    {
        uint8_t capId;

        // Reading first DWORD of ExtCap and decoding next capability pointer /
        // capability ID
        uint32_t firstExtCapDword = xhciRead32(extCapSfrIter);
        // Next Extended Capabilities pointer is always in the same place
        nextCapPtr = (uint8_t)CPS_FLD_READ(
            USBSSP__XEC_USBLEGSUP, NEXT_XHCI_XCAP_PTR, firstExtCapDword);
        // Capability ID is always in the same place
        capId = (uint8_t)CPS_FLD_READ(USBSSP__XEC_USBLEGSUP, USBLEGSUP_CAP_ID,
                                      firstExtCapDword);

        if (capId > 0U)
        {
            // Storing first DWORD of data, Capability ID and pointer to first
            // SFR
            extCapElemIter->firstDwordVal = firstExtCapDword;
            extCapElemIter->capId = capId;
            extCapElemIter->firstCapSfrPtr = extCapSfrIter;
        }

        // Advancing number of capabilities and moving pointers
        numOfExtCaps++;
        extCapElemIter++;
        extCapSfrIter = &extCapSfrIter[nextCapPtr];
    }

    // Updating Ext. Caps number
    regs->xhciExtCaps.extCapsCount = numOfExtCaps;
}

/**
 * Initialization of register object. Fields of regs object are consistent with
 * XHCI specification, all pointers of xhci_sfr_t object are set according to
 * capability registers information
 *
 * @param[in] res driver resources
 * @param[in] base physical address where SSP controller is mapped
 */
static void initSfrObj (USBSSP_DriverResourcesT *res, uintptr_t base)
{
    // set driver resources internal pointer to controller register
    setSwRegs(res, base);

    // display port info
    initSfrObjDispPorts(res);

    // display device slot number info
    initSfrObjDispDevSlotNum(res);

    // display interrupters info
    initSfrObjDispInterrupter(res);

    // initialize extended capabilities
    initSfrObjExtCap(res);

    // display extended capabilities
    initSfrObjDispExtCap(res);
}

/**
 * Allocate memory buffers required by SSP controller
 * @param res driver resources
 */
static uint32_t initAllocateBuffers (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = 0;
    // initialize xhci memory, all buffers should be allocated according to
    // rules described in table 54 (chapter 6) of XHCI specification

    USBSSP_XhciResourcesT *memRes = res->xhciMemRes;
    res->ep0Buff = memRes->ep0Buffer;

    // set address of input context
    res->inputContext = memRes->inputContext;

    ret = checkStructAlign(
        "INPUT CONTEXT",
        get64PhyAddrOf32ptr(res->inputContext->inputControlContext),
        sizeof(USBSSP_InputContexT), USBSSP_CONTEXT_ALIGNMENT,
        USBSSP_PAGE_SIZE);

    ret |= checkStructAlign(
        "OUTPUT CONTEXT",
        get64PhyAddrOf32ptr(res->xhciMemRes->outputContext->slot),
        sizeof(USBSSP_OutputContexT), USBSSP_CONTEXT_ALIGNMENT,
        USBSSP_PAGE_SIZE);

    ret |= checkStructAlign(
        "DCBAA", get64PhyAddrOf64ptr(&res->xhciMemRes->dcbaa->scratchPadPtr),
        sizeof(USBSSP_DcbaaT), USBSSP_DCBAA_ALIGNMENT, USBSSP_PAGE_SIZE);

    return ret;
}

/**
 * Setup of scratch pad buffers
 * @param res driver resources
 * @return CDN_EOK if setup successful, error code elsewhere
 */
static uint32_t initXhcSetupScratchPad (USBSSP_DriverResourcesT *res)
{
    uint16_t i;
    uint32_t ret = CDN_EOK;
    USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;

    // check number of scratch pad buffers
    uint16_t maxScratchBuf = (uint16_t)((uint16_t)CPS_FLD_READ(
                                            USBSSP__HCSPARAMS2, MAXSPBUFHI,
                                            xHCCaps->hcsparams2)
                                        << 5U) |
                             (uint16_t)CPS_FLD_READ(USBSSP__HCSPARAMS2,
                                                    MAXSPBUFLO,
                                                    xHCCaps->hcsparams2);
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> Number of scratch pad buffers: %d\n", res->instanceNo,
            maxScratchBuf);

    if (maxScratchBuf > USBSSP_SCRATCHPAD_BUFF_NUM)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Incorrect scratch pad number\n",
                res->instanceNo);
        ret = CDN_ENOMEM;  // set error code
    }

    if (ret == CDN_EOK)
    {
        // set pointers to all buffers
        for (i = 0; i < maxScratchBuf; i++)
        {
            // parasoft-begin-suppress MISRA2012-RULE-11_4-4 "uint8_t* converted
            // to uint64_t*, DRV-5634"
            res->xhciMemRes->scratchpad[i] = cpuToLe64(get64PhyAddrOf8ptr(
                &res->xhciMemRes->scratchpadPool[i * USBSSP_PAGE_SIZE]));
            // parasoft-end-suppress MISRA2012-RULE-11_4-4
        }
        res->xhciMemRes->scratchpad[maxScratchBuf] = 0;

        // set first element of DCBAA to 0 if no scratch pad buffers
        if (maxScratchBuf == 0U)
        {
            res->xhciMemRes->dcbaa->scratchPadPtr = 0U;
        }
        else
        {
            res->xhciMemRes->dcbaa->scratchPadPtr = cpuToLe64(
                get64PhyAddrOf64ptr(res->xhciMemRes->scratchpad));
        }
    }
    return ret;
}

/**
 * Display capabilities
 * @param res driver resources
 */
/* parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter res not used in
 * function initXhcSetupDispCap, DRV-5631" */
static void initXhcSetupDispCap (const USBSSP_DriverResourcesT *res)
{
#ifdef DEBUG
    const USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;
#endif
    // display all capability parameters 5.3.6 in spec
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> HCCPARAMS1:"
            "AC64=%d, "
            "BNC=%d, "
            "CSZ=%d, "
            "PPC=%d, "
            "PIND=%d, "
            "LHRC=%d, "
            "LTC=%d, "
            "NSS=%d, "
            "PAE=%d, "
            "SPC=%d, "
            "SEC=%d, "
            "CFC=%d, "
            "MaxPSASize=%d, "
            "xECP=0x%08X\n",
            res->instanceNo,
            CPS_FLD_READ(USBSSP__HCCPARAMS1, AC64, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, BNC, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, CSZ, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, PPC, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, PIND, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, LHRC, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, LTC, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, NSS, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, PAE, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, SPC, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, SEC, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, CFC, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, MAXPSASIZE, xHCCaps->hccparams1),
            CPS_FLD_READ(USBSSP__HCCPARAMS1, XECP, xHCCaps->hccparams1));

    // display all capability parameters 5.3.9 in spec
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> HCCPARAMS2:"
            " U3C=%d,"
            " CMC=%d,"
            " FSC=%d,"
            " CTC=%d,"
            " LEC=%d,"
            " CIC=%d\n",
            res->instanceNo,
            CPS_FLD_READ(USBSSP__HCCPARAMS2, U3C, xHCCaps->hccparams2),
            CPS_FLD_READ(USBSSP__HCCPARAMS2, CMC, xHCCaps->hccparams2),
            CPS_FLD_READ(USBSSP__HCCPARAMS2, FSC, xHCCaps->hccparams2),
            CPS_FLD_READ(USBSSP__HCCPARAMS2, CTC, xHCCaps->hccparams2),
            CPS_FLD_READ(USBSSP__HCCPARAMS2, LEC, xHCCaps->hccparams2),
            CPS_FLD_READ(USBSSP__HCCPARAMS2, CIC, xHCCaps->hccparams2));
}
/* parasoft-end-suppress MISRA2012-RULE-2_7-4 */

/**
 * set slot number in controller
 * @param res driver resources
 */
static void initXhcSetupSlotNum (USBSSP_DriverResourcesT *res)
{
    // select minimal slot device number:
#ifdef DEBUG
    uint32_t devSlotNum = (res->maxDeviceSlot > USBSSP_MAX_DEVICE_SLOT_NUM)
                              ? USBSSP_MAX_DEVICE_SLOT_NUM
                              : res->maxDeviceSlot;
#endif
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> Set slot number: \n",
            res->instanceNo, devSlotNum);

    // Program the Max Device Slots Enabled (MaxSlotsEn)
    if (USBSSP_MAX_DEVICE_SLOT_NUM > res->maxDeviceSlot)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "<%d> Maximum slots (%0d) greater than number supported by "
                "Controller (%d)\n",
                res->instanceNo, USBSSP_MAX_DEVICE_SLOT_NUM,
                res->maxDeviceSlot);
        xhciWrite32(&res->regs.xhciOperational->config, res->maxDeviceSlot);
    }
    else
    {
        xhciWrite32(&res->regs.xhciOperational->config,
                    USBSSP_MAX_DEVICE_SLOT_NUM);
    }
}

/**
 * Check context size
 * @param res driver resources
 * @return CDN_EOK on success, error code elsewhere
 */
static uint32_t initXhcSetupCheckCntxSize (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    USBSSP_CapabilityT *xHCCaps = &res->qaRegs.xHCCaps;
    uint8_t csz;  // keeps context size

    // set slot number
    initXhcSetupSlotNum(res);
    csz = (uint8_t)CPS_FLD_READ(USBSSP__HCCPARAMS1, CSZ, xHCCaps->hccparams1);

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> Context size bit: %d\n",
            res->instanceNo, csz);
    if (csz != USBSSP_EXTENDED_CONTEXT)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "<%d> Context size doesn't suit declared CSZ value\n",
                res->instanceNo);
        ret = CDN_EINVAL;
    }
    return ret;
}

/**
 * Check memory page size which is supported by controller
 * @param res driver resources
 * @return CDN_EOK on success, error code elsewhere
 */
static uint32_t initXhcSetupCheckPageSize (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;

    uint16_t pageSize;
    uint32_t reg32Value;

    // check page size
    reg32Value = xhciRead32(&res->regs.xhciOperational->pagesize);
    pageSize = (uint16_t)CPS_FLD_READ(USBSSP__PAGESIZE, PAGESIZE, reg32Value);
    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> Page size: %d\n", res->instanceNo,
            pageSize);

    // page size = 1 means 4096 size in bytes
    // page size = 16 means 64K size in bytes
    if ((pageSize != 1U) && (pageSize != 16U))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "<%d> Incorrect page size\n",
                res->instanceNo);
        ret = CDN_ENOMEM;  // set error code
    }
    return ret;
}

/**
 * Halt xHC.
 * @param[in] res driver resources
 */
static uint32_t xhcHalt (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    USBSSP_OperationalT *xhciOperationalRegs = res->regs.xhciOperational;
    uint32_t usbCmdReg = xhciRead32(&xhciOperationalRegs->usbcmd);

    if ((usbCmdReg & USBSSP__USBCMD__R_S_MASK) == USBSSP__USBCMD__R_S_MASK)
    {
        // Clear Run/Stop bit in USBCMD
        usbCmdReg &= ~(uint32_t)USBSSP__USBCMD__R_S_MASK;
        xhciWrite32(&res->regs.xhciOperational->usbcmd, usbCmdReg);
    }
    ret = waitForReg(&xhciOperationalRegs->usbsts, USBSSP__USBSTS__HCH_MASK,
                     USBSSP__USBSTS__HCH_MASK, USBSSP_DEFAULT_TIMEOUT);
    return ret;
}

/**
 *
 * @param res
 * @return
 */
static uint32_t xhcReset (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    USBSSP_OperationalT *xhciOperationalReg = res->regs.xhciOperational;
    uint32_t usbCmdReg = xhciRead32(&xhciOperationalReg->usbcmd);
    uint32_t usbStsReg = xhciRead32(&xhciOperationalReg->usbsts);

    /* Software shall not reset XHC if XHC is not halted */
    if ((usbStsReg & USBSSP__USBSTS__HCH_MASK) == USBSSP__USBSTS__HCH_MASK)
    {
        usbCmdReg |= (uint32_t)USBSSP__USBCMD__HCRST_MASK;
        xhciWrite32(&res->regs.xhciOperational->usbcmd, usbCmdReg);
        ret = waitForReg(&res->regs.xhciOperational->usbcmd,
                         USBSSP__USBCMD__HCRST_MASK, 0U,
                         USBSSP_DEFAULT_TIMEOUT);
    }
    else
    {
        ret = CDN_EPERM;
    }
    return ret;
}

/**
 * Program the device context base array fields
 * @param res driver resources
 * @return CDN_EOK on success, error code elsewhere
 */
static void initXhcSetupDcbaa (USBSSP_DriverResourcesT *res)
{
    uint64_t dcbaap_sfr_val;

    res->xhciMemRes->dcbaa->deviceSlot[0] = cpuToLe64(
        get64PhyAddrOf32ptr(res->xhciMemRes->outputContext->slot));

    // Program the Device Context Base Address Array Pointer (DCBAAP) for the
    // scratchpad (slot0)
    dcbaap_sfr_val = get64PhyAddrOf64ptr(
        &res->xhciMemRes->dcbaa->scratchPadPtr);
    
    xhciDCBAAFlush();
    
    CPS_MemoryBarrier();

    xhciWrite64(&res->regs.xhciOperational->dcbaap, dcbaap_sfr_val);
}

/**
 * Check of sizes for Setup of xHC.
 *
 * @param[in] res driver resources
 * @return CDN_EINVAL or CDN_ENOMEM when unsuccessful
 * @return CDN_EOK if no errors
 */
static uint32_t initXhcSetupChecks (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;

    // check context size
    ret = initXhcSetupCheckCntxSize(res);

    if (ret == CDN_EOK)
    {
        ret = initXhcSetupCheckPageSize(res);
    }

    return ret;
}

/**
 * Initialization of SSPDriverResourcesT object: Setup of xHC.
 *
 * @param[in] res driver resources
 * @return CDN_EINVAL or CDN_ENOMEM when unsuccessful
 * @return CDN_EOK if no errors
 */
static uint32_t initXhcSetup (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;

    vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
            "<%d> Initializing, DEV_MODE=%d, USB_MODE=%d\n", res->instanceNo,
            res->deviceModeFlag, res->usbModeFlag);

    // display HCC capabilities
    initXhcSetupDispCap(res);

    // ------ do XHCI reset --------------
    // wait until the Controller Not Ready (CNR) flag in the USBSTS is '0'
    ret = waitForReg(&res->regs.xhciOperational->usbsts,
                     USBSSP__USBSTS__CNR_MASK, 0U, USBSSP_DEFAULT_TIMEOUT);

    /* Ensure that XHC is halted */
    if (ret == CDN_EOK)
    {
        ret = xhcHalt(res);
    }

    /* reset xhc */
    if (ret == CDN_EOK)
    {
        ret = xhcReset(res);
    }
    // ------ reset end -----------------

    // check context size
    if (ret == CDN_EOK)
    {
        ret = initXhcSetupChecks(res);
    }

    if (ret == CDN_EOK)
    {
        ret = initXhcSetupScratchPad(res);
    }

    if (ret == CDN_EOK)
    {
        initXhcSetupDcbaa(res);
    }

    return ret;
}

/**
 * Program interrupters
 * @param res driver resources
 */
static void initRingsInterruptsInterupters (USBSSP_DriverResourcesT *res)
{
    uint64_t erdp_sfr_val;
    uintptr_t erstba_sfr_val;

    erstba_sfr_val = get64PhyAddrOf64ptr(
        res->xhciMemRes->eventRingSegmentEntry);

    xhciEventRingFlush();
    
    CPS_MemoryBarrier();

    // Program the Interrupter Event Ring Segment Table Size (ERSTSZ)
    xhciWrite32(&res->regs.xhciInterrupter[0].erstsz, 1);  // one event ring

    // Program the Interrupter Event Ring Dequeue Pointer (ERDP)
    erdp_sfr_val = get64PhyAddrOf32ptr(&res->eventPtr->dword0);
    xhciWrite64(&res->regs.xhciInterrupter[0].erdp, erdp_sfr_val);

    // Program the Interrupter Event Ring Segment Table Base Address (ERSTBA)
    xhciWrite64(&res->regs.xhciInterrupter[0].erstba, erstba_sfr_val);

    // Initializing the Interval field of the Interrupt Moderation register
    xhciWrite32(&res->regs.xhciInterrupter[0].imod, 0);

    // Enable the Interrupter by writing a '1' to the Interrupt Enable (IE)
    xhciWrite32(&res->regs.xhciInterrupter[0].iman, USBSSP__IMAN0__IE_MASK);
}

/**
 * initialize endpoints rings
 * @param res driver resources
 */
static void initRingsInterruptsEpRings (USBSSP_DriverResourcesT *res)
{
    uint8_t i;
    USBSSP_ProducerQueueT *ep;

    // initialize all software endpoints
    for (i = 1U; i < (USBSSP_MAX_EP_CONTEXT_NUM + USBSSP_EP_CONT_OFFSET); i++)
    {
        // index 1 is a default endpoint
        if (i == 1U)
        {
            ep = &res->ep0;
        }
        else
        {
            // no default endpoint are organized in container
            ep = &res->ep[i];
        }

        // set ring and enqueue/dequeue pointers
        ep->ring = &res->xhciMemRes->epRingPool[USBSSP_PRODUCER_QUEUE_SIZE * i];
        ep->toogleBit = 1;
        ep->enqueuePtr = ep->ring;
        ep->dequeuePtr = ep->ring;
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB,
                "<%d> Endpoint context[%d] transfer ring v-address: %p "
                "phy-addr: %p\n",
                res->instanceNo, i, (void *)ep->ring,
                get64PhyAddrOfVoidPtr((void *)ep->ring));
    }
}

/**
 * initialize commend ring
 * @param res driver resources
 */
static void initRingsInterruptsCommandRing (USBSSP_DriverResourcesT *res)
{
    // set start of ring
    res->commandQ.ring = res->xhciMemRes->epRingPool;

    // set toggle bit
    res->commandQ.toogleBit = 1;

    // reset dequeue and enqueue pointer
    res->commandQ.dequeuePtr = res->commandQ.ring;
    res->commandQ.enqueuePtr = res->commandQ.ring;
}

/**
 * Initialization of SSPDriverResourcesT object: Setup of rings and interrupts.
 *
 * @param[in] res driver resources
 * @return CDN_EINVAL when unsuccessful
 * @return CDN_EOK if no errors
 */
static uint32_t initRingsInterrupts (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    uintptr_t crcr_sfr_val;

    // initialize software command ring
    initRingsInterruptsCommandRing(res);

    // get start address of command ring
    crcr_sfr_val = get64PhyAddrOf32ptr(&res->commandQ.ring[0].dword0);

    // set event pointer to start of allocated buffer
    res->eventPtr = res->xhciMemRes->eventPool;

    // check if pointers are allocated according to spec requirements
    ret = checkStructAlign(
        "CMD_RING", crcr_sfr_val,
        sizeof(USBSSP_RingElementT) * USBSSP_PRODUCER_QUEUE_SIZE,
        USBSSP_RING_ALIGNMENT, USBSSP_RING_BOUNDARY);
    ret |= checkStructAlign(
        "EVT_RING", get64PhyAddrOf32ptr(&res->eventPtr->dword0),
        sizeof(USBSSP_RingElementT) * USBSSP_EVENT_QUEUE_SIZE,
        USBSSP_RING_ALIGNMENT, USBSSP_RING_BOUNDARY);
    ret |= checkStructAlign(
        "ERST", get64PhyAddrOf64ptr(res->xhciMemRes->eventRingSegmentEntry),
        sizeof(uint64_t) * (2U), USBSSP_ERST_ALIGNMENT, USBSSP_ERST_BOUNDARY);

    if (ret == CDN_EOK)
    {
        // Define the Command Ring Dequeue Pointer by programming the Command
        // Ring Control Register
        xhciWrite64(&res->regs.xhciOperational->crcr,
                    (uint64_t)(crcr_sfr_val | res->commandQ.toogleBit));

        // initialize endpoint rings
        initRingsInterruptsEpRings(res);

        // Allocate the Event Ring Segment Table (ERST) (section 6.5).
        // Initialize ERST table entries to point to and to define the size (in
        // TRBs) of the respective Event Ring Segment.
        res->eventToogleBit = 1;
        res->xhciMemRes->eventRingSegmentEntry[0] = cpuToLe64(
            get64PhyAddrOf32ptr(&res->eventPtr->dword0));
        res->xhciMemRes->eventRingSegmentEntry[1] = cpuToLe64(
            USBSSP_EVENT_QUEUE_SIZE);

        // initialize interrupters
        initRingsInterruptsInterupters(res);
    }
    return ret;
}

/**
 * Reset Command ring: called after controller stopped (during SaveState).
 *
 * @param[in] res driver resources
 */
static void resetCmdRing (USBSSP_DriverResourcesT *res)
{
    res->commandQ.toogleBit = 1;
    res->commandQ.dequeuePtr = res->commandQ.ring;  // set dequeue pointer to
                                                    // the begin of ring
    res->commandQ.enqueuePtr = res->commandQ.ring;  // set enqueue pointer to
                                                    // the begin of ring

    // clear whole ring to zero
    (void)memset((void *)res->commandQ.ring, 0,
                 sizeof(USBSSP_RingElementT) * USBSSP_PRODUCER_QUEUE_SIZE);
}

/**
 * Clear port bits
 *
 * @param[in] portsc port control register
 * @param[in] portbits to clear
 */
static void devPortscTestClear (uint32_t *portsc, uint32_t portBits)
{
    uint32_t temp = 0;

    /* test and clear port bits */
    temp = xhciRead32(portsc);
    if ((temp & portBits) != 0U)
    {
        temp &= USBSSP_DEV_USB23_PORT_RO;
        temp &= USBSSP_DEV_USB23_PORT_RWS;
        temp |= portBits;
        xhciWrite32(portsc, temp);
    }
}

/**
 * Set link state.
 *
 * @param[in] portsc port control register
 * @param[in] linkstate
 */
static void devPortscSetLinkState (uint32_t *portsc, uint32_t linkState)
{
    uint32_t temp = 0;

    /* set link state */
    temp = xhciRead32(portsc);
    temp &= USBSSP_DEV_USB23_PORT_RO;
    temp &= USBSSP_DEV_USB23_PORT_RWS;
    temp &= ~USBSSP__PORTSC1USB3__PLS_MASK;
    temp |= USBSSP__PORTSC1USB3__LWS_MASK | linkState;
    xhciWrite32(portsc, temp);
}

/**
 * Initialization of SSPDriverResourcesT object: Set up port control registers.
 *
 * @param[in] res driver resources
 */
/* parasoft-begin-suppress MISRA2012-RULE-8_13_a "Pass parameter
 * extCap3xPortRegs with const specifier, DRV-3806" */
static void setPortControlRegisters (USBSSP_DriverResourcesT *res,
                                     uint32_t *extCap3xPortRegs)
{
    uint32_t *extCap3xPortMdReg = &extCap3xPortRegs[1];
    uint32_t extCap3xPortMdRegVal = xhciRead32(extCap3xPortMdReg);

    /* clear pending interrupts and power down USB 3 port */
    devPortscTestClear(
        &res->regs.xhciPortControl[USBSSP_DEV_MODE_3_PORT].portsc,
        USBSSP_DEV_PORTSC_CHANGE_BITS | USBSSP__PORTSC1USB3__PED_MASK);
    xhciWrite32(&res->regs.xhciPortControl[USBSSP_DEV_MODE_3_PORT].portsc,
                USBSSP__PORTSC1USB3__PED_MASK);
    (void)waitForReg(&res->regs.xhciPortControl[USBSSP_DEV_MODE_3_PORT].portsc,
                     USBSSP__PORTSC1USB3__PED_MASK, 0, USBSSP_DEFAULT_TIMEOUT);

    if (res->usbModeFlag == 2U)
    {
        extCap3xPortMdRegVal = CPS_FLD_SET(USBSSP__XEC_CFG_3XPORT_MODE_ADDR,
                                           CFG_DISABLE_3XPORT_UFP,
                                           extCap3xPortMdRegVal);
        extCap3xPortMdRegVal = CPS_FLD_SET(USBSSP__XEC_CFG_3XPORT_MODE_ADDR,
                                           CFG_DISABLE_3XPORT_DFP,
                                           extCap3xPortMdRegVal);
        xhciWrite32(extCap3xPortMdReg, extCap3xPortMdRegVal);
    }
    else
    {
        extCap3xPortMdRegVal = xhciRead32(extCap3xPortMdReg);
        extCap3xPortMdRegVal = CPS_FLD_CLEAR(USBSSP__XEC_CFG_3XPORT_MODE_ADDR,
                                             CFG_DISABLE_3XPORT_UFP,
                                             extCap3xPortMdRegVal);
        extCap3xPortMdRegVal = CPS_FLD_CLEAR(USBSSP__XEC_CFG_3XPORT_MODE_ADDR,
                                             CFG_DISABLE_3XPORT_DFP,
                                             extCap3xPortMdRegVal);
        if (res->usbModeFlag == 3U)
        {
            extCap3xPortMdRegVal = CPS_FLD_CLEAR(
                USBSSP__XEC_CFG_3XPORT_MODE_ADDR, CFG_SSP_SUPPORT,
                extCap3xPortMdRegVal);
        }
        else
        {
            /* SSP_SUPPORT enabled */
            extCap3xPortMdRegVal = CPS_FLD_SET(USBSSP__XEC_CFG_3XPORT_MODE_ADDR,
                                               CFG_SSP_SUPPORT,
                                               extCap3xPortMdRegVal);
        }
        xhciWrite32(extCap3xPortMdReg, extCap3xPortMdRegVal);
        /* Set USB 3 PORT TO RX-detect */
        devPortscSetLinkState(
            &res->regs.xhciPortControl[USBSSP_DEV_MODE_3_PORT].portsc,
            USBSSP_PORTSCUSB_PLS__RXDETECT << USBSSP__PORTSC1USB3__PLS_SHIFT);
    }
}
/* parasoft-end-suppress MISRA2012-RULE-8_13_a */

/**
 * Initialization of SSPDriverResourcesT object: Set up port control registers.
 *
 * @param[in] res driver resources
 * @return CDN_EINVAL when unsuccessful
 * @return CDN_EOK if no errors
 */
static uint32_t initPortControl (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    // In device mode set port PLS=RxDetect, LWS=1
    // For USB3.x active port is 2
    // cadence specific code
    if (res->deviceModeFlag == 1U)
    {
        uint32_t *extCap3xPortRegs = getExtCapRegPtr(
            res, USBSSP_D_XEC_CFG_3XPORT_CAP_ID);

        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> SSP works in DEVICE mode\n",
                res->instanceNo);

        if (extCap3xPortRegs != NULL)
        {
            setPortControlRegisters(res, extCap3xPortRegs);
        }

        devPortscTestClear(
            &res->regs.xhciPortControl[USBSSP_DEV_MODE_2_PORT].portsc,
            USBSSP_DEV_PORTSC_CHANGE_BITS);
        xhciWrite32(&res->regs.xhciPortControl[USBSSP_DEV_MODE_2_PORT].portsc,
                    USBSSP__PORTSC1USB3__PED_MASK);
        ret = waitForReg(
            &res->regs.xhciPortControl[USBSSP_DEV_MODE_2_PORT].portsc,
            USBSSP__PORTSC1USB3__PED_MASK, 0, USBSSP_DEFAULT_TIMEOUT);

        // Set USB 2 PORT TO RX-detect
        devPortscSetLinkState(
            &res->regs.xhciPortControl[USBSSP_DEV_MODE_2_PORT].portsc,
            USBSSP_PORTSCUSB_PLS__RXDETECT << USBSSP__PORTSC1USB3__PLS_SHIFT);
    }
    else
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_HIVERB, "<%d> SSP works in HOST mode\n",
                res->instanceNo);
    }

    return ret;
}

/**
 * Initialization of SSPDriverResourcesT object: Enable xHC.
 *
 * @param[in] res driver resources
 * @return CDN_EINVAL when unsuccessful
 * @return CDN_EOK if no errors
 */
static void initEnableXhc (USBSSP_DriverResourcesT *res)
{
    // Write the USBCMD (5.4.1) to turn the host controller ON via setting the
    // Run/Stop (R/S) bit to 1. Enable system bus interrupt generation by
    // writing a '1' to the Interrupter Enable (INTE)

    uint32_t flags = USBSSP__USBCMD__HSEE_MASK | USBSSP__USBCMD__INTE_MASK |
                     USBSSP__USBCMD__R_S_MASK;
    uint32_t etc = 0U;

    etc = CPS_FLD_READ(USBSSP__HCCPARAMS2, ETC, res->qaRegs.xHCCaps.hccparams2);

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> hccparams2:(0x%08X) ete(0x%X)\n",
            res->instanceNo, res->qaRegs.xHCCaps.hccparams2, etc);

    // Check for device mode
    if (res->deviceModeFlag == 1U)
    {
        flags |= USBSSP__D_USBCMD__DEVEN_MASK;
    }

    // configure ETE
    if ((res->extendedTBCMode == 1U) && (etc != 0U))
    {
        flags |= USBSSP__D_USBCMD__ETE_MASK;
    }

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> Starting xHC with flags: (0x%08X)\n",
            res->instanceNo, flags);
    xhciWrite32(&res->regs.xhciOperational->usbcmd, flags);
}

/**
 * Initialization of DDUSB register object.
 *
 * @param[in] res driver resources
 * @param[in] unmodified base physical address where SSP controller is mapped
 */
static void initDDUSBObj (USBSSP_DriverResourcesT *res, uintptr_t base)
{
    uint8_t bufIdx = 0;
    USBSSP_SfrT *regs = &res->regs;
    uintptr_t eventRingBaseOffset = base + USBSSP_DDUSB_ERBASE_OFFSET;

    // set ddusb config base
    /* parasoft-begin-suppress MISRA2012-RULE-11_4 "unsigned long converted to
     * USBSSP_DDUSBConfigT*, DRV-5634" */
    regs->ddusbConfig = (USBSSP_DDUSBConfigT *)(base +
                                                USBSSP_DDUSB_CONFIG_OFFSET);
    /* parasoft-end-suppress MISRA2012-RULE-11_4-4 */
    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "<%d> DDUSB Config set to: (0x%08X)\n",
            res->instanceNo, regs->ddusbConfig);

    for (bufIdx = 0; bufIdx < USBSSP_DDUSB_ERBUF_COUNT; bufIdx++)
    {
        regs->ddusbEventRegs.eventRingAddr[bufIdx] = eventRingBaseOffset +
                                                     ((uintptr_t)bufIdx *
                                                      0x200ULL);
    }
}

/**
 * Check if base address is correct. There is magic number in the fixed place.
 *
 * @param[in] baseAddress physical address of SSP controller registers
 * @return CDN_EINVAL when unsuccessful
 * @return CDN_EOK if no errors
 */
/* parasoft-begin-suppress MISRA2012-RULE-2_7-4 "Parameter baseAddress not used
 * in function checkMagicNbr" */
static uint32_t checkMagicNbr (const uintptr_t baseAddress)
{
    uint32_t ret = CDN_EOK;
#ifdef DUT_USBSSP_DRD
    // parasoft-begin-suppress MISRA2012-RULE-11_4-4 "uintptr_t converted to
    // USBSSP_Regs*, DRV-5634"
    USBSSP_Regs *sspRegs = (USBSSP_Regs *)(baseAddress + USBSSP_OTG_OFFSET);
    // parasoft-end-suppress MISRA2012-RULE-11_4-4

    uint32_t magicNbr;
    magicNbr = xhciRead32(&sspRegs->CDNS_DID);
    magicNbr = CPS_FLD_READ(USBSSP__CDNS_DID, DID, magicNbr);
    if (magicNbr != USBSSP_MAGIC_NUMBER)
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Incorrect magic number! (read:0x%X, should be:0x%X)\n",
                magicNbr, USBSSP_MAGIC_NUMBER);
        ret = CDN_EINVAL;
    }
#endif
    return ret;
}
/* parasoft-end-suppress MISRA2012-RULE-2_7-4 */

/**
 * Adjust base address to host or device.
 *
 * @param[in] res driver resources
 * @param[in] base physical address of SSP controller where it is mapped in
 * system return adjusted base address
 */
static uintptr_t adjustBaseAddress (const USBSSP_DriverResourcesT *const res,
                                    uintptr_t base)
{
    uintptr_t newBase;
    uint32_t pbar0;
    /* parasoft-begin-suppress MISRA2012-RULE-11_4-4 "unsigned long converted to
     * USBSSP_Regs*, DRV-5634" */
    USBSSP_Regs *sspRegs = (USBSSP_Regs *)(base + USBSSP_OTG_OFFSET);
    /* parasoft-end-suppress MISRA2012-RULE-11_4-4 */

    if (res->deviceModeFlag == 0U)
    {
        // USB SSP host
        pbar0 = xhciRead32(&sspRegs->PBAR0);
        newBase = base + pbar0;
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT, "PBAR0=0x%X; host offset=0x%X\n",
                pbar0, base);
    }
    else
    {
        // USB SSP device
        newBase = base + USBSSP_DEVICE_OFFSET;
    }
    return newBase;
}

/**
 * Function sets memory resources used by driver
 * @param res driver resources
 * @param memRes user's memory resources
 * @return EOK on success, error code elsewhere
 */
uint32_t USBSSP_SetMemRes (USBSSP_DriverResourcesT *res,
                           USBSSP_XhciResourcesT *memRes)
{
    uint32_t ret = CDN_EOK;

    // check input parameters
    if ((res == NULL) || (memRes == NULL))
    {
        ret = CDN_EINVAL;
    }

    if (ret == CDN_EOK)
    {
        // set internal memory resources field
        res->xhciMemRes = memRes;
    }

    return ret;
}

/**
 * set base address
 * @param[in] res driver resources
 * @param[in] config
 * @param[in] base physical address of SSP controller where it is mapped in
 * system return modified base address
 */
static uintptr_t setBaseAddress (USBSSP_DriverResourcesT *res,
                                 const USBSSP_DriverConfigT *config,
                                 uintptr_t base)
{
    uintptr_t baseAddress;

    // initialize DDUSB obj before adjusting the base address
    initDDUSBObj(res, config->otgRegs);

    // base address adjustment
    if ((config->deviceModeFlag != 0U) && (config->deviceRegs != 0U))
    {
        baseAddress = config->deviceRegs;
    }
    else if ((config->deviceModeFlag == 0U) && (config->hostRegs != 0U))
    {
        baseAddress = config->hostRegs;
    }
    else
    {
        baseAddress = adjustBaseAddress(res, base);
    }

    return baseAddress;
}

/**
 * initialise driver
 * @param[in] res driver resources
 * @param[in] config
 * @param[in] base physical address of SSP controller where it is mapped in
 * system
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
static uint32_t initDriver (USBSSP_DriverResourcesT *res,
                            const USBSSP_DriverConfigT *config, uintptr_t base)
{
    uint32_t ret = CDN_EOK;
    uintptr_t baseAddress = base;

    if (res->noControllerSetup == 0U)
    {
        baseAddress = setBaseAddress(res, config, base);
    }
    // build register object
    initSfrObj(res, baseAddress);

    if (res->noControllerSetup == 0U)
    {
        // Set up xHC and slot0 (scratchpad)
        ret = initXhcSetup(res);
    }

    if (ret == CDN_EOK)
    {
        // Set up rings (control, transfer, event) and interrupts
        ret = initRingsInterrupts(res);
    }

    if ((ret == CDN_EOK) && (res->noControllerSetup == 0U))
    {
        // Set up Port Control registers
        ret = initPortControl(res);
    }

    if (ret == CDN_EOK)
    {
        // Enable interrupts and place Controller in Run state.
        initEnableXhc(res);
        //  Wait for HCHalted bit to be set in USBSTS to become 0
        ret = waitForReg(&res->regs.xhciOperational->usbsts,
                         USBSSP__USBSTS__HCH_MASK, 0, USBSSP_DEFAULT_TIMEOUT);
    }

    return ret;
}

/**
 * Initialization of SSPDriverResourcesT object. SSPDriverResourcesT object
 * keeps all resources required by SSP controller. It represents SSP hardware
 * controller.
 *
 * @param[in] res driver resources
 * @param[in] base physical address of SSP controller where it is mapped in
 * system
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_Init (USBSSP_DriverResourcesT *res,
                      const USBSSP_DriverConfigT *config)
{
    uint32_t ret = USBSSP_InitSF(res, config);
    uintptr_t baseAddress = 0;

    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        baseAddress = config->otgRegs;
        if (res->noControllerSetup == 0U)
        {
            ret = checkMagicNbr(baseAddress);
        }
        //ret |= checkAddrWidth();
        //ret |= checkEndianness();
    }

    if (ret == CDN_EOK)
    {
        res->xhciMemRes = config->xhciMemRes;

        // check if controller memory reserved
        if (res->xhciMemRes == NULL)
        {
            ret = CDN_ENOMEM;
        }

        if (ret == CDN_EOK)
        {
            // Allocate buffers
            ret = initAllocateBuffers(res);
        }

        if (ret == CDN_EOK)
        {
            // initialise the driver
            ret = initDriver(res, config, baseAddress);
        }
    }

    return ret;
}

/**
 * Stops all end points which are not stopped/disabled.
 * @param res driver resources
 * @return flag indicating which all endpoints are being stopped.
 */
static uint32_t stopActiveEndpoints (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = CDN_EOK;
    uint32_t activeEpMask = 0U;
    uint8_t epIndex = 0;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
            "Stopping endpoints for driver instance: <%d>\n", res->instanceNo);
    for (epIndex = 31U; epIndex > 0U; epIndex--)
    {
        // get endpoint status
        USBSSP_EpContexEpState epState = getEndpointStatus(res, epIndex);
        if (epState == USBSSP_EP_CONTEXT_EP_STATE_RUNNING)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                    "Stopping endpoints <%d> from EP_STATE_RUNNING\n", epIndex);
            enqueueStopEndpointCmd(res, epIndex);  // issue stop
            activeEpMask |= ((uint32_t)1U << epIndex);
        }
        else if (epState == USBSSP_EP_CONTEXT_EP_STATE_ERROR)
        {
            // TODO: issue Set TR Dequeue Ptr command. How ?
            vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                    "Error: Epindex (%d) Unexpected EP_STATE_ERROR\n", epIndex);
        }
        else if (epState == USBSSP_EP_CONTEXT_EP_STATE_HALTED)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                    "Reset endpoint <%d> from EP_STATE_HALTED\n", epIndex);
            enqueueResetEndpointCmd(res, epIndex);
            activeEpMask |= ((uint32_t)1U << epIndex);
        }
        else
        {
            // endpoint either stopped or disabled
        }
    }

    if (activeEpMask != 0U)
    {
        hostCmdDoorbell(res);
        // wait for all endpoints to be stopped.
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Waiting for all endpoints to stop\n",
                0);
        ret = waitUntilEpStoppedDisabled(res, USBSSP_DEFAULT_TIMEOUT);
        if (ret != CDN_EOK)
        {
            vDbgMsg(
                USBSSP_DBG_DRV, DBG_CRIT,
                "Critical error: Timed-out waiting for endpoints to stop \n",
                0);
        }
    }
    return ret;
}

/**
 * Blocking call to stop the command ring
 * Stop command ring
 * @param res driver resources
 */
static uint32_t stopCommandRing (USBSSP_OperationalT *xhciOperationalRegs)
{
    uint32_t ret = CDN_EOK;
    uint64_t crcr = 0;

    vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Stopping command ring\n", 0);
    crcr = xhciRead64(&xhciOperationalRegs->crcr);

    // check if command ring is running
    if ((crcr & ((uint64_t)USBSSP__CRCR_LO__CRR_MASK)) > 0U)
    {
        crcr |= (uint64_t)USBSSP__CRCR_LO__CS_MASK;
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "CRCR:%016X\n", crcr);

        // stop command ring
        xhciWrite64((uint64_t *)&xhciOperationalRegs->crcr, crcr);

        // wait for the command ring to be stopped
        ret = waitForReg64(&xhciOperationalRegs->crcr,
                           (uint64_t)USBSSP__CRCR_LO__CRR_MASK, 0U,
                           USBSSP_DEFAULT_TIMEOUT);
    }

    return ret;
}

/**
 * Save the operational and interrupter registers
 * @param res: driver resources
 * @param drvContext: Saved driver context
 */
static void saveStateRegisters (const USBSSP_DriverResourcesT *res,
                                USBSSP_DriverContextT *drvContext)
{
    USBSSP_OperationalT *xhciOperationalRegs = res->regs.xhciOperational;
    USBSSP_InterrupterT *xhciInterrupter0Regs = &res->regs.xhciInterrupter[0];

    // save operational registers
    drvContext->usbcmd = xhciRead32(&xhciOperationalRegs->usbcmd);
    drvContext->dnctrl = xhciRead32(&xhciOperationalRegs->dnctrl);
    drvContext->dcbaap = xhciRead64(&xhciOperationalRegs->dcbaap);
    drvContext->config = xhciRead32(&xhciOperationalRegs->config);

    // save interrupter registers
    drvContext->erstsz = xhciRead32(&xhciInterrupter0Regs->erstsz);
    drvContext->erstba = xhciRead64(&xhciInterrupter0Regs->erstba);
    drvContext->erdp = xhciRead64(&xhciInterrupter0Regs->erdp);
    drvContext->iman = xhciRead32(&xhciInterrupter0Regs->iman);
    drvContext->imod = xhciRead32(&xhciInterrupter0Regs->imod);
}

/**
 * Non-Blocking functions which triggers xHC to save state
 * @param xhciOperationalRegs
 */
static void saveXHCState (const USBSSP_DriverResourcesT *res,
                          USBSSP_DriverContextT *drvContext)
{
    uint32_t regValue = 0U;
    USBSSP_OperationalT *xhciOperationalRegs = res->regs.xhciOperational;

    // save operational and interrupter registers
    saveStateRegisters(res, drvContext);

    regValue = xhciRead32(&xhciOperationalRegs->usbcmd);
    regValue |= (uint32_t)USBSSP__USBCMD__CSS_MASK;
    xhciWrite32(&xhciOperationalRegs->usbcmd, regValue);

    // Wait for SSS (Save State Status) in the USBSTS (USB Status) reg to
    // transition to 0 we might not be able to catch the transition. Hence will
    // poll the flag after a delay.
    CPS_DelayUs(1);
}

/**
 * Function used for saving state when going to suspend mode
 * @param res driver resources
 * @param drvContext driver context
 * @return CDN_EOK on success, error code elsewhere
 */
uint32_t USBSSP_SaveState (USBSSP_DriverResourcesT *res,
                           USBSSP_DriverContextT *drvContext)
{
    uint32_t ret = CDN_EOK;

    ret = USBSSP_SaveStateSF(res, drvContext);
    if (CDN_EOK != ret)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        USBSSP_OperationalT *xhciOperationalRegs = res->regs.xhciOperational;
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "Saving state for driver instance: <%d>\n", res->instanceNo);

        if (ret == CDN_EOK)
        {
            // Step 1: stop all endpoints epIndex = 1 to 31
            ret = stopActiveEndpoints(res);
        }

        // Step-2: Ensure that the Command ring is stopped
        if (ret == CDN_EOK)
        {
            ret = stopCommandRing(xhciOperationalRegs);
        }

        // Step-3: Stop Controller. Set Run/Stop = 0 in USBCMD reg
        //         Wait for HCHalted bit to be set in USBSTS (USB Status
        //         register)
        if (ret == CDN_EOK)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Stopping Controller\n", 0);
            ret = xhcHalt(res);
        }

        if (ret == CDN_EOK)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "xHC Halted (0x%X) \n",
                    xhciRead32(&xhciOperationalRegs->usbsts));

            // Step-4: Read the operational and run-time registers and save it
            // in the state struct Step-5: Set the CCS (Controller Save State)
            // flag in USBCMD reg
            saveXHCState(res, drvContext);

            ret = waitForReg(&xhciOperationalRegs->usbsts,
                             USBSSP__USBSTS__SSS_MASK, 0U,
                             USBSSP_DEFAULT_TIMEOUT);
        }

        if (ret == CDN_EOK)
        {
            vDbgMsg(USBSSP_DBG_DRV, DBG_FYI, "Resetting command ring\n", 0);
            // At this point all required registers are saved.
            // the ownership of all memory "res.xhciResources" is now with
            // software
            resetCmdRing(res);  // this clears the content of the command ring
        }
    }
    return ret;
}

/**
 * Functions used for restoring from suspend mode
 * @param res driver resources
 * @param drvContext driver context
 * @return CDN_EOK on success, error code elsewhere
 */
uint32_t USBSSP_RestoreState (USBSSP_DriverResourcesT *res,
                              const USBSSP_DriverContextT *drvContext)
{
    uint32_t ret = CDN_EOK;

    ret = USBSSP_SaveStateSF(res, drvContext);
    if (CDN_EOK != ret)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        USBSSP_OperationalT *xhciOperationalRegs = res->regs.xhciOperational;
        USBSSP_InterrupterT *xhciInterrupter0Regs = &res->regs
                                                         .xhciInterrupter[0];
        vDbgMsg(USBSSP_DBG_DRV, DBG_FYI,
                "Restoring state for driver instance: <%d>\n", res->instanceNo);

        // Step-2: Restore the saved memory image of the DCBAA, Contexts and
        // other data structures
        //          to their original physical locations

        // Step-3: Restore image of scratchpad if saved

        if (ret == CDN_EOK)
        {
            // Step-4: Restore Operational and Runtime registers
            xhciWrite32(&xhciOperationalRegs->dnctrl, drvContext->dnctrl);
            xhciWrite64(&xhciOperationalRegs->dcbaap, drvContext->dcbaap);
            xhciWrite32(&xhciOperationalRegs->config, drvContext->config);
            xhciWrite32(&xhciInterrupter0Regs->erstsz, drvContext->erstsz);
            xhciWrite64(&xhciInterrupter0Regs->erstba, drvContext->erstba);
            xhciWrite64(&xhciInterrupter0Regs->erdp, drvContext->erdp);
            xhciWrite32(&xhciInterrupter0Regs->iman, drvContext->iman);
            xhciWrite32(&xhciInterrupter0Regs->imod, drvContext->imod);

            // Step-5a: Set the controller restore state (CRS) flag in the
            // USBCMD to 1
            xhciWrite32(&xhciOperationalRegs->usbcmd, USBSSP__USBCMD__CRS_MASK);
            CPS_DelayUs(1);
            // Step-5b: Wait for Restore State Status (RSS) in USBSTS to
            // transition to zero.
            ret = waitForReg(&xhciOperationalRegs->usbsts,
                             USBSSP__USBSTS__RSS_MASK, 0U,
                             USBSSP_DEFAULT_TIMEOUT);
            if (ret != CDN_EOK)
            {
                vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                        "Critical error: Timed-out waiting for Restore State "
                        "Status (RSS) to be (%d) \n",
                        0);
            }
        }
        // Step 6 & 7: Re-initialize the command ring
        if (ret == CDN_EOK)
        {
            uint64_t crcr_sfr_val = get64PhyAddrOf32ptr(
                &res->commandQ.ring[0].dword0);
            // Set the Command Ring Dequeue Pointer by programming the Command
            // Ring Control Register
            xhciWrite64(&xhciOperationalRegs->crcr,
                        (uint64_t)(crcr_sfr_val | res->commandQ.toogleBit));
        }

        // Step 8: Start the controller
        initEnableXhc(res);
    }
    return ret;
}

/**
 * Function issues SET_ADDRESS command to controller
 * @param res driver resources
 */
uint32_t USBSSP_SetAddress (USBSSP_DriverResourcesT *res)
{
    uint32_t ret = USBSSP_SetAddressSF(res);
    if (ret != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        setAddress(res, 0U);
    }
    return ret;
}

/**
 * Function returns index of actual micro frame
 * @param res driver resources
 * @param index pointer to memory where actual micro frame index will be stored
 * @return CDN_EOK on success, CDN_EINVAL when any of input parameter is NULL
 */
uint32_t USBSSP_GetMicroFrameIndex (USBSSP_DriverResourcesT *res,
                                    uint32_t *index)
{
    uint32_t ret = CDN_EOK;

    // check correctness of input parameters
    if (USBSSP_GetMicroFrameIndexSF(res, index) != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
        ret = CDN_EINVAL;
    }
    else
    {
        // read frame index from hardware
        *index = xhciRead32(&res->regs.xhciRuntime->mfindex);
        ret = CDN_EOK;
    }

    return ret;
}

/**
 * Function sets frameID value in first TRB of isochronous transfer descriptor
 *
 * @param[in] res driver resources
 * @param[in] epIndex index of selected endpoint
 * @param[in] frameID value of frameID to set
 */
uint32_t USBSSP_SetFrameID (USBSSP_DriverResourcesT *res, uint8_t epIndex,
                            uint32_t frameID)
{
    uint32_t ret = USBSSP_SetFrameIDSF(res);
    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        res->ep[epIndex].frameID = frameID;
    }
    return ret;
}

/**
 * Enables U1 for (t > 0), disables U1 for (t = 0)
 */
static void setU1timeout (const USBSSP_DriverResourcesT *res, uint8_t t)
{
    uint32_t regValue = xhciRead32(
        &res->regs.xhciPortControl[res->actualPort - 1U].portpmsc);
    if (t > 0U)
    {
        regValue |= 0x00000001U;
    }
    else
    {
        regValue &= ~0x00000001U;
    }
    xhciWrite32(&res->regs.xhciPortControl[res->actualPort - 1U].portpmsc,
                regValue);
}

/**
 * Enables U2 for (t > 0), disables U2 for (t = 0)
 */
static void setU2timeout (const USBSSP_DriverResourcesT *res, uint8_t t)
{
    uint32_t regValue = xhciRead32(
        &res->regs.xhciPortControl[res->actualPort - 1U].portpmsc);
    if (t > 0U)
    {
        regValue |= 0x00000100U;
    }
    else
    {
        regValue &= ~0x00000100U;
    }
    xhciWrite32(&res->regs.xhciPortControl[res->actualPort - 1U].portpmsc,
                regValue);
}

/**
 * Function sets port status/control register to given value
 *
 * @param[in] res driver resources
 * @param[in] portId index of selected port
 * @param[in] portRegIdx ...
 * @param[in] regValue memory place where port status/control register value
 *                      will be stored
 * @return CDN_EOK if success, CDN_EINVAL when some input parameter is incorrect
 */
uint32_t USBSSP_SetPortControlReg (const USBSSP_DriverResourcesT *res,
                                   uint8_t portId,
                                   USBSSP_PortControlRegIdx portRegIdx,
                                   uint32_t regValue)
{
    uint32_t ret = 0U;

    ret = USBSSP_SetPortControlRegSF(res, portRegIdx);

    // check input parameters correctness
    if ((CDN_EOK != ret) || (0U == portId))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters "
                "res: 0x%X, portId = %d\n",
                res, portId);
        ret = CDN_EINVAL;
    }
    else
    {
        // set value of port status/control register
        uint32_t *portCtrlPtr = &(
            res->regs.xhciPortControl[portId - 1U].portsc);
        xhciWrite32(&portCtrlPtr[portRegIdx], regValue);
    }
    return ret;
}

/**
 * Function returns port control register value
 *
 * @param[in] res driver resources
 * @param[in] portId index of selected port
 * @param[in] portRegIdx ...
 * @param[out] regValue memory place where port status/control register value
 *                      will be stored
 * @return CDN_EOK if success, CDN_EINVAL when some input parameter is incorrect
 */
uint32_t USBSSP_GetPortControlReg (const USBSSP_DriverResourcesT *res,
                                   uint8_t portId,
                                   USBSSP_PortControlRegIdx portRegIdx,
                                   uint32_t *regValue)
{
    uint32_t ret = CDN_EOK;

    ret = USBSSP_GetPortControlRegSF(res, portRegIdx, regValue);

    // check input parameters correctness
    if ((CDN_EOK != ret) || (0U == portId))
    {
        vDbgMsg(USBSSP_DBG_DRV, DBG_CRIT,
                "Critical error! Wrong value in one of function parameters "
                "res: 0x%X, portId = %d\n",
                res, portId);
        ret = CDN_EINVAL;
    }
    else
    {
        uint32_t *portCtrlPtr = &(
            res->regs.xhciPortControl[portId - 1U].portsc);
        *regValue = xhciRead32(&portCtrlPtr[portRegIdx]);
    }
    return ret;
}

//----------- tester extension functions------------------------------------

/**
 * Function sets selected extra flags associated with selected endpoint.
 *
 * @param[in] res driver resources
 * @param[in] epIndex index of selected endpoint
 * @param[out] flag  bitmap of flags to set, 1 on selected bit clears
 * corresponding flag
 */
uint32_t USBSSP_SetEndpointExtraFlag (USBSSP_DriverResourcesT *res,
                                      uint8_t epIndex,
                                      USBSSP_ExtraFlagsEnumT flags)
{
    // check input parameters correctness
    uint32_t ret = USBSSP_SetEndpointExtraFlagSF(res, flags);
    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        // set selected flags
        res->ep[epIndex].extraFlags |= (uint8_t)flags;
    }
    return ret;
}

/**
 * Function clears selected extra flags associated with selected endpoint.
 *
 * @param[in] res driver resources
 * @param[in] epIndex index of selected endpoint
 * @param[out] flag  bitmap of flags to clear, 0 on selected bit clears
 * corresponding flag
 */
uint32_t USBSSP_CleanEndpointExtraFlag (USBSSP_DriverResourcesT *res,
                                        uint8_t epIndex,
                                        USBSSP_ExtraFlagsEnumT flags)
{
    // check input parameters correctness
    uint32_t ret = USBSSP_CleanEndpointExtraFlaSF(res, flags);
    if (ret != (uint32_t)CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
    }
    else
    {
        // clear selected flags
        res->ep[epIndex].extraFlags &= ~((uint8_t)flags);
    }
    return ret;
}

/**
 * Function returns extra flags associated with selected endpoint.
 *
 * @param[in] res driver resources
 * @param[in] epIndex index of selected endpoint
 * @param[out] flag pointer to memory where flags will be stored
 * @return CDN_EOK if no errors, CDN_EINVAL for incorrect input parameters
 */
uint32_t USBSSP_GetEndpointExtraFlag (const USBSSP_DriverResourcesT *res,
                                      uint8_t epIndex, uint8_t *flag)
{
    uint32_t ret;

    // check if input pointers are not NULL
    if (USBSSP_GetEndpointExtraFlagSF(res, flag) != CDN_EOK)
    {
        vDbgMsg(
            USBSSP_DBG_DRV, DBG_CRIT,
            "<%d> Critical error! Wrong value in one of function parameters\n",
            res->instanceNo);
        ret = CDN_EINVAL;
    }
    else
    {
        // set output parameter with extra flag value
        *flag = res->ep[epIndex].extraFlags;
        ret = CDN_EOK;
    }

    return ret;
}
//--------------------------------------------------------------------------
