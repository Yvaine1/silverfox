/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/

/*****************************************************************************/
#ifndef IPIPSU_H_
#define IPIPSU_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_common.h"
#include "fmsh_ipi_hw.h"

/************************** Constant Definitions *****************************/
#define IPIPSU_BUF_TYPE_MSG           (0x001U)
#define IPIPSU_BUF_TYPE_RESP          (0x002U)
#define IPIPSU_MAX_MSG_LEN            IPIPSU_MSG_BUF_SIZE

// #define FMSH_SUCCESS     0U
// #define FMSH_FAILURE     1U
#define FMSH_INVALID_PARAM            -1

#define PAR_IPIPSU_0_DEVICE_ID        0U

#define PAR_IPIPSU_NUM_INSTANCES      7U /* the pmu is special */

/* Parameter definitions for peripheral psu_ipi_0 */
#define PAR_PSU_IPI_0_DEVICE_ID       0U
#define PAR_PSU_IPI_1_DEVICE_ID       1U
#define PAR_PSU_IPI_2_DEVICE_ID       2U
#define PAR_PSU_IPI_3_DEVICE_ID       3U
#define PAR_PSU_IPI_4_DEVICE_ID       4U
#define PAR_PSU_IPI_5_DEVICE_ID       5U
#define PAR_PSU_IPI_6_DEVICE_ID       6U
#define PAR_PSU_IPI_7_DEVICE_ID       7U
#define PAR_PSU_IPI_8_DEVICE_ID       8U
#define PAR_PSU_IPI_9_DEVICE_ID       9U
#define PAR_PSU_IPI_10_DEVICE_ID      10U

#define PAR_PSU_IPI_0_S_AXI_BASEADDR  0xFF300000U
#define PAR_PSU_IPI_1_S_AXI_BASEADDR  0xFF310000U
#define PAR_PSU_IPI_2_S_AXI_BASEADDR  0xFF320000U
#define PAR_PSU_IPI_7_S_AXI_BASEADDR  0xFF340000U
#define PAR_PSU_IPI_8_S_AXI_BASEADDR  0xFF350000U
#define PAR_PSU_IPI_9_S_AXI_BASEADDR  0xFF360000U
#define PAR_PSU_IPI_10_S_AXI_BASEADDR 0xFF370000U

// #define  PAR_PSU_IPI_0_BIT_MASK  0x00000001U
// #define  PAR_PSU_IPI_0_BUFFER_INDEX  2U
#define PAR_PSU_IPI_0_INT_ID          67U
#define PAR_PSU_IPI_1_INT_ID          65U
#define PAR_PSU_IPI_2_INT_ID          66U
#define PAR_PSU_IPI_7_INT_ID          61U
#define PAR_PSU_IPI_8_INT_ID          62U
#define PAR_PSU_IPI_9_INT_ID          63U
#define PAR_PSU_IPI_10_INT_ID         64U

#define PAR_IPIPSU_NUM_TARGETS        11U

#define PAR_PSU_IPI_0_BIT_MASK        0x00000001U
#define PAR_PSU_IPI_0_BUFFER_INDEX    2U
#define PAR_PSU_IPI_1_BIT_MASK        0x00000100U
#define PAR_PSU_IPI_1_BUFFER_INDEX    0U
#define PAR_PSU_IPI_2_BIT_MASK        0x00000200U
#define PAR_PSU_IPI_2_BUFFER_INDEX    1U
#define PAR_PSU_IPI_3_BIT_MASK        0x00010000U
#define PAR_PSU_IPI_3_BUFFER_INDEX    7U
#define PAR_PSU_IPI_4_BIT_MASK        0x00020000U
#define PAR_PSU_IPI_4_BUFFER_INDEX    7U
#define PAR_PSU_IPI_5_BIT_MASK        0x00040000U
#define PAR_PSU_IPI_5_BUFFER_INDEX    7U
#define PAR_PSU_IPI_6_BIT_MASK        0x00080000U
#define PAR_PSU_IPI_6_BUFFER_INDEX    7U
#define PAR_PSU_IPI_7_BIT_MASK        0x01000000U
#define PAR_PSU_IPI_7_BUFFER_INDEX    3U
#define PAR_PSU_IPI_8_BIT_MASK        0x02000000U
#define PAR_PSU_IPI_8_BUFFER_INDEX    4U
#define PAR_PSU_IPI_9_BIT_MASK        0x04000000U
#define PAR_PSU_IPI_9_BUFFER_INDEX    5U
#define PAR_PSU_IPI_10_BIT_MASK       0x08000000U
#define PAR_PSU_IPI_10_BUFFER_INDEX   6U

