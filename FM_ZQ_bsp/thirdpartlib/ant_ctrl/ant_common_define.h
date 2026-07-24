#ifndef __ANT_COMMON_DEFINE_
#define __ANT_COMMON_DEFINE_

#pragma once
#include <math.h>
#include <stdint.h>

#define FRAME_HEAD_END  0x7E
#define RESP_KEYWORD_OFFSET   4
#define MAX_UDP_MESSAGE_SIZE  2048

typedef enum
{
    RESP_SUCCESS             = 0x20,
    RESP_FAIL                = 0x21,
    RESP_TIMEOUT             = 0x22,
    RESP_UNKNOWN             = 0x23,
    RESP_OBJECT_NOT_EXIST    = 0x25,
    RESP_PARA_ERROR          = 0x26,
    RESP_CONDITION_NOT_HAVE  = 0x27,
    RESP_REFUSE_EXECUTE      = 0x28,
    RESP_FRAME_ERROR         = 0x29
}RESP_STATE;

typedef enum
{
    ANT_WRITE_OPEN                      = 0x00,
    ANT_WRITE_SWITCH_STAR               = 0x01,
    ANT_WRITE_KA_SEND_FREQ              = 0x02,
    ANT_WRITE_SEND_BWP_CENTER_FREQ      = 0x03,
    ANT_WRITE_KA_RCV_FREQ               = 0x04,
    ANT_WRITE_RCV_BWP_CENTER_FREQ       = 0x05,
    ANT_WRITE_KA_BUC_ATTENUATION        = 0x06,
    ANT_WRITE_KA_BDC_ATTENUATION        = 0x07,
    ANT_WRITE_KA_SWITCH                 = 0x08,
    ANT_WRITE_HIGHRAIL_RCV_POLARIZATION = 0x09,
    ANT_WRITE_RSRP                      = 0x0C,
    ANT_WRITE_AZIMUTH_PITCHANGLE        = 0x0D,
    ANT_WRITE_SEND_POLARIZATION         = 0x11,
    ANT_WRITE_RCV_POLARIZATION          = 0x12,
    ANT_WRITE_SATELLITE_ID              = 0x13,
    ANT_WRITE_BEACON_FREQ               = 0x1C,
    ANT_WRITE_C_BUC_ATTENUATION         = 0x2A,
    ANT_WRITE_C_BDC_ATTENUATION         = 0x2B,
    ANT_WRITE_C_SWITCH                  = 0x2C,
    ANT_WRITE_C_SEND_FREQ               = 0x2E,
    ANT_WRITE_C_RCV_FREQ                = 0x2F,
    ANT_WRITE_VECTOR_POSITION           = 0x30,
    ANT_WRITE_SATELLITE_LON_AND_LAT     = 0x31,
    ANT_WRITE_MASTER_OR_SLAVE           = 0x50,    //1:主  2：从
    ANT_WRITE_RUNNING_STATE             = 0x52,    //0:休眠  1:工作
    ANT_WRITE_NET_STATE                 = 0x71
}ANT_WRITE_PARAMETER_KEYWORD;

