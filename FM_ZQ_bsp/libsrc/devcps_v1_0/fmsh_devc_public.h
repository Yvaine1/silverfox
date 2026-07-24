/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_devc_lib.h
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   wfb  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

#ifndef _FDEVCPS_PUBLIC_H_ /* prevent circular inclusions */
#define _FDEVCPS_PUBLIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_devc.h"

/************************** Constant Definitions *****************************/

/*FOR FMSH 325T MUST PATCH IT USING PROCISE DEVELOPED BY FMSH LTD BEFORE USE
 * IT*/
#define FMSH_325T /********for FMSH 325T, must open the define********/
// #define TEST_TIME /********for output dma carry time, must open the
// define********/
#define DEVC_READBACK

#define DEVC_POLL_DONE_MS                 1500
#define DEVC_POLL_INIT_TIMEOUT_VAL        15000
#define PCAP_WR_DATA_ADDR                 0xFFFFFFFF
#define PCAP_RD_DATA_ADDR                 0xFFFFFFFF
#define FMSH_DMA_INVALID_ADDRESS          PCAP_RD_DATA_ADDR

// Types of PCAP transfers
#define FMSH_NON_SECURE_PCAP_WRITE        0
#define FMSH_SECURE_PCAP_WRITE            1
#define FMSH_PCAP_READBACK                2
#define FMSH_PCAP_LOOPBACK                3
#define FMSH_NON_SECURE_PCAP_WRITE_DUMMMY 4

/*
 * Addresses of the Configuration Registers
 */
#define CRC                               0 /* Status Register */
#define FAR                               1 /* Frame Address Register */
#define FDRI                              2 /* FDRI Register */
#define FDRO                              3 /* FDRO Register */
#define CMD                               4 /* Command Register */
#define CTL0                              5 /* Control Register 0 */
#define MASK                              6 /* MASK Register */
#define STAT                              7 /* Status Register */
#define LOUT                              8 /* LOUT Register */
#define COR0                              9 /* Configuration Options Register 0 */
#define MFWR                              10 /* MFWR Register */
#define CBC                               11 /* CBC Register */
#define IDCODE                            12 /* IDCODE Register */
#define AXSS                              13 /* AXSS Register */
#define COR1                              14 /* Configuration Options Register 1 */
#define WBSTAR                            16 /* Warm Boot Start Address Register */
#define TIMER                             17 /* Watchdog Timer Register */
#define BOOTSTS                           22 /* Boot History Status Register */
#define CTL1                              24 /* Control Register 1 */

/**************************** Type Definitions *******************************/
enum CMD_CODE {
    CMD_WCFG = 0x01,
    CMD_MFW,
    CMD_LFRM,
    CMD_RCFG,
    CMD_START,
    CMD_RCAP,
    CMD_RCRC,
    CMD_AGHIGH,
    CMD_SWITCH,
    CMD_GRESTORE,
    CMD_SHUTDOWN,
    CMD_GCAPTURE,
    CMD_DESYNC,
    CMD_RESERVED,
    CMD_IPROG,
    CMD_CRCC,
    CMD_LTIMER
};

/*download mode*/
enum download_mode {
    DOWNLOAD_BITSTREAM = 0x0,
    READBACK_BITSTREAM = 0x2,
    DATA_LOOPBACK = 0x3,
    SECURE_DOWNLOAD_BITSTREAM = 0x08
};

#define DEVC_FRAME_WORD_NUM    (93)
/***************** Macros (Inline Functions) Definitions *********************/
#define TMP_PL_BUF_LEN 1024
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void FDevcPs_unLockCSU(FDevcPs_T *pDevc);
u32 FDevcPs_init(FDevcPs_T *pDevc, FDevcPs_Config *cfg);
u32 FDevcPs_getPlPowerStatus(FDevcPs_T *pDevc);
FDevcPs_Config *FDevcPs_LookupConfig(u16 DeviceId);

u32 FDevcPs_fabricInit(FDevcPs_T *pDevc, u32 TransferType);

void FDevcPs_IV(FDevcPs_T *pDevc, u32 *p, u32 len);

u32 FDevcPs_pollFpgaDone(FDevcPs_T *pDevc, u32 maxcount);

u32 FDevcPs_pcapLoadPartition(FDevcPs_T *pDevc, u32 SourceDataAddr,
                              u32 DestinationDataAddr, u32 SourceLength,
                              u32 DestinationLength, u32 SecureTransfer);

u32 FDevcPs_wrFrameData(FDevcPs_T *pDevc, u32 far_addr, u32 *wr_data);

u32 FDevcPs_writeReg(FDevcPs_T *pDevc, u32 addr, u32 wrdata);

#ifdef DEVC_READBACK
u32 FDevcPs_getConfigdata(FDevcPs_T *pDevc, u32 *DestinationDataPtr,
                          u32 DestinationLength, u32 addr, u32 ConfigReg);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
