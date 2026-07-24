/******************************************************************************
*
* Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_gmac_example.c
*
* gmac phyloop example
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 1_0   Danyang Wang  6/25/2024  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>

#include "fmsh_gmac_interface.h"

#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"
#include "fmsh_psu_parameters.h"

#include "fmsh_common.h"
#include "fmsh_gmac_mdio.h"
#include "marvell_88e1512.h"
#include "microchip_ksz9031RNX.h"

#include "fmsh_gmac.h"
#include "fmsh_gmac_bd.h"
#include "fmsh_gmac_hw.h"
#include "sys.h"
#include "sys_arch.h"
#include "gmac_init.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "fmsh_gmac_mem.h"

#include "sys_arch.h"
#include "fmsh_gmac_mutex.h"
/************************** Constant Definitions *****************************/

#define RXBD_CNT         1024 /* Number of RxBDs to use */
#define TXBD_CNT         4096 /* Number of TxBDs to use */
#define FRAME_NUM        4096
#define JUMBO_FRAME_SIZE 10240
#define FRAME_HDR_SIZE   18


char FGMACPS_MACADDR_rxtx0[] = {0x00, 0x0a, 0x35, 0x01, 0x02, 0x0a};
char FGMACPS_MACADDR_rxtx1[] = {0x00, 0x0a, 0x35, 0x01, 0x02, 0x04};
char FGMACPS_MACADDR_rxtx2[] = {0x00, 0x0a, 0x35, 0x01, 0x02, 0x05};
char FGMACPS_MACADDR_rxtx3[] = {0x00, 0x0a, 0x35, 0x01, 0x02, 0x06};


static UINTPTR rx_frame_storage_0[RXBD_CNT];
static UINTPTR rx_frame_storage_2[RXBD_CNT];
/************************** Variable Definitions *****************************/

volatile s32 DeviceErrors0; /* Number of errors detected in the device */
volatile s32 DeviceErrors1; /* Number of errors detected in the device */
volatile s32 DeviceErrors2; /* Number of errors detected in the device */
volatile s32 DeviceErrors3; /* Number of errors detected in the device */
extern void on_nic_data_arrived(u8 idx, u8* data, u32 len);

//__attribute__ ((section(".txframe"), aligned (64)))
__no_init EthernetFrame TxFrame_rxtx0[TXBD_CNT] __attribute__ ((aligned (64)));/*  Send buffer */

__attribute__ ((section(".txframe"), aligned (64)))
__no_init EthernetFrame TxFrame_rxtx1[1] __attribute__ ((aligned (64)));/*  Send buffer */

//__attribute__ ((section(".txframe"), aligned (64)))
__no_init EthernetFrame TxFrame_rxtx2[TXBD_CNT] __attribute__ ((aligned (64)));/*  Send buffer */

__attribute__ ((section(".txframe"), aligned (64)))
__no_init EthernetFrame TxFrame_rxtx3[1] __attribute__ ((aligned (64)));/*  Send buffer */

#if GMAC0_TEST_EXAMPLE
//    __attribute__ ((section(".txframe"), aligned (64)))
    __no_init EthernetFrame TxFrame_rxtx0_bak[TXBD_CNT] __attribute__ ((aligned (64)));/*  Send buffer */
    u32 TxFrame_0_count = 0;
#endif

u32 TxFrame_rxtx0_recv_index = 0;
u32 TxFrame_rxtx0_send_index = 0;
u32 TxFrame_rxtx0_length[TXBD_CNT];

u32 TxFrame_rxtx1_recv_index = 0;
u32 TxFrame_rxtx1_send_index = 0;
u32 TxFrame_rxtx1_length[TXBD_CNT];

u32 TxFrame_rxtx2_recv_index = 0;
u32 TxFrame_rxtx2_send_index = 0;
u32 TxFrame_rxtx2_length[TXBD_CNT];

u32 TxFrame_rxtx3_recv_index = 0;
u32 TxFrame_rxtx3_send_index = 0;
u32 TxFrame_rxtx3_length[TXBD_CNT];

FGmacPs GmacPsInstance_rxtx0;
FGmacPs GmacPsInstance_rxtx1;
FGmacPs GmacPsInstance_rxtx2;
FGmacPs GmacPsInstance_rxtx3;

extern struct frame_desc gmac_desc_0;
extern struct frame_desc gmac_desc_2;

FGmacPs_PhyConfig PhyCfg_rxtx0 = {
  .phy_device = PHY_JL2XX1,  
  .auto_detect_ad_en = 0,
  .phy_address = 1,
  .auto_nag_en = 0,
  .speed = speed_1000,
};
FGmacPs_PhyConfig PhyCfg_rxtx1 = {
  .phy_device = PHY_JL2XX1,  
  .auto_detect_ad_en = 0,
  .phy_address = 1,
  .auto_nag_en = 0,
};
FGmacPs_PhyConfig PhyCfg_rxtx2 = {
  .phy_device = PHY_YT8521,
  .auto_detect_ad_en = 0,
  .phy_address = 4,
  .auto_nag_en = 0,
  .is_fixlink = 1,
  .speed = speed_1000,
};
FGmacPs_PhyConfig PhyCfg_rxtx3 = {
  .phy_device = PHY_YT8521,  
  .auto_detect_ad_en = 1,
  .phy_address = 6,
  .auto_nag_en = 1,
};

#if defined __aarch64__

__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x20000)))
__no_init u8_t bd_space_rxtx0[0x20000];
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x20000)))
__no_init u8_t bd_space_rxtx1[0x20000];
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x20000)))
__no_init u8_t bd_space_rxtx2[0x20000];
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x20000)))
__no_init u8_t bd_space_rxtx3[0x20000];

__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdTxTerminate0;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdTxTerminate1;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdTxTerminate2;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdTxTerminate3;

__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdRxTerminate0;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdRxTerminate1;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdRxTerminate2;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (64)))
__no_init FGmacPs_Bd BdRxTerminate3;
#else

__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x10000)))
__no_init u8_t bd_space_rxtx0[0x100000];
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x10000)))
__no_init u8_t bd_space_rxtx1[0x100000];
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x10000)))
__no_init u8_t bd_space_rxtx2[0x100000];
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (0x10000)))
__no_init u8_t bd_space_rxtx3[0x100000];

__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdTxTerminate0;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdTxTerminate1;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdTxTerminate2;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdTxTerminate3;

__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdRxTerminate0;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdRxTerminate1;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdRxTerminate2;
__attribute__ ((section(".GMAC_RXTX_DESC"), aligned (32)))
__no_init FGmacPs_Bd BdRxTerminate3;
#endif

FGmacPs_Bd BdTemplate0;
FGmacPs_Bd BdTemplate1;
FGmacPs_Bd BdTemplate2;
FGmacPs_Bd BdTemplate3;

#define FGMACPS_BD_TO_INDEX(ringptr, bdptr)				\
(((UINTPTR)bdptr - (UINTPTR)(ringptr)->BaseBdAddr) / (ringptr)->Separation)

/****************************************************************************/          
u8 rxok_flag0; 
u8 rxok_flag1; 
u8 rxok_flag2; 
u8 rxok_flag3; 

