#include <stdlib.h>
#include "fmsh_common.h"
#include "platform.h"
#include "fpga.h"
#include "ff.h"
#include "fmsh_sac.h"
#include "boot_main.h"
#include "fmsh_gic_hw.h"
#include "fmsh_qspi.h"
#include "release_rpu.h"
#include "version_info.h"

extern FATFS fs1; 
FIL fil; 
FIL FIL_R50;
extern FDevcPs_T g_DEVC;

int update_fpga(u32 boot_addr, u32 bitSize)
{
    u32 Status = FMSH_SUCCESS;
    u32 Size = 0U;
    u32 LoadAddress=0;
    

    Size = bitSize ;
    LoadAddress = boot_addr;

    psu_ps_pl_isolation_removal_data();
    psu_ps_pl_reset_config_data();


    Status = FmshFsbl_InitDevc();
    if (Status == FMSH_FAILURE)
    {
        return FMSH_FAILURE;
    }

    Status = FDevcPs_getPlPowerStatus(&g_DEVC);
    if (Status == FMSH_FAILURE)
    {
        return FMSH_FAILURE;
    }

    FmshFsbl_OpenCfgLevelShifter();

    Status = FDevcPs_noneSecureDownload(&g_DEVC, LoadAddress, Size);
    if (Status == FMSH_FAILURE)
    {
        return FMSH_FAILURE;
    }

    // int version = FMSH_ReadReg(FPGA_BASE, 0x0);
    // fmsh_print("Fpga version is:  0x%x \r\n", version);

    return FMSH_SUCCESS;

}

int Load_PLBit(u32 load_addr, u32 *bin_size)
{
    int ret = FMSH_SUCCESS;

    Fmsh_DCacheDisable();
   
    
    ret = f_open(&fil, FILE_BIT, FA_READ);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("Fail to open file, errNum: %d\r\n", ret);
        return FMSH_FAILURE;
    }

    ret = f_read(&fil, (void *)load_addr, f_size(&fil), bin_size); 
    if (ret != FR_OK)
    {
        fmsh_print("Fail to read file, errNum: %d\r\n", ret);
        f_close(&fil);
        return FMSH_FAILURE;
    }


    Fmsh_DCacheFlushRange(load_addr, *bin_size);
    f_close(&fil);

    fmsh_print("PL addr: 0x%08X len:0x%08X \r\n", load_addr, *bin_size);
    
    Fmsh_DCacheEnable();
    // update_fpga(load_addr, *bin_size);
    return FMSH_SUCCESS;
}


typedef  void (*pFunction)(void); 
pFunction Jump_To_Application;   
volatile uint32_t JumpAddress;

void Jump_to_bootloader (u32 boot_addr)
{
  /* Reinitialize the Stack pointer and jump to application address */ 
    // JumpAddress = (load_addr);
    // Jump_To_Application = (pFunction)JumpAddress;
    /* Initialize user application's Stack Pointer */
    // Jump_To_Application(); 
    u32 HandOffAddress = boot_addr;

    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x0,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x80,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x84,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x88,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x8c,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x90,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x94,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x98,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x9c,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_DIST_BASEADDR,0x180,0x20000000);

    FMSH_WriteReg(FPAR_SCUGIC_CPU_BASEADDR,0x0,0x0);
    FMSH_WriteReg(FPAR_SCUGIC_CPU_BASEADDR,0x4,0x0);


    FmshFsbl_HandoffExit(HandOffAddress, FSBL_HANDOFFEXIT);
}

int Load_CA53(u32 load_addr)
{
    u32 Status = FMSH_SUCCESS;
    u32 bin_size = 0;
    int ret = FMSH_SUCCESS;

    Fmsh_DCacheDisable();

        
    ret = f_open(&fil, A53_FILE, FA_READ);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("Fail to open file, errNum: %d\r\n", ret);
        return FMSH_FAILURE;
    }

    ret = f_read(&fil, (void *)load_addr, f_size(&fil), &bin_size); 
    if (ret != FR_OK)
    {
        fmsh_print("Fail to read file, errNum: %d\r\n", ret);
        f_close(&fil);
        return FMSH_FAILURE;
    }
    

    Fmsh_DCacheFlushRange(load_addr, bin_size);
    f_close(&fil);

    //Jump_to_bootloader(load_addr);
    Fmsh_DCacheEnable();
    return Status;
}
extern FQspiPsu_T qspi0;

