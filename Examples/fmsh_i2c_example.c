/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_i2c_example.c
 *
 * This file contains a example of i2c.
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   LQ  11/23/2018  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_gic.h"
#include "fmsh_i2c_example.h"


/************************** Constant Definitions *****************************/
#define SLAVE_ADDRESS    0x5A
#define EEPROM_ADDR      0x52
#define I2C_BUF_LEN      40
#define I2C_LOOP_TIMEOUT 1000  // us

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FI2cPs_T gI2c0_dev;
FI2cPs_T gI2c1_dev;
FIicPs_Instance_T gI2c0_Instance;
FIicPs_Instance_T gI2c1_Instance;

static volatile bool s_i2cMasterWaitFlag;

static volatile bool s_i2cSlaveWaitFlag;

static u8 s_i2cMasterTxBuffer[I2C_BUF_LEN];

static u8 s_i2cSlaveRxBuffer[I2C_BUF_LEN];

/************************** Function Prototypes ******************************/

void iic0_interrupt_handler (void *InstancePtr)
{
    FI2cPs_irqHandler(&gI2c0_dev);
}

void iic1_interrupt_handler (void *InstancePtr)
{
    FI2cPs_irqHandler(&gI2c1_dev);
}

void user_iic0_interrupt_handler (void *InstancePtr)
{
    FI2cPs_userIrqHandler(&gI2c0_dev);
}

void user_iic1_interrupt_handler (void *InstancePtr)
{
    FI2cPs_userIrqHandler(&gI2c1_dev);
}

void FI2cPs_MasterCallback (void *dev, int32_t numBytes)
{
    s_i2cMasterWaitFlag = false;
}

void FI2cPs_SlaveCallback (void *dev, int32_t numBytes)
{
    s_i2cSlaveWaitFlag = false;
}

void FI2cPsu_LoopEnable ()
{
    u32 reg=0;
    reg = FMSH_ReadReg(0XFF180000,0x200);
    reg |= (1<<3);
    FMSH_WriteReg(0XFF180000, 0x200, reg);
}

void FI2cPsu_LoopDisable ()
{
    u32 reg=0;
    reg = FMSH_ReadReg(0XFF180000,0x200);
    reg &= ~(1<<3);
    FMSH_WriteReg(0XFF180000, 0x200, reg);
}

void FI2cPsu_Reset (FI2cPs_T *dev)
{
    u32 value = 0;
    if (dev->id == FPAR_I2CPS_0_DEVICE_ID)
    {
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value |= (1 << 9);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
        delay_us(5);
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value &= ~(1 << 9);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
    }
    else if (dev->id == FPAR_I2CPS_1_DEVICE_ID)
    {
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value |= (1 << 10);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
        delay_us(5);
        value = FMSH_ReadReg(0xff5e0000, 0x238);
        value &= ~(1 << 10);
        FMSH_WriteReg(0xff5e0000, 0x238, value);
    }
    else
    {
        ;
    }
}

void FI2cPs_Recovery (FI2cPs_T *dev)
{
    FI2cPs_sdaStuckRecoveryEnable(dev);
    if (FI2cPs_isSdaStuckNotRecovery(dev))
    {
        FI2cPsu_Reset(dev);
    }
}

