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

/* parasoft-begin-suppress METRICS-18-3 "Follow the Cyclomatic Complexity limit
 * of 10" */
/* parasoft-begin-suppress METRICS-36-3 "A function should not be called from
 * more than 5 different functions" */
/* parasoft-begin-suppress METRICS-39-3 "The value of VOCF metric for a function
 * should not be higher than 4" */
/* parasoft-begin-suppress METRICS-41-3 "Number of blocks of comments per
 * statement" */

/**
 * This file contains sanity API functions. The purpose of sanity functions
 * is to check input parameters validity. They take the same parameters as
 * original API functions and return 0 on success or CDN_EINVAL on wrong
 * parameter value(s).
 */

#ifndef CDN_XHCI_SANITY_H
#define CDN_XHCI_SANITY_H

#include "cdn_errno.h"
#include "cdn_stdtypes.h"
#include "cdn_xhci_if.h"
#include "cusb_ch9_sanity.h"

static inline uint32_t USBSSP_DriverConfigTSF(const USBSSP_DriverConfigT* obj);
static inline uint32_t USBSSP_DriverContextTSF(
    const USBSSP_DriverContextT* obj);
static inline uint32_t USBSSP_DriverResourcesTSF(
    const USBSSP_DriverResourcesT* obj);
static inline uint32_t USBSSP_ProducerQueueTSF(
    const USBSSP_ProducerQueueT* obj);
static inline uint32_t USBSSP_RingElementTSF(const USBSSP_RingElementT* obj);
static inline uint32_t USBSSP_XhciResourcesTSF(
    const USBSSP_XhciResourcesT* obj);

static inline uint32_t USBSSP_SanityFunction1(
    const USBSSP_DriverResourcesT* res, const uint8_t epIndex);
static inline uint32_t USBSSP_SanityFunction3(
    const USBSSP_DriverResourcesT* res, const uint8_t endpoint);
static inline uint32_t USBSSP_SanityFunction5(
    const USBSSP_DriverResourcesT* res);
static inline uint32_t USBSSP_SanityFunction7(
    const USBSSP_DriverResourcesT* res, const USBSSP_XhciResourcesT* memRes);
static inline uint32_t USBSSP_SanityFunction8(
    const USBSSP_DriverResourcesT* res, const USBSSP_DriverConfigT* config);
static inline uint32_t USBSSP_SanityFunction10(
    const USBSSP_DriverResourcesT* res, const USBSSP_RingElementT* eventPtr);
static inline uint32_t USBSSP_SanityFunction15(
    const USBSSP_DriverResourcesT* res, const uint8_t* epCfgBuffer);
static inline uint32_t USBSSP_SanityFunction16(
    const USBSSP_DriverResourcesT* res, const CH9_UsbSetup* setup);
static inline uint32_t USBSSP_SanityFunction26(
    const USBSSP_DriverResourcesT* res, const uint32_t* index);
static inline uint32_t USBSSP_SanityFunction27(
    const USBSSP_DriverResourcesT* res, const USBSSP_ExtraFlagsEnumT flags);
static inline uint32_t USBSSP_SanityFunction33(
    const USBSSP_DriverResourcesT* res,
    const USBSSP_PortControlRegIdx portRegIdx);
static inline uint32_t USBSSP_SanityFunction34(
    const USBSSP_DriverResourcesT* res,
    const USBSSP_PortControlRegIdx portRegIdx, const uint32_t* regValue);
static inline uint32_t USBSSP_SanityFunction35(
    const USBSSP_DriverResourcesT* res, const uint32_t* ddusbTxRegs,
    const uint32_t* ddusbRxRegs);
static inline uint32_t USBSSP_SanityFunction37(
    const USBSSP_DriverResourcesT* res,
    const USBSSP_DriverContextT* drvContext);

