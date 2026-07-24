/* parasoft suppress item  MISRA2012-DIR-4_8 "Consider hiding implementation of
 * structure" */
/**********************************************************************
 * Copyright (C) 2014-2021 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 **********************************************************************
 * WARNING: This file is auto-generated using api-generator utility.
 *          api-generator: 12.02.13bb8d5
 *          Do not edit it manually.
 **********************************************************************
 * XHCI driver for both host and device mode header file
 **********************************************************************/

#ifndef CDN_XHCI_PRIV_H
#define CDN_XHCI_PRIV_H

/** @defgroup ConfigInfo  Configuration and Hardware Operation Information
 *  The following definitions specify the driver operation environment that
 *  is defined by hardware configuration or client code. These defines are
 *  located in the header file of the core driver.
 *  @{
 */

/**********************************************************************
 * Defines
 **********************************************************************/
/** Maximum number of 32-bit double words for single ext. capability */
#define USBSSP_MAX_EXT_CAP_ELEM_DWORDS  256U

/**
 * Maximum number of extended capabilities
 * @remarks USB legacy support are not taken under consideration
 */
#define USBSSP_MAX_EXT_CAPS_COUNT       8U

#define USBSSP_TRB_NORMAL               (uint32_t)1U

#define USBSSP_TRB_SETUP_STAGE          (uint32_t)2U

#define USBSSP_TRB_DATA_STAGE           (uint32_t)3U

#define USBSSP_TRB_STATUS_STAGE         (uint32_t)4U

#define USBSSP_TRB_ISOCH                (uint32_t)5U

#define USBSSP_TRB_LINK                 (uint32_t)6U

#define USBSSP_TRB_EVENT_DATA           (uint32_t)7U

#define USBSSP_TRB_NO_OP                (uint32_t)8U

#define USBSSP_TRB_ENABLE_SLOT_COMMAND  (uint32_t)9U

#define USBSSP_TRB_DISABLE_SLOT_COMMAND (uint32_t)10U

#define USBSSP_TRB_ADDR_DEV_CMD         (uint32_t)11U

#define USBSSP_TRB_CONF_EP_CMD          (uint32_t)12U

#define USBSSP_TRB_EVALUATE_CXT_CMD     (uint32_t)13U

#define USBSSP_TRB_RESET_EP_CMD         (uint32_t)14U

#define USBSSP_TRB_STOP_EP_CMD          (uint32_t)15U

#define USBSSP_TRB_SET_TR_DQ_PTR_CMD    (uint32_t)16U

#define USBSSP_TRB_RESET_DEVICE_COMMAND (uint32_t)17U

#define USBSSP_TRB_FORCE_EVENT_COMMAND  (uint32_t)18U

#define USBSSP_TRB_NEGOTIATE_BANDWIDTH  (uint32_t)19U

#define USBSSP_TRB_SET_LAT_TOL_VAL_CMD  (uint32_t)20U

#define USBSSP_TRB_GET_PORT_BNDWTH_CMD  (uint32_t)21U

#define USBSSP_TRB_FORCE_HEADER_COMMAND (uint32_t)22U

#define USBSSP_TRB_NO_OP_COMMAND        (uint32_t)23U

#define USBSSP_TRB_TRANSFER_EVENT       (uint32_t)32U

#define USBSSP_TRB_CMD_CMPL_EVT         (uint32_t)33U

#define USBSSP_TRB_PORT_ST_CHG_EVT      (uint32_t)34U

#define USBSSP_TRB_BNDWTH_RQ_EVT        (uint32_t)35U

#define USBSSP_TRB_DOORBELL_EVENT       (uint32_t)36U

#define USBSSP_TRB_HOST_CTRL_EVT        (uint32_t)37U

#define USBSSP_TRB_DEV_NOTIFCN_EVT      (uint32_t)38U

#define USBSSP_TRB_MFINDEX_WRAP_EVENT   (uint32_t)39U

/** Vendor-defined TRBs (for USBSSP) */
#define USBSSP_TRB_NRDY_EVT             (uint32_t)48U

