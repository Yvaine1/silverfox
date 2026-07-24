#include "dma_mem2bin.h"


void mem2bin(void) 
{
    FRESULT fr;
    FIL file;
    DWORD free_clusters;
    FATFS *fs;
    uint8_t *chunk_buf = NULL;
    const uint8_t *src_ptr = (const uint8_t *)BASE_ADDR;
    uint64_t remaining = BATCH_SIZE_BYTES;
    uint32_t bytes_written;
    uint32_t write_size;
    uint32_t progress = 0;   
    uint32_t last_progress = 0;
    bool is_success = true;

    fmsh_print("Start saving data, total size: %d MB\r\n", TOTAL_SIZE_MB);

    chunk_buf = (uint8_t*)malloc(CHUNK_SIZE);
    if (chunk_buf == NULL) 
    {
        fmsh_print("Error: Malloc %d MB buffer failed\r\n", CHUNK_SIZE / 1024 / 1024);
        return;
    }

    fr = f_getfree("0:", &free_clusters, &fs);
    if (fr != FR_OK) 
    {
        fmsh_print("Error: Get disk free space failed, code: %d\r\n", fr);
        free(chunk_buf);
        return;
    }

    uint64_t free_size = (uint64_t)free_clusters * fs->csize * 512ULL;
    fmsh_print("Disk free space: %llu MB (required: %d MB)\r\n",
              free_size / 1024 / 1024, TOTAL_SIZE_MB);

    if (free_size < BATCH_SIZE_BYTES) 
    {
        fmsh_print("Error: Insufficient disk space\r\n");
        free(chunk_buf);
        return;
    }

    fmsh_print("\r\n===== Target file: %s =====\r\n", FILE_PATH);
    fmsh_print("Memory range: 0x%llX - 0x%llX\r\n",
              BASE_ADDR, BASE_ADDR + BATCH_SIZE_BYTES - 1);

    fr = f_open(&file, FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) 
    {
        fmsh_print("Error: Open file failed, code: %d\r\n", fr);
        free(chunk_buf);
        return;
    }

    while (remaining > 0 && is_success)
    {
        write_size = (remaining < CHUNK_SIZE) ? (uint32_t)remaining : CHUNK_SIZE;

        memcpy(chunk_buf, src_ptr, write_size);

        fr = f_write(&file, chunk_buf, write_size, &bytes_written);
        if (fr != FR_OK || bytes_written != write_size) 
        {
            fmsh_print("Error: Write chunk failed, code: %d\r\n", fr);
            fmsh_print("Expected: %d Bytes, Actual: %d Bytes\r\n", write_size, bytes_written);
            is_success = false;
            break;
        }

        fr = f_sync(&file);
        if (fr != FR_OK)
        {
            fmsh_print("Error: Sync file failed, code: %d\r\n", fr);
            is_success = false;
            break;
        }

        src_ptr += write_size;
        remaining -= write_size;

        progress = (uint32_t)(((BATCH_SIZE_BYTES - remaining) * 100) / BATCH_SIZE_BYTES);
        if (progress != last_progress)
        {
            fmsh_print("Write progress: %d%%\r\n", progress);
            last_progress = progress;
        }
    }

    fr = f_close(&file);
    if (fr != FR_OK)
    {
        fmsh_print("Error: Close file failed, code: %d\r\n", fr);
        is_success = false;
    }

    free(chunk_buf);

    if (is_success)
    {
        fmsh_print("Success: All data written completed!\r\n");
    } else
    {
        fmsh_print("Error: Data writing failed\r\n");
    }
}