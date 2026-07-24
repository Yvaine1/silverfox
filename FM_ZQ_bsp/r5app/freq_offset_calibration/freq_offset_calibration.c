/*Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.*/
/**
 * @file    freq_offset_calibration.c
 * @brief   Frequency offset calibration implementation
 */

#include <stdint.h>
#include <string.h>
#include "freq_offset_calibration.h"

// Page signature for validation (must match EEPROM stored value)
static int g_freq_off_signature = 0x4E3C2B1A;

// In-memory cache for target pages (initialized to invalid)
static FreqOffMemCacheItem g_freq_off_cache[FREQ_OFF_TARGET_PAGE_COUNT] = {
    {5,  0, {0}, {0}}, {6,  0, {0}, {0}}, {7,  0, {0}, {0}}, {8,  0, {0}, {0}},\
    {9,  0, {0}, {0}}, {10, 0, {0}, {0}}, 
};

// Target EEPROM pages (matches FREQ_OFF_TARGET_PAGES)
static const u8 g_freq_off_target_pages[] = FREQ_OFF_TARGET_PAGES;

/**
 * @brief   Get cache index by EEPROM page number
 * @param   actual_page: Target EEPROM page number
 * @return  Cache index (>=0) if found; -1 if page not in target list
 */
static int freq_off_get_cache_idx(u8 actual_page)
{
    for (int i = 0; i < FREQ_OFF_TARGET_PAGE_COUNT; i++)
    {
        if (g_freq_off_cache[i].actual_page == actual_page)
        {
            return i;
        }
    }
    fmsh_print("ERR: page %u not in target list!\r\n", actual_page);
    return -1;
}

/**
 * @brief   Calculate CRC32 checksum for byte stream
 * @param   buf: Input byte buffer
 * @param   len: Length of input buffer (bytes)
 * @return  32-bit CRC checksum (u32)
 */
static u32 freq_off_crc32_calc(u8 *buf, u32 len) 
{
    u32 crc = 0xFFFFFFFF;  // CRC initial value (CRC-32/MPEG-2 standard)
    
    for (u32 i = 0; i < len; i++) 
    {
        crc ^= buf[i];  // XOR current byte with CRC register
        
        // Process 8 bits of current byte
        for (int j = 0; j < 8; j++) 
        {
            // Shift right + XOR with polynomial if LSB is 1
            crc = (crc >> 1) ^ ((crc & 1) ? CRC32_POLY : 0);
        }
    }
    
    return crc ^ 0xFFFFFFFF;  // Final XOR (CRC-32/MPEG-2 standard)
}

/**
 * @brief   Initialize single frequency offset page (load from EEPROM to cache)
 * @param   actual_page: Target EEPROM page number
 * @return  FMSH_SUCCESS: Load success; FMSH_FAILURE: Load failed
 */
static int freq_off_single_page_init(uint8_t actual_page)
{
    int i = 0;
    int cache_idx = freq_off_get_cache_idx(actual_page);
    
    // Check if page is in target list
    if (cache_idx < 0)
    {
        return FMSH_FAILURE;
    }

    FreqOffPageHeader header;                // Temp storage for page header
    int calib_data_temp[FREQ_OFF_MAX_DATA_COUNT];  // Temp storage for calibration data
    int calc_crc;                            // Calculated CRC for validation

    // Read page header from EEPROM
    eeprom_read_dac_calib_table(actual_page, FREQ_OFF_ADD_SIGNATURE,  (int*)&header.signature);
    eeprom_read_dac_calib_table(actual_page, FREQ_OFF_ADD_DATA_COUNT, (int*)&header.data_count);
    eeprom_read_dac_calib_table(actual_page, FREQ_OFF_ADD_CRC,        (int*)&header.crc);
    eeprom_read_dac_calib_table(actual_page, FREQ_OFF_ADD_RESERVERD,  (int*)&header.reserved);

    uint8_t valid = 1;  // Assume valid initially
    
    // Validate signature
    if (header.signature != g_freq_off_signature)
    {
        fmsh_print("WARN: page %u invalid signature\r\n", actual_page);
        valid = 0;
    }
    
    // Validate data count (must not exceed max)
    if (header.data_count > FREQ_OFF_MAX_DATA_COUNT)
    {
        fmsh_print("WARN: page %u invalid data_count\r\n", actual_page);
        valid = 0;
    }

    // If header is valid, read calibration data and verify CRC
    if (valid) 
    {
        // Read calibration data (data_count items)
        for (; i < header.data_count; i++) 
        {
            eeprom_read_dac_calib_table(actual_page, (FREQ_OFF_PAGE_HEAD_OFFSET + i), &calib_data_temp[i]);
        }
        
        // Read extra data (offset = FREQ_OFF_PAGE_HEAD_OFFSET - 1) for CRC calculation
        eeprom_read_dac_calib_table(actual_page, (FREQ_OFF_PAGE_HEAD_OFFSET - 1), &calib_data_temp[header.data_count]);
        
        // Calculate CRC for calibration data (including extra item)
        calc_crc = (int)freq_off_crc32_calc((u8*)calib_data_temp, (header.data_count + 1) * sizeof(int));
        
        // Verify CRC match
        if (calc_crc != header.crc) 
        { 
            valid = 0; 
            fmsh_print("WARN: page %u CRC mismatch\r\n", actual_page);
            return FMSH_FAILURE;
        }
    }
    else 
    {
        return FMSH_FAILURE;
    }

    // Update cache with valid data
    g_freq_off_cache[cache_idx].valid = 1;
    g_freq_off_cache[cache_idx].actual_page = actual_page;
    memcpy(&g_freq_off_cache[cache_idx].header, &header, sizeof(FreqOffPageHeader));
    memcpy(&g_freq_off_cache[cache_idx].calib_data, calib_data_temp, header.data_count * sizeof(int));

    fmsh_print("INFO: page %u loaded (count=%u)\r\n", actual_page, header.data_count);

    return FMSH_SUCCESS;
}