#define USBSSP_TRB_SETUP_PROTO_ENDP_CMD (uint32_t)49U

#define USBSSP_TRB_GET_PROTO_ENDP_CMD   (uint32_t)50U

#define USBSSP_TRB_SET_ENDPS_ENA_CMD    (uint32_t)51U

#define USBSSP_TRB_GET_ENDPS_ENA_CMD    (uint32_t)52U

#define USBSSP_TRB_ADD_TDL_CMD          (uint32_t)53U

#define USBSSP_TRB_HALT_ENDP_CMD        (uint32_t)54U

#define USBSSP_TRB_SETUP_STAGE1         (uint32_t)55U

#define USBSSP_TRB_HALT_ENDP_CMD1       (uint32_t)56U

#define USBSSP_TRB_DRBL_OVERFLOW_EVENT  (uint32_t)57U

#define USBSSP_TRB_FLUSH_EP_CMD         (uint32_t)58U

#define USBSSP_TRB_VF_SEC_VIOLN_EVT     (uint32_t)59U

#define USBSSP_TRB_TBC_TBSTS_POS        (uint32_t)7U

#define USBSSP_TRB_TBC_TBSTS_MASK       (uint32_t)0x180U

#define USBSSP_TRB_TDSIZE_TBC_POS       (uint32_t)17U

#define USBSSP_TRB_TDSIZE_TBC_MASK      (uint32_t)0x3E0000U

/** Interrupter target position in TRB dword2 */
#define USBSSP_TRB_INTR_TRGT_POS        (uint32_t)22U

#define USBSSP_TRB_TYPE_POS             (uint32_t)10U

#define USBSSP_BSR_POS                  9U

#define USBSSP_TRB_MAX_TRANSFER_LENGTH  (uint32_t)0x10000U

#define USBSSP_SYSTEM_MEMORY_PAGE_SIZE  (uint32_t)0x10000U

#define USBSSP_TRB_NORMAL_ISP_MASK      (uint32_t)0x4U

#define USBSSP_TRB_NORMAL_CH_MASK       (uint32_t)0x10U

#define USBSSP_TRB_BMREQUESTTYPE_POS    0U

#define USBSSP_TRB_BREQUEST_POS         8U

#define USBSSP_TRB_WVALUE_POS           16U

#define USBSSP_TRB_WINDEX_POS           0U

#define USBSSP_TRB_FORCEEV_VF_ID_POS    16U

#define USBSSP_TRB_FRCEVT_VFINTTGT_POS  22U

#define USBSSP_TRB_WLENGTH_POS          16U

#define USBSSP_TRB_SETUPID_POS          8U

#define USBSSP_TRB_SETUPID_MASK         (uint32_t)0x300U

#define USBSSP_TRB_STS_STG_STAT_POS     6U

#define USBSSP_TRB_SPEED_ID_2           0x00U

#define USBSSP_TRB_SPEED_ID_3           0x80U

#define USBSSP_TRB_STS_STG_STAT_ACK     1U

#define USBSSP_TRB_STS_STG_STAT_STALL   0U

#define USBSSP_TRB_NORMAL_IOC_MASK      (uint32_t)0x20U

#define USBSSP_TRB_NORMAL_IDT_MASK      (uint32_t)0x40U

#define USBSSP_TRB_NORMAL_ENT_MASK      (uint32_t)0x02U

#define USBSSP_TRB_NORM_TRFR_LEN_MSK    (uint32_t)0x1FFFFU

#define USBSSP_TRB_LNK_TGLE_CYC_MSK     (uint32_t)0x02U

#define USBSSP_TRB_EVT_RESIDL_LEN_MSK   (uint32_t)0xFFFFFFU

#define USBSSP_TRB_COMPLETE_INVALID     0U

#define USBSSP_TRB_COMPLETE_SUCCESS     1U

#define USBSSP_TRB_CMPL_DATA_BUFF_ER    2U

#define USBSSP_TRB_CMPL_BBL_DETECT_ER   3U