typedef enum
{
    ANT_READ_KA_SEND_FREQ                 = 0x02,
    ANT_READ_SEND_BWP_CENTER_FREQ         = 0x03,
    ANT_READ_KA_RCV_FREQ                  = 0x04,
    ANT_READ_RCV_BWP_CENTER_FREQ          = 0x05,
    ANT_READ_KA_BUC_ATTENUATION           = 0x06,
    ANT_READ_KA_BDC_ATTENUATION           = 0x07,
    ANT_READ_KA_SWITCH                    = 0x08,
    ANT_READ_HIGHRAIL_RCV_POLARIZATION    = 0x09,
    ANT_READ_PHASE_ARRAY_STATE            = 0x0A,
    ANT_READ_TYPE                         = 0x0B,
    ANT_READ_AZIMUTH_PITCHANGLE           = 0x0D,
    ANT_READ_PARABOLIC_STATE              = 0x0E,
    ANT_READ_SEND_POLARIZATION            = 0x11,
    ANT_READ_RCV_POLARIZATION             = 0x12,
    ANT_READ_SATELLITE_ID                 = 0x13,
    ANT_READ_TEMP                         = 0x14,
    ANT_READ_HIGHRAIL_POLARIZATION        = 0x1D,
    ANT_READ_PARABOLOID_ACU_STATE         = 0x20,
    ANT_READ_PARABOLOID_RF_STATE          = 0x21,
    ANT_READ_PARABOLOID_BEACON_STATE      = 0x22,
    ANT_READ_KA_AMP_SEND_POWER            = 0x23,
    ANT_READ_KA_AMP_TEMP                  = 0x24,
    ANT_READ_KA_RX_LOCAL_OSCILLATOR_FREQ  = 0x25,
    ANT_READ_KA_TX_LOCAL_OSCILLATOR_FREQ  = 0x26,
    ANT_READ_C_BUC_ATTENUATION            = 0x27,
    ANT_READ_C_BDC_ATTENUATION            = 0x28,
    ANT_READ_C_SWITCH                     = 0x29,
    ANT_READ_C_AMP_SEND_FREQ              = 0x2A,
    ANT_READ_C_AMP_TEMP                   = 0x2B,
    ANT_READ_C_AMP_SWITCH                 = 0x2C,
    ANT_READ_C_AMP_FAULT_STATE            = 0x2D,
    ANT_READ_C_RX_LOCAL_OSCILLATOR_FREQ   = 0x2E,
    ANT_READ_C_TX_LOCAL_OSCILLATOR_FREQ   = 0x2F,
    ANT_READ_C_AMP_VERSION                = 0x30,
    ANT_READ_BEACON_RCV_SIGNAL_STRENGTH   = 0x31,
    ANT_READ_BEACON_FREQ                  = 0x32,
    ANT_READ_BEACON_SNR                   = 0x33,
    ANT_READ_WORKING_STATE                = 0x34,
    ANT_READ_LOCK_STATE                   = 0x35,
    ANT_READ_ACU_DEVICE_NAME              = 0x37,
    ANT_READ_ACU_MANUFACTURER             = 0x38,
    ANT_READ_ACU_SERIAL_NUMBER            = 0x39,
    ANT_READ_ACU_MANU_DATE                = 0x3A,
    ANT_READ_ACU_SOFT_VERSION             = 0x3B,
    ANT_READ_RF_DEVICE_NAME               = 0x3C,
    ANT_READ_RF_MANUFACTURER              = 0x3D,
    ANT_READ_RF_SERIAL_NUMBER             = 0x3E,
    ANT_READ_RF_MANU_DATE                 = 0x3F,
    ANT_READ_RF_SOFT_VERSION              = 0x40,
    ANT_READ_BEACON_DEVICE_NAME           = 0x41,
    ANT_READ_BEACON_MANUFACTURER          = 0x42,
    ANT_READ_BEACON_SERIAL_NUMBER         = 0x43,
    ANT_READ_BEACON_MANU_DATE             = 0x44,
    ANT_READ_BEACON_SOFT_VERSION          = 0x45,
    ANT_READ_ABILITY                      = 0x46,
    ANT_READ_SEND_CHANNEL_GAIN            = 0x47,
    ANT_READ_BUC_ATTENUATION_RANGE        = 0x48,
    ANT_READ_RECV_CHANNEL_GAIN            = 0x49,
    ANT_READ_BDC_ATTENUATION_RANGE        = 0x4A,
    ANT_READ_MASTER_OR_SLAVE              = 0x51,    //1:主  2：从
    ANT_READ_RUNNING_STATE                = 0x52     //0:休眠  1:工作
}ANT_READ_PARAMETER_KEYWORD;

