/******************************************************************************
*
* Copyright (C) 2002 - 2015 FMSH, Inc. All rights reserved.
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file pm_status.h
*
* @addtogroup common_status_codes FMSH&reg; software status codes
*
* The pm_status.h file contains the FMSH&reg; software status codes.These codes are
* used throughout the FMSH device drivers.
*
* @{
******************************************************************************/

#ifndef PM_STATUS_H		/* prevent circular inclusions */
#define PM_STATUS_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "fmsh_common_types.h"

/************************** Constant Definitions *****************************/

/*********************** Common statuses 0 - 500 *****************************/
/**
@name Common Status Codes for All Device Drivers
@{
*/
#ifndef FPMST_SUCCESS
#define FPMST_SUCCESS                     0L
#endif
#ifndef FPMST_FAILURE
#define FPMST_FAILURE                     1L
#endif
#define FPMST_DEVICE_NOT_FOUND            2L
#define FPMST_DEVICE_BLOCK_NOT_FOUND      3L
#define FPMST_INVALID_VERSION             4L
#define FPMST_DEVICE_IS_STARTED           5L
#define FPMST_DEVICE_IS_STOPPED           6L
#define FPMST_FIFO_ERROR                  7L	/*!< An error occurred during an
						   operation with a FIFO such as
						   an underrun or overrun, this
						   error requires the device to
						   be reset */
#define FPMST_RESET_ERROR                 8L	/*!< An error occurred which requires
						   the device to be reset */
#define FPMST_DMA_ERROR                   9L	/*!< A DMA error occurred, this error
						   typically requires the device
						   using the DMA to be reset */
#define FPMST_NOT_POLLED                  10L	/*!< The device is not configured for
						   polled mode operation */
#define FPMST_FIFO_NO_ROOM                11L	/*!< A FIFO did not have room to put
						   the specified data into */
#define FPMST_BUFFER_TOO_SMALL            12L	/*!< The buffer is not large enough
						   to hold the expected data */
#define FPMST_NO_DATA                     13L	/*!< There was no data available */
#define FPMST_REGISTER_ERROR              14L	/*!< A register did not contain the
						   expected value */
#define FPMST_INVALID_PARAM               15L	/*!< An invalid parameter was passed
						   into the function */
#define FPMST_NOT_SGDMA                   16L	/*!< The device is not configured for
						   scatter-gather DMA operation */
#define FPMST_LOOPBACK_ERROR              17L	/*!< A loopback test failed */
#define FPMST_NO_CALLBACK                 18L	/*!< A callback has not yet been
						   registered */
#define FPMST_NO_FEATURE                  19L	/*!< Device is not configured with
						   the requested feature */
#define FPMST_NOT_INTERRUPT               20L	/*!< Device is not configured for
						   interrupt mode operation */
#define FPMST_DEVICE_BUSY                 21L	/*!< Device is busy */
#define FPMST_ERROR_COUNT_MAX             22L	/*!< The error counters of a device
						   have maxed out */
#define FPMST_IS_STARTED                  23L	/*!< Used when part of device is
						   already started i.e.
						   sub channel */
#define FPMST_IS_STOPPED                  24L	/*!< Used when part of device is
						   already stopped i.e.
						   sub channel */
#define FPMST_DATA_LOST                   26L	/*!< Driver defined error */
#define FPMST_RECV_ERROR                  27L	/*!< Generic receive error */
#define FPMST_SEND_ERROR                  28L	/*!< Generic transmit error */
#define FPMST_NOT_ENABLED                 29L	/*!< A requested service is not
						   available because it has not
						   been enabled */
#define FPMST_NO_ACCESS			30L	/* Generic access error */
/** @} */
/***************** Utility Component statuses 401 - 500  *********************/
/**
@name Utility Component Status Codes 401 - 500
@{
*/
#define FPMST_MEMTEST_FAILED              401L	/*!< Memory test failed */

/** @} */
/***************** Common Components statuses 501 - 1000 *********************/
/**
@name Packet Fifo Status Codes 501 - 510
@{
*/
/********************* Packet Fifo statuses 501 - 510 ************************/

#define FPMST_PFIFO_LACK_OF_DATA          501L	/*!< Not enough data in FIFO   */
#define FPMST_PFIFO_NO_ROOM               502L	/*!< Not enough room in FIFO   */
#define FPMST_PFIFO_BAD_REG_VALUE         503L	/*!< Self test, a register value
						   was invalid after reset */