int flash_update_bootbin(u32 boot_addr, u32 bitSize)
{
    FQspiPsu_T *qspi0Ptr;
    qspi0Ptr = &qspi0;
    u32 Status = FMSH_SUCCESS;
    u8 *WriteBuffer = NULL;

    WriteBuffer=(u8 *)boot_addr;
    Status = FmshFsbl_InitQspi(0); 
    /*
     * Erase data
     */
    Status = FQspiPsu_Nor_Erase(qspi0Ptr, FLASH_BOOTBIN_START_ADDRESS, FLASH_BOOTBIN_ADDRESS+bitSize);
    if (Status)
    {
        fmsh_print("QSPI Erase Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Write data
     */
    Status = FQspiPsu_Nor_FastWrite(qspi0Ptr, FLASH_BOOTBIN_ADDRESS, bitSize, WriteBuffer);
    if (Status)
    {
        fmsh_print("QSPI ACMD Write Failed\r\n");
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print("QSPI ACMD Write Successed\r\n");
        return FMSH_SUCCESS;
    }
#if 0
    /*
     * Read data
     */
    static u8* ReadBuffer;
    ReadBuffer = malloc(bitSize);
    if (ReadBuffer == NULL)
    {
        fmsh_print("Heap is not enough, ReadBuffer is NULL\r\n");
        return FMSH_FAILURE;
    }
    Status = FQspiPsu_Nor_FastRead(qspi0Ptr, FLASH_BOOTBIN_ADDRESS, bitSize, ReadBuffer);
    if (Status)
    {
        fmsh_print("QSPI ACMD Read Failed\r\n");
        return FMSH_FAILURE;
    }
    for (int count = 0; count < bitSize; count++)
    {
        if (ReadBuffer[count] != WriteBuffer[count])
        {
            fmsh_print(
                "QSPI Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, WriteBuffer[count], ReadBuffer[count]);
            return FMSH_FAILURE;
        }
    }
#endif
}

int flash_update_bootbin2(u32 boot_addr, u32 bitSize)
{
    FQspiPsu_T *qspi0Ptr;
    qspi0Ptr = &qspi0;
    u32 Status = FMSH_SUCCESS;
    u8 *WriteBuffer = NULL;

    WriteBuffer=(u8 *)boot_addr;
    Status = FmshFsbl_InitQspi(0); 
    /*
     * Erase data
     */
    Status = FQspiPsu_Nor_Erase(qspi0Ptr, FLASH_BOOTBIN2_START_ADDRESS, bitSize);
    if (Status)
    {
        fmsh_print("QSPI Erase Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Write data
     */
    Status = FQspiPsu_Nor_FastWrite(qspi0Ptr, FLASH_BOOTBIN2_START_ADDRESS, bitSize, WriteBuffer);
    if (Status)
    {
        fmsh_print("QSPI ACMD Write Failed\r\n");
        return FMSH_FAILURE;
    }
    else
    {
        fmsh_print("QSPI ACMD Write Successed\r\n");
        return FMSH_SUCCESS;
    }

}


int Load_bootbin(u32 load_addr, u32 *bin_size)
{
    int ret = FMSH_SUCCESS;

    Fmsh_DCacheDisable();

        
    ret = f_open(&fil, FILE_BOOTBIN, FA_READ);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("Fail to open file, errNum: %d\r\n", ret);
        return FMSH_FAILURE;
    }

    ret = f_read(&fil, (void *)load_addr, f_size(&fil), bin_size); 
    if (ret != FR_OK)
    {
        fmsh_print("Fail to read file, errNum: %d\r\n", ret);
        f_close(&fil);
        return FMSH_FAILURE;
    }
    
    

    Fmsh_DCacheFlushRange(load_addr, *bin_size);
    f_close(&fil);

    // flash_update_bootbin(load_addr, *bin_size);
    Fmsh_DCacheEnable();
    return FMSH_SUCCESS;
}


int update_r5(u8 device_id, u32 load_addr)
{
    int res = 0;
    u32 rpu_tcm_size;
    u32 bin_size;
    u32 nr= 0;
    char *path = NULL;
    u32 atcm_load_addr;
    u32 btcm_load_addr;

    nr = device_id + FMZQ_CORE_RPU0;
    switch (nr) 
    {
        case FMZQ_CORE_RPU0:
            path = FILE_R50;
            atcm_load_addr = FMZQ_R5_0_TCM_START_ADDR;
            btcm_load_addr = FMZQ_R5_0_BTCM_START_ADDR;
            break;
        case FMZQ_CORE_RPU1:
            path = FILE_R51;
            atcm_load_addr = FMZQ_R5_1_TCM_START_ADDR;
            btcm_load_addr = FMZQ_R5_1_BTCM_START_ADDR;
            break;
        default:
            fmsh_print("nr is %d !\r\n", nr);
            return FMSH_FAILURE;
    }

    Fmsh_DCacheDisable();
      
    res = f_open(&fil, path, FA_READ);
    if (res != FR_OK)
    {
        fmsh_print("fail to open file, errNum: %d\r\n", res);
        return FMSH_FAILURE;
    }

    res = f_read(&fil, (void *)atcm_load_addr, RPU_TCM_SIZE, &rpu_tcm_size); 
    if (res != FR_OK)
    {
        fmsh_print("fail to read file, errNum: %d\r\n", res);
        f_close(&fil);
        return FMSH_FAILURE;
    }
    fmsh_print("read %d Byte to ATCM: 0x%08X\r\n", rpu_tcm_size, atcm_load_addr);

    res = f_read(&fil, (void *)load_addr, f_size(&fil)- RPU_TCM_SIZE, &bin_size); 
    if (res != FR_OK)
    {
        fmsh_print("fail to read file, errNum: %d\r\n", res);
        f_close(&fil);
        return FMSH_FAILURE;
    }
    fmsh_print("read %d Byte to MEM: 0x%08X\r\n", bin_size, load_addr);

    Fmsh_DCacheFlushRange(load_addr, bin_size);
    f_close(&fil);

    Fmsh_DCacheEnable();
    // cpu_release(nr, load_addr);
    return FMSH_SUCCESS;
}

static void printCode(char *name, unsigned char* addr, unsigned int size)
{
    unsigned int i;
    fmsh_print("%s Hex:",name);
    for(i=0;i<size;i++)
    {
        if((i % 16) == 0)
            fmsh_print("\n%02x: ",i);
        fmsh_print("%02x ",addr[i]);
    }
    fmsh_print("\n");

}

int read_file(char *path)
{
    u32 Status = FMSH_SUCCESS;
    u32 bin_size = 0;
    int ret = FMSH_SUCCESS;
    u32 load_addr = DDR_TEMP_ADDR;

    Fmsh_DCacheDisable();
        
    ret = f_open(&fil, path, FA_READ);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("Fail to open file, errNum: %d\r\n", ret);
        return FMSH_FAILURE;
    }

    ret = f_read(&fil, (void *)load_addr, f_size(&fil), &bin_size); 
    if (ret != FR_OK)
    {
        fmsh_print("Fail to read file, errNum: %d\r\n", ret);
        f_close(&fil);
        return FMSH_FAILURE;
    }
    

    Fmsh_DCacheFlushRange(load_addr, bin_size);
    f_close(&fil);

   // printCode(path, (unsigned char*)(load_addr), bin_size);
    printCode(path, (unsigned char*)(load_addr), 256);
    //Jump_to_bootloader(load_addr);
    Fmsh_DCacheEnable();
    return Status;
}

void show_bootbin_version()
{
    struct build_info build;
    memset(&build, 0, sizeof(struct build_info));

    strncpy(build.compile_time,COMPILE_DATE,(sizeof(build.compile_time)-1));
    strncpy(build.version_info,COMPILE_VERSION,(sizeof(build.version_info)-1));

    fmsh_print("****************BOOTBIN INFO***********\r\n");

    fmsh_print("Compile time: %s\r\n", build.compile_time);
    fmsh_print("Version: %s\r\n",build.version_info);
    fmsh_print("***************************************\r\n");
}

int pl_bitstream_parse(u8 *pBuf8) 
{
    u32 length;
    u32 swapsize = 0;
    u8 *dataptr = (u8 *)pBuf8;

    /*
    skip the first bytes of the bitsteam, their meaning is unknown
    -------
    Field 1
    2 bytes 		 length 0x0009			 (big endian)
    9 bytes 		 some sort of header
    */
    length = (*dataptr << 8) + *(dataptr + 1);
    dataptr += 2;
    dataptr += length;

    /*
    get design name (identifier, length, string)
    -------
    Field 2
    2 bytes 		 length 0x0001
    1 byte			 key 0x61				 (The letter "a")
    */
    length = (*dataptr << 8) + *(dataptr + 1);
    dataptr += 2;
    if (*dataptr++ != 0x61) /* 'a'*/
    {
        fmsh_print("Design name id not recognized in bitstream \r\n");
        return -1;
    }

    /*
    Field 3
    2 bytes 		 length 0x000a			 (value depends on file name length)
    10 bytes		 string design name "xform.ncd" (including a trailing 0x00)
    */
    length = (*dataptr << 8) + *(dataptr + 1);
    dataptr += 2;
    fmsh_print("  Design Filename = \"%s\" \r\n", dataptr);
    dataptr += length;

    /*
    get part number (identifier, length, string)
    -------
    Field 4
    1 byte			 key 0x62				 (The letter "b")
    2 bytes 		 length 0x000c			 (value depends on part name length)
    12 bytes		 string part name "v1000efg860" (including a trailing 0x00)
    */
    if (*dataptr++ != 0x62) /* 'b'*/
    {
        fmsh_print("Part number id not recognized in bitstream \r\n");
        return -1;
    }
    length = (*dataptr << 8) + *(dataptr + 1);
    dataptr += 2;
    fmsh_print("  Part Number     = \"%s\" \r\n", dataptr);
    dataptr += length;

    /*
    get date (identifier, length, string)
    -------
    Field 5
    1 byte			 key 0x63				 (The letter "c")
    2 bytes 		 length 0x000b
    11 bytes		 string date "2001/08/10"  (including a trailing 0x00)
    */
    if (*dataptr++ != 0x63) /* 'c'*/
    {
        fmsh_print("Date identifier not recognized in bitstream \r\n");
        return -1;
    }
    length = (*dataptr << 8) + *(dataptr + 1);
    dataptr += 2;
    fmsh_print("  Date            = \"%s\" \r\n", dataptr);
    dataptr += length;

    /*
    get time (identifier, length, string)
    -------
    Field 6
    1 byte			 key 0x64				 (The letter "d")
    2 bytes 		 length 0x0009
    9 bytes 		 string time "06:55:04"    (including a trailing 0x00)
    */
    if (*dataptr++ != 0x64) /* 'd'*/
    {
        fmsh_print("Time identifier not recognized in bitstream \r\n");
        return -1;
    }
    length = (*dataptr << 8) + *(dataptr + 1);
    dataptr += 2;
    fmsh_print("  Time            = \"%s\" \r\n", dataptr);
    dataptr += length;

    /*
    get fpga data length (identifier, length)
    -------
    Field 7
    1 byte			 key 0x65				  (The letter "e")
    4 bytes 		 length 0x000c9090		  (value depends on device type, and maybe design details)
    */
    if (*dataptr++ != 0x65) /* 'e'*/
    {
        fmsh_print("Data length id not recognized in bitstream \r\n");
        return -1;
    }
    swapsize =
        ((u32)*dataptr << 24) + ((u32) * (dataptr + 1) << 16) + ((u32) * (dataptr + 2) << 8) + ((u32) * (dataptr + 3));
    dataptr += 4;
    fmsh_print("  Bytes_Bitstream = %d\r\n", swapsize);

    return swapsize;
}


