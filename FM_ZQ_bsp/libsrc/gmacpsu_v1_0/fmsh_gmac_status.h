/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_status.h
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

/*********************** Return Code Definitions ****************************/
#define GMAC_RETURN_CODE_OK              0
#define GMAC_RETURN_CODE_ERR             1
#define GMAC_RETURN_CODE_TIME_OUT        2
#define GMAC_RETURN_CODE_PARAM_ERR       3 /* Parameter error */
#define GMAC_RETURN_CODE_TX_BUSY         4
#define GMAC_RETURN_CODE_RX_NULL         5 /* No data received */
#define GMAC_RETURN_CODE_SIZE_TOO_SMALL  6
#define GMAC_RETURN_CODE_NOT_INITIALIZED 7
#define GMAC_RETURN_CODE_SIZE_TOO_BIG    8

#define FGMACPS_DEVICE_NOT_FOUND         2L
#define FGMACPS_DEVICE_BLOCK_NOT_FOUND   3L
#define FGMACPS_INVALID_VERSION          4L
#define FGMACPS_DEVICE_IS_STARTED        5L
#define FGMACPS_DEVICE_IS_STOPPED        6L
#define FGMACPS_FIFO_ERROR                                                \
    7L                                   /*!< An error occurred during an \
                    operation with a FIFO such as           \
                    an underrun or overrun, this            \
                    error requires the device to            \
                    be reset */
#define FGMACPS_RESET_ERROR                                                    \
    8L                                   /*!< An error occurred which requires \
                    the device to be reset */
#define FGMACPS_DMA_ERROR                                                      \
    9L                                   /*!< A DMA error occurred, this error \
                    typically requires the device                \
                    using the DMA to be reset */
#define FGMACPS_NOT_POLLED                                                     \
    10L                                  /*!< The device is not configured for \
                    polled mode operation */
#define FGMACPS_FIFO_NO_ROOM                                                  \
    11L                                  /*!< A FIFO did not have room to put \
                    the specified data into */
#define FGMACPS_BUFFER_TOO_SMALL                                             \
    12L                                  /*!< The buffer is not large enough \
                    to hold the expected data */
#define FGMACPS_NO_DATA 13L              /*!< There was no data available */
#define FGMACPS_REGISTER_ERROR                                               \
    14L                                  /*!< A register did not contain the \
                    expected value */
#define FGMACPS_INVALID_PARAM                                                 \
    15L                                  /*!< An invalid parameter was passed \
                    into the function */
#define FGMACPS_NOT_SGDMA                                                      \
    16L                                  /*!< The device is not configured for \
                    scatter-gather DMA operation */
#define FGMACPS_LOOPBACK_ERROR 17L       /*!< A loopback test failed */
#define FGMACPS_NO_CALLBACK                                               \
    18L                                  /*!< A callback has not yet been \
                    registered */
#define FGMACPS_NO_FEATURE                                                  \
    19L                                  /*!< Device is not configured with \
                    the requested feature */
#define FGMACPS_NOT_INTERRUPT                                              \
    20L                                  /*!< Device is not configured for \
                    interrupt mode operation */
#define FGMACPS_DEVICE_BUSY 21L          /*!< Device is busy */
#define FGMACPS_ERROR_COUNT_MAX                                              \
    22L                                  /*!< The error counters of a device \
                    have maxed out */
#define FGMACPS_IS_STARTED                                                \
    23L                                  /*!< Used when part of device is \
                    already started i.e.                     \
                    sub channel */
#define FGMACPS_IS_STOPPED                                                \
    24L                                  /*!< Used when part of device is \
                    already stopped i.e.                     \
                    sub channel */
#define FGMACPS_DATA_LOST  26L           /*!< Driver defined error */
#define FGMACPS_RECV_ERROR 27L           /*!< Generic receive error */
#define FGMACPS_SEND_ERROR 28L           /*!< Generic transmit error */
#define FGMACPS_NOT_ENABLED                                              \
    29L                                  /*!< A requested service is not \
                    available because it has not            \
                    been enabled */

#define FGMACPS_DMA_TRANSFER_ERROR                                    \
    511L                                 /*!< Self test, DMA transfer \
                failed */
#define FGMACPS_DMA_RESET_REGISTER_ERROR                                  \
    512L                                 /*!< Self test, a register value \
                was invalid after reset */
#define FGMACPS_DMA_SG_LIST_EMPTY                                          \
    513L                                 /*!< Scatter gather list contains \
                no buffer descriptors ready                \
                to be processed */
#define FGMACPS_DMA_SG_IS_STARTED 514L   /*!< Scatter gather not stopped */
#define FGMACPS_DMA_SG_IS_STOPPED 515L   /*!< Scatter gather not running */
#define FGMACPS_DMA_SG_LIST_FULL                                           \
    517L                                 /*!< All the buffer desciptors of \
                the scatter gather list are                \
                being used */
#define FGMACPS_DMA_SG_BD_LOCKED                                        \
    518L                                 /*!< The scatter gather buffer \
                descriptor which is to be               \
                copied over in the scatter              \
                list is locked */
#define FGMACPS_DMA_SG_NOTHING_TO_COMMIT                                      \
    519L                                 /*!< No buffer descriptors have been \
                put into the scatter gather                   \
                list to be commited */
#define FGMACPS_DMA_SG_COUNT_EXCEEDED                                    \
    521L                                 /*!< The packet count threshold \
                specified was larger than the            \
                total # of buffer descriptors            \
                in the scatter gather list */
#define FGMACPS_DMA_SG_LIST_EXISTS                                        \
    522L                                 /*!< The scatter gather list has \
                already been created */
#define FGMACPS_DMA_SG_NO_LIST                                           \
    523L                                 /*!< No scatter gather list has \
                been created */
#define FGMACPS_DMA_SG_BD_NOT_COMMITTED                                       \
    524L                                 /*!< The buffer descriptor which was \
                being started was not committed               \
                to the list */
#define FGMACPS_DMA_SG_NO_DATA                                               \
    525L                                 /*!< The buffer descriptor to start \
                has already been used by the                 \
                hardware so it can't be reused               \
              */
#define FGMACPS_DMA_SG_LIST_ERROR                                         \
    526L                                 /*!< General purpose list access \
                error */
#define FGMACPS_DMA_BD_ERROR                                            \
    527L                                 /*!< General buffer descriptor \
                error */
#define FGMACPS_MEMORY_SIZE_ERROR                                            \
    1001L                                /*!< Memory space is not big enough \
                                          * to hold the minimum number of    \
                                          * buffers or descriptors */
#define FGMACPS_MEMORY_ALLOC_ERROR 1002L /*!< Memory allocation failed */
#define FGMACPS_MII_READ_ERROR     1003L /*!< MII read error */
#define FGMACPS_MII_BUSY           1004L /*!< An MII operation is in progress */
#define FGMACPS_OUT_OF_BUFFERS     1005L /*!< Driver is out of buffers */
#define FGMACPS_PARSE_ERROR        1006L /*!< Invalid driver init string */
#define FGMACPS_COLLISION_ERROR                                       \
    1007L                                /*!< Excess deferral or late \
                                          * collision on polled send */
typedef s32 FStatus;