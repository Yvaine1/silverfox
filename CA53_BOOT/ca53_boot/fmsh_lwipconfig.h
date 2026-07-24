#ifndef __FMSH_LWIPCONFIG_H__
#define __FMSH_LWIPCONFIG_H__

#include "fmsh_psu_parameters.h"
#include "fmsh_psu_parameters.h"
#include "fmsh_gmac.h"


#define FLWIP_CONFIG_INCLUDE_GEM 1

#define FLWIP_CONFIG_N_TX_DESC 1024
#define FLWIP_CONFIG_N_RX_DESC 1024

//debug print
#define GMAC_DEBUG_OUT                          1

#define EthernetFrameSize                       FGMACPS_MAX_VLAN_FRAME_SIZE_JUMBO
typedef char EthernetFrame[EthernetFrameSize];

/***************** Macros (Inline Functions) Definitions *********************/
#define GMAC_TRACE_OUT(flag, ...)       \
    do{                                 \
        if(flag) {                      \
            fmsh_print(__VA_ARGS__);    \
        }                               \
    } while(0)


//GMAC baseaddr and device id set
#define GMAC_SELECT_BASEADDR                    FPAR_GMACPS_1_BASEADDR
#define GMAC_SELECT_ID                          FPAR_GMACPS_1_DEVICE_ID

#define GMAC_SELECT_INTR                        GEM1_INT_ID

#define LWIP_AUTO_NAG_EN                        1
#define LWIP_AUTO_PHY_DET                       0

#define LWIP_PHY_DEVICE0                         PHY_YT8521
#define LWIP_PHY_ADDR0                           0

#define LWIP_PHY_DEVICE1                         PHY_YT8521
#define LWIP_PHY_ADDR1                           4

#define LWIP_PHY_DEVICE2                         PHY_YT8521
#define LWIP_PHY_ADDR2                           4

#define LWIP_PHY_DEVICE3                         PHY_YT8521
#define LWIP_PHY_ADDR3                           6
#endif