void FI2cPs_MasterListener (void *device, int32_t ecode)
{
    // variable to store the reason(s) for transfer aborts
    FIicPs_TxAbort_T txAbort;
    FI2cPs_T *dev = device;
    // Note that the I2c_irq_tx_empty and I2c_irq_rx_done interrupts are
    // always handled internally by the driver and are never passed to
    // the listener function.  Also note that the interrupt handler is
    // responsible for the clearing of all interrupts, which it does
    // after calling the user listener function.
    switch (ecode)
    {
    case I2c_irq_rx_under:
        // Master receiver FIFO underflow.
        printf("*** Master: Rx FIFO underflow ***\n");
        // FMSH_ASSERT(false);
        break;
    case I2c_irq_rx_over:
        // Master receiver FIFO overflow.
        printf("*** Master: Rx FIFO overflow ***\n");
        // FMSH_ASSERT(false);
        break;
    case I2c_irq_rx_full:
        // i2c0_rxBuffer_idx++;
        // I2C0_RxBuffer[i2c0_rxBuffer_idx] = i2c_read(dev);
        // i2c_masterReceive(dev,I2C_Master_RxBuffer,10,test_i2c_master_callback);
        break;
    case I2c_irq_tx_over:
        // Master transmitter FIFO overflow.
        printf("*** Master: Tx FIFO overflow ***\n");
        // FMSH_ASSERT(false);
        break;
    case I2c_irq_rd_req:
        // In this example, the master device slave FSM is disabled.
        // Therefore, this interrupt should never be triggered as
        // the master device initiates all transfers.
        printf("\n*** Master: unexpected read request ***\n");
        // FMSH_ASSERT(false);
        break;
    case I2c_irq_tx_abrt:
        // Find out the reason for the transfer abort.
        txAbort = FI2cPs_getTxAbortSource(dev);
        // Print an error message with the value of the
        // tx_abort_source register.
        printf("*** Master tx abort: 0x%04x ***\n", (u16)txAbort);
        FI2cPs_Recovery(dev);
        break;
    case I2c_irq_activity:
        // This interrupt is disabled by default by the driver
        // (in the i2c_setListener function).  It is triggered
        // whenever there is activity on the I2C bus.
        break;
    case I2c_irq_stop_det:
        // This interrupt is disabled by default by the driver
        // (in the i2c_setListener function).  It is triggered
        // whenever a stop condition is detected on the I2C bus.
        break;
    case I2c_irq_start_det:
        // This interrupt is disabled by default by the driver
        // (in the i2c_setListener function).  It is triggered
        // whenever a start condition is detected on the I2C bus.
        break;
    case I2c_irq_gen_call:
        // In this example, the master device slave FSM is disabled.
        // Therefore, this interrupt should never be triggered as
        // the master device initiates all transfers.
        printf("\n*** Master: unexpected general call ***\n");
        // FMSH_ASSERT(false);
        break;
    case I2c_scl_stuck_at_low:
        // Abort tx, issue stop
        FI2cPs_masterAbort(dev);
        break;
    default:
        // Print an error message if an unexpected value is passed
        // as the argument to this function.
        printf("*** unexpected argument: 0x%x ***\n", ecode);
        printf("*** disabling master device... ");
        while (FI2cPs_disable(dev) == -FMSH_EBUSY);
        printf("***\n");
        // FMSH_ASSERT(false);
        break;
    }
}

// A user listener function for the slave device.
void FI2cPs_SlaveListener (void *device, int32_t ecode)
{
    FIicPs_TxAbort_T txAbort;
    FI2cPs_T *dev = device;

    // Note that the I2c_irq_tx_empty and I2c_irq_rx_done interrupts are
    // always handled internally by the driver and are never passed to
    // the listener function.  Also note that the interrupt handler is
    // responsible for the clearing of all interrupts, which it does
    // after calling the user listener function.
    switch (ecode)
    {
    case I2c_irq_rx_under:
        // Slave receiver FIFO underflow.
        printf("*** Slave Rx FIFO underflow ***\n");
        break;
    case I2c_irq_rx_over:
        // Slave receiver FIFO overflow.
        printf("*** Slave Rx FIFO overflow ***\n");
        break;
    case I2c_irq_rx_full:
        // In this example, the slave device is configured to use
        // the 20-byte rx_buffer for all receive transfers.  The
        // Rx full interrupt is passed to the user listener function
        // only if there is no receive buffer already set up to
        // accept data.  The example slave callback function is
        // called when 'master_writes' bytes have been received.

        break;
    case I2c_irq_tx_over:
        // Slave transmitter FIFO overflow.
        printf("*** Slave: Tx FIFO overflow ***\n");
        break;
    case I2c_irq_rd_req:
        // In this example, the slave device is configured to use
        // the 20-byte tx_buffer for all transmit transfers.  No
        // callback function is called at the end of the transfer as
        // none is specified (i.e. argument is NULL).

        break;
    case I2c_irq_tx_abrt:
        // Find out the reason for the transfer abort.
        txAbort = FI2cPs_getTxAbortSource(dev);
        // Print an error message with the value of the
        // tx_abort_source register.
        printf( "*** Slave tx abort: 0x%04x ***\n", (u16)txAbort);

        break;
    case I2c_irq_activity:
        // This interrupt is disabled by default by the driver
        // (in the i2c_setListener function).  It is triggered
        // whenever there is activity on the I2C bus.
        break;
    case I2c_irq_stop_det:
        // This interrupt is disabled by default by the driver
        // (in the i2c_setListener function).  It is triggered
        // whenever a stop condition is detected on the I2C bus.
        break;
    case I2c_irq_start_det:
        // This interrupt is disabled by default by the driver
        // (in the i2c_setListener function).  It is triggered
        // whenever a start condition is detected on the I2C bus.
        break;
    case I2c_irq_gen_call:
        // In this example, the slave device is configured to use
        // the 20-byte rx_buffer for all receive transfers.  The
        // example slave callback function is called when
        // 'master_writes' bytes have been received.

        break;
    default:
        // Print an error message if an unexpected value is passed
        // as the argument to this function.
        printf("*** unexpected argument: 0x%x ***\n", ecode);
        printf("*** disabling slave device... ");
        while (FI2cPs_disable(dev) == -FMSH_EBUSY);
        printf("***\n");
        break;
    }
}

