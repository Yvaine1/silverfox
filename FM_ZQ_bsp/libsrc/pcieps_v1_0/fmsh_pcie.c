#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "verification_config.h"

#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_pcie_verify.h"
#include "../gtrpsu_v1_0/fmsh_gtr.h"

#define TEST_ADDR   0x400

void pcie_reset_config(u8 reset, u32 mask)
{
	u32 value;

	value = FMSH_ReadReg(0xfd1a0000, RST_FPD_GTR);
	if (reset)
		value |= mask;
	else
		value &= ~mask;
	FMSH_WriteReg(0xfd1a0000, RST_FPD_GTR, value);
}


/* 0 - 128bytes, 1 - 256bytes */
void FSpcie_MaxPayLoadSizeChange(struct fmsh_pcie *pcie, u32 level)
{
    u32 DevCtrl = 0xc8;
    u32 mask = GENMASK(7, 5);
    u32 value;

    value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, DevCtrl);
    value &= (~mask);
    value |= level << 5;
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, DevCtrl, value);

    value = FSpcie_CfgRead(pcie, 1, 0, 0, DevCtrl);
    value &= (~mask);
    value |= level << 5;
    FSpcie_CfgWrite(pcie, 1, 0, 0, DevCtrl, value);
}

void FSpcie_FuncLevelReset(struct fmsh_pcie *pcie, u16 func)
{
    u32 value;
    u32 DevCtrl = 0xc8;
    u32 mask = BIT(15);
  
    value = FSpcie_CfgRead(pcie, 1, 0, func, DevCtrl);
    FSpcie_CfgWrite(pcie, 1, 0, func, DevCtrl, value | mask);
}

void FSpcie_MsgSend(struct fmsh_pcie *pcie, u32 msg_code)
{
    FMSH_WriteReg(pcie->msg_base, msg_code, 0x0);
}

void FSpcie_VdMsgSend(struct fmsh_pcie *pcie, u32 msg_code)
{
    FMSH_WriteReg(pcie->vd_msg_base, msg_code, 0x0);
}

 u32 FSpcie_CfgRead(struct fmsh_pcie *pcie, u16 bus, u16 dev, u16 func, u32 reg)
{
    u32 var;
    u32 offset;

    offset = (((bus)   & 0xffU ) << 20) | \
             (((dev)   & 0x1fU ) << 15) | \
             (((func)  & 0x7U  ) << 12) | \
             (((reg)   & 0xfffU));

    var = FMSH_ReadReg(pcie->cfg_base, offset);

    return var;
}

void FSpcie_CfgWrite(struct fmsh_pcie *pcie, u16 bus, u16 dev, u16 func, u32 reg, u32 data)
{
    u32 offset;

    offset = (((bus)   & 0xffU ) << 20) | \
             (((dev)   & 0x1fU ) << 15) | \
             (((func)  & 0x7U  ) << 12) | \
             (((reg)   & 0xfffU));

    FMSH_WriteReg(pcie->cfg_base, offset, data);
}

u32 FSpcie_AxiCfgRead(u16 func, u32 reg)
{
	return FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(func), reg);
}

 void FSpcie_AxiCfgWrite(u16 func, u32 reg, u32 data)
 {
	FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(func), reg, data);
 }

  u32 FSpcie_AriCfgRead(struct fmsh_pcie *pcie, u16 bus, u16 func, u32 reg)
 {
    u32 var;
    u32 offset;
 
    offset = (((bus)   & 0xffU ) << 20) | \
             (((func)  & 0xffU	) << 12) | \
             (((reg)   & 0xfffU));
 
    var = FMSH_ReadReg(pcie->cfg_base, offset);
    return var;
 }
 
 void FSpcie_AriCfgWrite(struct fmsh_pcie *pcie, u16 bus, u16 func, u32 reg, u32 data)
 {
    u32 offset;
 
    offset = (((bus)   & 0xffU ) << 20) | \
             (((func)  & 0xffU	) << 12) | \
             (((reg)   & 0xfffU));
 
    FMSH_WriteReg(pcie->cfg_base, offset, data);
 }

u32 FSpcie_MemRead(struct fmsh_pcie *pcie, u32 offset)
{
    return FMSH_ReadReg(pcie->mem_base, offset);
}

void FSpcie_MemWrite(struct fmsh_pcie *pcie, u32 offset, u32 data)
{
    FMSH_WriteReg(pcie->mem_base, offset, data);
}

 u32 pcie_ddr_read(u32 offset)
{
    return FMSH_ReadReg(PCIE_HIGH_BASE1, offset);
}

 void pcie_ddr_write(u32 offset, u32 data)
{
    FMSH_WriteReg(PCIE_HIGH_BASE1, offset, data);
}

 u32 pcie_ocm_read(u32 offset)
{
    return FMSH_ReadReg(PCIE_HIGH_BASE0, offset);
}

 void pcie_ocm_write(u32 offset, u32 data)
{
    FMSH_WriteReg(PCIE_HIGH_BASE0, offset, data);
}

void FSpcie_MsixSend(u32 offset, u32 data)
{
    if (offset > 0xc000) {
      printf("%s : invalid input\n", __func__);
      return;
    }
    FMSH_WriteReg(PCIE_LOW_BASE, 0x4000 + offset, data);
}

void Pcie_PoisonMemWriteSend()
{
    u32 value;

    //send poison TLP
    value = FMSH_ReadReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(1) + OUTBOUND_DESC0);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(1) + OUTBOUND_DESC0, value | BIT(20));
    pcie_ocm_write(0x1000, 0xae860000);

    //restore
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(1) + OUTBOUND_DESC0, value);
}

/* 
 * 0 - 128bytes, 1 - 256bytes, 2 - 512bytes
 * 3 - 1024bytes, 4 - 2048bytes, 5 - 4096bytes
 */
void FSpcie_MaxReadRequestSizeChange(u32 level)
{
    u32 DevCtrl = 0xc8;
    u32 mask = GENMASK(14, 12);
    u32 value;

    value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, DevCtrl);
    value &= (~mask);
    value |= level << 12;
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, DevCtrl, value);
}

void FSpcie_LinkInfoShow()
{
    u32 value;
    u32 mask;
    u32 LinkSts = 0xd0;
    
    value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, LinkSts);
    
    //link speed
    mask = GENMASK(19, 16);
    printf("Negotiated Link Speed : GEN%d\n", (value & mask) >> 16);
    
    //link width
    mask = GENMASK(25, 20);
    printf("Negotiated Link Width : %d\n", (value & mask) >> 20);  
}

u32 pcie_get_ltssm_sts()
{
    u32 value;
    u32 mask = GENMASK(29, 24);

    value = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, 0);
    value &= mask;
    return (value >> 24);
}

u32 FSpcie_LtssmTest()
{
    u32 lts = pcie_get_ltssm_sts();

    printf("LTSSM : ");
    switch (lts) {
    case PCIE_DETECT_QUIET                   : printf (" DETECT_QUIET"                  ); break;
    case PCIE_DETECT_ACTIVE                  : printf (" DETECT_ACTIVE"                 ); break;
    case PCIE_POLLING_ACTIVE                 : printf (" POLLING_ACTIVE"                ); break;
    case PCIE_POLLING_COMPLIANCE             : printf (" POLLING_COMPLIANCE"            ); break;
    case PCIE_POLLING_CONFIGURATION          : printf (" POLLING_CONFIGURATION"         ); break;
    case PCIE_CONFIGURATION_LINKWIDTH_START  : printf (" CONFIGURATION_LINKWIDTH_START" ); break;
    case PCIE_CONFIGURATION_LINKWIDTH_ACCEPT : printf (" CONFIGURATION_LINKWIDTH_ACCEPT"); break;
    case PCIE_CONFIGURATION_LANENUM_ACCEPT   : printf (" CONFIGURATION_LANENUM_ACCEPT"  ); break;
    case PCIE_CONFIGURATION_LANENUM_WAIT     : printf (" CONFIGURATION_LANENUM_WAIT"    ); break;
    case PCIE_CONFIGURATION_COMPLETE         : printf (" CONFIGURATION_COMPLETE"        ); break;
    case PCIE_CONFIGURATION_IDLE             : printf (" CONFIGURATION_IDLE"            ); break;
    case PCIE_RECOVERY_RCVRLOCK              : printf (" RECOVERY_RCVRLOCK"             ); break;
    case PCIE_RECOVERY_SPEED                 : printf (" RECOVERY_SPEED"                ); break;
    case PCIE_RECOVERY_RCVRCFG               : printf (" RECOVERY_RCVRCFG"              ); break;
    case PCIE_RECOVERY_IDLE                  : printf (" RECOVERY_IDLE"                 ); break;
    case PCIE_L0                             : printf (" L0"                            ); break;
    case PCIE_RX_L0S_ENTRY                   : printf (" RX_L0S_ENTRY"                  ); break;
    case PCIE_RX_L0S_IDLE                    : printf (" RX_L0S_IDLE"                   ); break;
    case PCIE_RX_L0S_FTS                     : printf (" RX_L0S_FTS"                    ); break;
    case PCIE_TX_L0S_ENTRY                   : printf (" TX_L0S_ENTRY"                  ); break;
    case PCIE_TX_L0S_IDLE                    : printf (" TX_L0S_IDLE"                   ); break;
    case PCIE_TX_L0S_FTS                     : printf (" TX_L0S_FTS"                    ); break;
    case PCIE_L1_ENTRY                       : printf (" L1_ENTRY"                      ); break;
    case PCIE_L1_IDLE                        : printf (" L1_IDLE"                       ); break;
    case PCIE_L2_IDLE                        : printf (" L2_IDLE"                       ); break;
    case PCIE_L2_TRANSMITWAKE                : printf (" L2_TRANSMITWAKE"               ); break;
    case PCIE_DISABLED                       : printf (" DISABLED"                      ); break;
    case PCIE_LOOPBACK_ENTRY_MASTER          : printf (" LOOPBACK_ENTRY_MASTER"         ); break;
    case PCIE_LOOPBACK_ACTIVE_MASTER         : printf (" LOOPBACK_ACTIVE_MASTER"        ); break;
    case PCIE_LOOPBACK_EXIT_MASTER           : printf (" LOOPBACK_EXIT_MASTER"          ); break;
    case PCIE_LOOPBACK_ENTRY_SLAVE           : printf (" LOOPBACK_ENTRY_SLAVE"          ); break;
    case PCIE_LOOPBACK_ACTIVE_SLAVE          : printf (" LOOPBACK_ACTIVE_SLAVE"         ); break;
    case PCIE_LOOPBACK_EXIT_SLAVE            : printf (" LOOPBACK_EXIT_SLAVE"           ); break;
    case PCIE_HOT_RESET                      : printf (" HOT_RESET"                     ); break;
    case PCIE_RECOVERY_EQ_PHASE_0  : printf (" RECOVERY_EQUALIZATION_PHASE_0" ); break;
    case PCIE_RECOVERY_EQ_PHASE_1  : printf (" RECOVERY_EQUALIZATION_PHASE_1" ); break;
    case PCIE_RECOVERY_EQ_PHASE_2  : printf (" RECOVERY_EQUALIZATION_PHASE_2" ); break;
    case PCIE_RECOVERY_EQ_PHASE_3  : printf (" RECOVERY_EQUALIZATION_PHASE_3" ); break;
    default: return 0;
    }
    printf("\n");
    
    if(lts != PCIE_L0)
      return FMSH_FAILURE;
    
    FSpcie_LinkInfoShow();
    return FMSH_SUCCESS;
}

