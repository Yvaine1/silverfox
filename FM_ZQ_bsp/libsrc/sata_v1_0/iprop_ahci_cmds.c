
///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_ahci_cmds.c
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

// #include <sys/alt_irq.h>
#include "iprop_ahci_cmds.h"
#include "iprop_ahci_core.h"
#include "iprop_ahci_registers.h"
#include "iprop_ata.h"
#include "iprop_reg_hw.h"
#include "iprop_types.h"

// Send an Identify

u32 iprop_ahci_identify (Iprop_AHCI_HostDesc *hd, Iprop_AHCI_PortDesc *pd,
                         u8 *id_data)
{
    struct ata_taskfile tf;
    Iprop_AHCI_PRD prd;
    u16 drive_model1;
    u8 *drive_model_addr1;
    u16 serial_num;
    u8 *serial_num_addr;
    u32 ii;
    u8 drive_data[21];
    u8 serial_data[25];
    prd.DBA = (u32)id_data;
    prd.DBAU = 0;
    prd.DBC = ATA_SECT_SIZE - 1;

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_PIO;
    tf.flags = ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
    tf.command = ATA_CMD_ID_ATA;

    /*****************************************************************************
     *-- Send IDENTIFY DEVICE Commands to attached Device --
     * Returned Identify Data will be stored in a 512Byte Array pointed to by
     * id_data[PortNumber]
     *****************************************************************************/
    Iprop_AHCI_SendDMTFCmd(pd, &tf, 0, 1, (Iprop_AHCI_PRD *)&prd);

    // wait for ID to finish
    while ((Iprop_AHCI_CheckCommandSlotBusy((u32 *)pd->AhciBaseAddress, 0x1,
                                            pd->PortNum)) != 0)
    {
        // check for errors
        if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
            (PORT_IRQ_ERROR))
        {
            iprop_printf(
                "Error Occurred while waiting for command slot to finish");
            iprop_printf(
                "\t AHCI P[%x]-IS Register %x \n\r", pd->PortNum,
                Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum));
        }
    }
    iprop_printf("Identify Command Sent\n\r");

    iprop_printf("Serial Number of the Device:  ");
    // Print out Drive Model
    u32 jj = 0;
    for (ii = 20; ii < 43; ii++)
    {
        drive_model_addr1 = id_data + ii;
        // iprop_printf("0x%08X \n\r", drive_model_addr);
        drive_model1 = *drive_model_addr1;
        drive_data[jj] = drive_model1;
        // drive_model_addr2 = id_data + (ii+1);
        // drive_model2 = *drive_model_addr2;
        // iprop_printf("\n\r");
        // iprop_printf("%c \n\r", drive_model1);
        // iprop_printf("%c \n\r", drive_data[ii]);
        jj++;
    }
    ii = 0;
    while (ii < 21)
    {
        iprop_printf("%c", drive_data[ii + 1]);
        iprop_printf("%c", drive_data[ii]);
        ii = ii + 2;
    }
    iprop_printf("\r\n");
    iprop_printf("Device Model:  ");
    // Print out Serial Number
    jj = 0;
    for (ii = 54; ii < 84; ii++)
    {
        serial_num_addr = id_data + ii;
        serial_num = *serial_num_addr;
        serial_data[jj] = serial_num;
        // iprop_printf("0x%08X \n\r", serial_num_addr);
        // iprop_printf("%c \n\r", serial_num);
        // iprop_printf("\n\r");
        jj++;
    }

    ii = 0;
    while (ii < 25)
    {
        iprop_printf("%c", serial_data[ii + 1]);
        iprop_printf("%c", serial_data[ii]);
        ii = ii + 2;
    }
    iprop_printf("\n\r");
    return 0;
}

// Send a polled Read Command, DMA RD command

