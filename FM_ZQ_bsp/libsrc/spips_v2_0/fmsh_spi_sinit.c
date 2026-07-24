#include "fmsh_common.h"
#include "fmsh_spi.h"

extern FSpiPs_Config_T FSpiPs_ConfigTable[];

/*****************************************************************************
 * This function looks up the device configuration based on the unique device
 *ID. The table s_ConfigTable contains the configuration info for each device in
 *the system.
 *
 * @param
 *       - DeviceId contains the ID of the device for which the
 *		device configuration pointer is to be returned.
 *
 * @return
 *		- A pointer to the configuration found.
 *		- NULL if the specified device ID was not found.
 *
 * @note		None.
 *
 ******************************************************************************/
FSpiPs_Config_T* FSpiPs_LookupConfig (u16 device_id)
{
    int index;
    FSpiPs_Config_T* cfgPtr = NULL;

    for (index = 0; index < 2; index++)
    {
        if (FSpiPs_ConfigTable[index].device_id == device_id)
        {
            cfgPtr = &FSpiPs_ConfigTable[index];
            break;
        }
    }
    return cfgPtr;
}
