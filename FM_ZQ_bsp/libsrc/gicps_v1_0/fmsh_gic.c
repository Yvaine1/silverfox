/******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fmsh_gic.c
 * @addtogroup scugic_v3_1
 * @{
 *
 * Contains required functions for the FGicPs driver for the Interrupt
 * Controller. See FGicPs.h for a detailed description of the driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- --------------------------------------------------------
 * 1.00  lxl  29/12/2018 Fisrt edit
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

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FGicPs IntcInstance; /* Instance of the Interrupt Controller */

/************************** Function Prototypes ******************************/

static void StubHandler(void *CallBackRef);

/*****************************************************************************/
/**
 *
 * DistributorInit initializes the distributor of the GIC. The
 * initialization entails:
 *
 * - Write the trigger mode, priority and target CPU
 * - All interrupt sources are disabled
 * - Enable the distributor
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 * @param	CpuID is the Cpu ID to be initialized.
 *
 * @return	None
 *
 * @note		None.
 *
 ******************************************************************************/
static void DistributorInit (FGicPs *InstancePtr, u32 CpuID)
{
    u32 Int_Id;
    u32 LocalCpuID = CpuID;

#if USE_AMP == 1
#warning "Building GIC for AMP"

    /*
     * The distrubutor should not be initialized by FreeRTOS in the case of
     * AMP -- it is assumed that Linux is the master of this device in that
     * case.
     */
    return;
#endif
    FMSH_ASSERT(InstancePtr != NULL);
    FGicPs_DistWriteReg(InstancePtr, FGicPs_DIST_EN_OFFSET, 0U);

    /*
     * Set the security domains in the int_security registers for
     * non-secure interrupts
     * All are secure, so leave at the default. Set to 1 for non-secure
     * interrupts.
     */

    /*
     * For the Shared Peripheral Interrupts INT_ID[MAX..32], set:
     */

    /*
     * 1. The trigger mode in the int_config register
     * Only write to the SPI interrupts, so start at 32
     */
    for (Int_Id = 32U; Int_Id < FGicPs_MAX_NUM_INTR_INPUTS;
         Int_Id = Int_Id + 16U)
    {
        /*
         * Each INT_ID uses two bits, or 16 INT_ID per register
         * Set them all to be level sensitive, active HIGH.
         */
        FGicPs_DistWriteReg(InstancePtr, FGicPs_INT_CFG_OFFSET_CALC(Int_Id),
                            0U);
    }

#define DEFAULT_PRIORITY 0xa0a0a0a0U
    for (Int_Id = 0U; Int_Id < FGicPs_MAX_NUM_INTR_INPUTS; Int_Id = Int_Id + 4U)
    {
        /*
         * 2. The priority using int the priority_level register
         * The priority_level and spi_target registers use one byte per
         * INT_ID.
         * Write a default value that can be changed elsewhere.
         */
        FGicPs_DistWriteReg(InstancePtr, FGicPs_PRIORITY_OFFSET_CALC(Int_Id),
                            DEFAULT_PRIORITY);
    }

    for (Int_Id = 32U; Int_Id < FGicPs_MAX_NUM_INTR_INPUTS;
         Int_Id = Int_Id + 4U)
    {
        /*
         * 3. The CPU interface in the spi_target register
         * Only write to the SPI interrupts, so start at 32
         */
        LocalCpuID |= LocalCpuID << 8U;
        LocalCpuID |= LocalCpuID << 16U;

        FGicPs_DistWriteReg(InstancePtr, FGicPs_SPI_TARGET_OFFSET_CALC(Int_Id),
                            LocalCpuID);
    }

    for (Int_Id = 0U; Int_Id < FGicPs_MAX_NUM_INTR_INPUTS;
         Int_Id = Int_Id + 32U)
    {
        /*
         * 4. Clear SPI pending interrupt. add 2022.11.14 by lxl...
         */
        FGicPs_DistWriteReg(
            InstancePtr,
            FGicPs_EN_DIS_OFFSET_CALC(FGicPs_PENDING_CLR_OFFSET, Int_Id),
            0xFFFFFFFFU);
    }

    for (Int_Id = 0U; Int_Id < FGicPs_MAX_NUM_INTR_INPUTS;
         Int_Id = Int_Id + 32U)
    {
        /*
         * 5. Disable the SPI using the enable_set register. Leave all
         * disabled for now.
         */
        FGicPs_DistWriteReg(
            InstancePtr,
            FGicPs_EN_DIS_OFFSET_CALC(FGicPs_DISABLE_OFFSET, Int_Id),
            0xFFFFFFFFU);
    }

    FGicPs_DistWriteReg(InstancePtr, FGicPs_DIST_EN_OFFSET, FGicPs_EN_INT_MASK);
}