#define USBSSP_TransferDataSF          USBSSP_SanityFunction1
#define USBSSP_TransferData2SF         USBSSP_SanityFunction1
#define USBSSP_StopEndpointSF          USBSSP_SanityFunction3
#define USBSSP_ResetEndpointSF         USBSSP_SanityFunction3
#define USBSSP_ResetDeviceSF           USBSSP_SanityFunction5
#define USBSSP_IsrSF                   USBSSP_SanityFunction5
#define USBSSP_SetMemResSF             USBSSP_SanityFunction7
#define USBSSP_InitSF                  USBSSP_SanityFunction8
#define USBSSP_GetDescriptorSF         USBSSP_SanityFunction5
#define USBSSP_ForceEventSF            USBSSP_SanityFunction10
#define USBSSP_SetAddressSF            USBSSP_SanityFunction5
#define USBSSP_ResetRootHubPortSF      USBSSP_SanityFunction5
#define USBSSP_IssueGenericCommandSF   USBSSP_SanityFunction5
#define USBSSP_EndpointSetFeatureSF    USBSSP_SanityFunction1
#define USBSSP_SetConfigurationSF      USBSSP_SanityFunction15
#define USBSSP_ControlTransferSF       USBSSP_SanityFunction16
#define USBSSP_NBControlTransferSF     USBSSP_SanityFunction16
#define USBSSP_ControlTransferDevSF    USBSSP_SanityFunction5
#define USBSSP_NoOpTestSF              USBSSP_SanityFunction5
#define USBSSP_EnableSlotSF            USBSSP_SanityFunction5
#define USBSSP_DisableSlotSF           USBSSP_SanityFunction5
#define USBSSP_EnableEndpointSF        USBSSP_SanityFunction15
#define USBSSP_DisableEndpointSF       USBSSP_SanityFunction5
#define USBSSP_GetEpStateSF            USBSSP_SanityFunction5
#define USBSSP_GetMicroFrameIndexSF    USBSSP_SanityFunction26
#define USBSSP_SetEndpointExtraFlagSF  USBSSP_SanityFunction27
#define USBSSP_CleanEndpointExtraFlaSF USBSSP_SanityFunction27
#define USBSSP_GetEndpointExtraFlagSF  USBSSP_SanityFunction15
#define USBSSP_SetFrameIDSF            USBSSP_SanityFunction5
#define USBSSP_AddEventDataTRBSF       USBSSP_SanityFunction5
#define USBSSP_ForceHeaderSF           USBSSP_SanityFunction5
#define USBSSP_SetPortControlRegSF     USBSSP_SanityFunction33
#define USBSSP_GetPortControlRegSF     USBSSP_SanityFunction34
#define USBSSP_EnableDDUSBSF           USBSSP_SanityFunction35
#define USBSSP_DisableDDUSBSF          USBSSP_SanityFunction5
#define USBSSP_SaveStateSF             USBSSP_SanityFunction37
#define USBSSP_RestoreStateSF          USBSSP_SanityFunction37

/**
 * Function to validate struct RingElementT
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns CDN_EINVAL for invalid
 */
static inline uint32_t USBSSP_RingElementTSF (const USBSSP_RingElementT* obj)
{
    uint32_t ret = 0;

    if (obj == NULL)
    {
        ret = CDN_EINVAL;
    }

    return ret;
}

/**
 * Function to validate struct XhciResourcesT
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns CDN_EINVAL for invalid
 */
static inline uint32_t USBSSP_XhciResourcesTSF (
    const USBSSP_XhciResourcesT* obj)
{
    uint32_t ret = 0;

    if (obj == NULL)
    {
        ret = CDN_EINVAL;
    }

    return ret;
}

/**
 * Function to validate struct DriverResourcesT
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns CDN_EINVAL for invalid
 */
