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

/****************************************************************************/
/**
*
* @file fmsh_ipi_buf.c
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	Changes
* ----- ------ -------- ----------------------------------------------

*****************************************************************************/

/***************************** Include Files ********************************/
#include "fmsh_ipi.h"
#include "fmsh_ipi_hw.h"
#include "fmsh_ipi_buf.h"

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
 * @brief	Get the Buffer Index for a CPU specified by Mask
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	CpuMask is the Mask of the CPU form which Index is required
 *
 * @return	Buffer Index value if CPU Mask is valid
 * 			IPIPSU_MAX_BUFF_INDEX+1 if not valid
 *
 */
u32 IpiPsu_GetBufferIndex (const IpiPsu *InstancePtr, u32 CpuMask)
{
    u32 BufferIndex;
    u32 Index;
    /* Init Index with an invalid value */
    BufferIndex = IPIPSU_MAX_BUFF_INDEX + 1U;

    /*Search for CPU in the List */
    for (Index = 0U; Index < InstancePtr->Config.TargetCount; Index++)
    {
        /*If we find the CPU , then set the Index and break the loop*/
        if (InstancePtr->Config.TargetList[Index].Mask == CpuMask)
        {
            BufferIndex = InstancePtr->Config.TargetList[Index].BufferIndex;
            break;
        }
    }

    /* Return the Index */
    return BufferIndex;
}
/**
 * @brief	Get the Buffer Address for a given pair of CPUs
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	SrcCpuMask is the Mask for Source CPU
 * @param	DestCpuMask is the Mask for Destination CPU
 * @param	BufferType is either IPIPSU_BUF_TYPE_MSG or IPIPSU_BUF_TYPE_RESP
 *
 * @return	Valid Buffer Address if no error
 * 			NULL if an error occurred in calculating Address
 *
 */

u32 *IpiPsu_GetBufferAddress (IpiPsu *InstancePtr, u32 SrcCpuMask,
                              u32 DestCpuMask, u32 BufferType)
{
#ifdef __aarch64__
    u64 BufferAddr;
#else
    u32 BufferAddr;
#endif
    u32 SrcIndex;
    u32 DestIndex;
    /* Get the buffer indices */
    SrcIndex = IpiPsu_GetBufferIndex(InstancePtr, SrcCpuMask);
    DestIndex = IpiPsu_GetBufferIndex(InstancePtr, DestCpuMask);

    /* If we got an invalid buffer index, then return NULL pointer, else valid
     * address */
    if ((SrcIndex > IPIPSU_MAX_BUFF_INDEX) ||
        (DestIndex > IPIPSU_MAX_BUFF_INDEX))
    {
        BufferAddr = 0U;
    }
    else
    {
        if (IPIPSU_BUF_TYPE_MSG == BufferType)
        {
            BufferAddr = IPIPSU_MSG_RAM_BASE +
                         (SrcIndex * IPIPSU_BUFFER_OFFSET_GROUP) +
                         (DestIndex * IPIPSU_BUFFER_OFFSET_TARGET);
        }
        else if (IPIPSU_BUF_TYPE_RESP == BufferType)
        {
            BufferAddr = IPIPSU_MSG_RAM_BASE +
                         (DestIndex * IPIPSU_BUFFER_OFFSET_GROUP) +
                         (SrcIndex * IPIPSU_BUFFER_OFFSET_TARGET) +
                         (IPIPSU_BUFFER_OFFSET_RESPONSE);
        }
        else
        {
            BufferAddr = 0U;
        }
    }
    return (u32 *)BufferAddr;
}
/** @} */