/**
 * @brief   Initialize all target frequency offset pages
 * @note    Loads all target pages (5~10) into in-memory cache
 * @return  FMSH_SUCCESS: All pages init success; FMSH_FAILURE: Any page init failed
 */
int freq_off_pages_init()
{
    int ret = FMSH_FAILURE;

    fmsh_print("=== Start init 5~10 freq offset pages ===\r\n");
    
    // Initialize each target page
    for (int i = 0; i < FREQ_OFF_TARGET_PAGE_COUNT; i++) 
    {
        ret = freq_off_single_page_init(g_freq_off_target_pages[i]);
        
        // Return failure if any page init fails
        if (FMSH_FAILURE == ret) 
        {
            fmsh_print("freq off pages init failed!\r\n");
            return FMSH_FAILURE;
        }
    }
    
    fmsh_print("=== 5~10 pages init/load completed ===\r\n");
    return FMSH_SUCCESS;
}

/**
 * @brief   Check if page and offset are within valid range
 * @param   page: EEPROM page number
 * @param   offset: Offset in target page
 * @return  FMSH_SUCCESS: Valid range; FMSH_FAILURE: Invalid range
 */
static int freq_off_range_check(u8 page, u16 offset)
{
    if ((page < EEPROM_PAGENUM_MIN) || (page > EEPROM_PAGENUM_MAX))
    {
        fmsh_print("page num error!\r\n");
    }
    else if ((offset < EEPROM_OFFSET_MIN) || (offset > EEPROM_OFFSET_MAX))
    {
        fmsh_print("offset num error!\r\n");
    } 
    else
    {
        return FMSH_SUCCESS;
    }
    return FMSH_FAILURE;
}

/**
 * @brief   Set calibration data count of target page
 * @param   page: EEPROM page number (5~10)
 * @param   data: Number of calibration data items (0~FREQ_OFF_MAX_DATA_COUNT)
 */
void freq_off_set_cnt(u8 page, int data)
{
    if ((page < EEPROM_PAGENUM_MIN) || (page > EEPROM_PAGENUM_MAX))
    {
      fmsh_print("page num error!, page min:%d ,page max: %d\r\n", EEPROM_PAGENUM_MIN, EEPROM_PAGENUM_MAX);
        return ;
    }
    
    // Write data count to EEPROM
    eeprom_write_dac_calib_table(page, FREQ_OFF_ADD_DATA_COUNT, &data);
}

/**
 * @brief   Save page configuration (signature + CRC) to EEPROM
 * @note    Recalculates CRC and updates EEPROM signature + CRC fields
 * @param   page: EEPROM page number (5~10)
 */