u32 FSpcie_PerstStatusShow()
{
    u32 value;
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(572));
    value &= BIT(0);
    if (value)
      printf("PERST# high\n");
    else
      printf("PERST# low\n");
    return value;
}

void FSpcie_MsgFifoFilterErr()
{
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, (~PCIE_ERR_MSG_FWD));
}

void FSpcie_MsgFifoFilterPm()
{
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, (~PCIE_PM_MSG_FWD));
}

void FSpcie_MsgFifoFilterSlt()
{
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, (~PCIE_SLT_MSG_FWD));
}

void FSpcie_MsgFifoFilterOth()
{
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, (~PCIE_OTH_MSG_FWD));
}

void PSpcie_MsgFifoFilterIntx()
{
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, (~PCIE_INT_MSG_FWD));
}

void FSpcie_MsgFifoFilterVendor()
{
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, (~PCIE_VEN_MSG_FWD));
}

void FSpcie_MsgFifoPerpare()
{
    u32 value;
    u32 DevCtrl2 = 0xe8;

    /* disable INTx */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_DIS, 0xf);

    /* enable fifo receiver INTx msg */
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, value | PCIE_INT_MSG_FWD);

    /* enable LTR */
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE, DevCtrl2, 0x2400);

    /* enable slot power limit */
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_DEBUG_MUX_CONTROL_REG, 0x82000000);
}

void Pcie_DmaErrorHandler(u32 channel)
{
    u32 i;
    u32 status, num = 0;
    struct PCIE_DmaDesc *desc = (struct PCIE_DmaDesc *) pcie_dma_desc_base;
  
    while(1) {
      status = desc[num].status.chnl_status;
      printf("[ Descriptor %d ] : ", num);
      for (i = 0; i < 8; i++) {
        switch (status & BIT(i)) {
        case BIT(0):
          printf("Descriptor action completed\n");
          break;
        case BIT(1):
          printf("PCIe transfer completed early with incomplete data\n");
          break;
        case BIT(2):
          printf("AXI transfer completed early with incomplete data\n");
          break;
        case BIT(3):
          printf("Data Integrity Error, Internal data integrity detected (when accessing internal RAMs)\n");
          break;
        case BIT(4):
          printf("Descriptor Error, An invalid decode of the descriptor was detected\n");
          break;
        case BIT(5):
          printf("Buffer Overflow, When gathering data more data is required than the size of the buffer\n");
          break;
        case BIT(6):
          printf("Buffer Underflow, When scattering data more data is required than the size of the buffer\n");
          break;
        case BIT(7):
          printf("Buffer Not Empty, There is outstanding data in the buffer at the end of executing a linked-list\n");
          break;
        }
      }
      if ((desc[num].next == 0) && (desc[num].next_hi_addr == 0))
        return;
      num++;
    }
}

void Pcie_DmaIntrHandler()
{
    u32 value, channel, ctrl;
    u32 mask = 0x10;

#if 0
    FMSH_WriteReg(0xff260000, 0, 0);
    printf("lower : 0x%x\n", FMSH_ReadReg(0xff260000, 0x8));
    printf("high : 0x%x\n", FMSH_ReadReg(0xff260000, 0xc));
#endif
     /* disable slcr dma */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_DIS, mask);
    
    /* clear DMA status */
    value = FMSH_ReadReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_STS);
    FMSH_WriteReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_STS, value);
    
    /* clear slcr dma */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_STS, mask);
    
    for (channel = 0; channel < 8; channel++) {
      if (value & PCIE_DONE_INTR_BIT(channel)) {
        ctrl =  FMSH_ReadReg(PCIE_UDMA_REG_BASE, PCIE_DMA_CHANNEL_OFFSET(channel));
        printf("DMA channel %d %s DONE!\n",
                         channel, (ctrl & BIT(1)) ? "Outbound" : "Inbound");
      }

      if (value & PCIE_ERROR_INTR_BIT(channel)) {
        ctrl =  FMSH_ReadReg(PCIE_UDMA_REG_BASE, PCIE_DMA_CHANNEL_OFFSET(channel));
        printf("DMA channel %d %s ERROR!\n",
                         channel, (ctrl & BIT(1)) ? "Outbound" : "Inbound");
        Pcie_DmaErrorHandler(channel);
      }
    }
    
    /* ensable slcr dma */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_EN, mask);
}

void Pcie_MsgInfoDecode(struct pcie_msg_info msg)
{
    u8 MsgCode;
    u16 ReqID, SteerOrTag;
    u8 Routing, Tag, TPH, vendor;
    u8 MsgD = FALSE;

    vendor = (u8)(msg.header[0] & BIT(0));
    MsgCode = (u8)(msg.header[0] >> 24);

    printf("[ MSG type     ] :");
    switch (MsgCode) {
    case PCIE_MSG_UNLOCK		: printf (" Unlock"); break;
    case PCIE_INVALID_REQ_MSG		: printf (" Invalidate Request Message"); break;
    case PCIE_INVALID_CPL_MSG		: printf (" Invalidate Completion Message"); break;
    case PCIE_PAGE_REQ_MSG 		: printf (" Page Request Message"); break;
    case PCIE_PRG_REQ_MSG		: printf (" PRG Response Message"); break;
    case PCIE_LTR_MSG			: printf (" Latency Tolerance Reporting Message"); break;
    case PCIE_OBFF_MSG			: printf (" OBFF"); break;
    case PCIE_PM_ACTIVE_STATE_NAK	: printf (" PM_Active_State_Nak"); break;
    case PCIE_PM_PME_MSG		: printf (" PM PME"); break;
    case PCIE_PME_TURN_OFF		: printf (" PME_Turn_off"); break;
    case PCIE_PME_TO_ACK		: printf (" PME_To_Ack"); break;
    case PCIE_ASSERT_INTA		: printf (" Assert_INTA"); break;
    case PCIE_ASSERT_INTB		: printf (" Assert_INTB"); break;
    case PCIE_ASSERT_INTC		: printf (" Assert_INTC"); break;
    case PCIE_ASSERT_INTD		: printf (" Assert_INTD"); break;
    case PCIE_DEASSERT_INTA		: printf (" Deassert_INTA"); break;
    case PCIE_DEASSERT_INTB		: printf (" Deassert_INTB"); break;
    case PCIE_DEASSERT_INTC		: printf (" Deassert_INTC"); break;
    case PCIE_DEASSERT_INTD		: printf (" Deassert_INTD"); break;
    case PCIE_ERR_CORR			: printf (" ERR_CORR"); break;
    case PCIE_ERR_NONFATAL		: printf (" ERR_NONFATAL"); break;
    case PCIE_ERR_FATAL			: printf (" ERR_FATAL"); break;
    case PCIE_SET_SLOT_POWER_LIMIT	: printf (" Set_Slot_Power_Limit"); break;
    case PCIE_PTM_REQ			: printf (" PTM Request"); break;
    case PCIE_PTM_RESPONSE		: printf (" PTM Response"); break;
    case PCIE_VENDOR_DEFINE_TYPE0	: printf (" Vendor Defined Msg Type0"); break;
    case PCIE_VENDOR_DEFINE_TYPE1	: printf (" Vendor Defined Msg Type1"); break;
    case 0x40: printf (" Ignored message"); break;
    case 0x41: printf (" Ignored message"); break;
    case 0x43: printf (" Ignored message"); break;
    case 0x44: printf (" Ignored message"); break;
    case 0x45: printf (" Ignored message"); break;
    case 0x47: printf (" Ignored message"); break;
    default:   printf (" Unknow message"); break;
    }printf(" \n");

    Routing = (u8)((msg.header[0] & GENMASK(6, 4)) >> 4);
    printf("[ Routing info ] : ");
    switch (Routing) {
    case 0: printf(" Routed to Root Complex"); break;
    case 1: printf(" Routed by Address"); break;
    case 2: printf(" Routed by ID"); break;
    case 3: printf(" Broadcast from Root Complex"); break;
    case 4: printf(" Local - Terminate at Receiver"); break;
    case 5: printf(" Gathered and routed to Root Comple"); break;
    default: printf(" Reserved\n"); break;
    } printf("\n");

    ReqID = (u16)((msg.header[0] & GENMASK(23, 8)) >> 8);
    printf("[ Request id   ] : %x\n", ReqID);

    TPH = (u8)(msg.header[1] & BIT(0));
    SteerOrTag = (u16)((msg.header[1] & GENMASK(19, 4)) >> 4);
    if (TPH)
        printf("[ TPH steering (%s) ] : 0x%x\n",
            (msg.header[1] & BIT(1)) ? "16-bit" : "8-bit", SteerOrTag);

    Tag = (u8)((msg.header[1] & GENMASK(27, 20)) >> 20);
    if(vendor) {
        if(!TPH)
            printf("[ PCIe Tag     ] : %d\n", SteerOrTag);
        printf("[ Invalidation request msg (VD) ] : 0x%x\n", Tag);
        printf("[ Vendor defined msg header ] : 0x%x 0x%x\n", msg.data[2], msg.data[3]);
	} else
        printf("[ PCIe Tag     ] : %d\n", Tag);

    if (msg.status[0]) {
        (msg.status[0] & BIT(16)) ? (MsgD = TRUE) : (MsgD = FALSE);
        if (msg.status[0] & 0xffff)
            printf("[ MSG Byte Enbale ] : 0x%x\n", msg.status[0] & 0xffff);
    }

    if (msg.status[1]) {
      if (msg.status[1] & BIT(22)) {
         msg.status[1] &= GENMASK(21, 0);
         printf("[ MSG PASID    ] : 0x%x\n", msg.status[1]);
      }
    }

    if(MsgD) 
        printf("[ MsgD info (DW0-3) ] : 0x%x 0x%x 0x%x 0x%x\n",
                msg.data[0], msg.data[1], msg.data[2], msg.data[3]);

}

