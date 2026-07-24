#ifndef _FMSH_PCIE_VERIFY_H_   /* prevent circular inclusions */
#define _FMSH_PCIE_VERIFY_H_

#define PCIE_LOW_BASE				0xE0000000
#define PCIE_HIGH_BASE0				0x600000000UL
#define PCIE_HIGH_BASE1				0x8000000000UL

#define PCIE_ICM         1
#define PCIE_LANE_NUM    4
#define PCIE_GEN         3

struct fmsh_pcie
{
	u32 mode;
	u32 speed;
	u32 lane;

	u64 cfg_base;
	u64 mem_base;
	u64 msix_base;
	u64 msg_base;
	u64 vd_msg_base;
};

//DMA
//static u8 * pcie_dma_desc_base = (u8 *) (0xfffe0000);
static unsigned long pcie_dma_desc_base = (0xfffe0000);
static int pcie_dma_bulk_axi_addr0 = 0x40000000;
static int pcie_dma_bulk_axi_addr1 = 0x60000000;

/*--------------- XILINE INFO ---------------*/
#define PCIE_XILINE_BROAD_RP		        0x60000000

/*--------------- CFG SPACE -----------------*/
#define PCIE_CFG_SPACE_REG_BASE		        0xFA800000
#define   PF_OFFSET(n)                          (n*0x1000)

/*---------------- LOCAL MGMT ----------------*/
#define PCIE_LOCAL_MGMT_REG_BASE	        0xFA900000
#define  I_PL_CONFIG_2_REG		        0x54
#define    LINK_TRAINING_EN		         BIT(0)
#define  I_DEBUG_MUX_CONTROL_REG		0x208
#define  I_DEBUG_MUX_CONTROL_2_REG              0x234
#define  I_PF_0_BAR_CONFIG_0_REG	        0x240
#define  I_PF_0_BAR_CONFIG_1_REG	        0x244
#define  I_PF_BAR_CONFIG_0_REG(n)	        (0x240 + n*0x8)
#define  I_PF_BAR_CONFIG_1_REG(n)	        (0x244 + n*0x8)
#define  EP_FUNC_BAR_CFG_BAR_APERTURE_MASK(b) \
	(GENMASK(4, 0) << ((b) * 8))
#define  EP_FUNC_BAR_CFG_BAR_APERTURE(b, a) \
		(((a) << ((b) * 8)) & EP_FUNC_BAR_CFG_BAR_APERTURE_MASK(b))

#define  EP_FUNC_BAR_CFG_BAR_CTRL_MASK(b) \
	(GENMASK(7, 5) << ((b) * 8))
#define  EP_FUNC_BAR_CFG_BAR_CTRL(b, c) \
	(((c) << ((b) * 8 + 5)) & EP_FUNC_BAR_CFG_BAR_CTRL_MASK(b))
/* BAR control values applicable to both Endpoint Function and Root Complex */
#define  BAR_CFG_CTRL_DISABLED		0x0
#define  BAR_CFG_CTRL_IO_32BITS		0x1
#define  BAR_CFG_CTRL_MEM_32BITS		0x4
#define  BAR_CFG_CTRL_PREFETCH_MEM_32BITS	0x5
#define  BAR_CFG_CTRL_MEM_64BITS		0x6
#define  BAR_CFG_CTRL_PREFETCH_MEM_64BITS	0x7

#define  I_VF_BAR_CONFIG_0_PF(n)	        (0x280 + n*0x8)
#define  I_VF_BAR_CONFIG_1_PF(n)	        (0x284 + n*0x8)
#define  I_RC_BAR_CONFIG_REG		        0x300	

/*----------------- AXI CFG ------------------*/
#define PCIE_AXI_CFG_REG_BASE		        0xFAC00000
#define  OUTBOUND_REGION_OFFSET(n)	        (n*0x20)
#define  OUTBOUND_PCI_ADDR0		        0x0
#define  OUTBOUND_PCI_ADDR1		        0x4
#define  OUTBOUND_DESC0			        0x8
#define  OUTBOUND_AXI_ADDR0		        0x18
#define  OUTBOUND_AXI_ADDR1		        0x1c

