/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
*
* @file  fmsh_cci.c
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

#include <string.h>

#include "fmsh_cci.h"
#include "fmsh_cci_hw.h"

u32 FCciPs_Initialize (FCciPs_T *dev)
{
    dev->BaseAddress = FPS_CCI_BASEADDR;
    (void)memset(dev->Counter, 0, sizeof(dev->Counter));
    int id;
    for (id = 0; id < FCCIPS_PFMCOUNT_NUM; id++)
    {
        dev->Counter[id].cci = dev;
        dev->Counter[id].counterId = id;
    }
    return FCCIST_SUCCESS;
}

/*
 * Initialize a counter with a config
 */
u32 FCciPs_pfmCntCfgInit (FCciPs_PfmCnt_T *counterPtr,
                          FCciPs_PfmCnt_Config_T *CfgPtr)
{
    counterPtr->Event = CfgPtr->Event;
    counterPtr->interface = CfgPtr->interface;
    counterPtr->eventCount = 0;
    return FCCIST_SUCCESS;
}

void FCciPs_enableMonitor (FCciPs_T *dev)
{
    FCciPs_WriteReg(dev->BaseAddress, FCCI_DBG_CTRL_OFFSET,
                    FCCI_DBG_CTRL_ENABLE);
}

void FCciPs_disableMonitor (FCciPs_T *dev)
{
    FCciPs_WriteReg(dev->BaseAddress, FCCI_DBG_CTRL_OFFSET,
                    FCCI_DBG_CTRL_DISABLE);
}

void FCciPs_waitPending (FCciPs_T *dev)
{
    while (FCciPs_getPendingStatus(dev)){};
}

u32 FCciPs_getPendingStatus (FCciPs_T *dev)
{
    u32 RegVal;
    RegVal = FCciPs_ReadReg(dev->BaseAddress, FCCI_STATUS_OFFSET);
    return (RegVal & FCCI_STATUS_PENDING_MASK);
}

void FCciPs_enableACE (FCciPs_T *dev, u32 portNum)
{
    FCciPs_WriteReg(dev->BaseAddress, FCCI_SNOOP_CTRL_OFFSET(portNum),
                    FCCI_SNOOP_CTRL_DVM_ENABLE | FCCI_SNOOP_CTRL_SNOOP_ENABLE);
    dsb();
    FCciPs_waitPending(dev);
}

void FCciPs_disableACE (FCciPs_T *dev, u32 portNum)
{
    FCciPs_WriteReg(
        dev->BaseAddress, FCCI_SNOOP_CTRL_OFFSET(portNum),
        FCCI_SNOOP_CTRL_DVM_DISABLE | FCCI_SNOOP_CTRL_SNOOP_DISABLE);
    dsb();
    FCciPs_waitPending(dev);
}

void FCciPs_enable_APU_ACE (FCciPs_T *dev)
{
    FCciPs_enableACE(dev, CCI_APU_PORT);
}

void FCciPs_disable_APU_ACE (FCciPs_T *dev)
{
    FCciPs_disableACE(dev, CCI_APU_PORT);
}

void FCciPs_enable_PL_ACE (FCciPs_T *dev)
{
    FCciPs_enableACE(dev, CCI_PL_ACE_PORT);
}

void FCciPs_disable_PL_ACE (FCciPs_T *dev)
{
    FCciPs_disableACE(dev, CCI_PL_ACE_PORT);
}

void FCciPs_resetCounterAll (FCciPs_T *dev)
{
    FCciPs_MaskWrite(dev->BaseAddress, FCCI_PMU_CTRL_OFFSET,
                     FCCI_PMU_CTRL_RST_RESET << FCCI_PMU_CTRL_RST_SHIFT,
                     FCCI_PMU_CTRL_RST_MASK);
}

void FCciPs_enableCounterAll (FCciPs_T *dev)
{
    FCciPs_debugEnable();
    FCciPs_MaskWrite(dev->BaseAddress, FCCI_PMU_CTRL_OFFSET,
                     FCCI_PMU_CTRL_CEN_ENABLE << FCCI_PMU_CTRL_CEN_SHIFT,
                     FCCI_PMU_CTRL_CEN_MASK);
}

void FCciPs_disableCounterAll (FCciPs_T *dev)
{
    FCciPs_MaskWrite(dev->BaseAddress, FCCI_PMU_CTRL_OFFSET,
                     FCCI_PMU_CTRL_CEN_DISABLE << FCCI_PMU_CTRL_CEN_SHIFT,
                     FCCI_PMU_CTRL_CEN_MASK);
}