u32 iprop_ahci_poll_RdCmd (Iprop_AHCI_PortDesc *pd, u32 *dma_pool,
                           u64 LBA,  // lba is 48-bits...
                           u16 sect)
{
    struct ata_taskfile tf;
    Iprop_AHCI_PRD prd;
    u32 slot;

    prd.DBA = (u32)dma_pool;
    prd.DBAU = 0;
    prd.Reserved = 0;
    prd.DBC = (ATA_SECT_SIZE * sect - 1);

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_DMA;

    tf.flags = (ATA_TFLAG_LBA48 | ATA_TFLAG_DEVICE);
    tf.command = ATA_CMD_READ_EXT;
    // tf.command = ATA_CMD_PMP_READ;
    tf.device = ATA_LBA;
    tf.hob_lbah = (LBA & 0xff0000000000) >> 40;
    tf.hob_lbam = (LBA & 0x00ff00000000) >> 32;
    tf.hob_lbal = (LBA & 0x0000ff000000) >> 24;
    tf.lbah = (LBA & 0x000000ff0000) >> 16;
    tf.lbam = (LBA & 0x00000000ff00) >> 8;
    tf.lbal = (LBA & 0x0000000000ff) >> 0;
    tf.hob_nsect = (sect & 0xff00) >> 8;
    tf.nsect = (sect & 0x00ff) >> 0;

    // find an open command slot
    slot = Iprop_AHCI_FindOpenSlot(pd, 32);

    if (slot > 32)
    {
        return IPROP_STATUS_ERR;
    }

    // send ATA command to port

    Iprop_AHCI_SendDMTFCmd(pd, &tf, slot, 1, (Iprop_AHCI_PRD *)&prd);

    // wait for ID to finish
    while ((Iprop_AHCI_CheckCommandSlotBusy((u32 *)pd->AhciBaseAddress,
                                            (1 << slot), pd->PortNum)) != 0)
    {
        // check for errors
        if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
            (PORT_IRQ_ERROR))
        {
            iprop_printf(
                "Error Occurred while waiting for command slot to finish");
            iprop_printf(
                "\t AHCI P[%x]-IS Register %x \n\r", pd->PortNum,
                Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum));
        }
    }

    // wait for data to complete
    return 0;
}

// Send a polled Write Command, DMA WR command
u32 iprop_ahci_poll_WrCmd (Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                           u16 sect)
{
    struct ata_taskfile tf;
    Iprop_AHCI_PRD prd;
    // u32 * ahci_addr = (u32*)pd[0]->AhciBaseAddress;
    u32 slot;
    prd.DBA = (u32)dma_pool;
    prd.DBAU = 0;
    prd.Reserved = 0;
    prd.DBC = (ATA_SECT_SIZE * sect) - 1;

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_DMA;
    tf.command = ATA_CMD_WRITE_EXT;
    // tf.command = ATA_CMD_PMP_WRITE;

    tf.flags = (ATA_TFLAG_WRITE | ATA_TFLAG_LBA48);
    tf.device = ATA_LBA;
    tf.hob_lbah = (LBA & 0xff0000000000) >> 40;
    tf.hob_lbam = (LBA & 0x00ff00000000) >> 32;
    tf.hob_lbal = (LBA & 0x0000ff000000) >> 24;
    tf.lbah = (LBA & 0xff0000) >> 16;
    tf.lbam = (LBA & 0x00ff00) >> 8;
    tf.lbal = (LBA & 0x0000ff) >> 0;
    tf.hob_nsect = (sect & 0xff00) >> 8;
    tf.nsect = (sect & 0x00ff) >> 0;

    slot = Iprop_AHCI_FindOpenSlot(
        pd,
        32);  // if no open slot, use a slot which is not the current command
              // slot. The reason for this is to minimize the disturbance to the
              // command header info for the hung command.

    if (slot > 32)
    {
        return IPROP_STATUS_ERR;
    }

    // send ATA command to port
    Iprop_AHCI_SendDMTFCmd(pd, &tf, slot, 1, (Iprop_AHCI_PRD *)&prd);

    while ((Iprop_AHCI_CheckCommandSlotBusy((u32 *)pd->AhciBaseAddress,
                                            (1 << slot), pd->PortNum)) != 0)
    {
        // check for errors
        if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
            (PORT_IRQ_ERROR))
        {
            iprop_printf(
                "Error Occurred while waiting for command slot to finish\n\r");
            iprop_printf(
                "\t AHCI P[%x]-IS Register %x \n\r", pd->PortNum,
                Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum));
        }
    }
    // wait for data to complete
    return 0;
}

