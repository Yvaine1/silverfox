#include "lwipopts.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/inet_chksum.h"


#include "fmsh_gmacpsif.h"
#include "fmsh_gmac_bd.h"

#include "fmsh_lwipconfig.h"
#include "fmsh_parameters.h"

#include "fmsh_psu_parameters.h"
#include "fmsh_gic_hw.h"
#include "fmsh_gmac_status.h"
#include "fmsh_cache.h"
#include "ftopology.h"
#include "fmsh_gic.h"
#include "fmsh_lwipconfig.h"

#include "arch/sys_arch.h" 

#define INTC_BASE_ADDR		FPAR_SCUGIC_CPU_BASEADDR
#define INTC_DIST_BASE_ADDR	FPAR_SCUGIC_DIST_BASEADDR

extern FGicPs IntcInstance; 
extern sys_sem_t mac_test;
extern QueueHandle_t xQueue_l;

u32_t bd_irq_count_1 = 0;
u32_t bd_irq_count_2 = 0;

/* Byte alignment of BDs */
#define BD_ALIGNMENT (FGMACPS_DMABD_MINIMUM_ALIGNMENT*2)

/* A max of 4 different ethernet interfaces are supported */
static UINTPTR tx_pbufs_storage[4*FLWIP_CONFIG_N_TX_DESC];
static UINTPTR rx_pbufs_storage[4*FLWIP_CONFIG_N_RX_DESC];

static s32_t gmac_intr_num;


struct ftopology_t ftopology[] = {
	{
		FPAR_GMACPS_0_BASEADDR,
		fgmac_type_gmacps,
		0x0,
		0x0,
		0xF8F00100,
		GEM0_INT_ID,
	},
        {
		FPAR_GMACPS_1_BASEADDR,
		fgmac_type_gmacps,
		0x0,
		0x0,
		0xF8F00100,
		GEM1_INT_ID,
	},
        {
		FPAR_GMACPS_2_BASEADDR,
		fgmac_type_gmacps,
		0x0,
		0x0,
		0xF8F00100,
		GEM2_INT_ID,
	},
        {
		FPAR_GMACPS_3_BASEADDR,
		fgmac_type_gmacps,
		0x0,
		0x0,
		0xF8F00100,
		GEM3_INT_ID,
	}
};

/******************************************************************************
 * Each BD is of 8 bytes of size and the BDs (BD chain) need to be  put
 * at uncached memory location. If they are not put at uncached
 * locations, the user needs to flush or invalidate for each BD/packet.
 * However, the flush or invalidate can happen over a cache line which can
 * span multiple BDs. This means a flush or invalidate of one BD can actually
 * flush/invalidate multiple BDs adjacent to the targeted BD.Assuming that
 * the user and hardware both update the BD fields, this operation from user
 * can potentially overwrite the updates done by hardware or user.
 * To avoid this, it is always safe to put the BD chains for Rx and tx side
 * at uncached memory location.
 *
 * The Xilinx standalone BSP for Cortex A9 implements only primary page tables.
 * Each table entry corresponds to 1 MB of address map. This means, if a memory
 * region has to be made uncached, the minimum granularity will be of 1 MB.
 *
 * The implementation below allocates a 1 MB of u8 array aligned to 1 MB.
 * This ensures that this array is put at 1 MB aligned memory (e.g. 0x1200000)
 * and accupies memory of 1 MB. The init_dma function then changes 1 MB of this
 * region to make it uncached (strongly ordered).
 * This increases the bss section of the program significantly and can be a
 * wastage of memory. The reason beings, BDs will hardly occupy few KBs of
 * memory and the rest of 1 MB of memory will be unused.
 *
 * If a program uses other peripherals that have DMAs/bus masters and need
 * uncached memory, they may also end of following the same approach. This
 * definitely aggravates the memory wastage issue. To avoid all this, the user
 * can create a new 1 MB section in the linker script and reserve it for such
 * use cases that need uncached memory location. They can then have their own
 * memory allocation logic in their application that allocates uncached memory
 * from this 1 MB location. For such a case, changes need to be done in this
 * file and appropriate uncached memory allocated through other means can be
 * used.
 *
 * The present implementation here allocates 1 MB of uncached memory. It
 * reserves of 64 KB of memory for each BD chain. 64 KB of memory means 8192 of
 * BDs for each BD chain which is more than enough for any application.
 * Assuming that both emac0 and emac1 are present, 256 KB of memory is allocated
 * for BDs. The rest 768 KB of memory is just unused.
 *********************************************************************************/

