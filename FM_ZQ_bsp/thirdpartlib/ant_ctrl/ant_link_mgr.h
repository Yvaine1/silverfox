#ifndef __ANT_LINK_MSG_
#define __ANT_LINK_MSG_
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ant_ctrl.h"
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

// 常量定义
#define ANT_LOCK_NUM    1
#define UDP_ANT_ETH_NAME "eth3:1"
#define UDP_WEB_ETH_NAME "eth3"
#define ANT_BROADCAST_PORT 54000
#define ANT_BROADCAST_IP "255.255.255.255"
#define LX2160_IP "192.168.70.10"
#define UDP_WEB_DEST_PORT 60501
#define UDP_WEB_PORT 51101

extern uint16_t host_udp_port;
extern uint16_t ant_udp_port;

// 状态枚举
typedef enum
{
    ERROR = -1,
    OK = 0,
} STATUS;

// 外部变量声明
extern bool web_addr_valid;

// 函数声明
void ant_link_init(void);
struct udp_pcb* get_ant_host_udp(void);
void    ant_link_startup_phase(void);
void    ant_link_read_ant_udp_info(void);
int32_t ant_link_mgr_create_ant_udp_socket(void);
int32_t ant_link_mgr_create_web_udp_socket(void);
int32_t ant_link_mgr_send_udp_msg_to_ant(char *pBuffer, int32_t bytesRead);
int32_t ant_link_mgr_send_udp_msg_to_web(char *pBuffer, int32_t bytesRead);
int32_t ant_link_close_ant_udp_socket(void);
#endif