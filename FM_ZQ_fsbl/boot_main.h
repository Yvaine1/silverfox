/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  boot_main.h
 *
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/28/2022  First Release.
 *</pre>
 ******************************************************************************/
#ifndef _BOOT_MAIN_H_
#define _BOOT_MAIN_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "bspconfig.h"
#include "fmsh_aes_sm4.h"
#include "fmsh_authentication.h"
#include "fmsh_board.h"
#include "fmsh_common.h"
#include "fmsh_csudma.h"
#include "fmsh_devc_private.h"
#include "fmsh_devc_public.h"
#include "fmsh_dma.h"
#include "fmsh_efuse.h"
#include "fmsh_error.h"
#include "fmsh_fnand.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"
#include "fmsh_header.h"
#include "fmsh_hpnfc_hw.h"
#include "fmsh_hw.h"
#include "fmsh_pmu_interactive.h"
#include "fmsh_psu_parameters.h"
#include "fmsh_qspi.h"
#include "fmsh_sac.h"
#include "fmsh_sd.h"
#include "fmsh_secure_rsa.h"
#include "fmsh_sha.h"
#include "fmsh_uart_common.h"
#include "fmsh_uart_logout.h"
#include "fmsh_watchdog.h"
#include "fsbl_config.h"
#include "psu_init.h"
#include "sdmmc_fatfs.h"
#include "fmsh_ipi.h"
#include "dfu_app.h"
  
/************************** Constant Definitions *****************************/
#ifdef DDRPS_0_DEVICE_ID
#define FSBL_PS_DDR
#endif

#if defined(CORTEX_A53)
#define FSBL_PS_DDR_START_ADDRESS        (0x00000000U)
#define FSBL_PS_DDR_END_ADDRESS          (0x7FFFFFFFU)
#define FSBL_PS_HI_DDR_START_ADDRESS     (0x800000000U)
#define FSBL_PS_HI_DDR_END_ADDRESS       (0x1000000000U)    
#else
#define FSBL_PS_DDR_START_ADDRESS        (0x100000U)
#define FSBL_PS_DDR_END_ADDRESS          (0x7FFFFFFFU)
#endif

#define FSBL_DUMMY_PL_ADDR               (0xFFFFFFFFU)

/* PMU RAM address for PMU FW */
#define FSBL_PMU_RAM_START_ADDRESS       (0xFFDC0000U)
#define FSBL_PMU_RAM_END_ADDRESS         (0xFFDDFFFFU)

#define FSBL_PS_OCM_START_ADDRESS        (0xFFFC0000U)
#define FSBL_PS_OCM_END_ADDRESS          (0xFFFFFFFFU)

/**
 * TCM address for R5
 */
#define FSBL_R5_TCM_START_ADDRESS        (u32)(0x0U)
#define FSBL_R5_BTCM_START_ADDRESS       (0x20000U)

#define FSBL_R50_HIGH_ATCM_START_ADDRESS (0xFFE00000U)
#define FSBL_R50_HIGH_BTCM_START_ADDRESS (0xFFE20000U)
#define FSBL_R51_HIGH_ATCM_START_ADDRESS (0xFFE90000U)
#define FSBL_R51_HIGH_BTCM_START_ADDRESS (0xFFEB0000U)

#define FSBL_R5_TCM_BANK_LENGTH          (0x10000U)

/* Reset Reason */
#define PS_ONLY_RESET                    0x1U

#define REBOOT_STATUS_OFFSET             0x0258
#define REBOOT_POR                       0x00400000
#define REBOOT_NON_POR                   0x00600000

#define REBOOT_STATUS_REG                (FPS_SLCR_BASEADDR + 0x400)
#define BOOT_MODE_REG                    (FPS_SLCR_BASEADDR + 0x404U)
#define MULTI_BOOT_REG                   (FPS_CSU_BASEADDR + 0x38)

#define APU_RVBARADDR0                   (FPS_SLCR_BASEADDR + 0xA40)
#define APU_RVBARADDR1                   (FPS_SLCR_BASEADDR + 0xA48)
#define APU_RVBARADDR2                   (FPS_SLCR_BASEADDR + 0xA50)
#define APU_RVBARADDR3                   (FPS_SLCR_BASEADDR + 0xA58)

