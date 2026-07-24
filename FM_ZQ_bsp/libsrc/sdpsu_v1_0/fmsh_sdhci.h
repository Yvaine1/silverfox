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
 * @file fmsh_sdhci.h
 * @addtogroup sdpsu_v1_0
 * @{
 *
 * This header file contains the identifiers and driver functions (or  macros)
 * that can be used to access the device.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ---    -------- -----------------------------------------------
 * 1.00  hzq  22/10/31 Initial release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef _FMSH_SDHCI_H_
#define _FMSH_SDHCI_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/
/***** config *****/

#if !NO_OS
    #define SDMMC_CONFIG_DMA_ADDR_64    (1)
#else
    #define SDMMC_CONFIG_DMA_ADDR_64    (0)
#endif
#define SDMMC_CONFIG_DDR50_SUPPORT  (1)
#define SDMMC_CONFIG_SDR104_SUPPORT (1)
#define SDMMC_CONFIG_DDR52_SUPPORT  (1)
#define SDMMC_CONFIG_HS200_SUPPORT  (1)
#define SDMMC_CONFIG_TUNING_SUPPORT (0)

/****************************/
#define SDMMC_ENORESP               (-1)
#define SDMMC_MAX_BLOCK_LEN         (512)
#define SDMMC_MAX_BLOCK_CNT         (65535)
#define SDMMC_DEFAULT_BOUNDARY_SIZE (512 * 1024)

/***** card power state *****/
#define SDMMC_POWER_OFF             (0)
#define SDMMC_POWER_ON              (1)
#define SDMMC_POWER_CYCLE           (2)

/***** SDMMC bus width & speed mode *****/
#define SDMMC_DDR_MODE_MARK         (0x80)

#define SDMMC_BUS_1BIT              (0x01)
#define SDMMC_BUS_4BIT              (0x04)
#define SDMMC_BUS_8BIT              (0x08)
#define SDMMC_BUS_4BIT_DDR          (0x04 | SDMMC_DDR_MODE_MARK)
#define SDMMC_BUS_8BIT_DDR          (0x08 | SDMMC_DDR_MODE_MARK)

#define SDMMC_DS_ID                 (0x00)
#define SDMMC_DS                    (0x01)

#define SD_HS                       (0x02)
#define UHS_SDR12                   (0x03)
#define UHS_SDR25                   (0x04)
#define UHS_SDR50                   (0x05)
#define UHS_DDR50                   (0x05 | SDMMC_DDR_MODE_MARK)
#define UHS_SDR104                  (0x06)

#define MMC_HS26                    (0x12)
#define MMC_HS52                    (0x13)
#define MMC_HS52_DDR                (0x13 | SDMMC_DDR_MODE_MARK)
#define MMC_HS200                   (0x14)
#define MMC_HS400                   (0x14 | SDMMC_DDR_MODE_MARK)
#define MMC_HS400_ES                (0x15 | SDMMC_DDR_MODE_MARK)

/***** SDMMC card capabilities *****/
#define SDMMC_CAPS_BUS_8BIT         (0x40000000)
#define SDMMC_CAPS_BUS_4BIT         (0x20000000)
#define SDMMC_CAPS_BUS_1BIT         (0x10000000)
// for sd uhs1
#define SDMMC_CAPS_SDR104           (0x01000000)
#define SDMMC_CAPS_DDR50            (0x00800000)
#define SDMMC_CAPS_SDR50            (0x00400000)
#define SDMMC_CAPS_SDR25            (0x00200000)
#define SDMMC_CAPS_SDR12            (0x00100000)
// for mmc
#define SDMMC_CAPS_HS400            (0x00020000)
#define SDMMC_CAPS_HS200            (0x00010000)
#define SDMMC_CAPS_HS52_DDR         (0x00004000)
#define SDMMC_CAPS_HS52             (0x00002000)
#define SDMMC_CAPS_HS26             (0x00001000)

#define SDMMC_CAPS_HS               (0x00000800)
#define SDMMC_CAPS_DS               (0x00000400)

#define SDMMC_CAPS_BUS_WIDTH        (0x70000000)
#define SDMMC_CAPS_SD_MODE          (0x01f00c00)
#define SDMMC_CAPS_UHS_MODE         (0x01f00000)
#define SDMMC_CAPS_MMC_MODE         (0x0001fc00)

/***** SDMMC DAT pattern *****/
#define SDMMC_DAT_0000              (0x0)
#define SDMMC_DAT_1111              (0x1)
#define SDMMC_DAT_xxx0              (0x2)
#define SDMMC_DAT_xxx1              (0x3)

/***** host version *****/
#define SDMMC_HOST_VERSION_3        (0x300)
#define SDMMC_HOST_VERSION_4        (0x400)

/***** usercfg *****/
#define SDMMC_F_DDR_SUPPORT         (0x200)
#define SDMMC_F_UHS_SUPPORT         (0x100)
#define SDMMC_F_HOST_VERSION_4      (0x40)
#define SDMMC_F_USE_PRESET_VALUE    (0x20)
#define SDMMC_F_USE_INTR            (0x1)

#define SDMMC_USE_NO_DMA            (0)
#define SDMMC_USE_SDMA              (1)
#define SDMMC_USE_SDMA64            (2)
#define SDMMC_USE_ADMA              (3)
#define SDMMC_USE_ADMA64            (4)

#define SDMMC_USERCFG(name)         static struct sdmmc_usercfg sdmmc_##name##_cfg
#define GET_SDMMC_USERCFG(name)     &sdmmc_##name##_cfg

/***** sdmmc cmd & data flags *****/
#define SDMMC_FLCMD_SUSPEND         (0x1)
#define SDMMC_FLCMD_RESUME          (0x2)
#define SDMMC_FLCMD_ABORT           (0x4)

#define SDMMC_FLDATA_READ           (0x1)
#define SDMMC_FLDATA_AUTOCMD12      (0x2)
#define SDMMC_FLDATA_AUTOCMD23      (0x4)
#define SDMMC_FLDATA_AUTOCMD        (0x6)
#define SDMMC_FLDATA_NOTRANSFER     (0x8)

/**************************** Type Definitions *******************************/
typedef struct sdmmc_usercfg FSdPsu_UserCfg_T;
typedef struct sdhci FSdPsu_Host_T;
typedef struct sdmmc_card FSdPsu_Card_T;
typedef struct sdmmc_config FSdPsu_Config_T;
typedef struct sdmmc FSdPsu_T;

typedef void (*FSdPsu_StatusHandler)(void *callBackRef, u32 statusEvent,
                                     u32 byteCount);

/**********************************Macro (inline function)
 * Definition***********/

/**********************************Variable
 * Definition**************************/
struct sdmmc_usercfg {
    u32 flags;
    u32 dma;
    u32 sdclk_max;
    u32 force_bus_width;
    u32 force_speed_mode;
};

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
struct sdhci_adma_desc {
    u16 attr;
    u16 length;
#if SDMMC_CONFIG_DMA_ADDR_64 == 1
    u64 address;
#else
    u32 address;
#endif
#ifdef __ICCARM__
};
#pragma pack(pop)
#else
} __attribute__((__packed__));
#endif

