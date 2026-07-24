///////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_ahci_core.c
// PROJECT :
// KEYWORDS:
// LANGUAGE: C
// INTELLIPROP AUTHOR  : ehanke
// CREATED : 10/14/2010
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
#include "iprop_ahci_cmds.h"
#include "iprop_ahci_core.h"
#include "iprop_ahci_registers.h"
#include "iprop_ata.h"
#include "iprop_reg_hw.h"
#include "iprop_types.h"

// #include "stdlib.h"
// #include "sys/alt_stdio.h"

/**
 *  ata_dev_set_feature - Issue SET FEATURES - SATA FEATURES
 *  @pd: Port Description of device to which command will be sent
 *  @enable: Whether to enable or disable the feature
 *  @feature: The sector count represents the feature to set
 *
 *  Issue SET FEATURES - SATA FEATURES command to device @dev
 *  on port @ap with sector count
 *
 *  LOCKING:
 *  PCI/etc. bus probe sem.
 *
 *  RETURNS:
 *  0 on success, AC_ERR_* mask otherwise.
 */
unsigned int ata_dev_set_feature (Iprop_AHCI_PortDesc *pd, u8 enable,
                                  u8 feature, u32 ms_timeout)
{
    u32 rc;
    struct ata_taskfile tf;
    iprop_clear_tf(&tf);

    /* set up set-features taskfile */
    // iprop_printf("set features - SATA features\n");

    tf.command = ATA_CMD_SET_FEATURES;
    tf.feature = enable;
    tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
    tf.protocol = ATA_PROT_NODATA;
    tf.nsect = feature;

    /*-- Issue ND Comamnd --*/
    Iprop_AHCI_SendDMTFCmd(pd, &tf, 0, 0, (Iprop_AHCI_PRD *)0);

    rc = iprop_ahci_wait_status((u32 *)pd->AhciBaseAddress, pd->PortNum, 0,
                                ms_timeout);
    if (rc & IPROP_STATUS_ERR)
    {
        return IPROP_STATUS_ERR;
    }
    if (rc & IPROP_STATUS_TIMEOUT)
    {
        return IPROP_STATUS_TIMEOUT;
    }
    return IPROP_STATUS_SUCCESS;
}

/*-- Bring ports online. Reset the ports if there are outstanding commands
pending, or if D2H FIS isn't received on time INPUTS: loops --> number of
attempts before giving up. If loop == 0, try forever. RETURNS: the number of
ports which are in a ready state after loop count is exceeded, or all ports are
ready.
--*/
u32 iprop_ahci_cleanse_ports (Iprop_AHCI_PortDesc *pd[],
                              Iprop_AHCI_HostDesc *hd, u32 loops)
{
    u32 port_ready;
    u32 loop_count = 1;
    u8 ii;
    // Bring all the Host Ports online
    while (loop_count != loops)
    {
        port_ready = 0;
        for (ii = 0; ii < hd->n_ports; ii++)
        {
            if (iprop_bringup_port(pd[ii]) == IPROP_STATUS_SUCCESS)
            {
                port_ready |= (1 << ii);
            }
        }

        if (port_ready == ((1 << hd->n_ports) - 1))
        {
            return IPROP_STATUS_SUCCESS;
        }
        loop_count++;
        if (loop_count == 0)
        {  // Handle case where we roll from 2^32-1 to 0. (loops value of 0
           // implies 'try forever')
            loop_count = 1;
        }
    }
    return IPROP_STATUS_TIMEOUT;
}

u32 iprop_do_portreset (Iprop_AHCI_PortDesc *pd)
{
    u32 rc;
    u32 reg;
    /*- stop engine -*/
    rc = Iprop_AHCI_StopEngine(pd);
    if (rc)
    {
        // iprop_printf("Failed to StopEngine\n");
    }

    /*-- Reset Port --*/
    reg = Iprop_RegRead32((u32 *)pd->AhciBaseAddress,
                          BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSCTL);
    reg = (reg & 0xFFFFFFF0);
    Iprop_RegWrite32((u32 *)pd->AhciBaseAddress,
                     BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSCTL,
                     (reg | 1));

    /*-- Hold reset for ~1us --*/
    usleep(1);

    /*-- Clear Port Reset --*/
    Iprop_RegWrite32((u32 *)pd->AhciBaseAddress,
                     BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSCTL, reg);

    /*-- Wait up to 20ms to receive a cominit from the drive... way longer than
     * needed --*/
    rc = iprop_wait_reg(
        ((u32 *)(pd->AhciBaseAddress + BASE_ADDRESS_AHCI_PORT((pd->PortNum)) +
                 AHCI_PxSSTS)),
        0x1,     // only look at bit 0.
        0x0,     // if bit-0 == 1, no cominit has been received
        1,       // wait 1us between register reads
        20000);  // 20ms

    /*-- Clear SERR --*/
    Iprop_RegWrite32((u32 *)pd->AhciBaseAddress,
                     BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSERR,
                     0xffffffff);

    /*- start engine -*/
    Iprop_AHCI_StartEngine(pd);

    if ((rc & 1) == 1)
    {
        return IPROP_STATUS_SUCCESS;
    }

    return IPROP_STATUS_TIMEOUT;
}

/*-- Returns IPROP_STATUS_SUCCESS if port is phy-ready and status has been
 * received --*/