void Pcie_MsgFifoHandler()
{
    u32 value;
    u32 pending;
    struct pcie_msg_info msg;
    
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_DIS,
                  PCIE_FIFO_AVAIL | PCIE_FIFO_OVERFLOW | PCIE_FIFO_RECEIVED);
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_STS);
    pending = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_PENDING_NUM); 
    
    if (value & PCIE_FIFO_OVERFLOW) {
      printf("MSG overflow\n");
      if (pending != PCIE_FIFO_SIZE - 1) 
        printf("error, pending num is %d\n", pending);
    }
    
    do {
      msg.header[0] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_HEADER0);
      msg.header[1] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_HEADER1);
      msg.header[2] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_HEADER2);
      msg.header[3] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_HEADER3);
      msg.data[0] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_DATA0);
      msg.data[1] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_DATA1);
      msg.data[2] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_DATA2);
      msg.data[3] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_DATA3);
      msg.status[0] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_STATUS0);
      msg.status[1] = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_STATUS1);
      Pcie_MsgInfoDecode(msg);
      printf(" Pending : %d\n\n", pending);
      
      //pop
      FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_POP, 0x1);
      pending = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSG_PENDING_NUM);
    } while (pending);
    
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_STS, value);
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS,
                  value | PCIE_MSG_OUT);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_EN,
                  PCIE_FIFO_AVAIL | PCIE_FIFO_OVERFLOW | PCIE_FIFO_RECEIVED);
}

void Pcie_MsiFifoHandler()
{
    u32 value;
    u32 pending, addr, data;
    
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI_MSIX_DIS,
                  PCIE_FIFO_AVAIL | PCIE_FIFO_OVERFLOW | PCIE_FIFO_RECEIVED);
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSI_MSIX_STS);
    pending = FMSH_ReadReg(PCIE_SLCR_REG_BASE, MSI_MSIX_PENDING_NUM); 
    
    if (value & PCIE_FIFO_OVERFLOW) {
      printf("MSIX overflow\n");
      if (pending != PCIE_FIFO_SIZE - 1) 
        printf("error, pending num is %d\n", pending);
    }
    
    do {
      addr = FMSH_ReadReg(PCIE_SLCR_REG_BASE, MSI_MSIX_ADDRESS); 
      data = FMSH_ReadReg(PCIE_SLCR_REG_BASE, MSI_MSIX_DATA); 
      printf("MSI-X received info [addr: 0x%x, data: 0x%x, pending : %d]\n",
             addr, data, pending);
      
      //pop
      FMSH_WriteReg(PCIE_SLCR_REG_BASE, MSI_MSIX_POP, 0x1);
      pending = FMSH_ReadReg(PCIE_SLCR_REG_BASE, MSI_MSIX_PENDING_NUM);
    } while (pending);
    
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI_MSIX_STS, value);
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS,
                  value | PCIE_MSI_MSIX_OUT);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI_MSIX_EN,
                  PCIE_FIFO_AVAIL | PCIE_FIFO_OVERFLOW | PCIE_FIFO_RECEIVED);
}

void Pcie_Msi0VectorHandler()
{
    u32 value, i;

    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI0_DIS, 0xffffffff);
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSI0_STS);

    printf("RC receive MSI0 : ");
	for (i = 0; i < 32; i++) {
        if (value & BIT(i))
            printf("%d ", i);
	}
	printf("\n");
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI0_STS, value);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI0_EN, 0xffffffff);
}

void Pcie_Msi1VectorHandler()
{
    u32 value, i;

    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI1_DIS, 0xffffffff);
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSI1_STS);

    printf("RC receive MSI1 : ");
    for (i = 0; i < 32; i++) {
    if (value & BIT(i))
        printf("%d ", i);
    }
    printf("\n");
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI1_STS, value);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI1_EN, 0xffffffff);
}

void Pcie_IntaHandler()
{

	printf("RC receive Legacy Intr : INTa\n");
}

void Pcie_IntbHandler()
{

	printf("RC receive Legacy Intr : INTb\n");
}

void Pcie_IntcHandler()
{

	printf("RC receive Legacy Intr : INTc\n");
}

void Pcie_IntdHandler()
{

	printf("RC receive Legacy Intr : INTd\n");
}

void Pcie_IntxHandler()
{
    int value;
    int mask = 0xf;

    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_INTX_STS);
	if (value & BIT(0)) {
		Pcie_IntaHandler();
	}

	if (value & BIT(1)) {
		Pcie_IntbHandler();
	}

	if (value & BIT(2)) {
		Pcie_IntcHandler();
	}

	if (value & BIT(3)) {
		Pcie_IntdHandler();
	}
	
    do {
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_STS, value & mask);
        value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_INTX_STS);
    } while (value & mask);
    printf("Legacy Intr out\n");
}

void Pcie_VfMsiMaskHandler()
{
    u32 i;
    u32 value, mask, vf;
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS);
    mask = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_MSK);

    value &= ~mask;
    if (value & PCIE_INTR_MSC(4)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC4_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC4_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(4));
    }

    if (value & PCIE_INTR_MSC(5)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC5_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i + 32);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC5_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(5));
    }

	if (value & PCIE_INTR_MSC(6)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC6_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i + 32*2);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC6_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(6));
    }

    if (value & PCIE_INTR_MSC(7)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC7_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i + 32*3);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC7_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(7));
    }

    if (value & PCIE_INTR_MSC(8)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC8_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i + 32*4);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC8_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(8));
    }

	if (value & PCIE_INTR_MSC(9)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC9_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i + 32*5);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC9_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(9));
    }

    if (value & PCIE_INTR_MSC(10)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC10_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i + 32*6);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC10_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(10));
    }

	if (value & PCIE_INTR_MSC(11)) {
        vf = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC11_STS);
        for (i = 0; i < 32; i++) {
            if (vf & BIT(i))
                printf("VF-%d msi mask change\n", i + 32*7);
        }
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC11_STS, vf);
	    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(11));
    }

}

void Pcie_Msc1Handler()
{
    u32 i;
    u32 value, mask;
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC1_STS);
    mask = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSC1_MSK);
    
    /* clear status*/
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC1_STS, value);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, PCIE_INTR_MSC(1));
    
    for (i = 0; i < 23; i++) {
      if (~mask & value & BIT(i)) {
        switch (BIT(i)) {
        case LOCAL_INTERRUPT:
          printf("MSC-1 intrrupt: LOCAL_INTERRUPT\n");
          break;
        case POWER_STATE_CHANGE_INTR:
          printf("MSC-1 intrrupt: POWER_STATE_CHANGE_INTR\n");
          break;
        case DPA_INTERRUPT1:
          printf("MSC-1 intrrupt: DPA_INTERRUPT1\n");
          break;
        case DPA_INTERRUPT2:
          printf("MSC-1 intrrupt: DPA_INTERRUPT2\n");
          break;
        case DPA_INTERRUPT3:
          printf("MSC-1 intrrupt: DPA_INTERRUPT3\n");
          break;
        case DPA_INTERRUPT4:
          printf("MSC-1 intrrupt: DPA_INTERRUPT4\n");
          break;
        case F0_VSEC_INTERRUPT_OUT:
          printf("MSC-1 intrrupt: F0_VSEC_INTERRUPT_OUT\n");
          break;
        case F1_VSEC_INTERRUPT_OUT:
          printf("MSC-1 intrrupt: F1_VSEC_INTERRUPT_OUT\n");
          break;
        case F2_VSEC_INTERRUPT_OUT:
          printf("MSC-1 intrrupt: F2_VSEC_INTERRUPT_OUT\n");
          break;
        case F3_VSEC_INTERRUPT_OUT:
          printf("MSC-1 intrrupt: F3_VSEC_INTERRUPT_OUT\n");
          break;
        case MSI_MASK_VALUE_CHANGE_PF0:
          printf("MSC-1 intrrupt: MSI_MASK_VALUE_CHANGE_PF0\n");
          break;
        case MSI_MASK_VALUE_CHANGE_PF1:
          printf("MSC-1 intrrupt: MSI_MASK_VALUE_CHANGE_PF1\n");
          break;
        case MSI_MASK_VALUE_CHANGE_PF2:
          printf("MSC-1 intrrupt: MSI_MASK_VALUE_CHANGE_PF2\n");
          break;  
        case MSI_MASK_VALUE_CHANGE_PF3:
          printf("MSC-1 intrrupt: MSI_MASK_VALUE_CHANGE_PF3\n");
          break;
        case FATAL_ERROR_OUT:
          printf("MSC-1 intrrupt: FATAL_ERROR_OUT\n");
          break;
        case NON_FATAL_ERROR_OUT:
          printf("MSC-1 intrrupt: NON_FATAL_ERROR_OUT\n");
          break;
        case CORRECTABLE_ERROR_OUT:
          printf("MSC-1 intrrupt: CORRECTABLE_ERROR_OUT\n");
          break;
        case CONFIG_READ_RECEIVED:
          printf("MSC-1 intrrupt: CONFIG_READ_RECEIVED\n");
          break;
        case CONFIG_WRITE_RECEIVED:
          printf("MSC-1 intrrupt: CONFIG_WRITE_RECEIVED\n");
          break;
        case HOT_RESET_OUT:
          printf("MSC-1 intrrupt: HOT_RESET_OUT\n");
          break;
        case LINK_DOWN_RESET_OUT:
          printf("MSC-1 intrrupt: LINK_DOWN_RESET_OUT\n");
          break;
        }
      }
    }
}

