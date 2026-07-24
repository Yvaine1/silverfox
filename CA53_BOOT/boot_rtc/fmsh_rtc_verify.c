/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_rtc_verify.c
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
* 0.01   tyf  05/06/2023  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
//#include "verification_config.h"
#include "fmsh_rtc_verify.h"
#include "fmsh_rtc_mix.h"
#include "stdio.h"
#include "fmsh_psu_parameters.h"
#include "fmsh_rtc_lib.h"
#include "fmsh_Vcommon.h"
#include "psu_init.h"
#include <stdio.h>
#include <time.h>

#ifdef MPSOC_GTC
#include "fmsh_gtc_lib.h"
#endif

/************************** Constant Definitions *****************************/

#define RTC_REG_LENGTH 9
#define RTC_HR_REG_LENGTH 5
#define RTC_WRRDREG_LENGTH 3
#define RTC_BPD_REG_LENGTH 6

#define FRtcPsu_RoundOff(Number) \
	(u32)(((Number) < (float)0) ? ((Number) - (float)0.5) : \
		((Number) + (float)0.5))
/*
 *reg default information
 */
const RegInstance_TypeDef rtc_reg[RTC_REG_LENGTH] = {
{0x00000004, 0x00000000, 0xffffffff, 0x00000000}, //SET_TIME_READ
{0x0000000c, 0x00000000, 0x001fffff, 0x00000000}, //CALIB_READ
{0x00000010, 0x00000000, 0xffffffff, 0x00000000}, //CURRENT_TIME
{0x00000018, 0x00000001, 0xffffffff, 0x00000000}, //ALARM
{0x00000020, 0x00000001, 0x00000003, 0x00000000}, //RTC_INT_STATUS
{0x00000024, 0x00000000, 0x00000003, 0x00000003}, //RTC_INT_MASK
//{0x00000030, 0x00000001, 0x00000001, 0x00000000}, //ADDR_ERROR 
{0x00000034, 0x00000000, 0x00000001, 0x00000001}, //ADDR_ERROR_INT_MASK
{0x00000040, 0x00000001, 0x8f0000ff, 0x01000002}, //CONTROL
{0x00000050, 0x00000001, 0xffffffff, 0x00000000}, //Safety Check
};

/*
 *reg default information
 */
const RegInstance_TypeDef rtc_hr_reg[RTC_HR_REG_LENGTH] = {
{0x00000018, 0x00000001, 0xffffffff, 0x00000000}, //ALARM
{0x00000024, 0x00000000, 0x00000003, 0x00000003}, //RTC_INT_MASK
{0x00000034, 0x00000000, 0x00000001, 0x00000001}, //ADDR_ERROR_INT_MASK
{0x00000040, 0x00000001, 0x8f0000ff, 0x01000002}, //CONTROL
{0x00000050, 0x00000001, 0xffffffff, 0x00000000}, //Safety Check
};

const RegInstance_TypeDef rtc_wrrd_reg[RTC_WRRDREG_LENGTH] ={
{0x00000018, 0x00000001, 0xffffffff, 0x00000000}, //ALARM
{0x00000040, 0x00000001, 0x8f0000ff, 0x01000002}, //CONTROL
{0x00000050, 0x00000001, 0xffffffff, 0x00000000}, //Safety Check
};

const RegInstance_TypeDef rtc_wr_reg[RTC_BPD_REG_LENGTH] ={
{0x00000000, 0x00000002, 0xffffffff, 0x00000000}, //SET_TIME_WRITE
{0x00000008, 0x00000002, 0x0001ffff, 0x00000000}, //CALIB_WRITE
{0x00000028, 0x00000002, 0x00000003, 0x00000000}, //Interrupt Enable
{0x0000002c, 0x00000002, 0x00000003, 0x00000000}, //Interrupt Disable
{0x00000038, 0x00000002, 0x00000001, 0x00000000}, //Address Decode Error Interrupt Enable
{0x0000003c, 0x00000002, 0x00000001, 0x00000000}, //Address Decode Error Interrupt Disable
};