void freq_off_save_page_cfg(u8 page)
{
    int i = 0;
    int data_count = 0;
    int calc_crc = 0;
    int crc_buf[FREQ_OFF_MAX_DATA_COUNT];  // Buffer for CRC calculation
    
    // Check page range validity
    if ((page < EEPROM_PAGENUM_MIN) || (page > EEPROM_PAGENUM_MAX))
    {
        fmsh_print("page num error!\r\n");
        return ;
    }
    
    // Write page signature to EEPROM
    eeprom_write_dac_calib_table(page, FREQ_OFF_ADD_SIGNATURE, &g_freq_off_signature);
    
    // Read current data count from EEPROM
    eeprom_read_dac_calib_table(page, FREQ_OFF_ADD_DATA_COUNT, &data_count);
    
    // Validate data count range
    if ((data_count < 0) || (data_count > FREQ_OFF_MAX_DATA_COUNT))
    {
        fmsh_print("data_count error: %d!\r\n", data_count);
        return ;     
    }
    
    // Read calibration data for CRC calculation
    for (; i < data_count; i++) 
    {
        eeprom_read_dac_calib_table(page, (FREQ_OFF_PAGE_HEAD_OFFSET + i), &crc_buf[i]);
    }
    
    // Read extra data (offset = FREQ_OFF_PAGE_HEAD_OFFSET - 1) for CRC calculation
    eeprom_read_dac_calib_table(page, (FREQ_OFF_PAGE_HEAD_OFFSET - 1), &crc_buf[data_count]);
    
    // Calculate CRC (including extra data item)
    calc_crc = (int)freq_off_crc32_calc((u8*)crc_buf, (data_count + 1) * sizeof(int));
    
    // Write calculated CRC to EEPROM
    eeprom_write_dac_calib_table(page, FREQ_OFF_ADD_CRC, &calc_crc);
    fmsh_print("page %u crc: %d!\r\n", page, calc_crc);
    
    return ;
}

/**
 * @brief   Set single calibration data item
 * @param   page: EEPROM page number (5~10)
 * @param   offset: Calibration data index (0~data_count-1)
 * @param   data: Calibration data to write
 */
void freq_off_set_item(u8 page, u16 offset, int data)
{
    int ret = FMSH_FAILURE;
    int count = 0;

    // Read current data count to check offset validity
    eeprom_read_dac_calib_table(page, FREQ_OFF_ADD_DATA_COUNT, &count);
    
    if (offset < EEPROM_OFFSET_MIN)
    {
        fmsh_print("U can't write the configuration area 0~3!\r\n");
        return ;        
    }
    
    // Check if offset exceeds current data count
    if ((offset - EEPROM_OFFSET_MIN + 1) > count)
    {
        fmsh_print("the offset %u is out of range %d failed!\r\n", offset, (count + EEPROM_OFFSET_MIN - 1));
        return ;        
    }

    // Check page and offset range
    ret = freq_off_range_check(page, offset);
    if (FMSH_FAILURE == ret)
    {
        fmsh_print("freq off set item %u failed!\r\n", offset);
        return ;
    }
    
    // Write calibration data to EEPROM
    eeprom_write_dac_calib_table(page, offset, &data);
    
    return ;
}

/**
 * @brief   Read data from frequency offset EEPROM page
 * @param   page: EEPROM page number (5~10)
 * @param   offset: Offset in target page
 * @return  Read data (int type)
 * @warning data pointer is NULL (potential crash risk, keep original logic)
 */
int freq_off_get_from_eeprom(u8 page, u16 offset)
{
    int data = 0;

    if ((page < EEPROM_PAGENUM_MIN) || (page > EEPROM_PAGENUM_MAX))
    {
        fmsh_print("page num error!\r\n");
    }
    else if ((offset < 0) || (offset >= EEPROM_OFFSET_MAX))
    {
        fmsh_print("offset num error!\r\n");
    }
    else
    {
        eeprom_read_dac_calib_table(page, offset, &data);
        fmsh_print("value: %d \r\n", data);
    }

    return data;
}

/**
 * @brief   Read calibration data from DDR cache (g_freq_off_cache)
 * @note    No EEPROM access, fast read (use after freq_off_pages_init success)
 * @param   page: Target EEPROM page number (5~10)
 * @param   offset: Calibration data index (0 ~ data_count-1)
 * @return  FMSH_SUCCESS: Read success; FMSH_FAILURE: Read failed (invalid param/cache)
 */
int freq_off_get_from_cache(u8 page, u16 offset)
{
    if ((page < EEPROM_PAGENUM_MIN) || (page > EEPROM_PAGENUM_MAX))
    {
        fmsh_print("ERR: freq_off_get_from_cache - page %u out of range (%d~%d)!\r\n",
                  page, EEPROM_PAGENUM_MIN, EEPROM_PAGENUM_MAX);
        return FMSH_FAILURE;
    }

    int cache_idx = freq_off_get_cache_idx(page);
    if (cache_idx < 0)
    {
        fmsh_print("ERR: freq_off_get_from_cache - page %u not in target list!\r\n", page);
        return FMSH_FAILURE;
    }

    if (g_freq_off_cache[cache_idx].valid != 1)
    {
        fmsh_print("ERR: freq_off_get_from_cache - page %u cache is invalid (not initialized)!\r\n", page);
        return FMSH_FAILURE;
    }

    int data_count = g_freq_off_cache[cache_idx].header.data_count;

    if (offset >= (u16)data_count)
    {
        fmsh_print("ERR: freq_off_get_from_cache - page %u offset %u out of range (0~%d)!\r\n",
                  page, offset, data_count - 1);
        return FMSH_FAILURE;
    }
    
    fmsh_print("cache value: %d\r\n", g_freq_off_cache[cache_idx].calib_data[offset]);

    return FMSH_SUCCESS;
}