//
u8 g_gmac2_int_disable = 0;

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
void gmac_interrupt_handler_rxtx0 (void )
{
  FGmacPs_IntrHandler(&GmacPsInstance_rxtx0);
}
void gmac_interrupt_handler_rxtx1 (void )
{
  FGmacPs_IntrHandler(&GmacPsInstance_rxtx1);
}
void gmac_interrupt_handler_rxtx2 (void )
{
  FGmacPs_IntrHandler(&GmacPsInstance_rxtx2);
}
void gmac_interrupt_handler_rxtx3 (void )
{
  FGmacPs_IntrHandler(&GmacPsInstance_rxtx3);
}
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
static void FGmacPsSendHandler_rxtx0 (void *Callback)
{
  
  s32 n_bds,n_pbufs_freed,bdindex;
  s32 i;
  u32 *temp,*temp_end;
  int status;
  FGmacPs_BdRing *txring;
  FGmacPs_Bd *txbdset,*curbdpntr;
  u32 regval;
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
  
  /*
  * Disable the transmit related interrupts
  */
   FGmacPs_IntDisable(InstancePtr,
                      (FGMACPS_IXR_TXCOMPL_MASK | FGMACPS_IXR_TX_ERR_MASK));
   FGmacPs_IntQ1Disable(InstancePtr, FGMACPS_INTQ1_IXR_ALL_MASK);
  
  
  txring = &(FGmacPs_GetTxRing(InstancePtr));
  
  while (1) {
    /* obtain processed BD's */
    n_bds = FGmacPs_BdRingFromHwTx(txring, TXBD_CNT, &txbdset);
    if (n_bds == 0)  {
      SYS_ARCH_UNPROTECT(lev);
      break;
    }
    /* free the processed BD's */
    n_pbufs_freed = n_bds;
    curbdpntr = txbdset;
    while (n_pbufs_freed > 0) {
      bdindex = FGMACPS_BD_TO_INDEX(txring, curbdpntr);
      temp = (u32 *)curbdpntr;
      *temp = 0;
      temp++;
      if (bdindex == (TXBD_CNT - 1)) {
        *temp = 0xC0000000;
      } else {
        *temp = 0x80000000;
      }
      dsb();
      
      curbdpntr = FGmacPs_BdRingNext(txring, curbdpntr);
      n_pbufs_freed--;
      dsb();
    }
    
    status = FGmacPs_BdRingFree(txring, n_bds, txbdset);
    if (status != FMSH_SUCCESS) {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Failure while freeing in Tx Done ISR\r\n"));
    }
  }
  
  //mtcpsr(lev);
  FGmacPs_Start(InstancePtr);
  
  SYS_ARCH_UNPROTECT(lev);
}
static void FGmacPsSendHandler_rxtx1 (void *Callback)
{
  
  s32 n_bds,n_pbufs_freed,bdindex;
  s32 i;
  u32 *temp,*temp_end;
  int status;
  FGmacPs_BdRing *txring;
  FGmacPs_Bd *txbdset,*curbdpntr;
  u32 regval;
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
  
  /*
  * Disable the transmit related interrupts
  */
  // FGmacPs_IntDisable(InstancePtr,
  //                    (FGMACPS_IXR_TXCOMPL_MASK | FGMACPS_IXR_TX_ERR_MASK));
  // FGmacPs_IntQ1Disable(InstancePtr, FGMACPS_INTQ1_IXR_ALL_MASK);
  
  
  txring = &(FGmacPs_GetTxRing(InstancePtr));
  
  while (1) {
    /* obtain processed BD's */
    n_bds = FGmacPs_BdRingFromHwTx(txring, TXBD_CNT, &txbdset);
    if (n_bds == 0)  {
      SYS_ARCH_UNPROTECT(lev);
      break;
    }
    /* free the processed BD's */
    n_pbufs_freed = n_bds;
    curbdpntr = txbdset;
    while (n_pbufs_freed > 0) {
      bdindex = FGMACPS_BD_TO_INDEX(txring, curbdpntr);
      temp = (u32 *)curbdpntr;
      *temp = 0;
      temp++;
      if (bdindex == (TXBD_CNT - 1)) {
        *temp = 0xC0000000;
      } else {
        *temp = 0x80000000;
      }
      dsb();
      
      
      curbdpntr = FGmacPs_BdRingNext(txring, curbdpntr);
      n_pbufs_freed--;
      dsb();
    }
    
    status = FGmacPs_BdRingFree(txring, n_bds, txbdset);
    if (status != FMSH_SUCCESS) {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Failure while freeing in Tx Done ISR\r\n"));
    }
  }
  
  //mtcpsr(lev);
  //FGmacPs_Start(InstancePtr);
  
  SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsSendHandler_rxtx2 (void *Callback)
{
  
  s32 n_bds,n_pbufs_freed,bdindex;
  s32 i;
  u32 *temp,*temp_end;
  int status;
  FGmacPs_BdRing *txring;
  FGmacPs_Bd *txbdset,*curbdpntr;
  u32 regval;
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
  
  /*
  * Disable the transmit related interrupts
  */
  u8 tmp_gmac2_int_disable = g_gmac2_int_disable;
  if (tmp_gmac2_int_disable) {
    FGmacPs_IntDisable(InstancePtr,
                        (FGMACPS_IXR_TXCOMPL_MASK | FGMACPS_IXR_TX_ERR_MASK));
    FGmacPs_IntQ1Disable(InstancePtr, FGMACPS_INTQ1_IXR_ALL_MASK);
  }
  
  
  txring = &(FGmacPs_GetTxRing(InstancePtr));
  
  while (1) {
    /* obtain processed BD's */
    n_bds = FGmacPs_BdRingFromHwTx(txring, TXBD_CNT, &txbdset);
    if (n_bds == 0)  {
      SYS_ARCH_UNPROTECT(lev);
      break;
    }
    /* free the processed BD's */
    n_pbufs_freed = n_bds;
    curbdpntr = txbdset;
    while (n_pbufs_freed > 0) {
      bdindex = FGMACPS_BD_TO_INDEX(txring, curbdpntr);
      temp = (u32 *)curbdpntr;
      *temp = 0;
      temp++;
      if (bdindex == (TXBD_CNT - 1)) {
        *temp = 0xC0000000;
      } else {
        *temp = 0x80000000;
      }
      dsb();
      
      
      curbdpntr = FGmacPs_BdRingNext(txring, curbdpntr);
      n_pbufs_freed--;
      dsb();
    }
    
    status = FGmacPs_BdRingFree(txring, n_bds, txbdset);
    if (status != FMSH_SUCCESS) {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Failure while freeing in Tx Done ISR\r\n"));
    }
  }
  
  //mtcpsr(lev);
  if (tmp_gmac2_int_disable) {
    FGmacPs_Start(InstancePtr);
  }
  SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsSendHandler_rxtx3 (void *Callback)
{
  
  s32 n_bds,n_pbufs_freed,bdindex;
  s32 i;
  u32 *temp,*temp_end;
  int status;
  FGmacPs_BdRing *txring;
  FGmacPs_Bd *txbdset,*curbdpntr;
  u32 regval;
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
  
  /*
  * Disable the transmit related interrupts
  */
  // FGmacPs_IntDisable(InstancePtr,
  //                    (FGMACPS_IXR_TXCOMPL_MASK | FGMACPS_IXR_TX_ERR_MASK));
  // FGmacPs_IntQ1Disable(InstancePtr, FGMACPS_INTQ1_IXR_ALL_MASK);
  
  
  txring = &(FGmacPs_GetTxRing(InstancePtr));
  
  while (1) {
    /* obtain processed BD's */
    n_bds = FGmacPs_BdRingFromHwTx(txring, TXBD_CNT, &txbdset);
    if (n_bds == 0)  {
      SYS_ARCH_UNPROTECT(lev);
      break;
    }
    /* free the processed BD's */
    n_pbufs_freed = n_bds;
    curbdpntr = txbdset;
    while (n_pbufs_freed > 0) {
      bdindex = FGMACPS_BD_TO_INDEX(txring, curbdpntr);
      temp = (u32 *)curbdpntr;
      *temp = 0;
      temp++;
      if (bdindex == (TXBD_CNT - 1)) {
        *temp = 0xC0000000;
      } else {
        *temp = 0x80000000;
      }
      dsb();
      
      
      curbdpntr = FGmacPs_BdRingNext(txring, curbdpntr);
      n_pbufs_freed--;
      dsb();
    }
    
    status = FGmacPs_BdRingFree(txring, n_bds, txbdset);
    if (status != FMSH_SUCCESS) {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Failure while freeing in Tx Done ISR\r\n"));
    }
  }
  
  //mtcpsr(lev);
  //FGmacPs_Start(InstancePtr);
  
  SYS_ARCH_UNPROTECT(lev);
}


/****************************************************************************/
/**
*
* This is the Receive handler callback function and will increment a shared
* counter that can be shared by the main thread of operation.
*
* @param	Callback is a pointer to the instance of the GmacPs device.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void FGmacPsRecvHandler_rxtx0 (void *Callback)
{
  
  volatile s32 bd_processed;
  FGmacPs_BdRing *rxring;
  FGmacPs_Bd *rxbdset, *curbdptr, *rxbd;
  s32 rx_bytes;
  s32 k;
  u32 d_temp;
  u32 bdindex;
  
  
  u32 freebds;
  FStatus status;
  u32 *temp;
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
  
  
  rxring = &(FGmacPs_GetRxRing(InstancePtr));
  
  /*
  * Disable the transmit related interrupts
  */
   FGmacPs_IntDisable(InstancePtr,
                      (FGMACPS_IXR_FRAMERX_MASK | FGMACPS_IXR_RX_ERR_MASK));
  
  while(1) {
    
    bd_processed = FGmacPs_BdRingFromHwRx(rxring, RXBD_CNT, &rxbdset);
    
    if (bd_processed <= 0) {
      SYS_ARCH_UNPROTECT(lev);
      break;
    }
    
    for (k = 0, curbdptr=rxbdset; k < bd_processed; k++) {
      void *gmac_buff_addr;
      int queue_res;
      Fmsh_DCacheInvalidateRange((UINTPTR)curbdptr, (UINTPTR)128);
      rx_bytes = FGmacPs_BdGetLength(curbdptr);
      if (rx_bytes < 34) { // 14+20
        fmsh_print("[plat]: wrong rx_bytes[%u], bdptr[%p]\r\n", rx_bytes, curbdptr);
      }
      bdindex = FGMACPS_BD_TO_INDEX(rxring, curbdptr);
      gmac_buff_addr = (void *)rx_frame_storage_0[bdindex];
      Fmsh_DCacheInvalidateRange((UINTPTR)gmac_buff_addr, (UINTPTR)EthernetFrameSize);
      on_nic_data_arrived(0, gmac_buff_addr, rx_bytes);
#if GMAC0_TEST_EXAMPLE          
      memset(TxFrame_rxtx0_bak[TxFrame_rxtx0_recv_index],0x0,EthernetFrameSize);
      memcpy(TxFrame_rxtx0_bak[TxFrame_rxtx0_recv_index],gmac_buff_addr,rx_bytes);
      TxFrame_rxtx0_length[TxFrame_rxtx0_recv_index] = rx_bytes;
      Fmsh_DCacheInvalidateRange((UINTPTR)TxFrame_rxtx0_bak[TxFrame_rxtx0_recv_index], (UINTPTR)EthernetFrameSize);
      TxFrame_rxtx0_recv_index++;
      TxFrame_0_count++;
      TxFrame_rxtx0_recv_index %= FRAME_NUM;
#endif
      frame_mem_free(&gmac_desc_0,gmac_buff_addr);
      
      curbdptr = FGmacPs_BdRingNext( rxring, curbdptr);
    }
    /* free up the BD's */
    FGmacPs_BdRingFree(rxring, bd_processed, rxbdset);
    //fmsh_print("FGmacPsRecvHandler_rxtx0 %d \r\n",bd_processed);
    
    freebds = FGmacPs_BdRingGetFreeCnt (rxring);
	while (freebds > 0) {
      freebds--;
      
      void *frame_addr = frame_mem_malloc(&gmac_desc_0);
      if (frame_addr == NULL)
      {
        GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("unable to alloc buff \r\n"));
        SYS_ARCH_UNPROTECT(lev);
        return; 
      }
      
      status = FGmacPs_BdRingAlloc(rxring, 1, &rxbd);
      if (status != FMSH_SUCCESS) {
        GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("setup_rx_bds: Error allocating RxBD\r\n"));
        SYS_ARCH_UNPROTECT(lev);
        return;
      }
      status = FGmacPs_BdRingToHw(rxring, 1, rxbd);
      if (status != FMSH_SUCCESS) {
        GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Error committing RxBD to hardware: "));
        
        if (status == FGMACPS_DMA_SG_LIST_ERROR) {
          GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FGmacPs_BdRingAlloc()\r\n"));
          SYS_ARCH_UNPROTECT(lev);
        }
        else {
          GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT,("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
          SYS_ARCH_UNPROTECT(lev);
        }
        FGmacPs_BdRingUnAlloc(rxring, 1, rxbd);
        SYS_ARCH_UNPROTECT(lev);
        return;
      }
      Fmsh_DCacheInvalidateRange((UINTPTR)frame_addr, (UINTPTR)EthernetFrameSize);
      bdindex = FGMACPS_BD_TO_INDEX(rxring, rxbd);
      temp = (u32 *)rxbd;
      if (bdindex == (RXBD_CNT - 1)) {
        *temp = 0x00000002;
      } else {
        *temp = 0;
      }
      temp++;
      *temp = 0;
      dsb();
      
      rx_frame_storage_0[bdindex] = (UINTPTR)frame_addr;
      FGmacPs_BdSetAddressRx(rxbd, (UINTPTR)frame_addr);
	}
    
    
  }
  
  FGmacPs_Start(InstancePtr);
  rxok_flag0=1;

  //    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  //    xSemaphoreGiveFromISR( gmac_sem_rx_0, &xHigherPriorityTaskWoken );
  //    if (xHigherPriorityTaskWoken == pdTRUE) {
  //      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  //    }
  
  SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsRecvHandler_rxtx1 (void *Callback)
{
  
  volatile s32 bd_processed;
  FGmacPs_BdRing *rxring;
  FGmacPs_Bd *rxbdset, *curbdptr, *rxbd;
  s32 rx_bytes;
  s32 k;
  u32 d_temp;
  u32 bdindex;
  
  
  u32 freebds;
  FStatus status;
  u32 *temp;
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
  
  
  rxring = &(FGmacPs_GetRxRing(InstancePtr));
  
  /*
  * Disable the transmit related interrupts
  */
  FGmacPs_IntDisable(InstancePtr,
                     (FGMACPS_IXR_FRAMERX_MASK | FGMACPS_IXR_RX_ERR_MASK));
  
  
  
  while(1) {
    
    bd_processed = FGmacPs_BdRingFromHwRx(rxring, RXBD_CNT, &rxbdset);
    
    if (bd_processed <= 0) {
      SYS_ARCH_UNPROTECT(lev);
      break;
    }
    
    for (k = 0, curbdptr=rxbdset; k < bd_processed; k++) {
      
      rx_bytes = FGmacPs_BdGetLength(curbdptr);   
      curbdptr = FGmacPs_BdRingNext( rxring, curbdptr);
    }
    /* free up the BD's */
    FGmacPs_BdRingFree(rxring, bd_processed, rxbdset);
    
    
    
    
    freebds = FGmacPs_BdRingGetFreeCnt (rxring);
    u32 i = 0;
	while (freebds > 0) {
      freebds--;
      
      status = FGmacPs_BdRingAlloc(rxring, 1, &rxbd);
      if (status != FMSH_SUCCESS) {
        GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("setup_rx_bds: Error allocating RxBD\r\n"));
        SYS_ARCH_UNPROTECT(lev);
        return;
      }
      status = FGmacPs_BdRingToHw(rxring, 1, rxbd);
      if (status != FMSH_SUCCESS) {
        GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Error committing RxBD to hardware: "));
        if (status == FGMACPS_DMA_SG_LIST_ERROR) {
          GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FGmacPs_BdRingAlloc()\r\n"));
          SYS_ARCH_UNPROTECT(lev);
        }
        else {
          GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT,("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
          SYS_ARCH_UNPROTECT(lev);
        }
        FGmacPs_BdRingUnAlloc(rxring, 1, rxbd);
        SYS_ARCH_UNPROTECT(lev);
        return;
      }
      bdindex = FGMACPS_BD_TO_INDEX(rxring, rxbd);
      temp = (u32 *)rxbd;
      if (bdindex == (RXBD_CNT - 1)) {
        *temp = 0x00000002;
      } else {
        *temp = 0;
      }
      temp++;
      *temp = 0;
      dsb();
      i++;
	}
    
    
  }
  FGmacPs_Start(InstancePtr);
  //rxok_flag1=1;
  SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsRecvHandler_rxtx2 (void *Callback)
{
  
    volatile s32 bd_processed;
    FGmacPs_BdRing *rxring;
    FGmacPs_Bd *rxbdset, *curbdptr, *rxbd;
    s32 rx_bytes;
    s32 k;
    u32 d_temp;
    u32 bdindex;
    
    
    u32 freebds;
    FStatus status;
    u32 *temp;
    
    SYS_ARCH_DECL_PROTECT(lev);
    SYS_ARCH_PROTECT(lev);
    FGmacPs *InstancePtr = (FGmacPs *)Callback;
    
    
    rxring = &(FGmacPs_GetRxRing(InstancePtr));

    /*
     * Disable the transmit related interrupts
     */
    u8 tmp_gmac2_int_disable = g_gmac2_int_disable;
    if (tmp_gmac2_int_disable) {
      FGmacPs_IntDisable(InstancePtr,
                        (FGMACPS_IXR_FRAMERX_MASK | FGMACPS_IXR_RX_ERR_MASK));
    }
    
    while(1) 
    {

        bd_processed = FGmacPs_BdRingFromHwRx(rxring, RXBD_CNT, &rxbdset);
      
        if (bd_processed <= 0) 
        {
            SYS_ARCH_UNPROTECT(lev);
            break;
        }

        for (k = 0, curbdptr=rxbdset; k < bd_processed; k++) 
        {
            void *gmac_buff_addr;
            rx_bytes = FGmacPs_BdGetLength(curbdptr);
            bdindex = FGMACPS_BD_TO_INDEX(rxring, curbdptr);
            gmac_buff_addr = (void *)rx_frame_storage_2[bdindex];
            Fmsh_DCacheInvalidateRange((UINTPTR)gmac_buff_addr, (UINTPTR)EthernetFrameSize);
            on_nic_data_arrived(2,gmac_buff_addr,rx_bytes);
            frame_mem_free(&gmac_desc_2,gmac_buff_addr);                   
            curbdptr = FGmacPs_BdRingNext( rxring, curbdptr);
        }
        /* free up the BD's */
        FGmacPs_BdRingFree(rxring, bd_processed, rxbdset);
        //fmsh_print("FGmacPsRecvHandler_rxtx2 %d \r\n",bd_processed);  
        
        freebds = FGmacPs_BdRingGetFreeCnt (rxring);

        while (freebds > 0) 
        {
            freebds--;
            void *frame_addr = frame_mem_malloc(&gmac_desc_2);
            if (frame_addr == NULL)
            {
                GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("unable to alloc buff \r\n"));
                SYS_ARCH_UNPROTECT(lev);
                return; 
            }

            status = FGmacPs_BdRingAlloc(rxring, 1, &rxbd);
            if (status != FMSH_SUCCESS) 
            {
                GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("setup_rx_bds: Error allocating RxBD\r\n"));
                SYS_ARCH_UNPROTECT(lev);
                frame_mem_free(&gmac_desc_2,frame_addr);
                return;
            }
            status = FGmacPs_BdRingToHw(rxring, 1, rxbd);
            if (status != FMSH_SUCCESS) 
            {
                GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Error committing RxBD to hardware: "));
                
                if (status == FGMACPS_DMA_SG_LIST_ERROR) 
                {
                    GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FGmacPs_BdRingAlloc()\r\n"));
                    SYS_ARCH_UNPROTECT(lev);
                }
                else 
                {
                    GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT,("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
                    SYS_ARCH_UNPROTECT(lev);
                }
                FGmacPs_BdRingUnAlloc(rxring, 1, rxbd);
                SYS_ARCH_UNPROTECT(lev);
                frame_mem_free(&gmac_desc_2,frame_addr);
                return;
            }
            Fmsh_DCacheInvalidateRange((UINTPTR)frame_addr, (UINTPTR)EthernetFrameSize);
            bdindex = FGMACPS_BD_TO_INDEX(rxring, rxbd);
            temp = (u32 *)rxbd;
            if (bdindex == (RXBD_CNT - 1)) 
            {
                *temp = 0x00000002;
            } else 
            {
                *temp = 0;
            }
            temp++;
            *temp = 0;
            dsb();

            rx_frame_storage_2[bdindex] = (UINTPTR)frame_addr;
            FGmacPs_BdSetAddressRx(rxbd, (UINTPTR)frame_addr);
        }             
    }
    if (tmp_gmac2_int_disable) {
      FGmacPs_Start(InstancePtr);
    }
    rxok_flag2=1;
    SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsRecvHandler_rxtx3 (void *Callback)
{
  
  volatile s32 bd_processed;
  FGmacPs_BdRing *rxring;
  FGmacPs_Bd *rxbdset, *curbdptr, *rxbd;
  s32 rx_bytes;
  s32 k;
  u32 d_temp;
  u32 bdindex;
  
  
  u32 freebds;
  FStatus status;
  u32 *temp;
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
  
  
  rxring = &(FGmacPs_GetRxRing(InstancePtr));
  
  /*
  * Disable the transmit related interrupts
  */
  FGmacPs_IntDisable(InstancePtr,
                     (FGMACPS_IXR_FRAMERX_MASK | FGMACPS_IXR_RX_ERR_MASK));
  
  
  
  while(1) {
    
    bd_processed = FGmacPs_BdRingFromHwRx(rxring, RXBD_CNT, &rxbdset);
    
    if (bd_processed <= 0) {
      SYS_ARCH_UNPROTECT(lev);
      break;
    }
    
    for (k = 0, curbdptr=rxbdset; k < bd_processed; k++) {
      
      rx_bytes = FGmacPs_BdGetLength(curbdptr);
      curbdptr = FGmacPs_BdRingNext( rxring, curbdptr);
    }
    /* free up the BD's */
    FGmacPs_BdRingFree(rxring, bd_processed, rxbdset);
    
    
    
    
    freebds = FGmacPs_BdRingGetFreeCnt (rxring);
    u32 i = 0;
	while (freebds > 0) {
      freebds--;
      
      status = FGmacPs_BdRingAlloc(rxring, 1, &rxbd);
      if (status != FMSH_SUCCESS) {
        GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("setup_rx_bds: Error allocating RxBD\r\n"));
        SYS_ARCH_UNPROTECT(lev);
        return;
      }
      status = FGmacPs_BdRingToHw(rxring, 1, rxbd);
      if (status != FMSH_SUCCESS) {
        GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("Error committing RxBD to hardware: "));
        if (status == FGMACPS_DMA_SG_LIST_ERROR) {
          GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, ("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FGmacPs_BdRingAlloc()\r\n"));
          SYS_ARCH_UNPROTECT(lev);
        }
        else {
          GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT,("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n"));
          SYS_ARCH_UNPROTECT(lev);
        }
        FGmacPs_BdRingUnAlloc(rxring, 1, rxbd);
        SYS_ARCH_UNPROTECT(lev);
        return;
      }
      bdindex = FGMACPS_BD_TO_INDEX(rxring, rxbd);
      temp = (u32 *)rxbd;
      if (bdindex == (RXBD_CNT - 1)) {
        *temp = 0x00000002;
      } else {
        *temp = 0;
      }
      temp++;
      *temp = 0;
      dsb();
      
      i++;
	}
    
    
  }