#if defined __aarch64__
__attribute__ ((section(".GMAC_LWIP_DESC"), aligned (0x200000)))
u8_t bd_space[0x200000] __attribute__ ((aligned (0x200000)));
#else
u8_t bd_space[0x100000] __attribute__ ((aligned (0x100000)));
#endif
static volatile u32_t bd_space_index = 0;
static volatile u32_t bd_space_attr_set = 0;

#ifdef OS_IS_FREERTOS
long xInsideISR = 0;
#endif

#define FGMACPS_BD_TO_INDEX(ringptr, bdptr)				\
	(((UINTPTR)bdptr - (UINTPTR)(ringptr)->BaseBdAddr) / (ringptr)->Separation)


s32_t is_tx_space_available(fgmacpsif_s *gmac)
{
	FGmacPs_BdRing *txring;
	s32_t freecnt = 0;

	txring = &(FGmacPs_GetTxRing(&gmac->gmacps));

	/* tx space is available as long as there are valid BD's */
	freecnt = FGmacPs_BdRingGetFreeCnt(txring);
	return freecnt;
}


static inline
u32_t get_base_index_txpbufsstorage (fgmacpsif_s *xgmacpsif)
{
	u32_t index;
#ifdef FPAR_GMACPS_0_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_0_BASEADDR) {
		index = 0;
	}
#endif
#ifdef FPAR_GMACPS_1_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_1_BASEADDR) {
		index = FLWIP_CONFIG_N_TX_DESC;
	}
#endif
#ifdef FPAR_GMACPS_2_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_2_BASEADDR) {
		index = 2 * FLWIP_CONFIG_N_TX_DESC;
	}
#endif
#ifdef FPAR_GMACPS_3_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_3_BASEADDR) {
		index = 3 * FLWIP_CONFIG_N_TX_DESC;
	}
#endif
	return index;
}

static inline
u32_t get_base_index_rxpbufsstorage (fgmacpsif_s *xgmacpsif)
{
	u32_t index;
#ifdef FPAR_GMACPS_0_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_0_BASEADDR) {
		index = 0;
	}
#endif 
#ifdef FPAR_GMACPS_1_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_1_BASEADDR) {
		index = FLWIP_CONFIG_N_RX_DESC;
	}
#endif
#ifdef FPAR_GMACPS_2_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_2_BASEADDR) {
		index = 2 * FLWIP_CONFIG_N_RX_DESC;
	}
#endif
#ifdef FPAR_GMACPS_3_BASEADDR
	if (xgmacpsif->gmacps.Config.BaseAddress == FPAR_GMACPS_3_BASEADDR) {
		index = 3 * FLWIP_CONFIG_N_RX_DESC;
	}
#endif
	return index;
}

void process_sent_bds(fgmacpsif_s *xgmacpsif, FGmacPs_BdRing *txring)
{
	FGmacPs_Bd *txbdset;
	FGmacPs_Bd *curbdpntr;
	s32_t n_bds;
	FStatus status;
	s32_t n_pbufs_freed = 0;
	u32_t bdindex;
	struct pbuf *p;
	u32 *temp;
	u32_t index;

	index = get_base_index_txpbufsstorage (xgmacpsif);

	while (1) {
		/* obtain processed BD's */
		n_bds = FGmacPs_BdRingFromHwTx(txring, FLWIP_CONFIG_N_TX_DESC, &txbdset);
		if (n_bds == 0)  {
			return;
		}
		/* free the processed BD's */
		n_pbufs_freed = n_bds;
		curbdpntr = txbdset;
		while (n_pbufs_freed > 0) {
			bdindex = FGMACPS_BD_TO_INDEX(txring, curbdpntr);
			temp = (u32 *)curbdpntr;
			*temp = 0;
			temp++;
			if (bdindex == (FLWIP_CONFIG_N_TX_DESC - 1)) {
				*temp = 0xC0000000;
			} else {
				*temp = 0x80000000;
			}
			dsb();
			p = (struct pbuf *)tx_pbufs_storage[index + bdindex];
			if (p != NULL) {
				pbuf_free(p);
			}
			tx_pbufs_storage[index + bdindex] = 0;
			curbdpntr = FGmacPs_BdRingNext(txring, curbdpntr);
			n_pbufs_freed--;
			dsb();
		}

		status = FGmacPs_BdRingFree(txring, n_bds, txbdset);
		if (status != FMSH_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Failure while freeing in Tx Done ISR\r\n"));
		}
	}
	return;
}

