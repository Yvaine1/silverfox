/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_timer.h
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

#include "stdbool.h"
#ifndef __FMSH_I2C_PUBLIC_H_
#define __FMSH_I2C_PUBLIC_H_

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/
#include "fmsh_i2c_common.h"

/************************** Constant Definitions *****************************/
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/**************************** Type Definitions *******************************/
typedef enum FI2cPs_irq {
    I2c_irq_none = 0x000,           // Specifies no interrupt
    I2c_irq_rx_under = 0x001,       // Set if the processor attempts to read
                                    // the receive FIFO when it is empty.
    I2c_irq_rx_over = 0x002,        // Set if the receive FIFO was
                                    // completely filled and more data
                                    // arrived.  That data is lost.
    I2c_irq_rx_full = 0x004,        // Set when the transmit FIFO reaches or
                                    // goes above the receive FIFO
                                    // threshold. It is automatically
                                    // cleared by hardware when the receive
                                    // FIFO level goes below the threshold.
    I2c_irq_tx_over = 0x008,        // Set during transmit if the transmit
                                    // FIFO is filled and the processor
                                    // attempts to issue another I2C command
                                    // (read request or write).
    I2c_irq_tx_empty = 0x010,       // Set when the transmit FIFO is at or
                                    // below the transmit FIFO threshold
                                    // level. It is automatically cleared by
                                    // hardware when the transmit FIFO level
                                    // goes above the threshold.
    I2c_irq_rd_req = 0x020,         // Set when the I2C is acting as a slave
                                    // and another I2C master is attempting
                                    // to read data from the slave.
    I2c_irq_tx_abrt = 0x040,        // In general, this is set when the I2C
                                    // acting as a master is unable to
                                    // complete a command that the processor
                                    // has sent.
    I2c_irq_rx_done = 0x080,        // When the I2C is acting as a
                                    // slave-transmitter, this is set if the
                                    // master does not acknowledge a
                                    // transmitted byte. This occurs on the
                                    // last byte of the transmission,
                                    // indicating that the transmission is
                                    // done.
    I2c_irq_activity = 0x100,       // This is set whenever the I2C is busy
                                    // (reading from or writing to the I2C
                                    // bus).
    I2c_irq_stop_det = 0x200,       // Indicates whether a stop condition
                                    // has occurred on the I2C bus.
    I2c_irq_start_det = 0x400,      // Indicates whether a start condition
                                    // has occurred on the I2C bus.
    I2c_irq_gen_call = 0x800,       // Indicates that a general call request
                                    // was received. The I2C stores the
                                    // received data in the receive FIFO.
    I2c_irq_restart_det = 0x1000,
    I2c_irq_mst_on_hold = 0x2000,
    I2c_scl_stuck_at_low = 0x4000,  //
    I2c_irq_all = 0xfff             // Specifies all I2C interrupts.  This
                                    // combined enumeration that can be
                                    // used with some functions such as
                                    // FIicPs_clearIrq(), FIicPs_maskIrq(),
                                    // and so on.
} FIicPs_Irq_T;

typedef enum FI2cPs_tx_abort {
    // Master in 7-bit address mode and the address sent was not
    // acknowledged by any slave.
    I2c_abrt_7b_addr_noack = 0x0001,
    // Master in 10-bit address mode and the first address byte of the
    // 10-bit address was not acknowledged by the slave.
    I2c_abrt_10addr1_noack = 0x0002,
    // Master in 10-bit address mode and the second address byte of the
    // 10-bit address was not acknowledged by the slave.
    I2c_abrt_10addr2_noack = 0x0004,
    // Master has received an acknowledgement for the address, but when
    // it sent data byte(s) following the address, it did not receive
    // and acknowledge from the remote slave(s).
    I2c_abrt_txdata_noack = 0x0008,
    // Master sent a general call address and no slave on the bus
    // responded with an ack.
    I2c_abrt_gcall_noack = 0x0010,
    // Master sent a general call but the user tried to issue a read
    // following this call.
    I2c_abrt_gcall_read = 0x0020,
    // Master is in high-speed mode and the high speed master code was
    // acknowledged (wrong behavior).
    I2c_abrt_hs_ackdet = 0x0040,
    // Master sent a start byte and the start byte was acknowledged
    // (wrong behavior).
    I2c_abrt_sbyte_ackdet = 0x0080,
    // The restart is disabled and the user is trying to use the master
    // to send data in high speed mode.
    I2c_abrt_hs_norstrt = 0x0100,
    // The restart is disabled and the user is trying to send a start
    // byte.
    I2c_abrt_sbyte_norstrt = 0x0200,
    // The restart is disabled and the master sends a read command in
    // the 10-bit addressing mode.
    I2c_abrt_10b_rd_norstrt = 0x0400,
    // User attempted to use disabled master.
    I2c_abrt_master_dis = 0x0800,
    // Arbitration lost.
    I2c_abrt_arb_lost = 0x1000,
    // Slave has received a read command and some data exists in the
    // transmit FIFO so that the slave issues a TX_ABRT to flush old
    // data in the transmit FIFO.
    I2c_abrt_slvflush_txfifo = 0x2000,
    // Slave lost bus while it is transmitting data to a remote master.
    I2c_abrt_slv_arblost = 0x5000,
    // Slave requests data to transfer and the user issues a read.
    I2c_abrt_slvrd_intx = 0x8000
} FIicPs_TxAbort_T;