//   FGmacPs_Start(InstancePtr);
//   rxok_flag3=1;
  SYS_ARCH_UNPROTECT(lev);
  
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
static void FGmacPsErrorHandler_rxtx0 (void *Callback, u8 Direction, u32 ErrorWord)
{
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
#if GMAC_DEBUG_RESET_ON_ERR
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
#endif
  /*
  * Increment the counter so that main thread knows something
  * happened. Reset the device and reallocate resources ...
  */
  DeviceErrors0++;
  
  switch (Direction)
  {
  case FGMACPS_RECV:
    if (ErrorWord & FGMACPS_RXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive over run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_BUFFNA_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive buffer not available");
      SYS_ARCH_UNPROTECT(lev);
    }
    break;
  case FGMACPS_SEND:
    if (ErrorWord & FGMACPS_TXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_URUN_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit under run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_BUFEXH_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer exhausted");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit retry excessed limits");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_FRAMERX_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit collision");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_USEDREAD_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer not available");
      SYS_ARCH_UNPROTECT(lev);
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
  SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsErrorHandler_rxtx1 (void *Callback, u8 Direction, u32 ErrorWord)
{
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
#if GMAC_DEBUG_RESET_ON_ERR
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
#endif
  /*
  * Increment the counter so that main thread knows something
  * happened. Reset the device and reallocate resources ...
  */
  DeviceErrors1++;
  
  switch (Direction)
  {
  case FGMACPS_RECV:
    if (ErrorWord & FGMACPS_RXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive over run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_BUFFNA_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive buffer not available");
      SYS_ARCH_UNPROTECT(lev);
    }
    break;
  case FGMACPS_SEND:
    if (ErrorWord & FGMACPS_TXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_URUN_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit under run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_BUFEXH_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer exhausted");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit retry excessed limits");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_FRAMERX_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit collision");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_USEDREAD_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer not available");
      SYS_ARCH_UNPROTECT(lev);
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
  SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsErrorHandler_rxtx2 (void *Callback, u8 Direction, u32 ErrorWord)
{
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
#if GMAC_DEBUG_RESET_ON_ERR
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
#endif
  /*
  * Increment the counter so that main thread knows something
  * happened. Reset the device and reallocate resources ...
  */
  DeviceErrors2++;
  
  switch (Direction)
  {
  case FGMACPS_RECV:
    if (ErrorWord & FGMACPS_RXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive over run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_BUFFNA_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive buffer not available");
      SYS_ARCH_UNPROTECT(lev);
    }
    break;
  case FGMACPS_SEND:
    if (ErrorWord & FGMACPS_TXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_URUN_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit under run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_BUFEXH_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer exhausted");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit retry excessed limits");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_FRAMERX_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit collision");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_USEDREAD_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer not available");
      SYS_ARCH_UNPROTECT(lev);
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
  SYS_ARCH_UNPROTECT(lev);
}

static void FGmacPsErrorHandler_rxtx3 (void *Callback, u8 Direction, u32 ErrorWord)
{
  
  SYS_ARCH_DECL_PROTECT(lev);
  SYS_ARCH_PROTECT(lev);
#if GMAC_DEBUG_RESET_ON_ERR
  FGmacPs *InstancePtr = (FGmacPs *)Callback;
#endif
  /*
  * Increment the counter so that main thread knows something
  * happened. Reset the device and reallocate resources ...
  */
  DeviceErrors3++;
  
  switch (Direction)
  {
  case FGMACPS_RECV:
    if (ErrorWord & FGMACPS_RXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive over run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_RXSR_BUFFNA_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Receive buffer not available");
      SYS_ARCH_UNPROTECT(lev);
    }
    break;
  case FGMACPS_SEND:
    if (ErrorWord & FGMACPS_TXSR_HRESPNOK_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit DMA error");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_URUN_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit under run");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_BUFEXH_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer exhausted");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_RXOVR_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit retry excessed limits");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_FRAMERX_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit collision");
      SYS_ARCH_UNPROTECT(lev);
    }
    if (ErrorWord & FGMACPS_TXSR_USEDREAD_MASK)
    {
      GMAC_ISR_TRACE_OUT(GMAC_DEBUG_OUT, "Transmit buffer not available");
      SYS_ARCH_UNPROTECT(lev);
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
  SYS_ARCH_UNPROTECT(lev);
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

int fmsh_gmac_verify_device_initial (FGmacPs *InstancePtr, u8 GMAC_ID, const u8* mac_addr)
{
    int Status;
    FGmacPs_Config *ConfigPtr;
    
    
    ConfigPtr = FGmacPs_LookupConfig(GMAC_ID);
    
    Status = FGmacPs_CfgInitialize(InstancePtr, ConfigPtr,
                                    ConfigPtr->BaseAddress);
    
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error in cfg initialize");
        return FMSH_FAILURE;
    }
    
    /* Enable jumbo frames for zynqmp */
    FGmacPs_SetOptions(InstancePtr, FGMACPS_JUMBO_ENABLE_OPTION);

    if (GMAC_ID == FPAR_GMACPS_0_DEVICE_ID || GMAC_ID == FPAR_GMACPS_2_DEVICE_ID)
    {
        FGmacPs_SetOptions(InstancePtr, FGMACPS_PROMISC_OPTION);  
    }
    
    /*
    * Set the MAC address
    */
    
    switch(GMAC_ID){
        
    case  FPAR_GMACPS_0_DEVICE_ID:{
        
        Status = FGmacPs_SetMacAddress(InstancePtr, (NULL == mac_addr ? FGMACPS_MACADDR_rxtx0 : (void*)mac_addr), 1);
        if (Status != FMSH_SUCCESS)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error setting MAC address");
            return FMSH_FAILURE;
        } 
        /*
        * Setup callbacks
        */
        Status = FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMASEND,
                                    (void *)FGmacPsSendHandler_rxtx0, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMARECV,
                                    (void *)FGmacPsRecvHandler_rxtx0, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_ERROR,
                                    (void *)FGmacPsErrorHandler_rxtx0, InstancePtr);
        break;
    } 
    case  FPAR_GMACPS_1_DEVICE_ID:{
        
        Status = FGmacPs_SetMacAddress(InstancePtr, (NULL == mac_addr ? FGMACPS_MACADDR_rxtx1 : (void*)mac_addr), 1);
        if (Status != FMSH_SUCCESS)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error setting MAC address");
            return FMSH_FAILURE;
        }
        /*
        * Setup callbacks
        */
        Status = FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMASEND,
                                    (void *)FGmacPsSendHandler_rxtx1, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMARECV,
                                    (void *)FGmacPsRecvHandler_rxtx1, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_ERROR,
                                    (void *)FGmacPsErrorHandler_rxtx1, InstancePtr);
        break;
    }
    case  FPAR_GMACPS_2_DEVICE_ID:{
        
        Status = FGmacPs_SetMacAddress(InstancePtr, (NULL == mac_addr ? FGMACPS_MACADDR_rxtx2 : (void*)mac_addr), 1);
        if (Status != FMSH_SUCCESS)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error setting MAC address");
            return FMSH_FAILURE;
        }  
        /*
        * Setup callbacks
        */
        Status = FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMASEND,
                                    (void *)FGmacPsSendHandler_rxtx2, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMARECV,
                                    (void *)FGmacPsRecvHandler_rxtx2, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_ERROR,
                                    (void *)FGmacPsErrorHandler_rxtx2, InstancePtr);
        break;
    }      
    case  FPAR_GMACPS_3_DEVICE_ID:{
        
        Status = FGmacPs_SetMacAddress(InstancePtr, (NULL == mac_addr ? FGMACPS_MACADDR_rxtx3 : (void*)mac_addr), 1);
        if (Status != FMSH_SUCCESS)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error setting MAC address");
            return FMSH_FAILURE;
        }  
        /*
        * Setup callbacks
        */
        Status = FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMASEND,
                                    (void *)FGmacPsSendHandler_rxtx3, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_DMARECV,
                                    (void *)FGmacPsRecvHandler_rxtx3, InstancePtr);
        Status |= FGmacPs_SetHandler(InstancePtr, FGMACPS_HANDLER_ERROR,
                                    (void *)FGmacPsErrorHandler_rxtx3, InstancePtr);
        break;
    }      
    }        
    
    if (Status != FMSH_SUCCESS)
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Error assigning handlers");
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
static void PSU_Mask_Write_temp(unsigned long offset, unsigned long mask,
                                unsigned long val)
{
  unsigned long RegVal = 0x0;
  
  RegVal = FMSH_IN32_32(offset);
  RegVal &= ~(mask);
  RegVal |= (val & mask);
  FMSH_OUT32_32(RegVal, offset);
}