void gmacps_send_handler(void *arg)
{
	struct fgmac_s *fgmac;
	fgmacpsif_s   *xgmacpsif;
	FGmacPs_BdRing *txringptr;
	u32_t regval;
#ifdef OS_IS_FREERTOS
	xInsideISR++;
#endif
	fgmac = (struct fgmac_s *)(arg);
	xgmacpsif = (fgmacpsif_s *)(fgmac->state);
	txringptr = &(FGmacPs_GetTxRing(&xgmacpsif->gmacps));
	regval = FGmacPs_ReadReg(xgmacpsif->gmacps.Config.BaseAddress, FGMACPS_TXSR_OFFSET);
	FGmacPs_WriteReg(xgmacpsif->gmacps.Config.BaseAddress,FGMACPS_TXSR_OFFSET, regval);

	/* If Transmit done interrupt is asserted, process completed BD's */
    
	process_sent_bds(xgmacpsif, txringptr);
    //delay_us(5);
#ifdef OS_IS_FREERTOS
	xInsideISR--;
#endif
}

FStatus gmacps_sgsend(fgmacpsif_s *xgmacpsif, struct pbuf *p)
{
	struct pbuf *q;
	s32_t n_pbufs;
	FGmacPs_Bd *txbdset, *txbd, *last_txbd = NULL;
	FGmacPs_Bd *temp_txbd;
	FStatus status;
	FGmacPs_BdRing *txring;
	u32_t bdindex;
	u32_t lev;
	u32_t index;
	u32_t max_fr_size;
        
        
    
        
        mfcpsr(lev);


	mtcpsr(lev | 0x000000C0);

	txring = &(FGmacPs_GetTxRing(&xgmacpsif->gmacps));

	index = get_base_index_txpbufsstorage (xgmacpsif);

	/* first count the number of pbufs */
	for (q = p, n_pbufs = 0; q != NULL; q = q->next)
		n_pbufs++;

	/* obtain as many BD's */
	status = FGmacPs_BdRingAlloc(txring, n_pbufs, &txbdset);
	if (status != FMSH_SUCCESS) {
		mtcpsr(lev);
		LWIP_DEBUGF(NETIF_DEBUG, ("sgsend: Error allocating TxBD\r\n"));
		return FMSH_FAILURE;
	}

	for(q = p, txbd = txbdset; q != NULL; q = q->next) {
		bdindex = FGMACPS_BD_TO_INDEX(txring, txbd);
		if (tx_pbufs_storage[index + bdindex] != 0) {
			mtcpsr(lev);
			LWIP_DEBUGF(NETIF_DEBUG, ("PBUFS not available\r\n"));
			return FMSH_FAILURE;
		}

		/* Send the data from the pbuf to the interface, one pbuf at a
		   time. The size of the data in each pbuf is kept in the ->len
		   variable. */
		if (xgmacpsif->gmacps.Config.IsCacheCoherent == 0) {
			Fmsh_DCacheFlushRange((UINTPTR)q->payload, (UINTPTR)q->len);
		}

		FGmacPs_BdSetAddressTx(txbd, (UINTPTR)q->payload);

#ifdef FMZQ_USE_JUMBO
		max_fr_size = MAX_FRAME_SIZE_JUMBO - 18;
#else
		max_fr_size = FGMACPS_MAX_FRAME_SIZE - 18;
#endif
		if (q->len > max_fr_size)
			FGmacPs_BdSetLength(txbd, max_fr_size & 0x3FFF);
		else
			FGmacPs_BdSetLength(txbd, q->len & 0x3FFF);

		tx_pbufs_storage[index + bdindex] = (UINTPTR)q;

		pbuf_ref(q);
		last_txbd = txbd;
		FGmacPs_BdClearLast(txbd);
		txbd = FGmacPs_BdRingNext(txring, txbd);
	}
	FGmacPs_BdSetLast(last_txbd);
	/* For fragmented packets, remember the 1st BD allocated for the 1st
	   packet fragment. The used bit for this BD should be cleared at the end
	   after clearing out used bits for other fragments. For packets without
	   just remember the allocated BD. */
	temp_txbd = txbdset;
	txbd = txbdset;
	txbd = FGmacPs_BdRingNext(txring, txbd);
	q = p->next;
	for(; q != NULL; q = q->next) {
		FGmacPs_BdClearTxUsed(txbd);
		dsb();
		txbd = FGmacPs_BdRingNext(txring, txbd);
	}
	FGmacPs_BdClearTxUsed(temp_txbd);
	dsb();

	status = FGmacPs_BdRingToHw(txring, n_pbufs, txbdset);
	if (status != FMSH_SUCCESS) {
		mtcpsr(lev);
		LWIP_DEBUGF(NETIF_DEBUG, ("sgsend: Error submitting TxBD\r\n"));
		return FMSH_FAILURE;
	}
	/* Start transmit */
	FGmacPs_WriteReg((xgmacpsif->gmacps).Config.BaseAddress,
	FGMACPS_NWCTRL_OFFSET,
	(FGmacPs_ReadReg((xgmacpsif->gmacps).Config.BaseAddress,
	FGMACPS_NWCTRL_OFFSET) | FGMACPS_NWCTRL_STARTTX_MASK));

	mtcpsr(lev);
	return status;
}

