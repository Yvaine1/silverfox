/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_header.h
 *
 * This file contains header boot_main.h
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/28/2022  First Release
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_HEADER_H_
#define _FMSH_HEADER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/
/*Maximum partition number*/
#define IH_MIN_PARTITIONS                   (1U)
#define IH_MAX_PARTITIONS                   (15U)
#define IH_PARTITION_WORD_LENGTH            (0x4U)

#define QSPI_WIDTH_DETECT_WORD              0xAA995566

/**
 * Boot header field offset
 */
#define IH_BH_OFFSET                        (0x20U)
#define IH_BH_ENC_STS_OFFSET                (0x28U)
#define IH_BH_FSBL_ORIGINAL_LENGTH_OFFSET   (0x3CU)

#define IH_BH_IMAGE_ATTRB_OFFSET            (0x44U)
#define IH_BH_ENCRYPTED_KEY_OFFSET          (0x4CU)
#define IH_BH_ENCRYPTED_KEY_PARITY_OFFSET   (0x6CU)
#define IH_BH_ENCRYPTED_KEY_IV_OFFSET       (0xACU)
#define IH_BH_SECURE_HEADER_IV_OFFSET       (0xA0U)
#define IH_BH_WIDTH_DETECTION               0x20U
#define IH_BH_ENC_ALG_OFFSET                (0x84U)

#define IH_BH_PUF_HELP_DATA_OFFSET          (0x8B8U)
#define IH_BH_IH_TABLE_OFFSET               (0x98U)
#define IH_BH_PH_TABLE_OFFSET               (0x9CU)

#define IH_BH_IMAGE_ATTRB_RSA_MASK          (0xC000U)
#define IH_BH_IMAGE_ATTRB_SHA2_MASK         (0x3000U)
#define IH_BH_IMAGE_ATTRB_AUTH_ONLY_MASK    (0x0030U)
#define IH_BH_IMAGE_ATTRB_OPKEY_MASK        (0x0000000C)
#define IH_BH_IV_OFFSET                     (0xA0U)
#define IH_BH_IV_LENGTH                     (0x10U)

#define EFUSE_RED_KEY_ENC                   0x5A69C387
#define XLNX_EFUSE_RED_KEY_ENC              0xA5C3C5A3
#define EFUSE_OBFUSCATED_KEY_ENC            0x5A8769C3
#define XLNX_EFUSE_OBFUSCATED_KEY_ENC       0xA5C3C5A7

#define EFUSE_BLACK_KEY_ENC                 0x5AC38769
#define BOOT_HEADER_OBFUSCATED_KEY_ENC      0x55AA6699
#define XLNX_BOOT_HEADER_OBFUSCATED_KEY_ENC 0xA35C7CA5
#define XLNX_BOOT_HEADER_BLACK_KEY_ENC      0xA35C7C53

#define BOOT_HEADER_BLACK_KEY_ENC           0xA55A9669
#define XLNX_EFUSE_BLACK_KEY_ENC            0xA5C3C5A5

/**
 * Defines for length of the headers
 */
#define IH_FIELD_LEN                        (4U)
#define IH_PFW_LEN_FIELD_LEN                (4U)
#define IH_IHT_LEN                          (64U)
#define IH_PH_LEN                           (64U)

/*Defines for register initialization pairs*/
#define REG_INITIALIZATION_LEN              (2048U)
#define REG_INITIALIZATION_OFFSET           (0xB8U)

/**
 * Image header table field offsets
 */
#define IH_IHT_VERSION_OFFSET               (0x0U)
#define IH_IHT_NO_OF_PARTITONS_OFFSET       (0x4U)
#define IH_IHT_PH_ADDR_OFFSET               (0x8U)
#define IH_IHT_AC_OFFSET                    (0x10U)
#define IH_IHT_PPD_OFFSET                   (0x14U)
#define IH_IHT_CHECKSUM_OFFSET              (0x3CU)