#define USBSSP_TRB_CMPL_USB_TRANSCN_ER  4U

#define USBSSP_TRB_COMPLETE_TRB_ERROR   5U

#define USBSSP_TRB_COMPLETE_STALL_ERROR 6U

#define USBSSP_TRB_CMPL_RSRC_ER         7U

#define USBSSP_TRB_CMPL_BDWTH_ER        8U

#define USBSSP_TRB_CMPL_NO_SLTS_AVL_ER  9U

#define USBSSP_TRB_CMPL_INVSTRM_TYP_ER  10U

#define USBSSP_TRB_CMPL_SLT_NOT_EN_ER   11U

#define USBSSP_TRB_CMPL_EP_NOT_EN_ER    12U

#define USBSSP_TRB_CMPL_SHORT_PKT       13U

#define USBSSP_TRB_CMPL_RING_UNDERRUN   14U

#define USBSSP_TRB_CMPL_RING_OVERRUN    15U

#define USBSSP_TRB_CMPL_VF_EVTRNGFL_ER  16U

#define USBSSP_TRB_CMPL_PARAMETER_ER    17U

#define USBSSP_TRB_CMPL_BDWTH_OVRRN_ER  18U

#define USBSSP_TRB_CMPL_CXT_ST_ER       19U

#define USBSSP_TRB_CMPL_NO_PNG_RSP_ER   20U

#define USBSSP_TRB_CMPL_EVT_RNG_FL_ER   21U

#define USBSSP_TRB_CMPL_INCMPT_DEV_ER   22U

#define USBSSP_TRB_CMPL_MISSED_SRV_ER   23U

#define USBSSP_TRB_CMPL_CMD_RNG_STOPPED 24U

#define USBSSP_TRB_CMPL_CMD_ABORTED     25U

#define USBSSP_TRB_COMPLETE_STOPPED     26U

#define USBSSP_TRB_CMPL_STOP_LEN_INV    27U

#define USBSSP_TRB_CMPL_STOP_SHORT_PKT  28U

#define USBSSP_TRB_CMPL_MAXEXTLT_LG_ER  29U

#define USBSSP_TRB_CMPL_ISO_BUFF_OVRUN  31U

#define USBSSP_TRB_CMPL_EVT_LOST_ER     32U

#define USBSSP_TRB_CMPL_UNDEFINED_ER    33U

#define USBSSP_TRB_CMPL_INV_STRM_ID_ER  34U

#define USBSSP_TRB_CMPL_SEC_BDWTH_ER    35U

#define USBSSP_TRB_CMPL_SPLT_TRNSCN_ER  36U

#define USBSSP_TRB_CMPL_CDNSDEF_ERCODES 192U

#define USBSSP_TRB_SETUP_TRT_POS        16U

#define USBSSP_TRB_SETUP_TRT_NO_DATA    (uint32_t)0x0U

#define USBSSP_TRB_SETUP_TRT_OUT_DATA   (uint32_t)0x2U

#define USBSSP_TRB_SETUP_TRT_IN_DATA    (uint32_t)0x3U

#define USBSSP_TRB_ISOCH_FRAME_ID_POS   20U

#define USBSSP_TRB_ISOCH_FRAME_ID_MASK  (uint32_t)0x7FF00000U

#define USBSSP_TRB_ISOCH_SIA_POS        31U

#define USBSSP_TRB_ISOCH_SIA_MASK       (uint32_t)0x80000000U

#define USBSSP_TRB_TRANSFER_LENGTH_MASK (uint32_t)0x1FFFFU

#define USBSSP_COMPLETION_CODE_POS      24U

#define USBSSP_SLOT_ID_POS              24U

#define USBSSP_ENDPOINT_POS             16U

#define USBSSP_INTERRUPTER_TARGET_POS   22U

#define USBSSP_TRANSFER_DIR_POS         16U

#define USBSSP_PORTSCUSB_PLS__RXDETECT  5U

#define USBSSP_EP_CONTEXT_INTERVAL_POS  16U

