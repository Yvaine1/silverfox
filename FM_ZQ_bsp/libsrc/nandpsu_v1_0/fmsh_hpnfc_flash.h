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
 * @file fmsh_hpnfc_flash.h
 * @addtogroup nandpsu_v1_0
 * @{
 *
 *  This header file contains the device operating functions (or macros).
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
#ifndef _FMSH_HPNFC_FLASH_H_ /* prevent circular inclusions */
#define _FMSH_HPNFC_FLASH_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************Include File*********************************/
#include "fmsh_hpnfc.h"

/******************************Constant Definition**************************/
#define NAND_MAX_CHIPS          (2)

#define NAND_MAKER_ID_SPANSION  (0x01)
#define NAND_MAKER_ID_MICRON    (0x2C)
#define NAND_MAKER_ID_SAMSUNG   (0xEC)
#define NAND_MAKER_ID_FMSH      (0xA1)
#define NAND_MAKER_ID_GD        (0xC8)
#define NAND_MAKER_ID_UNKNOWN   (0xff)

/* Nand Commands */
#define NAND_CMD_RESET          (0xFF)
#define NAND_CMD_RDID           (0x90)
#define NAND_CMD_PARAMPAGE      (0xEC)
#define NAND_CMD_SETFEATURE     (0xEF)
#define NAND_CMD_GETFEATURE     (0xEE)
#define NAND_CMD_STATUS         (0x70)
#define NAND_CMD_STATUSENHANCED (0x78)
#define NAND_CMD_ERASE1         (0x60)
#define NAND_CMD_ERASE2         (0xD0)
#define NAND_CMD_READ0          (0x00)
#define NAND_CMD_READ1          (0x01)
#define NAND_CMD_READ2          (0x05)
#define NAND_CMD_READSTART      (0x30)
#define NAND_CMD_RNDOUTSTART    (0xE0)
#define NAND_CMD_SEQIN          (0x80)
#define NAND_CMD_PAGEPROG       (0x10)
#define NAND_CMD_CACHEDPROG     (0x15)

/* Nand Status */
#define NAND_STATUS_FAIL        (0x1)
#define NAND_STATUS_FAILC       (0x2)
#define NAND_STATUS_ARDY        (0x20)
#define NAND_STATUS_RDY         (0x40)
#define NAND_STATUS_WP          (0x80)

/* Nand Timings */
#define NAND_TRST_MAX           (5000)
#define NAND_TBERS_MAX          (35000)
#define NAND_TPROG_MAX          (600)
#define NAND_TR_MAX             (35)
#define NAND_TCCS_MIN           (200)

/***** nand interface type *****/
#define NAND_SDR_IFACE          0
#define NAND_NVDDR_IFACE        1
#define NAND_NVDDR23_IFACE      2

#define NAND_TIMING_NS(iface, name)                                       \
    ((iface->type == NAND_SDR_IFACE)                                      \
         ? iface->timings.sdr.name                                        \
         : ((iface->type == NAND_NVDDR_IFACE) ? iface->timings.nvddr.name \
                                              : iface->timings.nvddr23.name))

#define NAND_TIMING_US(iface, name)                \
    ((iface->type == NAND_SDR_IFACE)               \
         ? iface->timings.sdr.name / 1000          \
         : ((iface->type == NAND_NVDDR_IFACE)      \
                ? iface->timings.nvddr.name / 1000 \
                : iface->timings.nvddr23.name / 1000))

#define NAND_IS_DDR_IFACE(iface) \
    ((iface->type == NAND_NVDDR_IFACE) || (iface->type == NAND_NVDDR23_IFACE))

#define NAND_BLOCK_TO_ADDR(block, device) \
    ((u64)block << device->model.erase_shift)
#define NAND_PAGE_TO_ADDR(page, device) ((u64)page << device->model.page_shift)
#define NAND_BLOCK_TO_PAGE(block, device) \
    (block << (device->model.erase_shift - device->model.page_shift))
#define NAND_PAGE_TO_BLOCK(page, device) \
    (page >> (device->model.erase_shift - device->model.page_shift))

/***** nand_device options *****/
#define NAND_SUPPORT_ONFI  (0x1) /* set automatically */
#define NAND_BUSWIDTH_AUTO (0x2)

/***** nand_device bbt_options *****/
#define NAND_USE_FLASH_BBT (0x1)  /* set by user */
#define NAND_BBT_WRITE     (0x2)  /* set by user */
#define NAND_BBT_PERCHIP   (0x4)  /* set by user */
#define NAND_SKIP_BBTSCAN  (0x8)  /* set by user */
#define NAND_AUTO_MARKBAD  (0x10) /* set by user */

/******************************Type Definition******************************/
typedef struct nand_parapage FNandPsu_ParaPage_T;
typedef struct nand_bbt_desc FNandPsu_BbtDesc_T;

/******************************Macro (inline function) Definition***********/

/******************************Variable Definition**************************/
/* ECC */
struct nand_ecc {
    int steps;              /* number of ECC steps per page */
    int size;               /* data bytes per ECC step */
    int bytes;              /* ECC bytes per step */
    int str_idx;
    int strength;           /* max number of correctible bits per ECC step */
    int total;              /* total number of ECC bytes per page */
    int available_oob_size; /* max oob bytes to use ECC */
};

/* Bad block table descriptor */
struct nand_bbt_desc {
    u32 page_offset[NAND_MAX_CHIPS]; /* Page offset where BBT resides */
    u32 sig_offset;                  /* Signature offset in Spare area */
    u32 ver_offset;                  /* Offset of BBT version */
    u32 sig_length;                  /* Length of the signature */
    u32 max_blocks;                  /* Max block num to search for BBT */
    char signature[4];               /* BBT signature */
    u8 version[NAND_MAX_CHIPS];      /* BBT version */
    u32 valid[NAND_MAX_CHIPS];       /* BBT descriptor is valid or not */
};

/* Bad block pattern */
struct nand_bb_pattern {
    u32 options;   /* Options to search the bad block pattern */
    u32 offset;    /* Offset to search for specified pattern */
    u32 length;    /* Number of bytes to check the pattern */
    u8 pattern[2]; /* Pattern format to search for */
};

/* Nand chip organization */
struct nand_model {
    u8 manufacture;
    u8 device_id;
    u8 ids[4];

    u8 io_width;

    u8 page_shift;   /* page size */
    u8 erase_shift;  /* block size */
    u8 lun_shift;    /* lun size */
    u8 target_shift; /* target size(1 cs) */
    unsigned int pagesize;
    unsigned int subpagesize;
    unsigned int oobsize;
    unsigned int suboobsize;
    unsigned int blocksize;

    unsigned int page_num;  /* page num per block */
    unsigned int block_num; /* block num per lun(die) */
    unsigned int lun_num;   /* lun_num per target */

    unsigned int ntargets;  /* target num */
    unsigned long long device_size;
};

struct nand_feature {
    u16 revisions;
#define NAND_SUPPORT_ONFI_1_0 (0x2)
#define NAND_SUPPORT_ONFI_2_0 (0x4)
#define NAND_SUPPORT_ONFI_2_1 (0x8)
#define NAND_SUPPORT_ONFI_2_2 (0x10)
#define NAND_SUPPORT_ONFI_2_3 (0x20)
#define NAND_SUPPORT_ONFI_3_0 (0x40)
#define NAND_SUPPORT_ONFI_3_1 (0x80)
#define NAND_SUPPORT_ONFI_3_2 (0x100)
#define NAND_SUPPORT_ONFI_4_0 (0x200)
    u16 features;
#define NAND_SUPPORT_WIDTH16         (0x1)
#define NAND_SUPPORT_MULTILUN        (0x2)
#define NAND_SUPPORT_NONSEQ_PAGEPROG (0x4)
#define NAND_SUPPORT_MPL_WRITE       (0x8)
#define NAND_SUPPORT_NVDDR           (0x20)
#define NAND_SUPPORT_MPL_READ        (0x40)
#define NAND_SUPPORT_EZNAND          (0x200)
#define NAND_SUPPORT_NVDDR2          (0x400)
#define NAND_SUPPORT_VOLUME          (0x800)
#define NAND_SUPPORT_NVDDR3          (0x2000)
#define NAND_SUPPORT_ZQCAL           (0x4000)
    u16 opt_cmds;
#define NAND_SUPPORT_CACHE_WRITE (0x1)
#define NAND_SUPPORT_CACHE_READ  (0x2)
    u16 adv_cmds;
#define NAND_SUPPORT_RAND_DATAOUT  (0x1)
#define NAND_SUPPORT_MPL_PAGE_PROG (0x2)
#define NAND_SUPPORT_MPL_CB_PROG   (0x4)
#define NAND_SUPPORT_MPL_ERASE     (0x8)
    u16 sdr_mode;
    u16 nvddr_mode;
    u16 nvddr2_mode;
    u16 nvddr3_mode;
#define NAND_SUPPORT_TIMING_MODE_0 (0x1)
#define NAND_SUPPORT_TIMING_MODE_1 (0x2)
#define NAND_SUPPORT_TIMING_MODE_2 (0x4)
#define NAND_SUPPORT_TIMING_MODE_3 (0x8)
#define NAND_SUPPORT_TIMING_MODE_4 (0x10)
#define NAND_SUPPORT_TIMING_MODE_5 (0x20)
#define NAND_SUPPORT_TIMING_MODE_6 (0x40)
#define NAND_SUPPORT_TIMING_MODE_7 (0x80)
#define NAND_SUPPORT_TIMING_MODE_8 (0x100)
#define NAND_SUPPORT_TIMING_MODE_9 (0x200)

    int tRST_max;   // unit in us
    int tBRES_max;  // unit in us
    int tPROG_max;  // unit in us
    int tR_max;     // unit in us
    int tCCS_min;   // unit in ns

    u8 wr_warmup;
    u8 rd_warmup;

    u8 ecc_corr_str;
    u8 bits_per_cell;

    u8 row_cycles;
};

/* Parameter page structure of ONFI 4.0 specification. */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
struct nand_parapage {
    /*
     * Revision information and features block
     */
    u8 Signature[4];     /**< Parameter page signature */
    u16 Revision;        /**< Revision Number */
    u16 Features;        /**< Features supported */
    u16 OptionalCmds;    /**< Optional commands supported */
    u8 AdvancedCmds;     /**< ONFI-JEDEC JTG primary advanced command support */
    u8 TrainingCmds;     /**< Training commands supported */
    u16 ExtParamPageLen; /**< ONFI 2.3: extended parameter page length */
    u8 NumOfParamPages;  /**< ONFI 2.3: No of parameter pages */
    u8 Reserved1[17];    /**< Reserved */
    /*
     * Manufacturer information block
     */
    u8 DeviceManufacturer[12]; /**< Device manufacturer */
    u8 DeviceModel[20];        /**< Device model */
    u8 JedecManufacturerId;    /**< JEDEC Manufacturer ID */
    u8 DateCode[2];            /**< Date code */
    u8 Reserved2[13];          /**< Reserved */
    /*0-pui
     * Memory organization block
     */
    u32 BytesPerPage;             /**< Number of data bytes per page */
    u16 SpareBytesPerPage;        /**< Number of spare bytes per page */
    u32 BytesPerPartialPage;      /**< Number of data bytes per partial page */
    u16 SpareBytesPerPartialPage; /**< Number of spare bytes per partial page */
    u32 PagesPerBlock;            /**< Number of pages per block */
    u32 BlocksPerLun;        /**< Number of blocks per logical unit (LUN) */
    u8 NumLuns;              /**< Number of LUN's */
    u8 AddrCycles;           /**< Number of address cycles */
    u8 BitsPerCell;          /**< Number of bits per cell */
    u16 MaxBadBlocksPerLun;  /**< Bad blocks maximum per LUN */
    u16 BlockEndurance;      /**< Block endurance */
    u8 GuaranteedValidBlock; /**< Guaranteed valid blocks at beginning of target
                              */
    u16 BlockEnduranceGvb;   /**< Block endurance for guaranteed valid block */
    u8 ProgramsPerPage;      /**< Number of programs per page */
    u8 PartialProgAttr;      /**< Partial programming attributes */
    u8 EccBits;              /**< Number of bits ECC correctability */
    u8 InterleavedAddrBits;  /**< Number of interleaved address bits */
    u8 InterleavedOperation; /**< Interleaved operation attributes */
    u8 EzNandSupport;        /**< ONFI 2.3: EZ NAND support parameters */
    u8 Reserved3[12];        /**< Reserved */
    /*
     * Electrical parameters block
     */
    u8 IOPinCapacitance;     /**< I/O pin capacitance */
    u16 TimingMode;          /**< Timing mode support */
    u16 PagecacheTimingMode; /**< Program cache timing mode */
    u16 TProg;               /**< Maximum page program time */
    u16 TBers;               /**< Maximum block erase time */
    u16 TR;                  /**< Maximum page read time */
    u16 TCcs;                /**< Maximum change column setup time */
    u8 SynTimingMode;   /**< ONFI 2.3: Source synchronous timing mode support */
    u8 Nvddr2TimingMode1;
    u8 SynFeatures;     /**< ONFI 2.3: Source synchronous features */
    u16 ClkInputPinCap; /**< ONFI 2.3: CLK input pin capacitance */
    u16 IOPinCap;       /**< ONFI 2.3: I/O pin capacitance */
    u16 InputPinCap;    /**< ONFI 2.3: Input pin capacitance typical */
    u8 InputPinCapMax;  /**< ONFI 2.3: Input pin capacitance maximum */
    u8 DrvStrength;     /**< ONFI 2.3: Driver strength support */
    u16 TMr;            /**< ONFI 2.3: Maximum multi-plane read time */
    u16 TAdl; /**< ONFI 2.3: Program page register clear enhancement value */
    u16 TEr;  /**< ONFI 2.3: Typical page read time for EZ NAND */
    u8 Nvddr3Features;    /**< NV-DDR2/3 features */
    u8 Nvddr3Warmup;      /**< NV-DDR2/3 warmup cycles */
    u16 Nvddr3TimingMode; /**< NV-DDR3 timing mode support */
    u8 Nvddr2TimingMode2; /**< NV-DDR2 timing mode support */
    u8 Reserved4;
    /*
     * Vendor block
     */
    u16 VendorRevisionNum; /**< Vendor specific revision number */
    u8 VendorSpecific[88]; /**< Vendor specific */
    u16 Crc;               /**< Integrity CRC */
#ifdef __ICCARM__
};
#pragma pack(pop)
#else
} __attribute__(packed);
#endif

