#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ant_ctrl.h"
#include "ant_link_mgr.h"
#include "fmsh_print.h"

AntCtrlState g_ant_state;
static ANT_INFO g_ant_info;

void map_uint8_uint8_insert(MapUint8Uint8* map, uint8_t key, uint8_t value)
{
    if (map == NULL || map->size >= MAX_MAP_SIZE) return;
    for (size_t i = 0; i < map->size; i++)
    {
        if (map->keys[i] == key)
        {
            map->values[i] = value;
            return;
        }
    }

    map->keys[map->size] = key;
    map->values[map->size] = value;
    map->size++;
}

uint8_t map_uint8_uint8_get(MapUint8Uint8* map, uint8_t key) {
    for (size_t i = 0; i < map->size; i++) {
        if (map->keys[i] == key) {
            return map->values[i];
        }
    }
    return 0;
}

void map_uint8_int32_insert(MapUint8Int32* map, uint8_t key, int32_t value) {
    if (map->size >= MAX_MAP_SIZE) return;
    
    for (size_t i = 0; i < map->size; i++) {
        if (map->keys[i] == key) {
            map->values[i] = value;
            return;
        }
    }
    
    map->keys[map->size] = key;
    map->values[map->size] = value;
    map->size++;
}

int32_t map_uint8_int32_get(MapUint8Int32* map, uint8_t key) {
    for (size_t i = 0; i < map->size; i++) {
        if (map->keys[i] == key) {
            return map->values[i];
        }
    }
    return 0;
}

void map_uint8_string_insert(MapUint8String* map, uint8_t key, const char* value) {
    if (map->size >= MAX_MAP_SIZE) return;
    
    for (size_t i = 0; i < map->size; i++) {
        if (map->keys[i] == key) {
            strncpy(map->values[i], value, sizeof(map->values[i]) - 1);
            map->values[i][sizeof(map->values[i]) - 1] = '\0';
            return;
        }
    }
    
    map->keys[map->size] = key;
    strncpy(map->values[map->size], value, sizeof(map->values[0]) - 1);
    map->values[map->size][sizeof(map->values[0]) - 1] = '\0';
    map->size++;
}

const char* map_uint8_string_get(MapUint8String* map, uint8_t key) {
    for (size_t i = 0; i < map->size; i++) {
        if (map->keys[i] == key) {
            return map->values[i];
        }
    }
    return NULL;
}

void map_uint8_uint8_clear(MapUint8Uint8* map) {
    map->size = 0;
}

void map_uint8_int32_clear(MapUint8Int32* map) {
    map->size = 0;
}

void map_uint8_string_clear(MapUint8String* map) {
    map->size = 0;
}

// 初始化函数
void ant_ctrl_init(void) {
    memset(&g_ant_state, 0, sizeof(g_ant_state));
    
    g_ant_state.self_check_state = 0;
    g_ant_state.ant_master_or_slave = 0;
    g_ant_state.broadcast_count = 0;
    g_ant_state.web_ant_msg = 0;
    g_ant_state.ant_type = 0;
    g_ant_state.eph_value = 0;
    g_ant_state.ue_min_working_elevation = 0;
    g_ant_state.broascast_flag = false;
    g_ant_state.all_check_flag = false;
    g_ant_state.ephe_flag = true;
    g_ant_state.manu_recv_flag = false;
    g_ant_state.all_check_recv_flag = false;

    memset(&g_ant_state.m_ephemeris_info, 0, sizeof(ANT_EPHEMERIS_DATAS));
    memset(&g_ant_state.m_bcast_info, 0, sizeof(BROADCAST_SEARCH_RESP));
    memset(&g_ant_info, 0, sizeof(ANT_INFO));

    g_ant_state.web_waiting_combo = false;
    g_ant_state.web_all_check_len = 0;
    g_ant_state.web_factory_len = 0;
    memset(g_ant_state.web_all_check_buf, 0, sizeof(g_ant_state.web_all_check_buf));
    memset(g_ant_state.web_factory_buf, 0, sizeof(g_ant_state.web_factory_buf));

    // 初始化map
    g_ant_state.para_resp_map.size = 0;
    g_ant_state.all_check_resp_map.size = 0;
    g_ant_state.ant_report_map.size = 0;
    g_ant_state.factory_info_map.size = 0;
    g_ant_state.all_check_factory_info_map.size = 0;
}

void ant_ctrl_cleanup(void) {
    // 清理资源
}

uint64_t htonll(uint64_t val)
{
    return (((uint64_t)htonl(val)) << 32) + htonl(val >> 32);
}

double hton_double(double val)
{
    uint64_t result = htonll(*((uint64_t *)&val));
    return *((double *)&result);
}

void ant_ctrl_byte4_data_format_conversion(uint8_t* data, uint32_t value, int32_t start_index) {
    data[start_index] = (value >> 24) & 0xff;
    data[start_index + 1] = (value >> 16) & 0xff;
    data[start_index + 2] = (value >> 8) & 0xff;
    data[start_index + 3] = value & 0xff;
}

void ant_ctrl_byte2_data_format_conversion(uint8_t* data, uint16_t value, int32_t start_index) {
    data[start_index] = (value >> 8) & 0xff;
    data[start_index + 1] = value & 0xff;
}

int16_t ant_ctrl_byte2_data_format_conversion_rollback(uint8_t* data, int32_t start_index)
{
    int16_t value = 0;
    // 将两个字节组合成一个16位有符号整数
    // 高位字节左移8位，然后与低位字节进行或操作
    value = ((int16_t)(data[start_index]) << 8) | (int16_t)(data[start_index + 1]);
    
    return value;
}

int32_t ant_ctrl_byte4_data_format_conversion_rollback(uint8_t* data, int32_t start_index)
{
    int32_t value = 0;
    value = ((int32_t)(data[start_index]) << 24) | ((int32_t)(data[start_index + 1]) << 16) |
            ((int32_t)(data[start_index + 2]) << 8) | (int32_t)(data[start_index + 3]);
    return value;
}

void ant_ctrl_escape_bytes_to_send_msg(uint8_t *src, uint8_t *dst, int32_t *len)
{
    dst[0] = src[0];
    int32_t i = 1;
    int32_t j = 1;
    
    while(i < *len - 1)
    {
        switch(src[i])
        {
            case 0x7d:
                dst[j] = 0x7d;
                dst[j+1] = 0x5d;
                j += 2;
                break;
            case 0x7e:
                dst[j] = 0x7d;
                dst[j+1] = 0x5e;
                j += 2;
                break;
            default:
                dst[j] = src[i];
                j += 1;
                break;
        }
        i += 1;
    }
    dst[j] = src[i];
    *len = j + 1;
}