static inline uint32_t USBSSP_DriverResourcesTSF (
    const USBSSP_DriverResourcesT* obj)
{
    uint32_t ret = 0;

    if (obj == NULL)
    {
        ret = CDN_EINVAL;
    }
    else
    {
        if (USBSSP_ProducerQueueTSF(&obj->commandQ) == CDN_EINVAL)
        {
            ret = CDN_EINVAL;
        }
        if (USBSSP_ProducerQueueTSF(&obj->ep0) == CDN_EINVAL)
        {
            ret = CDN_EINVAL;
        }
        uint32_t idx_ep;

        for (idx_ep = 0;
             idx_ep < (USBSSP_MAX_EP_CONTEXT_NUM + USBSSP_EP_CONT_OFFSET);
             idx_ep++)
        {
            if (USBSSP_ProducerQueueTSF(&obj->ep[idx_ep]) == CDN_EINVAL)
            {
                ret = CDN_EINVAL;
            }
        }
        if ((obj->actualSpeed != CH9_USB_SPEED_UNKNOWN) &&
            (obj->actualSpeed != CH9_USB_SPEED_LOW) &&
            (obj->actualSpeed != CH9_USB_SPEED_FULL) &&
            (obj->actualSpeed != CH9_USB_SPEED_HIGH) &&
            (obj->actualSpeed != CH9_USB_SPEED_SUPER) &&
            (obj->actualSpeed != CH9_USB_SPEED_SUPER_PLUS))
        {
            ret = CDN_EINVAL;
        }
        if ((obj->ep0State != USBSSP_EP0_UNCONNECTED) &&
            (obj->ep0State != USBSSP_EP0_HALT_PENDING) &&
            (obj->ep0State != USBSSP_EP0_HALT_SETUP_PENDING) &&
            (obj->ep0State != USBSSP_EP0_HALTED) &&
            (obj->ep0State != USBSSP_EP0_SETUP_PENDING) &&
            (obj->ep0State != USBSSP_EP0_SETUP_PHASE) &&
            (obj->ep0State != USBSSP_EP0_DATA_PHASE) &&
            (obj->ep0State != USBSSP_EP0_STATUS_PHASE))
        {
            ret = CDN_EINVAL;
        }
    }

    return ret;
}

/**
 * Function to validate struct DriverConfigT
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns CDN_EINVAL for invalid
 */
static inline uint32_t USBSSP_DriverConfigTSF (const USBSSP_DriverConfigT* obj)
{
    uint32_t ret = 0;

    if (obj == NULL)
    {
        ret = CDN_EINVAL;
    }

    return ret;
}

/**
 * Function to validate struct DriverContextT
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns CDN_EINVAL for invalid
 */
static inline uint32_t USBSSP_DriverContextTSF (
    const USBSSP_DriverContextT* obj)
{
    uint32_t ret = 0;

    if (obj == NULL)
    {
        ret = CDN_EINVAL;
    }

    return ret;
}

/**
 * Function to validate struct ProducerQueueT
 *
 * @param[in] obj pointer to struct to be verified
 * @returns 0 for valid
 * @returns CDN_EINVAL for invalid
 */
static inline uint32_t USBSSP_ProducerQueueTSF (
    const USBSSP_ProducerQueueT* obj)
{
    uint32_t ret = 0;

    if (obj == NULL)
    {
        ret = CDN_EINVAL;
    }
    else
    {
        if ((obj->req_pending != USBSSP_REQUEST_COMPLETE) &&
            (obj->req_pending != USBSSP_REQUEST_PENDING) &&
            (obj->req_pending != USBSSP_REQUEST_HALTED))
        {
            ret = CDN_EINVAL;
        }
    }

    return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] epIndex index of endpoint according to xhci spec e.g for ep1out
 *    epIndex=2, for ep1in epIndex=3, for ep2out epIndex=4 end so on
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction1 (
    const USBSSP_DriverResourcesT* res, const uint8_t epIndex)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if ((epIndex < (1U)) || (epIndex > (USBSSP_EP_CONT_MAX)))
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] endpoint Index of endpoint to stop
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction3 (
    const USBSSP_DriverResourcesT* res, const uint8_t endpoint)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if ((endpoint < (USBSSP_EP0_CONT_OFFSET)) ||
             (endpoint > (USBSSP_EP_CONT_MAX)))
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction5 (
    const USBSSP_DriverResourcesT* res)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }

    return ret;
}

