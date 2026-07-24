/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_gpio.c
 *
 * This file contains all private & pbulic function for gpio.
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

/***************************** Include Files *********************************/

#include "fmsh_gpio_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/******************************************************************************
 *
 *
 * This function initial GPIO structure
 *
 * @param    dev is gpio device handle.
 * @param    addr is the base address of gpio device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u8 FGpioPs_init (FGpioPs_T *dev, FGpioPs_Config *pGpioCfg)
{
    dev->base_address = (void *)(pGpioCfg->BaseAddress);
    dev->comp_version = FGpioPs_getVerIdCode(dev);
    dev->comp_type = FGpioPs_getIdCode(dev);
    dev->max_pin = pGpioCfg->MaxPin;
    return 0;
}

u8 FGpioPs_bank_init (u16 gpioDeviceId, FGpioPs_T *dev)
{
    FGpioPs_Config *pGpioCfg;
    pGpioCfg = FGpioPs_LookupConfig(gpioDeviceId);
    FGpioPs_init(dev, pGpioCfg);

    return 0;
}

/****************************************************************************/
/**
 *
 * reset GPIO controller by reset apb interface
 *
 * @param    None.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u8 FGpioPs_reset ()
{
    u32 reg;

    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg | (1 << 18));

    delay_us(10);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg & ~(1 << 18));
    return 0;
}

/******************************************************************************
 *
 *
 * This function write data to GPIO port
 *
 * @param    dev is gpio device handle.
 * @param    data is the data of data write to GPIO port
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u8 FGpioPs_writeData (FGpioPs_T *dev, u32 data)
{
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    GPIO_OUT32P(data, portmap->swport_dr);

    return 0;
}

/******************************************************************************
 *
 *
 *  This function returns the value of external port data register.
 *
 * @param    dev is gpio device handle.
 *
 * @return    data of extern PORT data
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_getExtPort (FGpioPs_T *dev)
{
    u32 data;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    data = GPIO_IN32P(portmap->ext_port);

    return data;
}

/******************************************************************************
 *
 *  This function sets the value of a single pin of a bank to the
 *  specified value, while other pin value remains.
 * @param    dev is gpio device handle.
 * @param    value is value to set(Gpio_low/Gpio_high).
 * @param    bit_num -- the bit to change. An int value from 0 to max_pin-1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u8 FGpioPs_writeBit (FGpioPs_T *dev, enum FGpioPs_state value,
                     enum FGpioPs_bit bit_num)
{
    u32 data_in = 0;
    u32 data_out = 0;
    u32 retval = 0;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    data_in = GPIO_IN32P(portmap->swport_dr);

    switch (value)
    {
    case Gpio_low:
        data_out = data_in & ~(1 << bit_num);
        break;
    case Gpio_high:
        data_out = data_in | (1 << bit_num);
        break;
    default:
        retval = 1;
        break;
    }

    // avoid bus write if possible
    if ((data_in != data_out) && (retval == 0))
    {
        GPIO_OUT32P(data_out, portmap->swport_dr);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns the logic level of single pin of a bank.  Only
 *  one bit may be specified per invocation of this function.
 *
 * @param    dev is gpio device handle.
 * @param    bit_num -- the bit to change. An int value from 0 to max_pin-1
 *
 * @return
 *  Gpio_high      -- if the specified pin is set (0x1)
 *  Gpio_low      -- if the specified pin is clear (0x0)
 *
 * @note    None.
 *
 ******************************************************************************/
enum FGpioPs_state FGpioPs_readBit (FGpioPs_T *dev, enum FGpioPs_bit bit_num)
{
    u32 reg;
    enum FGpioPs_state retval;

    reg = FGpioPs_read(dev);

    retval = (enum FGpioPs_state)((reg & (0x1 << bit_num)) >> bit_num);

    return retval;
}