void setup_rx_bds(fgmacpsif_s *xgmacpsif, FGmacPs_BdRing *rxring)
{
	FGmacPs_Bd *rxbd;
	FStatus status;
	struct pbuf *p;
	u32_t freebds;
	u32_t bdindex;
	u32 *temp;
	u32_t index;

	index = get_base_index_rxpbufsstorage (xgmacpsif);

	freebds = FGmacPs_BdRingGetFreeCnt (rxring);
	while (freebds > 0) {
		freebds--;
#ifdef FMZQ_USE_JUMBO
		p = pbuf_alloc(PBUF_RAW, MAX_FRAME_SIZE_JUMBO, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, FGMACPS_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) {
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			fmsh_print("unable to alloc pbuf in recv_handler\r\n");
			return;
		}
		status = FGmacPs_BdRingAlloc(rxring, 1, &rxbd);
		if (status != FMSH_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("setup_rx_bds: Error allocating RxBD\r\n"));
			pbuf_free(p);
			return;
		}
		status = FGmacPs_BdRingToHw(rxring, 1, rxbd);
		if (status != FMSH_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error committing RxBD to hardware: "));
			if (status == FGMACPS_DMA_SG_LIST_ERROR) {
				LWIP_DEBUGF(NETIF_DEBUG, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FGmacPs_BdRingAlloc()\r\n"));
			}
			else {
				LWIP_DEBUGF(NETIF_DEBUG, ("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
			}

			pbuf_free(p);
			FGmacPs_BdRingUnAlloc(rxring, 1, rxbd);
			return;
		}
#ifdef FMZQ_USE_JUMBO
		if (xgmacpsif->gmacps.Config.IsCacheCoherent == 0) {
			Fmsh_DCacheInvalidateRange((UINTPTR)p->payload, (UINTPTR)MAX_FRAME_SIZE_JUMBO);
		}
#else
		if (xgmacpsif->gmacps.Config.IsCacheCoherent == 0) {
			Fmsh_DCacheInvalidateRange((UINTPTR)p->payload, (UINTPTR)FGMACPS_MAX_FRAME_SIZE);
		}
#endif
		bdindex = FGMACPS_BD_TO_INDEX(rxring, rxbd);
		temp = (u32 *)rxbd;
		if (bdindex == (FLWIP_CONFIG_N_RX_DESC - 1)) {
			*temp = 0x00000002;
		} else {
			*temp = 0;
		}
		temp++;
		*temp = 0;
		dsb();

		FGmacPs_BdSetAddressRx(rxbd, (UINTPTR)p->payload);
		rx_pbufs_storage[index + bdindex] = (UINTPTR)p;
	}
}

void gmacps_recv_handler(void *arg)
{
	struct pbuf *p;
	FGmacPs_Bd *rxbdset, *curbdptr;
	struct fgmac_s *fgmac;
	fgmacpsif_s *xgmacpsif;
	FGmacPs_BdRing *rxring;
	volatile s32_t bd_processed;
	s32_t rx_bytes, k;
	u32_t bdindex;
	u32_t regval;
	u32_t index;
	u32_t gigeversion;

	fgmac = (struct fgmac_s *)(arg);
	xgmacpsif = (fgmacpsif_s *)(fgmac->state);
	rxring = &FGmacPs_GetRxRing(&xgmacpsif->gmacps);
        
#ifdef OS_IS_FREERTOS
	xInsideISR++;
#endif

	gigeversion = ((*(volatile u32 *)(xgmacpsif->gmacps.Config.BaseAddress + 0xFC)) >> 16) & 0xFFF;
	index = get_base_index_rxpbufsstorage (xgmacpsif);
	/*
	 * If Reception done interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
    
	regval = FGmacPs_ReadReg(xgmacpsif->gmacps.Config.BaseAddress, FGMACPS_RXSR_OFFSET);
	FGmacPs_WriteReg(xgmacpsif->gmacps.Config.BaseAddress, FGMACPS_RXSR_OFFSET, regval);
	if (gigeversion <= 2) {
			resetrx_on_no_rxdata(xgmacpsif);
	}

	while(1) {
    //if(regval & 0x2) {
    
		bd_processed = FGmacPs_BdRingFromHwRx(rxring, FLWIP_CONFIG_N_RX_DESC, &rxbdset);
		if (bd_processed <= 0) {
			break;
		}

		for (k = 0, curbdptr=rxbdset; k < bd_processed; k++) {

			bdindex = FGMACPS_BD_TO_INDEX(rxring, curbdptr);
			p = (struct pbuf *)rx_pbufs_storage[index + bdindex];

			/*
			 * Adjust the buffer size to the actual number of bytes received.
			 */
#ifdef FMZQ_USE_JUMBO
			rx_bytes = FGmacPs_GetRxFrameSize(&xgmacpsif->gmacps, curbdptr);
#else
			rx_bytes = FGmacPs_BdGetLength(curbdptr);
#endif
			pbuf_realloc(p, rx_bytes);

			/* Invalidate RX frame before queuing to handle
			 * L1 cache prefetch conditions on any architecture.
			 */
			///Fmsh_DCacheInvalidateRange((UINTPTR)p->payload, rx_bytes);

			/* store it in the receive queue,
			 * where it'll be processed by a different handler
			 */
			if (pq_enqueue(xgmacpsif->recv_q, (void*)p) < 0) {
#if LINK_STATS
				lwip_stats.link.memerr++;
				lwip_stats.link.drop++;
#endif
				pbuf_free(p);
			}

			curbdptr = FGmacPs_BdRingNext( rxring, curbdptr);
            bd_irq_count_1++;
		}
		/* free up the BD's */
        
		FGmacPs_BdRingFree(rxring, bd_processed, rxbdset);
		setup_rx_bds(xgmacpsif, rxring);
	}
#if !NO_SYS
	sys_sem_signal(&fgmac->sem_rx_data_available);
        ///sys_sem_signal(&mac_test);
		//xQueueSendToBackFromISR(xQueue_l,&bd_free_count_s,NULL);

#endif
#ifdef OS_IS_FREERTOS
	xInsideISR--;
#endif
    
	return;
}