void Pcie_MscNumHandler()
{
    u32 value, mask;
    
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS);
    mask = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_MSK);

	if (~mask & value & BIT(0)) {
		printf("MSC intrrupt: MSC 1\n");
		Pcie_Msc1Handler();
	}

	if (~mask & value & BIT(1)) {
		printf("MSC intrrupt: MSC 2 (sideband)\n");
		//Pcie_Msc2Handler();
	}

	if (~mask & value & BIT(2)){
		printf("MSC intrrupt: MSC 3 (sideband)\n");
		//Pcie_Msc3Handler();
	}

	if (~mask & value & PCIE_MSI_MSIX_OUT) {
		printf("MSC intrrupt: MSIX\n");
        Pcie_MsiFifoHandler();
	}
		
	if (~mask & value & PCIE_MSI_MSIX_OUT) {
		printf("MSC intrrupt: MSG\n");
		Pcie_MsgFifoHandler();
	}

	if (~mask & value & GENMASK(10, 3)) {
		Pcie_VfMsiMaskHandler();
	}
	
    printf("MSC Intr out\n");
}

void Pcie_ResetHandler()
{
    int value;
    
    /* disable link training */
    value = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, I_PL_CONFIG_2_REG);
    value &= ~LINK_TRAINING_EN;
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_PL_CONFIG_2_REG, value);

	/* PCIE */
    pcie_reset_config(1, PCIE_CFG_RESET); //apb
    pcie_reset_config(1, PCIE_PIPE_RESET); //pipe
    pcie_reset_config(1, PCIE_CTRL_RESET); //core
    pcie_reset_config(1, PCIE_BRIDGE_RESET); //axi
    pcie_reset_config(1, PCIE_MGMT_STICKY_RESET); //mgmt sticky
    pcie_reset_config(1, PCIE_MGMT_RESET); //mgmt
    pcie_reset_config(1, PCIE_PM_RESET); //pm

    /* reset PHY */
#if 1
    pcie_reset_config(1, GTR_PHY0_RESET);
    pcie_reset_config(1, GTR_PHY1_RESET);
    pcie_reset_config(1, GTR_L00_RESET);
    pcie_reset_config(1, GTR_L01_RESET);
    pcie_reset_config(1, GTR_L02_RESET);
    pcie_reset_config(1, GTR_L03_RESET);
    pcie_reset_config(1, GTR_SLCR_RESET);
#endif

    /* clear pos int status */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PERST_IN_NEG_STS, 0x1);	
    printf("pcie reset done\n");
}

void Pcie_StartUpHandler()
{
    int value;
    int ready;
    
    /* core_apb_reset_N */
	pcie_reset_config(0, PCIE_CFG_RESET);

    /* reset PHY */
#if 0
    pcie_reset_config(0, GTR_SLCR_RESET);
	Pcie_LaneInit(PCIE_LANE_NUM);

	//config GTR
    FMSH_WriteReg(0xfd400000, 0xc00f * 4, 0x2);
    FMSH_WriteReg(0xfd400000, 0xc006 * 4, 0xa);
    FMSH_WriteReg(0xfd400000, 0xc002 * 4, 0x4010);
    FMSH_WriteReg(0xfd400000, 0xc003 * 4, 0x810);
    FMSH_WriteReg(0xfd400000, 0xc004 * 4, 0x101);
    FMSH_WriteReg(0xfd400000, 0xd013 * 4, 0x1111);
    FMSH_WriteReg(0xfd400000, 0xd213 * 4, 0x1111);
    FMSH_WriteReg(0xfd400000, 0xd413 * 4, 0x1111);
    FMSH_WriteReg(0xfd400000, 0xd613 * 4, 0x1111);
	
    /* phy_pipe_reset_N */
	value = GTR_PHY0_RESET | GTR_PHY1_RESET | GTR_L00_RESET |
		GTR_L01_RESET | GTR_L02_RESET | GTR_L03_RESET;
	pcie_reset_config(0, value);
#endif

    /* wait for PHY ready */
    do {
    	ready = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(573));
    } while(ready);

    /* ctrl_pipe_reset_N */
	pcie_reset_config(0, PCIE_PIPE_RESET);

    /* reset_N */
	pcie_reset_config(0, PCIE_CTRL_RESET);

    /* axi_reset_N */
	pcie_reset_config(0, PCIE_BRIDGE_RESET);

    /* mgmt_sticky_reset_N */
	pcie_reset_config(0, PCIE_MGMT_STICKY_RESET);

    /* mgmt_reset_N */
	pcie_reset_config(0, PCIE_MGMT_RESET);

    /* pm_reset_N */
	pcie_reset_config(0, PCIE_PM_RESET);

    /* enable link training */
    value = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, I_PL_CONFIG_2_REG);
    value |= LINK_TRAINING_EN;
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_PL_CONFIG_2_REG, value);

    /* clear pos int status */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PERST_IN_POS_STS, 0x1);
    printf("pcie start up done\n");
}

void FSpcie_HotResetTrigger()
{
    int value, mask = BIT(22);
    
    value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, 0x3c);
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x3c, value | mask);
    printf("RP trigger Hot reset done\n");
}

void Pcie_TriggerIntx(struct fmsh_pcie *pcie, u32 id)
{
    switch (id) {
    case 0:
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTA);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTA);
        printf("EP trigger INTa\n");
        break;
    case 1:
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTB);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTB);
        printf("EP trigger INTb\n");
        break;
    case 2:
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTC);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTC);
        printf("EP trigger INTc\n");
        break;
    case 3:
        FSpcie_MsgSend(pcie, MSG_ASSERT_INTD);
        FSpcie_MsgSend(pcie, MSG_DEASSERT_INTD);
        printf("EP trigger INTd\n");
        break;
    default:
        printf("Error INTx index\n");
    }
}


/* for ms change */
void Pcie_localIntrInit()
{
  u32 value;
  
  value = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, I_DEBUG_MUX_CONTROL_2_REG);
  FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE,
                  I_DEBUG_MUX_CONTROL_2_REG, value | BIT(24));
}

/* ARI cap all read-only, no need to config*/
void FSpcie_SriovInit(struct fmsh_pcie *pcie, u16 pf)
{
    u32 value;
	u32 SriovCap = 0x200;
    u32 Ctrl = 0x8, NumVFs = 0x10;
    u16 bus = 1, dev = 0;

    FSpcie_CfgWrite(pcie, bus, dev, pf, SriovCap + NumVFs, 0x3f);

    value = FSpcie_CfgRead(pcie, bus, dev, pf, SriovCap + Ctrl);
    value |= BIT(0) | BIT(3); //VF enable, Mem space enable
    if (pf == 0)
        value |= BIT(4); //enable ARI Capable Hierarchy
    FSpcie_CfgWrite(pcie, bus, dev, pf, SriovCap + Ctrl, value);
}

void Pcie_AerInit(u32 type)
{
    u32 DevCtrl = 0xc8;
    u32 value;

    value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, DevCtrl);
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE,
                  DevCtrl, value | BIT(0) | BIT(1) | BIT(2));

    if (type == 0) {
      value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, PF_OFFSET(1) + DevCtrl);
      FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE,
                    PF_OFFSET(1) + DevCtrl, value | BIT(0) | BIT(1) | BIT(2));
      
      value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, PF_OFFSET(2) + DevCtrl);
      FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE,
                    PF_OFFSET(2) + DevCtrl, value | BIT(0) | BIT(1) | BIT(2));
      
      value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, PF_OFFSET(3) + DevCtrl);
      FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE,
                    PF_OFFSET(3) + DevCtrl, value | BIT(0) | BIT(1) | BIT(2));
    }
}

int Pcie_PerstInit(FGicPs* InstancePtr)
{
    int mask = 0x1;
    u32 Status;

    /* clear status */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PERST_IN_POS_STS, mask);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PERST_IN_NEG_STS, mask);

    /* enable */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PERST_IN_POS_EN, mask);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PERST_IN_NEG_EN, mask);

    /* pos PERST# */
    Status = FGicPs_Connect(InstancePtr, PCIE_INTR0_INT_ID,
                       (FMSH_InterruptHandler)Pcie_StartUpHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    FGicPs_Enable(InstancePtr, PCIE_INTR0_INT_ID);

    /* neg PERST# */
    Status = FGicPs_Connect(InstancePtr, PCIE_INTR1_INT_ID,
                       (FMSH_InterruptHandler)Pcie_ResetHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    FGicPs_Enable(InstancePtr, PCIE_INTR1_INT_ID);
    return FMSH_SUCCESS;
}

/* 
 * @mode : 0 - trigger by every received msi
           1 - trigger by over-flow
 */
u32 Pcie_MsgFifoInit(u32 mode)
{
    u32 mask, value;

    if (mode == 0)
      mask = PCIE_FIFO_AVAIL | PCIE_FIFO_OVERFLOW | PCIE_FIFO_RECEIVED;
    else
      mask = PCIE_FIFO_OVERFLOW;
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_EN, mask);

    mask = PCIE_OTH_MSG_FWD | PCIE_VEN_MSG_FWD |
           PCIE_ERR_MSG_FWD | PCIE_PM_MSG_FWD;
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSG_CFG_ENABLE, mask);

    /* MSCNUM */
    mask = PCIE_MSG_OUT;
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, mask | value);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_EN, mask);
    return FMSH_SUCCESS;
}

/* 
 * @mode : 0 - trigger by every received msi
           1 - trigger by over-flow
 */
void FSpcie_MsiFifoInit(u32 mode)
{
    u32 mask, value;

    /* enable fifo mode*/
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, MSI_STATUS_ENABLE, MSI_FIFO_MODE);
    
    if (mode == 0)
      mask = PCIE_FIFO_AVAIL | PCIE_FIFO_OVERFLOW | PCIE_FIFO_RECEIVED;
    else
      mask = PCIE_FIFO_OVERFLOW;
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI_MSIX_EN, mask);
    
    /* MSCNUM */
    mask = PCIE_MSI_MSIX_OUT;
    value = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, mask | value);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_EN, mask);
}

