/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_board.c
 *
 * This file contains "boot_main.h".
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  01/01/2024  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"
#include "fmsh_i2c_lib.h"

/************************** Constant Definitions *****************************/
//#define FPS_BOARD_JFMZQ28DR_DEMO
//#define FPS_BOARD_JFMZQ2EG_DEMO

/**************************** Type Definitions *******************************/
FI2cPs_T gI2c0_dev={NULL};
FIicPs_Instance_T gI2c0_Instance;
FIicPs_Param_T I2c0_Param;

/***************** Macros (Inline Functions) Definitions *********************/
#define IIC_TCA6416A_ADDR          (0x20)
#if defined(FPS_BOARD_JFMZQ28DR_DEMO)
#define PS_GTR_LANE_SEL0  14
#define PS_GTR_LANE_SEL1  15
#define PS_GTR_LANE_SEL2  16
#define PS_GTR_LANE_SEL3  17
#define ULPI_RESET_MIO    65
#elif defined(FPS_BOARD_JFMZQ2EG_DEMO)
#define PS_GTR_LANE_SEL0  3
#define PS_GTR_LANE_SEL1  4
#define PS_GTR_LANE_SEL2  5
#define PS_GTR_LANE_SEL3  6
#define ULPI_RESET_MIO    7
#endif
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
#if defined(FPS_BOARD_JFMZQ28DR_DEMO) || defined(FPS_BOARD_JFMZQ2EG_DEMO)
static u32 I2C_ByteRead(FI2cPs_T *dev,u8 addr,u8* data)
{
  u32 cnt=0;
  u32 ret=FMSH_SUCCESS;
  FI2cPs_write(dev,addr);
  FI2cPs_issueReadStop(dev);
  while(FI2cPs_isRxFifoEmpty(dev) == true)
  {
    cnt++;
    if(cnt>100)
    {
      return FMSH_FAILURE;
    }
    delay_1ms();
  }
  *data=FI2cPs_read(dev);
  return ret;
}

static u32 I2C_ByteWrite(FI2cPs_T *dev,u8 regdata, u8 value)
{
  u32 cnt=0;
  FI2cPs_write(dev,regdata);
  
  FI2cPs_issueSTOP(dev,value);
  while(FI2cPs_isTxFifoEmpty(dev)==false)
  {
    cnt++;
    if(cnt>100)
    {
      return FMSH_FAILURE;
    }
    delay_1ms();
  }
  return FMSH_SUCCESS;
}

static u8 FI2c0Ps_DeviceInit(
                      FI2cPs_T *pDev,
                      void* pI2cInstance,
                      void* I2cParam )
{
  u8 ret=FMSH_SUCCESS;     
  FI2cPs_Config* Config=NULL;
  Config = FI2cPs_LookupConfig(FPAR_I2CPS_0_DEVICE_ID);
  if(Config==NULL){
    return FMSH_FAILURE;
  }
  ret=FI2cPs_init(pDev, Config,pI2cInstance,I2cParam);
  if(ret!=FMSH_SUCCESS){
    return ret;
  }
  return ret;
}

static void FI2cPs_Reset(FI2cPs_T *dev)
{
  u32 value = 0;
  if (dev->id == FPAR_I2CPS_0_DEVICE_ID)
  {
    value = FMSH_ReadReg(CRL_APB_BASEADDR, 0x238);
    value |= (1 << 9);
    FMSH_WriteReg(CRL_APB_BASEADDR, 0x238, value);
    delay_us(5);
    value = FMSH_ReadReg(CRL_APB_BASEADDR, 0x238);
    value &= ~(1 << 9);
    FMSH_WriteReg(CRL_APB_BASEADDR, 0x238, value);
  }
  else if (dev->id == FPAR_I2CPS_1_DEVICE_ID)
  {
    value = FMSH_ReadReg(CRL_APB_BASEADDR, 0x238);
    value |= (1 << 10);
    FMSH_WriteReg(CRL_APB_BASEADDR, 0x238, value);
    delay_us(5);
    value = FMSH_ReadReg(CRL_APB_BASEADDR, 0x238);
    value &= ~(1 << 10);
    FMSH_WriteReg(CRL_APB_BASEADDR, 0x238, value);
  }
  else
  {
    ;
  }
}

static void FI2cPs_MasterInit(FI2cPs_T *dev)
{
  /*disable the dev I2C device*/
  FI2cPs_disable(dev);
  
  FIicPs_PortMap_T *pPortmap = (FIicPs_PortMap_T *)dev -> base_address;
  
  /* Config  */  
  I2C_OUTP(0x1,pPortmap->fs_spklen);  //offset = 0xA0 IC_FS_SPKLEN
  I2C_OUTP(0x15,pPortmap->reserved1); //set sda hold time
  
  /* Set up the clock count register.  The argument I2C1_CLOCK is specified as the I2C dev input clock.*/
  FI2cPs_ClockSetup(dev, (dev->input_clock)/1000000);
  
  /* set the speed mode to standard*/
  FI2cPs_setSpeedMode(dev, I2c_speed_standard);
  
  /* use 7&10-bit addressing*/
  FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
  FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);
  
  /* enable restart conditions*/
  FI2cPs_enableRestart(dev);
  
  /* enable master FSM*/
  FI2cPs_enableMaster(dev);
  
  /* Use the start byte protocol with the target address when initiating transfer.*/
  FI2cPs_setTxMode(dev, I2c_tx_target);
  
  /* set target address to the I2C slave address*/
  FI2cPs_setTargetAddress(dev, IIC_TCA6416A_ADDR);
  
  /* clear Irq */
  FI2cPs_clearIrq(dev,I2c_irq_all);
  
  /*enable the dev I2C device*/
  FI2cPs_enable(dev);
  
}

