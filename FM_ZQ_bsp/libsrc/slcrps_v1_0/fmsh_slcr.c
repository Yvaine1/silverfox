/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_slcr.c
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lsq  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fmsh_psu_parameters.h"
#include "fmsh_slcr.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/****************************************************************************/
/**
 *
 *  This function set a bit of the register to 0
 *
 * @param
 *  baseAddr  -- the base address of the register which is to be modified
 *  offSet    -- the base address of the register which is to be modified
 *  bit_num   -- this bit of the register is to be modified
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPs_setBitTo0 (u32 baseAddr, u32 offSet, u32 bit_num)
{
    u32 value = 0;
    // First get the current value of the register
    value = FMSH_ReadReg(baseAddr, offSet);
    // Then write the given bit of data as 0
    value &= ~(1 << bit_num);
    // Finally, write the modified data to the register
    FMSH_WriteReg(baseAddr, offSet, value);
}

/****************************************************************************/
/**
 *
 *  This function set a bit of the register to 1
 *
 * @param
 *  baseAddr  -- the base address of the register which is to be modified
 *  offSet    -- the base address of the register which is to be modified
 *  bit_num   -- this bit of the register is to be modified
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPs_setBitTo1 (u32 baseAddr, u32 offSet, u32 bit_num)
{
    u32 value = 0;
    // First get the current value of the register
    value = FMSH_ReadReg(baseAddr, offSet);
    // Then write the given bit of data as 1
    value |= (1 << bit_num);
    // Finally, write the modified data to the register
    FMSH_WriteReg(baseAddr, offSet, value);
}

/****************************************************************************/
/**
 *
 *  This function write the lock key 0xDF0D767B to protect the slcr registers.
 *
 * @param
 *  Null
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPs_lock (void)
{
    FMSH_WriteReg(FPS_SLCR_BASEADDR, SLCR_LOCK, 0xDF0D767B);  // SLCR LOCK
}

/****************************************************************************/
/**
 *
 *  This function write the unlock key 0xDF0D767B to enable writes to the
 *  slcr registers.
 *
 * @param
 *  Null
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPs_unlock (void)
{
    FMSH_WriteReg(FPS_SLCR_BASEADDR, SLCR_UNLOCK, 0xDF0D767B);  // SLCR UNLOCK
}

/****************************************************************************/
/**
 *
 *  This function enable/unable the pss soft_rst.
 *
 * @param
 *  soft_rst_en -- 0:unable soft_rst;1:enable soft_rst
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPs_softRst (u32 soft_rst_en)
{
    FSlcrPs_unlock();
    FMSH_WriteReg(FPS_SLCR_BASEADDR, PSS_RST_CTRL, soft_rst_en);  // PS soft_rst
    FSlcrPs_lock();
}

/****************************************************************************/
/**
 *
 *  This function set reset of the given IP feature.
 *
 * @param
 *  rst_id   -- the address of reset_ctrl register
 *  rst_mode -- the reset mode of the ip feature
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPs_ipSetRst (u32 rst_id, u32 rst_mode)
{
    FSlcrPs_unlock();
    FSlcrPs_setBitTo1(FPS_SLCR_BASEADDR, rst_id, rst_mode);
    FSlcrPs_lock();
}

/****************************************************************************/
/**
 *
 *  This function release reset of the given IP feature.
 *
 * @param
 *  rst_id   -- the address of reset_ctrl register
 *  rst_mode -- the reset mode of the ip feature
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPs_ipReleaseRst (u32 rst_id, u32 rst_mode)
{
    FSlcrPs_unlock();
    FSlcrPs_setBitTo0(FPS_SLCR_BASEADDR, rst_id, rst_mode);
    FSlcrPs_lock();
}

/****************************************************************************/
/**
 *
 *  This function read the data of the given slcr address and printf it out.
 *
 * @param
 *  Addr  --  the exist address of slcr module
 *
 * @return
 *  0 -- read successful
 *
 * @note
 *  Null
 *
 ****************************************************************************/
u32 FSlcrPS_regRead (u32 addr)
{
    u32 read_data;
    read_data = FMSH_ReadReg(FPS_SLCR_BASEADDR, addr);
    TRACE_OUT(DEBUG_OUT, "Addr = 0x%x, Read_Data = 0x%x!\n", addr, read_data);

    return 0;
}

/****************************************************************************/
/**
 *
 *  This function read all the data of the slcr address and print them out.
 *
 * @param
 *  Null
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPS_rsvRegPrint (void)
{
    int i;
    FSlcrPs_unlock();  // SLCR UNLOCK
    for (i = 0; i < 600; i++)
    {
        FSlcrPS_regRead(0x100 + 4 * i);
    }
}

/****************************************************************************/
/**
 *
 *  This function loop I2C0's outputs to I2C1's inputs,and I2C1's outputs to
 *  I2C0's inputs
 *
 * @param
 *  loop_en -- 0:connect I2C inputs according to MIO mapping;1:set the loop
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPS_setI2cLoop (u32 loop_en)
{
    FSlcrPs_unlock();  // SLCR UNLOCK
    if (loop_en == 0)
    {
        FSlcrPs_setBitTo0(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 3);
    }
    else if (loop_en == 1)
    {
        FSlcrPs_setBitTo1(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 3);
    }
    FSlcrPs_lock();  // SLCR LOCK
}

/****************************************************************************/
/**
 *
 *  This function loop CAN0's Tx to CAN1's Rx,and CAN1's Tx to CAN0's Rx.
 *
 * @param
 *  loop_en -- 0:connect CAN inputs according to MIO mapping;1:set the loop
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPS_setCanLoop (u32 loop_en)
{
    FSlcrPs_unlock();  // SLCR UNLOCK
    if (loop_en == 0)
    {
        FSlcrPs_setBitTo0(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 2);
    }
    else if (loop_en == 1)
    {
        FSlcrPs_setBitTo1(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 2);
    }
    FSlcrPs_lock();  // SLCR LOCK
}

/****************************************************************************/
/**
 *
 *  This function loop UART0's Tx to UART1's Rx,and UART1's Tx to UART0's Rx.
 *
 * @param
 *  loop_en -- 0:connect UART inputs according to MIO mapping;1:set the loop
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPS_setUartLoop (u32 loop_en)
{
    FSlcrPs_unlock();  // SLCR UNLOCK
    if (loop_en == 0)
    {
        FSlcrPs_setBitTo0(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 1);
    }
    else if (loop_en == 1)
    {
        FSlcrPs_setBitTo1(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 1);
    }
    FSlcrPs_lock();  // SLCR LOCK
}

/****************************************************************************/
/**
 *
 *  This function loop SPI0's outputs to SPI1's inputs,and SPI1's outputs to
 *  SPI0's inputs
 *
 * @param
 *  loop_en -- 0:connect SPI inputs according to MIO mapping;1:set the loop
 *
 * @return
 *  Null
 *
 * @note
 *  Null
 *
 ****************************************************************************/
void FSlcrPS_setSpiLoop (u32 loop_en)
{
    FSlcrPs_unlock();  // SLCR UNLOCK
    if (loop_en == 0)
    {
        FSlcrPs_setBitTo0(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 0);
    }
    else if (loop_en == 1)
    {
        FSlcrPs_setBitTo1(FPS_SLCR_BASEADDR, SLCR_MIO_LOOPBACK, 0);
    }
    FSlcrPs_lock();  // SLCR LOCK
}