int fmsh_gmac_verify_phy_initial (FGmacPs *InstancePtr,
                                  FGmacPs_PhyConfig *PhyCfgPtr)
{
    int speed = speed_1000;
    u16 PhyAddr = 0;
    int Status;

   u16 test_phy_data;
    
   if (!PhyCfgPtr->is_fixlink){
      
      switch (InstancePtr->Config.BaseAddress){
        
      //MDIO set 60 80 A0 C0 for gmac 0 1 2 3
        
      case FPS_GMAC0_BASEADDR:{
        
        /* IOU_SLCR_MIO_PIN_76_OFFSET */
        PSU_Mask_Write_temp(0xFF180130, 0x000000FEU, 0x00000060U);
        /* IOU_SLCR_MIO_PIN_77_OFFSET */
        PSU_Mask_Write_temp(0xFF180134, 0x000000FEU, 0x00000060U);
              break;}

      case FPS_GMAC1_BASEADDR:{
         
        /* IOU_SLCR_MIO_PIN_76_OFFSET */
        PSU_Mask_Write_temp(0xFF180130, 0x000000FEU, 0x00000080U);
        /* IOU_SLCR_MIO_PIN_77_OFFSET */
        PSU_Mask_Write_temp(0xFF180134, 0x000000FEU, 0x00000080U);
              break;}    
              
      case FPS_GMAC2_BASEADDR:{
        //sys_sem_new(&sem_PhyInit_available, 0);
              
        /* IOU_SLCR_MIO_PIN_76_OFFSET */
        PSU_Mask_Write_temp(0xFF180130, 0x000000FEU, 0x000000a0U);
        /* IOU_SLCR_MIO_PIN_77_OFFSET */
        PSU_Mask_Write_temp(0xFF180134, 0x000000FEU, 0x000000a0U);
              break;}
              
      case FPS_GMAC3_BASEADDR:{
        //sys_sem_wait( &sem_PhyInit_available);
        /* IOU_SLCR_MIO_PIN_76_OFFSET */
        PSU_Mask_Write_temp(0xFF180130, 0x000000FEU, 0x000000c0U);
        /* IOU_SLCR_MIO_PIN_77_OFFSET */
        PSU_Mask_Write_temp(0xFF180134, 0x000000FEU, 0x000000c0U);
              break;}        
      }
      
    
      FGmacPs_SetMdioDivisor(InstancePtr, MDC_DIV_224);
      delay_ms(1000);   

      /* detect phy */
      if (PhyCfgPtr->auto_detect_ad_en == 1)
      {
          PhyAddr = FGmacPs_PHYDetect(InstancePtr);
          PhyCfgPtr->phy_address = PhyAddr;
      }
      PhyCfgPtr->speed = InstancePtr->Config.Speed;

      /* set phy address & phy device */
      switch (InstancePtr->Config.BaseAddress)
      {
        case FPS_GMAC0_BASEADDR:
        {
            PhyCfgPtr->phy_address = 1;
            PhyCfgPtr->phy_device = PHY_JL2XX1;
            break;
        }
        case FPS_GMAC1_BASEADDR:
        {
            PhyCfgPtr->phy_address = 1;
            PhyCfgPtr->phy_device = PHY_JL2XX1;
            break;
        }
        case FPS_GMAC2_BASEADDR:
        {
            PhyCfgPtr->phy_address = 4;
            PhyCfgPtr->phy_device = PHY_YT8521;
            break;
        }
        case FPS_GMAC3_BASEADDR:
        {
            PhyCfgPtr->phy_address = 6;
            PhyCfgPtr->phy_device = PHY_YT8521;
            break;
        }
      }

      /* operate phy Init */
      Status = FGmacPs_PHYInit(InstancePtr, PhyCfgPtr);
      if (Status != FMSH_SUCCESS)
      {
          GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "PHY init fail\r\n");
      }
      else
      {
          GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                         "PHY init success, Address = %x, type = %d\r\n",
                         PhyCfgPtr->phy_address, PhyCfgPtr->phy_device);
      }

      /* get phy operating speed if autoneg is on */
      if (PhyCfgPtr->auto_nag_en == 1)
      {
          speed = PhyCfgPtr->speed;
      }
      else
      {
          speed = PhyCfgPtr->speed;
      }
   }

    /* set operating speed */
    FGmacPs_SetOperatingSpeed(InstancePtr, speed);

    return FMSH_SUCCESS;
  
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
int fmsh_gmac_verify_gic_setup (FGmacPs *InstancePtr)
{
  
  u32 int_id = 0x59U;
  
  
  // u32 Status;
  switch (InstancePtr->Config.DeviceId)
  {
  case FPAR_GMACPS_0_DEVICE_ID:
    int_id = 0x59U;
    FGicPs_InterruptMaptoCpu (&IntcInstance, GICMAP_CPUID3, int_id);
    FGicPs_Connect(&IntcInstance, int_id,
                   (FMSH_InterruptHandler)gmac_interrupt_handler_rxtx0, 0);
    break;
  case FPAR_GMACPS_1_DEVICE_ID:
    int_id = 0x5BU;
    FGicPs_InterruptMaptoCpu (&IntcInstance, GICMAP_CPUID0, int_id);
    FGicPs_Connect(&IntcInstance, int_id,
                   (FMSH_InterruptHandler)gmac_interrupt_handler_rxtx1, 0);
    break;
  case FPAR_GMACPS_2_DEVICE_ID:
    int_id = 0x5DU;
    FGicPs_InterruptMaptoCpu (&IntcInstance, GICMAP_CPUID3, int_id);
    FGicPs_Connect(&IntcInstance, int_id,
                   (FMSH_InterruptHandler)gmac_interrupt_handler_rxtx2, 0);
    break;
  case FPAR_GMACPS_3_DEVICE_ID:
    int_id = 0x5FU;
    FGicPs_InterruptMaptoCpu (&IntcInstance, 4, int_id);
    FGicPs_Connect(&IntcInstance, int_id,
                   (FMSH_InterruptHandler)gmac_interrupt_handler_rxtx3, 0);
    break;
  default:
    int_id = 0x59U;
    break;
  }
  
  FMSH_ExceptionRegisterHandler(
                                FMSH_EXCEPTION_ID_FIQ_INT,
                                (FMSH_ExceptionHandler)FGicPs_InterruptHandler_FIQ, &IntcInstance);
  FGicPs_Enable(&IntcInstance, int_id);
  
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
int fmsh_gmac_verify_gic_stop (FGmacPs *InstancePtr)
{
  u32 int_id;
  
  switch (InstancePtr->Config.DeviceId)
  {
  case FPAR_GMACPS_0_DEVICE_ID:
    int_id = 0x59U;
    break;
  case FPAR_GMACPS_1_DEVICE_ID:
    int_id = 0x5BU;
    break;
  case FPAR_GMACPS_2_DEVICE_ID:
    int_id = 0x5DU;
    break;
  case FPAR_GMACPS_3_DEVICE_ID:
    int_id = 0x5FU;
    break;
  }
  
  FGicPs_Disconnect(&IntcInstance, int_id);
  
  return 0;
}


/****************************************************************************/
/**
* This function sets all bytes of a frame to 0.
*
* @param    FramePtr is a pointer to the frame itself.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void fmsh_gmac_verify_FrameMemClear (EthernetFrame *FramePtr)
{
  u32 *Data32Ptr = (u32 *)FramePtr;
  u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);
  
  /* frame should be an integral number of words */
  while (WordsLeft--)
  {
    *Data32Ptr++ = 0xDEADBEEF;
  }
}

