#ifndef __RK_UDP_H_
#define __RK_UDP_H_



#define SERVER_PORT           8888
#define CMD_RESP_ERROR        0xFE
#define CMD_RESP_OK           0xFF

typedef enum
{
    HW_ID_00 = 0, //yinhu
    HW_ID_01,     //LRM
    HW_ID_UNKNOWN,
} HW_ID;

typedef enum 
{
    CMD_SET_EEPROM,
    CMD_GET_EEPROM,
    CMD_GET_HWID,
    CMD_SET_RESET,
    CMD_GET_VESION_SYSTEM,
    CMD_SET_CP_MODE,
    CMD_TYPE_MAX,
}RK_CMD_TYPE;

typedef struct
{
    unsigned char cmd_type;
    char cmd_context[64];
}RK_INFO_FORMAT;

typedef struct
{
    unsigned short eeprom_addr;
    unsigned char data_size;
    unsigned char data[16];
}EEPROM_CMD_INFO;


void rk_udp_server_init(void);

#endif