/* Nand timings */
struct nand_sdr_timings {
    /* general timings */
    float tADL_min;
    float tCEH_min;
    float tCS_min;
    float tFEAT_max;
    float tITC_max;
    float tRR_min;
    float tWHR_min;
    float tWB_max;
    float tWW_min;
    /* sdr timings */
    float tRC_min;
    float tREA_max;
    float tREH_min;
    float tRHW_min;
    float tRHZ_max;
    float tRP_min;
    float tWC_min;
    float tWH_min;
    float tWP_min;
};

struct nand_nvddr_timings {
    /* general timings */
    float tADL_min;
    float tCEH_min;
    float tCS_min;
    float tFEAT_max;
    float tITC_max;
    float tRR_min;
    float tWHR_min;
    float tWB_max;
    float tWW_min;
    /* nvddr timings */
    float tCK_avg;
    float tCAD_min;
    float tDQSCK_max;
    float tDQSCK_min;
    float tRHW_min;
    float tWRCK_min;
    float tDQSD_max;
    float tDQSHZ_max;
};

struct nand_nvddr23_timings {
    /* general timings */
    float tADL_min;
    float tCEH_min;
    float tCS_min;
    float tFEAT_max;
    float tITC_max;
    float tRR_min;
    float tWHR_min;
    float tWB_max;
    float tWW_min;
    /* NVDDR2/3 timings */
    float tRC_min;
};

