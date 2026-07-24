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
/**
 *
 * @file fmsh_xspi.c
 * @addtogroup qspipsu_v1_0
 * @{
 *
 * Contains implements the interface functions of the FQspiPsu driver.
 * See fmsh_xspi.h for a detailed description of the device and driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date        Changes
 * ----- --- --------    -----------------------------------------------
 * 1.00  hzq 2022/12/22  First release
 *
 * </pre>
 *
 ******************************************************************************/
#include <string.h>

#include "fmsh_xspi.h"
#include "fmsh_xspi_hw.h"
#include "fmsh_xspi_nor.h"

static void StubStatusHandler(void* callBackRef, u32 statusEvent,
                              unsigned byteCount);

static struct qspi_sdma qspi_sdma_default = {
    .burst_type = 0,
    .single_type = 0,
    .tx_iface = 0,
    .rx_iface = 0,
    .io = 0,
};

/*****************************************************************************
 * This is a stub for the status callback. The stub is here in case the upper
 * layers forget to set the handler.
 *
 * @param	CallBackRef is a pointer to the upper layer callback reference
 * @param	StatusEvent is the event that just occurred.
 * @param	ByteCount is the number of bytes transferred up until the event
 *		occurred.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
static void StubStatusHandler (void* callBackRef, u32 statusEvent,
                               unsigned byteCount)
{
    (void)callBackRef;
    (void)statusEvent;
    (void)byteCount;
}

/*****************************************************************************/
int FQspiPsu_CfgInitialize (FQspiPsu_T* qspiPtr, FQspiPsu_Config_T* configPtr)
{
    FMSH_ASSERT(qspiPtr != NULL);
    FMSH_ASSERT(configPtr != NULL);

    (void)memset(qspiPtr, 0, sizeof(FQspiPsu_T));
    /* set default value */
    qspiPtr->config.device_id = configPtr->device_id;
    qspiPtr->config.pad_lpbk = configPtr->pad_lpbk;
    qspiPtr->config.base = configPtr->base;
    qspiPtr->config.data_base = configPtr->data_base;
    qspiPtr->config.sclk_hz = configPtr->sclk_hz;
    qspiPtr->config.board_delay = configPtr->board_delay;

    qspiPtr->version = QSPIPSU_DRV_VERSION;

    qspiPtr->status_handler = StubStatusHandler;

    return FMSH_SUCCESS;
}

int FQspiPsu_Reset (FQspiPsu_T* qspiPtr)
{
    u32 value, value1, value2;
    int timeout_us;

    value = FMSH_ReadReg(0xff5e0000, 0x0238);
    if (qspiPtr->config.device_id == 0)
    {
        value1 = value | ((0x1 << 21) | (0x1 << 22));
        value2 = value & ~((0x1 << 21) | (0x1 << 22));
    }
    else
    {
        value1 = value | ((0x1 << 23) | (0x1 << 24));
        value2 = value & ~((0x1 << 23) | (0x1 << 24));
    }

    FMSH_WriteReg(0xff5e0000, 0x0238, value1);
    delay_us(5);
    FMSH_WriteReg(0xff5e0000, 0x0238, value2);
    delay_us(5);

    timeout_us = 5000;
    while (1)
    {
        value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_CTRL_STATUS);
        if ((value & QSPI_INIT_COMP) == QSPI_INIT_COMP)
        {
            break;
        }

        if (timeout_us <= 0)
        {
            return FMSH_ETIME;
        }

        delay_us(1);
        timeout_us -= 1;
    }

    return FMSH_SUCCESS;
}

void FQspiPsu_SetStatusHandler (FQspiPsu_T* qspiPtr, void* callBackRef,
                                FQspiPsu_StatusHandler funcPtr)
{
    FMSH_ASSERT(qspiPtr != NULL);
    FMSH_ASSERT(funcPtr != NULL);

    qspiPtr->status_handler = funcPtr;
    qspiPtr->status_ref = callBackRef;
}

int FQspiPsu_SelfTest (FQspiPsu_T* qspiPtr)
{
    u8 csda_old, cseot_old, cssot_old;
    u8 csda, cseot, cssot;

    FQspiPsu_GetDelays(qspiPtr, &csda_old, &cseot_old, &cssot_old);

    csda = 0x5A;
    cseot = 0xAA;
    cssot = 0x55;

    FQspiPsu_SetDelays(qspiPtr, csda, cseot, cssot);
    FQspiPsu_GetDelays(qspiPtr, &csda, &cseot, &cssot);
    if ((0x5A != csda) || (0xAA != cseot) || (0x55 != cssot))
    {
        return FMSH_FAILURE;
    }

    FQspiPsu_SetDelays(qspiPtr, csda_old, cseot_old, cssot_old);

    return FMSH_SUCCESS;
}