u32 Pcie_MsiVectorInit(FGicPs* InstancePtr)
{
    int mask = ~0x0;
    u32 Status;

    /* enable fifo mode*/
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, MSI_STATUS_ENABLE, MSI_VECTOR_MODE);
    
    /* clear status */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI0_STS, mask);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI1_STS, mask);

    /* enable */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI0_EN, mask);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSI1_EN, mask);

    /* MSI0 */
    Status = FGicPs_Connect(InstancePtr, PCIE_MSI0_INT_ID,
                       (FMSH_InterruptHandler)Pcie_Msi0VectorHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    FGicPs_Enable(InstancePtr, PCIE_MSI0_INT_ID);

    /* MSI1 */
    Status = FGicPs_Connect(InstancePtr, PCIE_MSI1_INT_ID,
                       (FMSH_InterruptHandler)Pcie_Msi1VectorHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    FGicPs_Enable(InstancePtr, PCIE_MSI1_INT_ID);
    return FMSH_SUCCESS;
}

u32 Pcie_MsiCapInit()
{
    u32 i, value;
    u32 msi_cap = 0x90;
    u32 MsiCapVec = 0x5 << 17; 
    u32 MsiEnVec = 0x5 << 20;  
    u32 MsiEn = BIT(16);

    for (i = 0; i < 4; i++) {
        value = FMSH_ReadReg(PCIE_CFG_SPACE_REG_BASE, PF_OFFSET(i) + msi_cap);
        //vector num change to 32
        value |= MsiCapVec;
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, PF_OFFSET(i) + msi_cap, value);
        
        //en vector num 32
        value |= MsiEnVec;
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, PF_OFFSET(i) + msi_cap, value);
        
        value |= MsiEn;
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, PF_OFFSET(i) + msi_cap, value);
    }
    return FMSH_SUCCESS;
}

int Pcie_IntxInit(FGicPs* InstancePtr)
{
    int mask = 0xf;
    u32 Status;

    /* clear status */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_STS, mask);

    /* enable */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_EN, mask);

    /* INTx */
    Status = FGicPs_Connect(InstancePtr, PCIE_INTX_INT_ID,
                       (FMSH_InterruptHandler)Pcie_IntxHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    FGicPs_Enable(InstancePtr, PCIE_INTX_INT_ID);
    return FMSH_SUCCESS;
}

u32 Pcie_MscInit(FGicPs* InstancePtr, u16 type)
{
    u32 Status, mask = 0;
    u32 i;

    /* MSC1 */
    if (type == 0)
        mask = HOT_RESET_OUT | CONFIG_READ_RECEIVED | CONFIG_WRITE_RECEIVED |
            MSI_MASK_VALUE_CHANGE_PF0 | MSI_MASK_VALUE_CHANGE_PF1 |
            MSI_MASK_VALUE_CHANGE_PF2 | MSI_MASK_VALUE_CHANGE_PF3;
    mask |= CORRECTABLE_ERROR_OUT | NON_FATAL_ERROR_OUT | FATAL_ERROR_OUT |
            POWER_STATE_CHANGE_INTR | PHY_INTERRUPT_OUT;
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC1_STS, mask);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSC1_EN, mask);

    /* MSC4-11 */
    if (type == 0) {
        for (i = 4; i <= 11; i++) {
            FMSH_WriteReg(PCIE_SLCR_REG_BASE,
                            PCIE_MSC4_STS + 0x10*(i-4), 0xffffffff);
            FMSH_WriteReg(PCIE_SLCR_REG_BASE,
                            PCIE_MSC4_EN + 0x10*(i-4), 0xffffffff);
        }
    }

    /* MSCNUM */
    mask = PCIE_INTR_MSC(1);
    if (type == 0) {
        for (i = 4; i <= 11; i++)
		    mask |= PCIE_INTR_MSC(i);
    }
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_STS, mask);
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MSCNUM_EN, mask);

    /* MSC */
    Status = FGicPs_Connect(InstancePtr, PCIE_MSC_INT_ID,
                       (FMSH_InterruptHandler)Pcie_MscNumHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    FGicPs_Enable(InstancePtr, PCIE_MSC_INT_ID); 
    return FMSH_SUCCESS;
}

u32 Pcie_DmaIntrInit(FGicPs* InstancePtr)
{
    u32 Status;
    int mask = 0x10;

    /* clear status */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_STS, mask);

    /* enable */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_INTX_EN, mask);
    
    /* DMA */
    Status = FGicPs_Connect(InstancePtr, PCIE_DMA_INT_ID,
                       (FMSH_InterruptHandler)Pcie_DmaIntrHandler, InstancePtr);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    FGicPs_Enable(InstancePtr, PCIE_DMA_INT_ID);
    return FMSH_SUCCESS;
}

/*
    VF inbound info        
    PF0`s VF : 0x4 -0x42
    PF1`s VF : 0x43-0x81
    PF2`s VF : 0x82-0xc0
    PF3`s VF : 0xc1-0xff

    Func-num  Bar	PCI-addr				          AXI-addr
    0x4 -0x42 0 	0x90_0000_0000+(vf-4)*0x1000      0x4000_0000+vf*0x1000
              2 	0x91_0000_0000+(vf-4)*0x1000      0x5000_0000+vf*0x1000
              4 	0x92_0000_0000+(vf-4)*0x1000      0x6000_0000+vf*0x1000

    0x43-0x81 0 	0x94_0000_0000+(vf-0x43)*0x1000   0x4000_0000+vf*0x1000
              2 	0x95_0000_0000+(vf-0x43)*0x1000   0x5000_0000+vf*0x1000
              4 	0x97_0000_0000+(vf-0x43)*0x1000   0x6000_0000+vf*0x1000

    0x82-0xc0 0 	0x98_0000_0000+(vf-0x82)*0x1000   0x4000_0000+vf*0x1000
              2 	0x99_0000_0000+(vf-0x82)*0x1000   0x5000_0000+vf*0x1000
              4 	0x9a_0000_0000+(vf-0x82)*0x1000   0x6000_0000+vf*0x1000

    0xc1-0xff 0 	0x9c_0000_0000+(vf-0xc1)*0x1000   0x4000_0000+vf*0x1000
              2 	0x9d_0000_0000+(vf-0xc1)*0x1000   0x5000_0000+vf*0x1000
              4 	0x9e_0000_0000+(vf-0xc1)*0x1000   0x6000_0000+vf*0x1000
*/
u32 Pcie_VfInit()
{
    u32 pf, vf;
    u32 VfBar0 = 0x224, VfBar1 = 0x228;
    u32 VfBar2 = 0x22c, VfBar3 = 0x230;
    u32 VfBar4 = 0x234, VfBar5 = 0x238;
    
    for (pf = 0; pf < 4; pf++) {
        //bar attribute, 256bytes 64-bit
        FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_VF_BAR_CONFIG_0_PF(pf), 0xc500c5);
        FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_VF_BAR_CONFIG_1_PF(pf), 0xc5);

        //VF bar pci addr
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(pf),
                                                VfBar0, 0);
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(pf),
                                                VfBar1, 0x90 + 0x4 * pf);
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(pf),
                                                VfBar2, 0);
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(pf),
                                                VfBar3, 0x91 + 0x4 * pf);
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(pf),
                                                VfBar4, 0);
        FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE + PF_OFFSET(pf),
                                                VfBar5, 0x92 + 0x4 * pf);
    }

    //axi addr
    for (vf = 4; vf < 256; vf++) {
        FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                        INBOUND_EP_FUNC_NUM(vf) + INBOUND_EP_BAR_OFFSET(0),
                        0x40000000 + vf * 0x1000);
        FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                        INBOUND_EP_FUNC_NUM(vf) + INBOUND_EP_BAR_OFFSET(2),
                        0x50000000 + vf * 0x1000);
        FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                        INBOUND_EP_FUNC_NUM(vf) + INBOUND_EP_BAR_OFFSET(4),
                        0x60000000 + vf * 0x1000);
    }
    return FMSH_SUCCESS;
}

static inline void SetControlBit(u8 *dst, u8 bitNum) {

    /* Restrict shift value to 8 bits */
    if (bitNum < 8U) {
        *dst |= (1U << bitNum);
    }
    return;
}

static inline void ClearControlBit(u8 *dst, u8 bitNum) {
    /* Restrict shift value to 8 bits */
    if (bitNum < 8U) {
        *dst &= ~(1U << bitNum);
    }
    return;
}

static inline void ClearReservedBits(u8 *p_ctrl_bits)
{
    /* Clear reserved_0 bits */
    ClearControlBit(p_ctrl_bits, 3);
    ClearControlBit(p_ctrl_bits, 4);

    /* Clear reserved_1 bits */
    ClearControlBit(p_ctrl_bits, 6);
    ClearControlBit(p_ctrl_bits, 7);
    return;
}


static inline u8 SetUDMA_ControlBits(
    u8 *           p_ctrl_bits,
    PCIE_UdmaMode       mode,
    u8                  interrupt_value,
    u8                  continue_on_value)
{
    /* set/clear interrupt bit */
    if (interrupt_value == 1)
    {
        SetControlBit(p_ctrl_bits,0);
    }
    else {
        ClearControlBit(p_ctrl_bits,0);
    }
    /* Set/clear continuity bits */
    if (mode == PCIE_READ_WRITE) {
        ClearControlBit(p_ctrl_bits, 1);
        ClearControlBit(p_ctrl_bits, 2);
    }
    if (mode == PCIE_PREFETCH) {
        SetControlBit(p_ctrl_bits, 1);
        ClearControlBit(p_ctrl_bits, 2);
    }
    if (mode == PCIE_POSTWRITE) {
        ClearControlBit(p_ctrl_bits, 1);
        SetControlBit(p_ctrl_bits, 2);
    }

    /* Set/clear continue_on bit*/
    if (continue_on_value == 1) {
        SetControlBit(p_ctrl_bits, 5);
    }
    else {
        ClearControlBit(p_ctrl_bits, 5);
    }
    /* clear reserved_0 and reserved_1 bits */
    ClearReservedBits(p_ctrl_bits);

    return (*p_ctrl_bits);
}
	
