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
 * @file fmsh_xqspi.h
 * @addtogroup qspipsu_v1_0
 * @{
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
#ifndef _FMSH_XSPI_H_ /* prevent circular inclusions */
#define _FMSH_XSPI_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/
#define QSPIPSU_DRV_VERSION             (0x100)

/***** Global configuration *****/

/***** device type *****/
#define QSPI_TYPE_NOR                   (0)
#define QSPI_TYPE_HYPERFLASH            (1)
#define QSPI_TYPE_HYPERRAM              (2)
#define QSPI_TYPE_NAND                  (3)

/***** device maker *****/
#define QSPI_MAKER_ID_SPANSION          (0x01)
#define QSPI_MAKER_ID_MICRON            (0x20)
#define QSPI_MAKER_ID_WINBOND           (0xEF)
#define QSPI_MAKER_ID_MACRONIX          (0xC2)
#define QSPI_MAKER_ID_ISSI              (0x9D)
#define QSPI_MAKER_ID_FMSH              (0xA1)
#define QSPI_MAKER_ID_GD                (0xC8)
#define QSPI_MAKER_ID_UNKNOWN           (0xFF)

/***** device size *****/
#define QSPI_UNKNOWN_SIZE               (0xFFFFFFFF)

/***** qspi_usercfg flags *****/
#define QSPI_F_INTR_EN                  (0x1)

#define QSPI_USERCFG(name)              static struct qspi_usercfg qspi_##name##_cfg
#define GET_QSPI_USERCFG(name)          &qspi_##name##_cfg

/***** dma type *****/
#define QSPI_NODMA                      (0)
#define QSPI_SDMA                       (1)
#define QSPI_MDMA                       (2)

/***** erase mode *****/
#define QSPI_ERS_SE                     (0)
#define QSPI_ERS_P4E                    (1)

/***** prog mode *****/
#define QSPI_PROG_PP                    (0)
#define QSPI_PROG_QPP                   (1)

/***** read mode *****/
#define QSPI_RD_READ                    (0)
#define QSPI_RD_FR                      (1)
#define QSPI_RD_DOR                     (2)
#define QSPI_RD_QOR                     (3)
#define QSPI_RD_DIOR                    (4)
#define QSPI_RD_QIOR                    (5)

/***** qspi seq type *****/
#define QSPI_SEQ_TYPE_CMD               (0)
#define QSPI_SEQ_TYPE_DATA              (1)

/***** qspi_cmd flags *****/
#define QSPI_CMD_F_LINK                 (0x1)
#define QSPI_CMD_F_XIP_EN               (0x2)
#define QSPI_CMD_F_OPCODE_EXT_EN        (0x4)
#define QSPI_CMD_F_ADDR_SHIFT           (0x8)
#define QSPI_CMD_F_TCMS_EN              (0x10)
#define QSPI_CMD_F_INST_EDGE_DDR        (0x100)
#define QSPI_CMD_F_ADDR_EDGE_DDR        (0x200)
#define QSPI_CMD_F_CRC_EN               (0x1000)
#define QSPI_CMD_F_CRC_VARIANT          (0x2000)

/***** qspi_data flags *****/
#define QSPI_DATA_F_DATA_IN             (0x1)
#define QSPI_DATA_F_CMD_FIFO            (0x2)
#define QSPI_DATA_F_DATA_EDGE_DDR       (0x100)
#define QSPI_DATA_F_2B_PER_ADDR         (0x200)
#define QSPI_DATA_F_CRC_EN              (0x1000)
#define QSPI_DATA_F_CRC_INV             (0x2000)
#define QSPI_DATA_F_CRC_OE              (0x4000)
#define QSPI_DATA_F_CRC_UAL_CHUNK_EN    (0x10000)
#define QSPI_DATA_F_CRC_UAL_CHUNK_CHECK (0x20000)
#define QSPI_DATA_F_DATA_SWAP           (0x100000)
#define QSPI_DATA_F_HF_READ_BOUND_EN    (0x200000)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
typedef void (*FQspiPsu_StatusHandler)(void* callBackRef, u32 statusEvent,
                                       u32 byteCount);

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
struct qspi_acmd_desc {
    u64 nxt_ptr;
    u64 mem_ptr;
    u64 dev_ptr;
    u64 resv1;
    u16 cmd_type;
    u16 cmd_flags;
    u32 cmd_cnt;
    u32 status;
    u32 resv3;
#ifdef __ICCARM__
};
#pragma pack(pop)
#else
} __attribute__((packed));
#endif

struct qspi_cmd {
    u32 flags;
    int inst_type;
    int bank;
    u8 opcode;
    u8 opcode_ext;
    u8 op_nios;
    u32 addr_h;
    u32 addr_l;
    u8 naddrs;
    u8 addr_nios;
    u8* data;
    u8 ndata;

    u32 status;
};

struct qspi_data {
    u32 flags;
    int bank;
    int dummy;
    u8* data;
    int ndata;
    int nios;
};

struct qspi_status {
    int sync;
    u32 intr_mask;
    u32 intr_status;
    u32 trd_comp;
    u32 trd_err;
};

