/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_devc.c
 *
 * This file contains all c function code for devc(device configure) usind.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  07/01/2022  First Release
 * 0.02   lq  01/01/2023  Add qspi config pl without ddr.
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>

#include "fmsh_common.h"
#include "fmsh_devc_private.h"
#include "fmsh_devc_public.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/
#define DELAY_FOR_PLCLK_MS 10
#define DUMMY_CLOCK_COUNT  10000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
#ifdef DEVC_READBACK
__attribute__((aligned(32))) static u32 g_devcTmpBuffer[TMP_PL_BUF_LEN] = {0};
#endif

/************************** Function Prototypes ******************************/
void *allocate_aligned_space (size_t size, size_t alignment)
{
    void *original_ptr = malloc(size + alignment - 1);
    if (original_ptr != NULL)
    {
        uintptr_t misalignment = (uintptr_t)original_ptr % alignment;
        uintptr_t adjustment = (misalignment != 0) ? (alignment - misalignment)
                                                   : 0;
        void *aligned_ptr = (void *)((uintptr_t)original_ptr + adjustment);
        *((void **)((uintptr_t)aligned_ptr - sizeof(void *))) = original_ptr;
        return aligned_ptr;
    }
    else
    {
        return NULL;  // Allocation failed
    }
}

void deallocate_aligned_space (void *aligned_ptr)
{
    void *original_ptr = *((void **)((uintptr_t)aligned_ptr - sizeof(void *)));
    free(original_ptr);
    aligned_ptr = NULL;
}

