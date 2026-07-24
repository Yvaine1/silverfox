#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_spi_lib.h"
#include "fmsh_spi_eeprom.h"
#include "fmsh_spi_flash.h"

//#define SPI_NOR_EXAMPLE
//#define SPI_EEPROM_EXAMPLE
//#define SPI_LOOP_EXAMPLE
//#define SPI_INTR_EXAMPLE

#define MAX_DATA     16

#define TEST_ADDRESS 0x00000000
#define UNIQUE_VALUE 5

static FSpiPs_T spi0;
static FSpiPs_T spi1;

static u8 RecvBuffer[512];
static int intr_flags = 0, spi_flags = 0;

static u8* ReadBuffer;
static u8* WriteBuffer;

__attribute__((unused)) static int FSpiPs_nor_example (FSpiPs_T* spiPtr);
__attribute__((unused)) static int FSpiPs_eeprom_example (FSpiPs_T* spiPtr);
__attribute__((unused)) static int FSpips_IntrExample (FSpiPs_T *spi, u16 device_id);

static int spi_cleanup (FSpiPs_T* spiPtr)
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

static void FSlcrPS_setSpiLoop (int loop_en)
{
    u32 value;

    value = FMSH_ReadReg(FPS_IOU_SLCR_BASEADDR, 0x200);
    if (loop_en)
    {
        value |= 0x1;
    }
    else
    {
        value &= ~0x1;
    }
    FMSH_WriteReg(FPS_IOU_SLCR_BASEADDR, 0x200, value);
}

/******************************************************************************
 * user interrupt handler
 *
 *
 ******************************************************************************/
void FSpiPs_Handler (void *callBackRef, u32 statusEvent, u32 byteCount)
{
    __attribute__((unused))
    FSpiPs_T *spiPtr;

    spiPtr = (FSpiPs_T *)callBackRef;

    if (statusEvent == SPI_INTR_MSTIS_MASK)
    {
        intr_flags |= SPI_INTR_MSTIS_MASK;
    }

    if (statusEvent == SPI_INTR_RXOIS_MASK)
    {
        intr_flags |= SPI_INTR_RXOIS_MASK;
    }

    if (statusEvent == SPI_INTR_RXUIS_MASK)
    {
        intr_flags |= SPI_INTR_RXUIS_MASK;
    }

    if (statusEvent == SPI_INTR_TXOIS_MASK)
    {
        intr_flags |= SPI_INTR_TXOIS_MASK;
    }

    if (statusEvent == SPI_INTR_TXEIS_MASK)
    {
        intr_flags |= SPI_INTR_TXEIS_MASK;
    }

    if (statusEvent == SPI_INTR_RXFIS_MASK)
    {
        intr_flags |= SPI_INTR_RXFIS_MASK;
    }

    if (statusEvent & SPI_TRANSFER_DONE)
    {
        if (spi_flags & 0x80000000)
        {
            intr_flags |= SPI_INTR_TXEIS_MASK;
        }
    }
}

/*****************************************************************************
 * This function initializes devices
 *
 * @param
 *
 * @return
 *
 * @note		None.
 *
 ******************************************************************************/
int spi_init (FSpiPs_T *spiPtr, u16 device_id)
{
    int ret;
    int int_id;

    ret = FSpiPs_Initialize(spiPtr, device_id);
    if (ret)
    {
        return ret;
    }

    FSpiPs_Reset(spiPtr);

    FSpiPs_SetStatusHandler(spiPtr, spiPtr, FSpiPs_Handler);
    if (device_id == 0)
    {
        int_id = 51;
    }
    else
    {
        int_id = 52;
    }
    FGicPs_Connect(&IntcInstance, int_id, FSpiPs_InterruptHandler, spiPtr);
    FGicPs_Enable(&IntcInstance, int_id);

    return 0;
}

/*****************************************************************************/
/**
 *
 * This function use low level function to transfer data from spi0 to spi1.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/

SPI_USERCFG(mst_slcr_lpbk) = {
    .flags = SPI_F_MASTER | SPI_F_INTR_EN,
    .tx_empty_lvl = 10,
    .rx_full_lvl = 0,
    .cpol = 0,
    .cpha = 0,
    .frame_size = 8,
    .frame_len = 128,
    .baudrate = 1000,
    .sample_delay = 0,
};

SPI_USERCFG(slv_slcr_lpbk) = {
    .flags = SPI_F_INTR_EN,
    .tx_empty_lvl = 10,
    .rx_full_lvl = 0,
    .cpol = 0,
    .cpha = 0,
    .frame_size = 8,
    .frame_len = 128,
    .sample_delay = 0,
};

/*****************************************************************************
 * This function is interrupt test
 *
 * @param
 *
 * @return
 *
 * @note		None.
 *
 ******************************************************************************/
