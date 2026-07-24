#include <stdio.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_xspi.h"
#include "fmsh_xspi_hw.h"
#include "fmsh_xspi_nor.h"

FQspiPsu_T qspi0;

static int s_status;

/************************qspi direct mode**************************************/
QSPI_USERCFG(direct)
__attribute__((aligned(4))) = {
    .flags = 0,                 // QSPI_F_INTR_EN,
    .dma_type = QSPI_MDMA,
    .ers_mode = QSPI_ERS_SE,    // standard sector erase
    .prog_mode = QSPI_PROG_PP,  // x1 program
    .read_mode = QSPI_RD_QOR,   // x4 read
    .naddrs = 0,
};

/************************perf**************************************************/

/************************reset*************************************************/

/************************phy***************************************************/

/******************************************************************************/
void FQspiPs_Handler (void *callBackRef, u32 statusEvent, u32 byteCount) {}

int qspi_init (FQspiPsu_T *qspiPtr, int device_id)
{
    int ret;

    ret = FQspiPsu_Initialize(qspiPtr, device_id);
    if (ret)
    {
        return ret;
    }

    FQspiPsu_SetStatusHandler(qspiPtr, &s_status, FQspiPs_Handler);

    return ret;
}

u32 FmshFsbl_InitQspi (u32 DeviceFlags)
{
    FQspiPsu_T *qspi0Ptr;

    qspi0Ptr = &qspi0;
    u32 Status = FMSH_SUCCESS;
    Status = qspi_init(qspi0Ptr, 0);
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print_info("ERR(qspi init): qspi_init failed!\r\n");
        Status = FMSH_FAILURE;
        return Status;
    }
    (void)FQspiPsu_Reset(qspi0Ptr);
    Status = FQspiPsu_Nor_Init(qspi0Ptr, GET_QSPI_USERCFG(direct));  // direct
    if (Status != FMSH_SUCCESS)
    {
        fmsh_print_info("ERR(qspi init): FQspiPsu_Nor_Init failed!\r\n");
        Status = FMSH_FAILURE;
        return Status;
    }
    fmsh_print_info("INFO(qspi init): qspi init succeed!\r\n");

    return Status;
}

u32 FmshFsbl_QspiAccess (u32 SrcAddress, u32 DestAddress, u32 length)
{
    FQspiPsu_T *qspi0Ptr;

    qspi0Ptr = &qspi0;
    u32 Status = FMSH_SUCCESS;
    u8 *dstBuf;
    dstBuf = (u8 *)DestAddress;

    Status = FQspiPsu_Nor_Read(qspi0Ptr, SrcAddress, length, dstBuf);
    if (Status != FMSH_SUCCESS)
    {
        Status = FMSH_FAILURE;
        goto end;
    }

end:
    return Status;
}