/***************************** test function *********************************/

/*****************************************************************************/
/**
* Gmac rxtx test.
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return
* - FMSH_SUCCESS if initialization was successful
*
******************************************************************************/
#if GMAC0_TEST_EXAMPLE

    int fmsh_gmac_0_tx_send_example()
    {
        FGmacPs_Bd *BdTxPtr;
        FStatus status;
        LONG Status;
        s32 i;
        FGmacPs_BdRing *txring_tmp;
        u32 txbd_cnts,tx_bytes;
        FGmacPs_Bd  *txbd, *last_txbd = NULL,*temp_txbd;
        
        SYS_ARCH_DECL_PROTECT(lev);
        SYS_ARCH_PROTECT(lev);

        FGmacPs * InstancePtr=&GmacPsInstance_rxtx0;
        txbd_cnts = TxFrame_0_count;
        txring_tmp = &(FGmacPs_GetTxRing(InstancePtr));
        
        status = FGmacPs_BdRingAlloc(&(FGmacPs_GetTxRing(InstancePtr)), txbd_cnts, &BdTxPtr);
        if (status != FMSH_SUCCESS) 
        {
            //mtcpsr(lev);
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,(": Error allocating TxBD\r\n"));
            SYS_ARCH_UNPROTECT(lev);
            return status;
        }
        txbd = BdTxPtr;
        
        void *tmp_addr;
        for (i = 0; i < txbd_cnts; i++) 
        {         
            memset(TxFrame_rxtx0[TxFrame_rxtx0_send_index],0x0,EthernetFrameSize);
            memcpy(TxFrame_rxtx0[TxFrame_rxtx0_send_index],TxFrame_rxtx0_bak[TxFrame_rxtx0_send_index],EthernetFrameSize);
            tmp_addr = TxFrame_rxtx0[TxFrame_rxtx0_send_index];
            tx_bytes = TxFrame_rxtx0_length[TxFrame_rxtx0_send_index];
            TxFrame_rxtx0_send_index++;
            TxFrame_rxtx0_send_index %= FRAME_NUM;
            
            /* Send the data from the pbuf to the interface, one pbuf at a
            time. The size of the data in each pbuf is kept in the ->len
            variable. */
            if (InstancePtr->Config.IsCacheCoherent == 0) 
            {
                Fmsh_DCacheFlushRange((UINTPTR)tmp_addr, (UINTPTR)tx_bytes);
            }
            
            FGmacPs_BdSetAddressTx(txbd, (UINTPTR)tmp_addr);
            
            if (tx_bytes > EthernetFrameSize)
            FGmacPs_BdSetLength(txbd, EthernetFrameSize & 0x3FFF);
            else
            FGmacPs_BdSetLength(txbd, tx_bytes & 0x3FFF);
            
            FGmacPs_BdClearTxUsed(txbd);
            last_txbd = txbd;
            
            FGmacPs_BdSetLast(last_txbd);
            status = FGmacPs_BdRingToHw(&(FGmacPs_GetTxRing(InstancePtr)), 1, txbd);
            if (status != FMSH_SUCCESS) 
            {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, ("Error: committing TxBD to HW\r\n"));
            FGmacPs_BdRingUnAlloc(&(FGmacPs_GetTxRing(InstancePtr)), 1, txbd);
            SYS_ARCH_UNPROTECT(lev);
            return status;
            }
            
            Status = FGmacPs_BdRingFree(&(FGmacPs_GetTxRing(InstancePtr)), 1, txbd);
            txbd = FGmacPs_BdRingNext(&(FGmacPs_GetTxRing(InstancePtr)), txbd);
        } 
        
        TxFrame_0_count -= txbd_cnts;
        //dsb();
        /* Start transmit */
        FGmacPs_Transmit(InstancePtr); 
        
        SYS_ARCH_UNPROTECT(lev);        
        return status;
    }

#endif

