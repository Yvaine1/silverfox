/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#ifndef _FPGA_H_
#define _FPGA_H_

#define DDR_TEMP_ADDR     (0x44000000)
#define DDR_PL_ADDR       (0x41000000)   
#define DDR_BOOTBIN_ADDR  (0x46000000)   
#define DDR_A53_ADDR      (0x19000000) 
#define ADDR_R50          (0x09000000)
#define ADDR_R51          (0x11000000)

#define FILE_BIT      "0:zu28dr.bit"
#define FILE_BOOTBIN  "0:BOOT.bin"
#define A53_FILE      "0:CA53.bin"
#define FILE_R50      "0:CR50.bin"
#define FILE_R51      "0:CR51.bin"

#define FLASH_BOOTBIN_START_ADDRESS      (0x0)
#define FLASH_BOOTBIN_ADDRESS            (0x40000)
#define FLASH_BOOTBIN2_START_ADDRESS     (0x200000)
#define FSBL_DUMMY_PL_ADDR               (0xFFFFFFFFU)
#define GTC_CLK_FREQ                     99990000U

// #define FPGA_BASE       0xA0000000


struct build_info
{
    char compile_time[64];
    char version_info[64];
};


int load_bitstream(u32 bitInDdrAddr, u32 bit_file_size);
int Load_PLBit(u32 load_addr,u32 *bin_size);
int Load_CA53(u32 load_addr);
int Load_bootbin(u32 load_addr, u32 *bin_size);
int update_r5(u8 device_id,u32 load_addr);
int update_fpga(u32 boot_addr, u32 bitSize);
void Jump_to_bootloader (u32 boot_addr);
int flash_update_bootbin(u32 boot_addr, u32 bitSize);
int flash_update_bootbin2(u32 boot_addr, u32 bitSize);
int read_file(char *path);
void show_version(void);

#endif	/* _FPGA_H_ */