#define  INBOUND_RP_REG_OFFSET(bar)	        (bar*0x8 + 0x800)
#define  RP_BAR(n)			        (n == 7 ? 2 : n)
#define  INBOUND_AXI_LOW		        0x0
#define  INBOUND_AXI_HIGH		        0x4
#define  RP_INBOUND_SIZE(n)        ((2 + n) <<)

#define  INBOUND_EP_FUNC_NUM(n)		        (n*0x40 + 0x840)
#define  INBOUND_EP_BAR_OFFSET(bar)	        (bar*0x8)

/*------------------- uDMA -------------------*/
#define PCIE_UDMA_REG_BASE                      0xFAE00000
#define  PCIE_DMA_CHANNEL_OFFSET(n)             (n*0x14)
#define  PCIE_DMA_CONTROL                       0x0
#define  PCIE_DMA_DESC_ADDR_LOW                 0x4
#define  PCIE_DMA_DESC_ADDR_HIGH                0x8
#define  PCIE_DMA_INTR_STS                      0xa0
#define  PCIE_DMA_INTR_EN                       0xa4
#define  PCIE_DMA_INTR_DIS                      0xa8
#define   PCIE_DONE_INTR_BIT(chnl)               (BIT(chnl))
#define   PCIE_ERROR_INTR_BIT(chnl)              (BIT(chnl + 8))

/*-----------------PCIE SLCR -----------------*/
#define PCIE_SLCR_REG_BASE			0xFd480000
#define PCIE_MSIX_SPACE                         0xFd484000
#define  PCIE_MISC_CTRL(index)		\
		(index > 53 ? (index - 1) * 4 : index * 4)
