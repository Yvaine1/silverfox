
#include "gmac_init.h"
#include "lwip/tcp.h"
#include "fadapter.h"
#include "ftpd.h"

#include "udp_igmp.h"
#include "tcp_demo.h"
#include "udp_demo.h"

#include "platform.h"
#include "rk_udp.h"

struct netif server_netif;
ip_addr_t ipaddr, netmask, gw;

extern void app_init(void);
extern void udp_test_task(void *pvParameters);
extern void tcp_client_test_task(void *pvParameters);
extern void tcp_server_test_task();

#define UDP_SEND_DEMO_TEST    0
#define TCP_CLIENT_SEND_DEMO_TEST 0
#define TCP_SERVER_RECV_DEMO_TEST 0

void prv_init_task(void *pvParameters)
{
    unsigned char mac_ethernet_address[] =
    { 0x00, 0x0a, 0x35, 0x01, 0x02, 0x03 };
    int ret;
    
    vPortTaskUsesFPU();
    
    tcpip_init(NULL,NULL);
    
    vTaskDelay(100);
    IP4_ADDR(&ipaddr,  10, 255,0,  2);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw,      10, 255,0,  10);
    
    fmsh_gmac_add(&server_netif, &ipaddr, &netmask,&gw, mac_ethernet_address,GMAC_SELECT_BASEADDR);
    netif_set_default(&server_netif);
    netif_set_up(&server_netif);
    ftpd_init();
    rk_udp_server_init();
    app_init();
    
#if UDP_SEND_DEMO_TEST
    xTaskCreateAffinitySet(udp_test_task, "udp send test", 2048, NULL,
                                TASK_PRIORITY_1, AFFINITY_CORE0, NULL);
#endif

#if TCP_CLIENT_SEND_DEMO_TEST
    xTaskCreateAffinitySet(tcp_client_test_task, "tcp client test", 2048, NULL,
                                TASK_PRIORITY_1, AFFINITY_CORE0, NULL);
#endif
    
#if TCP_SERVER_RECV_DEMO_TEST
    tcp_server_test_task();
#endif

    struct fgmac_s *fgmac = (struct fgmac_s *)(server_netif.state);

    for (;;)
    {      
      sys_sem_wait( &fgmac->sem_rx_data_available );
      fmsh_gmacpsif_input(&server_netif);
    }
}