const RegInstance_TypeDef rtc_rd_reg[RTC_BPD_REG_LENGTH] ={
{0x00000004, 0x00000000, 0xffffffff, 0x00000000}, //SET_TIME_READ
{0x0000000c, 0x00000000, 0x0001ffff, 0x00000000}, //CALIB_READ
{0x00000024, 0x00000000, 0x00000003, 0x00000000}, //Interrupt MASK
{0x00000024, 0x00000000, 0x00000003, 0x00000003}, //Interrupt MASK
{0x00000034, 0x00000000, 0x00000001, 0x00000000}, //Address Decode Error Interrupt MASK
{0x00000034, 0x00000000, 0x00000001, 0x00000001}, //Address Decode Error Interrupt MASK
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static u8 rtc_caseFlag[100];

#ifdef MPSOC_GTC
extern FGtcPs_T g_GTC;
//u32 r_gtc_count0, r_gtc_count1;
uint64_t r_gtc_count0, r_gtc_count1;
f64 r_gtc_time[100];
#endif

/****************************************************************************/
/**
*
* This function is used to verify g_RTC, then send back to PC.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 fmsh_rtc_verify()
{
    u8 sts = 0;
    u8 ret = 0;
    u8 i;
    for (i = 0; i < 100; i++)
        rtc_caseFlag[i] = 0;
    
    //rtc_init();
 #if 0
    /**********CASE 1 2**********/
    sts = rtc_por_reset_verify(&g_RTC);
    if(sts !=0)
    {
        ret |= 1;
        rtc_caseFlag[1] = 0xe1;  
        fmsh_print("por reset verify fail\n\r");
    }
    else
    {
        rtc_caseFlag[1] = 0x55;
        fmsh_print("por reset verify pass\n\r");
    }
#endif
    
#if 0    
    /**********CASE 2**********/
    sts = rtc_hr_reset_verify(&g_RTC);
    if(sts != 0)
    {
        ret |= 1;
        rtc_caseFlag[2] = 0xe1;
        fmsh_print("hardware reset verify fail\n\r");
    }
    else
    {
        rtc_caseFlag[2] = 0x55;
        fmsh_print("hardware reset verify pass\n\r");
    }
#endif 

#if 1
    /**********CASE 3**********/   
    sts = rtc_RdWrReg_verify(&g_RTC);
    if(sts !=0)
    {
        ret |= 1;
        rtc_caseFlag[3] = 0xe1;     
        fmsh_print("rw reg verify fail\n\r");
    }
    else
    {
        rtc_caseFlag[3] = 0x55;
        fmsh_print("rw reg verify pass\n\r");
    }
#endif  
        
#if 1
    /**********CASE 2**********/
    sts = rtc_slcr_reset_verify(&g_RTC);
    if(sts !=0)
    {
        ret |= 1;
        rtc_caseFlag[1] = 0xe1;
        rtc_caseFlag[2] = 0xe1;     
        fmsh_print("slcr reset verify fail\n\r");
    }
    else
    {
        rtc_caseFlag[1] = 0x55;
        rtc_caseFlag[2] = 0x55;
        fmsh_print("slcr reset verify pass\n\r");
    }
#endif
    
#if 1
    /**********CASE 4**********/
    sts = rtc_integer_calib_verify(&g_RTC);
    if(sts !=0)
    {
        ret |= 1;
        rtc_caseFlag[4] = 0xe1;     
        fmsh_print("rtc integer calibration verify fail\n\r");
    }
    else
    {
        rtc_caseFlag[4] = 0x55;
        fmsh_print("rtc integer calibration verify pass\n\r");
    }
#endif
    
#if 1
    /**********CASE 5**********/
    sts = rtc_frac_calib_verify(&g_RTC);
    if(sts !=0)
    {
        ret |= 1;
        rtc_caseFlag[5] = 0xe1;     
        fmsh_print("rtc fractional calibration verify fail\n\r");
    }
    else
    {
        rtc_caseFlag[5] = 0x55;
        fmsh_print("rtc fractional calibration verify pass\n\r");
    }
    
#endif
    
#if 1
    /**********CASE 6**********/
    sts = rtc_alrm_int_verify(&g_RTC);
    if(sts !=0)
    {
        ret |= 1;
        rtc_caseFlag[6] = 0xe1;     
        fmsh_print("alarm interrupt verify fail\n\r");
    }
    else
    {
        rtc_caseFlag[6] = 0x55;
        fmsh_print("alarm interrupt verify pass\n\r");
    }
#endif
    
    return ret;
}