void FSpcie_DmaGatherInit(u32 channel, u32 size)
{  
    u64 axi_addr, pci_addr;
    u32 value, dataSize;
    
    struct PCIE_DmaDesc *desc = (struct PCIE_DmaDesc *) (pcie_dma_desc_base);
    
    dataSize     =  size ;
    axi_addr    = (pcie_dma_bulk_axi_addr0);
    pci_addr   =  pcie_dma_bulk_axi_addr0 + 0x10000000;
    
    
    desc[0].sys_lo_addr  = (u32) (axi_addr & 0xFFFFFFFF) ;
    desc[0].sys_attr     = 0 ;
    desc[0].ext_lo_addr  = 0 ;
    desc[0].ext_hi_addr  = 0 ;
    desc[0].ext_attr     = 0 ;
    desc[0].ext_attr_hi  = 0 ;

    
    desc[0].size_and_ctrl.size = (dataSize>>1) & 0xffffff;
    
   desc[0].size_and_ctrl.ctrl_bits = SetUDMA_ControlBits(&(desc[0].size_and_ctrl.ctrl_bits), PCIE_PREFETCH, 0, 1);

    
    desc[0].status.sys_status = 0;
    desc[0].status.ext_status = 0;
    desc[0].status.chnl_status = 0;
    desc[0].status.reserved_0 = 0;

    desc[0].next  =  ((u32)(pcie_dma_desc_base) + PCIE_UDMA_DESC_SIZE) & 0xFFFFFFFF;;
    
    ////////////// populate descriptor
    
    desc[1].sys_lo_addr =  (u32) (axi_addr+dataSize & 0xFFFFFFFF);
    desc[1].sys_attr    =  0 ;
    desc[1].ext_lo_addr =  0 ;
    desc[1].ext_hi_addr =  0 ;
    desc[1].ext_attr    =  0 ;
    desc[1].ext_attr_hi =  0 ;

    desc[1].size_and_ctrl.size = (dataSize>>1) & 0xffffff ;
    desc[1].size_and_ctrl.ctrl_bits = SetUDMA_ControlBits(&(desc[1].size_and_ctrl.ctrl_bits), PCIE_PREFETCH, 0, 1);
    desc[1].status.sys_status = 0;
    desc[1].status.ext_status = 0;
    desc[1].status.chnl_status = 0;
    desc[1].status.reserved_0 = 0;
    
    desc[1].next         =   ((((u32) pcie_dma_desc_base) + PCIE_UDMA_DESC_SIZE*2) & 0xFFFFFFFF);
    
    ////////////// populate descriptor
    
    desc[2].sys_lo_addr =  0 ;
    desc[2].sys_attr    =  0 ;
    desc[2].ext_lo_addr =  (u32)( pci_addr & 0xFFFFFFFF);
    desc[2].ext_hi_addr =  (u32)(((pci_addr) >> 32) & 0xFFFFFFFF);
    desc[2].ext_attr    =  0 ;
    desc[2].ext_attr_hi =  0 ;

    
     desc[2].size_and_ctrl.size = (dataSize) & 0xffffff;
    
     desc[2].size_and_ctrl.ctrl_bits = SetUDMA_ControlBits(&(desc[2].size_and_ctrl.ctrl_bits), PCIE_POSTWRITE, 1, 0);
     desc[2].status.sys_status = 0;
     desc[2].status.ext_status = 0;
     desc[2].status.chnl_status = 0;
     desc[2].status.reserved_0 = 0;
    
     desc[2].next         = 0;
    
    desc[0].sys_hi_addr   = 0;
    desc[0].next_hi_addr  = 0;
    desc[1].sys_hi_addr   = 0;
    desc[1].next_hi_addr  = 0;
    desc[2].sys_hi_addr   = 0;
    desc[2].next_hi_addr  = 0;
    
    
    /* interrupt init */
    value = FMSH_ReadReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_EN);
    FMSH_WriteReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_EN,
                  value | PCIE_DONE_INTR_BIT(channel) | PCIE_ERROR_INTR_BIT(channel));
    
    /* desc addr init */
    FMSH_WriteReg(PCIE_UDMA_REG_BASE,
                  PCIE_DMA_DESC_ADDR_LOW + PCIE_DMA_CHANNEL_OFFSET(channel),
                  pcie_dma_desc_base);
    FMSH_WriteReg(PCIE_UDMA_REG_BASE,
                  PCIE_DMA_DESC_ADDR_HIGH + PCIE_DMA_CHANNEL_OFFSET(channel),
                  0);
}

void FSpcie_DmaGatherTrans(u32 channel)
{
#ifdef MPSOC_PCIE_EP
    if (size >= 0x1000) {
      printf("invalid input, bulk mode max size 4k\n");
      size = 0x1000;
    }    
    
    //Pcie_DmaGatherInit(channel, size);
#if 0
    FMSH_WriteReg(0xff260000, 0, 0x0);
    FMSH_WriteReg(0xff260000, 0x8, 0x0);
    FMSH_WriteReg(0xff260000, 0xc, 0x0);
#endif

    FMSH_WriteReg(PCIE_UDMA_REG_BASE,
              PCIE_DMA_CONTROL + PCIE_DMA_CHANNEL_OFFSET(channel), 0x3);
#endif

}

void FSpcie_DmaScatterInit(u32 channel, u32 size) {

    u64 axi_addr, pci_addr;
    u32 value, dataSize = size;
    struct PCIE_DmaDesc *desc = (struct PCIE_DmaDesc *) pcie_dma_desc_base;
    
    //dataSize     =  0x100 ;
    axi_addr    = (pcie_dma_bulk_axi_addr0);
    pci_addr   =  pcie_dma_bulk_axi_addr0 + 0x10000000;
    

     // Set up descriptors to read all the data, then write out one half at a time to different addresses
    
     ////////////// populate descriptor
    
     desc[0].sys_lo_addr = 0;
     desc[0].sys_attr    = 0;
     desc[0].ext_lo_addr    = (u32)( pci_addr & 0xFFFFFFFF);
     desc[0].ext_hi_addr    = (u32)(((pci_addr) >> 32) & 0xFFFFFFFF);
     desc[0].ext_attr    = 0;
     desc[0].ext_attr_hi = 0;

    
     desc[0].size_and_ctrl.size = dataSize & 0xffffff;
    
    desc[0].size_and_ctrl.ctrl_bits = SetUDMA_ControlBits(&(desc[0].size_and_ctrl.ctrl_bits), PCIE_PREFETCH, 0, 1);
    
    desc[0].status.sys_status = 0;
    desc[0].status.ext_status = 0;
    desc[0].status.chnl_status = 0;
    desc[0].status.reserved_0 = 0;
    
     desc[0].next         = (((u32) pcie_dma_desc_base) + PCIE_UDMA_DESC_SIZE) & 0xFFFFFFFF;
    
     ////////////// populate descriptor
    
     desc[1].sys_lo_addr = (u32)( axi_addr & 0xFFFFFFFF);
     desc[1].sys_attr    = 0 ;
     desc[1].ext_lo_addr = 0 ;
     desc[1].ext_hi_addr = 0 ;
     desc[1].ext_attr    = 0 ;
     desc[1].ext_attr_hi = 0 ;

    
     desc[1].size_and_ctrl.size = (dataSize>>1) & 0xffffff;
    
    desc[1].size_and_ctrl.ctrl_bits = SetUDMA_ControlBits(&(desc[1].size_and_ctrl.ctrl_bits), PCIE_POSTWRITE, 0, 1);
    desc[1].status.sys_status = 0;
    desc[1].status.ext_status = 0;
    desc[1].status.chnl_status = 0;
    desc[1].status.reserved_0 = 0;
    
     desc[1].next          = (((u32) pcie_dma_desc_base) + PCIE_UDMA_DESC_SIZE*2) & 0xFFFFFFFF;
    
     ////////////// populate descriptor
    
     desc[2].sys_lo_addr =  (u32)( axi_addr+dataSize & 0xFFFFFFFF) ;
     desc[2].sys_attr    =  0 ;
     desc[2].ext_lo_addr =  0 ;
     desc[2].ext_hi_addr =  0 ;
     desc[2].ext_attr    =  0 ;
     desc[2].ext_attr_hi =  0 ;

    
     desc[2].size_and_ctrl.size = (dataSize>>1) & 0xffffff;
    
    desc[2].size_and_ctrl.ctrl_bits = SetUDMA_ControlBits(&(desc[2].size_and_ctrl.ctrl_bits), PCIE_POSTWRITE, 1, 0);
    desc[2].status.sys_status = 0;
    desc[2].status.ext_status = 0;
    desc[2].status.chnl_status = 0;
    desc[2].status.reserved_0 = 0;
    
     desc[2].next          = 0;
    
         desc[0].sys_hi_addr  = 0;
     desc[0].next_hi_addr = 0;
     desc[1].sys_hi_addr  = 0;
     desc[1].next_hi_addr = 0;
     desc[2].sys_hi_addr  = 0;
     desc[2].next_hi_addr = 0;
    
	 /* interrupt init */
	 value = FMSH_ReadReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_EN);
	 FMSH_WriteReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_EN,
				   value | PCIE_DONE_INTR_BIT(channel) | PCIE_ERROR_INTR_BIT(channel));
	 
	 /* desc addr init */
	 FMSH_WriteReg(PCIE_UDMA_REG_BASE,
				   PCIE_DMA_DESC_ADDR_LOW + PCIE_DMA_CHANNEL_OFFSET(channel),
				   pcie_dma_desc_base);
	 FMSH_WriteReg(PCIE_UDMA_REG_BASE,
				   PCIE_DMA_DESC_ADDR_HIGH + PCIE_DMA_CHANNEL_OFFSET(channel),
				   0);


}

void FSpcie_DmaScatterTrans(u32 channel)
{
#ifdef MPSOC_PCIE_RP
    //Pcie_DmaScatterInit(channel, size);
#if 0
    FMSH_WriteReg(0xff260000, 0, 0x0);
    FMSH_WriteReg(0xff260000, 0x8, 0x0);
    FMSH_WriteReg(0xff260000, 0xc, 0x0);
#endif

    FMSH_WriteReg(PCIE_UDMA_REG_BASE,
              PCIE_DMA_CONTROL + PCIE_DMA_CHANNEL_OFFSET(channel), 0x1);
#endif

}

