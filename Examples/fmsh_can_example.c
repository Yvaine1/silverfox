/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_can_example.c
*
* This file contains a example of can.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   wfb  11/23/2018  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "fmsh_can_lib.h" 
#include "fmsh_gic.h"
#include "fmsh_slcr.h"  
#include "fmsh_common.h"
#include "fmsh_can_example.h"
/************************** Constant Definitions *****************************/
#define CAN_LOOP_TIMEOUT 1000  //us

#define CAN_BUAD_1MHZ 1000000
#define CAN_BUAD_500KHZ 500000
#define CAN_BUAD_250KHZ 250000
#define CAN_BUAD_100KHZ 100000
#define CAN_BUAD_50KHZ 50000
#define CAN_BUAD_25KHZ 25000

#define CANFD_BUAD_2MHZ 2000000
#define CANFD_BUAD_4MHZ 4000000
#define CANFD_BUAD_5MHZ 5000000
#define CANFD_BUAD_10MHZ 10000000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

FCanPs_T g_CAN0, g_CAN1;

u8 g_can_recv_intr_flag, g_can0_recv_intr_flag;
u8 g_can1rbuf[50];

/****************************************************************************/
/**
*
* This function is used to enable CAN0 and CAN1 Loop
*
* @param    loop_en: 1 if CAN0 and CAN1 Loop enabled, otherwise 0.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
void FSlcrPS_setCanLoop (u32 loop_en)
{
    u32 value;

    value = FMSH_ReadReg(FPS_IOU_SLCR_BASEADDR, 0x200);
    if (loop_en)
    {
        value |= 0x4;
    }
    else
    {
        value &= ~0x4;
    }
    FMSH_WriteReg(FPS_IOU_SLCR_BASEADDR, 0x200, value);
}

/****************************************************************************/
/**
*
* This function is used to register interrupt
*
* @param    dev is a pointer to the instance of device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
static u8 can_setHanlder(FCanPs_T* dev, u32 id, FMSH_InterruptHandler hanlder)
{
    FGicPs_Connect(&IntcInstance, id, hanlder,  dev);
    FGicPs_Enable(&IntcInstance, id); 

    return 0;
}

/****************************************************************************/
/**
*
* This function is used to handle g_CAN1 interrupt
*
* @param  None.
*  
* @return   None.
*
* @note     None.
*
****************************************************************************/
static void CAN1_interrupt_hanlder (void *InstancePtr)
{
    u32 reg;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) (&g_CAN1)->base_address; 
    reg = CAN_INP(portmap->reg_grp2);
    /*Check Rx interrupt and Processing the receive interrupt routine*/
    if(FMSH_BIT_GET(reg, CAN_RTIF_RIF))
    {
        g_can_recv_intr_flag = 1;
        /*write 0 to all interrupt flag to avoid independence clear flag*/
        reg = FCanPs_clearIntFlage(reg);
        /*write 1 to RIF to reset the interrupt flag*/
        FMSH_BIT_SET(reg, CAN_RTIF_RIF, 1);
        CAN_OUTP(reg, portmap->reg_grp2); 
    }
}

/****************************************************************************/
/**
*
* This function is used to handle g_CAN0 interrupt
*
* @param  None.
*  
* @return   None.
*
* @note     None.
*
****************************************************************************/
static void CAN0_interrupt_hanlder (void *InstancePtr)
{
    u32 reg;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) (&g_CAN0)->base_address; 
    reg = CAN_INP(portmap->reg_grp2);
    /*Check Rx interrupt and Processing the receive interrupt routine*/
    if(FMSH_BIT_GET(reg, CAN_RTIF_RIF))
    {
        g_can0_recv_intr_flag = 1;
        /*write 0 to all interrupt flag to avoid independence clear flag*/
        reg = FCanPs_clearIntFlage(reg);
        /*write 1 to RIF to reset the interrupt flag*/
        FMSH_BIT_SET(reg, CAN_RTIF_RIF, 1);
        CAN_OUTP(reg, portmap->reg_grp2); 
    }
}