u32 iprop_ahci_poll_fpdmaWr (Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                             u16 sect)
{
    struct ata_taskfile tf;
    Iprop_AHCI_PRD prd;

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_NCQ;
    tf.command = ATA_CMD_FPDMA_WRITE;
    tf.flags = (ATA_TFLAG_WRITE | ATA_TFLAG_LBA48);
    tf.device = ATA_LBA;
    tf.hob_lbah = (LBA & 0xff0000000000) >> 40;
    tf.hob_lbam = (LBA & 0x00ff00000000) >> 32;
    tf.hob_lbal = (LBA & 0x0000ff000000) >> 24;
    tf.lbah = (LBA & 0xff0000) >> 16;
    tf.lbam = (LBA & 0x00ff00) >> 8;
    tf.lbal = (LBA & 0x0000ff) >> 0;
    tf.hob_nsect = (sect & 0xff00) >> 8;
    tf.nsect = (sect & 0x00ff) >> 0;

    prd.DBA = (u32)dma_pool;
    prd.DBAU = 0;
    prd.Reserved = 0;
    prd.DBC = (ATA_SECT_SIZE * sect) - 1;

    u8 slot;
    u32 SActive;
    u32 Empty;
    u32 SErr;
    // Read PxSACT Register
    SActive = Iprop_RegRead32(
        (u32 *)pd->AhciBaseAddress,
        BASE_ADDRESS_AHCI_PORT(pd->PortNum) + AHCI_PxSACT);
    // iprop_printf("SActive :: 0x%08X\n\r", SActive);

    while (1)
    {
        if (SActive != 0xFFFFFFFF)
        {
            // Check for empty slot in the PxSACT register
            Empty = iprop_get_lowest_cleared(SActive);
            // convert to decimal
            slot = iprop_oneHot2Dec(Empty);
            break;
        }
        SActive = Iprop_RegRead32(
            (u32 *)pd->AhciBaseAddress,
            BASE_ADDRESS_AHCI_PORT(pd->PortNum) + AHCI_PxSACT);
    }

    // send ATA command to port
    Iprop_AHCI_SendDMTFCmd(pd, &tf, slot, 1, (Iprop_AHCI_PRD *)&prd);

    // check for errors
    if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
        (PORT_IRQ_ERROR))
    {
        //  iprop_printf("Error Occurred while waiting for command slot to
        //  finish\n\r"); iprop_printf ("\t AHCI P[%x]-IS Register %x \n\r",
        //  pd->PortNum, Iprop_AHCI_GetISReg((u32*)pd->AhciBaseAddress,
        //  pd->PortNum));
    }

    return 0;
}