typedef enum FI2cPs_address_mode {
    I2c_7bit_address = 0x0,  // 7-bit address mode.  Only the 7 LSBs
                             // of the slave and/or target address
                             // are relevant.
    I2c_10bit_address = 0x1  // 10-bit address mode.  The 10 LSBs of
                             // the slave and/or target address are
                             // relevant.
} FIicPs_AddressMode_T;

typedef enum FI2cPs_speed_mode {
    I2c_speed_standard = 0x1,  // standard speed (100 kbps)
    I2c_speed_fast = 0x2,      // fast speed (400 kbps)
    I2c_speed_high = 0x3       // high speed (3400 kbps)
} FIicPs_SpeedMode_T;

typedef enum FI2cPs_tx_mode {
    I2c_tx_target = 0x0,     // normal transfer using target address
    I2c_tx_gen_call = 0x2,   // issue a general call
    I2c_tx_start_byte = 0x3  // issue a start byte I2C command
} FIicPs_TxMode_T;

typedef enum FI2cPs_scl_phase {
    I2c_scl_low = 0x0,  // SCL clock count low phase
    I2c_scl_high = 0x1  // SCL clock count high phase
} FIicPs_SclPhase_T;

FI2cPs_Config *FI2cPs_LookupConfig(u16 DeviceId);

int FI2cPs_init(FI2cPs_T *dev, FI2cPs_Config *cfg, void *pI2cInstance,
                void *pI2cParam);

void FI2cPs_enable(FI2cPs_T *dev);

int FI2cPs_disable(FI2cPs_T *dev);

bool FI2cPs_isEnabled(FI2cPs_T *dev);

bool FI2cPs_isBusy(FI2cPs_T *dev);

int FI2cPs_setSpeedMode(FI2cPs_T *dev, FIicPs_SpeedMode_T mode);

FIicPs_SpeedMode_T FI2cPs_getSpeedMode(FI2cPs_T *dev);

int FI2cPs_setMasterAddressMode(FI2cPs_T *dev, FIicPs_AddressMode_T mode);

FIicPs_AddressMode_T FI2cPs_getMasterAddressMode(FI2cPs_T *dev);

int FI2cPs_setSlaveAddressMode(FI2cPs_T *dev, FIicPs_AddressMode_T mode);

FIicPs_AddressMode_T FI2cPs_getSlaveAddressMode(FI2cPs_T *dev);

int FI2cPs_enableSlave(FI2cPs_T *dev);

int FI2cPs_disableSlave(FI2cPs_T *dev);

bool FI2cPs_isSlaveEnabled(FI2cPs_T *dev);

int FI2cPs_enableMaster(FI2cPs_T *dev);

int FI2cPs_disableMaster(FI2cPs_T *dev);

bool FI2cPs_isMasterEnabled(FI2cPs_T *dev);

int FI2cPs_enableRestart(FI2cPs_T *dev);

int FI2cPs_disableRestart(FI2cPs_T *dev);

bool FI2cPs_isRestartEnabled(FI2cPs_T *dev);

int FI2cPs_setTargetAddress(FI2cPs_T *dev, uint16_t address);

uint16_t FI2cPs_getTargetAddress(FI2cPs_T *dev);

int FI2cPs_setSlaveAddress(FI2cPs_T *dev, uint16_t address);

uint16_t FI2cPs_getSlaveAddress(FI2cPs_T *dev);

int FI2cPs_setTxMode(FI2cPs_T *dev, FIicPs_TxMode_T mode);

FIicPs_TxMode_T FI2cPs_getTxMode(FI2cPs_T *dev);

int FI2cPs_setSclCount(FI2cPs_T *dev, FIicPs_SpeedMode_T mode,
                        FIicPs_SclPhase_T phase, uint16_t count);