/*****************************************************************************/
/**
 *
 * CPUInitialize initializes the CPU Interface of the GIC. The initialization
 *entails:
 *
 *	- Set the priority of the CPU
 *	- Enable the CPU interface
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 *
 * @return	None
 *
 * @note		None.
 *
 ******************************************************************************/
static void CPUInitialize (FGicPs *InstancePtr)
{
    /*
     * Program the priority mask of the CPU using the Priority mask register
     */
    FGicPs_CPUWriteReg(InstancePtr, FGicPs_CPU_PRIOR_OFFSET, 0xF0U);

    /*
     * If the CPU operates in both security domains, set parameters in the
     * control_s register.
     * 1. Set FIQen=1 to use FIQ for secure interrupts,
     * 2. Program the AckCtl bit
     * 3. Program the SBPR bit to select the binary pointer behavior
     * 4. Set EnableS = 1 to enable secure interrupts
     * 5. Set EnbleNS = 1 to enable non secure interrupts
     */

    /*
     * If the CPU operates only in the secure domain, setup the
     * control_s register.
     * 1. Set FIQen=1,
     * 2. Set EnableS=1, to enable the CPU interface to signal secure
     * interrupts. Only enable the IRQ output unless secure interrupts are
     * needed.
     */
    FGicPs_CPUWriteReg(InstancePtr, FGicPs_CONTROL_OFFSET, 0xA3U);
}

/*****************************************************************************/
/**
 *
 * CfgInitialize a specific interrupt controller instance/driver. The
 * initialization entails:
 *
 * - Initialize fields of the FGicPs structure
 * - Initial vector table with stub function calls
 * - All interrupt sources are disabled
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 * @param	ConfigPtr is a pointer to a config table for the particular
 *		device this driver is associated with.
 * @param	EffectiveAddr is the device base address in the virtual memory
 *		address space. The caller is responsible for keeping the address
 *		mapping from EffectiveAddr to the device physical base address
 *		unchanged once this function is invoked. Unexpected errors may
 *		occur if the address mapping changes after this function is
 *		called. If address translation is not used, use
 *		Config->BaseAddress for this parameters, passing the physical
 *		address instead.
 *
 * @return
 *		- GIC_SUCCESS if initialization was successful
 *
 * @note		None.
 *
 ******************************************************************************/
