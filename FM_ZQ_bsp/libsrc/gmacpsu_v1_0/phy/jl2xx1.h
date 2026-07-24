#ifndef _JL2XX1_H_
#define _JL2XX1_H_
/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_gmac_mdio.h"

/************************** Constant Definitions *****************************/

#define PHY_PAGE_SEL_REG		31 // select page
#define JL2XX1_PHY_ID1          0x2
#define JL2XX1_PHY_ID2          0x3
#define JL2XX1_PHY_ID1_VAL		0x937c
#define JL2XX1_PHY_ID2_VAL		0x4030
#define PHY_JL2XX1_PAGE_0	    0
#define PHY_JL2XX1_PAGE_18		18
#define PHY_JL2XX1_PAGE_181		181
#define PHY_JL2XX1_PAGE_171		171
#define PHY_JL2XX1_PAGE_3336	3336

#define PHY_JL2XX1_USER_CONFIG_REG		21
#define PHY_JL2XX1_REG0					0
#define PHY_JL2XX1_REG17				17
#define PHY_JL2XX1_REG16				16

u8 jl2xx1_setup (FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr);
u8 jl2xx1_detect (FGmacPs *InstancePtr);
u8 jl2xx1_PhyRead (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                   u16 RegisterNum, u16 *PhyDataPtr);
u8 jl2xx1_PhyWrite (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                    u16 RegisterNum, u16 PhyData);


#ifdef FREERTOS_CONFIG_H
  #define DELAY(time)     TaskDelay(time)
#else
  #define DELAY(time)     delay_ms(time)
#endif 

#endif 