void FCciPs_setCounterCtrl (FCciPs_PfmCnt_T *counter_Ptr)
{
    u32 RegVal;
    RegVal = (counter_Ptr->interface << 5) | (counter_Ptr->Event);
    FCciPs_WriteReg(counter_Ptr->cci->BaseAddress,
                    FCCI_EVNT_SEL_OFFSET(counter_Ptr->counterId), RegVal);
}

void FCciPs_enableCounter (FCciPs_PfmCnt_T *counter_Ptr)
{
    FCciPs_WriteReg(counter_Ptr->cci->BaseAddress,
                    FCCI_ECNT_CTRL_OFFSET(counter_Ptr->counterId),
                    FCCI_ECNT_CTRL_COUNT_ENABLE);
}

void FCciPs_disableCounter (FCciPs_PfmCnt_T *counter_Ptr)
{
    FCciPs_WriteReg(counter_Ptr->cci->BaseAddress,
                    FCCI_ECNT_CTRL_OFFSET(counter_Ptr->counterId),
                    FCCI_ECNT_CTRL_COUNT_DISABLE);
}

u32 FCciPs_readCounter (FCciPs_PfmCnt_T *counter_Ptr)
{
    struct FCciPs *dev = counter_Ptr->cci;
    int counterId = counter_Ptr->counterId;
    counter_Ptr->eventCount = FCciPs_ReadReg(dev->BaseAddress,
                                             FCCI_ECNT_DATA_OFFSET(counterId));

    return counter_Ptr->eventCount;
}

void FCciPs_shareOverride (FCciPs_T *dev, u32 portNum, u32 val)
{
    FCciPs_WriteReg(dev->BaseAddress, FCCI_SHARE_OVR_OFFSET(portNum), val);
}

u32 FCciPs_readSlaveMonitor (FCciPs_T *dev, u32 portNum)
{
    return FCciPs_ReadReg(dev->BaseAddress, FCCI_SLAVE_MONITOR_OFFSET(portNum));
}

u32 FCciPs_readMasterMonitor (FCciPs_T *dev, u32 portNum)
{
    return FCciPs_ReadReg(dev->BaseAddress,
                          FCCI_MASTER_MONITOR_OFFSET(portNum));
}

void FCciPs_enableIrq (FCciPs_T *dev, u32 counterId)
{
    FCciPs_WriteReg(CCI_SLCR_BASEADDR, CCI_SLCR_IER, 1 << (counterId + 1));
}

void FCciPs_disableIrq (FCciPs_T *dev, u32 counterId)
{
    FCciPs_WriteReg(CCI_SLCR_BASEADDR, CCI_SLCR_IDR, 1 << (counterId + 1));
}

void FCciPs_clearIrq (FCciPs_T *dev, u32 counterId)
{
    FCciPs_WriteReg(dev->BaseAddress, FCCI_ECNT_CLR_OVFL_OFFSET(counterId), 1);
    FCciPs_WriteReg(CCI_SLCR_BASEADDR, CCI_SLCR_ISR, 1 << (counterId + 1));
}

u32 FCciPs_getIrqStatus (FCciPs_T *dev, u32 counterId)
{
    return FCciPs_ReadReg(dev->BaseAddress,
                          FCCI_ECNT_CLR_OVFL_OFFSET(counterId));
}

void FCciPs_debugEnable ()
{
    FCciPs_WriteReg(CCI_SLCR_BASEADDR, CCI_SLCR_MISC_DEBUG, 0xF);
}

u32 FCciPs_pmu_start (FCciPs_T *cci, FCciPs_PfmCnt_Config_T *cci_pmu_cfg_array,
                      int configSize)
{
    int id;

    FCciPs_disableCounterAll(cci);

    if (configSize > FCCIPS_PFMCOUNT_NUM)
    {
        configSize = FCCIPS_PFMCOUNT_NUM;
    }

    for (id = 0; id < configSize; id++)
    {
        FCciPs_pfmCntCfgInit(&cci->Counter[id], &cci_pmu_cfg_array[id]);
        FCciPs_setCounterCtrl(&cci->Counter[id]);
        FCciPs_enableCounter(&cci->Counter[id]);
    }
    /*  PMU start  */
    FCciPs_resetCounterAll(cci);
    FCciPs_enableCounterAll(cci);
    return FCCIST_SUCCESS;
}

u32 FCciPs_pmu_stop (FCciPs_T *cci)
{
    int id;
    FCciPs_disableCounterAll(cci);
    for (id = 0; id < FCCIPS_PFMCOUNT_NUM; id++)
    {
        FCciPs_readCounter(&cci->Counter[id]);
    }
    FCciPs_disableCounter(&cci->Counter[id]);
    return 0;
}
