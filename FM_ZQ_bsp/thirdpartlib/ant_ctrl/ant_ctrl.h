/* Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. */

#ifndef __ANT_CTRL_H_
#define __ANT_CTRL_H_

#pragma once

#include "ant_common_define.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"

#include "netif/etharp.h"
#include "fmsh_gmacpsif.h"
#include "fadapter.h"
#include "fmsh_print.h"


#define PRINT_BYTES 8
#define MAX_MAP_SIZE 100

typedef struct
{
    int8_t valid;
    int16_t year;
    int8_t month;
    int8_t day;
    int32_t utc_second;
    double longitude;
    double latitude;
    double elevation;
    double E_speed;
    double N_speed;
    double D_speed;
} TOD_PARAS;

// Map结构定义
typedef struct {
    uint8_t keys[MAX_MAP_SIZE];
    uint8_t values[MAX_MAP_SIZE];
    size_t size;
} MapUint8Uint8;

typedef struct {
    uint8_t keys[MAX_MAP_SIZE];
    int32_t values[MAX_MAP_SIZE];
    size_t size;
} MapUint8Int32;

typedef struct {
    uint8_t keys[MAX_MAP_SIZE];
    char values[MAX_MAP_SIZE][256]; // 固定长度字符串
    size_t size;
} MapUint8String;

// 全局状态结构
typedef struct {
    uint8_t self_check_state;
    uint8_t ant_master_or_slave;
    uint16_t broadcast_count;
    uint16_t web_ant_msg;
    uint8_t ant_type;
    uint8_t eph_value;
    bool broascast_flag;
    bool all_check_flag;
    bool ephe_flag;
    bool manu_recv_flag;
    bool all_check_recv_flag;
    uint8_t ue_min_working_elevation;

    MapUint8Uint8 para_resp_map;
    MapUint8Int32 all_check_resp_map;
    MapUint8Uint8 ant_report_map;
    MapUint8String factory_info_map;
    MapUint8String all_check_factory_info_map;
    
    ANT_EPHEMERIS_DATAS m_ephemeris_info;
    BROADCAST_SEARCH_RESP m_bcast_info;

    bool web_waiting_combo;
    uint8_t web_all_check_buf[MAX_UDP_MESSAGE_SIZE];
    int32_t web_all_check_len;
    uint8_t web_factory_buf[MAX_UDP_MESSAGE_SIZE];
    int32_t web_factory_len;
} AntCtrlState;


typedef struct {
    uint16_t  satellite_id;    // 卫星编号
    double    azimuth;       // 卫星方位角 (度)
    double    pitch_angle;   // 卫星俯仰角 (度)
    uint32_t  recv_freq;     // 接收频率 (Hz)
    uint32_t  send_freq;     // 发射频率 (Hz)
    uint8_t   ant_status;    // 天线状态 0:未就绪 1:就绪
} ANT_INFO;

// 函数声明
void ant_ctrl_init(void);
void ant_ctrl_cleanup(void);
void ant_ctrl_byte4_data_format_conversion(uint8_t* data, uint32_t value, int32_t start_index);
int32_t ant_ctrl_byte4_data_format_conversion_rollback(uint8_t* data, int32_t start_index);
void ant_ctrl_byte2_data_format_conversion(uint8_t* data, uint16_t value, int32_t start_index);
int16_t ant_ctrl_byte2_data_format_conversion_rollback(uint8_t* data, int32_t start_index);
void ant_ctrl_escape_bytes_to_send_msg(uint8_t *src, uint8_t *dst, int32_t *len);
void ant_ctrl_escape_bytes_to_recv_msg(uint8_t *src, uint8_t *dst, uint32_t *len, uint32_t pack_len);
void ant_ctrl_parse_udp_ant_resp_msg(uint8_t *msg, int32_t msg_len);
void ant_ctrl_parse_udp_web_resp_msg(uint8_t *msg, int32_t msg_len);
int32_t ant_ctrl_ant_report_resp_msg(void);
int32_t ant_ctrl_send_all_check_msg(void);
int32_t ant_ctrl_send_factory_info_msg(void);
int32_t ant_ctrl_parabolic_ant_switch(int32_t ant_id);
void ant_ctrl_write_ant_ability_to_file(void);
void ant_ctrl_print_msg(uint8_t *msg, int32_t len);
void ant_ctrl_reset_device(void);
void ant_ctrl_check_ant_crc_word(uint8_t *msg, int32_t len, uint32_t *crc_word);
void ant_ctrl_set_ue_min_working_elevation(uint8_t angle);
uint8_t ant_ctrl_get_ue_min_working_elevation();
void ant_ctrl_get_ant_info(ANT_INFO* info);

// Map操作辅助函数
void map_uint8_uint8_insert(MapUint8Uint8* map, uint8_t key, uint8_t value);
uint8_t map_uint8_uint8_get(MapUint8Uint8* map, uint8_t key);
void map_uint8_int32_insert(MapUint8Int32* map, uint8_t key, int32_t value);
int32_t map_uint8_int32_get(MapUint8Int32* map, uint8_t key);
void map_uint8_string_insert(MapUint8String* map, uint8_t key, const char* value);
const char* map_uint8_string_get(MapUint8String* map, uint8_t key);
void map_uint8_uint8_clear(MapUint8Uint8* map);
void map_uint8_int32_clear(MapUint8Int32* map);
void map_uint8_string_clear(MapUint8String* map);
void ant_ctrl_init_func(void);
int32_t ant_ctrl_send_ant_msg_for_test(void);

#endif