u8 FI2c0Ps_DeviceInit (FI2cPs_T *pDev, void *pI2cInstance, void *I2cParam)
{
    u8 ret = FMSH_SUCCESS;
    FI2cPs_Config *Config = NULL;
    Config = FI2cPs_LookupConfig(FPAR_I2CPS_0_DEVICE_ID);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }
    ret = FI2cPs_init(pDev, Config, pI2cInstance, I2cParam);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }
    return ret;
}

u8 FI2c1Ps_DeviceInit (FI2cPs_T *pDev, void *pI2cInstance, void *I2cParam)
{
    u8 ret = FMSH_SUCCESS;
    FI2cPs_Config *Config = NULL;
    Config = FI2cPs_LookupConfig(FPAR_I2CPS_1_DEVICE_ID);
    if (Config == NULL)
    {
        return FMSH_FAILURE;
    }
    ret = FI2cPs_init(pDev, Config, pI2cInstance, I2cParam);
    if (ret != FMSH_SUCCESS)
    {
        return ret;
    }
    return ret;
}

void FI2cPs_MasterInit (FI2cPs_T *dev)
{
    /*disable the dev I2C device*/
    FI2cPs_disable(dev);

    FIicPs_PortMap_T *pPortmap = (FIicPs_PortMap_T *)dev->base_address;

    /* Config  */
    I2C_OUTP(0x1, pPortmap->fs_spklen);  // offset = 0xA0 IC_FS_SPKLEN
    I2C_OUTP(0xa, pPortmap->reserved1);

    /* Set up the clock count register.  The argument I2C1_CLOCK is specified as
     * the I2C dev input clock.*/
    FI2cPs_ClockSetup(dev, (dev->input_clock) / 1000000);

    /* set the speed mode to standard*/
    FI2cPs_setSpeedMode(dev, I2c_speed_standard);

    /* use 7&10-bit addressing*/
    FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
    FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);

    /* enable restart conditions*/
    FI2cPs_enableRestart(dev);

    /* enable master FSM*/
    FI2cPs_enableMaster(dev);

    /* Use the start byte protocol with the target address when initiating
     * transfer.*/
    FI2cPs_setTxMode(dev, I2c_tx_target);

    /* set target address to the I2C slave address*/
    FI2cPs_setTargetAddress(dev, SLAVE_ADDRESS);

    /*Set the user listener function*/
    FI2cPs_setListener(dev, FI2cPs_MasterListener);

    /* clear Irq */
    FI2cPs_clearIrq(dev, I2c_irq_all);

    /*enable the dev I2C device*/
    // i2c_enable(dev);
}

void FI2cPs_SlaveInit (FI2cPs_T *dev)
{
    /*disable the dev I2C device*/
    FI2cPs_disable(dev);

    FIicPs_PortMap_T *pPortmap = (FIicPs_PortMap_T *)dev->base_address;

    /* Config  */
    I2C_OUTP(0x1D7, pPortmap->reserved3[12]);  // offset = C4

    /* Set up the clock count register.  The argument I2C_CLOCK is specified as
     * the I2C dev input clock.*/
    FI2cPs_ClockSetup(dev, (dev->input_clock) / 1000000);

    /* set the speed mode to standard*/
    FI2cPs_setSpeedMode(dev, I2c_speed_standard);

    /* use 7&10-bit addressing*/
    FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
    FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);

    /* enable restart conditions*/
    FI2cPs_enableRestart(dev);

    /* enable slave FSM*/
    FI2cPs_enableSlave(dev);
    FI2cPs_disableMaster(dev);

    /* set target address to the I2C slave address*/
    FI2cPs_setSlaveAddress(dev, SLAVE_ADDRESS);

    /*Set the user listener function*/
    FI2cPs_setListener(dev, FI2cPs_SlaveListener);

    /* clear Irq */
    FI2cPs_clearIrq(dev, I2c_irq_all);
}