struct sdmmc_cmd {
    u32 flags;
    u32 idx;
    u32 arg;
    u32 resp_type;
    u32 resp[4];
};

struct sdmmc_data {
    u32 flags;
    void *buf;
    int blocks;
    int blocksize;
};

struct sdhci {
    u32 version;

    u32 int_status;

    u32 sdclk;
    u32 mode;
    u32 bus_width;

    int (*powercycle)(FSdPsu_T *sdPtr);
};

struct sdmmc_card {
    u32 version;

    u32 caps;

    u32 card_type;
    u32 voltages;
    u32 freq;
    u32 mode;
    u32 bus_width;
    u32 bus_voltage;

    unsigned int max_trans_rate;

    unsigned long long device_size;
    unsigned long long block_max;
    unsigned int high_capacity;

    unsigned int write_blk_len;
    unsigned int read_blk_len;

    u32 cid[4];
    struct {
        u32 mid;
        u16 oid;
        u8 cbx_mmc;  /* for mmc only */
        char pnm[6]; /* pnm[6] for mmc only */
        u8 prv;
        u8 psn;
        u8 month;
        u16 year;
    } cid_decode;

    u32 csd[4];
    struct {
        u8 csd_struct;
        u8 taac;
        u8 nsac;
        u8 tran_rate_unit : 3, tran_speed_value : 5;
        u16 ccc;
        u8 read_bl_len : 4, read_bl_partial : 1, read_blk_misalign : 1;
        u8 write_bl_len : 4, write_bl_partial : 1, write_bl_misalign : 1;
        u32 c_size;
        u8 c_size_mult;
        u8 r2w_factor;
        u8 file_fomat;
    } csd_decode;

    u32 rca;
    u32 ocr;
};

struct sdmmc_config {
    u16 device_id;      /* Unique ID  of device */
    u16 card_type;
    u32 base;           /* Base address of the device */
    u32 input_clock_hz; /* Input clock frequency */
    u32 init_freq;
    u32 bus_width;
    u32 has_cd;
    u32 has_wp;
    u32 has_buspwr;
    u32 is_cache_coherent;
};

