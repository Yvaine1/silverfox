

#ifndef __UDP_IGMP_H_
#define __UDP_IGMP_H_

#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "lwip/inet.h"

#define IGMP_ADDR_RPU_1   239
#define IGMP_ADDR_RPU_2   0
#define IGMP_ADDR_RPU_3   0
#define IGMP_ADDR_RPU_4   11


#define IGMP_ADDR_APU_1   239
#define IGMP_ADDR_APU_2   0
#define IGMP_ADDR_APU_3   0
#define IGMP_ADDR_APU_4   15


#define RPU_SEND_PORT 6006
#define APU_SEND_PORT 8008

int rpu_log_send(unsigned char *send_buf,unsigned int send_len);
int apu_log_send(unsigned char *send_buf,unsigned int send_len,unsigned int send_port);
void udp_igmp_init();

#endif 