void ant_ctrl_escape_bytes_to_recv_msg(uint8_t *src, uint8_t *dst, uint32_t *len, uint32_t pack_len)
{
    dst[0] = src[0];
    int i = 1;
    int j = 1;
    int count = 0;

    // 统计转义序列的数量
    for(int z = 1; z < pack_len - 1; z++)
    {
        if((src[z] == 0x7D && src[z+1] == 0x5E) || (src[z] == 0x7D && src[z+1] == 0x5D))
        {
            count++;
        }
    }

    // 处理转义序列
    while(i < pack_len - count)
    {
        if(src[i] == 0x7D && src[i+1] == 0x5E)
        {
            dst[j] = 0x7E;  // 转义序列 0x7D 0x5E → 0x7E
            i += 2;
        }
        else if(src[i] == 0x7D && src[i+1] == 0x5D)
        {
            dst[j] = 0x7D;  // 转义序列 0x7D 0x5D → 0x7D
            i += 2;
        }
        else
        {
            dst[j] = src[i];  // 普通字节直接复制
            i += 1;
        }
        j += 1;
    }

    // 复制剩余字节
    for(int z = j; z < j + count; z++)
    {
        dst[z] = src[i + z - j];
    }

    *len = pack_len - count;
}

void ant_ctrl_print_msg(uint8_t *msg, int32_t len)
{
    char element[MAX_UDP_MESSAGE_SIZE * 3 + 1]; // 每个字节2个字符+空格，+1用于结束符
    char temp[4]; // 临时存储单个字节的十六进制表示
    int element_pos = 0;
    
    memset(element, 0, sizeof(element));
    
    for(int32_t i = 0; i < len; i++)
    {
        // 格式化单个字节为十六进制
        snprintf(temp, sizeof(temp), "%02X ", msg[i]);
        
        // 添加到元素字符串
        strcat(element, temp);
        element_pos += 3; // 2个字符+1个空格
        
        // 每PRINT_BYTES个字节换行打印
        if(((i + 1) % PRINT_BYTES == 0))
        {
            fmsh_print("%s \r\n", element);
            memset(element, 0, sizeof(element));
            element_pos = 0;
        }
    }
    
    // 打印剩余的不够PRINT_BYTES的部分
    if(len % PRINT_BYTES != 0 && element[0] != '\0')
    {
        fmsh_print("%s \r\n", element);
    }
}

void ant_ctrl_check_ant_crc_word(uint8_t *msg, int32_t len, uint32_t *crc_word)
{
    *crc_word = 0;
    
    for(int32_t i = 3; i < len - 2; i++)
    {
        *crc_word += msg[i];
    }
}

extern void handle_ant_udp_rcv_msg(const ant_udp_rcv_msg_t* result);

static const char* ant_ctrl_get_para_name(uint8_t keyword)
{
    switch(keyword)
    {
        case ANT_WRITE_OPEN:                      return "天线状态";
        case ANT_WRITE_SWITCH_STAR:               return "切星指令";
        case ANT_WRITE_KA_SEND_FREQ:              return "发射频率";
        case ANT_WRITE_SEND_BWP_CENTER_FREQ:      return "发射BWP中心频率";
        case ANT_WRITE_KA_RCV_FREQ:               return "接收频率";
        case ANT_READ_RCV_BWP_CENTER_FREQ:        return "接收BWP中心频率";
        case ANT_READ_KA_BUC_ATTENUATION:         return "BUC衰减";
        case ANT_READ_KA_BDC_ATTENUATION:         return "BDC衰减";
        case ANT_READ_KA_SWITCH:                  return "天线开关";
        case ANT_WRITE_HIGHRAIL_RCV_POLARIZATION: return "高轨接收极化";
        case ANT_WRITE_RSRP:                      return "参考信号功率";
        case ANT_WRITE_AZIMUTH_PITCHANGLE:        return "波束指向角";
        case ANT_WRITE_SEND_POLARIZATION:         return "发射极化";
        case ANT_WRITE_RCV_POLARIZATION:          return "接收极化";
        case ANT_WRITE_SATELLITE_ID:              return "卫星号";
        case ANT_WRITE_BEACON_FREQ:               return "信标频率";
        case ANT_WRITE_C_BUC_ATTENUATION:         return "C-BUC衰减";
        case ANT_WRITE_C_BDC_ATTENUATION:         return "C-BDC衰减";
        case ANT_WRITE_C_SWITCH:                  return "C天线开关";
        case ANT_WRITE_C_SEND_FREQ:               return "C发射频率";
        case ANT_WRITE_C_RCV_FREQ:                return "C接收频率";
        case ANT_WRITE_VECTOR_POSITION:          return "载体位置";
        case ANT_WRITE_SATELLITE_LON_AND_LAT:     return "卫星经度";
        case ANT_WRITE_MASTER_OR_SLAVE:           return "主从天线";
        case ANT_WRITE_RUNNING_STATE:             return "运行状态";
        case ANT_WRITE_NET_STATE:                 return "入网状态";
        default:                                  return "未知参数";
    }
}

static const char* ant_ctrl_get_result_name(uint8_t result)
{
    switch(result)
    {
        case RESP_SUCCESS:             return "成功";
        case RESP_FAIL:                return "失败";
        case RESP_TIMEOUT:             return "超时";
        case RESP_UNKNOWN:             return "未知";
        case RESP_OBJECT_NOT_EXIST:    return "被控对象不存在";
        case RESP_PARA_ERROR:          return "参数错误";
        case RESP_CONDITION_NOT_HAVE:  return "条件不具备";
        case RESP_REFUSE_EXECUTE:      return "设备拒绝执行";
        case RESP_FRAME_ERROR:         return "帧格式错误";
        default:                       return "未定义结果";
    }
}

static void ant_ctrl_send_para_set_resp_to_web(uint8_t keyword, uint8_t result)
{
    char resp_str[128] = {0};

    snprintf(resp_str, sizeof(resp_str), "%s:%s\r\n",
             ant_ctrl_get_para_name(keyword), ant_ctrl_get_result_name(result));
    ant_link_mgr_send_udp_msg_to_web(resp_str, strlen(resp_str));
}

