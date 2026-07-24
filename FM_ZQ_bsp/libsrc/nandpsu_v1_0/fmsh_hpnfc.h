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
 * @file fmsh_hpnfc.h
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This header file contains the identification and functions (or macros)
 *  that are used for hpnfc operation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2023/02/16  First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_HPNFC_H_ /* prevent circular inclusions */
#define _FMSH_HPNFC_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************Include File*********************************/
#include "fmsh_common.h"

/******************************Constant Definition**************************/
/***** global configuration *****/
#define NAND_CONFIG_IFACE_SUPP (0x3)
#define NAND_PHY_NAND2DELAY    (28)

/********************************/
#define CTRL_TO_NAND(ctrl)     (ctrl->device)
#define NAND_TO_CTRL(device)   (FNfcPs_t *)(chip->ctrl)

/***** nand read/write flags *****/
#define NAND_OP_RAW            (0x1)
#define NAND_OP_OOBREQ         (0x2)

/***** nand read/write type *****/
#define NAND_TT_PAGE_RAW       (0)
#define NAND_TT_MAIN_OOB       (1)
#define NAND_TT_OOB            (2)

/***** nand options *****/
#define NAND_USE_INTR          (0x1)
#define NAND_USE_RNB_LINE      (0x2)
#define NAND_USE_VOLUME        (0x40)
#define NAND_ECC_ONDIE         (0x800)
#define NAND_ECC_SCRAMBLER     (0x1000)
#define NAND_ERASED_DET        (0x2000)
#define NAND_NO_SKIP_BYTE      (0x4000)
#define NAND_BBM_WITHECC       (0x8000)

/***** dma type *****/
#define NAND_NODMA             (0)
#define NAND_SDMA              (1)
#define NAND_MDMA              (2)

#define NAND_USERCFG(name)     static struct hpnfc_usercfg nand_##name##_cfg
#define GET_NAND_USERCFG(name) &nand_##name##_cfg

#define NAND_ASYNC_RST         (0)
#define NAND_SYNC_RST          (1)
#define NAND_LUN_RST           (2)

/******************************Type Definition******************************/
typedef void (*FNandPsu_StatusHandler)(void *callBackRef, u32 status, u32 len);
typedef int (*FNandPsu_DmaHandler)(void *callBackRef);

typedef struct hpnfc_cdma_desc FNandPsu_CdmaDesc_T;
typedef enum hpnfc_ecc_mode FNandPsu_EccMode_E;
typedef struct hpnfc_timings FNandPsu_Timings_T;
typedef struct hpnfc_usercfg FNandPsu_UserCfg_T;
typedef struct hpnfc_hwcaps FNandPsu_HwCaps_T;
typedef struct hpnfc_dma FNandPsu_DMA_T;
typedef struct hpnfc_config FNandPsu_Config_T;
typedef struct hpnfc FNandPsu_T;

/******************************Macro (inline function) Definition***********/

/******************************Variable Definition**************************/
enum hpnfc_tsf_dir { READ_FROM_DEVICE = 0, WRITE_TO_DEVICE = 1 };

/* This enum contains ECC Mode */
enum hpnfc_ecc_mode {
    ECC_NONE = 0,  /* No ECC */
    ECC_ONDIE = 1, /* On-Die ECC */
    ECC_HW = 2,    /* Hardware controller ECC */
    ECC_SW = 3     /* Software generated ECC */
};

/* Command DMA descriptor. */
struct hpnfc_cdma_desc {
    /* Next descriptor address. */
    u64 next_pointer;
    /* Flash address is a 32-bit address comprising of BANK and ROW ADDR. */
    u32 flash_pointer;
    /*field appears in HPNFC version 13*/
    u16 bank;
    u16 rsvd0;
    /* Operation the controller needs to perform. */
    u16 command_type;
    u16 rsvd1;
    /* Flags for operation of this command. */
    u16 command_flags;
    u16 rsvd2;
    /* System/host memory address required for data DMA commands. */
    u64 memory_pointer;
    /* Status of operation. */
    u64 status;
    /* Address pointer to sync buffer location. */
    u64 sync_flag_pointer;
    /* Controls the buffer sync mechanism. */
    u32 sync_arguments;
    u32 rsvd4;
    /* control data stored */
    u64 ctrl_data_pointer;
};

