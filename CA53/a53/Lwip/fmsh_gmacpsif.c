

#include <stdio.h>
#include <string.h>

#include "fmsh_psu_parameters.h"
#include "fmsh_parameters.h"
#include "lwipopts.h"
#include "fmsh_lwipconfig.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/igmp.h"

#include "netif/etharp.h"
#include "fmsh_gmacpsif.h"
#include "fmsh_pqueue.h"

#include "fmsh_gic.h"
#include "fmsh_gmac.h"
#include "fmsh_gmac_status.h"
#include "fmsh_gmac_mdio.h"
#include "fmsh_gic_hw.h"
#include "fmsh_common.h"
#include "fmsh_gmac_hw.h"
#include "ftopology.h"


/* Define those to better describe your network interface. */
#define IFNAME0 'F'
#define IFNAME1 'M'


FGmacPs_Config *mac_config;
struct netif *NetIf;

///////

/************************** Constant Definitions *****************************/

#define JUMBO_FRAME_SIZE	10240
#define FRAME_HDR_SIZE		18
#define RXBD_CNT                32	/* Number of RxBDs to use */
#define TXBD_CNT                32	/* Number of TxBDs to use */


/************************** Variable Definitions *****************************/

volatile s32 DeviceErrors;	/* Number of errors detected in the device */



FGmacPs GmacPsInstance;
FGmacPs_PhyConfig PhyCfg = {
    .phy_device=LWIP_PHY_DEVICE1, //PHY_KSZ9031RNX   PHY_88E1116R  PHY_88E1111
    .auto_detect_ad_en=LWIP_AUTO_PHY_DET,
    .phy_address=LWIP_PHY_ADDR1,
    .auto_nag_en=LWIP_AUTO_NAG_EN,
    .is_fixlink=1,
    .speed=speed_1000,
};

int fmsh_gmac_device_initial();
int fmsh_gmac_phy_initial();
int fmsh_gmac_gic_setup();


int ftopology_n_gmacs = 4;
int
ftopology_find_index(unsigned base)
{
	int i;

	for (i = 0; i < ftopology_n_gmacs; i++) {
		if (ftopology[i].gmac_baseaddr == base)
			return i;
	}

	return -1;
}


/*
 * this function is always called with interrupts off
 * this function also assumes that there are available BD's
 */
static err_t _unbuffered_low_level_output(fgmacpsif_s *fgmacpsif,struct pbuf *p)
{
	FStatus status = 0;


	status = gmacps_sgsend(fgmacpsif, p);
	if (status != FMSH_SUCCESS) {
#if LINK_STATS
	lwip_stats.link.drop++;
#endif
	}


#if LINK_STATS
	lwip_stats.link.xmit++;
#endif /* LINK_STATS */

	return ERR_OK;

}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{

    err_t err;
    s32_t freecnt;
    FGmacPs_BdRing *txring;
    struct pbuf *q;

	struct fgmac_s *fgmac = (struct fgmac_s *)(netif->state);
	fgmacpsif_s *fgmacpsif = (fgmacpsif_s *)(fgmac->state);


	/* check if space is available to send */
    freecnt = is_tx_space_available(fgmacpsif);
    if (freecnt <= 5) {
	txring = &(FGmacPs_GetTxRing(&fgmacpsif->gmacps));
		process_sent_bds(fgmacpsif, txring);
	}

    if (is_tx_space_available(fgmacpsif)) {
        for (q = p; q != NULL; q = q->next) 
        {
            Fmsh_DCacheFlushRange((uintptr_t)q->payload, q->len);
        }
		_unbuffered_low_level_output(fgmacpsif, p);
		err = ERR_OK;
	} else {
#if LINK_STATS
		lwip_stats.link.drop++;
#endif
		fmsh_print("pack dropped, no space\r\n");
		err = ERR_MEM;
	}

	return err;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
static struct pbuf * low_level_input(struct netif *netif)
{
    struct fgmac_s *fgmac = (struct fgmac_s *)(netif->state);
	fgmacpsif_s *fgmacpsif = (fgmacpsif_s *)(fgmac->state);
	struct pbuf *p;
    struct pbuf *q;

	/* see if there is data to process */
	if (pq_qlength(fgmacpsif->recv_q) == 0)
		return NULL;

	/* return one packet from receive q */
	p = (struct pbuf *)pq_dequeue(fgmacpsif->recv_q);
    for (q = p; q != NULL; q = q->next) 
    {
        Fmsh_DCacheInvalidateRange((uintptr_t)q->payload, q->len);
        Fmsh_DCacheFlushRange((uintptr_t)q->payload, q->len);
    }
	return p;
}

/*
 * fgmacpsif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t fgmacpsif_output(struct netif *netif, struct pbuf *p,
		const ip_addr_t *ipaddr)
{
	/* resolve hardware address, then send (or queue) packet */
	return etharp_output(netif, p, ipaddr);
}

