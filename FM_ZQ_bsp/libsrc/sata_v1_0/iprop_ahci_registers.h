
///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_ahci_registers.h
// PROJECT :
// KEYWORDS:
// LANGUAGE: C
// MAINTAINED BY  : ehanke
//
// DESCRIPTION:
//
// TESTS USED/CREATED:
//
// REVISION HISTORY: Rev1.0
// Date     Person      Description
// -------- ----------- -------------------------------------------------------
//
// CURRENT ISSUES: none.
//
// REMAINING WORK:
//
//  Copyright 2004-2005 Red Hat, Inc.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __IPROP_AHCI_REGISTERS__
#define __IPROP_AHCI_REGISTERS__

enum {
    AHCI_MAX_PORTS = 32,
    AHCI_MAX_SG = 4, /* hardware max is 64K */
    AHCI_SG_ENTRY_SZ = 16,
    AHCI_DMA_BOUNDARY = 0xffffffff,
    AHCI_MAX_CMDS = 32,
    AHCI_CMD_SZ = 32,
    AHCI_CMD_SLOT_SZ = AHCI_MAX_CMDS * AHCI_CMD_SZ,
    AHCI_RX_FIS_SZ = 256,
    AHCI_CMD_TBL_CDB = 0x40,
    AHCI_CMD_TBL_HDR_SZ = 0x80,
    AHCI_CMD_TBL_SZ = AHCI_CMD_TBL_HDR_SZ + (AHCI_MAX_SG * AHCI_SG_ENTRY_SZ),
    AHCI_CMD_TBL_AR_SZ = AHCI_CMD_TBL_SZ * AHCI_MAX_CMDS,
    AHCI_PORT_PRIV_DMA_SZ = AHCI_CMD_SLOT_SZ + AHCI_CMD_TBL_AR_SZ +
                            AHCI_RX_FIS_SZ,
    // 32 * 32 + (0x80 + (16 * 16))*32 + 256 =
    AHCI_PORT_PRIV_FBS_DMA_SZ = AHCI_CMD_SLOT_SZ + AHCI_CMD_TBL_AR_SZ +
                                (AHCI_RX_FIS_SZ * 16),
    AHCI_IRQ_ON_SG = (1 << 31),
    AHCI_KEYHOLE_ON_SG = (1 << 30),
    AHCI_CMD_ATAPI = (1 << 5),
    AHCI_CMD_WRITE = (1 << 6),
    AHCI_CMD_PREFETCH = (1 << 7),
    AHCI_CMD_RESET = (1 << 8),
    AHCI_CMD_CLR_BUSY = (1 << 10),

    RX_FIS_D2H_REG = 0x40, /* offset of D2H Register FIS data */
    RX_FIS_SDB = 0x58,     /* offset of SDB FIS data */
    RX_FIS_UNK = 0x60,     /* offset of Unknown FIS data */

                           /* global controller registers */
    HOST_CAP = 0x00,        /* host capabilities */
    HOST_CTL = 0x04,        /* global host control */
    HOST_IRQ_STAT = 0x08,   /* interrupt status */
    HOST_PORTS_IMPL = 0x0c, /* bitmap of implemented ports */
    HOST_VERSION = 0x10,    /* AHCI spec. version compliancy */
    HOST_EM_LOC = 0x1c,     /* Enclosure Management location */
    HOST_EM_CTL = 0x20,     /* Enclosure Management Control */
    HOST_CAP2 = 0x24,       /* host capabilities, extended */

    /* HOST_CTL bits */
    HOST_RESET = (1 << 0),    /* reset controller; self-clear */
    HOST_IRQ_EN = (1 << 1),   /* global IRQ enable */
    HOST_AHCI_EN = (1 << 31), /* AHCI enabled */