static void ant_ctrl_try_send_combo_to_web(void)
{
    if(g_ant_state.web_all_check_len > 0 && g_ant_state.web_factory_len > 0)
    {
        uint8_t combo[MAX_UDP_MESSAGE_SIZE * 2];
        memcpy(combo, g_ant_state.web_factory_buf, g_ant_state.web_factory_len);
        memcpy(combo + g_ant_state.web_factory_len, g_ant_state.web_all_check_buf, g_ant_state.web_all_check_len);
        ant_link_mgr_send_udp_msg_to_web((char *)combo, g_ant_state.web_factory_len + g_ant_state.web_all_check_len);
        g_ant_state.web_all_check_len = 0;
        g_ant_state.web_factory_len = 0;
        g_ant_state.web_waiting_combo = false;
    }
}

void ant_ctrl_parse_udp_ant_resp_msg(uint8_t *msg, int32_t msg_len) {
    // web请求拼接时，缓存原始83H/87H帧
    if(g_ant_state.web_waiting_combo && msg_len > 4 && msg[0] == FRAME_HEAD_END)
    {
        if(msg[3] == ANT_ALL_CHECK_RESP && msg_len <= MAX_UDP_MESSAGE_SIZE)
        {
            memcpy(g_ant_state.web_all_check_buf, msg, msg_len);
            g_ant_state.web_all_check_len = msg_len;
            ant_ctrl_try_send_combo_to_web();
        }
        else if(msg[3] == ANT_FACTORY_INFO_RESP && msg_len <= MAX_UDP_MESSAGE_SIZE)
        {
            memcpy(g_ant_state.web_factory_buf, msg, msg_len);
            g_ant_state.web_factory_len = msg_len;
            ant_ctrl_try_send_combo_to_web();
        }
    }

    uint8_t final_recv_msg[MAX_UDP_MESSAGE_SIZE];
    memset(final_recv_msg, 0, sizeof(final_recv_msg));
    int32_t final_recv_len = 0;
    int32_t keyword_offset = 0;
    uint32_t ant_crc_word = 0;

    int32_t value = 0;
    char str[256] = {0};
    
    ant_ctrl_escape_bytes_to_recv_msg(msg, final_recv_msg, &final_recv_len, msg_len);
    
    fmsh_print("final_recv_len = %d\r\n", final_recv_len);
    
    ant_ctrl_check_ant_crc_word(final_recv_msg, final_recv_len, &ant_crc_word);
    if((ant_crc_word & 0xff) != final_recv_msg[final_recv_len-2]) {
        fmsh_print("check ant crc word error!\r\n");
        return;
    }

    ant_ctrl_print_msg(final_recv_msg, final_recv_len);
    fmsh_print("final_recv_msg0 = 0x%X,final_recv_msg3 = 0x%X\r\n", 
           final_recv_msg[0], final_recv_msg[3]);
    
    if(final_recv_msg[0] == FRAME_HEAD_END && final_recv_msg[3] == ANT_PARA_SET_RESP) //81H
    {
        map_uint8_uint8_clear(&g_ant_state.para_resp_map);
        uint8_t last_kw = 0, last_res = 0;
        while(keyword_offset + RESP_KEYWORD_OFFSET < final_recv_len - 2)
        {
            last_kw = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET];
            last_res = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
            map_uint8_uint8_insert(&g_ant_state.para_resp_map, last_kw, last_res);
            if(last_kw == ANT_WRITE_SWITCH_STAR) //   切星回复
            {
                ant_udp_rcv_msg_t star_switch_result;
                memset(&star_switch_result, 0x0, sizeof(ant_udp_rcv_msg_t));
                star_switch_result.msg_id = (uint32_t)ANT_SWITCH_SAT_RESP;
                star_switch_result.result = (uint32_t)last_res;
                handle_ant_udp_rcv_msg(&star_switch_result);
            }
            keyword_offset += 2;
        }

        if(web_addr_valid)
        {
            ant_ctrl_send_para_set_resp_to_web(last_kw, last_res);
        }
    }
    else if(final_recv_msg[0] == FRAME_HEAD_END && final_recv_msg[3] == ANT_ALL_CHECK_RESP)  //83H
    {
        g_ant_state.all_check_recv_flag = true;
        map_uint8_int32_clear(&g_ant_state.all_check_resp_map);
        map_uint8_string_clear(&g_ant_state.all_check_factory_info_map);
        
        while(keyword_offset + RESP_KEYWORD_OFFSET < final_recv_len - 2) {
            value = 0;
            memset(str, 0, sizeof(str));
            
            switch(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET]) {
                case ANT_READ_KA_SEND_FREQ:
                case ANT_READ_SEND_BWP_CENTER_FREQ:
                case ANT_READ_KA_RCV_FREQ:
                case ANT_READ_RCV_BWP_CENTER_FREQ:
                case ANT_READ_KA_RX_LOCAL_OSCILLATOR_FREQ:
                case ANT_READ_KA_TX_LOCAL_OSCILLATOR_FREQ:
                case ANT_READ_C_AMP_FAULT_STATE:
                case ANT_READ_C_RX_LOCAL_OSCILLATOR_FREQ:
                case ANT_READ_C_TX_LOCAL_OSCILLATOR_FREQ:
                case ANT_READ_BEACON_RCV_SIGNAL_STRENGTH:
                case ANT_READ_BEACON_FREQ:
                case ANT_READ_BEACON_SNR:
                {
                    for(uint8_t j = 0; j < final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1]; j++) {
                        value |= (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1 + j + 1] << 
                                8 * (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1] - j - 1));
                    }
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map,
                                         final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    fmsh_print("keyword = 0x%X,value=%d\r\n", final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);

                    uint8_t key = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET];
                    if (key == ANT_READ_KA_SEND_FREQ)
                    {
                        g_ant_info.send_freq = (uint32_t)value;
                    }
                    else if (key == ANT_READ_KA_RCV_FREQ)
                    {
                        g_ant_info.recv_freq = (uint32_t)value;
                    }

                    keyword_offset += 6;
                    break;
                }
                case ANT_READ_AZIMUTH_PITCHANGLE:
                {
                    int16_t azimuth_raw = ((int16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2] << 8)) |
                                    (int16_t)final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 3];
                    int16_t pitch_angle_raw = ((int16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 4] << 8)) |
                                        (int16_t)final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 5];

                    g_ant_info.azimuth = azimuth_raw / 100.0;
                    g_ant_info.pitch_angle = pitch_angle_raw / 100.0;

                    // 使用map操作函数替代C++ map（保持原始值）
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, ANT_AZIMUTH, azimuth_raw);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, ANT_PITCH_ANGLE, pitch_angle_raw);

                    fmsh_print("keyword = 0x%X, azimuth=%.2f, pitch_angle=%.2f\r\n",
                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], g_ant_info.azimuth, g_ant_info.pitch_angle);

                    keyword_offset += 6;
                    break;
                }

                case ANT_READ_KA_BUC_ATTENUATION: //02
                case ANT_READ_KA_BDC_ATTENUATION:
                case ANT_READ_PHASE_ARRAY_STATE:
                case ANT_READ_PARABOLIC_STATE:
                case ANT_READ_SATELLITE_ID:
                case ANT_READ_PARABOLOID_ACU_STATE:
                case ANT_READ_PARABOLOID_RF_STATE:
                case ANT_READ_PARABOLOID_BEACON_STATE:
                case ANT_READ_C_BUC_ATTENUATION:
                case ANT_READ_C_BDC_ATTENUATION:
                {
                    value = 0; // 确保value初始化为0
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];

                    for(uint8_t j = 0; j < data_length; j++)
                    {
                        value |= (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2 + j] <<
                                (8 * (data_length - j - 1)));
                    }

                    // 使用map操作函数替代C++ map
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map,
                                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);

                    if (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET] == ANT_READ_SATELLITE_ID)
                    {
                        g_ant_info.satellite_id = (uint16_t)value;
                    }

                    fmsh_print("keyword = 0x%X, value=%d\r\n",
                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);

                    keyword_offset += 4; // 固定偏移4字节
                    break;
                }
                case ANT_READ_TEMP:
                case ANT_READ_KA_AMP_SEND_POWER:
                case ANT_READ_KA_AMP_TEMP:
                case ANT_READ_C_AMP_SEND_FREQ:
                case ANT_READ_C_AMP_TEMP:
                {
                    value = 0; // 确保value初始化为0
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
                    
                    for(uint8_t j = 0; j < data_length; j++)
                    {
                        int16_t temp_value = (int16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2 + j] << 
                                                (8 * (data_length - j - 1)));
                        value |= temp_value;
                    }
                    
                    // 使用map操作函数替代C++ map
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map,
                                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    
                    fmsh_print("keyword = 0x%X, value=%d\r\n", 
                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    
                    keyword_offset += 4;
                    break;
                }

                case ANT_READ_KA_SWITCH:
                case ANT_READ_HIGHRAIL_RCV_POLARIZATION:
                case ANT_READ_TYPE:
                case ANT_READ_SEND_POLARIZATION:
                case ANT_READ_RCV_POLARIZATION:
                case ANT_READ_HIGHRAIL_POLARIZATION:
                case ANT_READ_SEND_CHANNEL_GAIN:
                case ANT_READ_BUC_ATTENUATION_RANGE:
                case ANT_READ_RECV_CHANNEL_GAIN:
                case ANT_READ_BDC_ATTENUATION_RANGE:
                case ANT_READ_C_SWITCH:
                case ANT_READ_C_AMP_SWITCH:
                case ANT_READ_LOCK_STATE:
                case ANT_READ_MASTER_OR_SLAVE:
                case ANT_READ_RUNNING_STATE:
                {
                    value = 0; // 确保value初始化为0
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
                    
                    for(uint8_t j = 0; j < data_length; j++)
                    {
                        value |= (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2 + j] << 
                                (8 * (data_length - j - 1)));
                    }
                    
                    if (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET] == ANT_READ_RUNNING_STATE)
                    {
                        g_ant_info.ant_status = (uint8_t)value;
                    }
                    // 使用map操作函数替代C++ map
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map,
                                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    
                    fmsh_print("keyword = 0x%X, value=%d\r\n", 
                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    
                    keyword_offset += 3;
                    break;
                }

                case ANT_READ_ABILITY:
                {
                    uint8_t faxiang_eirp = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2];
                    uint8_t min_working_elevation_eirp = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 3];
                    int16_t faxiang_gt = ((int16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 4] << 8)) | (int16_t)final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 5];
                    int16_t min_working_elevation_gt = ((int16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 6] << 8)) | (int16_t)final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 7];
                    uint8_t ue_min_working_elevation = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 8];
                    ant_ctrl_set_ue_min_working_elevation(ue_min_working_elevation);
                    // 使用map操作函数替代C++ map
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, FAXIANG_EIRP, faxiang_eirp);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, MIN_ELEVATION_EIRP, min_working_elevation_eirp);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, FAXAING_GT, faxiang_gt);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, MIN_ELEVATION_GT, min_working_elevation_gt);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, UE_MIN_ELEVATION, ue_min_working_elevation);
                    
                    // 调用函数写入文件
                    ant_ctrl_write_ant_ability_to_file();
                    
                    fmsh_print("keyword = 0x%X, faxiang_eirp=%d, min_working_elevation_eirp=%d, faxiang_gt=%d, min_working_elevation_gt=%d, ue_min_working_elevation=%d\r\n",
                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], 
                        faxiang_eirp, min_working_elevation_eirp, faxiang_gt, 
                        min_working_elevation_gt, ue_min_working_elevation);
                    
                    keyword_offset += 9;
                    break;
                }

                case ANT_READ_C_AMP_VERSION:
                case ANT_READ_ACU_DEVICE_NAME:
                case ANT_READ_ACU_MANUFACTURER:
                case ANT_READ_ACU_SERIAL_NUMBER:
                case ANT_READ_ACU_MANU_DATE:
                case ANT_READ_ACU_SOFT_VERSION:
                case ANT_READ_RF_DEVICE_NAME:
                case ANT_READ_RF_MANUFACTURER:
                case ANT_READ_RF_SERIAL_NUMBER:
                case ANT_READ_RF_MANU_DATE:
                case ANT_READ_RF_SOFT_VERSION:
                case ANT_READ_BEACON_DEVICE_NAME:
                case ANT_READ_BEACON_MANUFACTURER:
                case ANT_READ_BEACON_SERIAL_NUMBER:
                case ANT_READ_BEACON_MANU_DATE:
                case ANT_READ_BEACON_SOFT_VERSION:
                {
                    char str[256] = {0}; // 使用固定大小的字符数组替代std::string
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
                    int32_t str_index = 0;
                    
                    // 复制字符串数据
                    for (int32_t i = keyword_offset + RESP_KEYWORD_OFFSET + 2; 
                        i < keyword_offset + RESP_KEYWORD_OFFSET + 2 + data_length && str_index < sizeof(str) - 1; 
                        i++)
                    {
                        str[str_index++] = (char)final_recv_msg[i];
                    }
                    str[str_index] = '\0'; // 确保字符串以null结尾
                    
                    fmsh_print("keyword = 0x%X, str=%s\r\n", 
                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], str);
                    
                    // 使用map操作函数替代C++ map
                    map_uint8_string_insert(&g_ant_state.all_check_factory_info_map, final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], str); 
                    keyword_offset += data_length + 2;
                    break;
                }

                case ANT_READ_WORKING_STATE:
                {
                    uint8_t ant_working_mode = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2];
                    uint16_t ant_azimuth = ((uint16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 3] << 8)) | 
                                        (uint16_t)final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 4];
                    uint16_t ant_pitch_angle = ((uint16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 5] << 8)) | 
                                            (uint16_t)final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 6];
                    uint16_t ant_polarization_angle = ((uint16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 7] << 8)) | 
                                                (uint16_t)final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 8];
                    
                    fmsh_print("keyword = 0x%X, ant_working_mode=%d, ant_azimuth=%d, ant_pitch_angle=%d, ant_polarization_angle=%d\r\n",
                        final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], 
                        ant_working_mode, ant_azimuth, ant_pitch_angle, ant_polarization_angle);
                    
                    // 使用map操作函数替代C++ map
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, ANT_WORKING_MODE, ant_working_mode);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, ANT_WORKING_AZIMUTH, ant_azimuth);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, ANT_WORKING_PITCH_ANGLE, ant_pitch_angle);
                    map_uint8_int32_insert(&g_ant_state.all_check_resp_map, ANT_WORKING_POLARIZATION_ANGLE, ant_polarization_angle);
                    
                    keyword_offset += 9;
                    break;
                }
                default:
                {
                    fmsh_print("error keyword=0x%X\r\n", final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET]);
                    keyword_offset += final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1] + 2;
                    break;
                }
            }
        }
    }
    else if (final_recv_msg[0] == FRAME_HEAD_END && final_recv_msg[3] == ANT_FACTORY_INFO_RESP)  //87H
    {
        g_ant_state.manu_recv_flag = true;
        g_ant_state.ant_type = 0;
        map_uint8_string_clear(&g_ant_state.factory_info_map);

        while (keyword_offset + RESP_KEYWORD_OFFSET < final_recv_len - 2) {
            value = 0;
            memset(str, 0, sizeof(str));

            switch (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET]) {
                case ANT_MANUFACTURER_CODE: {
                    uint16_t code_value = ((uint16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2]) << 8) | 
                                         (uint16_t)(final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 3]);
                    fmsh_print("keyword = 0x%02X, code_value = %d\r\n", 
                           final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], code_value);
                    keyword_offset += 4;
                    break;
                }
                case ANT_READ_TYPE: {
                    g_ant_state.ant_type = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2];
                    fmsh_print("keyword = 0x%02X, value = %d\r\n", 
                           final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], g_ant_state.ant_type);
                    keyword_offset += 3;
                    break;
                }
                case ANT_DEVICE_NAME:
                case ANT_DEVICE_MANUFACTURER:
                case ANT_DEVICE_SERIAL_NUMBER:
                case ANT_DEVICE_FACTORY_DATE:
                case ANT_SOFTWARE_VERSION: {
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
                    for (int32_t i = keyword_offset + RESP_KEYWORD_OFFSET + 2; 
                         i < keyword_offset + RESP_KEYWORD_OFFSET + 2 + data_length; i++) {
                        // string_push_back(&str, (char)final_recv_msg[i]);
                    }
                    fmsh_print("keyword = 0x%02X, len= %d str = %s\r\n", final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], sizeof(str), str);
                    fmsh_print("keyword_offset = %d\r\n", keyword_offset);
                    map_uint8_string_insert(&g_ant_state.factory_info_map, final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], str);
       
                    keyword_offset += data_length + 2;
                    break;
                }
                default: {
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
                    keyword_offset += data_length + 2;
                    break;
                }
            }
        }
    } 
    else if (final_recv_msg[0] == FRAME_HEAD_END && final_recv_msg[3] == ANT_EPHE_SET_RESP)  //8BH
    {
        g_ant_state.ephe_flag = true;
        uint8_t result = final_recv_msg[4];
        ant_udp_rcv_msg_t ephe_result;
        memset(&ephe_result, 0x0, sizeof(ant_udp_rcv_msg_t));
        ephe_result.msg_id = (uint32_t)ANT_EPHEM_SET_RESP;
        ephe_result.result = (uint32_t)result;
        if (result == RESP_SUCCESS) {
            g_ant_state.eph_value = RESP_SUCCESS;
            fmsh_print("recv success\r\n");
        } 
        else if (result == RESP_FAIL) {
            g_ant_state.eph_value = RESP_FAIL;
            fmsh_print("recv fail\r\n");
        } 
        else if (result == RESP_TIMEOUT) {
            g_ant_state.eph_value = RESP_TIMEOUT;
            fmsh_print("recv timeout\r\n");
        } 
        else if (result == RESP_UNKNOWN) {
            g_ant_state.eph_value = RESP_UNKNOWN;
            fmsh_print("recv unknown\r\n");
        }
        handle_ant_udp_rcv_msg(&ephe_result);
    }
    else if (final_recv_msg[0] == FRAME_HEAD_END && final_recv_msg[3] == ANT_REPORT)  //84H
    {
        g_ant_state.self_check_state = 0;
        g_ant_state.ant_master_or_slave = 0;
        map_uint8_uint8_clear(&g_ant_state.ant_report_map);

        while (keyword_offset + RESP_KEYWORD_OFFSET < final_recv_len - 2)
        {
            value = 0;

            switch (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET]) {
                case SELF_TEST_STATE: {
                    value = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2];
                    g_ant_state.self_check_state = value;
                    map_uint8_uint8_insert(&g_ant_state.ant_report_map, final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], 0x20);
                    fmsh_print("keyword = 0x%02X, value = %u\r\n", final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    keyword_offset += 3;
                    break;
                }
                case PARABOLOID_LOCATION: {
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
                    
                    // 解析多字节数据（大端序）
                    for (uint8_t j = 0; j < data_length; j++) {
                        value |= (final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2 + j] << 
                                 (8 * (data_length - j - 1)));
                    }
                    
                    map_uint8_uint8_insert(&g_ant_state.ant_report_map, final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], 0x20);
                    fmsh_print("keyword = 0x%02X, value = %u\r\n", final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    keyword_offset += 2 + data_length;  // keyword + length + data
                    break;
                }
                case ANT_MASTER_OR_SLAVE: {
                    value = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 2];
                    g_ant_state.ant_master_or_slave = value;
                    map_uint8_uint8_insert(&g_ant_state.ant_report_map, final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], 0x20);
                    fmsh_print("keyword = 0x%02X, value = %u\r\n",  final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET], value);
                    keyword_offset += 3;
                    break;
                }
                default: {
                    uint8_t data_length = final_recv_msg[keyword_offset + RESP_KEYWORD_OFFSET + 1];
                    keyword_offset += data_length + 2;
                    fmsh_print("keyword_offset = %d\r\n", keyword_offset);
                    break;
                }
            }
        }
    ant_ctrl_ant_report_resp_msg();
    }
}