/*
 * fmsh_gmacpsif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 * Returns the number of packets read (max 1 packet on success,
 * 0 if there are no packets)
 *
 */

s32_t fmsh_gmacpsif_input(struct netif *netif)
{
	struct eth_hdr *ethhdr;
	struct pbuf *p;
    int qlength;
    struct fgmac_s *fgmac = (struct fgmac_s *)(netif->state);
	fgmacpsif_s *fgmacpsif = (fgmacpsif_s *)(fgmac->state);

    qlength = pq_qlength(fgmacpsif->recv_q);
	while (qlength-->0) {
        /* move received packet into a new pbuf */

        p = low_level_input(netif);


        /* no packet could be read, silently ignore this */
        if (p == NULL) {
                return 0;
        }
        
        /* points to packet payload, which starts with an Ethernet header */
        ethhdr = p->payload;

#if LINK_STATS
        lwip_stats.link.recv++;
#endif /* LINK_STATS */

        switch (htons(ethhdr->type)) {
                /* IP or ARP packet? */
                case ETHTYPE_IP:
                case ETHTYPE_ARP:

                /* full packet send to tcpip_thread to process */
                if (netif->input(p, netif) != ERR_OK) {
                    LWIP_DEBUGF(NETIF_DEBUG, ("fmsh_gmacpsif_input: IP input error\r\n"));
                    pbuf_free(p);
                    p = NULL;
                }
                break;

                default:
                    pbuf_free(p);
                    p = NULL;
                    break;
        }
	} 

	return 1;
}



static err_t low_level_init(struct netif *netif)
{
	UINTPTR mac_address = (UINTPTR)(netif->state);
	struct fgmac_s *fgmac;
	fgmacpsif_s *fgmacpsif;
	u32 dmacrreg;

	s32_t status = FMSH_SUCCESS;

	NetIf = netif;

	fgmacpsif = mem_malloc(sizeof *fgmacpsif);
	if (fgmacpsif == NULL) {
		LWIP_DEBUGF(NETIF_DEBUG, ("fgmacpsif_init: out of memory\r\n"));
		return ERR_MEM;
	}

	fgmac = mem_malloc(sizeof *fgmac);
	if (fgmac == NULL) {
		LWIP_DEBUGF(NETIF_DEBUG, ("fgmacpsif_init: out of memory\r\n"));
		return ERR_MEM;
	}

	fgmac->state = (void *)fgmacpsif;
	fgmac->topology_index = ftopology_find_index(mac_address);
	fgmac->type = fgmac_type_gmacps;

	fgmacpsif->send_q = NULL;
	fgmacpsif->recv_q = pq_create_queue();
	if (!fgmacpsif->recv_q)
		return ERR_MEM;

	/* maximum transfer unit */
	netif->mtu = FGMACPS_MTU - FGMACPS_HDR_SIZE;

	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
								NETIF_FLAG_LINK_UP;

#if !NO_SYS
    sys_sem_new(&fgmac->sem_rx_data_available, 0);
#endif
    
	/* obtain config of this emac */
	mac_config = (FGmacPs_Config *)FGmacPs_LookupConfig(GMAC_SELECT_ID);  

	status = FGmacPs_CfgInitialize(&fgmacpsif->gmacps, mac_config,
						mac_config->BaseAddress);
	if (status != FMSH_SUCCESS) {
		fmsh_print("In %s:gmacps Configuration Failed....\r\n", __func__);
	}

	/* initialize the mac */
        fmsh_gmac_device_initial(fgmac,netif);
	//nit_emacps(fgmacpsif, netif);

        FGmacPs_PhyConfig *PhyCfgPtr;
        PhyCfgPtr = &PhyCfg;
        fmsh_gmac_phy_initial(&fgmacpsif->gmacps, PhyCfgPtr);
        
	dmacrreg = FGmacPs_ReadReg(fgmacpsif->gmacps.Config.BaseAddress,FGMACPS_DMACR_OFFSET);
	dmacrreg = dmacrreg | (0x00000010);
	FGmacPs_WriteReg(fgmacpsif->gmacps.Config.BaseAddress,FGMACPS_DMACR_OFFSET, dmacrreg);

        status = fmsh_gmac_gic_setup(&fgmacpsif->gmacps);
	//setup_isr(fgmac);
	init_dma(fgmac);
	FGmacPs_Start(&fgmacpsif->gmacps);

        
	/* replace the state in netif (currently the emac baseaddress)
	 * with the mac instance pointer.
	 */
	netif->state = (void *)fgmac;

	return ERR_OK;
 
}