void clean_dma_txdescs(struct fgmac_s *fgmac)
{
	FGmacPs_Bd bdtemplate;
	FGmacPs_BdRing *txringptr;
	fgmacpsif_s *xgmacpsif = (fgmacpsif_s *)(fgmac->state);

	txringptr = &FGmacPs_GetTxRing(&xgmacpsif->gmacps);

	FGmacPs_BdClear(&bdtemplate);
	FGmacPs_BdSetStatus(&bdtemplate, FGMACPS_TXBUF_USED_MASK);

	/*
	 * Create the TxBD ring
	 */
	FGmacPs_BdRingCreate(txringptr, (UINTPTR) xgmacpsif->tx_bdspace,
			(UINTPTR) xgmacpsif->tx_bdspace, BD_ALIGNMENT,
				 FLWIP_CONFIG_N_TX_DESC);
	FGmacPs_BdRingClone(txringptr, &bdtemplate, FGMACPS_SEND);
}

FStatus init_dma(struct fgmac_s *fgmac)
{
	FGmacPs_Bd bdtemplate;
	FGmacPs_BdRing *rxringptr, *txringptr;
	FGmacPs_Bd *rxbd;
	struct pbuf *p;
	FStatus status;
	s32_t i;
	u32_t bdindex;
	volatile UINTPTR tempaddress;
	u32_t index;
	u32_t gigeversion;
	FGmacPs_Bd *bdtxterminate;
	FGmacPs_Bd *bdrxterminate;
	u32 *temp;

       
        
	fgmacpsif_s *xgmacpsif = (fgmacpsif_s *)(fgmac->state);
	struct ftopology_t *ftopologyp = &ftopology[fgmac->topology_index];

	index = get_base_index_rxpbufsstorage (xgmacpsif);
	gigeversion = ((*(volatile u32 *)(xgmacpsif->gmacps.Config.BaseAddress + 0xFC)) >> 16) & 0xFFF;
	/*
	 * The BDs need to be allocated in uncached memory. Hence the 1 MB
	 * address range allocated for Bd_Space is made uncached
	 * by setting appropriate attributes in the translation table.
	 * The Bd_Space is aligned to 1MB and has a size of 1 MB. This ensures
	 * a reserved uncached area used only for BDs.
	 */
	if (bd_space_attr_set == 0) {
        #if defined __aarch64__
          Fmsh_SetTlbAttributes((u64)bd_space, NORM_NONCACHE | INNER_SHAREABLE);
        #else
          Fmsh_SetAttribute((s32_t)bd_space, REGION_1M, 7, NORM_NSHARED_NCACHE | PRIV_RW_USER_RW);
         // addr, attr
        #endif
		bd_space_attr_set = 1;
	}

	rxringptr = &FGmacPs_GetRxRing(&xgmacpsif->gmacps);
	txringptr = &FGmacPs_GetTxRing(&xgmacpsif->gmacps);
	LWIP_DEBUGF(NETIF_DEBUG, ("rxringptr: 0x%08x\r\n", rxringptr));
	LWIP_DEBUGF(NETIF_DEBUG, ("txringptr: 0x%08x\r\n", txringptr));

	/* Allocate 64k for Rx and Tx bds each to take care of extreme cases */
	tempaddress = (UINTPTR)&(bd_space[bd_space_index]);
	xgmacpsif->rx_bdspace = (void *)tempaddress;
	bd_space_index += 0x10000;
	tempaddress = (UINTPTR)&(bd_space[bd_space_index]);
	xgmacpsif->tx_bdspace = (void *)tempaddress;
	bd_space_index += 0x10000;
	if (gigeversion > 2) {
		tempaddress = (UINTPTR)&(bd_space[bd_space_index]);
		bdrxterminate = (FGmacPs_Bd *)tempaddress;
		bd_space_index += 0x10000;
		tempaddress = (UINTPTR)&(bd_space[(bd_space_index)]);
		bdtxterminate = (FGmacPs_Bd *)tempaddress;
		bd_space_index += 0x10000;
	}

	LWIP_DEBUGF(NETIF_DEBUG, ("rx_bdspace: %p \r\n", xgmacpsif->rx_bdspace));
	LWIP_DEBUGF(NETIF_DEBUG, ("tx_bdspace: %p \r\n", xgmacpsif->tx_bdspace));

	if (!xgmacpsif->rx_bdspace || !xgmacpsif->tx_bdspace) {
		fmsh_print("%s@%d: Error: Unable to allocate memory for TX/RX buffer descriptors",
				__FILE__, __LINE__);
		return ERR_IF;
	}

	/*
	 * Setup RxBD space.
	 *
	 * Setup a BD template for the Rx channel. This template will be copied to
	 * every RxBD. We will not have to explicitly set these again.
	 */
	FGmacPs_BdClear(&bdtemplate);

	/*
	 * Create the RxBD ring
	 */

	status = FGmacPs_BdRingCreate(rxringptr, (UINTPTR) xgmacpsif->rx_bdspace,
				(UINTPTR) xgmacpsif->rx_bdspace, BD_ALIGNMENT,
				     FLWIP_CONFIG_N_RX_DESC);

	if (status != FMSH_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error setting up RxBD space\r\n"));
		return ERR_IF;
	}

	status = FGmacPs_BdRingClone(rxringptr, &bdtemplate, FGMACPS_RECV);
	if (status != FMSH_SUCCESS) {
		LWIP_DEBUGF(NETIF_DEBUG, ("Error initializing RxBD space\r\n"));
		return ERR_IF;
	}

	FGmacPs_BdClear(&bdtemplate);
	FGmacPs_BdSetStatus(&bdtemplate, FGMACPS_TXBUF_USED_MASK);
	/*
	 * Create the TxBD ring
	 */
	status = FGmacPs_BdRingCreate(txringptr, (UINTPTR) xgmacpsif->tx_bdspace,
				(UINTPTR) xgmacpsif->tx_bdspace, BD_ALIGNMENT,
				     FLWIP_CONFIG_N_TX_DESC);

	if (status != FMSH_SUCCESS) {
		return ERR_IF;
	}

	/* We reuse the bd template, as the same one will work for both rx and tx. */
	status = FGmacPs_BdRingClone(txringptr, &bdtemplate, FGMACPS_SEND);
	if (status != FMSH_SUCCESS) {
		return ERR_IF;
	}

	/*
	 * Allocate RX descriptors, 1 RxBD at a time.
	 */
	for (i = 0; i < FLWIP_CONFIG_N_RX_DESC; i++) {
#ifdef FMZQ_USE_JUMBO
		p = pbuf_alloc(PBUF_RAW, MAX_FRAME_SIZE_JUMBO, PBUF_POOL);
#else
		p = pbuf_alloc(PBUF_RAW, FGMACPS_MAX_FRAME_SIZE, PBUF_POOL);
#endif
		if (!p) {
#if LINK_STATS
			lwip_stats.link.memerr++;
			lwip_stats.link.drop++;
#endif
			fmsh_print("unable to alloc pbuf in init_dma\r\n");
			return ERR_IF;
		}
		status = FGmacPs_BdRingAlloc(rxringptr, 1, &rxbd);
		if (status != FMSH_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("init_dma: Error allocating RxBD\r\n"));
			pbuf_free(p);
			return ERR_IF;
		}
		/* Enqueue to HW */
		status = FGmacPs_BdRingToHw(rxringptr, 1, rxbd);
		if (status != FMSH_SUCCESS) {
			LWIP_DEBUGF(NETIF_DEBUG, ("Error: committing RxBD to HW\r\n"));
			pbuf_free(p);
			FGmacPs_BdRingUnAlloc(rxringptr, 1, rxbd);
			return ERR_IF;
		}

		bdindex = FGMACPS_BD_TO_INDEX(rxringptr, rxbd);
		temp = (u32 *)rxbd;
		*temp = 0;
		if (bdindex == (FLWIP_CONFIG_N_RX_DESC - 1)) {
			*temp = 0x00000002;
		}
		temp++;
		*temp = 0;
		dsb();
#ifdef FMZQ_USE_JUMBO
		if (xgmacpsif->gmacps.Config.IsCacheCoherent == 0) {
			Fmsh_DCacheInvalidateRange((UINTPTR)p->payload, (UINTPTR)MAX_FRAME_SIZE_JUMBO);
		}
#else
		if (xgmacpsif->gmacps.Config.IsCacheCoherent == 0) {
			Fmsh_DCacheInvalidateRange((UINTPTR)p->payload, (UINTPTR)FGMACPS_MAX_FRAME_SIZE);
		}
#endif
		FGmacPs_BdSetAddressRx(rxbd, (UINTPTR)p->payload);

		rx_pbufs_storage[index + bdindex] = (UINTPTR)p;
	}
	FGmacPs_SetQueuePtr(&(xgmacpsif->gmacps), xgmacpsif->gmacps.RxBdRing.BaseBdAddr, 0, FGMACPS_RECV);
	if (gigeversion > 2) {
		FGmacPs_SetQueuePtr(&(xgmacpsif->gmacps), xgmacpsif->gmacps.TxBdRing.BaseBdAddr, 1, FGMACPS_SEND);
	}else {
		FGmacPs_SetQueuePtr(&(xgmacpsif->gmacps), xgmacpsif->gmacps.TxBdRing.BaseBdAddr, 0, FGMACPS_SEND);
	}
	if (gigeversion > 2)
	{
		/*
		 * This version of GEM supports priority queuing and the current
		 * driver is using tx priority queue 1 and normal rx queue for
		 * packet transmit and receive. The below code ensure that the
		 * other queue pointers are parked to known state for avoiding
		 * the controller to malfunction by fetching the descriptors
		 * from these queues.
		 */
		FGmacPs_BdClear(bdrxterminate);
		FGmacPs_BdSetAddressRx(bdrxterminate, (FGMACPS_RXBUF_NEW_MASK |
						FGMACPS_RXBUF_WRAP_MASK));
		FGmacPs_Out32((UINTPTR)bdrxterminate,
                              (xgmacpsif->gmacps.Config.BaseAddress + FGMACPS_RXQ1BASE_OFFSET));
		FGmacPs_BdClear(bdtxterminate);
		FGmacPs_BdSetStatus(bdtxterminate, (FGMACPS_TXBUF_USED_MASK |
						FGMACPS_TXBUF_WRAP_MASK));
		FGmacPs_Out32((UINTPTR)bdtxterminate, 
                              (xgmacpsif->gmacps.Config.BaseAddress + FGMACPS_TXQBASE_OFFSET));
	}


	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
        

        
        
	FGicPs_registerInt(&IntcInstance, ftopologyp->scugic_gmac_intr,
				(FMSH_ExceptionHandler)FGmacPs_IntrHandler,
						(void *)&xgmacpsif->gmacps);
	/*
	 * Enable the interrupt for gmacps.
	 */
	FGicPs_Enable(&IntcInstance, (u32) ftopologyp->scugic_gmac_intr);
	gmac_intr_num = (u32) ftopologyp->scugic_gmac_intr;
	return 0;
}

