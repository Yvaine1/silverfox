/******************************************************************************
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fmsh_gic_sinit.c
 * @addtogroup scugic_v3_1
 * @{
 *
 * Contains static init functions for the FGicPs driver for the Interrupt
 * Controller. See FGicPs.h for a detailed description of the driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- --------------------------------------------------------
 * 1.00a drg  01/19/10 First release
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_common_types.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"

/************************** Constant Definitions *****************************/
FGicPs_Config FGicPs_ConfigTable[] = {
    {0, FPAR_SCUGIC_CPU_BASEADDR, FPAR_SCUGIC_DIST_BASEADDR}};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

// extern FGicPs_Config FGicPs_ConfigTable[FPAR_SCUGIC_NUM_INSTANCES];

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 *
 * Looks up the device configuration based on the unique device ID. A table
 * contains the configuration info for each device in the system.
 *
 * @param	DeviceId is the unique identifier for a device.
 *
 * @return	A pointer to the FGicPs configuration structure for the
 *		specified device, or NULL if the device was not found.
 *
 * @note		None.
 *
 ******************************************************************************/
FGicPs_Config *FGicPs_LookupConfig (u16 DeviceId)
{
    FGicPs_Config *CfgPtr = NULL;

    if (DeviceId == 0U)
    {
        CfgPtr = &FGicPs_ConfigTable[DeviceId];
    }
    return (FGicPs_Config *)CfgPtr;
}
/** @} */