uint16_t FI2cPs_getSclCount(FI2cPs_T *dev, FIicPs_SpeedMode_T mode,
                            FIicPs_SclPhase_T phase);

uint8_t FI2cPs_read(FI2cPs_T *dev);

void FI2cPs_write(FI2cPs_T *dev, uint8_t character);

void FI2cPs_issueRead(FI2cPs_T *dev);

void FI2cPs_issueReadStop(FI2cPs_T *dev);

FIicPs_TxAbort_T FI2cPs_getTxAbortSource(FI2cPs_T *dev);

uint16_t FI2cPs_getTxFifoDepth(FI2cPs_T *dev);

uint16_t FI2cPs_getRxFifoDepth(FI2cPs_T *dev);

bool FI2cPs_isTxFifoFull(FI2cPs_T *dev);

bool FI2cPs_isTxFifoEmpty(FI2cPs_T *dev);

bool FI2cPs_isRxFifoFull(FI2cPs_T *dev);

bool FI2cPs_isRxFifoEmpty(FI2cPs_T *dev);

uint16_t FI2cPs_getTxFifoLevel(FI2cPs_T *dev);

uint16_t FI2cPs_getRxFifoLevel(FI2cPs_T *dev);

int FI2cPs_setTxThreshold(FI2cPs_T *dev, uint8_t level);

uint8_t FI2cPs_getTxThreshold(FI2cPs_T *dev);

int FI2cPs_setRxThreshold(FI2cPs_T *dev, uint8_t level);

uint8_t FI2cPs_getRxThreshold(FI2cPs_T *dev);

void FI2cPs_setListener(FI2cPs_T *dev, FMSH_callback userFunction);

int FI2cPs_masterBack2Back(FI2cPs_T *dev, uint16_t *txBuffer,
                            uint32_t txLength, uint8_t *rxBuffer,
                            uint32_t rxLength, FMSH_callback userFunction);

int FI2cPs_masterTransmit(FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                           FMSH_callback userFunction);

int FI2cPs_slaveTransmit(FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                          FMSH_callback userFunction);

int FI2cPs_slaveBulkTransmit(FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                              FMSH_callback userFunction);

int FI2cPs_masterReceive(FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                          FMSH_callback userFunction);

int FI2cPs_slaveReceive(FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                         FMSH_callback userFunction);

int FI2cPs_terminate(FI2cPs_T *dev);

void FI2cPs_unmaskIrq(FI2cPs_T *dev, FIicPs_Irq_T interrupts);

void FI2cPs_maskIrq(FI2cPs_T *dev, FIicPs_Irq_T interrupts);

void FI2cPs_clearIrq(FI2cPs_T *dev, FIicPs_Irq_T interrupts);

bool FI2cPs_isIrqMasked(FI2cPs_T *dev, FIicPs_Irq_T interrupt);

uint32_t FI2cPs_getIrqMask(FI2cPs_T *dev);

bool FI2cPs_isIrqActive(FI2cPs_T *dev, FIicPs_Irq_T interrupt);

bool FI2cPs_isRawIrqActive(FI2cPs_T *dev, FIicPs_Irq_T interrupt);

int FI2cPs_irqHandler(FI2cPs_T *dev);

int FI2cPs_userIrqHandler(FI2cPs_T *dev);

/****************************************
 *   Added functions
 *********************************************/
void FI2cPs_issueSTOP(FI2cPs_T *dev, uint8_t character);
void FI2cPs_setACK_GenCall(FI2cPs_T *dev);
void FI2cPs_issueRESTART(FI2cPs_T *dev, uint8_t character);
uint32_t FI2cPs_clearTxAbrt(FI2cPs_T *dev);

int FI2cPs_setSCLTimeout(FI2cPs_T *dev, u32 timeout);
int FI2cPs_setSDATimeout(FI2cPs_T *dev, u32 timeout);
int FI2cPs_enableBusClear(FI2cPs_T *dev);
void FI2cPs_masterAbort(FI2cPs_T *dev);
void FI2cPs_sdaStuckRecoveryEnable(FI2cPs_T *dev);
bool FI2cPs_isSdaStuckNotRecovery(FI2cPs_T *dev);
void FI2cPs_resetRxInstance(FI2cPs_T *dev);

uint32_t FI2cPs_getRawIntrStat(FI2cPs_T *dev);

void FI2cPs_ClockSetup(FI2cPs_T *dev, float ic_clk);

#ifdef __cplusplus
}
#endif

#endif  // DW_APB_I2C_PUBLIC_H