void FSpcie_DmaBulkInit(u32 channel, u32 size)
{
    u64 axi_addr, pci_addr;
    u32 value;
    struct PCIE_DmaDesc *desc = (struct PCIE_DmaDesc *) pcie_dma_desc_base;
    
    /* #1 */
    axi_addr = pcie_dma_bulk_axi_addr0;
    pci_addr = pcie_dma_bulk_axi_addr0 + 0x10000000;
    
    desc[0].sys_lo_addr = (u32)( axi_addr & 0xFFFFFFFF);;
    desc[0].sys_hi_addr =  (u32)(((axi_addr) >> 32) & 0xFFFFFFFF);;
    desc[0].sys_attr    = 0;
    desc[0].ext_lo_addr = (u32)( pci_addr & 0xFFFFFFFF);
    desc[0].ext_hi_addr = (u32)(((pci_addr) >> 32) & 0xFFFFFFFF);
    desc[0].ext_attr    = 0;
    desc[0].ext_attr_hi = 0;
    desc[0].sys_hi_addr = 0;
    desc[0].next_hi_addr = 0;
    
    desc[0].size_and_ctrl.size = size & 0xffffff;
    
    /* set control */
    desc[0].size_and_ctrl.ctrl_bits=
      SetUDMA_ControlBits(&(desc[0].size_and_ctrl.ctrl_bits), PCIE_READ_WRITE, 1, 1);
    
    desc[0].status.sys_status = 0;
    desc[0].status.ext_status = 0;
    desc[0].status.chnl_status = 0;
    desc[0].status.reserved_0 = 0;
    
    desc[0].next = (((u32) pcie_dma_desc_base) + PCIE_UDMA_DESC_SIZE) & 0xFFFFFFFF;
    
    /* #2 */
    axi_addr = pcie_dma_bulk_axi_addr1;
    pci_addr = pcie_dma_bulk_axi_addr1 + 0x10000000;
    
    desc[1].sys_lo_addr = (u32)( axi_addr & 0xFFFFFFFF);;
    desc[1].sys_hi_addr =  (u32)(((axi_addr) >> 32) & 0xFFFFFFFF);;
    desc[1].sys_attr    = 0;
    desc[1].ext_lo_addr = (u32)( pci_addr & 0xFFFFFFFF);
    desc[1].ext_hi_addr = (u32)(((pci_addr) >> 32) & 0xFFFFFFFF);
    desc[1].ext_attr    = 0;
    desc[1].ext_attr_hi = 0;
    desc[1].sys_hi_addr = 0;
    desc[1].next_hi_addr = 0;
    
    desc[1].size_and_ctrl.size = size & 0xffffff;
    
    /* set control */
    desc[1].size_and_ctrl.ctrl_bits=
      SetUDMA_ControlBits(&(desc[1].size_and_ctrl.ctrl_bits), PCIE_READ_WRITE, 1, 0);
    
    desc[1].status.sys_status = 0;
    desc[1].status.ext_status = 0;
    desc[1].status.chnl_status = 0;
    desc[1].status.reserved_0 = 0;
    
    desc[1].next = 0;

    /* interrupt init */
    value = FMSH_ReadReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_EN);
    FMSH_WriteReg(PCIE_UDMA_REG_BASE, PCIE_DMA_INTR_EN,
                  value | PCIE_DONE_INTR_BIT(channel) | PCIE_ERROR_INTR_BIT(channel));
    
    /* desc addr init */
    FMSH_WriteReg(PCIE_UDMA_REG_BASE,
                  PCIE_DMA_DESC_ADDR_LOW + PCIE_DMA_CHANNEL_OFFSET(channel),
                  pcie_dma_desc_base);
    FMSH_WriteReg(PCIE_UDMA_REG_BASE,
                  PCIE_DMA_DESC_ADDR_HIGH + PCIE_DMA_CHANNEL_OFFSET(channel),
                  0);
}

void FSpcie_DmaBulkTrans(u32 channel, u32 dir)
{
    //FSpcie_DmaBulkInit(channel, size);
#if 0
    FMSH_WriteReg(0xff260000, 0, 0x0);
    FMSH_WriteReg(0xff260000, 0x8, 0x0);
    FMSH_WriteReg(0xff260000, 0xc, 0x0);
#endif
    /* Direction init */
    if (dir == 0)
        FMSH_WriteReg(PCIE_UDMA_REG_BASE,
                  PCIE_DMA_CONTROL + PCIE_DMA_CHANNEL_OFFSET(channel), 0x1);
    else
        FMSH_WriteReg(PCIE_UDMA_REG_BASE,
                  PCIE_DMA_CONTROL + PCIE_DMA_CHANNEL_OFFSET(channel), 0x3);

}


void FSpcie_BarInboundSet(u8 func, u8 bar, u8 is_64, u32 aperture, u64 cpu_address)
{
	u32 b, reg, cfg, ctrl;

	if (bar < 4) {
		reg = I_PF_BAR_CONFIG_0_REG(func);
		b = bar;
	} else {
		reg = I_PF_BAR_CONFIG_1_REG(func);
		b = bar - 4;
	}

	//AXI address
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_EP_FUNC_NUM(func) + INBOUND_EP_BAR_OFFSET(bar),
                     lower_32_bits(cpu_address));
	FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_EP_FUNC_NUM(func) + INBOUND_EP_BAR_OFFSET(bar) + 0x4,
                     upper_32_bits(cpu_address));

	if (is_64)
		ctrl = BAR_CFG_CTRL_MEM_64BITS;
	else
		ctrl = BAR_CFG_CTRL_MEM_32BITS;

	cfg = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, reg);
	cfg &= ~(EP_FUNC_BAR_CFG_BAR_APERTURE_MASK(b) |
		 EP_FUNC_BAR_CFG_BAR_CTRL_MASK(b));
	cfg |= (EP_FUNC_BAR_CFG_BAR_APERTURE(b, aperture) |
		EP_FUNC_BAR_CFG_BAR_CTRL(b, ctrl));
	FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, reg, cfg);
}

void Pcie_EpInboundInit() {
    printf("inbound info:\n");
    //bar attribute
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_PF_0_BAR_CONFIG_0_REG, 0x0598058b);
    //FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_PF_0_BAR_CONFIG_1_REG, 0x050505e9);

    /* MEM access for OCM */
    //AXI address
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_EP_FUNC_NUM(0) + INBOUND_EP_BAR_OFFSET(0),
                     0xfffc0000);
    //config space
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x10, 0xfffc0000);
    printf("1) BAR0 for OCM, 256K, 32bit\n");
    printf("   PCI: 0xfffc_0000 - 0xffff_ffff\n");

    /* MEM access for DDR */
    //AXI address
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_EP_FUNC_NUM(0) + INBOUND_EP_BAR_OFFSET(2),
                     0);
    //config space
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x18, 0);
    printf("2) BAR1 for DDR, 2G 32bit\n");
    printf("   PCI: 0x0 - 0x7fff_ffff\n");
#if 0
    pcie_set_bar(0, 0, 0, EP_APERTURE_SIZE_256K, 0xfffc0000);
    pcie_set_bar(0, 2, 0, EP_APERTURE_SIZE_2G, 0);
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x10, 0xfffc0000);
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x18, 0);
#endif
}

void FSpcie_RpInboundBypass()
{
	//bar attribute
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_RC_BAR_CONFIG_REG, 0);

	FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_RP_REG_OFFSET(RP_BAR(7)) + INBOUND_AXI_LOW,
                     0x3f);
	FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_RP_REG_OFFSET(RP_BAR(7)) + INBOUND_AXI_HIGH,
                     0x0);
}

void Pcie_RpInboundInit() {
    u32 bypass_bit;
    
    printf("inbound info:\n");
    //bar attribute
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_RC_BAR_CONFIG_REG, 0x13b10);

    /* MEM access for OCM */
    bypass_bit = 17; //256K
    //AXI address
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_RP_REG_OFFSET(RP_BAR(0)) + INBOUND_AXI_LOW,
                     0xfffc0000 + bypass_bit);
    //config space
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x10, 0xfffc0000);
    printf("1) BAR0 for OCM, 256K, 32bit\n");
    printf("   PCI: 0xfffc_0000 - 0xffff_ffff\n");

    /* MEM access for DDR */
    bypass_bit = 30; //2G
    //AXI address
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_RP_REG_OFFSET(RP_BAR(1)) + INBOUND_AXI_LOW,
                     0x0 + bypass_bit);
    //config space
    FMSH_WriteReg(PCIE_CFG_SPACE_REG_BASE, 0x14, 0x0);
    printf("2) BAR1 for DDR, 2G 32bit\n");
    printf("   PCI: 0x0 - 0x7fff_ffff\n");
    
    /* bar7 for msix space*/
    bypass_bit = 15; //64K
    //AXI address
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                     INBOUND_RP_REG_OFFSET(RP_BAR(7)) + INBOUND_AXI_LOW,
                     0xfd480000 + bypass_bit);
    printf("3) BAR7 for MSI-SPACE, 64k 32bit\n");
}

void FSpcie_SetOutboundRegionForCfg(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr, u32 bypass_bit)
{
    if (bypass_bit > 27)
		bypass_bit = 27;

	if(region > 31)
		return;

    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_PCI_ADDR0, bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_DESC0, 0x80000a);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR0,
                    lower_32_bits(cpu_addr) + bypass_bit);
	FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR1,
                    upper_32_bits(cpu_addr));
	pcie->cfg_base = cpu_addr;
}

void FSpcie_SetOutboundRegionForMem(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr, u64 pci_addr, u32 bypass_bit)
{
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_PCI_ADDR0,
                    (lower_32_bits(pci_addr) & GENMASK(31, 8)) + bypass_bit);
	FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_PCI_ADDR1,
                    upper_32_bits(pci_addr));

    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_DESC0, 0x200002);

    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR0,
                    (lower_32_bits(cpu_addr) & GENMASK(31, 8)) + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR1,
                     upper_32_bits(cpu_addr));
	pcie->mem_base = cpu_addr;
}

