
/* Connection handle for a TCP Client session */

#include "lwip/tcp.h"
#include "tcp_demo.h"
#include "fmsh_common.h"
#include "sys_arch.h"

struct tcp_pcb *tcp_server_pcb,*tcp_listen_pcb;
struct tcp_pcb *tcp_client_pcb,*tcp_connect_pcb;
extern ip_addr_t ipaddr;

#define TEST_BUFFER_SIZE 1024

u32 tcp_client_flag = 0;
u32 tcp_server_flag = 0;

u32 *tcp_recv_count;
u32 tcp_cal_count = 0;

u16 cli_send_cnt = 0;

static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *tcp_recv_pbuf, err_t err)
{
      if (tcp_recv_pbuf != NULL)
      {
          tcp_write(pcb, tcp_recv_pbuf->payload, tcp_recv_pbuf->len, 1);
          tcp_recved(pcb, tcp_recv_pbuf->tot_len);
//          tcp_recv_count = (u32 *)(tcp_recv_pbuf->payload);
//          fmsh_print("%u %p \r\n",*tcp_recv_count,tcp_recv_pbuf->payload);
//          if (tcp_cal_count != *tcp_recv_count)
//          {
//             fmsh_print("tcp recv error(%u %u)\r\n",*tcp_recv_count,tcp_cal_count);
//          }         
//          tcp_cal_count++;
      }
      else
      {
          return tcp_close(pcb);
      }
      
      pbuf_free(tcp_recv_pbuf);

      return ERR_OK;
}

static void tcp_server_err(void *arg, err_t err)
{
	if (tcp_listen_pcb != NULL) {
		tcp_recv(tcp_listen_pcb, NULL);
		tcp_err(tcp_listen_pcb, NULL);
		err = tcp_close(tcp_listen_pcb);
		if (err != ERR_OK) {
			/* Free memory with abort */
			tcp_abort(tcp_listen_pcb);
		}
	}
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    tcp_listen_pcb = pcb;
    tcp_arg(pcb, NULL);
    tcp_recv(pcb, tcp_server_recv);
    tcp_err(pcb, tcp_server_err);
    
    return ERR_OK;
}


void tcp_server_init(void)
{ 
    tcp_server_pcb = tcp_new();
    err_t err;
    
    if (tcp_server_pcb != NULL)
    {
       err = tcp_bind(tcp_server_pcb, &ipaddr, TCP_SERVER_PORT);
       if (err == ERR_OK)
       {
          tcp_listen_pcb = tcp_listen(tcp_server_pcb);
        //   tcp_arg(tcp_listen_pcb, NULL);
          tcp_accept(tcp_listen_pcb, tcp_server_accept);
       }
       else
       {
          memp_free(MEMP_TCP_PCB, tcp_server_pcb);
          LWIP_DEBUGF(NETIF_DEBUG, ("can not bind tcp_server_pcb\r\n"));
       }
    }
    else
    {
       LWIP_DEBUGF(NETIF_DEBUG, ("tcp server init failed \r\n"));
    }
}

void tcp_server_test_task()
{
    tcp_server_init();
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb,
                             struct pbuf *p, err_t err)
{
    uint32_t i;
    unsigned char recv_data[1500];
    unsigned int recv_len;

    memset(recv_data,0x0,sizeof(recv_data));

    if (p != NULL)
    {
        recv_len = p->len;
        memcpy(recv_data,p->payload,recv_len);
        tcp_recved(tpcb, p->tot_len);
        tcp_write(tpcb, recv_data, recv_len, 1);
        pbuf_free(p);
    }
    else 
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("tcp client recv NULL\r\n"));
    }
    

    return ERR_OK;
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{    
//    tcp_write(tpcb, "tcp client connected", strlen("tcp client connected"), 0);
    
    tcp_recv(tpcb, tcp_client_recv);
    tcp_connect_pcb = tpcb;
    tcp_client_flag = 1;
    return ERR_OK;
}

void tcp_client_init(void)
{
    ip_addr_t serverIp;
    err_t err;
    
    IP4_ADDR(&serverIp, 192, 168, 255, 19);
    
    tcp_client_pcb = tcp_new();
    if (tcp_client_pcb != NULL)
    {
        tcp_set_flags(tcp_client_pcb,TF_NODELAY);
        err = tcp_bind(tcp_client_pcb, &ipaddr, TCP_CLIENT_PORT);
        if (err == ERR_OK)
        {
            tcp_connect(tcp_client_pcb, &serverIp, 6005,tcp_client_connected);
        }
        else 
        {
            memp_free(MEMP_TCP_PCB, tcp_client_pcb);
            LWIP_DEBUGF(NETIF_DEBUG, ("can not bind pcb\r\n"));
        }

    }
    else 
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("tcp_client_init failed\r\n"));
    }
}

void tcp_client_test_task(void *pvParameters)
{
    tcp_client_init();
    unsigned char test_buffer[TEST_BUFFER_SIZE];
    unsigned int *send_cnt = (unsigned int*)(&test_buffer[0]);
    unsigned int i;
    err_t err;
    u8 apiflags = TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE;
    
    memset(test_buffer,0x0,sizeof(test_buffer));
    
    for(i = 4; i < TEST_BUFFER_SIZE; i++)
    {
        test_buffer[i] = 0xaa;
    }
    
    vTaskDelay(1000);

    while(1)
    {
        if(tcp_client_flag)
        {
            while (tcp_sndbuf(tcp_client_pcb) > TEST_BUFFER_SIZE) {
                err = tcp_write(tcp_client_pcb, test_buffer, TEST_BUFFER_SIZE, apiflags);
                if (err != ERR_OK) {
                    fmsh_print("TCP client: Error on tcp_write: %d\r\n",err);
                }

                err = tcp_output(tcp_client_pcb);
                if (err != ERR_OK) {
                    fmsh_print("TCP client: Error on tcp_output: %d\r\n",err);
                }
                (*send_cnt)++;
            }
        }       
       
    }
}