/*
 * resetrx_on_no_rxdata():
 *
 * It is called at regular intervals through the API xemacpsif_resetrx_on_no_rxdata
 * called by the user.
 * The gmacps has a HW bug (SI# 692601) on the Rx path for heavy Rx traffic.
 * Under heavy Rx traffic because of the HW bug there are times when the Rx path
 * becomes unresponsive. The workaround for it is to check for the Rx path for
 * traffic (by reading the stats registers regularly). If the stats register
 * does not increment for sometime (proving no Rx traffic), the function resets
 * the Rx data path.
 *
 */

void resetrx_on_no_rxdata(fgmacpsif_s *xgmacpsif)
{
	u32_t regctrl;
	u32_t tempcntr;
	u32_t gigeversion;

	gigeversion = ((*(volatile u32 *)(xgmacpsif->gmacps.Config.BaseAddress + 0xFC)) >> 16) & 0xFFF;
	if (gigeversion == 2) {
          tempcntr = FGmacPs_ReadReg(xgmacpsif->gmacps.Config.BaseAddress, FGMACPS_RXCNT_OFFSET);
          if ((!tempcntr) && (!(xgmacpsif->last_rx_frms_cntr))) {
              regctrl = FGmacPs_ReadReg(xgmacpsif->gmacps.Config.BaseAddress,
                              FGMACPS_NWCTRL_OFFSET);
              regctrl &= (~FGMACPS_NWCTRL_RXEN_MASK);
              FGmacPs_WriteReg(xgmacpsif->gmacps.Config.BaseAddress,
                              FGMACPS_NWCTRL_OFFSET, regctrl);
              regctrl = FGmacPs_ReadReg(xgmacpsif->gmacps.Config.BaseAddress, FGMACPS_NWCTRL_OFFSET);
              regctrl |= (FGMACPS_NWCTRL_RXEN_MASK);
              FGmacPs_WriteReg(xgmacpsif->gmacps.Config.BaseAddress, FGMACPS_NWCTRL_OFFSET, regctrl);
          }
          xgmacpsif->last_rx_frms_cntr = tempcntr;
	}
}