#define FPMST_PFIFO_ERROR                 504L	/*!< Generic packet FIFO error */
#define FPMST_PFIFO_DEADLOCK              505L	/*!< Packet FIFO is reporting
						 * empty and full simultaneously
						 */
/** @} */
/**
@name DMA Status Codes 511 - 530
@{
*/
/************************** DMA statuses 511 - 530 ***************************/

#define FPMST_DMA_TRANSFER_ERROR          511L	/*!< Self test, DMA transfer
						   failed */
#define FPMST_DMA_RESET_REGISTER_ERROR    512L	/*!< Self test, a register value
						   was invalid after reset */
#define FPMST_DMA_SG_LIST_EMPTY           513L	/*!< Scatter gather list contains
						   no buffer descriptors ready
						   to be processed */
#define FPMST_DMA_SG_IS_STARTED           514L	/*!< Scatter gather not stopped */
#define FPMST_DMA_SG_IS_STOPPED           515L	/*!< Scatter gather not running */
#define FPMST_DMA_SG_LIST_FULL            517L	/*!< All the buffer desciptors of
						   the scatter gather list are
						   being used */
#define FPMST_DMA_SG_BD_LOCKED            518L	/*!< The scatter gather buffer
						   descriptor which is to be
						   copied over in the scatter
						   list is locked */
#define FPMST_DMA_SG_NOTHING_TO_COMMIT    519L	/*!< No buffer descriptors have been
						   put into the scatter gather
						   list to be commited */
#define FPMST_DMA_SG_COUNT_EXCEEDED       521L	/*!< The packet count threshold
						   specified was larger than the
						   total # of buffer descriptors
						   in the scatter gather list */
#define FPMST_DMA_SG_LIST_EXISTS          522L	/*!< The scatter gather list has
						   already been created */
#define FPMST_DMA_SG_NO_LIST              523L	/*!< No scatter gather list has
						   been created */
#define FPMST_DMA_SG_BD_NOT_COMMITTED     524L	/*!< The buffer descriptor which was
						   being started was not committed
						   to the list */
#define FPMST_DMA_SG_NO_DATA              525L	/*!< The buffer descriptor to start
						   has already been used by the
						   hardware so it can't be reused
						 */
#define FPMST_DMA_SG_LIST_ERROR           526L	/*!< General purpose list access
						   error */
#define FPMST_DMA_BD_ERROR                527L	/*!< General buffer descriptor
						   error */
/** @} */
/**
@name IPIF Status Codes Codes 531 - 550
@{
*/
/************************** IPIF statuses 531 - 550 ***************************/

#define FPMST_IPIF_REG_WIDTH_ERROR        531L	/*!< An invalid register width
						   was passed into the function */
#define FPMST_IPIF_RESET_REGISTER_ERROR   532L	/*!< The value of a register at
						   reset was not valid */
#define FPMST_IPIF_DEVICE_STATUS_ERROR    533L	/*!< A write to the device interrupt
						   status register did not read
						   back correctly */
#define FPMST_IPIF_DEVICE_ACK_ERROR       534L	/*!< The device interrupt status
						   register did not reset when
						   acked */
#define FPMST_IPIF_DEVICE_ENABLE_ERROR    535L	/*!< The device interrupt enable
						   register was not updated when
						   other registers changed */
#define FPMST_IPIF_IP_STATUS_ERROR        536L	/*!< A write to the IP interrupt
						   status register did not read
						   back correctly */
#define FPMST_IPIF_IP_ACK_ERROR           537L	/*!< The IP interrupt status register
						   did not reset when acked */
#define FPMST_IPIF_IP_ENABLE_ERROR        538L	/*!< IP interrupt enable register was
						   not updated correctly when other
						   registers changed */
#define FPMST_IPIF_DEVICE_PENDING_ERROR   539L	/*!< The device interrupt pending
						   register did not indicate the
						   expected value */
#define FPMST_IPIF_DEVICE_ID_ERROR        540L	/*!< The device interrupt ID register
						   did not indicate the expected
						   value */
#define FPMST_IPIF_ERROR                  541L	/*!< Generic ipif error */
/** @} */