    /* HOST_CAP bits */
    HOST_CAP_SXS = (1 << 5),        /* Supports External SATA */
    HOST_CAP_EMS = (1 << 6),        /* Enclosure Management support */
    HOST_CAP_CCC = (1 << 7),        /* Command Completion Coalescing */
    HOST_CAP_PART = (1 << 13),      /* Partial state capable */
    HOST_CAP_SSC = (1 << 14),       /* Slumber state capable */
    HOST_CAP_PIO_MULTI = (1 << 15), /* PIO multiple DRQ support */
    HOST_CAP_FBS = (1 << 16),       /* FIS-based switching support */
    HOST_CAP_PMP = (1 << 17),       /* Port Multiplier support */
    HOST_CAP_ONLY = (1 << 18),      /* Supports AHCI mode only */
    HOST_CAP_CLO = (1 << 24),       /* Command List Override support */
    HOST_CAP_LED = (1 << 25),       /* Supports activity LED */
    HOST_CAP_ALPM = (1 << 26),      /* Aggressive Link PM support */
    HOST_CAP_SSS = (1 << 27),       /* Staggered Spin-up */
    HOST_CAP_MPS = (1 << 28),       /* Mechanical presence switch */
    HOST_CAP_SNTF = (1 << 29),      /* SNotification register */
    HOST_CAP_NCQ = (1 << 30),       /* Native Command Queueing */
    HOST_CAP_64 = (1 << 31),        /* PCI DAC (64-bit DMA) support */

    /* HOST_CAP2 bits */
    HOST_CAP2_BOH = (1 << 0),    /* BIOS/OS handoff supported */
    HOST_CAP2_NVMHCI = (1 << 1), /* NVMHCI supported */
    HOST_CAP2_APST = (1 << 2),   /* Automatic partial to slumber */
    HOST_CAP2_SDS = (1 << 3),    /* Supports Device Sleep */
    HOST_CAP2_SADM = (1 << 4), /* Supports Aggressive Device Sleep Management */
    HOST_CAP2_DESO = (1 << 5), /* DevSleep Entrance from Slumber Only*/

    /* registers for each SATA port */
    PORT_LST_ADDR = 0x00,               /* command list DMA addr */
    PORT_LST_ADDR_HI = 0x04,            /* command list DMA addr hi */
    PORT_FIS_ADDR = 0x08,               /* FIS rx buf addr */
    PORT_FIS_ADDR_HI = 0x0c,            /* FIS rx buf addr hi */
    PORT_IRQ_STAT = 0x10,               /* interrupt status */
    PORT_IRQ_MASK = 0x14,               /* interrupt enable/disable mask */
    PORT_CMD = 0x18,                    /* port command */
    PORT_TFDATA = 0x20,                 /* taskfile data */
    PORT_SIG = 0x24,                    /* device TF signature */
    PORT_CMD_ISSUE = 0x38,              /* command issue */
    PORT_SCR_STAT = 0x28,               /* SATA phy register: SStatus */
    PORT_SCR_CTL = 0x2c,                /* SATA phy register: SControl */
    PORT_SCR_ERR = 0x30,                /* SATA phy register: SError */
    PORT_SCR_ACT = 0x34,                /* SATA phy register: SActive */
    PORT_SCR_NTF = 0x3c,                /* SATA phy register: SNotification */
    PORT_FBS = 0x40,                    /* FIS-based Switching */
                                        /* PORT_IRQ_{STAT,MASK} bits */
    PORT_IRQ_COLD_PRES = (1 << 31),     /* cold presence detect */
    PORT_IRQ_TF_ERR = (1 << 30),        /* task file error */
    PORT_IRQ_HBUS_ERR = (1 << 29),      /* host bus fatal error */
    PORT_IRQ_HBUS_DATA_ERR = (1 << 28), /* host bus data error */
    PORT_IRQ_IF_ERR = (1 << 27),        /* interface fatal error */
    PORT_IRQ_IF_NONFATAL = (1 << 26),   /* interface non-fatal error */
    PORT_IRQ_OVERFLOW = (1 << 24),      /* xfer exhausted available S/G */
    PORT_IRQ_BAD_PMP = (1 << 23),       /* incorrect port multiplier */

    PORT_IRQ_PHYRDY = (1 << 22),        /* PhyRdy changed */
    PORT_IRQ_DEV_ILCK = (1 << 7),       /* device interlock */
    PORT_IRQ_CONNECT = (1 << 6),        /* port connect change status */
    PORT_IRQ_SG_DONE = (1 << 5),        /* descriptor processed */
    PORT_IRQ_UNK_FIS = (1 << 4),        /* unknown FIS rx'd */
    PORT_IRQ_SDB_FIS = (1 << 3),        /* Set Device Bits FIS rx'd */
    PORT_IRQ_DMAS_FIS = (1 << 2),       /* DMA Setup FIS rx'd */
    PORT_IRQ_PIOS_FIS = (1 << 1),       /* PIO Setup FIS rx'd */
    PORT_IRQ_D2H_REG_FIS = (1 << 0),    /* D2H Register FIS rx'd */