/*****************************************************************************/
/**
 * This function pull down Prog_B, then pull up
 *
 * @param	pDevc is devc handle.
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
u32 FDevcPs_Prog_B (FDevcPs_T *pDevc)
{
    u32 reg = 0U;
    u32 timeout = DEVC_POLL_INIT_TIMEOUT_VAL;
    u32 Status = FMSH_SUCCESS;
    // enable isolation
    FMSH_WriteReg(FPS_PMU_GLOBAL_BASEADDR, 0x318, 1 << 2);
    FMSH_WriteReg(FPS_PMU_GLOBAL_BASEADDR, 0x320, 1 << 2);
    delay_ms(1);
    // Setting PCFG_PROG_B signal to high
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_PROGRAM_B_MASK) != DEVC_CFG_PROGRAM_B_MASK)
    {
        reg |= DEVC_CFG_PROGRAM_B_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    // Setting PCFG_PROG_B signal to low
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_PROGRAM_B_MASK) == DEVC_CFG_PROGRAM_B_MASK)
    {
        reg &= ~DEVC_CFG_PROGRAM_B_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    // Polling the PCAP_INIT sts for Reset
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    while ((reg & DEVC_STATUS_PCFG_INIT_MASK) != 0U)
    {
        reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
        if ((reg & DEVC_STATUS_PCFG_INIT_MASK) == 0U)
        {
            break;
        }
        delay_1ms();
        timeout--;
        if (timeout == 0U)
        {
            Status = FMSH_FAILURE;
            return Status;
        }
    }

    // Setting PCFG_PROG_B signal to high
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_PROGRAM_B_MASK) != DEVC_CFG_PROGRAM_B_MASK)
    {
        reg |= DEVC_CFG_PROGRAM_B_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    // Polling the PCAP_INIT sts for set
    timeout = DEVC_POLL_INIT_TIMEOUT_VAL;
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    while ((reg & DEVC_STATUS_PCFG_INIT_MASK) != DEVC_STATUS_PCFG_INIT_MASK)
    {
        reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
        if ((reg & DEVC_STATUS_PCFG_INIT_MASK) == DEVC_STATUS_PCFG_INIT_MASK)
        {
            break;
        }
        delay_1ms();
        timeout--;
        if (timeout == 0U)
        {
            Status = FMSH_FAILURE;
            return Status;
        }
    }

    return Status;
}

/*****************************************************************************/
/**
 * This function pull high CSI_B
 *
 * @param	pDevc is devc handle.
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_CSI_B_HIGH (FDevcPs_T *pDevc)
{
    u32 reg = 0U;

    // Setting PCFG_CSI_B signal to HIGH
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_CSI_B_MASK) != DEVC_CFG_CSI_B_MASK)
    {
        reg |= DEVC_CFG_CSI_B_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function pull high RDWR_B
 *
 * @param	pDevc is devc handle.
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_RDWR_B_HIGH (FDevcPs_T *pDevc)
{
    u32 reg = 0U;

    // Setting PCFG_RDWR_B signal to HIGH
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_RDWR_B_MASK) != DEVC_CFG_RDWR_B_MASK)
    {
        reg |= DEVC_CFG_RDWR_B_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function pull low CSI_B
 *
 * @param	pDevc is devc handle.
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_CSI_B_LOW (FDevcPs_T *pDevc)
{
    u32 reg = 0U;

    // Setting PCFG_CSI_B signal to LOW
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_CSI_B_MASK) == DEVC_CFG_CSI_B_MASK)
    {
        reg &= ~DEVC_CFG_CSI_B_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function pull low RDWR_B
 *
 * @param	pDevc is devc handle.
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_RDWR_B_LOW (FDevcPs_T *pDevc)
{
    u32 reg = 0U;

    // Setting PCFG_RDWR_B signal to LOW
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_RDWR_B_MASK) == DEVC_CFG_RDWR_B_MASK)
    {
        reg &= ~DEVC_CFG_RDWR_B_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets secure download data byteswap
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *              none_swap = 0x0,
 *              half_word_swap = 0x1,
 *              byte_swap = 0x2,
 *              bit_swap = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_secDownDataByteSwap (FDevcPs_T *pDevc, enum data_swap mode)
{
    u32 reg = 0U;
    u32 swap_reg = ((u32)mode) << DEVC_CFG_SEC_DOWN_DATA_BYTE_SWAP_SHIFT;
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_SEC_DOWN_DATA_BYTE_SWAP_MASK) != swap_reg)
    {
        reg &= ~DEVC_CFG_SEC_DOWN_DATA_BYTE_SWAP_MASK;
        reg |= swap_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets tx data byte swap.
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *       none_swap = 0x0,
 *       half_word_swap = 0x1,
 *       byte_swap = 0x2,
 *       bit_swap = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_txDataSwap (FDevcPs_T *pDevc, enum data_swap mode)
{
    u32 reg = 0U;
    u32 swap_reg = ((u32)mode) << DEVC_CFG_TXFIFO_DATA_SWAP_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_TXFIFO_DATA_SWAP_MASK) != swap_reg)
    {
        reg &= ~DEVC_CFG_TXFIFO_DATA_SWAP_MASK;
        reg |= swap_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets bitstream download mode.
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *       DOWNLOAD_BITSTREAM = 0x0,
 *       READBACK_BITSTREAM = 0x2,
 *       DATA_LOOPBACK = 0x3,
 *       SECURE_DOWNLOAD_BITSTREAM = 0x08
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_downloadMode (FDevcPs_T *pDevc, enum download_mode mode)
{
    u32 reg = 0U;
    u32 download_reg = ((u32)mode) << DEVC_CFG_DMA_SWITCH_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_DMA_SWITCH_MASK) != download_reg)
    {
        reg &= ~DEVC_CFG_DMA_SWITCH_MASK;
        reg |= download_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}
/*****************************************************************************/
/**
 * This function sets rx data byte swap.
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *       none_swap = 0x0,
 *       half_word_swap = 0x1,
 *       byte_swap = 0x2,
 *       bit_swap = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_rxDataSwap (FDevcPs_T *pDevc, enum data_swap mode)
{
    u32 reg = 0U;
    u32 swap_reg = ((u32)mode) << DEVC_CFG_RXFIFO_DATA_SWAP_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_RXFIFO_DATA_SWAP_MASK) != swap_reg)
    {
        reg &= ~DEVC_CFG_RXFIFO_DATA_SWAP_MASK;
        reg |= swap_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets swap32 data byte swap.
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *        smap32_swap_disable = 0x0,
 *        smap32_swap_enable = 0x1
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_smap32Swap (FDevcPs_T *pDevc, enum smap32_swap mode)
{
    u32 reg = 0U;
    u32 swap_reg = ((u32)mode) << DEVC_CFG_SMAP32_SWAP_CTRL_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_SMAP32_SWAP_CTRL_MASK) != swap_reg)
    {
        reg &= ~DEVC_CFG_SMAP32_SWAP_CTRL_MASK;
        reg |= swap_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets readback dummmy counter, that is where data is sampled
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *        smap32_swap_disable = 0x0,
 *        smap32_swap_enable = 0x1
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_readbackDummyCount (FDevcPs_T *pDevc, enum dummy_num mode)
{
    u32 reg = 0U;
    u32 dummy_num_reg = ((u32)mode) << DEVC_CFG_READBACK_DUMMY_NUM_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_READBACK_DUMMY_NUM_MASK) != dummy_num_reg)
    {
        reg &= ~DEVC_CFG_READBACK_DUMMY_NUM_MASK;
        reg |= dummy_num_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets read FIFO threshold
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *       readFifoThre_hex_0x40 = 0x0,//default
 *       readFifoThre_hex_0x80 = 0x1,
 *       readFifoThre_hex_0xc0 = 0x2,
 *       readFifoThre_hex_0x100 = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_readFifoThre (FDevcPs_T *pDevc, enum readFifoThre mode)
{
    u32 reg = 0U;
    u32 rfifo_th_reg = ((u32)mode) << DEVC_CFG_RFIFO_TH_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_RFIFO_TH_MASK) != rfifo_th_reg)
    {
        reg &= ~DEVC_CFG_RFIFO_TH_MASK;
        reg |= rfifo_th_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }
    return;
}

/*****************************************************************************/
/**
 * This function sets write FIFO threshold
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *       writeFifoThre_hex_0x80 = 0x0,//default
 *       writeFifoThre_hex_0x60 = 0x1,
 *       writeFifoThre_hex_0x40 = 0x2,
 *       writeFifoThre_hex_0x10 = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_writeFifoThre (FDevcPs_T *pDevc, enum writeFifoThre mode)
{
    u32 reg = 0U;
    u32 wfifo_th_reg = ((u32)mode) << DEVC_CFG_RFIFO_TH_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_WFIFO_TH_MASK) != wfifo_th_reg)
    {
        reg &= ~DEVC_CFG_WFIFO_TH_MASK;
        reg |= wfifo_th_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets read clock edge
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *       rising_edge = 0x1,
 *       failing_edge = 0x0
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_rclk_edge (FDevcPs_T *pDevc, enum clk_edge mode)
{
    u32 reg = 0U;
    u32 rclk_edge_reg = ((u32)mode) << DEVC_CFG_RCLK_EDGE_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_RCLK_EDGE_MASK) != rclk_edge_reg)
    {
        reg &= ~DEVC_CFG_RCLK_EDGE_MASK;
        reg |= rclk_edge_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function sets write clock edge
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *       rising_edge = 0x1,
 *       failing_edge = 0x0
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_wclk_edge (FDevcPs_T *pDevc, enum clk_edge mode)
{
    u32 reg = 0U;

    u32 wclk_edge_reg = ((u32)mode) << DEVC_CFG_WCLK_EDGE_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    if ((reg & DEVC_CFG_WCLK_EDGE_MASK) != wclk_edge_reg)
    {
        reg &= ~DEVC_CFG_WCLK_EDGE_MASK;
        reg |= wclk_edge_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function disable AES
 *
 * @param	pDevc is devc handle.
 * @param	None
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_disableGcm (FDevcPs_T *pDevc)
{
    u32 reg = 0U;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET);
    if ((reg & DEVC_GCM_EN_MASK) == DEVC_GCM_EN_MASK)
    {
        reg &= ~DEVC_GCM_EN_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function enable AES
 *
 * @param	pDevc is devc handle.
 * @param	None
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_enableGcm (FDevcPs_T *pDevc)
{
    u32 reg = 0U;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET);
    if ((reg & DEVC_GCM_EN_MASK) != DEVC_GCM_EN_MASK)
    {
        reg |= DEVC_GCM_EN_MASK;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function selects algorithm
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *               AES = 0x0,
 *               SM4 = 0x1,
 *               NONE = 0x2
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_setGcmAlg (FDevcPs_T *pDevc, enum ALG mode)
{
    u32 reg = 0U;

    u32 alg_reg = ((u32)mode) << DEVC_GCM_ALG_SEL_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET);
    if ((reg & DEVC_GCM_ALG_SEL_MASK) != alg_reg)
    {
        reg &= ~DEVC_GCM_ALG_SEL_MASK;
        reg |= alg_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function selects encrypt or decrypt
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *               ENCODE = 0x0,
 *               DCODE = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_setGcmMode (FDevcPs_T *pDevc, enum MODE mode)
{
    u32 reg = 0U;

    u32 mode_reg = ((u32)mode) << DEVC_GCM_MODE_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET);
    if ((reg & DEVC_GCM_MODE_MASK) != mode_reg)
    {
        reg &= ~DEVC_GCM_MODE_MASK;
        reg |= mode_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function selects data stream handle mode
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *               ECB = 0x0,
 *               CTR = 0x2,
 *               MULTH = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_setGcmChMode (FDevcPs_T *pDevc, enum CHMOD mode)
{
    u32 reg = 0U;
    u32 mode_reg = ((u32)mode) << DEVC_GCM_CHMODE_SHIFT;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET);
    if ((reg & DEVC_GCM_CHMODE_MASK) != mode_reg)
    {
        reg &= ~DEVC_GCM_CHMODE_MASK;
        reg |= mode_reg;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_GCM_CTRL_OFFSET, reg);
    }
    return;
}

/*****************************************************************************/
/**
 * This function selects KEY source
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *              DEV_KEY = 0x1,
 *              KUP = 0x2,
 *              MULTH_H = 0x3
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_setKeySource (FDevcPs_T *pDevc, enum KEYSRC mode)
{
    u32 reg = 0U;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_KEY_SRC_OFFSET);
    if ((reg & DEVC_KEY_SRC_MASK) != mode)
    {
        reg &= ~DEVC_KEY_SRC_MASK;
        reg |= mode;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_KEY_SRC_OFFSET, reg);
    }

    return;
}

/*****************************************************************************/
/**
 * This function loads AES's KEY & IV
 *
 * @param	pDevc is devc handle.
 * @param	None
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_loadKeyIV (FDevcPs_T *pDevc)
{
    FMSH_ASSERT(pDevc != NULL);

    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_KEY_IV_LOAD_OFFSET,
                  DEVC_KEY_IV_LOAD);

    return;
}

/*****************************************************************************/
/**
 * This function sets DECFLAG
 *
 * @param	pDevc is devc handle.
 * @param	mode
 *               use_opkey = 0xe,
 *               no_opkey = 0xa ,
 *               ivup_kup_wr_en = 0x3 ,
 *               clear = 0x0
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_setDecFlag (FDevcPs_T *pDevc, enum DECFLAG mode)
{
    u32 reg = 0U;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_DEC_FLAG_OFFSET);
    if ((reg & DEVC_DEC_FLAG_MASK) != mode)
    {
        reg &= ~DEVC_DEC_FLAG_MASK;
        reg |= mode;
        FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DEC_FLAG_OFFSET, reg);
    }
    return;
}

/******************************common data swap*****************************/