#define FMSH_CPU_ID = 0U;  
s32 FGicPs_CfgInitialize (FGicPs *InstancePtr, FGicPs_Config *ConfigPtr,
                          u32 EffectiveAddr)
{
    u32 Int_Id;
    // u32 Cpu_Id = (u32)FPAR_CPU_ID + (u32)1;
    u32 Cpu_Id = (u32)1U;
    

    (void)EffectiveAddr;

    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(ConfigPtr != NULL);

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

    if (InstancePtr->IsReady != COMPONENT_IS_READY)
    {
        InstancePtr->IsReady = 0;
        InstancePtr->Config = ConfigPtr;

        for (Int_Id = 0U; Int_Id < FGicPs_MAX_NUM_INTR_INPUTS; Int_Id++)
        {
            /*
             * Initalize the handler to point to a stub to handle an
             * interrupt which has not been connected to a handler. Only
             * initialize it if the handler is 0 which means it was not
             * initialized statically by the tools/user. Set the callback
             * reference to this instance so that unhandled interrupts
             * can be tracked.
             */
            if ((InstancePtr->Config->HandlerTable[Int_Id].Handler == NULL))
            {
                InstancePtr->Config->HandlerTable[Int_Id].Handler = StubHandler;
            }
            InstancePtr->Config->HandlerTable[Int_Id].CallBackRef = InstancePtr;
        }

        DistributorInit(InstancePtr, Cpu_Id);
        CPUInitialize(InstancePtr);

        InstancePtr->IsReady = COMPONENT_IS_READY;
    }

    return GIC_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Makes the connection between the Int_Id of the interrupt source and the
 * associated handler that is to run when the interrupt is recognized. The
 * argument provided in this call as the Callbackref is used as the argument
 * for the handler when it is called.
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 * @param	Int_Id contains the ID of the interrupt source and should be
 *		in the range of 0 to FGicPs_MAX_NUM_INTR_INPUTS - 1
 * @param	Handler to the handler for that interrupt.
 * @param	CallBackRef is the callback reference, usually the instance
 *		pointer of the connecting driver.
 *
 * @return
 *
 *		- GIC_SUCCESS if the handler was connected correctly.
 *
 * @note
 *
 * WARNING: The handler provided as an argument will overwrite any handler
 * that was previously connected.
 *
 ****************************************************************************/
s32 FGicPs_Connect (FGicPs *InstancePtr, u32 Int_Id,
                    FMSH_InterruptHandler Handler, void *CallBackRef)
{
    /*
     * Assert the arguments
     */
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(Int_Id < FGicPs_MAX_NUM_INTR_INPUTS);
    FMSH_ASSERT(Handler != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);

    /*
     * The Int_Id is used as an index into the table to select the proper
     * handler
     */
    InstancePtr->Config->HandlerTable[Int_Id].Handler = Handler;
    InstancePtr->Config->HandlerTable[Int_Id].CallBackRef = CallBackRef;

    return GIC_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Updates the interrupt table with the Null Handler and NULL arguments at the
 * location pointed at by the Int_Id. This effectively disconnects that
 *interrupt source from any handler. The interrupt is disabled also.
 *
 * @param	InstancePtr is a pointer to the FGicPs instance to be worked on.
 * @param	Int_Id contains the ID of the interrupt source and should
 *		be in the range of 0 to FGicPs_MAX_NUM_INTR_INPUTS - 1
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void FGicPs_Disconnect (FGicPs *InstancePtr, u32 Int_Id)
{
    u32 Mask;

    /*
     * Assert the arguments
     */
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(Int_Id < FGicPs_MAX_NUM_INTR_INPUTS);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    /*
     * The Int_Id is used to create the appropriate mask for the
     * desired bit position. Int_Id currently limited to 0 - 31
     */
    Mask = 0x00000001U << (Int_Id % 32U);

    /*
     * Disable the interrupt such that it won't occur while disconnecting
     * the handler, only disable the specified interrupt id without modifying
     * the other interrupt ids
     */
    FGicPs_DistWriteReg(
        InstancePtr, (u32)FGicPs_DISABLE_OFFSET + ((Int_Id / 32U) * 4U), Mask);

    /*
     * Disconnect the handler and connect a stub, the callback reference
     * must be set to this instance to allow unhandled interrupts to be
     * tracked
     */
    InstancePtr->Config->HandlerTable[Int_Id].Handler = StubHandler;
    InstancePtr->Config->HandlerTable[Int_Id].CallBackRef = InstancePtr;
}

/*****************************************************************************/
/**
 *
 * Enables the interrupt source provided as the argument Int_Id. Any pending
 * interrupt condition for the specified Int_Id will occur after this function
 *is called.
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 * @param	Int_Id contains the ID of the interrupt source and should be
 *		in the range of 0 to FGicPs_MAX_NUM_INTR_INPUTS - 1
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void FGicPs_Enable (FGicPs *InstancePtr, u32 Int_Id)
{
    u32 Mask;

    /*
     * Assert the arguments
     */
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(Int_Id < FGicPs_MAX_NUM_INTR_INPUTS);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    /*
     * The Int_Id is used to create the appropriate mask for the
     * desired bit position. Int_Id currently limited to 0 - 31
     */
    Mask = 0x00000001U << (Int_Id % 32U);

    /*
     * Enable the selected interrupt source by setting the
     * corresponding bit in the Enable Set register.
     */
    FGicPs_DistWriteReg(InstancePtr,
                        (u32)FGicPs_ENABLE_SET_OFFSET + ((Int_Id / 32U) * 4U),
                        Mask);
}

/*****************************************************************************/
/**
 *
 * Disables the interrupt source provided as the argument Int_Id such that the
 * interrupt controller will not cause interrupts for the specified Int_Id. The
 * interrupt controller will continue to hold an interrupt condition for the
 * Int_Id, but will not cause an interrupt.
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 * @param	Int_Id contains the ID of the interrupt source and should be
 *		in the range of 0 to FGicPs_MAX_NUM_INTR_INPUTS - 1
 *
 * @return	None.
 *
 * @note		None.
 *
 ****************************************************************************/
void FGicPs_Disable (FGicPs *InstancePtr, u32 Int_Id)
{
    u32 Mask;

    /*
     * Assert the arguments
     */
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(Int_Id < FGicPs_MAX_NUM_INTR_INPUTS);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    /*
     * The Int_Id is used to create the appropriate mask for the
     * desired bit position. Int_Id currently limited to 0 - 31
     */
    Mask = 0x00000001U << (Int_Id % 32U);

    /*
     * Disable the selected interrupt source by setting the
     * corresponding bit in the IDR.
     */
    FGicPs_DistWriteReg(
        InstancePtr, (u32)FGicPs_DISABLE_OFFSET + ((Int_Id / 32U) * 4U), Mask);
}

/*****************************************************************************/
/**
 *
 * Allows software to simulate an interrupt in the interrupt controller.  This
 * function will only be successful when the interrupt controller has been
 * started in simulation mode.  A simulated interrupt allows the interrupt
 * controller to be tested without any device to drive an interrupt input
 * signal into it.
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 * @param	Int_Id is the software interrupt ID to simulate an interrupt.
 * @param	Cpu_Id is the list of CPUs to send the interrupt.
 *CPU0:CPu_Id=1;CPU1:CPu_Id = 2;CPU2:CPu_Id = 4;CPU3:CPu_Id = 8
 *
 * @return
 *
 * GIC_SUCCESS if successful, or GIC_FAILURE if the interrupt could not be
 * simulated
 *
 * @note		None.
 *
 ******************************************************************************/
s32 FGicPs_SoftwareIntr (FGicPs *InstancePtr, u32 Int_Id, u32 Cpu_Id)
{
    u32 Mask;

    /*
     * Assert the arguments
     */
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    FMSH_ASSERT(Int_Id <= 15U);
    FMSH_ASSERT(Cpu_Id <= 255U);

    /*
     * The Int_Id is used to create the appropriate mask for the
     * desired interrupt. Int_Id currently limited to 0 - 15
     * Use the target list for the Cpu ID.
     */
    Mask = ((Cpu_Id << 16U) | Int_Id) &
           (FGicPs_SFI_TRIG_CPU_MASK | FGicPs_SFI_TRIG_INTID_MASK);

    /*
     * Write to the Software interrupt trigger register. Use the appropriate
     * CPU Int_Id.
     */
    FGicPs_DistWriteReg(InstancePtr, FGicPs_SFI_TRIG_OFFSET, Mask);

    /* Indicate the interrupt was successfully simulated */

    return GIC_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * A stub for the asynchronous callback. The stub is here in case the upper
 * layers forget to set the handler.
 *
 * @param	CallBackRef is a pointer to the upper layer callback reference
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
static void StubHandler (void *CallBackRef)
{
    /*
     * verify that the inputs are valid
     */
    FMSH_ASSERT(CallBackRef != NULL);

    /*
     * Indicate another unhandled interrupt for stats
     */
    ((FGicPs *)((void *)CallBackRef))->UnhandledInterrupts++;
}

s32 FGicPs_registerInt (FGicPs *InstancePtr, u32 Int_Id,
                        FMSH_InterruptHandler Handler, void *CallBackRef)
{
    s32 Status;
    Status = FGicPs_Connect(InstancePtr, Int_Id, Handler, CallBackRef);
    if (Status != GIC_SUCCESS)
    {
        return Status;
    }

    FGicPs_Enable(InstancePtr, Int_Id);

    return GIC_SUCCESS;
}

/****************************************************************************/
/**
 * Sets the interrupt priority and trigger type for the specificd IRQ source.
 *
 * @param	InstancePtr is a pointer to the instance to be worked on.
 * @param	Int_Id is the IRQ source number to modify
 * @param	Priority is the new priority for the IRQ source. 0 is highest
 * 			priority, 0xF8 (248) is lowest. There are 32 priority levels
 *			supported with a step of 8. Hence the supported priorities are
 *			0, 8, 16, 32, 40 ..., 248.
 * @param	Trigger is the new trigger type for the IRQ source.
 * Each bit pair describes the configuration for an INT_ID.
 * SFI    Read Only    b10 always
 * PPI    Read Only    depending on how the PPIs are configured.
 *                    b01    Active HIGH level sensitive
 *                    b11 Rising edge sensitive
 * SPI                LSB is read only.
 *                    b01    Active HIGH level sensitive
 *                    b11 Rising edge sensitive/
 *
 * @return	None.
 *
 * @note		None.
 *
 *****************************************************************************/
void FGicPs_SetPriorityTriggerType (FGicPs *InstancePtr, u32 Int_Id,
                                    u8 Priority, u8 Trigger)
{
    u32 RegValue;
    u8 LocalPriority;
    LocalPriority = Priority;

    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    FMSH_ASSERT(Int_Id < FGicPs_MAX_NUM_INTR_INPUTS);
    FMSH_ASSERT(Trigger <= (u8)FGicPs_INT_CFG_MASK);
    FMSH_ASSERT(LocalPriority <= (u8)FGicPs_MAX_INTR_PRIO_VAL);

    /*
     * Determine the register to write to using the Int_Id.
     */
    RegValue = FGicPs_DistReadReg(InstancePtr,
                                  FGicPs_PRIORITY_OFFSET_CALC(Int_Id));

    /*
     * The priority bits are Bits 7 to 3 in GIC Priority Register. This
     * means the number of priority levels supported are 32 and they are
     * in steps of 8. The priorities can be 0, 8, 16, 32, 48, ... etc.
     * The lower order 3 bits are masked before putting it in the register.
     */
    LocalPriority = LocalPriority & (u8)FGicPs_INTR_PRIO_MASK;
    /*
     * Shift and Mask the correct bits for the priority and trigger in the
     * register
     */
    RegValue &= ~(FGicPs_PRIORITY_MASK << ((Int_Id % 4U) * 8U));
    RegValue |= (u32)LocalPriority << ((Int_Id % 4U) * 8U);

    /*
     * Write the value back to the register.
     */
    FGicPs_DistWriteReg(InstancePtr, FGicPs_PRIORITY_OFFSET_CALC(Int_Id),
                        RegValue);

    /*
     * Determine the register to write to using the Int_Id.
     */
    RegValue = FGicPs_DistReadReg(InstancePtr,
                                  FGicPs_INT_CFG_OFFSET_CALC(Int_Id));

    /*
     * Shift and Mask the correct bits for the priority and trigger in the
     * register
     */
    RegValue &= ~(FGicPs_INT_CFG_MASK << ((Int_Id % 16U) * 2U));
    RegValue |= (u32)Trigger << ((Int_Id % 16U) * 2U);

    /*
     * Write the value back to the register.
     */
    FGicPs_DistWriteReg(InstancePtr, FGicPs_INT_CFG_OFFSET_CALC(Int_Id),
                        RegValue);
}

/****************************************************************************/
/**
 * Gets the interrupt priority and trigger type for the specificd IRQ source.
 *
 * @param	InstancePtr is a pointer to the instance to be worked on.
 * @param	Int_Id is the IRQ source number to modify
 * @param	Priority is a pointer to the value of the priority of the IRQ
 *		source. This is a return value.
 * @param	Trigger is pointer to the value of the trigger of the IRQ
 *		source. This is a return value.
 *
 * @return	None.
 *
 * @note		None
 *
 *****************************************************************************/
void FGicPs_GetPriorityTriggerType (FGicPs *InstancePtr, u32 Int_Id,
                                    u8 *Priority, u8 *Trigger)
{
    u32 RegValue;

    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    FMSH_ASSERT(Int_Id < FGicPs_MAX_NUM_INTR_INPUTS);
    FMSH_ASSERT(Priority != NULL);
    FMSH_ASSERT(Trigger != NULL);

    /*
     * Determine the register to read to using the Int_Id.
     */
    RegValue = FGicPs_DistReadReg(InstancePtr,
                                  FGicPs_PRIORITY_OFFSET_CALC(Int_Id));

    /*
     * Shift and Mask the correct bits for the priority and trigger in the
     * register
     */
    RegValue = RegValue >> ((Int_Id % 4U) * 8U);
    *Priority = (u8)(RegValue & FGicPs_PRIORITY_MASK);

    /*
     * Determine the register to read to using the Int_Id.
     */
    RegValue = FGicPs_DistReadReg(InstancePtr,
                                  FGicPs_INT_CFG_OFFSET_CALC(Int_Id));

    /*
     * Shift and Mask the correct bits for the priority and trigger in the
     * register
     */
    RegValue = RegValue >> ((Int_Id % 16U) * 2U);

    *Trigger = (u8)(RegValue & FGicPs_INT_CFG_MASK);
}
/****************************************************************************/
/**
 * Sets the target CPU for the interrupt of a peripheral
 *
 * @param	InstancePtr is a pointer to the instance to be worked on.
 * @param	Cpu_Id is a CPU number for which the interrupt has to be targeted
 * @param	Int_Id is the IRQ source number to modify,CPU0:CPu_Id=1;CPU1:CPu_Id
 *= 2;CPU2:CPu_Id = 4;CPU3:CPu_Id = 8
 *
 * @return	None.
 *
 * @note		None
 *
 *****************************************************************************/
void FGicPs_InterruptMaptoCpu (FGicPs *InstancePtr, u8 Cpu_Id, u32 Int_Id)
{
    u32 RegValue, Offset;
    RegValue = FGicPs_DistReadReg(InstancePtr,
                                  FGicPs_SPI_TARGET_OFFSET_CALC(Int_Id));

    Offset = (Int_Id & 0x3);

    // RegValue = (RegValue | (~(0xFF << (Offset*8))) );
    RegValue = (RegValue & (0xFFFFFFFF & (~(0xFF << (Offset * 8)))));
    RegValue |= ((Cpu_Id) << (Offset * 8));

    FGicPs_DistWriteReg(InstancePtr, FGicPs_SPI_TARGET_OFFSET_CALC(Int_Id),
                        RegValue);
}
/** @} */

/******Interrupt Setup********/
u32 FGicPs_SetupInterruptSystem (FGicPs *InstancePtr)
{
    u32 RegValue1 = 0U;
    u32 Index;
    u32 Status;
    static FGicPs_Config *GicConfig;

    GicConfig = FGicPs_LookupConfig(GIC_DEVICE_ID);
    if (NULL == GicConfig)
    {
        return GIC_FAILURE;
    }
    InstancePtr->Config = GicConfig;
    /*
     * Read the ID registers.
     */
    for (Index = 0U; Index <= 3U; Index++)
    {
        RegValue1 |= FGicPs_DistReadReg(
                         InstancePtr,
                         ((u32)FGicPs_PCELLID_OFFSET + (Index * 4U)))
                     << (Index * 8U);
    }

    if (FGicPs_PCELL_ID != RegValue1)
    {
        return GIC_FAILURE;
    }
    /*
    FGicPs_DistWriteReg(InstancePtr,
    FGicPs_INT_CFG_OFFSET_CALC(32U),
    0U);
    */
    {
        // int Status;
        Status = FGicPs_CfgInitialize(InstancePtr, GicConfig,
                                      GicConfig->CpuBaseAddress);
        if (Status != GIC_SUCCESS)
        {
            return GIC_FAILURE;
        }
    }

    // FMSH_ExceptionEnable();
    return Status;
}
/******Enable interrupt  Group********/
void FGicPs_EnableSelGroup (FGicPs *InstancePtr)
{
    FGicPs_DistWriteReg(InstancePtr, FGicPs_DIST_EN_OFFSET, 0x03);
    FGicPs_CPUWriteReg(InstancePtr, FGicPs_CONTROL_OFFSET, 0x1FU);
}
/******Set interrupt  Group********/
void FGicPs_SetGroup (FGicPs *InstancePtr, u32 Int_Id, u8 groupNo)
{
    u32 RegValue;

    RegValue = FGicPs_DistReadReg(InstancePtr,
                                  FGicPs_SECURITY_OFFSET_CALC(Int_Id));

    /*
     * Enable the selected interrupt source by setting the
     * corresponding bit in the Enable Set register.

     */

    RegValue &= ~(0x00000001 << (Int_Id % 32U));
    RegValue |= ((u32)groupNo << (Int_Id % 32U));

    FGicPs_DistWriteReg(InstancePtr,
                        (u32)FGicPs_SECURITY_OFFSET + ((Int_Id / 32U) * 4U),
                        RegValue);
}

u32 FMSH_In32 (u32 Addr) { return *(volatile u32 *)Addr; }

void FMSH_Out32 (u32 Addr, u32 Value)
{
    u32 *LocalAddr = (u32 *)Addr;
    *LocalAddr = Value;
}
