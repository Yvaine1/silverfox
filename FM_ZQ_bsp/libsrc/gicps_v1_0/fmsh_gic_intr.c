/******************************************************************************
*
* @file fmsh_gic_intr.c
* @addtogroup scugic_v3_1
* @{
*
* This file contains the interrupt processing for the driver for the FMSH
* Interrupt Controller.  The interrupt processing is partitioned separately such
* that users are not required to use the provided interrupt processing.  This
* file requires other files of the driver to be linked in also.
*
* The interrupt handler, FGicPs_InterruptHandler, uses an input argument which
* is an instance pointer to an interrupt controller driver such that multiple
* interrupt controllers can be supported.  This handler requires the calling
* function to pass it the appropriate argument, so another level of indirection
* may be required.
*
* The interrupt processing may be used by connecting the interrupt handler to
* the interrupt system.  The handler does not save and restore the processor
* context but only handles the processing of the Interrupt Controller. The user
* is encouraged to supply their own interrupt handler when performance tuning is
* deemed necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a drg  01/19/10 First release

* </pre>
*
* @internal
*
* This driver assumes that the context of the processor has been saved prior to
* the calling of the Interrupt Controller interrupt handler and then restored
* after the handler returns. This requires either the running RTOS to save the
* state of the machine or that a wrapper be used as the destination of the
* interrupt vector to save the state of the processor and restore the state
* after the interrupt handler returns.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_common_types.h"
#include "fmsh_gic.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function is the primary interrupt handler for the driver.  It must be
 * connected to the interrupt source such that it is called when an interrupt of
 * the interrupt controller is active. It will resolve which interrupts are
 * active and enabled and call the appropriate interrupt handler. It uses
 * the Interrupt Type information to determine when to acknowledge the
 *interrupt. Highest priority interrupts are serviced first.
 *
 * This function assumes that an interrupt vector table has been previously
 * initialized.  It does not verify that entries in the table are valid before
 * calling an interrupt handler.
 *
 *
 * @param	InstancePtr is a pointer to the FGicPs instance.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FGicPs_InterruptHandler_IRQ (FGicPs *InstancePtr)
{
    u32 InterruptID;
    u32 IntIDFull;
    FGicPs_VectorTableEntry *TablePtr;

    /* Assert that the pointer to the instance is valid
     */
    FMSH_ASSERT(InstancePtr != NULL);

    /*
     * Read the int_ack register to identify the highest priority interrupt ID
     * and make sure it is valid. Reading Int_Ack will clear the interrupt
     * in the GIC.
     */
    IntIDFull = FGicPs_CPUReadReg(InstancePtr, FGicPs_INT_ACK_OFFSET);
    InterruptID = IntIDFull & FGicPs_ACK_INTID_MASK;

    if (FGicPs_MAX_NUM_INTR_INPUTS < InterruptID)
    {
        goto IntrExit;
    }

    /*
     * If the interrupt is shared, do some locking here if there are multiple
     * processors.
     */
    /*
     * If pre-eption is required:
     * Re-enable pre-emption by setting the CPSR I bit for non-secure ,
     * interrupts or the F bit for secure interrupts
     */

    /*
     * If we need to change security domains, issue a SMC instruction here.
     */

    /*
     * Execute the ISR. Jump into the Interrupt service routine based on the
     * IRQSource. A software trigger is cleared by the ACK.
     */
    TablePtr = &(InstancePtr->Config->HandlerTable[InterruptID]);
    if (TablePtr != NULL)
    {
        TablePtr->Handler(TablePtr->CallBackRef);
    }

IntrExit:
    /*
     * Write to the EOI register, we are all done here.
     * Let this function return, the boot code will restore the stack.
     */
    FGicPs_CPUWriteReg(InstancePtr, FGicPs_EOI_OFFSET, IntIDFull);

    /*
     * Return from the interrupt. Change security domains could happen here.
     */
}

void FGicPs_InterruptHandler_FIQ (FGicPs *InstancePtr)
{
    u32 InterruptID;
    u32 IntIDFull;
    FGicPs_VectorTableEntry *TablePtr;

    /* Assert that the pointer to the instance is valid
     */
    FMSH_ASSERT(InstancePtr != NULL);

    /*
     * Read the int_ack register to identify the highest priority interrupt ID
     * and make sure it is valid. Reading Int_Ack will clear the interrupt
     * in the GIC.
     */
    IntIDFull = FGicPs_CPUReadReg(InstancePtr, FGicPs_INT_ACK_OFFSET);
    InterruptID = IntIDFull & FGicPs_ACK_INTID_MASK;

    if (FGicPs_MAX_NUM_INTR_INPUTS < InterruptID)
    {
        goto IntrExit;
    }

    /*
     * If the interrupt is shared, do some locking here if there are multiple
     * processors.
     */
    /*
     * If pre-eption is required:
     * Re-enable pre-emption by setting the CPSR I bit for non-secure ,
     * interrupts or the F bit for secure interrupts
     */

    /*
     * If we need to change security domains, issue a SMC instruction here.
     */

    /*
     * Execute the ISR. Jump into the Interrupt service routine based on the
     * IRQSource. A software trigger is cleared by the ACK.
     */
    TablePtr = &(InstancePtr->Config->HandlerTable[InterruptID]);
    if (TablePtr != NULL)
    {
        TablePtr->Handler(TablePtr->CallBackRef);
    }

IntrExit:
    /*
     * Write to the EOI register, we are all done here.
     * Let this function return, the boot code will restore the stack.
     */
    FGicPs_CPUWriteReg(InstancePtr, FGicPs_EOI_OFFSET, IntIDFull);

    /*
     * Return from the interrupt. Change security domains could happen here.
     */
}
/** @} */
