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
 * @file fmsh_qspi_example.c
 * @addtogroup qspipsu_v1_0
 * @{
 *
 * This File is used for FMZQ series MPSOC.
 * Contains example of the FQspiPsu driver.
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
 * 1. Before do QSPI_NOR_PERF test, make sure ddr is initialized successfully
 *
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdlib.h>

#include "fmsh_common.h"
#include "fmsh_xspi_lib.h"

/************************** Constant Definitions *****************************/
// #define QSPI_NOR_DIRECT_EXAMPLE
#define QSPI_NOR_ACMD_EXAMPLE
// #define QSPI_NOR_CHECK_REG
// #define QSPI_NOR_PERF
// #define QSPI_NAND_ACMD_EXAMPLE
// #define QSPI_NAND_STIG_EXAMPLE

/*
 * Flash address to which data is ot be written.
 */
#define TEST_ADDRESS    0x00900000
#define TEST_ROWADDR    0x100
#define TEST_COLUMNADDR 0
#define UNIQUE_VALUE    5

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA        2048

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
__attribute__((unused)) static int FQspiPsu_nor_direct_example(
    FQspiPsu_T* qspiPtr);
__attribute__((unused)) static int FQspiPsu_nor_acmd_example(
    FQspiPsu_T* qspiPtr);
__attribute__((unused)) static int FQspiPsu_nor_check_reg(FQspiPsu_T* qspiPtr);
__attribute__((unused)) static int FQspiPsu_nor_perf(FQspiPsu_T* qspiPtr);

__attribute__((unused)) static int FQspiPsu_nand_acmd_example(
    FQspiPsu_T* qspiPtr);
__attribute__((unused)) static int FQspiPsu_nand_stig_example(
    FQspiPsu_T* qspiPtr);

/************************** Variable Definitions *****************************/
/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
static u8* ReadBuffer;
static u8* WriteBuffer;

static FQspiPsu_T qspi;

static int qspi_cleanup (FQspiPsu_T* qspiPtr)
{
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
int FQspiPsu_example (u16 deviceId)
{
    int ret;
    FQspiPsu_T* qspiPtr;
    FQspiPsu_Config_T* configPtr;

    qspiPtr = &qspi;
    /*
     * Initialize the NFC driver so that it's ready to use
     */
    configPtr = FQspiPsu_LookupConfig(deviceId);
    if (NULL == configPtr)
    {
        return FMSH_FAILURE;
    }
    ret = FQspiPsu_CfgInitialize(qspiPtr, configPtr);
    if (ret != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    // initialize Write and Read Buffer
    WriteBuffer = malloc(MAX_DATA);
    if (WriteBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }
    ReadBuffer = malloc(MAX_DATA);
    if (ReadBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }

#ifdef QSPI_NOR_DIRECT_EXAMPLE
    /*
     *   qspi example using direct(linear) mode
     */
    ret = FQspiPsu_nor_direct_example(qspiPtr);
    if (ret)
    {
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef QSPI_NOR_ACMD_EXAMPLE
    /*
     *   qspi example using internal dma
     */
    ret = FQspiPsu_nor_acmd_example(qspiPtr);
    if (ret)
    {
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef QSPI_NOR_CHECK_REG
    /*
     *   dump qspi registers
     */
    ret = FQspiPsu_nor_check_reg(qspiPtr);
    if (ret)
    {
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef QSPI_NOR_PERF
    /*
     *   test qspi performance
     */
    ret = FQspiPsu_nor_perf(qspiPtr);
    if (ret)
    {
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef QSPI_NAND_ACMD_EXAMPLE
    /*
     *   qspi nand example using internal dma
     */
    ret = FQspiPsu_nand_acmd_example(qspiPtr);
    if (ret)
    {
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef QSPI_NAND_STIG_EXAMPLE
    /*
     *   qspi nand example using stig mode
     */
    ret = FQspiPsu_nand_stig_example(qspiPtr);
    if (ret)
    {
        qspi_cleanup(qspiPtr);
        return FMSH_FAILURE;
    }
#endif

    qspi_cleanup(qspiPtr);
    return ret;
}

QSPI_USERCFG(example)
__attribute__((aligned(4))) = {
    .flags = 0,                 // QSPI_F_INTR_EN,
    .dma_type = QSPI_MDMA,
    .ers_mode = QSPI_ERS_SE,    // standard sector erase
    .prog_mode = QSPI_PROG_PP,  // x1 program
    .read_mode = QSPI_RD_QOR,   // x4 read
    .naddrs = 4,                // 4 bytes address
};

static int FQspiPsu_nor_direct_example (FQspiPsu_T* qspiPtr)
{
    int ret;
    u32 count, uniqueValue;

    /*
     * reset controller
     */
    FQspiPsu_Reset(qspiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FQspiPsu_SelfTest(qspiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("QSPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Initialize controller
     */
    ret = FQspiPsu_Nor_Init(qspiPtr, GET_QSPI_USERCFG(example));
    if (ret)
    {
        return ret;
    }

    /*
     * Unlock flash & enable quad if needed
     * enable QSPI_CONFIG_TOUCH_NONVOLATILE in fmsh_qspips.h first
     */
    // FQspiPsu_Nor_Unlock(qspiPtr);
    FQspiPsu_Nor_EnableQuad(qspiPtr);

    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        uniqueValue++;
    }

    /*
     * Erase data
     */
    ret = FQspiPsu_Nor_Erase(qspiPtr, TEST_ADDRESS, MAX_DATA);  //
    if (ret)
    {
        fmsh_print("QSPI Erase Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Write data
     */
    ret = FQspiPsu_Nor_Write(qspiPtr, TEST_ADDRESS, MAX_DATA, WriteBuffer);
    if (ret)
    {
        fmsh_print("QSPI Direct Write Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Read data
     */
    ret = FQspiPsu_Nor_Read(qspiPtr, TEST_ADDRESS, MAX_DATA, ReadBuffer);
    if (ret)
    {
        fmsh_print("QSPI Direct Read Failed\r\n");
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
                "QSPI Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    fmsh_print("QSPI Direct Test Pass\r\n");

    return FMSH_SUCCESS;
}

static int FQspiPsu_nor_acmd_example (FQspiPsu_T* qspiPtr)
{
    int ret;
    u32 count, uniqueValue;

    /*
     * reset controller
     */
    FQspiPsu_Reset(qspiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FQspiPsu_SelfTest(qspiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("QSPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Initialize controller
     */
    ret = FQspiPsu_Nor_Init(qspiPtr, GET_QSPI_USERCFG(example));
    if (ret)
    {
        return ret;
    }

    /*
     * Unlock flash & enable quad if needed
     * enable QSPI_CONFIG_TOUCH_NONVOLATILE in fmsh_qspips.h first
     */
    // FQspiPsu_Nor_Unlock(qspiPtr);
    // FQspiPsu_Nor_EnableQuad(qspiPtr);

    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        uniqueValue++;
    }

    /*
     * Erase data
     */
    ret = FQspiPsu_Nor_Erase(qspiPtr, TEST_ADDRESS, MAX_DATA);
    if (ret)
    {
        fmsh_print("QSPI Erase Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Write data
     */
    ret = FQspiPsu_Nor_FastWrite(qspiPtr, TEST_ADDRESS, MAX_DATA, WriteBuffer);
    if (ret)
    {
        fmsh_print("QSPI ACMD Write Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * Read data
     */
    ret = FQspiPsu_Nor_FastRead(qspiPtr, TEST_ADDRESS, MAX_DATA, ReadBuffer);
    if (ret)
    {
        fmsh_print("QSPI ACMD Read Failed\r\n");
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
                "QSPI Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    fmsh_print("QSPI ACMD Test Pass\r\n");

    return FMSH_SUCCESS;
}

static int FQspiPsu_nor_check_reg (FQspiPsu_T* qspiPtr)
{
    int ret;
    u8 id[8];
    u8 sr;

    /*
     * reset controller
     */
    FQspiPsu_Reset(qspiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FQspiPsu_SelfTest(qspiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("QSPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Initialize controller
     */
    ret = FQspiPsu_Nor_Init(qspiPtr, GET_QSPI_USERCFG(example));
    if (ret)
    {
        return ret;
    }

    /*
     * check device id
     */
    FQspiPsu_Nor_ReadId(qspiPtr, (void*)id);
    fmsh_print(
        "QSPI Flash ID is 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x "
        "0x%02x\r\n",
        id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);

    /*
     * check register value
     */
    if (qspiPtr->maker == QSPI_MAKER_ID_SPANSION)
    {
        u8 sr2, bar;

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status register value is 0x%02x\r\n", sr);
        if (sr & 0x1c)
        {
            fmsh_print(
                "device is write protected, make sure unlock device before "
                "erase or write operation\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x35, &sr2, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("configure register value is 0x%02x\r\n", sr2);
        if ((sr2 & 0x2) == 0)
        {
            fmsh_print("QUAD is not enabled\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x16, &bar, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("bank address register value is 0x%02x\r\n", bar);
    }

    else if (qspiPtr->maker == QSPI_MAKER_ID_MICRON)
    {
        u8 fsr, vcr, evcr;
        u16 nvcr, ear;

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status register value is 0x%02x\r\n", sr);
        if (sr & 0x5c)
        {
            fmsh_print(
                "device is write protected, make sure unlock device before "
                "erase or write operation\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x70, &fsr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("flag statis register value is 0x%02x\r\n", fsr);

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0xB5, (u8*)&nvcr, 2);
        if (ret)
        {
            return ret;
        }
        fmsh_print("non volatile configuration register value is 0x%04x\r\n",
                   nvcr);

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x85, &vcr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("volatile configuration register value is 0x%02x\r\n", vcr);

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x65, &evcr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print(
            "enhanced volatile configuration register value is 0x%02x\r\n",
            evcr);

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0xC8, (u8*)&ear, 2);
        if (ret)
        {
            return ret;
        }
        fmsh_print("extended address register value is 0x%02x\r\n", ear);
    }

    else if (qspiPtr->maker == QSPI_MAKER_ID_MACRONIX)
    {
        u8 cfg, ear;

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status register value is 0x%02x\r\n", sr);
        if (sr & 0x3c)
        {
            fmsh_print(
                "device is write protected, make sure unlock device before "
                "erase or write operation\r\n");
        }
        if ((sr & 0x40) == 0)
        {
            fmsh_print("QUAD is not enabled\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x15, &cfg, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("configuration register value is 0x%02x\r\n", cfg);

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0xc8, &ear, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("extended address register value is 0x%02x\r\n", ear);
    }

    else if (qspiPtr->maker == QSPI_MAKER_ID_ISSI)
    {
        u8 par, ear;

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status register value is 0x%02x\r\n", sr);
        if (sr & 0x3c)
        {
            fmsh_print(
                "device is write protected, make sure unlock device before "
                "erase or write operation\r\n");
        }
        if ((sr & 0x40) == 0)
        {
            fmsh_print("QUAD is not enabled\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x61, &par, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("read parameters register value is 0x%02x\r\n", par);

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x16, &ear, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("extended address register value is 0x%02x\r\n", ear);
    }

    else if (qspiPtr->maker == QSPI_MAKER_ID_WINBOND)
    {
        u8 sr2, ear;

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status register value is 0x%02x\r\n", sr);
        if (sr & 0x3c)
        {
            fmsh_print(
                "device is write protected, make sure unlock device before "
                "erase or write operation\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x35, &sr2, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status2 register value is 0x%02x\r\n", sr2);
        if ((sr2 & 0x02) == 0)
        {
            fmsh_print("Quad is not enabled\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0xc8, &ear, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("extended address register value is 0x%02x\r\n", ear);
    }

    else if (qspiPtr->maker == QSPI_MAKER_ID_FMSH)
    {
        u8 sr2;

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x05, &sr, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status register value is 0x%02x\r\n", sr);
        if (sr & 0x1c)
        {
            fmsh_print(
                "device is write protected, make sure unlock device before "
                "erase or write operation\r\n");
        }

        ret = FQspiPsu_Nor_GetReg(qspiPtr, 0x35, &sr2, 1);
        if (ret)
        {
            return ret;
        }
        fmsh_print("status2 register value is 0x%02x\r\n", sr2);
        if ((sr2 & 0x2) == 0)
        {
            fmsh_print("Quad is not enabled\r\n");
        }
    }

    return FMSH_SUCCESS;
}

static int FQspiPsu_nor_perf (FQspiPsu_T* qspiPtr)
{
    int ret;
    u64 time_st, time_ed, time;
    unsigned int i, loopcnt, size, addr;
    unsigned char *srcPtr, *dstPtr;

    /*
     * reset controller
     */
    FQspiPsu_Reset(qspiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FQspiPsu_SelfTest(qspiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("QSPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    // initialize
    global_timer_enable();

    time_st = get_current_time();
    ret = FQspiPsu_Nor_Init(qspiPtr, GET_QSPI_USERCFG(example));
    if (ret)
    {
        fmsh_print("ERROR: Failed to initialize QSPI device!\r\n");
        return 1;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: QSPI device Initailize cost %lld us.\r\n", time);

    // prepare 10MB test data
    size = 0xa00000;
    loopcnt = size / 0x100000;
    srcPtr = (unsigned char*)0x10000000;
    dstPtr = (unsigned char*)0x18000000;

    // erase perf
    addr = TEST_ADDRESS;
    time_st = get_current_time();
    ret = FQspiPsu_Nor_Erase(qspiPtr, addr, 0x100000);
    if (ret != 0)
    {
        fmsh_print("ERROR: Failed to erase QSPI device!\r\n");
        return 1;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: Erase 10MB data cost %lld us.\r\n", time);

    // write perf
    addr = TEST_ADDRESS;
    time_st = get_current_time();
    for (i = 0; i < loopcnt; i++)
    {
        ret = FQspiPsu_Nor_FastWrite(qspiPtr, addr, 0x100000, srcPtr);
        if (ret != 0)
        {
            fmsh_print("ERROR: Failed to write QSPI device!\r\n");
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
        ret = FQspiPsu_Nor_FastRead(qspiPtr, addr, 0x100000, dstPtr);
        if (ret != 0)
        {
            fmsh_print("ERROR: Failed to read QSPI device!\r\n");
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

    return 0;
}

static int FQspiPsu_nand_acmd_example (FQspiPsu_T* qspiPtr)
{
    int ret;
    u32 count, uniqueValue;

    /*
     * reset controller
     */
    FQspiPsu_Reset(qspiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FQspiPsu_SelfTest(qspiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("QSPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Initialize controller
     */
    ret = FQspiPsu_Nand_Init(qspiPtr, GET_QSPI_USERCFG(example));
    if (ret)
    {
        return ret;
    }

    /*
     * Unlock flash & enable quad if needed
     * enable QSPI_CONFIG_TOUCH_NONVOLATILE in fmsh_qspips.h first
     */
    ret = FQspiPsu_Nand_Unlock(qspiPtr);
    ret |= FQspiPsu_Nand_EnableQuad(qspiPtr);
    if (ret)
    {
        return FMSH_FAILURE;
    }

    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        uniqueValue++;
    }

    /*
     * Erase data
     */
    ret = FQspiPsu_Nand_Erase(qspiPtr, TEST_ROWADDR, 1);
    if (ret)
    {
        fmsh_print("QSPI Erase Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Write data
     */

    ret = FQspiPsu_Nand_Write(qspiPtr, TEST_ROWADDR, TEST_COLUMNADDR, MAX_DATA,
                              WriteBuffer);
    if (ret)
    {
        fmsh_print("QSPI Write Failed\r\n");
        return FMSH_FAILURE;
    }

    ret = FQspiPsu_Nand_Read(qspiPtr, TEST_ROWADDR, TEST_COLUMNADDR, MAX_DATA,
                             ReadBuffer);
    if (ret)
    {
        fmsh_print("QSPI Read Failed\r\n");
        return FMSH_FAILURE;
    }

    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        if (ReadBuffer[count] != (u8)(uniqueValue))
        {
            fmsh_print(
                "QSPI Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    return FMSH_SUCCESS;
}

static int FQspiPsu_nand_stig_example (FQspiPsu_T* qspiPtr)
{
    int ret;
    u32 count, uniqueValue;

    /*
     * reset controller
     */
    FQspiPsu_Reset(qspiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FQspiPsu_SelfTest(qspiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("QSPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Initialize controller
     */
    ret = FQspiPsu_Nand_Init(qspiPtr, GET_QSPI_USERCFG(example));
    if (ret)
    {
        return ret;
    }

    /*
     * Unlock flash & enable quad if needed
     * enable QSPI_CONFIG_TOUCH_NONVOLATILE in fmsh_qspips.h first
     */
    ret = FQspiPsu_Nand_Unlock(qspiPtr);
    ret |= FQspiPsu_Nand_EnableQuad(qspiPtr);
    if (ret)
    {
        return FMSH_FAILURE;
    }

    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        uniqueValue++;
    }

    /*
     * Erase data
     */
    ret = FQspiPsu_Nand_Erase(qspiPtr, TEST_ROWADDR, 1);
    if (ret)
    {
        fmsh_print("QSPI Erase Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Write data
     */

    ret = FQspiPsu_Nand_P_LOAD(qspiPtr, TEST_COLUMNADDR, MAX_DATA, WriteBuffer);
    if (ret)
    {
        fmsh_print("QSPI read program load Failed\r\n");
        return FMSH_FAILURE;
    }

    FQspiPsu_Nand_WREN(qspiPtr);

    ret = FQspiPsu_Nand_P_EXEC(qspiPtr, TEST_ROWADDR);
    if (ret)
    {
        fmsh_print("QSPI nand program execute cache Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Read data
     */

    ret = FQspiPsu_Nand_PAGE_RD(qspiPtr, TEST_ROWADDR);
    if (ret)
    {
        fmsh_print("QSPI nand page read Failed\r\n");
        return FMSH_FAILURE;
    }

    ret = FQspiPsu_Nand_RD_CACHE(qspiPtr, TEST_COLUMNADDR, MAX_DATA,
                                 ReadBuffer);
    if (ret)
    {
        fmsh_print("QSPI nand read cache Failed\r\n");
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
                "QSPI Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    return FMSH_SUCCESS;
}