#define  PCIE_INTX_EN				0x1000
#define  PCIE_INTX_DIS				0x1004
#define  PCIE_INTX_MSK				0x1008
#define  PCIE_INTX_STS				0x100c
#define  PCIE_MSI0_EN				0x1010
#define  PCIE_MSI0_DIS				0x1014
#define  PCIE_MSI0_MSK				0x1018
#define  PCIE_MSI0_STS				0x101c
#define  PCIE_MSI1_EN				0x1020
#define  PCIE_MSI1_DIS				0x1024
#define  PCIE_MSI1_MSK				0x1028
#define  PCIE_MSI1_STS				0x102c
#define  PCIE_MSC1_EN				0x1030
#define  PCIE_MSC1_DIS				0x1034
#define  PCIE_MSC1_MSK				0x1038
#define  PCIE_MSC1_STS				0x103c
#define    LINK_DOWN_RESET_OUT		          BIT(22)
#define    HOT_RESET_OUT			  BIT(21)
#define    CONFIG_WRITE_RECEIVED	          BIT(20)
#define    CONFIG_READ_RECEIVED		          BIT(19)
#define    CORRECTABLE_ERROR_OUT	          BIT(18)
#define    NON_FATAL_ERROR_OUT		          BIT(17)
#define    FATAL_ERROR_OUT			  BIT(16)
#define    MSI_MASK_VALUE_CHANGE_PF3              BIT(15)
#define    MSI_MASK_VALUE_CHANGE_PF2              BIT(14)
#define    MSI_MASK_VALUE_CHANGE_PF1              BIT(13)
#define    MSI_MASK_VALUE_CHANGE_PF0              BIT(12)
#define    F3_VSEC_INTERRUPT_OUT                  BIT(11)
#define    F2_VSEC_INTERRUPT_OUT	          BIT(10)
#define    F1_VSEC_INTERRUPT_OUT	          BIT(9)
#define    F0_VSEC_INTERRUPT_OUT	          BIT(8)
#define    PHY_INTERRUPT_OUT		          BIT(7)
#define    DPA_INTERRUPT4			  BIT(6)
#define    DPA_INTERRUPT3			  BIT(5)
#define    DPA_INTERRUPT2			  BIT(4)
#define    DPA_INTERRUPT1			  BIT(3)
#define    POWER_STATE_CHANGE_INTR	          BIT(2)
#define    LOCAL_INTERRUPT			  BIT(1)
#define    HOT_PLUG_INTERRUPT_OUT	          BIT(0)
#define  PCIE_MSC4_EN				0x1060
#define  PCIE_MSC4_DIS				0x1064
#define  PCIE_MSC4_MSK				0x1068
#define  PCIE_MSC4_STS				0x106c
#define  PCIE_MSC5_EN				0x1070
#define  PCIE_MSC5_DIS				0x1074
#define  PCIE_MSC5_MSK				0x1078
#define  PCIE_MSC5_STS				0x107c
#define  PCIE_MSC6_EN				0x1080
#define  PCIE_MSC6_DIS				0x1084
#define  PCIE_MSC6_MSK				0x1088
#define  PCIE_MSC6_STS				0x108c
#define  PCIE_MSC7_EN				0x1090
#define  PCIE_MSC7_DIS				0x1094
#define  PCIE_MSC7_MSK				0x1098
#define  PCIE_MSC7_STS				0x109c
#define  PCIE_MSC8_EN				0x10a0
#define  PCIE_MSC8_DIS				0x10a4
#define  PCIE_MSC8_MSK				0x10a8
#define  PCIE_MSC8_STS				0x10ac
#define  PCIE_MSC9_EN				0x10b0
#define  PCIE_MSC9_DIS				0x10b4
#define  PCIE_MSC9_MSK				0x10b8
#define  PCIE_MSC9_STS				0x10bc
#define  PCIE_MSC10_EN				0x10c0
#define  PCIE_MSC10_DIS				0x10c4
#define  PCIE_MSC10_MSK				0x10c8
#define  PCIE_MSC10_STS				0x10cc
#define  PCIE_MSC11_EN				0x10d0
#define  PCIE_MSC11_DIS				0x10d4
#define  PCIE_MSC11_MSK				0x10d8
#define  PCIE_MSC11_STS				0x10dc
#define  PCIE_MSI_MSIX_EN			0x10e0
#define  PCIE_MSI_MSIX_DIS			0x10e4
#define  PCIE_MSI_MSIX_MSK			0x10e8
#define  PCIE_MSI_MSIX_STS			0x10ec
#define  PCIE_MSG_EN				0x10f0
#define  PCIE_MSG_DIS				0x10f4
#define  PCIE_MSG_MSK				0x10f8
#define  PCIE_MSG_STS				0x10fc
#define    PCIE_FIFO_RECEIVED                     BIT(2)
#define    PCIE_FIFO_OVERFLOW                     BIT(1)
#define    PCIE_FIFO_AVAIL                        BIT(0)
#define  PCIE_MSCNUM_EN				0x1100
#define  PCIE_MSCNUM_DIS			0x1104
#define  PCIE_MSCNUM_MSK			0x1108
#define  PCIE_MSCNUM_STS			0x110c
#define    PCIE_MSG_OUT				 BIT(12)
#define    PCIE_MSI_MSIX_OUT		         BIT(11)
#define    PCIE_INTR_MSC(n)			 BIT(n -1)
#define  PERST_IN_POS_EN			0x1110
#define  PERST_IN_POS_DIS			0x1114
#define  PERST_IN_POS_MSK			0x1118
#define  PERST_IN_POS_STS			0x111c
#define  PERST_IN_NEG_EN			0x1120
#define  PERST_IN_NEG_DIS			0x1124
#define  PERST_IN_NEG_MSK			0x1128
#define  PERST_IN_NEG_STS			0x112c
#define  MSI_MSIX_DATA                          0x2000
#define  MSI_MSIX_ADDRESS                       0x2004
#define  MSI_MSIX_PENDING_NUM                   0x2008                  
#define  MSI_MSIX_POP                           0x200c
#define  MSI_STATUS_ENABLE                      0x2010
#define    MSI_VECTOR_MODE                       (1)
#define    MSI_FIFO_MODE                         (0)
#define  PCIE_MSG_STATUS0						0x2014
#define  PCIE_MSG_STATUS1						0x2018
#define  PCIE_MSG_DATA0							0x201c
#define  PCIE_MSG_DATA1							0x2020
#define  PCIE_MSG_DATA2							0x2024
#define  PCIE_MSG_DATA3							0x2028
#define  PCIE_MSG_HEADER0						0x202c
#define  PCIE_MSG_HEADER1						0x2030
#define  PCIE_MSG_HEADER2						0x2034
#define  PCIE_MSG_HEADER3						0x2038
#define  PCIE_MSG_POP							0x203c
#define  PCIE_MSG_PENDING_NUM					0x2040
#define  PCIE_MSG_CFG_ENABLE					0x2044
#define    PCIE_OTH_MSG_FWD						  BIT(13)
#define    PCIE_VEN_MSG_FWD						  BIT(7)
#define    PCIE_SLT_MSG_FWD						  BIT(5)
#define    PCIE_ERR_MSG_FWD						  BIT(3)
#define    PCIE_INT_MSG_FWD						  BIT(2)
#define    PCIE_PM_MSG_FWD						  BIT(1)

