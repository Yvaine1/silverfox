/** 
 * @file   dg_tod_utc.c
 * @note   Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved.
 * @brief   
 *
 * @author guodecai	
 * @date   2026/05/18
 *
 * @version
 *  date        |version |author              |message
 *  :----       |:----   |:----               |:------
 *  2026/05/18  |V1.0    |guodecai            |create base code
 * @warning 
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "dg_tod_utc.h"

struct GPS_INFO r50_utc = {0};
struct GPS_INFO r51_utc = {0};

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

static void print_gps_info(struct GPS_INFO gps_info)
{
    fmsh_print("GPS_INFO:\r\n");
    fmsh_print("  total_sec: %llu\r\n", (u64)gps_info.total_sec);
    fmsh_print("  utc-time: %04u-%02u-%02u %02u:%02u:%02u.%03u\r\n",
              gps_info.year, gps_info.mon, gps_info.day,
              gps_info.hour, gps_info.min, gps_info.sec, gps_info.ms);
#if 0         
    fmsh_print("  status:    %s\r\n", (VALID == gps_info.status) ? "Valid (A)" : "Invalid (V)");
    fmsh_print("  latitude:  %.7f \r\n", gps_info.latitude);
    fmsh_print("  longitude: %.7f \r\n", gps_info.longitude);
    fmsh_print("  altitude:  %.3f m\r\n", gps_info.altitute);
    fmsh_print("  E_speed:  %.2f m/s\r\n", gps_info.E_speed);
    fmsh_print("  N_speed:  %.2f m/s\r\n", gps_info.N_speed);
    fmsh_print("  D_speed:  %.2f m/s\r\n", gps_info.D_speed);
#endif
}

int get_gps_info(struct GPS_INFO *gps_info)
{
    uint64_t total_sec_tmp = 0;
    uint64_t cur_fpga_sec = 0;
    uint16_t cur_fpga_sec_in_ms = 0;
    uint32_t cur_fpga_sec_in_cycle = 0;

    struct GPS_INFO tmp = {0};
    
    if (NULL == gps_info)
    {
        fmsh_print("%s-nullptr\r\n", __func__);
        return -1;
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

    cur_fpga_sec_in_cycle = FMSH_ReadReg(0x0, 0x80005594);
    cur_fpga_sec_in_ms = (cur_fpga_sec_in_cycle * 10ULL) / 1000000;
    gps_info->ms = (uint16_t)cur_fpga_sec_in_ms;
    sec2utc(cur_fpga_sec, &gps_info->year, &gps_info->mon, &gps_info->day, 
                &gps_info->hour, &gps_info->min, &gps_info->sec);
    return 0;
}

#ifdef R50_UTC_TEST
void r50_get_utc_time(void)
{
    while(1)
    {
        get_gps_info(&r50_utc);
        print_gps_info(r50_utc);
        delay_ms(1000);
    }
}
#endif

#ifdef R51_UTC_TEST
void r51_get_utc_time(void)
{
    get_gps_info(&r51_utc);
    print_gps_info(r51_utc);
}
#endif