u32 iprop_ahci_poll_fpdmaRd (Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                             u16 sect)
{
    struct ata_taskfile tf;
    Iprop_AHCI_PRD prd;

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_NCQ;
    tf.command = ATA_CMD_FPDMA_READ;
    tf.flags = (ATA_TFLAG_LBA48 | ATA_TFLAG_DEVICE);
    tf.device = ATA_LBA;
    tf.hob_lbah = (LBA & 0xff0000000000) >> 40;
    tf.hob_lbam = (LBA & 0x00ff00000000) >> 32;
    tf.hob_lbal = (LBA & 0x0000ff000000) >> 24;
    tf.lbah = (LBA & 0xff0000) >> 16;
    tf.lbam = (LBA & 0x00ff00) >> 8;
    tf.lbal = (LBA & 0x0000ff) >> 0;
    tf.hob_nsect = (sect & 0xff00) >> 8;
    tf.nsect = (sect & 0x00ff) >> 0;
    u8 slot;
    u32 SActive;
    u32 Empty;

    prd.DBA = (u32)dma_pool;
    prd.DBAU = 0;
    prd.Reserved = 0;
    prd.DBC = (ATA_SECT_SIZE * sect) - 1;

    // Read PxSACT Register
    SActive = Iprop_RegRead32(
        (u32 *)pd->AhciBaseAddress,
        BASE_ADDRESS_AHCI_PORT(pd->PortNum) + AHCI_PxSACT);
    // iprop_printf("SActive: 0x%08X \n\r", SActive);
    while (1)
    {
        if (SActive != 0xFFFFFFFF)
        {
            // Check for empty slot in the PxSACT register
            Empty = iprop_get_lowest_cleared(Iprop_RegRead32(
                (u32 *)pd->AhciBaseAddress,
                BASE_ADDRESS_AHCI_PORT(pd->PortNum) + AHCI_PxSACT));
            // convert to decimal
            slot = iprop_oneHot2Dec(Empty);
            break;
        }
        SActive = Iprop_RegRead32(
            (u32 *)pd->AhciBaseAddress,
            BASE_ADDRESS_AHCI_PORT(pd->PortNum) + AHCI_PxSACT);
    }

    // send ATA command to port
    Iprop_AHCI_SendDMTFCmd(pd, &tf, slot, 1, (Iprop_AHCI_PRD *)&prd);

    if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
        (PORT_IRQ_ERROR))
    {
        // iprop_printf("Error Occurred while waiting for command slot to
        // finish\n\r"); iprop_printf ("\t AHCI P[%x]-IS Register %x \n\r",
        // pd->PortNum, Iprop_AHCI_GetISReg((u32*)pd->AhciBaseAddress,
        // pd->PortNum));
    }

    return 0;
}

// Send a polled Read Command ,PIO RD Command

u32 iprop_ahci_poll_pio_RdCmd (Iprop_AHCI_PortDesc *pd, u32 *dma_pool,
                               u64 LBA,  // lba is 48-bits...
                               u16 sect)
{
    struct ata_taskfile tf;
    Iprop_AHCI_PRD prd;
    u32 slot;

    prd.DBA = (u32)dma_pool;
    prd.DBAU = 0;
    prd.Reserved = 0;
    prd.DBC = (ATA_SECT_SIZE * sect - 1);

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_DMA;

    tf.flags = (ATA_TFLAG_LBA48 | ATA_TFLAG_DEVICE);
    // tf.command = ATA_CMD_READ_EXT;
    tf.command = ATA_CMD_PMP_READ;
    tf.device = ATA_LBA;
    tf.hob_lbah = (LBA & 0xff0000000000) >> 40;
    tf.hob_lbam = (LBA & 0x00ff00000000) >> 32;
    tf.hob_lbal = (LBA & 0x0000ff000000) >> 24;
    tf.lbah = (LBA & 0x000000ff0000) >> 16;
    tf.lbam = (LBA & 0x00000000ff00) >> 8;
    tf.lbal = (LBA & 0x0000000000ff) >> 0;
    tf.hob_nsect = (sect & 0xff00) >> 8;
    tf.nsect = (sect & 0x00ff) >> 0;

    // find an open command slot
    slot = Iprop_AHCI_FindOpenSlot(pd, 32);

    if (slot > 32)
    {
        return IPROP_STATUS_ERR;
    }

    // send ATA command to port

    Iprop_AHCI_SendDMTFCmd(pd, &tf, slot, 1, (Iprop_AHCI_PRD *)&prd);

    // wait for ID to finish
    while ((Iprop_AHCI_CheckCommandSlotBusy((u32 *)pd->AhciBaseAddress,
                                            (1 << slot), pd->PortNum)) != 0)
    {
        // check for errors
        if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
            (PORT_IRQ_ERROR))
        {
            iprop_printf(
                "Error Occurred while waiting for command slot to finish");
            iprop_printf(
                "\t AHCI P[%x]-IS Register %x \n\r", pd->PortNum,
                Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum));
        }
    }

    // wait for data to complete
    return 0;
}

