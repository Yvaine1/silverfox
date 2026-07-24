

#ifndef __TCP_DEMO_H_
#define __TCP_DEMO_H_

#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "lwip/inet.h"


#define TCP_SERVER_PORT 5001
#define TCP_CLIENT_PORT 5003

void tcp_server_init(void);
void tcp_client_init(void);

#endif 