    PORT_IRQ_FREEZE = PORT_IRQ_HBUS_ERR | PORT_IRQ_IF_ERR | PORT_IRQ_CONNECT |
                      PORT_IRQ_PHYRDY | PORT_IRQ_UNK_FIS | PORT_IRQ_BAD_PMP,
    PORT_IRQ_ERROR = PORT_IRQ_FREEZE | PORT_IRQ_TF_ERR | PORT_IRQ_HBUS_DATA_ERR,
    DEF_PORT_IRQ = PORT_IRQ_ERROR | PORT_IRQ_SG_DONE | PORT_IRQ_SDB_FIS |
                   PORT_IRQ_DMAS_FIS | PORT_IRQ_PIOS_FIS | PORT_IRQ_D2H_REG_FIS,

    /* PORT_CMD bits */
    PORT_CMD_ASP = (1 << 27),            /* Aggressive Slumber/Partial */
    PORT_CMD_ALPE = (1 << 26),           /* Aggressive Link PM enable */
    PORT_CMD_ATAPI = (1 << 24),          /* Device is ATAPI */
    PORT_CMD_FBSCP = (1 << 22),          /* FBS Capable Port */
    PORT_CMD_PMP = (1 << 17),            /* PMP attached */
    PORT_CMD_LIST_ON = (1 << 15),        /* cmd list DMA engine running */
    PORT_CMD_FIS_ON = (1 << 14),         /* FIS DMA engine running */
    PORT_CMD_FIS_RX = (1 << 4),          /* Enable FIS receive DMA engine */
    PORT_CMD_CLO = (1 << 3),             /* Command list override */
    PORT_CMD_POWER_ON = (1 << 2),        /* Power up device */
    PORT_CMD_SPIN_UP = (1 << 1),         /* Spin up device */
    PORT_CMD_START = (1 << 0),           /* Enable port DMA engine */

    PORT_CMD_ICC_MASK = (0xf << 28),     /* i/f ICC state mask */
    PORT_CMD_ICC_ACTIVE = (0x1 << 28),   /* Put i/f in active state */
    PORT_CMD_ICC_PARTIAL = (0x2 << 28),  /* Put i/f in partial state */
    PORT_CMD_ICC_SLUMBER = (0x6 << 28),  /* Put i/f in slumber state */
    PORT_CMD_ICC_DEVSLEEP = (0x8 << 28), /* Put i/f in slumber state */
    PORT_FBS_DWE_OFFSET = 16,            /* FBS device with error offset */
    PORT_FBS_ADO_OFFSET = 12, /* FBS active dev optimization offset */
    PORT_FBS_DEV_OFFSET = 8,  /* FBS device to issue offset */
    PORT_FBS_DEV_MASK = (0xf << PORT_FBS_DEV_OFFSET), /* FBS.DEV */
    PORT_FBS_SDE = (1 << 2), /* FBS single device error */
    PORT_FBS_DEC = (1 << 1), /* FBS device error clear */
    PORT_FBS_EN = (1 << 0),  /* Enable FBS */

    /* hpriv->flags bits */
    AHCI_HFLAG_NO_NCQ = (1 << 0),
    AHCI_HFLAG_IGN_IRQ_IF_ERR = (1 << 1),        /* ignore IRQ_IF_ERR */
    AHCI_HFLAG_IGN_SERR_INTERNAL = (1 << 2),     /* ignore SERR_INTERNAL */
    AHCI_HFLAG_32BIT_ONLY = (1 << 3),            /* force 32bit */
    AHCI_HFLAG_MV_PATA = (1 << 4),               /* PATA port */
    AHCI_HFLAG_NO_MSI = (1 << 5),                /* no PCI MSI */
    AHCI_HFLAG_NO_PMP = (1 << 6),                /* no PMP */
    AHCI_HFLAG_NO_HOTPLUG = (1 << 7),            /* ignore PxSERR.DIAG.N */
    AHCI_HFLAG_SECT255 = (1 << 8),               /* max 255 sectors */
    AHCI_HFLAG_YES_NCQ = (1 << 9),               /* force NCQ cap on */
    AHCI_HFLAG_NO_SUSPEND = (1 << 10),           /* don't suspend */
    AHCI_HFLAG_SRST_TOUT_IS_OFFLINE = (1 << 11), /* treat SRST timeout as
                link offline */
    AHCI_HFLAG_NO_SNTF = (1 << 12),              /* no sntf */
    AHCI_HFLAG_NO_FPDMA_AA = (1 << 13),          /* no FPDMA AA */
    AHCI_HFLAG_YES_FBS = (1 << 14),              /* force FBS cap on */