/*
#define  IPI_ID_APU0 PAR_PSU_IPI_0_DEVICE_ID
#define  IPI_ID_RPU0 PAR_PSU_IPI_1_DEVICE_ID
#define  IPI_ID_RPU1 PAR_PSU_IPI_2_DEVICE_ID
#define  IPI_ID_PMU0 PAR_PSU_IPI_3_DEVICE_ID
#define  IPI_ID_PMU1 PAR_PSU_IPI_4_DEVICE_ID
#define  IPI_ID_PMU2 PAR_PSU_IPI_5_DEVICE_ID
#define  IPI_ID_PMU3 PAR_PSU_IPI_6_DEVICE_ID
#define  IPI_ID_APU1 PAR_PSU_IPI_7_DEVICE_ID
#define  IPI_ID_APU2 PAR_PSU_IPI_8_DEVICE_ID
#define  IPI_ID_APU3 PAR_PSU_IPI_9_DEVICE_ID
#define  IPI_ID_PL0  PAR_PSU_IPI_10_DEVICE_ID
*/

/**************************** Type Definitions *******************************/
/**
 * Data structure used to refer IPI Targets
 */
typedef struct {
    u32 Mask;        /**< Bit Mask for the target */
    u32 BufferIndex; /**< Buffer Index used for calculating buffer address */
} IpiPsu_Target;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u32 DeviceId;    /**< Unique ID  of device */
    u32 BaseAddress; /**< Base address of the device */
    u32 BitMask;     /**< BitMask to be used to identify this CPU */
    u32 BufferIndex; /**< Index of the IPI Message Buffer */
    u32 IntId;       /**< Interrupt ID on GIC **/
    u32 TargetCount; /**< Number of available IPI Targets */
    IpiPsu_Target TargetList[IPIPSU_MAX_TARGETS]; /** < List of IPI Targets */
} IpiPsu_Config;

/**
 * The IpiPsu driver instance data. The user is required to allocate a
 * variable of this type for each IPI device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
    IpiPsu_Config Config; /**< Configuration structure */
    u32 IsReady;          /**< Device is initialized and ready */
    u32 Options;          /**< Options set in the device */
} IpiPsu;

/***************** Macros (Inline Functions) Definitions *********************/
/**
 *
 * Read the register specified by the base address and offset
 *
 * @param	BaseAddress is the base address of the IPI instance
 * @param	RegOffset is the offset of the register relative to base
 *
 * @return	Value of the specified register
 * @note
 * C-style signature
 *	u32 IpiPsu_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 *****************************************************************************/

#define IpiPsu_ReadReg(BaseAddress, RegOffset) \
    FMSH_ReadReg((BaseAddress), (RegOffset))

/****************************************************************************/
/**
 *
 * Write a value into a register specified by base address and offset
 *
 * @param BaseAddress is the base address of the IPI instance
 * @param RegOffset is the offset of the register relative to base
 * @param Data is a 32-bit value that is to be written into the specified
 *register
 *
 * @note
 * C-style signature
 *	void IpiPsu_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 *****************************************************************************/

#define IpiPsu_WriteReg(BaseAddress, RegOffset, Data) \
    FMSH_WriteReg((BaseAddress), (RegOffset), (Data))

/****************************************************************************/
/**
 *
 * Enable interrupts specified in <i>Mask</i>. The corresponding interrupt for
 * each bit set to 1 in <i>Mask</i>, will be enabled.
 *
 * @param	InstancePtr is a pointer to the instance to be worked on.
 * @param	Mask contains a bit mask of interrupts to enable. The mask can
 *			be formed using a set of bitwise or'd values of individual CPU masks
 *
 * @note
 * C-style signature
 *	void IpiPsu_InterruptEnable(IpiPsu *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define IpiPsu_InterruptEnable(InstancePtr, Mask)                         \
    IpiPsu_WriteReg((InstancePtr)->Config.BaseAddress, IPIPSU_IER_OFFSET, \
                    ((Mask) & IPIPSU_ALL_MASK));

/****************************************************************************/
/**
 *
 * Disable interrupts specified in <i>Mask</i>. The corresponding interrupt for
 * each bit set to 1 in <i>Mask</i>, will be disabled.
 *
 * @param	InstancePtr is a pointer to the instance to be worked on.
 * @param	Mask contains a bit mask of interrupts to disable. The mask can
 *			be formed using a set of bitwise or'd values of individual CPU masks
 *
 * @note
 * C-style signature
 *	void IpiPsu_InterruptDisable(IpiPsu *InstancePtr, u32 Mask)
 *
 *****************************************************************************/
