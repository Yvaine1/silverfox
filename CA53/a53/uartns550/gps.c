/** 
 * @file   gps.c
 * @note   Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.
 * @brief   
 *
 * @author guodecai	
 * @date   2026/05/09
 *
 * @version
 *  date        |version |author              |message
 *  :----       |:----   |:----               |:------
 *  2026/05/09  |V1.0    |guodecai            |create base code
 * @warning 
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "uartns550.h"

extern struct GPS_INFO g_gps_info;

/* 历元时间：2006-01-01 00:00:00.000 */
static time_t init_epoch_2006(void)
{
    struct tm epoch_tm = {0};
    epoch_tm.tm_year = 2006 - 1900;
    epoch_tm.tm_mon = 0;
    epoch_tm.tm_mday = 1;
    epoch_tm.tm_hour = 0;
    epoch_tm.tm_min = 0;
    epoch_tm.tm_sec = 0;
    epoch_tm.tm_isdst = -1;
    return mktime(&epoch_tm);
}

/**
 * @brief NMEA经纬度转十进制度
 */
static double nmea2deg(double nmea_val)
{
    uint32_t deg = (uint32_t)(nmea_val / 100.0);
    double min = nmea_val - deg * 100.0;
    return deg + min / 60.0;
}

/**
 * @brief 解析GPS报文
 */
