/* =============================================================
 * 文件作用：A53 shell 命令实现原型声明
 *
 * 对应实现：CA53/a53/shell_cmd_a53_impl.c
 * 调用方：FM_ZQ_bsp/thirdpartlib/LetterShell/src/shell_export_cmd_a53.c
 * ============================================================= */

#ifndef SHELL_CMD_A53_IMPL_H
#define SHELL_CMD_A53_IMPL_H

#include "shell.h"
#include "fmsh_common.h"
#include "string.h"
#include "fmsh_sdmmc_example.h"
#include "eeprom_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "mem_common.h"
#include "ddr_capture.h"
#include "fmsh_uart_at.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "uartns550.h"
#include "fmsh_rtc_mix.h"
#include "load_img.h"
#include "powermeter.h"
#include "mem2bin.h"
#include "nr_shm_oam.h"
#include "fmsh_mailbox_ipips.h"


#if (SHELL_CMD_MASTER == 0)

void readreg(u64 addr);
void writereg(u64 addr, u64 Data);

void fpga_readRegs(u32 offSet);
void fpga_writeRegs(u32 offSet, u32 Data);

void cmd_showversion(void);
void cmd_update_img(void);
void cmd_update_module(char *filename, u8 load_id);
void cmd_ls(char *path);
void cmd_mem2bin(u8 granularity, u8 direction, u32 number, u8 frame_count);

void cmd_set_rtc_clock(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec);
void cmd_get_rtc_clock(void);

void cmd_getbandinfo(void);
void cmd_setbandinfo(u8 bandid);

void cmd_stack_main(int argc, char *argv[]);

void cmd_get_tod_baudrate(void);
void cmd_set_tod_baudrate(u8 baudrate_id);

void cmd_set_ant_type(u8 ant_type);
void cmd_get_ant_type(void);

void cmd_set_tod_print_level(u8 print_type);
void cmd_get_tod_print_level(void);

void cmd_uart_at_print(int argc, char *argv[]);

#ifdef UDP_TOD_TEST
void cmd_udp_send_time(int argc, char *argv[]);
#endif

#ifdef PPS_TIME_TEST
void cmd_pps_delta_test(void);
#endif

#endif /* SHELL_CMD_MASTER == 0 */

#endif /* SHELL_CMD_A53_IMPL_H */