typedef enum
{
    ANT_MANUFACTURER_CODE      = 0x00,
    ANT_DEVICE_NAME            = 0x6E,
    ANT_DEVICE_MANUFACTURER    = 0x6F,
    ANT_DEVICE_SERIAL_NUMBER   = 0x70,
    ANT_DEVICE_FACTORY_DATE    = 0x71,
    ANT_SOFTWARE_VERSION       = 0x72
}ANT_FACTORY_DATA;

typedef enum
{
    ANT_PARA_SET               = 0x80,
    ANT_PARA_SET_RESP          = 0x81,
    ANT_ALL_CHECK              = 0x82,
    ANT_ALL_CHECK_RESP         = 0x83,
    ANT_REPORT                 = 0x84,
    ANT_REPORT_RESP            = 0x85,
    ANT_FACTORY_INFO           = 0x86,
    ANT_FACTORY_INFO_RESP      = 0x87,
    ANT_BROAD_SEARCH           = 0x88,
    ANT_BROAD_SEARCH_RESP      = 0x89,
    ANT_EPHE_SET               = 0x8A,
    ANT_EPHE_SET_RESP          = 0x8B
}ANT_MSG_KEYWORD;

typedef enum
{
    SELF_TEST_STATE            = 0x81,
    PARABOLOID_LOCATION        = 0x83,
    ANT_MASTER_OR_SLAVE        = 0x84
}ANT_DATA_REPORT;

typedef enum
{
    ANT_WORKING_MODE               = 0x30,
    ANT_WORKING_AZIMUTH            = 0x31,
    ANT_WORKING_PITCH_ANGLE        = 0x32,
    ANT_WORKING_POLARIZATION_ANGLE = 0x33,
    FAXIANG_EIRP                   = 0x34,
    MIN_ELEVATION_EIRP             = 0x35,
    FAXAING_GT                     = 0x36,
    MIN_ELEVATION_GT               = 0x37,
    UE_MIN_ELEVATION               = 0x38,
    ANT_AZIMUTH                    = 0x39,
    ANT_PITCH_ANGLE                = 0x3A
}STAR_SWITCH_DATA;

typedef enum
{
    WEB_CMD_ANTENNA_ON            = 0x01,
    WEB_CMD_ANTENNA_OFF           = 0x02,
    WEB_CMD_SEND_FREQ             = 0x03,
    WEB_CMD_RECV_FREQ             = 0x04,
    WEB_CMD_HIGH_ORBIT_POLAR      = 0x05,
    WEB_CMD_SEND_POLARIZATION     = 0x06,
    WEB_CMD_RECV_POLARIZATION     = 0x07,
    WEB_CMD_BUC_SET               = 0x08,
    WEB_CMD_BDC_SET               = 0x09,
    WEB_CMD_BEACON_FREQ           = 0x0A,
    WEB_CMD_VECTOR_POSITION       = 0x0B,
    WEB_CMD_SATELLITE_LON         = 0x0C
} WEB_CMD_KEYWORD;

typedef enum
{
    ANT_TYPE_SET                 = 0x0003,
    ANT_TYPE_SET_RESP            = 0x0004,
    ANT_TYPE_GET                 = 0x0007,
    ANT_TYPE_GET_RESP            = 0x0008,
    ANT_TYPE_REPORT              = 0x0009,
    ANT_TYPE_TEST                = 0x0010
}UDP_WEB_MSG_TYPE;

typedef enum
{
    WEB_SEND_FREQ              = 0,
    WEB_RECV_FREQ              = 1,
    WEB_SEND_POLARIZATION      = 2,
    WEB_RECV_POLARIZATION      = 3,
    WEB_BDC_ATTENUATION        = 4,
    WEB_BUC_ATTENUATION        = 5,
    WEB_BAUD_RATE              = 6
}UDP_WEB_ANT_CFG_TYPE;

