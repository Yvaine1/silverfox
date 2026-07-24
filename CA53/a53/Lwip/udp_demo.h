

#ifndef __UDP_DEMO_H_
#define __UDP_DEMO_H_

#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "fmsh_print.h"


#define UDP_SERVER_PORT  6001
#define UDP_CLIENT_PORT  6003
#define UDP_REMOTE_PORT  6005

void udp_server_init(void);
void udp_client_init(void);

#endif 
