
///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_ahci_ata.c
// PROJECT :
// KEYWORDS:
// LANGUAGE: C
// INTELLIPROP AUTHOR  : ehanke
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

#include "iprop_ata.h"

// void iprop_syraid_clear_tf(struct ata_taskfile * tf) {
void iprop_clear_tf (struct ata_taskfile *tf)
{
    tf->flags = 0;
    tf->protocol = 0;
    tf->ctl = 0;
    tf->hob_feature = 0;
    tf->hob_nsect = 0;
    tf->hob_lbal = 0;
    tf->hob_lbam = 0;
    tf->hob_lbah = 0;
    tf->feature = 0;
    tf->nsect = 0;
    tf->lbal = 0;
    tf->lbam = 0;
    tf->lbah = 0;
    tf->device = 0;
    tf->command = 0;
}

// Convert SATA Command fis into TaskFile structure
// void iprop_syraid_tf_from_fis (
void iprop_tf_from_fis (u32 *rfis_addr, struct ata_taskfile *tf, u8 flags,
                        u8 protocol)
{
    volatile int dw;
    dw = Iprop_RegRead32(rfis_addr, 0);
    tf->feature = (dw >> 24) & 0xFF;
    tf->command = (dw >> 16) & 0xFF;

    dw = Iprop_RegRead32(rfis_addr, 4);
    tf->device = (dw >> 24) & 0xFF;
    tf->lbah = (dw >> 16) & 0xFF;
    tf->lbam = (dw >> 8) & 0xFF;
    tf->lbal = (dw >> 0) & 0xFF;

    dw = Iprop_RegRead32(rfis_addr, 8);
    tf->hob_feature = (dw >> 24) & 0xFF;
    tf->hob_lbah = (dw >> 16) & 0xFF;
    tf->hob_lbam = (dw >> 8) & 0xFF;
    tf->hob_lbal = (dw >> 0) & 0xFF;

    dw = Iprop_RegRead32(rfis_addr, 12);
    tf->ctl = (dw >> 24) & 0xFF;
    tf->hob_nsect = (dw >> 8) & 0xFF;
    tf->nsect = (dw >> 0) & 0xFF;

    tf->flags = flags;
    tf->protocol = protocol;
    return;
}

// void iprop_syraid_tf_to_fis(struct ata_taskfile *tf, unsigned char pmp, int
// is_cmd, unsigned int * fis)
void iprop_tf_to_fis (struct ata_taskfile *tf, u8 pmp, int is_cmd, u32 *fis)
{
    fis[0] = (tf->feature << 24) | (tf->command << 16) |
             ((is_cmd == 1) << 15) | /*-- Command Bit --*/
             ((pmp & 0xf) << 8) |    /* Port multiplier number*/
             (0x27);                 /* Register - Host to Device FIS */

    fis[1] = (tf->device << 24) | (tf->lbah << 16) | (tf->lbam << 8) |
             (tf->lbal);

    fis[2] = (tf->hob_feature << 24) | (tf->hob_lbah << 16) |
             (tf->hob_lbam << 8) | (tf->hob_lbal);

    fis[3] = (tf->ctl << 24) | (tf->hob_nsect << 8) | (tf->nsect);

    fis[4] = 0;
}

// FPDMA version of iprop_tf_to_fis
void iprop_fpdma_tf_to_fis (struct ata_taskfile *tf, u8 pmp, int is_cmd,
                            u32 *fis, u32 commandSlot)
{
    tf->feature = tf->nsect & 0xFF;
    tf->hob_feature = tf->hob_nsect & 0xFF;

    fis[0] = ((u32)tf->feature << 24) | ((u32)tf->command << 16) |
             ((u32)(is_cmd == 1) << 15) | /*-- Command Bit --*/
             ((u32)(pmp & 0xf) << 8) |    /* Port multiplier number*/
             (0x27);                      /* Register - Host to Device FIS */

    fis[1] = ((u32)tf->device << 24) | ((u32)tf->lbah << 16) |
             ((u32)tf->lbam << 8) | ((u32)tf->lbal);

    fis[2] = ((u32)tf->hob_feature << 24) | ((u32)tf->hob_lbah << 16) |
             ((u32)tf->hob_lbam << 8) | ((u32)tf->hob_lbal);

    fis[3] = ((u32)tf->ctl << 24) | ((u32)(commandSlot & 0x1F) << 3);

    fis[4] = 0;
}
