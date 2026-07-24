/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_devc_private.h
 *
 * This file contains ......
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

#ifndef _FDEVCPS_PRIVATE_H_ /* prevent circular inclusions */
#define _FDEVCPS_PRIVATE_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fmsh_devc.h"
#include "fmsh_devc_public.h"

/************************** Constant Definitions *****************************/
// csu lock
#define DEVC_UNLOCK                            (0x757bdf0dU)
// KEY IV load
#define DEVC_KEY_IV_LOAD                       (0x3U)
// mask define
#define DEVC_ERROR_FLAGS_MASK                  (0x00148040U)
// dummy value
#define DEVC_DUMMY_VALUE                       (0x20000000)

// Register offset
#define DEVC_SAC_CFG_OFFSET                    (0x0C)
#define DEVC_SAC_INT_STATUS_OFFSET             (0x10)
#define DEVC_SAC_STATUS_OFFSET                 (0x18)
#define DEVC_DMA_SRC_ADDR_OFFSET               (0x1C)
#define DEVC_DMA_DEST_ADDR_OFFSET              (0x20)
#define DEVC_DMA_SRC_LEN_OFFSET                (0x24)
#define DEVC_DMA_DEST_LEN_OFFSET               (0x28)
#define DEVC_UNLOCK_OFFSET                     (0x2C)
#define DEVC_KEY_IV_LOAD_OFFSET                (0x84)
#define DEVC_KEY_SRC_OFFSET                    (0x88)
#define DEVC_DEC_FLAG_OFFSET                   (0x90)
#define DEVC_GCM_CTRL_OFFSET                   (0x100)
#define DEVC_IVUP0_OFFSET                      (0x130)
#define DEVC_IVUP1_OFFSET                      (0x134)
#define DEVC_IVUP2_OFFSET                      (0x138)
#define DEVC_IVUP3_OFFSET                      (0x13c)
#define DEVC_PL_PCAP_CTRL_OFFSET               (0x75C)

// SAC CFG
#define DEVC_CFG_PROGRAM_B_MASK                (0x80000000U)
#define DEVC_CFG_PCAP_MODE_EN_MASK             (0x40000000U)
#define DEVC_CFG_SEC_DOWN_DATA_BYTE_SWAP_MASK  (0x30000000U)
#define DEVC_CFG_SEC_DOWN_DATA_BYTE_SWAP_SHIFT (28U)
#define DEVC_CFG_TXFIFO_DATA_SWAP_MASK         (0x03000000U)
#define DEVC_CFG_TXFIFO_DATA_SWAP_SHIFT        (24U)
#define DEVC_CFG_DMA_SWITCH_MASK               (0x00F00000U)
#define DEVC_CFG_DMA_SWITCH_SHIFT              (20U)
#define DEVC_CFG_RXFIFO_DATA_SWAP_MASK         (0x00060000U)
#define DEVC_CFG_RXFIFO_DATA_SWAP_SHIFT        (17U)
#define DEVC_CFG_SMAP32_SWAP_CTRL_MASK         (0x00010000U)
#define DEVC_CFG_SMAP32_SWAP_CTRL_SHIFT        (16U)
#define DEVC_CFG_READBACK_DUMMY_NUM_MASK       (0x0000E000U)
#define DEVC_CFG_READBACK_DUMMY_NUM_SHIFT      (13U)
#define DEVC_CFG_RFIFO_TH_MASK                 (0x00000300U)
#define DEVC_CFG_RFIFO_TH_SHIFT                (8U)
#define DEVC_CFG_WFIFO_TH_MASK                 (0x000000C0U)
#define DEVC_CFG_WFIFO_TH_SHIFT                (6U)
#define DEVC_CFG_RCLK_EDGE_MASK                (0x00000020U)
#define DEVC_CFG_RCLK_EDGE_SHIFT               (5U)
#define DEVC_CFG_WCLK_EDGE_MASK                (0x00000010U)
#define DEVC_CFG_WCLK_EDGE_SHIFT               (4U)
#define DEVC_CFG_CSI_B_MASK                    (0x00000002U)
#define DEVC_CFG_RDWR_B_MASK                   (0x00000001U)

// SAC INT STATUS
#define DEVC_INT_STS_MASK                      (0xFFFFFFFFU)
#define DEVC_DMA_DONE_MASK                     (0x00002000U)
#define DEVC_DMA_PCAP_DONE_MASK                (0x00001000U)
#define DEVC_PCFG_DONE_MASK                    (0x00000004U)

// SAC STATUS
#define DEVC_STATUS_DMA_BUSY_MASK              (0x80000000U)
#define DEVC_STATUS_PL_POR_MASK                (0x40000000U)
#define DEVC_STATUS_PCFG_DONE_MASK             (0x00000200U)
#define DEVC_STATUS_PCFG_INIT_MASK             (0x00000010U)

// KEY SRC
#define DEVC_KEY_SRC_MASK                      (0x0000000FU)

// DEC FLAG
#define DEVC_DEC_FLAG_MASK                     (0x0000000FU)

// GCM CTRL
#define DEVC_GCM_CHMODE_MASK                   (0x00000060U)
#define DEVC_GCM_CHMODE_SHIFT                  (5U)
#define DEVC_GCM_MODE_MASK                     (0x00000018U)
#define DEVC_GCM_MODE_SHIFT                    (3U)
#define DEVC_GCM_ALG_SEL_MASK                  (0x00000002U)
#define DEVC_GCM_ALG_SEL_SHIFT                 (1U)
#define DEVC_GCM_EN_MASK                       (0x00000001U)

