#ifndef LOAD_IMG_H
#define LOAD_IMG_H

#include <stdint.h>
#include <stdint.h>
#include "fmsh_common.h"
#include "ff.h"
#include "fmsh_sdhci_lib.h"
#include "fmsh_qspi.h"

#pragma pack(push, 1)

#define IMAGE_HEADER_SIZE  4096
#define MODULE_HEADER_SIZE 1024
#define MODULE_CA53_SIZE   (0x8000000/EMMC_BLOCK_SIZE) //128M
#define MODULE_BOOT_R5_BIT_SIZE  (0x4000000/EMMC_BLOCK_SIZE) //64M

#define EMMC_BLOCK_SIZE 512
#define MAX_FILENAME_LEN 32
#define EMMC_BLOCK_LEN   100
#define EMMC_IMAGE_SECTOR_START_ADDR  0x1800100
#define EMMC_IMAGE_SECTOR_CA53_ADDR    (EMMC_IMAGE_SECTOR_START_ADDR + MODULE_BOOT_R5_BIT_SIZE)
#define EMMC_IMAGE_SECTOR_CR50_ADDR    (EMMC_IMAGE_SECTOR_CA53_ADDR + MODULE_CA53_SIZE)
#define EMMC_IMAGE_SECTOR_CR51_ADDR    (EMMC_IMAGE_SECTOR_CR50_ADDR + MODULE_BOOT_R5_BIT_SIZE)
#define EMMC_IMAGE_SECTOR_BIT28DR_ADDR (EMMC_IMAGE_SECTOR_CR51_ADDR + MODULE_BOOT_R5_BIT_SIZE)
#define EMMC_IMAGE_SECTOR_UECONF_ADDR  (EMMC_IMAGE_SECTOR_BIT28DR_ADDR + MODULE_BOOT_R5_BIT_SIZE)

#define TEMP_ADDR               0x44000000

#define CMP_CRC_BIN_VERSION     "V1.0.6"
#define CMP_CRC_FPGA_VERSION     "26012803"

typedef struct {
    char state[4];          // 
    char signature[4];          // 'DGKJ'
    u32 header_crc;         // Header CRC
    u32 header_offset;      // Always 0
    u32 header_size;        // Always 128
    char board_type[32];         // 'zq28_silverfox_freertos'
    char build_date[12];         // Build date
    char build_time[12];         // Build time  
    char image_version[8];       // 'V0.0.1'
    char boot_version[8];        // 'V0.0.1'
    u32 boot_offset;         // boot file offset
    u32 boot_size;           // boot file size
    char a53_version[8];        // 'V0.0.1'
    u32 a53_offset;         // A53 file offset
    u32 a53_size;           // A53 file size
    char cr50_version[8];        // 'V0.0.1'
    u32 cr50_offset;        // CR50 file offset
    u32 cr50_size;          // CR50 file size
    char cr51_version[8];        // 'V0.0.1'
    u32 cr51_offset;        // CR51 file offset
    u32 cr51_size;          // CR51 file size
    char bit28dr_version[8];        // 'V0.0.1'
    u32 bit28dr_offset;     // 28dr file offset
    u32 bit28dr_size;       // 28dr file size
    u32 ue_offset;     // ue.conf file offset
    u32 ue_size;       // ue.conf file size
} image_header_t;


typedef struct {
    char signature[8];          // 'DGMODIMG'
    char type[8];          // 
    u32 module_header_crc;         // Module Header CRC
    u32 module_header_size;        // Always 128
    char build_date[12];         // Build date
    char build_time[12];         // Build time  
    char module_version[8];       // 'V0.0.1'
    char fsbl_version[8];        // 'V0.0.1'
    u32 fsbl_size;           // fsbl file size
    char caboot_version[8];        // 'V0.0.1'
    u32 caboot_size;           // caboot file size
    char bootbin_version[8];        // 'V0.0.1'
    u32 bootbin_size;           // bootbin file size
    u32 boot_crc;           //caboot+bootbin crc
} boot_header_t;


