#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fmsh_common.h"
#include "ff.h"
#include "fpga.h"
#include "load_img.h"
#include "crc32.h"
#include "release_rpu.h"
#include "fmsh_sdhci_lib.h"

static FIL file;
extern FQspiPsu_T qspi0;

module_config_t module_configs[] = {
    {LOAD_IMAGE_BOOT,    MODULE_TYPE_BOOT, "BOOT",    DDR_BOOTBIN_ADDR, (EMMC_IMAGE_SECTOR_START_ADDR + IMAGE_HEADER_SIZE/EMMC_BLOCK_SIZE)},
    {LOAD_IMAGE_CA53,    MODULE_TYPE_STANDARD, "CA53",    DDR_A53_ADDR, EMMC_IMAGE_SECTOR_CA53_ADDR},
    {LOAD_IMAGE_CR50,    MODULE_TYPE_STANDARD, "CR50",    ADDR_R50,     EMMC_IMAGE_SECTOR_CR50_ADDR},
    {LOAD_IMAGE_CR51,    MODULE_TYPE_STANDARD, "CR51",    ADDR_R51,     EMMC_IMAGE_SECTOR_CR51_ADDR},
    {LOAD_IMAGE_28DRBIT, MODULE_TYPE_STANDARD, "BIT28",   DDR_PL_ADDR,  EMMC_IMAGE_SECTOR_BIT28DR_ADDR},
    {LOAD_IMAGE_UECONF,  MODULE_TYPE_STANDARD, "UE",      DDR_TEMP_ADDR,EMMC_IMAGE_SECTOR_UECONF_ADDR},
};

int get_module_offset(LOAD_IMAGE_LIST type, image_header_t *header)
{
    switch (type) {
        case LOAD_IMAGE_BOOT:    return header->boot_offset;
        case LOAD_IMAGE_CA53:    return header->a53_offset;
        case LOAD_IMAGE_CR50:   return header->cr50_offset;
        case LOAD_IMAGE_CR51:   return header->cr51_offset;
        case LOAD_IMAGE_28DRBIT: return header->bit28dr_offset;
        case LOAD_IMAGE_UECONF: return header->ue_offset;
        default: return FMSH_FAILURE;
    }
}

int get_module_size(LOAD_IMAGE_LIST type, image_header_t *header)
{
    switch (type) {
        case LOAD_IMAGE_BOOT:    return header->boot_size;
        case LOAD_IMAGE_CA53:    return header->a53_size;
        case LOAD_IMAGE_CR50:   return header->cr50_size;
        case LOAD_IMAGE_CR51:   return header->cr51_size;
        case LOAD_IMAGE_28DRBIT: return header->bit28dr_size;
        case LOAD_IMAGE_UECONF: return header->ue_size;
        default: return FMSH_FAILURE;
    }
}

char* get_module_version(LOAD_IMAGE_LIST type, image_header_t *header) 
{
    switch (type) {
        case LOAD_IMAGE_BOOT:    return header->boot_version;
        case LOAD_IMAGE_CA53:    return header->a53_version;
        case LOAD_IMAGE_CR50:   return header->cr50_version;
        case LOAD_IMAGE_CR51:   return header->cr51_version;
        case LOAD_IMAGE_28DRBIT: return header->bit28dr_version;
        default: return "Unknow";
    }
}

module_config_t * get_module_config(LOAD_IMAGE_LIST load_img) 
{
    for (size_t i = 0; i < sizeof(module_configs)/sizeof(module_configs[0]); i++) 
    {
        if (module_configs[i].load_type == load_img) 
        {
            return &module_configs[i];
        }
    }
    return NULL;
}

int verify_image_header(image_header_t *header) 
{

    if (memcmp(header->signature, "DGKJ", 4) != 0)
    {
        fmsh_print("Invalid Image signature\r\n");
        return FMSH_FAILURE;
    }
    u32 crc_calc_size = sizeof(image_header_t) - 4 - 4 - 4;  //state[4]; signature[4]; header_crc;
    u32 calculated_crc =  calculate_crc32((uint8_t*)header + 12 , crc_calc_size);
    if (calculated_crc != header->header_crc) 
    {
        fmsh_print("Header CRC mismatch. Expected: 0x%08X, Got: 0x%08X\r\n", 
            header->header_crc, calculated_crc);
        return FMSH_FAILURE;
    }
    
    return FMSH_SUCCESS;
}

int verify_module_header(void *mod_header,  LOAD_IMAGE_LIST load_img) 
{
    int header_size = 0;
    module_config_t *config = get_module_config(load_img);
    if (config) 
    {
        fmsh_print("\r\nCurrent Module (%s):\r\n", config->name);
        if (config->module_type == MODULE_TYPE_BOOT) 
        {
            boot_header_t *boot_hdr = (boot_header_t *)mod_header;
            header_size = sizeof(boot_header_t);
            if (memcmp(boot_hdr->signature, "DGMODIMG", 8) != 0) 
            {
                fmsh_print("Error: Invalid module signature: %.8s\r\n", boot_hdr->signature);
                return FMSH_FAILURE;
            }
            u32 crc_calc_size = header_size - 8 - 8 - 4 - 4;
            u32 calculated_crc =  calculate_crc32((uint8_t*)mod_header + 20 , crc_calc_size);
            if (calculated_crc != boot_hdr->module_header_crc) 
            {
                fmsh_print("Module CRC mismatch. Expected: 0x%08X, Got: 0x%08X\r\n", 
                    boot_hdr->module_header_crc, calculated_crc);
                return FMSH_FAILURE;
            }
        } 
        else 
        {
            module_header_t *mod_hdr = (module_header_t *)mod_header;
            header_size = sizeof(module_header_t);
            if (memcmp(mod_hdr->signature, "DGMODIMG", 8) != 0) 
            {
                fmsh_print("Error: Invalid module signature: %.8s\r\n", mod_hdr->signature);
                return FMSH_FAILURE;
            }
            u32 crc_calc_size = header_size - 8 - 8 - 4 - 4;
            u32 calculated_crc =  calculate_crc32((uint8_t*)mod_header + 20 , crc_calc_size);
            if (calculated_crc != mod_hdr->module_header_crc) 
            {
                fmsh_print("Module CRC mismatch. Expected: 0x%08X, Got: 0x%08X\r\n", 
                    mod_hdr->module_header_crc, calculated_crc);
                return FMSH_FAILURE;
            }
        }
    }


    return FMSH_SUCCESS;
}