void FQspiPsu_InterruptHandler (void* instancePtr)
{
    FQspiPsu_T* qspiPtr;
    u32 value;

    qspiPtr = (FQspiPsu_T*)instancePtr;

    /* Get & Clear interrupt status */
    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_INTR_STATUS);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_INTR_STATUS, value);
    qspiPtr->status.intr_status |= value;

    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_TRD_COMP_INTR_STATUS);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_TRD_COMP_INTR_STATUS, value);
    qspiPtr->status.trd_comp |= value;

    value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_TRD_ERROR_INTR_STATUS);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_TRD_ERROR_INTR_STATUS, value);
    qspiPtr->status.trd_err |= value;

    qspiPtr->status.sync++;
    qspiPtr->status_handler(qspiPtr, 0, 0);
}

int FQspiPsu_Initialize (FQspiPsu_T* qspiPtr, u16 deviceId)
{
    int ret;
    FQspiPsu_Config_T* cfgPtr;

    FMSH_ASSERT(qspiPtr != NULL);

    cfgPtr = FQspiPsu_LookupConfig(deviceId);
    if (cfgPtr == NULL)
    {
        return FMSH_FAILURE;
    }

    ret = FQspiPsu_CfgInitialize(qspiPtr, cfgPtr);
    if (ret != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_InitHw (FQspiPsu_T* qspiPtr, struct qspi_usercfg* usercfg)
{
    FMSH_ASSERT(qspiPtr != NULL);

    /* reset controller */
    // FQspiPsu_Reset();

    FQspiPsu_Phy_Config(qspiPtr);

    FQspiPsu_SetDma(qspiPtr, 3, 0, 0, 0);

    qspiPtr->sdma = &qspi_sdma_default;

    return FMSH_SUCCESS;
}

/*****************************************************************************
 * This function calculate delay value and config device delay register
 *
 * @param
 *
 * @return
 *		- FMSH_SUCCESS if success.
 *		- FMSH_FAILURE if failure.
 *
 * @note
 *
 ******************************************************************************/
int FQspiPsu_Delay (FQspiPsu_T* qspiPtr, u32 sclkHz, u32 csdans, u32 cseotns,
                    u32 cssotns)
{
    u32 sclk_ns;
    u32 csda, cseot, cssot;

    sclk_ns = (1000000000 + sclkHz - 1) / sclkHz;

    if (csdans < sclk_ns)
    {
        csdans = sclk_ns;
    }

    if (cseotns < sclk_ns)
    {
        cseotns = sclk_ns;
    }

    csda = (csdans + sclk_ns - 1) / sclk_ns;
    cseot = (cseotns + sclk_ns - 1) / sclk_ns;
    cssot = (cssotns + sclk_ns - 1) / sclk_ns;

    FQspiPsu_SetDelays(qspiPtr, csda & 0xff, cseot & 0xff, cssot & 0xff);

    return FMSH_SUCCESS;
}

/**************************** Slave DMA ********************************/
static int wait_sdma_trigg (FQspiPsu_T* qspiPtr, u64* sdma_addr, u32* sdma_size,
                            u32* sdma_trd_info, int timeout_us)
{
    int ret;

    // get sdma_trig
    if ((qspiPtr->usercfg->flags & QSPI_F_INTR_EN) == 0)
    {
        ret = FQspiPsu_WaitSdmaTrigg(qspiPtr, timeout_us);
        if (ret)
        {
            return ret;
        }
    }
    else
    {
        while (1)
        {
            if (qspiPtr->status.intr_status & QSPI_INTR_SDMA_TRIGG)
            {
                ret = 0;
                break;
            }

            if (timeout_us <= 0)
            {
                return FMSH_ETIME;
            }

            delay_us(1);
            timeout_us--;
        }
    }

    if (sdma_size)
    {
        *sdma_size = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_SDMA_SIZE);
    }
    if (sdma_trd_info)
    {
        *sdma_trd_info = FMSH_ReadReg(qspiPtr->config.base,
                                      QSPI_R_SDMA_TRD_INFO);
    }
    if (sdma_addr)
    {
        //*sdma_addr = (u64)FMSH_ReadReg(qspiPtr->config.base,
        // QSPI_R_SDMA_ADDR1) << 32; *sdma_addr |=
        // FMSH_ReadReg(qspiPtr->config.base, QSPI_R_SDMA_ADDR0);
        *sdma_addr = qspiPtr->config.data_base;
    }

    return ret;
}

static int sdma_transfer (FQspiPsu_T* qspiPtr, u64 sdma_addr, u32 sdma_size,
                          u32 sdma_trd_info, void* buf)
{
    u32 sdma_dir;

    sdma_dir = sdma_trd_info & QSPI_SDMA_DIR;

    switch (sdma_dir)
    {
    case QSPI_SDMA_DIR_READ:
        (void)memcpy(buf, (void*)(uintptr_t)sdma_addr, sdma_size);
        break;
    case QSPI_SDMA_DIR_WRITE:
        (void)memcpy((void*)(uintptr_t)sdma_addr, buf, sdma_size);
        break;
    default:
        break;
    }

    return 0;
}

int FQspiPsu_SDMA_Transfer (FQspiPsu_T* qspiPtr, void* buf)
{
    int ret;
    u32 sdma_size;
    u32 sdma_trd_info;
    u64 sdma_addr;

    if (qspiPtr->usercfg->flags & QSPI_F_INTR_EN)
    {
        FQspiPsu_SetIntrMask(qspiPtr, QSPI_INTR_MASK_SDMA, 1);
    }

    ret = wait_sdma_trigg(qspiPtr, &sdma_addr, &sdma_size, &sdma_trd_info,
                          1000);
    if (ret)
    {
        fmsh_print_err("wait SDMA_TRIGG failed, timeout");
        return ret;
    }

    ret = sdma_transfer(qspiPtr, sdma_addr, sdma_size, sdma_trd_info, buf);

    if (qspiPtr->usercfg->flags & QSPI_F_INTR_EN)
    {
        FQspiPsu_SetIntrMask(qspiPtr, 0, 0);
    }

    return ret;
}

/***************************** CDMA work mode *********************************/
static int FQspiPsu_CDMA_Prepare (FQspiPsu_T* qspiPtr)
{
    int ret;

    ret = FQspiPsu_SetWorkMode(qspiPtr, QSPI_WORK_MODE_ACMD);
    if (ret)
    {
        return ret;
    }

    // wait for thread ready
    ret = FQspiPsu_WaitTrdReady(qspiPtr, 10);
    if (ret)
    {
        return ret;
    }

    qspiPtr->status.intr_status = 0;
    FQspiPsu_ClearIntr(qspiPtr, QSPI_INTR_MASK_SDMA);

    return FMSH_SUCCESS;
}

int FQspiPsu_CDMA_Exec (FQspiPsu_T* qspiPtr, struct qspi_acmd_desc* desc,
                        int timeout_us)
{
    int ret;

#if (DCACHE_ENABLE == 1)
    Fmsh_DCacheFlushRange((uintptr_t)desc, sizeof(struct qspi_acmd_desc));
#endif

    ret = FQspiPsu_CDMA_Prepare(qspiPtr);
    if (ret)
    {
        return ret;
    }

    // trigger acmd
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG3,
                  ((unsigned long long)desc >> 32));
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG2, (uintptr_t)desc);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0,
                  (qspiPtr->cur_cs & 0x7) << 24);
    
    if (((desc->cmd_type == QSPI_CT_READ) ||
         (desc->cmd_type == QSPI_CT_PROG)) &&
        ((desc->cmd_flags & QSPI_CF_MDMA) == 0))
    {
        ret = FQspiPsu_SDMA_Transfer(qspiPtr,
                                     (void*)(uintptr_t)(desc->mem_ptr));
        if (ret)
        {
            return ret;
        }
    }
    
    ret = FQspiPsu_WaitCmdComplete(qspiPtr, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_CDMA_SoftReset (FQspiPsu_T* qspiPtr, int timeout_us)
{
    int ret;
    struct qspi_acmd_desc desc = {0};

    // set cmd_type
    desc.cmd_type = QSPI_CT_RESET;

    // set cmd flag
    desc.cmd_flags |= QSPI_CF_INT;
    desc.cmd_flags |= qspiPtr->cur_cs & 0x7;

    // trigger adma exec
    ret = FQspiPsu_CDMA_Exec(qspiPtr, &desc, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_CDMA_EraseSectors (FQspiPsu_T* qspiPtr, u64 offs, u32 cnt,
                                int timeout_ms)
{
    int ret;
    struct qspi_acmd_desc desc = {0};

    // set cmd_type
    desc.cmd_type = QSPI_CT_ERASE;
    // set xspi pointer
    desc.dev_ptr = offs;
    // set cmd flag
    desc.cmd_flags = QSPI_CF_INT;
    desc.cmd_flags |= qspiPtr->cur_cs & 0x7;
    // set cmd counter
    desc.cmd_cnt = cnt - 1;

    // trigger adma exec
    ret = FQspiPsu_CDMA_Exec(qspiPtr, &desc, timeout_ms * 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_CDMA_EraseAll (FQspiPsu_T* qspiPtr, int timeout_ms)
{
    int ret;
    struct qspi_acmd_desc desc = {0};

    // set cmd_type
    desc.cmd_type = QSPI_CT_ERASE_ALL;

    // set cmd flag
    desc.cmd_flags = QSPI_CF_INT;
    desc.cmd_flags |= qspiPtr->cur_cs & 0x7;

    // trigger adma exec
    ret = FQspiPsu_CDMA_Exec(qspiPtr, &desc, timeout_ms * 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_CDMA_Read (FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                        int timeout_us)
{
    int ret;
    struct qspi_acmd_desc desc = {0};

#if (DCACHE_ENABLE == 1)
    if (qspiPtr->usercfg->dma_type & QSPI_MDMA)
    {
        Fmsh_DCacheFlushRange((uintptr_t)memptr, len);
    }
#endif

    // set cmd_type
    desc.cmd_type = QSPI_CT_READ;

    // set mem pointer
    desc.mem_ptr = memptr;

    // set xspi pointer
    desc.dev_ptr = offs;

    // set cmd flags
    desc.cmd_flags = QSPI_CF_INT;
    desc.cmd_flags |= qspiPtr->cur_cs & 0x7;
    if (qspiPtr->usercfg->dma_type & QSPI_MDMA)
    {
        desc.cmd_flags |= QSPI_CF_MDMA;
    }

    // set cmd counter
    desc.cmd_cnt = len - 1;

    // trigger adma exec
    ret = FQspiPsu_CDMA_Exec(qspiPtr, &desc, timeout_us);
    if (ret)
    {
        return ret;
    }

#if (DCACHE_ENABLE == 1)
    if (qspiPtr->usercfg->dma_type & QSPI_MDMA)
    {
        Fmsh_DCacheInvalidateRange((uintptr_t)memptr, len);
    }
#endif
    
    return FMSH_SUCCESS;
}

int FQspiPsu_CDMA_Program (FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                           int timeout_us)
{
    int ret;
    struct qspi_acmd_desc desc = {0};

#if (DCACHE_ENABLE == 1)
    Fmsh_DCacheFlushRange((uintptr_t)memptr, len);
#endif

    // set cmd_type
    desc.cmd_type = QSPI_CT_PROG;

    // set mem pointer
    desc.mem_ptr = memptr;

    // set xspi pointer
    desc.dev_ptr = offs;

    // set cmd flags
    desc.cmd_flags = QSPI_CF_INT;
    desc.cmd_flags |= qspiPtr->cur_cs & 0x7;
    if (qspiPtr->usercfg->dma_type & QSPI_MDMA)
    {
        desc.cmd_flags |= QSPI_CF_MDMA;
    }

    // set cmd counter
    desc.cmd_cnt = len - 1;

    // trigger adma exec
    ret = FQspiPsu_CDMA_Exec(qspiPtr, &desc, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

/***************************** PIO work mode *********************************/
static int FQspiPsu_PIO_Prepare (FQspiPsu_T* qspiPtr)
{
    int ret;

    ret = FQspiPsu_SetWorkMode(qspiPtr, QSPI_WORK_MODE_ACMD);
    if (ret)
    {
        return ret;
    }

    // wait for thread ready
    ret = FQspiPsu_WaitTrdReady(qspiPtr, 10);
    if (ret)
    {
        return ret;
    }

    qspiPtr->status.intr_status = 0;
    FQspiPsu_ClearIntr(qspiPtr, QSPI_INTR_MASK_SDMA);

    return FMSH_SUCCESS;
}

int FQspiPsu_PIO_SoftReset (FQspiPsu_T* qspiPtr, int timeout_us)
{
    int ret;
    u32 cmd0;

    ret = FQspiPsu_PIO_Prepare(qspiPtr);
    if (ret)
    {
        return ret;
    }

    cmd0 = (0x1 << 30) | (0x1 << 18);
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 24;
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 20;
    cmd0 |= QSPI_CT_RESET;

    // trig stig mode execution
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0, cmd0);

    // wait for trd comp
    ret = FQspiPsu_WaitCmdComplete(qspiPtr, timeout_us);

    return ret;
}

int FQspiPsu_PIO_EraseSectors (FQspiPsu_T* qspiPtr, u64 offs, u32 cnt,
                               int timeout_ms)
{
    int ret;
    u32 cmd0;

    ret = FQspiPsu_PIO_Prepare(qspiPtr);
    if (ret)
    {
        return ret;
    }

    cmd0 = (0x1 << 30) | (0x1 << 18);
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 24;
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 20;
    cmd0 |= 0x1000;

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG1, (u32)offs);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG5, (u32)(offs >> 32));
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG4, (u16)cnt - 1);
    // trig stig mode execution
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0, cmd0);

    // wait for trd comp
    ret = FQspiPsu_WaitCmdComplete(qspiPtr, timeout_ms * 1000);

    return ret;
}

int FQspiPsu_PIO_EraseAll (FQspiPsu_T* qspiPtr, int timeout_ms)
{
    int ret;
    u32 cmd0;

    ret = FQspiPsu_PIO_Prepare(qspiPtr);
    if (ret)
    {
        return ret;
    }

    cmd0 = (0x1 << 30) | (0x1 << 18);
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 24;
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 20;
    cmd0 |= 0x1001;

    // trig stig mode execution
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0, cmd0);

    // wait for trd comp
    ret = FQspiPsu_WaitCmdComplete(qspiPtr, timeout_ms * 1000);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

int FQspiPsu_PIO_Read (FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                       int timeout_us)
{
    int ret;
    u32 cmd0;

#if (DCACHE_ENABLE == 1)
    if (qspiPtr->usercfg->dma_type & QSPI_MDMA)
    {
        Fmsh_DCacheFlushRange((uintptr_t)memptr, len);
    }
#endif

    ret = FQspiPsu_PIO_Prepare(qspiPtr);
    if (ret)
    {
        return ret;
    }

    cmd0 = (0x1 << 30) | (0x1 << 18);
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 24;
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 20;
    if (qspiPtr->usercfg->dma_type == QSPI_MDMA)
    {
        cmd0 |= 0x1 << 19;
    }
    cmd0 |= 0x2200;

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG1, (u32)offs);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG5, (u32)(offs >> 32));
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG2, (u32)memptr);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG3, (u32)(memptr >> 32));
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG4, len - 1);
    // trig pio mode execution
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0, cmd0);

    // wait for trd comp
    if (qspiPtr->usercfg->dma_type != QSPI_MDMA)
    {
        ret = FQspiPsu_SDMA_Transfer(qspiPtr, (void*)(uintptr_t)memptr);
        if (ret)
        {
            return ret;
        }
    }
    ret = FQspiPsu_WaitCmdComplete(qspiPtr, timeout_us);
    if (ret)
    {
        return ret;
    }
    