struct qspi_usercfg {
    u32 flags;
    int dma_type;
    int ers_mode;
    int prog_mode;
    int read_mode;
    int naddrs;
};

struct qspi_sdma {
    u8 burst_type;
    u8 single_type;
    u8 tx_iface;
    u8 rx_iface;
    void* io;
};

typedef struct qspi_config {
    u16 device_id;
    u16 pad_lpbk;
    uintptr_t base;
    uintptr_t data_base;
    u32 sclk_hz;
    float board_delay;
} FQspiPsu_Config_T;

typedef struct qspi {
    FQspiPsu_Config_T config;

    int version;

    struct qspi_usercfg* usercfg;
    struct qspi_sdma* sdma;

    struct qspi_status status;

    int type;
    u32 maker;
    u64 dev_size;
    u32 blk_size;
    u32 page_size;
    int cur_cs;

    u8* buf;
    int len;

    FQspiPsu_StatusHandler status_handler;
    void* status_ref;

    void* priv;

} FQspiPsu_T;

/************************** Function Prototypes ******************************/

FQspiPsu_Config_T* FQspiPsu_LookupConfig(uint16_t device_id);

/*****************************************************************************
 * This function initializes a specific FQspiPs_T device/instance. This function
 * must be called prior to using the device to read or write any data.
 *
 * @param	spi is a pointer to the FQspiPs_T instance.
 * @param	configPtr points to the FQspiPs_T device configuration structure.
 *
 * @return
 *		- FMSH_SUCCESS if successful.
 *		- FMSH_FAILURE if fail.
 *
 * @note		The user needs to first call the FQspiPs_LookupConfig() API
 *		which returns the Configuration structure pointer which is
 *		passed as a parameter to the FQspiPs_CfgInitialize() API.
 *
 ******************************************************************************/
int FQspiPsu_CfgInitialize(FQspiPsu_T* qspiPtr, FQspiPsu_Config_T* configPtr);

/*****************************************************************************
 * This function resets qspi device registers to default value.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Reset(FQspiPsu_T* qspiPtr);

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
void FQspiPsu_SetStatusHandler(FQspiPsu_T* qspiPtr, void* callBackRef,
                               FQspiPsu_StatusHandler funcPtr);

/*****************************************************************************
 * This function tests if qspi device exists.
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if qspi device exists.
 *		- FMSH_FAILURE if qspi device not exists.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_SelfTest(FQspiPsu_T* qspiPtr);

/*****************************************************************************
 * The interrupt handler for QSPI interrupts. This function must be connected
 * by the user to an interrupt source. This function does not save and restore
 * the processor context such that the user must provide this processing.
 *
 * The interrupts that are handled are:
 *
 * @param	InstancePtr is a pointer to the FQspiPs_T instance to be worked on.
 *
 * @return	None.
 *
 * @note
 *
 *
 ******************************************************************************/
void FQspiPsu_InterruptHandler(void* instancePtr);

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
int FQspiPsu_Initialize(FQspiPsu_T* qspiPtr, u16 deviceId);

/*****************************************************************************
 * This function initializes controller hardware
 *
 * @param
 *       - capsPtr point to FQspiPs_Caps
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_InitHw(FQspiPsu_T* qspiPtr, struct qspi_usercfg* usercfg);

/*****************************************************************************
 * This function calibrate read delay capture
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
int FQspiPsu_Delay(FQspiPsu_T* qspiPtr, u32 sclkHz, u32 csdans, u32 cseotns,
                   u32 cssotns);

/*****************************************************************************
 * This function executes data transfer using sdma or nodma.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_SDMA_Transfer(FQspiPsu_T* qspiPtr, void* buf);

int FQspiPsu_CDMA_Exec(FQspiPsu_T* qspiPtr, struct qspi_acmd_desc* desc,
                       int timeout_us);
int FQspiPsu_CDMA_SoftReset(FQspiPsu_T* qspiPtr, int timeout_us);
int FQspiPsu_CDMA_EraseSectors(FQspiPsu_T* qspiPtr, u64 offs, u32 cnt,
                               int timeout_ms);
int FQspiPsu_CDMA_EraseAll(FQspiPsu_T* qspiPtr, int timeout_ms);
int FQspiPsu_CDMA_Read(FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                       int timeout_us);
int FQspiPsu_CDMA_Program(FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                          int timeout_us);

int FQspiPsu_PIO_SoftReset(FQspiPsu_T* qspiPtr, int timeout_us);
int FQspiPsu_PIO_EraseSectors(FQspiPsu_T* qspiPtr, u64 offs, u32 cnt,
                              int timeout_ms);
int FQspiPsu_PIO_EraseAll(FQspiPsu_T* qspiPtr, int timeout_ms);
int FQspiPsu_PIO_Read(FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                      int timeout_us);
int FQspiPsu_PIO_Program(FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                         int timeout_us);

/*****************************************************************************
 * This function executes qspi commands and data in stig mode.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Stig_Exec(FQspiPsu_T* qspiPtr, struct qspi_cmd* cmd,
                       struct qspi_data* data, int timeout_us);

/*****************************************************************************
 * This function config phy register
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
int FQspiPsu_Phy_Config(FQspiPsu_T* qspiPtr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
