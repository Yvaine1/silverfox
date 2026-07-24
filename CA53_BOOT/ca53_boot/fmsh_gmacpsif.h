

#ifndef __FMSH_GMACPSIF_H__
#define __FMSH_GMACPSIF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fmsh_lwipconfig.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/sys.h"


#include "fmsh_pqueue.h"
#include "fmsh_lwipconfig.h"
#include "fmsh_gmac.h"

#define CRL_APB_GEM0_REF_CTRL	0xFF5E0050
#define CRL_APB_GEM1_REF_CTRL	0xFF5E0054
#define CRL_APB_GEM2_REF_CTRL	0xFF5E0058
#define CRL_APB_GEM3_REF_CTRL	0xFF5E005C

#define CRL_APB_GEM_DIV0_MASK	0x00003F00
#define CRL_APB_GEM_DIV0_SHIFT	8
#define CRL_APB_GEM_DIV1_MASK	0x003F0000
#define CRL_APB_GEM_DIV1_SHIFT	16



#if defined (USE_JUMBO_FRAMES)
#define FMPSOC_USE_JUMBO
#endif

err_t 	fmsh_gmacpsif_init(struct netif *netif);



enum fgmac_types { fgmac_type_unknown = -1, fgmac_type_xps_emaclite, fgmac_type_xps_ll_temac, fgmac_type_axi_ethernet, fgmac_type_gmacps };

struct fgmac_s {
  
	enum fgmac_types type;
	int  topology_index;
	void *state;
};


/* structure within each netif, encapsulating all information required for
 * using a particular temac instance
 */
typedef struct {
	FGmacPs gmacps;

	/* queue to store overflow packets */
	pq_queue_t *recv_q;
	pq_queue_t *send_q;

	/* pointers to memory holding buffer descriptors (used only with SDMA) */
	void *rx_bdspace;
	void *tx_bdspace;

	unsigned int last_rx_frms_cntr;

} fgmacpsif_s;

extern fgmacpsif_s fgmacpsif;


s32_t	is_tx_space_available(fgmacpsif_s *gmac);



void  process_sent_bds(fgmacpsif_s *fgmacpsif, FGmacPs_BdRing *txring);
void gmacps_send_handler(void *arg);
FStatus gmacps_sgsend(fgmacpsif_s *fgmacpsif, struct pbuf *p);
void gmacps_recv_handler(void *arg);
void gmacps_error_handler(void *arg,u8 Direction, u32 ErrorWord);
void setup_rx_bds(fgmacpsif_s *fgmacpsif, FGmacPs_BdRing *rxring);
void HandleTxErrors(struct fgmac_s *fgmac);
void HandleEmacPsError(struct fgmac_s *fgmac);
FGmacPs_Config *fgmacps_lookup_config(unsigned mac_base);
void setup_isr (struct fgmac_s *fgmac);
FStatus init_dma(struct fgmac_s *fgmac);
void free_txrx_pbufs(fgmacpsif_s *fgmacpsif);
void free_onlytx_pbufs(fgmacpsif_s *fgmacpsif);
void clean_dma_txdescs(struct fgmac_s *fgmac);
void resetrx_on_no_rxdata(fgmacpsif_s *fgmacpsif);
void reset_dma(struct fgmac_s *fgmac);


s32_t 	fmsh_gmacpsif_input(struct netif *netif);

#ifdef __cplusplus
}
#endif

#endif 