/*****************************************************************************/
/**
 * This function byte swap data
 *
 * @param	srcPtr is pointer of data buffer
 *               len is length to be swap
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void devc_byte_swap (u32 *srcPtr, u32 len)
{
    u32 i;
    u32 tmp;

    u32 byte_higher, byte_high, byte_low, byte_lower;

    for (i = 0; i < len; i++)
    {
        tmp = *(srcPtr + i);
        byte_higher = tmp & 0x000000ff;
        byte_high = tmp & 0x0000ff00;
        byte_low = tmp & 0x00ff0000;
        byte_lower = tmp & 0xff000000;

        *(srcPtr + i) = (byte_higher << 24) | ((byte_high >> 8) << 16) |
                        ((byte_low >> 16) << 8) | ((byte_lower >> 24) << 0);
    }
    return;
}

/*****************************************************************************/
/**
 * This function byte swap data, swap data save in destination address
 *
 * @param	srcPtr is pointer of data buffer
 *               len is length to be swap
 *               desPtr is pointer of data buffer saveing data after swap
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void devc_byte_swap_todes (u32 *srcPtr, u32 len, u32 *desPtr)
{
    u32 i;
    u32 tmp;

    u32 byte_higher, byte_high, byte_low, byte_lower;

    for (i = 0; i < len; i++)
    {
        tmp = *(srcPtr + i);
        byte_higher = tmp & 0x000000ff;
        byte_high = tmp & 0x0000ff00;
        byte_low = tmp & 0x00ff0000;
        byte_lower = tmp & 0xff000000;

        *(desPtr + i) = (byte_higher << 24) | ((byte_high >> 8) << 16) |
                        ((byte_low >> 16) << 8) | ((byte_lower >> 24) << 0);
    }
    return;
}

/******************************bitstream configure*****************************/

/****************************************************************************/
/**
 *
 * This function clears the specified interrupts in the Interrupt Status
 * Register.
 *
 * @param	pDevc is devc handle.
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 *****************************************************************************/
u32 FDevcPs_clearPcapStatus (FDevcPs_T *pDevc)
{
    u32 reg = 0U;

    // Clear it all, so if Boot ROM comes back, it can proceed
    // write 0xffffffff to INT_STS
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET,
                  DEVC_INT_STS_MASK);

    // Get PCAP Interrupt Status Register
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    if ((reg & DEVC_ERROR_FLAGS_MASK) != 0)
    {
        return FMSH_FAILURE;
    }

    // Read the PCAP sts register for DMA sts
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    // If the queue is full(busy), return 1
    if ((reg & DEVC_STATUS_DMA_BUSY_MASK) == DEVC_STATUS_DMA_BUSY_MASK)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/******************************************************************************/
/**
 *
 * This function initiates the DMA transfer.
 *
 * @param	pDevc is devc handle.
 * @param	InstancePtr is a pointer to the XDcfg instance.
 * @param	SourceAddr contains a pointer to the source memory where the data
 *		is to be transferred from.
 * @param	SrcWordLength is the number of words (32 bit) to be transferred
 *		for the source transfer.
 * @param	DestAddr contains a pointer to the destination memory
 *		where the data is to be transferred to.
 * @param	DestWordLength is the number of words (32 bit) to be transferred
 *		for the Destination transfer.
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 ****************************************************************************/
u32 FDevcPs_initiateDma (FDevcPs_T *pDevc, u32 SourceAddr, u32 DestAddr,
                         u32 SrcWordLength, u32 DestWordLength)
{
    u32 Status = FMSH_SUCCESS;

#if DCACHE_ENABLE == 1

    if ((SourceAddr >= 0x00040000) && (SourceAddr < 0X7FFFFFFF))
    {
        Fmsh_DCacheFlushRange(SourceAddr, SrcWordLength * 4);
    }
    else if ((SourceAddr >= 0xFFFC0000U) && (SourceAddr < 0xFFFFFFFFU))
    {
        Fmsh_DCacheFlushRange(SourceAddr, SrcWordLength * 4);
    }
    else{
        ;/* no deal with */
    }
    
    if ((DestAddr >= 0x00040000) && (DestAddr < 0X7FFFFFFF))
    {
        Fmsh_DCacheFlushRange(DestAddr, DestWordLength * 4);
    }
    else if ((DestAddr >= 0xFFFC0000U) && (DestAddr < 0xFFFFFFFFU))
    {
        Fmsh_DCacheFlushRange(DestAddr, DestWordLength * 4);
    }
    else{
        ;/* no deal with */
    }

#endif

    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_SRC_ADDR_OFFSET,
                  SourceAddr);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_DEST_ADDR_OFFSET,
                  DestAddr);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_SRC_LEN_OFFSET,
                  SrcWordLength);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_DEST_LEN_OFFSET,
                  DestWordLength);

    // Poll for the DMA done
    // Poll for the D_P done
    Status = FDevcPs_pollDPDone(pDevc, DEVC_POLL_DONE_MS);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/******************************************************************************/