void FSpcie_SetOutboundRegionForMsg(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr)
{
	u32 bypass_bit = 16;

    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_DESC0, 0x80000c);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR0,
                    (lower_32_bits(cpu_addr) & GENMASK(31, 8)) + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR1,
                     upper_32_bits(cpu_addr));
	pcie->msg_base = cpu_addr;
}

void FSpcie_SetOutboundRegionForVdMsg(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr)
{
	u32 bypass_bit = 16;

    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_DESC0, 0x80000d);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR0,
                    (lower_32_bits(cpu_addr) & GENMASK(31, 8)) + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(region) + OUTBOUND_AXI_ADDR1,
                     upper_32_bits(cpu_addr));
	pcie->vd_msg_base = cpu_addr;
}

int Pcie_OutboundInit(struct fmsh_pcie *pcie) {
    u32 bypass_bit;

    printf("outbound info:\n");
#ifdef MPSOC_PCIE_RP  
    //CFG outbound init
    bypass_bit = 27;
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(0) + OUTBOUND_PCI_ADDR0, bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(0) + OUTBOUND_DESC0, 0x80000a);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(0) + OUTBOUND_AXI_ADDR0,
                    PCIE_LOW_BASE + bypass_bit);
	pcie->cfg_base = PCIE_LOW_BASE;
    printf("1) Map for CFG TLP\n");
    printf("   AXI: 0xe000_0000 - 0xefff_ffff\n");
#endif

#ifdef MPSOC_PCIE_EP    
    //MEM access for RC MSI/MSIx
    bypass_bit = 15; //64k
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(0) + OUTBOUND_PCI_ADDR0,
                    PCIE_SLCR_REG_BASE + bypass_bit); 
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(0) + OUTBOUND_DESC0, 0x200002);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(0) + OUTBOUND_AXI_ADDR0,
                    PCIE_LOW_BASE + bypass_bit);
	pcie->msix_base = PCIE_LOW_BASE;
    printf("1) Map for RC MSI/MSI-X space\n");
    printf("   AXI: 0xe000_0000 - 0xe000_ffff\n");
    printf("   PCI: 0xfd48_0000 - 0xfd48_ffff\n");
#endif

    //MEM access for OCM
    bypass_bit = 17; //256K
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(1) + OUTBOUND_PCI_ADDR0,
                    0xfffc0000 + bypass_bit); 
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(1) + OUTBOUND_DESC0, 0x200002);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(1) + OUTBOUND_AXI_ADDR0,
                    (PCIE_HIGH_BASE0 & 0xffffffff) + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(1) + OUTBOUND_AXI_ADDR1,
                    (PCIE_HIGH_BASE0 >> 32));
    printf("2) Map for OCM\n");
    printf("   AXI: 0x6_0000_0000 - 0x6_0003_ffff\n");
    printf("   PCI:   0xfffc_0000 - 0xffff_ffff\n");

    //MEM access for DDR
    bypass_bit = 30; //2G
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(2) + OUTBOUND_PCI_ADDR0,
                    0 + bypass_bit); 
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(2) + OUTBOUND_DESC0, 0x200002);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(2) + OUTBOUND_AXI_ADDR0,
                    0 + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(2) + OUTBOUND_AXI_ADDR1,
                    (PCIE_HIGH_BASE1 >> 32));
    printf("3) Map for DDR\n");
    printf("   AXI: 0x80_0000_0000 - 0x80_7fff_ffff\n");
    printf("   PCI: 0x0            - 0x7fff_ffff\n");

    //normal MSG outbound
    bypass_bit = 16; //128K
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(3) + OUTBOUND_DESC0, 0x80000c);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(3) + OUTBOUND_AXI_ADDR0,
                    0x80000000 + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(3) + OUTBOUND_AXI_ADDR1,
                    (PCIE_HIGH_BASE1 >> 32));
	pcie->msg_base = PCIE_HIGH_BASE1 + 0x80000000;
    printf("4) Map for MSG TLP\n");
    printf("   AXI: 0x80_8000_0000 - 0x80_8001_ffff\n");

    //vendor MSG outbound
    bypass_bit = 16; //128K
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(4) + OUTBOUND_DESC0, 0x80000d);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(4) + OUTBOUND_AXI_ADDR0,
                    0x80020000 + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(4) + OUTBOUND_AXI_ADDR1,
                    (PCIE_HIGH_BASE1 >> 32));
	pcie->vd_msg_base = PCIE_HIGH_BASE1 + 0x80020000;
    printf("5) Map for Vendor MSG TLP\n");
    printf("   AXI: 0x80_8002_0000 - 0x80_8003_ffff\n");

#ifdef MPSOC_PCIE_RP  
    //VF outbound init
    bypass_bit = 35;
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(5) + OUTBOUND_PCI_ADDR0,
                    0 + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(5) + OUTBOUND_PCI_ADDR1,
                    0x90);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(5) + OUTBOUND_DESC0, 0x200002);   
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(5) + OUTBOUND_AXI_ADDR0,
                    0 + bypass_bit);
    FMSH_WriteReg(PCIE_AXI_CFG_REG_BASE,
                    OUTBOUND_REGION_OFFSET(5) + OUTBOUND_AXI_ADDR1,
                    0x90);

    printf("6) Map for Access EP VF\n");
    printf("   AXI: 0x90_0000_0000 - 0x9f_ffff_ffff\n");
    printf("   PCI: 0x90_0000_0000 - 0x9f_ffff_ffff\n");
#endif
    return FMSH_SUCCESS;
}

void pcie_lane_map(struct fmsh_pcie *pcie)
{
	u32 map, i = 0;
	
	map = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, 0x200);
	printf("lane map: ");
	while(i < 4) {
		if (map & BIT(i))
			printf("%d ", i);
		i++;
	}
	printf("\n");
}

/* macro setting in ../gtrpsu_v1_0/fmsh_gtr.h
#define FPAR_GTRPSU_LANE0_PROTOCOL     (1)
#define FPAR_GTRPSU_LANE1_PROTOCOL     (1)
#define FPAR_GTRPSU_LANE2_PROTOCOL     (1)
#define FPAR_GTRPSU_LANE3_PROTOCOL     (1)
#define FPAR_GTRPSU_DP_SPEED           (0)
#define FPAR_GTRPSU_REFCLK0_FREQ_HZ    (100)
#define FPAR_GTRPSU_REFCLK1_FREQ_HZ    (100)
#define FPAR_GTRPSU_PMA0_SSC_EN        (0)
#define FPAR_GTRPSU_PMA1_SSC_EN        (0)
*/
void Pcie_PowerOn(struct fmsh_pcie *pcie)
{
    u32 value, ready;
    u32 reg;

    if (pcie->lane == 0x1) {
        reg = 0;
    } else if (pcie->lane == 0x2) {
        reg = 0x10;
    } else {
        reg = 0x20;
    }
    
    if (pcie->speed == 0x2) {
        reg |= 0x1;
    } else if (pcie->speed == 0x3) {
        reg |= 0x2;
    }
    
    if (pcie->mode == 1) {
        reg |= 0x8;
    }
    
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, 0, reg);

    pcie_reset_config(0, PCIE_CFG_RESET);
    gtr_initial();

    /* wait for PHY ready */
    do {
    	ready = FMSH_ReadReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(573));
    } while(ready);

	
    /* ctrl_pipe_reset_N */
	pcie_reset_config(0, PCIE_PIPE_RESET);

    /* reset_N */
	pcie_reset_config(0, PCIE_CTRL_RESET);

    /* axi_reset_N */
	pcie_reset_config(0, PCIE_BRIDGE_RESET);

    /* mgmt_sticky_reset_N */
	pcie_reset_config(0, PCIE_MGMT_STICKY_RESET);

    /* mgmt_reset_N */
	pcie_reset_config(0, PCIE_MGMT_RESET);

    /* pm_reset_N */
	pcie_reset_config(0, PCIE_PM_RESET);

    /* enable link training */
    value = FMSH_ReadReg(PCIE_LOCAL_MGMT_REG_BASE, I_PL_CONFIG_2_REG);
    value |= LINK_TRAINING_EN;
    FMSH_WriteReg(PCIE_LOCAL_MGMT_REG_BASE, I_PL_CONFIG_2_REG, value);
    
    if (pcie->mode == 0) {
    	/* ARI enable */
    	FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(1), 0x1);
    	/* SRIS enable */
        FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(2), 0x1);
        /* SRIOV enbale */
    	FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(3), 0x1);
    } else {
        FMSH_WriteReg(0xfaa00000, 0x30c, 0x04040404);
        FMSH_WriteReg(0xfaa00000, 0x310, 0x04040404);
    }

}
extern FGicPs IntcInstance; /* Instance of the Interrupt Controller */
/*************************************************************************/

void FSpcie_Init(struct fmsh_pcie *pcie)
{
    u32 type = pcie->mode;

    /* disable link */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(4), 0x0);

    Pcie_PowerOn(pcie);
    
    /* enable link */
    FMSH_WriteReg(PCIE_SLCR_REG_BASE, PCIE_MISC_CTRL(4), 0x1);
    
    if (type == 0) {
        printf("EP init...\n");
        Pcie_EpInboundInit();
        Pcie_MsiCapInit();
        Pcie_PerstInit(&IntcInstance);
        Pcie_MscInit(&IntcInstance, 0);
        Pcie_AerInit(0);
        Pcie_localIntrInit();
    } else {
        printf("RP init...\n");
        Pcie_RpInboundInit();
        Pcie_IntxInit(&IntcInstance);
        Pcie_MsiVectorInit(&IntcInstance);
        Pcie_MscInit(&IntcInstance, 1);
        Pcie_AerInit(1);
    }
    Pcie_OutboundInit(pcie);
    

    Pcie_MsgFifoInit(0);
    //Pcie_MsiFifoInit(0);
    Pcie_DmaIntrInit(&IntcInstance);

	FSpcie_AxiCfgWrite(0, 0x4, 0x6);
    if (type == 0) {
    	FSpcie_AxiCfgWrite(1, 0x4, 0x6);
    	FSpcie_AxiCfgWrite(2, 0x4, 0x6);
    	FSpcie_AxiCfgWrite(3, 0x4, 0x6);
    }

    printf("init done\n\n");
}

