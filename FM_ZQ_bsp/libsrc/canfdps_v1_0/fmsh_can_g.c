/***************************** Include Files *********************************/
#include "fmsh_can_lib.h"
#include "fmsh_psu_parameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FCanPs_Config FCanPs_ConfigTable[] = {
    {FPAR_CANPS_0_DEVICE_ID, FPAR_CANPS_0_BASEADDR,
     FPAR_CANPS_0_CAN_CLK_FREQ_HZ, FPAR_CANPS_0_TX_MODE,
     FPAR_CANPS_0_SAMPLE_POINT, FPAR_CANPS_0_D_SAMPLE_POINT},
    {FPAR_CANPS_1_DEVICE_ID, FPAR_CANPS_1_BASEADDR,
     FPAR_CANPS_1_CAN_CLK_FREQ_HZ, FPAR_CANPS_1_TX_MODE,
     FPAR_CANPS_1_SAMPLE_POINT, FPAR_CANPS_1_D_SAMPLE_POINT}};