/******************************************************************************
 *
 * @description
 *    A example of i2c, send data.
 *
 * @param    None.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FI2cPs_example (void)
{
    u8 Status = 0;
    u32 i = 0;

    FI2cPsu_LoopEnable ();
    FI2cPs_T *pI2c0_dev = &gI2c0_dev;
    FIicPs_Instance_T *pI2c0_Instance = &gI2c0_Instance;
    FIicPs_Param_T I2c0_Param;

    FI2cPs_T *pI2c1_dev = &gI2c1_dev;
    FIicPs_Instance_T *pI2c1_Instance = &gI2c1_Instance;
    FIicPs_Param_T I2c1_Param;

    Status = FGicPs_Connect(&IntcInstance, I2C0_INT_ID,
                            (FMSH_InterruptHandler)iic0_interrupt_handler, 0);

    if (Status != GIC_SUCCESS)
    {
        printf( "Failed to generate I2C0 int.\r\n");
    }
    FGicPs_Enable(&IntcInstance, I2C0_INT_ID);

    Status = FGicPs_Connect(&IntcInstance, I2C1_INT_ID,
                            (FMSH_InterruptHandler)iic1_interrupt_handler, 0);

    if (Status != GIC_SUCCESS)
    {
        printf( "Failed to generate I2C1 int.\r\n");
    }
    FGicPs_Enable(&IntcInstance, I2C1_INT_ID);

    for (i = 0; i < I2C_BUF_LEN; i++)
    {
        s_i2cMasterTxBuffer[i] = i * 2 + 1;
        s_i2cSlaveRxBuffer[i] = 0;
    }

    /* Initial I2C0 */
    FI2c0Ps_DeviceInit(pI2c0_dev, pI2c0_Instance, &I2c0_Param);
    FI2cPs_MasterInit(pI2c0_dev);  // In default config the device as master

    /* Initial I2C1 */
    FI2c1Ps_DeviceInit(pI2c1_dev, pI2c1_Instance, &I2c1_Param);
    FI2cPs_SlaveInit(pI2c1_dev);  // In default config the device as slave

    FI2cPs_enable(pI2c1_dev);

    FI2cPs_enable(pI2c0_dev);

    /* Test Master-Transmitter & Master-Receiver */
    s_i2cMasterWaitFlag = s_i2cSlaveWaitFlag = true;

    /* Initiate the transfer. A callback function will be called when the
      last byte has been transmitted by the master device. */
    printf( "Issue Master-Transmit...\n");

    FI2cPs_slaveReceive(pI2c1_dev, s_i2cSlaveRxBuffer, I2C_BUF_LEN,
                        FI2cPs_SlaveCallback);
    FI2cPs_masterTransmit(pI2c0_dev, s_i2cMasterTxBuffer, I2C_BUF_LEN,
                          FI2cPs_MasterCallback);

    /* Wait until Master has finished the transfer */
    while (s_i2cMasterWaitFlag == true);
    printf( "Master-Transmit is done.\n");
    s_i2cMasterWaitFlag = true;

    /* Wait until the transfer has finished */
    /* The Slave_listener automatically responds to an Rx FIFO full Irq*/
    while (s_i2cSlaveWaitFlag == true);
    printf( "Slave-Receive is done.\n");
    s_i2cSlaveWaitFlag = true;

    FI2cPs_masterAbort(pI2c0_dev);
    for (i = 0; i < I2C_BUF_LEN; i++)
    {
        if (s_i2cMasterTxBuffer[i] != s_i2cSlaveRxBuffer[i])
        {
            FI2cPsu_LoopDisable ();
            return FMSH_FAILURE;
        }
    }
    FI2cPsu_LoopDisable ();
    return FMSH_SUCCESS;
}