/*-------------FPD SLCR SECURE----------------*/
#define FPD_SLCR_SECURE_PCIE			0x210

/*---------------CRF APB SLCR ----------------*/
#define RST_FPD_GTR			        0x10c
#define  GTR_PHY0_RESET				BIT(0)
#define  GTR_PHY1_RESET				BIT(1)
#define  GTR_L00_RESET				BIT(2)
#define  GTR_L01_RESET				BIT(3)
#define  GTR_L02_RESET				BIT(4)
#define  GTR_L03_RESET				BIT(5)
#define  GTR_SLCR_RESET				BIT(6)
#define  PCIE_CTRL_RESET			BIT(8)
#define  PCIE_BRIDGE_RESET			BIT(9)
#define  PCIE_CFG_RESET				BIT(10)
#define  PCIE_PM_RESET				BIT(11)
#define  PCIE_PIPE_RESET			BIT(12)
#define  PCIE_MGMT_RESET			BIT(13)
#define  PCIE_MGMT_STICKY_RESET		        BIT(14)
#define  PCIE_SLCR_RESET			BIT(15)

/* test API */
u32 pcie_ddr_read(u32 offset);
void pcie_ddr_write(u32 offset, u32 data);
u32 pcie_ocm_read(u32 offset);
void pcie_ocm_write(u32 offset, u32 data);
void Pcie_PoisonMemWriteSend(void);
u32 Pcie_VfInit();

/* driver */
u32 FSpcie_MemRead(struct fmsh_pcie *pcie, u32 offset);
void FSpcie_MemWrite(struct fmsh_pcie *pcie, u32 offset, u32 data);
void FSpcie_MsgSend(struct fmsh_pcie *pcie, u32 msg_code);
void FSpcie_VdMsgSend(struct fmsh_pcie *pcie, u32 msg_code);
u32 FSpcie_CfgRead(struct fmsh_pcie *pcie, u16 bus, u16 dev, u16 func, u32 reg);
void FSpcie_CfgWrite(struct fmsh_pcie *pcie, u16 bus, u16 dev, u16 func, u32 reg, u32 data);
u32 FSpcie_AxiCfgRead(u16 func, u32 reg);
void FSpcie_AxiCfgWrite(u16 func, u32 reg, u32 data);
u32 FSpcie_AriCfgRead(struct fmsh_pcie *pcie, u16 bus, u16 func, u32 reg);
void FSpcie_AriCfgWrite(struct fmsh_pcie *pcie, u16 bus, u16 func, u32 reg, u32 data);
void FSpcie_MsixSend(u32 offset, u32 data);
void FSpcie_HotResetTrigger();
void FSpcie_FuncLevelReset(struct fmsh_pcie *pcie, u16 func);
void FSpcie_TriggerIntx(struct fmsh_pcie *pcie, u32 id);
void FSpcie_MsiFifoInit(u32 mode);
void FSpcie_SriovInit(struct fmsh_pcie *pcie, u16 pf);
void FSpcie_MaxPayLoadSizeChange(struct fmsh_pcie *pcie, u32 level);
void FSpcie_MaxReadRequestSizeChange(u32 level);
void FSpcie_MsgFifoPerpare();
void FSpcie_MsgFifoFilterOth();
void FSpcie_MsgFifoFilterErr();
void FSpcie_MsgFifoFilterPm();
void FSpcie_MsgFifoFilterSlt();
void FSpcie_MsgFifoFilterIntx();
void FSpcie_MsgFifoFilterVendor();
void FSpcie_SetOutboundRegionForCfg(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr, u32 bypass_bit);
void FSpcie_SetOutboundRegionForMem(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr, u64 pci_addr, u32 bypass_bit);
void FSpcie_SetOutboundRegionForMsg(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr);
void FSpcie_SetOutboundRegionForVdMsg(struct fmsh_pcie *pcie,
				u32 region, u64 cpu_addr);
