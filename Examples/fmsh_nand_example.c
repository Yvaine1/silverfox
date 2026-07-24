/******************************************************************************
 *
 * Copyright (C) FMSH, Corp.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * FMSH BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the FMSH shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from FMSH.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file fmsh_nand_example.c
 * @addtogroup nandpsu_v1_0
 * @{
 *
 * This File is used for FMZQ series MPSOC.
 * Contains example of the FNandPsu driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0  hzq 2024/03/29
 * 		     First Release
 * </pre>
 *
 ******************************************************************************/

/*************************!!!! Attention !!!! ********************************/
/*
 * 1. Before do NAND_PERF test, make sure ddr is initialized successfully
 * 2. Make sure MIO Voltage is compatible to device requirement
 */

/***************************** Include Files *********************************/
#include <stdlib.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_hpnfc_lib.h"

/************************** Constant Definitions *****************************/

// #define NAND_NOSKIP_EXAMPLE
#define NAND_SKIP_EXAMPLE
// #define NAND_PERF

/*
 * Flash address to which data is ot be written.
 */
#define TEST_ADDRESS 0x00900000
#define UNIQUE_VALUE 0x06

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA     2048

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
__attribute__((unused)) static int FNandPsu_noskip_example(FNandPsu_T *nfcPtr);
__attribute__((unused)) static int FNandPsu_skip_example(FNandPsu_T *nfcPtr);
__attribute__((unused)) static int FNandPsu_perf(FNandPsu_T *nfcPtr);

/************************** Variable Definitions *****************************/

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
static u8 *ReadBuffer;
static u8 *WriteBuffer;

static FNandPsu_T nand;

static int nand_cleanup (FNandPsu_T *nfcPtr)
{
    struct nand_device *device;

    device = CTRL_TO_NAND(nfcPtr);

    if (device->data_buf)
    {
        free(device->data_buf);
        device->data_buf = 0;
    }

    if (device->bb_info)
    {
        free(device->bb_info);
        device->bb_info = 0;
    }

    if (WriteBuffer)
    {
        free(WriteBuffer);
        WriteBuffer = 0;
    }

    if (ReadBuffer)
    {
        free(ReadBuffer);
        ReadBuffer = 0;
    }

    return FMSH_SUCCESS;
}

/******************************************************************************
 *  @description: Erase, Write, Read function test
 *
 *
 ******************************************************************************/