/**
 *
 * This function polls DMA done
 *
 * @param	pDevc is devc handle.
 * @param	maxcount is the max poll times
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 ****************************************************************************/
u32 FDevcPs_pollDmaDone (FDevcPs_T *pDevc, u32 MaxCount)
{
    u32 reg = 0;
    u32 timeout = MaxCount;
    u32 Status = FMSH_SUCCESS;

    // Polling DMA DONE
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    while ((reg & DEVC_DMA_DONE_MASK) != DEVC_DMA_DONE_MASK)
    {
        reg = FMSH_ReadReg(pDevc->config.BaseAddress,
                           DEVC_SAC_INT_STATUS_OFFSET);

        if (reg & DEVC_ERROR_FLAGS_MASK)
        {
            Status = FMSH_FAILURE;
            break;
        }

        if ((reg & DEVC_DMA_DONE_MASK) == DEVC_DMA_DONE_MASK)
        {
            break;
        }

        delay_1ms();
        timeout--;
        if (timeout == 0U)
        {
            Status = FMSH_FAILURE;
            break;
        }
    }

    // clear dma done int status.
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    reg |= DEVC_DMA_DONE_MASK;
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET, reg);

    return Status;
}

/******************************************************************************/
/**
 *
 * This function polls D_P done
 *
 * @param	pDevc is devc handle.
 * @param	maxcount is the max poll times
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 ****************************************************************************/
u32 FDevcPs_pollDPDone (FDevcPs_T *pDevc, u32 MaxCount)
{
    u32 reg = 0;
    u32 timeout = MaxCount;
    u32 Status = FMSH_SUCCESS;

    // Polling DMA DONE
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    while ((reg & DEVC_DMA_PCAP_DONE_MASK) != DEVC_DMA_PCAP_DONE_MASK)
    {
        reg = FMSH_ReadReg(pDevc->config.BaseAddress,
                           DEVC_SAC_INT_STATUS_OFFSET);

        if (reg & DEVC_ERROR_FLAGS_MASK)
        {
            Status = FMSH_FAILURE;
            break;
        }

        if ((reg & DEVC_DMA_PCAP_DONE_MASK) == DEVC_DMA_PCAP_DONE_MASK)
        {
            break;
        }

        delay_1ms();
        timeout--;
        if (timeout == 0U)
        {
            Status = FMSH_FAILURE;
            break;
        }
    }

    // clear dma done int status.
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    reg |= DEVC_DMA_PCAP_DONE_MASK;
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET, reg);

    return Status;
}

/****************************************************************************/
/**
 *
 * Generates a Type 1 packet head that reads back the requested Configuration
 * register.
 *
 * @param	Register is the address of the register to be read back.
 * @param	OpCode is the read/write operation code.
 * @param	Size is the size of the word to be read.
 *
 * @return	Type 1 packet head to read the specified register
 *
 * @note		None.
 *
 *****************************************************************************/
u32 FDevcPs_regAddr (u8 Register, u8 OpCode, u8 Size)
{
    return (((XDC_TYPE_1 << XDC_TYPE_SHIFT) | (Register << XDC_REGISTER_SHIFT) |
             (OpCode << XDC_OP_SHIFT)) |
            Size);
}

/****************************************************************************/
/**
 *
 * This function starts the DMA transfer. This function only starts the
 * operation and returns before the operation may be completed.
 * If the interrupt is enabled, an interrupt will be generated when the
 * operation is completed, otherwise it is necessary to poll the Status register
 * to determine when it is completed. It is the responsibility of the caller to
 * determine when the operation is completed by handling the generated interrupt
 * or polling the Status Register.
 *
 * @param	pDevc is devc handle.
 * @param	SourcePtr contains a pointer to the source memory where the data
 *		is to be transferred from.
 * @param	SrcWordLength is the number of words (32 bit) to be transferred
 *		for the source transfer.
 * @param	DestPtr contains a pointer to the destination memory
 *		where the data is to be transferred to.
 * @param	DestWordLength is the number of words (32 bit) to be transferred
 *		for the Destination transfer.
 * @param	TransferType
 *           FMSH_NON_SECURE_PCAP_WRITE		0
 *           FMSH_SECURE_PCAP_WRITE			1
 *           FMSH_PCAP_READBACK			2
 *           FMSH_NON_SECURE_PCAP_WRITE_DUMMMY	4
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 *****************************************************************************/
u32 FDevcPs_transfer (FDevcPs_T *pDevc, uintptr_t SourceDataAddr,
                      u32 SrcWordLength, uintptr_t DestinationDataAddr,
                      u32 DestWordLength, u32 TransferType)
{
    u32 reg = 0;
    u32 Status = FMSH_SUCCESS;
    // check if DMA command queue is full
    // Read the PCAP sts register for DMA sts
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    if ((reg & DEVC_STATUS_DMA_BUSY_MASK) == DEVC_STATUS_DMA_BUSY_MASK)
    {
        return FMSH_FAILURE;
    }

    // Check whether the fabric is in initialized state
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    if ((reg & DEVC_STATUS_PCFG_INIT_MASK) != DEVC_STATUS_PCFG_INIT_MASK)
    {
        return FMSH_FAILURE;
    }

    // We don't need to check PCFG_INIT to be high for non-encrypted loopback
    // transfers
    if ((TransferType == FMSH_SECURE_PCAP_WRITE) ||
        (TransferType == FMSH_NON_SECURE_PCAP_WRITE))
    {
        // Check for valid source pointer and length
        if (SrcWordLength == 0U)
        {
            return FMSH_FAILURE;
        }

        if (TransferType == FMSH_NON_SECURE_PCAP_WRITE)
        {
            FDevcPs_downloadMode(
                pDevc, DOWNLOAD_BITSTREAM);  // set the mode:download bitstream
        }

        if (TransferType == FMSH_SECURE_PCAP_WRITE)
        {
            FDevcPs_downloadMode(
                pDevc, SECURE_DOWNLOAD_BITSTREAM);  // set sec_down mode
        }

        FDevcPs_RDWR_B_LOW(pDevc);
        FDevcPs_CSI_B_LOW(pDevc);

        Status = FDevcPs_initiateDma(pDevc, SourceDataAddr, DestinationDataAddr,
                                     SrcWordLength, DestWordLength);
    }
    else if (TransferType == FMSH_PCAP_READBACK)
    {
        if (DestWordLength == 0U)
        {
            return FMSH_FAILURE;
        }

        // Send READ Frame command to FPGA
        // set the mode:download
        FDevcPs_downloadMode(
            pDevc, DOWNLOAD_BITSTREAM);  // set the mode:download bitstream

        FDevcPs_RDWR_B_LOW(pDevc);
        FDevcPs_CSI_B_LOW(pDevc);

        (void)FDevcPs_initiateDma(pDevc, SourceDataAddr, PCAP_RD_DATA_ADDR,
                            SrcWordLength, 0);

        FDevcPs_CSI_B_HIGH(pDevc);

        FDevcPs_downloadMode(
            pDevc, READBACK_BITSTREAM);  // set the mode:readback bitstream mode

        // Initiate the DMA write command.
        Status = FDevcPs_initiateDma_readback(
            pDevc, PCAP_WR_DATA_ADDR, DestinationDataAddr, 0, DestWordLength);
    }
    else if (TransferType == FMSH_NON_SECURE_PCAP_WRITE_DUMMMY)
    {
        if (SrcWordLength == 0U)
        {
            return FMSH_FAILURE;
        }

        FDevcPs_downloadMode(
            pDevc, DOWNLOAD_BITSTREAM);  // set the mode:download bitstream

        FDevcPs_CSI_B_HIGH(pDevc);
        FDevcPs_RDWR_B_LOW(pDevc);
        FDevcPs_CSI_B_LOW(pDevc);
        Status = FDevcPs_initiateDma(pDevc, SourceDataAddr, DestinationDataAddr,
                                     SrcWordLength, DestWordLength);
        FDevcPs_RDWR_B_HIGH(pDevc);
        FDevcPs_CSI_B_HIGH(pDevc);
    }
    else{
        ;/* no deal with */
    }

    return Status;
}