/****************** Device specific statuses 1001 - 4095 *********************/
/**
@name Ethernet Status Codes 1001 - 1050
@{
*/
/********************* Ethernet statuses 1001 - 1050 *************************/

#define FPMST_EMAC_MEMORY_SIZE_ERROR  1001L	/*!< Memory space is not big enough
						 * to hold the minimum number of
						 * buffers or descriptors */
#define FPMST_EMAC_MEMORY_ALLOC_ERROR 1002L	/*!< Memory allocation failed */
#define FPMST_EMAC_MII_READ_ERROR     1003L	/*!< MII read error */
#define FPMST_EMAC_MII_BUSY           1004L	/*!< An MII operation is in progress */
#define FPMST_EMAC_OUT_OF_BUFFERS     1005L	/*!< Driver is out of buffers */
#define FPMST_EMAC_PARSE_ERROR        1006L	/*!< Invalid driver init string */
#define FPMST_EMAC_COLLISION_ERROR    1007L	/*!< Excess deferral or late
						 * collision on polled send */
/** @} */
/**
@name UART Status Codes 1051 - 1075
@{
*/
/*********************** UART statuses 1051 - 1075 ***************************/
#define FPMST_UART

#define FPMST_UART_INIT_ERROR         1051L
#define FPMST_UART_START_ERROR        1052L
#define FPMST_UART_CONFIG_ERROR       1053L
#define FPMST_UART_TEST_FAIL          1054L
#define FPMST_UART_BAUD_ERROR         1055L
#define FPMST_UART_BAUD_RANGE         1056L

/** @} */
/**
@name IIC Status Codes 1076 - 1100
@{
*/
/************************ IIC statuses 1076 - 1100 ***************************/

#define FPMST_IIC_SELFTEST_FAILED         1076	/*!< self test failed            */
#define FPMST_IIC_BUS_BUSY                1077	/*!< bus found busy              */
#define FPMST_IIC_GENERAL_CALL_ADDRESS    1078	/*!< mastersend attempted with   */
					     /* general call address        */
#define FPMST_IIC_STAND_REG_RESET_ERROR   1079	/*!< A non parameterizable reg   */
					     /* value after reset not valid */
#define FPMST_IIC_TX_FIFO_REG_RESET_ERROR 1080	/*!< Tx fifo included in design  */
					     /* value after reset not valid */
#define FPMST_IIC_RX_FIFO_REG_RESET_ERROR 1081	/*!< Rx fifo included in design  */
					     /* value after reset not valid */
#define FPMST_IIC_TBA_REG_RESET_ERROR     1082	/*!< 10 bit addr incl in design  */
					     /* value after reset not valid */
#define FPMST_IIC_CR_READBACK_ERROR       1083	/*!< Read of the control register */
					     /* didn't return value written */
#define FPMST_IIC_DTR_READBACK_ERROR      1084	/*!< Read of the data Tx reg     */
					     /* didn't return value written */
#define FPMST_IIC_DRR_READBACK_ERROR      1085	/*!< Read of the data Receive reg */
					     /* didn't return value written */
#define FPMST_IIC_ADR_READBACK_ERROR      1086	/*!< Read of the data Tx reg     */
					     /* didn't return value written */
#define FPMST_IIC_TBA_READBACK_ERROR      1087	/*!< Read of the 10 bit addr reg */
					     /* didn't return written value */
#define FPMST_IIC_NOT_SLAVE               1088	/*!< The device isn't a slave    */
#define FPMST_IIC_ARB_LOST 				1089 	/*!< Arbitration lost for master	*/
/** @} */
/**
@name ATMC Status Codes 1101 - 1125
@{
*/
/*********************** ATMC statuses 1101 - 1125 ***************************/

#define FPMST_ATMC_ERROR_COUNT_MAX    1101L	/*!< the error counters in the ATM
						   controller hit the max value
						   which requires the statistics
						   to be cleared */
/** @} */
/**
@name Flash Status Codes 1126 - 1150
@{
*/
/*********************** Flash statuses 1126 - 1150 **************************/

#define FPMST_FLASH_BUSY                1126L	/*!< Flash is erasing or programming
						 */
#define FPMST_FLASH_READY               1127L	/*!< Flash is ready for commands */
#define FPMST_FLASH_ERROR               1128L	/*!< Flash had detected an internal
						   error. Use XFlash_DeviceControl
						   to retrieve device specific codes
						 */