#define APU_RST_CTRL                     (FPS_SLCR_BASEADDR + 0x380)
#define APU0_RST_SHIFT                   (0U)
#define APU1_RST_SHIFT                   (1U)
#define APU2_RST_SHIFT                   (2U)
#define APU3_RST_SHIFT                   (3U)
#define APU0_PWR_RST_SHIFT               (10U)
#define APU1_PWR_RST_SHIFT               (11U)
#define APU2_PWR_RST_SHIFT               (12U)
#define APU3_PWR_RST_SHIFT               (13U)

#define FSBL_IVT_LENGTH                  (u32)(0x40U)
#define FSBL_R5_HIVEC                    (u32)(0xffff0000U)
#define FSBL_R5_LOVEC                    (u32)(0x0U)

/* Pattern to be filled for DDR ECC Initialization */
#define FSBL_ECC_INIT_VAL_WORD           0xDEADBEEFU

#define FSBL_R50_TCM_ECC_INIT_STATUS     0x00000001U
#define FSBL_R51_TCM_ECC_INIT_STATUS     0x00000002U

/* R5 vectors value*/
#define FSBL_R5_LOVEC_VALUE              (0xEAFEFFFEU)
#define FSBL_R5_HIVEC_VALUE              (0xEAFF3FFEU)

/*
 * FSBL processor reporting to PMU
 */
#define FSBL_RUNNING_ON_A53              (0x1U)
#define FSBL_RUNNING_ON_R5_0             (0x2U)
#define FSBL_RUNNING_ON_R5_L             (0x3U)
#define FSBL_STATE_PROC_SHIFT            (0x1U)

#define FSBL_STATE_PROC_INFO_MASK        (0x3U << FSBL_STATE_PROC_SHIFT)
#define FSBL_FSBL_ENCRYPTED_MASK         (0x8U)

#define PCAP_UNLOCK_OFFSET               0x30

/* PS reset control register define*/
#define PS_RST_REASON_MASK               0x00600000 /**< PS software reset */

/*
 * SLCR BOOT Mode Register defines
 */
#define BOOT_MODES_MASK                  0x00000007 /**< FLASH types */

/* Boot Modes */
#define JTAG_BOOT_MODE                   (0x00000000U)
#define QSPI24_BOOT_MODE                 (0x00000001U)
#define QSPI32_BOOT_MODE                 (0x00000002U)
#define SD0_BOOT_MODE                    (0x00000003U)
#define NAND_BOOT_MODE                   (0x00000004U)
#define SD1_BOOT_MODE                    (0x00000005U)
#define EMMC_BOOT_MODE                   (0x00000006U)
#define USB0_BOOT_MODE                   (0x00000007U)
#define SD1_LS_BOOT_MODE                 (0x0000000EU)

#define FSBL_SD_DRV_NUM_0                0U
#define FSBL_SD_DRV_NUM_1                1U
#define FSBL_BASE_FILE_NAME_LEN_SD_0     8
#define FSBL_BASE_FILE_NAME_LEN_SD_1     11
#define FSBL_NUM_DIGITS_IN_FILE_NAME     4

/*Boot Device*/
#define JTAG                             (0x00000000U) /**< JTAG Boot Mode */
#define QSPI_FLASH                       (0x00000001U) /**< QSPI Boot Mode */
#define NAND_FLASH                       (0x00000003U) /**< NAND Boot Mode */
#define SD_CARD                          (0x00000004U) /**< SD Boot Mode */
#define USB_CON                          (0x00000005U) /**< USB Boot Mode */
  
#define BOOT_STATUS_JTAG                 0x3U

#define MULTIBOOT_ADDR_MASK     0x1FFF

#define Fmsh_In8(Addr)          *(volatile u8 *)(Addr)
#define Fmsh_Out8(Addr, Value)  *(volatile u8 *)(Addr) = Value

