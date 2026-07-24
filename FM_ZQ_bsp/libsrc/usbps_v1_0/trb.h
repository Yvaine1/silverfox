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
 * trb.h
 * Host/Device Super Speed Plus controller driver,
 *
 * Functions, macros, definitions for TRB manipulating - header file
 *****************************************************************************/

#ifndef CDN_TRB_H
#define CDN_TRB_H

#include "cdn_xhci_if.h"
#include "cdn_xhci_structs_if.h"
#include "byteorder.h"

/*
 * Get the trb type
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline uint32_t getTrbType (const USBSSP_RingElementT *trb)
{
    return ((le32ToCpu(trb->dword3) >> USBSSP_TRB_TYPE_POS) & 0x3FU);
}
/* parasoft-end-suppress METRICS-36-3 */

/*
 * Get the toggle bit
 */
static inline uint8_t getToogleBit (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword3) >> 0) & 1U));
}

/*
 * Get the ED bit
 */
static inline uint8_t getEDbit (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword3) >> 2) & 1U));
}

/*
 * Get the endpoint
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline uint8_t getEndpoint (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword3) >> USBSSP_ENDPOINT_POS) & 0x1FU));
}
/* parasoft-end-suppress METRICS-36-3 */

/*
 * Get the slot ID
 */
static inline uint8_t getSlotId (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword3) >> USBSSP_SLOT_ID_POS) & 0xFFU));
}

/*
 * Get the completion code position
 */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions, DRV-3823" */
static inline uint32_t getCompletionCode (const USBSSP_RingElementT *trb)
{
    return (((le32ToCpu(trb->dword2) >> USBSSP_COMPLETION_CODE_POS) & 0xFFU));
}
/* parasoft-end-suppress METRICS-36-3 */

/*
 * Get the trb EVENT transfer length
 */
static inline uint32_t getTrEvtTrbTransLen (const USBSSP_RingElementT *trb)
{
    return (le32ToCpu(trb->dword2) & USBSSP_TRB_EVT_RESIDL_LEN_MSK);
}

/*
 * Get the trb transfer length
 */
static inline uint32_t getTrTrbTransLen (const USBSSP_RingElementT *trb)
{
    return (le32ToCpu(trb->dword2) & USBSSP_TRB_TRANSFER_LENGTH_MASK);
}

/*
 * Get the chain
 */
static inline uint8_t getTrTrbChain (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword3) >> 4) & 0x01U));
}

/*
 * Get the event trb low pointer
 */
static inline uint32_t getTrEvtTrbPtrLo (const USBSSP_RingElementT *trb)
{
    return (le32ToCpu(trb->dword0));
}

/*
 * Get the event trb high pointer
 */
static inline uint32_t getTrEvtTrbPtrHi (const USBSSP_RingElementT *trb)
{
    return (le32ToCpu(trb->dword1));
}

/*
 * Get the port ID
 */
static inline uint8_t getPortId (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword0) >> 24) & 0xFFU));
}

/*
 * Get the request type
 */
static inline uint8_t getSetupBmRequestType (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword0) >> 0) & 0xFFU));
}

/*
 * Get the setup request
 */
static inline uint8_t getSetupBrequest (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword0) >> 8) & 0xFFU));
}

/*
 * Get the value
 */
static inline uint16_t getwValue (const USBSSP_RingElementT *trb)
{
    return ((uint16_t)((le32ToCpu(trb->dword0) >> 16) & 0xFFFFU));
}

/*
 * Get the length
 */
static inline uint16_t getwLength (const USBSSP_RingElementT *trb)
{
    return ((uint16_t)((le32ToCpu(trb->dword1) >> 16) & 0xFFFFU));
}

/*
 * Get the Index
 */
static inline uint16_t getwIndex (const USBSSP_RingElementT *trb)
{
    return ((uint16_t)((le32ToCpu(trb->dword1) >> 0) & 0xFFFFU));
}

/*
 * Get SID
 */
static inline uint16_t getSID (const USBSSP_RingElementT *trb)
{
    return ((uint16_t)((le32ToCpu(trb->dword2) >> 0) & 0xFFFFU));
}

/*
 * Get setup ID
 */
static inline uint8_t getSetupId (const USBSSP_RingElementT *trb)
{
    return ((uint8_t)((le32ToCpu(trb->dword3) & USBSSP_TRB_SETUPID_MASK) >>
                      USBSSP_TRB_SETUPID_POS));
}

#endif
