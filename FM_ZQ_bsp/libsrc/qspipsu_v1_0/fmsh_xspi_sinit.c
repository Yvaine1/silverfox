
#include "fmsh_psu_parameters.h"
#include "fmsh_xspi_lib.h"

extern FQspiPsu_Config_T FQspiPsu_ConfigTable[];

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
FQspiPsu_Config_T *FQspiPsu_LookupConfig (uint16_t device_id)
{
    int index;
    FQspiPsu_Config_T *cfgPtr = NULL;

    for (index = 0; index < 2; index++)
    {
        if (FQspiPsu_ConfigTable[index].device_id == device_id)
        {
            cfgPtr = &FQspiPsu_ConfigTable[index];
            break;
        }
    }
    return cfgPtr;
}