/******************************************************************************
 *
 *
 * This function set GpioX direction
 *
 * @param    dev is gpio device handle.
 *           bitX in data: 0  --  input; bitX = 1  --   output.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u8 FGpioPs_setDirection (FGpioPs_T *dev, u32 data)
{
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    GPIO_OUT32P(data, portmap->swport_ddr);

    return 0;
}

/******************************************************************************
 *
 *
 * This function get GpioX direction
 *
 * @param    dev is gpio device handle.
 *           bitX = 0  --  input; bitX = 1  --   output.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_getDirection (FGpioPs_T *dev)
{
    FGpioPs_Portmap_T *portmap;
    u32 retval;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    retval = GPIO_IN32P(portmap->swport_ddr);

    return retval;
}

/******************************************************************************
 *
 *  This function sets the direction of a single pin of a bank to the
 *  specified value, while other pin direction remains.
 *
 * @param    dev is GpioX device handle.
 * @param    bits -- port bits to change.
 * @param    direction is value to set(Gpio_input/Gpio_output).
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_setBitDirection (FGpioPs_T *dev, enum FGpioPs_bit bit_num,
                             enum FGpioPs_direction direction)
{
    FGpioPs_Portmap_T *portmap;
    u32 data_in = 0;
    u32 data_out = 0;
    u32 retval = 0;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    data_in = GPIO_IN32P(portmap->swport_ddr);

    switch (direction)
    {
    case Gpio_input:
        data_out = data_in & ~(1 << bit_num);
        break;
    case Gpio_output:
        data_out = data_in | (1 << bit_num);
        break;
    default:
        retval = 1;
        break;
    }

    // avoid bus write if possible
    if ((data_in != data_out) && (retval == 0))
    {
        GPIO_OUT32P(data_out, portmap->swport_ddr);
    }

    return retval;
}

/******************************************************************************
 *
 *  This function returns the value of a port data direction bit.  Only
 *  one bit may be specified per invocation of this function.
 *
 * @param    dev is GpioX device handle.
 * @param    bit_num -- bit to get
 *
 * @return
 *  Gpio_input          -- if the port bit is an input
 *  Gpio_output         -- if the port bit is an output
 *  Gpio_no_direction   -- if the specified port bit is not available
 *
 * @note    None.
 *
 ******************************************************************************/
enum FGpioPs_direction FGpioPs_getBitDirection (FGpioPs_T *dev,
                                                enum FGpioPs_bit bit)
{
    FGpioPs_Portmap_T *portmap;
    u32 reg;
    enum FGpioPs_direction retval;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = Gpio_no_direction;

    reg = GPIO_IN32P(portmap->swport_ddr);
    retval = (enum FGpioPs_direction)((reg & (0x1 << bit)) >> bit);

    return retval;
}

/******************************************************************************
 *
 *
 * This function enables interrupts for the specified bit(s) of port
 * Multiple bits can be specified in the function arguments, using the
 * bitwise 'OR' operator.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to enable
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_enableIrq (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;

    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->inten);
    data_out = data_in | interrupts;

    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(data_out, portmap->inten);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function disables interrupts for the specified bit(s) of port
 *  .  Multiple bits can be specified in the function arguments, using
 *  the bitwise 'OR' operator.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to enable
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_disableIrq (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;

    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->inten);
    data_out = data_in & ~interrupts;

    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(data_out, portmap->inten);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns whether interrupts are enabled for a
 *  particular bit of port or not.  Only one bit may be specified per
 *  invocation of this function.
 *
 * @param    dev is gpio device handle.
 * @param    interrupt   -- bit of port to check
 *
 * @return
 *  TRUE        -- if interrupts are enabled for the specified bit
 *  FALSE       -- if interrupts are disabled for the specified bit
 *
 * @note    None.
 *
 ******************************************************************************/