// PL PCAP CTRL
#define DEVC_PCAP_PR_MASK                      (0x00000001U)

/**
 * @name Configuration Type1 packet headers masks
 * @{
 */
#define XDC_TYPE_SHIFT                         29
#define XDC_REGISTER_SHIFT                     13
#define XDC_OP_SHIFT                           27
#define XDC_TYPE_1                             1
#define DEVC_OPCODE_READ                       1
#define DEVC_OPCODE_WRITE                      2
/* @} */

// data_swap
enum data_swap {
    none_swap = 0x0,
    half_word_swap = 0x1,
    byte_swap = 0x2,
    bit_swap = 0x3
};

// smap32_swap
enum smap32_swap { smap32_swap_disable = 0x0, smap32_swap_enable = 0x1 };

enum dummy_num {
    dummy_0 = 0x0,
    dummy_1 = 0x1,
    dummy_2 = 0x2,
    dummy_3 = 0x3,
    dummy_4 = 0x4,
    dummy_5 = 0x5,
    dummy_6 = 0x6,
    dummy_7 = 0x7,
};

enum readFifoThre {
    readFifoThre_hex_0x40 = 0x0,  // default
    readFifoThre_hex_0x80 = 0x1,
    readFifoThre_hex_0xc0 = 0x2,
    readFifoThre_hex_0x100 = 0x3
};

enum writeFifoThre {
    writeFifoThre_hex_0x80 = 0x0,  // default
    writeFifoThre_hex_0x60 = 0x1,
    writeFifoThre_hex_0x40 = 0x2,
    writeFifoThre_hex_0x10 = 0x3
};

enum clk_edge { rising_edge = 0x1, falling_edge = 0x0 };

enum ALG {
    AES = 0x0,
    SM4 = 0x1  //,
               // NONE = 0x2
};

enum MODE { ENCODE = 0x0, DCODE = 0x3 };

enum CHMOD {
    ECB = 0x0,
    CTR = 0x2,  // use for encrypt or decrypt
    MULTH = 0x3
};

enum KEYSRC { OTHER = 0x0, DEV_KEY = 0x1, KUP = 0x2, MULTH_H = 0x3 };

enum DECFLAG {
    use_opkey = 0xe,
    no_opkey = 0xa,
    ivup_kup_wr_en = 0x3,
    clear = 0x0
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
u32 FDevcPs_Prog_B(FDevcPs_T *pDevc);
void FDevcPs_CSI_B_HIGH(FDevcPs_T *pDevc);
void FDevcPs_RDWR_B_HIGH(FDevcPs_T *pDevc);
void FDevcPs_CSI_B_LOW(FDevcPs_T *pDevc);
void FDevcPs_RDWR_B_LOW(FDevcPs_T *pDevc);
void FDevcPs_secDownDataByteSwap(FDevcPs_T *pDevc, enum data_swap mode);
void FDevcPs_txDataSwap(FDevcPs_T *pDevc, enum data_swap mode);
void FDevcPs_downloadMode(FDevcPs_T *pDevc, enum download_mode mode);
void FDevcPs_rxDataSwap(FDevcPs_T *pDevc, enum data_swap mode);
void FDevcPs_smap32Swap(FDevcPs_T *pDevc, enum smap32_swap mode);
void FDevcPs_readbackDummyCount(FDevcPs_T *pDevc, enum dummy_num mode);
void FDevcPs_readFifoThre(FDevcPs_T *pDevc, enum readFifoThre mode);
void FDevcPs_writeFifoThre(FDevcPs_T *pDevc, enum writeFifoThre mode);
void FDevcPs_rclk_edge(FDevcPs_T *pDevc, enum clk_edge mode);
void FDevcPs_wclk_edge(FDevcPs_T *pDevc, enum clk_edge mode);

void FDevcPs_disableGcm(FDevcPs_T *pDevc);
void FDevcPs_enableGcm(FDevcPs_T *pDevc);
void FDevcPs_setGcmAlg(FDevcPs_T *pDevc, enum ALG mode);
void FDevcPs_setGcmMode(FDevcPs_T *pDevc, enum MODE mode);
void FDevcPs_setGcmChMode(FDevcPs_T *pDevc, enum CHMOD mode);

void FDevcPs_setKeySource(FDevcPs_T *pDevc, enum KEYSRC mode);
void FDevcPs_loadKeyIV(FDevcPs_T *pDevc);
void FDevcPs_setDecFlag(FDevcPs_T *pDevc, enum DECFLAG mode);

void devc_byte_swap(u32 *srcPtr, u32 len);
void devc_byte_swap_todes(u32 *srcPtr, u32 len, u32 *desPtr);

u32 FDevcPs_clearPcapStatus(FDevcPs_T *pDevc);
u32 FDevcPs_initiateDma(FDevcPs_T *pDevc, u32 SourceAddr, u32 DestAddr,
                        u32 SrcWordLength, u32 DestWordLength);
u32 FDevcPs_pollDmaDone(FDevcPs_T *pDevc, u32 MaxCount);
u32 FDevcPs_pollDPDone(FDevcPs_T *pDevc, u32 MaxCount);

u32 FDevcPs_regAddr(u8 Register, u8 OpCode, u8 Size);

u32 FDevcPs_transfer(FDevcPs_T *pDevc, uintptr_t SourceDataAddr,
                     u32 SrcWordLength, uintptr_t DestinationDataAddr,
                     u32 DestWordLength, u32 TransferType);

u32 FDevcPs_initiateDma_readback(FDevcPs_T *pDevc, u32 SourceAddr,
                                  u32 DestAddr, u32 SrcWordLength,
                                  u32 DestWordLength);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
