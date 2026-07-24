#include "fmsh_psu_parameters.h"
#include "fmsh_spi.h"

/******************************************************************************
 * This table contains configuration information for each SPI
 * device in the system.
 ******************************************************************************/
FSpiPs_Config_T FSpiPs_ConfigTable[] = {{0, FPS_SPI0_BASEADDR},

                                        {1, FPS_SPI1_BASEADDR}};