typedef struct {
    char signature[8];          // 'DGMODIMG'
    char type[8];          // 
    u32 module_header_crc;         // Module Header CRC
    u32 module_header_size;        // Always 128
    char build_date[12];         // Build date
    char build_time[12];         // Build time  
    char module_version[8];       // 'V0.0.1'
    u32 map_size;           // map file size
    char bin_version[8];        // 'V0.0.1'
    u32 bin_size;           // bin file size
    u32 module_crc;         //map+bin crc
} module_header_t;

#pragma pack(pop)


typedef enum
{
    LOAD_IMAGE_BOOT = 0,
    LOAD_IMAGE_CA53,
    LOAD_IMAGE_CR50,
    LOAD_IMAGE_CR51,
    LOAD_IMAGE_28DRBIT,
    LOAD_IMAGE_UECONF,
    LOAD_IMAGE_ALL,
}LOAD_IMAGE_LIST;

typedef enum
{
    EMMC_SYSTEM_TYPE_FS=0,
    EMMC_SYSTEM_TYPE_NOFS,
}EMMC_SYSTEM_TYPE_LIST;

typedef enum {
    MODULE_TYPE_BOOT = 0,
    MODULE_TYPE_STANDARD,
} module_type_t;

typedef struct {
    LOAD_IMAGE_LIST load_type;      
    module_type_t module_type;  
    const char *name;
    u32 ddr_addr;
    u32 sector;
} module_config_t;

typedef enum {
    BAND_FREERTOS = 0,
    BAND_LINUX_SAMPLE_200,
    BAND_LINUX_SAMPLE_245dot76,
    BAND_LINUX_KGRHOP,
    BAND_LINUX_KGRDS,
    BAND_LINUX_SAMPLE_MAX,
} bandinfo_t;

int get_module_offset(LOAD_IMAGE_LIST type, image_header_t *header);
int get_module_size(LOAD_IMAGE_LIST type, image_header_t *header);
char* get_module_version(LOAD_IMAGE_LIST type, image_header_t *header);
module_config_t * get_module_config(LOAD_IMAGE_LIST load_img);
int verify_image_header(image_header_t *header);
int verify_module_header(void *mod_header,  LOAD_IMAGE_LIST load_img);
int load_file_to_ddr(FSdPsu_T *emmcPtr, const char *filename, u32 file_offset, u32 file_size, u32 ddr_address, EMMC_SYSTEM_TYPE_LIST system_type, LBA_t sector, UINT count);
int load_emmc_firmware(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header,  void *mod_header, EMMC_SYSTEM_TYPE_LIST system_type, LOAD_IMAGE_LIST load_img);
void print_image_header_info(image_header_t *header);
void print_module_version_info(void *mod_header, LOAD_IMAGE_LIST load_img);
int emmc_read_module_header(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header, void *mod_header, LOAD_IMAGE_LIST load_img, EMMC_SYSTEM_TYPE_LIST system_type);
int emmc_read_image_header(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header, EMMC_SYSTEM_TYPE_LIST system_type);
int emmc_check_image_header_status(FSdPsu_T *emmcPtr, const char *filename, image_header_t *header,  EMMC_SYSTEM_TYPE_LIST system_type);
int emmc_load_image(const char *filename, u8 device_id, LOAD_IMAGE_LIST load_img);
int emmc_update_module(const char *filename, u8 device_id, LOAD_IMAGE_LIST load_img);
int emmc_update_image(const char *filename, u8 device_id);
int emmc_read_module_version(u8 device_id, LOAD_IMAGE_LIST load_img);
int emmc_read_image_header_version(u8 device_id, image_header_t *header);
int emmc_check_module_header_status(FSdPsu_T *emmcPtr,const char *filename,  image_header_t *header, void *mod_header, EMMC_SYSTEM_TYPE_LIST system_type, LOAD_IMAGE_LIST load_img);
int emmc_read(FSdPsu_T *emmcPtr, BYTE *buff, LBA_t sector, UINT count);
int emmc_write(FSdPsu_T *emmcPtr, const BYTE *buff, LBA_t sector, UINT count);
int emmc_initialize(u8 device_id, FSdPsu_T *emmcPtr);
int emmc_cleanup(FSdPsu_T *emmcPtr);
int check_and_create_ue_conf(void);
#endif 
// EMMC_IMAGE_H