u32 iprop_bringup_port (Iprop_AHCI_PortDesc *pd)
{
    u32 port_addr = (pd->AhciBaseAddress + BASE_ADDRESS_AHCI_PORT(pd->PortNum));
    u32 rc;
    u32 reset_attempts = 0;

    /*-- A Reset is required if there are any outstanding commands on the port
     * --*/
    u8 initial_rst = ((Iprop_RegRead32((u32 *)port_addr, AHCI_PxSACT)) != 0);
    initial_rst |= ((Iprop_RegRead32((u32 *)port_addr, AHCI_PxCI)) != 0);
// Check if port is aleady phy-ready.
check_phy_ready:
    rc = iprop_wait_reg((u32 *)(port_addr + AHCI_PxSSTS),
                        0x2,      // only look at bit 1.
                        0x0,      // if bit-1 == 1, We're phy-ready
                        1,        // wait 1us between register reads
                        100000);  // ~100ms
                                  // 50000); // 50ms

    if (((rc & 0x2) == 0) || initial_rst)
    {
reset_port:
        /*- We didn't get to phy-ready on a port with a drive expected in the
           allocated time. Reset port! --*/
        initial_rst = 0;
        while (iprop_do_portreset(pd))
        {  // returns 0 if a cominit is detected.
            reset_attempts++;
            if (reset_attempts > IPROP_AHCI_PORT_MAX_PHY_ATTEMPTS)
            {
                return IPROP_STATUS_TIMEOUT;
            }
        }
        goto check_phy_ready;
    }

    /*-- OK, if we've made it here, we're phy-ready.
     *    Now, we need to get initial status from the drive --*/
    rc = iprop_wait_reg(((u32 *)(port_addr + AHCI_PxTFD)), ATA_BUSY, ATA_BUSY,
                        1, IPROP_AHCI_INIT_STATUS_USEC_WAIT);

    if (rc &
        ATA_BUSY)  // If no status received yet, reset the port and try again.
    {
        goto reset_port;
    }
    else
    {
        return IPROP_STATUS_SUCCESS;
    }
}

void Iprop_FillCommandListStructure (u32 commandSlot, u32 PrdtLength, u8 Pmp,
                                     u8 CBRPWA, u8 CommandFisLength,
                                     u32 XferByteCount,
                                     u32 *CommandListBaseAddress,
                                     u32 *CommandTableBaseAddress)
{
    volatile u32 *cle = (volatile u32 *)CommandListBaseAddress;
    cle += (8 * commandSlot);
    *(cle + 0) = (((PrdtLength & 0xFF) << 16) | ((Pmp & 0x0F) << 12) |
                  ((CBRPWA & 0x3F) << 5) | (CommandFisLength & 0x1F));
    *(cle + 1) = 0x0;     // I think this field is updated by the HBA??
    *(cle + 2) = (u32)CommandTableBaseAddress;
    *(cle + 3) = 0x0000;  // 32bit only --- zeros/

    return;
}

int Iprop_AHCI_SensePort (unsigned int AhciLiteBaseAddr,
                          unsigned int PortsImplemented)
{
    unsigned int ii;
    unsigned int reg_val;
    unsigned int PortPhyReady = 0;
    for (ii = 0; ii < AHCI_MAX_PORTS; ii++)
    {
        if (PortsImplemented & (1 << ii))
        {
            /*-- Check if PhyReady --*/
            reg_val = Iprop_RegRead32((u32 *)AhciLiteBaseAddr,
                                      BASE_ADDRESS_AHCI_PORT(ii) + AHCI_PxSSTS);
            /*-- if Not PhyReady, check if there is a drive attached. --*/
            if ((reg_val & 0x3) != 0x3)
            {
                /*-- if No Drive Attached, report and skip. --*/
                if ((reg_val & 0x1) != 0x1)
                {
                    PortPhyReady &= ~(1 << ii);
                }
                else
                {
                    /*-- drive sensed, wait for Speed Negotiation to
                     * complete--*/
                    usleep(100);
                    if (Iprop_AHCI_CheckPortPhyReady((u32 *)AhciLiteBaseAddr,
                                                     ii))
                    {
                        PortPhyReady |= (1 << ii);
                    }
                }
            }
            else
            {
                PortPhyReady |= (1 << ii);
            }
        }
    }
    return PortPhyReady;
}

int Iprop_AHCI_WaitClear (u32 *RegAddr, u32 mask, u32 interval_msec,
                          u32 timeout_msec)
{
    u32 ii = 0;
    u32 rc = 0;

    // Check for instant return
    if (timeout_msec == 0)
    {
        return Iprop_RegRead32(RegAddr, 0);
    }

    for (ii = 0; ii < (timeout_msec / interval_msec); ii++)
    {
        rc = Iprop_RegRead32(RegAddr, 0);
        if ((rc & mask) == 0)
        {
            return 0;
        }
        // branch slot pad...
        Iprop_RegRead32(RegAddr, 0);
        usleep(interval_msec * 1000);
    }
    // iprop_printf("%s:: ata wait reg timout. RegVal == %08X\n\r",__func__,rc);
    return rc;
}

int Iprop_AHCI_WaitRegister (unsigned int RegAddr, unsigned int mask,
                             unsigned int val, unsigned int interval_msec,
                             unsigned int timeout_msec)
{
    unsigned int ii = 0;
    unsigned int rc = 0;
    for (ii = 0; ii < (timeout_msec / interval_msec); ii++)
    {
        rc = Iprop_RegRead32((u32 *)RegAddr, 0);
        if ((rc & mask) != val)
        {
            return rc;
        }
        // branch slot pad...
        Iprop_RegRead32((u32 *)RegAddr, 0);
        usleep(interval_msec * 1000);
    }
    // iprop_printf("%s:: ata wait reg timout. RegVal == %08X\n\r",__func__,rc);
    return rc;
}

