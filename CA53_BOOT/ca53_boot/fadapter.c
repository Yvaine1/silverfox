
#include "lwipopts.h"
#include "fmsh_common.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"

#include "netif/etharp.h"
#include "fmsh_gmacpsif.h"
#include "fadapter.h"



/* global lwip debug variable used for debugging */
int lwip_runtime_debug = 0;

volatile int TcpFastTmrFlag = 1;
volatile int TcpSlowTmrFlag = 0;

void
lwip_raw_init()
{
	ip_init();	/* Doesn't do much, it should be called to handle future changes. */
#if LWIP_UDP
	udp_init();	/* Clears the UDP PCB list. */
#endif
#if LWIP_TCP
	tcp_init();	/* Clears the TCP PCB list and clears some internal TCP timers. */
			/* Note: you must call tcp_fasttmr() and tcp_slowtmr() at the */
			/* predefined regular intervals after this initialization. */
#endif
}


struct netif *
fmsh_gmac_add(struct netif *netif,
	ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
	unsigned char *mac_ethernet_address,
	unsigned mac_baseaddr)
{
	int i;


	/* set mac address */
	netif->hwaddr_len = 6;
	for (i = 0; i < 6; i++)
		netif->hwaddr[i] = mac_ethernet_address[i];

	/* initialize based on MAC type */

	return netif_add(netif, ipaddr, netmask, gw,
                         (void*)(UINTPTR)mac_baseaddr,
                         fmsh_gmacpsif_init,
                         ethernet_input
                           );

}

int fmsh_gmacif_input(struct netif *netif)
{
	struct fgmac_s *emac = (struct fgmac_s *)netif->state;

	int n_packets = fmsh_gmacpsif_input(netif);

	return n_packets;
}