#define IpiPsu_InterruptDisable(InstancePtr, Mask)                        \
    IpiPsu_WriteReg((InstancePtr)->Config.BaseAddress, IPIPSU_IDR_OFFSET, \
                    ((Mask) & IPIPSU_ALL_MASK));
/****************************************************************************/
/**
 *
 * Get the <i>STATUS REGISTER</i> of the current IPI instance.
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @return Returns the Interrupt Status register(ISR) contents
 * @note User needs to parse this 32-bit value to check the source CPU
 * C-style signature
 *	u32 IpiPsu_GetInterruptStatus(IpiPsu *InstancePtr)
 *
 *****************************************************************************/
#define IpiPsu_GetInterruptStatus(InstancePtr) \
    IpiPsu_ReadReg((InstancePtr)->Config.BaseAddress, IPIPSU_ISR_OFFSET)
/****************************************************************************/
/**
 *
 * Clear the <i>STATUS REGISTER</i> of the current IPI instance.
 * The corresponding interrupt status for
 * each bit set to 1 in <i>Mask</i>, will be cleared
 *
 * @param InstancePtr is a pointer to the instance to be worked on.
 * @param Mask corresponding to the source CPU*
 *
 * @note This function should be used after handling the IPI.
 * Clearing the status will automatically clear the corresponding bit in
 * OBSERVATION register of Source CPU
 * C-style signature
 *	void IpiPsu_ClearInterruptStatus(IpiPsu *InstancePtr, u32 Mask)
 *
 *****************************************************************************/

#define IpiPsu_ClearInterruptStatus(InstancePtr, Mask)                    \
    IpiPsu_WriteReg((InstancePtr)->Config.BaseAddress, IPIPSU_ISR_OFFSET, \
                    ((Mask) & IPIPSU_ALL_MASK));
/****************************************************************************/
/**
 *
 * Get the <i>OBSERVATION REGISTER</i> of the current IPI instance.
 *
 * @param	InstancePtr is a pointer to the instance to be worked on.
 * @return	Returns the Observation register(OBS) contents
 * @note		User needs to parse this 32-bit value to check the status of
 *			individual CPUs
 * C-style signature
 *	u32 IpiPsu_GetObsStatus(IpiPsu *InstancePtr)
 *
 *****************************************************************************/
#define IpiPsu_GetObsStatus(InstancePtr) \
    IpiPsu_ReadReg((InstancePtr)->Config.BaseAddress, IPIPSU_OBS_OFFSET)
/****************************************************************************/
/************************** Function Prototypes *****************************/

/* Static lookup function implemented in xipipsu_sinit.c */

IpiPsu_Config *IpiPsu_LookupConfig(u32 DeviceId);

/* Interface Functions implemented in xipipsu.c */

s32 IpiPsu_CfgInitialize(IpiPsu *InstancePtr, IpiPsu_Config *CfgPtr,
                         UINTPTR EffectiveAddress);

void IpiPsu_Reset(IpiPsu *InstancePtr);

s32 IpiPsu_TriggerIpi(IpiPsu *InstancePtr, u32 DestCpuMask);

s32 IpiPsu_PollForAck(IpiPsu *InstancePtr, u32 DestCpuMask, u32 TimeOutCount);

s32 IpiPsu_ReadMessage(IpiPsu *InstancePtr, u32 SrcCpuMask, u32 *MsgPtr,
                       u32 MsgLength, u8 BufferType);

s32 IpiPsu_WriteMessage(IpiPsu *InstancePtr, u32 DestCpuMask, u32 *MsgPtr,
                        u32 MsgLength, u8 BufferType);
u32 *IpiPsu_GetBufferAddress(IpiPsu *InstancePtr, u32 SrcCpuMask,
                             u32 DestCpuMask, u32 BufferType);

u32 IpiPsu_GetBufferIndex(const IpiPsu *InstancePtr, u32 CpuMask);
void IpiPsu_SetConfigTable(u32 DeviceId, IpiPsu_Config *ConfigTblPtr);

#ifdef __cplusplus
}
#endif

#endif /* IPIPSU_H_ */
/** @} */