/* ret = FSpips_IntrExample(spi0Ptr, 0);*/
static int FSpips_IntrExample (FSpiPs_T *spi, u16 device_id)
{
    int ret, i = 0, int_id;

    spi->msg_num = 0;
    intr_flags = 0x0;

    spi_flags &= ~(u32)0x80000000;

    FSpiPs_Reset(spi);

    FSpiPs_SetStatusHandler(spi, spi, FSpiPs_Handler);
    if (device_id == 0)
    {
        int_id = 51;
    }
    else
    {
        int_id = 52;
    }
    FGicPs_Connect(&IntcInstance, int_id, FSpiPs_InterruptHandler, spi);
    FGicPs_Enable(&IntcInstance, int_id);

    // tx empty
    spi->is_busy = FALSE;
    FSpiPs_SetEnable(spi, 0);
    FSpiPs_HwInit(spi, GET_SPI_USERCFG(mst_slcr_lpbk));
    FSpiPs_SetTxEmptyLvl(spi, 4);
    FSpiPs_SetLoopBack(spi, TRUE);
    FSpiPs_SetEnable(spi, 1);
    FSpiPs_DisableIntr(spi, SPI_INTR_ALL);
    spi->is_busy = TRUE;
    spi_flags |= 0x80000000;
    FSpiPs_EnableIntr(spi, 0x01);
    FMSH_WriteReg(spi->config.base, SPI_DR_OFFSET, 0xaabbccdd);
    delay_ms(100);

    // tx overflow
    spi->is_busy = FALSE;
    FSpiPs_SetEnable(spi, 0);
    FSpiPs_HwInit(spi, GET_SPI_USERCFG(slv_slcr_lpbk));
    spi->is_busy = TRUE;
    FSpiPs_SetEnable(spi, 1);
    FSpiPs_DisableIntr(spi, SPI_INTR_ALL);
    FSpiPs_EnableIntr(spi, 0x02);
    for (i = 0; i < 100; i++)
    {
        FMSH_WriteReg(spi->config.base, SPI_DR_OFFSET, 0xaabbccdd);
    }
    delay_ms(100);

    // rx underflow
    spi->is_busy = FALSE;
    FSpiPs_SetEnable(spi, 0);
    FSpiPs_HwInit(spi, GET_SPI_USERCFG(slv_slcr_lpbk));
    spi->is_busy = TRUE;
    FSpiPs_SetEnable(spi, 1);
    FSpiPs_DisableIntr(spi, SPI_INTR_ALL);
    FSpiPs_EnableIntr(spi, 0x04);
    (void)FMSH_ReadReg(spi->config.base, SPI_DR_OFFSET);
    delay_ms(100);

    // rx overflow
    spi->is_busy = FALSE;
    FSpiPs_SetEnable(spi, 0);
    spi->cur_cs = 0x1;
    FSpiPs_HwInit(spi, GET_SPI_USERCFG(mst_slcr_lpbk));
    FSpiPs_SetLoopBack(spi, TRUE);
    spi->is_busy = TRUE;
    FSpiPs_SetEnable(spi, 1);
    FSpiPs_DisableIntr(spi, SPI_INTR_ALL);
    FSpiPs_EnableIntr(spi, 0x08);
    for (i = 0; i < 20000; i++)
    {
        FMSH_WriteReg(spi->config.base, SPI_DR_OFFSET, 0xaabbccdd);
    }
    delay_ms(100);

    // rx full
    spi->is_busy = FALSE;
    FSpiPs_SetEnable(spi, 0);
    spi->cur_cs = 0x1;
    FSpiPs_HwInit(spi, GET_SPI_USERCFG(mst_slcr_lpbk));
    FSpiPs_SetLoopBack(spi, TRUE);
    spi->is_busy = TRUE;
    FSpiPs_SetEnable(spi, 1);
    FSpiPs_SetRxFullLvl(spi, 20);
    FSpiPs_DisableIntr(spi, SPI_INTR_ALL);
    FSpiPs_EnableIntr(spi, 0x10);
    for (i = 0; i < 20000; i++)
    {
        FMSH_WriteReg(spi->config.base, SPI_DR_OFFSET, 0xaabbccdd);
    }
    delay_ms(100);

    // multi master
    FSpiPs_DisableIntr(spi, SPI_INTR_ALL);
    FSpiPs_EnableIntr(spi, 0x20);
    delay_ms(100);

    if (intr_flags != 0x1F)
    {
        fmsh_print_info(
            "-E- spi interrupt test failed, result, intrFlag = %08x \r\n",
            intr_flags);
        ret = 1;
    }
    else
    {
        fmsh_print_info("-I- spi interrupt test pass\r\n");
        ret = 0;
    }

    return ret;
}