#if (DCACHE_ENABLE == 1)
    if (qspiPtr->usercfg->dma_type & QSPI_MDMA)
    {
        Fmsh_DCacheInvalidateRange((uintptr_t)memptr, len);
    }
#endif
    
    return FMSH_SUCCESS;
}

int FQspiPsu_PIO_Program (FQspiPsu_T* qspiPtr, u64 offs, u32 len, u64 memptr,
                          int timeout_us)
{
    int ret;
    u32 cmd0;

#if (DCACHE_ENABLE == 1)
    Fmsh_DCacheFlushRange((uintptr_t)memptr, len);
#endif

    ret = FQspiPsu_PIO_Prepare(qspiPtr);
    if (ret)
    {
        return ret;
    }

    cmd0 = (0x1 << 30) | (0x1 << 18);
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 24;
    cmd0 |= (qspiPtr->cur_cs & 0x7) << 20;
    if (qspiPtr->usercfg->dma_type == QSPI_MDMA)
    {
        cmd0 |= 0x1 << 19;
    }
    cmd0 |= 0x2100;

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG1, (u32)offs);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG5, (u32)(offs >> 32));
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG2, (u32)memptr);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG3, (u32)(memptr >> 32));
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG4, len - 1);
    // trig pio mode execution
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0, cmd0);

    // wait for trd comp
    if (qspiPtr->usercfg->dma_type != QSPI_MDMA)
    {
        ret = FQspiPsu_SDMA_Transfer(qspiPtr, (void*)(uintptr_t)memptr);
        if (ret)
        {
            return ret;
        }
    }
    ret = FQspiPsu_WaitCmdComplete(qspiPtr, timeout_us);
    if (ret)
    {
        return ret;
    }

    return FMSH_SUCCESS;
}