typedef struct
{
    uint8_t frame_head;
    uint16_t length;
    uint8_t keyword;
    uint8_t selfcheck_keyword;
    uint8_t selfcheck_result;
    uint8_t posture_keyword;
    uint8_t posture_result;
    uint8_t checkword;
    uint8_t frame_end;
}ANT_REPORT_RESP_INFO;

typedef struct
{
    int16_t satellite_id;
    int16_t data_sum;
    int32_t gps_week;
    int32_t gps_w_s;
    double wgs_84_location_x;
    double wgs_84_location_y;
    double wgs_84_location_z;
    double wgs_84_speed_x;
    double wgs_84_speed_y;
    double wgs_84_speed_z;
    int16_t data_num;
}EPHEMERIS_DATAS;

typedef struct
{
    uint8_t frame_head;
    uint16_t length;
    uint8_t keyword;
    EPHEMERIS_DATAS epheris_data;
    uint8_t checkword;
    uint8_t frame_end;
}ANT_EPHEMERIS_DATAS;

typedef struct
{
    uint8_t frame_head;
    uint8_t type;
    uint8_t subtype;
    uint16_t address;
    uint16_t frame_num;
    uint16_t length;
    uint8_t keyword;
    uint8_t checkword;
    uint8_t frame_end; 
}BROADCAST_SEARCH;

typedef struct
{
    uint8_t frame_head;
    uint8_t type;
    uint8_t subtype;
    uint16_t address;
    uint16_t frame_num;
    uint16_t length;
    uint8_t keyword;
    uint8_t type_1;
    uint8_t length_1;
    char device_ip[32];
    char mask[32];
    uint16_t device_port;
    uint8_t checkword;
    uint8_t frame_end;
}BROADCAST_SEARCH_RESP;

typedef struct
{
    uint16_t length;
    char dest_addr[3];
    uint16_t type;
}UDP_HEAD;

#define MAX_IP_ADDR_COUNT   15
#pragma pack(1)
typedef struct
{
    char device_name[32];
    char manufacturer[32];
    char serial_number[32];
    char manufacture_date[32];
    char sw_version[32];
    uint32_t ka_send_freq;
    uint32_t send_bwp_freq;
    uint32_t ka_recv_freq;
    uint32_t recv_bwp_freq;
    uint16_t ka_buc;
    uint16_t ka_bdc;
    uint8_t ka_ant_switch;
    uint16_t phased_array_state;
    uint8_t ant_type;
    uint16_t azimuth;
    uint16_t pitch_angle;
    uint16_t paraboloid_state;
    uint8_t send_polarization;
    uint8_t recv_polarization;
    uint16_t satellite_id;
    int16_t ant_temp;
    uint8_t faxiang_eirp;
    uint8_t min_eirp;
    int16_t faxiang_gt;
    int16_t min_gt;
    uint8_t ue_min_angle;
    uint8_t send_channel_gain;
    uint8_t buc_attenuation_range;
    uint8_t recv_channel_gain;
    uint8_t bdc_attenuation_range;
    uint8_t work_ant_id;
    uint8_t run_state;
}ANT_ALL_CHECK_DATA;

typedef struct
{
    uint16_t type;
    uint16_t length;
    char ant_ip[MAX_IP_ADDR_COUNT];
    uint16_t ant_port;
    char local_ip[MAX_IP_ADDR_COUNT];
    uint16_t local_port;
}ANT_UDP_CFG_DATA;

typedef struct{
    uint16_t type;
    uint16_t length;
    int16_t year;
    int8_t month;
    int8_t day;
    int32_t utc_s;
    int32_t lat;
    int32_t lon;
    int32_t height;
}LON_LAT_HEIGHT_DATA;

typedef enum{
    ANT_SWITCH_SAT_RESP = 0,
    ANT_EPHEM_SET_RESP
}ant_udp_rcv_msg_type_e;

typedef struct{
    uint32_t msg_id;
    uint32_t result;
}ant_udp_rcv_msg_t;

#pragma pack()
#endif