void FSpcie_RpInboundBypass();
void FSpcie_BarInboundSet(u8 func, u8 bar, u8 is_64, u32 aperture, u64 cpu_address);

void FSpcie_DmaGatherInit(u32 channel, u32 size);
void FSpcie_DmaGatherTrans(u32 channel);
void FSpcie_DmaScatterInit(u32 channel, u32 size);
void FSpcie_DmaScatterTrans(u32 channel);
void FSpcie_DmaBulkInit(u32 channel, u32 size);
void FSpcie_DmaBulkTrans(u32 channel, u32 dir);

u32 FSpcie_PerstStatusShow();
void FSpcie_LinkInfoShow();
u32 FSpcie_LtssmTest();
void FSpcie_Init(struct fmsh_pcie *pcie);




int fmsh_pcie_verify(void);    

//MSG CODE
#define MSG_PME				        	0x11800
#define MSG_ASSERT_INTA			        0x12080
#define MSG_DEASSERT_INTA		        0x12480
#define MSG_ASSERT_INTB			        0x12180
#define MSG_DEASSERT_INTB		        0x12580
#define MSG_ASSERT_INTC			        0x12280
#define MSG_DEASSERT_INTC		        0x12680
#define MSG_ASSERT_INTD			        0x12380
#define MSG_DEASSERT_INTD		        0x12780
#define MSG_ERR_CORR			        0x13000
#define MSG_ERR_NO_FATAL		        0x13100
#define MSG_ERR_FATAL			        0x13300
#define MSG_LTR_MSG						0x11080
#define MSG_OBFF_MSG					0x11280
#define MSG_PM_ACTIVE_STATE_NAK			0x11480
#define MSG_PME_TO_ACK					0x11ba0
#define MSG_INGNORED					0x14080
#define MSG_SET_SLOT_POWER_LIMIT		0x5080
#define MSG_PTM_REQ						0x15280
#define MSG_PTM_RESPONSE				0x15380
#define MSG_VENDOR_MSG_TYPE0            0x10000
#define MSG_VENDOR_MSG_TYPE1            0x18000


//GIC num
#define PCIE_INTR0_INT_ID			90 //pos
#define PCIE_INTR1_INT_ID			92 //neg
#define PCIE_MSI0_INT_ID			146
#define PCIE_MSI1_INT_ID			147
#define PCIE_INTX_INT_ID			148
#define PCIE_DMA_INT_ID				149
#define PCIE_MSC_INT_ID				150

//FIFO info
#define PCIE_FIFO_SIZE                          0x80

struct pcie_msg_info {
    u32 status[2];
	u32 data[4];
	u32 header[4];
};

