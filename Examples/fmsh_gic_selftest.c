/******************************************************************************
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file FGicPs_selftest.c
 * @addtogroup scugic_v3_1
 * @{
 *
 * Contains diagnostic self-test functions for the FGicPs driver.
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a drg  01/19/10 First release
 * 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_common_types.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"

/************************** Constant Definitions *****************************/
#define SGI_ID 0U
#define CPU_ID 1U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void SGI0_hanlder(void *InstancePtr);

/************************** Variable Definitions *****************************/
u32 gicTestFlag = GIC_FAILURE;

/*****************************************************************************/
/**
 *
 * Run a self-test on the driver/device. This test reads the ID registers and
 * compares them.
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 *
 * @return
 *
 * 		- GIC_SUCCESS if self-test is successful.
 * 		- GIC_FAILURE if the self-test is not successful.
 *
 * @note		None.
 *
 ******************************************************************************/

u32 FGicPs_CommonInit (FGicPs *InstancePtr)
{
    u32 Status;

    Status = FGicPs_SetupInterruptSystem(InstancePtr);
    if (Status != GIC_SUCCESS)
    {
        return GIC_FAILURE;
    }

    FMSH_ExceptionRegisterHandler(
        FMSH_EXCEPTION_ID_IRQ_INT,
        (FMSH_ExceptionHandler)FGicPs_InterruptHandler_IRQ, InstancePtr);

    return Status;
}

u32 FGicPs_SelfTest (FGicPs *InstancePtr)
{
    u32 Cpu_Id = 0x1;
    u32 Status;
    u32 counter;

#if defined(CORTEX_A53)
    u64 mpidr;
    
    mfcp(MPIDR_EL1, mpidr);
    mpidr = mpidr & 0xff;
    if (mpidr == 0)
    {
        Cpu_Id = 0x1;
    }
    else if (mpidr == 1)
    {
        Cpu_Id = 0x2;
    }
    else if (mpidr == 2)
    {
        Cpu_Id = 0x4;
    }
    else if (mpidr == 3)
    {
        Cpu_Id = 0x8;
    }
    else
    {
        Cpu_Id = 0x1;
    }

#elif defined(CORTEX_R5)
    u32 mpidr;
    
    mfcp(CP15_MULTI_PROC_AFFINITY, mpidr);
    mpidr = mpidr & 0xff;
    if (mpidr == 0)
    {
        Cpu_Id = 0x1;
    }
    else if (mpidr == 1)
    {
        Cpu_Id = 0x2;
    }
    else
    {
        Cpu_Id = 0x1;
    }

#endif

    /*
        Status = FGicPs_CommonInit(InstancePtr);
        if (Status != GIC_SUCCESS)
        {
            return GIC_FAILURE;
        }
    */

    gicTestFlag = GIC_FAILURE;

    Status = FGicPs_Connect(InstancePtr, SGI_ID,
                            (FMSH_InterruptHandler)SGI0_hanlder, InstancePtr);
    if (Status != GIC_SUCCESS)
    {
        return GIC_FAILURE;
    }

    FGicPs_Enable(InstancePtr, SGI_ID);

    FGicPs_SoftwareIntr(InstancePtr, SGI_ID, Cpu_Id);

    counter = 1000000;
    while ((gicTestFlag == GIC_FAILURE) && (--counter > 0));

    return gicTestFlag;
}

void SGI0_hanlder (void *InstancePtr) { gicTestFlag = GIC_SUCCESS; }
