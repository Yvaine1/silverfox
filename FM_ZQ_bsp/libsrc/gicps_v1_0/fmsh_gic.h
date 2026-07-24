/******************************************************************************/
/*****************************************************************************/
/**
*
* @file fmsh_gic.h
* @{
* @details
*
* The generic interrupt controller driver component.
*
* The interrupt controller driver uses the idea of priority for the various
* handlers. Priority is an integer within the range of 1 and 31 inclusive with
* default of 1 being the highest priority interrupt source. The priorities
* of the various sources can be dynamically altered as needed through
* hardware configuration.
*
* The generic interrupt controller supports the following
* features:
*
*   - specific individual interrupt enabling/disabling
*   - specific individual interrupt acknowledging
*   - attaching specific callback function to handle interrupt source
*   - assigning desired priority to interrupt source if default is not
*     acceptable.
*
* Details about connecting the interrupt handler of the driver are contained
* in the source file specific to interrupt processing, FGicPs_intr.c.
*
* This driver is intended to be RTOS and processor independent.  It works with
* physical addresses only.  Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* <b>Interrupt Vector Tables</b>
*
* The device ID of the interrupt controller device is used by the driver as a
* direct index into the configuration data table. The user should populate the
* vector table with handlers and callbacks at run-time using the
* FGicPs_Connect() and FGicPs_Disconnect() functions.
*
* Each vector table entry corresponds to a device that can generate an
* interrupt. Each entry contains an interrupt handler function and an
* argument to be passed to the handler when an interrupt occurs.  The
* user must use FGicPs_Connect() when the interrupt handler takes an
* argument other than the base address.
*
* <b>Nested Interrupts Processing</b>
*
* Nested interrupts are not supported by this driver.
*
* NOTE:
* The generic interrupt controller is not a part of the snoop control unit
* as indicated by the prefix "scu" in the name of the driver.
* It is an independent module in APU.

******************************************************************************/

#ifndef _FMSH_GIC_H_ /* prevent circular inclusions */
#define _FMSH_GIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_gic_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* The following data type defines each entry in an interrupt vector table.
 * The callback reference is the base address of the interrupting device
 * for the low level driver and an instance pointer for the high level driver.
 */
#define GIC_SUCCESS 0L
#define GIC_FAILURE 1L

/*
CPU0:CPu_Id=1;CPU1:CPu_Id= 2;CPU2:CPu_Id = 4;CPU3:CPu_Id = 8
*/
#define GICMAP_CPUID0   1
#define GICMAP_CPUID1   2
#define GICMAP_CPUID2   4
#define GICMAP_CPUID3   8

typedef struct {
    FMSH_InterruptHandler Handler;
    void *CallBackRef;
} FGicPs_VectorTableEntry;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u16 DeviceId;        /**< Unique ID  of device */
    u32 CpuBaseAddress;  /**< CPU Interface Register base address */
    u32 DistBaseAddress; /**< Distributor Register base address */
    FGicPs_VectorTableEntry HandlerTable[FGicPs_MAX_NUM_INTR_INPUTS]; /**<
                  Vector table of interrupt handlers */
} FGicPs_Config;

/**
 * The FGicPs driver instance data. The user is required to allocate a
 * variable of this type for every intc device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
    FGicPs_Config *Config;   /**< Configuration table entry */
    u32 IsReady;             /**< Device is initialized and ready */
    u32 UnhandledInterrupts; /**< Intc Statistics */
} FGicPs;

extern FGicPs IntcInstance;  /* Instance of the Interrupt Controller */
/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
 *
 * Write the given CPU Interface register
 *
 * @param    InstancePtr is a pointer to the instance to be worked on.
 * @param    RegOffset is the register offset to be written
 * @param    Data is the 32-bit value to write to the register
 *
 * @return   None.
 *
 * @note
 * C-style signature:
 *    void FGicPs_CPUWriteReg(FGicPs *InstancePtr, u32 RegOffset, u32 Data)
 *
 *****************************************************************************/
#define FGicPs_CPUWriteReg(InstancePtr, RegOffset, Data)                   \
    (FGicPs_WriteReg(((InstancePtr)->Config->CpuBaseAddress), (RegOffset), \
                     ((u32)(Data))))

