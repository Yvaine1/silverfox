/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_devc_example.c
 *
 * This file contains a example of devc.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  12/13/2023  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_devc_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static __attribute__((aligned(64))) u32 s_devc_rData[16] = {0};
/******************************************************************************
 *
 * @description
 *    A example of devc, write cmd reg of pl.
 *
 * @param    None.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FDevcPs_wrReg_example (void)
{
    FDevcPs_T devcDev;
    u8 ret = FMSH_SUCCESS;

    FDevcPs_Config* Config = NULL;
    Config = FDevcPs_LookupConfig(FPAR_DEVCPS_DEVICE_ID);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }
    ret = FDevcPs_init(&devcDev, Config);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }

    FDevcPs_fabricInit(&devcDev, FMSH_PCAP_READBACK);

    // IDCODE
    FDevcPs_getConfigdata(&devcDev, s_devc_rData, 10, IDCODE, 0);
    fmsh_print("pl idcode=0x%8x\r\n",s_devc_rData[0]);

    // GRESTORE
    ret = FDevcPs_writeReg(&devcDev, CMD, CMD_GRESTORE);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }

    FDevcPs_getConfigdata(&devcDev, s_devc_rData, 10, CMD, 0);
    if (s_devc_rData[0] != CMD_GRESTORE)
    {
        return 1;
    }

    // GCAPTURE
    ret = FDevcPs_writeReg(&devcDev, CMD, CMD_GCAPTURE);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }

    FDevcPs_getConfigdata(&devcDev, s_devc_rData, 10, CMD, 0);
    if (s_devc_rData[0] != CMD_GCAPTURE)
    {
        return 1;
    }

    return ret;
}

/******************************************************************************
 *
 * @description
 *    A example of devc, write frame data in far addr of PL.
 *
 * @param    None.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FDevcPs_wrFrame_example (void)
{
    FDevcPs_T devcDev;
    u32 rdData[DEVC_FRAME_WORD_NUM*2+25] = {0};
    u32 wrData[DEVC_FRAME_WORD_NUM] = {0};
    u32 row=0;
    u32 col=148;
    u32 min=5;
    u32 far_addr=(row<<18)|(col<<8)|(min);
    u32 indx = 0;
    u32 i = 0;
    u8 ret = FMSH_SUCCESS;

    for (i = 0; i < DEVC_FRAME_WORD_NUM; i++)
    {
        wrData[i] = i;
    }

    FDevcPs_Config* Config = NULL;
    Config = FDevcPs_LookupConfig(FPAR_DEVCPS_DEVICE_ID);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }
    ret = FDevcPs_init(&devcDev, Config);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }

    FDevcPs_fabricInit(&devcDev, FMSH_NON_SECURE_PCAP_WRITE);

    ret = FDevcPs_wrFrameData(&devcDev, far_addr, wrData);
    if (1 == ret)
    {
        return ret;
    }

    FDevcPs_fabricInit(&devcDev, FMSH_PCAP_READBACK);
    FDevcPs_getConfigdata(&devcDev, rdData, 2 * DEVC_FRAME_WORD_NUM+25, far_addr, 0xaa55);

    for (indx = 0; indx < DEVC_FRAME_WORD_NUM; indx++)
    {
        if(indx==47)
             continue;
        if (rdData[DEVC_FRAME_WORD_NUM + 25 + indx] != wrData[indx])
        {
            fmsh_print("%d Word :rd=0x%08x,wd=0x%08x\r\n", indx,
                       rdData[DEVC_FRAME_WORD_NUM + indx], wrData[indx]);
            ret = 1;
        }
    }

    return ret;
}
