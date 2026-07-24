#include "fmsh_gmac_mdio.h"
#include "jl2xx1.h"


u8 jl2xx1_PhyWrite (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                    u16 RegisterNum, u16 PhyData)
{
    LONG Status;

    Status = FGmacPs_PhyWrite(InstancePtr, PhyAddress, PHY_PAGE_SEL_REG,Page);
    
    Status |= FGmacPs_PhyWrite(InstancePtr, PhyAddress, RegisterNum,PhyData);


    return Status;
}


u8 jl2xx1_PhyRead (FGmacPs *InstancePtr, u16 PhyAddress, u16 Page,
                   u16 RegisterNum, u16 *PhyDataPtr)
{
    LONG Status;

    Status = FGmacPs_PhyWrite(InstancePtr, PhyAddress, PHY_PAGE_SEL_REG,
                              Page);
    Status |= FGmacPs_PhyRead(InstancePtr, PhyAddress, RegisterNum,
                              PhyDataPtr);

    return Status;
}

u8 jlxx1_detect (FGmacPs *InstancePtr)
{
    u16 PhyAddr;
    LONG Status;
    u16 PhyReg1;
    u16 PhyReg2;

    for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++)
    {
        Status = jl2xx1_PhyRead(InstancePtr, PhyAddr, PHY_JL2XX1_PAGE_0, JL2XX1_PHY_ID1,
                                &PhyReg1);
        Status |= jl2xx1_PhyRead(InstancePtr, PhyAddr, PHY_JL2XX1_PAGE_0, JL2XX1_PHY_ID2,
                                 &PhyReg2);
        if ((Status == FMSH_SUCCESS) && (PhyReg1 == JL2XX1_PHY_ID1_VAL) &&
            (PhyReg2 == JL2XX1_PHY_ID2_VAL))
        {
            /* Found a valid PHY address */
            PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                          "JL2XX1 or alike PHY detected, Addr%d.\r\n", PhyAddr);
            return PhyAddr;
        }
    }

    /* PhyAddr default to 0 */
    PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                  "JL2XX1 PHY detect fail, set phyaddr to 0.\r\n");
    return 0;
}

u8 jl2xx1_setup(FGmacPs *InstancePtr, FGmacPs_PhyConfig *PhyCfgPtr)
{
    u16 PhyReg;
    u32 Phy_timeout;
    LONG Status;
    u16 phy_reg_data;

    switch (InstancePtr->Config.InterFaceType)
    {
      case gmac_path_rgmii:
      {
          Status  = jl2xx1_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_18,   PHY_JL2XX1_USER_CONFIG_REG,0x4040);
          Status |= jl2xx1_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_18,   PHY_JL2XX1_REG0,0x9140);
          DELAY(1000);
          Status |= jl2xx1_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_181,  PHY_JL2XX1_REG17,0x140);
          DELAY(1000);
          Status |= jl2xx1_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_3336, PHY_JL2XX1_REG16,0x6);
          DELAY(1000);
          if (PhyCfgPtr->phy_address == 1)
          {
              Status |= jl2xx1_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_171,  PHY_JL2XX1_REG16,0x49b4);
          }
          else 
          {
              Status |= jl2xx1_PhyWrite(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_171,  PHY_JL2XX1_REG16,0x29b4);
          }
          
        //   jl2xx1_PhyRead(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_18,   PHY_JL2XX1_USER_CONFIG_REG,&phy_reg_data);
        //   fmsh_print("%s 0x%hhx 0x%hx \r\n",__func__,PhyCfgPtr->phy_address,phy_reg_data);
        //   jl2xx1_PhyRead(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_18,   PHY_JL2XX1_REG0,&phy_reg_data);
        //   fmsh_print("%s 0x%hhx 0x%hx \r\n",__func__,PhyCfgPtr->phy_address,phy_reg_data);
        //   jl2xx1_PhyRead(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_181,   PHY_JL2XX1_REG17,&phy_reg_data);
        //   fmsh_print("%s 0x%hhx 0x%hx \r\n",__func__,PhyCfgPtr->phy_address,phy_reg_data);
        //   jl2xx1_PhyRead(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_3336,   PHY_JL2XX1_REG16,&phy_reg_data);
        //   fmsh_print("%s 0x%hhx 0x%hx \r\n",__func__,PhyCfgPtr->phy_address,phy_reg_data);
        //   jl2xx1_PhyRead(InstancePtr, PhyCfgPtr->phy_address, PHY_JL2XX1_PAGE_171,   PHY_JL2XX1_REG16,&phy_reg_data);
        //   fmsh_print("%s 0x%hhx 0x%hx \r\n",__func__,PhyCfgPtr->phy_address,phy_reg_data);
                      
          
          PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG, "JL2XX1 RGMII setup OK \r\n");
          break;
      }
      default:
      {
          PHY_TRACE_OUT(FMSH_ENET_PHY_DEBUG,
                        "Error: JL2XX1 InterFaceType not recognized \r\n");
          Status = FMSH_FAILURE;
          break;
      }
    }

    return Status;
}

