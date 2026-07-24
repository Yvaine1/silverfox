
/* Connection handle for a UDP Client session */

#include "udp_igmp.h"
#include "lwip/tcp.h"
#include "fmsh_common.h"
#include "fadapter.h"

//extern struct netif server_netif;
static struct udp_pcb *udp_multi_pcb[2];

ip_addr_t group_addr_rpu,group_addr_apu;

int udp_packet_send(struct udp_pcb * udp_send_pcb, unsigned char *send_buf,unsigned int send_len, ip_addr_t *send_addr,unsigned int send_port)
{
	struct pbuf *packet;

	packet = pbuf_alloc(PBUF_RAW, send_len, PBUF_POOL);
	if (!packet) 
    {
		return -1;
	} 
    else 
    {
		memcpy(packet->payload, send_buf, send_len);
	}

	udp_sendto(udp_send_pcb, packet,send_addr,send_port);

	vTaskDelay(10);

	pbuf_free(packet);
    
    return 0;
}


int rpu_log_send(unsigned char *send_buf,unsigned int send_len)
{
   return udp_packet_send(udp_multi_pcb[0],send_buf,send_len,&group_addr_rpu,RPU_SEND_PORT);
}

int apu_log_send(unsigned char *send_buf,unsigned int send_len,unsigned int send_port)
{
   return udp_packet_send(udp_multi_pcb[1],send_buf,send_len,&group_addr_apu,send_port);
}

void udp_igmp_init()
{
	IP4_ADDR(&group_addr_rpu,IGMP_ADDR_RPU_1,IGMP_ADDR_RPU_2,IGMP_ADDR_RPU_3,IGMP_ADDR_RPU_4);
    IP4_ADDR(&group_addr_apu,IGMP_ADDR_APU_1,IGMP_ADDR_APU_2,IGMP_ADDR_APU_3,IGMP_ADDR_APU_4);

	udp_multi_pcb[0] = udp_new();

    udp_multi_pcb[1] = udp_new();
}