/****************************************************************************/
/**
*
* This function read the default value after system reset.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_regResetRead_verify(FRtcPs_T *dev)
{
    u32 i, reg;
    //reset readback
    FRtcPs_portmap_T *portmap;
    
    portmap = (FRtcPs_portmap_T *)dev->base_address; 
    
    for(i = 0; i < RTC_REG_LENGTH; i++)
    {
        reg = FMSH_ReadReg(portmap, rtc_reg[i].offset);
        if((reg & rtc_reg[i].active_bit) != rtc_reg[i].reset_value )
            return 1;
    }

    return 0;
}

/****************************************************************************/
/**
*
* hardware reset verify
*
* @param    dev is a pointer to g_RTC device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_hr_reset_verify(FRtcPs_T *dev)
{
    u32 i, reg;
    //reset readback
    FRtcPs_portmap_T *portmap;
    
    portmap = (FRtcPs_portmap_T *)dev->base_address; 
    for(i = 0; i < RTC_HR_REG_LENGTH; i++)
    {
        reg = FMSH_ReadReg(portmap, rtc_hr_reg[i].offset);
        if((reg & rtc_hr_reg[i].active_bit) != (rtc_hr_reg[i].reset_value & rtc_hr_reg[i].active_bit))
            return 1;
    }

    return 0;
}

/****************************************************************************/
/**
*
* slcr reset verify
*
* @param    dev is a pointer to g_RTC device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_slcr_reset_verify(FRtcPs_T *dev)
{
    u8 ret = 0;
    u32 reg_value;
    lpd_rtc_enter_apbRefRst();
    //reg_value = FMSH_IN32_32(0xFFA60030);
    //fmsh_print("The value of RTC ERROR value is 0x%x\n\r",reg_value);
    lpd_rtc_exit_apbRefRst();
    //reg_value = FMSH_IN32_32(0xFFA60030);
    //fmsh_print("The value of RTC ERROR value is 0x%x\n\r",reg_value);
    //FMSH_OUT32_32(0x1,0xFFA60030);
    //reg_value = FMSH_IN32_32(0xFFA60030);
    //fmsh_print("The value of RTC ERROR value is 0x%x\n\r",reg_value);
    ret |= rtc_regResetRead_verify(dev);
    //FMSH_OUT32_32(0x1,0xFFA60060);
    //reg_value = FMSH_IN32_32(0xFFA60030);
    //fmsh_print("The value of RTC ERROR value is 0x%x\n\r",reg_value);
    return ret;
}

/****************************************************************************/
/**
*
* por reset verify
*
* @param    dev is a pointer to g_RTC device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_por_reset_verify(FRtcPs_T *dev)
{
    u8 ret = 0;

    ret |= rtc_regResetRead_verify(dev);

    return ret;
}

/****************************************************************************/
/**
*
* This function write all effective bits 0/1 then read back check.
*
* @param    dev is a pointer to g_WDT device
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_RdWrReg_verify(FRtcPs_T *dev)
{
    u32 i, reg;
    u32 addr;
    u8 ret = 0;
    addr = FPS_RTC_BASEADDR;    
    
    //write all 0 and readback verify
    for(i = 0; i < RTC_WRRDREG_LENGTH; i++)
    {
        FMSH_WriteReg(addr, rtc_wrrd_reg[i].offset, 0x00000000);
        reg = FMSH_ReadReg(addr, rtc_wrrd_reg[i].offset);
        if( (reg & rtc_wrrd_reg[i].active_bit) != (0x00000000 & rtc_wrrd_reg[i].active_bit))
            ret |= 1;
    }
    
    //write all 1 and readback verify
    for(i = 0; i < RTC_WRRDREG_LENGTH; i++)
    {
        FMSH_WriteReg(addr, rtc_wrrd_reg[i].offset, 0xffffffff);
        reg = FMSH_ReadReg(addr, rtc_wrrd_reg[i].offset);
        if((reg & rtc_wrrd_reg[i].active_bit) != (0xffffffff & rtc_wrrd_reg[i].active_bit))
            ret |= 1;
    }
    
    //for SET_TIME_WRITESET_TIME_WRITE(READ) and CALIB_WRITE(READ) test
    lpd_rtc_enter_apbRefRst();
    lpd_rtc_exit_apbRefRst();
    //enable BPD
    FRtcPs_enableCounter(dev);
    //for SET_TIME_WRITE(READ)、CALIB_WRITE(READ) test
    //write all 1 and readback verify
    for(i = 0; i < RTC_BPD_REG_LENGTH-4; i++)
    {
        FMSH_WriteReg(addr, rtc_wr_reg[i].offset, 0xffffffff);
        reg = FMSH_ReadReg(addr, rtc_rd_reg[i].offset);
        if((reg & rtc_wr_reg[i].active_bit) != (0xffffffff & rtc_wr_reg[i].active_bit))
            ret |= 1;
    }
    //write all 0 and readback verify
    for(i = 0; i < RTC_BPD_REG_LENGTH-4; i++)
    {
        FMSH_WriteReg(addr, rtc_wr_reg[i].offset, 0x0);
        reg = FMSH_ReadReg(addr, rtc_rd_reg[i].offset);
        if((reg & rtc_wr_reg[i].active_bit) != (0x0 & rtc_wr_reg[i].active_bit))
            ret |= 1;
    }
    
    //for interrupt test
    for(i=RTC_BPD_REG_LENGTH-4; i<RTC_BPD_REG_LENGTH; i++)
    {
        FMSH_WriteReg(addr, rtc_wr_reg[i].offset, 0x3);
        reg = FMSH_ReadReg(addr, rtc_rd_reg[i].offset);
        if((reg & rtc_wr_reg[i].active_bit) != (rtc_rd_reg[i].reset_value & rtc_wr_reg[i].active_bit))
            ret |= 1;
    }
    return ret;
}

/****************************************************************************/
/**
*
* This function is used to verify the seconds integer calibration.
*
* @param    dev is a pointer to g_WDT device
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_integer_calib_verify(FRtcPs_T *dev)
{
    u32 reg = 0;
    u32 i;
    u16 s_val;
    uint64_t r_gtc_count0, r_gtc_count1;
    f64 r_gtc_time[100];
    rtc_time tm ={
    .tm_sec = 49,
    .tm_min = 24,
    .tm_hour = 9,
    .tm_mday = 8,
    .tm_mon = 5,
    .tm_year = 123,
    .tm_wday = 1,
    .tm_yday = 128,
    .tm_isdst = 0
    };
    FRtcPs_portmap_T *portmap;  
    rtc_time ctm;
    portmap = (FRtcPs_portmap_T *)dev->base_address;
    //enable BPD
    FRtcPs_enableCounter(dev);
    //FRtcPs_read_time(dev, &ctm);
    //fmsh_print("RTC记录的结束时的当前时间：%d年%d月%d日", ctm.tm_year+1900, ctm.tm_mon+1, ctm.tm_mday);  
    //fmsh_print(":%d:%d:%d\n", ctm.tm_hour, ctm.tm_min, ctm.tm_sec); 
    //set the current time
    FRtcPs_set_time(dev, &tm);
    FRtcPs_seconds_irq_enable(dev, 1);
    
    for (i = 0; i < 10; i++)
    {
        g_rtc_SecFlag = 0;
        //s_val = (i+1) * 320;
        s_val = 32768;
        //s_val = 65535;
        //suppose the input clock of rtc is 10KHZ
        FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, s_val);
        RTC_OUT32P(reg, portmap->CALIB_WRITE);  
        while(g_rtc_SecFlag == 0);
        g_rtc_SecFlag = 0;
        //FRtcPs_read_time(dev, &ctm);
        //fmsh_print("RTC记录的结束时的当前时间：%d年%d月%d日", ctm.tm_year+1900, ctm.tm_mon+1, ctm.tm_mday);  
        //fmsh_print(":%d:%d:%d\n", ctm.tm_hour, ctm.tm_min, ctm.tm_sec);  

        //r_gtc_count0 = FGtcPs_getConuterL(&g_GTC);
        r_gtc_count0 = get_current_time();
      
         while(g_rtc_SecFlag == 0);
        //r_gtc_count1 = FGtcPs_getConuterL(&g_GTC);
        r_gtc_count1 = get_current_time();
        //the time unit of gtc_time is ms
        r_gtc_time[i] = (r_gtc_count1 - r_gtc_count0) * 1000/ (f64)(GTC_FREQ);
        //r_gtc_time[i] = (r_gtc_count1 - r_gtc_count0);
        g_rtc_SecFlag = 0;

    }
    

    for (i = 0; i < 10; i++)
    {
    	fmsh_print("RTC Max Tick value %d: %f ms\r\n", 32768, r_gtc_time[i]);
        r_gtc_time[i] = 0;
    }

    
    return 0;
}

/****************************************************************************/
/**
*
* This function is used to verify the seconds fractional calibration.
*
* @param    dev is a pointer to g_WDT device
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_frac_calib_verify(FRtcPs_T *dev)
{
    u32 reg = 0;
    u32 i;
    rtc_time tm ={
    .tm_sec = 49,
    .tm_min = 24,
    .tm_hour = 9,
    .tm_mday = 8,
    .tm_mon = 5,
    .tm_year = 123,
    .tm_wday = 1,
    .tm_yday = 128,
    .tm_isdst = 0
    };
    FRtcPs_portmap_T *portmap; 
    uint64_t r_gtc_count0, r_gtc_count1;
    f64 r_gtc_time[100];    
    portmap = (FRtcPs_portmap_T *)dev->base_address;
    //enable BPD
    FRtcPs_enableCounter(dev);
    //set the current time
    //FRtcPs_set_time(dev, &tm);
    //set the calibration value, include integer 
    //FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, 10000); //1ms when input clk of rtc is 10MHz
    //FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, 1600); //1ms when input clk of rtc is 160KHz
    
    FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, 327); //1ms when input clk of rtc is 32KHz
    FMSH_BIT_SET(reg, CALIB_RW_Fraction_Data, 15);
    FMSH_BIT_SET(reg, CALIB_RW_Fraction_En, 1);
    
    RTC_OUT32P(reg, portmap->CALIB_WRITE);
      
    FRtcPs_seconds_irq_enable(dev, 1);
    g_rtc_SecFlag = 0;
    while(g_rtc_SecFlag == 0);
    g_rtc_SecFlag = 0;
    while(g_rtc_SecFlag == 0);
    for (i = 0; i < 32; i++)
    {
       g_rtc_SecFlag = 0;

       //r_gtc_count0 = FGtcPs_getConuterL(&g_GTC);
       r_gtc_count0 = get_current_time();

       while(g_rtc_SecFlag == 0);

       //r_gtc_count1 = FGtcPs_getConuterL(&g_GTC);
       r_gtc_count1 = get_current_time();
       //the time unit of gtc_time is ms
       r_gtc_time[i] = (r_gtc_count1 - r_gtc_count0) * 1000/ (f64)(GTC_FREQ);
        

    }
    

    for (i = 0; i < 32; i++)
    {
        fmsh_print("RTC Seconds value %d: %f ms\r\n", i, r_gtc_time[i]);
        r_gtc_time[i] = 0;
    }

    
    return 0;
}

/****************************************************************************/
/**
*
* This function is used to verify the alarm interrupt function.
*
* @param    dev is a pointer to g_WDT device
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 rtc_alrm_int_verify(FRtcPs_T *dev)
{
    u32 reg = 0;
    int i,y;
    u32 test1,test2;
    rtc_wkalrm alrm;
    rtc_time tm ={
    .tm_sec = 49,
    .tm_min = 24,
    .tm_hour = 9,
    .tm_mday = 8,
    .tm_mon = 5,
    .tm_year = 123,
    .tm_wday = 1,
    .tm_yday = 128,
    .tm_isdst = 0
    };
    rtc_time ctm;
    uint64_t r_gtc_count0, r_gtc_count1;
    f64 r_gtc_time[100]; 
    FRtcPs_portmap_T *portmap; 
    
    portmap = (FRtcPs_portmap_T *)dev->base_address;
    //enable BPD
    FRtcPs_enableCounter(dev);

    //FRtcPs_seconds_irq_enable(dev, 1);
#if 1
    
    for(y=0; y<1; y++)
    {
        for(i=0; i<20 ;i++)
        {
    
    /*
    for(y=1; y>-1; y--)
    {
        for(i=59; i>-1 ;i--)
        {  
    */ 
          
    //r_gtc_count0 = 0;
    //r_gtc_count1 = 0;
    test1 = 0;
    test2 = 0;
    g_rtc_SecFlag = 0;
    //set the calibration value, include integer 
    //for 1MHz
    //FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, 10000);
    //for 160KHz
    //FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, 1600);
    //for 32KHz
    FMSH_BIT_SET_NOREAD(reg, CALIB_RW_Max_Tick, 32768);
    FMSH_BIT_SET(reg, CALIB_RW_Fraction_Data, 0);
    //FMSH_BIT_SET(reg, CALIB_RW_Fraction_En, 1);
    RTC_OUT32P(reg, portmap->CALIB_WRITE);
    //set the current time
    FRtcPs_set_time(dev, &tm); 
    
    //disable seconds and alarm interrupt
    FRtcPs_seconds_irq_enable(dev, 0);
    FRtcPs_alarm_irq_enable(dev, 0);
    FRtcPs_clearSInterruptStatus(&g_RTC);
    FRtcPs_clearAInterruptStatus(&g_RTC);
    FRtcPs_seconds_irq_enable(dev, 1);
    while(g_rtc_SecFlag == 0);
    FRtcPs_seconds_irq_enable(dev, 0);