struct sdmmc {
    FSdPsu_Config_T config;

    struct sdmmc_usercfg *usercfg;

    int is_inited;
    u32 flags;

    struct sdhci host;
    struct sdmmc_card *card;
    struct sdhci_adma_desc *desc;

    FSdPsu_StatusHandler statusHandler;
    void *statusRef;
};

/************************** Function Prototypes ******************************/
FSdPsu_Config_T *FSdPsu_LookupConfig(uint16_t device_id);
int FSdPsu_CfgInitialize(FSdPsu_T *sdPtr, FSdPsu_Config_T *configPtr);

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
void FSdPsu_SetStatusHandler(FSdPsu_T *sdPtr, void *callBackRef,
                             FSdPsu_StatusHandler funcPtr);

/*****************************************************************************
 * This function tests if sdhci exists.
 *
 * @param
 *
 * @return
 *		- 0 if sdhci exists.
 *		- 1 if sdfci not exists.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_SelfTest(FSdPsu_T *sdPtr);

/*****************************************************************************
 * This function handle sdhci interrupts.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
void FSdPsu_InterruptHandler(void *instancePtr);

/*****************************************************************************
 * This function is used to switch io mux.
 * type0 is sd card, if device is sd card, io4~7 is used to drive level shifter.
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- FMSH_RIO if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_MuxIOs(FSdPsu_T *sdPtr, int type);

/*****************************************************************************
 * This function is used to sd slcr reset.
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- FMSH_ETIME if fail.
 *
 * @note
 *
 ******************************************************************************/
void FSdPsu_Reset(FSdPsu_T *sdPtr);

/*****************************************************************************
 * This function is used to reset host controller.
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- FMSH_ETIME if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_Reset(FSdPsu_T *sdPtr);

/*****************************************************************************
 * This function is used to detect cd pin.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_CardDetect(FSdPsu_T *sdPtr);

/*****************************************************************************
 * This function is used to detect wp pin.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_WriteProt(FSdPsu_T *sdPtr);

/*****************************************************************************
 * This function is used to initialize host controller (struct).
 *
 * @param
 *   cfg
 *       - user configuretion
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_InitHw(FSdPsu_T *sdPtr, FSdPsu_UserCfg_T *usercfg);

/*****************************************************************************
 * This function sets host to default io state to initialize card .
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_DefaultIOs(FSdPsu_T *sdPtr);

void FSdPsu_Host_SetWakeUpEnable(FSdPsu_T *sdPtr, int enable);

/*****************************************************************************
 * This function is used to control host power supply .
 *
 * @param
 *   power
 *       - 0: power off
 *       - 1: power on
 *       - 2: power cycle
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_SetPower(FSdPsu_T *sdPtr, u32 power);

/*****************************************************************************
 * This function control host clock supply.
 *
 * @param
 *       - freq: 0 stand for disable clk
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_SetClock(FSdPsu_T *sdPtr, u32 freq);

/*****************************************************************************
 * This function control host bus width.
 *
 * @param
 *       - freq: 0 stand for disable clk
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_SetBusWidth(FSdPsu_T *sdPtr, int width);

/*****************************************************************************
 * This function control host bus speed .
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_SetBusSpeed(FSdPsu_T *sdPtr, int mode);

/*****************************************************************************
 * This function control host signal voltage .
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_SetSignalVoltage(FSdPsu_T *sdPtr, int signal_voltage);

/*****************************************************************************
 * This function is used to send cmd to card.
 *
 * @param
 *
 * @return
 *		- 0 if success.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_SendCmd(FSdPsu_T *sdPtr, struct sdmmc_cmd *cmd,
                        struct sdmmc_data *data);

/*****************************************************************************
 * This function is used to check DAT line.
 *
 * @param
 *
 * @return
 *		- 0 if DAT line is ready.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Host_CheckDAT(FSdPsu_T *sdPtr, int pattern, int timeout_us);

/*****************************************************************************
 * This function is used to set phy value.
 *
 * @param
 *
 * @return
 *		- 0 if DAT line is ready.
 *		- other value if fail.
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Phy_Config(FSdPsu_T *sdPtr, u32 mode);
int FSdPsu_Phy_SetDqsDelay(FSdPsu_T *sdPtr, u8 value);

int FSdPsu_Host_ExecuteTuning(FSdPsu_T *sdPtr);
int FSdPsu_Host_ErrorRecovery(FSdPsu_T *sdPtr);

#ifdef __cplusplus
}
#endif

#endif