    /* ap->flags bits */

    // AHCI_FLAG_COMMON    = ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
    // ATA_FLAG_MMIO | ATA_FLAG_PIO_DMA |
    // ATA_FLAG_ACPI_SATA | ATA_FLAG_AN |
    // ATA_FLAG_IPM,

    ICH_MAP = 0x90, /* ICH MAP register */

    /* em constants */
    EM_MAX_SLOTS = 8,
    EM_MAX_RETRY = 5,

    /* em_ctl bits */
    EM_CTL_RST = (1 << 9),   /* Reset */
    EM_CTL_TM = (1 << 8),    /* Transmit Message */
    EM_CTL_MR = (1 << 0),    /* Message Recieved */
    EM_CTL_ALHD = (1 << 26), /* Activity LED */
    EM_CTL_XMT = (1 << 25),  /* Transmit Only */
    EM_CTL_SMB = (1 << 24),  /* Single Message Buffer */

    /* em message type */
    EM_MSG_TYPE_LED = (1 << 0),   /* LED */
    EM_MSG_TYPE_SAFTE = (1 << 1), /* SAF-TE */
    EM_MSG_TYPE_SES2 = (1 << 2),  /* SES-2 */
    EM_MSG_TYPE_SGPIO = (1 << 3), /* SGPIO */
};

#define IPROP_REGISTER_PORT_OFFSET 0x80

/*-------------------------------------*/
/* BLOCK_NAME AHCI_GENERIC_HC Registers*/
/*-------------------------------------*/
#define BASE_ADDRESS_AHCI_HC       0x00
// REG_FILE_SIZE 0x100
// INSTANTIATION yes
#define AHCI_CAP                   0x00
#define AHCI_GHC                   0x04
#define AHCI_IS                    0x08
#define AHCI_PI                    0x0C
#define AHCI_VS                    0x10
#define AHCI_CCC_CTL               0x14
#define AHCI_CCC_PORTS             0x18
#define AHCI_EM_LOC                0x1C
#define AHCI_EM_CTL                0x20
#define AHCI_CAP2                  0x24
#define AHCI_BOHC                  0x28
/*BLOCK END*/

/*---------------------------------*/
/* BLOCK_NAME AHCI_PORT_0 Registers*/
/*---------------------------------*/
#define BASE_ADDRESS_AHCI_P0       0x100
#define BASE_ADDRESS_AHCI_PORT(PortNum) \
    (0x100 + (IPROP_REGISTER_PORT_OFFSET * (PortNum)))

// REG_FILE_SIZE 0x100
// INSTANTIATION yes
#define AHCI_PxCLB    0x00
#define AHCI_PxCLBU   0x04
#define AHCI_PxFB     0x08
#define AHCI_PxFBU    0x0C
#define AHCI_PxIS     0x10
#define AHCI_PxIE     0x14
#define AHCI_PxCMD    0x18
#define AHCI_PxTFD    0x20
#define AHCI_PxSIG    0x24
#define AHCI_PxSSTS   0x28
#define AHCI_PxSCTL   0x2C
#define AHCI_PxSERR   0x30
#define AHCI_PxSACT   0x34
#define AHCI_PxCI     0x38
#define AHCI_PxSNTF   0x3C
#define AHCI_PxFBS    0x40
#define AHCI_PxDEVSLP 0x44
#define AHCI_PxVS_0   0x70
#define AHCI_PxVS_1   0x74
#define AHCI_PxVS_2   0x78
#define AHCI_PxVS_3   0x7C
/*BLOCK END*/

#endif  // #define __IPROP_AHCI_REGISTERS__