/************************* irq handler function ******************************/


/****************************************************************************/
/**
*
* This the Transmit handler callback function and will increment a shared
* counter that can be shared by the main thread of operation.
*
* @param	Callback is the pointer to the instance of the GmacPs device.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void gmac_interrupt_handler(void)
{
	//printf("1\n");
    FGmacPs_IntrHandler(&GmacPsInstance);
}

/****************************************************************************/
/**
*
* This is the Error handler callback function and this function increments
* the error counter so that the main thread knows the number of errors.
*
* @param	Callback is the callback function for the driver. This
*		parameter is not used in this example.
* @param	Direction is passed in from the driver specifying which
*		direction error has occurred.
* @param	ErrorWord is the status register value passed in.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void FGmacPsErrorHandler(void *Callback, u8 Direction, u32 ErrorWord)
{
#if GMAC_DEBUG_RESET_ON_ERR
    FGmacPs *InstancePtr = (FGmacPs *) Callback;
#endif    
    /*
     * Increment the counter so that main thread knows something
     * happened. Reset the device and reallocate resources ...
     */
    DeviceErrors++;

    switch (Direction) {
    case FGMACPS_RECV:
        if (ErrorWord & FGMACPS_RXSR_HRESPNOK_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Receive DMA error");
        }
        if (ErrorWord & FGMACPS_RXSR_RXOVR_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Receive over run");
        }
        if (ErrorWord & FGMACPS_RXSR_BUFFNA_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Receive buffer not available");
        }
    break;
    case FGMACPS_SEND:
        if (ErrorWord & FGMACPS_TXSR_HRESPNOK_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Transmit DMA error");
        }
        if (ErrorWord & FGMACPS_TXSR_URUN_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Transmit under run");
        }
        if (ErrorWord & FGMACPS_TXSR_BUFEXH_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Transmit buffer exhausted");
        }
        if (ErrorWord & FGMACPS_TXSR_RXOVR_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Transmit retry excessed limits");
        }
        if (ErrorWord & FGMACPS_TXSR_FRAMERX_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Transmit collision");
        }
        if (ErrorWord & FGMACPS_TXSR_USEDREAD_MASK) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Transmit buffer not available");
        }
    break;
    }
    /*
     * Bypassing the reset functionality as the default tx status for q0 is
     * USED BIT READ. so, the first interrupt will be tx used bit and it resets
     * the core always.
     */
#if GMAC_DEBUG_RESET_ON_ERR
    GmacPsResetDevice(InstancePtr);
#endif
    
}



/*************************** initial function ********************************/

/*****************************************************************************/
/**
* Initialize a specific FGmacPs instance/driver. 
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return
* - FMSH_SUCCESS if initialization was successful
*
******************************************************************************/