/******************************************************************************/
/**
 *
 * This function initiates the DMA transfer for readback.
 *
 * @param	pDevc is devc handle.
 * @param	InstancePtr is a pointer to the XDcfg instance.
 * @param	SourceAddr contains a pointer to the source memory where the data
 *		is to be transferred from.
 * @param	SrcWordLength is the number of words (32 bit) to be transferred
 *		for the source transfer.
 * @param	DestAddr contains a pointer to the destination memory
 *		where the data is to be transferred to.
 * @param	DestWordLength is the number of words (32 bit) to be transferred
 *		for the Destination transfer.
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 ****************************************************************************/
u32 FDevcPs_initiateDma_readback (FDevcPs_T *pDevc, u32 SourceAddr,
                                  u32 DestAddr, u32 SrcWordLength,
                                  u32 DestWordLength)
{
    u32 Status = FMSH_SUCCESS;

#if DCACHE_ENABLE == 1

    if ((SourceAddr >= 0x00040000) && (SourceAddr < 0X7FFFFFFF))
    {
        Fmsh_DCacheFlushRange(SourceAddr, SrcWordLength * 4);
    }
    else if ((SourceAddr >= 0xFFFC0000U) && (SourceAddr < 0xFFFFFFFFU))
    {
        Fmsh_DCacheFlushRange(SourceAddr, SrcWordLength * 4);
    }
    else{
        ;/* no deal with */
    }
    
    if ((DestAddr >= 0x00040000) && (DestAddr < 0X7FFFFFFF))
    {
        Fmsh_DCacheFlushRange(DestAddr, DestWordLength * 4);
    }
    else if ((DestAddr >= 0xFFFC0000U) && (DestAddr < 0xFFFFFFFFU))
    {
        Fmsh_DCacheFlushRange(DestAddr, DestWordLength * 4);
    }
    else{
        ;/* no deal with */
    }

#endif

    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_SRC_ADDR_OFFSET,
                  SourceAddr);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_DEST_ADDR_OFFSET,
                  DestAddr);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_SRC_LEN_OFFSET,
                  SrcWordLength);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_DMA_DEST_LEN_OFFSET,
                  DestWordLength);

    FDevcPs_RDWR_B_HIGH(pDevc);
    FDevcPs_CSI_B_LOW(pDevc);

    // Poll for the DMA done
    // Poll for the D_P done
    Status = FDevcPs_pollDPDone(pDevc, DEVC_POLL_DONE_MS);
    if (Status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    return Status;
}

/*****************************************************************************
 * This function unlocks CSU module
 *
 * @param	pDevc is devc handle.
 *
 * @return
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_unLockCSU (FDevcPs_T *pDevc)
{
    FMSH_ASSERT(pDevc != NULL);

    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_UNLOCK_OFFSET, DEVC_UNLOCK);

    return;
}

/******************************************************************************
 *
 * This function initializes devc device
 *
 * @param	dev is devc handle.
 * @param	addr is the base address of CSU.
 *
 * @return
 *		- 0 if successful
 *		- 1 not support
 *
 * @note		None
 *
 ******************************************************************************/
u32 FDevcPs_init (FDevcPs_T *pDevc, FDevcPs_Config *cfg)
{
    u32 reg = 0;
    if (cfg == NULL)
    {
        return FMSH_FAILURE;
    }

    pDevc->config.BaseAddress = cfg->BaseAddress;
    pDevc->config.DeviceId = cfg->DeviceId;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_CFG_OFFSET,
                  reg | DEVC_CFG_PCAP_MODE_EN_MASK);

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_PL_PCAP_CTRL_OFFSET);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_PL_PCAP_CTRL_OFFSET,
                  reg | DEVC_PCAP_PR_MASK);

    return FMSH_SUCCESS;
}

/******************************************************************************
 *
 * This function initializes devc device
 *
 * @param	dev is devc handle.
 * @param	addr is the base address of CSU.
 *
 * @return
 *		- 0 if successful
 *		- 1 not support
 *
 * @note		None
 *
 ******************************************************************************/
u32 FDevcPs_getPlPowerStatus (FDevcPs_T *pDevc)
{
    u32 reg = 0;
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    if ((reg & DEVC_STATUS_PL_POR_MASK) != DEVC_STATUS_PL_POR_MASK)
    {
        return FMSH_FAILURE;
    }
    return FMSH_SUCCESS;
}

/*****************************************************************************/
/**
 * This function initializes Device Configuration module, configure pcap
 *interface work parameters
 *
 * @param	TransferType.
 *		- FMSH_PCAP_READBACK
 *		- FMSH_NON_SECURE_PCAP_WRITE
 *		- FMSH_PCAP_LOOPBACK
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None
 *
 ****************************************************************************/