// 构建并发送天线参数设置帧(80H)
static int32_t ant_ctrl_send_para_set(uint8_t keyword, uint8_t *param, uint8_t param_len)
{
    uint8_t raw[MAX_UDP_MESSAGE_SIZE];
    uint8_t final[MAX_UDP_MESSAGE_SIZE];
    int32_t idx = 0;
    uint8_t checksum = 0;

    memset(raw, 0, sizeof(raw));

    raw[idx++] = FRAME_HEAD_END;
    raw[idx++] = 0x00;
    raw[idx++] = 1 + 2 + param_len;
    raw[idx++] = ANT_PARA_SET;

    raw[idx++] = keyword;
    raw[idx++] = param_len;
    memcpy(&raw[idx], param, param_len);
    idx += param_len;

    checksum = ANT_PARA_SET;
    for(int32_t i = 4; i < idx; i++)
    {
        checksum += raw[i];
    }
    raw[idx++] = checksum;

    raw[idx++] = FRAME_HEAD_END;

    int32_t final_len = idx;
    ant_ctrl_escape_bytes_to_send_msg(raw, final, &final_len);

    fmsh_print("send 80H para: kw=0x%02X, len=%d\r\n", keyword, param_len);
    return ant_link_mgr_send_udp_msg_to_ant((char *)final, final_len);
}