int fmsh_gmac_tx_send (u32 gmac_index, void * send_addr, u32 send_cnt, u32* send_len)
{
    FGmacPs_Bd *BdTxPtr;
    FStatus status;
    // LONG Status;
    s32 i;
    FGmacPs_BdRing *txring_tmp;
    FGmacPs *InstancePtr;
    u32 txbd_cnts,tx_bytes;
    FGmacPs_Bd  *txbd, *last_txbd = NULL,*temp_txbd;
    
    SYS_ARCH_DECL_PROTECT(lev);
    SYS_ARCH_PROTECT(lev);
    void *tmp_addr;

    txbd_cnts = send_cnt;
    switch (gmac_index)
    {
        case 0:{
            InstancePtr=&GmacPsInstance_rxtx0;
            break;}
            
        case 1:{
            InstancePtr=&GmacPsInstance_rxtx1;
            break;}    
            
        case 2:{
            InstancePtr=&GmacPsInstance_rxtx2;
            break;}
            
        case 3:{
            InstancePtr=&GmacPsInstance_rxtx3;
            break;}  
        default: {
            InstancePtr=&GmacPsInstance_rxtx0;
            break;}    
    }
    
    txring_tmp = &(FGmacPs_GetTxRing(InstancePtr));
    
    status = FGmacPs_BdRingAlloc(&(FGmacPs_GetTxRing(InstancePtr)), txbd_cnts, &BdTxPtr);
    if (status != FMSH_SUCCESS) 
    {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,(": Error allocating TxBD\r\n"));
        SYS_ARCH_UNPROTECT(lev);
        return status;
    }
    txbd = BdTxPtr;

    u32 offset = 0;
    for (i = 0; i < txbd_cnts; i++) 
    {         
        switch (gmac_index)
        {
            case 0:{
                //memset(TxFrame_rxtx0[TxFrame_rxtx0_send_index],0x0,EthernetFrameSize);
                memcpy(TxFrame_rxtx0[TxFrame_rxtx0_send_index],(char *)send_addr+offset,send_len[i]);
                tmp_addr = TxFrame_rxtx0[TxFrame_rxtx0_send_index];
                TxFrame_rxtx0_send_index++;
                TxFrame_rxtx0_send_index %= FRAME_NUM;
                break;}
                
            case 1:{
                //memset(TxFrame_rxtx1[TxFrame_rxtx1_send_index],0x0,EthernetFrameSize);
                memcpy(TxFrame_rxtx1[TxFrame_rxtx1_send_index],(char *)send_addr+offset,send_len[i]);
                tmp_addr = TxFrame_rxtx1[TxFrame_rxtx1_send_index];
                TxFrame_rxtx1_send_index++;
                TxFrame_rxtx1_send_index %= FRAME_NUM;
                break;}    
                
            case 2:{          
               //memset(TxFrame_rxtx2[TxFrame_rxtx2_send_index],0x0,EthernetFrameSize);
               memcpy(TxFrame_rxtx2[TxFrame_rxtx2_send_index],(char *)send_addr+offset,send_len[i]);
               tmp_addr = TxFrame_rxtx2[TxFrame_rxtx2_send_index];
               TxFrame_rxtx2_send_index++;
               TxFrame_rxtx2_send_index %= FRAME_NUM;
                break;}
                
            case 3:{
                //memset(TxFrame_rxtx3[TxFrame_rxtx3_send_index],0x0,EthernetFrameSize);
                memcpy(TxFrame_rxtx3[TxFrame_rxtx3_send_index],(char *)send_addr+offset,send_len[i]);
                tmp_addr = TxFrame_rxtx3[TxFrame_rxtx3_send_index];
                TxFrame_rxtx3_send_index++;
                TxFrame_rxtx3_send_index %= FRAME_NUM;
                break;}  
            default: {
                //memset(TxFrame_rxtx0[TxFrame_rxtx0_send_index],0x0,EthernetFrameSize);
                memcpy(TxFrame_rxtx0[TxFrame_rxtx0_send_index],(char *)send_addr+offset,send_len[i]);
                tmp_addr = TxFrame_rxtx0[TxFrame_rxtx0_send_index];
                TxFrame_rxtx0_send_index++;
                TxFrame_rxtx0_send_index %= FRAME_NUM;
                break;}    
        }

        tx_bytes = send_len[i];
        /* Send the data from the pbuf to the interface, one pbuf at a
        time. The size of the data in each pbuf is kept in the ->len
        variable. */
        Fmsh_DCacheFlushRange((UINTPTR)tmp_addr, (UINTPTR)tx_bytes);
        
        FGmacPs_BdSetAddressTx(txbd, (UINTPTR)tmp_addr);
        
        if (tx_bytes > ETH_MAX_FRAME_SIZE_FOR_APP)
          FGmacPs_BdSetLength(txbd, ETH_MAX_FRAME_SIZE_FOR_APP & 0x3FFF);
        else
          FGmacPs_BdSetLength(txbd, tx_bytes & 0x3FFF);
        
        FGmacPs_BdClearTxUsed(txbd);
        last_txbd = txbd;
        
        FGmacPs_BdSetLast(last_txbd);
        status = FGmacPs_BdRingToHw(&(FGmacPs_GetTxRing(InstancePtr)), 1, txbd);
        if (status != FMSH_SUCCESS) 
        {
          GMAC_TRACE_OUT(GMAC_DEBUG_OUT, ("Error: committing TxBD to HW\r\n"));
          FGmacPs_BdRingUnAlloc(&(FGmacPs_GetTxRing(InstancePtr)), 1, txbd);
          SYS_ARCH_UNPROTECT(lev);
          return status;
        }
        
        // Status = FGmacPs_BdRingFree(&(FGmacPs_GetTxRing(InstancePtr)), 1, txbd);
        txbd = FGmacPs_BdRingNext(&(FGmacPs_GetTxRing(InstancePtr)), txbd);
        //
        offset += send_len[i];
    } 
    dsb();
    /* Start transmit */
    FGmacPs_Transmit(InstancePtr); 
    
    SYS_ARCH_UNPROTECT(lev);        
    return status;
}

/*****************************************************************************/
/**
* Gmac tests entry.
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return
* - FMSH_SUCCESS if initialization was successful
*
******************************************************************************/

int fmsh_gmac_initial_config(u8 GMAC_ID, const u8* mac_addr)
{
  int Status = FMSH_SUCCESS;
  FGmacPs_Config *GmacPsConfigPtr;
  FGmacPs *GmacPsInstancePtr;
  FGmacPs_PhyConfig *PhyCfgPtr;
  FGmacPs_Bd *BdTxPtr;
  FGmacPs_Bd *BdRxPtr;
  
  
  u8 *RxBdSpacePtr;
  u8 *TxBdSpacePtr;
  
  GMAC_TRACE_OUT(GMAC_DEBUG_OUT, "Entering into fmsh gmac example \r\n");
  
  /* Allocate Rx and Tx BD space each */
  switch(GMAC_ID){
    
  case  FPAR_GMACPS_0_DEVICE_ID:{
    
    RxBdSpacePtr = &(bd_space_rxtx0[0]);
    TxBdSpacePtr = &(bd_space_rxtx0[0x5000]);
    
    GmacPsInstancePtr = &GmacPsInstance_rxtx0;
    GmacPsConfigPtr = &GmacPsInstancePtr->Config;
    PhyCfgPtr = &PhyCfg_rxtx0;
    
    frame_mem_init(&gmac_desc_0);
    
    /*************************************/
    /* Setup device for first-time usage */
    /*************************************/
    /*
    *  Initialize instance. Should be configured for DMA
    *  This example calls _CfgInitialize instead of _Initialize due to
    *  retiring _Initialize. So in _CfgInitialize we use
    *  XPAR_(IP)_BASEADDRESS to make sure it is not virtual address.
    */
    fmsh_gmac_verify_device_initial(GmacPsInstancePtr, GMAC_ID, mac_addr);
    
    /*
    * Setup the PL isolation if necessary
    */
    u32 REQ_ISO_INT_EN;
    u32 REQ_ISO_INT_TRIG;
    if (GmacPsInstancePtr->Config.InterFaceType == gmac_path_gmii)
    {
      // enable pmu service
      REQ_ISO_INT_EN = FGmacPs_ReadReg(0xFFD80000U, 0x0318U);
      REQ_ISO_INT_EN |= 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0318U, REQ_ISO_INT_EN);
      
      // trigger pmu service
      delay_ms(10);
      REQ_ISO_INT_TRIG = 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0320U, REQ_ISO_INT_TRIG);
    }
    
    if(xSemaphoreTake(phy_mutex,portMAX_DELAY) == pdTRUE){
      fmsh_gmac_verify_phy_initial(GmacPsInstancePtr, PhyCfgPtr);
      xSemaphoreGive(phy_mutex);
    }
    
    /*
    * Setup RxBD space.
    *
    * We have already defined a properly aligned area of memory to store
    * RxBDs at the beginning of this source code file so just pass its
    * address into the function. No MMU is being used so the physical
    * and virtual addresses are the same.
    *
    * Setup a BD template for the Rx channel. This template will be
    * copied to every RxBD. We will not have to explicitly set these
    * again.
    */
    FGmacPs_BdClear(&BdTemplate0);
    
    /*
    * Create the RxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                  (UINTPTR)RxBdSpacePtr, (UINTPTR)RxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, RXBD_CNT);
    
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                 &BdTemplate0, FGMACPS_RECV);
    if (Status != FMSH_SUCCESS) {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error setting up RxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    /*
    * Setup TxBD space.
    *
    * Like RxBD space, we have already defined a properly aligned area
    * of memory to use.
    *
    * Also like the RxBD space, we create a template. Notice we don't
    * set the "last" attribute. The example will be overriding this
    * attribute so it does no good to set it up here.
    */
    FGmacPs_BdClear(&BdTemplate0);
    FGmacPs_BdSetStatus(&BdTemplate0, FGMACPS_TXBUF_USED_MASK);
    
    /*
    * Create the TxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                  (UINTPTR)TxBdSpacePtr, (UINTPTR)TxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, TXBD_CNT);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error setting up TxBD space, BdRingCreate");
      return FMSH_FAILURE;
    }
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                 &BdTemplate0, FGMACPS_SEND);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error setting up TxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    FGmacPs_BdClear(&BdRxTerminate0);
    FGmacPs_BdSetAddressRx(&BdRxTerminate0,
                           (FGMACPS_RXBUF_NEW_MASK | FGMACPS_RXBUF_WRAP_MASK));
    
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_RXQ1BASE_OFFSET,
                     (UINTPTR)&BdRxTerminate0);
    FGmacPs_BdClear(&BdTxTerminate0);
    FGmacPs_BdSetStatus(&BdTxTerminate0, (FGMACPS_TXBUF_USED_MASK |
                                          FGMACPS_TXBUF_WRAP_MASK));
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_TXQBASE_OFFSET, 
                     (UINTPTR)&BdTxTerminate0);
    
    
    /*
    * Set the Queue pointers
    */
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->RxBdRing.BaseBdAddr, 0, FGMACPS_RECV);
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->TxBdRing.BaseBdAddr, 1, FGMACPS_SEND);
    
    /*
    * Start the device
    */
    FGmacPs_Start(GmacPsInstancePtr);
    
    /*
    * Setup the interrupt controller and enable interrupts
    */
    Status = fmsh_gmac_verify_gic_setup(GmacPsInstancePtr);
    
    for (u32 i = 0; i < RXBD_CNT; i++) 
    {
      Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, &BdRxPtr);
      if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,("init_dma: Error allocating RxBD\r\n"));
        return Status;
      }
      Status = FGmacPs_BdRingToHw(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
      if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, ("Error: committing RxBD to HW\r\n"));
        FGmacPs_BdRingUnAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
        return Status;
      }
      
      void *bd_addr = frame_mem_malloc(&gmac_desc_0);
      if (bd_addr == NULL)
      {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,("mem malloc failed \r\n"));
        return FMSH_FAILURE;
      }
      u32 bdindex = FGMACPS_BD_TO_INDEX(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), BdRxPtr);
      Fmsh_DCacheInvalidateRange((UINTPTR)bd_addr, (UINTPTR)EthernetFrameSize);
      rx_frame_storage_0[bdindex] = (UINTPTR)bd_addr;
      FGmacPs_BdSetAddressRx(BdRxPtr, (UINTPTR)bd_addr);
      