u8 FI2cPs_EepromInit (FI2cPs_T *dev)
{
    /* Set up the clock count register.  The argument I2C1_CLOCK is
     specified as the I2C dev input clock.*/
    FI2cPs_ClockSetup(dev, (dev->input_clock) / 1000000);
    /* set the speed mode to standard*/
    FI2cPs_setSpeedMode(dev, I2c_speed_standard);
    /* use 7-bit addressing*/
    FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
    FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);
    /* enable restart conditions*/
    FI2cPs_enableRestart(dev);
    /* enable master FSM*/
    FI2cPs_enableMaster(dev);
    FI2cPs_disableSlave(dev);

    // Use the start byte protocol with the target address when
    // initiating transfer.
    FI2cPs_setTxMode(dev, I2c_tx_target);

    /* set target address to the I2C slave address*/
    FI2cPs_setTargetAddress(dev, EEPROM_ADDR);

    /*enable the dev I2C device*/
    FI2cPs_enable(dev);

    return 0;
}

static void FI2cPs_EepromByteWrite (FI2cPs_T *master, u16 iaddress, u8 byte)
{
    u8 bHaddr, bLaddr;

    bHaddr = (u8)(iaddress >> 8);
    bLaddr = (u8)iaddress;

    /* write internal address */
    FI2cPs_write(master, bHaddr);
    FI2cPs_write(master, bLaddr);

    FI2cPs_issueSTOP(master, byte);
}

static u8 FI2cPs_EepromByteRead (FI2cPs_T *master, u16 iaddress, u8 *data)
{
    u8 bHaddr, bLaddr;
    u32 timeout_cnt = I2C_LOOP_TIMEOUT;

    bHaddr = (u8)(iaddress >> 8);
    bLaddr = (u8)iaddress;

    /*dummy write*/
    FI2cPs_write(master, bHaddr);
    FI2cPs_write(master, bLaddr);
    /*Issue read*/
    FI2cPs_issueReadStop(master);

    while (FI2cPs_isRxFifoEmpty(master) == true)
    {
        delay_1us();
        timeout_cnt--;
        if (timeout_cnt == 0)
        {
            return FMSH_FAILURE;
        }
    }

    *data = FI2cPs_read(master);

    return FMSH_SUCCESS;
}

static u8 FI2cPs_EepromPageWrite (FI2cPs_T *master, u16 iaddress, u8 *buffer,
                                  u8 len)
{
    u8 bHaddr, bLaddr;
    u8 i = 0;
    u32 timeout_cnt = I2C_LOOP_TIMEOUT;

    bHaddr = (u8)(iaddress >> 8);
    bLaddr = (u8)iaddress;

    /* write internal address */
    FI2cPs_write(master, bHaddr);
    FI2cPs_write(master, bLaddr);
    for (i = 0; i < len; i++)
    {
        timeout_cnt = I2C_LOOP_TIMEOUT;
        while (FI2cPs_isTxFifoEmpty(master) != true)
        {
            delay_1us();
            timeout_cnt--;
            if (timeout_cnt == 0)
            {
                return FMSH_FAILURE;
            }
        }
        if (i == len - 1)
        {
            FI2cPs_issueSTOP(master, buffer[i]);
        }
        else
        {
            FI2cPs_write(master, buffer[i]);
        }
    }
    return FMSH_SUCCESS;
}

static u8 FI2cPs_EepromSequentialRead (FI2cPs_T *master, u16 iaddress,
                                       u8 *buffer, u8 len)
{
    u8 bHaddr, bLaddr;
    u32 i = 0;
    u32 timeout_cnt = I2C_LOOP_TIMEOUT;

    bHaddr = (u8)(iaddress >> 8);
    bLaddr = (u8)iaddress;

    /*dummy write*/
    FI2cPs_write(master, bHaddr);
    FI2cPs_write(master, bLaddr);

    /*Issue read*/
    for (i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            FI2cPs_issueReadStop(master);
        }
        else
        {
            FI2cPs_issueRead(master);
        }
        timeout_cnt = I2C_LOOP_TIMEOUT;
        while (FI2cPs_isRxFifoEmpty(master) == true)
        {
            delay_1us();
            timeout_cnt--;
            if (timeout_cnt == 0)
            {
                return FMSH_FAILURE;
            }
        }
        buffer[i] = FI2cPs_read(master);
    }

    return FMSH_SUCCESS;
}