void ant_ctrl_parse_udp_web_resp_msg(uint8_t *msg, int32_t msg_len)
{
    if(msg_len < 2)
    {
        fmsh_print("web msg too short, len=%d\r\n", msg_len);
        return;
    }

    if(msg[0] != 0x09)
    {
        fmsh_print("web msg head=0x%02X, dropped\r\n", msg[0]);
        return;
    }

    uint8_t cmd = msg[1];

    // 09 00 00: 触发82H全检+86H工厂信息，等待83H和87H拼接回复
    if(cmd == 0x00 && msg_len >= 3 && msg[2] == 0x00)
    {
        fmsh_print("web cmd=0x00, trigger all check + factory info\r\n");
        g_ant_state.web_waiting_combo = true;
        g_ant_state.web_all_check_len = 0;
        g_ant_state.web_factory_len = 0;
        ant_ctrl_send_all_check_msg();
        ant_ctrl_send_factory_info_msg();
        return;
    }

    char body[128] = {0};
    int32_t body_len = msg_len - 2;
    if(msg[msg_len - 1] == '\r\n')
    {
        body_len--;
    }
    if(body_len >= sizeof(body))
    {
        body_len = sizeof(body) - 1;
    }
    if(body_len > 0)
    {
        memcpy(body, msg + 2, body_len);
        body[body_len] = '\0';
    }

    fmsh_print("web cmd=0x%02X, body=[%s]\r\n", cmd, body);

    int antenna_num = 0;
    int val_int = 0;
    double val_double = 0;
    double lon = 0, lat = 0, height = 0;

    switch(cmd)
    {
        case WEB_CMD_ANTENNA_ON:
        {
            uint8_t para = 0x3f;
            ant_ctrl_send_para_set(ANT_WRITE_KA_SWITCH, &para, 1);
            break;
        }

        case WEB_CMD_ANTENNA_OFF:
        {
            uint8_t para = 0x2a;
            ant_ctrl_send_para_set(ANT_WRITE_KA_SWITCH, &para, 1);
            break;
        }

        case WEB_CMD_SEND_FREQ:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, send_freq=%d MHz\r\n", antenna_num, val_int);
                uint8_t para[4];
                ant_ctrl_byte4_data_format_conversion(para, (uint32_t)val_int, 0);
                ant_ctrl_send_para_set(ANT_WRITE_KA_SEND_FREQ, para, 4);
            }
            break;

        case WEB_CMD_RECV_FREQ:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, recv_freq=%d MHz\r\n", antenna_num, val_int);
                uint8_t para[4];
                ant_ctrl_byte4_data_format_conversion(para, (uint32_t)val_int, 0);
                ant_ctrl_send_para_set(ANT_WRITE_KA_RCV_FREQ, para, 4);
            }
            break;

        case WEB_CMD_HIGH_ORBIT_POLAR:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, high_orbit_polar=%d\r\n", antenna_num, val_int);
                uint8_t para = (val_int == 0) ? 0x00 : 0x01;
                ant_ctrl_send_para_set(ANT_WRITE_HIGHRAIL_RCV_POLARIZATION, &para, 1);
            }
            break;

        case WEB_CMD_SEND_POLARIZATION:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, send_polar=%d\r\n", antenna_num, val_int);
                uint8_t para = (val_int == 0) ? 0x00 : 0x01;
                ant_ctrl_send_para_set(ANT_WRITE_SEND_POLARIZATION, &para, 1);
            }
            break;

        case WEB_CMD_RECV_POLARIZATION:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, recv_polar=%d\r\n", antenna_num, val_int);
                uint8_t para = (val_int == 0) ? 0x00 : 0x01;
                ant_ctrl_send_para_set(ANT_WRITE_RCV_POLARIZATION, &para, 1);
            }
            break;

        case WEB_CMD_BUC_SET:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, buc=%d (0.01dB)\r\n", antenna_num, val_int);
                uint8_t para[2];
                ant_ctrl_byte2_data_format_conversion(para, (uint16_t)val_int, 0);
                ant_ctrl_send_para_set(ANT_WRITE_KA_BUC_ATTENUATION, para, 2);
            }
            break;

        case WEB_CMD_BDC_SET:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, bdc=%d (0.01dB)\r\n", antenna_num, val_int);
                uint8_t para[2];
                ant_ctrl_byte2_data_format_conversion(para, (uint16_t)val_int, 0);
                ant_ctrl_send_para_set(ANT_WRITE_KA_BDC_ATTENUATION, para, 2);
            }
            break;

        case WEB_CMD_BEACON_FREQ:
            if(sscanf(body, "%d,%d", &antenna_num, &val_int) == 2)
            {
                fmsh_print("  antenna=%d, beacon=%d (10kHz)\r\n", antenna_num, val_int);
                uint8_t para[8] = {0};
                ant_ctrl_byte4_data_format_conversion(&para[4], (uint32_t)val_int, 0);
                ant_ctrl_send_para_set(ANT_WRITE_BEACON_FREQ, para, 8);
            }
            break;

        case WEB_CMD_VECTOR_POSITION:
            if(sscanf(body, "%d,%lf,%lf,%lf", &antenna_num, &lon, &lat, &height) == 4)
            {
                fmsh_print("  antenna=%d, lon=%.6f, lat=%.6f, height=%.1f\r\n",
                           antenna_num, lon, lat, height);
                uint8_t para[6];
                ant_ctrl_byte2_data_format_conversion(para, (uint16_t)(int16_t)(lon * 10), 0);
                ant_ctrl_byte2_data_format_conversion(para, (uint16_t)(int16_t)(lat * 10), 2);
                ant_ctrl_byte2_data_format_conversion(para, (uint16_t)(int16_t)height, 4);
                ant_ctrl_send_para_set(ANT_WRITE_VECTOR_POSITION, para, 6);
            }
            break;

        case WEB_CMD_SATELLITE_LON:
            if(sscanf(body, "%d,%lf", &antenna_num, &val_double) == 2)
            {
                fmsh_print("antenna=%d, sat_lon=%.6f\r\n", antenna_num, val_double);
                uint8_t para[3];
                int16_t lon_val = (int16_t)(val_double * 10);
                if(lon_val < 0)
                {
                    para[0] = 0x01;
                    lon_val = -lon_val;
                }
                else
                {
                    para[0] = 0x00;
                }
                ant_ctrl_byte2_data_format_conversion(&para[1], (uint16_t)lon_val, 0);
                ant_ctrl_send_para_set(ANT_WRITE_SATELLITE_LON_AND_LAT, para, 3);
            }
            break;

        default:
            fmsh_print("unknown web cmd 0x%02X\r\n", cmd);
            break;
    }
}

