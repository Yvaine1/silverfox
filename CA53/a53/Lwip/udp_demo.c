
#include "udp_demo.h"
#include "lwip/udp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fmsh_common_types.h"
#include <string.h>
#include "sys.h"

extern ip_addr_t ipaddr;
static struct udp_pcb *upcb;
ip_addr_t serverIP;
unsigned char pData[1024];

struct pbuf *p_record = NULL;

static void udp_receive_callback(void *arg, struct udp_pcb *upcb,
       struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    udp_sendto(upcb, p, addr, port);
    pbuf_free(p);
}

void udp_client_send(unsigned char *pData,unsigned int send_len)
{
     struct pbuf *p;
     
     p = pbuf_alloc(PBUF_TRANSPORT, 1024, PBUF_POOL);
     
     if (p != NULL)
     {
         p_record = p;
         memcpy(p->payload, pData, send_len);
  
         udp_send(upcb, p);
     }
     
     pbuf_free(p);
}

void udp_client_init(void)
{
    err_t err;

    IP4_ADDR(&serverIP, 192, 168, 255, 19);

    upcb = udp_new();

    if (upcb!=NULL)
    {

        upcb->local_port = UDP_CLIENT_PORT;
        
        err= udp_connect(upcb, &serverIP, UDP_REMOTE_PORT);

        if (err == ERR_OK)
        {           
            udp_recv(upcb, udp_receive_callback, NULL);                              
            fmsh_print("udp client connected\r\n");
        }
        else
        {
            udp_remove(upcb);
            
            fmsh_print("can not connect udp pcb\r\n");
        }
    }
}

static void udp_server_receive_callback(void *arg, struct udp_pcb *upcb,
     struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  
    udp_sendto(upcb, p, addr, port);
    pbuf_free(p);
}


void udp_server_init(void)
{
    err_t err;

    upcb = udp_new();

    if (upcb)
    {
        err = udp_bind(upcb, &ipaddr, UDP_SERVER_PORT);

        if(err == ERR_OK)
        {
            udp_recv(upcb, udp_server_receive_callback, NULL);
        }
        else
        {
            udp_remove(upcb);
            
            fmsh_print("can not bind pcb\r\n");
        }
    }
}

void udp_test_task(void *pvParameters)
{
    udp_client_init();
    
    unsigned int *send_count = (unsigned int*)(&pData[0]);
    unsigned int m;
    memset(pData,0x0,sizeof(pData));
    
    for (m = 4; m < 1024; m++)
    {
       pData[m] = 0xaa;
    }
             
    while(1)
    {
        udp_client_send(pData,1024);
        (*send_count)++;
        vTaskDelay(2);

    }
}