int load_file_to_ddr(FSdPsu_T *emmcPtr, const char *filename, u32 file_offset, u32 file_size, u32 ddr_address, EMMC_SYSTEM_TYPE_LIST system_type, LBA_t sector, UINT count) 
{
    int res = FMSH_SUCCESS;
    if (file_size == 0 || file_offset == 0) 
    {
        fmsh_print("Invalid file parameters\r\n");
        return FMSH_FAILURE;
    }
    
    if(system_type == EMMC_SYSTEM_TYPE_FS )
    {
        res = f_open(&file, filename, FA_READ);
        if (res != FR_OK) 
        {
            fmsh_print("Failed to open image file: %s (error: %d)\r\n", filename, res);
            return FMSH_FAILURE;
        }

        res = f_lseek(&file, file_offset);
        if (res != FR_OK) 
        {
            fmsh_print("Failed to seek to file offset 0x%08X: %d\r\n", file_offset, res);
            f_close(&file);
            return FMSH_FAILURE;
        }       
        // Read file data
        UINT bytes_read;
        res = f_read(&file, (void *)ddr_address, file_size, &bytes_read);
        if (res != FR_OK || bytes_read != file_size) 
        {
            fmsh_print("Failed to read file data: %d, bytes_read: %u\r\n", res, bytes_read);
            f_close(&file);
            return FMSH_FAILURE;
        }
        f_close(&file);
    }
    else if(system_type == EMMC_SYSTEM_TYPE_NOFS)
    {
        res = emmc_read(emmcPtr, (BYTE *)ddr_address, sector, count);
        if(res)
        {
            fmsh_print("Emmc read failed!\r\n");
            return FMSH_FAILURE;
        }
    }
    fmsh_print("File loaded successfully to DDR 0x%08X\r\n", ddr_address);
    return FMSH_SUCCESS;
}