int32_t ant_ctrl_ant_report_resp_msg(void)
{
    uint8_t send_buf[MAX_UDP_MESSAGE_SIZE];
    uint8_t final_send_buf[MAX_UDP_MESSAGE_SIZE];
    int32_t keyword_offset = 0;
    uint8_t checksum = 0;

    // 初始化缓冲区
    memset(send_buf, 0x0, sizeof(send_buf));
    memset(final_send_buf, 0x0, sizeof(final_send_buf));

    // 设置帧头和消息类型
    send_buf[0] = FRAME_HEAD_END;
    send_buf[3] = ANT_REPORT_RESP;

    // 遍历ant_report_map（替代C++的auto迭代器）
    for (int i = 0; i < g_ant_state.ant_report_map.size; i++)
    {
        uint8_t key = g_ant_state.ant_report_map.keys[i];
        uint8_t value = g_ant_state.ant_report_map.values[i];

        switch (key)
        {
            case SELF_TEST_STATE:
            case PARABOLOID_LOCATION:
            {
                send_buf[keyword_offset + RESP_KEYWORD_OFFSET] = key;
                send_buf[keyword_offset + RESP_KEYWORD_OFFSET + 1] = value;
                keyword_offset += 2;
                break;
            }
            default:
            {
                fmsh_print("error keyword=0x%02X\r\n", key);
                break;
            }
        }
    }

    fmsh_print("keyword_offset=%d\r\n", keyword_offset);

    // 设置长度字段
    send_buf[1] = 0x00;  // 假设的高字节
    send_buf[2] = sizeof(uint8_t) + keyword_offset;  // 长度字段

    // 计算校验和（从第3字节开始到数据结束）
    for (int i = 3; i < keyword_offset + RESP_KEYWORD_OFFSET; i++)
    {
        checksum += send_buf[i];
    }
    send_buf[keyword_offset + RESP_KEYWORD_OFFSET] = checksum;

    // 设置帧尾
    send_buf[keyword_offset + RESP_KEYWORD_OFFSET + 1] = FRAME_HEAD_END;

    // 计算总长度
    int32_t buf_len = keyword_offset + RESP_KEYWORD_OFFSET + 2;

    // 转义处理
    ant_ctrl_escape_bytes_to_send_msg(send_buf, final_send_buf, &buf_len);

    // 打印消息
    ant_ctrl_print_msg(final_send_buf, buf_len);

    // 发送UDP消息
    return ant_link_mgr_send_udp_msg_to_ant((char *)final_send_buf, buf_len);
}

