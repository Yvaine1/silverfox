/******************************************************************************
 *
 * Copyright (C) 2023 - 2033 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_gmac_lib.c
 *
 * gmac driver
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 1_0   Danyang Wang  6/25/2023  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_gmac_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * This function perform the reset sequence to the given gmacps interface by
 * configuring the appropriate control bits in the gmacps specifc registers.
 * the gmacps reset squence involves the following steps
 *	Disable all the interuupts
 *	Clear the status registers
 *	Disable Rx and Tx engines
 *	Update the Tx and Rx descriptor queue registers with reset values
 *	Update the other relevant control registers with reset value
 *
 * @param   BaseAddr of the interface
 *
 * @return N/A
 *
 * @note
 * This function will not modify the slcr registers that are relavant for
 * gmacps controller
 ******************************************************************************/
void FGmacPs_ResetHw (u32 BaseAddr)
{
    u32 RegVal;

    /* Disable the interrupts  */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_IDR_OFFSET, 0x0U);

    /* Stop transmission,disable loopback and Stop tx and Rx engines */
    RegVal = FGmacPs_ReadReg(BaseAddr, FGMACPS_NWCTRL_OFFSET);
    RegVal &= ~((u32)FGMACPS_NWCTRL_TXEN_MASK | (u32)FGMACPS_NWCTRL_RXEN_MASK |
                (u32)FGMACPS_NWCTRL_HALTTX_MASK |
                (u32)FGMACPS_NWCTRL_LOOPEN_MASK);
    /* Clear the statistic registers, flush the packets in DPRAM*/
    RegVal |= (FGMACPS_NWCTRL_STATCLR_MASK | FGMACPS_NWCTRL_FLUSH_DPRAM_MASK);
    FGmacPs_WriteReg(BaseAddr, FGMACPS_NWCTRL_OFFSET, RegVal);
    /* Clear the interrupt status */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_ISR_OFFSET, FGMACPS_IXR_ALL_MASK);
    /* Clear the tx status */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_TXSR_OFFSET,
                     (FGMACPS_TXSR_ERROR_MASK | (u32)FGMACPS_TXSR_TXCOMPL_MASK |
                      (u32)FGMACPS_TXSR_TXGO_MASK));
    /* Clear the rx status */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_RXSR_OFFSET, FGMACPS_RXSR_FRAMERX_MASK);
    /* Clear the tx base address */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_TXQBASE_OFFSET, 0x0U);
    /* Clear the rx base address */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_RXQBASE_OFFSET, 0x0U);
    /* Update the network config register with reset value */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_NWCFG_OFFSET, FGMACPS_NWCFG_RESET_MASK);
    /* Update the hash address registers with reset value */
    FGmacPs_WriteReg(BaseAddr, FGMACPS_HASHL_OFFSET, 0x0U);
    FGmacPs_WriteReg(BaseAddr, FGMACPS_HASHH_OFFSET, 0x0U);
}
/** @} */
