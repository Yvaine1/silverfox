
#include "fmsh_psu_parameters.h"
#include "fmsh_sdhci_lib.h"

/******************************************************************************
 * This table contains configuration information for each SSI
 * device in the system.
 ******************************************************************************/
FSdPsu_Config_T FSdPsu_ConfigTable[] = {
    {
        FPAR_SDPSU_0_DEVICE_ID,
        FPAR_SDPSU_0_TYPE,
        FPAR_SDPSU_0_BASEADDR,
        FPAR_SDPSU_0_SDIO_CLK_FREQ_HZ,
        FPAR_SDPSU_0_INIT_FREQ,
        FPAR_SDPSU_0_BUSWIDTH,
        FPAR_SDPSU_0_HAS_CD,
        FPAR_SDPSU_0_HAS_WP,
        FPAR_SDPSU_0_HAS_BUSPWR,
        FPAR_SDPSU_0_IS_CACHE_COHERENT,
    },

    {
        FPAR_SDPSU_1_DEVICE_ID,
        FPAR_SDPSU_1_TYPE,
        FPAR_SDPSU_1_BASEADDR,
        FPAR_SDPSU_1_SDIO_CLK_FREQ_HZ,
        FPAR_SDPSU_1_INIT_FREQ,
        FPAR_SDPSU_1_BUSWIDTH,
        FPAR_SDPSU_1_HAS_CD,
        FPAR_SDPSU_1_HAS_WP,
        FPAR_SDPSU_1_HAS_BUSPWR,
        FPAR_SDPSU_1_IS_CACHE_COHERENT,
    },
};