void ant_ctrl_write_ant_ability_to_file(void)
{
}

int32_t ant_ctrl_parabolic_ant_switch(int32_t ant_id)
{
    char send_buf[MAX_UDP_MESSAGE_SIZE];
    memset(send_buf, 0x0, sizeof(send_buf));

    // 格式化命令字符串
    sprintf(send_buf, "$SEL ANT,%d*FF\r\n", ant_id);
    int32_t buf_len = strlen(send_buf);
    
    // 发送UDP消息
    return ant_link_mgr_send_udp_msg_to_ant(send_buf, buf_len);
}

// 其他函数实现...
int32_t ant_ctrl_send_all_check_msg(void)
{
    uint8_t send_buf[MAX_UDP_MESSAGE_SIZE];
    memset(send_buf, 0x0, sizeof(send_buf));

    // 构建消息帧
    send_buf[0] = FRAME_HEAD_END;  // 帧头
    send_buf[1] = 0x00;            // 长度高字节
    send_buf[2] = sizeof(uint8_t); // 长度低字节（数据部分长度）
    send_buf[3] = ANT_ALL_CHECK;   // 命令类型
    send_buf[4] = send_buf[3];     // 校验和（简单重复命令字节）
    send_buf[5] = FRAME_HEAD_END;  // 帧尾

    int32_t buf_len = 6;
    
    // 打印消息（调试用）
    ant_ctrl_print_msg(send_buf, buf_len);
    
    // 发送UDP消息
    return ant_link_mgr_send_udp_msg_to_ant((char *)send_buf, buf_len);
}

int32_t ant_ctrl_send_factory_info_msg(void)
{
    uint8_t send_buf[MAX_UDP_MESSAGE_SIZE];
    memset(send_buf, 0x0, sizeof(send_buf));

    send_buf[0] = FRAME_HEAD_END;
    send_buf[1] = 0x00;
    send_buf[2] = sizeof(uint8_t);
    send_buf[3] = ANT_FACTORY_INFO;
    send_buf[4] = send_buf[3];
    send_buf[5] = FRAME_HEAD_END;

    int32_t buf_len = 6;
    ant_ctrl_print_msg(send_buf, buf_len);
    return ant_link_mgr_send_udp_msg_to_ant((char *)send_buf, buf_len);
}

