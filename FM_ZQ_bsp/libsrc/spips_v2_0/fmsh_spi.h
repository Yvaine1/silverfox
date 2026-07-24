/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fmsh_spi.h
 * @addtogroup spips_v2_0
 * @{
 *
 * This header file contains the identifiers and driver
 * functions (or macros) that can be used to access the device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00  hzq 7/1/19
 * 		     First release
 * 1.10  hzq 11/26/20
 * 		     Add macro of configuration used for initialization.
 *            Add type defination FSpiPs_Caps which is a struct of
 *            configuration.
 *            Add type defination FSpiPs_Dma which is a struct of
 *            dma related parameters.
 *            Modify FSpiPs_T to increase members.
 *            Add FSpiPs_Initialize function prototype.
 *            Add FSpiPs_InitHw function prototype.
 *            Add FSpiPs_Transfer function prototype.
 *            Add FSpiPs_PolledTransfer function prototype.
 * 1.12 hzq 2022/03/04
 * 			Add member in FSpiPs_T to support select decode
 * 2.00 hzq 2023/03/23
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef _FMSH_SPI_H_ /* prevent circular inclusions */
#define _FMSH_SPI_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/
// StatusEvent
#define SPI_TRANSFER_DONE      (0x80)
#define SPI_DMA_TRANSFER       (0x10000)

// property
#define SPI_MAX_SLAVE          (5)
#define SPI_BUSY_TIMEOUT       (1000000)

/***** usercfg flags *****/
#define SPI_F_MASTER           (0x1)
#define SPI_F_LPBK             (0x2)
#define SPI_F_INTR_EN          (0x4)
#define SPI_F_DMA_EN           (0x8)
#define SPI_F_GPIO_CS          (0x100)
#define SPI_F_DECODE_CS        (0x200)
#define SPI_F_USR_INTRHANDLE   (0x400)

#define SPI_USERCFG(name)      static FSpiPs_UserCfg_T spi_##name##_usercfg
#define GET_SPI_USERCFG(name)  &spi_##name##_usercfg

/***** msg flags *****/
#define SPI_F_USE_POLL         (0x1)
#define SPI_F_TX_ADDR_NOCHANGE (0x10)
#define SPI_F_RX_ADDR_NOCHANGE (0x20)

/**************************** Type Definitions *******************************/
typedef void (*FSpiPs_StatusHandler)(void* CallBackRef, u32 StatusEvent,
                                     u32 ByteCount);

typedef struct spi_usercfg FSpiPs_UserCfg_T;
typedef struct spi_msg FSpiPs_Msg_T;
typedef struct spi_dma FSpiPs_Dma_T;
typedef struct spi_config FSpiPs_Config_T;
typedef struct spi FSpiPs_T;

/**********************************Variable
 * Definition**************************/
struct spi_usercfg {
    u32 flags;
    u32 tx_empty_lvl;
    u32 rx_full_lvl;
    u32 cpol;
    u32 cpha;
    u32 frame_size;
    u32 frame_len;
    u32 baudrate;
    u32 sample_delay;
};

struct spi_msg {
    u32 flags;
    u8* tx_buf;
    u8* rx_buf;
    u32 tx_bytes;
    u32 rx_bytes;
    u32 total_bytes;
};

struct spi_dma {
    u8 type;
    u8 tx_if;
    u8 rx_if;
    u32 io;
};

struct spi_config {
    u16 device_id;
    intptr_t base;
};

struct spi {
    struct spi_config config;
    u32 version;

    struct spi_usercfg* usercfg;

    u8 is_busy;

    u8 cur_cs;
    u8 transfer_mode;

    struct spi_dma* dma;

    struct spi_msg* msg;
    int msg_num;
    int cur_msg;

    FSpiPs_StatusHandler status_handler;
    void* status_ref;

    void* priv;
};

/**********************************Function
 * Prototype***************************/
FSpiPs_Config_T* FSpiPs_LookupConfig(u16 device_id);

/*****************************************************************************
 * This function initializes a specific FSpiPs_T device/instance. This function
 * must be called prior to using the device to read or write any data.
 *
 * @param	spi is a pointer to the FSpiPs_T instance.
 * @param	configPtr points to the FSpiPs_T device configuration structure.
 *
 * @return
 *		- FMSH_SUCCESS if successful.
 *		- FMSH_FAILURE if fail.
 *
 * @note		The user needs to first call the FSpiPs_LookupConfig() API
 *		which returns the Configuration structure pointer which is
 *		passed as a parameter to the FSpiPs_CfgInitialize() API.
 *
 ******************************************************************************/
int FSpiPs_CfgInitialize(FSpiPs_T* spiPtr, FSpiPs_Config_T* configPtr);

/*****************************************************************************
 * This function resets spi device registers to default value.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FSpiPs_Reset(FSpiPs_T* spiPtr);

/*****************************************************************************
 * This function tests if spi device exists.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if spi device exists.
 *		- FMSH_FAILURE if spi device not exists.
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SelfTest(FSpiPs_T* spiPtr);

/*****************************************************************************
 * This function sets point to status handler as well as its callback parameter
 *.
 *
 * @param
 *
 * @return
 *
 * @note
 *       - this function is usually used called in interrupt
 *       - implemented by user
 *
 ******************************************************************************/
void FSpiPs_SetStatusHandler(FSpiPs_T* spiPtr, void* callBackRef,
                             FSpiPs_StatusHandler funcPtr);

/*****************************************************************************
 * The interrupt handler for SPI interrupts. This function must be connected
 * by the user to an interrupt source. This function does not save and restore
 * the processor context such that the user must provide this processing.
 *
 * The interrupts that are handled are:
 *
 * - Multi Master Fault. This interrupt is generated if both device set as
 *master and try to transfer with the same slave. The driver aborts this
 *transfer. The upper layer software is informed of the error.
 *
 * - Data Receive Register (FIFO) Overrun. This interrupt is generated when the
 *   SPI device attempts to write a received byte to an already full DRR/FIFO.
 *   A full DRR/FIFO usually means software is not emptying the data in a timely
 *   manner.  No action is taken by the driver other than to inform the upper
 *   layer software of the error.
 *
 * - Data Receive Register (FIFO) Underrun. This interrupt is generated when the
 *   SPI device attempts to read a received byte from an empty DRR/FIFO.
 *   A empty DRR/FIFO usually means software is not fill the data in a timely
 *   manner.  No action is taken by the driver other than to inform the upper
 *   layer software of the error.
 *
 * - Data Transmit Register (FIFO) Overrun. This interrupt is generated when
 *   the SPI device attempts to write data to an already full DTR/FIFO.
 *   An full DTR/FIFO usually means that software is not giving the
 *   device data in a timely manner. No action is taken by the driver other than
 *   to inform the upper layer software of the error.
 *
 * - Data Transmit Register (FIFO) Empty. This interrupt is generated when the
 *   transmit register or FIFO is empty. The driver uses this interrupt during a
 *   transmission to continually send/receive data until there is no more data
 *   to send/receive.
 *
 * - Data Receive Register (FIFO) Full. This interrupt is generated when the
 *   receive register or FIFO is full. The driver uses this interrupt during a
 *   transmission, used as slave, to continually send/receive data until
 *   there is no more data to send/receive.
 *
 * @param	InstancePtr is a pointer to the FSpiPs_T instance to be worked on.
 *
 * @return	None.
 *
 * @note
 *
 * The slave select register is being set to deselect the slave when a transfer
 * is complete.  This is being done regardless of whether it is a slave or a
 * master since the hardware does not drive the slave select as a slave.
 *
 ******************************************************************************/
void FSpiPs_InterruptHandler(void* instancePtr);

/*****************************************************************************
 * This function initializes controller struct
 *
 * @param
 *       - DeviceId contains the ID of the device
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_Initialize(FSpiPs_T* spiPtr, u16 device_id);

/*****************************************************************************
 * This function initializes controller hardware
 *
 * @param
 *       - capsPtr point to FSpiPs_Caps
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_HwInit(FSpiPs_T* spiPtr, FSpiPs_UserCfg_T* usercfg);
int FSpiPs_SlaveManually(FSpiPs_T* spiPtr, int select_n);

/*****************************************************************************
 * This function transfer messger
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_TransferMsg(FSpiPs_T* spiPtr, FSpiPs_Msg_T* msg, int msg_num);

/*****************************************************************************
 * This function transfers data
 *
 * @param
 *       - sendBuffer is a point to write data
 *       - recvBuffer is a point to read data
 *       - byteCount is a number of bytes to transfer
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_Transfer(FSpiPs_T* spiPtr, void* send_buf, void* recv_buf, int len);

/*****************************************************************************
 * This function transfers data with polled
 *
 * @param
 *       - sendBuffer is a point to write data
 *       - recvBuffer is a point to read data
 *       - byteCount is a number of bytes to transfer
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_PolledTransfer(FSpiPs_T* spiPtr, void* send_buf, void* recv_buf,
                          u32 len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