/*
#ifdef MPSOC_GTC
    r_gtc_count0 = FGtcPs_getConuterL(&g_GTC);
#endif
*/
    
    //read the current time from RTC
    FRtcPs_read_time(dev, &ctm);
    //set the alarm value (current time + 1 min)  
    //ctm.tm_min = ctm.tm_min + 2;
    //ctm.tm_sec = ctm.tm_sec + 1;
    //ctm.tm_min = ctm.tm_min + 2;
    //ctm.tm_sec = ctm.tm_sec + 1;
    ctm.tm_min = ctm.tm_min + y;
    ctm.tm_sec = ctm.tm_sec + i+1;
    alrm.time = ctm;
    //enable the alarm interrupt
    alrm.enabled = 1;
    g_rtc_AlrFlag = 0;
    FRtcPs_clearSInterruptStatus(&g_RTC);
    FRtcPs_clearAInterruptStatus(&g_RTC);
    FRtcPs_set_alarm(dev, &alrm);
    

    r_gtc_count0 = get_current_time();

    
    while(g_rtc_AlrFlag == 0);


    r_gtc_count1 = get_current_time();
    //the time unit of gtc_time is ms
    r_gtc_time[i] = (r_gtc_count1 - r_gtc_count0) * 1000/ (f64)(GTC_FREQ);
    //r_gtc_time[0] = (r_gtc_count1 - r_gtc_count0) / (f32)(GTC_CLK_FREQ * 1000); 
    
    //fmsh_print("the period between current time and alarm time is %f ms\r\n", r_gtc_time[y*60+i]);
    //fmsh_print("the period between current time and alarm time is %f ms\r\n", r_gtc_time[0]);
    fmsh_print("%f ms\r\n", r_gtc_time[y*60+i]);
    //r_gtc_time[0] = 0;


         }
    }
#endif

    return 0;
}
