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

/***************************** Include Files ********************************/

#include "fmsh_common.h"
#include "fmsh_ipi.h"
#include "fmsh_ipi_hw.h"

/************************** Variable Definitions *****************************/
extern IpiPsu_Config IpiPsu_ConfigTable[];

/****************************************************************************/
/**
 * Initialize the Instance pointer based on a given Config Pointer
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	CfgPtr is the device configuration structure containing required
 *		  	hardware build data
 * @param	EffectiveAddress is the base address of the device. If address
 *        	translation is not utilized, this parameter can be passed in using
 *        	CfgPtr->Config.BaseAddress to specify the physical base address.
 * @return	FMSH_SUCCESS if initialization was successful
 * 			FMSH_FAILURE in case of failure
 *
 */

s32 IpiPsu_CfgInitialize (IpiPsu *InstancePtr, IpiPsu_Config *CfgPtr,
                          UINTPTR EffectiveAddress)
{
    u32 Index;
    /* Verify arguments */
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(CfgPtr != NULL);
    /* Set device base address and ID */
    InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
    InstancePtr->Config.BaseAddress = EffectiveAddress;
    InstancePtr->Config.BitMask = CfgPtr->BitMask;
    InstancePtr->Config.IntId = CfgPtr->IntId;

    InstancePtr->Config.TargetCount = CfgPtr->TargetCount;

    for (Index = 0U; Index < CfgPtr->TargetCount; Index++)
    {
        InstancePtr->Config.TargetList[Index].Mask = CfgPtr->TargetList[Index]
                                                         .Mask;
        InstancePtr->Config.TargetList[Index]
            .BufferIndex = CfgPtr->TargetList[Index].BufferIndex;
    }

    /* Mark the component as Ready */
    InstancePtr->IsReady = COMPONENT_IS_READY;
    return (FMSH_SUCCESS);
}

/**
 * @brief	Reset the given IPI register set.
 *        	This function can be called to disable the IPIs from all
 *        	the sources and clear any pending IPIs in status register
 *
 * @param 	InstancePtr is the pointer to current IPI instance
 *
 */

void IpiPsu_Reset (IpiPsu *InstancePtr)
{
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);

    /**************Disable***************/

    IpiPsu_WriteReg(InstancePtr->Config.BaseAddress, IPIPSU_IDR_OFFSET,
                    IPIPSU_ALL_MASK);

    /**************Clear***************/
    IpiPsu_WriteReg(InstancePtr->Config.BaseAddress, IPIPSU_ISR_OFFSET,
                    IPIPSU_ALL_MASK);
}

/**
 * @brief	Trigger an IPI to a Destination CPU
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	ChBaseAddr  is IPI Channel addrress offset
 * @param	DestCpuMask is the Mask of the CPU to which IPI is to be triggered
 *
 *
 * @return	FMSH_SUCCESS if successful
 * 			FMSH_FAILURE if an error occurred
 */

// s32 IpiPsu_TriggerIpi(IpiPsu *InstancePtr,u32 ChBaseAddr,u32 DestCpuMask)
s32 IpiPsu_TriggerIpi (IpiPsu *InstancePtr, u32 DestCpuMask)
{
    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);

    /* Trigger an IPI to the Target */
    /*
    IpiPsu_WriteReg(InstancePtr->Config.BaseAddress,ChBaseAddr +
    IPIPSU_TRIG_OFFSET,   // modify lxl 2023.9.13 DestCpuMask);
            */
    IpiPsu_WriteReg(InstancePtr->Config.BaseAddress, IPIPSU_TRIG_OFFSET,
                    DestCpuMask);

    return FMSH_SUCCESS;
}

/**
 * @brief Poll for an acknowledgement using Observation Register
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	DestCpuMask is the Mask of the destination CPU from which ACK is
 * expected
 * @param	TimeOutCount is the Count after which the routines returns failure
 *
 * @return	FMSH_SUCCESS if successful
 * 			FMSH_FAILURE if a timeout occurred
 */

s32 IpiPsu_PollForAck (IpiPsu *InstancePtr, u32 DestCpuMask, u32 TimeOutCount)
{
    u32 Flag, PollCount;
    s32 Status;

    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);

    PollCount = 0U;
    /* Poll the OBS register until the corresponding DestCpu bit is cleared */
    do
    {
        Flag = (IpiPsu_ReadReg(InstancePtr->Config.BaseAddress,
                               IPIPSU_OBS_OFFSET)) &
               (DestCpuMask);  // modify lxl 2023.9.13
        PollCount++;
        /* Check if the IPI was Acknowledged by the Target or we Timed Out*/
    } while ((0x00000000U != Flag) && (PollCount < TimeOutCount));

    if (PollCount >= TimeOutCount)
    {
        Status = FMSH_FAILURE;
    }
    else
    {
        Status = FMSH_SUCCESS;
    }

    return Status;
}

