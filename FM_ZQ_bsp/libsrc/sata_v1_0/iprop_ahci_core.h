//////////////////////////////////////////////////////////////////////////////
//
// FILENAME: iprop_ahci_core.h
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
// -------- ----------- ------------------------------------------------------
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
//////////////////////////////////////////////////////////////////////////////

#ifndef _IPROP_AHCI_COMMANDS_
#define _IPROP_AHCI_COMMANDS_

#include "iprop_ahci_registers.h"
#include "iprop_ata.h"
#include "iprop_lib.h"
#include "iprop_types.h"

// #include "iprop_ahci_cmds.h"

enum {
    RFIS_DMAS_OFFSET = 0,
    RFIS_PIOS_OFFSET = 0x20,
    RFIS_D2H_OFFSET = 0x40,
    RFIS_SDB_OFFSET = 0x58,
    RFIS_UFIS_OFFSET = 0x60,
};

enum {
    IPROP_AHCI_MAX_PORTS = 2,
    IPROP_AHCI_PORT_MAX_PHY_ATTEMPTS = 1000,
    IPROP_AHCI_NUM_SPEED_WINDOWS = 2,
    IPROP_AHCI_INIT_STATUS_USEC_WAIT = 1000000, /*1 sec*/
    /* Active high masking of active ports*/
    IPROP_AHCI_PORTS_IMP = ((1 << IPROP_AHCI_MAX_PORTS) - 1),
    IPROP_AHCI_MAX_SG = 16,
    IPROP_AHCI_Q_DEPTH = 32,
    IPROP_AHCI_STRIPE_SIZE = 8, /*sectors*/
    IPROP_AHCI_ALMP = 0,
    IPROP_AHCI_ALMS = 0,
};

typedef struct {
    volatile unsigned int CFIS[16];
    volatile unsigned int ACMD[4];
    volatile unsigned int reserved[12];
} Iprop_AHCI_CommandTable;

typedef struct {
    unsigned int DBA;
    unsigned int DBAU;
    unsigned int Reserved;
    unsigned int DBC;
} Iprop_AHCI_PRD;

typedef struct {
    unsigned int PortNum;
    unsigned int AhciBaseAddress;
    void *CommandListBase;
    void *CommandTableList[AHCI_MAX_CMDS];
    void *ReceiveFisTable;
    unsigned int cap;
    unsigned int cap2;
} Iprop_AHCI_PortDesc;

typedef struct {
    void *addr;
    unsigned char n_ports;
    unsigned char ports_imp;
    unsigned int cap;
    unsigned int cap2;
} Iprop_AHCI_HostDesc;

int Iprop_AHCI_SensePort(unsigned int AhciLiteBaseAddr,
                         unsigned int PortsImplemented);

unsigned int Iprop_AHCI_HBA_GetIrqStat(Iprop_AHCI_HostDesc *ah);

unsigned int Iprop_AHCI_Port_GetIrqStat(Iprop_AHCI_PortDesc *ap);

void Iprop_AHCI_Error_Intr(Iprop_AHCI_PortDesc *ap, unsigned int irq_stat);

void Iprop_AHCI_Port_Intr(Iprop_AHCI_PortDesc *ap, unsigned int irq_stat);

void Iprop_AHCI_EnableAgressivePartial(Iprop_AHCI_PortDesc *ap);

void Iprop_AHCI_EnableAgressiveSlumber(Iprop_AHCI_PortDesc *ap);

void Iprop_AHCI_DEVSLP(Iprop_AHCI_HostDesc *ah, Iprop_AHCI_PortDesc *ap);

void Iprop_AHCI_Active(Iprop_AHCI_PortDesc *ap);

void Iprop_AHCI_StartEngine(Iprop_AHCI_PortDesc *ap);

int Iprop_AHCI_StopEngine(Iprop_AHCI_PortDesc *ap);

int Iprop_AHCI_KickEngine(Iprop_AHCI_PortDesc *ap);

int Iprop_AHCI_DeInitPort(Iprop_AHCI_PortDesc *ap);

int Iprop_AHCI_StopFisRx(Iprop_AHCI_PortDesc *ap);

void Iprop_AHCI_StartFisRx(Iprop_AHCI_PortDesc *ap);

int Iprop_AHCI_HBA_Reset(Iprop_AHCI_HostDesc *ah);

u32 iprop_do_portreset(Iprop_AHCI_PortDesc *pd);

int Iprop_AHCI_WaitRegister(unsigned int RegAddr, unsigned int mask,
                            unsigned int val, unsigned int interval_msec,
                            unsigned int timeout_msec);
/*---------------------------------------------------------------------------------------
* Wait for timeout_msec for the mask & reg_value to be equal to zero, checking
reg_value
*  every interval_msec
----------------------------------------------------------------------------------------*/
int Iprop_AHCI_WaitClear(u32 *RegAddr, u32 mask, u32 interval_msec,
                         u32 timeout_msec);