#define FPMST_FLASH_ERASE_SUSPENDED     1129L	/*!< Flash is in suspended erase state
						 */
#define FPMST_FLASH_WRITE_SUSPENDED     1130L	/*!< Flash is in suspended write state
						 */
#define FPMST_FLASH_PART_NOT_SUPPORTED  1131L	/*!< Flash type not supported by
						   driver */
#define FPMST_FLASH_NOT_SUPPORTED       1132L	/*!< Operation not supported */
#define FPMST_FLASH_TOO_MANY_REGIONS    1133L	/*!< Too many erase regions */
#define FPMST_FLASH_TIMEOUT_ERROR       1134L	/*!< Programming or erase operation
						   aborted due to a timeout */
#define FPMST_FLASH_ADDRESS_ERROR       1135L	/*!< Accessed flash outside its
						   addressible range */
#define FPMST_FLASH_ALIGNMENT_ERROR     1136L	/*!< Write alignment error */
#define FPMST_FLASH_BLOCKING_CALL_ERROR 1137L	/*!< Couldn't return immediately from
						   write/erase function with
						   XFL_NON_BLOCKING_WRITE/ERASE
						   option cleared */
#define FPMST_FLASH_CFI_QUERY_ERROR     1138L	/*!< Failed to query the device */
/** @} */
/**
@name SPI Status Codes 1151 - 1175
@{
*/
/*********************** SPI statuses 1151 - 1175 ****************************/

#define FPMST_SPI_MODE_FAULT          1151	/*!< master was selected as slave */
#define FPMST_SPI_TRANSFER_DONE       1152	/*!< data transfer is complete */
#define FPMST_SPI_TRANSMIT_UNDERRUN   1153	/*!< slave underruns transmit register */
#define FPMST_SPI_RECEIVE_OVERRUN     1154	/*!< device overruns receive register */
#define FPMST_SPI_NO_SLAVE            1155	/*!< no slave has been selected yet */
#define FPMST_SPI_TOO_MANY_SLAVES     1156	/*!< more than one slave is being
						 * selected */
#define FPMST_SPI_NOT_MASTER          1157	/*!< operation is valid only as master */
#define FPMST_SPI_SLAVE_ONLY          1158	/*!< device is configured as slave-only
						 */
#define FPMST_SPI_SLAVE_MODE_FAULT    1159	/*!< slave was selected while disabled */
#define FPMST_SPI_SLAVE_MODE          1160	/*!< device has been addressed as slave */
#define FPMST_SPI_RECEIVE_NOT_EMPTY   1161	/*!< device received data in slave mode */

#define FPMST_SPI_COMMAND_ERROR       1162	/*!< unrecognised command - qspi only */
#define FPMST_SPI_POLL_DONE           1163        /*!< controller completed polling the
						   device for status */
/** @} */
/**
@name OPB Arbiter Status Codes 1176 - 1200
@{
*/
/********************** OPB Arbiter statuses 1176 - 1200 *********************/

#define FPMST_OPBARB_INVALID_PRIORITY  1176	/*!< the priority registers have either
						 * one master assigned to two or more
						 * priorities, or one master not
						 * assigned to any priority
						 */
#define FPMST_OPBARB_NOT_SUSPENDED     1177	/*!< an attempt was made to modify the
						 * priority levels without first
						 * suspending the use of priority
						 * levels
						 */
#define FPMST_OPBARB_PARK_NOT_ENABLED  1178	/*!< bus parking by id was enabled but
						 * bus parking was not enabled
						 */
#define FPMST_OPBARB_NOT_FIXED_PRIORITY 1179	/*!< the arbiter must be in fixed
						 * priority mode to allow the
						 * priorities to be changed
						 */
/** @} */
/**
@name INTC Status Codes 1201 - 1225
@{
*/
/************************ Intc statuses 1201 - 1225 **************************/

#define FPMST_INTC_FAIL_SELFTEST      1201	/*!< self test failed */
#define FPMST_INTC_CONNECT_ERROR      1202	/*!< interrupt already in use */
/** @} */
/**
@name TmrCtr Status Codes 1226 - 1250
@{
*/
/********************** TmrCtr statuses 1226 - 1250 **************************/

