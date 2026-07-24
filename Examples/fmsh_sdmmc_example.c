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
 * @file fmsh_sdmmc_example.c
 * @addtogroup sdpsu_v1_0
 * @{
 *
 * This File is used for FMZQ series MPSOC.
 * Contains example of the FSdPsu driver.
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
 * 1. Before do SDMMC_PERF test, make sure ddr is initialized successfully
 *
 */

/***************************** Include Files *********************************/
#include <stdlib.h>
#include <string.h>

#include "Fatfs15\ff.h"
#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_sdhci_lib.h"

/************************** Constant Definitions *****************************/

// #define SDMMC_NOFS_EXAMPLE
#define SDMMC_FS_EXAMPLE
// #define SDMMC_PERF

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SDHCI_DEVICE_ID 0

/*
 * Flash address to which data is ot be written.
 */
#define TEST_ADDRESS    0x00800
#define UNIQUE_VALUE    0x06

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA        2048

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
__attribute__((unused)) static int FSdPsu_nofs_example(FSdPsu_T *sdPtr);
__attribute__((unused)) static int FSdPsu_fs_example(FSdPsu_T *sdPtr);
__attribute__((unused)) static int FSdPsu_perf(FSdPsu_T *sdPtr);

__weak FRESULT f_fdisk(BYTE pdrv, const LBA_t ptbl[], void *work);

/************************** Variable Definitions *****************************/
extern FGicPs IntcInstance;
/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
static u8 *ReadBuffer;
static u8 *WriteBuffer;

static FSdPsu_T sdhci;

static u32 s_intrflags;

static FIL fil; /* File object */
static FATFS fatfs;

static int sdmmc_cleanup (FSdPsu_T *sdPtr)
{
    if (sdPtr->card)
    {
        free(sdPtr->card);
        sdPtr->card = 0;
    }

    if (sdPtr->desc)
    {
        free(sdPtr->desc);
        sdPtr->desc = 0;
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

static void FSdPsu_Handler (void *callBackRef, u32 statusEvent, u32 byteCount)
{}

static void FSdPsu_WakeupHandler (void *instancePtr)
{
    u32 status;
    FSdPsu_T *sdPtr;

    FMSH_ASSERT(instancePtr != NULL);

    sdPtr = (FSdPsu_T *)instancePtr;

    status = FMSH_ReadReg(sdPtr->config.base, SDHCI_SRS12);
    status &= (SDHCI_INT_CIN | SDHCI_INT_CR | SDHCI_INT_CINT);
    // clear interrupt status(W1C)
    FMSH_WriteReg(sdPtr->config.base, SDHCI_SRS12, status);

    if (status & SDHCI_INT_CINT)
    {
        // do nothing
    }

    // card insert
    if (status & SDHCI_INT_CIN)
    {
        // set flags and do init outside interrupt handler
    }

    // card remove
    if (status & SDHCI_INT_CR)
    {
        FSdPsu_Host_SetPower(sdPtr, SDMMC_POWER_OFF);
        sdPtr->is_inited = 0;
    }
}

static int InitSD (char *filename)
{
    FRESULT rc;
    TCHAR path[64] = ""; /* Logical drive number is 0/1 for MMC device */
    u8 workbuf[512];

    strncat(path, filename, 3);

    /* Register volume work area, initialize device */
    rc = f_mount(&fatfs, path, 1);

    if (rc == FR_NO_FILESYSTEM)
    {
        fmsh_print_err("No filesystem exist, make a new FAT32 filesystem!\r\n");

        MKFS_PARM param;

        param.fmt = FM_FAT32;

        rc = f_mkfs(path, &param, workbuf, 512);
        if (rc != FR_OK)
        {
            fmsh_print_err("Failed to mkfs, rc=%d\r\n", rc);
            return FMSH_FAILURE;
        }

        fmsh_print_info("MKFS succeed, try to mount again!\r\n");
        // unmount after format
        rc = f_mount(NULL, path, 0);
        if (rc != FR_OK)
        {
            fmsh_print_err("Failed to unmount filesystem!\r\n");
            return FMSH_FAILURE;
        }
        // mount again
        rc = f_mount(&fatfs, path, 0);
        if (rc != FR_OK)
        {
            fmsh_print_err("Failed to mount filesystem!\r\n");
            return FMSH_FAILURE;
        }
        fmsh_print_info("Mount filesystem succeed!\r\n");
    }
    else if (rc != FR_OK)
    {
        fmsh_print_err("Failed to mount filesystem, rc=%d\r\n", rc);
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print_info("Mount filesystem succeed!\r\n");
    }

    if (filename == NULL)
    {
        fmsh_print_err("Filename not specified!\r\n");
        return FMSH_FAILURE;
    }

    // create a new file
    fmsh_print_info("Open file %s!\r\n", filename);
    rc = f_open(&fil, filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if (rc != FR_OK)
    {
        fmsh_print_err("Failed to open file, rc = %d\r\n", rc);
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

static int ReleaseSD (void)
{
    FRESULT rc;

    fmsh_print_info("Close file!\r\n");

    rc = f_close(&fil);
    if (rc)
    {
        fmsh_print_err("Failed to close file!\r\n");
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

static int SDWrite (u32 offset, void *buf, int len)
{
    FRESULT rc; /* Result code*/
    UINT bw;

    rc = f_lseek(&fil, offset);
    if (rc)
    {
        fmsh_print_err("Failed to lseek.\r\n");
        return FMSH_FAILURE;
    }

    rc = f_write(&fil, buf, len, &bw);
    if (rc)
    {
        fmsh_print_err("Failed to write file, rc = 0x%x\r\n", rc);
        return FMSH_FAILURE;
    }

    rc = f_sync(&fil);
    if (rc)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

static int SDRead (u32 offset, void *buf, int len)
{
    FRESULT rc; /* Result code*/
    UINT br;

    rc = f_lseek(&fil, offset);
    if (rc)
    {
        fmsh_print_err("Failed to lseek.\r\n");
        return FMSH_FAILURE;
    }

    rc = f_read(&fil, buf, len, &br);
    if (rc)
    {
        fmsh_print_err("Failed to read file, rc = 0x%x\r\n", rc);
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

int FSdPsu_example (u16 deviceId)
{
    int ret;
    FSdPsu_T *sdPtr;
    FSdPsu_Config_T *configPtr;
    int intrId, wakeupId;

    sdPtr = &sdhci;

    configPtr = FSdPsu_LookupConfig(deviceId);
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
    // enable normal interrupt and wakeup interrupt
    if (deviceId == 0)
    {
        intrId = 80;
        wakeupId = 82;
    }
    else
    {
        intrId = 81;
        wakeupId = 83;
    }

    FGicPs_Connect(&IntcInstance, intrId, FSdPsu_InterruptHandler, sdPtr);
    FGicPs_Enable(&IntcInstance, intrId);

    FGicPs_Connect(&IntcInstance, wakeupId, FSdPsu_WakeupHandler, sdPtr);
    FGicPs_Enable(&IntcInstance, wakeupId);

    // initialize Write and Read Buffer
    WriteBuffer = malloc(MAX_DATA);
    if (WriteBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        sdmmc_cleanup(sdPtr);
        return FMSH_FAILURE;
    }
    ReadBuffer = malloc(MAX_DATA);
    if (ReadBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        sdmmc_cleanup(sdPtr);
        return FMSH_FAILURE;
    }

#ifdef SDMMC_NOFS_EXAMPLE
    ret = FSdPsu_nofs_example(sdPtr);
    if (ret)
    {
        sdmmc_cleanup(sdPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef SDMMC_FS_EXAMPLE
    ret = FSdPsu_fs_example(sdPtr);
    if (ret)
    {
        sdmmc_cleanup(sdPtr);
        return FMSH_FAILURE;
    }
#endif

#ifdef SDMMC_PERF
    ret = FSdPsu_perf(sdPtr);
    if (ret)
    {
        sdmmc_cleanup(sdPtr);
        return FMSH_FAILURE;
    }
#endif

    sdmmc_cleanup(sdPtr);
    return FMSH_SUCCESS;
}

__attribute__((unused)) SDMMC_USERCFG(sd_2_example) = {
    .flags = 0,
    .dma = SDMMC_USE_ADMA,
};

__attribute__((unused)) SDMMC_USERCFG(sd_3_example) = {
    .flags = SDMMC_F_UHS_SUPPORT,
    .dma = SDMMC_USE_ADMA,
};

static int FSdPsu_nofs_example (FSdPsu_T *sdPtr)
{
    int ret;
    u32 uniqueValue, count;
    u32 blockCount = MAX_DATA / 512;

    ret = FSdPsu_CardInit(sdPtr, GET_SDMMC_USERCFG(sd_3_example));
    if (ret)
    {
        return FMSH_FAILURE;
    }

    /*
     * Initialize the write buffer for a pattern to write to the SDMMC
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

    // multi blocks transfer
    ret = FSdPsu_Bwrite(sdPtr, TEST_ADDRESS, blockCount, WriteBuffer);
    if (ret != blockCount)
    {
        fmsh_print("SDMMC NOFS Write Failed\r\n");
        return FMSH_FAILURE;
    }

    ret = FSdPsu_Bread(sdPtr, TEST_ADDRESS, blockCount, ReadBuffer);
    if (ret != blockCount)
    {
        fmsh_print("SDMMC NOFS Read Failed\r\n");
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
                "SDMMC Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    fmsh_print("SDMMC NOFS Example Pass\r\n");
    return FMSH_SUCCESS;
}

static int FSdPsu_fs_example (FSdPsu_T *sdPtr)
{
    int ret;
    char s[80];

    if (sdPtr->config.device_id == 0)
    {
        ret = InitSD("0:/testfile.txt");
    }
    else
    {
        ret = InitSD("1:/testfile.txt");
    }
    if (ret)
    {
        return FMSH_FAILURE;
    }

    strcpy(s, "This is the testfile for sdmmc!");
    ret = SDWrite(0, s, sizeof(s));
    if (ret)
    {
        fmsh_print("SDMMC FS Write Failed\r\n");
        ReleaseSD();
        return FMSH_FAILURE;
    }

    memset(s, 0, sizeof(s));
    ret = SDRead(0, s, sizeof(s));
    if (ret)
    {
        fmsh_print("SDMMC FS Read Failed\r\n");
        ReleaseSD();
        return FMSH_FAILURE;
    }

    fmsh_print("SDMMC FS Example Pass\r\n");

    ReleaseSD();
    return FMSH_SUCCESS;
}

static int FSdPsu_perf (FSdPsu_T *sdPtr)
{
    int ret;
    u64 time_st, time_ed, time;
    unsigned int i, loopcnt, size, addr;
    unsigned char *srcPtr, *dstPtr;

    // initialize
    global_timer_enable();

    time_st = get_current_time();
    if (sdPtr->config.device_id == 0)
    {
        ret = InitSD("0:/perffile.txt");
    }
    else
    {
        ret = InitSD("1:/perffile.txt");
    }
    if (ret)
    {
        return FMSH_FAILURE;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: NAND device Initailize cost %lld us.\r\n", time);

    // prepare 10MB test data
    size = 0xa00000;
    loopcnt = size / 0x100000;
    srcPtr = (unsigned char *)0x10000000;
    dstPtr = (unsigned char *)0x18000000;

    // write perf
    addr = TEST_ADDRESS;
    time_st = get_current_time();
    for (i = 0; i < loopcnt; i++)
    {
        ret = SDWrite(0, srcPtr, 0x100000);
        if (ret)
        {
            fmsh_print("SDMMC FS Write Failed\r\n");
            ReleaseSD();
            return FMSH_FAILURE;
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
        ret = SDRead(0, dstPtr, 0x100000);
        if (ret)
        {
            fmsh_print("SDMMC FS Read Failed\r\n");
            ReleaseSD();
            return FMSH_FAILURE;
        }
        addr += 0x100000;
        dstPtr += 0x100000;
    }
    time_ed = get_current_time();
    time = (u64)((time_ed - time_st) * ((float)1e9 / GTC_FREQ) / 1000);
    fmsh_print("INFO: Read 10MB data cost %lld us.\r\n", time);
    fmsh_print("INFO: Read speed is %lld B/s.\r\n",
               (unsigned long long)1e7 * 1024 * 1024 / time);

    ReleaseSD();

    return FMSH_SUCCESS;
}

/*****************************SDFatfs multi partitions************************/

#define FWRITE_READ_BUFFER_SIZE_MAX (1 * 1024 * 1024)
#define CONFIG_VOLUMES_NUM          (4)
#define MANUAL_FORCE_FDISK_EN       0

static FATFS fatfs_s[FF_VOLUMES];
static BYTE f_mkfsBuff[FF_VOLUMES][FF_MAX_SS];
static BYTE work[FF_MAX_SS] __attribute__((aligned(64))); /* Working buffer */
static char m_dir_buffer[256] __attribute__((aligned(64)));
static TCHAR path_buf[256] = {0};

#if FF_MULTI_PARTITION
/* Volume mapping table (FF_MULTI_PARTITION == 1) */
PARTITION VolToPart[FF_VOLUMES] = {
    {0, 1}, /* "0:" ==> 1st partition in PD#0 */
    {0, 2}, /* "1:" ==> 2nd partition in PD#0 */
    {0, 3}, /* "2:" ==> 3nd partition in PD#0 */
    {0, 4}, /* "3:" ==> 4th partition in PD#0 */
    {1, 1}, /* "4:" ==> 1st partition in PD#1 */
    {1, 2}, /* "5:" ==> 2nd partition in PD#1 */
    {1, 3}, /* "6:" ==> 3nd partition in PD#1 */
    {1, 4}  /* "7:" ==> 4th partition in PD#1 */
};
#endif

static FRESULT show_partition_usage(u32 ulPhyDriveNo, u32 ulPartitionNum);

/*****************************************************************************
* u32 ulPhyDriveNo---physical drive number : 0 or 1
* u32 ulPartitionNum----logical partition number of a physical drive:1,2,3 or 4
* DWORD plist[]----List of partition size to create on the physical drive eg:
    {50, 50, 0, 0};  Divide the drive into two equal partitions
    {0x10000000,50,50,0}; 256M sectors for 1st partition and 50% of the left
        for 2nd partition and 3nd partition each
    {20, 20, 20, 0}; 20% for 3 partitions each and remaing space is left not
        allocated
    {25, 25, 25, 25}; 25% for 4 partitions each
    {100, 0, 0, 0}; only one partition,all allocated to this partition
******************************************************************************/
u32 fmsh_SdEmmcInitPartFAT32 (u32 ulPhyDriveNo, u32 ulPartitionNum,
                                     DWORD plist[])
{
    FRESULT Res = FR_OK;
    TCHAR *Path[] = {"0:", "1:", "2:", "3:",
                     "4:", "5:", "6:", "7:"}; /*Logical drive number */
    const MKFS_PARM Opt = {FM_FAT32, 0, 0, 0, 0};
    u32 i;

    if (CONFIG_VOLUMES_NUM < ulPartitionNum)
    {
        return FMSH_FAILURE;
    }

    for (i = 0; i < ulPartitionNum; i++)
    {
        /* Try to mount FAT file system */
        Res |= f_mount(&fatfs_s[i + 4 * ulPhyDriveNo],
                       Path[i + 4 * ulPhyDriveNo], 1);
    }
    if (Res != FR_OK)
    {
        fmsh_print(
            "Volume is not FAT formatted, formatting FAT32,please "
            "waiting......\r\n");
        Res = f_fdisk(ulPhyDriveNo, plist, work); /* Divide physical drive */
        if (Res != FR_OK)
        {
            fmsh_print("f_fdisk err[%d]!\r\n", Res);
            return FMSH_FAILURE;
        }
        else
        {
            fmsh_print("f_fdisk OK!\r\n");
        }

        for (i = 0; i < ulPartitionNum; i++)
        {
            /*make FAT32 fs for each partition*/
            Res = f_mkfs(Path[i + 4 * ulPhyDriveNo], &Opt,
                         f_mkfsBuff[i + 4 * ulPhyDriveNo],
                         sizeof(f_mkfsBuff[i + 4 * ulPhyDriveNo]));
            if (Res != FR_OK)
            {
                fmsh_print("Unable to format FATfs[%d],err[%d]\r\n", i, Res);
                return FMSH_FAILURE;
            }
            else
            {
                fmsh_print("partition[%d],mkfs FAT32 OK!\r\n", i);
            }

            Res = f_mount(&fatfs_s[i + 4 * ulPhyDriveNo],
                          Path[i + 4 * ulPhyDriveNo], 1);
            if (Res != FR_OK)
            {
                fmsh_print("Unable to mount FATfs[%d],err[%d]\r\n", i, Res);
                return FMSH_FAILURE;
            }
            else
            {
                fmsh_print("partition[%d],mount FAT32 OK!\r\n", i);
            }
        }
    }

    (void)show_partition_usage(ulPhyDriveNo, ulPartitionNum);
    fmsh_print("File system initialization successful\r\n");
    return FMSH_SUCCESS;
}

FRESULT scan_files (TCHAR *path)
{
    FRESULT rc; /* Result code */
    DIR dir;
    static FILINFO fno;
    UINT i;

    rc = f_opendir(&dir, path);
    if (FR_OK == rc)
    {
        for (;;)
        {
            rc = f_readdir(&dir, &fno); /* Read a directory item */
            if (rc != FR_OK || fno.fname[0] == 0)
            {
                break;                  /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)
            {                           /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                rc = scan_files(path);  /* Enter the directory */
                if (rc != FR_OK)
                {
                    break;
                }
                path[i] = 0;
            }
            else
            { /* It is a file. */
                fmsh_print(
                    "Timestamp: %u/%02u/%02u, %02u:%02u "
                    "Attributes: %c%c%c%c%c "
                    "Size: %-10lu  %s\r\n",
                    (fno.fdate >> 9) + 1980,
                    fno.fdate >> 5 & 15, fno.fdate & 31, fno.ftime >> 11,
                    fno.ftime >> 5 & 63, (fno.fattrib & AM_DIR) ? 'D' : '-',
                    (fno.fattrib & AM_RDO) ? 'R' : '-',
                    (fno.fattrib & AM_HID) ? 'H' : '-',
                    (fno.fattrib & AM_SYS) ? 'S' : '-',
                    (fno.fattrib & AM_ARC) ? 'A' : '-',
                     fno.fsize, fno.fname); 
            }
        }
        f_closedir(&dir);
    }
    else
    {
        fmsh_print("directory [%s] not exist!\r\n", path);
        return FR_NO_PATH;
    }
    return rc;
}

static FRESULT scan_dirs (TCHAR *path)
{
    FRESULT rc; /* Result code */
    DIR dir;
    static FILINFO fno;
    UINT i;

    rc = f_opendir(&dir, path);
    if (FR_OK == rc)
    {
        for (;;)
        {
            rc = f_readdir(&dir, &fno); /* Read a directory item */
            if (rc != FR_OK || fno.fname[0] == 0)
            {
                break;                  /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)
            {                           /* It is a directory */
                fmsh_print(
                    "Timestamp: %u/%02u/%02u, %02u:%02u "
                    "Attributes: %c%c%c%c%c "
                    "Size: %-10lu  %s\r\n",
                    (fno.fdate >> 9) + 1980,
                    fno.fdate >> 5 & 15, fno.fdate & 31, fno.ftime >> 11,
                    fno.ftime >> 5 & 63, (fno.fattrib & AM_DIR) ? 'D' : '-',
                    (fno.fattrib & AM_RDO) ? 'R' : '-',
                    (fno.fattrib & AM_HID) ? 'H' : '-',
                    (fno.fattrib & AM_SYS) ? 'S' : '-',
                    (fno.fattrib & AM_ARC) ? 'A' : '-',
                    fno.fsize, fno.fname);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                rc = scan_dirs(path); /* Enter the directory */
                if (rc != FR_OK)
                {
                    break;
                }
                path[i] = 0;
            }
            else
            { /* It is a file. */
            }
        }
        f_closedir(&dir);
    }
    else
    {
        fmsh_print("directory [%s] not exist!\r\n", path);
        return FR_NO_PATH;
    }
    return rc;
}

void fdisk_physicaldrive (u32 ulPhyDriveNo)
{
    DWORD force_fdisk_plist[] = {0, 0, 0, 0};
    FRESULT rc; /* Result code */

    fmsh_print("warning!!!,data on disk will lost!\r\n");
    rc = f_fdisk(ulPhyDriveNo, force_fdisk_plist,
                 work); /* Divide physical drive */
    if (rc != FR_OK)
    {
        fmsh_print("force f_fdisk err[%d]!\r\n", rc);
        return;
    }
    else
    {
        fmsh_print("force f_fdisk OK!\r\n");
        return;
    }
}


FRESULT remove_file (TCHAR *path)
{
    FRESULT rc; /* Result code */

    rc = f_unlink(path);
    if (rc != FR_OK)
    {
        fmsh_print("remove %s err[%d]!\r\n", path, rc);
    }
    else
    {
        fmsh_print("remove %s OK!\r\n", path);
    }
    return rc;
}

static FRESULT rm_all_files_in_dir (TCHAR *path)
{
    FRESULT rc; /* Result code */
    DIR dir;
    static FILINFO fno;
    UINT i;
    TCHAR *pfname_buf;
    u32 ulPathlen;

    strcpy(path_buf, path);

    rc = f_opendir(&dir, path);
    if (FR_OK == rc)
    {
        for (;;)
        {
            rc = f_readdir(&dir, &fno); /* Read a directory item */
            if (rc != FR_OK || fno.fname[0] == 0)
            {
                break;                  /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)
            {                           /* It is a directory */
                i = strlen(path_buf);
                sprintf(&path_buf[i], "/%s", fno.fname);
                rc = rm_all_files_in_dir(path_buf); /* Enter sub directory */
                if (rc != FR_OK)
                {
                    break;
                }
                path_buf[i] = 0;
            }
            else
            { /* It is a file,delete the file*/
                ulPathlen = strlen(path_buf);
                pfname_buf = strcat(path_buf, "/");
                pfname_buf = strcat(pfname_buf, fno.fname);
                rc = f_unlink(pfname_buf);
                if (rc != FR_OK)
                {
                    fmsh_print("remove %s err[%d]!\r\n", pfname_buf, rc);
                }
                else
                {
                    fmsh_print("remove %s OK!\r\n", pfname_buf);
                    path_buf[ulPathlen] = 0x0;
                }
            }
        }
        f_closedir(&dir);
    }
    else
    {
        fmsh_print("directory [%s] not exist!\r\n", path);
        return FR_NO_PATH;
    }
    return rc;
}

static FRESULT rm_all_empty_dir (TCHAR *path)
{
    FRESULT rc; /* Result code */
    DIR dir;
    static FILINFO fno;
    UINT i;
    TCHAR *pfname_buf = path_buf;

    strcpy(pfname_buf, path);

    rc = f_opendir(&dir, path);
    if (FR_OK == rc)
    {
        for (;;)
        {
            rc = f_readdir(&dir, &fno); /* Read a directory item */
            if (rc == FR_OK)
            {
                if (fno.fname[0] == 0)
                {
                    f_closedir(&dir);

                    rc = f_unlink(pfname_buf);
                    if (rc != FR_OK)
                    {
                        fmsh_print("remove %s err[%d]!\r\n", pfname_buf, rc);
                    }
                    else
                    {
                        fmsh_print("remove %s OK!\r\n", pfname_buf);
                        // path_buf[ulPathlen]= 0x0;
                    }
                    return rc;
                }
                else
                {
                    if (fno.fattrib & AM_DIR)
                    { /* It is a directory */
                        i = strlen(pfname_buf);
                        sprintf(&pfname_buf[i], "/%s", fno.fname);
                        rc = rm_all_empty_dir(
                            pfname_buf); /* Enter sub directory */
                        if (rc != FR_OK)
                        {
                            break;
                        }
                        pfname_buf[i] = 0;
                    }
                    else
                    { /* It is a file,skip*/
                        fmsh_print("directory [%s] not empty!\r\n", fno.fname);
                        return FR_DENIED;
                    }
                }
            }
            else
            {
                break; /* Break on error or end of dir */
            }
        }

        f_closedir(&dir);
    }
    else
    {
        fmsh_print("directory [%s] not exist!\r\n", path);
        return FR_NO_PATH;
    }
    return rc;
}



void show_all_file_info_of_dir (TCHAR *path)
{
    scan_files(path);
    return;
}


void show_all_dir_of_partition (TCHAR *path)
{
    scan_dirs(path);
    return;
}


void remove_all_dirs (TCHAR *path)
{
    if (FR_OK == rm_all_files_in_dir(path))
    {
        rm_all_empty_dir(path);
    }

    return;
}

__attribute__((unused))
static void remove_one_dir_or_file (TCHAR *path)
{
    FRESULT rc; /* Result code */
    rc = f_unlink(path);
    if (rc != FR_OK)
    {
        fmsh_print("remove %s err[%d]!\r\n", path, rc);
    }
    else
    {
        fmsh_print("remove %s OK!\r\n", path);
    }

    return;
}

static FRESULT show_partition_usage (u32 ulPhyDriveNo, u32 ulPartitionNum)
{
    FRESULT Res = FR_OK;
    TCHAR *Path[] = {"0:", "1:", "2:", "3:",
                     "4:", "5:", "6:", "7:"}; /*Logical drive number */
    u32 i;
    DWORD fre_clust, fre_sect, tot_sect;
    FATFS *fs;

    for (i = 0; i < ulPartitionNum; i++)
    {
        /* Get total sectors and free sectors */
        Res = f_getfree(Path[i + 4 * ulPhyDriveNo], &fre_clust, &fs);
        if (Res)
        {
            fmsh_print("f_getfree err[%d]!\r\n", Res);
            continue;
        }
        tot_sect = (fs->n_fatent - 2) * fs->csize;
        fre_sect = fre_clust * fs->csize;
        /*���sector size����512�ֽ�ʱ��ʹ��(tot_sect*fs->ssize)/ 1024,
         * (fre_sect*fs->ssize) / 1024);*/
        fmsh_print(
            "partition [%d] :%10lu KiB total space.%10lu KiB available.\r\n", i,
            tot_sect / 2, fre_sect / 2); /*sector size = 512 bytes*/
    }
    return Res;
}

__attribute__((unused))
static u32 upload_bin_to_sdmmc (char *host_file, char *dst_path)
{
    FILE *fptr = NULL;
    size_t rdsize;
    long fileLen;
    char *pbuf;

    FIL fileinst;
    FIL *fp = &fileinst;
    FRESULT rc; /* Result code */
    u32 ulbw;

    fptr = fopen(host_file, "rb");
    if (NULL != fptr)
    {
        fseek(fptr, 0, SEEK_END);
        fileLen = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        if (-1 != fileLen)
        {
            pbuf = malloc(fileLen);
            rdsize = fread(pbuf, 1, fileLen, fptr);
            if (rdsize == fileLen)
            {
                rc = f_open(fp, dst_path, FA_CREATE_ALWAYS | FA_WRITE);
                if (rc)
                {
                    fmsh_print("Unable to create file %s: %d\r\n", dst_path,
                               rc);
                    fclose(fptr);
                    free(pbuf);
                    return FMSH_FAILURE;
                }

                rc = f_write(fp, (void *)pbuf, fileLen, &ulbw);
                if (rc || (fileLen != ulbw))
                {
                    fmsh_print("update bin %s failed.\r\n", host_file);
                    fclose(fptr);
                    free(pbuf);
                    f_close(fp);
                    return FMSH_FAILURE;
                }
            }
            else
            {
                fmsh_print("read bin %s failed.\r\n", host_file);
                fclose(fptr);
                free(pbuf);
                return FMSH_FAILURE;
            }
        }
        else
        {
            fmsh_print("get bin %s length failed.\r\n", host_file);
            fclose(fptr);
            return FMSH_FAILURE;
        }
    }
    else
    {
        fmsh_print("open bin %s failed.\r\n", host_file);
        return FMSH_FAILURE;
    }

    fclose(fptr);
    free(pbuf);
    f_close(fp);
    fmsh_print("update bin %s OK!\r\n", host_file);
    return FMSH_SUCCESS;
}

__attribute__((unused)) static u32 sdmmc_wr_rd_test (u32 ulPhyDriveNo, u32 ulPartitionNum)
{
    FIL fileinst;
    FIL *fp = &fileinst;
    FRESULT rc; /* Result code */
                // u32 ulret = FMSH_SUCCESS;
    char *pwbuf = NULL;
    char *pwbuf_AlignStart;
    char *pwbuf_AlignEnd;
    char *prbuf = NULL;
    char *prbuf_AlignStart;
    char *prbuf_AlignEnd;
    u32 ulbw, ulbr;
    FSIZE_t fileLen;
    TCHAR *Path[] = {"0:", "1:", "2:", "3:",
                     "4:", "5:", "6:", "7:"}; /*Logical drive number */
    char filename[32] = {0};
    u32 i, j, k;
    u64 Pretime = 0;
    u64 Curtime = 0;
    u64 timeUsed = 0;
    FSIZE_t fp_offset;
    u32 cahcelinesize = DCACHE_LINE_SIZE;  // Fmsh_GetCacheLineSize();

    DWORD fre_clust, fre_sect;
    FATFS *fs;
    TCHAR *subdir[FF_VOLUMES] = {"/subdir0", "/subdir1", "/subdir2",
                                 "/subdir3", "/subdir4", "/subdir5",
                                 "/subdir6", "/subdir7"}; /*��Ŀ¼���� */
    FILINFO finfo;

    if (CONFIG_VOLUMES_NUM < ulPartitionNum)
    {
        return FMSH_FAILURE;
    }

    pwbuf = malloc(FWRITE_READ_BUFFER_SIZE_MAX + 2 * cahcelinesize);
    if (NULL == pwbuf)
    {
        fmsh_print("pwbuf malloc err\r\n");
        return FMSH_FAILURE;
    }

    pwbuf_AlignStart = (char *)(((long long)pwbuf + cahcelinesize) &
                                (~((long long)cahcelinesize - 1)));
    pwbuf_AlignEnd = (char *)(((long long)pwbuf_AlignStart +
                               FWRITE_READ_BUFFER_SIZE_MAX + cahcelinesize) &
                              (~((long long)cahcelinesize - 1)));

    prbuf = malloc(FWRITE_READ_BUFFER_SIZE_MAX + 2 * cahcelinesize);
    if (NULL == prbuf)
    {
        fmsh_print("prbuf malloc err\r\n");
        free(pwbuf);
        return FMSH_FAILURE;
    }

    prbuf_AlignStart = (char *)(((long long)prbuf + cahcelinesize) &
                                (~((long long)cahcelinesize - 1)));
    prbuf_AlignEnd = (char *)(((long long)prbuf_AlignStart +
                               FWRITE_READ_BUFFER_SIZE_MAX + cahcelinesize) &
                              (~((long long)cahcelinesize - 1)));

    fmsh_print("sdmmc%d FAT32 baremetal test start.....\r\n", ulPhyDriveNo);
    for (i = 0; i < 3; i++)
    {
        fmsh_print("%d round test in progress.....\r\n", i + 1);
        for (k = 0; k < ulPartitionNum; k++)
        {
            f_chdrive(Path[k + 4 * ulPhyDriveNo]); /*Change Current Drive*/
            strcpy(m_dir_buffer, subdir[k + 4 * ulPhyDriveNo]);

            if (FR_OK != f_stat(m_dir_buffer, &finfo))
            {
                rc = f_mkdir(m_dir_buffer);
                if (rc)
                {
                    free(pwbuf);
                    free(prbuf);
                    return FMSH_FAILURE;
                }
            }
            // scan_files(subdir[k]);

            if (FR_OK != f_chdir(m_dir_buffer))
            {
                free(pwbuf);
                free(prbuf);
                return FMSH_FAILURE;
            }

            for (j = 0; j < 100; j++)
            {
                /* Get volume information and free clusters of drive */
                rc = f_getfree(Path[k + 4 * ulPhyDriveNo], &fre_clust, &fs);
                if (rc)
                {
                    free(pwbuf);
                    free(prbuf);
                    return FMSH_FAILURE;
                }

                /* Get free sectors */
                fre_sect = fre_clust * fs->csize;
                if (fre_sect < (FWRITE_READ_BUFFER_SIZE_MAX /
                                512)) /*sector size = 512 bytes */
                {
                    fmsh_print(
                        "partition[%d]:left space not enough to write:%10lu "
                        "KiB available.\r\n",
                        k, fre_sect / 2);
                    break;
                }

                sprintf(filename, "test%d.bin", j);
                rc = f_open(fp, filename, FA_OPEN_APPEND | FA_READ | FA_WRITE);
                if (rc)
                {
                    fmsh_print("Unable to open file %s: %d\r\n", filename, rc);
                    free(pwbuf);
                    free(prbuf);
                    return FMSH_FAILURE;
                }

                memset(pwbuf_AlignStart, 0x5a + j + i + k,
                       FWRITE_READ_BUFFER_SIZE_MAX);
#if DCACHE_ENABLE
                Fmsh_DCacheFlushRange((long long)pwbuf_AlignStart,
                                      (long long)pwbuf_AlignEnd -
                                          (long long)pwbuf_AlignStart + 1);
#endif

                Pretime = get_current_time();
                rc = f_write(fp, (void *)pwbuf_AlignStart,
                             FWRITE_READ_BUFFER_SIZE_MAX, &ulbw);
                if (rc || (FWRITE_READ_BUFFER_SIZE_MAX != ulbw))
                {
                    fmsh_print("Write File To SD EMMC Card Failed[%d].\r\n",
                               rc);
                    f_close(fp);
                    free(pwbuf);
                    free(prbuf);
                    return FMSH_FAILURE;
                }
                // f_sync(fp);/*flush cached data into file*/

                fileLen = f_size(fp);

                Curtime = get_current_time();
                timeUsed = (Curtime - Pretime) / GTC_FREQ / 1000;
                fmsh_print(
                    "FAT32 partition[%d]: write file[%d]--size[0x%x],Use time: "
                    "%lldms,speed:%fKiB/s\r\n",
                    k, j, fileLen, timeUsed,
                    (double)(FWRITE_READ_BUFFER_SIZE_MAX) / timeUsed / 1.024);

                fp_offset = f_tell(fp);
                f_close(fp);

                rc = f_open(fp, filename, FA_READ);
                if (rc)
                {
                    fmsh_print("Unable to open file %s: %d\r\n", filename, rc);
                    free(pwbuf);
                    free(prbuf);
                    return FMSH_FAILURE;
                }

                fp_offset = fp_offset - FWRITE_READ_BUFFER_SIZE_MAX;
                rc = f_lseek(fp, fp_offset);
                if (rc)
                {
                    fmsh_print("f_lseek err Failed[%d],fp_offset:%d\r\n", rc,
                               fp_offset);
                    f_close(fp);
                    free(pwbuf);
                    free(prbuf);
                    return FMSH_FAILURE;
                }
                rc = f_read(fp, prbuf_AlignStart, ulbw, &ulbr);
                if (rc || (ulbr != ulbw))
                {
                    fmsh_print("read FAT32 partition[%d] file[%d] err\r\n", k,
                               j);
                }
#if DCACHE_ENABLE
                Fmsh_DCacheFlushRange((long long)prbuf_AlignStart,
                                      (long long)prbuf_AlignEnd -
                                          (long long)pwbuf_AlignStart + 1);
#endif
                if (0 != memcmp(prbuf_AlignStart, pwbuf_AlignStart,
                                FWRITE_READ_BUFFER_SIZE_MAX))
                {
                    fmsh_print(
                        "FAT32 partition[%d] file[%d] readback check err\r\n",
                        k, j);
                    free(pwbuf);
                    free(prbuf);
                    f_close(fp);
                    return FMSH_FAILURE;
                }
                else
                {
                    fmsh_print(
                        "FAT32 partition[%d] file[%d]--size[0x%x] readback "
                        "check OK!\r\n",
                        k, j, fileLen);
                }

                rc = f_close(fp);
                if (rc)
                {
                    free(pwbuf);
                    free(prbuf);
                    return FMSH_FAILURE;
                }
            }
        }
        fmsh_print("%d round test finish.....\r\n", i + 1);
    }
    fmsh_print("sdmmc%d FAT32 baremetal demo works good!!!\r\n", ulPhyDriveNo);
    free(pwbuf);
    free(prbuf);

    return FMSH_SUCCESS;
}

int FSdPsu_fs_multi_partitions_example ()
{
    __attribute__((unused)) u32 ulret = FMSH_SUCCESS;

    __attribute__((unused)) u64 Pretime = 0;
    __attribute__((unused)) u64 Curtime = 0;
    __attribute__((unused)) u64 timeUsed = 0;

    __attribute__((unused)) DWORD plist1[] = {0x1800000, 0, 0, 0}; /* 25% for 4 partitions each */
    __attribute__((unused)) DWORD plist2[] = {
        30, 20, 30,
        20}; /* 30% for 1st partition,20% for 2nd,30% for 3rd,20% for 4th*/

#if MANUAL_FORCE_FDISK_EN
    {
#ifdef SDMMCPS_0_DEVICE_ID
        {
            fdisk_physicaldrive(SDMMCPS_0_DEVICE_ID);
        }
#endif /* SDMMCPS_0_DEVICE_ID */

#ifdef SDMMCPS_1_DEVICE_ID
        {
            fdisk_physicaldrive(SDMMCPS_1_DEVICE_ID);
        }
#endif /* SDMMCPS_1_DEVICE_ID */

        return FMSH_SUCCESS;
    }
#endif /* MANUAL_FORCE_FDISK_EN */

#ifdef SDMMCPS_0_DEVICE_ID
    {
        Pretime = get_current_time();
        ulret = fmsh_SdEmmcInitPartFAT32(SDMMCPS_0_DEVICE_ID, 1, plist1);
        if (ulret)
        {
            fmsh_print("fmsh_SdEmmcInitPartFAT32 sdmmc0 err\r\n");
            return FMSH_FAILURE;
        }

        Curtime = get_current_time();
        timeUsed = (Curtime - Pretime) / GTC_FREQ / 1000;
        fmsh_print("sdmmc0 format FAT32 time Used:%lldms\r\n", timeUsed);
    }
#endif /* SDMMCPS_0_DEVICE_ID */

#ifdef SDMMCPS_1_DEVICE_ID
    {
        Pretime = get_current_time();
        ulret = fmsh_SdEmmcInitPartFAT32(SDMMCPS_1_DEVICE_ID, 1, plist2);
        if (ulret)
        {
            fmsh_print("fmsh_SdEmmcInitPartFAT32 sdmmc0 err\r\n");
            return FMSH_FAILURE;
        }

        Curtime = get_current_time();
        timeUsed = (Curtime - Pretime) / GTC_FREQ / 1000;
        fmsh_print("sdmmc0 format FAT32 time Used:%lldms\r\n", timeUsed);
    }
#endif /* SDMMCPS_1_DEVICE_ID */

#if 0
#ifdef SDMMCPS_0_DEVICE_ID
    if (FMSH_SUCCESS != sdmmc_wr_rd_test(SDMMCPS_0_DEVICE_ID, 1))
    {
        fmsh_print("sdmmc%d wr_rd_test err\r\n", SDMMCPS_0_DEVICE_ID);
    }
#endif

#ifdef SDMMCPS_1_DEVICE_ID
    if (FMSH_SUCCESS != sdmmc_wr_rd_test(SDMMCPS_1_DEVICE_ID, 1))
    {
        fmsh_print("sdmmc%d wr_rd_test err\r\n", SDMMCPS_1_DEVICE_ID);
    }
#endif
#endif
    // upload_bin_to_sdmmc("D:\\BOOT.bin","0:/BOOT.bin");
    return 0;
}