/****************************************************************************/
/**
 *
 * Read the given CPU Interface register
 *
 * @param    InstancePtr is a pointer to the instance to be worked on.
 * @param    RegOffset is the register offset to be read
 *
 * @return   The 32-bit value of the register
 *
 * @note
 * C-style signature:
 *    u32 FGicPs_CPUReadReg(FGicPs *InstancePtr, u32 RegOffset)
 *
 *****************************************************************************/
#define FGicPs_CPUReadReg(InstancePtr, RegOffset) \
    (FGicPs_ReadReg(((InstancePtr)->Config->CpuBaseAddress), (RegOffset)))

/****************************************************************************/
/**
 *
 * Write the given Distributor Interface register
 *
 * @param    InstancePtr is a pointer to the instance to be worked on.
 * @param    RegOffset is the register offset to be written
 * @param    Data is the 32-bit value to write to the register
 *
 * @return   None.
 *
 * @note
 * C-style signature:
 *    void FGicPs_DistWriteReg(FGicPs *InstancePtr, u32 RegOffset, u32 Data)
 *
 *****************************************************************************/
#define FGicPs_DistWriteReg(InstancePtr, RegOffset, Data)                   \
    (FGicPs_WriteReg(((InstancePtr)->Config->DistBaseAddress), (RegOffset), \
                     ((u32)(Data))))

/****************************************************************************/
/**
 *
 * Read the given Distributor Interface register
 *
 * @param    InstancePtr is a pointer to the instance to be worked on.
 * @param    RegOffset is the register offset to be read
 *
 * @return   The 32-bit value of the register
 *
 * @note
 * C-style signature:
 *    u32 FGicPs_DistReadReg(FGicPs *InstancePtr, u32 RegOffset)
 *
 *****************************************************************************/
#define FGicPs_DistReadReg(InstancePtr, RegOffset) \
    (FGicPs_ReadReg(((InstancePtr)->Config->DistBaseAddress), (RegOffset)))

/************************** Function Prototypes ******************************/

/*
 * Required functions in FGicPs.c
 */

s32 FGicPs_Connect(FGicPs *InstancePtr, u32 Int_Id,
                   FMSH_InterruptHandler Handler, void *CallBackRef);
void FGicPs_Disconnect(FGicPs *InstancePtr, u32 Int_Id);

void FGicPs_Enable(FGicPs *InstancePtr, u32 Int_Id);
void FGicPs_Disable(FGicPs *InstancePtr, u32 Int_Id);

s32 FGicPs_CfgInitialize(FGicPs *InstancePtr, FGicPs_Config *ConfigPtr,
                         u32 EffectiveAddr);

s32 FGicPs_SoftwareIntr(FGicPs *InstancePtr, u32 Int_Id, u32 Cpu_Id);
s32 FGicPs_registerInt(FGicPs *InstancePtr, u32 Int_Id,
                       FMSH_InterruptHandler Handler, void *CallBackRef);

void FGicPs_GetPriorityTriggerType(FGicPs *InstancePtr, u32 Int_Id,
                                   u8 *Priority, u8 *Trigger);
void FGicPs_SetPriorityTriggerType(FGicPs *InstancePtr, u32 Int_Id, u8 Priority,
                                   u8 Trigger);
void FGicPs_InterruptMaptoCpu(FGicPs *InstancePtr, u8 Cpu_Id, u32 Int_Id);
/*
 * Initialization functions in FGicPs_sinit.c
 */
FGicPs_Config *FGicPs_LookupConfig(u16 DeviceId);

/*
 * Interrupt functions in FGicPs_intr.c
 */
void FGicPs_InterruptHandler_IRQ(FGicPs *InstancePtr);
void FGicPs_InterruptHandler_FIQ(FGicPs *InstancePtr);

/*
 * Self-test functions in FGicPs_selftest.c
 */
// u32  FGicPs_SelfTest(FGicPs *InstancePtr);
u32 FGicPs_SetupInterruptSystem(FGicPs *InstancePtr);

/*
 * Enalbe Interrupt Group
 */
void FGicPs_EnableSelGroup(FGicPs *InstancePtr);
/*
 * Set Interrupt  Group
 */

void FGicPs_SetGroup(FGicPs *InstancePtr, u32 Int_Id, u8 groupNo);

u32 FMSH_In32(u32 Addr);
void FMSH_Out32(u32 Addr, u32 Value);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