#define FPMST_TMRCTR_TIMER_FAILED     1226	/*!< self test failed */
/** @} */
/**
@name WdtTb Status Codes 1251 - 1275
@{
*/
/********************** WdtTb statuses 1251 - 1275 ***************************/

#define FPMST_WDTTB_TIMER_FAILED      1251L
/** @} */
/**
@name PlbArb status Codes 1276 - 1300
@{
*/
/********************** PlbArb statuses 1276 - 1300 **************************/

#define FPMST_PLBARB_FAIL_SELFTEST    1276L
/** @} */
/**
@name Plb2Opb Status Codes 1301 - 1325
@{
*/
/********************** Plb2Opb statuses 1301 - 1325 *************************/

#define FPMST_PLB2OPB_FAIL_SELFTEST   1301L
/** @} */
/**
@name Opb2Plb Status 1326 - 1350
@{
*/
/********************** Opb2Plb statuses 1326 - 1350 *************************/

#define FPMST_OPB2PLB_FAIL_SELFTEST   1326L
/** @} */
/**
@name SysAce Status Codes 1351 - 1360
@{
*/
/********************** SysAce statuses 1351 - 1360 **************************/

#define FPMST_SYSACE_NO_LOCK          1351L	/*!< No MPU lock has been granted */
/** @} */
/**
@name PCI Bridge Status Codes 1361 - 1375
@{
*/
/********************** PCI Bridge statuses 1361 - 1375 **********************/

#define FPMST_PCI_INVALID_ADDRESS     1361L
/** @} */
/**
@name FlexRay Constants 1400 - 1409
@{
*/
/********************** FlexRay constants 1400 - 1409 *************************/

#define FPMST_FR_TX_ERROR			1400
#define FPMST_FR_TX_BUSY			1401
#define FPMST_FR_BUF_LOCKED		1402
#define FPMST_FR_NO_BUF			1403
/** @} */
/**
@name USB constants 1410 - 1420
@{
*/
/****************** USB constants 1410 - 1420  *******************************/

#define FPMST_USB_ALREADY_CONFIGURED	1410
#define FPMST_USB_BUF_ALIGN_ERROR		1411
#define FPMST_USB_NO_DESC_AVAILABLE	1412
#define FPMST_USB_BUF_TOO_BIG		1413
#define FPMST_USB_NO_BUF			1414
/** @} */
/**
@name HWICAP constants 1421 - 1429
@{
*/
/****************** HWICAP constants 1421 - 1429  *****************************/

#define FPMST_HWICAP_WRITE_DONE		1421

/** @} */
/**
@name AXI VDMA constants 1430 - 1440
@{
*/
/****************** AXI VDMA constants 1430 - 1440  *****************************/

#define FPMST_VDMA_MISMATCH_ERROR		1430
/** @} */
/**
@name NAND Flash Status Codes 1441 - 1459
@{
*/
/*********************** NAND Flash statuses 1441 - 1459  *********************/

#define FPMST_NAND_BUSY			1441L	/*!< Flash is erasing or
						 * programming
						 */
#define FPMST_NAND_READY			1442L	/*!< Flash is ready for commands
						 */
#define FPMST_NAND_ERROR			1443L	/*!< Flash had detected an
						 * internal error.
						 */
#define FPMST_NAND_PART_NOT_SUPPORTED	1444L	/*!< Flash type not supported by
						 * driver
						 */
#define FPMST_NAND_OPT_NOT_SUPPORTED	1445L	/*!< Operation not supported
						 */
#define FPMST_NAND_TIMEOUT_ERROR		1446L	/*!< Programming or erase
						 * operation aborted due to a
						 * timeout
						 */
#define FPMST_NAND_ADDRESS_ERROR		1447L	/*!< Accessed flash outside its
						 * addressible range
						 */
#define FPMST_NAND_ALIGNMENT_ERROR	1448L	/*!< Write alignment error
						 */
#define FPMST_NAND_PARAM_PAGE_ERROR	1449L	/*!< Failed to read parameter
						 * page of the device
						 */
#define FPMST_NAND_CACHE_ERROR		1450L	/*!< Flash page buffer error
						 */

#define FPMST_NAND_WRITE_PROTECTED	1451L	/*!< Flash is write protected
						 */
/** @} */

/**************************** Type Definitions *******************************/

typedef s32 FPmStatus;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/**
* @} End of "addtogroup common_status_codes".
*/