int load_emmc_firmware(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header,  void *mod_header, EMMC_SYSTEM_TYPE_LIST system_type, LOAD_IMAGE_LIST load_img) 
{

    if (!emmcPtr || !header || !mod_header)
    {
        fmsh_print("Invalid parameters\r\n");
        return FMSH_FAILURE;
    }
    int offset = 0;
    int sector = 0;
    int blk_cnt = 0;
    int bin_size;
    u32 ddr_address;
    u32 module_sector;
        
    module_config_t *config = get_module_config(load_img);
    if (!config) 
        return FMSH_FAILURE;

    ddr_address = config->ddr_addr;
    module_sector = config->sector;

    if(system_type == EMMC_SYSTEM_TYPE_FS )
    {
        offset = get_module_offset(load_img, header);
        bin_size = get_module_size(config->load_type, header);
        fmsh_print("Module: %s, offset = 0x%x, bin_size = %u byte \r\n", config->name, offset, bin_size);
    }
    else if(system_type == EMMC_SYSTEM_TYPE_NOFS)
    {
        if (config->module_type == MODULE_TYPE_BOOT) 
        {
            boot_header_t *boot_hdr = (boot_header_t *)mod_header;
            offset = MODULE_HEADER_SIZE;
            sector = module_sector + (offset  + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;                                                                                                                                                                                                                         
            blk_cnt = (boot_hdr->bootbin_size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
            bin_size = boot_hdr->bootbin_size;
        }
        else
        {
            module_header_t *mod_hdr = (module_header_t *)mod_header;
            offset = MODULE_HEADER_SIZE + ((mod_hdr->map_size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE) * EMMC_BLOCK_SIZE;
            sector = module_sector + (offset  + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;                                                                                                                                                                                                                         
            blk_cnt = (mod_hdr->bin_size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
            bin_size = mod_hdr->bin_size;

            if(load_img == LOAD_IMAGE_CR50 || load_img == LOAD_IMAGE_CR51 )
            {
                u32 atcm_load_addr;
                int res = FMSH_SUCCESS;
                switch (load_img) 
                {
                    case LOAD_IMAGE_CR50:
                        atcm_load_addr = FMZQ_R5_0_TCM_START_ADDR;
                        break;
                    case LOAD_IMAGE_CR51:
                        atcm_load_addr = FMZQ_R5_1_TCM_START_ADDR;
                        break;
                }
            
                int count = (RPU_TCM_SIZE + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
                res = emmc_read(emmcPtr, (BYTE *)atcm_load_addr, sector, count);
                if(res)
                {
                    fmsh_print("Emmc read failed!\r\n");
                    return FMSH_FAILURE;
                }
                
                sector = sector + count;
                blk_cnt = blk_cnt - count;
                bin_size =  mod_hdr->bin_size - RPU_TCM_SIZE;

            }
        }   
    }
    
    return load_file_to_ddr(emmcPtr, filename, offset, bin_size, ddr_address, system_type, sector, blk_cnt);
}

void print_image_header_info(image_header_t *header)
{

    fmsh_print("\r\n*****************IMAGE INFO************\r\n");

    fmsh_print("Image  Version: %.8s\r\n", header->image_version);
    fmsh_print("BOOT  Version: %.8s\r\n", header->boot_version);
    fmsh_print("Build  Date: %.12s\r\n", header->build_date);
    fmsh_print("Build  Time: %.12s\r\n", header->build_time);
    fmsh_print("Board  Type: %.32s\r\n", header->board_type);

    fmsh_print("***************************************\r\n");

}
   

void print_module_version_info(void *mod_header, LOAD_IMAGE_LIST load_img) 
{
    fmsh_print("*****************MODULE INFO************\r\n");
    
    module_config_t *config = get_module_config(load_img);
    if (config) 
    {
        fmsh_print("\nCurrent Module (%s):\r\n", config->name);
        if (config->module_type == MODULE_TYPE_BOOT) 
        {
            boot_header_t *boot_hdr = (boot_header_t *)mod_header;
            fmsh_print("Type:               %.8s\r\n", boot_hdr->type);
            fmsh_print("Module Version:     %.8s\r\n", boot_hdr->module_version);
            fmsh_print("Build Date:         %.12s\r\n", boot_hdr->build_date);
            fmsh_print("Build Time:         %.12s\r\n", boot_hdr->build_time);
            fmsh_print("FSBL Version:       %.8s\r\n", boot_hdr->fsbl_version);
            fmsh_print("FSBL Size:          %u bytes\r\n", boot_hdr->fsbl_size);
            fmsh_print("CABOOT Version:     %.8s\r\n", boot_hdr->caboot_version);
            fmsh_print("CABOOT Size:        %u bytes\r\n", boot_hdr->caboot_size);
            fmsh_print("BOOTBIN Version:    %.8s\r\n", boot_hdr->bootbin_version);
            fmsh_print("BOOTBIN Size:       %u bytes\r\n", boot_hdr->bootbin_size);
        } 
        else 
        {
            module_header_t *mod_hdr = (module_header_t *)mod_header;
            fmsh_print("Module Type: %.8s\r\n", mod_hdr->type);
            fmsh_print("Module Version: %.8s\r\n", mod_hdr->module_version);
            fmsh_print("Module Date: %.12s\r\n", mod_hdr->build_date);
            fmsh_print("Module Time: %.12s\r\n", mod_hdr->build_time);
            fmsh_print("MAP Size: %u bytes\r\n", mod_hdr->map_size);
            fmsh_print("BIN Size: %u bytes\r\n", mod_hdr->bin_size);
        }
    }

    fmsh_print("***************************************\r\n");
}

int emmc_read_module_header(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header, void *mod_header, LOAD_IMAGE_LIST load_img, EMMC_SYSTEM_TYPE_LIST system_type)
{
    int mod_blockCount;
    int module_sector = 0;
    int header_size;
    int mod_header_size = 0;
    const char *module_name = "";
    int res = FMSH_SUCCESS;

    if (!emmcPtr || !header || !mod_header) 
    {
        fmsh_print("Error: Invalid parameters\r\n");
        return FMSH_FAILURE;
    }

    module_config_t *config = get_module_config(load_img);
    
    if (!config) 
        return FMSH_FAILURE;

    int offset;
    offset = get_module_offset(load_img, header);
    module_name = config->name;
    module_sector = config->sector;

    if (config->module_type == MODULE_TYPE_BOOT) 
    {
        header_size = sizeof(boot_header_t);
    } 
    else 
    {
        header_size = sizeof(module_header_t);
    }

    mod_blockCount = MODULE_HEADER_SIZE / EMMC_BLOCK_SIZE;


    uint8_t *temp_buffer = (uint8_t*)malloc(mod_blockCount * EMMC_BLOCK_SIZE);
    if (!temp_buffer) 
    {
        fmsh_print("Error: Cannot allocate temp buffer\r\n");
        return FMSH_FAILURE;
    }

    memset(temp_buffer, 0, mod_blockCount * EMMC_BLOCK_SIZE);

    if (system_type == EMMC_SYSTEM_TYPE_FS) 
    {
        res = f_open(&file, filename, FA_READ);
        if (res != FR_OK) 
        {
            fmsh_print("Failed to open image file: %s (error: %d)\r\n", filename, res);
            free(temp_buffer);
            return FMSH_FAILURE;
        }
        if (filename != NULL && strcmp(filename, "0:image") == 0) 
        {
            res = f_lseek(&file, offset);
            if (res != FR_OK) 
            {
                fmsh_print("Failed to seek to file offset 0x%08X: %d\r\n", offset, res);
                f_close(&file);
                free(temp_buffer);
                return FMSH_FAILURE;
            }
        }
        res = f_read(&file, (void *)temp_buffer, header_size, &mod_header_size);
        if (res != FR_OK || mod_header_size !=  header_size ) 
        {
            fmsh_print("Failed to read module header: %d, header_size: %u\r\n", res, mod_header_size);
            f_close(&file);
            free(temp_buffer);
            return FMSH_FAILURE;
        }

        f_close(&file);
    }
    else
    {

        res = emmc_read(emmcPtr, temp_buffer, module_sector, mod_blockCount);
        if(res)
        {
            fmsh_print("Emmc read failed!\r\n");
            free(temp_buffer);
            return FMSH_FAILURE;
        }

    }

    memcpy(mod_header, temp_buffer, header_size);
    free(temp_buffer);
    return FMSH_SUCCESS;
}

int emmc_read_image_header(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header, EMMC_SYSTEM_TYPE_LIST system_type) 
{
    int res = FMSH_SUCCESS;
    int header_size;


    if (!emmcPtr || !header) 
    {
        fmsh_print("Error: Invalid parameters\r\n");
        return FMSH_FAILURE;
    }
    

    int blockCount;
    header_size = sizeof(image_header_t);
    blockCount = (header_size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;

    uint8_t *temp_buffer = (uint8_t*)malloc(blockCount * EMMC_BLOCK_SIZE);
    if (!temp_buffer) 
    {
        fmsh_print("Error: Cannot allocate temp buffer\r\n");
        return FMSH_FAILURE;
    }

    memset(temp_buffer, 0, blockCount * EMMC_BLOCK_SIZE);


    if (system_type == EMMC_SYSTEM_TYPE_FS)
    {
        res = f_open(&file, filename, FA_READ);
        if (res != FR_OK) 
        {
            fmsh_print("Failed to open image file: %s (error: %d)\r\n", filename, res);
            free(temp_buffer);
            return FMSH_FAILURE;
        }

        res = f_read(&file, (void *)temp_buffer, sizeof(image_header_t), &header_size);
        if (res != FR_OK || header_size != sizeof(image_header_t)) 
        {
            fmsh_print("Failed to read image header: %d, header_size: %u\r\n", res, header_size);
            f_close(&file);
            free(temp_buffer);
            return FMSH_FAILURE;
        }

        f_close(&file);
    }
    else
    {
        res = emmc_read(emmcPtr, temp_buffer, EMMC_IMAGE_SECTOR_START_ADDR, blockCount);
        if(res)
        {
            fmsh_print("Emmc read failed!\r\n");
            free(temp_buffer);
            return FMSH_FAILURE;
        }

    }

    memcpy(header, temp_buffer, sizeof(image_header_t));
    free(temp_buffer);
    return FMSH_SUCCESS;
}

int emmc_check_image_header_status(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header, EMMC_SYSTEM_TYPE_LIST system_type)
{
    int res = FMSH_SUCCESS;


    res = emmc_read_image_header(emmcPtr, filename, header, system_type);
    if (res)
    {
        fmsh_print("Emmc read header failed!\r\n");
        return FMSH_FAILURE;
    }

    res = verify_image_header(header);
    if(res)
    {
        fmsh_print("Emmc verify header failed!\r\n");
        return FMSH_FAILURE;
    }
    
    print_image_header_info(header);

    return FMSH_SUCCESS;
}



static int version_compare(const char *v1, const char *v2)
{
    char s1[16] = {0}, s2[16] = {0}; 
    int n1 = 0, n2 = 0;

    for (; *v1 == 'V' || *v1 == 'v'; v1++);
    for (; *v2 == 'V' || *v2 == 'v'; v2++);
    strncpy(s1, v1, sizeof(s1) - 1);
    strncpy(s2, v2, sizeof(s2) - 1);

    if (strchr(s1, '.') && strchr(s2, '.'))
    {
        char *t1 = NULL, *t2 = NULL;
        char *saveptr1 = NULL, *saveptr2 = NULL;
        int ret = 0;
        t1 = strtok_r(s1, ".", &saveptr1);
        t2 = strtok_r(s2, ".", &saveptr2);
        
        while ((t1 || t2) && ret == 0)
        {
            n1 = t1 ? atoi(t1) : 0;
            n2 = t2 ? atoi(t2) : 0;
            ret = n1 - n2;
            t1 = t1 ? strtok_r(NULL, ".", &saveptr1) : NULL;
            t2 = t2 ? strtok_r(NULL, ".", &saveptr2) : NULL;
        }
        return ret;
    }
    else
    {
        n1 = atoi(s1);
        n2 = atoi(s2);
        return n1 - n2;
    }
}

static FSdPsu_T emmchci;

int emmc_load_image(const char *filename, u8 device_id, LOAD_IMAGE_LIST load_img) 
{
    int res = FMSH_SUCCESS;
    image_header_t header;
    module_header_t mod_header;
    boot_header_t boot_header;
    FSdPsu_T *emmcPtr = NULL;
    u32 bin_crc;
    u32 calc_crc;
    char *version;
    int image_size, fileLen, blockCount;
    if(device_id == 0)
    {
        emmcPtr = &emmchci;
    }
    
    if(emmcPtr == NULL) 
    {
        fmsh_print("emmcPtr is NULL\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_initialize(device_id, emmcPtr);
    if (res)
    {
        fmsh_print("Emmc init failed!\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_check_image_header_status(emmcPtr,  filename, &header, EMMC_SYSTEM_TYPE_NOFS);
    if (res)
    {
        fmsh_print("Emmc image check failed\r\n");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    u32 crc_calc_len;
    if(load_img == LOAD_IMAGE_BOOT)
    {
        res = emmc_check_module_header_status(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_NOFS, load_img);
        if (res)
        {
            fmsh_print("Emmc module check failed\r\n");
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }
        bin_crc = boot_header.boot_crc;  
        version = boot_header.bootbin_version;
        u32 boot_size = boot_header.bootbin_size;
        u32 caboot_size = boot_header.caboot_size;
        u32 caboot_size_aligned = (caboot_size + (EMMC_BLOCK_SIZE -  1)) & ~(EMMC_BLOCK_SIZE -  1);
        crc_calc_len = boot_size + caboot_size_aligned;
    }
    else
    {
        res = emmc_check_module_header_status(emmcPtr, filename, &header, &mod_header, EMMC_SYSTEM_TYPE_NOFS, load_img);
        if (res)
        {
            fmsh_print("Emmc module check failed\r\n");
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }
        bin_crc = mod_header.module_crc;
        version = mod_header.module_version;
        u32 map_size = mod_header.map_size;
        u32 bin_size = mod_header.bin_size;
        u32 map_size_aligned = (map_size + (EMMC_BLOCK_SIZE -  1)) & ~(EMMC_BLOCK_SIZE -  1);
        u32 bin_size_aligned = (bin_size + (EMMC_BLOCK_SIZE -  1)) & ~(EMMC_BLOCK_SIZE -  1);
        crc_calc_len = map_size_aligned + bin_size_aligned;
    }

    u32 ddr_address;
    int module_sector, blk_cnt;
    module_config_t *config = get_module_config(load_img);
    if (!config) 
    {
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    ddr_address = config->ddr_addr;
    module_sector = config->sector;
    blk_cnt = (MODULE_HEADER_SIZE + crc_calc_len + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
    res = emmc_read(emmcPtr, (BYTE *)ddr_address, module_sector, blk_cnt);
    if(res)
    {
        fmsh_print("Emmc read failed!\r\n");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }
    fmsh_print("Module %s, Version %.8s \r\n", config->name, version);
    if(load_img != LOAD_IMAGE_28DRBIT)
    {
        if(version_compare(version, CMP_CRC_BIN_VERSION) > 0)
        {
            calc_crc = calculate_crc32((uint8_t*)ddr_address + MODULE_HEADER_SIZE , crc_calc_len);
            fmsh_print("(%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
            if(bin_crc != calc_crc)
            {
                fmsh_print("Crc compare error (%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
                emmc_cleanup(emmcPtr);
                return FMSH_FAILURE;
            }
        }
    }
    else if(load_img == LOAD_IMAGE_28DRBIT)
    {
        if(version_compare(version, CMP_CRC_FPGA_VERSION) > 0)
        {
            calc_crc = calculate_crc32((uint8_t*)ddr_address + MODULE_HEADER_SIZE , crc_calc_len);
            fmsh_print("(%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
            if(bin_crc != calc_crc)
            {
                fmsh_print("Crc compare error (%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
                emmc_cleanup(emmcPtr);
                return FMSH_FAILURE;
            }
        }
    }

    if(load_img == LOAD_IMAGE_BOOT)
    {
        res = load_emmc_firmware(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_NOFS, load_img);
    }
    else
    {
        res = load_emmc_firmware(emmcPtr, filename, &header, &mod_header, EMMC_SYSTEM_TYPE_NOFS,  load_img);
    }

    if(res)
    {
        fmsh_print("Load firmware failed!\r\n");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    emmc_cleanup(emmcPtr);

    switch (load_img) 
    {
        case LOAD_IMAGE_BOOT:
            flash_update_bootbin(DDR_BOOTBIN_ADDR, boot_header.bootbin_size);
            break;
        case LOAD_IMAGE_CA53:
            Jump_to_bootloader(DDR_A53_ADDR);
            break;
        case LOAD_IMAGE_CR50:
            res = cpu_release(FMZQ_CORE_RPU0, ADDR_R50);
            if(res != FMSH_SUCCESS)
            {
                fmsh_print("Load r50 failed!\r\n");
                return FMSH_FAILURE;
            }
            break;
        case LOAD_IMAGE_CR51:
            res= cpu_release(FMZQ_CORE_RPU1, ADDR_R51);
            if(res != FMSH_SUCCESS)
            {
                fmsh_print("Load r51 failed!\r\n");
                return FMSH_FAILURE;
            }
            break;
        case LOAD_IMAGE_28DRBIT:
            res = update_fpga(DDR_PL_ADDR, mod_header.bin_size);
            if(res != FMSH_SUCCESS)
            {
                fmsh_print("Load fpga failed!\r\n");
                return FMSH_FAILURE;
            }
            break;
        default:
            fmsh_print("Unknown type: %d\r\n", load_img);
            return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}


int emmc_check_module_header_status(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header, void *mod_header, EMMC_SYSTEM_TYPE_LIST system_type, LOAD_IMAGE_LIST load_img)
{
    int res = FMSH_SUCCESS;

    res = emmc_read_module_header(emmcPtr,  filename, header, mod_header, load_img, system_type);
    if(res)
    {
        fmsh_print("Emmc read failed!\r\n");
        return FMSH_FAILURE;
    } 

    res = verify_module_header(mod_header, load_img);
    if(res)
    {
        fmsh_print("Emmc verify header failed!\r\n");
        return FMSH_FAILURE;
    }

    print_module_version_info(mod_header, load_img);
    return FMSH_SUCCESS;
}


int emmc_update_module(const char *filename, u8 device_id, LOAD_IMAGE_LIST load_img) 
{
    int res = FMSH_SUCCESS;
    image_header_t header;
    module_header_t mod_header;
    boot_header_t boot_header;
    FSdPsu_T *emmcPtr = NULL;
    int module_size, fileLen;
    u32 bin_crc;
    u32 calc_crc;
    char *version;

    if(device_id == 0)
    {
        emmcPtr = &emmchci;
    }
    
    if(emmcPtr == NULL) 
    {
        fmsh_print("emmcPtr is NULL\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_initialize(device_id, emmcPtr);
    if (res)
    {
        fmsh_print("Emmc init failed!\r\n");
        return FMSH_FAILURE;
    }

    if(load_img == LOAD_IMAGE_BOOT)
    {
        res = emmc_check_module_header_status(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_FS, load_img);
        if (res)
        {
            fmsh_print("Emmc image check failed\r\n");
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }
        bin_crc = boot_header.boot_crc; 
        version = boot_header.bootbin_version;
    }
    else
    {
        res = emmc_check_module_header_status(emmcPtr, filename, &header, &mod_header, EMMC_SYSTEM_TYPE_FS, load_img);
        if (res)
        {
            fmsh_print("Emmc image check failed\r\n");
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }
        bin_crc = mod_header.module_crc;
        version = mod_header.module_version;
    }
    
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) 
    {
        fmsh_print("Failed to open image file: %s (error: %d)\r\n", filename, res);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    fileLen = f_size(&file);
    
    res = f_read(&file, (void *)TEMP_ADDR, fileLen, &module_size);
   
    if (res != FR_OK || module_size != fileLen) 
    {
        fmsh_print("Failed to read image: %d, module_size: %u\r\n", res, module_size);
        f_close(&file);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }
    fmsh_print("Version %.8s \r\n", version);
    if(load_img != LOAD_IMAGE_28DRBIT)
    {
        if(version_compare(version, CMP_CRC_BIN_VERSION) > 0)
        {
            calc_crc = calculate_crc32((uint8_t*)TEMP_ADDR + MODULE_HEADER_SIZE , module_size - MODULE_HEADER_SIZE);
            fmsh_print(" Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", calc_crc, bin_crc);
            if(bin_crc != calc_crc)
            {
                fmsh_print("ERR: Module_size %u byte, Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", module_size, calc_crc, module_size);
                f_close(&file);
                emmc_cleanup(emmcPtr);
                return FMSH_FAILURE;
            }
        }
    }
    else if(load_img == LOAD_IMAGE_28DRBIT)
    {
        if(version_compare(version, CMP_CRC_FPGA_VERSION) > 0)
        {
            calc_crc = calculate_crc32((uint8_t*)TEMP_ADDR + MODULE_HEADER_SIZE , module_size - MODULE_HEADER_SIZE);
            fmsh_print(" Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", calc_crc, bin_crc);
            if(bin_crc != calc_crc)
            {
                fmsh_print("ERR: Module_size %u byte, Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", module_size, calc_crc, module_size);
                f_close(&file);
                emmc_cleanup(emmcPtr);
                return FMSH_FAILURE;
            }
        }
    }
    int blk_cnt;
    int module_sector;

    module_config_t *config = get_module_config(load_img);
    
    if (!config) 
    {
        f_close(&file);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }
    
    module_sector = config->sector;
                                                                                                                                                                                                                       
    blk_cnt = (fileLen + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
    
    res = emmc_write(emmcPtr, (BYTE *)TEMP_ADDR, module_sector, blk_cnt);
    if(res)
    {
        fmsh_print("Emmc write failed!\r\n");
        f_close(&file);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    fmsh_print("Emmc write successful!\r\n");
    f_close(&file);
    emmc_cleanup(emmcPtr);
    fmsh_print("Remove %s!\r\n", filename);
    remove_file(filename);
    return FMSH_SUCCESS;
}


int emmc_update_image(const char *filename, u8 device_id)
{
    int res = FMSH_SUCCESS;
    image_header_t header;
    boot_header_t boot_header;
    module_header_t mod_header;
    FSdPsu_T *emmcPtr = NULL;
    int blockCount;
    int header_size;
    u32 bin_crc;
    u32 calc_crc;
    char *version;
    char fs_boot_version[8]; 
    char nofs_boot_version[8];

    bool upboot_flag = false;

    if(device_id == 0)
    {
        emmcPtr = &emmchci;
    }
    
    if(emmcPtr == NULL) 
    {
        fmsh_print("emmcPtr is NULL\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_initialize(device_id, emmcPtr);
    if (res)
    {
        fmsh_print("Emmc init failed!\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_check_image_header_status(emmcPtr, filename, &header,  EMMC_SYSTEM_TYPE_FS);
    if (res)
    {
        fmsh_print("Emmc image check failed\r\n");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    int size, offset, bin_size;
    int sector;
    u32 ddr_address;

    for (int i = 0; i < (sizeof(module_configs)/sizeof(module_configs[0])-1); i++) 
    {
        module_config_t *config = &module_configs[i];
        size = get_module_size(config->load_type, &header);
        offset = get_module_offset(config->load_type, &header);
        ddr_address = config->ddr_addr;
        fmsh_print("%s:\r\n", config->name);

        if(i == LOAD_IMAGE_BOOT)
        {
            res = emmc_check_module_header_status(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_FS, i);
            if (res)
            {
                fmsh_print("Emmc module check faile\r\n");
                emmc_cleanup(emmcPtr);
                return FMSH_FAILURE;
            }
            bin_crc = boot_header.boot_crc;   
            version = boot_header.bootbin_version;
            strncpy(fs_boot_version, boot_header.bootbin_version, sizeof(boot_header.bootbin_version)-1);
            fs_boot_version[sizeof(fs_boot_version)-1] = '\0';
            res = emmc_check_module_header_status(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_NOFS, i);
            if (res)
            {
                fmsh_print("Emmc module check faile\r\n");
                emmc_cleanup(emmcPtr);
                return FMSH_FAILURE;
            }
            strncpy(nofs_boot_version, boot_header.bootbin_version, sizeof(boot_header.bootbin_version)-1);
            nofs_boot_version[sizeof(nofs_boot_version)-1] = '\0';
            fmsh_print("fs_boot_version = %s nofs_boot_version = %s\r\n", fs_boot_version, nofs_boot_version);
            if (0 != strcmp(fs_boot_version, nofs_boot_version))
            {
                fmsh_print("Boot.bin need update\r\n");
                upboot_flag = true;

            }
            else
            {
                upboot_flag = false;
                fmsh_print("Boot.bin No need update!\r\n");

            }
            // res = load_emmc_firmware(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_FS, i);
        }
        else
        {
            res = emmc_check_module_header_status(emmcPtr, filename, &header, &mod_header, EMMC_SYSTEM_TYPE_FS, i);
            if (res)
            {
                fmsh_print("Emmc module check failed\r\n");
                emmc_cleanup(emmcPtr);
                return FMSH_FAILURE;
            }
            bin_crc = mod_header.module_crc;
            version = mod_header.module_version;
            // res = load_emmc_firmware(emmcPtr, filename, &header, &mod_header, EMMC_SYSTEM_TYPE_FS,  i);
        }
        res = f_open(&file, filename, FA_READ);
        if (res != FR_OK) 
        {
            fmsh_print("Failed to open image file: %s (error: %d)\r\n", filename, res);
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }
        res = f_lseek(&file, offset);
        if (res != FR_OK) 
        {
            fmsh_print("Failed to seek to file offset 0x%08X: %d\r\n", offset, res);
            f_close(&file);
            return FMSH_FAILURE;
        }

        res = f_read(&file, (void *)TEMP_ADDR, size, &bin_size);
   
        if (res != FR_OK || bin_size != size) 
        {
            fmsh_print("Failed to read %s: %d, bin_size: %u\r\n", config->name, res, bin_size);
            f_close(&file);
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }
        fmsh_print("Module %s, Version %.8s \r\n", config->name, version);
        if(i != LOAD_IMAGE_28DRBIT)
        {
            if(version_compare(version, CMP_CRC_BIN_VERSION) > 0)
            {
                calc_crc = calculate_crc32((uint8_t*)TEMP_ADDR + MODULE_HEADER_SIZE , size - MODULE_HEADER_SIZE);
                fmsh_print("(%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
                if(bin_crc != calc_crc)
                {
                    fmsh_print("Crc compare error (%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
                    emmc_cleanup(emmcPtr);
                    f_close(&file);
                    return FMSH_FAILURE;
                }
            }
        }
        else if(i == LOAD_IMAGE_28DRBIT)
        {
            if(version_compare(version, CMP_CRC_FPGA_VERSION) > 0)
            {
                calc_crc = calculate_crc32((uint8_t*)TEMP_ADDR + MODULE_HEADER_SIZE , size - MODULE_HEADER_SIZE);
                fmsh_print("(%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
                if(bin_crc != calc_crc)
                {
                    fmsh_print("Crc compare error (%s) Calculated CRC: 0x%08X  Readback bin crc: 0x%08X \r\n", config->name, calc_crc, bin_crc);
                    emmc_cleanup(emmcPtr);
                    f_close(&file);
                    return FMSH_FAILURE;
                }
            }
        }
        f_close(&file);
    }

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) 
    {
        fmsh_print("Failed to open image file: %s (error: %d)\r\n", filename, res);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }
    res = f_read(&file, (void *)TEMP_ADDR,  sizeof(image_header_t), &header_size);
    if (res != FR_OK || header_size != sizeof(image_header_t)) 
    {
        fmsh_print("Failed to read image header: %d, header_size: %u\r\n", res, header_size);
        f_close(&file);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    blockCount = (header_size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
    res = emmc_write(emmcPtr, (BYTE *)TEMP_ADDR, EMMC_IMAGE_SECTOR_START_ADDR, blockCount);
    if(res)
    {
        fmsh_print("Emmc write failed!\r\n");
        f_close(&file);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    for (int i = 0; i < (sizeof(module_configs)/sizeof(module_configs[0])); i++) 
    {
        module_config_t *config = &module_configs[i];
        
        offset = get_module_offset(config->load_type, &header);
        size = get_module_size(config->load_type, &header);
        ddr_address = config->ddr_addr;
        sector = config->sector;
        blockCount = (size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;

        res = f_lseek(&file, offset);
        if (res != FR_OK) 
        {
            fmsh_print("Failed to seek to file offset 0x%08X: %d\r\n", offset, res);        
            f_close(&file);
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }

        res = f_read(&file, (void *)TEMP_ADDR, size, &bin_size);
   
        if (res != FR_OK || bin_size != size) 
        {
            fmsh_print("Failed to read %s: %d, bin_size: %u\r\n", config->name, res, bin_size);
            f_close(&file);
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }

        blockCount = (bin_size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;
        res = emmc_write(emmcPtr, (BYTE *)TEMP_ADDR, sector, blockCount);
        if(res)
        {
            fmsh_print("Emmc write failed!\r\n");
            f_close(&file);
            emmc_cleanup(emmcPtr);
            return FMSH_FAILURE;
        }

        if (config->load_type == LOAD_IMAGE_BOOT )
        {
            if(upboot_flag)
            {
                res = emmc_check_module_header_status(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_NOFS, LOAD_IMAGE_BOOT);
                if (res)
                {
                    fmsh_print("Emmc module check failed\r\n");
                    f_close(&file);
                    emmc_cleanup(emmcPtr);
                    return FMSH_FAILURE;
                }

                res = load_emmc_firmware(emmcPtr, filename, &header, &boot_header, EMMC_SYSTEM_TYPE_NOFS, LOAD_IMAGE_BOOT);
                if(res)
                {
                    fmsh_print("Load firmware failed!\r\n");
                    f_close(&file);
                    emmc_cleanup(emmcPtr);
                    return FMSH_FAILURE;
                }

                fmsh_print("Start update Boot.bin!\r\n");
                flash_update_bootbin(DDR_BOOTBIN_ADDR, boot_header.bootbin_size);
            }
        }
    
    }

    fmsh_print("Emmc write successful!\r\n");
    fmsh_print("Remove %s!\r\n", filename);
    remove_file(filename);

    f_close(&file);
    emmc_cleanup(emmcPtr);
    return FMSH_SUCCESS;

}


int check_and_create_ue_conf(void)
{
    FILINFO fno;
    FIL file;
    UINT bytes_written;
    u8 device_id = 0;
    FSdPsu_T *emmcPtr = NULL;
    int res = FMSH_SUCCESS;
    const char *filename = "0:image";


    res = f_stat("0:ue.conf", &fno);
    if (res == FMSH_SUCCESS)
    {
        fmsh_print("ue.conf already exists, skip.\r\n");
        return FMSH_SUCCESS;
    }
    // else if (res != FR_NO_FILE)
    // {
    //     fmsh_print("Check ue.conf failed! Error: %d\r\n", res);
    //     return res;
    // }
    image_header_t header;
    int blk_cnt;
    u32 ddr_address;
    u32 module_sector;
    int size;
    
    if(device_id == 0)
    {
        emmcPtr = &emmchci;
    }
    
    if(emmcPtr == NULL) 
    {
        fmsh_print("emmcPtr is NULL\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_initialize(device_id, emmcPtr);
    if (res)
    {
        fmsh_print("Emmc init failed!\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_read_image_header(emmcPtr, filename, &header, EMMC_SYSTEM_TYPE_NOFS);
    if (res)
    {
        fmsh_print("Emmc read header failed!\r\n");
        return FMSH_FAILURE;
    }

    module_config_t *config = get_module_config(LOAD_IMAGE_UECONF);
    if (!config) 
        return FMSH_FAILURE;

    // offset = get_module_offset(load_img, header);
    ddr_address = config->ddr_addr;
    module_sector = config->sector;

    size = get_module_size(config->load_type, &header);
                                                                                                                                                                                                                       
    blk_cnt = (size + EMMC_BLOCK_SIZE - 1) / EMMC_BLOCK_SIZE;

    fmsh_print("size: %d bytes, blk_cnt: %d\r\n", size, blk_cnt );


    res = emmc_read(emmcPtr, (BYTE *)ddr_address, module_sector, blk_cnt);
    if(res)
    {
        fmsh_print("Emmc read failed!\r\n");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    res = f_open(&file, "0:ue.conf", FA_CREATE_NEW | FA_WRITE);
    if (res != FMSH_SUCCESS)
    {
        fmsh_print("Create ue.conf failed! Error: %d\r\n", res);
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    res = f_write(&file, (void *)ddr_address, size, &bytes_written);
    if (res == FMSH_SUCCESS && bytes_written == size)
    {
        fmsh_print("Write ue.conf success: %d bytes\r\n", bytes_written);
    }
    else
    {
        fmsh_print("Write ue.conf failed! Written: %d, Error: %d\r\n", bytes_written, res);
        f_close(&file);
        f_unlink("0:ue.conf");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    f_sync(&file);
    f_close(&file);
    emmc_cleanup(emmcPtr);
    fmsh_print("ue.conf created successfully!\r\n");
    return FMSH_SUCCESS;
}

int emmc_initialize(u8 device_id, FSdPsu_T *emmcPtr)
{
    int res = FMSH_SUCCESS;
    FSdPsu_Config_T *configPtr;

    configPtr = FSdPsu_LookupConfig(device_id);
    if (configPtr == NULL)
    {
        return FMSH_FAILURE;
    }

    res = FSdPsu_CfgInitialize(emmcPtr, configPtr);
    if (res)
    {
        return FMSH_FAILURE;
    }
    
    res = FSdPsu_CardInit(emmcPtr, NULL);
    if (res)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int emmc_cleanup(FSdPsu_T *emmcPtr)
{
    if (emmcPtr->card)
    {
        free(emmcPtr->card);
        emmcPtr->card = 0;
    }

    if (emmcPtr->desc)
    {
        free(emmcPtr->desc);
        emmcPtr->desc = 0;
    }

    return FMSH_SUCCESS;
}

int emmc_write(FSdPsu_T *emmcPtr, const BYTE *buff, LBA_t sector, UINT count)
{
    int res = FMSH_SUCCESS;
    char *pwbuf = NULL;
    int blkremain;
    int block_cnt = EMMC_BLOCK_LEN;
    int start = 0;

    pwbuf = (char *)buff;
    blkremain = count;

    while (blkremain > 0)
    {
        res = FSdPsu_Bwrite(emmcPtr, sector + start*block_cnt, block_cnt, (unsigned char*)pwbuf);
        
        if (res != block_cnt)
        {
            fmsh_print("EMMC Write(0x%x) Failed\r\n", sector + start*block_cnt);
            return FMSH_FAILURE;
        }
        blkremain = blkremain - block_cnt;
        pwbuf = pwbuf + block_cnt * EMMC_BLOCK_SIZE;
        start++;
    }
    
    return FMSH_SUCCESS;
}


int emmc_read(FSdPsu_T *emmcPtr, BYTE *buff, LBA_t sector, UINT count)
{
    int res = FMSH_SUCCESS;
    char *prbuf = (char *)buff;
    int blkremain = count;;
    int block_cnt = EMMC_BLOCK_LEN;
    int start = 0;

    if (!emmcPtr || !buff) 
    {
        fmsh_print("Error: emmc_read invalid parameters\r\n");
        return FMSH_FAILURE;
    }

    if(count < EMMC_BLOCK_LEN)
    {
        block_cnt = count;
    }

    while (blkremain > 0)
    {
        LBA_t current_sector = sector + start * block_cnt;
        res = FSdPsu_Bread(emmcPtr, current_sector, block_cnt,  (unsigned char*)prbuf);
        if (res != block_cnt)
        {
            fmsh_print("EMMC Read(0x%x) Failed\r\n", current_sector);
            return FMSH_FAILURE;
        }
        blkremain = blkremain - block_cnt;
        prbuf = prbuf + block_cnt * EMMC_BLOCK_SIZE;
        start++;
    }

    return FMSH_SUCCESS;
}

int emmc_read_module_version(u8 device_id, LOAD_IMAGE_LIST load_img) 
{
    int res = FMSH_SUCCESS;
    image_header_t header;
    module_header_t mod_header;
    boot_header_t boot_header;
    char *filename;
    FSdPsu_T *emmcPtr = NULL;
    int image_size, fileLen, blockCount;
    if(device_id == 0)
    {
        emmcPtr = &emmchci;
    }
    
    if(emmcPtr == NULL) 
    {
        fmsh_print("emmcPtr is NULL\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_initialize(device_id, emmcPtr);
    if (res)
    {
        fmsh_print("Emmc init failed!\r\n");
        return FMSH_FAILURE;
    }
    
    if(load_img == LOAD_IMAGE_BOOT)
    {
        res = emmc_read_module_header(emmcPtr, filename, &header, &boot_header,  load_img, EMMC_SYSTEM_TYPE_NOFS);
    }
    else
    {
        res = emmc_read_module_header(emmcPtr, filename, &header, &mod_header, load_img, EMMC_SYSTEM_TYPE_NOFS);
    }

    if (res)
    {
        fmsh_print("Emmc read module failed!\r\n");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    if(load_img == LOAD_IMAGE_BOOT)
    {
       print_module_version_info(&boot_header, load_img);
    }
    else
    {
        print_module_version_info(&mod_header, load_img);
    }

    emmc_cleanup(emmcPtr);
    return FMSH_SUCCESS;
}

int emmc_read_image_header_version(u8 device_id, image_header_t *header) 
{
    int res = FMSH_SUCCESS;
    // image_header_t header;
    // module_header_t mod_header;
    char *filename;
    FSdPsu_T *emmcPtr = NULL;
    int image_size, fileLen, blockCount;
    if(device_id == 0)
    {
        emmcPtr = &emmchci;
    }
    
    if(emmcPtr == NULL) 
    {
        fmsh_print("emmcPtr is NULL\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_initialize(device_id, emmcPtr);
    if (res)
    {
        fmsh_print("Emmc init failed!\r\n");
        return FMSH_FAILURE;
    }

    res = emmc_read_image_header(emmcPtr, filename, header, EMMC_SYSTEM_TYPE_NOFS);
    if (res)
    {
        fmsh_print("Emmc read module failed!\r\n");
        emmc_cleanup(emmcPtr);
        return FMSH_FAILURE;
    }

    // print_image_header_info(header);
    
    emmc_cleanup(emmcPtr);
    return FMSH_SUCCESS;
}