struct nand_timings {
    unsigned int mode;
    union {
        struct nand_sdr_timings sdr;
        struct nand_nvddr_timings nvddr;
        struct nand_nvddr23_timings nvddr23;
    };
};

/* Nand Interface Timmings */
struct nand_interface_config {
    int type;
    struct nand_timings timings;
};

/* Nand op functions */
struct nand_ctrl_ops {
    int (*setup_interface)(FNandPsu_T *nfcPtr,
                           struct nand_interface_config *iface);
    int (*exec_op)(FNandPsu_T *nfcPtr, struct nand_operation *ops);
    int (*reset)(FNandPsu_T *nfcPtr, int cs);
    int (*set_feature)(FNandPsu_T *nfcPtr, int cs, u8 feature, void *data);
    int (*erase)(FNandPsu_T *nfcPtr, int cs, unsigned int page,
                 unsigned int len);
    int (*write_page)(FNandPsu_T *nfcPtr, int cs, unsigned int page,
                      unsigned int offset, void *buf, unsigned int len,
                      int raw);
    int (*read_page)(FNandPsu_T *nfcPtr, int cs, unsigned int page,
                     unsigned int offset, void *buf, unsigned int len, int raw);
    int (*copyback)(FNandPsu_T *nfcPtr, int cs, u32 src_page, u32 dst_page,
                    u8 npages);
};

