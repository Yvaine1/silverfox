/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gpio_pbulic.h
 *
 * This file contains public constant & function define
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

#ifndef _FMSH_GPIO_PUBLIC_H_ /* prevent circular inclusions */
#define _FMSH_GPIO_PUBLIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_gpio_common.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/* config struct */
typedef struct _gpio_config {
    u16 DeviceId;    /**< Unique ID  of device */
    u32 BaseAddress; /**< Base address of device */
    u32 MaxPin;      /**< Max pin number if a bank im MPSoC */
} FGpioPs_Config;
/**
 * DESCRIPTION
 *  This is the data type used for specifying a single bit (of a gpio
 *  port).  Functions which utilise this data type allow only a single
 *  bit of a port to be specified pre invocation.
 */
enum FGpioPs_bit {
    Gpio_bit_0 = 0,
    Gpio_bit_1 = 1,
    Gpio_bit_2 = 2,
    Gpio_bit_3 = 3,
    Gpio_bit_4 = 4,
    Gpio_bit_5 = 5,
    Gpio_bit_6 = 6,
    Gpio_bit_7 = 7,
    Gpio_bit_8 = 8,
    Gpio_bit_9 = 9,
    Gpio_bit_10 = 10,
    Gpio_bit_11 = 11,
    Gpio_bit_12 = 12,
    Gpio_bit_13 = 13,
    Gpio_bit_14 = 14,
    Gpio_bit_15 = 15,
    Gpio_bit_16 = 16,
    Gpio_bit_17 = 17,
    Gpio_bit_18 = 18,
    Gpio_bit_19 = 19,
    Gpio_bit_20 = 20,
    Gpio_bit_21 = 21,
    Gpio_bit_22 = 22,
    Gpio_bit_23 = 23,
    Gpio_bit_24 = 24,
    Gpio_bit_25 = 25,
    Gpio_bit_26 = 26,
    Gpio_bit_27 = 27,
    Gpio_bit_28 = 28,
    Gpio_bit_29 = 29,
    Gpio_bit_30 = 30,
    Gpio_bit_31 = 31
};

/**
 *  This is the data type used for specifying the data direction.
 */
enum FGpioPs_direction {
    Gpio_no_direction = -1,
    Gpio_input = 0x0,
    Gpio_output = 0x1
};

/**
 * DESCRIPTION
 *  This is a generic data type used for 1-bit wide bitfields which have
 *  a "set/clear" property.  This is used when modifying registers
 *  within a peripheral's memory map.
 */
enum FGpioPs_state {
    Gpio_high = 1,
    Gpio_low = 0,
    Gpio_err = -1,
};

/**
 *  This is the data type used for specifying the interrupt type.
 */
enum FGpioPs_irq_type {
    Gpio_no_type = -1,
    Gpio_level_sensitive = 0x0,
    Gpio_edge_sensitive = 0x1
};

/**
 *  This is the data type used for specifying the interrupt polarity.
 */
enum FGpioPs_irq_polarity {
    Gpio_no_polarity = -1,
    Gpio_active_low_falling_edge = 0x0,
    Gpio_active_high_rising_edge = 0x1
};

/***************** Macros (Inline Functions) Definitions *********************/
#define FGpioPs_read(dev)        FGpioPs_getExtPort(dev)
#define FGpioPs_write(dev, data) FGpioPs_writeData(dev, data)
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
FGpioPs_Config *FGpioPs_LookupConfig(u16 DeviceId);
u8 FGpioPs_init(FGpioPs_T *dev, FGpioPs_Config *pGpioCfg);
u8 FGpioPs_bank_init(u16 gpioDeviceId, FGpioPs_T *dev);
u8 FGpioPs_writeData(FGpioPs_T *dev, u32 data);
u8 FGpioPs_writeBit(FGpioPs_T *dev, enum FGpioPs_state value,
                    enum FGpioPs_bit bit_num);
enum FGpioPs_state FGpioPs_readBit(FGpioPs_T *dev, enum FGpioPs_bit bit_num);
u32 FGpioPs_getExtPort(FGpioPs_T *dev);

u8 FGpioPs_setDirection(FGpioPs_T *dev, u32 data);
u32 FGpioPs_getDirection(FGpioPs_T *dev);
u32 FGpioPs_setBitDirection(FGpioPs_T *dev, enum FGpioPs_bit bit_num,
                            enum FGpioPs_direction direction);
enum FGpioPs_direction FGpioPs_getBitDirection(FGpioPs_T *dev,
                                                enum FGpioPs_bit bit);

u32 FGpioPs_enableIrq(FGpioPs_T *dev, u32 interrupts);
u32 FGpioPs_disableIrq(FGpioPs_T *dev, u32 interrupts);
BOOL FGpioPs_isIrqEnabled(FGpioPs_T *dev, enum FGpioPs_bit interrupt);
u32 FGpioPs_getEnabledIrq(FGpioPs_T *dev);

u8 FGpioPs_maskIrq(FGpioPs_T *dev, u32 interrupts);
u8 FGpioPs_unmaskIrq(FGpioPs_T *dev, u32 interrupts);
BOOL FGpioPs_isIrqMasked(FGpioPs_T *dev, enum FGpioPs_bit interrupt);
u32 FGpioPs_getIrqMask(FGpioPs_T *dev);

u32 FGpioPs_setIrqType(FGpioPs_T *dev, enum FGpioPs_irq_type type,
                       u32 interrupts);
enum FGpioPs_irq_type dw_gpio_getIrqType(FGpioPs_T *dev,
                                         enum FGpioPs_bit interrupt);

u32 FGpioPs_setIrqPolarity(FGpioPs_T *dev, enum FGpioPs_irq_polarity polarity,
                           u32 interrupts);
enum FGpioPs_irq_polarity FGpioPs_getIrqPolarity(FGpioPs_T *dev,
                                                 enum FGpioPs_bit interrupt);

u32 FGpioPs_getActiveIrq(FGpioPs_T *dev);
u32 FGpioPs_clearIrq(FGpioPs_T *dev, u32 interrupts);

u32 FGpioPs_enableDebounce(FGpioPs_T *dev, u32 interrupts);
u32 FGpioPs_disableDebounce(FGpioPs_T *dev, u32 interrupts);
BOOL FGpioPs_isDebounceEnabled(FGpioPs_T *dev, enum FGpioPs_bit interrupt);

u32 FGpioPs_enableSync(FGpioPs_T *dev);
u32 FGpioPs_disableSync(FGpioPs_T *dev);
BOOL FGpioPs_isSynced(FGpioPs_T *dev);

u8 FGpioPs_enableIrqBothEdge(FGpioPs_T *dev, u32 interrupts);
u8 FGpioPs_disableIrqBothEdge(FGpioPs_T *dev, u32 interrupts);
u32 FGpioPs_getBothEdgeInt(FGpioPs_T *dev);

u32 FGpioPs_getIdCode(FGpioPs_T *dev);
u32 FGpioPs_getVerIdCode(FGpioPs_T *dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