#define Fmsh_In32(Addr)         *(volatile u32 *)(Addr)
#define Fmsh_Out32(Addr, Value) *(volatile u32 *)(Addr) = Value

#define ReadReg(Addr)           *(volatile u32 *)(Addr)
#define WriteReg(Addr, Value)   *(volatile u32 *)(Addr) = Value

/**
 * BOOT stages definition
 */
#define BOOT_STAGE0             (0x0U)
#define BOOT_STAGE1             (0x1U)
#define BOOT_STAGE2             (0x2U)
#define BOOT_STAGE3             (0x3U)
#define BOOT_STAGE4             (0x4U)
#define BOOT_STAGE_ERR          (0x5U)
#define BOOT_STAGE_DEFAULT      (0x6U)

/*Boot header research */
/*Research boundry of boot devices*/
#define NAND_LIMITATION         (128 * 1024 * 1024)
#define QSPI_24B_LIMITATION     (16 * 1024 * 1024)
#define QSPI_32B_LIMITATION     (256 * 1024 * 1024)
#define SD_LIMITATION           (256 * 1024 * 1024)
#define USB_LIMITATION          (0)
#define SEARCH_STEP_SIZE        0x8000

/* FSBL exit definition */
#define FSBL_NO_HANDOFFEXIT     (0x00U)
#define FSBL_HANDOFFEXIT        (0x01U)
#define FSBL_HANDOFFEXIT_32     (0x02U)

#define FSBL_RUNNING            (0xFFFFU)
#define FSBL_COMPLETED          (0x0U)

/* Different Memory types */
#define FSBL_R5_0_TCM           (0x1U)
#define FSBL_R5_1_TCM           (0x2U)
#define FSBL_R5_L_TCM           (0x3U)

/* Reset Reason */
#define FSBL_SYSTEM_RESET       0U
#define FSBL_MASTER_ONLY_RESET  2U

/**
 * Definition for NAND to be included
 */
#if (!defined(FSBL_NAND_EXCLUDE) && defined(NANDPS_0_DEVICE_ID))
#define FSBL_NAND
#endif

/**
 * Definition for QSPI to be included
 */
#if (!defined(FSBL_QSPI_EXCLUDE) && defined(QSPIPS_0_DEVICE_ID))
#define FSBL_QSPI
#endif

/**
 * Definitions for SD to be included
 */
#if (!defined(FSBL_SD_EXCLUDE) && defined(SDMMCPS_0_DEVICE_ID))
#define FSBL_SD_0
#endif

#if (!defined(FSBL_SD_EXCLUDE) && defined(SDMMCPS_1_DEVICE_ID))
#define FSBL_SD_1
#endif

/**
 * Definition for SECURE to be included
 */
#if !defined(FSBL_SECURE_EXCLUDE)
#define FSBL_SECURE
#endif

/**
 * Definition for PL bitsream feature to be included
 */
#if !defined(FSBL_BS_EXCLUDE)
#define FSBL_BS
#endif

/**
 * Definition for early handoff feature to be included
 */
#if !defined(FSBL_EARLY_HANDOFF_EXCLUDE)
#define FSBL_EARLY_HANDOFF
#endif

/**
 * Definition for WDT to be included
 */
#if (!defined(FSBL_WDT_EXCLUDE) && defined(XPAR_PSU_WDT_0_DEVICE_ID))
#define FSBL_WDT_ENABLE 1
#define FSBL_WDT_TOP    15
#define FSBL_WDT_MASK   PMU_GLOBAL_ERROR_SRST_EN_1_LPD_SWDT_MASK
#else
#define FSBL_WDT_ENABLE 0
#define FSBL_WDT_TOP    15
#endif

/* Definition for PL config force */
#if !defined(FSBL_PL_SKIP_EXCLUDE)
#define FSBL_PL_SKIP_CONFIGED
#endif

#if (!defined(FSBL_USB_EXCLUDE) && defined(USBPS_0_DEVICE_ID) && \
     defined(FSBL_PS_DDR))
#define FSBL_USB
#endif

/**************************** Type Definitions *******************************/