int Iprop_AHCI_KickEngine (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;
    int rc;

    /*- stop engine -*/
    rc = Iprop_AHCI_StopEngine(ap);
    if (rc)
    {
        // iprop_printf("Failed to StopEngine\n");
        Iprop_RegWrite32((u32 *)port_mmio, 0x80, 0xDEAD1234);  // Debug Write
        Iprop_AHCI_StartEngine(ap);
        return rc;
    }

    if (!(ap->cap & HOST_CAP_CLO))
    {
        // rc = -EOPNOTSUPP;
        // iprop_printf("HOST Command List Override not supported!\n");
        rc = -1;
        Iprop_AHCI_StartEngine(ap);
        return rc;
    }

    /*-- perform a Command List Overide --*/
    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
    tmp |= PORT_CMD_CLO;
    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);

    rc = 0;
    tmp = Iprop_AHCI_WaitRegister(
        (port_mmio + BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD),
        PORT_CMD_CLO, PORT_CMD_CLO, 1, 500);
    if (tmp & PORT_CMD_CLO)
    {
        rc = -2;
    }

    /* restart engine */
    Iprop_AHCI_StartEngine(ap);
    return rc;
}

int Iprop_AHCI_DeInitPort (Iprop_AHCI_PortDesc *ap)
{
    int rc;
    /* disable DMA */
    rc = Iprop_AHCI_StopEngine(ap);
    if (rc)
    {
        // iprop_printf("failed to stop engine\n");
        return rc;
    }
    /* disable FIS reception */
    rc = Iprop_AHCI_StopFisRx(ap);
    if (rc)
    {
        // iprop_printf("failed stop FIS RX\n");
        return rc;
    }
    return 0;
}

int Iprop_AHCI_StopFisRx (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;

    /* disable FIS reception */
    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
    tmp &= ~PORT_CMD_FIS_RX;
    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);

    /* wait for completion, spec says 500ms, give it 1000 */
    tmp = Iprop_AHCI_WaitRegister(port_mmio + PORT_CMD, PORT_CMD_FIS_ON,
                                  PORT_CMD_FIS_ON, 10, 1000);
    if (tmp & PORT_CMD_FIS_ON)
    {
        return -1;
    }
    // while(Iprop_RegRead32(port_mmio,
    // BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD) & PORT_CMD_FIS_ON){
    // dummy wait until clear...
    //}
    return 0;
}

void Iprop_AHCI_StartFisRx (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;

    /* set FIS registers */
    // iprop_printf("Start FIS Rx\n");

    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxFB,
                     (unsigned int)ap->ReceiveFisTable);

    // iprop_printf("Read from PxFB reg\n");

    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCLB,
                     (unsigned int)ap->CommandListBase);

    // iprop_printf("Read from CLB reg\n");

    /* enable FIS reception */
    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
    tmp |= PORT_CMD_FIS_RX;
    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);
    // iprop_printf("enable FIS reception\n");

    /* flush */
    Iprop_RegRead32((u32 *)port_mmio,
                    BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
}

void Iprop_AHCI_EnableAgressivePartial (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;

    /*-- Start DMA --*/
    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
    tmp |= PORT_CMD_ALPE;
    tmp &= ~(PORT_CMD_ASP);

    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);
    Iprop_RegRead32((u32 *)port_mmio, BASE_ADDRESS_AHCI_PORT(ap->PortNum) +
                                          AHCI_PxCMD); /* flush write... */
}

void Iprop_AHCI_EnableAgressiveSlumber (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;

    /*-- Start DMA --*/
    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
    tmp |= (PORT_CMD_ALPE | PORT_CMD_ASP);

    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);
    Iprop_RegRead32((u32 *)port_mmio, BASE_ADDRESS_AHCI_PORT(ap->PortNum) +
                                          AHCI_PxCMD); /* flush write... */
}

void Iprop_AHCI_DEVSLP (Iprop_AHCI_HostDesc *ah, Iprop_AHCI_PortDesc *ap)
{
    unsigned int mmio = ((unsigned int)ah->addr);
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;

    /*Check if HBA Supports DevSleep */
    tmp = Iprop_RegRead32((u32 *)mmio, AHCI_CAP2);

    if ((tmp & HOST_CAP2_SDS))
    {
        tmp = Iprop_RegRead32((u32 *)port_mmio,
                              BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
        tmp |= (PORT_CMD_ICC_DEVSLEEP);
        Iprop_RegWrite32((u32 *)port_mmio,
                         BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);
        // wait 10 us
        usleep(10);
        Iprop_RegRead32((u32 *)port_mmio, BASE_ADDRESS_AHCI_PORT(ap->PortNum) +
                                              AHCI_PxCMD); /* flush write... */
    }
    else
    {
        iprop_printf("HBA does not support Device Sleep\n\r");
    }
}

void Iprop_AHCI_Active (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;
    char port_ready = 0;

    /*Put HBA in Active State */
    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
    tmp &= 0x0FFFFFFF;
    tmp |= (PORT_CMD_ICC_ACTIVE);
    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);
    Iprop_RegRead32((u32 *)port_mmio, BASE_ADDRESS_AHCI_PORT(ap->PortNum) +
                                          AHCI_PxCMD); /* flush write... */

// Bring the Port online
wait_d2h:
    port_ready = 0;
    if (iprop_ahci_bringup_port(ap) == IPROP_STATUS_SUCCESS)
    {
        port_ready = 1;
    }
    if (port_ready)
    {
        return;
    }
    goto wait_d2h;
    return;
}