struct nand_device {
    u32 options;

    struct nand_model model;
    struct nand_feature feat;

    /* Data interface */
    struct nand_interface_config *cur_iface;
    struct nand_interface_config *best_iface;

    /* ECC */
    struct nand_ecc ecc;

    /* Bad block information */
    u32 bbt_options;
    struct nand_bbt_desc bbt_desc;        /* Bad block table descriptor */
    struct nand_bbt_desc bbt_mirror_desc; /* Mirror BBT descriptor */
    struct nand_bb_pattern bb_pattern;    /* Bad block pattern to search */
    u8 *bb_info;

    /* This buffers are used internal */
    u8 *data_buf; /* buf size should include main + oob */
    u8 *oob_poi;  /* oob position in data_buf */

    /* Used for OOB read/write */
    u8 *oob_buf;
    unsigned int ooblen;

    /* Externals */
    void *ctrl;
};

/******************************Function Prototype***************************/
/**
 * FNandPsu_Scan - Scan a nand device
 *
 * This function scan the nand device to detect the num of
 * target as well as the device information.
 * It must be used before controller configuration which needs flash
 * information.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_Scan(FNandPsu_T *nfcPtr);

/**
 * FNandPsu_ScanTail - Complete a nand device scan
 *
 * This function finish the nand device scan, including
 * setup interface to its best performance and scan bbt.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_ScanTail(FNandPsu_T *nfcPtr);

/**
 * FNandPsu_ResetInterface - Reset interface of selected chip
 *
 * This function must be used before sends a RESET command.
 * Setup controller interface and timing to SDR mode.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_ResetInterface(FNandPsu_T *nfcPtr);

int FNandPsu_ChooseBestIface(FNandPsu_T *nfcPtr, u8 mask,
                             struct nand_interface_config *ext_iface);
/**
 * FNandPsu_SetupInterface - Setup interface of selected chip to
 * its best performace.
 *
 * This function is used during nand initialize. The best interface
 * is read from parameter page.
 * Setup controller interface and timing according to the best interface.
 *
 * @nfcPtr: The NAND controller
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_SetupInterface(FNandPsu_T *nfcPtr);

/**
 * FNandPsu_Reset_Op - Reset Nand operation
 *
 * This function sends a RESET command and puts the target
 * in its default power-up state and places the target in
 * the SDR data interface if device is in NVDDR/NVDDR2 and
 * remain its data interface if device is in NVDDR3.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_Reset_Op(FNandPsu_T *nfcPtr, int cs);

/**
 * FNandPsu_ReadId_Op - Read ID operation
 *
 * This function sends a READID command and reads back the ID returned by the
 * NAND. ID will be read twice in NVDDR/NVDDR2/NVDDR3
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @addr: address cycle to pass after the READID command
 * @buf: buffer used to store the ID
 * @len: length of the buffer
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_ReadId_Op(FNandPsu_T *nfcPtr, int cs, u8 addr, void *buf,
                       unsigned int len);

/**
 * FNandPsu_ParamPage_Op - Read Parameter page operation
 *
 * This function sends a READ_PARAMETER_PAGE command and
 * reads back the parameter page. A minimum of three copies of the
 * parameter page are stored in the device. Each parameter page is
 * 256 bytes.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @page: page cycle to pass after the READ_PARAMETER_PAGE command
 * @buf: buffer used to store the parameter page
 * @len: length of the buffer
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_ParamPage_Op(FNandPsu_T *nfcPtr, int cs, u8 page, void *buf,
                          unsigned int len);

/**
 * FNandPsu_SetFeature_Op - Set Feature operation
 *
 * This function sends a SET_FEATURE command and modifies
 * the settings of a particular feature. Each data byte is
 * transmitted twice in NVDDR/NVDDR2/NVDDR3. When changing
 * the timing mode, the device is busy for tITC, not tFEAT.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @feature: feature cycle to pass after the SET_FEATURE command
 * @data: feature data
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_SetFeature_Op(FNandPsu_T *nfcPtr, int cs, u8 feature, void *data);

/**
 * FNandPsu_GetFeature_Op - Get Feature operation
 *
 * This function sends a GET_FEATURE command and get
 * the settings of a particular feature. Each data byte is
 * transmitted twice in NVDDR/NVDDR2/NVDDR3.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @feature: feature cycle to pass after the SET_FEATURE command
 * @data: feature data
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_GetFeature_Op(FNandPsu_T *nfcPtr, int cs, u8 feature, void *data);

/**
 * FNandPsu_Status_Op - Get Status operation
 *
 * This function sends a READ_STATUS command and get
 * the status of selected target. Each data byte is
 * transmitted twice in NVDDR/NVDDR2/NVDDR3.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @status: status data
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_Status_Op(FNandPsu_T *nfcPtr, int cs, u8 *status);

/**
 * FNandPsu_Erase_Op - Erase nand operation
 *
 * This function sends a BLOCK_ERASE command and erase the block
 * in selected target.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @eraseblock: block num to erase
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_Erase_Op(FNandPsu_T *nfcPtr, int cs, unsigned int eraseblock);

/**
 * FNandPsu_WritePage_Op - Write page operation
 *
 * This function sends a PAGE_PROG command and write the page
 * in selected target.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @page: page num in selected target
 * @offset: offset in the page
 * @buf: data buffer in memory
 * @len: bytes to write, must align to blocksize
 * @raw: do not use ecc
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_WritePage_Op(FNandPsu_T *nfcPtr, int cs, unsigned int page,
                          unsigned int offset, void *buf, unsigned int len,
                          u8 raw);

/**
 * FNandPsu_ReadPage_Op - Read page operation
 *
 * This function sends a PAGE_READ command and read the page
 * in selected target.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @page: page num in selected target
 * @offset: offset in the page
 * @buf: data buffer in memory
 * @len: bytes to write, must align to blocksize
 * @raw: do not use ecc
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_ReadPage_Op(FNandPsu_T *nfcPtr, int cs, unsigned int page,
                         unsigned int offset, void *buf, unsigned int len,
                         u8 raw);

/**
 * FNandPsu_ChangeReadColumn_Op - Change column address and read data
 *
 * This function sends a Change Column command and read the page
 * in selected target.
 * This function always reads raw data.
 *
 * @nfcPtr: The NAND controller
 * @cs: cs to select target
 * @offset: offset in the page
 * @buf: data buffer in memory
 * @len: bytes to write, must align to blocksize
 * @force_8bit: use 8bits io
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_ChangeReadColumn_Op(FNandPsu_T *nfcPtr, int cs,
                                 unsigned int offset, void *buf,
                                 unsigned int len, int force_8bit);

/**
 * FNandPsu_TranslateFlashAddress  - Translate address to cs, page and offset
 *
 * This function is used to translate address to cs, page and offset.
 * if cs, page or offset is not desired to be translated, the pointer can be
 *NULL. if cs pointer is NULL, page will be the page_num in all device,
 *otherwise it will be the num in certain target.
 *
 * @nfcPtr: The NAND controller
 * @addr: flash address
 * @cs: chip select in device
 * @page: page num
 * @offset: offset in the page
 *
 * Returns 0 on success, a negative error code otherwise.
 **/
