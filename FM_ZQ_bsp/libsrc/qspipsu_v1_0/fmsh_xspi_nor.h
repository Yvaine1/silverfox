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
 * @file fmsh_xqspi_nor.h
 * @addtogroup qspipsu_v1_0
 * @{
 *
 * This header file contains the functions (or macros) that can be used on
 * spi-nor device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2022/12/22  First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_XSPI_NOR_H_ /* prevent circular inclusions */
#define _FMSH_XSPI_NOR_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_xspi.h"

/************************** Constant Definitions *****************************/
/***** qspi nor id(size) *****/
#define QSPINOR_SIZE_ID_32KB        (0x09)
#define QSPINOR_SIZE_ID_64KB        (0x10)
#define QSPINOR_SIZE_ID_128KB       (0x11)
#define QSPINOR_SIZE_ID_256KB       (0x12)
#define QSPINOR_SIZE_ID_512KB       (0x13)
#define QSPINOR_SIZE_ID_1MB         (0x14)
#define QSPINOR_SIZE_ID_2MB         (0x15)
#define QSPINOR_SIZE_ID_4MB         (0x16)
#define QSPINOR_SIZE_ID_8MB         (0x17)
#define QSPINOR_SIZE_ID_16MB        (0x18)
#define QSPINOR_SIZE_ID_32MB        (0x19)
#define QSPINOR_SIZE_ID_64MB        (0x20)
#define QSPINOR_SIZE_ID_64MB_TYPE2  (0x1A)
#define QSPINOR_SIZE_ID_128MB       (0x21)
#define QSPINOR_SIZE_ID_128MB_TYPE2 (0x1B)
#define QSPINOR_SIZE_ID_256MB       (0x22)
#define QSPINOR_SIZE_ID_256MB_TYPE2 (0x1C)

/***** qspi nor cmd & reg definition *****/
#define QSPINOR_CMD_RDID            (0x9F)
#define QSPINOR_CMD_WREN            (0x06)
#define QSPINOR_CMD_WRDI            (0x04)
#define QSPINOR_CMD_RDSR1           (0x05)
#define QSPINOR_SR1_SRWD            (0x80)  // ignore WRR command when WP# is low
#define QSPINOR_SR1_WEL             (0x02)
#define QSPINOR_SR1_BUSY            (0x01)

#define QSPINOR_CMD_WRR             (0x01)
#define QSPINOR_CMD_READ            (0x03)
#define QSPINOR_CMD_FR              (0x0B)
#define QSPINOR_CMD_DOR             (0x3B)
#define QSPINOR_CMD_QOR             (0x6B)
#define QSPINOR_CMD_DIOR            (0xBB)
#define QSPINOR_CMD_QIOR            (0xEB)
#define QSPINOR_CMD_SE              (0xD8)
#define QSPINOR_CMD_PP              (0x02)
#define QSPINOR_CMD_4SE             (0xC7)
// spansion
#define QSPINOR_SPANSION_SR1_P_ERR  (0x40)
#define QSPINOR_SPANSION_SR1_E_ERR  (0x20)
#define QSPINOR_SPANSION_SR1_BP     (0x1C)

#define QSPINOR_CMD_SPANSION_RDSR2  (0x07)  // Read Status Rregister-2
#define QSPINOR_CMD_SPANSION_RDCR   (0x35)  // Read Configuration Rregister
#define QSPINOR_SPANSION_CR_LC      (0xC0)
#define QSPINOR_SPANSION_CR_TBPROT  (0x20)
#define QSPINOR_SPANSION_CR_BPNV    (0x08)
#define QSPINOR_SPANSION_CR_TBPARM  (0x04)
#define QSPINOR_SPANSION_CR_QE      (0x02)
#define QSPINOR_SPANSION_CR_FREEZE  (0x01)

#define QSPINOR_CMD_SPANSION_BRRD   (0x16)  // Read Bank Rregister
#define QSPINOR_CMD_SPANSION_BRWR   (0x17)  // Write Bank Rregister
#define QSPINOR_SPANSION_BR_EXTADDR (0x80)

// micron
#define QSPINOR_MICRON_SR1_BP       (0x5C)

// Read Nonvolatile Configuration Rregister
#define QSPINOR_CMD_MICRON_RDNVCR   (0xB5)
// Write Nonvolatile Configuration Rregister
#define QSPINOR_CMD_MICRON_WRNVCR   (0xB1)
// Read Volatile Configuration Rregister
#define QSPINOR_CMD_MICRON_RDVCR    (0x85)
// Write Volatile Configuration Rregister
#define QSPINOR_CMD_MICRON_WRVCR    (0x81)
// Read Enhanced Volatile Configuration Rregister
#define QSPINOR_CMD_MICRON_RDECR    (0x65)
// Read Enhanced Address Rregister
#define QSPINOR_CMD_MICRON_RDEAR    (0xC8)
// Write Enhanced Volatile Configuration Rregister
#define QSPINOR_CMD_MICRON_WRECR    (0x61)
// Write Enhanced Address Rregister
#define QSPINOR_CMD_MICRON_WREAR    (0xC5)

// winbond
// Read Status Rregister-2
#define QSPINOR_CMD_WINBOND_RDSR2   (0x35)
// Write Status Rregister-2
#define QSPINOR_CMD_WINBOND_WRSR2   (0x31)
#define QSPINOR_WINBOND_SR2_QE      (0x02)
// Read Status Rregister-3
#define QSPINOR_CMD_WINBOND_RDSR3   (0x15)
// Write Status Rregister-3
#define QSPINOR_CMD_WINBOND_WRSR3   (0x11)
// Read Extended Address Rregister
#define QSPINOR_CMD_WINBOND_RDEAR   (0xC8)
// Write Extended Address Rregister
#define QSPINOR_CMD_WINBOND_WREAR   (0xC5)

// macronix
#define QSPINOR_MACRONIX_SR1_QE     (0x40)
#define QSPINOR_MACRONIX_SR1_BP     (0x3C)
// Read Configuration Rregister
#define QSPINOR_CMD_MACRONIX_RDCR   (0x15)
// Read Extended Address Rregister
#define QSPINOR_CMD_MACRONIX_RDEAR  (0xC8)
// Write Extended Address Rregister
#define QSPINOR_CMD_MACRONIX_WREAR  (0xC5)
// Enter 4B Mode
#define QSPINOR_CMD_MACRONIX_EN4B   (0xB7)
// Exit 4B Mode
#define QSPINOR_CMD_MACRONIX_EX4B   (0xE9)

// issi
#define QSPINOR_ISSI_SR1_QE         (0x40)
#define QSPINOR_ISSI_SR1_BP         (0x3C)

// Read Read Parameter(Volatile) Rregister
#define QSPINOR_CMD_ISSI_RDPAR      (0x61)
// Set Read Parameter(Volatile) Rregister
#define QSPINOR_CMD_ISSI_SPRV       (0xC0)
// Set Read Parameter(Non-Volatile) Rregister
#define QSPINOR_CMD_ISSI_SPRNV      (0x65)
// Read Extended Read Parameter(Volatile) Rregister
#define QSPINOR_CMD_ISSI_RDWRP      (0x81)
// Set Extended Read Parameter(Volatile) Rregister
#define QSPINOR_CMD_ISSI_SEPRV      (0x83)
// Set Extended Read Parameter(Non-Volatile) Rregister
#define QSPINOR_CMD_ISSI_SEPRNV     (0x85)
// Read Bank Address Rregister
#define QSPINOR_CMD_ISSI_RDBR       (0x16)
// Write Bank Address(Volatile) Rregister
#define QSPINOR_CMD_ISSI_WRBRV      (0x17)
// Write Bank Address(Non-Volatile) Rregister
#define QSPINOR_CMD_ISSI_WRBRNV     (0x18)
// Enter 4B Mode
#define QSPINOR_CMD_ISSI_EN4B       (0xB7)
// Exit 4B Mode
#define QSPINOR_CMD_ISSI_EX4B       (0x29)

// fmsh
// Read Status Rregister-2
#define QSPINOR_CMD_FMSH_RDSR2      (0x35)
#define QSPINOR_FMSH_SR2_QE         (0x02)

// gd
#define QSPINOR_GD_SR2_QE           (0x02)

/***** qspi timings *****/
#define QSPINOR_CSDA_NS             (40)
#define QSPINOR_CSEOT_NS            (0)
#define QSPINOR_CSSOT_NS            (0)

/***** qspi operation timeout *****/
#define QSPINOR_TIMING_TRST_MS      (1)
#define QSPINOR_TIMING_TW_MS        (2000)
#define QSPINOR_TIMING_TPP_US       (5000)
#define QSPINOR_TIMING_TSE_MS       (3000)
#define QSPINOR_TIMING_TBE_S        (600)

/***** qspi nor device flags *****/
#define QSPINOR_F_FSR               0x1
#define QSPINOR_F_JFM_128           0x2

/************* Macros (Inline Functions) Definitions *****************/

/************* Variable Definitions *****************************/
struct qspi_nor_param {
    u8 csda_ns;
    u8 cseot_ns;
    u8 cssot_ns;

    u16 trst_max_ms;
    u16 tw_max_ms;
    u16 tpp_max_us;
    u16 tse_max_ms;
    u16 tbe_max_s;
};

struct qspi_nor {
    u32 flags;
    int blk_shift;
    struct qspi_nor_param* param;
};

/************************** Function Prototypes ******************************/
/*****************************************************************************
 * detect nor flash
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Init(FQspiPsu_T* qspiPtr, struct qspi_usercfg* usercfg);

/*****************************************************************************
 * This function chcek flash id & fill nor device struct
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_GetFlashInfo(FQspiPsu_T* qspiPtr, u8* id);

/*****************************************************************************
 * This function set controller mode for read/write/erase...(x1, x2, x4)
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_SetFlashMode(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function set controller mode for read(x1, x2, x4)
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_ChangeReadMode(FQspiPsu_T* qspiPtr, int read_mode);

/*****************************************************************************
 * This function executes READ ID.
 *
 * @param
 *
 * @return
 *		- ID value.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_ReadId(FQspiPsu_T* qspiPtr, void* id);

/*****************************************************************************
 * This function executes CHIP ERASE.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_ChipErase(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function erase flash
 *
 * @param
 *       - offset is a value where sector erase start from
 *       - byteCount is a number of bytes to sector erase
 *       - blockSize is a value of flash sector bytes
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Erase(FQspiPsu_T* qspiPtr, u32 offs, u32 len);

/*****************************************************************************
 * This function write flash using direct mode
 *
 * @param
 *       - offset is a value where data write
 *       - byteCount is a number of bytes to write
 *       - sendBuffer is a point to write data
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Write(FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* sendBuf);

/*****************************************************************************
 * This function read flash using direct mode
 *
 * @param
 *       - offset is a value where data read
 *       - byteCount is a number of bytes to read
 *       - recvBuffer is a point to read data
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Read(FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* recvBuf);

/*****************************************************************************
 * This function write flash using acmd mode
 *
 * @param
 *       - offset is a value where data write
 *       - byteCount is a number of bytes to write
 *       - sendBuffer is a point to write data
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_FastWrite(FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* sendBuf);

/*****************************************************************************
 * This function read flash using acmd mode
 *
 * @param
 *       - offset is a value where data read
 *       - byteCount is a number of bytes to read
 *       - recvBuffer is a point to read data
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_FastRead(FQspiPsu_T* qspiPtr, u32 offs, u32 len, u8* recvBuf);

/*****************************************************************************
 * This function reset flash
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Reset(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function get flash qe status
 *
 * @param
 *
 * @return
 *		- 1 if qe .
 *		- 0 if not qe.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_IsFlashQuad(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function enable flash quad io
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_EnableQuad(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function lock flash protect
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Lock(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function unlock flash protect
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Unlock(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function check bank regiser
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_GetBankReg(FQspiPsu_T* qspiPtr, u8* value);

/*****************************************************************************
 * This function set flash segment
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_SetSegment(FQspiPsu_T* qspiPtr, u8 high_addr);

/*****************************************************************************
 * This function enter flash into 4B mode
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Enter4B(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function exit flash from 4B mode
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_Exit4B(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function enter flash into XIP mode
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_EnterXIP(FQspiPsu_T* qspiPtr, int mode);
int FQspiPsu_Nor_ExitXIP(FQspiPsu_T* qspiPtr, int mode);

/*****************************************************************************
 * This function executes READ STATUS1 and wait for WIP.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if overtime.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_WaitForWIP(FQspiPsu_T* qspiPtr, int timeout_us);

/*****************************************************************************
 * This function executes READ STATUS1 and wait for WIP.
 * This function executes READ Flag Status if needed.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if overtime.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_WaitForReady(FQspiPsu_T* qspiPtr, int timeout_us);

/*****************************************************************************
 * This function executes WREN.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_WREN(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function executes WRDI.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_WRDI(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function executes READ STATUS1.
 *
 * @param
 *
 * @return
 *		- return 1 if failed.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_GetStatus1(FQspiPsu_T* qspiPtr, u8* status);

/*****************************************************************************
 * This function executes get flag status register for micron device.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_GetFlagStatus(FQspiPsu_T* qspiPtr, u8* status);

/*****************************************************************************
 * This function executes clear flag status register for micron device.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_ClearFlagStatus(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function executes WRITE 16 bits register.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_SetNVReg(FQspiPsu_T* qspiPtr, u8 opcode, u8* value, u8 len);
int FQspiPsu_Nor_SetReg(FQspiPsu_T* qspiPtr, u8 opcode, u8* value, u8 len);

/*****************************************************************************
 * This function executes READ 16 bits register.
 *
 * @param
 *
 * @return
 *		- Register value.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_GetReg(FQspiPsu_T* qspiPtr, u8 opcode, u8* value, u8 len);

/*****************************************************************************
 * This function executes flash cmd without data attached.
 *
 * @param
 *
 * @return
 *		- Register value.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nor_ExecCmd(FQspiPsu_T* qspiPtr, u8 opcode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