/** Upper 8 bits. The MAX ESIT Payload represent the total number of bytes this
 * endpoint will transfer during an ESIT.  */
#define USBSSP_EP_CXT_MAXESITPLD_HI_POS 24U

#define USBSSP_EP_CONTEXT_LSA_POS       15U

#define USBSSP_EP_CONTEXT_LSA_MASK      (uint32_t)0x8000U

#define USBSSP_EP_CXT_MAX_PKT_SZ_POS    16U

#define USBSSP_EP_CXT_MAX_BURST_SZ_POS  8U

#define USBSSP_EP_CXT_MAX_BURST_SZ_MASK (uint32_t)0xFF00U

#define USBSSP_EP_CONTEXT_MULT_POS      8U

#define USBSSP_EP_CONTEXT_MULT_MASK     (uint32_t)0x300U

#define USBSSP_EP_CONTEXT_CERR_POS      1U

#define USBSSP_EP_CONTEXT_CERR_MASK     (uint32_t)0x6U

#define USBSSP_EP_CXT_PMAXSTREAMS_POS   10U

#define USBSSP_EP_CONTEXT_3ERR          3U

#define USBSSP_EP_CXT_EP_DIR_IN         4U

#define USBSSP_EP_CXT_EP_DIR_OUT        0U

/**
 * The xHCI Spec says that for a control ep the average trb
 * length must be set by SW to 8.
 * It also defines good non-zero initial values for eps
 * of different types: INT = 1024 (1K)
 * and BULK/ISOC = 3072 (3K)
 */
#define USBSSP_EP_CXT_EP_CTL_AVGTRB_LEN 8U

#define USBSSP_EP_CXT_EP_INT_AVGTRB_LEN 1024U

#define USBSSP_EP_CXT_EP_ISO_AVGTRB_LEN 3072U

#define USBSSP_EP_CXT_EP_BLK_AVGTRB_LEN 3072U

#define USBSSP_EP_CXT_EP_AVGTRBLEN_POS  0U

/** Lower 16 bits. The MAX ESIT Payload represent the total number of bytes this
 * endpoint will transfer during an ESIT.  */
#define USBSSP_EP_CXT_MAXESITPLD_LO_POS 16U

#define USBSSP_EP_CONTEXT_EP_TYPE_POS   3U

#define USBSSP_EP_CONTEXT_EP_TYPE_MASK  (uint32_t)0x38U

#define USBSSP_EP_CONTEXT_STATE_MASK    7U

#define USBSSP_EP0_CONTEXT_OFFSET       1U

#define USBSSP_SLOT_CXT_CXT_ENT_POS     27U

#define USBSSP_SLOT_CONTEXT_SPEED_POS   20U

#define USBSSP_SLOT_CXT_NUM_PORTS_POS   24U

#define USBSSP_SLOT_CXT_PORT_NUM_POS    16U

#define USBSSP_SLOT_CONTEXT_STATE_POS   27U

/** DDUSB Event ring buffer count */
#define USBSSP_DDUSB_ERBUF_COUNT        8U

#define USBSSP_DDUSB_TX_CTRL_REG_COUNT  4U

#define USBSSP_DDUSB_RX_CTRL_REG_COUNT  4U

#define USBSSP_DDUSB_CTRL_REG_EP_POS    8U

#define USBSSP_DDUSB_CTRL_REG_EP_MASK   0xFU

/**
 *  @}
 */

/** @defgroup DataStructure Dynamic Data Structures
 *  This section defines the data structures used by the driver to provide
 *  hardware information, modification and dynamic operation of the driver.
 *  These data structures are defined in the header file of the core driver
 *  and utilized by the API.
 *  @{
 */

/**********************************************************************
 * Forward declarations
 **********************************************************************/
