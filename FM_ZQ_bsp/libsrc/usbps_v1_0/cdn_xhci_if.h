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

#ifndef CDN_XHCI_IF_H
#define CDN_XHCI_IF_H

#include "cdn_errno.h"
#include "cdn_stdtypes.h"
#include "cdn_xhci_priv.h"
#include "cps.h"
#include "cusb_ch9_if.h"
#include "cusb_ch9_structs_if.h"
#include "cusbd_if.h"

/** @defgroup ConfigInfo  Configuration and Hardware Operation Information
 *  The following definitions specify the driver operation environment that
 *  is defined by hardware configuration or client code. These defines are
 *  located in the header file of the core driver.
 *  @{
 */

/**********************************************************************
 * Defines
 **********************************************************************/
#define USBSSP_EXTENDED_CONTEXT    1U

#define USBSSP_CONTEXT_WIDTH       16U

#define USBSSP_DBG_DRV             0x000000010U

#define USBSSP_DBG_CUSBD           0x000000020U

#define USBSSP_HOST_OFFSET         0x8000U

#define USBSSP_DEVICE_OFFSET       0x4000U

#define USBSSP_OTG_OFFSET          0x0000U

#define USBSSP_DDUSB_CONFIG_OFFSET 0x1000U

#define USBSSP_DDUSB_ERBASE_OFFSET 0x1100U

#define USBSSP_MAGIC_NUMBER        0x0004034EU

#define USBSSP_RING_ALIGNMENT      64U

#define USBSSP_RING_BOUNDARY       65536U

#define USBSSP_ERST_ALIGNMENT      64U

#define USBSSP_ERST_BOUNDARY       0U

#define USBSSP_CONTEXT_ALIGNMENT   64U

#define USBSSP_DCBAA_ALIGNMENT     64U

#define USBSSP_DEFAULT_TIMEOUT     100000U

/** Number of interrupters supported */
#define USBSSP_INTERRUPTER_COUNT   4U

#define USBSSP_DBG_TEST            0x000000040U

#define USBSSP_DBG_EXTERNAL_STACK  0x000000080U

#define USBSSP_EVENT_QUEUE_SIZE    64U

#define USBSSP_SCRATCHPAD_BUFF_NUM 63U

#define USBSSP_PAGE_SIZE           4096U

#define USBSSP_PRODUCER_QUEUE_SIZE 64U

#define USBSSP_MAX_SPEED           6U

#define USBSSP_MAX_STRING_NUM      5U

//#define	USBSSP_MAX_EP_CONTEXT_NUM 30U
#define	USBSSP_MAX_EP_CONTEXT_NUM 4U

#define USBSSP_MAX_EP_NUM_STRM_EN  30U

#define USBSSP_MAX_DEVICE_SLOT_NUM 64U

/** given according to XHCI register value: 1 = 4 streams, 2 = 8 streams, 3 = 16
 * streams and so on */
#define USBSSP_MAX_STREMS_PER_EP   2U

/** Should be calculated according to formula: STREAM_ARRAY_SIZE = 2
 * exp(MAX_STREMS_PER_EP + 1) */
#define USBSSP_STREAM_ARRAY_SIZE   8U

/** Device mode ports settings */
#define USBSSP_DEV_MODE_2_PORT     0U

#define USBSSP_DEV_MODE_3_PORT     1U

/** Endpoint0 container offset value */
#define USBSSP_EP0_CONT_OFFSET     1U

/** Endpoint container offset value */
#define USBSSP_EP_CONT_OFFSET      2U

/** Endpoint container offset max value */
#define USBSSP_EP_CONT_MAX         32U

/** Endpoint 0 data buffer size - used in enumeration */
#define USBSSP_EP0_DATA_BUFF_SIZE  1024U

/**
 *  @}
 */

/** @defgroup DataStructure Dynamic Data Structures
 *  This section defines the data structures used by the driver to provide
 *  hardware information, modification and dynamic operation of the driver.
 *  These data structures are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
 * Type defines
 **********************************************************************/

typedef uint32_t USBSSP_DmaAddrT;

/**********************************************************************
 * Forward declarations
 **********************************************************************/