u32 FDevcPs_fabricInit (FDevcPs_T *pDevc, u32 TransferType)
{
    FDevcPs_unLockCSU(pDevc);

    FDevcPs_smap32Swap(pDevc, smap32_swap_disable);
    FDevcPs_rclk_edge(
        pDevc, falling_edge);  // default failing_edge , rising_edge will failed
    FDevcPs_wclk_edge(pDevc, falling_edge);  // default failing_edge

    FDevcPs_CSI_B_HIGH(pDevc);
    FDevcPs_RDWR_B_HIGH(pDevc);

    if (TransferType == FMSH_PCAP_READBACK)
    {
        FDevcPs_rxDataSwap(pDevc, none_swap);
        FDevcPs_txDataSwap(pDevc, none_swap);

        // default 3 , for FMSH 325t the value must be set 4,
        // for fmsh 325t the value must be set 3 for readback reg, but the value
        // must be set 4 for readback bitstream
        FDevcPs_readbackDummyCount(pDevc, dummy_6);

        FDevcPs_downloadMode(pDevc, DOWNLOAD_BITSTREAM);
        FDevcPs_writeFifoThre(pDevc, writeFifoThre_hex_0x80);
    }
    else if (TransferType == FMSH_NON_SECURE_PCAP_WRITE)
    {
        if (FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET) &
            DEVC_STATUS_PCFG_DONE_MASK)
        {
            if (FDevcPs_Prog_B(pDevc) != FMSH_SUCCESS)
            {
                return FMSH_FAILURE;
            }
        }
        // FDevcPs_rxDataSwap(pDevc, byte_swap);
        FDevcPs_rxDataSwap(pDevc, none_swap);
        FDevcPs_downloadMode(pDevc, DOWNLOAD_BITSTREAM);
        FDevcPs_writeFifoThre(pDevc, writeFifoThre_hex_0x80);
    }
    else if (TransferType == FMSH_PCAP_LOOPBACK)
    {
        FDevcPs_rxDataSwap(pDevc, byte_swap);
        FDevcPs_txDataSwap(pDevc, byte_swap);
        FDevcPs_readbackDummyCount(pDevc, dummy_4);  // default 3
        FDevcPs_downloadMode(pDevc, DATA_LOOPBACK);
    }
    else
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

/*****************************************************************************/
/**
 * This function configures fixed IV
 * this IV value maybe changed by different bitstream
 *
 * @param	pDevc is devc handle.
 * @param	p is pointer to IV buffer.
 * @param	len is length of IV, this should be fix as 4.
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None
 *
 ****************************************************************************/
void FDevcPs_IV (FDevcPs_T *pDevc, u32 *p, u32 len)
{
    FMSH_ASSERT(len == 3);
    __attribute__((aligned(4))) u32 TmpIV[4] = {0};

    devc_byte_swap_todes(p, len, TmpIV);

    // set iv_reg
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_IVUP0_OFFSET, 0x2);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_IVUP3_OFFSET, *(TmpIV));
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_IVUP2_OFFSET, *(TmpIV + 1));
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_IVUP1_OFFSET, *(TmpIV + 2));

    return;
}

/******************************************************************************/
/**
 *
 * This function polls FPGA done
 *
 * @param	pDevc is devc handle.
 * @param	maxcount is the max poll times
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 ****************************************************************************/
u32 FDevcPs_pollFpgaDone (FDevcPs_T *pDevc, u32 maxcount)
{
    u32 reg = 0;
    u32 timeout = maxcount;
    u32 Status = FMSH_SUCCESS;
    u32 i = 0;

    __attribute__((aligned(4))) u32 Dummy[64] = {0};
    for (i = 0; i < sizeof(Dummy) / sizeof(u32); i++)
    {
        Dummy[i++] = DEVC_DUMMY_VALUE;
    }

    for (i = 0; i < DUMMY_CLOCK_COUNT; i++)
    {
        FDevcPs_transfer(pDevc, (uintptr_t)Dummy, sizeof(Dummy) / sizeof(u32),
                         FMSH_DMA_INVALID_ADDRESS, 0,
                         FMSH_NON_SECURE_PCAP_WRITE_DUMMMY);
         reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
         if( (reg & DEVC_STATUS_PCFG_DONE_MASK) == DEVC_STATUS_PCFG_DONE_MASK )
         {
            for (i = 0; i < 5; i++)
            {
              FDevcPs_transfer(pDevc, (uintptr_t)Dummy, sizeof(Dummy) / sizeof(u32),
                         FMSH_DMA_INVALID_ADDRESS, 0,
                         FMSH_NON_SECURE_PCAP_WRITE_DUMMMY);
            }
           break;
         }
    }

    // Polling DMA DONE
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    while ((reg & DEVC_STATUS_PCFG_DONE_MASK) != DEVC_STATUS_PCFG_DONE_MASK)
    {
        reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);

        if (reg & DEVC_ERROR_FLAGS_MASK)
        {
            Status = FMSH_FAILURE;
            return Status;
        }

        if ((reg & DEVC_STATUS_PCFG_DONE_MASK) == DEVC_STATUS_PCFG_DONE_MASK)
        {
            break;
        }

        delay_1ms();
        timeout--;
        if (timeout == 0U)
        {
            Status = FMSH_FAILURE;
            return Status;
        }
    }
    // dis isolation
    //FMSH_WriteReg(FPS_PMU_GLOBAL_BASEADDR, 0x318, 1 << 14);
    //FMSH_WriteReg(FPS_PMU_GLOBAL_BASEADDR, 0x320, 1 << 14);

    return Status;
}

#ifdef DEVC_READBACK
/*****************************************************************************/
/**
 *
 * This function returns the value of the specified configuration register or
 *bitstream.
 *
 * @param	pDevc is devc handle.
 * @param	DestinationDataPtr contains a pointer to the destination memory
 *		    where the data is to be transferred to.
 * @param	DestinationLength is the number of words (32 bit) to be transferred
 *		    for the Destination transfer.
 * @param	addr is the value of the specified configuration
 *			register.
 * @param	ConfigReg  is a constant which represents the configuration
 *			register value to be returned.
 *           if(ConfigReg == 0xaa55) radback bitstream
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note	None.
 *
 ****************************************************************************/
