#include "mem2bin.h"



static uint64_t clamp_addr_to_ddr_range(uint64_t addr)
{
    uint64_t clamped_addr = addr;
    uint64_t ddr_range_len = DDR_CAP_END_ADDR - DDR_CAP_BASE_ADDR + 1;

    if (clamped_addr < DDR_CAP_BASE_ADDR || clamped_addr > DDR_CAP_END_ADDR)
    {
        int64_t offset = (int64_t)(clamped_addr - DDR_CAP_BASE_ADDR);

        offset = offset % (int64_t)ddr_range_len;
        if (offset < 0)
        {
            offset += ddr_range_len;
        }
        clamped_addr = DDR_CAP_BASE_ADDR + (uint64_t)offset;
        
        fmsh_print("Addr 0x%llX out of range, wrap to 0x%llX\r\n", addr, clamped_addr);
    }

    if ((clamped_addr & DDR_4BYTE_ALIGN_MASK) != 0)
    {
        uint64_t unalign_addr = clamped_addr;
        clamped_addr = clamped_addr & (~DDR_4BYTE_ALIGN_MASK);
        fmsh_print("Addr 0x%llX is not 4-byte aligned, adjust to 0x%llX\r\n", unalign_addr, clamped_addr);
    }

    if (clamped_addr < DDR_CAP_BASE_ADDR || clamped_addr > DDR_CAP_END_ADDR)
    {
        clamped_addr = DDR_CAP_BASE_ADDR;
        fmsh_print("Align error, fallback to DDR_CAP_BASE_ADDR: 0x%llX\r\n", clamped_addr);
    }

    return clamped_addr;
}

static uint8_t* memcpy_to_aligned_buf(uint8_t *src_start, uint32_t total_len, uint8_t **p_origin_buf)
{
    uint8_t *origin_buf = NULL;
    *p_origin_buf = NULL;
    uint8_t *pwbuf_AlignStart = NULL;
    u32 cahcelinesize = 64;

    origin_buf = (uint8_t *)malloc(total_len + 2 * cahcelinesize);
    if (NULL == origin_buf)
    {
        fmsh_print("origin_buf malloc err\r\n");
        return NULL;
    }

    pwbuf_AlignStart = (uint8_t *)(((long long)origin_buf + cahcelinesize) &
                            (~((long long)cahcelinesize - 1)));
    uint64_t src_start_64 = (uint64_t)src_start;


    if (src_start_64 + total_len <= DDR_CAP_END_ADDR)
    {
        fmsh_print("Start: 0x%llX, End: 0x%llX, Size: %u KB (0x%X Bytes)\r\n", src_start_64, src_start_64 + total_len - 1, total_len / 1024, total_len);
        fmsh_print("Read from addr: 0x%llX (match calc addr)\r\n", (uint64_t)src_start);
        memcpy(pwbuf_AlignStart, src_start, total_len);
        *p_origin_buf = origin_buf; 
        return pwbuf_AlignStart;
    }


    uint32_t part1_len = (uint32_t)(DDR_CAP_END_ADDR - src_start_64);
    fmsh_print("Part1: 0x%llX ~ 0x%llX, Size: %u KB (0x%X Bytes)\r\n", src_start_64, DDR_CAP_END_ADDR, part1_len / 1024, part1_len);
    fmsh_print("Part1 read from addr: 0x%llX (match calc addr)\r\n", (uint64_t)src_start);

    memcpy(pwbuf_AlignStart, src_start, part1_len);

    uint32_t part2_len = total_len - part1_len;
    fmsh_print("Part2: 0x%llX ~ 0x%llX, Size: %u KB (0x%X Bytes)\r\n", DDR_CAP_BASE_ADDR, DDR_CAP_BASE_ADDR + part2_len, part2_len / 1024, part2_len);
    fmsh_print("Part2 read from addr: 0x%llX (DDR base addr)\r\n", (uint64_t)DDR_CAP_BASE_ADDR);

    memcpy(&pwbuf_AlignStart[part1_len], (uint8_t *)DDR_CAP_BASE_ADDR, part2_len);

    fmsh_print("Ddr frame copy to aligned buf success: Total %u KB (0x%X Bytes)\r\n", total_len / 1024, total_len);
    *p_origin_buf = origin_buf; 
    return pwbuf_AlignStart;
}


static int write_ddr_frame_data(FIL *file, uint8_t *aligned_buf, uint32_t total_len)
{
    UINT bytes_written;
    int fr = FMSH_SUCCESS;
    if (aligned_buf == NULL)
    {
        fmsh_print("Error: Aligned buffer is NULL\r\n");
        return FMSH_FAILURE;
    }

    fr = f_write(file, aligned_buf, total_len, &bytes_written);
    if (fr != FMSH_SUCCESS || bytes_written != total_len)
    {
        fmsh_print("DMA write failed! Written: %u KB, Expected: %u KB, fr: %d\r\n", bytes_written / 1024, total_len / 1024, fr);
        return FMSH_FAILURE;
    }

    fmsh_print("Ddr frame write success: Total %u KB (0x%X Bytes)\r\n", total_len / 1024, total_len);
    return FMSH_SUCCESS;
}

uint32_t clk_num[MAX_FRAME_NUM]={0};