// Send a polled Write Command, PIO WR Command
u32 iprop_ahci_poll_pio_WrCmd (Iprop_AHCI_PortDesc *pd, u32 *dma_pool, u64 LBA,
                               u16 sect)
{
    struct ata_taskfile tf;
    Iprop_AHCI_PRD prd;
    // u32 * ahci_addr = (u32*)pd[0]->AhciBaseAddress;
    u32 slot;
    prd.DBA = (u32)dma_pool;
    prd.DBAU = 0;
    prd.Reserved = 0;
    prd.DBC = (ATA_SECT_SIZE * sect) - 1;

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_DMA;
    // tf.command = ATA_CMD_WRITE_EXT;
    tf.command = ATA_CMD_PMP_WRITE;

    tf.flags = (ATA_TFLAG_WRITE | ATA_TFLAG_LBA48);
    tf.device = ATA_LBA;
    tf.hob_lbah = (LBA & 0xff0000000000) >> 40;
    tf.hob_lbam = (LBA & 0x00ff00000000) >> 32;
    tf.hob_lbal = (LBA & 0x0000ff000000) >> 24;
    tf.lbah = (LBA & 0xff0000) >> 16;
    tf.lbam = (LBA & 0x00ff00) >> 8;
    tf.lbal = (LBA & 0x0000ff) >> 0;
    tf.hob_nsect = (sect & 0xff00) >> 8;
    tf.nsect = (sect & 0x00ff) >> 0;

    slot = Iprop_AHCI_FindOpenSlot(
        pd,
        32);  // if no open slot, use a slot which is not the current command
              // slot. The reason for this is to minimize the disturbance to the
              // command header info for the hung command.

    if (slot > 32)
    {
        return IPROP_STATUS_ERR;
    }

    // send ATA command to port
    Iprop_AHCI_SendDMTFCmd(pd, &tf, slot, 1, (Iprop_AHCI_PRD *)&prd);

    while ((Iprop_AHCI_CheckCommandSlotBusy((u32 *)pd->AhciBaseAddress,
                                            (1 << slot), pd->PortNum)) != 0)
    {
        // check for errors
        if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
            (PORT_IRQ_ERROR))
        {
            iprop_printf(
                "Error Occurred while waiting for command slot to finish\n\r");
            iprop_printf(
                "\t AHCI P[%x]-IS Register %x \n\r", pd->PortNum,
                Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum));
        }
    }
    // wait for data to complete
    return 0;
}