/******************************************************************************
 *
 * @description
 *    A example of i2c write and read data from the eeprom.
 *
 * @param    None.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FI2cPs_example2 (void)
{
    u32 i = 0;
    u8 rcv_data = 0;
    u8 rcv_buffer[I2C_BUF_LEN] = {0};

    FI2cPs_T *pI2c1_dev = &gI2c1_dev;
    FIicPs_Instance_T *pI2c1_Instance = &gI2c1_Instance;
    FIicPs_Param_T I2c1_Param;

    FI2c1Ps_DeviceInit(pI2c1_dev, pI2c1_Instance, &I2c1_Param);
    FI2cPs_EepromInit(pI2c1_dev);

    for (i = 0; i < I2C_BUF_LEN; i++)
    {
        s_i2cMasterTxBuffer[i] = i * 2;
    }

    for (i = 0; i < I2C_BUF_LEN; i++)
    {
        // Write
        FI2cPs_EepromByteWrite(pI2c1_dev, i, s_i2cMasterTxBuffer[i]);
        delay_ms(20);
        // Read
        if (FI2cPs_EepromByteRead(pI2c1_dev, i, &rcv_data) != FMSH_SUCCESS)
        {
            return FMSH_FAILURE;
        }
        // check
        if (rcv_data != s_i2cMasterTxBuffer[i])
        {
            break;
        }
    }

    if (i != I2C_BUF_LEN)
    {
        return FMSH_FAILURE;
    }

    for (i = 0; i < I2C_BUF_LEN; i++)
    {
        s_i2cMasterTxBuffer[i] = i * 2 + 1;
    }

    // Write
    if (FI2cPs_EepromPageWrite(pI2c1_dev, 0, s_i2cMasterTxBuffer,
                               I2C_BUF_LEN) != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    delay_ms(20);
    // Read
    if (FI2cPs_EepromSequentialRead(pI2c1_dev, 0, rcv_buffer, I2C_BUF_LEN) !=
        FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    // check
    for (i = 0; i < I2C_BUF_LEN; i++)
    {
        if (rcv_buffer[i] != s_i2cMasterTxBuffer[i])
        {
            break;
        }
    }

    if (i != I2C_BUF_LEN)
    {
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

static u32 FI2cPs_ReadNRS1800Reg (FI2cPs_T *master, u16 ReadNum, u32 MemoryAddr)
{
    u32 i, reg = 0;
    u32 timeout_cnt = 0;
    /* --POLL-- */

    FI2cPs_write(master, 0x80 | (ReadNum >> 2) & 0xff);
    FI2cPs_write(master, (ReadNum << 6) | (MemoryAddr >> 18) & 0xff);
    FI2cPs_write(master, (MemoryAddr >> 10) & 0xff);
    FI2cPs_write(master, (MemoryAddr >> 2) & 0xff);

    for (i = 0; i < 4; i++)
    {
        if (i == 3)
        {
            FI2cPs_issueReadStop(master);
        }
        else
        {
            FI2cPs_issueRead(master);
        }
        timeout_cnt = I2C_LOOP_TIMEOUT;
        while (FI2cPs_isRxFifoEmpty(master) == true)
        {
            delay_1us();
            timeout_cnt--;
            if (timeout_cnt == 0)
            {
                return FMSH_FAILURE;
            }
        }
        reg += FI2cPs_read(master);
        if (i < 3)
        {
            reg <<= 8;
        }
    }

    return reg;
}

/******************************************************************************
 *
 * @description
 *    A example of reading NRS1800 Reg by i2c interface.
 *
 * @param    None.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FI2cPs_example3 (void)
{
    u8 Status = FMSH_SUCCESS;

    FI2cPs_T *pI2c0_dev = &gI2c0_dev;
    FIicPs_Instance_T *pI2c0_Instance = &gI2c0_Instance;
    FIicPs_Param_T I2c0_Param;

    Status = FGicPs_Connect(&IntcInstance, I2C0_INT_ID,
                            (FMSH_InterruptHandler)iic0_interrupt_handler, 0);
    if (Status != GIC_SUCCESS)
    {
        printf( "Failed to generate I2C0 int.\r\n");
        return Status;
    }
    FGicPs_Enable(&IntcInstance, I2C0_INT_ID);

    /* Initial I2C0 */
    FI2c0Ps_DeviceInit(pI2c0_dev, pI2c0_Instance, &I2c0_Param);
    FI2cPs_MasterInit(pI2c0_dev);  // In default config the device as master

    FI2cPs_enable(pI2c0_dev);

    FI2cPs_ReadNRS1800Reg(pI2c0_dev, 0x55 << 2, 0xaa << 2);

    return Status;
}