void free_txrx_pbufs(fgmacpsif_s *xgmacpsif)
{
	s32_t index;
	s32_t index1;
	struct pbuf *p;

	index1 = get_base_index_txpbufsstorage (xgmacpsif);

	for (index = index1; index < (index1 + FLWIP_CONFIG_N_TX_DESC); index++) {
            if (tx_pbufs_storage[index] != 0) {
                    p = (struct pbuf *)tx_pbufs_storage[index];
                    pbuf_free(p);
                    tx_pbufs_storage[index] = 0;
            }
	}

	for (index = index1; index < (index1 + FLWIP_CONFIG_N_TX_DESC); index++) {
            p = (struct pbuf *)rx_pbufs_storage[index];
            pbuf_free(p);

	}
}

void free_onlytx_pbufs(fgmacpsif_s *xgmacpsif)
{
	s32_t index;
	s32_t index1;
	struct pbuf *p;

	index1 = get_base_index_txpbufsstorage (xgmacpsif);
	for (index = index1; index < (index1 + FLWIP_CONFIG_N_TX_DESC); index++) {
            if (tx_pbufs_storage[index] != 0) {
                p = (struct pbuf *)tx_pbufs_storage[index];
                pbuf_free(p);
                tx_pbufs_storage[index] = 0;
		}
	}
}