void Iprop_FillCommandListStructure(u32 commandSlot, u32 PrdtLength, u8 Pmp,
                                    u8 CBRPWA, u8 CommandFisLength,
                                    u32 XferByteCount,
                                    u32 *CommandListBaseAddress,
                                    u32 *CommandTableBaseAddress);

int Iprop_AHCI_SendSoftReset(Iprop_AHCI_PortDesc *ap);

void Iprop_AHCI_ClearPortInterrupts(Iprop_AHCI_PortDesc *ap);

u32 iprop_bringup_port(Iprop_AHCI_PortDesc *pd);

u32 iprop_ahci_cleanse_ports(Iprop_AHCI_PortDesc *pd[], Iprop_AHCI_HostDesc *hd,
                             u32 loops);

unsigned int *Iprop_AHCI_FPDMAWriteToSATA(
    Iprop_AHCI_PortDesc *PortDesc, unsigned int LBA_High, unsigned int LBA_Low,
    unsigned int SectorCount, unsigned int commandSlot,
    unsigned int numPrtdEntries, Iprop_AHCI_PRD *PhysicalRegionDescriptorList);

unsigned int *Iprop_AHCI_FPDMAReadFromSATA(
    Iprop_AHCI_PortDesc *PortDesc, unsigned int LBA_High, unsigned int LBA_Low,
    unsigned int SectorCount, unsigned int commandSlot,
    unsigned int numPrtdEntries, Iprop_AHCI_PRD *PhysicalRegionDescriptorList);

unsigned int *Iprop_AHCI_WriteToSATA(
    Iprop_AHCI_PortDesc *PortDesc, unsigned int LBA_High, unsigned int LBA_Low,
    unsigned int SectorCount, unsigned int commandSlot,
    unsigned int numPrtdEntries, Iprop_AHCI_PRD *PhysicalRegionDescriptorList);

unsigned int *Iprop_AHCI_TRIM(Iprop_AHCI_PortDesc *PortDesc,
                              unsigned int SectorCount,
                              unsigned int commandSlot,
                              unsigned int numPrtdEntries,
                              Iprop_AHCI_PRD *PhysicalRegionDescriptorList);

u32 *Iprop_AHCI_ReadFromSATA(Iprop_AHCI_PortDesc *pd[],
                             unsigned int AhciLiteBaseAddr,
                             unsigned int LBA_High, unsigned int LBA_Low,
                             unsigned int SectorCount, unsigned int commandSlot,
                             unsigned int numPrtdEntries,
                             Iprop_AHCI_PRD *PhysicalRegionDescriptorList,
                             volatile int *CommandListBaseAddress,
                             unsigned int Port);

unsigned int *Iprop_AHCI_WriteToSATAForcedUnitAccess(
    unsigned int AhciLiteBaseAddr, unsigned int LBA_High, unsigned int LBA_Low,
    unsigned int SectorCount, unsigned int commandSlot,
    unsigned int numPrtdEntries, Iprop_AHCI_PRD *PhysicalRegionDescriptorList,
    volatile int *CommandListBaseAddress, unsigned int Port);

unsigned int *Iprop_AHCI_SendIdentify(
    Iprop_AHCI_PortDesc *PortDesc, unsigned int commandSlot,
    unsigned int numPrtdEntries, Iprop_AHCI_PRD *PhysicalRegionDescriptorList);

/*
 *unsigned int * Iprop_AHCI_SendIdentify(
 *                unsigned int AhciLiteBaseAddr,
 *                unsigned int commandSlot,
 *                unsigned int numPrtdEntries,
 *                Iprop_AHCI_PRD * PhysicalRegionDescriptorList,
 *                volatile int * CommandListBaseAddress,
 *                unsigned int Port
 *               );
 */

unsigned int *Iprop_AHCI_SendExecuteDevDiag(
    unsigned int AhciLiteBaseAddr, unsigned int commandSlot,
    volatile int *CommandListBaseAddress, unsigned int Port);

unsigned int *Iprop_AHCI_SmartEnableOp(unsigned int AhciLiteBaseAddr,
                                       unsigned int commandSlot,
                                       volatile int *CommandListBaseAddress,
                                       unsigned int Port);

unsigned int *Iprop_AHCI_SmartExecuteOfflineImmediate(
    unsigned int AhciLiteBaseAddr, unsigned int commandSlot,
    unsigned char SmartMode, volatile int *CommandListBaseAddress,
    unsigned int Port);

unsigned int Iprop_AHCI_CheckSActive(Iprop_AHCI_PortDesc *ap);

void *Iprop_CreateAlignedCommandTable(int numEntries);

void *Iprop_CreateAlignedCommandList(int numCommandSlots);

void *Iprop_CreateAlignedReceiveFisTable();

void Iprop_FreeAlignedCommandTable(void *ptr);

// Returns decimal value of first found open slot.
// This does _not_ look at the max slot depth of the controller,
//  nor does it look at the max slot depth of the drive. The max_slots
// input is the only control of our queue depth. Use Cautiously!
// If no slots available, returns -1
int Iprop_AHCI_FindOpenSlot(Iprop_AHCI_PortDesc *ap, unsigned char max_slots);