static u32 FI2cPs_IIC0TCA6416Set(u8 sel,u8 mask,u8 value)
{
    u32 Status=FMSH_SUCCESS;  
    u8 reg=0;
    Status=I2C_ByteRead(&gI2c0_dev,sel==0?6:7,&reg);
    if(FMSH_FAILURE==Status){
       return Status;
    }
    I2C_ByteWrite(&gI2c0_dev,sel==0?6:7,reg&(~mask));
    Status=I2C_ByteRead(&gI2c0_dev,sel==0?2:3,&reg);
    if(FMSH_FAILURE==Status){
       return Status;
    }
    reg&=(~mask);
    I2C_ByteWrite(&gI2c0_dev,sel==0?2:3,reg|value);
    return Status;
}

static void FmshFsbl_usbPhyRst()
{
#ifdef ULPI_RESET_MIO
    u32 reg = 0;
    int offset = 0x4 * (u32)(ULPI_RESET_MIO / 32);
    reg = FMSH_ReadReg(0XFF180204, offset);
    reg &= ~(1 << (ULPI_RESET_MIO % 32));
    FMSH_WriteReg(0XFF180204, offset, reg);

    offset = 0x100 * (ULPI_RESET_MIO / 26);
    reg = FMSH_ReadReg(FPS_GPIO_BASEADDR, offset + 0x4);
    reg |= (1 << (ULPI_RESET_MIO % 26));
    FMSH_WriteReg(FPS_GPIO_BASEADDR, offset + 0x04, reg);

    reg = FMSH_ReadReg(FPS_GPIO_BASEADDR, offset);
#ifdef ULPI_RESET_H_ACTIVE
    reg |= (1 << (ULPI_RESET_MIO % 26));
#else
    reg &= ~(1 << (ULPI_RESET_MIO % 26));
#endif

    FMSH_WriteReg(FPS_GPIO_BASEADDR, offset, reg);
    delay_ms(10);
    reg = FMSH_ReadReg(FPS_GPIO_BASEADDR, offset);
#ifdef ULPI_RESET_H_ACTIVE
    reg &= ~(1 << (ULPI_RESET_MIO % 26));
#else
    reg |= (1 << (ULPI_RESET_MIO % 26));
#endif
    FMSH_WriteReg(FPS_GPIO_BASEADDR, offset, reg);
#endif
}
#endif
/*****************************************************************************/
/**
 * This function does board specific initialization.
 * Currently this is done for FPS_BOARD_JFMZQ28DR_DEMO board.
 * If there isn't any board specific initialization required, it just returns.
 *
 * @param none
 *
 * @return
 * 		- FMSH_SUCCESS for successful configuration
 * 		- errors as mentioned in fsbl_error.h
 *
 *****************************************************************************/
u32 FmshFsbl_BoardInit (void)
{
    u32 Status = FMSH_SUCCESS;

    /* Program I2C to configure GT lanes */
#if defined(FPS_BOARD_JFMZQ28DR_DEMO) || defined(FPS_BOARD_JFMZQ2EG_DEMO)
    u8 LaneSelVal=0;
    u8 mask=0;
    Status = FI2c0Ps_DeviceInit(&gI2c0_dev,&gI2c0_Instance,&I2c0_Param);
    if(FMSH_FAILURE==Status){
       return Status;
    }
    FI2cPs_Reset(&gI2c0_dev);
    FI2cPs_MasterInit(&gI2c0_dev);
    
    if(FPAR_GTRPSU_LANE0_PROTOCOL==4)
    {
      //DP
      LaneSelVal|=1<<(PS_GTR_LANE_SEL0%10);
      mask|=1<<(PS_GTR_LANE_SEL0%10);
    }
    if(FPAR_GTRPSU_LANE1_PROTOCOL==4)
    {
      //DP
      LaneSelVal|=1<<(PS_GTR_LANE_SEL1%10);
      mask|=1<<(PS_GTR_LANE_SEL1%10);
    }
    if(FPAR_GTRPSU_LANE2_PROTOCOL==2)
    {
      //USB
      LaneSelVal|=1<<(PS_GTR_LANE_SEL2%10);
      mask|=1<<(PS_GTR_LANE_SEL2%10);
    }
    if(FPAR_GTRPSU_LANE3_PROTOCOL==3)
    {
      //SATA
      LaneSelVal|=1<<(PS_GTR_LANE_SEL3%10);
      mask|=1<<(PS_GTR_LANE_SEL3%10);
    }

    Status = FI2cPs_IIC0TCA6416Set(PS_GTR_LANE_SEL0/10,mask,LaneSelVal);
    if(FMSH_FAILURE==Status){
       return Status;
    }
#endif    
    /* PCIE RESET */
    
    /* USB PHY RESET */
#if defined(FPS_BOARD_JFMZQ28DR_DEMO) || defined(FPS_BOARD_JFMZQ2EG_DEMO)
#ifdef FSBL_USB
    FmshFsbl_usbPhyRst();
#endif
#endif
    return Status;
}