int FNandPsu_TranslateFlashAddress(FNandPsu_T *nfcPtr, u64 addr, u32 *cs,
                                   u32 *page, u32 *offset);

/**
 * FNandPsu_NoSkip_Erase - Erase Nand
 *
 * This function write data to nand from memory.
 *
 * @nfcPtr: The NAND controller
 * @addr: nand device address, must align to blocksize
 * @len: bytes to write, must align to blocksize
 *
 * Returns 0 on success, a positive left len, a negative error code.
 */
int FNandPsu_NoSkip_Erase(FNandPsu_T *nfcPtr, u64 addr, int len);

/**
 * FNandPsu_NoSkip_Write - Write Nand
 *
 * This function write data to nand from memory.
 * FNandPsu_SetOobBuf must be used if use NAND_OP_OOBREQ flag
 *
 * @nfcPtr: The NAND controller
 * @addr: nand device address
 * @len: bytes to write
 * @buf: data buffer in memory
 * @flags: can be set to the value below
 *      NAND_OP_RAW - do not use ecc
 *      NAND_OP_OOBREQ - write data from device->oob_buf
 *          with device->ooblen size to oob area in flash
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_NoSkip_Write(FNandPsu_T *nfcPtr, u64 addr, int len, u8 *buf,
                          u32 flags);

/**
 * FNandPsu_NoSkip_Read - Read Nand
 *
 * This function read data to memory from nand.
 * FNandPsu_SetOobBuf must be used if use NAND_OP_OOBREQ flag
 *
 * @nfcPtr: The NAND controller
 * @addr: nand device address
 * @len: bytes to read
 * @buf: data buffer in memory
 * @flags: can be set to the value below
 *      NAND_OP_RAW - do not use ecc
 *      NAND_OP_OOBREQ - read data to device->oob_buf
 *          with device->ooblen size from oob area in flash
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_NoSkip_Read(FNandPsu_T *nfcPtr, u64 addr, int len, u8 *buf,
                         u32 flags);

/**
 * FNandPsu_NoSkip_WriteOob - Write OOB Area
 *
 * This function write oob area from nand.
 *
 * @nfcPtr: The NAND controller
 * @page: nand device page to write
 * @len: bytes to write
 * @buf: data buffer in memory
 * @raw: do not use ecc
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_NoSkip_WriteOob(FNandPsu_T *nfcPtr, u32 page, int len, u8 *buf,
                             int raw);

/**
 * FNandPsu_NoSkip_ReadOob - Read OOB Area
 *
 * This function read oob area from nand.
 *
 * @nfcPtr: The NAND controller
 * @page: nand device page to read
 * @len: bytes to read
 * @buf: data buffer in memory
 * @raw: do not use ecc
 *
 * Returns 0 on success, a negative error code otherwise.
 */
int FNandPsu_NoSkip_ReadOob(FNandPsu_T *nfcPtr, u32 page, int len, u8 *buf,
                            int raw);

int FNandPsu_Device_Init(FNandPsu_T *nfcPtr, FNandPsu_UserCfg_T *usercfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* prevent circular inclusions */