typedef struct {
    u32 DeviceBaseAddress; /**< Flash device base address */
    u32 (*DeviceInit)(u32 DeviceFlags);
    /**< Function pointer for Device initialization code */
    u32 (*DeviceCopy)(u32 SrcAddress, u32 DestAddress, u32 Length);
    /**< Function pointer for device copy */
    u32 (*DeviceRelease)();
    /**< Function pointer for device release */
} Ps_DeviceOps;

/**
 * This stores the handoff Address of the different cpu's
 */
typedef struct {
    u32 CpuSettings;
    u32 PartitionAttributes;
    u64 HandoffAddress;
} Ps_HandoffValues;

/**
 * This is FSBL instance pointer. This stores all the information
 * required for FSBL
 */
typedef struct {
    float Version;        /**<  Version */
    u32 ProcessorID;      /**< One of R5-0, R5-LS, A53-0 */
    u32 A53ExecState;     /**< One of A53 64-bit, A53 32-bit */
    u32 HandoffCpuNo;     /**< Number of CPU's FSBL will handoff to */
    u32 SecureModeFlag;   
    u32 OcmrunningFlag;   /**< Number of CPU's FSBL will handoff to */
    u32 TcmEccInitStatus; /**< Bits 0, 1 indicate TCM ECC Init status */
    u32 ResetReason;
    u32 SearchRange;
    u32 XipMode;
    u32 EncryptionStatus;
    u32 EncrytionAlgorithm;
    u32 BootHdrAttributes;      /**< Boot Header attributes */
    u32 ImageOffsetAddress;     /**< Flash offset address */
    Ps_ImageHeader ImageHeader; /** Image header */
    u32 ErrorCode;              /**< Error code during FSBL failure */
    u32 PrimaryBootDevice;      /**< Primary boot device used for booting  */
    u32 SecondaryBootDevice;    /**< Secondary boot device in image header*/
    u32 BootMode;               /**< Primary boot device used for booting  */
    Ps_DeviceOps DeviceOps;     /**< Device operations for bootmodes */
    Ps_HandoffValues
        HandoffValues[10];      /**< Handoff address for different CPU's  */
} BootPs;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern BootPs BootInstance;
extern u64 gtc_count0, gtc_count1;
extern double gtc_time;

/************************** Function Prototypes ******************************/
u32 FGicPs_SelfTest(FGicPs *InstancePtr);

// fmsh_initialization.c
u32 FmshFsbl_BootInitialize(BootPs *BootInstancePtr);
u32 FmshFsbl_BootDeviceInitAndValidate(BootPs *BootInstancePtr);
u32 BootHeaderSearchAndValidate(BootPs *BootInstancePtr);
u32 FmshFsbl_TcmEccInit(BootPs *BootInstancePtr, u32 CpuId);

// fmsh_partition_load.c
u32 FmshFsbl_PowerUpIsland(u32 PwrIslandMask);
u32 FmshFsbl_PowerUpMemory(u32 MemoryType);
u32 FmshFsbl_PartitionLoad(BootPs *BootInstancePtr, u32 PartitionNum);

// boothandoff.c
void ErrorLockDown(void);
u32 FmshFsbl_CheckEarlyHandoff(BootPs *BootInstancePtr, u32 PartitionNum);
u32 BootHandoff(const BootPs *BootInstancePtr, u32 PartitionNum,
                u32 EarlyHandoff);
void DefaultHandoffExit();
void FmshFsbl_HandoffExit(UINTPTR HandoffAddress, u32 Flag);

// fmsh_header.c
u32 FmshFsbl_ValidateHeader(BootPs *BootInstancePtr);
u32 FmshFsbl_PartitionHeaderValidation(BootPs *BootInstance, u32 PartitionNum);
u32 FmshFsbl_ReadImageHeader(Ps_ImageHeader *ImageHeader,
                             Ps_DeviceOps *DeviceOps,
                             u32 FlashImageOffsetAddress, u32 RunningCpu);

void FmshFsbl_MarkUsedRPUCores(BootPs *BootInstance, u32 PartitionNum);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