void Iprop_AHCI_StartEngine (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;

    /*-- Start DMA --*/
    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);
    tmp |= PORT_CMD_START;

    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);
    Iprop_RegRead32((u32 *)port_mmio, BASE_ADDRESS_AHCI_PORT(ap->PortNum) +
                                          AHCI_PxCMD); /* flush write... */
}

unsigned int Iprop_AHCI_HBA_GetIrqStat (Iprop_AHCI_HostDesc *ah)
{
    unsigned int *mmio = (unsigned int *)ah->addr;
    return Iprop_RegRead32((u32 *)mmio, HOST_IRQ_STAT);
}

unsigned int Iprop_AHCI_Port_GetIrqStat (Iprop_AHCI_PortDesc *ap)
{
    return Iprop_RegRead32((u32 *)ap->AhciBaseAddress,
                           BASE_ADDRESS_AHCI_PORT(ap->PortNum) + PORT_IRQ_STAT);
}

void Iprop_AHCI_Error_Intr (Iprop_AHCI_PortDesc *ap, unsigned int irq_stat) {}

void Iprop_AHCI_Port_Intr (Iprop_AHCI_PortDesc *ap, unsigned int irq_stat)
{
    unsigned int status;
    unsigned int cmd_active;
    unsigned int port_addr = ap->AhciBaseAddress +
                             BASE_ADDRESS_AHCI_PORT(ap->PortNum);

    status = Iprop_AHCI_Port_GetIrqStat(ap);

    if (status & PORT_IRQ_ERROR)
    {
        Iprop_AHCI_Error_Intr(ap, status);
        return;
    }
    cmd_active = Iprop_RegRead32((u32 *)port_addr, AHCI_PxSACT);
    cmd_active |= Iprop_RegRead32((u32 *)port_addr, AHCI_PxCI);
}

int Iprop_AHCI_HBA_Reset (Iprop_AHCI_HostDesc *ah)
{
    unsigned int tmp;
    unsigned int mmio = ((unsigned int)ah->addr);

    Iprop_RegWrite32((u32 *)mmio, HOST_CTL, HOST_RESET);

    /*-- Wait for hba to complete it's reset sequence
     *    This could be as long as 500msec, but will probably be _much_ faster
     * --*/
    tmp = Iprop_AHCI_WaitRegister(mmio, HOST_RESET, HOST_RESET,
                                  1,   // interval_msec,
                                  500  // timeout_msec
    );

    return tmp;
}

int Iprop_AHCI_StopEngine (Iprop_AHCI_PortDesc *ap)
{
    unsigned int port_mmio = ap->AhciBaseAddress;
    unsigned int tmp;

    tmp = Iprop_RegRead32((u32 *)port_mmio,
                          BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD);

    if ((tmp & (PORT_CMD_START | PORT_CMD_LIST_ON)) == 0)
    {
        return 0;            // port already stopped
    }

    tmp &= ~PORT_CMD_START;  // Clear Start Bit
    Iprop_RegWrite32((u32 *)port_mmio,
                     BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD, tmp);

    /*-- Wait for ahci engine to stop
     *    This could be as long as 500msec, but will probably be _much_ faster
     * --*/
    tmp = Iprop_AHCI_WaitRegister(
        (port_mmio + BASE_ADDRESS_AHCI_PORT(ap->PortNum) + AHCI_PxCMD),
        PORT_CMD_LIST_ON, PORT_CMD_LIST_ON,
        1,   // interval_msec,
        500  // timeout_msec
    );

    return (tmp & PORT_CMD_LIST_ON);
}