u32 FDevcPs_getConfigdata (FDevcPs_T *pDevc, u32 *DestinationDataPtr,
                           u32 DestinationLength, u32 addr, u32 ConfigReg)
{
    u32 count = 0;
    u32 *ptr = NULL;

    u32 reg = 0;

    for (count = 0; count < 12; count++)
    {
        g_devcTmpBuffer[count] = DEVC_DUMMY_VALUE;
    }

    FDevcPs_transfer(pDevc, (uintptr_t)g_devcTmpBuffer, count,
                     FMSH_DMA_INVALID_ADDRESS, 0,
                     FMSH_NON_SECURE_PCAP_WRITE_DUMMMY);

    /*for fmsh PL the dummy value must be set 3 for readback reg, but the value
     * must be set 4 for readback bitstream*/
    FDevcPs_readbackDummyCount(pDevc, dummy_6);

    // Clear the interrupt sts bits
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    reg |= (DEVC_DMA_DONE_MASK | DEVC_DMA_PCAP_DONE_MASK | DEVC_PCFG_DONE_MASK);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET, reg);

    /* Check if DMA command queue is full */
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    if ((reg & DEVC_STATUS_DMA_BUSY_MASK) == DEVC_STATUS_DMA_BUSY_MASK)
    {
        return FMSH_FAILURE;
    }

    /*
     * Register Readback in non secure mode
     * Create the data to be written to read back the
     * Configuration Registers from PL Region.
     */

    ptr = g_devcTmpBuffer;
    if (ConfigReg == 0xaa55U)
    {
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0x000000BB; /* Bus Width Sync Word */
        *ptr++ = 0x11220044; /* Bus Width Detect */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xAA995566; /* Sync Word */
        *ptr++ = 0x02000000; /* Type 1 NOOP Word 0 */
        //  *ptr++ = 0x30008001;  //Type 1 Write 1 Word to CMD
        //  *ptr++ = 0x0000000B;  //SHUTDOWN Command
        //  *ptr++ = 0x02000000;  //Type 1 NOOP Word 0
        *ptr++ = 0x30008001;  // Type 1 Write 1 Word to CMD
        *ptr++ = 0x00000007;  // RCRC Command
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x30008001;  // Type 1 Write 1 Word to CMD
        *ptr++ = 0x00000004;  // RCFG Command
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x30002001;  // Type 1 Write 1 Word to FAR
        *ptr++ = addr;        // FAR Address = 00000000
        *ptr++ = 0x28006000;  // Type 1 Read 0 Words from FDRO
        *ptr++ = 0x48000000 | DestinationLength;  // Type 2 Read 202 Words from
                                                  // FDRO(for 7K325T)
        *ptr++ = 0x20000000;                      // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;                      // Type 1 31 More NOOPs Word 0
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;

        count = ptr - g_devcTmpBuffer;
    }
    else
    {
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0x000000BB; /* Bus Width Sync Word */
        *ptr++ = 0x11220044; /* Bus Width Detect */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xFFFFFFFF; /* Dummy Word */
        *ptr++ = 0xAA995566; /* Sync Word */
        *ptr++ = 0x20000000; /* Type 1 NOOP Word 0 */
                             // *ptr++ = 0x30008001;
                             // *ptr++ = 0x00000007;
        *ptr++ = FDevcPs_regAddr(addr, DEVC_OPCODE_READ, 0x1);  // addr
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;

        count = ptr - g_devcTmpBuffer;
    }

    // devc_byte_swap(g_devcTmpBuffer, count);
    FDevcPs_transfer(pDevc, (uintptr_t)g_devcTmpBuffer, count,
                     (uintptr_t)DestinationDataPtr, DestinationLength,
                     FMSH_PCAP_READBACK);

    FDevcPs_CSI_B_HIGH(pDevc);

    if (ConfigReg == 0xaa55U)
    {
        ptr = g_devcTmpBuffer;
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x30008001;  // Type 1 Write 1 Word to CMD
        *ptr++ = 0x0000000D;  // START Command
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 Write 1 Word to CMD
        *ptr++ = 0x20000000;  // RCRC Command
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 Write 1 Word to CMD
        *ptr++ = 0x20000000;  // DESYNC Command
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        *ptr++ = 0x20000000;  // Type 1 NOOP Word 0
        count = ptr - g_devcTmpBuffer;
    }
    else
    {
        ptr = g_devcTmpBuffer;
        *ptr++ = 0x20000000;
        *ptr++ = 0x20000000;
        *ptr++ = 0x30008001;  // Type 1 Write 1 Word to CMD
        *ptr++ = 0x0000000D;  // DESYNC Command
        *ptr++ = 0x20000000;  // NOOP
        *ptr++ = 0x20000000;  // NOOP
        *ptr++ = 0x20000000;  // NOOP
        *ptr++ = 0x20000000;  // NOOP
        *ptr++ = 0x20000000;  // NOOP
        *ptr++ = 0x20000000;  // NOOP
        *ptr++ = 0x20000000;  // NOOP
        *ptr++ = 0x20000000;  // NOOP
        count = ptr - g_devcTmpBuffer;
    }
    // devc_byte_swap(g_devcTmpBuffer, count);
    FDevcPs_transfer(pDevc, (uintptr_t)g_devcTmpBuffer, count,
                     (uintptr_t)FMSH_DMA_INVALID_ADDRESS, 0,
                     FMSH_NON_SECURE_PCAP_WRITE);

    FDevcPs_CSI_B_HIGH(pDevc);
    FDevcPs_RDWR_B_HIGH(pDevc);

    return FMSH_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 *
 * This function write one frame data.
 *
 * @param	pDevc is devc handle.
 * @param	far_addr is pl address.
 * @param	wr_data is write data point.
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note	None.
 *
 ****************************************************************************/
