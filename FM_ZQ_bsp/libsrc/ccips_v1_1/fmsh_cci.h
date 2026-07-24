/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
*
* @file  fmsh_cci.h
*
* This file contains
*
* @note		CCI driver for FMZQ MPSoC.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   zyh  03/29/2024  First Release

*</pre>
******************************************************************************/

#ifndef FMSH_CCI_H /* prevent circular inclusions */
#define FMSH_CCI_H /* by using protection macros */
#include "fmsh_cci_hw.h"
#include "fmsh_common.h"

#define arm_cci500

#define FCCIST_SUCCESS      FMSH_SUCCESS
#define FCCIST_FAILURE      FMSH_FAILURE

#define FCCIPS_SLAVES_NUM   6
#define FCCIPS_MASTER_NUM   3
#define FCCIPS_PFMCOUNT_NUM 8

#define CCI_APU_PORT        (4)
#define CCI_PL_HPC_PORT     (0)
#define CCI_PL_ACE_PORT     (5)
#define CCI_LPD_PORT        (2)

enum Cci_Event_Source {
    Cci_SI0 = 0x0,   /* Slave interface 0  */
    Cci_SI1 = 0x1,   /* Slave interface 1  */
    Cci_SI2 = 0x2,   /* Slave interface 2  */
    Cci_SI3 = 0x3,   /* Slave interface 3  */
    Cci_SI4 = 0x4,   /* Slave interface 4  */
    Cci_SI5 = 0x5,   /* Slave interface 5  */
    Cci_SI6 = 0x6,   /* Slave interface 6  */
    Cci_MI0 = 0x9,   /* Master interface 0 */
    Cci_MI1 = 0xA,   /* Master interface 1 */
    Cci_MI2 = 0xB,   /* Master interface 2 */
    Cci_MI3 = 0xC,   /* Master interface 3 */
    Cci_MI4 = 0xD,   /* Master interface 4 */
    Cci_MI5 = 0xE,   /* Master interface 5 */
    Cci_Global = 0xF /* Global Event */
};

enum Cci_PMU_Event {
    /* Cci Slave Interface Event */
    Cci_S_Read_Request = 0x00,
    Cci_S_Read_Request_Device,
    Cci_S_Read_Request_Normal_NonShareable,
    Cci_S_Read_Request_Normal_Shareable_NonAllocating,
    Cci_S_Read_Request_Normal_Shareable_Allocating,
    Cci_S_Read_Request_Invalidation,
    Cci_S_Read_Request_Cache_Maintenance,
    Cci_S_Read_Request_DVM,
    Cci_S_Read_Data,
    Cci_S_Read_Data_Snoop_Hit,
    Cci_S_Write_Request,
    Cci_S_Write_Request_Device,
    Cci_S_Write_Request_NonShareable,
    Cci_S_Write_Request_Shareable_WriteBack_and_WriteClean,
    Cci_S_Write_Request_Shareable_WriteLineUnique,
    Cci_S_Write_Request_Shareable_WriteUnique,
    Cci_S_Write_Request_Evict,
    Cci_S_Write_Request_WriteEvict, /* Not implemented in CCI-500 */
    Cci_S_Write_Data,
    Cci_S_Snoop_Request,
    Cci_S_Snoop_Request_Read,
    Cci_S_Snoop_Request_Clear_or_Incalidate,
    Cci_S_Snoop_Request_Data_Transfer,
    Cci_S_Read_Request_Stall,
    Cci_S_Read_Data_Stall,
    Cci_S_Write_Request_Stall,
    Cci_S_Write_Data_Stall,
    Cci_S_Write_Response_Stall,
    Cci_S_Snoop_Request_Stall,
    Cci_S_Snoop_Data_Stall,
    Cci_S_Request_stall_cycle_for_OT_limit,
    Cci_S_Read_stall_for_arbitration,

    /* CCI Master Interface Event */
    Cci_M_Read_data = 0x0,
    Cci_M_Write_data_handshake,
    Cci_M_Read_request_stall,
    Cci_M_Read_data_stall,
    Cci_M_Write_request_stall,
    Cci_M_Write_data_stall,
    Cci_M_Write_response_stall,