/**
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] memRes User defined memory resources.
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction7 (
    const USBSSP_DriverResourcesT* res, const USBSSP_XhciResourcesT* memRes)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_XhciResourcesTSF(memRes) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] config Driver configuration for initialization
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction8 (
    const USBSSP_DriverResourcesT* res, const USBSSP_DriverConfigT* config)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_DriverConfigTSF(config) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] eventPtr eventPtr pointer to event that will be sent
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction10 (
    const USBSSP_DriverResourcesT* res, const USBSSP_RingElementT* eventPtr)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_RingElementTSF(eventPtr) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] epCfgBuffer Configuration buffer address
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction15 (
    const USBSSP_DriverResourcesT* res, const uint8_t* epCfgBuffer)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (epCfgBuffer == NULL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] setup Keeps setup packet
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction16 (
    const USBSSP_DriverResourcesT* res, const CH9_UsbSetup* setup)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if (CH9_UsbSetupSF(setup) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res driver resources
 * @param[out] index Micro Frame Index returned by function.
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction26 (
    const USBSSP_DriverResourcesT* res, const uint32_t* index)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (index == NULL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res
 * @param[in] flags
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction27 (
    const USBSSP_DriverResourcesT* res, const USBSSP_ExtraFlagsEnumT flags)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if ((flags != USBSSP_EXTRAFLAGSENUMT_UNDEFINED) &&
             (flags != USBSSP_EXTRAFLAGSENUMT_NODORBELL) &&
             (flags != USBSSP_EXTRAFLAGSENUMT_FORCELINKTRB))
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res
 * @param[in] portRegIdx
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction33 (
    const USBSSP_DriverResourcesT* res,
    const USBSSP_PortControlRegIdx portRegIdx)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if ((portRegIdx != USBSSP_PORTSC_REG_IDX) &&
             (portRegIdx != USBSSP_PORTPMSC_REG_IDX) &&
             (portRegIdx != USBSSP_PORTLI_REG_IDX) &&
             (portRegIdx != USBSSP_PORTHLPMC_REG_IDX))
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res
 * @param[in] portRegIdx
 * @param[out] regValue
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction34 (
    const USBSSP_DriverResourcesT* res,
    const USBSSP_PortControlRegIdx portRegIdx, const uint32_t* regValue)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (regValue == NULL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if ((portRegIdx != USBSSP_PORTSC_REG_IDX) &&
             (portRegIdx != USBSSP_PORTPMSC_REG_IDX) &&
             (portRegIdx != USBSSP_PORTLI_REG_IDX) &&
             (portRegIdx != USBSSP_PORTHLPMC_REG_IDX))
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] ddusbTxRegs Pointer to array of 4 ddusb_tx_valid registers
 * @param[in] ddusbRxRegs Pointer to array of 4 ddusb_rx_valid registers
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction35 (
    const USBSSP_DriverResourcesT* res, const uint32_t* ddusbTxRegs,
    const uint32_t* ddusbRxRegs)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (ddusbTxRegs == NULL)
    {
        ret = CDN_EINVAL;
    }
    else if (ddusbRxRegs == NULL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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
 * A common function to check the validity of API functions with
 * following parameter types
 * @param[in] res Driver resources
 * @param[in] drvContext Pointer to driver context struct
 * @return 0 success
 * @return CDN_EINVAL invalid parameters
 */
static inline uint32_t USBSSP_SanityFunction37 (
    const USBSSP_DriverResourcesT* res, const USBSSP_DriverContextT* drvContext)
{
    /* Declaring return variable */
    uint32_t ret = 0;

    if (USBSSP_DriverResourcesTSF(res) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
    }
    else if (USBSSP_DriverContextTSF(drvContext) == CDN_EINVAL)
    {
        ret = CDN_EINVAL;
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

/* parasoft-end-suppress METRICS-41-3 */
/* parasoft-end-suppress METRICS-39-3 */
/* parasoft-end-suppress METRICS-36-3 */
/* parasoft-end-suppress METRICS-18-3 */

#endif /* CDN_XHCI_SANITY_H */
