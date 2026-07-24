#ifndef __FADAPTER_H_
#define __FADAPTER_H_


#ifdef __cplusplus
extern "C" {
#endif
#include "fmsh_gmacpsif.h"
#include "lwipopts.h"


#include "lwip/netif.h"
#include "lwip/ip.h"
  
int fmsh_gmacif_input(struct netif *netif);

struct netif *
fmsh_gmac_add(struct netif *netif,
	ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
	unsigned char *mac_ethernet_address,
	unsigned mac_baseaddr);























#ifdef __cplusplus
}
#endif
#endif