int fmsh_gmac_device_initial(struct fgmac_s *fgmac,struct netif *netif)
{
    int Status;
    FGmacPs_Config *ConfigPtr;
    FGmacPs *InstancePtr = &(((fgmacpsif_s*)(fgmac->state))->gmacps);
    ConfigPtr = FGmacPs_LookupConfig(GMAC_SELECT_ID);

    Status = FGmacPs_CfgInitialize(InstancePtr, ConfigPtr,
 					ConfigPtr->BaseAddress);
    
    if (Status != FMSH_SUCCESS) {
		GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error in cfg initialize");
		return FMSH_FAILURE;
    }
    
    /* Enable jumbo frames for  */
    FGmacPs_SetOptions(InstancePtr, FGMACPS_JUMBO_ENABLE_OPTION);
    //FGmacPsClkSetup(InstancePtr, GmacPsIntrId);

    /*
     * Set the MAC address
     */
    Status = FGmacPs_SetMacAddress(InstancePtr, netif->hwaddr, 1);
    if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error setting MAC address");
        return FMSH_FAILURE;
    }
    /*
     * Setup callbacks
     */
    Status = FGmacPs_SetHandler(InstancePtr,
			    FGMACPS_HANDLER_DMASEND,
			    (void *) gmacps_send_handler,
			    fgmac);
    Status |= FGmacPs_SetHandler(InstancePtr,
			    FGMACPS_HANDLER_DMARECV,
			    (void *) gmacps_recv_handler,
			    fgmac);
    Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_ERROR,
			    (void *) FGmacPsErrorHandler,
			    fgmac);
    if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error assigning handlers");
        return FMSH_FAILURE;
    }
    
    return Status;
}


/*****************************************************************************/
/**
* Initialize phy for a specific FGmacPs instance. 
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return
* - FMSH_SUCCESS if initialization was successful
*
******************************************************************************/

int fmsh_gmac_phy_initial(FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    int speed = speed_1000;
    u16 PhyAddr = 4;
    int Status;
    
    if (!PhyCfgPtr->is_fixlink)
    {
    
      FGmacPs_SetMdioDivisor(InstancePtr, MDC_DIV_224);
      delay_ms(1000);
      /* detect phy */
      if (PhyCfgPtr->auto_detect_ad_en == 1){
          PhyAddr = FGmacPs_PHYDetect(InstancePtr);
          PhyCfgPtr->phy_address = PhyAddr;
      }
      PhyCfgPtr->speed = InstancePtr->Config.Speed;
      
      /* set phy address & phy device */
      switch(InstancePtr->Config.BaseAddress)
      {
        case FPS_GMAC0_BASEADDR: {
            PhyCfgPtr->phy_address = 0; 
            PhyCfgPtr->phy_device = PHY_YT8521; 
            break;}
        case FPS_GMAC1_BASEADDR: {
            PhyCfgPtr->phy_address = 1; 
            PhyCfgPtr->phy_device = PHY_YT8521; 
            break;}
        case FPS_GMAC2_BASEADDR: {
            PhyCfgPtr->phy_address = 4; 
            PhyCfgPtr->phy_device = PHY_YT8521; 
            break;}
        case FPS_GMAC3_BASEADDR: {
            PhyCfgPtr->phy_address = 6; 
            PhyCfgPtr->phy_device = PHY_YT8521; 
            break;}
      }
      
      /* operate phy Init */
      Status = FGmacPs_PHYInit(InstancePtr, PhyCfgPtr);
      if (Status != FMSH_SUCCESS)
      {
          GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"PHY init fail\r\n");
      }
      else
      {
          GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"PHY init success, Address = %x, type = %d\r\n", 
                         PhyCfgPtr->phy_address, PhyCfgPtr->phy_device);
      }
    }
    /* get phy operating speed if autoneg is on */
    if (PhyCfgPtr->auto_nag_en == 1){
        speed = PhyCfgPtr->speed;
    }else{
        speed = PhyCfgPtr->speed;
    }
    
    /* set operating speed */
    FGmacPs_SetOperatingSpeed(InstancePtr, speed);
    
    return FMSH_SUCCESS; 
}