#ifdef PSU_CACHE_ENABLE_GMAC
      Fmsh_DCacheInvalidateRange((UINTPTR)BdRxPtr, 64);
#endif                       
    }   
    break;
  }
  
  case  FPAR_GMACPS_1_DEVICE_ID:{
    
    RxBdSpacePtr = &(bd_space_rxtx1[0]);
    TxBdSpacePtr = &(bd_space_rxtx1[0x5000]);
    
    GmacPsInstancePtr = &GmacPsInstance_rxtx1;
    GmacPsConfigPtr = &GmacPsInstancePtr->Config;
    PhyCfgPtr = &PhyCfg_rxtx1;
    
    /*************************************/
    /* Setup device for first-time usage */
    /*************************************/
    /*
    *  Initialize instance. Should be configured for DMA
    *  This example calls _CfgInitialize instead of _Initialize due to
    *  retiring _Initialize. So in _CfgInitialize we use
    *  XPAR_(IP)_BASEADDRESS to make sure it is not virtual address.
    */
    fmsh_gmac_verify_device_initial(GmacPsInstancePtr, GMAC_ID, mac_addr);
    
    /*
    * Setup the PL isolation if necessary
    */
    u32 REQ_ISO_INT_EN;
    u32 REQ_ISO_INT_TRIG;
    if (GmacPsInstancePtr->Config.InterFaceType == gmac_path_gmii)
    {
      // enable pmu service
      REQ_ISO_INT_EN = FGmacPs_ReadReg(0xFFD80000U, 0x0318U);
      REQ_ISO_INT_EN |= 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0318U, REQ_ISO_INT_EN);
      
      // trigger pmu service
      delay_ms(10);
      REQ_ISO_INT_TRIG = 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0320U, REQ_ISO_INT_TRIG);
    }
    
    
    if(xSemaphoreTake(phy_mutex,portMAX_DELAY) == pdTRUE){
      fmsh_gmac_verify_phy_initial(GmacPsInstancePtr, PhyCfgPtr);
      xSemaphoreGive(phy_mutex);
    }
    
    /*
    * Setup RxBD space.
    *
    * We have already defined a properly aligned area of memory to store
    * RxBDs at the beginning of this source code file so just pass its
    * address into the function. No MMU is being used so the physical
    * and virtual addresses are the same.
    *
    * Setup a BD template for the Rx channel. This template will be
    * copied to every RxBD. We will not have to explicitly set these
    * again.
    */
    FGmacPs_BdClear(&BdTemplate1);
    
    /*
    * Create the RxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                  (UINTPTR)RxBdSpacePtr, (UINTPTR)RxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, RXBD_CNT);
    
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                 &BdTemplate1, FGMACPS_RECV);
    if (Status != FMSH_SUCCESS) {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error setting up RxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    /*
    * Setup TxBD space.
    *
    * Like RxBD space, we have already defined a properly aligned area
    * of memory to use.
    *
    * Also like the RxBD space, we create a template. Notice we don't
    * set the "last" attribute. The example will be overriding this
    * attribute so it does no good to set it up here.
    */
    FGmacPs_BdClear(&BdTemplate1);
    FGmacPs_BdSetStatus(&BdTemplate1, FGMACPS_TXBUF_USED_MASK);
    
    /*
    * Create the TxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                  (UINTPTR)TxBdSpacePtr, (UINTPTR)TxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, TXBD_CNT);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                     "Error setting up TxBD space, BdRingCreate");
      return FMSH_FAILURE;
    }
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                 &BdTemplate1, FGMACPS_SEND);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                     "Error setting up TxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    
    FGmacPs_BdClear(&BdRxTerminate1);
    FGmacPs_BdSetAddressRx(&BdRxTerminate1,
                           (FGMACPS_RXBUF_NEW_MASK | FGMACPS_RXBUF_WRAP_MASK));
    
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_RXQ1BASE_OFFSET,
                     (UINTPTR)&BdRxTerminate1);
    FGmacPs_BdClear(&BdTxTerminate1);
    FGmacPs_BdSetStatus(&BdTxTerminate1, (FGMACPS_TXBUF_USED_MASK |
                                          FGMACPS_TXBUF_WRAP_MASK));
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_TXQBASE_OFFSET, 
                     (UINTPTR)&BdTxTerminate1);
    
    
    /*
    * Set the Queue pointers
    */
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->RxBdRing.BaseBdAddr, 0, FGMACPS_RECV);
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->TxBdRing.BaseBdAddr, 1, FGMACPS_SEND);
    
    /*
    * Start the device
    */
    FGmacPs_Start(GmacPsInstancePtr);
    
    /*
    * Setup the interrupt controller and enable interrupts
    */
    Status = fmsh_gmac_verify_gic_setup(GmacPsInstancePtr);
    
    
    for (u32 i = 0; i < RXBD_CNT; i++) 
    {
      Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, &BdRxPtr);
      if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,("init_dma: Error allocating RxBD\r\n"));
        return Status;
      }
      Status = FGmacPs_BdRingToHw(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
      if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, ("Error: committing RxBD to HW\r\n"));
        FGmacPs_BdRingUnAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
        return Status;
      }
      
      //FGmacPs_BdSetAddressRx(BdRxPtr, (UINTPTR)&RxFrame_rxtx1[i]);             
#ifdef PSU_CACHE_ENABLE_GMAC
      Fmsh_DCacheInvalidateRange((UINTPTR)BdRxPtr, 64);