/**
 * @brief   Set reserved field of target page
 * @param   page: EEPROM page number (5~10)
 * @param   data: Data to write to reserved field
 */
void freq_off_set_reserved(u8 page, int data)
{
    // Write data to reserved field in EEPROM
    eeprom_write_dac_calib_table(page, FREQ_OFF_ADD_RESERVERD, &data);
}

/**
 * @brief   Reset target page to default value (EEPROM_DEFAULT_VALUE)
 * @param   page: EEPROM page number (5~10)
 * @note    Writes default value to all offsets (0~EEPROM_OFFSET_MAX-1)
 */
void freq_off_reset_page(u8 page)
{
    int i = 0;
    int data = EEPROM_DEFAULT_VALUE;  // Default value (0xFFFFFFFF)
    
    // Check page range validity
    if ((page < EEPROM_PAGENUM_MIN) || (page > EEPROM_PAGENUM_MAX))
    {
        fmsh_print("page %u out of range:%d ~ %d!\r\n", page, EEPROM_PAGENUM_MIN, EEPROM_PAGENUM_MAX);
    }

    // Write default value to all offsets in target page
    for (; i < EEPROM_OFFSET_MAX; i++)
    {
        eeprom_write_dac_calib_table(page, i, &data);
    }
    
    return ;
}

/**
 * @brief   Print target page's full data (header + calibration data)
 * @param   page: EEPROM page number (5~10)
 * @note    Outputs formatted data with alignment (hex + decimal)
 */
void freq_off_show_page(u8 page)
{
    int read_ret;
    int signature, data_count;
    int crc, reserved;
    int i;
    int calib_val;

    fmsh_print("==================== FreqOff Page %02d Data ====================\r\n", page);

    // Print signature
    fmsh_print("%12s: ", "signature");
    eeprom_read_dac_calib_table(page, FREQ_OFF_ADD_SIGNATURE, &read_ret);
    if (read_ret < 0)
    {
        fmsh_print("%-32s\r\n", "Read failed!");
        goto END_PRINT;
    }
    signature = read_ret;
    fmsh_print("0x%08X (Expected: 0x%08X)\r\n", signature, g_freq_off_signature);

    // Skip if signature is invalid
    if (signature != g_freq_off_signature)
    {
        fmsh_print("%12s: %-32s\r\n", "", "Invalid signature! Skip page data.");
        goto END_PRINT;
    }

    // Print data count
    fmsh_print("%12s: ", "data_count");
    eeprom_read_dac_calib_table(page, FREQ_OFF_ADD_DATA_COUNT, &read_ret);
    if (read_ret < 0)
    {
        fmsh_print("%-32s\r\n", "Read failed!");
        goto END_PRINT;
    }
    data_count = read_ret;
    fmsh_print("%3d\r\n", data_count);

    // Validate data count
    if (data_count == (u32)-1)
    {
        fmsh_print("%12s: %-32s\r\n", "", "Error: data_count is -1! Skip page data.");
        goto END_PRINT;
    }
    if (data_count > FREQ_OFF_MAX_DATA_COUNT)
    {
        fmsh_print("%12s: %-32s\r\n", "", 
                  "Warning: data_count exceeds max, limit to max!");
        data_count = FREQ_OFF_MAX_DATA_COUNT;
    }

    // Print CRC
    fmsh_print("%12s: ", "crc");
    eeprom_read_dac_calib_table(page, FREQ_OFF_ADD_CRC, &read_ret);
    crc = read_ret;
    fmsh_print("0x%08X\r\n", crc);

    // Print reserved field
    fmsh_print("%12s: ", "reserved");
    eeprom_read_dac_calib_table(page, FREQ_OFF_ADD_RESERVERD, &read_ret);
    reserved = read_ret;
    fmsh_print("0x%08X\r\n", reserved);

    // Print calibration data
    fmsh_print("%12s: (Total %d items)\r\n", "calib_data", data_count);
    if (data_count > 0)
    {
        for (i = 0; i < data_count; i++)
        {
            int calib_idx = 4 + i;
            eeprom_read_dac_calib_table(page, calib_idx, &read_ret);

            fmsh_print("%12s[%2d]: ", "", i);
            calib_val = read_ret;
            fmsh_print("0x%08X (Decimal: %6d)\r\n", calib_val, calib_val);
            
        }
    }
    else
    {
        fmsh_print("%12s: %-32s\r\n", "", "No calib_data (data_count = 0)");
    }

END_PRINT:
    fmsh_print("==============================================================\r\n\r\n");
}