u32 FDevcPs_wrFrameData (FDevcPs_T *pDevc, u32 far_addr, u32 *wr_data)
{
    u32 len, i;
    u32 *p;
    u32 reg = 0;
    u32 *src_addr;
   
    src_addr = (u32 *)allocate_aligned_space(2048, 64);
    p = src_addr;

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    reg |= (DEVC_DMA_PCAP_DONE_MASK | DEVC_DMA_DONE_MASK | DEVC_PCFG_DONE_MASK);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET, reg);

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    if ((reg & DEVC_STATUS_DMA_BUSY_MASK) == DEVC_STATUS_DMA_BUSY_MASK)
    {
        return FMSH_FAILURE;
    }

    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0x000000BB; /* Bus Width Sync Word */
    *p++ = 0x11220044; /* Bus Width Detect */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xAA995566; /* Sync Word */
    *p++ = 0x02000000; /* Type 1 NOOP Word 0 */
    *p++ = 0x30022001;
    *p++ = 0x00000000;
    *p++ = 0x30020001;
    *p++ = 0x00000000;
    *p++ = 0x30008001;
    *p++ = 0x00000000;
    *p++ = 0x20000000;
    *p++ = 0x30008001;
    *p++ = 0x00000007;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x30026001;
    *p++ = 0x00000000;
    *p++ = 0x30012001;
    *p++ = 0x02003fe5;
    *p++ = 0x3001c001;
    *p++ = 0x00000000;
    *p++ = 0x30018001;
    //idcode
    *p++ = 0x04A82093;
    *p++ = 0x30008001;
    *p++ = 0x00000009;
    *p++ = 0x20000000;
    *p++ = 0x3000c001;
    *p++ = 0x00000409;
    *p++ = 0x3000a001;
    *p++ = 0x00000509;
    *p++ = 0x3000c001;
    *p++ = 0x00000000;
    *p++ = 0x30030001;
    *p++ = 0x00000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    
    *p++ = 0x30002001;
    *p++ = far_addr;
    *p++ = 0x30008001;
    *p++ = 0x00000001;
    *p++ = 0x20000000;
    *p++ = 0x30004000;
    *p++ = (0x50000000+DEVC_FRAME_WORD_NUM+DEVC_FRAME_WORD_NUM);
    
    for (i = 0; i < DEVC_FRAME_WORD_NUM; i++)
        {
        *p++ =*(wr_data+i); 
            }
    
    for (i = 0; i < DEVC_FRAME_WORD_NUM; i++)
    {
        *p++ = 0;
    }
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x30008001;  // Type 1 Write 1 Word to CMD
    *p++ = 0x0000000D;  // DESYNC Command
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    len = p - src_addr;
    FDevcPs_transfer(pDevc, (UINTPTR)src_addr, len, 0xffffffff, 0,
                     FMSH_NON_SECURE_PCAP_WRITE);
    FDevcPs_CSI_B_HIGH(pDevc);
    FDevcPs_RDWR_B_HIGH(pDevc);
    deallocate_aligned_space(src_addr);
    return 0;
}

/*****************************************************************************/
/**
 *
 * This function write reg data.
 *
 * @param	pDevc is devc handle.
 * @param	addr is reg address.
 * @param	wrdata is write data.
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note	None.
 *
 ****************************************************************************/
u32 FDevcPs_writeReg (FDevcPs_T *pDevc, u32 addr, u32 wrdata)
{
    u32 len;
    u32 *p;
    u32 reg = 0;
    u32 *src_addr;

    src_addr = g_devcTmpBuffer;  // (u32*)allocate_aligned_space(2048,64);
    p = src_addr;
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    reg |= (DEVC_DMA_PCAP_DONE_MASK | DEVC_DMA_DONE_MASK | DEVC_PCFG_DONE_MASK);
    FMSH_WriteReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET, reg);

    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_STATUS_OFFSET);
    if ((reg & DEVC_STATUS_DMA_BUSY_MASK) == DEVC_STATUS_DMA_BUSY_MASK)
    {
        return FMSH_FAILURE;
    }

    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0x000000BB; /* Bus Width Sync Word */
    *p++ = 0x11220044; /* Bus Width Detect */
    *p++ = 0xFFFFFFFF; /* Dummy Word */
    *p++ = 0xAA995566; /* Sync Word */
    *p++ = 0x02000000; 	/* Type 1 NOOP Word 0 */ 
    *p++ = FDevcPs_regAddr(addr, DEVC_OPCODE_WRITE, 0x1);  // addr;
    *p++ = wrdata;
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
    *p++ = 0x20000000;  // NOOP
#if 0
    *p++ = 0x20000000;
    *p++ = 0x20000000;
    *p++ = 0x30008001;   //Type 1 Write 1 Word to CMD
    *p++ = 0x0000000D;   //DESYNC Command
    *p++ = 0x20000000;   //NOOP
    *p++ = 0x20000000;   //NOOP
    *p++ = 0x20000000;   //NOOP
    *p++ = 0x20000000;   //NOOP
    *p++ = 0x20000000;   //NOOP
    *p++ = 0x20000000;   //NOOP
    *p++ = 0x20000000;   //NOOP
    *p++ = 0x20000000;     //NOOP
#endif
    len = p - src_addr;
    FDevcPs_transfer(pDevc, (UINTPTR)src_addr, len, (UINTPTR)0xffffffff, 0,
                     FMSH_NON_SECURE_PCAP_WRITE);
    FDevcPs_CSI_B_HIGH(pDevc);
    FDevcPs_RDWR_B_HIGH(pDevc);
    // deallocate_aligned_space(src_addr);
    return FMSH_SUCCESS;
}

/****************************************************************************/
/**
 *
 * This function starts the DMA transfer. This function only starts the
 * operation and returns before the operation may be completed.
 * If the interrupt is enabled, an interrupt will be generated when the
 * operation is completed, otherwise it is necessary to poll the Status register
 * to determine when it is completed. It is the responsibility of the caller to
 * determine when the operation is completed by handling the generated interrupt
 * or polling the Status Register.
 *
 * @param	pDevc is devc handle.
 * @param	SourceDataPtr contains a pointer to the source memory where the data
 *		is to be transferred from.
 * @param	SourceLength is the number of words (32 bit) to be transferred
 *		for the source transfer.
 * @param	DestinationDataPtr contains a pointer to the destination memory
 *		where the data is to be transferred to.
 * @param	DestinationLength is the number of words (32 bit) to be transferred
 *		for the Destination transfer.
 * @param	SecureTransfer
 *           FMSH_NON_SECURE_PCAP_WRITE		0
 *           FMSH_SECURE_PCAP_WRITE			1
 *           FMSH_PCAP_READBACK			2
 *
 * @return
 *		- 0 if successful
 *		- 1 if unsuccessful
 *
 * @note		None.
 *
 *****************************************************************************/
u32 FDevcPs_pcapLoadPartition (FDevcPs_T *pDevc, u32 SourceDataAddr,
                               u32 DestinationDataAddr, u32 SourceLength,
                               u32 DestinationLength, u32 SecureTransfer)
{
    u32 sts = 0U;
    u32 reg = 0U;

    sts = FDevcPs_clearPcapStatus(pDevc);

    if (FMSH_SUCCESS != sts)
    {
        return FMSH_FAILURE;
    }

    sts = FDevcPs_transfer(pDevc, SourceDataAddr, SourceLength,
                           DestinationDataAddr, DestinationLength,
                           SecureTransfer);

    if (FMSH_SUCCESS != sts)
    {
        return FMSH_FAILURE;
    }

    FDevcPs_CSI_B_HIGH(pDevc);
    FDevcPs_RDWR_B_HIGH(pDevc);

    //Check for errors
    reg = FMSH_ReadReg(pDevc->config.BaseAddress, DEVC_SAC_INT_STATUS_OFFSET);
    if ((reg & DEVC_ERROR_FLAGS_MASK) != 0U)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}