    /* CCI Global Event */
    Cci_G_snoop_filter_bank0or1 = 0x0,
    Cci_G_snoop_filter_bank2or3,
    Cci_G_snoop_filter_bank4or5,
    Cci_G_snoop_filter_bank6or7,
    Cci_G_snoop_filter_bank0or1_miss,
    Cci_G_snoop_filter_bank2or3_miss,
    Cci_G_snoop_filter_bank4or5_miss,
    Cci_G_snoop_filter_bank6or7_miss,
    Cci_G_Back_invalidation,
    Cci_G_all_ways_used,
    Cci_G_Stall_because_TT_full,
    Cci_G_CCI_generated_write_request,
    Cci_G_CD_handshake_in_snoop_network,
    Cci_G_Request_stall_because_address_hazard
};

/*
 *Performance counter type
 */
typedef struct FCciPs_PfmCnt {
    struct FCciPs *cci;
    int counterId;
    enum Cci_Event_Source interface;
    enum Cci_PMU_Event Event;
    u32 eventCount;
} FCciPs_PfmCnt_T;

typedef struct FCciPs {
    u32 BaseAddress; /* Base address of the device */
    FCciPs_PfmCnt_T Counter[FCCIPS_PFMCOUNT_NUM]; /* performance counters */
} FCciPs_T;

typedef struct FCciPs_PfmCnt_Config {
    enum Cci_Event_Source interface;
    enum Cci_PMU_Event Event;
} FCciPs_PfmCnt_Config_T;

// extern FCciPs_T cci;
/************************** Function Prototypes ******************************/

/* Initialization and reset */
u32 FCciPs_Initialize(FCciPs_T *dev);
u32 FCciPs_pfmCntCfgInit(FCciPs_PfmCnt_T *counterPtr,
                         FCciPs_PfmCnt_Config_T *CfgPtr);

void FCciPs_waitPending(FCciPs_T *dev);
u32 FCciPs_getPendingStatus(FCciPs_T *dev);

/* Slave interface control functions */
void FCciPs_enableACE(FCciPs_T *dev, u32 portNum);
void FCciPs_disableACE(FCciPs_T *dev, u32 portNum);
void FCciPs_shareOverride(FCciPs_T *dev, u32 portNum, u32 val);

/* Performance counters control functions */
void FCciPs_resetCounterAll(FCciPs_T *dev);
void FCciPs_setCounterCtrl(FCciPs_PfmCnt_T *counter_Ptr);
void FCciPs_enableCounter(FCciPs_PfmCnt_T *counter_Ptr);
void FCciPs_disableCounter(FCciPs_PfmCnt_T *counter_Ptr);
void FCciPs_disableCounterAll(FCciPs_T *dev);
void FCciPs_enableCounterAll(FCciPs_T *dev);
u32 FCciPs_readCounter(FCciPs_PfmCnt_T *counter_Ptr);
void FCciPs_enableMonitor(FCciPs_T *dev);
void FCciPs_disableMonitor(FCciPs_T *dev);
u32 FCciPs_readSlaveMonitor(FCciPs_T *dev, u32 portNum);
u32 FCciPs_readMasterMonitor(FCciPs_T *dev, u32 portNum);
void FCciPs_debugEnable(void);
u32 FCciPs_pmu_start(FCciPs_T *cci, FCciPs_PfmCnt_Config_T *cci_pmu_cfg_array,
                     int configSize);
u32 FCciPs_pmu_stop(FCciPs_T *cci);

/* IRQ ops*/
void FCciPs_enableIrq(FCciPs_T *dev, u32 counterId);
void FCciPs_disableIrq(FCciPs_T *dev, u32 counterId);
void FCciPs_clearIrq(FCciPs_T *dev, u32 counterId);
u32 FCciPs_getIrqStatus(FCciPs_T *dev, u32 counterId);
#endif /* end of protection macro */
