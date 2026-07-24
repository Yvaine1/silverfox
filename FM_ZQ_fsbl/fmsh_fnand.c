#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_hpnfc.h"
#include "fmsh_hpnfc_bbm.h"
#include "fmsh_hpnfc_flash.h"
#include "fmsh_hpnfc_hw.h"
#include "fmsh_hpnfc_skip.h"

static FNandPsu_T nand;

static int s_status = 0;

__attribute__((unused)) NAND_USERCFG(mdma_transfer) = {
    .options = NAND_USE_RNB_LINE | NAND_ERASED_DET,
    .dma_type = NAND_MDMA,
    //.dma_type = NAND_NODMA,
    .dev_bbt_options = NAND_BBT_PERCHIP | NAND_SKIP_BBTSCAN,
};

__attribute__((unused)) NAND_USERCFG(sdma_transfer) = {
    .options = NAND_USE_RNB_LINE | NAND_ERASED_DET,
    .dma_type = NAND_SDMA,
    .dev_bbt_options = NAND_BBT_PERCHIP | NAND_SKIP_BBTSCAN,
};

__attribute__((unused)) NAND_USERCFG(intr) = {
    .options = NAND_USE_RNB_LINE | NAND_ERASED_DET | NAND_USE_INTR,
    .dma_type = NAND_MDMA,
    .dev_bbt_options = NAND_BBT_PERCHIP | NAND_SKIP_BBTSCAN,
};

__attribute__((unused)) NAND_USERCFG(multilun) = {
    .options = NAND_USE_RNB_LINE | NAND_ERASED_DET | NAND_USE_INTR,
    .dma_type = NAND_MDMA,
    .dev_bbt_options = NAND_BBT_PERCHIP | NAND_SKIP_BBTSCAN,
};

__attribute__((unused)) NAND_USERCFG(perf) = {
    .options = NAND_USE_RNB_LINE | NAND_ERASED_DET,
    .dma_type = NAND_MDMA,
    .dev_bbt_options = NAND_BBT_PERCHIP,
};

/*****************************************************************************/
static void FNandPsu_Handler (void *callBackRef, u32 statusEvent, u32 byteCount)
{}

int nand_init (FNandPsu_T *nfcPtr, int device_id)
{
    int ret;

    ret = FNandPsu_Initialize(nfcPtr);
    if (ret)
    {
        return ret;
    }

    FNandPsu_SetStatusHandler(nfcPtr, &s_status, FNandPsu_Handler);

    FNandPsu_Reset();

    return 0;
}

u32 FmshFsbl_InitNand (u32 DeviceFlags)
{
    u32 Status = FMSH_SUCCESS;
    FNandPsu_T *nfcPtr;

    nfcPtr = &nand;

    Status = nand_init(nfcPtr, 0);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print_info("ERR(nand_init): nand init failed!\r\n");
        Status = FMSH_FAILURE;
        goto end;
    }

    Status = FNandPsu_Device_Init(nfcPtr, GET_NAND_USERCFG(mdma_transfer));
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print_info("ERR(nand_init): nand init failed!\r\n");
        Status = FMSH_FAILURE;
        goto end;
    }
    fmsh_print_info("INFO(nand_init): nand init succeed!\r\n");
end:

    return Status;
}

u32 FmshFsbl_NandAccess (u32 SrcAddress, u32 DestAddress, u32 Length)
{
    FNandPsu_T *nfcPtr;

    nfcPtr = &nand;
    u32 Status = FMSH_SUCCESS;

    Status = FNandPsu_NoSkip_Read(nfcPtr, SrcAddress, Length, (u8 *)DestAddress,
                                  0x0);
    
    return Status;

}
