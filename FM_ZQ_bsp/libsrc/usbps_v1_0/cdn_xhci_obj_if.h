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
#ifndef CDN_XHCI_OBJ_IF_H
#define CDN_XHCI_OBJ_IF_H

#include "cdn_xhci_if.h"

/** @defgroup DriverObject Driver API Object
 *  API listing for the driver. The API is contained in the object as
 *  function pointers in the object structure. As the actual functions
 *  resides in the Driver Object, the client software must first use the
 *  global GetInstance function to obtain the Driver Object Pointer.
 *  The actual APIs then can be invoked using obj->(api_name)() syntax.
 *  These functions are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
 * API methods
 **********************************************************************/
typedef struct USBSSP_OBJ_s {
    /**
     * Transfer data on given endpoint. This function is non-blocking
     * type. The XHCI operation result should be checked in complete
     * callback function.
     * @param[in] res Driver resources
     * @param[in] epIndex index of endpoint according to xhci spec e.g for
     * ep1out epIndex=2, for ep1in epIndex=3, for ep2out epIndex=4 end so on
     * $RANGE $FROM 1U $TO USBSSP_EP_CONT_MAX $
     * @param[in] buff Buffer for data to send or to receive
     * @param[in] size Size of data in bytes
     * @param[in] complete pointer to function which will be returned in
     * callback in input parameter, can be set to NULL when no extra parameter
     * used
     * @return CDN_EINVAL if selected endpoint index is out of available range
     * @return CDN_EOK if selected endpoint is within available endpoint range
     */
    uint32_t (*transferData)(USBSSP_DriverResourcesT* res, uint8_t epIndex,
                             const uintptr_t buff, uint32_t size,
                             USBSSP_Complete complete);

    /**
     * @param[in] res Driver resources
     * @param[in] epIndex index of endpoint according to xhci spec e.g for
     * ep1out epIndex=2, for ep1in epIndex=3, for ep2out epIndex=4 end so on
     * $RANGE $FROM 1U $TO USBSSP_EP_CONT_MAX $
     * @param[in] paramT Structure having Pointer to buffer for data,size of
     * data
     * @param[in] complete pointer to function which will be returned in
     * callback in input parameter, can be set to NULL when no extra parameter
     * used
     * @return CDN_EINVAL if selected endpoint index is out of available range
     * @return CDN_EOK if selected endpoint is within available endpoint range
     */
    uint32_t (*transferData2)(USBSSP_DriverResourcesT* res, uint8_t epIndex,
                              USBSSP_param paramT, USBSSP_Complete complete);

    /**
     * Stop endpoint. Function sends STOP_ENDPOINT_COMMAND command to
     * USBSSP controller
     * @param[in] res Driver resources
     * @param[in] endpoint Index of endpoint to stop $RANGE $FROM
     * USBSSP_EP0_CONT_OFFSET $TO USBSSP_EP_CONT_MAX $
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*stopEndpoint)(USBSSP_DriverResourcesT* res, uint8_t endpoint);

    /**
     * Endpoint reset. Function sends RESET_ENDPOINT_COMMAND to USBSSP
     * controller
     * @param[in] res Driver resources
     * @param[in] endpoint Index of endpoint to reset $RANGE $FROM
     * USBSSP_EP0_CONT_OFFSET $TO USBSSP_EP_CONT_MAX $
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*resetEndpoint)(USBSSP_DriverResourcesT* res, uint8_t endpoint);

    /**
     * Reset of connected device. Function sends RESET_DEVICE_COMMAND to
     * USBSSP controller in order to issue reset state on USB bus.
     * @param[in] res Driver resources
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK selected endpoint is within available endpoint range
     */
    uint32_t (*resetDevice)(USBSSP_DriverResourcesT* res);

    /**
     * Handling of USBSSP controller interrupt. Function is called from
     * USBSSP interrupt context.
     * @param[in] res Driver resources
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*isr)(USBSSP_DriverResourcesT* res);

    /**
     * Function sets memory resources used by driver.
     * @param[in] res Driver resources
     * @param[in] memRes User defined memory resources.
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*SetMemRes)(USBSSP_DriverResourcesT* res,
                          USBSSP_XhciResourcesT* memRes);

    /**
     * Initialization of USBSSP_DriverResourcesT object.
     * USBSSP_DriverResourcesT object keeps all resources required by
     * USBSSP controller. It represents USBSSP hardware controller.
     * @param[in] res Driver resources
     * @param[in] config Driver configuration for initialization
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*init)(USBSSP_DriverResourcesT* res,
                     const USBSSP_DriverConfigT* config);

    /**
     * Get descriptor. Function gets descriptor from connected device,
     * used in host mode and stores it in internal res->ep0Buff buffer.
     * Maximal descriptor length is limited to 255. Function is blocking
     * type and must not be called from interrupt context.
     * @param[in] res Driver resources
     * @param[in] descType Type of descriptor to get (CH9_USB_DT_DEVICE,
     * CH9_USB_DT_CONFIGURATION,...)
     * @param[in] complete Complete callback function
     * @return CDN_EOK on success
     * @return complete_code XHCI transfer complete status code
     */
    uint32_t (*getDescriptor)(USBSSP_DriverResourcesT* res, uint8_t descType,
                              USBSSP_Complete complete);

    /**
     * Function to send a force event command.
     * @param[in] res Driver resources
     * @param[in] vf_id vf_id ID of VF whose Event Ring will receive event
     * @param[in] vf_int_target vf_int_target ID of the interruptor whose Event
     * Ring will receive event
     * @param[in] eventPtr eventPtr pointer to event that will be sent
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*forceEvent)(USBSSP_DriverResourcesT* res, uint32_t vf_id,
                           uint32_t vf_int_target,
                           const USBSSP_RingElementT* eventPtr);

    /**
     * Set address. Function executes set address request on connected
     * device.
     * @param[in] res Driver resources
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*setAddress)(USBSSP_DriverResourcesT* res);

    /**
     * Reset port.
     * @param[in] res Driver resources
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*resetRootHubPort)(const USBSSP_DriverResourcesT* res);

    /**
     * Issue generic command to SSP controller.
     * @param[in] res Driver resources
     * @param[in] dword0 word 0 of command
     * @param[in] dword1 word 1 of command
     * @param[in] dword2 word 2 of command
     * @param[in] dword3 word 3 of command
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*issueGenericCommand)(USBSSP_DriverResourcesT* res,
                                    uint32_t dword0, uint32_t dword1,
                                    uint32_t dword2, uint32_t dword3);

    /**
     * Set feature on device's endpoint. Functions sends setup requested
     * to device with set/cleared endpoint feature
     * @param[in] res Driver resources
     * @param[in] epIndex Index of endpoint to set/clear feature on $RANGE $FROM
     * 1U $TO USBSSP_EP_CONT_MAX $
     * @param[in] feature When 1 sets stall, when 0 clears stall
     * @return CDN_EOK on success
     * @return complete_code XHCI transfer complete status code
     */
    uint32_t (*endpointSetFeature)(USBSSP_DriverResourcesT* res,
                                   uint8_t epIndex, uint8_t feature);

    /**
     * Set configuration. Function configures USBSSP controller as well
     * as device connected to this USBSSP controller. Function must not
     * be called from interrupt context.
     * @param[in] res Driver resources
     * @param[in] configValue USB device's configuration selector
     * @param[in] epCfgBuffer Configuration buffer address
     * @param[in] epCfgBufferLen Configuration buffer len
     * @param[in] complete Complete callback function
     * @return CDN_EOK on success
     * @return complete_code XHCI transfer complete status code
     */
    uint32_t (*setConfiguration)(USBSSP_DriverResourcesT* res,
                                 uint32_t configValue, uint8_t* epCfgBuffer,
                                 uint16_t epCfgBufferLen,
                                 USBSSP_Complete complete);

    /**
     * Control transfer. Function executes control transfer. Information
     * about transfer like: data direction, data length, wIndex, wValue
     * etc. are passed in 'setup' parameter.
     * @param[in] res Driver resources
     * @param[in] setup Keeps setup packet
     * @param[in] pdata Pointer for data to send/receive $RANGE $NULLABLE $
     * @return CDN_EOK on success
     * @return complete_code XHCI transfer complete status code*
     */
    uint32_t (*controlTransfer)(USBSSP_DriverResourcesT* res,
                                const CH9_UsbSetup* setup,
                                const uint8_t* pdata);

    /**
     * No blocking control transfer. Function executes control transfer.
     * Information about transfer like: data direction, data length,
     * wIndex, wValue etc. are passed in 'setup' parameter.
     * @param[in] res Driver resources
     * @param[in] setup Keeps setup packet
     * @param[in] pdata Pointer for data to send/receive $RANGE $NULLABLE $
     * @param[in] complete Complete callback function
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*nBControlTransfer)(USBSSP_DriverResourcesT* res,
                                  const CH9_UsbSetup* setup,
                                  const uint8_t* pdata,
                                  USBSSP_Complete complete);

    /**
     * Control transfer in device mode. Function used in response to
     * setup event
     * @param[in] res Driver resources
     * @param[in] pdata Pointer for data to send/receive $RANGE $NULLABLE $
     * @param[in] length Data length
     * @param[in] dirFlag
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*controlTransferDev)(USBSSP_DriverResourcesT* res,
                                   const uint8_t* pdata, uint32_t length,
                                   uint8_t dirFlag);

    /**
     * No Operation test. Function used for testing purposes:
     * NO_OP_COMMAND is send to USBSSP controller. When event ring
     * receives NO_OP_COMMAND complete it calls complete callback
     * @param[in] res Driver resources
     * @param[in] complete Callback
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*noOpTest)(USBSSP_DriverResourcesT* res,
                         USBSSP_NopComplete complete);

    /**
     * Calculate full/low speed endpoint interval based on bInterval See
     * xHCI spec Section 6.2.3.6 for more details.
     * @param[in] bInterval bInterval
     * @return value valid endpoint contxt interval value
     */
    uint8_t (*calcFsLsEPIntrptInterval)(uint8_t bInterval);

    /**
     * Function enables slot for new connected device.
     * @param[in] res Driver resources
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*enableSlot)(USBSSP_DriverResourcesT* res);

    /**
     * Function disables slot of connected device.
     * @param[in] res Driver resources
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*disableSlot)(USBSSP_DriverResourcesT* res);

    /**
     * Enable endpoint. Function used in device context.
     * @param[in] res Driver resources
     * @param[in] desc pointer to endpoint descriptor
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*enableEndpoint)(USBSSP_DriverResourcesT* res,
                               const uint8_t* desc);

    /**
     * Disable endpoint. Function used in device context.
     * @param[in] res Driver resources
     * @param[in] epAddress address of endpoint to be disabled
     * @return CDN_EINVAL when driver's settings doesn't suit to native platform
     * settings
     * @return CDN_EOK if no errors
     */
    uint32_t (*disableEndpoint)(USBSSP_DriverResourcesT* res,
                                uint8_t epAddress);

    /**
     * Get Endpoint state
     * @param[in] res Pointer to Driver resources
     * @param[in] epIndex EP-index as per device context array (XHCI 6.2.1)
     */
    uint32_t (*GetEpState)(const USBSSP_DriverResourcesT* res,
                           uint32_t epIndex);

    /**
     * Get actual frame number. Function returns actual frame number on
     * USB bus. Remember that maximal frame number can be 0x7FF. Next
     * frame increments returned value from 0.
     * @param[in] res driver resources
     * @param[out] index Micro Frame Index returned by function.
     */
    uint32_t (*getMicroFrameIndex)(USBSSP_DriverResourcesT* res,
                                   uint32_t* index);

    /**
     * @param[in] res
     * @param[in] epIndex
     * @param[in] flags
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*setEndpointExtraFlag)(USBSSP_DriverResourcesT* res,
                                     uint8_t epIndex,
                                     USBSSP_ExtraFlagsEnumT flags);

    /**
     * @param[in] res
     * @param[in] epIndex
     * @param[in] flags
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*cleanEndpointExtraFlag)(USBSSP_DriverResourcesT* res,
                                       uint8_t epIndex,
                                       USBSSP_ExtraFlagsEnumT flags);

    /**
     * @param[in] res
     * @param[in] epIndex
     * @param[out] flag Endpoint Extra Flag returned by pointer
     */
    uint32_t (*getEndpointExtraFlag)(const USBSSP_DriverResourcesT* res,
                                     uint8_t epIndex, uint8_t* flag);

    /**
     * @param[in] res
     * @param[in] epIndex
     * @param[in] frameID
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*setFrameID)(USBSSP_DriverResourcesT* res, uint8_t epIndex,
                           uint32_t frameID);

    /**
     * @param[in] res
     * @param[in] epIndex
     * @param[in] eventDataLo
     * @param[in] eventDataHi
     * @param[in] flags
     * @return value TBC
     */
    uint32_t (*addEventDataTRB)(USBSSP_DriverResourcesT* res, uint8_t epIndex,
                                uint32_t eventDataLo, uint32_t eventDataHi,
                                uint32_t flags);

    /**
     * @param[in] res
     * @param[in] word
     * @param[in] complete
     * @return CDN_EINVAL Invalid Input parameters
     * @return CDN_EOK if no errors
     */
    uint32_t (*forceHeader)(USBSSP_DriverResourcesT* res,
                            const USBSSP_dword word,
                            USBSSP_ForceHeaderComplete complete);

    /**
     * @param[in] res
     * @param[in] portId
     * @param[in] portRegIdx
     * @param[in] regValue
     */
    uint32_t (*setPortControlReg)(const USBSSP_DriverResourcesT* res,
                                  uint8_t portId,
                                  USBSSP_PortControlRegIdx portRegIdx,
                                  uint32_t regValue);

    /**
     * @param[in] res
     * @param[in] portId
     * @param[in] portRegIdx
     * @param[out] regValue
     */
    uint32_t (*getPortControlReg)(const USBSSP_DriverResourcesT* res,
                                  uint8_t portId,
                                  USBSSP_PortControlRegIdx portRegIdx,
                                  uint32_t* regValue);

    /**
     * Enable DDUSB. This function needs to be called with xHC stopped
     * @param[in] res Driver resources
     * @param[in] ddusbTxRegs Pointer to array of 4 ddusb_tx_valid registers
     * @param[in] ddusbRxRegs Pointer to array of 4 ddusb_rx_valid registers
     * @param[in] ddusbEpIntIdx Interrupter index for DDUSB enabled Endpoints
     */
    uint32_t (*EnableDDUSB)(USBSSP_DriverResourcesT* res,
                            const uint32_t* ddusbTxRegs,
                            const uint32_t* ddusbRxRegs,
                            uint32_t ddusbEpIntIdx);

    /**
     * Disable DDUSB. This function needs to be called with xHC stopped
     * @param[in] res Driver resources
     * @param[in] epIntIdx Restored Interrupter index for DDUSB enabled
     * Endpoints
     */
    uint32_t (*DisableDDUSB)(USBSSP_DriverResourcesT* res, uint32_t epIntIdx);

    /**
     * Save state and stop xHC
     * @param[in] res Driver resources
     * @param[in] drvContext Pointer to driver context struct
     */
    uint32_t (*SaveState)(USBSSP_DriverResourcesT* res,
                          USBSSP_DriverContextT* drvContext);

    /**
     * Restore state and start xHC
     * @param[in] res Driver resources
     * @param[in] drvContext Pointer to driver context struct
     */
    uint32_t (*RestoreState)(USBSSP_DriverResourcesT* res,
                             const USBSSP_DriverContextT* drvContext);

} USBSSP_OBJ;

/**
 * In order to access the USBSSP APIs, the upper layer software must call
 * this global function to obtain the pointer to the driver object.
 * @return USBSSP_OBJ* Driver Object Pointer
 */
extern USBSSP_OBJ* USBSSP_GetInstance(void);

/**
 *  @}
 */

#endif /* CDN_XHCI_OBJ_IF_H */