/** BAR Aperture Coding */
typedef enum
{
    EP_APERTURE_SIZE_128B = 0U,
    EP_APERTURE_SIZE_256B = 1U,
    EP_APERTURE_SIZE_512B = 2U,
    EP_APERTURE_SIZE_1K = 3U,
    EP_APERTURE_SIZE_2K = 4U,
    EP_APERTURE_SIZE_4K = 5U,
    EP_APERTURE_SIZE_8K = 6U,
    EP_APERTURE_SIZE_16K = 7U,
    EP_APERTURE_SIZE_32K = 8U,
    EP_APERTURE_SIZE_64K = 9U,
    EP_APERTURE_SIZE_128K = 10U,
    EP_APERTURE_SIZE_256K = 11U,
    EP_APERTURE_SIZE_512K = 12U,
    EP_APERTURE_SIZE_1M = 13U,
    EP_APERTURE_SIZE_2M = 14U,
    EP_APERTURE_SIZE_4M = 15U,
    EP_APERTURE_SIZE_8M = 16U,
    EP_APERTURE_SIZE_16M = 17U,
    EP_APERTURE_SIZE_32M = 18U,
    EP_APERTURE_SIZE_64M = 19U,
    EP_APERTURE_SIZE_128M = 20U,
    EP_APERTURE_SIZE_256M = 21U,
    EP_APERTURE_SIZE_512M = 22U,
    EP_APERTURE_SIZE_1G = 23U,
    EP_APERTURE_SIZE_2G = 24U,
    EP_APERTURE_SIZE_4G = 25U,
    EP_APERTURE_SIZE_8G = 26U,
    EP_APERTURE_SIZE_16G = 27U,
    EP_APERTURE_SIZE_32G = 28U,
    EP_APERTURE_SIZE_64G = 29U,
    EP_APERTURE_SIZE_128G = 30U,
    EP_APERTURE_SIZE_256G = 31U,

	RP_APERTURE_SIZE_4B = 0U,
    RP_APERTURE_SIZE_8B = 1U,
    RP_APERTURE_SIZE_16B = 2U,
    RP_APERTURE_SIZE_32B = 3U,
    RP_APERTURE_SIZE_64B = 4U,
	RP_APERTURE_SIZE_128B = 5U,
    RP_APERTURE_SIZE_256B = 6U,
    RP_APERTURE_SIZE_512B = 7U,
    RP_APERTURE_SIZE_1K = 8U,
    RP_APERTURE_SIZE_2K = 9U,
    RP_APERTURE_SIZE_4K = 10U,
    RP_APERTURE_SIZE_8K = 11U,
    RP_APERTURE_SIZE_16K = 12U,
    RP_APERTURE_SIZE_32K = 13U,
    RP_APERTURE_SIZE_64K = 14U,
    RP_APERTURE_SIZE_128K = 15U,
    RP_APERTURE_SIZE_256K = 16U,
    RP_APERTURE_SIZE_512K = 17U,
    RP_APERTURE_SIZE_1M = 18U,
    RP_APERTURE_SIZE_2M = 19U,
    RP_APERTURE_SIZE_4M = 20U,
    RP_APERTURE_SIZE_8M = 21U,
    RP_APERTURE_SIZE_16M = 22U,
    RP_APERTURE_SIZE_32M = 23U,
    RP_APERTURE_SIZE_64M = 24U,
    RP_APERTURE_SIZE_128M = 25U,
    RP_APERTURE_SIZE_256M = 26U,
    RP_APERTURE_SIZE_512M = 27U,
    RP_APERTURE_SIZE_1G = 28U,
    RP_APERTURE_SIZE_2G = 29U,
    RP_APERTURE_SIZE_4G = 30U,
    RP_APERTURE_SIZE_8G = 31U,
    RP_APERTURE_SIZE_16G = 32U,
    RP_APERTURE_SIZE_32G = 33U,
    RP_APERTURE_SIZE_64G = 34U,
    RP_APERTURE_SIZE_128G = 35U,
    RP_APERTURE_SIZE_256G = 36U
} PCIE_BarApertureSize;

typedef enum
{
    PCIE_MSG_UNLOCK = 0x0,
    PCIE_INVALID_REQ_MSG = 0x1, //MSGD
    PCIE_INVALID_CPL_MSG = 0x2,
    PCIE_PAGE_REQ_MSG = 0x4,
    PCIE_PRG_REQ_MSG = 0x5,
    PCIE_LTR_MSG = 0x10,
    PCIE_OBFF_MSG = 0x12,
    PCIE_PM_ACTIVE_STATE_NAK = 0x14,
    PCIE_PM_PME_MSG= 0x18,
    PCIE_PME_TURN_OFF = 0x19,
    PCIE_PME_TO_ACK = 0x1b,
    PCIE_ASSERT_INTA = 0x20,
    PCIE_ASSERT_INTB = 0x21,
    PCIE_ASSERT_INTC = 0x22,
    PCIE_ASSERT_INTD = 0x23,
    PCIE_DEASSERT_INTA = 0x24,
    PCIE_DEASSERT_INTB = 0x25,
    PCIE_DEASSERT_INTC = 0x26,
    PCIE_DEASSERT_INTD = 0x27,
    PCIE_ERR_CORR = 0x30,
    PCIE_ERR_NONFATAL = 0x31,
    PCIE_ERR_FATAL = 0x33,
    PCIE_SET_SLOT_POWER_LIMIT = 0x50,
    PCIE_PTM_REQ = 0x52,
    PCIE_PTM_RESPONSE = 0x53,
    PCIE_VENDOR_DEFINE_TYPE0 = 0x7e,
    PCIE_VENDOR_DEFINE_TYPE1 = 0x7f
} PCIE_MsgCode;

/** UDMA continuity modes */
typedef enum
{
    /** Read the data and write the data */
    PCIE_READ_WRITE = 0U,
    /** Only read data do not write it */
    PCIE_PREFETCH = 1U,
    /** Write previously read data */
    PCIE_POSTWRITE = 2U
} PCIE_UdmaMode;

