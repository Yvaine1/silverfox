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
 * @file fmsh_spi_hw.h
 * @addtogroup spips_v2_0
 * @{
 *
 * This header file contains the identifiers and basic HW access driver
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
 * 		     Add defination of SPIPS_CTRL0_MASK.
 * 		     Add defination of SPIPS_CTRL0_SCPOL_SHIFT.
 * 		     Add defination of SPIPS_CTRL0_SLVOE_SHIFT.
 * 		     Add defination of SPIPS_CTRL0_SRL_SHIFT.
 * 2.00 hzq 2023/03/23
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef _FMSH_SPI_HW_H_ /* prevent circular inclusions */
#define _FMSH_SPI_HW_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define SPI_MSTR_OFFSET         (0x100)
#define SPI_CTRLR0_OFFSET       (0x00)
#define SPI_CTRLR1_OFFSET       (0x04)
#define SPI_SSIENR_OFFSET       (0x08)
#define SPI_MVCR_OFFSET         (0x0C)
#define SPI_SER_OFFSET          (0x10)
#define SPI_BAUDR_OFFSET        (0x14)
#define SPI_TXFTLR_OFFSET       (0x18)
#define SPI_RXFTLR_OFFSET       (0x1C)
#define SPI_TXFLR_OFFSET        (0x20)
#define SPI_RXFLR_OFFSET        (0x24)
#define SPI_SR_OFFSET           (0x28)
#define SPI_IMR_OFFSET          (0x2C)
#define SPI_ISR_OFFSET          (0x30)
#define SPI_RISR_OFFSET         (0x34)
#define SPI_TXOICR_OFFSET       (0x38)
#define SPI_RXOICR_OFFSET       (0x3C)
#define SPI_RXUICR_OFFSET       (0x40)
#define SPI_MSTICR_OFFSET       (0x44)
#define SPI_ICR_OFFSET          (0x48)
#define SPI_DMACR_OFFSET        (0x4C)
#define SPI_DMATDLR_OFFSET      (0x50)
#define SPI_DMARDLR_OFFSET      (0x54)
#define SPI_IDR_OFFSET          (0x58)
#define SPI_VERSION_OFFSET      (0x5C)
#define SPI_DR_OFFSET           (0x60)
#define SPI_RX_SAMPLE_OFFSET    (0xf0)
#define SPI_SCTRLR0_OFFSET      (0xf4)
#define SPI_RSVD1_OFFSET        (0xf8)
#define SPI_RSVD2_OFFSET        (0xfc)

/***** CTRL0 *****/
#define SPI_CTRL0_MASK          (0x1f0fc0)

#define SPI_CTRL0_SCPH_MASK     (0x1 << 6)
#define SPI_CTRL0_SCPOL_MASK    (0x1 << 7)
#define SPI_CTRL0_TMOD_MASK     (0x3 << 8)
#define SPI_CTRL0_SLVOE_MASK    (0x1 << 10)
#define SPI_CTRL0_SRL_MASK      (0x1 << 11)
#define SPI_CTRL0_DFS32_MASK    (0x1f << 16)

#define SPI_CTRL0_SCPH_SHIFT    (6)
#define SPI_CTRL0_SCPOL_SHIFT   (7)
#define SPI_CTRL0_TMOD_SHIFT    (8)
#define SPI_CTRL0_SLVOE_SHIFT   (10)
#define SPI_CTRL0_SRL_SHIFT     (11)
#define SPI_CTRL0_DFS32_SHIFT   (16)

#define SPI_TRANSFER_STATE      (0x0)
#define SPI_TRANSMIT_ONLY_STATE (0x1)
#define SPI_RECEIVE_ONLY_STATE  (0x2)
#define SPI_EEPROM_STATE        (0x3)

/***** Status & Interrupt *****/
#define SPI_SR_DCOL             (0x40)
#define SPI_SR_TXE              (0x20)
#define SPI_SR_RFF              (0x10)
#define SPI_SR_RFNE             (0x08)
#define SPI_SR_TFE              (0x04)
#define SPI_SR_TFNF             (0x02)
#define SPI_SR_BUSY             (0x01)

#define SPI_INTR_MSTIS_MASK     (0x20)
#define SPI_INTR_RXFIS_MASK     (0x10)
#define SPI_INTR_RXOIS_MASK     (0x08)
#define SPI_INTR_RXUIS_MASK     (0x04)
#define SPI_INTR_TXOIS_MASK     (0x02)
#define SPI_INTR_TXEIS_MASK     (0x01)
#define SPI_INTR_ALL            (0x3f)

#define SPI_FIFO_DEPTH          (0x20)

/**************************** Type Definitions *******************************/

/**********************************Variable
 * Definition**************************/

/**********************************Function
 * Prototype***************************/
/*****************************************************************************
 * This function sets spi device work in master/slave
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FSpiPs_SetMst(FSpiPs_T* spiPtr, int master);

/*****************************************************************************
 * This function enables/disables spi device
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FSpiPs_SetEnable(FSpiPs_T* spiPtr, int enable);

/*****************************************************************************
 * This function sets spi device CPOL/CPHA with 4 clock modes. It can only be
 *set when device is disabled.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS.
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetSckMode(FSpiPs_T* spiPtr, u32 mode);

/*****************************************************************************
 * This function sets spi device transfer modes. It can only be set
 * when device is disabled. tmod 0 is transfer, tmod 1 is transmit only
 * tmod 2 is receive only, tmod 3 is eeprom mode.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS.
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetTMod(FSpiPs_T* spiPtr, u32 tmod);

/*****************************************************************************
 * This function sets spi slvout. It can only be set when device is disabled.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetSlvOut(FSpiPs_T* spiPtr, int enable);

/*****************************************************************************
 * This function sets slave select. it can only be set when device works in
 *master
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetLoopBack(FSpiPs_T* spiPtr, int enable);

/*****************************************************************************
 * This function sets data frame size. it can only be set when device works in
 *master
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetDFS32(FSpiPs_T* spiPtr, int dfs32);

/*****************************************************************************
 * This function sets data frame number. it can only be set when device works in
 *master
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetDFNum(FSpiPs_T* spiPtr, int dfnum);

/*****************************************************************************
 * This function sets slave select. it can only be set when device works in
 *master
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetSlave(FSpiPs_T* spiPtr, u32 cs);

/*****************************************************************************
 * This function sets baudrate. it can only be set when device works in master
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetSckDv(FSpiPs_T* spiPtr, int sckdv);

/*****************************************************************************
 * This function sets fifo level. it can only be set when device is disabled
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSpiPs_SetTxEmptyLvl(FSpiPs_T* spiPtr, int tlvl);
u32 FSpiPs_GetTxLevel(FSpiPs_T* spiPtr);
int FSpiPs_SetRxFullLvl(FSpiPs_T* spiPtr, int tlvl);
u32 FSpiPs_GetRxLevel(FSpiPs_T* spiPtr);

/*****************************************************************************
 * This function enables/disables interrupt mask
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FSpiPs_EnableIntr(FSpiPs_T* spiPtr, u32 mask);
void FSpiPs_DisableIntr(FSpiPs_T* spiPtr, u32 mask);
void FSpiPs_ClearIntrStatus(FSpiPs_T* spiPtr);

/*****************************************************************************
 * This function sets DMA trigger level
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FSpiPs_SetDMATLvl(FSpiPs_T* spiPtr, int tlvl);
void FSpiPs_SetDMARLvl(FSpiPs_T* spiPtr, int tlvl);
void FSpiPs_EnableDMATx(FSpiPs_T* spiPtr);
void FSpiPs_EnableDMARx(FSpiPs_T* spiPtr);
void FSpiPs_DisableDMA(FSpiPs_T* spiPtr);

/*****************************************************************************
 * This function gets status register value.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
u32 FSpiPs_GetStatus(FSpiPs_T* spiPtr);

/*****************************************************************************
 * This function receives data from register(FIFO) if it is not empty. exit if
 * timeover
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
u32 FSpiPs_Recv(FSpiPs_T* spiPtr);

/*****************************************************************************
 * This function sends data to register(FIFO) if it is not full. exit if
 * timeover
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FSpiPs_Send(FSpiPs_T* spiPtr, u32 data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
