#include "fmsh_psu_parameters.h"
#include "fmsh_xspi_lib.h"

#ifndef FPAR_QSPIPSU_0_PAD_LPBK
#define FPAR_QSPIPSU_0_PAD_LPBK (0)
#endif

#ifndef FPAR_QSPIPSU_1_PAD_LPBK
#define FPAR_QSPIPSU_1_PAD_LPBK (0)
#endif

/******************************************************************************
 * This table contains configuration information for each QSPI
 * device in the system.
 ******************************************************************************/
FQspiPsu_Config_T FQspiPsu_ConfigTable[] = {{
                                                FPAR_QSPIPSU_0_DEVICE_ID,
                                                FPAR_QSPIPSU_0_PAD_LPBK,
                                                FPAR_QSPIPSU_0_BASEADDR,
                                                FPAR_QSPIPSU_0_D_BASEADDR,
                                                FPAR_QSPIPSU_0_CLK_FREQ_HZ,
                                                FPAR_QSPIPSU_0_BOARD_DELAY,
                                            },
                                            {
                                                FPAR_QSPIPSU_1_DEVICE_ID,
                                                FPAR_QSPIPSU_1_PAD_LPBK,
                                                FPAR_QSPIPSU_1_BASEADDR,
                                                FPAR_QSPIPSU_1_D_BASEADDR,
                                                FPAR_QSPIPSU_1_CLK_FREQ_HZ,
                                                FPAR_QSPIPSU_1_BOARD_DELAY,
                                            }};