#define IH_PH_ATTRB_VECTOR_LOCATION_SHIFT   (23U)

/**
 * Partition Attribute fields
 */
#define IH_PH_ATTRB_VEC_LOCATION_MASK       (0x800000U)
#define IH_PH_ATTR_BLOCK_SIZE_MASK          (0x700000U)
#define IH_PH_ATTRB_ENDIAN_MASK             (0x40000U)
#define IH_PH_ATTRB_PART_OWNER_MASK         (0x30000U)
#define IH_PH_ATTRB_RSA_SIGNATURE_MASK      (0x8000U)
#define IH_PH_ATTRB_CHECKSUM_MASK           (0x7000U)
#define IH_PH_ATTRB_DEST_CPU_MASK           (0x0F00U)
#define IH_PH_ATTRB_ENCRYPTION_MASK         (0x0080U)
#define IH_PH_ATTRB_DEST_DEVICE_MASK        (0x0070U)
#define IH_PH_ATTRB_A53_EXEC_ST_MASK        (0x0008U)
#define IH_PH_ATTRB_TARGET_EL_MASK          (0x0006U)
#define IH_PH_ATTRB_TR_SECURE_MASK          (0x0001U)

/* BLOCK SIZE multiplier */
#define FSBL_MUL_MEGABYTES                  (1024U * 1024U)

#define ATTRB_DEST_CPU_NONE                 (0x0000U)
#define ATTRB_DEST_CPU_A53_0                (u32)(0x100U)
#define ATTRB_DEST_CPU_A53_1                (u32)(0x200U)
#define ATTRB_DEST_CPU_A53_2                (u32)(0x300U)
#define ATTRB_DEST_CPU_A53_3                (u32)(0x400U)
#define ATTRB_DEST_CPU_R5_0                 (u32)(0x500U)
#define ATTRB_DEST_CPU_R5_1                 (u32)(0x600U)
#define ATTRB_DEST_CPU_R5_L                 (u32)(0x700U)
#define ATTRB_DEST_CPU_PMU                  (u32)(0x800U)

#define ATTRB_A53_EXEC_ST_AA32              (u32)(0x0008U)
#define ATTRB_A53_EXEC_ST_AA64              (u32)(0x0000U)
#define ATTRB_INVALID_EXEC_ST               (u32)(0xFFFFU)

/**
 * Partition Present Devices(PPD) in IHT
 */
#define IH_IHT_PPD_SAME                     (0x0U)
#define IH_IHT_PPD_QSPI32                   (0x1U)
#define IH_IHT_PPD_QSPI24                   (0x2U)
#define IH_IHT_PPD_NAND                     (0x3U)
#define IH_IHT_PPD_SD_0                     (0x4U)
#define IH_IHT_PPD_SD_1                     (0x5U)
#define IH_IHT_PPD_SD_LS                    (0x6U)
#define IH_IHT_PPD_MMC                      (0x7U)
#define IH_IHT_PPD_USB                      (0x8U)
#define IH_IHT_PPD_ETHERNET                 (0x9U)
#define IH_IHT_PPD_PCIE                     (0xAU)
#define IH_IHT_PPD_SATA                     (0xBU)

/**
 * Partition Attribute Values
 */
#define IH_AC_ATTRB_PPK_SELECT_MASK         (u32)(0x30000U)
#define IH_PH_ATTRB_ENCRYPTION              (u32)(0x80U)

/**
 * Partition Attribute Values
 */
#define IH_PH_ATTRB_PART_OWNER_FSBL         (0x00000U)
#define IH_PH_ATTRB_PART_OWNER_UBOOT        0x10000
#define IH_PH_ATTRB_RSA_SIGNATURE           (0x8000U)
#define IH_PH_ATTRB_NOCHECKSUM              (0x0000U)
#define IH_PH_ATTRB_CHECKSUM_MD5            (0x1000U)
#define IH_PH_ATTRB_HASH_SHA2               (0x2000U)
#define IH_PH_ATTRB_HASH_SHA3               (0x3000U)