typedef struct USBSSP_RingElementT_s USBSSP_RingElementT;
typedef struct USBSSP_InputContexT_s USBSSP_InputContexT;
typedef struct USBSSP_OutputContexT_s USBSSP_OutputContexT;
typedef struct USBSSP_ProducerQueueT_s USBSSP_ProducerQueueT;
typedef struct USBSSP_DescT_s USBSSP_DescT;
typedef struct USBSSP_DcbaaT_s USBSSP_DcbaaT;
typedef struct USBSSP_XhciResourcesT_s USBSSP_XhciResourcesT;
typedef struct USBSSP_Callbacks_s USBSSP_Callbacks;
typedef struct USBSSP_DriverResourcesT_s USBSSP_DriverResourcesT;
typedef struct USBSSP_DriverConfigT_s USBSSP_DriverConfigT;
typedef struct USBSSP_DriverContextT_s USBSSP_DriverContextT;
typedef struct USBSSP_dword_s USBSSP_dword;
typedef struct USBSSP_param_s USBSSP_param;

typedef struct USBSSP_QuickAccessRegs_s USBSSP_QuickAccessRegs;
typedef struct USBSSP_SfrT_s USBSSP_SfrT;

/**********************************************************************
 * Enumerations
 **********************************************************************/
typedef enum {
    USBSSP_EP0_UNCONNECTED = 0U,
    USBSSP_EP0_HALT_PENDING = 1U,
    USBSSP_EP0_HALT_SETUP_PENDING = 2U,
    USBSSP_EP0_HALTED = 3U,
    USBSSP_EP0_SETUP_PENDING = 4U,
    USBSSP_EP0_SETUP_PHASE = 5U,
    USBSSP_EP0_DATA_PHASE = 6U,
    USBSSP_EP0_STATUS_PHASE = 7U
} USBSSP_Ep0StateEnum;

typedef enum {
    USBSSP_EXTRAFLAGSENUMT_UNDEFINED = 0U,
    USBSSP_EXTRAFLAGSENUMT_NODORBELL = 1U,
    USBSSP_EXTRAFLAGSENUMT_FORCELINKTRB = 2U
} USBSSP_ExtraFlagsEnumT;

/**********************************************************************
 * Callbacks
 **********************************************************************/
/** Completion callback */
typedef void (*USBSSP_Complete)(USBSSP_DriverResourcesT* arg, uint32_t status,
                                const USBSSP_RingElementT* eventPtr,
                                uint8_t* buffer, uint32_t actualLength);

/** No-Op completion callback */
typedef void (*USBSSP_NopComplete)(USBSSP_DriverResourcesT* arg);

/** Force Header complete callback function */
typedef void (*USBSSP_ForceHeaderComplete)(USBSSP_DriverResourcesT* arg);

/** Used for testing purposes */
typedef uint8_t (*USBSSP_GenericCallback)(USBSSP_DriverResourcesT* res,
                                          USBSSP_RingElementT* eventPtr);

/** Used for testing purposes */
typedef void (*USBSSP_PostCallback)(USBSSP_DriverResourcesT* res,
                                    USBSSP_RingElementT* eventPtr);

/** Used for testing purposes */
typedef void (*USBSSP_PreportChangeDetectCallback)(USBSSP_DriverResourcesT* res,
                                                   uint32_t portsc_value,
                                                   uint8_t port_id);

/** Used for virtualization */
typedef void (*USBSSP_InputContextCallback)(USBSSP_DriverResourcesT* arg);

/** Used for testing purposes */
typedef void (*USBSSP_SetInterfaceCallback)(
    USBSSP_DriverResourcesT* res, USBSSP_SetInterfaceState* configEpCmd,
    uint32_t epMask);

/** Pointer to function that maps logical pointer to physical one */
typedef uintptr_t (*USBSSP_get_phys_from_log_ptr_proc_t)(void* log_ptr,
                                                         int32_t byte_size);

/** Pointer to function that maps physical pointer to logical one */
typedef void* (*USBSSP_get_log_from_phys_ptr_proc_t)(uintptr_t phys_ptr,
                                                     int32_t byte_size);