typedef struct USBSSP_DDUSBConfigT_s USBSSP_DDUSBConfigT;
typedef struct USBSSP_DDUSBEventRegsT_s USBSSP_DDUSBEventRegsT;
typedef struct USBSSP_CapabilityT_s USBSSP_CapabilityT;
typedef struct USBSSP_PortControlT_s USBSSP_PortControlT;
typedef struct USBSSP_OperationalT_s USBSSP_OperationalT;
typedef struct USBSSP_InterrupterT_s USBSSP_InterrupterT;
typedef struct USBSSP_RuntimeT_s USBSSP_RuntimeT;
typedef struct USBSSP_ExtCapElemT_s USBSSP_ExtCapElemT;
typedef struct USBSSP_ExtCapSetT_s USBSSP_ExtCapSetT;

/**********************************************************************
 * Enumerations
 **********************************************************************/
typedef enum {
    USBSSP_EP_CXT_EPTYP_ISO_OUT = 1U,
    USBSSP_EP_CXT_EPTYP_BLK_OUT = 2U,
    USBSSP_EP_CXT_EPTYP_INT_OUT = 3U,
    USBSSP_EP_CXT_EPTYP_CTL_BI = 4U,
    USBSSP_EP_CXT_EPTYP_ISO_IN = 5U,
    USBSSP_EP_CXT_EPTYP_BLK_IN = 6U,
    USBSSP_EP_CXT_EPTYP_INT_IN = 7U
} USBSSP_EpContextEpTypeT;

typedef enum {
    USBSSP_EP_CONTEXT_EP_STATE_DISABLED = 0U,
    USBSSP_EP_CONTEXT_EP_STATE_RUNNING = 1U,
    USBSSP_EP_CONTEXT_EP_STATE_HALTED = 2U,
    USBSSP_EP_CONTEXT_EP_STATE_STOPPED = 3U,
    USBSSP_EP_CONTEXT_EP_STATE_ERROR = 4U
} USBSSP_EpContexEpState;

typedef enum {
    USBSSP_SLOT_CONTEXT_STATE_DISABLED_ENABLED = 0U,
    USBSSP_SLOT_CONTEXT_STATE_DEFAULT = 1U,
    USBSSP_SLOT_CONTEXT_STATE_ADDRESSED = 2U,
    USBSSP_SLOT_CONTEXT_STATE_CONFIGURED = 3U,
    USBSSP_SLOT_CONTEXT_STATE_RESERVED = 4U
} USBSSP_SlotContexState;

typedef enum {
    USBSSP_REQUEST_COMPLETE = 0U,
    USBSSP_REQUEST_PENDING = 1U,
    USBSSP_REQUEST_HALTED = 2U
} USBSSP_ReqState;

typedef enum {
    USBSSP_EP_DISABLE = 0U,
    USBSSP_EP_ENABLE = 1U,
    USBSSP_EP_CONFIGURE = 2U
} USBSSP_SetInterfaceState;

typedef enum {
    USBSSP_PORTSC_REG_IDX = 0U,
    USBSSP_PORTPMSC_REG_IDX = 1U,
    USBSSP_PORTLI_REG_IDX = 2U,
    USBSSP_PORTHLPMC_REG_IDX = 3U
} USBSSP_PortControlRegIdx;

/**********************************************************************
 * Structures and unions
 **********************************************************************/
struct USBSSP_DDUSBConfigT_s {
    /** Enable DDUSB functionality */
    uint32_t ddusbEnable;
    /** Maps the discrete IO ddusb_tx_valid signal to a specific slot and
     * endpoint */
    uint32_t ddusbTx[USBSSP_DDUSB_TX_CTRL_REG_COUNT];
    /** Maps the discrete IO ddusb_rx_valid signal to a specific slot and
     * endpoint */
    uint32_t ddusbRx[USBSSP_DDUSB_RX_CTRL_REG_COUNT];
} __attribute__((packed));

struct USBSSP_DDUSBEventRegsT_s {
    /** DDUSB Event ring buffer address */
    uintptr_t eventRingAddr[USBSSP_DDUSB_ERBUF_COUNT];
} __attribute__((packed));