#define IH_PH_ATTRB_DEST_CPU_NONE           (0x0000U)
#define IH_PH_ATTRB_DEST_CPU_A53_0          (u32)(0x0100U)
#define IH_PH_ATTRB_DEST_CPU_A53_1          (u32)(0x0200U)
#define IH_PH_ATTRB_DEST_CPU_A53_2          (u32)(0x0300U)
#define IH_PH_ATTRB_DEST_CPU_A53_3          (u32)(0x0400U)
#define IH_PH_ATTRB_DEST_CPU_R5_0           (u32)(0x0500U)
#define IH_PH_ATTRB_DEST_CPU_R5_1           (u32)(0x0600U)
#define IH_PH_ATTRB_DEST_CPU_R5_L           (u32)(0x0700U)
#define IH_PH_ATTRB_DEST_CPU_PMU            (u32)(0x0800U)

#define IH_PH_ATTRB_DEST_DEVICE_NONE        (u32)(0x0000U)
#define IH_PH_ATTRB_DEST_DEVICE_PS          (u32)(0x0010U)
#define IH_PH_ATTRB_DEST_DEVICE_PL          (u32)(0x0020U)
#define IH_PH_ATTRB_DEST_DEVICE_PMU         (u32)(0x0030U)

#define IH_PH_ATTRB_A53_EXEC_ST_AA32        (u32)(0x0008U)
#define IH_PH_ATTRB_A53_EXEC_ST_AA64        (u32)(0x0000U)

/**
 * Below is the bit mapping of fields in the ATF Handoff parameters
 * with that of Partition header. The number of bits shifted is
 * is based on the difference between these two offsets
 *
 *                   ATFHandoffParams	PartitionHeader         Shift
 *     Parameter     PartitionFlags     PartitionAttributes   difference
 * ----------------------------------------------------------------------
 *	Exec State            0			         3                  3 right
 *	ENDIAN	              1	                 18                 17 right
 *	SECURE                2                  0                  2 left
 *	EL                    3:4                1:2                2 left
 *	CPU_A53               5:6                8:10
 */
#define IH_ATTRB_VECTOR_LOCATION_SHIFT      (23U)
#define IH_ATTRB_BLOCK_SIZE_SHIFT           (20U)
#define IH_ATTRB_A53_EXEC_ST_SHIFT_DIFF     (3U)
#define IH_ATTRB_ENDIAN_SHIFT_DIFF          (17U)
#define IH_ATTRB_TR_SECURE_SHIFT_DIFF       (2U)
#define IH_ATTRB_TARGET_EL_SHIFT_DIFF       (2U)

#define IH_PART_FLAGS_DEST_CPU_A53_MASK     (0x60U)
#define IH_PART_FLAGS_DEST_CPU_A53_0        (u32)(0x00U)
#define IH_PART_FLAGS_DEST_CPU_A53_1        (u32)(0x20U)
#define IH_PART_FLAGS_DEST_CPU_A53_2        (u32)(0x40U)
#define IH_PART_FLAGS_DEST_CPU_A53_3        (u32)(0x60U)

/***************** Macros (Inline Functions) Definitions *********************/

#define FMSH_IMAGE_ID                       0x464d5348
/*
 * linear memory address
 */
#define QSPI_LINEAR_BASE_ADDRESS_START      FPS_QSPI0_D_BASEADDR
#define QSPI_LINEAR_BASE_ADDRESS_END        (FPS_QSPI0_D_BASEADDR + 0x0FFFFFFFU)

