/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    freq_offset_calibration.h
 * @brief   This file provides functions for frequency offset calibration
 */

#ifndef _FREQ_OFFSET_CALIBRATION_H_
#define _FREQ_OFFSET_CALIBRATION_H_

#include <stdint.h>
#include <string.h>
#include "fmsh_common.h"
#include "eeprom_main.h"

#ifdef __cplusplus
extern "C" {
#endif

// EEPROM page range for frequency offset calibration
#define EEPROM_PAGENUM_MIN                        5
#define EEPROM_PAGENUM_MAX                       10
// EEPROM offset range for calibration data
#define EEPROM_OFFSET_MIN                         4
#define EEPROM_OFFSET_MAX                       256
// Default EEPROM value (32-bit all 1s)
#define EEPROM_DEFAULT_VALUE             0xFFFFFFFF
// CRC32 polynomial (standard CRC-32/MPEG-2)
#define CRC32_POLY                     0xEDB88320UL  

// EEPROM offset definitions for page header
#define FREQ_OFF_ADD_SIGNATURE                    0   // Page signature offset
#define FREQ_OFF_ADD_DATA_COUNT                   1   // Calibration data count offset
#define FREQ_OFF_ADD_CRC                          2   // CRC checksum offset
#define FREQ_OFF_ADD_RESERVERD                    3   // Reserved field offset

// Offset of first calibration data (after header)
#define FREQ_OFF_PAGE_HEAD_OFFSET                 4
// Target EEPROM pages for calibration
#define FREQ_OFF_TARGET_PAGES      { 5,6,7,8,9,10,}
// Number of target pages
#define FREQ_OFF_TARGET_PAGE_COUNT                6
// Max number of calibration data per page
#define FREQ_OFF_MAX_DATA_COUNT                 252

/**
 * @brief   Frequency offset page header structure
 * @note    Stored in EEPROM, 4 ints (16 bytes total)
 */
typedef struct {
    int signature;    // Page validation signature (must match g_freq_off_signature)
    int data_count;   // Number of valid calibration data items
    int crc;          // CRC32 checksum of calibration data
    int reserved;     // Reserved field (for future use)
} FreqOffPageHeader;

/**
 * @brief   Frequency offset in-memory cache item
 * @note    Caches EEPROM page data for fast access
 */
typedef struct {
    u8 actual_page;       // Corresponding EEPROM page number
    u8 valid;             // Cache validity flag (1=valid, 0=invalid)
    FreqOffPageHeader header;  // Cached page header
    int calib_data[FREQ_OFF_MAX_DATA_COUNT];  // Cached calibration data
} FreqOffMemCacheItem;

/**
 * @brief   Read data from frequency offset EEPROM page
 * @param   page: EEPROM page number (5~10)
 * @param   offset: Offset in target page
 * @return  Read data (int type)
 */
int freq_off_get_from_eeprom(u8 page, u16 offset);

/**
 * @brief   Read calibration data from DDR cache (g_freq_off_cache)
 * @note    No EEPROM access, fast read (use after freq_off_pages_init success)
 * @param   page: Target EEPROM page number (5~10)
 * @param   offset: Calibration data index (0 ~ data_count-1)
 * @return  FMSH_SUCCESS: Read success; FMSH_FAILURE: Read failed (invalid param/cache)
 */
int freq_off_get_from_cache(u8 page, u16 offset);

/**
 * @brief   Set single calibration data item
 * @param   page: EEPROM page number (5~10)
 * @param   offset: Calibration data index (0~FREQ_OFF_MAX_DATA_COUNT-1)
 * @param   data: Calibration data to write
 */
void freq_off_set_item(u8 page, u16 offset, int data);

/**
 * @brief   Set reserved field of target page
 * @param   page: EEPROM page number (5~10)
 * @param   data: Data to write to reserved field
 */
void freq_off_set_reserved(u8 page, int data);

/**
 * @brief   Set calibration data count of target page
 * @param   page: EEPROM page number (5~10)
 * @param   data: Number of calibration data items
 */
void freq_off_set_cnt(u8 page, int data);

/**
 * @brief   Save page configuration (signature + CRC) to EEPROM
 * @param   page: EEPROM page number (5~10)
 */
void freq_off_save_page_cfg(u8 page);

/**
 * @brief   Reset target page to default value (EEPROM_DEFAULT_VALUE)
 * @param   page: EEPROM page number (5~10)
 */
void freq_off_reset_page(u8 page);

/**
 * @brief   Initialize all target frequency offset pages
 * @note    Loads EEPROM data into in-memory cache
 * @return  FMSH_SUCCESS: Init success; FMSH_FAILURE: Init failed
 */
int freq_off_pages_init();

/**
 * @brief   Print target page's full data (header + calibration data)
 * @param   page: EEPROM page number (5~10)
 */
void freq_off_show_page(u8 page);

#ifdef __cplusplus
}
#endif

#endif /*_FREQ_OFFSET_CALIBRATION_H_*/