/**
 * @brief	Read an Incoming Message from a Source
 *
 * @param 	InstancePtr is the pointer to current IPI instance
 * @param 	SrcCpuMask is the Device Mask for the CPU which has sent the message
 * @param 	MsgPtr is the pointer to Buffer to which the read message needs to
 * be stored
 * @param 	MsgLength is the length of the buffer/message
 * @param 	BufferType is the type of buffer (IPIPSU_BUF_TYPE_MSG or
 * IPIPSU_BUF_TYPE_RESP)
 *
 * @return	FMSH_SUCCESS if successful
 * 			FMSH_FAILURE if an error occurred
 */

s32 IpiPsu_ReadMessage (IpiPsu *InstancePtr, u32 SrcCpuMask, u32 *MsgPtr,
                        u32 MsgLength, u8 BufferType)
{
    u32 *BufferPtr;
    u32 Index;
    s32 Status;

    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    FMSH_ASSERT(MsgPtr != NULL);
    FMSH_ASSERT(MsgLength <= IPIPSU_MAX_MSG_LEN);

    BufferPtr = IpiPsu_GetBufferAddress(
        InstancePtr, SrcCpuMask, InstancePtr->Config.BitMask, BufferType);
    if (BufferPtr != NULL)
    {
        /* Copy the IPI Buffer contents into Users's Buffer*/
        for (Index = 0U; Index < MsgLength; Index++)
        {
            MsgPtr[Index] = BufferPtr[Index];
        }
        Status = FMSH_SUCCESS;
    }
    else
    {
        Status = FMSH_FAILURE;
    }

    return Status;
}

/**
 * @brief	Send a Message to Destination
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	DestCpuMask is the Device Mask for the destination CPU
 * @param	MsgPtr is the pointer to Buffer which contains the message to be
 * sent
 * @param	MsgLength is the length of the buffer/message
 * @param	BufferType is the type of buffer (IPIPSU_BUF_TYPE_MSG or
 * IPIPSU_BUF_TYPE_RESP)
 *
 * @return	FMSH_SUCCESS if successful
 * 			FMSH_FAILURE if an error occurred
 */

s32 IpiPsu_WriteMessage (IpiPsu *InstancePtr, u32 DestCpuMask, u32 *MsgPtr,
                         u32 MsgLength, u8 BufferType)
{
    u32 *BufferPtr;
    u32 Index;
    s32 Status;

    FMSH_ASSERT(InstancePtr != NULL);
    FMSH_ASSERT(InstancePtr->IsReady == COMPONENT_IS_READY);
    FMSH_ASSERT(MsgPtr != NULL);
    FMSH_ASSERT(MsgLength <= IPIPSU_MAX_MSG_LEN);

    BufferPtr = IpiPsu_GetBufferAddress(
        InstancePtr, InstancePtr->Config.BitMask, DestCpuMask, BufferType);
    if (BufferPtr != NULL)
    {
        /* Copy the Message to IPI Buffer */
        for (Index = 0U; Index < MsgLength; Index++)
        {
            BufferPtr[Index] = MsgPtr[Index];
        }
        Status = FMSH_SUCCESS;
    }
    else
    {
        Status = FMSH_FAILURE;
    }

    return Status;
}

/*****************************************************************************/
/**
 *
 * Set up the device configuration based on the unique device ID. A table
 * contains the configuration info for each device in the system.
 *
 * @param	DeviceId contains the ID of the device to set up the
 *			configuration for.
 * @param	ConfigTblPtr is the device configuration structure containing
 *required hardware build data
 *
 * @return	A pointer to the device configuration for the specified
 *			device ID. See xipipsu.h for the definition of
 *			IpiPsu_Config.
 *
 * @note		This is for safety use case where in this function has to
 * 			be called before CfgInitialize. So that driver will be
 * 			initialized with the provided configuration. For non-safe
 * 			use cases, this is not needed.
 *
 ******************************************************************************/
void IpiPsu_SetConfigTable (u32 DeviceId, IpiPsu_Config *ConfigTblPtr)
{
    u32 Index;

    FMSH_ASSERT(ConfigTblPtr != NULL);

    for (Index = 0U; Index < 1U; Index++)
    {
        if (IpiPsu_ConfigTable[Index].DeviceId == DeviceId)
        {
            IpiPsu_ConfigTable[Index].BaseAddress = ConfigTblPtr->BaseAddress;
            IpiPsu_ConfigTable[Index].BitMask = ConfigTblPtr->BitMask;
            IpiPsu_ConfigTable[Index].BufferIndex = ConfigTblPtr->BufferIndex;
            IpiPsu_ConfigTable[Index].IntId = ConfigTblPtr->IntId;
        }
    }
}

/** @} */