void ant_ctrl_set_ue_min_working_elevation(uint8_t angle)
{
    g_ant_state.ue_min_working_elevation = angle;
}

uint8_t ant_ctrl_get_ue_min_working_elevation()
{
    return g_ant_state.ue_min_working_elevation;
}

void ant_ctrl_get_ant_info(ANT_INFO* info)
{
    if (info != NULL)
    {
        *info = g_ant_info;
    }
}

void ant_ctrl_init_func(void)
{
    fmsh_print("ant_ctrl start\r\n");
    ant_ctrl_init();
    ant_link_init();
    ant_link_startup_phase();//建udp，起任务回调收
    // ant_ctrl_send_ant_msg_for_test();
}

int32_t ant_ctrl_send_ant_msg_for_test(void)
{
    //切星
    // 卫星编号   1
    // 有效指示   1（无意义）
    // GPS周     2635
    // GPS周内秒 45678
    // GPS毫秒   0
    fmsh_print("send ant msg 1: switch star\r\n");
    uint8_t send_buf1[20] = {0x7e, 0x00, 0x0f, 0x80, 0x01, 0x0c, 0x00, 0x01, 0x01, 0x00, 0x0a, 0x4b, 0x00, 0x00, 0xb2, 0x6e, 0x00, 0x00, 0x04, 0x7e};
    int32_t buf_len1 = 20;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf1, buf_len1);
    delay_ms(2000);

    //发射频率 28000MHz
    fmsh_print("send ant msg 2: send freq 28000MHz\r\n");
    uint8_t send_buf2[12] = {0x7e, 0x00, 0x07, 0x80, 0x02, 0x04, 0x00, 0x00, 0x6d, 0x60, 0x53, 0x7e};
    int32_t buf_len2 = 12;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf2, buf_len2);
    delay_ms(2000);

    //发射BWP中心频率 27500MHz
    fmsh_print("send ant msg 3: send BWP center freq 27500MHz\r\n");
    uint8_t send_buf3[12] = {0x7e, 0x00, 0x07, 0x80, 0x03, 0x04, 0x00, 0x00, 0x6b, 0x6c, 0x5e, 0x7e};
    int32_t buf_len3 = 12;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf3, buf_len3);
    delay_ms(2000);

    //接收频率 19000MHz
    fmsh_print("send ant msg 4: receive freq 19000MHz\r\n");
    uint8_t send_buf4[12] = {0x7e, 0x00, 0x07, 0x80, 0x04, 0x04, 0x00, 0x00, 0x4a, 0x38, 0x0a, 0x7e};
    int32_t buf_len4 = 12;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf4, buf_len4);
    delay_ms(2000);

    //接收BWP中心频率 18000MHz
    fmsh_print("send ant msg 5: receive BWP center freq 18000MHz\r\n");
    uint8_t send_buf5[12] = {0x7e, 0x00, 0x07, 0x80, 0x05, 0x04, 0x00, 0x00, 0x46, 0x50, 0x1f, 0x7e};
    int32_t buf_len5 = 12;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf5, buf_len5);
    delay_ms(2000);

    //BUC衰减 20*100
    fmsh_print("send ant msg 6: BUC atten 20*100\r\n");
    uint8_t send_buf6[10] = {0x7e, 0x00, 0x05, 0x80, 0x06, 0x02, 0x07, 0xd0, 0x5f, 0x7e};
    int32_t buf_len6 = 10;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf6, buf_len6);
    delay_ms(2000);

    //BDC衰减 19.5*100
    fmsh_print("send ant msg 7: BDC atten 19.5*100\r\n");
    uint8_t send_buf7[10] = {0x7e, 0x00, 0x05, 0x80, 0x07, 0x02, 0x07, 0x9e, 0x2e, 0x7e};
    int32_t buf_len7 = 10;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf7, buf_len7);
    delay_ms(2000);

    //天线开关 全开
    fmsh_print("send ant msg 8: ant switch all open\r\n");
    uint8_t send_buf8[9] = {0x7e, 0x00, 0x04, 0x80, 0x08, 0x01, 0x3f, 0xc8, 0x7e};
    int32_t buf_len8 = 9;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf8, buf_len8);
    delay_ms(2000);

    // //天线开关 全关
    fmsh_print("send ant msg 9: ant switch all close\r\n");
    uint8_t send_buf9[9] = {0x7e, 0x00, 0x04, 0x80, 0x08, 0x01, 0x2a, 0xb3, 0x7e};
    int32_t buf_len9 = 9;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf9, buf_len9);
    delay_ms(2000);

    //参考信号功率 -20*10
    fmsh_print("send ant msg 10: reference signal power -20*10\r\n");
    uint8_t send_buf10[10] = {0x7e, 0x00, 0x05, 0x80, 0x0c, 0x02, 0xff, 0x38, 0xc5, 0x7e};
    int32_t buf_len10 = 10;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf10, buf_len10);
    delay_ms(2000);

    //天线波束指向角 方位角：87.6*100     俯仰角：45.7*100
    fmsh_print("send ant msg 11: pitch:45.7*100 azimuth:87.6*100\r\n");
    uint8_t send_buf11[12] = {0x7e, 0x00, 0x07, 0x80, 0x0d, 0x04, 0x22, 0x38, 0x11, 0xda, 0xd6, 0x7e};
    int32_t buf_len11 = 12;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf11, buf_len11);
    delay_ms(2000);

    // //发射极化 左旋圆极化
    fmsh_print("send ant msg 12: send polarization: left\r\n");
    uint8_t send_buf12[9] = {0x7e, 0x00, 0x04, 0x80, 0x11, 0x01, 0x00, 0x92, 0x7e};
    int32_t buf_len12 = 9;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf12, buf_len12);
    delay_ms(2000);

    // //接收极化 右旋圆极化
    fmsh_print("send ant msg 13: receive polarization: right\r\n");
    uint8_t send_buf13[9] = {0x7e, 0x00, 0x04, 0x80, 0x12, 0x01, 0x01, 0x94, 0x7e};
    int32_t buf_len13 = 9;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf13, buf_len13);
    delay_ms(2000);

    //信标频率 3500000（10kHz）
    fmsh_print("send ant msg 14: : xinbiao freq 3500000（10kHz）\r\n");
    uint8_t send_buf14[16] = {0x7e, 0x00, 0x0b, 0x80, 0x1c, 0x08, 0x00, 0x35, 0x67, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x20, 0x7e};
    int32_t buf_len14 = 16;
    ant_link_mgr_send_udp_msg_to_ant((char *)send_buf14, buf_len14);
    delay_ms(2000);
}