int spi_loop_test(FSpiPs_T *spi0Ptr, FSpiPs_T *spi1Ptr)
{
    u32 data, ret;

    /* enable loop */
    FSlcrPS_setSpiLoop(TRUE);

    /* spi0 master, spi1 slaver */
    FSpiPs_Initialize(spi0Ptr, 0);
    FSpiPs_HwInit(spi0Ptr, GET_SPI_USERCFG(mst_slcr_lpbk));
    FSpiPs_SetSlave(spi0Ptr, 1);
    FSpiPs_SetEnable(spi0Ptr, 1);

    FSpiPs_Initialize(spi1Ptr, 1);
    FSpiPs_HwInit(spi1Ptr, GET_SPI_USERCFG(slv_slcr_lpbk));
    FSpiPs_SetEnable(spi1Ptr, 1);

    /* slave sends first, then master sends */
    FSpiPs_Send(spi1Ptr, 0x66);
    FSpiPs_Send(spi0Ptr, 0xaa);
    delay_ms(1);
    data = FMSH_ReadReg(spi0Ptr->config.base, SPI_DR_OFFSET);
    if (data != 0x66)
    {
        ret = FMSH_FAILURE;
    }
    data = FMSH_ReadReg(spi1Ptr->config.base, SPI_DR_OFFSET);
    if (data != 0xaa)
    {
        ret = FMSH_FAILURE;
    }

    FSpiPs_SetEnable(spi0Ptr, 0);
    FSpiPs_SetEnable(spi1Ptr, 0);

    /* disable loop */
    FSlcrPS_setSpiLoop(FALSE);
    
    return ret;
}

int FSpiPs_transfer_example (FSpiPs_T* spiPtr, int deviceId)
{
    int ret;
    FSpiPs_Config_T* configPtr;

    /*
     * Initialize the NFC driver so that it's ready to use
     */
    configPtr = FSpiPs_LookupConfig(deviceId);
    if (NULL == configPtr)
    {
        return FMSH_FAILURE;
    }
    ret = FSpiPs_CfgInitialize(spiPtr, configPtr);
    if (ret != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }

    // initialize Write and Read Buffer
    WriteBuffer = malloc(MAX_DATA+4);
    if (WriteBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        spi_cleanup(spiPtr);
        return FMSH_FAILURE;
    }
    ReadBuffer = malloc(MAX_DATA+4);
    if (ReadBuffer == NULL)
    {
        fmsh_print("Heap is not enough, WriteBuffer is NULL\r\n");
        spi_cleanup(spiPtr);
        return FMSH_FAILURE;
    }

    /*
     *   spi transfer example
     */
    
#ifdef SPI_EEPROM_EXAMPLE
    ret = FSpiPs_eeprom_example(spiPtr);
    if (ret)
    {
        spi_cleanup(spiPtr);
        return FMSH_FAILURE;
    }
#endif
    
#ifdef SPI_NOR_EXAMPLE
    ret = FSpiPs_nor_example(spiPtr);
    if (ret)
    {
        spi_cleanup(spiPtr);
        return FMSH_FAILURE;
    }
#endif

    spi_cleanup(spiPtr);
    return ret;
}

SPI_USERCFG(example)
__attribute__((aligned(4))) = {
    .flags = SPI_F_MASTER,
    .tx_empty_lvl = 10,
    .rx_full_lvl = 0,
    .cpol = 0,
    .cpha = 0,
    .frame_size = 8,
    .frame_len = 256,
    .baudrate = 100,
    .sample_delay = 0,
};

static u8 RecvBuffer[512];

