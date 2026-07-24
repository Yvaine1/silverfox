
///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_ahci_cmds.h
// PROJECT :
// KEYWORDS:
// LANGUAGE: C
// INTELLIPROP AUTHOR  : hsheffield
// CREATED : 2/4/13
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
//
// This media contains an authorized copy or copies of material owned by
// Intelliprop Inc.  This ownership notice and any
// other notices included in machine readable copies must be reproduced on all
// authorized copies.
//
// This is confidential and unpublished property of Intelliprop Inc.
//
// All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef IPROP_AHCI_CMDS
#define IPROP_AHCI_CMDS

#include "iprop_ahci_core.h"
#include "iprop_ahci_registers.h"
#include "iprop_ata.h"
#include "iprop_lib.h"
#include "iprop_types.h"

u32 iprop_ahci_identify(Iprop_AHCI_HostDesc *hd, Iprop_AHCI_PortDesc *pd,
                        u8 *id_data);

u32 iprop_ahci_poll_RdCmd(Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                          u16 sect);

u32 iprop_ahci_poll_WrCmd(Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                          u16 sect);

u32 iprop_ahci_poll_fpdmaRd(Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                            u16 sect);

u32 iprop_ahci_poll_fpdmaWr(Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                            u16 sect);

u32 iprop_ahci_poll_pio_RdCmd(Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                              u16 sect);

u32 iprop_ahci_poll_pio_WrCmd(Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                              u16 sect);

u32 iprop_ahci_transfer_Partial_state(Iprop_AHCI_PortDesc *ap);

u32 iprop_ahci_transfer_Slumber_state(Iprop_AHCI_PortDesc *ap);

u32 iprop_ahci_transfer_Active_state(Iprop_AHCI_PortDesc *ap);

void power_management_test(Iprop_AHCI_PortDesc *ap);

u32 iprop_ahci_bist(Iprop_AHCI_PortDesc *pd);

void Speed_negotiated_test(int PortNum, u32 SpeedGen);

#endif