/***************************** Stig mode *********************************/
static void set_stig_inst_reg (u32* regs, u32 value, int field)
{
    if (field < 32)
    {
        regs[0] |= value << field;
    }
    else if (field < 64)
    {
        regs[1] |= value << (field - 32);
    }
    else if (field < 96)
    {
        regs[2] |= value << (field - 64);
    }
    else
    {
        regs[3] |= value << (field - 96);
    }
}

static void FQspiPsu_Stig_PrepareInst (struct qspi_cmd* cmd,
                                       struct qspi_data* data, u32* seq)
{
    u32* cmd_reg;
    u64 temp;

    if (cmd)
    {
        cmd_reg = seq;

        cmd_reg[0] = 0;
        cmd_reg[1] = 0;
        cmd_reg[2] = 0;
        cmd_reg[3] = 0;

        set_stig_inst_reg(cmd_reg, cmd->inst_type & 0x3f, 0);  // bits 0~6
        set_stig_inst_reg(cmd_reg, cmd->bank & 0x7, 108);      // bits 108~110

        // opcode phase
        set_stig_inst_reg(cmd_reg, cmd->opcode & 0xff, 80);  // bits 80~87
        if (cmd->flags & QSPI_CMD_F_OPCODE_EXT_EN)
        {
            set_stig_inst_reg(cmd_reg, cmd->opcode_ext & 0xff,
                              72);                 // bits 72~79
            set_stig_inst_reg(cmd_reg, 0x1, 100);  // bits 100
        }
        switch (cmd->op_nios)
        {
        case 1:
            temp = 0;
            break;
        case 2:
            temp = 1;
            break;
        case 4:
            temp = 2;
            break;
        case 8:
            temp = 3;
            break;
        default:
            break;
        }
        set_stig_inst_reg(cmd_reg, temp, 104);     // bits 104~105
        if (cmd->flags & QSPI_CMD_F_INST_EDGE_DDR)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 107);  // bits 107
        }

        // address phase
        set_stig_inst_reg(cmd_reg, cmd->naddrs & 0x7, 92);
        if (cmd->naddrs)
        {
            temp = cmd->addr_l;
            set_stig_inst_reg(cmd_reg, temp & 0xff, 24);
            set_stig_inst_reg(cmd_reg, (temp >> 8) & 0xff, 32);
            set_stig_inst_reg(cmd_reg, (temp >> 16) & 0xff, 40);
            set_stig_inst_reg(cmd_reg, (temp >> 24) & 0xff, 48);
            temp = cmd->addr_h;
            set_stig_inst_reg(cmd_reg, temp & 0xff, 56);
            set_stig_inst_reg(cmd_reg, (temp >> 8) & 0xff, 64);
            switch (cmd->addr_nios)
            {
            case 1:
                temp = 0;
                break;
            case 2:
                temp = 1;
                break;
            case 4:
                temp = 2;
                break;
            case 8:
                temp = 3;
                break;
            default:
                break;
            }
            set_stig_inst_reg(cmd_reg, temp, 96);
            if (cmd->flags & QSPI_CMD_F_ADDR_EDGE_DDR)
            {
                set_stig_inst_reg(cmd_reg, 0x1, 99);
            }

            if (cmd->flags & QSPI_CMD_F_ADDR_SHIFT)
            {
                set_stig_inst_reg(cmd_reg, 0x1, 115);
            }
        }
        // data phase
        set_stig_inst_reg(cmd_reg, cmd->ndata & 0x3, 88);
        if (cmd->ndata)
        {
            set_stig_inst_reg(cmd_reg, cmd->data[0] & 0xff, 8);
            if (cmd->ndata > 1)
            {
                set_stig_inst_reg(cmd_reg, cmd->data[1] & 0xff, 16);
            }
        }

        if (cmd->flags & QSPI_CMD_F_XIP_EN)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 116);
        }
        if (cmd->flags & QSPI_CMD_F_CRC_EN)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 117);
            if (cmd->flags & QSPI_CMD_F_CRC_VARIANT)
            {
                set_stig_inst_reg(cmd_reg, 0x1, 118);
            }
        }
        if (cmd->flags & QSPI_CMD_F_LINK)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 124);
        }
        if (cmd->flags & QSPI_CMD_F_TCMS_EN)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 125);
        }
    }

    else if (data)
    {
        cmd_reg = seq;

        cmd_reg[0] = 0;
        cmd_reg[1] = 0;
        cmd_reg[2] = 0;
        cmd_reg[3] = 0;

        set_stig_inst_reg(cmd_reg, QSPI_SEQ_DATA, 0);  // bits 0~6
        if (data->flags & QSPI_DATA_F_CMD_FIFO)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 24);
        }
        if (!(data->flags & QSPI_DATA_F_DATA_IN))
        {
            set_stig_inst_reg(cmd_reg, 0x1, 100);
        }
        set_stig_inst_reg(cmd_reg, data->ndata & 0xffff, 48);
        set_stig_inst_reg(cmd_reg, (data->ndata >> 16) & 0xffff, 64);
        switch (data->nios)
        {
        case 1:
            temp = 0;
            break;
        case 2:
            temp = 1;
            break;
        case 4:
            temp = 2;
            break;
        case 8:
            temp = 3;
            break;
        default:
            break;
        }
        set_stig_inst_reg(cmd_reg, temp, 104);
        if (data->flags & QSPI_DATA_F_DATA_EDGE_DDR)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 107);           // bits 107
        }
        set_stig_inst_reg(cmd_reg, data->bank & 0x7, 108);  // bits 108~110

        set_stig_inst_reg(cmd_reg, data->dummy & 0x3f, 84);

        if (data->flags & QSPI_DATA_F_HF_READ_BOUND_EN)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 83);
        }
        if (data->flags & QSPI_DATA_F_2B_PER_ADDR)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 92);
        }
        if (data->flags & QSPI_DATA_F_DATA_SWAP)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 112);
        }
        if (data->flags & QSPI_DATA_F_2B_PER_ADDR)
        {
            set_stig_inst_reg(cmd_reg, 0x1, 92);
        }
    }
    else{
        ;/* no deal with */
    }
}