/*****************************************************************************/
/**
* Initialize a specific FGmacPs Gic instance. 
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return
* - FMSH_SUCCESS if initialization was successful
*
******************************************************************************/
int fmsh_gmac_gic_setup(FGmacPs *InstancePtr)
{
    u32 int_id = 0x59U;
    u32 Status;
    switch (InstancePtr->Config.DeviceId)
    {
        case FPAR_GMACPS_0_DEVICE_ID: int_id = GEM0_INT_ID; break;
        case FPAR_GMACPS_1_DEVICE_ID: int_id = GEM1_INT_ID; break;
        case FPAR_GMACPS_2_DEVICE_ID: int_id = GEM2_INT_ID; break;
        case FPAR_GMACPS_3_DEVICE_ID: int_id = GEM3_INT_ID; break;
        default: int_id = GEM1_INT_ID; break;
    }
    
    /*Status = FGicPs_SetupInterruptSystem(&IntcInstance);
    if(Status != GIC_SUCCESS)
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "FGicPs_SetupInterruptSystem fail\r\n");
    */
    FGicPs_InterruptMaptoCpu (&IntcInstance, GICMAP_CPUID0, int_id);
    FGicPs_Connect(&IntcInstance, int_id, (FMSH_InterruptHandler)gmac_interrupt_handler, 0);
    //FMSH_ExceptionRegisterHandler(FMSH_EXCEPTION_ID_IRQ_INT, (FMSH_ExceptionHandler)FGicPs_InterruptHandler_IRQ, &IntcInstance);
    FGicPs_Enable(&IntcInstance, int_id);
    
    
    //timer
    
    return 0;
}


/*****************************************************************************/
/**
* Stop a specific FGmacPs Gic instance. 
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return
* - FMSH_SUCCESS if initialization was successful
*
******************************************************************************/
int fmsh_gmac_gic_stop(FGmacPs *InstancePtr)
{
    u32 int_id;
    
    switch (InstancePtr->Config.DeviceId)
    {
        case FPAR_GMACPS_0_DEVICE_ID: 
          int_id = GEM0_INT_ID;
        case FPAR_GMACPS_1_DEVICE_ID: 
          int_id = GEM1_INT_ID;
        case FPAR_GMACPS_2_DEVICE_ID: 
          int_id = GEM2_INT_ID;
        case FPAR_GMACPS_3_DEVICE_ID: 
          int_id = GEM3_INT_ID;
    }

    FGicPs_Disconnect(&IntcInstance, int_id);
    
    return 0;
}

/***************************** frame function ********************************/

///****************************************************************************/
///**
//* This function sets all bytes of a frame to 0.
//*
//* @param    FramePtr is a pointer to the frame itself.
//*
//* @return   None.
//*
//* @note     None.
//*
//*****************************************************************************/
//void fmsh_gmac_FrameMemClear(EthernetFrame * FramePtr)
//{
//	u32 *Data32Ptr = (u32 *) FramePtr;
//	u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);
//
//	/* frame should be an integral number of words */
//	while (WordsLeft--) {
//		*Data32Ptr++ = 0xDEADBEEF;
//	}
//}






/*
 * fmsh_gmacpsif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t fmsh_gmacpsif_init(struct netif *netif)
{


	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = fgmacpsif_output;
	netif->linkoutput = low_level_output;

	low_level_init(netif);
	return ERR_OK;
}

/*
 * fgmacpsif_resetrx_on_no_rxdata():
 *
 * Should be called by the user at regular intervals, typically
 * from a timer (100 msecond). This is to provide a SW workaround
 * for the HW bug (SI #692601). Please refer to the function header
 * for the function resetrx_on_no_rxdata in femacpsif_dma.c to
 * know more about the SI.
 *
 */

void fgmacpsif_resetrx_on_no_rxdata(struct netif *netif)
{
	struct fgmac_s *fgmac = (struct fgmac_s *)(netif->state);
	fgmacpsif_s *fgmacpsif = (fgmacpsif_s *)(fgmac->state);

	resetrx_on_no_rxdata(fgmacpsif);
}