/************************** Variable Definitions *****************************/
typedef struct StructBootHeader {
    //	u32 VectorTable[8];	/* 0x0~0x1F */
    u32 WidthDetection;   /* 0x20 */
    u32 ImageId;          /* 0x24 */
    u32 EncryptionStatus; /* 0x28 */
    u32 SExecutionAddr;   /* 0x2C */
    u32 SourceOffset;     /* 0x30 */
    u32 ImageLen;         /* 0x34 */
    u32 Reserved0;        /* 0x38 */
    u32 StartExecution;   /* 0x3C */
    u32 TotalImageLen;    /* 0x40 */
    u32 ImageAttr;        /* 0x44 */
    u32 CheckSum;         /* 0x48 */
} Ps_BootHeader;

/**
 * Structure to store the image header table details.
 * It contains all the information of image header table in order.
 */
typedef struct {
    u32 Version;                /**< bootgen version used  */
    u32 NoOfPartitions;         /**< No of partition present  */
    u32 PartitionHeaderAddress; /**< Address to start of partition header*/
    u32 Reserved_0xC;           /**< Reserved */
    u32 AuthCertificateOffset;  /** Authentication certificate address */
    u32 PartitionPresentDevice;
    /**< Partition present device for secondary boot modes*/
    u32 Reserved[9]; /**< Reserved */
    u32 Checksum;    /**< Checksum of the image header table */
} Ps_ImageHeaderTable;

/**
 * Structure to store the partition header details.
 * It contains all the information of partition header in order.
 */
typedef struct {
    u32 EncryptedDataWordLength;   /**< Encrypted word length of partition*/
    u32 UnEncryptedDataWordLength; /**< unencrypted word length */
    u32 TotalDataWordLength;
    /**< Total word length including the authentication
        certificate if any*/
    u32 NextPartitionOffset;         /**< Address of the next partition header*/
    u32 DestinationExecutionAddress; /**< Execution address */
    u32 DestinationExecutionAddress_HI;
    u32 DestinationLoadAddress;      /**< Load address in DDR/TCM */
    u32 DestinationLoadAddress_HI;
    u32 DataWordOffset;              /**< */
    u32 PartitionAttributes;         /**< partition attributes */
    u32 SectionCount;                /**< section count */
    u32 ChecksumWordOffset;          /**< address to checksum when enabled */
    u32 ImageHeaderOffset;           /**< address to image header */
    u32 AuthCertificateOffset;
    /**< address to the authentication certificate when enabled */
    u32 Reserved[1]; /**< Reserved */
    u32 Checksum;    /**< checksum of the partition header */
} Ps_PartitionHeader;

/**
 * Structure of the image header which contains
 * information of image header table and
 * partition headers.
 */
typedef struct {
    Ps_ImageHeaderTable ImageHeaderTable;
    /**< Image header table structure */
    Ps_PartitionHeader PartitionHeader[IH_MAX_PARTITIONS];
    /**< Partition header */
} Ps_ImageHeader;

/*define register initialization pairs*/

typedef struct {
    u32 Addr;
    u32 Data;
} RegPairs;

/* Structure corresponding to each partition entry */
typedef struct {
    u64 EntryPoint;
    /**< Entry point */
    u64 PartitionFlags;
    /**< Attributes of partition */
} Ps_PartitionEntry;

/* Structure for handoff parameters to ARM Trusted Firmware (ATF) */
typedef struct {
    char8 MagicValue[4];
    /**< 32 bit magic string */
    u32 NumEntries;
    /**< Number of Entries */
    Ps_PartitionEntry Entry[8];
    /**< Structure corresponding to each entry */
} Ps_ATFHandoffParams;

/************************** Function Prototypes ******************************/
u32 FmshFsbl_GetPartitionOwner(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_IsRsaSignaturePresent(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetChecksumType(Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetDestinationCpu(Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_IsEncryptedPresent(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetDestinationDevice(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetExecState(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetPartitionOwner(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_IsRsaSignaturePresent(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetChecksumType(Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetDestinationCpu(Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_IsEncryptedPresent(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetDestinationDevice(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_GetBlockSize(const Ps_PartitionHeader *PartitionHeader);

u32 FmshFsbl_ValidataBootHeader(Ps_BootHeader *Header);

#ifdef __cplusplus
}
#endif

#endif