int parse_gps_info(char *raw_data, struct GPS_INFO *gps_info)
{
    time_t EPOCH_2006;
    int field_cnt = 0;
    char *token = NULL; /* 存储 strtok 函数分割报文后得到的单个字段字符串（如 "$GNRMC"、"071006.00"） */
    char tmp_buf[256] = {0};
    char *fields[20] = {NULL}; /* 存储 GPS 报文分割后的所有字段指针，数组下标对应报文的字段序号（如 fields[1] 对应时间字段）*/
    int is_gnrmc = 0;
    int is_gngga = 0;

    if ((NULL == raw_data) || (gps_info == NULL))    
    {
        if ((ERRORS == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("%s-nullptr\r\n", __func__);
        }
         
        return -1;
    }

    if (strstr((const char *)raw_data, "$GNRMC") != NULL)
    {
        if ((DEBUG == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("IS GNRMC INFO\r\n");
        }
        
        is_gnrmc = 1;
    }
    
    if (strstr((const char *)raw_data, "$GNGGA") != NULL)
    {
        if ((DEBUG == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("IS GNGGA INFO\r\n");
        }

        is_gngga = 1;
    }
    
    if ((strstr((const char *)raw_data, "$GNRMC") != NULL) 
            && (strstr((const char *)raw_data, "$GNGGA") != NULL))
    {
        if ((ERRORS == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("%s: not a GNRMC/GNGGA message\r\n", __func__);
        }
        return -1;
    }

    /* 分割GNRMC报文 */
    strncpy(tmp_buf, raw_data, sizeof(tmp_buf) - 1);
    token = strtok(tmp_buf, ",");
    while (token != NULL && field_cnt < 20)
    {
        fields[field_cnt++] = token;
        token = strtok(NULL, ",");
    }

    /* 分割后GNRMC至少11个字段(,,,算作1个)，否则无效 */
    if (is_gnrmc && field_cnt < 11)
    {
        if ((ERRORS == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("%s: GNRMC field count less than 11, cnt=%d\r\n", __func__, field_cnt);
        }
        return -1;
    }
    /* GNGGA至少10个字段 */
    else if (is_gngga && field_cnt < 10)
    {
        if ((ERRORS == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("%s: GNGGA field count less than 10, cnt=%d\r\n", __func__, field_cnt);
        }
        return -1;
    }

    if (is_gnrmc)
    {
        if (strlen(fields[1]) == 0 || strlen(fields[2]) == 0 || strlen(fields[3]) == 0 || strlen(fields[9]) == 0)
        {
            if ((ERRORS == g_print_type) || (ALL == g_print_type))
            {
                fmsh_print("%s: Core GNRMC fields are empty\r\n", __func__);
            }
            
            return -1;
        }

        /* 解析UTC时间（hhmmss.ss）*/
        if (strlen(fields[1]) >= 6)
        {
            /* 提取时、分、秒 */
            gps_info->hour = atoi(fields[1]) / 10000;
            gps_info->min = (atoi(fields[1]) / 100) % 100;
            gps_info->sec = atoi(fields[1]) % 100;
            /* 提取毫秒 */
            char *dot = strchr(fields[1], '.');
            if (dot != NULL)
            {
                char ms_str[4] = "000";
                strncpy(ms_str, dot+1, 3);
                gps_info->ms = atoi(ms_str);
            }
        }

        /* 解析定位状态 */
        gps_info->status = (strcmp(fields[2], "A") == 0) ? VALID : INVALID;

        /* 解析纬度 */
        if (strlen(fields[3]) > 0 && strlen(fields[4]) > 0)
        {
            double lat_nmea = atof(fields[3]);
            gps_info->latitude = nmea2deg(lat_nmea);
            if (strcmp(fields[4], "S") == 0)
            {
                gps_info->latitude = -gps_info->latitude;
            }
        }

        /* 解析经度 */
        if (strlen(fields[5]) > 0 && strlen(fields[6]) > 0)
        {
            double lon_nmea = atof(fields[5]);
            gps_info->longitude = nmea2deg(lon_nmea);
            if (strcmp(fields[6], "W") == 0)
            {
                gps_info->longitude = -gps_info->longitude;
            }
        }

        /* 解析UTC日期（ddmmyy）*/
        if (strlen(fields[9]) >= 6)
        {
            int date = atoi(fields[9]);
            gps_info->day = date / 10000;
            gps_info->mon = (date / 100) % 100;
            gps_info->year = 2000 + (date % 100);

            /* 用struct tm + mktime计算总秒数 */
            struct tm gps_tm = {0};
            gps_tm.tm_year = gps_info->year - 1900;
            gps_tm.tm_mon = gps_info->mon - 1;
            gps_tm.tm_mday = gps_info->day;
            gps_tm.tm_hour = gps_info->hour;
            gps_tm.tm_min = gps_info->min;
            gps_tm.tm_sec = gps_info->sec;
            gps_tm.tm_isdst = -1;

            /* 计算GPS时间的time_t值（从1970-01-01 00:00:00 UTC开始的秒数） */
            time_t gps_time = mktime(&gps_tm);
            /* 计算相对于2006-01-01的总秒数 */
            EPOCH_2006 = init_epoch_2006();
            gps_info->total_sec = (uint64_t)(gps_time - EPOCH_2006);
        }

        if ((gps_info->status != 1) || (gps_info->total_sec == 0))
        {
            return -1;
        }
    }

    if (is_gngga)
    {
        /* 解析高度（GNGGA第9个字段，索引9）*/
        if (strlen(fields[9]) > 0)
        {
            gps_info->altitute = atof(fields[9]);
            //fmsh_print("GNGGA-altitude: %.2f m\r\n", gps_info->altitute);
        }
        else
        {
            if ((ERRORS == g_print_type) || (ALL == g_print_type))
            {
                fmsh_print("%s: GNGGA altitude field is empty\r\n", __func__);
            }

            return -1;
        }
    }

    return 0;
}

static void sec2utc(uint64_t sec, uint16_t *year, uint8_t *mon, uint8_t *day,
                    uint8_t *hour, uint8_t *min, uint8_t *sec_out)
{
    const uint32_t mon_days[2][12] = {
        /* 平年 */
        {31,28,31,30,31,30,31,31,30,31,30,31},
        /* 闰年 */
        {31,29,31,30,31,30,31,31,30,31,30,31}
    };

    uint32_t sec_per_day = 86400;
    uint64_t days = sec / sec_per_day;
    uint32_t rem = sec % sec_per_day;

    /* 解析时分秒 */
    *hour = rem / 3600;
    *min  = (rem % 3600) / 60;
    *sec_out = rem % 60;

    /* 解析年 */
    uint32_t y = 2006;
    while (1) 
    {
        uint32_t ydays = ((y % 4 == 0 && y % 100 != 0)||(y % 400 == 0)) ? 366 : 365;
        if (days < ydays) break;
        days -= ydays;
        y++;
    }

    *year = y;

    /* 解析月日 */
    uint32_t leap = ((y % 4 == 0 && y % 100 != 0)||(y % 400 == 0)) ? 1 : 0;
    uint32_t m = 0;
    while (m < 12) 
    {
        if (days < mon_days[leap][m])
        {
            break;
        }

        days -= mon_days[leap][m];
        m++;
    }

    *mon = m + 1;
    *day = days + 1;
}

/**
 * @brief 计算指定年月日 00:00:00 的UTC绝对秒数（1970起点）
 * @param year 年份（如2026）
 * @param mon  月份（1-12）
 * @param day  日期（1-31）
 * @return UTC绝对秒数
 */
static uint64_t get_day_start_utc_sec(uint16_t year, uint8_t mon, uint8_t day)
{
    struct tm day_tm = {0};
    day_tm.tm_year = year - 1900;
    day_tm.tm_mon = mon - 1;
    day_tm.tm_mday = day;
    day_tm.tm_hour = 0;
    day_tm.tm_min = 0;
    day_tm.tm_sec = 0;
    day_tm.tm_isdst = -1;

    return (uint64_t)mktime(&day_tm);
}

int parse_xl_gps_info(char *raw_data, uint16_t len, struct GPS_INFO *gps_info)
{
    uint16_t i = 0;
    uint32_t tmp = 0;
    uint16_t data_start = 0;
    uint16_t data_len = 0;
    uint8_t out_buf[256] = {0};
    uint16_t out_idx = 0;
    uint32_t day_second = 0;
    uint64_t day_start_utc = 0;
    uint64_t utc_total_sec = 0;
    time_t EPOCH_2006;

    int found = 0;
    for (i = 0; i < len - 1; i++) 
    {
        if (((uint8_t *)raw_data)[i] == 0x82 && ((uint8_t *)raw_data)[i + 1] == 0x20) 
        {
            data_start = i;
            found = 1;
            break;
        }
    }

    if (!found) 
    {
        fmsh_print("no 0x82 0x20 found\r\n");
        return -1;
    }

    /* 计算有效数据长度：从开始位置 到 帧最后一个字节（不包含最后的0x7E）
        因为整帧最后一个字节是 0x7E，所以有效数据是 len - data_start - 1 */
    data_len = len - data_start - 1;

    /* 转义处理 */
    for (i = 0; i < data_len && out_idx < 255; i++) 
    {
        uint8_t byte = ((uint8_t *)raw_data)[data_start + i];

        if (byte == 0x7D) 
        {
            /*  取下一个字节 */
            uint8_t next = ((uint8_t *)raw_data)[data_start + i + 1];

            if (next == 0x5E) 
            {
                out_buf[out_idx++] = 0x7E;
                i++;
            } 
            else if (next == 0x5D) 
            {
                out_buf[out_idx++] = 0x7D;
                i++;
            } 
            else 
            {
                out_buf[out_idx++] = byte;
            }
        } 
        else
        {
            out_buf[out_idx++] = byte;
        }
    }

    if ((DEBUG == g_print_type) || (ALL == g_print_type))
    {
        fmsh_print("after_parse_data:\r\n");
        for (i = 0; i < out_idx; i++) 
        {
            fmsh_print("%02x ", out_buf[i]);
            if ((i + 1) % 8 == 0)
            {
                fmsh_print("\r\n");
            }
                
        }

        fmsh_print("\r\n");
    }

    /* 年月日 */
    gps_info->year = (u16)((out_buf[2]<<8) | out_buf[3]);
    gps_info->mon = (u8)(out_buf[4]);
    gps_info->day = (u8)(out_buf[5]);

    /* 时分秒 */
    day_second = (u32)((out_buf[6] << 24) | (out_buf[7] << 16) | (out_buf[8] << 8) | out_buf[9]);
    gps_info->hour = day_second / 3600;
    gps_info->min = (day_second % 3600) / 60;
    gps_info->sec = day_second % 60;

    /* 计算完整UTC绝对秒数（1970起点 = 当天0点秒数 + 当天秒数） */
    day_start_utc = get_day_start_utc_sec(gps_info->year, gps_info->mon, gps_info->day);
    utc_total_sec = day_start_utc + day_second;

    if ((DEBUG == g_print_type) || (ALL == g_print_type))
    {
        fmsh_print("utc_total_sec:%llu\r\n", utc_total_sec);
    }
    
    /* 转换为2006年起点的秒数 */
    EPOCH_2006 = init_epoch_2006();
    gps_info->total_sec = (uint64_t)(utc_total_sec - EPOCH_2006);

    tmp = (out_buf[10] << 24) | (out_buf[11] << 16) | (out_buf[12] << 8) | out_buf[13];
    gps_info->longitude = (double)tmp / 10000000;

    tmp = (out_buf[14] << 24) | (out_buf[15] << 16) | (out_buf[16] << 8) | out_buf[17];
    gps_info->latitude = (double)tmp / 10000000;

    tmp = (out_buf[18] << 24) | (out_buf[19] << 16) | (out_buf[20] << 8) | out_buf[21];
    gps_info->altitute = (double)tmp /1000;

    tmp = (out_buf[22] << 24) | (out_buf[23] << 16) | (out_buf[24] << 8) | out_buf[25];
    gps_info->E_speed = (double)tmp / 100;

    tmp = (out_buf[26] << 24) | (out_buf[27] << 16) | (out_buf[28] << 8) | out_buf[29];
    gps_info->N_speed = (double)tmp / 100;

    tmp = (out_buf[30] << 24) | (out_buf[31] << 16) | (out_buf[32] << 8) | out_buf[33];
    gps_info->D_speed = (double)tmp / 100;

    gps_info->status = VALID;

    return 0;
}

int a53_get_gps_info(struct GPS_INFO *gps_info)
{
    uint64_t total_sec_tmp = 0;
    uint64_t cur_fpga_sec = 0;
    uint32_t cur_fpga_sec_in_cycle = 0;
    uint16_t cur_fpga_sec_in_ms = 0;
    struct GPS_INFO tmp = {0};

    if (NULL == gps_info)
    {
        if ((ERRORS == g_print_type) || (ALL == g_print_type))
        {
            fmsh_print("%s-nullptr\r\n", __func__);
        }

        return -1;
    }

    gps_info->status = g_gps_info.status;
    gps_info->latitude = g_gps_info.latitude;
    gps_info->longitude = g_gps_info.longitude;
    gps_info->altitute = g_gps_info.altitute;

    if (XL_TYPE == g_ant_type)
    {
        gps_info->E_speed = g_gps_info.E_speed;
        gps_info->N_speed = g_gps_info.N_speed;
        gps_info->D_speed = g_gps_info.D_speed;
    }
    
    total_sec_tmp = FMSH_ReadReg(0x0, 0x80005598);
    cur_fpga_sec = FMSH_ReadReg(0x0, 0x80005598);

    /* 如果出现了跨秒的情况，以最新的一次秒为准 */
    if (cur_fpga_sec == (total_sec_tmp + 1))
    {
        gps_info->total_sec = cur_fpga_sec;
    }
    else
    {
        gps_info->total_sec = total_sec_tmp;
    }

    if ((DEBUG == g_print_type) ||  (ALL == g_print_type))
    {
        fmsh_print("%s-%d,cur_fpga_sec_r:%u\r\n", __func__, __LINE__, cur_fpga_sec);
    }
    
    gps_info->total_sec = cur_fpga_sec;
    
    cur_fpga_sec_in_cycle = FMSH_ReadReg(0x0, 0x80005594);
    cur_fpga_sec_in_ms = (cur_fpga_sec_in_cycle * 10ULL) / 1000000;
    gps_info->ms = (uint16_t)cur_fpga_sec_in_ms;
    sec2utc(cur_fpga_sec, &gps_info->year, &gps_info->mon, &gps_info->day, 
                &gps_info->hour, &gps_info->min, &gps_info->sec);
    if ((DEBUG == g_print_type) ||  (ALL == g_print_type))
    {
        fmsh_print("%s-%d,cur_fpga_sec_in_r:%u\r\n", __func__, __LINE__, cur_fpga_sec_in_cycle);
    }

    return 0;
}