#endif                       
    }

    break;
  }
  
  case  FPAR_GMACPS_2_DEVICE_ID:{
    
    RxBdSpacePtr = &(bd_space_rxtx2[0]);
    TxBdSpacePtr = &(bd_space_rxtx2[0x5000]);
    
    GmacPsInstancePtr = &GmacPsInstance_rxtx2;
    GmacPsConfigPtr = &GmacPsInstancePtr->Config;
    PhyCfgPtr = &PhyCfg_rxtx2;
    frame_mem_init(&gmac_desc_2);
    /*************************************/
    /* Setup device for first-time usage */
    /*************************************/
    /*
    *  Initialize instance. Should be configured for DMA
    *  This example calls _CfgInitialize instead of _Initialize due to
    *  retiring _Initialize. So in _CfgInitialize we use
    *  XPAR_(IP)_BASEADDRESS to make sure it is not virtual address.
    */
    fmsh_gmac_verify_device_initial(GmacPsInstancePtr, GMAC_ID, mac_addr);
    
    /*
    * Setup the PL isolation if necessary
    */
    u32 REQ_ISO_INT_EN;
    u32 REQ_ISO_INT_TRIG;
    if (GmacPsInstancePtr->Config.InterFaceType == gmac_path_gmii)
    {
      // enable pmu service
      REQ_ISO_INT_EN = FGmacPs_ReadReg(0xFFD80000U, 0x0318U);
      REQ_ISO_INT_EN |= 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0318U, REQ_ISO_INT_EN);
      
      // trigger pmu service
      delay_ms(10);
      REQ_ISO_INT_TRIG = 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0320U, REQ_ISO_INT_TRIG);
    }
    
    
    if(xSemaphoreTake(phy_mutex,portMAX_DELAY) == pdTRUE){
      fmsh_gmac_verify_phy_initial(GmacPsInstancePtr, PhyCfgPtr);
      xSemaphoreGive(phy_mutex);
    }
    
    /*
    * Setup RxBD space.
    *
    * We have already defined a properly aligned area of memory to store
    * RxBDs at the beginning of this source code file so just pass its
    * address into the function. No MMU is being used so the physical
    * and virtual addresses are the same.
    *
    * Setup a BD template for the Rx channel. This template will be
    * copied to every RxBD. We will not have to explicitly set these
    * again.
    */
    FGmacPs_BdClear(&BdTemplate2);
    
    /*
    * Create the RxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                  (UINTPTR)RxBdSpacePtr, (UINTPTR)RxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, RXBD_CNT);
    
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                 &BdTemplate2, FGMACPS_RECV);
    if (Status != FMSH_SUCCESS) {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error setting up RxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    /*
    * Setup TxBD space.
    *
    * Like RxBD space, we have already defined a properly aligned area
    * of memory to use.
    *
    * Also like the RxBD space, we create a template. Notice we don't
    * set the "last" attribute. The example will be overriding this
    * attribute so it does no good to set it up here.
    */
    FGmacPs_BdClear(&BdTemplate2);
    FGmacPs_BdSetStatus(&BdTemplate2, FGMACPS_TXBUF_USED_MASK);
    
    /*
    * Create the TxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                  (UINTPTR)TxBdSpacePtr, (UINTPTR)TxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, TXBD_CNT);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                     "Error setting up TxBD space, BdRingCreate");
      return FMSH_FAILURE;
    }
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                 &BdTemplate2, FGMACPS_SEND);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                     "Error setting up TxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    
    FGmacPs_BdClear(&BdRxTerminate2);
    FGmacPs_BdSetAddressRx(&BdRxTerminate2,
                           (FGMACPS_RXBUF_NEW_MASK | FGMACPS_RXBUF_WRAP_MASK));
    
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_RXQ1BASE_OFFSET,
                     (UINTPTR)&BdRxTerminate2);
    FGmacPs_BdClear(&BdTxTerminate2);
    FGmacPs_BdSetStatus(&BdTxTerminate2, (FGMACPS_TXBUF_USED_MASK |
                                          FGMACPS_TXBUF_WRAP_MASK));
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_TXQBASE_OFFSET, 
                     (UINTPTR)&BdTxTerminate2);
    
    
    /*
    * Set the Queue pointers
    */
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->RxBdRing.BaseBdAddr, 0, FGMACPS_RECV);
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->TxBdRing.BaseBdAddr, 1, FGMACPS_SEND);
    
    /*
    * Start the device
    */
    FGmacPs_Start(GmacPsInstancePtr);
    
    /*
    * Setup the interrupt controller and enable interrupts
    */
    
    Status = fmsh_gmac_verify_gic_setup(GmacPsInstancePtr);
    
    
    for (u32 i = 0; i < RXBD_CNT; i++) 
    {
        Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, &BdRxPtr);
        if (Status != FMSH_SUCCESS) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,("init_dma: Error allocating RxBD\r\n"));
            return Status;
        }
        Status = FGmacPs_BdRingToHw(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
        if (Status != FMSH_SUCCESS) {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT, ("Error: committing RxBD to HW\r\n"));
            FGmacPs_BdRingUnAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
            return Status;
        }
        
        void *bd_addr = frame_mem_malloc(&gmac_desc_2);
        if (bd_addr == NULL)
        {
            GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,("mem malloc failed \r\n"));
            return FMSH_FAILURE;
        }
        u32 bdindex = FGMACPS_BD_TO_INDEX(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), BdRxPtr);
        Fmsh_DCacheInvalidateRange((UINTPTR)bd_addr, (UINTPTR)EthernetFrameSize);
        rx_frame_storage_2[bdindex] = (UINTPTR)bd_addr;
        FGmacPs_BdSetAddressRx(BdRxPtr, (UINTPTR)bd_addr);
        
    #ifdef PSU_CACHE_ENABLE_GMAC
        Fmsh_DCacheInvalidateRange((UINTPTR)BdRxPtr, 64);
    #endif                          
    }
    break;
  }
  
  case  FPAR_GMACPS_3_DEVICE_ID:{
    
    RxBdSpacePtr = &(bd_space_rxtx3[0]);
    TxBdSpacePtr = &(bd_space_rxtx3[0x5000]);
    
    GmacPsInstancePtr = &GmacPsInstance_rxtx3;
    GmacPsConfigPtr = &GmacPsInstancePtr->Config;
    PhyCfgPtr = &PhyCfg_rxtx3;
    
    
    
    /*************************************/
    /* Setup device for first-time usage */
    /*************************************/
    /*
    *  Initialize instance. Should be configured for DMA
    *  This example calls _CfgInitialize instead of _Initialize due to
    *  retiring _Initialize. So in _CfgInitialize we use
    *  XPAR_(IP)_BASEADDRESS to make sure it is not virtual address.
    */
    fmsh_gmac_verify_device_initial(GmacPsInstancePtr, GMAC_ID, mac_addr);
    
    /*
    * Setup the PL isolation if necessary
    */
    u32 REQ_ISO_INT_EN;
    u32 REQ_ISO_INT_TRIG;
    if (GmacPsInstancePtr->Config.InterFaceType == gmac_path_gmii)
    {
      // enable pmu service
      REQ_ISO_INT_EN = FGmacPs_ReadReg(0xFFD80000U, 0x0318U);
      REQ_ISO_INT_EN |= 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0318U, REQ_ISO_INT_EN);
      
      // trigger pmu service
      delay_ms(10);
      REQ_ISO_INT_TRIG = 0x4000U;
      Status = FGmacPs_WriteReg(0xFFD80000U, 0x0320U, REQ_ISO_INT_TRIG);
    }
    
    if(xSemaphoreTake(phy_mutex,portMAX_DELAY) == pdTRUE){
      fmsh_gmac_verify_phy_initial(GmacPsInstancePtr, PhyCfgPtr);
      xSemaphoreGive(phy_mutex);
    }
    /*
    * Setup RxBD space.
    *
    * We have already defined a properly aligned area of memory to store
    * RxBDs at the beginning of this source code file so just pass its
    * address into the function. No MMU is being used so the physical
    * and virtual addresses are the same.
    *
    * Setup a BD template for the Rx channel. This template will be
    * copied to every RxBD. We will not have to explicitly set these
    * again.
    */
    FGmacPs_BdClear(&BdTemplate3);
    
    /*
    * Create the RxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                  (UINTPTR)RxBdSpacePtr, (UINTPTR)RxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, RXBD_CNT);
    
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetRxRing(GmacPsInstancePtr)),
                                 &BdTemplate3, FGMACPS_RECV);
    if (Status != FMSH_SUCCESS) {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,"Error setting up RxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    /*
    * Setup TxBD space.
    *
    * Like RxBD space, we have already defined a properly aligned area
    * of memory to use.
    *
    * Also like the RxBD space, we create a template. Notice we don't
    * set the "last" attribute. The example will be overriding this
    * attribute so it does no good to set it up here.
    */
    FGmacPs_BdClear(&BdTemplate3);
    FGmacPs_BdSetStatus(&BdTemplate3, FGMACPS_TXBUF_USED_MASK);
    
    /*
    * Create the TxBD ring
    */
    Status = FGmacPs_BdRingCreate(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                  (UINTPTR)TxBdSpacePtr, (UINTPTR)TxBdSpacePtr,
                                  FGMACPS_BD_ALIGNMENT, TXBD_CNT);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                     "Error setting up TxBD space, BdRingCreate");
      return FMSH_FAILURE;
    }
    Status = FGmacPs_BdRingClone(&(FGmacPs_GetTxRing(GmacPsInstancePtr)),
                                 &BdTemplate3, FGMACPS_SEND);
    if (Status != FMSH_SUCCESS)
    {
      GMAC_TRACE_OUT(GMAC_DEBUG_OUT,
                     "Error setting up TxBD space, BdRingClone");
      return FMSH_FAILURE;
    }
    
    
    FGmacPs_BdClear(&BdRxTerminate3);
    FGmacPs_BdSetAddressRx(&BdRxTerminate3,
                           (FGMACPS_RXBUF_NEW_MASK | FGMACPS_RXBUF_WRAP_MASK));
    
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_RXQ1BASE_OFFSET,
                     (UINTPTR)&BdRxTerminate3);
    FGmacPs_BdClear(&BdTxTerminate3);
    FGmacPs_BdSetStatus(&BdTxTerminate3, (FGMACPS_TXBUF_USED_MASK |
                                          FGMACPS_TXBUF_WRAP_MASK));
    FGmacPs_WriteReg(GmacPsConfigPtr->BaseAddress, FGMACPS_TXQBASE_OFFSET, 
                     (UINTPTR)&BdTxTerminate3);
    
    
    /*
    * Set the Queue pointers
    */
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->RxBdRing.BaseBdAddr, 0, FGMACPS_RECV);
    FGmacPs_SetQueuePtr(GmacPsInstancePtr, GmacPsInstancePtr->TxBdRing.BaseBdAddr, 1, FGMACPS_SEND);
    
    /*
    * Start the device
    */
    FGmacPs_Start(GmacPsInstancePtr);
    
    /*
    * Setup the interrupt controller and enable interrupts
    */
    Status = fmsh_gmac_verify_gic_setup(GmacPsInstancePtr);
    
    
    for (u32 i = 0; i < RXBD_CNT; i++) 
    {
      Status = FGmacPs_BdRingAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, &BdRxPtr);
      if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT ,("init_dma: Error allocating RxBD\r\n"));
        return Status;
      }
      Status = FGmacPs_BdRingToHw(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
      if (Status != FMSH_SUCCESS) {
        GMAC_TRACE_OUT(GMAC_DEBUG_OUT, ("Error: committing RxBD to HW\r\n"));
        FGmacPs_BdRingUnAlloc(&(FGmacPs_GetRxRing(GmacPsInstancePtr)), 1, BdRxPtr);
        return Status;
      }
      
      //                FGmacPs_BdSetAddressRx(BdRxPtr, (UINTPTR)&RxFrame_rxtx3[i]);
      
      
#ifdef PSU_CACHE_ENABLE_GMAC
      Fmsh_DCacheInvalidateRange((UINTPTR)BdRxPtr, 64);
#endif                       
    }
    break;
   }
  }
  return 0;
}

void fmsh_gmac2_int_disable(u8 disable)
{
  g_gmac2_int_disable = disable;
  fmsh_print("fmsh_gmac2_int_disable: %u\r\n", disable);
}

void fmsh_gmac0_set_rxc_delay(uint16_t delay)
{
  FGmacPs *InstancePtr = &GmacPsInstance_rxtx0;
  FGmacPs_PhyConfig *PhyCfgPtr = &PhyCfg_rxtx0;

  uint16_t delay_read = 0;
  fmsh_print("jl2xx1_PhyWrite: 0x%x\r\n", delay);
  jl2xx1_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_171, PHY_JL2XX1_REG16, delay);
  vTaskDelay(2);
  jl2xx1_PhyRead(InstancePtr,  PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_171, PHY_JL2XX1_REG16, &delay_read);
  fmsh_print("jl2xx1_PhyRead: 0x%x\r\n", delay_read);
}