/** uDMA transfer size and control byte */
struct PCIE_sz_ctrl
{
    /** Number of bytes to be transferred.  For max bulk transfer size, set to zero */
    u32 size : 24;
    /** Control byte */
    u8 ctrl_bits;
};

/** uDMA status bytes */
struct PCIE_sbytes
{
    /** System (local) bus status */
    u8 sys_status;
    /** External (remote) bus status */
    u8 ext_status;
    /** uDMA channel status */
    u8 chnl_status;
    /** Reserved */
    u8 reserved_0;
};

/** uDMA Descriptor structure */
struct PCIE_DmaDesc
{
    /** Low 32 bits of system address */
    u32 sys_lo_addr;
    /** High 32 bits of system address */
    u32 sys_hi_addr;
    /** Access attributes for system bus */
    u32 sys_attr;
    /** Low 32 bits of external address */
    u32 ext_lo_addr;
    /** High 32 bits of external address */
    u32 ext_hi_addr;
    /** Access attributes for external bus */
    u32 ext_attr;
    /** High 32 bits of access attributes for external bus */
    u32 ext_attr_hi;
    /** Transfer size and control byte */
    struct PCIE_sz_ctrl size_and_ctrl;
    /** Transfer status.  This word is written by uDMA engine, and can be read to determine status. */
    struct PCIE_sbytes status;
    /** Low 32bits of pointer to next descriptor in linked list */
    u32 next;
    /** High 32bits of pointer to next descriptor in linked list */
    u32 next_hi_addr;
};

#define PCIE_UDMA_DESC_SIZE             (sizeof(struct PCIE_DmaDesc))

/* Enumerations */
typedef enum
{
    PCIE_DETECT_QUIET = 0U,
    PCIE_DETECT_ACTIVE = 1U,
    PCIE_POLLING_ACTIVE = 2U,
    PCIE_POLLING_COMPLIANCE = 3U,
    PCIE_POLLING_CONFIGURATION = 4U,
    PCIE_CONFIGURATION_LINKWIDTH_START = 5U,
    PCIE_CONFIGURATION_LINKWIDTH_ACCEPT = 6U,
    PCIE_CONFIGURATION_LANENUM_ACCEPT = 7U,
    PCIE_CONFIGURATION_LANENUM_WAIT = 8U,
    PCIE_CONFIGURATION_COMPLETE = 9U,
    PCIE_CONFIGURATION_IDLE = 10U,
    PCIE_RECOVERY_RCVRLOCK = 11U,
    PCIE_RECOVERY_SPEED = 12U,
    PCIE_RECOVERY_RCVRCFG = 13U,
    PCIE_RECOVERY_IDLE = 14U,
    PCIE_L0 = 16U,
    PCIE_RX_L0S_ENTRY = 17U,
    PCIE_RX_L0S_IDLE = 18U,
    PCIE_RX_L0S_FTS = 19U,
    PCIE_TX_L0S_ENTRY = 20U,
    PCIE_TX_L0S_IDLE = 21U,
    PCIE_TX_L0S_FTS = 22U,
    PCIE_L1_ENTRY = 23U,
    PCIE_L1_IDLE = 24U,
    PCIE_L2_IDLE = 25U,
    PCIE_L2_TRANSMITWAKE = 26U,
    PCIE_DISABLED = 32U,
    PCIE_LOOPBACK_ENTRY_MASTER = 33U,
    PCIE_LOOPBACK_ACTIVE_MASTER = 34U,
    PCIE_LOOPBACK_EXIT_MASTER = 35U,
    PCIE_LOOPBACK_ENTRY_SLAVE = 36U,
    PCIE_LOOPBACK_ACTIVE_SLAVE = 37U,
    PCIE_LOOPBACK_EXIT_SLAVE = 38U,
    PCIE_HOT_RESET = 39U,
    PCIE_RECOVERY_EQ_PHASE_0 = 40U,
    PCIE_RECOVERY_EQ_PHASE_1 = 41U,
    PCIE_RECOVERY_EQ_PHASE_2 = 42U,
    PCIE_RECOVERY_EQ_PHASE_3 = 43U
} PCIE_LtssmState;
#endif /* prevent circular inclusions */