int FQspiPsu_Stig_Exec (FQspiPsu_T* qspiPtr, struct qspi_cmd* cmd,
                        struct qspi_data* data, int timeout_us)
{
    int ret;
    u32 seq[4];

    // wait idle and set stig work mode
    ret = FQspiPsu_SetWorkMode(qspiPtr, QSPI_WORK_MODE_STIG);
    if (ret)
    {
        return ret;
    }

    if (qspiPtr->usercfg->flags & QSPI_F_INTR_EN)
    {
        qspiPtr->status.intr_status = 0;
        FQspiPsu_ClearIntr(qspiPtr, QSPI_INTR_MASK_STIG);
        FQspiPsu_SetIntrMask(qspiPtr, QSPI_INTR_MASK_STIG, 1);
    }

    // fill seq
    FQspiPsu_Stig_PrepareInst(cmd, 0, seq);
    // trig stig mode execution
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG1, seq[0]);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG2, seq[1]);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG3, seq[2]);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG4, seq[3]);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0, 0x0);

    if (data)
    {
        // fill seq
        FQspiPsu_Stig_PrepareInst(0, data, seq);
        // trig stig mode execution
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG1, seq[0]);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG2, seq[1]);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG3, seq[2]);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG4, seq[3]);
        FMSH_WriteReg(qspiPtr->config.base, QSPI_R_CMD_REG0, 0x0);

        // transfer data
        if ((data->flags & QSPI_DATA_F_CMD_FIFO) == 0)
        {
            FQspiPsu_SDMA_Transfer(qspiPtr, data->data);
        }
    }

    // get cmd status & wait for cmd done
    ret = FQspiPsu_WaitCmdComplete(qspiPtr, timeout_us);
    FQspiPsu_CheckCmdStatus(qspiPtr, &(cmd->status));

    if (qspiPtr->usercfg->flags & QSPI_F_INTR_EN)
    {
        FQspiPsu_SetIntrMask(qspiPtr, 0, 0);
    }

    return ret;
}