/**
 *  @}
 */

/** @defgroup DriverFunctionAPI Driver Function API
 *  Prototypes for the driver API functions. The user application can link
 * statically to the necessary API functions and call them directly.
 *  @{
 */

/**********************************************************************
 * API methods
 **********************************************************************/

/**
 * Transfer data on given endpoint. This function is non-blocking
 * type. The XHCI operation result should be checked in complete
 * callback function.
 * @param[in] res Driver resources
 * @param[in] epIndex index of endpoint according to xhci spec e.g for ep1out
 *    epIndex=2, for ep1in epIndex=3, for ep2out epIndex=4 end so on $RANGE
 * $FROM 1U $TO USBSSP_EP_CONT_MAX $
 * @param[in] buff Buffer for data to send or to receive
 * @param[in] size Size of data in bytes
 * @param[in] complete pointer to function which will be returned in callback in
 * input parameter, can be set to NULL when no extra parameter used
 * @return CDN_EINVAL if selected endpoint index is out of available range
 * @return CDN_EOK if selected endpoint is within available endpoint range
 */
uint32_t USBSSP_TransferData(USBSSP_DriverResourcesT* res, uint8_t epIndex,
                             const uintptr_t buff, uint32_t size,
                             USBSSP_Complete complete);

/**
 * @param[in] res Driver resources
 * @param[in] epIndex index of endpoint according to xhci spec e.g for ep1out
 *    epIndex=2, for ep1in epIndex=3, for ep2out epIndex=4 end so on $RANGE
 * $FROM 1U $TO USBSSP_EP_CONT_MAX $
 * @param[in] paramT Structure having Pointer to buffer for data,size of data
 * @param[in] complete pointer to function which will be returned in callback in
 * input parameter, can be set to NULL when no extra parameter used
 * @return CDN_EINVAL if selected endpoint index is out of available range
 * @return CDN_EOK if selected endpoint is within available endpoint range
 */