bool write_ddr_data_to_file(void* src_ptr, uint32_t index, const char* filename)
{
    bool is_success = true;
    FIL file = {0};          
    FRESULT fr = FMSH_SUCCESS; 
    uint32_t total_len = 0;  
    void* aligned_buf = NULL;
    void* origin_buf = NULL;  

    if (src_ptr == NULL || filename == NULL || index >= MAX_FRAME_NUM)
    {
        fmsh_print("Error: Invalid input parameter (src_ptr: %p, filename: %s, index: %u)\r\n", src_ptr, filename, index);
        return false;
    }

    total_len = 4 * clk_num[index];
    if (total_len == 0)
    {
        fmsh_print("Error: Total length is 0 (clk_num[%u] = %u)\r\n", index, clk_num[index]);
        return false;
    }
    fmsh_print("Src_ptr: 0x%llX, Frame size: 0x%lx \r\n", (uint64_t)src_ptr, total_len);

    fmsh_print("Start open %s \r\n", filename);
    fr = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FMSH_SUCCESS) 
    {
        fmsh_print("Error: Open file failed, code: %d\r\n", fr);
        is_success = false;
    }

    if (is_success)
    {
        aligned_buf = memcpy_to_aligned_buf(src_ptr, total_len, &origin_buf);
        if (aligned_buf == NULL) 
        {
            fmsh_print("Error: Copy DDR data to aligned buf failed\r\n");
            is_success = false;
        }
    }
    
    if (is_success)
    {
        fr = write_ddr_frame_data(&file, aligned_buf, total_len);
        if (fr != FMSH_SUCCESS)
        {
            fmsh_print("Error: Write failed, code: %d\r\n", fr);
            is_success = false;
        }
        else
        {
            fr = f_sync(&file);
            if (fr != FMSH_SUCCESS)
            {
                fmsh_print("Error: Sync file failed, code: %d\r\n", fr);
                is_success = false;
            }
            fmsh_print("Write progress: 100%%\r\n");
        }
    }

    fr = f_close(&file);
    if (fr != FMSH_SUCCESS)
    {
        fmsh_print("Error: Close file failed, code: %d\r\n", fr);
        is_success = false;
    }

    if (origin_buf != NULL)
    {
        free(origin_buf);
        origin_buf = NULL;
    }
    aligned_buf = NULL;

    return is_success;
}

void mem2bin(u8 granularity, u8 direction, u32 start_index, u8 frame_count) 
{
    int fr;
    DWORD free_clusters;
    FATFS *fs;
    uint64_t batch_size_bytes = 0;
    uint32_t bytes_written;
    float total_size_MB = (float)FRAME_SIZE / 1024 / 1024; // 18.75 MB
    char filename[64];
    uint8_t *src_ptr = NULL;
    uint64_t raw_src_addr = 0; 
    bool status = true;

    uint32_t ddr_addr_low = FMSH_ReadReg(FPGA_BASEADDR, FPGA_DDR_ADDR_LOW_REG);
    uint32_t ddr_addr_high = FMSH_ReadReg(FPGA_BASEADDR, FPGA_DDR_ADDR_HIGH_REG);
    uint64_t ddr_addr = ((uint64_t)ddr_addr_high << 32) | ddr_addr_low;
    fmsh_print("Original ddr_ptr (tail addr): 0x%llX\r\n", ddr_addr);

    if (ddr_addr < DDR_CAP_BASE_ADDR || ddr_addr >= DDR_CAP_END_ADDR)
    {
        fmsh_print("Warning: Invalid ddr_addr=0x%llx\r\n", ddr_addr);
        return;
    }
    
    if(start_index >= MAX_FRAME_NUM)
    {
        fmsh_print("Warning: Invalid start_index=%d\r\n", start_index);
        return;
    }
    if(frame_count < 1 || frame_count > MAX_FRAME_NUM - start_index)
    {
        fmsh_print("Warning: Invalid frame_count=%d, start_index=%d\r\n", frame_count, start_index);
        return;
    }

    fmsh_print("Valid: 0x%llX ~ 0x%llX (750MB)\r\n", DDR_CAP_BASE_ADDR, DDR_CAP_END_ADDR);
    fr = f_getfree("0:", &free_clusters, &fs);
    if (fr != FMSH_SUCCESS) 
    {
        fmsh_print("Error: Get disk free space failed, code: %d\r\n", fr);
        return;
    }
    uint64_t free_space = (uint64_t)free_clusters * fs->csize * 512ULL;
    fmsh_print("Disk free space: %llu MB\r\n", free_space / 1024 / 1024);

    if(granularity == GRANULARITY_FRAME && direction == DIR_BACKWARD)
    {
        uint64_t offset = 0;
        for (u32 i = 0; i < frame_count; i++)
        {
            u32 current_index = start_index + i;

            for (u32 j = 0; j <= current_index; j++)
            {
                clk_num[j] = FMSH_ReadReg(FPGA_BASEADDR, FPGA_DDR_ADDR_CLK_NUM_REG + j*4);
                
                offset = offset + (uint64_t)clk_num[i] * 4;     
            }
            fmsh_print("clk_num[%u] = 0x%x; offset = 0x%lx  \r\n", current_index, clk_num[current_index], offset);
            raw_src_addr = ddr_addr - offset;
            fmsh_print("Src_ptr (frame start): 0x%llX \r\n", (uint64_t)raw_src_addr);
            raw_src_addr = clamp_addr_to_ddr_range(raw_src_addr);
            src_ptr = (uint8_t *)raw_src_addr; 

            memset(filename, 0, sizeof(filename));
            snprintf(filename, sizeof(filename), "0:/capture_g%d_d%d_i%u.bin", granularity, direction, current_index);

            status = write_ddr_data_to_file(src_ptr, current_index, filename);

            if (status)
            {
                fmsh_print("Success: Data written completed! current_index = %d\r\n", current_index);
            }
            else
            {
                fmsh_print("Error: Data writing failed! current_index = %d\r\n", current_index);
            }
        }
    }
    else
    {
        fmsh_print("Warning: Unsupported granularity(%u) or direction(%u)\r\n", granularity, direction);
    }


}