#include "l1c_r50.h"
#include "fmsh_common.h"

extern uint32_t g_ul_data_stat_flag;
extern uint32_t g_dl_data_stat_flag;
extern uint32_t g_log_fn_num;
extern uint32_t g_log_sfn_num;
extern uint32_t g_log_slot_bitmap0;
extern uint32_t g_log_slot_bitmap1;
extern uint32_t g_log_slot_bitmap2;
extern uint32_t g_l1c_log_pos;
extern uint32_t g_l1c_data_cap_flag;
extern uint32_t g_pusch_payload_log_flag;
extern uint32_t g_pdsch_payload_log_flag;
void start_ul_stat()
{
    fmsh_print("start l1c ul rate stat\r\n");
    g_ul_data_stat_flag = 1; 
}

void stop_ul_stat()
{
    fmsh_print("stop l1c ul rate stat\r\n");
    g_ul_data_stat_flag = 0; 
}

void start_dl_stat()
{
    fmsh_print("start l1c dl rate stat\r\n");
    g_dl_data_stat_flag = 1; 
}

void stop_dl_stat()
{
    fmsh_print("stop l1c dl rate stat\r\n");
    g_dl_data_stat_flag = 0; 
}

void l1c_slot_log(u32 log_fn, u32 log_sfn, u32 log_slot_bitmap2, u32 log_slot_bitmap1, u32 log_slot_bitmap0)
{
    fmsh_print("l1c slot log config\r\n");

    g_log_fn_num = log_fn;
    g_log_sfn_num = log_sfn;
    g_log_slot_bitmap2 = log_slot_bitmap2;
    g_log_slot_bitmap1 = log_slot_bitmap1;
    g_log_slot_bitmap0 = log_slot_bitmap0;

    fmsh_print("frame num: %u, subframe num:%u\r\n", g_log_fn_num, g_log_sfn_num);
    fmsh_print("slot bitmap: 0x%x, 0x%x, 0x%x\r\n", g_log_slot_bitmap2, g_log_slot_bitmap1, g_log_slot_bitmap0);
}

void l1c_slot_log_flag(u32 log_flag)
{
    g_l1c_log_pos = log_flag;
    fmsh_print("L1c log flag: 0x%x\r\n", log_flag);
}

void l1c_data_cap_flag(u32 log_flag)
{
    g_l1c_data_cap_flag = log_flag;
    fmsh_print("L1c ddr cap flag: 0x%x\r\n", log_flag);
}
void open_pusch_payload_log()
{
    fmsh_print("open l1c pusch payload log\r\n");
    g_pusch_payload_log_flag = 1; 
}

void close_pusch_payload_log()
{
    fmsh_print("close l1c pusch payload log\r\n");
    g_pusch_payload_log_flag = 0; 
}

void open_pdsch_payload_log()
{
    fmsh_print("open l1c pdsch payload log\r\n");
    g_pdsch_payload_log_flag = 1; 
}

void close_pdsch_payload_log()
{
    fmsh_print("close l1c pdsch payload log\r\n");
    g_pdsch_payload_log_flag = 0; 
}