/* Controller timming */
struct hpnfc_timings {
    u32 common_settings;
    u32 toggle_timings0;
    u32 toggle_timings1;
    u32 async_toggle_timings;
    u32 sync_timings;
    u32 timings0;
    u32 timings1;
    u32 timings2;
    u32 mini_dll_phy_ctrl;

    u32 dll_phy_dq_timings;
    u32 dll_phy_dqs_timings;
    u32 dll_phy_gate_lpbk_ctrl;
    u32 dll_phy_dll_master_ctrl;
    u32 dll_phy_dll_slave_ctrl;
    u32 dll_phy_ctrl;
    u32 dll_phy_tsel;
};

/* Controller User Config Capability */
struct hpnfc_usercfg {
    u32 options;
    int dma_type;
    /* Used for nand_device */
    u32 dev_bbt_options;
};

/* Controller interrupt status */
struct hpnfc_status {
    int irq_sync;
    int trd_sync;
    u32 intr_status;
    u32 trd_error;
    u32 trd_comp;
    u32 trd_timeout;
};

/* Controller Hardware Capability */
struct hpnfc_hwcaps {
    /* Maximum number of banks supported by hardware. */
    u8 max_banks;
    /* Slave and Master DMA data width in bytes (4 or 8). */
    u8 data_dma_width;
    /* Is PHY type DLL. */
    u8 is_phy_type_dll;
};

struct hpnfc_bchcaps {
    u8 corr_str[8];
    u16 sector_size[2];
    u8 meta_size;
};

/* DMA Information */
struct hpnfc_dma {
    int status;
    void *src;
    void *dst;
    unsigned int len;
    enum hpnfc_tsf_dir dir;
    FNandPsu_DmaHandler handler;
};

/* Configuration Information */
struct hpnfc_config {
    u16 device_id; /* Unique ID  of device */
    uintptr_t base;
    uintptr_t data_base;
    u32 io_width;
    u32 clock_hz;
    float board_delay;
};

/* HPNFC */
struct hpnfc {
    struct hpnfc_config config;

    u32 flags;
    u32 ctrl_rev;
    u32 conf_done;

    struct hpnfc_usercfg *usercfg;
    struct hpnfc_hwcaps hwcaps;

    u32 skip_block_base;

    /* DMA */
    struct hpnfc_dma dma;

    /* Ecc */
    enum hpnfc_ecc_mode ecc_mode;

    /* Status */
    struct hpnfc_status status;
    u32 cmd_status;
    u32 fault_page;

    FNandPsu_StatusHandler status_handler;
    void *status_ref;

    /* Internal */

    /* Externals */
    int cur_cs;
    int cur_vol;
    struct nand_device *device;
    struct nand_ctrl_ops *ctrl_ops;
};

/* Nand op instrction type */
enum nand_instr_type {
    CMD_INSTR = 0,
    ADDR_INSTR = 1,
    DATAIN_INSTR = 2,
    DATAOUT_INSTR = 3,
    WAITRDY_INSTR = 4,
    READ_INSTR = 5,
    READ_STATUS_INSTR = 7,
    READ_STATUS_EHSD_INSTR = 8,
    GET_FEAT_INSTR = 24,
    RDID_INSTR = 27,
    READ_PARAPAGE_INSTR = 28,
};

/* Nand op instrction */
struct nand_instr {
    enum nand_instr_type instr_type;
    union {
        struct {
            u8 opcode;
        } cmd;
        struct {
            unsigned int naddrs;
            u8 *addrs;
        } addr;
        struct {
            unsigned int len;
            u8 *buf;
            u8 force_8bit;
            u8 raw;
            u32 options;
        } data;
        struct {
            unsigned int timeout_us;
        } waitrdy;
    } ctx;
    unsigned int delay_ns;
};

/* Nand op array */
struct nand_operation {
    int cs;
    struct nand_instr *instr;
    int ninstr;
};

/******************************Function Prototype***************************/

/**
 * FNandPsu_LookupConfig - Lookup config table.
 *
 * This function looks up the device configuration based on the unique device
 *ID. The table s_ConfigTable contains the configuration info for each device in
 *the system.
 *
 * @device_id contains the ID of the device for which the device configuration
 * pointer is to be returned.
 *
 * return a pointer to the configuration found,
 * NULL if the specified device ID was not found.
 *
 **/
