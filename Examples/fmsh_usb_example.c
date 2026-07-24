/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_usb_verify.c
*
* This file contains 
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   zzq  10/3/2023  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>
#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_usb_data.h"
#include "fmsh_usb_example.h"

int fmsh_usb_reg_test()
{
    u32 reg=0;
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x4000);
    if(reg!=0x1100080){
      printf("USB Capability Reg:%x\n",reg);
      return -1;
    }
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x4004);
    if(reg!=0x2000120){
      printf("USB HCSPARAMS1 Reg:%x\n",reg);
      return -1;
    }
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x4008);
    if(reg!=0x5A){
      printf("USB HCSPARAMS2 Reg:%x\n",reg);
      return -1;
    }
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x400c);
    if(reg!=0x40001){
      printf("USB HCSPARAMS3 Reg:%x\n",reg);
      return -1;
    }
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x4010);
    if(reg!=0x8007F05){
      printf("USB HCCPARAMS1 Reg:%x\n",reg);
      return -1;
    }
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x4014);
    if(reg!=0x3000){
      printf("USB DBOFF Reg:%x\n",reg);
      return -1;
    }
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x4018);
    if(reg!=0x1000){
      printf("USB RTSOFF Reg:%x\n",reg);
      return -1;
    }
    reg = FMSH_ReadReg(USB_REGS_BASE, 0x401c);
    if(reg!=0xDD){
      printf("USB HCCPARAMS2 Reg:%x\n",reg);
      return -1;
    }
    return 0;
}

/****************************************************************************/
/**
*
* usb verify 
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
int FUsbPsu_example()
{   
    u8 ret=0;
    u32 reg=0;
    u32 mask=0;


#if HOST_TEST
    FMSH_WriteReg(USB_SLCR, 0, 0x111);
#endif
    
#if DEVICE_TEST
    FMSH_WriteReg(USB_SLCR, 0, 0x112);//0x11102);
#endif
//
//    reg=FMSH_ReadReg(CRL_APB, USB_SOF_REF_CLK);
//    reg |= USB_SOF_CLKACT;
//    FMSH_WriteReg(CRL_APB, USB_SOF_REF_CLK, reg);
//    //bus clk
//    reg=FMSH_ReadReg(CRL_APB, USB_BUS_REF_CTRL);
//    reg |= USB_BUS_CLKACT;
//    FMSH_WriteReg(CRL_APB, USB_BUS_REF_CTRL, reg);
    
    //apb power brige
    reg=FMSH_ReadReg(CRL_APB, RST_LPD_TOP);
    mask = USB_APB_RESET | USB_PWRUP_RESET| USB_BRIDGE_RESET;
    reg |= mask;
    FMSH_WriteReg(CRL_APB, RST_LPD_TOP, reg);
  
    //apb brige
    reg=FMSH_ReadReg(CRL_APB, RST_LPD_TOP);
    mask = USB_APB_RESET|USB_BRIDGE_RESET;
    reg &= ~mask;
    FMSH_WriteReg(CRL_APB, RST_LPD_TOP, reg);
    //delay_ms(10);
    
//    reg = FMSH_ReadReg(USB_REGS_BASE,0xA118);
//    mask = (1<<10);
//    reg |= mask;
//    FMSH_WriteReg(USB_REGS_BASE, 0xA118, reg);
    
//    reg = FMSH_ReadReg(USB_REGS_BASE,0xA11C);
//    mask = (1<<26);
//    reg |= mask;
//    FMSH_WriteReg(USB_REGS_BASE, 0xA11C, reg);
//    
    reg=FMSH_ReadReg(CRL_APB, RST_LPD_TOP);
    mask = USB_APB_RESET|USB_PWRUP_RESET|USB_BRIDGE_RESET;
    reg &= ~mask;
    FMSH_WriteReg(CRL_APB, RST_LPD_TOP, reg);
    
#ifdef ULPI_RESET_MIO
    int offset=0x4*(u32)(ULPI_RESET_MIO/32);
    reg=FMSH_ReadReg(0XFF180204,offset);
    reg&=~(1<<(ULPI_RESET_MIO%32));
    FMSH_WriteReg(0XFF180204,offset,reg);
    
    offset=0x100*(ULPI_RESET_MIO/26);
    reg=FMSH_ReadReg(FPS_GPIO_BASEADDR,offset+0x4);
    reg|=(1<<(ULPI_RESET_MIO%26));
    FMSH_WriteReg(FPS_GPIO_BASEADDR,offset+0x04,reg);
    
    reg=FMSH_ReadReg(FPS_GPIO_BASEADDR,offset);
    reg&=~(1<<(ULPI_RESET_MIO%26));
    FMSH_WriteReg(FPS_GPIO_BASEADDR,offset,reg);
    delay_ms(100); 
    
    reg=FMSH_ReadReg(FPS_GPIO_BASEADDR,offset);
    reg|=(1<<(ULPI_RESET_MIO%26));
    FMSH_WriteReg(FPS_GPIO_BASEADDR,offset,reg);
#endif
    
#if HOST_TEST
    delay_ms(10);
    FMSH_WriteReg(USB_REGS_BASE, 0x3c,(1<<27)|(1<<26)|(1<<25)|(1<<24));
    reg = FMSH_ReadReg(USB_REGS_BASE,0x8490);
    if(reg != 0x2a0){
      printf("USB3.0 case test success\n");
      return 0;
    }
    assignXhciMemory();
    ret = hostEnumTest();
    //host_test();
#endif    

#if DEVICE_TEST 
    delay_ms(10);
    FMSH_WriteReg(USB_REGS_BASE, 0x3c, (1<<25)|(1<<24));
    //bot_app_Init(0);
    ret = Fmsh_InitDfu(0);
#endif 
    return ret;
}