BOOL FGpioPs_isIrqEnabled (FGpioPs_T *dev, enum FGpioPs_bit interrupt)
{
    BOOL retval;
    uint32_t reg;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = FALSE;
    reg = GPIO_IN32P(portmap->inten);
    if ((reg & (0x1 << interrupt)) != 0x0)
    {
        retval = TRUE;
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns the value of the interrupt enable register.
 *
 * @param    dev is gpio device handle.
 *
 * @return
 *     The value of the interrupt enable register.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_getEnabledIrq (FGpioPs_T *dev)
{
    uint32_t retval;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = GPIO_IN32P(portmap->inten);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function masks interrupts for the specified bit(s) of port.
 *  Multiple bits can be specified in the function arguments, using the
 *  bitwise 'OR' operator.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to mask
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u8 FGpioPs_maskIrq (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->intmask);
    data_out = data_in | interrupts;

    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(data_out, portmap->intmask);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function unmasks interrupts for the specified bit(s) of port.
 *  Multiple bits can be specified in the function arguments, using the
 *  bitwise 'OR' operator.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to unmask
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u8 FGpioPs_unmaskIrq (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->intmask);
    data_out = data_in & ~interrupts;

    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(data_out, portmap->intmask);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns whether interrupts are masked for a
 *  particular bit of port or not.  Only one bit may be specified per
 *  invocation of this function.
 *
 * @param    dev is gpio device handle.
 * @param    interrupt is bit of port A to check
 *
 * @return
 *  TRUE        -- if interrupts are masked for the specified bit
 *  FALSE       -- if interrupts are not masked for the specified bit
 *
 * @note    None.
 *
 ******************************************************************************/
BOOL FGpioPs_isIrqMasked (FGpioPs_T *dev, enum FGpioPs_bit interrupt)
{
    BOOL retval;
    u32 reg;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = FALSE;
    reg = GPIO_IN32P(portmap->intmask);
    if ((reg & (0x1 << interrupt)) != 0x0)
    {
        retval = TRUE;
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns the value of the interrupt mask register.
 *
 * @param    dev is gpio device handle.
 *
 * @return
 *  The interrupt mask register value.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_getIrqMask (FGpioPs_T *dev)
{
    uint32_t retval;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = GPIO_IN32P(portmap->intmask);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function sets the interrupt type for the specified bit(s) of
 *  port.  There are two types of interrupts available,
 *  level-sensitive and edge-sensitive.  The polarity of these interrupt
 *  types is set using dw_gpio_setIrqPolarity().  Multiple bits can be
 *  specified in the function arguments, using the bitwise 'OR'
 *  operator.
 *
 * @param    dev is gpio device handle.
 * @param    type -- interrupt type to set
 * @param    interrupts  -- interrupt bit(s) to set
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_setIrqType (FGpioPs_T *dev, enum FGpioPs_irq_type type,
                        u32 interrupts)
{
    int retval;
    u32 data_in;
    u32 data_out;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->inttype_level);

    switch (type)
    {
    case Gpio_level_sensitive:
        data_out = data_in & ~interrupts;
        break;
    case Gpio_edge_sensitive:
        data_out = data_in | interrupts;
        break;
    default:
        retval = 1;
        break;
    }

    // avoid bus write if possible
    if ((data_in != data_out) && (retval == 0))
    {
        GPIO_OUT32P(data_out, portmap->inttype_level);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns the interrupt type for a specified but of
 *  port .  Only one bit may be specified per invocation of this
 *  function.
 *
 * @param    dev is gpio device handle.
 * @param    interrupt  -- bit of port to check
 *
 * @return
 *  Gpio_level_sensitive    -- if the interrupt bit is level-sensitive
 *  Gpio_edge_sensitive     -- if the interrupt bit is edge-sensitive
 *  Gpio_no_type            -- if interrupts are not supported or the
 *                             specified bit is not available
 *
 * @note    None.
 *
 ******************************************************************************/
enum FGpioPs_irq_type dw_gpio_getIrqType (FGpioPs_T *dev,
                                          enum FGpioPs_bit interrupt)
{
    enum FGpioPs_irq_type retval;
    u32 reg;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    reg = GPIO_IN32P(portmap->inttype_level);
    retval = (enum FGpioPs_irq_type)((reg & (0x1 << interrupt)) >> interrupt);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function sets the interrupt polarity for the specified bit(s)
 *  of port.  The polarity can be either active-low or active-high.
 *  For edge-sensitive interrupts, active-low corresponds to a
 *  falling-edge interrupt while active-high corresponds to a
 *  rising-edge interrupt.  The interrupt type is set using
 *  FGpioPs_setIrqType().  Multiple bits can be specified in the
 *  function arguments, using the bitwise 'OR' operator.
 *
 * @param  dev is gpio device handle.
 * @param  polarity    -- interrupt polarity to set
 * @param  interrupts  -- interrupt bit(s) to set
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_setIrqPolarity (FGpioPs_T *dev, enum FGpioPs_irq_polarity polarity,
                            u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->int_polarity);

    switch (polarity)
    {
    case Gpio_active_low_falling_edge:
        data_out = data_in & ~interrupts;
        break;
    case Gpio_active_high_rising_edge:
        data_out = data_in | interrupts;
        break;
    default:
        retval = 1;
        break;
    }

    // avoid bus write if possible
    if ((data_in != data_out) && (retval == 0))
    {
        GPIO_OUT32P(data_out, portmap->int_polarity);
    }

    return retval;
}

/******************************************************************************
*
*
*  This function returns the interrupt polarity of a specified bit of
*  . Only one bit may be specified per invocation of this function.
*
* @param  dev is gpio device handle.
* @param  polarity    -- interrupt polarity to set
* @param  interrupts  -- bit of port to check
*
* @return
* Gpio_active_low     -- if interrupt bit is active-low/falling-edge
* Gpio_active_high    -- if interrupt bit is active-high/rising-edge
* Gpio_no_polarity    -- if interrupts are not supported

* @note    None.
*
******************************************************************************/
enum FGpioPs_irq_polarity FGpioPs_getIrqPolarity (FGpioPs_T *dev,
                                                  enum FGpioPs_bit interrupt)
{
    enum FGpioPs_irq_polarity retval;
    uint32_t reg;

    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    reg = GPIO_IN32P(portmap->int_polarity);
    retval = (enum FGpioPs_irq_polarity)((reg & (0x1 << interrupt)) >>
                                         interrupt);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns the value of the interrupt status register.
 *
 * @param    dev is gpio device handle.
 *
 * @return   The interrupt status register value.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_getActiveIrq (FGpioPs_T *dev)
{
    u32 retval;

    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    retval = GPIO_IN32P(portmap->intstatus);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function is used to clear edge-sensitive interrupts of port.
 *  Multiple bits can be specified in the function arguments, using the
 *  bitwise 'OR' operator.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts is interrupt bit(s) to clear
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_clearIrq (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;

    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    GPIO_OUT32P(interrupts, portmap->port_eoi);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function enables debounce logic for the specified bit(s) of
 *  port.  When enabled, a signal must be valid for two periods of the
 *  external debounce clock before it is internally processed.
 *  Multiple bits can be specified in the function arguments,
 *  using the bitwise 'OR' operator.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to set
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_enableDebounce (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->debounce);
    data_out = data_in | interrupts;
    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(interrupts, portmap->debounce);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function disables debounce logic for the specified bit(s) of
 *  port A.  Multiple bits can be specified in the function arguments,
 *  using the bitwise 'OR' operator.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to set
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 ******************************************************************************/
u32 FGpioPs_disableDebounce (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    data_in = GPIO_IN32P(portmap->debounce);
    data_out = data_in & ~interrupts;
    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(data_out, portmap->debounce);
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns whether debounce is enabled for the specified
 *  it or port or not.  Only one bit may be specified per invocation
 *  of this function.
 *
 * @param    dev is gpio device handle.
 * @param    interrupt   -- bit of port to check
 *
 * @return
 *  TRUE        -- if debounce is enabled for interrupt bit
 *  FALSE       -- if debounce is disabled for interrupt bit
 *
 * @note    None.
 *
 ******************************************************************************/
BOOL FGpioPs_isDebounceEnabled (FGpioPs_T *dev, enum FGpioPs_bit interrupt)
{
    BOOL retval;
    u32 reg;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = FALSE;
    reg = GPIO_IN32P(portmap->debounce);
    if ((reg & (0x1 << interrupt)) != 0x0)
    {
        retval = TRUE;
    }

    return retval;
}

/******************************************************************************
 *
 *
 *  This function enables interrupt synchronization.  When enabled, all
 *  level-sensitive interrupts are synchronized to pclk.
 *
 * @param    dev is gpio device handle.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 *****************************************************************************/
u32 FGpioPs_enableSync (FGpioPs_T *dev)
{
    u32 retval;
    u32 reg;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    reg = 0x0;
    FMSH_BIT_SET(reg, GPIO_LS_SYNC_SYNCLEVEL, 0x1);
    GPIO_OUT32P(reg, portmap->ls_sync);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function disables synchronization for level-sensitive
 *  interrupts.
 *
 * @param    dev is gpio device handle.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 *****************************************************************************/
u32 FGpioPs_disableSync (FGpioPs_T *dev)
{
    u32 retval;
    u32 reg;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    retval = 0;
    reg = 0x0;
    FMSH_BIT_SET(reg, GPIO_LS_SYNC_SYNCLEVEL, 0x0);
    GPIO_OUT32P(reg, portmap->ls_sync);

    return retval;
}

/******************************************************************************
 *
 *
 *  This function returns whether synchronization is enabled for
 *  level-sensitive interrupts or not.
 *
 * @param    dev is gpio device handle.
 *
 * @return
 *  TRUE        -- if interrupt synchronization is enabled
 *  FALSE       -- if interrupt synchronization is not enabled
 *
 * @note    None.
 *
 *****************************************************************************/
BOOL FGpioPs_isSynced (FGpioPs_T *dev)
{
    BOOL retval;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    if (GPIO_IN32P(portmap->ls_sync) != 0x0)
    {
        retval = TRUE;
    }

    return retval;
}

/******************************************************************************
 *
 *
 * This function enable both edge interrupt, when this function is called
 * , gpio level and polarity is no sense.
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to set
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 *****************************************************************************/
u8 FGpioPs_enableIrqBothEdge (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;

    FGpioPs_Portmap_T *portmap;
    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    FGPIOPS_COMMON_ASSERT(dev);

    retval = 0;
    data_in = GPIO_IN32P(portmap->int_bothedge);
    data_out = data_in | interrupts;

    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(data_out, portmap->int_bothedge);
    }

    return retval;
}

/******************************************************************************
 *
 *
 * This function disable both edge interrupt, when this function is called
 * , interrupt occurs depending on gpio level and polarity .
 *
 * @param    dev is gpio device handle.
 * @param    interrupts  -- interrupt bit(s) to set
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note    None.
 *
 *****************************************************************************/
u8 FGpioPs_disableIrqBothEdge (FGpioPs_T *dev, u32 interrupts)
{
    u32 retval;
    u32 data_in;
    u32 data_out;

    FGpioPs_Portmap_T *portmap;
    portmap = (FGpioPs_Portmap_T *)dev->base_address;

    FGPIOPS_COMMON_ASSERT(dev);

    retval = 0;
    data_in = GPIO_IN32P(portmap->int_bothedge);
    data_out = data_in & ~interrupts;

    // avoid bus write if possible
    if (data_in != data_out)
    {
        GPIO_OUT32P(data_out, portmap->int_bothedge);
    }

    return retval;
}

/******************************************************************************
 *
 *
 * This function returns value of both edge interrupt reg.
 *
 * @param    dev is gpio device handle.
 *
 * @return   value of both edge interrupt reg.
 *
 * @note    None.
 *
 *****************************************************************************/
u32 FGpioPs_getBothEdgeInt (FGpioPs_T *dev)
{
    u32 data;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    data = GPIO_IN32P(portmap->int_bothedge);

    return data;
}

/******************************************************************************
 *
 *
 * This function returns gpio version ID code.
 *
 * @param    dev is gpio device handle.
 *
 * @return  version ID code.
 *
 * @note    None.
 *
 *****************************************************************************/
u32 FGpioPs_getVerIdCode (FGpioPs_T *dev)
{
    u32 data;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    data = GPIO_IN32P(portmap->ver_id_code);

    return data;
}

/******************************************************************************
 *
 *
 * This function returns gpio ID code.
 *
 * @param    dev is gpio device handle.
 *
 * @return   ID code.
 *
 * @note    None.
 *
 *****************************************************************************/
u32 FGpioPs_getIdCode (FGpioPs_T *dev)
{
    u32 data = 0;
    FGpioPs_Portmap_T *portmap;

    FGPIOPS_COMMON_ASSERT(dev);

    portmap = (FGpioPs_Portmap_T *)dev->base_address;
    data = GPIO_IN32P(portmap->id_code);

    return data;
}
