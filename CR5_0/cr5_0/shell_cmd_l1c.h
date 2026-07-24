#ifndef __SHELL_CMD_L1C_H__
#define __SHELL_CMD_L1C_H__
#include "shell.h"

void start_ul_stat();
void stop_ul_stat();
void start_dl_stat();
void stop_dl_stat();
void l1c_slot_log(u32 log_fn, u32 log_sfn, u32 log_slot_bitmap2, u32 log_slot_bitmap1, u32 log_slot_bitmap0);
void l1c_slot_log_flag(u32 log_flag);
void l1c_data_cap_flag(u32 log_flag);
void open_pusch_payload_log();
void close_pusch_payload_log();
void open_pdsch_payload_log();
void close_pdsch_payload_log();
#endif

