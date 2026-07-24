/** 
 * @file   gps.h
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

#ifndef _GPS_H_
#define _GPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fmsh_common.h"

#define VALID        (1)
#define INVALID      (0)

/**
  * @brief GPS报文结构体
  */
struct GPS_INFO
{
  uint64_t total_sec;             /* 从2006-1-1 00:00:00.000开始的utc总秒数,FPGA寄存器维护 */  
  uint16_t year;                  /* utc年 */
  uint8_t mon;                    /* utc月 */
  uint8_t day;                    /* utc日 */
  uint8_t hour;                   /* utc时 */
  uint8_t min;                    /* utc分 */
  uint8_t sec;                    /* utc秒 */
  uint16_t ms;                    /* 秒内utc毫秒,FPGA寄存器维护 */
  uint8_t status;			            /* 定位状态 1(A)=有效定位，0(V)=无效定位 */
  double latitude;		            /* 纬度,单位：度 */
  double longitude;		            /* 经度 单位：度 */
  double altitute;                /* 高度，单位：米 */
  double E_speed;                 /* 星历东向速度，单位：米/秒 */
  double N_speed;                 /* 星历北向速度，单位：米/秒 */
  double D_speed;                 /* 星历天向速度，单位：米/秒 */
};

/**
 * @brief 获取GPS报文信息
 * @param gps_info GPS报文结构体指针
 * @return 0：获取成功 -1:获取失败
 */
int get_gps_info(struct GPS_INFO *gps_info);
#ifdef __cplusplus
}
#endif

#endif /*_GPS_H_*/