// Enter Partial state
u32 iprop_ahci_transfer_Partial_state (Iprop_AHCI_PortDesc *ap)
{
    // check port whether actived ?
    // u32 state;
    unsigned int addr = ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    unsigned int rc = Iprop_RegRead32(
        (u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSSTS);
    if ((rc & 0xf00) != 0x100)
    {
        iprop_printf(
            "Current state is not active state,Failed to enter Partial "
            "state!!!\n\r");
        return 1;
    }

    rc = Iprop_RegRead32((u32 *)addr,
                         BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCMD);
    if ((rc & 0xf0000000) != 0x0)
    {
        iprop_printf(
            "Current state is not ilde state,Failed to enter Partial "
            "state!!!\n\r");
        return 1;
    }

    rc = rc & 0xfffffff | 0x20000000;
    Iprop_RegWrite32((u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCMD,
                     rc);

    for (int ii = 0; ii < 0x100000; ii++)
    {
        rc = Iprop_RegRead32((u32 *)addr,
                             BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSSTS);
        if ((rc & 0xf00) == 0x200)
        {
            iprop_printf(
                "Current state is Partial state,Success to enter Partial "
                "state!!!\n\r");
            // clear interrupt phy ready bit PxIS[22] ,set PxSERR bit[18] and
            // bit[16]
            Iprop_RegWrite32((u32 *)addr,
                             BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSERR,
                             0x50000);
            rc = Iprop_RegRead32((u32 *)addr,
                                 BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxIS);
            while (rc != 0x0);
            return 0;
        }
    }

    iprop_printf(
        " Time out,Current state is not Partial state,Failed to enter Partial "
        "state!!   !!!\n\r");
    return 1;
}

// Enter Slumber state
u32 iprop_ahci_transfer_Slumber_state (Iprop_AHCI_PortDesc *ap)
{
    // check port whether actived ?
    // u32 state;
    unsigned int addr = ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    unsigned int rc = Iprop_RegRead32(
        (u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSSTS);
    if ((rc & 0xf00) != 0x100)
    {
        iprop_printf(
            "Current state is not active state,Failed to enter Slumber "
            "state!!!\n\r");
        return 1;
    }

    rc = Iprop_RegRead32((u32 *)addr,
                         BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCMD);
    if ((rc & 0xf0000000) != 0x0)
    {
        iprop_printf(
            "Current state is not ilde state,Failed to enter Slumber "
            "state!!!\n\r");
        return 1;
    }

    rc = rc & 0xfffffff | 0x60000000;
    Iprop_RegWrite32((u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCMD,
                     rc);

    for (int ii = 0; ii < 0x100000; ii++)
    {
        rc = Iprop_RegRead32((u32 *)addr,
                             BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSSTS);
        if ((rc & 0xf00) == 0x600)
        {
            iprop_printf(
                "Current state is Slumber state,Success to enter Slumber "
                "state!!!\n\r");
            // clear interrupt phy ready bit PxIS[22] ,set PxSERR bit[18] and
            // bit[16]
            Iprop_RegWrite32((u32 *)addr,
                             BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSERR,
                             0x50000);
            rc = Iprop_RegRead32((u32 *)addr,
                                 BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxIS);
            while (rc != 0x0);
            return 0;
        }
    }

    iprop_printf(
        " Time out,Current state is not Slumber state,Failed to enter Slumber "
        "state!!   !!!\n\r");
    return 1;
}

// Enter Active state
u32 iprop_ahci_transfer_Active_state (Iprop_AHCI_PortDesc *ap)
{
    // check port whether actived ?
    // u32 state;
    unsigned int addr = ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    unsigned int rc = Iprop_RegRead32(
        (u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSSTS);
    if ((rc & 0xf00) == 0x100)
    {
        iprop_printf("Current state is aready active state !!!\n\r");
        return 1;
    }

    rc = Iprop_RegRead32((u32 *)addr,
                         BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCMD);
    if ((rc & 0xf0000000) != 0x0)
    {
        iprop_printf(
            "Current state is not ilde state,Failed to enter active "
            "state!!!\n\r");
        return 1;
    }

    rc = rc & 0xfffffff | 0x10000000;
    Iprop_RegWrite32((u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCMD,
                     rc);

    for (int ii = 0; ii < 0x100000; ii++)
    {
        rc = Iprop_RegRead32((u32 *)addr,
                             BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSSTS);
        if ((rc & 0xf00) == 0x100)
        {
            iprop_printf(
                "Current state is active state,Success to enter active "
                "state!!!\n\r");

            // clear interrupt phy ready bit PxIS[22] ,set PxSERR bit[18] and
            // bit[16]
            Iprop_RegWrite32((u32 *)addr,
                             BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSERR,
                             0x50000);
            rc = Iprop_RegRead32((u32 *)addr,
                                 BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxIS);
            while (rc != 0x0);
            return 0;
        }
    }

    iprop_printf(
        " Time out,Current state is not active state,Failed to enter active "
        "state!!   !!!\n\r");
    return 1;
}

void power_management_test (Iprop_AHCI_PortDesc *ap)
{
    u32 state;
    iprop_ahci_transfer_Active_state(ap);

    // enter partial state
    state = iprop_ahci_transfer_Partial_state(ap);
    if (state == 0)
    {
        iprop_printf(
            "Power management state :From Active to Partial test passed!\n\r");
    }
    else
    {
        iprop_printf(
            "Power management state :From Active to Partial test failed!\n\r");
    }
    // recover  active  state
    state = iprop_ahci_transfer_Active_state(ap);
    if (state == 0)
    {
        iprop_printf(
            "Power management state :From Partial to Active state passed!\n\r");
    }
    else
    {
        iprop_printf(
            "Power management state :From Partial to Active state failed!\n\r");
    }

    // enter slubmer  state
    state = iprop_ahci_transfer_Slumber_state(ap);
    if (state == 0)
    {
        iprop_printf(
            "Power management state :From Active to Slumber test passed!\n\r");
    }
    else
    {
        iprop_printf(
            "Power management state :From Active to Slumber test failed!\n\r");
    }

    // recover active  state
    state = iprop_ahci_transfer_Active_state(ap);
    if (state == 0)
    {
        iprop_printf(
            "Power management state :From Slumber to Active state passed!\n\r");
    }
    else
    {
        iprop_printf(
            "Power management state :From Slumber to Active state failed!\n\r");
    }
}

// Send a BIST Actived fis
u32 iprop_ahci_bist (Iprop_AHCI_PortDesc *pd)
{
    u32 *fis;
    // u32 * ahci_addr = (u32*)pd[0]->AhciBaseAddress;
    u32 slot;

    slot = Iprop_AHCI_FindOpenSlot(
        pd,
        32);  // if no open slot, use a slot which is not the current command
              // slot. The reason for this is to minimize the disturbance to the
              // command header info for the hung command.

    if (slot > 32)
    {
        return IPROP_STATUS_ERR;
    }

    // command header fill
    Iprop_FillCommandListStructure(slot,
                                   0,     // numPrtdEntries
                                   0x0,   // Pmp,
                                   0x10,  // CBRPWA       B=1
                                   0x3,   // CommandFisLength
                                   0x0, (u32 *)pd->CommandListBase,
                                   (u32 *)pd->CommandTableList[slot]);

    // command table fill
    fis = pd->CommandTableList[slot];

    fis[0] = (0x0 << 24) |  // reserved
             (0x8 << 16) |  // pattern Definition  F-far end analog
             (0X0 << 8) |   /* PM*/
             (0x58);        /* BISC Activate FIS */
    fis[1] = 0;
    fis[2] = 0;

    // Release the command
    Iprop_RegWrite32((u32 *)pd->AhciBaseAddress,
                     BASE_ADDRESS_AHCI_PORT(pd->PortNum) + AHCI_PxCI,
                     (1 << slot));

    while ((Iprop_AHCI_CheckCommandSlotBusy((u32 *)pd->AhciBaseAddress,
                                            (1 << slot), pd->PortNum)) != 0)
    {
        // check for errors
        if (Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum) &
            (PORT_IRQ_ERROR))
        {
            iprop_printf(
                "Error Occurred while waiting for command slot to finish\n\r");
            iprop_printf(
                "\t AHCI P[%x]-IS Register %x \n\r", pd->PortNum,
                Iprop_AHCI_GetISReg((u32 *)pd->AhciBaseAddress, pd->PortNum));
        }
    }
    // wait for data to complete
    return 0;
}

/*---------------------
------portNum = 0/1
------speedGen = 1,2,3
-----------------------*/
void Speed_negotiated_test (int PortNum, u32 SpeedGen)
{
    u32 rc, reg;
    /*- stop engine -*/
    /*
    rc = Iprop_AHCI_StopEngine(pd);
    if (rc) {
       iprop_printf("Failed to StopEngine\n");
       return;
    }
    */
    if (SpeedGen == 0x1)
    {
        /*- set Limit gen1 speed ,and Reset port-*/
        reg = Iprop_RegRead32((u32 *)0xFD0C0000,
                              BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL);
        reg = (reg & 0xFFFFFF00);

        Iprop_RegWrite32((u32 *)0xFD0C0000,
                         BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL,
                         (reg | 0x11));

        /*-- Hold reset for ~1us --*/
        usleep(1);

        /*-- Clear Port Reset --*/
        Iprop_RegWrite32((u32 *)0xFD0C0000,
                         BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL, 0x10);

        /*-- Wait up to 20ms to receive a cominit from the drive... way longer
         * than needed --*/
        rc = iprop_wait_reg(
            ((u32 *)(0xFD0C0000 + BASE_ADDRESS_AHCI_PORT((PortNum)) +
                     AHCI_PxSSTS)),
            0x1,     // only look at bit 0.
            0x0,     // if bit-0 == 1, no cominit has been received
            1,       // wait 1us between register reads
            20000);  // 20ms

        if (rc == 0x113)
        {
            iprop_printf("Gen1 Phy negotiated test pass!\n\r");
        }
        else
        {
            iprop_printf("Gen1 Phy negotiated test failed!\n\r");
        }
        return;
    }

    if (SpeedGen == 0x2)
    {
        /************- set Limit gen2 speed ,and Reset port-************/
        reg = Iprop_RegRead32((u32 *)0xFD0C0000,
                              BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL);
        reg = (reg & 0xFFFFFF00);

        Iprop_RegWrite32((u32 *)0xFD0C0000,
                         BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL,
                         (reg | 0x21));

        /*-- Hold reset for ~1us --*/
        usleep(1);

        /*-- Clear Port Reset --*/
        Iprop_RegWrite32((u32 *)0xFD0C0000,
                         BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL, 0x20);

        /*-- Wait up to 20ms to receive a cominit from the drive... way longer
         * than needed --*/
        rc = iprop_wait_reg(
            ((u32 *)(0xFD0C0000 + BASE_ADDRESS_AHCI_PORT((PortNum)) +
                     AHCI_PxSSTS)),
            0x1,     // only look at bit 0.
            0x0,     // if bit-0 == 1, no cominit has been received
            1,       // wait 1us between register reads
            20000);  // 20ms

        if (rc == 0x123)
        {
            iprop_printf("Gen2 Phy negotiated test pass!\n\r");
        }
        else
        {
            iprop_printf("Gen2 Phy negotiated test failed!\n\r");
        }

        return;
    }

    if (SpeedGen == 0x3)
    {
        /************- set Limit gen3 speed ,and Reset port-************/
        reg = Iprop_RegRead32((u32 *)0xFD0C0000,
                              BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL);
        reg = (reg & 0xFFFFFF00);

        Iprop_RegWrite32((u32 *)0xFD0C0000,
                         BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL,
                         (reg | 0x31));

        /*-- Hold reset for ~1us --*/
        usleep(1);

        /*-- Clear Port Reset --*/
        Iprop_RegWrite32((u32 *)0xFD0C0000,
                         BASE_ADDRESS_AHCI_PORT((PortNum)) + AHCI_PxSCTL, 0x30);

        /*-- Wait up to 20ms to receive a cominit from the drive... way longer
         * than needed --*/
        rc = iprop_wait_reg(
            ((u32 *)(0xFD0C0000 + BASE_ADDRESS_AHCI_PORT((PortNum)) +
                     AHCI_PxSSTS)),
            0x1,     // only look at bit 0.
            0x0,     // if bit-0 == 1, no cominit has been received
            1,       // wait 1us between register reads
            20000);  // 20ms

        if (rc == 0x133)
        {
            iprop_printf("Gen3 Phy negotiated test pass!\n\r");
        }
        else
        {
            iprop_printf("Gen3 Phy negotiated test failed!\n\r");
        }
        return;
    }
}
