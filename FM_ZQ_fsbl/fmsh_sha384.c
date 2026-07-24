/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_sha256.c
 *
 * This file contains boot_main.h.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  08/28/2022  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "boot_main.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/*******************************************************************************
 *
 * This function is used to reset SHA.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/

u32 SPAcc_FIFO_CLEAR (void)
{
    u32 bit_flag = (u32)1U << 31;
    u32 FIFO_STAT_reg = Fmsh_In32(SPAcc_FIFO_STAT_ADDR);
    u32 time_out_count = 0U;
    /* pop SPAcc_STAT */

    while (!(FIFO_STAT_reg & bit_flag))
    {
        Fmsh_Out32(SPAcc_STAT_POP_ADDR, 0x00000001);
        FIFO_STAT_reg = Fmsh_In32(SPAcc_FIFO_STAT_ADDR);
        delay_1us();
        time_out_count++;
        if (time_out_count > SHA3_POLL_TIMEOUT_MICROSECONDS)
        {
            return FMSH_FAILURE;
        }
    }
    return FMSH_SUCCESS;
}

/*******************************************************************************
 *
 * This function is used to calculate SHA.
 *
 * @param
 *
 * @return
 *
 *******************************************************************************/
u32 FmshFsbl_sha384 (u8* Message, u32 MessageByteLen, u8* Digest)
{
    u32 Status = FMSH_SUCCESS;
    u32 bit_flag = (u32)1U << 31;
    u32 FIFO_STAT_reg = 0U;
    u32 STATUS_reg = 0U;
    u32 time_out_count = 0U;
    //   u8 SW_ID_reg;
    u8 RET_CODE_reg = 0U;
    Status = SPAcc_FIFO_CLEAR();
    Fmsh_Out32(SPAcc_IRQ_EN_ADDR, 0x0);
    FIFO_STAT_reg = Fmsh_In32(SPAcc_FIFO_STAT_ADDR);

    Fmsh_Out32(SPAcc_KEY_SZ_ADDR, 0x00000000);

    Fmsh_Out32(SPAcc_SRC_PTR_ADDR, (uintptr_t)Message);
    Fmsh_Out32(SPAcc_DST_PTR_ADDR, (uintptr_t)Digest);

    Fmsh_Out32(SPAcc_OFFSET_ADDR, 0x0);

    Fmsh_Out32(SPAcc_PROC_LEN_ADDR, MessageByteLen);

    Fmsh_Out32(SPAcc_ICV_LEN_ADDR, 0x00000030);
    Fmsh_Out32(SPAcc_ICV_OFFSET_ADDR, 0x0);

    Fmsh_Out32(SPAcc_IV_OFFSET_ADDR, 0x0);

    Fmsh_Out32(SPAcc_SW_CTRL_ADDR, 0x0);

    Fmsh_Out32(SPAcc_AUX_INFO_ADDR, 0x0);

    Fmsh_Out32(SPAcc_CTRL_ADDR, 0x0100C090);

    while (FIFO_STAT_reg & bit_flag)
    {
        FIFO_STAT_reg = Fmsh_In32(SPAcc_FIFO_STAT_ADDR);
        delay_1us();
        time_out_count++;
        if (time_out_count > SHA3_POLL_TIMEOUT_MICROSECONDS)
        {
            return FMSH_FAILURE;
        }
    }

    Fmsh_Out32(SPAcc_STAT_POP_ADDR, 0x00000001);

    STATUS_reg = Fmsh_In32(SPAcc_STATUS_ADDR);
    // SW_ID_reg = STATUS_reg & 0xff;
    RET_CODE_reg = (STATUS_reg >> 24) & 0x07;

    return RET_CODE_reg | Status;
}