FNandPsu_Config_T *FNandPsu_LookupConfig(u16 device_id);

/**
 * FNandPsu_CfgInitialize - Initialize FNandPsu_T value.
 *
 * This function initializes nfcPtr value based on found configuration table.
 *
 * @configPtr configuration table found by FNandPsu_LookupConfig.
 *
 * return FMSH_SUCCESS if execute finished.
 *
 **/
int FNandPsu_CfgInitialize(FNandPsu_T *nfcPtr, FNandPsu_Config_T *cfgPtr);

/**
 * FNandPsu_Reset - Reset controller.
 *
 * This function reset hpnfc logic.
 *
 **/
int FNandPsu_Reset(void);

/**
 * FNandPsu_SelfTest - Initialize FNandPsu_T value.
 *
 * This function initializes nfcPtr value based on found configuration table.
 *
 * return FMSH_SUCCESS if controller is accessable.
 *
 **/
int FNandPsu_SelfTest(FNandPsu_T *nfcPtr);

void FNandPsu_SetStatusHandler(FNandPsu_T *nfcPtr, void *callBackRef,
                               FNandPsu_StatusHandler funcPtr);

void FNandPsu_InterruptHandler(void *instancePtr);

/**
 * FNandPsu_Initialize  - Initialize controller and device
 *
 * This function scan device to initialize controller including
 * create bbt and setup device to best timing mode.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_Initialize(FNandPsu_T *nfcPtr);

int FNandPsu_HwInit(FNandPsu_T *nfcPtr, FNandPsu_UserCfg_T *usercfg);

int FNandPsu_HwInitr(FNandPsu_T *nfcPtr);

/**
 * FNandPsu_SetOobBuf  - Set oob buffer and its length
 *
 * This function must be used before read/write oob area.
 * device->oob_buf and device->ooblen are used to store oob data.
 *
 * @nfcPtr: The NAND controller
 * @buf: data buffer in memory
 * @len: bytes to read/write
 *
 * Returns 0 on success, a negative error code otherwise.
 **/
int FNandPsu_SetOobBuf(FNandPsu_T *nfcPtr, u8 *buf, unsigned int len);

int FNandPsu_PIO_Reset(FNandPsu_T *nfcPtr, int vol, u32 page, int type);

int FNandPsu_PIO_SetFeature(FNandPsu_T *nfcPtr, int vol, u8 feature, u32 data);

int FNandPsu_PIO_Erase(FNandPsu_T *nfcPtr, int vol, u32 page, u8 nblocks);

int FNandPsu_PIO_WritePage(FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                           u8 npages);

int FNandPsu_PIO_ReadPage(FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                          u8 npages);

int FNandPsu_PIO_CopyBack(FNandPsu_T *nfcPtr, int vol, u32 src_page,
                          u32 dst_page, u8 npages);

int FNandPsu_CDMA_Send(FNandPsu_T *nfcPtr, int thread,
                       struct hpnfc_cdma_desc *desc);

int FNandPsu_CDMA_SendAndWait(FNandPsu_T *nfcPtr, int thread,
                              struct hpnfc_cdma_desc *desc, int timeout_us);

int FNandPsu_CDMA_Reset(FNandPsu_T *nfcPtr, int vol, u32 page, int type);

int FNandPsu_CDMA_Erase(FNandPsu_T *nfcPtr, int vol, u32 page, u8 nblocks);

int FNandPsu_CDMA_WritePage(FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                            u8 npages);

int FNandPsu_CDMA_ReadPage(FNandPsu_T *nfcPtr, int vol, u32 page, void *buf,
                           u8 npages);

int FNandPsu_CDMA_CopyBack(FNandPsu_T *nfcPtr, int vol, u32 src_page,
                           u32 dst_page, u8 npages);

int FNandPsu_Generic_CmdSend(FNandPsu_T *nfcPtr, int thread, u64 mini_ctrl_cmd);

int FNandPsu_Generic_CmdSendAndWait(FNandPsu_T *nfcPtr, int thread,
                                     u64 mini_ctrl_cmd, int timeout_us);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