int FNandPsu_example (u16 deviceId)
{
    int ret;
    FNandPsu_T *nfcPtr;
    FNandPsu_Config_T *configPtr;

    nfcPtr = &nand;
    /*
     * Initialize the NFC driver so that it's ready to use
     */
    configPtr = FNandPsu_LookupConfig(deviceId);
    if (NULL == configPtr)
    {
        return FMSH_FAILURE;
    }
    ret = FNandPsu_CfgInitialize(nfcPtr, configPtr);
    if (ret != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    // initialize Write and Read Buffer
    WriteBuffer = malloc(MAX_DATA);
    if (WriteBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        nand_cleanup(nfcPtr);
        return FMSH_FAILURE;
    }
    ReadBuffer = malloc(MAX_DATA);
    if (ReadBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        nand_cleanup(nfcPtr);
        return FMSH_FAILURE;
    }

#ifdef NAND_SKIP_EXAMPLE
    ret = FNandPsu_skip_example(nfcPtr);
    if (ret)
    {
        nand_cleanup(nfcPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef NAND_NOSKIP_EXAMPLE
    ret = FNandPsu_noskip_example(nfcPtr);
    if (ret)
    {
        nand_cleanup(nfcPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef NAND_PERF
    ret = FNandPsu_perf(nfcPtr);
    if (ret)
    {
        nand_cleanup(nfcPtr);
        return FMSH_FAILURE;
    }
#endif

    nand_cleanup(nfcPtr);
    return ret;
}

__attribute__((unused)) NAND_USERCFG(example) = {
    .options = NAND_USE_RNB_LINE | NAND_ERASED_DET,
    .dma_type = NAND_MDMA,
    .dev_bbt_options = NAND_BBT_PERCHIP,
};

static int FNandPsu_noskip_example (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device;
    u32 uniqueValue, count;
    u64 eraseStart;
    u32 blockCount, eraseSize;

    device = CTRL_TO_NAND(nfcPtr);
    FNandPsu_Reset();
    /*
     * Initialize controller and detect nand flash.
     */
    ret = FNandPsu_Device_Init(nfcPtr, GET_NAND_USERCFG(example));
    if (ret != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    /*
     * Initialize the write buffer for a pattern to write to the FLASH
     * and the read buffer to zero so it can be verified after the read, the
     * test value that is added to the unique value allows the value to be
     * changed in a debug environment to guarantee
     */
    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        ReadBuffer[count] = 0;
        uniqueValue++;
    }

    /*
     * erase start addr and erase size must align to blocksize
     */
    eraseStart = TEST_ADDRESS & (~(u64)(device->model.blocksize - 1));
    blockCount = MAX_DATA / device->model.blocksize;
    if (MAX_DATA % device->model.blocksize)
    {
        blockCount += +1;
    }
    eraseSize = blockCount * device->model.blocksize;
    ret = FNandPsu_NoSkip_Erase(nfcPtr, eraseStart, eraseSize);
    if (ret)
    {
        fmsh_print("Nand NoSkip Erase Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * write data
     */
    ret = FNandPsu_NoSkip_Write(nfcPtr, TEST_ADDRESS, MAX_DATA, WriteBuffer, 0);
    if (ret)
    {
        fmsh_print("Nand NoSkip Write Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * read data
     */
    FNandPsu_NoSkip_Read(nfcPtr, TEST_ADDRESS, MAX_DATA, ReadBuffer, 0);
    if (ret)
    {
        fmsh_print("NoSkip Read Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * Setup a pointer to the start of the data that was read into the read
     * buffer and verify the data read is the data that was written
     */
    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        if (ReadBuffer[count] != (u8)(uniqueValue))
        {
            fmsh_print(
                "Nand Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    fmsh_print("Nand NoSkip Example Pass\r\n");
    return FMSH_SUCCESS;
}

static int FNandPsu_skip_example (FNandPsu_T *nfcPtr)
{
    int ret;
    struct nand_device *device;
    u32 uniqueValue, count;
    u64 eraseStart;
    u32 blockCount, eraseSize;

    device = CTRL_TO_NAND(nfcPtr);
    FNandPsu_Reset();
    /*
     * Initialize controller and detect nand flash.
     */
    ret = FNandPsu_Device_Init(nfcPtr, GET_NAND_USERCFG(example));
    if (ret != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    /*
     * Initialize the write buffer for a pattern to write to the FLASH
     * and the read buffer to zero so it can be verified after the read, the
     * test value that is added to the unique value allows the value to be
     * changed in a debug environment to guarantee
     */
    WriteBuffer = malloc(MAX_DATA);
    if (WriteBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        return FMSH_FAILURE;
    }
    ReadBuffer = malloc(MAX_DATA);
    if (ReadBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        return FMSH_FAILURE;
    }
    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        ReadBuffer[count] = 0;
        uniqueValue++;
    }

    /*
     * erase start addr and erase size must align to blocksize
     */
    eraseStart = TEST_ADDRESS & (~(u64)(device->model.blocksize - 1));
    blockCount = MAX_DATA / device->model.blocksize;
    if (MAX_DATA % device->model.blocksize)
    {
        blockCount += +1;
    }
    eraseSize = blockCount * device->model.blocksize;
    ret = FNandPsu_Skip_Erase(nfcPtr, eraseStart, eraseSize);
    if (ret)
    {
        fmsh_print("Nand Skip Erase Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * write data
     */
    ret = FNandPsu_Skip_Write(nfcPtr, TEST_ADDRESS, MAX_DATA, WriteBuffer, 0);
    if (ret)
    {
        fmsh_print("Nand Skip Write Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * read data
     */
    FNandPsu_Skip_Read(nfcPtr, TEST_ADDRESS, MAX_DATA, ReadBuffer, 0);
    if (ret)
    {
        fmsh_print("Nand Skip Read Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * Setup a pointer to the start of the data that was read into the read
     * buffer and verify the data read is the data that was written
     */
    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        if (ReadBuffer[count] != (u8)(uniqueValue))
        {
            fmsh_print(
                "Nand Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    fmsh_print("Nand Nand Skip Example Pass\r\n");
    nand_cleanup(nfcPtr);
    return FMSH_SUCCESS;
}

static int FNandPsu_perf (FNandPsu_T *nfcPtr)
{
    int ret;
    u64 time_st, time_ed, time;
    unsigned int i, loopcnt, size, addr;
    unsigned char *srcPtr, *dstPtr;

    /*
     * reset controller
     */
    FNandPsu_Reset();

    // initialize
    global_timer_enable();

    time_st = get_current_time();
    ret = FNandPsu_Device_Init(nfcPtr, GET_NAND_USERCFG(example));
    if (ret)
    {
        fmsh_print("ERROR: Failed to initialize NAND device!\r\n");
        return 1;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: NAND device Initailize cost %lld us.\r\n", time);

    // prepare 10MB test data
    size = 0xa00000;
    loopcnt = size / 0x100000;
    srcPtr = (unsigned char *)0x10000000;
    dstPtr = (unsigned char *)0x18000000;

    // erase perf
    addr = TEST_ADDRESS;
    time_st = get_current_time();
    ret = FNandPsu_Skip_Erase(nfcPtr, addr, size);
    if (ret)
    {
        fmsh_print("Nand Skip Erase Failed\r\n");
        return FMSH_FAILURE;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: Erase 10MB data cost %lld us.\r\n", time);

    // write perf
    addr = TEST_ADDRESS;
    time_st = get_current_time();
    for (i = 0; i < loopcnt; i++)
    {
        ret = FNandPsu_Skip_Write(nfcPtr, addr, 0x100000, srcPtr, 0);
        if (ret != 0)
        {
            fmsh_print("ERROR: Failed to write NAND device!\r\n");
            nand_cleanup(nfcPtr);
            return 1;
        }
        addr += 0x100000;
        srcPtr += 0x100000;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: Write 10MB data cost %lld us.\r\n", time);
    fmsh_print("INFO: Write speed is %lld B/s.\r\n",
               (unsigned long long)1e7 * 1024 * 1024 / time);

    // read perf
    addr = TEST_ADDRESS;
    Fmsh_DCacheFlushRange((uintptr_t)dstPtr, size);
    time_st = get_current_time();
    for (i = 0; i < loopcnt; i++)
    {
        ret = FNandPsu_Skip_Read(nfcPtr, addr, 0x100000, dstPtr, 0);
        if (ret != 0)
        {
            fmsh_print("ERROR: Failed to read NAND device!\r\n");
            nand_cleanup(nfcPtr);
            return 1;
        }
        addr += 0x100000;
        dstPtr += 0x100000;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: Read 10MB data cost %lld us.\r\n", time);
    fmsh_print("INFO: Read speed is %lld B/s.\r\n",
               (unsigned long long)1e7 * 1024 * 1024 / time);

    nand_cleanup(nfcPtr);

    return 0;
}
