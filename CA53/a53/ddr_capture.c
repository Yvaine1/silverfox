#include "ddr_capture.h"

// Read 8-11bit from physical address 0xa0000224, return config value (0-4)
static uint8_t read_capture_size_config(void)
{
    uint8_t size_cfg = ((FMSH_ReadReg(FPGA_BASEADDR, FPGA_CLKREG) & 0x00000F00) >> 8);
    if (size_cfg > 4)
    {
        fmsh_print("WARN: Invalid capture size config (0x%02X), use default 750MB\r\n", size_cfg);
        size_cfg = 0;
    }
    
    return size_cfg;
}

// Calculate total capture size (bytes) and MB based on config value
static void calc_capture_total_size(uint8_t size_cfg, uint64_t *total_bytes, float *total_mb)
{
    switch (size_cfg)
    {
        case 1:
            *total_mb = 562.5f;
            break;
        case 2:
            *total_mb = 375.0f;
            break;
        case 3:
            *total_mb = 187.5f;
            break;
        case 4:
            *total_mb = 93.75f;
            break;
        case 0:
        default:
            *total_mb = 750.0f;
            break;
    }

    *total_bytes = (uint64_t)(*total_mb * 1024 * 1024);
}

void ddr_capture(void) 
{
    FRESULT fr;
    FIL file;
    DWORD free_clusters;
    FATFS *fs;
    const uint8_t *src_ptr = (const uint8_t *)BASE_ADDR;
    uint64_t batch_size_bytes = 0;
    uint32_t bytes_written;
    bool is_success = true;
    uint8_t size_cfg = 0;
    float total_size_MB = 0.0f;
    
    size_cfg = read_capture_size_config();
    calc_capture_total_size(size_cfg, &batch_size_bytes, &total_size_MB);

    fmsh_print("Start saving data, total size: %.1f MB (config: %d)\r\n", total_size_MB, size_cfg);

    fr = f_getfree("0:", &free_clusters, &fs);
    if (fr != FR_OK) 
    {
        fmsh_print("Error: Get disk free space failed, code: %d\r\n", fr);
        return;
    }

    uint64_t free_size = (uint64_t)free_clusters * fs->csize * 512ULL;
    fmsh_print("Disk free space: %llu MB (required: %.1f MB)\r\n",
              free_size / 1024 / 1024, total_size_MB);

    if (free_size < batch_size_bytes) 
    {
        fmsh_print("Error: Insufficient disk space\r\n");
        return;
    }

    fmsh_print("\r\n===== Target file: %s =====\r\n", FILE_PATH);
    fmsh_print("Memory range: 0x%llX - 0x%llX\r\n",
              BASE_ADDR, BASE_ADDR + batch_size_bytes - 1);

    fr = f_open(&file, FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) 
    {
        fmsh_print("Error: Open file failed, code: %d\r\n", fr);
        return;
    }

    fr = f_write(&file, src_ptr, batch_size_bytes, &bytes_written);
    if (fr != FR_OK || bytes_written != batch_size_bytes) 
    {
        fmsh_print("Error: Write failed, code: %d\r\n", fr);
        fmsh_print("Expected: %llu Bytes, Actual: %d Bytes\r\n", batch_size_bytes, bytes_written);
        is_success = false;
    }
    else
    {
        fr = f_sync(&file);
        if (fr != FR_OK)
        {
            fmsh_print("Error: Sync file failed, code: %d\r\n", fr);
            is_success = false;
        }
        fmsh_print("Write progress: 100%%\r\n");
    }

    fr = f_close(&file);
    if (fr != FR_OK)
    {
        fmsh_print("Error: Close file failed, code: %d\r\n", fr);
        is_success = false;
    }

    if (is_success)
    {
        fmsh_print("Success: All data written completed! Total size: %.1f MB\r\n", total_size_MB);
    } 
    else
    {
        fmsh_print("Error: Data writing failed (Total size: %.1f MB)\r\n", total_size_MB);
    }
}