/****************************************************************************/
/**
*
* This function print recived can frame on terminal.
*
* @param    rbuf is buffer of recived can frame data 
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
void Trace_out_ReciveFrameData(u32 *rbuf)
{       
    u32 *p;
    u32 dlc, control;
    u32 can_id_tmp, can_id;
    int i;
    u8 len;
    p = rbuf;
    control = p[1] & 0xff;
    can_id_tmp = p[0];
    dlc = control & CAN_FD_SET_DLC_MASK;
    if(control & CAN_FD_SET_EDL_MASK)
    {
        len = can_dlc2len(dlc);
    }
    else
    {
        len = get_can_dlc(dlc);
    }
    /*change the CANFD id into socketcan id format*/
    /*canfd*/
    if(control & CAN_FD_SET_EDL_MASK)
    {
        /*extended format*/
        can_id = can_id_tmp;
        if(control & CAN_FD_SET_IDE_MASK)
        {
            can_id |= CAN_EFF_FLAG;
        }
        else
        {
            can_id &= (~CAN_EFF_FLAG);
        }
    /*bit 29,error message not defined here*/      
    }
    /*can2.0*/
    else
    {
         can_id = can_id_tmp;
         if(control & CAN_FD_SET_IDE_MASK)
         {
             can_id |= CAN_EFF_FLAG;
         }
         else
         {
             can_id &= (~CAN_EFF_FLAG);
         }
         /*deal with RTR in can2.0*/
         if(control & CAN_FD_SET_RTR_MASK)
         {
             can_id |=CAN_RTR_FLAG ;
         }
    }
    CAN_TRACE_OUT(CAN_DEBUG_OUT, "socketcan can frame CAN ID is 0x%08x\n", can_id);
    /* Data*/
    /*CANFD frames*/
    if(control&CAN_FD_SET_EDL_MASK)
    {
        for (i = 0; i < len/4; i += 1) 
         {       
             CAN_TRACE_OUT(CAN_DEBUG_OUT, "CAN FD recive DATA 0x%08x\n", p[2+i]);
         }
    }
    else
    {
        /*skb reads the received datas, if the RTR bit not set.*/
	if(!(control&CAN_FD_SET_RTR_MASK))
        {    
            CAN_TRACE_OUT(CAN_DEBUG_OUT, "CAN 2.0 recive DATA 0x%08x\n", p[2]);
            CAN_TRACE_OUT(CAN_DEBUG_OUT, "CAN 2.0 recive DATA 0x%08x\n", p[3]);
        }
    }
}

