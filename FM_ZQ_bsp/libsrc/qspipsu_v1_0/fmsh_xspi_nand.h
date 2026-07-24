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
 * @file fmsh_xqspi_nand.h
 * @addtogroup qspipsu_v1_0
 * @{
 *
 * This header file contains the functions (or macros) that can be used on
 * spi-nand device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2024/2/22  First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_XSPI_NAND_H_ /* prevent circular inclusions */
#define _FMSH_XSPI_NAND_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_xspi.h"

/************************** Constant Definitions *****************************/
/***** qspi nand id(size) *****/
#define QSPINAND_SIZE_ID_128MB       (0x21)

/***** qspi nand cmd & reg definition *****/
#define QSPINAND_CMD_RDID            (0x9F)
#define QSPINAND_CMD_WREN            (0x06)
#define QSPINAND_CMD_WRDI            (0x04)
#define QSPINAND_CMD_GETFEAT         (0x0F)
#define QSPINAND_CMD_SETFEAT         (0x1F)

#define QSPINAND_CMD_READ_CACHE      (0x03)
#define QSPINAND_CMD_READ            (0x13)
#define QSPINAND_CMD_DOR             (0x3B)
#define QSPINAND_CMD_QOR             (0x6B)
#define QSPINAND_CMD_DIOR            (0xBB)
#define QSPINAND_CMD_QIOR            (0xEB)
#define QSPINAND_CMD_BE              (0xD8)
#define QSPINAND_CMD_PP              (0x02)
#define QSPINAND_CMD_PROGRAM_EXECUTE (0x10)

/***** qspi timings *****/
#define QSPINAND_CSDA_NS             (40)
#define QSPINAND_CSEOT_NS            (0)
#define QSPINAND_CSSOT_NS            (0)

/***** qspi operation timeout *****/
#define QSPINAND_TIMING_TRST_MS      (1)
#define QSPINAND_TIMING_TPP_US       (3000)
#define QSPINAND_TIMING_TSE_MS       (30)

/***** qspi nand device flags *****/

#define QSPINAND_PROT                (0xA0)
#define QSPINAND_PROT_CMP            (0x2)
#define QSPINAND_PROT_INV            (0x4)
#define QSPINAND_PROT_BP0            (0x8)
#define QSPINAND_PROT_BP1            (0x10)
#define QSPINAND_PROT_BP2            (0x20)
#define QSPINAND_PROT_BRWD           (0x80)

#define QSPINAND_FEAT                (0xB0)
#define QSPINAND_FEAT_QE             (0x0)

#define QSPINAND_SR                  (0xC0)
#define QSPINAND_SR_OIP              (0x0)
#define QSPINAND_SR_WEL              (0x1)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
struct qspi_nand_param {
    u8 csda_ns;
    u8 cseot_ns;
    u8 cssot_ns;

    u16 trst_max_ms;
    u16 tpp_max_us;
    u16 tse_max_ms;
};

struct qspi_nand {
    u32 flags;
    int blk_shift;
    int page_shift;
    int oob_size;
    struct qspi_nand_param* param;
};

/************************** Function Prototypes ******************************/
/*****************************************************************************
 * detect nand flash
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
int FQspiPsu_Nand_Init(FQspiPsu_T* qspiPtr, struct qspi_usercfg* usercfg);

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
int FQspiPsu_Nand_GetFlashInfo(FQspiPsu_T* qspiPtr, u8* id);

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
int FQspiPsu_Nand_SetFlashMode(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * These function set controller mode for read/write/erase...(x1, x2, x4)
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
int FQspiPsu_Nand_ChangeReadMode(FQspiPsu_T* qspiPtr, int read_mode);

int FQspiPsu_Nand_ChangeProgMode(FQspiPsu_T* qspiPtr, int prog_mode);

int FQspiPsu_Nand_ChangeErsMode(FQspiPsu_T* qspiPtr);

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
int FQspiPsu_Nand_ReadId(FQspiPsu_T* qspiPtr, void* id);

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
int FQspiPsu_Nand_Erase(FQspiPsu_T* qspiPtr, u32 offs, u32 len);

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
int FQspiPsu_Nand_DirectWrite(FQspiPsu_T* qspiPtr, u32 offs, u32 len,
                              u8* sendBuf);

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
int FQspiPsu_Nand_DirectRead(FQspiPsu_T* qspiPtr, u32 offs, u32 len,
                             u8* recvBuf);

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
int FQspiPsu_Nand_Write(FQspiPsu_T* qspiPtr, u32 ra, u32 ca, u32 len,
                        u8* sendBuf);

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
int FQspiPsu_Nand_Read(FQspiPsu_T* qspiPtr, u32 ra, u32 ca, u32 len,
                       u8* recvBuf);

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
int FQspiPsu_Nand_Reset(FQspiPsu_T* qspiPtr);

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
int FQspiPsu_Nand_IsFlashQuad(FQspiPsu_T* qspiPtr);

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
int FQspiPsu_Nand_EnableQuad(FQspiPsu_T* qspiPtr);

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
int FQspiPsu_Nand_Lock(FQspiPsu_T* qspiPtr);

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
int FQspiPsu_Nand_Unlock(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function executes READ FEATURE and wait for OIP.
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
int FQspiPsu_Nand_WaitForOIP(FQspiPsu_T* qspiPtr, int timeout_us);

/*****************************************************************************
 * This function executes READ FEATURE and wait for OIP.
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
int FQspiPsu_Nand_WaitForReady(FQspiPsu_T* qspiPtr, int timeout_us);

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
int FQspiPsu_Nand_WREN(FQspiPsu_T* qspiPtr);

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
int FQspiPsu_Nand_WRDI(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * This function executes WRITE FEATURE register.
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
int FQspiPsu_Nand_SetFeature(FQspiPsu_T* qspiPtr, u8 addr, u8 value);

/*****************************************************************************
 * This function executes READ FEATURE register.
 *
 * @param
 *
 * @return
 *		- Register value.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Nand_GetFeature(FQspiPsu_T* qspiPtr, u8 addr, u8* value);

/*****************************************************************************
 * This function executes SPI NAND Read From Cache.
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
int FQspiPsu_Nand_RD_CACHE(FQspiPsu_T* qspiPtr, u32 addr, u32 len, u8* value);

/*****************************************************************************
 * This function executes SPI NAND Page Read.
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
int FQspiPsu_Nand_PAGE_RD(FQspiPsu_T* qspiPtr, u32 addr);

/*****************************************************************************
 * This function executes SPI NAND Program Load.
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
int FQspiPsu_Nand_P_LOAD(FQspiPsu_T* qspiPtr, u32 addr, u32 len, u8* value);

/*****************************************************************************
 * This function executes SPI NAND Program Execute.
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
int FQspiPsu_Nand_P_EXEC(FQspiPsu_T* qspiPtr, u32 addr);

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
int FQspiPsu_Nand_ExecCmd(FQspiPsu_T* qspiPtr, u8 cmd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