/* reset Tx and Rx DMA pointers after FGmacPs_Stop */
void reset_dma(struct fgmac_s *fgmac)
{
	u8 txqueuenum;
	u32_t gigeversion;
	fgmacpsif_s *xgmacpsif = (fgmacpsif_s *)(fgmac->state);
	FGmacPs_BdRing *txringptr = &FGmacPs_GetTxRing(&xgmacpsif->gmacps);
	FGmacPs_BdRing *rxringptr = &FGmacPs_GetRxRing(&xgmacpsif->gmacps);

	FGmacPs_BdRingPtrReset(txringptr, xgmacpsif->tx_bdspace);
	FGmacPs_BdRingPtrReset(rxringptr, xgmacpsif->rx_bdspace);

	gigeversion = ((*(volatile u32 *)(xgmacpsif->gmacps.Config.BaseAddress + 0xFC)) >> 16) & 0xFFF;
	if (gigeversion > 2) {
		txqueuenum = 1;
	} else {
		txqueuenum = 0;
	}

	FGmacPs_SetQueuePtr(&(xgmacpsif->gmacps), xgmacpsif->gmacps.RxBdRing.BaseBdAddr, 0, FGMACPS_RECV);
	FGmacPs_SetQueuePtr(&(xgmacpsif->gmacps), xgmacpsif->gmacps.TxBdRing.BaseBdAddr, txqueuenum, FGMACPS_SEND);
}

void gmac_disable_intr(void)
{       
	FGicPs_Disable(&IntcInstance, gmac_intr_num);
}

void gmac_enable_intr(void)
{       
	FGicPs_Enable(&IntcInstance, gmac_intr_num);
}