uint32_t USBSSP_TransferData2(USBSSP_DriverResourcesT* res, uint8_t epIndex,
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
uint32_t USBSSP_StopEndpoint(USBSSP_DriverResourcesT* res, uint8_t endpoint);

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
uint32_t USBSSP_ResetEndpoint(USBSSP_DriverResourcesT* res, uint8_t endpoint);

/**
 * Reset of connected device. Function sends RESET_DEVICE_COMMAND to
 * USBSSP controller in order to issue reset state on USB bus.
 * @param[in] res Driver resources
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK selected endpoint is within available endpoint range
 */
uint32_t USBSSP_ResetDevice(USBSSP_DriverResourcesT* res);

/**
 * Handling of USBSSP controller interrupt. Function is called from
 * USBSSP interrupt context.
 * @param[in] res Driver resources
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_Isr(USBSSP_DriverResourcesT* res);

/**
 * Function sets memory resources used by driver.
 * @param[in] res Driver resources
 * @param[in] memRes User defined memory resources.
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_SetMemRes(USBSSP_DriverResourcesT* res,
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
uint32_t USBSSP_Init(USBSSP_DriverResourcesT* res,
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
uint32_t USBSSP_GetDescriptor(USBSSP_DriverResourcesT* res, uint8_t descType,
                              USBSSP_Complete complete);

/**
 * USBSSP_ControlInTest.
 * @param[in] res Driver resources
 * @param[in] data Pointer for data to send/receive
 * @return CDN_EOK on success
 */
uint32_t USBSSP_ControlInTest(USBSSP_DriverResourcesT* res, uint8_t* data);

/**
 * USBSSP_ControlOutTest.
 * @param[in] res Driver resources
 * @param[in] data Pointer for data to send/receive
 * @return CDN_EOK on success
 */
uint32_t USBSSP_ControlOutTest(USBSSP_DriverResourcesT* res, uint8_t* data);

/**
 * Function to send a force event command.
 * @param[in] res Driver resources
 * @param[in] vf_id vf_id ID of VF whose Event Ring will receive event
 * @param[in] vf_int_target vf_int_target ID of the interruptor whose Event Ring
 * will receive event
 * @param[in] eventPtr eventPtr pointer to event that will be sent
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_ForceEvent(USBSSP_DriverResourcesT* res, uint32_t vf_id,
                           uint32_t vf_int_target,
                           const USBSSP_RingElementT* eventPtr);

/**
 * Set address. Function executes set address request on connected
 * device.
 * @param[in] res Driver resources
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_SetAddress(USBSSP_DriverResourcesT* res);

/**
 * Reset port.
 * @param[in] res Driver resources
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_ResetRootHubPort(const USBSSP_DriverResourcesT* res);

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
uint32_t USBSSP_IssueGenericCommand(USBSSP_DriverResourcesT* res,
                                    uint32_t dword0, uint32_t dword1,
                                    uint32_t dword2, uint32_t dword3);

/**
 * Set feature on device's endpoint. Functions sends setup requested
 * to device with set/cleared endpoint feature
 * @param[in] res Driver resources
 * @param[in] epIndex Index of endpoint to set/clear feature on $RANGE $FROM 1U
 * $TO USBSSP_EP_CONT_MAX $
 * @param[in] feature When 1 sets stall, when 0 clears stall
 * @return CDN_EOK on success
 * @return complete_code XHCI transfer complete status code
 */
uint32_t USBSSP_EndpointSetFeature(USBSSP_DriverResourcesT* res,
                                   uint8_t epIndex, uint8_t feature);

/**
 * Set configuration. Function configures USBSSP controller as well as
 * device connected to this USBSSP controller. Function must not be
 * called from interrupt context.
 * @param[in] res Driver resources
 * @param[in] configValue USB device's configuration selector
 * @param[in] epCfgBuffer Configuration buffer address
 * @param[in] epCfgBufferLen Configuration buffer len
 * @param[in] complete Complete callback function
 * @return CDN_EOK on success
 * @return complete_code XHCI transfer complete status code
 */
uint32_t USBSSP_SetConfiguration(USBSSP_DriverResourcesT* res,
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
uint32_t USBSSP_ControlTransfer(USBSSP_DriverResourcesT* res,
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
uint32_t USBSSP_NBControlTransfer(USBSSP_DriverResourcesT* res,
                                  const CH9_UsbSetup* setup,
                                  const uint8_t* pdata,
                                  USBSSP_Complete complete);

/**
 * Control transfer in device mode. Function used in response to setup
 * event
 * @param[in] res Driver resources
 * @param[in] pdata Pointer for data to send/receive $RANGE $NULLABLE $
 * @param[in] length Data length
 * @param[in] dirFlag
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_ControlTransferDev(USBSSP_DriverResourcesT* res,
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
uint32_t USBSSP_NoOpTest(USBSSP_DriverResourcesT* res,
                         USBSSP_NopComplete complete);

/**
 * Calculate full/low speed endpoint interval based on bInterval See
 * xHCI spec Section 6.2.3.6 for more details.
 * @param[in] bInterval bInterval
 * @return value valid endpoint contxt interval value
 */
uint8_t USBSSP_CalcFsLsEPIntrptInterval(uint8_t bInterval);

/**
 * Function enables slot for new connected device.
 * @param[in] res Driver resources
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_EnableSlot(USBSSP_DriverResourcesT* res);

/**
 * Function disables slot of connected device.
 * @param[in] res Driver resources
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_DisableSlot(USBSSP_DriverResourcesT* res);

/**
 * Enable endpoint. Function used in device context.
 * @param[in] res Driver resources
 * @param[in] desc pointer to endpoint descriptor
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_EnableEndpoint(USBSSP_DriverResourcesT* res,
                               const uint8_t* desc);

/**
 * Disable endpoint. Function used in device context.
 * @param[in] res Driver resources
 * @param[in] epAddress address of endpoint to be disabled
 * @return CDN_EINVAL when driver's settings doesn't suit to native platform
 * settings
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_DisableEndpoint(USBSSP_DriverResourcesT* res,
                                uint8_t epAddress);

/**
 * Get Endpoint state
 * @param[in] res Pointer to Driver resources
 * @param[in] epIndex EP-index as per device context array (XHCI 6.2.1)
 */
uint32_t USBSSP_GetEpState(const USBSSP_DriverResourcesT* res,
                           uint32_t epIndex);

/**
 * Get actual frame number. Function returns actual frame number on
 * USB bus. Remember that maximal frame number can be 0x7FF. Next
 * frame increments returned value from 0.
 * @param[in] res driver resources
 * @param[out] index Micro Frame Index returned by function.
 */
uint32_t USBSSP_GetMicroFrameIndex(USBSSP_DriverResourcesT* res,
                                   uint32_t* index);

/**
 * @param[in] res
 * @param[in] epIndex
 * @param[in] flags
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_SetEndpointExtraFlag(USBSSP_DriverResourcesT* res,
                                     uint8_t epIndex,
                                     USBSSP_ExtraFlagsEnumT flags);

/**
 * @param[in] res
 * @param[in] epIndex
 * @param[in] flags
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_CleanEndpointExtraFlag(USBSSP_DriverResourcesT* res,
                                       uint8_t epIndex,
                                       USBSSP_ExtraFlagsEnumT flags);

/**
 * @param[in] res
 * @param[in] epIndex
 * @param[out] flag Endpoint Extra Flag returned by pointer
 */
uint32_t USBSSP_GetEndpointExtraFlag(const USBSSP_DriverResourcesT* res,
                                     uint8_t epIndex, uint8_t* flag);

/**
 * @param[in] res
 * @param[in] epIndex
 * @param[in] frameID
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_SetFrameID(USBSSP_DriverResourcesT* res, uint8_t epIndex,
                           uint32_t frameID);

/**
 * @param[in] res
 * @param[in] epIndex
 * @param[in] eventDataLo
 * @param[in] eventDataHi
 * @param[in] flags
 * @return value TBC
 */
uint32_t USBSSP_AddEventDataTRB(USBSSP_DriverResourcesT* res, uint8_t epIndex,
                                uint32_t eventDataLo, uint32_t eventDataHi,
                                uint32_t flags);

/**
 * @param[in] res
 * @param[in] word
 * @param[in] complete
 * @return CDN_EINVAL Invalid Input parameters
 * @return CDN_EOK if no errors
 */
uint32_t USBSSP_ForceHeader(USBSSP_DriverResourcesT* res,
                            const USBSSP_dword word,
                            USBSSP_ForceHeaderComplete complete);

/**
 * @param[in] res
 * @param[in] portId
 * @param[in] portRegIdx
 * @param[in] regValue
 */
uint32_t USBSSP_SetPortControlReg(const USBSSP_DriverResourcesT* res,
                                  uint8_t portId,
                                  USBSSP_PortControlRegIdx portRegIdx,
                                  uint32_t regValue);

/**
 * @param[in] res
 * @param[in] portId
 * @param[in] portRegIdx
 * @param[out] regValue
 */
uint32_t USBSSP_GetPortControlReg(const USBSSP_DriverResourcesT* res,
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
uint32_t USBSSP_EnableDDUSB(USBSSP_DriverResourcesT* res,
                            const uint32_t* ddusbTxRegs,
                            const uint32_t* ddusbRxRegs,
                            uint32_t ddusbEpIntIdx);

/**
 * Disable DDUSB. This function needs to be called with xHC stopped
 * @param[in] res Driver resources
 * @param[in] epIntIdx Restored Interrupter index for DDUSB enabled Endpoints
 */
uint32_t USBSSP_DisableDDUSB(USBSSP_DriverResourcesT* res, uint32_t epIntIdx);

/**
 * Save state and stop xHC
 * @param[in] res Driver resources
 * @param[in] drvContext Pointer to driver context struct
 */
uint32_t USBSSP_SaveState(USBSSP_DriverResourcesT* res,
                          USBSSP_DriverContextT* drvContext);

/**
 * Restore state and start xHC
 * @param[in] res Driver resources
 * @param[in] drvContext Pointer to driver context struct
 */
uint32_t USBSSP_RestoreState(USBSSP_DriverResourcesT* res,
                             const USBSSP_DriverContextT* drvContext);

/**
 *  @}
 */

#endif /* CDN_XHCI_IF_H */
