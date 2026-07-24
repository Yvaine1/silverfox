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
 * @file fmsh_sdhci_card.h
 * @addtogroup sdpsu_v1_0
 * @{
 *
 * This header file contains the identifiers and  functions (or macros)
 * that can be used to access the sdmmc card.
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
#ifndef _FMSH_SDHCI_CARD_H_
#define _FMSH_SDHCI_CARD_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_sdhci.h"

/************************** Constant Definitions *****************************/
/***** SD CMD *****/
#define SDMMC_CMD_GO_IDLE_STATE           (0)
#define SDMMC_CMD_ALL_SEND_CID            (2)
#define SDMMC_CMD_SET_RELATIVE_ADDR       (3)
#define SDMMC_CMD_SET_DSR                 (4)
#define SDMMC_CMD_SWITCH_FUNC             (6)
#define SDMMC_CMD_SELECT_CARD             (7)
#define SDMMC_CMD_SEND_IF_COND            (8)
#define SDMMC_CMD_SEND_CSD                (9)
#define SDMMC_CMD_SEND_CID                (10)
#define SDMMC_CMD_SWITCH_UHS18V           (11)
#define SDMMC_CMD_STOP_TRANSMISSION       (12)
#define SDMMC_CMD_SEND_STATUS             (13)
#define SDMMC_CMD_GO_INACTIVE_STATE       (15)
#define SDMMC_CMD_SET_BLOCKLEN            (16)
#define SDMMC_CMD_READ_SINGLE_BLOCK       (17)
#define SDMMC_CMD_READ_MULTIPLE_BLOCK     (18)
#define SDMMC_CMD_SEND_TUNING_BLOCK       (19)
#define SDMMC_CMD_SPEED_CLASS_CONTROL     (20)
#define SDMMC_CMD_ADDRESS_EXTENSION       (22)
#define SDMMC_CMD_SET_BLOCK_COUNT         (23)
#define SDMMC_CMD_WRITE_SINGLE_BLOCK      (24)
#define SDMMC_CMD_WRITE_MULTIPLE_BLOCK    (25)
#define SDMMC_CMD_PROGRAM_CSD             (27)
#define SDMMC_CMD_SET_WRITE_PROTECT       (28)
#define SDMMC_CMD_CLR_WRITE_PROTECT       (29)
#define SDMMC_CMD_SEND_WRITE_PROTECT      (30)
#define SDMMC_CMD_ERASE_WR_BLK_START      (32)
#define SDMMC_CMD_ERASE_WR_BLK_END        (33)
#define SDMMC_CMD_ERASE                   (38)
#define SDMMC_CMD_LOCK_UNLOCK             (42)
#define SDMMC_CMD_APP_CMD                 (55)
#define SDMMC_CMD_GEN_CMD                 (56)

/***** SD ACMD *****/
#define SDMMC_ACMD_SET_BUS_WIDTH          (6)
#define SDMMC_ACMD_SD_STATUS              (13)
#define SDMMC_ACMD_SEND_NUM_WR_BLOCKS     (22)
#define SDMMC_ACMD_SET_WR_BLK_ERASE_COUNT (23)
#define SDMMC_ACMD_SEND_OP_COND           (41)
#define SDMMC_ACMD_SET_CLR_CARD_DETECT    (42)
#define SDMMC_ACMD_SEND_SCR               (51)

/***** SD SPI CMD EXTENSION*****/
#define SDMMC_CMD_SPI_READ_OCR            (58)
#define SDMMC_CMD_SPI_CRC_ON_OFF          (59)

/***** EMMC CMD EXTENSION*****/
#define SDMMC_CMD_SEND_OP_COND            (1)
#define SDMMC_CMD_SLEEP_AWAKE             (5)
#define SDMMC_CMD_SWITCH                  (6)
#define SDMMC_CMD_SEND_EXT_CSD            (8)
#define SDMMC_CMD_READ_DAT_UNTIL_STOP     (11)
#define SDMMC_CMD_BURST_R                 (14)
#define SDMMC_CMD_BURST_W                 (19)
#define SDMMC_CMD_WRITE_DAT_UNTIL_STOP    (20)
#define SDMMC_CMD_SEND_TUNING_BLOCK_HS200 (21)
#define SDMMC_CMD_ERASE_GROUP_START       (35)
#define SDMMC_CMD_ERASE_GROUP_END         (36)
#define SDMMC_CMD_SET_TIME                (49)

/***** SDMMC OCR register *****/
#define OCR_BUSY                          (0x80000000)
#define OCR_HCS                           (0x40000000)
#define OCR_S18R                          (0x01000000)
#define OCR_VOLTAGE_MASK                  (0x00FFFF80)

#define OCR_ACCESS_MODE                   (0x60000000) /* for eMMC only */

#define SDMMC_VDD_165_195                 (0x00000080) /* VDD voltage 1.65 - 1.95 */
#define SDMMC_VDD_20_21                   (0x00000100) /* VDD voltage 2.0 ~ 2.1 */
#define SDMMC_VDD_21_22                   (0x00000200) /* VDD voltage 2.1 ~ 2.2 */
#define SDMMC_VDD_22_23                   (0x00000400) /* VDD voltage 2.2 ~ 2.3 */
#define SDMMC_VDD_23_24                   (0x00000800) /* VDD voltage 2.3 ~ 2.4 */
#define SDMMC_VDD_24_25                   (0x00001000) /* VDD voltage 2.4 ~ 2.5 */
#define SDMMC_VDD_25_26                   (0x00002000) /* VDD voltage 2.5 ~ 2.6 */
#define SDMMC_VDD_26_27                   (0x00004000) /* VDD voltage 2.6 ~ 2.7 */
#define SDMMC_VDD_27_28                   (0x00008000) /* VDD voltage 2.7 ~ 2.8 */
#define SDMMC_VDD_28_29                   (0x00010000) /* VDD voltage 2.8 ~ 2.9 */
#define SDMMC_VDD_29_30                   (0x00020000) /* VDD voltage 2.9 ~ 3.0 */
#define SDMMC_VDD_30_31                   (0x00040000) /* VDD voltage 3.0 ~ 3.1 */
#define SDMMC_VDD_31_32                   (0x00080000) /* VDD voltage 3.1 ~ 3.2 */
#define SDMMC_VDD_32_33                   (0x00100000) /* VDD voltage 3.2 ~ 3.3 */
#define SDMMC_VDD_33_34                   (0x00200000) /* VDD voltage 3.3 ~ 3.4 */
#define SDMMC_VDD_34_35                   (0x00400000) /* VDD voltage 3.4 ~ 3.5 */
#define SDMMC_VDD_35_36                   (0x00800000) /* VDD voltage 3.5 ~ 3.6 */

/***** CSD register *****/
#define CSD_STRUCTURE                     (126)
#define CSD_TAAC                          (112)
#define CSD_NSAC                          (104)
#define CSD_TRAN_SPEED                    (96)
#define CSD_CCC                           (84)
#define CSD_READ_BL_LEN                   (80)
#define CSD_READ_BL_PARTIAL               (79)
#define CSD_WRITE_BLK_MISALIGN            (78)
#define CSD_READ_BLK_MISALIGN             (77)
#define CSD_DSR_IMP                       (76)
#define CSD_ERASE_BLK_EN                  (46)
#define CSD_SECTOR_SIZE                   (39)
#define CSD_WP_GRP_SIZE                   (32)
#define CSD_WP_GRP_ENABLE                 (31)
#define CSD_R2W_FACTOR                    (26)
#define CSD_WRITE_BL_LEN                  (22)
#define CSD_WRITE_BL_PARTIAL              (21)
#define CSD_FILE_FORMAT_GRP               (15)
#define CSD_COPY                          (14)
#define CSD_PERM_WRITE_PROTECT            (13)
#define CSD_TMP_WRITE_PROTECT             (12)
#define CSD_FILE_FORMAT                   (10)
#define CSD_CRC                           (1)

#define CSD_VERS                          (122) /* for eMMC only */
#define CSD_CONTENT_PROT_APP              (16)  /* for eMMC only */
#define CSD_ECC                           (8)   /* for eMMC only */

/***** SCR register (read only) *****/
#define SCR_STRUCTURE                     (60)
#define SCR_SD_SPEC                       (56)
#define SCR_DATA_STAT_AFTER_ERASE         (55)
#define SCR_CPRM_SECURITY                 (52)
#define SCR_SD_BUS_WIDTHS                 (48)
#define SCR_SD_SPEC3                      (47)
#define SCR_EX_SECURITY                   (43)
#define SCR_CMD_SUPPORT                   (32)

/***** EXT_CSD fields (for eMMC only) *****/
#define EXT_CSD_ENH_START_ADDR            (136) /* R/W */
#define EXT_CSD_ENH_SIZE_MULT             (140) /* R/W */
#define EXT_CSD_GP_SIZE_MULT              (143) /* R/W */
#define EXT_CSD_PARTITION_SETTING         (155) /* R/W */
#define EXT_CSD_PARTITIONS_ATTRIBUTE      (156) /* R/W */
#define EXT_CSD_MAX_ENH_SIZE_MULT         (157) /* R */
#define EXT_CSD_PARTITIONING_SUPPORT      (160) /* RO */
#define EXT_CSD_RST_N_FUNCTION            (162) /* R/W */
#define EXT_CSD_BKOPS_EN                  (163) /* R/W & R/W/E */
#define EXT_CSD_WR_REL_PARAM              (166) /* R */
#define EXT_CSD_WR_REL_SET                (167) /* R/W */
#define EXT_CSD_RPMB_MULT                 (168) /* RO */
#define EXT_CSD_USER_WP                   (171) /* R/W & R/W/C_P & R/W/E_P */
#define EXT_CSD_BOOT_WP                   (173) /* R/W & R/W/C_P */
#define EXT_CSD_BOOT_WP_STATUS            (174) /* R */
#define EXT_CSD_ERASE_GROUP_DEF           (175) /* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH            (177)
#define EXT_CSD_PART_CONF                 (179) /* R/W */
#define EXT_CSD_BUS_WIDTH                 (183) /* R/W */
#define EXT_CSD_STROBE_SUPPORT            (184) /* R/W */
#define EXT_CSD_HS_TIMING                 (185) /* R/W */
#define EXT_CSD_REV                       (192) /* RO */
#define EXT_CSD_CARD_TYPE                 (196) /* RO */
#define EXT_CSD_PART_SWITCH_TIME          (199) /* RO */
#define EXT_CSD_SEC_CNT                   (212) /* RO, 4 bytes */
#define EXT_CSD_HC_WP_GRP_SIZE            (221) /* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE         (224) /* RO */
#define EXT_CSD_BOOT_MULT                 (226) /* RO */
#define EXT_CSD_GENERIC_CMD6_TIME         (248) /* RO */
#define EXT_CSD_BKOPS_SUPPORT             (502) /* RO */

/***** EXT_CSD field definitions *****/
#define EXT_CSD_CMD_SET_NORMAL            (1 << 0)
#define EXT_CSD_CMD_SET_SECURE            (1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE          (1 << 2)

#define EXT_CSD_CARD_TYPE_26              (1 << 0) /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52              (1 << 1) /* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_DDR_1_8V        (1 << 2)
#define EXT_CSD_CARD_TYPE_DDR_1_2V        (1 << 3)
#define EXT_CSD_CARD_TYPE_DDR_52 \
    (EXT_CSD_CARD_TYPE_DDR_1_8V | EXT_CSD_CARD_TYPE_DDR_1_2V)
#define EXT_CSD_CARD_TYPE_HS200_1_8V (1 << 4)
#define EXT_CSD_CARD_TYPE_HS200_1_2V (1 << 5)
#define EXT_CSD_CARD_TYPE_HS200 \
    (EXT_CSD_CARD_TYPE_HS200_1_8V | EXT_CSD_CARD_TYPE_HS200_1_2V)
#define EXT_CSD_CARD_TYPE_HS400_1_8V (1 << 6)
#define EXT_CSD_CARD_TYPE_HS400_1_2V (1 << 7)
#define EXT_CSD_CARD_TYPE_HS400 \
    (EXT_CSD_CARD_TYPE_HS400_1_8V | EXT_CSD_CARD_TYPE_HS400_1_2V)

#define EXT_CSD_TIMING_LEGACY               (1 << 0) /* no high speed */
#define EXT_CSD_TIMING_HS                   (1 << 1) /* HS */
#define EXT_CSD_TIMING_HS200                (1 << 2) /* HS200 */
#define EXT_CSD_TIMING_HS400                (1 << 3) /* HS400 */
#define EXT_CSD_DRV_STR_SHIFT               (4)      /* Driver Strength shift */

#define EXT_CSD_BUS_WIDTH_1                 (1 << 0) /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4                 (1 << 1) /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8                 (1 << 2) /* Card is in 8 bit mode */
#define EXT_CSD_DDR_BUS_WIDTH_4             (1 << 5) /* Card is in 4 bit DDR mode */
#define EXT_CSD_DDR_BUS_WIDTH_8             (1 << 6) /* Card is in 8 bit DDR mode */

#define EXT_CSD_BOOT_ACK_ENABLE             (1 << 6)
#define EXT_CSD_BOOT_ACK(x)                 (x << 6)
#define EXT_CSD_EXTRACT_BOOT_ACK(x)         (((x) >> 6) & 0x1)
#define EXT_CSD_BOOT_PARTITION_ENABLE       (1 << 3)
#define EXT_CSD_BOOT_PART_NUM(x)            (x << 3)
#define EXT_CSD_EXTRACT_BOOT_PART(x)        (((x) >> 3) & 0x7)
#define EXT_CSD_PARTITION_ACCESS_ENABLE     (1 << 0)
#define EXT_CSD_PARTITION_ACCESS_DISABLE    (0 << 0)
#define EXT_CSD_PARTITION_ACCESS(x)         (x << 0)
#define EXT_CSD_EXTRACT_PARTITION_ACCESS(x) ((x) & 0x7)

#define EXT_CSD_BOOT_BUS_WIDTH_MODE(x)      (x << 3)
#define EXT_CSD_BOOT_BUS_WIDTH_RESET(x)     (x << 2)
#define EXT_CSD_BOOT_BUS_WIDTH_WIDTH(x)     (x)

#define EXT_CSD_PARTITION_SETTING_COMPLETED (1 << 0)

#define EXT_CSD_ENH_USR                     (1 << 0) /* user data area is enhanced */
#define EXT_CSD_ENH_GP(x)                   (1 << ((x) + 1)) /* GP part (x+1) is enhanced */

#define EXT_CSD_HS_CTRL_REL                 (1 << 0) /* host controlled WR_REL_SET */

#define EXT_CSD_WR_DATA_REL_USR             (1 << 0) /* user data area WR_REL */
#define EXT_CSD_WR_DATA_REL_GP(x)           (1 << ((x) + 1)) /* GP part (x+1) WR_REL */

/***** MMC switch cmd *****/
#define SDMMC_SWITCH_MODE_CMD_SET           (0x00) /* Change the command set */
#define SDMMC_SWITCH_MODE_SET_BITS                                             \
    (0x01) /* Set bits in EXT_CSD byte addressed by index which are 1 in value \
              field */
#define SDMMC_SWITCH_MODE_CLEAR_BITS                                        \
    (0x02) /* Clear bits in EXT_CSD byte addressed by index, which are 1 in \
              value field */
#define SDMMC_SWITCH_MODE_WRITE_BYTE (0x03) /* Set target byte to value */

/***** SD SEITCH FUNC *****/
#define SDMMC_SWITCH_GROUP1          (0x0)
#define SDMMC_ACCESS_MODE_DEFAULT    (0x0)
#define SDMMC_ACCESS_MODE_HS         (0x1)
#define SDMMC_ACCESS_MODE_SDR50      (0x2)
#define SDMMC_ACCESS_MODE_SDR104     (0x3)
#define SDMMC_ACCESS_MODE_DDR50      (0x4)

#define SDMMC_SWITCH_GROUP2          (0x1)
#define SDMMC_COMMAND_SYSTEM_DEFAULT (0x0)
#define SDMMC_COMMAND_SYSTEM_EC      (0x1)
#define SDMMC_COMMAND_SYSTEM_OTP     (0x3)
#define SDMMC_COMMAND_SYSTEM_ASSD    (0x4)

#define SDMMC_SWITCH_GROUP3          (0x2)
#define SDMMC_DRIVER_STRENGTH_TYPEB  (0x0)
#define SDMMC_DRIVER_STRENGTH_TYPEA  (0x1)
#define SDMMC_DRIVER_STRENGTH_TYPEC  (0x2)
#define SDMMC_DRIVER_STRENGTH_TYPED  (0x3)

#define SDMMC_SWITCH_GROUP4          (0x3)
#define SDMMC_CURRENT_LIMIT_200      (0x0)
#define SDMMC_CURRENT_LIMIT_400      (0x1)
#define SDMMC_CURRENT_LIMIT_600      (0x2)
#define SDMMC_CURRENT_LIMIT_800      (0x3)

#define SDMMC_SWITCH_CHECK           (0)
#define SDMMC_SWITCH_FUNC            (1)

/***** Status registrer *****/
#define SDMMC_STATUS_MASK            (~0x0206BF7F)
#define SDMMC_STATUS_ERROR           (1 << 19)
#define SDMMC_STATUS_CURR_STATE      (0xf << 9)
#define SDMMC_STATE_PRG              (7 << 9)
#define SDMMC_STATUS_RDY_FOR_DATA    (1 << 8)
#define SDMMC_STATUS_SWITCH_ERROR    (1 << 7)

/***** SDMMC type *****/
#define SDMMC_TYPE_UNKNOWN           (0x0)
#define SDMMC_TYPE_SD                (0x1)
#define SDMMC_TYPE_MMC               (0x2)
#define SDMMC_TYPE_SDIO              (0x3)

/***** SDMMC version *****/
#define SD_VERSION_MARK              (0x1U << 31)
#define MMC_VERSION_MARK             (0x1U << 30)
#define SDMMC_VERSION(a, b, c)       ((((u32)(a)) << 16) | ((u32)(b) << 8) | (u32)(c))
#define SD_VERSION(a, b, c)          (SD_VERSION_MARK | SDMMC_VERSION(a, b, c))
#define MMC_VERSION(a, b, c)         (MMC_VERSION_MARK | SDMMC_VERSION(a, b, c))

#define SDMMC_MAJOR_VERSION(x)       (((u32)(x) >> 16) & 0xff)
#define SDMMC_MINOR_VERSION(x)       (((u32)(x) >> 8) & 0xff)
#define SDMMC_CHANGE_VERSION(x)      ((u32)(x) & 0xff)

#define SD_VERSION_1_0               SD_VERSION(1, 0, 0)
#define SD_VERSION_1_10              SD_VERSION(1, 10, 0)
#define SD_VERSION_2_0               SD_VERSION(2, 0, 0)
#define SD_VERSION_3_0               SD_VERSION(3, 0, 0)

#define MMC_VERSION_UNKNOWN          MMC_VERSION(0, 0, 0)
#define MMC_VERSION_1_2              MMC_VERSION(1, 2, 0)
#define MMC_VERSION_1_4              MMC_VERSION(1, 4, 0)
#define MMC_VERSION_2_2              MMC_VERSION(2, 2, 0)
#define MMC_VERSION_3                MMC_VERSION(3, 0, 0)
#define MMC_VERSION_4                MMC_VERSION(4, 0, 0)
#define MMC_VERSION_4_1              MMC_VERSION(4, 1, 0)
#define MMC_VERSION_4_2              MMC_VERSION(4, 2, 0)
#define MMC_VERSION_4_3              MMC_VERSION(4, 3, 0)
#define MMC_VERSION_4_4              MMC_VERSION(4, 4, 0)
#define MMC_VERSION_4_41             MMC_VERSION(4, 4, 1)
#define MMC_VERSION_4_5              MMC_VERSION(4, 5, 0)
#define MMC_VERSION_5_0              MMC_VERSION(5, 0, 0)
#define MMC_VERSION_5_1              MMC_VERSION(5, 1, 0)

/***** SDMMC response *****/
#define SDMMC_RESP_PRESENT           (0x1)
#define SDMMC_RESP_BUSY              (0x2)
#define SDMMC_RESP_136               (0x4)
#define SDMMC_RESP_CRC               (0x8)
#define SDMMC_RESP_OPCODE            (0x10)

#define SDMMC_RESP_NONE              (0x0)
#define SDMMC_RESP_R1                (SDMMC_RESP_PRESENT | SDMMC_RESP_CRC | SDMMC_RESP_OPCODE)
#define SDMMC_RESP_R1b \
    (SDMMC_RESP_PRESENT | SDMMC_RESP_CRC | SDMMC_RESP_OPCODE | SDMMC_RESP_BUSY)
#define SDMMC_RESP_R2 (SDMMC_RESP_PRESENT | SDMMC_RESP_136 | SDMMC_RESP_CRC)
#define SDMMC_RESP_R3 (SDMMC_RESP_PRESENT)
#define SDMMC_RESP_R4 (SDMMC_RESP_PRESENT)
#define SDMMC_RESP_R5 (SDMMC_RESP_PRESENT | SDMMC_RESP_CRC | SDMMC_RESP_OPCODE)
#define SDMMC_RESP_R5b \
    (SDMMC_RESP_PRESENT | SDMMC_RESP_CRC | SDMMC_RESP_OPCODE | SDMMC_RESP_BUSY)
#define SDMMC_RESP_R6 (SDMMC_RESP_PRESENT | SDMMC_RESP_CRC | SDMMC_RESP_OPCODE)
#define SDMMC_RESP_R7 (SDMMC_RESP_PRESENT | SDMMC_RESP_CRC | SDMMC_RESP_OPCODE)

#define SDMMC_BVS_180 (0x5)
#define SDMMC_BVS_330 (0x7)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**********************************Variable
 * Definition**************************/

/************************** Function Prototypes ******************************/
/*****************************************************************************
 * This function is used to change sd bus width and speed mode
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_SD_ChangeBusWidthAndSpeed(FSdPsu_T *sdPtr, int width, int mode);

/*****************************************************************************
 * This function is used to change mmc bus width and speed mode
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_MMC_ChangeBusWidthAndSpeed(FSdPsu_T *sdPtr, int width, int mode);

/*****************************************************************************
 * This function is the principal part in FSdPsu_CardInit
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_CardDetect(FSdPsu_T *sdPtr);

/*****************************************************************************
 * This function is used to detect card and initialize it.
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_CardInit(FSdPsu_T *sdPtr, FSdPsu_UserCfg_T *usercfg);

/*****************************************************************************
 * This function is used to read data with block cnt
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Bread(FSdPsu_T *sdPtr, unsigned int start, unsigned int blkcnt,
                 unsigned char *dst);

/*****************************************************************************
 * This function is used to write data with block cnt
 *
 * @param
 *
 * @return
 *
 * @note
 *
 ******************************************************************************/
int FSdPsu_Bwrite(FSdPsu_T *sdPtr, unsigned int start, unsigned int blkcnt,
                  unsigned char *src);

#ifdef __cplusplus
}
#endif

#endif