void Iprop_AHCI_ClearPortInterrupts (Iprop_AHCI_PortDesc *ap)
{
    unsigned int *addr = (unsigned int *)ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    /*-- Clear SERR --*/
    Iprop_RegWrite32((u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSERR,
                     0xffffffff);
    /*-- Clear Interrupt Status --*/
    Iprop_RegWrite32((u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxIS,
                     0xffffffff);
    /*-- Clear Port IRQ --*/
    Iprop_RegWrite32((u32 *)addr, BASE_ADDRESS_AHCI_HC + AHCI_IS, (1 << port));

    return;
}

unsigned int Iprop_AHCI_CheckSActive (Iprop_AHCI_PortDesc *ap)
{
    unsigned int *addr = (unsigned int *)ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    return Iprop_RegRead32((u32 *)addr,
                           BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSACT);
}

// Returns decimal value of first found open slot.
// This does _not_ look at the max slot depth of the controller,
//  nor does it look at the max slot depth of the drive. The max_slots
// input is the only control of our queue depth. Use Cautiously!
// If no slots available, returns -1
int Iprop_AHCI_FindOpenSlot (Iprop_AHCI_PortDesc *ap, unsigned char max_slots)
{
    unsigned int *addr = (unsigned int *)ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    unsigned int slot_active = Iprop_RegRead32(
        (u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxSACT);
    slot_active |= Iprop_RegRead32((u32 *)addr,
                                   BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCI);
    unsigned char ii;
    max_slots = (max_slots > 32)
                    ? 32
                    : max_slots;  // quick clense of max_slots input. Should add
                                  // a check of the ap->cap.ncs field.

    if (slot_active == 0xFFFFFFFF)
    {
        return -1;
    }

    for (ii = 0; ii < max_slots; ii++)
    {
        if (((1 << ii) & slot_active) == 0)
        {
            // found open slot!
            return ii;
        }
    }

    // we should never get here, but the compiler likes this.
    return -1;
}

// Buffer is a pointer to a allocated 256Byte memory space accessable by the
// processor.
void Iprop_AHCI_DumpPortReg (Iprop_AHCI_PortDesc *ap, unsigned int *buffer)
{
    unsigned int ii;
    unsigned int addr = ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    volatile unsigned int *rfis_ptr = (unsigned int *)ap->ReceiveFisTable;
    volatile unsigned int *port_ptr = (unsigned int *)(addr +
                                                       BASE_ADDRESS_AHCI_PORT(
                                                           port));

    // Signature      -- 32-bytes
    // RFIS Structure -- 160 Bytes (The RFIS structure is really 256 bytes, but
    // only the top 160-bytes are valid.) Port Registers -- 64 Bytes

    // Signature
    *buffer++ = 0xAABBCCDD;
    *buffer++ = 0xAABBCCDD;
    *buffer++ = 0xAABBCCDD;
    *buffer++ = 0xAABBCCDD;
    *buffer++ = 0xAABBCCDD;
    *buffer++ = 0xAABBCCDD;
    *buffer++ = 0xAABBCCDD;
    *buffer++ = 0xAABBCCDD;

    // RFIS Structure
    for (ii = 0; ii < 40; ii++)
    {  // 40, 32-bit words -> 160 bytes
        *buffer++ = *rfis_ptr++;
    }

    // Port Register Structure
    for (ii = 0; ii < 40; ii++)
    {  // 40, 32-bit words -> 160 bytes
        *buffer++ = *port_ptr++;
    }
}

unsigned char Iprop_AHCI_GetCurrentSlot (Iprop_AHCI_PortDesc *ap)
{
    unsigned int addr = ap->AhciBaseAddress;
    unsigned char port = ap->PortNum;
    unsigned int rc = Iprop_RegRead32(
        (u32 *)addr, BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCMD);
    rc = rc >> 8;
    rc = rc & 0x1F;
    return rc;
}

u32 *Iprop_AHCI_SendDMTFCmd (Iprop_AHCI_PortDesc *pd, struct ata_taskfile *tf,
                             u32 tag, u32 numPrtdEntries, Iprop_AHCI_PRD *PRD)
{
    u32 *ded_mem_cmd_tbl = pd->CommandTableList[tag];

    return Iprop_AHCI_SendManualTFCmd(pd, tf, ded_mem_cmd_tbl, tag,
                                      numPrtdEntries, PRD);
}

u32 *Iprop_AHCI_SendManualTFCmd (Iprop_AHCI_PortDesc *PortDesc,
                                 struct ata_taskfile *tf, u32 *CommandTable,
                                 u32 commandSlot, u32 numPrtdEntries,
                                 Iprop_AHCI_PRD *PRD)
{
    u32 baseAddr = PortDesc->AhciBaseAddress;
    u32 *CLBAddr = (u32 *)PortDesc->CommandListBase;
    u32 Port = PortDesc->PortNum;
    u32 ii;
    int is_cmd = (tf->flags & ATA_TFLAG_ISSRST) ? 0 : 1;
    // int is_cmd = 0;
    u32 CBRPWA = (tf->flags & ATA_TFLAG_WRITE) ? 0x2 : 0x0;
    CBRPWA |= (tf->ctl & ATA_SRST) ? 0x28 : 0x0;

    Iprop_FillCommandListStructure(commandSlot, numPrtdEntries, 0x0, CBRPWA,
                                   0x5, 0x0, CLBAddr, CommandTable);
    if (tf->protocol == ATA_PROT_NCQ)
    {
        iprop_fpdma_tf_to_fis(tf, 0x00, is_cmd, CommandTable, commandSlot);

        for (ii = 0; ii < numPrtdEntries; ii++)
        {
            *(volatile int *)(CommandTable + 0x20 + (ii * 4)) = PRD->DBA;
            *(volatile int *)(CommandTable + 0x21 + (ii * 4)) = PRD->DBAU;
            *(volatile int *)(CommandTable + 0x22 +
                              (ii * 4)) = 0;  // <-- Reserved
            *(volatile int *)(CommandTable + 0x23 + (ii * 4)) = PRD->DBC;
            PRD++;
        }

        // Set up the command SATA Active Reg
        Iprop_RegWrite32((u32 *)baseAddr,
                         BASE_ADDRESS_AHCI_PORT(Port) + AHCI_PxSACT,
                         (1 << commandSlot));

        // Release the command
        Iprop_RegWrite32((u32 *)baseAddr,
                         BASE_ADDRESS_AHCI_PORT(Port) + AHCI_PxCI,
                         (1 << commandSlot));
    }
    else
    {
        iprop_tf_to_fis(tf, 0x00 /*PMP*/, is_cmd, CommandTable);

        for (ii = 0; ii < numPrtdEntries; ii++)
        {
            *(volatile int *)(CommandTable + 0x20 + (ii * 4)) = PRD->DBA;
            *(volatile int *)(CommandTable + 0x21 + (ii * 4)) = PRD->DBAU;
            *(volatile int *)(CommandTable + 0x22 +
                              (ii * 4)) = 0;  // <-- Reserved
            *(volatile int *)(CommandTable + 0x23 + (ii * 4)) = PRD->DBC;
            PRD++;
        }

        // Release the command
        Iprop_RegWrite32((u32 *)baseAddr,
                         BASE_ADDRESS_AHCI_PORT(Port) + AHCI_PxCI,
                         (1 << commandSlot));
    }
    return CommandTable;
}

u32 iprop_ahci_wait_status (u32 *ahci_addr, u8 port, u8 c_slot, u32 ms_timeout)
{
    u32 ii;
    u8 cmd_complete;
    u8 err;

    // check for command complete and if there's an error, if there's an error
    // on any port, quit
    for (ii = 0; ii < (ms_timeout * 1000); ii++)
    {
        cmd_complete = !(
            Iprop_RegRead32((u32 *)ahci_addr,
                            BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxCI) &
            (1 << c_slot));
        err = ((Iprop_RegRead32((u32 *)ahci_addr,
                                BASE_ADDRESS_AHCI_PORT(port) + AHCI_PxIS) &
                PORT_IRQ_TF_ERR) == PORT_IRQ_TF_ERR);
        if (err)
        {
            return (IPROP_STATUS_ERR);
        }
        if (cmd_complete)
        {
            return (u32)c_slot;
        }
        usleep(1);
    }
    return (IPROP_STATUS_TIMEOUT);
}

u32 iprop_ahci_do_softreset (Iprop_AHCI_PortDesc *pd)
{
    u32 rc;
    struct ata_taskfile tf;

    iprop_clear_tf(&tf);
    tf.protocol = ATA_PROT_NODATA;
    tf.ctl = ATA_SRST;
    tf.flags = ATA_TFLAG_ISSRST;

    // give ahci sm a kick in the pants
    rc = Iprop_AHCI_KickEngine(pd);

    // Send SoftReset Set
    Iprop_AHCI_SendDMTFCmd(pd, &tf, 0, 0, 0);  // slot 0

    usleep(500);  // Spec says you need to delay sending the clear for ~0.5ms
    // usleep(5); // Spec says you need to delay sending the clear for ~0.5ms

    // Send SoftReset Clear
    tf.ctl &= ~ATA_SRST;
    Iprop_AHCI_SendDMTFCmd(pd, &tf, 1, 0, 0);  // slot 1

    return iprop_ahci_wait_status((u32 *)pd->AhciBaseAddress, pd->PortNum, 1,
                                  5000);       // wait up to 5 seconds...
}

u32 iprop_ahci_check_pending_qd_cmd (Iprop_AHCI_PortDesc *pd, u32 ms_timeout)
{
    return Iprop_AHCI_WaitClear(
        (u32 *)(pd->AhciBaseAddress + BASE_ADDRESS_AHCI_PORT(pd->PortNum) +
                AHCI_PxSACT),
        0xFFFFFFFF,
        1,          // interval_msec,
        ms_timeout  // timeout_msec
    );
}

u32 iprop_ahci_check_rfis_err (Iprop_AHCI_PortDesc *pd, u8 rfis_type)
{
    // basic checking for error bit for now. If Error bit set, return non-zero,
    // else return zero.
    //  More elaborite error checking possible inthefuture
    u32 *base;
    switch (rfis_type)
    {
    case RFIS_D2H:
        // dword_count = 5;
        base = (u32 *)((u8 *)pd->ReceiveFisTable + 0x40);
        return (Iprop_RegRead32(base, 0) & (1 << 16));
    case RFIS_DMAS:
        // dword_count = 7;
        // base = (u32 *)(pd->ReceiveFisTable + 0x00);
        return IPROP_STATUS_SUCCESS;  //(Iprop_RegRead32(base, 0) & (1 << 16));
    case RFIS_PIOS:
        // dword_count = 5;
        base = (u32 *)((u8 *)pd->ReceiveFisTable + 0x20);
        return ((Iprop_RegRead32(base, 0) & (1 << 16)) |
                (Iprop_RegRead32(base, 3) & 0x24));
    case RFIS_SDB:
        // dword_count = 2;
        base = (u32 *)((u8 *)pd->ReceiveFisTable + 0x58);
        return (Iprop_RegRead32(base, 0) & (1 << 16));
    case RFIS_UNK:
        // dword_count = 16;
        base = (u32 *)((u8 *)pd->ReceiveFisTable + 0x60);
        return IPROP_STATUS_ERR;  // return error for all unknown FIS's for
                                  // now...
    default:
        return IPROP_STATUS_ERR;
    }
}

u8 iprop_ahci_wait_internal_command (Iprop_AHCI_PortDesc *pd, u32 slot,
                                     u32 ms_timeout)
{
    u32 rc;
    rc = Iprop_AHCI_WaitRegister(
        (pd->AhciBaseAddress + BASE_ADDRESS_AHCI_PORT(pd->PortNum) + AHCI_PxCI),
        (1 << slot), (1 << slot), 1, ms_timeout);

    if (rc)
    {
        return -1;
    }
    else
    {
        return IPROP_STATUS_SUCCESS;
    }
}

int Iprop_AHCI_InitController (u32 *AhciBaseAddr, Iprop_AHCI_PortDesc *pd[],
                               Iprop_AHCI_HostDesc *hd)
{
    u32 ii;
    hd->addr = (void *)AhciBaseAddr;
    hd->ports_imp = Iprop_RegRead32((u32 *)AhciBaseAddr,
                                    BASE_ADDRESS_AHCI_HC + AHCI_PI);
    hd->n_ports = (Iprop_RegRead32((u32 *)AhciBaseAddr,
                                   BASE_ADDRESS_AHCI_HC + AHCI_CAP) &
                   0x1F) +
                  1;
    hd->cap = Iprop_RegRead32((u32 *)AhciBaseAddr, AHCI_CAP);
    hd->cap2 = Iprop_RegRead32((u32 *)AhciBaseAddr, AHCI_CAP2);

    for (ii = 0; ii < (hd->n_ports); ii++)
    {
        if (hd->ports_imp & (1 << ii))
        {
            /*-- Clear SERR --*/
            Iprop_RegWrite32((u32 *)hd->addr,
                             BASE_ADDRESS_AHCI_PORT(ii) + AHCI_PxSERR,
                             0xffffffff);
            /*-- Clear Interrupt Status --*/
            Iprop_RegWrite32((u32 *)hd->addr,
                             BASE_ADDRESS_AHCI_PORT(ii) + AHCI_PxIS,
                             0xffffffff);
            /*-- Clear Port IRQ --*/
            Iprop_RegWrite32((u32 *)hd->addr, BASE_ADDRESS_AHCI_HC + AHCI_IS,
                             (1 << ii));
            // set the Interrupt Enable Mask (disallow all for now)
            Iprop_RegWrite32((u32 *)hd->addr,
                             BASE_ADDRESS_AHCI_PORT(ii) + AHCI_PxIE,
                             0x00000000);
        }
    }

    /* -- General Host Control Init-- */
    // Turn On AHCI Mode and enable interrupts
    Iprop_RegWrite32((u32 *)hd->addr, BASE_ADDRESS_AHCI_HC + AHCI_GHC,
                     (HOST_AHCI_EN | HOST_IRQ_EN));

    // return the implemented ports
    return hd->ports_imp;
}

void iprop_init_ahci (u32 *AhciBaseAddr, Iprop_AHCI_PortDesc *pd[],
                      Iprop_AHCI_HostDesc *hd, u32 *port_dedicated_mem[],
                      u16 *id_data[])
{
    u32 ii, jj;
    u32 port_ready = 0;

    Iprop_AHCI_InitController(AhciBaseAddr, pd, hd);

    /*-- Build Port Description Structs for each implemented ports--*/
    for (ii = 0; ii < hd->n_ports; ii++)
    {
        pd[ii]->PortNum = ii;
        pd[ii]->AhciBaseAddress = (unsigned int)hd->addr;
        pd[ii]->CommandListBase = (void *)port_dedicated_mem[ii];

        // build list of command tables spaced out by AHCI_CMD_TBL_SZ
        // Start first command table after end of the command list.
        pd[ii]->CommandTableList[0] = (u32 *)((u8 *)pd[ii]->CommandListBase +
                                              AHCI_CMD_SLOT_SZ);
        for (jj = 1; jj < AHCI_MAX_CMDS; jj++)
        {
            pd[ii]->CommandTableList[jj] = (u32 *)((u8 *)pd[ii]
                                                       ->CommandTableList[jj -
                                                                          1] +
                                                   AHCI_CMD_TBL_SZ);
        }
        // map the receive FIS table at the end of the command table list.
        pd[ii]->ReceiveFisTable = (u32 *)((u8 *)pd[ii]->CommandTableList[(
                                              AHCI_MAX_CMDS - 1)] +
                                          AHCI_CMD_TBL_SZ);
        // iprop_printf("RFIS Table Addressed\n");
        pd[ii]->cap = hd->cap;
        // iprop_printf("port capabilities 1 set to host capabilities 1\n");
        pd[ii]->cap2 = hd->cap2;
        // iprop_printf("port capabilities 2 set to host capabilities 2\n");

        Iprop_AHCI_StartFisRx(pd[ii]);
        Iprop_AHCI_StartEngine(pd[ii]);

#if IPROP_AHCI_ALMS == 1
        Iprop_AHCI_EnableAgressiveSlumber(pd[ii]);
#elif IPROP_AHCI_ALMP == 1
        Iprop_AHCI_EnableAgressivePartial(pd[ii]);
#else
#endif
    }

#ifdef HOST_DMA_ERROR_DUMP
    /*-- This is to debug hanging with a stuck ahci-dma engine due to an invalid
     * address pointer
     */
    u32 ahci_cmd_pending = iprop_syraid_check_pending_qd_cmd(hd, pd, 1, 0);
    u32 slot;
    struct ata_taskfile tf;
    iprop_clear_tf(&tf);
    u32 *CommandTable;
    u32 tmp;
    Iprop_AHCI_PRD prdt;

    for (ii = 0; ii < hd->n_ports; ii++)
    {
        if (1)
        {
            // load the registers at the very top of the id_data[] structure.
            // This takes the first 256 bytes
            Iprop_AHCI_DumpPortReg(pd[ii], (u32 *)id_data[ii]);
            slot = Iprop_AHCI_FindOpenSlot(
                pd[ii], 32);  // if no open slot, use a slot which is not the
                              // current command slot. The reason for this is to
                              // minimize the disturbance to the command header
                              // info for the hung command.
            if (slot > 32)
            {
                slot = Iprop_AHCI_GetCurrentSlot(pd[ii]) + 1;
            }
            // Reset port.
            if (iprop_ahci_bringup_port(pd[ii]) == IPROP_STATUS_SUCCESS)
            {
            }
            // Setup command to send the entire 8kB dedicated memory using the
            // found slot and the upper 144 bytes as
            //  a command table (128bytes) and prd table (16bytes)
            tmp = (u32)id_data[ii];
            tmp += 256;
            CommandTable = tmp;

            tf.command = ATA_CMD_WRITE_EXT;
            tf.flags = (ATA_TFLAG_WRITE | ATA_TFLAG_LBA48);
            tf.protocol = ATA_PROT_DMA;
            tf.device = ATA_LBA;
            tf.nsect = 16;

            prdt.DBA = (u32)pd[ii]->CommandListBase;
            prdt.DBAU = 0;
            prdt.Reserved = 0;
            prdt.DBC = (ATA_SECT_SIZE * 16) - 1;

            Iprop_AHCI_SendManualTFCmd(pd[ii], &tf, CommandTable, slot, 1,
                                       &prdt);

            Iprop_AHCI_WaitClear(
                (pd[ii]->AhciBaseAddress +
                 BASE_ADDRESS_AHCI_PORT(pd[ii]->PortNum) + AHCI_PxCI),
                0xFFFFFFFF,
                1,    // interval_msec,
                1000  // timeout_msec
            );
        }
    }

/*-- End Debug Dump */
#endif

// Bring all the Host Ports online
wait_d2h:
    port_ready = 0;
    // hd->n_ports = 0x1;// test 1 port
    for (ii = 0; ii < hd->n_ports; ii++)
    {
        if (iprop_ahci_bringup_port(pd[ii]) == IPROP_STATUS_SUCCESS)
        {
            port_ready |= (1 << ii);
        }
    }

    if (port_ready == ((1 << hd->n_ports) - 1))
    {
        // if (ipr_in_sim() == 0) {
        // iprop_ahci_set_drive_features(hd, pd, ((1 << hd->n_ports)-1),
        // bridge_id, id_data);
        //}
        return;
    }
    goto wait_d2h;
    return;
}

/*-- Returns IPROP_STATUS_SUCCESS if port is phy-ready and status has been
 * received --*/
u32 iprop_ahci_bringup_port (Iprop_AHCI_PortDesc *pd)
{
    u32 port_addr = (pd->AhciBaseAddress + BASE_ADDRESS_AHCI_PORT(pd->PortNum));
    u32 rc;
    u32 reset_attempts = 0;

    /*-- A Reset is required if there are any outstanding commands on the port
     * --*/
    u8 initial_rst = ((Iprop_RegRead32((u32 *)port_addr, AHCI_PxSACT)) != 0);
    initial_rst |= ((Iprop_RegRead32((u32 *)port_addr, AHCI_PxCI)) != 0);
// Check if port is aleady phy-ready.
check_phy_ready:
    rc = iprop_wait_reg((u32 *)(port_addr + AHCI_PxSSTS),
                        0x2,      // only look at bit 1.
                        0x0,      // if bit-1 == 1, We're phy-ready
                        1,        // wait 1us between register reads
                        100000);  // ~100ms
                                  // 50000); // 50ms

    if (((rc & 0x2) == 0) || initial_rst)
    {
reset_port:
        /*- We didn't get to phy-ready on a port with a drive expected in the
           allocated time. Reset port! --*/
        initial_rst = 0;
        while (iprop_ahci_do_portreset(pd))
        {  // returns 0 if a cominit is detected.
            reset_attempts++;
            if (reset_attempts > IPROP_AHCI_PORT_MAX_PHY_ATTEMPTS)
            {
                return IPROP_STATUS_TIMEOUT;
            }
        }
        goto check_phy_ready;
    }

    /*-- OK, if we've made it here, we're phy-ready.
     *    Now, we need to get initial status from the drive --*/
    rc = iprop_wait_reg(((u32 *)(port_addr + AHCI_PxTFD)), ATA_BUSY, ATA_BUSY,
                        1, IPROP_AHCI_INIT_STATUS_USEC_WAIT);

    if (rc &
        ATA_BUSY)  // If no status received yet, reset the port and try again.
    {
        goto reset_port;
    }
    else
    {
        return IPROP_STATUS_SUCCESS;
    }
}

u32 iprop_ahci_do_portreset (Iprop_AHCI_PortDesc *pd)
{
    u32 rc;
    u32 reg;
    /*- stop engine -*/
    rc = Iprop_AHCI_StopEngine(pd);
    if (rc)
    {
        // iprop_printf("Failed to StopEngine\n");
    }

    /*-- Reset Port --*/
    reg = Iprop_RegRead32((u32 *)pd->AhciBaseAddress,
                          BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSCTL);
    reg = (reg & 0xFFFFFFF0);
    Iprop_RegWrite32((u32 *)pd->AhciBaseAddress,
                     BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSCTL,
                     (reg | 1));

    /*-- Hold reset for ~1us --*/
    usleep(1);

    /*-- Clear Port Reset --*/
    Iprop_RegWrite32((u32 *)pd->AhciBaseAddress,
                     BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSCTL, reg);

    /*-- Wait up to 20ms to receive a cominit from the drive... way longer than
     * needed --*/
    rc = iprop_wait_reg(
        ((u32 *)(pd->AhciBaseAddress + BASE_ADDRESS_AHCI_PORT((pd->PortNum)) +
                 AHCI_PxSSTS)),
        0x1,     // only look at bit 0.
        0x0,     // if bit-0 == 1, no cominit has been received
        1,       // wait 1us between register reads
        20000);  // 20ms

    /*-- Clear SERR --*/
    Iprop_RegWrite32((u32 *)pd->AhciBaseAddress,
                     BASE_ADDRESS_AHCI_PORT((pd->PortNum)) + AHCI_PxSERR,
                     0xffffffff);

    /*- start engine -*/
    Iprop_AHCI_StartEngine(pd);

    if ((rc & 1) == 1)
    {
        return IPROP_STATUS_SUCCESS;
    }

    return IPROP_STATUS_TIMEOUT;
}