static int FSpiPs_eeprom_example (FSpiPs_T* spiPtr)
{
    int ret;
    u32 count, uniqueValue;

    /*
     * reset controller
     */
    FSpiPs_Reset(spiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FSpiPs_SelfTest(spiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("SPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Initialize controller
     */
    spiPtr->cur_cs = 0x1;
    ret = FSpiPs_HwInit(spiPtr, GET_SPI_USERCFG(example));  //eeprom_test
    if (ret)
    {
        return ret;
    }
    
    FSpiPs_ReadUid(spiPtr, RecvBuffer);
    fmsh_print_info("eeprom uid="); 
    for(int i=0;i<16;i++)
    {
         fmsh_print_info("0x%02x ", RecvBuffer[i]); 
    }
    fmsh_print_info("\r\n"); 
    //read lock status
    FSpiPs_ReadLockStatus(spiPtr);

    for (uniqueValue = UNIQUE_VALUE, count = 3; count < MAX_DATA+3; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        uniqueValue++;
    }

    /*
     * Write data
     */
    ret = FSpiPs_WriteArray(spiPtr, TEST_ADDRESS, WriteBuffer, MAX_DATA);
    if (ret)
    {
        fmsh_print("SPI EEPROM Write Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * Read data
     */
    ret = FSpiPs_ReadArray(spiPtr, TEST_ADDRESS, ReadBuffer, MAX_DATA);
    if (ret)
    {
        fmsh_print("SPI EEPROM Read Failed\r\n");
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
                "SPI Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    return FMSH_SUCCESS;
}

static int FSpiPs_nor_example (FSpiPs_T* spiPtr)
{
    int ret;
    u32 count, uniqueValue;

    /*
     * reset controller
     */
    FSpiPs_Reset(spiPtr);

    /*
     * preform a self-test to check hardware build
     */
    ret = FSpiPs_SelfTest(spiPtr);
    if (ret != FMSH_SUCCESS)
    {
        fmsh_print("SPI Controller SelfTest Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Initialize controller
     */
    spiPtr->cur_cs = 0x1;
    ret = FSpiPs_HwInit(spiPtr, GET_SPI_USERCFG(example));  //nor_test
    if (ret)
    {
        return ret;
    }
    
    FSpiPs_Nor_RDID(spiPtr, RecvBuffer);
    fmsh_print_info("Nor Flash ID="); 
    for(int i=1;i<4;i++)
    {
         fmsh_print_info("0x%02x ", RecvBuffer[i]); 
    }
    fmsh_print_info("\r\n"); 
    //read lock status
    //FSpiPs_ReadLockStatus(spiPtr);

    for (uniqueValue = 0, count = 4; count < MAX_DATA + 4; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        uniqueValue++;
    }
    
    //Write Enable
    FSpiPs_Nor_WREN(spiPtr);
    
    //Chip Erase
    ret = FSpiPs_Nor_CE(spiPtr);
    if (ret)
    {
        fmsh_print("SPI NOR FLASH Erase Failed\r\n");
        return FMSH_FAILURE;
    }

    /*
     * Write data
     */
    ret = FSpiPs_Nor_PP(spiPtr, 0, WriteBuffer, MAX_DATA); //TEST_ADDRESS
    if (ret)
    {
        fmsh_print("SPI NOR FLASH Write Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * Read data
     */
    ret = FSpiPs_Nor_READ(spiPtr, 0, ReadBuffer, MAX_DATA); //TEST_ADDRESS
    if (ret)
    {
        fmsh_print("SPI EEPROM Read Failed\r\n");
        return FMSH_FAILURE;
    }
    /*
     * Setup a pointer to the start of the data that was read into the read
     * buffer and verify the data read is the data that was written
     */
    for (uniqueValue = 0, count = 4; count < MAX_DATA + 4; count++)
    {
        if (ReadBuffer[count] != (u8)(uniqueValue))
        {
            fmsh_print(
                "SPI Data Check Failed, ReadBuffer[%d] should be 0x%x, but "
                "actually is 0x%x\r\n",
                count, uniqueValue, ReadBuffer[count]);
            return FMSH_FAILURE;
        }
        uniqueValue++;
    }

    return FMSH_SUCCESS;
}

int FSpiPs_example ()
{
    u32 ret = 0;
    
    __attribute__((unused)) FSpiPs_T *spi0Ptr;
    __attribute__((unused)) FSpiPs_T *spi1Ptr;

    spi0Ptr = &spi0;
    spi1Ptr = &spi1;
    
#ifdef SPI_LOOP_EXAMPLE
    ret = spi_loop_test(spi0Ptr, spi1Ptr);
    if (ret) 
      return FMSH_FAILURE;
#endif
    
#ifdef SPI_INTR_EXAMPLE    
    ret = FSpips_IntrExample(spi0Ptr, 0);
    if (ret)
      return FMSH_FAILURE;
#endif
    
#if defined(SPI_NOR_EXAMPLE) || defined(SPI_EEPROM_EXAMPLE) 
    ret = FSpiPs_transfer_example(spi0Ptr, 0);
    if (ret)
      return FMSH_FAILURE;
#endif

    return ret;
}