/****************************************************************************/
/**
*
* This function communicte messages between &g_CAN0 & &g_CAN1.
* &g_CAN0 & &g_CAN1 inner loopback by set SLCR register
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 FCanPs_example(void)
{
  u32 i;
  u8 ret=FMSH_SUCCESS;
  u32 r_buf[20];
  u32 buf[2];
  u32 timeout_cnt = CAN_LOOP_TIMEOUT;
  
  FSlcrPS_setCanLoop(1);
  
  for(i = 0; i < 2; i++)
    buf[i] = 0x55 + i;
  
  FCanPs_Config* Config=NULL;
  Config= FCanPs_LookupConfig(FPAR_CANPS_0_DEVICE_ID);
  if(Config==NULL)
    return FMSH_FAILURE;
  ret=FCanPs_init(&g_CAN0, Config);
  if(ret!=FMSH_SUCCESS)
    return FMSH_FAILURE;
  
  FCanPs_setBaudRate(&g_CAN0, CAN_BUAD_1MHZ, CAN_BUAD_1MHZ);
  //FCanPs_setFilter(&g_CAN0, 0xffffffff, 0, se_acf_mode, 0x1234, CAN_set);
  
  Config= FCanPs_LookupConfig(FPAR_CANPS_1_DEVICE_ID);
  if(Config==NULL)
    return FMSH_FAILURE;
  ret=FCanPs_init(&g_CAN1, Config);
  if(ret!=FMSH_SUCCESS)
    return FMSH_FAILURE;
  
  FCanPs_setBaudRate(&g_CAN1, CAN_BUAD_1MHZ, CAN_BUAD_1MHZ);
  //FCanPs_setFilter(&g_CAN1, 0xffffffff, 0, se_acf_mode, 0x1234, CAN_set);
  
  can_setHanlder(&g_CAN1, CAN1_INT_ID, (FMSH_InterruptHandler)CAN1_interrupt_hanlder);
    
  //Set Interrupt Sources
  /*Recive Interrupt Enable*/
  FCanPs_setReciveInterrupt(&g_CAN1, CAN_set);
  /*Disable Other Interrupts*/
  FCanPs_setReciveBufferOverrunInterrupt(&g_CAN1, CAN_clear);
  FCanPs_setReceiveBufferFullInterrupt (&g_CAN1, CAN_clear);
  FCanPs_setReceiveBufferAlmostFullInterrupt (&g_CAN1, CAN_clear);
  FCanPs_setTransmissionPrimaryInterrupt (&g_CAN1, CAN_clear);
  FCanPs_setTransmissionSecondaryInterrupt (&g_CAN1, CAN_clear);
  FCanPs_setErrorInterrupt (&g_CAN1, CAN_clear);
  FCanPs_setArbitrationLostInterrupt(&g_CAN1, CAN_clear);
  FCanPs_setErrorPassiveInterrupt(&g_CAN1, CAN_clear);
  FCanPs_setBusErrorInterrupt(&g_CAN1, CAN_clear);
  FCanPs_setWatchTriggerInterrupt(&g_CAN1, CAN_clear);
  FCanPs_setTimeTriggerInterrupt(&g_CAN1, CAN_clear);
  
  g_can_recv_intr_flag = 0;
  FCanPs_setXmitMode(&g_CAN0, ptb_mode);
  FCanPs_FrameTransmit(&g_CAN0, 0x55, buf, 8, can2, CAN_clear);
  FCanPs_TPEtransmissionRequest(&g_CAN0);
  
  while(FCanPs_getTransmissionCompleteStatus(&g_CAN0) == 1)
  {
    delay_1us();
    timeout_cnt--;
    if(timeout_cnt == 0)
      return FMSH_FAILURE;
  }
  
  while(g_can_recv_intr_flag == 0);
  
  if(g_can_recv_intr_flag == 1)
  {
      g_can_recv_intr_flag = 0;
      FCanPs_frameReceive(&g_CAN1, r_buf);
      for(i = 0; i < 2; i++)
      { 
          if(r_buf[i+2] != buf[i])
          return FMSH_FAILURE;
      }
  }
  else
    return FMSH_FAILURE;
  
  FSlcrPS_setCanLoop(0);        
  
  return FMSH_SUCCESS;
}
/****************************************************************************/
/**
*
* This function uses can0 to send and recive message. the test need connect the can0 with can analyzer.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 FCan0Ps_example(void)
{
  u32 i;
  u8 ret=FMSH_SUCCESS;
  u32 r_buf[20];
  u32 buf[2];
  u32 timeout_cnt = CAN_LOOP_TIMEOUT;
  
  FSlcrPS_setCanLoop(0);
  
  for(i = 0; i < 2; i++)
    buf[i] = 0x55 + i;
  
  FCanPs_Config* Config=NULL;
  Config= FCanPs_LookupConfig(FPAR_CANPS_0_DEVICE_ID);
  if(Config==NULL)
    return FMSH_FAILURE;
  ret=FCanPs_init(&g_CAN0, Config);
  if(ret!=FMSH_SUCCESS)
    return FMSH_FAILURE;
  
  FCanPs_setBaudRate(&g_CAN0, CAN_BUAD_1MHZ, CAN_BUAD_1MHZ);
  FCanPs_setFilter(&g_CAN0, 0xffffffff, 0, se_acf_mode, 0x1234, CAN_set);
  
  can_setHanlder(&g_CAN0, CAN0_INT_ID, (FMSH_InterruptHandler)CAN0_interrupt_hanlder);
    
  //Set Interrupt Sources
  /*Recive Interrupt Enable*/
  FCanPs_setReciveInterrupt(&g_CAN0, CAN_set);
  /*Disable Other Interrupts*/
  FCanPs_setReciveBufferOverrunInterrupt(&g_CAN0, CAN_clear);
  FCanPs_setReceiveBufferFullInterrupt (&g_CAN0, CAN_clear);
  FCanPs_setReceiveBufferAlmostFullInterrupt (&g_CAN0, CAN_clear);
  FCanPs_setTransmissionPrimaryInterrupt (&g_CAN0, CAN_clear);
  FCanPs_setTransmissionSecondaryInterrupt (&g_CAN0, CAN_clear);
  FCanPs_setErrorInterrupt (&g_CAN0, CAN_clear);
  FCanPs_setArbitrationLostInterrupt(&g_CAN0, CAN_clear);
  FCanPs_setErrorPassiveInterrupt(&g_CAN0, CAN_clear);
  FCanPs_setBusErrorInterrupt(&g_CAN0, CAN_clear);
  FCanPs_setWatchTriggerInterrupt(&g_CAN0, CAN_clear);
  FCanPs_setTimeTriggerInterrupt(&g_CAN0, CAN_clear);
  
  g_can0_recv_intr_flag = 0;
  FCanPs_setXmitMode(&g_CAN0, ptb_mode);
  FCanPs_FrameTransmit(&g_CAN0, 0x55, buf, 8, can2, CAN_clear);
  FCanPs_TPEtransmissionRequest(&g_CAN0);
  
  while(FCanPs_getTransmissionCompleteStatus(&g_CAN0) == 1)
  {
    delay_1us();
    timeout_cnt--;
    if(timeout_cnt == 0)
      return FMSH_FAILURE;
  }
  
  while(g_can0_recv_intr_flag == 0);
  
  if(g_can0_recv_intr_flag == 1)
  {
      g_can0_recv_intr_flag = 0;
      FCanPs_frameReceive(&g_CAN0, r_buf);
      Trace_out_ReciveFrameData(r_buf);
  }
  else
    return FMSH_FAILURE;  
  
  return FMSH_SUCCESS;    
}