struct USBSSP_CapabilityT_s {
    /** Capability Register Length and Interface Version Number */
    uint32_t caplength_hciver;
    /** Structural Parameters 1 */
    uint32_t hcsparams1;
    /** Structural Parameters 2 */
    uint32_t hcsparams2;
    /** Structural Parameters 3 */
    uint32_t hcsparams3;
    /** Capability Parameters 1 */
    uint32_t hccparams1;
    /** Doorbell Offset 1 */
    uint32_t dboff;
    /** Runtime Registers Space Offset */
    uint32_t rtsoff;
    /** Capability Parameters 2 */
    uint32_t hccparams2;
};

struct USBSSP_QuickAccessRegs_s {
    /** Copy of xhci capability register for quick access. */
    USBSSP_CapabilityT xHCCaps;
};

struct USBSSP_PortControlT_s {
    /** Port Status and Control */
    uint32_t portsc;
    /** Port Power Management Status and Control */
    uint32_t portpmsc;
    /** Port Link Info */
    uint32_t portli;
    /** Port Hardware LPM Control Register */
    uint32_t porthlpmc;
} __attribute__((packed));

struct USBSSP_OperationalT_s {
    /** USB Command */
    uint32_t usbcmd;
    /** USB Status */
    uint32_t usbsts;
    /** Page Size */
    uint32_t pagesize;
    uint32_t reserved_0[2];
    /** Device Notification Control */
    uint32_t dnctrl;
    /** Command Ring Control */
    uint64_t crcr;
    uint32_t reserved_1[4];
    /** Device Context Base Address Array Pointer */
    uint64_t dcbaap;
    /** Configure */
    uint32_t config;
    uint32_t reserved_2[241];
    /** Port Control Registers */
    USBSSP_PortControlT portControl;
} __attribute__((packed));

struct USBSSP_InterrupterT_s {
    /** Interrupter Management */
    uint32_t iman;
    /** Interrupter Moderation */
    uint32_t imod;
    /** Event Ring Segment Table Size */
    uint32_t erstsz;
    uint32_t reserved;
    /** Event Ring Segment Table Base Address */
    uint64_t erstba;
    /** Event Ring Dequeue Pointer */
    uint64_t erdp;
} __attribute__((packed));

struct USBSSP_RuntimeT_s {
    /** Microframe Index */
    uint32_t mfindex;
    uint8_t reserved[28];
    /** Interrupter Register Sets */
    USBSSP_InterrupterT interrupters;
} __attribute__((packed));

/** Structure that describes single Extended Capability */
struct USBSSP_ExtCapElemT_s {
    /** Value of first 32bit word for this Ext. Cap. (DWORD[0]) */
    uint32_t firstDwordVal;
    /** Capability ID (DWORD[0].CapabilityID) */
    uint8_t capId;
    /** Pointer to first SFR belonging to this capability */
    uint32_t* firstCapSfrPtr;
};

struct USBSSP_ExtCapSetT_s {
    /** Address of first Extended Capabilities' SFR (XEC_USBLEGSUP) */
    uint32_t* extCapsBaseAddr;
    /** Contents of USBLEGSUP SFR */
    uint32_t usbLegSup;
    /** Contents of USBLEGCTLSTS SFR */
    uint32_t usbLegCtlSts;
    /** Array with Extended Capabilities */
    USBSSP_ExtCapElemT extCaps[USBSSP_MAX_EXT_CAPS_COUNT];
    /** Number of Extended Capabilities recognized */
    uint8_t extCapsCount;
};

struct USBSSP_SfrT_s {
    USBSSP_DDUSBConfigT* ddusbConfig;
    USBSSP_CapabilityT* xhciCapability;
    USBSSP_OperationalT* xhciOperational;
    USBSSP_PortControlT* xhciPortControl;
    USBSSP_RuntimeT* xhciRuntime;
    USBSSP_InterrupterT* xhciInterrupter;
    uint32_t* xhciDoorbell;
    /** xHCI capabilities are not handled as ordinary SFRs */
    USBSSP_ExtCapSetT xhciExtCaps;
    USBSSP_DDUSBEventRegsT ddusbEventRegs;
};

/**
 *  @}
 */

#endif /* CDN_XHCI_PRIV_H */
