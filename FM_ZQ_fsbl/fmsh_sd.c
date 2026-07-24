#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_sd.h"
#include "fmsh_sdhci.h"
#include "fmsh_sdhci_card.h"
#include "fmsh_sdhci_hw.h"
#include "sdmmc_fatfs.h"

#define TEST_ADDR 0x400

FSdPsu_T sdmmc0;

#ifdef MPSOC_SDMMC1
FSdPsu_T sdmmc1;
#endif

__attribute__((unused)) static u8 SendBuffer[1024], RecvBuffer[1024];
__attribute__((unused)) static u32 s_intrflags = 0U;

/************************ function ******************************************/
__attribute__((unused)) static int sdmmc_host_reset (FSdPsu_T *sdPtr, u8 mask)
{
    u32 value = 0U;
    int timeout = 100;

    value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);

    // write software reset
    value |= mask;
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS11, value);

    // wait reset bit become 0
    while (1)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS11);
        if ((value & mask) == 0U)
        {
            break;
        }

        timeout--;
        if (timeout <= 0)
        {
            return FMSH_ETIME;
        }
        delay_ms(1);
    };

    return 0;
}

/***************************** filesystem ************************************/
__attribute__((unused)) static int filesystem_test (FSdPsu_T *sdPtr)
{
    int ret = 0;
    char s[80];

    fmsh_print_dbg("===========================\r\n");
    fmsh_print_dbg("INFO(filesystem_test):\r\n");

    ret = InitSD("1:/testfile.txt");
    if (ret)
    {
        goto end;
    }
    /*
    strcpy(s, "This is the testfile for sdmmc!");
    ret = SDWrite(0, s, sizeof(s));
    if(ret)
        goto end;*/

    (void)memset(s, 0, sizeof(s));
    ret = SDRead(0U, s, sizeof(s));
    if (ret)
    {
        goto end;
    }

end:
    if (ret)
    {
        fmsh_print_dbg("filesystem test failed!\r\n");
    }
    else
    {
        fmsh_print_dbg("filesystem test pass!\r\n");
    }

    return ret;
}

/***************************** data path *************************************/

/***************************** phy *******************************************/

/***************************** others ****************************************/
__attribute__((unused)) static int reset_test (FSdPsu_T *sdPtr) { return 0; }

/*****************************************************************************/
static void FSdPsu_Handler (void *callBackRef, u32 statusEvent, u32 byteCount)
{}

int sdmmc_init (FSdPsu_T *sdPtr, u16 id)
{
    int ret = 0;
    u32 value;
    FSdPsu_Config_T *configPtr;

    configPtr = FSdPsu_LookupConfig(id);
    if (configPtr == NULL)
    {
        return FMSH_FAILURE;
    }

    ret = FSdPsu_CfgInitialize(sdPtr, configPtr);
    if (ret)
    {
        return ret;
    }

    FSdPsu_SetStatusHandler(sdPtr, &s_intrflags, FSdPsu_Handler);

    value = 0;
    if (sdPtr->config.card_type == SDMMC_TYPE_MMC)
    {
        if (id == 0)
        {
            value |= 0x1;
        }
        else
        {
            value |= 0x10000;
        }
    }
    FMSH_WriteReg(0xff180000, 0x35c, value);

    return ret;
}

int fmsh_sdmmc_verify (void)
{
    int ret = 0;

#ifdef FSBL_SD_0
    FSdPsu_T *sd0Ptr;

    sd0Ptr = &sdmmc0;
    sdmmc_init(sd0Ptr, 0);

    ret = filesystem_test(sd0Ptr);

#endif

#ifdef FSBL_SD_1
    FSdPsu_T *sd1Ptr;

    sd1Ptr = &sdmmc1;
    sdmmc_init(sd1Ptr, 1);

    ret = filesystem_test(sd1Ptr);

#endif

    return ret;
}