// Buffer is a pointer to a allocated 256Byte memory space accessable by the
// processor.
void Iprop_AHCI_DumpPortReg(Iprop_AHCI_PortDesc *ap, unsigned int *buffer);

unsigned char Iprop_AHCI_GetCurrentSlot(Iprop_AHCI_PortDesc *ap);

u32 *Iprop_AHCI_SendManualTFCmd(Iprop_AHCI_PortDesc *PortDesc,
                                struct ata_taskfile *tf, u32 *CommandTable,
                                u32 commandSlot, u32 numPrtdEntries,
                                Iprop_AHCI_PRD *PRD);

u32 *Iprop_AHCI_SendDMTFCmd(Iprop_AHCI_PortDesc *pd, struct ata_taskfile *tf,
                            u32 tag, u32 numPrtdEntries, Iprop_AHCI_PRD *PRD);

unsigned int ata_dev_set_feature(Iprop_AHCI_PortDesc *pd, u8 enable, u8 feature,
                                 u32 ms_timeout);

u32 iprop_ahci_wait_status(u32 *ahci_addr, u8 port, u8 c_slot, u32 ms_timeout);

u32 iprop_ahci_do_softreset(Iprop_AHCI_PortDesc *pd);

u32 iprop_ahci_check_pending_qd_cmd(Iprop_AHCI_PortDesc *pd, u32 ms_timeout);

u32 iprop_ahci_check_rfis_err(Iprop_AHCI_PortDesc *pd, u8 rfis_type);

u8 iprop_ahci_wait_internal_command(Iprop_AHCI_PortDesc *pd, u32 slot,
                                    u32 ms_timeout);

int Iprop_AHCI_InitController(u32 *AhciBaseAddr, Iprop_AHCI_PortDesc *pd[],
                              Iprop_AHCI_HostDesc *hd);

void iprop_init_ahci(u32 *AhciBaseAddr, Iprop_AHCI_PortDesc *pd[],
                     Iprop_AHCI_HostDesc *hd, u32 *port_dedicated_mem[],
                     u16 *id_data[]);

u32 iprop_ahci_bringup_port(Iprop_AHCI_PortDesc *pd);

u32 iprop_ahci_do_portreset(Iprop_AHCI_PortDesc *pd);

#define Iprop_CreateCommandTable(numEntries) \
    (unsigned int *)Iprop_CreateAlignedCommandTable((numEntries))

#define Iprop_FreeCommandTable(Pointer) Iprop_FreeAlignedCommandTable((Pointer))

#define Iprop_CreateReceivedFisArea() \
    (unsigned int *)Iprop_CreateAlignedReceiveFisTable()

/*-- Returns 0 if not busy --*/
#define Iprop_AHCI_CheckCommandSlotBusy(BaseAddress, CommandSlot, Port) \
    (Iprop_RegRead32((BaseAddress),                                     \
                     BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxCI) &      \
     (CommandSlot))

/*-- Returns 0 if not busy --*/
#define Iprop_AHCI_CheckCommandSACTBusy(BaseAddress, TagMask, Port)  \
    (Iprop_RegRead32((BaseAddress),                                  \
                     BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxSACT) & \
     (TagMask))

/*-- Hard HBA reset, this will bring down all the ports and ahci-sm --*/
#define Iprop_AHCI_HbaReset(BaseAddress) \
    Iprop_RegWrite32((BaseAddress), BASE_ADDRESS_AHCI_HC + AHCI_GHC, HOST_RESET)

#define Iprop_AHCI_PortReset(BaseAddress, Port) \
    Iprop_RegWrite32((BaseAddress),             \
                     BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxSCTL, 0x1)

#define Iprop_AHCI_GetNumPorts(BaseAddress)                             \
    ((Iprop_RegRead32((BaseAddress), BASE_ADDRESS_AHCI_HC + AHCI_CAP) & \
      0x1F) +                                                           \
     1)

#define Iprop_AHCI_GetTFDReg(BaseAddress, Port) \
    (Iprop_RegRead32((BaseAddress),             \
                     BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxTFD))

#define Iprop_AHCI_GetISReg(BaseAddress, Port) \
    (Iprop_RegRead32((BaseAddress), BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxIS))

#define Iprop_AHCI_GetCurCmdSlot(BaseAddress, Port)                   \
    ((Iprop_RegRead32((BaseAddress),                                  \
                      BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxCMD) >> \
      8) &                                                            \
     0x1F)

#define Iprop_AHCI_CheckPortPhyReady(BaseAddress, Port)               \
    ((Iprop_RegRead32((BaseAddress),                                  \
                      BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxSSTS) & \
      0x3) == 0x3)

#define Iprop_AHCI_CheckStartBit(BaseAddress, Port)                 \
    (Iprop_RegRead32((BaseAddress),                                 \
                     BASE_ADDRESS_AHCI_PORT((Port)) + AHCI_PxCMD) & \
     0x1)

#endif  // _IPROP_AHCI_COMMANDS_
