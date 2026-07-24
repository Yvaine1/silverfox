/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * @file  fmsh_i2c_lib.c
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 * 0.01   lq  04/10/2024  First Release
 *</pre>
 ******************************************************************************/

/***************************** Include Files*********************************/
#include <string.h>

#include "fmsh_common.h"
#include "fmsh_i2c_lib.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions*****************************/

/**************************** Type Definitions*******************************/
#define SS_MIN_SCL_HIGH       4000
#define SS_MIN_SCL_LOW        4700
#define FS_MIN_SCL_HIGH       600
#define FS_MIN_SCL_LOW        1300
#define HS_MIN_SCL_HIGH_100PF 60
#define HS_MIN_SCL_LOW_100PF  120

/***************** Macros (Inline Functions) Definitions*********************/

/************************** Variable Definitions*****************************/

/************************** Function Prototypes******************************/

/*****************************************************************************
 *
 * @description
 *  This function initializes the I2C driver.  It disables and
 *  clears all interrupts, sets the DMA mode to software handshaking,
 *  sets the DMA transmit and receive notifier function pointers to NULL
 *  and disables the I2C.  It also attempts to determine the hardware
 *  parameters of the device, if supported by the device.
 *
 * @param   dev is i2c device handle.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_ENOSYS  -- hardware parameters for the device could not be
 *              automatically determined
 *
 * @note
 *  Accesses the following FMSH_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *   - ic_clr_intr/clr_intr
 *   - ic_rx_tl/rx_tl
 *   - ic_enable/enable
 *
 *  This function is affected by the ADD_ENCODED_PARAMS hardware
 *  parameter.  If set to false, it is necessary for the user to create
 *  an appropriate FIicPs_Param_T structure as part of the FI2cPs_T
 *  structure.  If set to true, FI2cPs_init() will automatically
 *  initialize this structure (space for which must have been already
 *  allocated).
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_init (FI2cPs_T *dev, FI2cPs_Config *cfg, void *pI2cInstance,
                 void *pI2cParam)
{
    int retval = 0;

    dev->id = cfg->DeviceId;
    if (cfg->DeviceId == FPAR_I2CPS_0_DEVICE_ID)
    {
        dev->name = "I2C0";
    }
    else if (cfg->DeviceId == FPAR_I2CPS_1_DEVICE_ID)
    {
        dev->name = "I2C1";
    }
    else
    {
        return -FMSH_ENOSYS;
    }

    dev->base_address = (FIicPs_PortMap_T *)(cfg->BaseAddress);
    dev->input_clock = cfg->InputClockHz;
    dev->comp_type = FMSH_APB_I2C;
    dev->instance = pI2cInstance;
    dev->comp_param = pI2cParam;
    // disable device
    retval = FI2cPs_disable(dev);
    // if device is not busy (i.e. it is now disabled)
    if (retval == 0)
    {
        // disable all interrupts
        FI2cPs_maskIrq(dev, I2c_irq_all);
        FI2cPs_clearIrq(dev, I2c_irq_all);

        // reset instance variables
        FI2cPs_resetInstance(dev);

        // autoConfigure component parameters if possible
        retval = FI2cPs_autoCompParams(dev);
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function enables the I2C.
 *
 * @param   dev is i2c device handle.
 *
 * @return
 *  none
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_enable/enable
 * SEE ALSO
 *  FI2cPs_disable(), FI2cPs_isEnabled()
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_enable (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = 0;
    FMSH_BIT_SET(reg, I2C_ENABLE_ENABLE, 0x1);
    I2C_OUTP(reg, portmap->enable);
}

/*****************************************************************************
 *
 * @description
 *  This functions disables the I2C, if it is not busy (determined by
 *  the activity interrupt bit).  The I2C should not be disabled during
 *  interrupt-driven transfers as the resulting driver behavior is
 *  undefined.
 *
 * @param   dev is i2c device handle.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- if the I2C is busy
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_enable/enable
 * SEE ALSO
 *  FI2cPs_disable(), FI2cPs_isEnabled()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_disable (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isBusy(dev) == false)
    {
        reg = retval = 0;
        FMSH_BIT_SET(reg, I2C_ENABLE_ENABLE, 0);
        I2C_OUTP(reg, portmap->enable);
    }
    else
    {
        retval = -FMSH_EBUSY;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function returns whether the I2C is enabled or not.
 *
 * @param   dev is i2c device handle.
 *
 * @return
 *  true        -- the I2C is enabled
 *  false       -- the I2C is disabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_enable/enable
 * SEE ALSO
 *  FI2cPs_disable(), FI2cPs_isEnabled()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isEnabled (FI2cPs_T *dev)
{
    bool retval = false;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->enable);
    retval = (bool)FMSH_BIT_GET(reg, I2C_ENABLE_ENABLE);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function returns whether the I2C is busy (transmitting
 *   or receiving) or not.
 *
 * @param   dev is i2c device handle.
 *
 * @return
 *  true        -- the I2C device is busy
 *  false       -- the I2C device is not busy
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_status/activity
 * SEE ALSO
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isBusy (FI2cPs_T *dev)
{
    bool retval = false;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->status);
    if (FMSH_BIT_GET(reg, I2C_STATUS_ACTIVITY) == 0x0)
    {
        retval = false;
    }
    else
    {
        retval = true;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function sets the speed mode used for I2C transfers.
 *
 * @param
 * dev is i2c device handle.
 * mode is the speed mode to set
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *  -FMSH_ENOSYS  -- if the specified speed is not supported
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/speed
 * The I2C must be disabled in order to change the speed mode.
 *  This function is affected by the MAX_SPEED_MODE hardware parameter.
 *  It is not possible to set the speed higher than the maximum speed
 *  mode.
 * SEE ALSO
 *  FI2cPs_getSpeedMode(), FIicPs_SpeedMode_T
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setSpeedMode (FI2cPs_T *dev, FIicPs_SpeedMode_T mode)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_Param_T *param = NULL;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    param = (FIicPs_Param_T *)dev->comp_param;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (mode > param->max_speed_mode)
    {
        retval = -FMSH_ENOSYS;
    }
    else if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_SPEED) != mode)
        {
            FMSH_BIT_SET(reg, I2C_CON_SPEED, mode);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function returns the speed mode currently in use by the I2C.
 *
 * @param
 * dev is i2c device handle.
 *
 * @return
 *  The current I2C speed mode.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/speed
 * SEE ALSO
 *  FI2cPs_setSpeedMode(), FIicPs_SpeedMode_T
 * SOURCE
 *
 ****************************************************************************/
FIicPs_SpeedMode_T FI2cPs_getSpeedMode (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_SpeedMode_T retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->con);
    retval = (FIicPs_SpeedMode_T)FMSH_BIT_GET(reg, I2C_CON_SPEED);

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function sets the master addressing mode (7-bit or 10-bit).
 *
 * @param
 * dev is i2c device handle.
 *  mode is the addressing mode to set.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_10bitaddr_master
 *
 *  The I2C must be disabled in order to change the master addressing
 *  mode.
 * SEE ALSO
 *  FI2cPs_getMasterAddressMode(), enum FI2cPs_address_mode
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setMasterAddressMode (FI2cPs_T *dev, FIicPs_AddressMode_T mode)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->tar);
        /*    if(FMSH_BIT_GET(reg, I2C_CON_10BITADDR_MASTER) != mode) {
                FMSH_BIT_SET(reg, I2C_CON_10BITADDR_MASTER, mode);
                I2C_OUTP(reg, portmap->con);
            }*/
        if (FMSH_BIT_GET(reg, I2C_TAR_10BITADDR_MASTER) != mode)
        {
            FMSH_BIT_SET(reg, I2C_TAR_10BITADDR_MASTER, mode);
            I2C_OUTP(reg, portmap->tar);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function returns the current master addressing mode (7-bit or
 *  10-bit).
 *
 * @param
 *  dev is i2c device handle.
 *
 *
 * @return
 *  The current master addresing mode.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_10bitaddr_master
 * SEE ALSO
 *  FI2cPs_setSlaveAddressMode(), enum FI2cPs_address_mode
 * SOURCE
 *
 ****************************************************************************/
FIicPs_AddressMode_T FI2cPs_getMasterAddressMode (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_AddressMode_T retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->con);
    retval = (FIicPs_AddressMode_T)FMSH_BIT_GET(reg, I2C_CON_10BITADDR_MASTER);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function sets the I2C slave addressing mode (7-bit or 10-bit).
 *
 * @param
 *  dev is i2c device handle.
 *   mode is the addressing mode to set.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *   Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_10bitaddr_slave
 *
 *  The I2C must be disabled in order to change the slave addressing
 *  mode.
 * SEE ALSO
 *  FI2cPs_getSlaveAddressMode(), enum FI2cPs_address_mode
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setSlaveAddressMode (FI2cPs_T *dev, FIicPs_AddressMode_T mode)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_10BITADDR_SLAVE) != mode)
        {
            FMSH_BIT_SET(reg, I2C_CON_10BITADDR_SLAVE, mode);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function returns the current slave addressing mode (7-bit or
 *  10-bit).
 *
 * @param
 *  dev is i2c device handle.
 *
 *
 * @return
 *  The current slave addressing mode.
 *
 * @note
 *  AAccesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_10bitaddr_slave
 * SEE ALSO
 *  FI2cPs_setSlaveAddressMode(), enum FI2cPs_address_mode
 * SOURCE
 *
 ****************************************************************************/
FIicPs_AddressMode_T FI2cPs_getSlaveAddressMode (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_AddressMode_T retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->con);
    retval = (FIicPs_AddressMode_T)FMSH_BIT_GET(reg, I2C_CON_10BITADDR_SLAVE);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function enables the I2C slave.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *   0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_slave_disable
 *
 *  The I2C must be disabled in order to enable the slave.
 * SEE ALSO
 *  FI2cPs_disableSlave(), FI2cPs_isSlaveEnabled()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_enableSlave (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_SLAVE_DISABLE) != 0x0)
        {
            FMSH_BIT_SET(reg, I2C_CON_SLAVE_DISABLE, 0x0);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function disables the I2C slave.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *   0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *   Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_slave_disable
 *
 *  The I2C must be disabled in order to disable the slave.
 * SEE ALSO
 *  FI2cPs_enableSlave(), FI2cPs_isSlaveEnabled()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_disableSlave (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_SLAVE_DISABLE) != 0x1)
        {
            FMSH_BIT_SET(reg, I2C_CON_SLAVE_DISABLE, 0x1);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function returns whether the I2C slave is enabled or not.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- slave is enabled
 *  false       -- slave is disabled
 *
 * @note
 *   Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_slave_disable
 * SEE ALSO
 *  FI2cPs_enableSlave(), FI2cPs_disableSlave()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isSlaveEnabled (FI2cPs_T *dev)
{
    bool retval = false;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->con);
    if (FMSH_BIT_GET(reg, I2C_CON_SLAVE_DISABLE) == 0x0)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function enables the I2C master.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/master_mode
 *
 *  The I2C must be disabled in order to enable the master.
 * SEE ALSO
 *  FI2cPs_disableMaster(), FI2cPs_isMasterEnabled()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_enableMaster (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_MASTER_MODE) != 0x1)
        {
            FMSH_BIT_SET(reg, I2C_CON_MASTER_MODE, 0x1);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function disables the I2C master.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/master_mode
 *
 *  The I2C must be disabled in order to disable the master.
 * SEE ALSO
 *  FI2cPs_enableMaster(), FI2cPs_isMasterEnabled()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_disableMaster (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_MASTER_MODE) != 0x0)
        {
            FMSH_BIT_SET(reg, I2C_CON_MASTER_MODE, 0x0);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function returns whether the I2C master is enabled or not.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- master is enabled
 *  false       -- master is disabled
 *
 * @note
 *   Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/master_mode
 * SEE ALSO
 *  FI2cPs_enableMaster(), FI2cPs_disableMaster()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isMasterEnabled (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    bool retval = false;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->con);
    if (FMSH_BIT_GET(reg, I2C_CON_MASTER_MODE) == 0x1)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function enables the use of restart conditions.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_restart_en
 *
 *  The I2C must be disabled in order to enable restart conditions.
 * SEE ALSO
 *  FI2cPs_disableRestart()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_enableRestart (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_RESTART_EN) != 0x1)
        {
            FMSH_BIT_SET(reg, I2C_CON_RESTART_EN, 0x1);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function disables the use of restart conditions.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_restart_en
 *
 *  The I2C must be disabled in order to disable restart conditions.
 * SEE ALSO
 *  FI2cPs_enableRestart()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_disableRestart (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_RESTART_EN) != 0x0)
        {
            FMSH_BIT_SET(reg, I2C_CON_RESTART_EN, 0x0);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function returns whether restart conditions are currently in
 *  use or not by the I2C.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- restart conditions are enabled
 *  false       -- restart conditions are disabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_con/ic_restart_en
 * SEE ALSO
 *  FI2cPs_enableRestart(), FI2cPs_disableRestart()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isRestartEnabled (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    bool retval = false;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->con);
    if (FMSH_BIT_GET(reg, I2C_CON_RESTART_EN) == 0x1)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *   This function sets the target address used by the I2C master.  When
 *  not issuing a general call or using a start byte, this is the
 *  address the master uses when performing transfers over the I2C bus.
 *
 * @param
 *  dev is i2c device handle.
 *  address is target address to set
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_tar/ic_tar
 *
 *  The I2C must be disabled in order to set the target address.  Only
 *  the 10 least significant bits of the address are relevant.
 * SEE ALSO
 *  FI2cPs_getTargetAddress()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setTargetAddress (FI2cPs_T *dev, uint16_t address)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    // address should be no more than 10 bits long
    FMSH_REQUIRE((address & 0xfc00) == 0);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->tar);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_TAR_ADDR) != address)
        {
            FMSH_BIT_SET(reg, I2C_TAR_ADDR, address);
            I2C_OUTP(reg, portmap->tar);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns the current target address in use by the I2C
 *  master.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The current target address.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_tar/ic_tar
 * SEE ALSO
 *  FI2cPs_setTargetAddress()
 * SOURCE
 *
 ****************************************************************************/
uint16_t FI2cPs_getTargetAddress (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    uint16_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->tar);
    retval = FMSH_BIT_GET(reg, I2C_TAR_ADDR);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function sets the slave address to which the I2C slave
 *  responds, when enabled.
 *
 * @param
 *  dev is i2c device handle.
 *  address is slave address to set
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_sar/ic_sar
 *
 *  The I2C must be disabled in order to set the target address.  Only
 *  the 10 least significant bits of the address are relevant.
 * SEE ALSO
 *  FI2cPs_getSlaveAddress()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setSlaveAddress (FI2cPs_T *dev, uint16_t address)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    // address should be no more than 10 bits long
    FMSH_REQUIRE((address & 0xfc00) == 0);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        reg = retval = 0;
        FMSH_BIT_SET(reg, I2C_SAR_ADDR, address);
        I2C_OUTP(reg, portmap->sar);
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns the current address in use by the I2C slave.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The current I2C slave address.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_sar/ic_sar
 * SEE ALSO
 *  FI2cPs_setSlaveAddress()
 * SOURCE
 *
 ****************************************************************************/
uint16_t FI2cPs_getSlaveAddress (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    uint16_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->sar);
    retval = FMSH_BIT_GET(reg, I2C_SAR_ADDR);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function sets the master transmit mode.  That is, whether to
 *  use a start byte, general call, or the programmed target address.
 *
 * @param
 *  dev is i2c device handle.
 *  mode is transfer mode to set
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *
 * @note
 * Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_tar/special
 *   - ic_tar/gc_or_start
 *
 *  The I2C must be disabled in order to set the master transmit mode.
 * SEE ALSO
 *  FI2cPs_getTxMode(), FIicPs_TxMode_T
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setTxMode (FI2cPs_T *dev, FIicPs_TxMode_T mode)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->tar);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_TAR_TX_MODE) != mode)
        {
            FMSH_BIT_SET(reg, I2C_TAR_TX_MODE, mode);
            I2C_OUTP(reg, portmap->tar);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns the current transmit mode in use by an I2C
 *  master.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The current master transmit mode.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_tar/special
 *   - ic_tar/gc_or_start
 * SEE ALSO
 *  FI2cPs_setTxMode(), FIicPs_TxMode_T
 * SOURCE
 *
 ****************************************************************************/
FIicPs_TxMode_T FI2cPs_getTxMode (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_TxMode_T retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->tar);
    retval = (FIicPs_TxMode_T)FMSH_BIT_GET(reg, I2C_TAR_TX_MODE);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function set the scl count value for a particular speed mode
 *  (standard, fast, high) and clock phase (low, high).
 *
 * @param
 *  dev is i2c device handle.
 *  mode is speed mode of count value to set
 *  phase is scl phase of count value to set
 *  value is count value to set
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPERM   -- if the I2C is enabled
 *  -FMSH_ENOSYS  -- if the scl count registers are hardcoded
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_ss_scl_lcnt/ic_ss_scl_lcnt
 *   - ic_ss_scl_hcnt/ic_ss_scl_hcnt
 *   - ic_fs_scl_lcnt/ic_fs_scl_lcnt
 *   - ic_fs_scl_hcnt/ic_fs_scl_hcnt
 *   - ic_hs_scl_lcnt/ic_hs_scl_lcnt
 *   - ic_hs_scl_hcnt/ic_hs_scl_hcnt
 *
 *  The I2C must be disabled in order to set any of the scl count
 *  values.  The minimum programmable value for any of these registers
 *  is 6.
 *  This function is affected by the HC_COUNT_VALUES hardware parameter.
 * SEE ALSO
 *  FI2cPs_getSclCount(), FIicPs_SpeedMode_T, FIicPs_SclPhase_T
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setSclCount (FI2cPs_T *dev, FIicPs_SpeedMode_T mode,
                        FIicPs_SclPhase_T phase, uint16_t count)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_Param_T *param = NULL;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;
    param = (FIicPs_Param_T *)dev->comp_param;

    if (param->hc_count_values == true)
    {
        retval = -FMSH_ENOSYS;
    }
    else if (FI2cPs_isEnabled(dev) == false)
    {
        reg = retval = 0;
        FMSH_BIT_SET(reg, I2C_SCL_COUNT, count);
        if (mode == I2c_speed_high)
        {
            if (phase == I2c_scl_low)
            {
                I2C_OUTP(reg, portmap->hs_scl_lcnt);
            }
            else
            {
                I2C_OUTP(reg, portmap->hs_scl_hcnt);
            }
        }
        else if (mode == I2c_speed_fast)
        {
            if (phase == I2c_scl_low)
            {
                I2C_OUTP(reg, portmap->fs_scl_lcnt);
            }
            else
            {
                I2C_OUTP(reg, portmap->fs_scl_hcnt);
            }
        }
        else if (mode == I2c_speed_standard)
        {
            if (phase == I2c_scl_low)
            {
                I2C_OUTP(reg, portmap->ss_scl_lcnt);
            }
            else
            {
                I2C_OUTP(reg, portmap->ss_scl_hcnt);
            }
        }
        else{
            ;/* no deal with */
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns the current scl count value for all speed
 *  modes (standard, fast, high) and phases (low, high).
 *
 * @param
 *  dev is i2c device handle.
 *  mode is speed mode to get count value of
 *  phase is scl phase to get count value of
 *
 * @return
 *  The current specified scl count value.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_ss_scl_lcnt/ic_ss_scl_lcnt
 *   - ic_ss_scl_hcnt/ic_ss_scl_hcnt
 *   - ic_fs_scl_lcnt/ic_fs_scl_lcnt
 *   - ic_fs_scl_hcnt/ic_fs_scl_hcnt
 *   - ic_hs_scl_lcnt/ic_hs_scl_lcnt
 *   - ic_hs_scl_hcnt/ic_hs_scl_hcnt
 *
 *  This function returns 0x0000 for any non-existent scl count
 *  registers.
 * SEE ALSO
 *  FI2cPs_setSclCount(), FIicPs_SpeedMode_T, FIicPs_SclPhase_T
 * SOURCE
 *
 ****************************************************************************/
uint16_t FI2cPs_getSclCount (FI2cPs_T *dev, FIicPs_SpeedMode_T mode,
                             FIicPs_SclPhase_T phase)
{
    uint32_t reg = 0U;
    uint16_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (mode == I2c_speed_high)
    {
        if (phase == I2c_scl_low)
        {
            reg = I2C_INP(portmap->hs_scl_lcnt);
        }
        else
        {
            reg = I2C_INP(portmap->hs_scl_hcnt);
        }
    }
    else if (mode == I2c_speed_fast)
    {
        if (phase == I2c_scl_low)
        {
            reg = I2C_INP(portmap->fs_scl_lcnt);
        }
        else
        {
            reg = I2C_INP(portmap->fs_scl_hcnt);
        }
    }
    else if (mode == I2c_speed_standard)
    {
        if (phase == I2c_scl_low)
        {
            reg = I2C_INP(portmap->ss_scl_lcnt);
        }
        else
        {
            reg = I2C_INP(portmap->ss_scl_hcnt);
        }
    }
    else{
        ;/* no deal with */
    }
    retval = (uint16_t)FMSH_BIT_GET(reg, I2C_SCL_COUNT);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function reads a single byte from the I2C receive FIFO.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The character read from the I2C FIFO
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_data_cmd/dat
 *
 *  This function also does not check whether there is valid data or not
 *  in the FIFO beforehand.  As such, it can cause an receive underflow
 *  error if used improperly.
 * SEE ALSO
 *  FI2cPs_write(), FI2cPs_issueRead(), FI2cPs_masterReceive(),
 *  FI2cPs_slaveReceive()
 * SOURCE
 *
 ****************************************************************************/
uint8_t FI2cPs_read (FI2cPs_T *dev)
{
    uint8_t retval;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    // read a byte from the DATA_CMD register
    reg = I2C_IN8P(portmap->data_cmd);
    retval = (reg & 0xff);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function writes a single byte to the I2C transmit FIFO.
 *
 * @param
 *  dev is i2c device handle.
 *  datum  is byte to write to FIFO
 *
 * @return
 *  none
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_data_cmd/dat
 *
 *  This function does not check whether the I2C transmit FIFO is full
 *  or not beforehand.  As such, it can cause a transmit overflow error
 *  if used improperly.
 * SEE ALSO
 *  FI2cPs_read(), FI2cPs_issueRead(), FI2cPs_masterTransmit(),
 *  FI2cPs_slaveTransmit()
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_write (FI2cPs_T *dev, uint8_t character)
{
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    // write a byte to the DATA_CMD register
    I2C_OUTP(character, portmap->data_cmd);
}

/*****************************************************************************
 *
 * @description
 *  This function writes a read command to the I2C transmit FIFO.  This
 *  is used during master-receiver/slave-transmitter transfers and is
 *  typically followed by a read from the master receive FIFO after the
 *  slave responds with data.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  none
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_data_cmd/cmd
 *
 *  This function does not check whether the I2C FIFO is full or not
 *  before writing to it.  As such, it can result in a transmit FIFO
 *  overflow if used improperly.
 * SEE ALSO
 *  FI2cPs_read(), FI2cPs_write(), FI2cPs_masterReceive()
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_issueRead (FI2cPs_T *dev)
{
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    I2C_OUTP(0x100, portmap->data_cmd);

    // FMSH_OUT16_16(0x300, portmap->data_cmd);
}

/*****************************************************************************
 *
 * @description
 *  This function writes a read command to the I2C transmit FIFO.  This
 *  is used during master-receiver/slave-transmitter transfers and is
 *  typically followed by a read from the master receive FIFO after the
 *  slave responds with data.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  none
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_data_cmd/cmd
 *
 *  This function does not check whether the I2C FIFO is full or not
 *  before writing to it.  As such, it can result in a transmit FIFO
 *  overflow if used improperly.
 * SEE ALSO
 *  FI2cPs_read(), FI2cPs_write(), FI2cPs_masterReceive()
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_issueReadStop (FI2cPs_T *dev)
{
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    I2C_OUTP(0x300, portmap->data_cmd);
}
/*****************************************************************************
 *
 * @description
 *  This function returns the current value of the I2C transmit abort
 *  status register.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The current transmit abort status.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_tx_abrt_source/all bits
 *
 *  The transmit abort status register is cleared by the I2C upon
 *  reading.  Note that it is possible for more than one bit of this
 *  register to be active simultaneously and this should be dealt with
 *  properly by any function operating on this return value.
 * SEE ALSO
 *  FIicPs_TxAbort_T
 * SOURCE
 *
 ****************************************************************************/
FIicPs_TxAbort_T FI2cPs_getTxAbortSource (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_TxAbort_T retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->tx_abrt_source);
    retval = (FIicPs_TxAbort_T)FMSH_BIT_GET(reg, I2C_TX_ABRT_SRC_ALL);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Returns how many bytes deep the  I2C transmit FIFO is.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  FIFO depth in bytes (from 2 to 256)
 *
 * @note
 *  This function is affected by the TX_BUFFER_DEPTH hardware parameter.
 * SEE ALSO
 *  FI2cPs_getRxFifoDepth(), FI2cPs_getTxFifoLevel()
 * SOURCE
 *
 ****************************************************************************/
uint16_t FI2cPs_getTxFifoDepth (FI2cPs_T *dev)
{
    uint32_t retval;
    FIicPs_Param_T *param = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    param = (FIicPs_Param_T *)dev->comp_param;

    retval = param->tx_buffer_depth;

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Returns how many bytes deep the I2C transmit FIFO is.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  FIFO depth in bytes (from 2 to 256)
 *
 * @note
 *  This function is affected by the RX_BUFFER_DEPTH hardware parameter.
 * SEE ALSO
 *  FI2cPs_getTxFifoDepth(), FI2cPs_getRxFifoLevel()
 * SOURCE
 *
 ****************************************************************************/
uint16_t FI2cPs_getRxFifoDepth (FI2cPs_T *dev)
{
    uint32_t retval;
    FIicPs_Param_T *param = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    param = (FIicPs_Param_T *)dev->comp_param;

    retval = param->rx_buffer_depth;

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Returns whether the transmitter FIFO is full or not.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- the transmit FIFO is full
 *  false       -- the transmit FIFO is not full
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_status/tfnf
 * SEE ALSO
 *  FI2cPs_isTxFifoEmpty()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isTxFifoFull (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    bool retval = false;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->status);
    if (FMSH_BIT_GET(reg, I2C_STATUS_TFNF) == 0)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Returns whether the transmitter FIFO is empty or not.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- the transmit FIFO is full
 *  false       -- the transmit FIFO is not full
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_status/tfe
 * SEE ALSO
 *  FI2cPs_isTxFifoFull()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isTxFifoEmpty (FI2cPs_T *dev)
{
    bool retval = false;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->status);
    if (FMSH_BIT_GET(reg, I2C_STATUS_TFE) == 1)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns whether the receive FIFO is full or not.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- the receive FIFO is full
 *  false       -- the receive FIFO is not full
 *
 * @note
 *   Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_status/rff
 * SEE ALSO
 *  FI2cPs_isRxFifoEmpty()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isRxFifoFull (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    bool retval = false;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->status);
    if (FMSH_BIT_GET(reg, I2C_STATUS_RFF) == 1)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns whether the receive FIFO is empty or not.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- the receive FIFO is empty
 *  false       -- the receive FIFO is not empty
 *
 * @note
 *   Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_status/rfne
 * SEE ALSO
 *  FI2cPs_isRxFifoFull()
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isRxFifoEmpty (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    bool retval = false;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->status);
    if (FMSH_BIT_GET(reg, I2C_STATUS_RFNE) == 0)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns the number of valid data entries currently
 *  present in the transmit FIFO.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  Number of valid data entries in the transmit FIFO.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_txflr/txflr
 * SEE ALSO
 *  FI2cPs_getRxFifoLevel(), FI2cPs_isTxFifoFull(),
 *  FI2cPs_isTxFifoEmpty(), FI2cPs_setTxThreshold(),
 *  FI2cPs_getTxThreshold()
 * SOURCE
 *
 ****************************************************************************/
uint16_t FI2cPs_getTxFifoLevel (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    uint16_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->txflr);
    retval = (uint16_t)FMSH_BIT_GET(reg, I2C_TXFLR_TXFL);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function returns the number of valid data entries currently
 *  present in the receiver FIFO.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  Number of valid data entries in the receive FIFO.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_rxflr/rxflr
 * SEE ALSO
 *  FI2cPs_getTxFifoLevel(), FI2cPs_isRxFifoFull(),
 *  FI2cPs_isRxFifoEmpty(), FI2cPs_setRxThreshold(),
 *  FI2cPs_getRxThreshold()
 * SOURCE
 *
 ****************************************************************************/
uint16_t FI2cPs_getRxFifoLevel (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    uint16_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->rxflr);
    retval = (uint16_t)FMSH_BIT_GET(reg, I2C_RXFLR_RXFL);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function sets the threshold level for the transmit FIFO.  When
 *  the number of data entries in the transmit FIFO is at or below this
 *  level, the tx_empty interrupt is triggered.  If an interrupt-driven
 *  transfer is already in progress, the transmit threshold level is not
 *  updated until the end of the transfer.
 *
 * @param
 *  dev is i2c device handle.
 *  level is level at which to set threshold
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EINVAL  -- if the level specified is greater than the transmit
 *                 FIFO depth; the threshold is set to the transmit FIFO
 *                 depth.
 *  -FMSH_EBUSY   -- if an interrupt-driven transfer is currently in
 *                 progress; the requested level will be written to the
 *                 transmit threshold register when the current transfer
 *                 completes.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_tx_tl/tx_tl
 *
 *  The driver keeps a copy of the last transmit threshold specified by
 *  the user as it manipulates the transmit threshold level at the end
 *  of interrupt-driven transmit transfers.  This copy is used to
 *  restore the transmit threshold upon completion of a transmit
 *  transfer.
 * SEE ALSO
 *  FI2cPs_getTxThreshold(), FI2cPs_setRxThreshold(),
 *  FI2cPs_getRxThreshold(), FI2cPs_getTxFifoLevel()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setTxThreshold (FI2cPs_T *dev, uint8_t level)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_Param_T *param = NULL;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    param = (FIicPs_Param_T *)dev->comp_param;
    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    // We need to be careful here not to overwrite the tx threshold if
    // the interrupt handler has altered it to trigger on the last byte
    // of the current transfer (in order to call the user callback
    // function at the appropriate time).  When the driver returns to
    // the idle state, it will update the tx threshold with the
    // user-specified value.
    if (level > param->tx_buffer_depth)
    {
        retval = -FMSH_EINVAL;
    }
    else
    {
        // store user tx threshold value
        instance->txThreshold = level;

        if (instance->state == I2c_state_idle)
        {
            reg = retval = 0;
            FMSH_BIT_SET(reg, I2C_TX_TL_TX_TL, level);
            I2C_OUTP(reg, portmap->tx_tl);
        }
        else
        {
            retval = -FMSH_EBUSY;
        }
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns the current threshold level for the transmit
 *  FIFO.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The transmit FIFO threshold level.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_tx_tl/tx_tl
 * SEE ALSO
 *  FI2cPs_setTxThreshold(), FI2cPs_setRxThreshold(),
 *  FI2cPs_getRxThreshold(), FI2cPs_getTxFifoLevel()
 * SOURCE
 *
 ****************************************************************************/
uint8_t FI2cPs_getTxThreshold (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    uint8_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->tx_tl);
    retval = FMSH_BIT_GET(reg, I2C_TX_TL_TX_TL);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function sets the threshold level for the receive FIFO.  When
 *  the number of data entries in the receive FIFO is at or above this
 *  level, the rx_full interrupt is triggered.  If an interrupt-driven
 *  transfer is already in progress, the receive threshold level is not
 *  updated until the end of the transfer.
 *
 * @param
 *  dev is i2c device handle.
 *  level is level at which to set threshold
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EINVAL  -- if the level specified is greater than the receive
 *                 FIFO depth, the threshold is set to the receive FIFO
 *                 depth.
 *  -FMSH_EBUSY   -- if an interrupt-driven transfer is currently in
 *                 progress, the requested level is written to the
 *                 receive threshold register when the current transfer
 *                 completes.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_rx_tl/rx_tl
 *
 *  The driver keeps a copy of the last receive threshold specified by
 *  the user as it manipulates the receive threshold level at the end of
 *  interrupt-driven receive transfers.  This copy is used to restore
 *  the receive threshold upon completion of an receive transfer.
 * WARNINGS
 *  When this function is called, if the following slave receive
 *  transfer is less than the threshold set, the rx_full interrupt is
 *  not immediately triggered.
 * SEE ALSO
 *  FI2cPs_getRxThreshold(), FI2cPs_setTxThreshold(),
 *  FI2cPs_getTxThreshold(), FI2cPs_terminate(), FI2cPs_getRxFifoLevel()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_setRxThreshold (FI2cPs_T *dev, uint8_t level)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_Param_T *param = NULL;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    param = (FIicPs_Param_T *)dev->comp_param;
    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    // We need to be careful here not to overwrite the rx threshold if
    // the interrupt handler has altered it to trigger on the last byte
    // of the current transfer (in order to call the user callback
    // function at the appropriate time).  When the driver returns to
    // the idle state, it will update the rx threshold with the
    // user-specified value.
    if (level > param->rx_buffer_depth)
    {
        retval = -FMSH_EINVAL;
    }
    else
    {
        // store user rx threshold value
        instance->rxThreshold = level;

        if (instance->state == I2c_state_idle)
        {
            reg = retval = 0;
            FMSH_BIT_SET(reg, I2C_RX_TL_RX_TL, level);
            I2C_OUTP(reg, portmap->rx_tl);
        }
        else
        {
            retval = -FMSH_EBUSY;
        }
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function returns the current threshold level for the receive
 *  FIFO.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The receive FIFO threshold level.
 *
 * @note
 *   Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_rx_tl/rx_tl
 *
 *  It is possible that the returned value is not equal to what the user
 *  has previously set the receive threshold to be.  This is because the
 *  driver manipulates the threshold value in order to complete receive
 *  transfers.  The previous user-specified threshold is restored upon
 *  completion of each transfer.
 * SEE ALSO
 *  FI2cPs_setRxThreshold(), FI2cPs_setTxThreshold(),
 *  FI2cPs_getTxThreshold(), FI2cPs_getRxFifoLevel()
 * SOURCE
 *
 ****************************************************************************/
uint8_t FI2cPs_getRxThreshold (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    uint8_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->rx_tl);
    retval = FMSH_BIT_GET(reg, I2C_RX_TL_RX_TL);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function is used to set a user listener function.  The listener
 *  function is responsible for handling all interrupts that are not
 *  handled by the Driver Kit interrupt handler.  This encompasses all
 *  error interrupts, general calls, read requests, and receive full
 *  when no receive buffer is available.  There is no need to clear any
 *  interrupts in the listener as this is handled automatically by the
 *  Driver Kit interrupt handlers.
 *  A listener must be setup up before using any of the other functions
 *  of the Interrupt API.  Note that if the FI2cPs_userIrqHandler
 *  interrupt handler is being used, none of the other Interrupt API
 *  functions can be used with it.  This is because they are symbiotic
 *  with the FI2cPs_irqHandler() interrupt handler.
 *
 * @param
 *  dev is i2c device handle.
 *  listener is function pointer to user listener function.
 *
 * @return
 *  none
 *
 * @note
 *  This function enables the following interrupts: I2c_irq_rx_under,
 *  I2c_irq_rx_over, I2c_irq_tx_over, I2c_irq_rd_req, I2c_irq_tx_abrt,
 *  I2c_irq_rx_done, and I2c_irq_gen_call.  It also enables
 *  I2c_irq_rx_full is the DmaRxMode is not set to hardware handshaking.
 *  The FMSH_callback function pointer typedef is defined in the common
 *  header files.
 *
 *  Whether the FI2cPs_userIrqHandler() or FI2cPs_irqHandler() interrupt
 *  handler is being used, this function is used to set the user
 *  listener function that is called by both of them.
 * EXAMPLE
 *  In the case of new data being received, the irq handler
 *  (FI2cPs_irqHandler) would call the user listener function as
 *  follows:
 *
 *  userListener(dev, I2c_irq_rx_full);
 *
 *  It is the listener function's responsibility to properly handle
 *  this. For example:
 *
 *  FI2cPs_slaveReceive(dev, buffer, length, callback);
 *
 * SEE ALSO
 *  FI2cPs_setDmaRxMode(), FI2cPs_userIrqHanler(), FI2cPs_irqHandler(),
 *  FI2cPs_irq, FMSH_callback
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_setListener (FI2cPs_T *dev, FMSH_callback userFunction)
{
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(userFunction != NULL);

    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables all FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    instance->listener = userFunction;

    instance->intr_mask_save = (FIicPs_Irq_T)(I2c_irq_rx_under |
                                              I2c_irq_rx_over |
                                              I2c_irq_tx_over | I2c_irq_rd_req |
                                              I2c_irq_tx_abrt |
                                              I2c_irq_rx_done |
                                              I2c_irq_gen_call);

    // don't enable Rx FIFO full if DMA hardware handshaking is in use
    // if(instance->dmaRx.mode != FMSH_dma_hw_handshake)
    instance->intr_mask_save |= I2c_irq_rx_full;

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();
}

/*****************************************************************************
 *
 * @description
 *  This function initiates an interrupt-driven master back-to-back
 *  transfer.  To do this, the I2C must first be properly configured,
 *  enabled and a transmit buffer must be setup which contains the
 *  sequential reads and writes to perform.  An associated receive
 *  buffer of suitable size must also be specified when issuing the
 *  transfer.  As data is received, it is written to the receive buffer.
 *  The callback function is called (if it is not NULL) when the final
 *  byte is received and there is no more data to send.
 *
 *  A transfer may be stopped at any time by calling FI2cPs_terminate(),
 *  which returns the number of bytes that are sent before the transfer
 *  is interrupted.  A terminated transfer's callback function is never
 *  called.
 *
 * @param
 *  dev is i2c device handle.
 *  txBuffer is buffer from which to send data
 *  txLength is length of transmit buffer/number of bytes to send
 *  rxBuffer is buffer to write received data to
 *  rxLength is length of receive buffer/number of bytes to receive
 *  callback is function to call when transfer is complete
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- if the I2C is busy (transfer already in progress)
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *   - ic_rx_tl/rx_tl
 *
 *  This function enables the tx_empty and tx_abrt interrupts.
 *  The FMSH_callback function pointer typedef is defined in the common
 *  header files.
 *  The transmit buffer must be 16 bits wide as a read command is 9 bits
 *  long (0x100).  Restart conditions must be enabled in order to
 *  perform back-to-back transfers.
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 *
 *  This function cannot be used when using an interrupt handler other
 *  than FI2cPs_irqHandler().
 * SEE ALSO
 *  FI2cPs_masterTransmit(), FI2cPs_masterReceive(),
 *  FI2cPs_slaveTransmit(), FI2cPs_slaveBulkTransmit(),
 *  FI2cPs_slaveReceive(), FI2cPs_terminate()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_masterBack2Back (FI2cPs_T *dev, uint16_t *txBuffer,
                            uint32_t txLength, uint8_t *rxBuffer,
                            uint32_t rxLength, FMSH_callback userFunction)
{
    int retval = 0;
    uint8_t *tmp;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(txBuffer != NULL);
    FMSH_REQUIRE(txLength != 0);
    FMSH_REQUIRE(rxBuffer != NULL);
    FMSH_REQUIRE(rxLength != 0);
    FMSH_REQUIRE(rxLength <= txLength);

    instance = (FIicPs_Instance_T *)dev->instance;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    FMSH_REQUIRE(instance->listener != NULL);

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    if (instance->state == I2c_state_idle)
    {
        retval = 0;
        // master back2back transfer mode
        instance->state = I2c_state_back2back;
        instance->rxBuffer = rxBuffer;
        instance->rxLength = rxLength;
        instance->rxRemain = rxLength;
        instance->rxCallback = userFunction;
        instance->rxHold = 0;
        instance->rxIdx = 4;
        instance->b2bBuffer = txBuffer;
        instance->txLength = txLength;
        instance->txRemain = txLength;
        instance->txCallback = userFunction;
        instance->txHold = 0;
        instance->txIdx = 0;

        // check if rx buffer is word-aligned
        if (((intptr_t)rxBuffer & 0x3) == 0)
        {
            instance->rxAlign = true;
        }
        else
        {
            instance->rxAlign = false;
        }

        // support non-word aligned 16-bit buffers
        tmp = (uint8_t *)txBuffer;
        while (((((intptr_t)tmp) & 0x3) != 0x0) &&
               ((instance->txLength - instance->txIdx) > 0))
        {
            instance->txHold |= ((*tmp++ & 0xff) << (instance->txIdx * 8));
            instance->txIdx++;
        }
        instance->txBuffer = tmp;

        // set rx fifo threshold if necessary
        if (rxLength <= instance->rxThreshold)
        {
            reg = 0;
            FMSH_BIT_SET(reg, I2C_RX_TL_RX_TL, rxLength - 1);
            I2C_OUTP(reg, portmap->rx_tl);
        }

        // ensure transfer is underway
        instance->intr_mask_save |= (I2c_irq_tx_empty | I2c_irq_tx_abrt |
                                     I2c_irq_rx_full);
    }
    else
    {
        retval = -FMSH_EBUSY;
    }

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function initiates an interrupt-driven master transmit
 *  transfer.  To do this, the I2C must first be properly configured and
 *  enabled.  This function configures a master transmit transfer and
 *  enables the transmit interrupt to keep the transmit FIFO filled.
 *  Upon completion, the callback function is called (if it is not
 *  NULL).
 *
 *  A transfer may be stopped at any time by calling FI2cPs_terminate(),
 *  which returns the number of bytes that are sent before the transfer
 *  is interrupted.  A terminated transfer's callback function is never
 *  called.
 *
 * @param
 *  dev is i2c device handle.
 *  buffer is buffer from which to send data
 *  length is length of transmit buffer/number of bytes to send
 *  callback is function to call when transfer is complete
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- if the I2C is busy (transfer already in progress)
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *
 *  This function enables the tx_empty and tx_abrt interrupts.
 *  The FMSH_callback function pointer typedef is defined in the common
 *  header files.
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 *
 *  This function cannot be used when using an interrupt handler other
 *  than FI2cPs_irqHandler().
 * SEE ALSO
 *  FI2cPs_masterBack2Back(), FI2cPs_masterReceive(),
 *  FI2cPs_slaveTransmit(), FI2cPs_slaveBulkTransmit(),
 *  FI2cPs_slaveReceive(), FI2cPs_terminate()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_masterTransmit (FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                           FMSH_callback userFunction)
{
    int retval = 0;
    uint8_t *tmp;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(buffer != NULL);
    FMSH_REQUIRE(length != 0);

    instance = (FIicPs_Instance_T *)dev->instance;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    FMSH_REQUIRE(instance->listener != NULL);

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    if (instance->state == I2c_state_idle)
    {
        // master-transmitter
        retval = 0;

        instance->state = I2c_state_master_tx;
        instance->txCallback = userFunction;
        instance->txLength = length;
        instance->txRemain = length;
        instance->txHold = 0;
        instance->txIdx = 0;
        // check for non word-aligned buffer as I2C_FIFO_WRITE() works
        // efficiently on words reads from instance->txHold.
        tmp = buffer;
        while (((((intptr_t)tmp) & 0x3) != 0x0) &&
               ((instance->txLength - instance->txIdx) > 0))
        {
            instance->txHold |= ((*tmp++ & 0xff) << (instance->txIdx * 8));
            instance->txIdx++;
        }
        instance->txBuffer = tmp;

        // ensure transfer is underway
        instance->intr_mask_save |= (I2c_irq_tx_empty | I2c_irq_tx_abrt);
    }
    else
    {
        retval = -FMSH_EBUSY;
    }

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function initiates an interrupt-driven slave transmit transfer.
 *  To do this, the I2C must first be properly configured, enabled and
 *  must also receive a read request (I2c_irq_rd_req) from an I2C
 *  master.  This function fills the transmit FIFO and, if there is more
 *  data to send, sets up and enables the transmit interrupts to keep
 *  the FIFO filled.  Upon completion, the callback function is called
 *  (if it is not NULL).
 *
 *  A transfer may be stopped at any time by calling FI2cPs_terminate(),
 *  which returns the number of bytes that were sent before the transfer
 *  was interrupted.  A terminated transfer's callback function is never
 *  called.
 *
 * @param
 *  dev is i2c device handle.
 *  buffer is buffer from which to send data
 *  length is length of transmit buffer/number of bytes to send
 *  callback is function to call when transfer is complete
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- if the I2C is busy (transfer already in progress)
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *
 *  This function enables the tx_empty and tx_abrt interrupts.
 *  This function may only be called from the user listener function
 *  after a read request has been received.
 *  The FMSH_callback function pointer typedef is defined in the common
 *  header files.
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 *
 *  This function cannot be used when using an interrupt handler other
 *  than FI2cPs_irqHandler().
 * SEE ALSO
 *  FI2cPs_masterBack2Back(), FI2cPs_masterTransmit(),
 *  FI2cPs_slaveBulkTransmit(), FI2cPs_masterReceive(),
 *  FI2cPs_slaveReceive(), FI2cPs_terminate()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_slaveTransmit (FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                          FMSH_callback userFunction)
{
    uint8_t *tmp;
    int retval = 0;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(buffer != NULL);

    FMSH_REQUIRE(length != 0);

    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    FMSH_REQUIRE(instance->listener != NULL);

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    if ((instance->state == I2c_state_rd_req) ||
        (instance->state == I2c_state_slave_rx_rd_req))
    {
        // slave-transmitter
        retval = 0;

        instance->txCallback = userFunction;
        instance->txLength = length;
        instance->txRemain = length;
        instance->txHold = 0;
        instance->txIdx = 0;
        // check for non word-aligned buffer as I2C_FIFO_WRITE() works
        // efficiently on words reads from instance->txHold.
        tmp = (uint8_t *)buffer;
        while (((((intptr_t)tmp) & 0x3) != 0x0) &&
               ((instance->txLength - instance->txIdx) > 0))
        {
            instance->txHold |= ((*tmp++ & 0xff) << (instance->txIdx * 8));
            instance->txIdx++;
        }
        // buffer is now word-aligned
        instance->txBuffer = tmp;
        // write only one byte of data to the slave tx fifo
        I2C_FIFO_WRITE(1);

        switch (instance->state)
        {
        case I2c_state_rd_req:
            instance->state = I2c_state_slave_tx;
            break;
        case I2c_state_slave_rx_rd_req:
            instance->state = I2c_state_slave_tx_rx;
            break;
        default:
            FMSH_ASSERT(false);
            break;
        }
        // Note: tx_empty is not enabled here as rd_req is the signal
        // used to write the next byte of data to the tx fifo.
    }
    else
    {
        retval = -FMSH_EPROTO;
    }

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function initiates an interrupt-driven slave transmit transfer.
 *  To do this, the I2C must first be properly configured, enabled and
 *  must also receive a read request (I2c_irq_rd_req) from an I2C
 *  master.  This function fills the transmit FIFO and, if there is more
 *  data to send, sets up and enables the transmit interrupts to keep
 *  the FIFO filled.  Upon completion, the callback function is called
 *  (if it is not NULL).
 *
 *  A transfer may be stopped at any time by calling FI2cPs_terminate(),
 *  which returns the number of bytes that were sent before the transfer
 *  was interrupted.  A terminated transfer's callback function is never
 *  called.
 *
 * @param
 *  dev is i2c device handle.
 *  buffer is buffer from which to send data
 *  length is length of transmit buffer/number of bytes to send
 *  callback is function to call when transfer is complete
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EPROTO  -- if a read request was not received
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *
 *  This function enables the tx_empty and tx_abrt interrupts.
 *  This function may only be called from the user listener function
 *  after a read request has been received.
 *  The FMSH_callback function pointer typedef is defined in the common
 *  header files.
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 *
 *  This function cannot be used when using an interrupt handler other
 *  than FI2cPs_irqHandler().
 * SEE ALSO
 *  FI2cPs_masterBack2Back(), FI2cPs_masterTransmit(),
 *  FI2cPs_slaveTransmit(), FI2cPs_masterReceive(),
 *  FI2cPs_slaveReceive(), FI2cPs_terminate()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_slaveBulkTransmit (FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                              FMSH_callback userFunction)
{
    uint8_t *tmp;
    int retval, maxBytes;
    FIicPs_Param_T *param = NULL;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(buffer != NULL);
    FMSH_REQUIRE(length != 0);

    param = (FIicPs_Param_T *)dev->comp_param;
    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    FMSH_REQUIRE(instance->listener != NULL);

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    if ((instance->state == I2c_state_rd_req) ||
        (instance->state == I2c_state_slave_rx_rd_req))
    {
        // slave-transmitter
        retval = 0;

        instance->txCallback = userFunction;
        instance->txLength = length;
        instance->txRemain = length;
        instance->txHold = 0;
        instance->txIdx = 0;
        // check for non word-aligned buffer as I2C_FIFO_WRITE() works
        // efficiently on words reads from instance->txHold.
        tmp = (uint8_t *)buffer;
        while (((((intptr_t)tmp) & 0x3) != 0x0) &&
               ((instance->txLength - instance->txIdx) > 0))
        {
            instance->txHold |= ((*tmp++ & 0xff) << (instance->txIdx * 8));
            instance->txIdx++;
        }
        // buffer is now word-aligned
        instance->txBuffer = tmp;
        // maximum available space in the tx fifo
        maxBytes = param->tx_buffer_depth - FI2cPs_getTxFifoLevel(dev);
        I2C_FIFO_WRITE(maxBytes);
        switch (instance->state)
        {
        case I2c_state_rd_req:
            instance->state = I2c_state_slave_bulk_tx;
            break;
        case I2c_state_slave_rx_rd_req:
            instance->state = I2c_state_slave_bulk_tx_rx;
            break;
        default:
            FMSH_ASSERT(false);
            break;
        }
        // ensure transfer is underway
        instance->intr_mask_save |= (I2c_irq_tx_empty | I2c_irq_tx_abrt);
    }
    else
    {
        retval = -FMSH_EPROTO;
    }

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function initiates an interrupt-driven master receive transfer.
 *  To do this, the I2C must first be properly configured and enabled.
 *  This function sets up the transmit FIFO to be loaded with read
 *  commands.  In parallel, this function sets up and enables the
 *  receive interrupt to fill the buffer from the receive FIFO (the same
 *  number of times as writes to the transmit FIFO).  Upon completion,
 *  the callback function is called (if it is not NULL).
 *
 *  A transfer may be stopped at any time by calling FI2cPs_terminate(),
 *  which returns the number of bytes that were received before the
 *  transfer was interrupted.  A terminated transfer's callback function
 *  is never called.
 *
 * @param
 *  dev is i2c device handle.
 *  buffer is buffer from which to send data
 *  length is length of transmit buffer/number of bytes to send
 *  callback is function to call when transfer is complete
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- if the I2C is busy (transfer already in progress)
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *   - ic_rx_tl/rx_tl
 *
 *  This function enables the tx_empty, tx_abrt & rx_full interrupts.
 *  The FMSH_callback function pointer typedef is defined in the common
 *  header files.
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 *
 *  This function cannot be used when using an interrupt handler other
 *  than FI2cPs_irqHandler().
 * SEE ALSO
 *  FI2cPs_masterBack2Back(), FI2cPs_masterTransmit(),
 *  FI2cPs_slaveTransmit(), FI2cPs_slaveBulkTransmit(),
 *  FI2cPs_slaveReceive(), FI2cPs_terminate()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_masterReceive (FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                          FMSH_callback userFunction)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(buffer != NULL);
    FMSH_REQUIRE(length != 0);

    instance = (FIicPs_Instance_T *)dev->instance;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    FMSH_REQUIRE(instance->listener != NULL);

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    if (instance->state == I2c_state_idle)
    {
        retval = 0;
        instance->state = I2c_state_master_rx;
        // check if rx buffer is not word-aligned
        if (((intptr_t)buffer & 0x3) == 0)
        {
            instance->rxAlign = true;
        }
        else
        {
            instance->rxAlign = false;
        }

        // set rx fifo threshold if necessary
        if (length <= instance->rxThreshold)
        {
            reg = 0;
            FMSH_BIT_SET(reg, I2C_RX_TL_RX_TL, length - 1);
            I2C_OUTP(reg, portmap->rx_tl);
        }

        // set transfer variables
        instance->rxBuffer = buffer;
        instance->rxLength = length;
        instance->rxRemain = length;
        instance->txRemain = length;
        instance->rxCallback = userFunction;
        instance->rxIdx = 4;
        // restore interrupts and ensure master-receive is underway
        instance->intr_mask_save |= (I2c_irq_rx_full | I2c_irq_tx_empty |
                                     I2c_irq_tx_abrt);
    }
    else
    {
        retval = -FMSH_EBUSY;
    }

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function initiates an interrupt-driven slave receive transfer.
 *  To do this, the I2C must first be properly configured and enabled.
 *  This function sets up and enables the receive interrupt to fill the
 *  buffer from the receive FIFO.  Upon completion, the callback
 *  function is called (if it is not NULL).
 *
 *  A transfer may be stopped at any time by calling FI2cPs_terminate(),
 *  which returns the number of bytes that were received before the
 *  transfer was interrupted.  A terminated transfer's callback function
 *  is never called.
 *
 * @param
 *  dev is i2c device handle.
 *  buffer is buffer to write received data to
 *  length is length of buffer/max number of bytes to receive
 *  callback is function to call when transfer is complete
 *
 * @return
 *  0           -- if successful
 *  -FMSH_EBUSY   -- if the I2C is busy (transfer already in progress)
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *   - ic_rx_tl/rx_tl
 *
 *  This function enables the rx_full interrupt.
 *  The FMSH_callback function pointer typedef is defined in the common
 *  header files.
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 *
 *  This function cannot be used when using an interrupt handler other
 *  than FI2cPs_irqHandler().
 * SEE ALSO
 *  FI2cPs_masterBack2Back(), FI2cPs_masterTransmit(),
 *  FI2cPs_masterReceive(), FI2cPs_slaveTransmit(),
 *  FI2cPs_slaveBulkTransmit(), FI2cPs_terminate()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_slaveReceive (FI2cPs_T *dev, uint8_t *buffer, uint32_t length,
                         FMSH_callback userFunction)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(buffer != NULL);
    FMSH_REQUIRE(length != 0);

    instance = (FIicPs_Instance_T *)dev->instance;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    FMSH_REQUIRE(instance->listener != NULL);

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    if ((instance->state == I2c_state_rx_req) ||
        (instance->state == I2c_state_idle) ||
        (instance->state == I2c_state_slave_tx) ||
        (instance->state == I2c_state_slave_bulk_tx) ||
        (instance->state == I2c_state_slave_tx_rx_req) ||
        (instance->state == I2c_state_slave_bulk_tx_rx_req) ||
        (instance->state == I2c_state_master_tx_gen_call))
    {
        retval = 0;
        // in case the state was idle
        switch (instance->state)
        {
        case I2c_state_idle:
        case I2c_state_rx_req:
            instance->state = I2c_state_slave_rx;
            break;
        case I2c_state_slave_tx:
        case I2c_state_slave_tx_rx_req:
            instance->state = I2c_state_slave_tx_rx;
            break;
        case I2c_state_slave_bulk_tx:
        case I2c_state_slave_bulk_tx_rx_req:
            instance->state = I2c_state_slave_bulk_tx_rx;
            break;
        case I2c_state_master_tx_gen_call:
            instance->state = I2c_state_master_tx_slave_rx;
            break;
        default:
            FMSH_ASSERT(false);
            break;
        }
        if (((intptr_t)buffer & 0x3) == 0)
        {
            instance->rxAlign = true;
        }
        else
        {
            instance->rxAlign = false;
        }

        // set rx fifo threshold if necessary
        if (length <= instance->rxThreshold)
        {
            reg = 0;
            FMSH_BIT_SET(reg, I2C_RX_TL_RX_TL, length - 1);
            I2C_OUTP(reg, portmap->rx_tl);
        }

        // set transfer variables
        instance->rxBuffer = buffer;
        instance->rxLength = length;
        instance->rxRemain = length;
        instance->rxCallback = userFunction;
        instance->rxIdx = 4;

        // ensure receive is underway
        instance->intr_mask_save |= I2c_irq_rx_full;
    }
    else
    {
        retval = -FMSH_EBUSY;
    }

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function terminates the current I2C interrupt-driven transfer
 *  in progress, if any.  This function must be called to end an
 *  unfinished interrupt-driven transfer as driver instability would
 *  ensue otherwise.
 *  Any data received after calling this function is treated as a new
 *  transfer by the driver.  Therefore, it would be prudent to wait
 *  until the next detected stop condition when receiving data in order
 *  to avoid a misalignment between the device and driver.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The number of bytes sent/received during the interrupted transfer,
 *  if any.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *   - ic_rxflr/rxflr
 *   - ic_data_cmd/dat
 *   - ic_rx_tl/rx_tl
 *
 *  This function is part of the interrupt-driven interface and should
 *  not be called when using the I2C in a poll-driven manner.
 *  This function disables the tx_empty and enables the rx_full
 *  interrupts.
 *  This function restores the receive FIFO threshold to the previously
 *  user-specified value.
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 *
 *  This function cannot be used when using an interrupt handler other
 *  than FI2cPs_irqHandler().
 * SEE ALSO
 *  FI2cPs_masterBack2Back(), FI2cPs_masterTransmit(),
 *  FI2cPs_masterReceive(), FI2cPs_slaveTransmit(),
 *  FI2cPs_slaveBulkTransmit(), FI2cPs_slaveReceive()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_terminate (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    int retval, maxBytes;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    instance = (FIicPs_Instance_T *)dev->instance;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    FMSH_REQUIRE(instance->listener != NULL);

    // Critical section of code.  Shared data needs to be protected.
    // This macro disables FMSH_apb_i2c interrupts.
    I2C_ENTER_CRITICAL_SECTION();

    // disable tx interrupt
    if ((instance->state == I2c_state_master_tx) ||
        (instance->state == I2c_state_back2back) ||
        (instance->state == I2c_state_slave_tx) ||
        (instance->state == I2c_state_slave_tx_rx) ||
        (instance->state == I2c_state_slave_bulk_tx) ||
        (instance->state == I2c_state_slave_bulk_tx_rx) ||
        (instance->state == I2c_state_master_tx_slave_rx))
    {
        // ensure tx empty is not re-enabled when interrupts are
        // restored
        instance->intr_mask_save &= ~I2c_irq_tx_empty;
    }

    // flush rx fifo if necessary
    if ((instance->state == I2c_state_master_rx) ||
        (instance->state == I2c_state_slave_rx) ||
        (instance->state == I2c_state_slave_tx_rx) ||
        (instance->state == I2c_state_slave_bulk_tx_rx) ||
        (instance->state == I2c_state_master_tx_slave_rx) ||
        (instance->state == I2c_state_back2back))
    {
        maxBytes = FI2cPs_getRxFifoLevel(dev);
        I2C_FIFO_READ(maxBytes);
        (void)FI2cPs_flushRxHold(dev);
        // number of bytes that were received during the last transfer
        retval = instance->rxLength - instance->rxRemain;
    }
    else
    {
        // number of bytes that were sent during the last transfer
        retval = instance->txLength - instance->txRemain;
    }
    // sanity check .. retval should never be less than zero
    FMSH_ASSERT(retval >= 0);

    // terminate current transfer
    instance->state = I2c_state_idle;
    instance->txCallback = NULL;
    instance->txBuffer = NULL;
    instance->rxCallback = NULL;
    instance->rxBuffer = NULL;

    // restore user-specified tx/rx fifo threshold
    reg = 0;
    FMSH_BIT_SET(reg, I2C_TX_TL_TX_TL, instance->txThreshold);
    I2C_OUTP(reg, portmap->tx_tl);
    reg = 0;
    FMSH_BIT_SET(reg, I2C_RX_TL_RX_TL, instance->rxThreshold);
    I2C_OUTP(reg, portmap->rx_tl);

    // End of critical section of code. This macros restores FMSH_apb_i2c
    // interrupts.
    I2C_EXIT_CRITICAL_SECTION();

    // sanity check
    FMSH_ASSERT(FI2cPs_isIrqMasked(dev, I2c_irq_tx_empty) == true);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Unmasks specified I2C interrupt(s).
 *
 * @param
 *  dev is i2c device handle.
 *  interrupts is interrupt(s) to enable
 *
 * @return
 *  none
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 * SEE ALSO
 *  FI2cPs_maskIrq(), FI2cPs_clearIrq(), FI2cPs_isIrqMasked(),
 *  FI2cPs_Irq_T
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_unmaskIrq (FI2cPs_T *dev, FIicPs_Irq_T interrupts)
{
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    reg = I2C_INP(portmap->intr_mask);
    // avoid bus write if irq already enabled
    if ((interrupts & reg) != interrupts)
    {
        reg |= interrupts;
        // save current value of interrupt mask register
        instance->intr_mask_save = reg;
        I2C_OUTP(reg, portmap->intr_mask);
    }
}

/*****************************************************************************
 *
 * @description
 *  Masks specified I2C interrupt(s).
 *
 * @param
 *  dev is i2c device handle.
 *  interrupts is interrupt(s) to disable
 *
 * @return
 *  none
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 * SEE ALSO
 *  FI2cPs_unmaskIrq(), FI2cPs_clearIrq(), FI2cPs_isIrqMasked(),
 *  FIicPs_Irq_T
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_maskIrq (FI2cPs_T *dev, FIicPs_Irq_T interrupts)
{
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    reg = I2C_INP(portmap->intr_mask);
    // avoid bus write if interrupt(s) already disabled
    if ((interrupts & reg) != 0)
    {
        reg &= ~interrupts;
        // save current value of interrupt mask register
        instance->intr_mask_save = reg;
        I2C_OUTP(reg, portmap->intr_mask);
    }
}

/*****************************************************************************
 *
 * @description
 *  Clears specified I2C interrupt(s).  Only the following interrupts
 *  can be cleared in this fashion: rx_under, rx_over, tx_over, rd_req,
 *  tx_abrt, rx_done, activity, stop_det, start_det, gen_call.  Although
 *  they can be specified, the tx_empty and rd_req interrupts cannot be
 *  cleared using this function.
 *
 * @param
 *  dev is i2c device handle.
 *  interrupts is interrupt(s) to clear
 *
 * @return
 *  none
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_clr_intr/clr_intr
 *   - ic_clr_rx_under/clr_rx_under
 *   - ic_clr_rx_over/clr_rx_over
 *   - ic_clr_tx_over/clr_tx_over
 *   - ic_clr_rd_req/clr_rd_req
 *   - ic_clr_tx_abrt/clr_tx_abrt
 *   - ic_clr_rx_done/clr_rx_done
 *   - ic_clr_activity/clr_activity
 *   - ic_clr_stop_det/clr_stop_det
 *   - ic_clr_start_det/clr_start_det
 *   - ic_clr_gen_call/clr_gen_call
 * SEE ALSO
 *  FI2cPs_unmaskIrq(), FI2cPs_maskIrq(), FI2cPs_isIrqMasked(),
 *  FIicPs_Irq_T
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_clearIrq (FI2cPs_T *dev, FIicPs_Irq_T interrupts)
{
    volatile uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (interrupts == I2c_irq_all)
    {
        reg = I2C_INP(portmap->clr_intr);
    }
    else
    {
        if ((interrupts & I2c_irq_rx_under) != 0)
        {
            reg = I2C_INP(portmap->clr_rx_under);
        }
        if ((interrupts & I2c_irq_rx_over) != 0)
        {
            reg = I2C_INP(portmap->clr_rx_over);
        }
        if ((interrupts & I2c_irq_tx_over) != 0)
        {
            reg = I2C_INP(portmap->clr_tx_over);
        }
        if ((interrupts & I2c_irq_rd_req) != 0)
        {
            reg = I2C_INP(portmap->clr_rd_req);
        }
        if ((interrupts & I2c_irq_tx_abrt) != 0)
        {
            reg = I2C_INP(portmap->clr_tx_abrt);
        }
        if ((interrupts & I2c_irq_rx_done) != 0)
        {
            reg = I2C_INP(portmap->clr_rx_done);
        }
        if ((interrupts & I2c_irq_activity) != 0)
        {
            reg = I2C_INP(portmap->clr_activity);
        }
        if ((interrupts & I2c_irq_stop_det) != 0)
        {
            reg = I2C_INP(portmap->clr_stop_det);
        }
        if ((interrupts & I2c_irq_start_det) != 0)
        {
            reg = I2C_INP(portmap->clr_start_det);
        }
        if ((interrupts & I2c_irq_restart_det) != 0)
        {
            reg = I2C_INP(portmap->clr_restart_det);
        }
        if ((interrupts & I2c_irq_gen_call) != 0)
        {
            reg = I2C_INP(portmap->clr_gen_call);
        }
        if ((interrupts & I2c_scl_stuck_at_low) != 0)
        {
            reg = I2C_INP(portmap->clr_scl_stuck_det);
        }
    }
}

/*****************************************************************************
 *
 * @description
 *  Returns whether the specified I2C interrupt is masked or not.  Only
 *  one interrupt can be specified at a time.
 *
 * @param
 *  dev is i2c device handle.
 *  interrupts is interrupt to check
 *
 * @return
 *  true        -- interrupt is enabled
 *  false       -- interrupt is disabled
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 * SEE ALSO
 *  FI2cPs_unmaskIrq(), FI2cPs_maskIrq(), FI2cPs_clearIrq(),
 *  FI2cPs_getIrqMask(), FIicPs_Irq_T
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isIrqMasked (FI2cPs_T *dev, FIicPs_Irq_T interrupt)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(
        (interrupt == I2c_irq_rx_under) || (interrupt == I2c_irq_rx_over) ||
        (interrupt == I2c_irq_rx_full) || (interrupt == I2c_irq_tx_over) ||
        (interrupt == I2c_irq_tx_empty) || (interrupt == I2c_irq_rd_req) ||
        (interrupt == I2c_irq_tx_abrt) || (interrupt == I2c_irq_rx_done) ||
        (interrupt == I2c_irq_activity) || (interrupt == I2c_irq_stop_det) ||
        (interrupt == I2c_irq_start_det) || (interrupt == I2c_irq_gen_call));

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->intr_mask);
    reg &= interrupt;

    if (reg == 0)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Returns the current interrupt mask.  For each bitfield, a value of
 *  '0' indicates that an interrupt is masked while a value of '1'
 *  indicates that an interrupt is enabled.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  The DW_apb_i2c interrupt mask.
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 * SEE ALSO
 *  FI2cPs_unmaskIrq(), FI2cPs_maskIrq(), FI2cPs_clearIrq(),
 *  FI2cPs_isIrqMasked(), FIicPs_Irq_T
 * SOURCE
 *
 ****************************************************************************/
uint32_t FI2cPs_getIrqMask (FI2cPs_T *dev)
{
    uint32_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    retval = I2C_INP(portmap->intr_mask);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Returns whether an I2C interrupt is active or not, after the masking
 *  stage.
 *
 * @param
 *  dev is i2c device handle.
 *  interrupt is interrupt to check
 *
 * @return
 *  true        -- irq is active
 *  false       -- irq is inactive
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_stat/all bits
 * SEE ALSO
 *  FI2cPs_isRawIrqActive(), FI2cPs_unmaskIrq(), FI2cPs_maskIrq(),
 *  FI2cPs_isIrqMasked(), FI2cPs_clearIrq(), FI2cPs_Irq_T
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isIrqActive (FI2cPs_T *dev, FIicPs_Irq_T interrupt)
{
    bool retval = false;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(
        (interrupt == I2c_irq_rx_under) || (interrupt == I2c_irq_rx_over) ||
        (interrupt == I2c_irq_rx_full) || (interrupt == I2c_irq_tx_over) ||
        (interrupt == I2c_irq_tx_empty) || (interrupt == I2c_irq_rd_req) ||
        (interrupt == I2c_irq_tx_abrt) || (interrupt == I2c_irq_rx_done) ||
        (interrupt == I2c_irq_activity) || (interrupt == I2c_irq_stop_det) ||
        (interrupt == I2c_irq_start_det) || (interrupt == I2c_irq_gen_call));

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->intr_stat);
    reg &= interrupt;

    if (reg == 0)
    {
        retval = false;
    }
    else
    {
        retval = true;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  Returns whether an I2C raw interrupt is active or not, regardless of
 *  masking.
 *
 * @param
 *  dev is i2c device handle.
 *  interrupt is interrupt to check
 *
 * @return
 *  true        -- irq is active
 *  false       -- irq is inactive
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_raw_intr_stat/all bits
 * SEE ALSO
 *  FI2cPs_isIrqActive(), FI2cPs_unmaskIrq(), FI2cPs_maskIrq(),
 *  FI2cPs_isIrqMasked(), FI2cPs_clearIrq(), FIicPs_Irq_T
 * SOURCE
 *
 ****************************************************************************/
bool FI2cPs_isRawIrqActive (FI2cPs_T *dev, FIicPs_Irq_T interrupt)
{
    bool retval = false;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);
    FMSH_REQUIRE(
        (interrupt == I2c_irq_rx_under) || (interrupt == I2c_irq_rx_over) ||
        (interrupt == I2c_irq_rx_full) || (interrupt == I2c_irq_tx_over) ||
        (interrupt == I2c_irq_tx_empty) || (interrupt == I2c_irq_rd_req) ||
        (interrupt == I2c_irq_tx_abrt) || (interrupt == I2c_irq_rx_done) ||
        (interrupt == I2c_irq_activity) || (interrupt == I2c_irq_stop_det) ||
        (interrupt == I2c_irq_start_det) || (interrupt == I2c_irq_gen_call));

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->raw_intr_stat);
    reg &= interrupt;

    if (reg == 0)
    {
        retval = false;
    }
    else
    {
        retval = true;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  This function handles and processes I2C interrupts.  It works in
 *  conjunction with the Interrupt API and a user listener function
 *  to manage interrupt-driven transfers.  When fully using the
 *  Interrupt API, this function should be called whenever a DW_apb_i2c
 *  interrupt occurs.  There is an alternate interrupt handler
 *  available, FI2cPs_userIrqHandler(), but this cannot be used in
 *  conjunction with the other Interrupt API functions.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- an interrupt was processed
 *  false       -- no interrupt was processed
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_mask/all bits
 *   - ic_intr_stat/all bits
 *   - ic_clr_rx_under/clr_rx_under
 *   - ic_clr_rx_over/clr_rx_over
 *   - ic_clr_tx_over/clr_tx_over
 *   - ic_clr_rd_req/clr_rd_req
 *   - ic_clr_tx_abrt/clr_tx_abrt
 *   - ic_clr_rx_done/clr_rx_done
 *   - ic_clr_activity/clr_activity
 *   - ic_clr_stop_det/clr_stop_det
 *   - ic_clr_start_det/clr_start_det
 *   - ic_clr_gen_call/clr_gen_call
 *   - ic_status/rfne
 *   - ic_rxflr/rxflr
 *   - ic_data_cmd/cmd
 *   - ic_data_cmd/dat
 *   - ic_rx_tl/rx_tl
 *   - ic_txflr/txflr
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 * WARNINGS
 *  The user listener function is run in interrupt context and, as such,
 *  care must be taken to ensure that any data shared between it and
 *  normal code is adequately protected from corruption.  Depending on
 *  the target platform, spinlocks, mutexes or semaphores may be used to
 *  achieve this.  The other Interrupt API functions disable I2C
 *  interrupts before entering critical sections of code to avoid any
 *  shared data issues.
 * SEE ALSO
 *  FI2cPs_masterTransmit(), FI2cPs_masterReceive(),
 *  FI2cPs_masterBack2Back(),  FI2cPs_slaveTransmit(),
 *  FI2cPs_slaveReceive(), FI2cPs_terminate(),
 *  FI2cPs_slaveBulkTransmit()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_irqHandler (FI2cPs_T *dev)
{
    int retval = 0;
    uint8_t *tmp;
    uint32_t reg = 0U;
    int i, maxBytes;
    int32_t callbackArg;
    FMSH_callback userCallback;
    FIicPs_Irq_T clearIrqMask;
    FIicPs_Param_T *param = NULL;
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    param = (FIicPs_Param_T *)dev->comp_param;
    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    userCallback = NULL;
    callbackArg = 0;
    clearIrqMask = I2c_irq_none;
    // Assume an interrupt will be processed.  This will be set to false
    // if no active interrupt is found.
    retval = true;

    // what caused the interrupt?
    reg = I2C_INP(portmap->intr_stat);

    // if an error has occurred
    if ((reg & (I2c_irq_tx_abrt | I2c_irq_rx_over | I2c_irq_rx_under |
                I2c_irq_tx_over)) != 0)
    {
        instance->state = I2c_state_error;
        userCallback = instance->listener;
        FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
        // if a tx transfer was aborted
        if ((reg & I2c_irq_tx_abrt) != 0)
        {
            callbackArg = I2c_irq_tx_abrt;
            clearIrqMask = I2c_irq_tx_abrt;
        }
        // rx fifo overflow
        else if ((reg & I2c_irq_rx_over) != 0)
        {
            callbackArg = I2c_irq_rx_over;
            clearIrqMask = I2c_irq_rx_over;
        }
        // rx fifo underflow
        else if ((reg & I2c_irq_rx_under) != 0)
        {
            callbackArg = I2c_irq_rx_under;
            clearIrqMask = I2c_irq_rx_under;
        }
        // tx fifo overflow
        else if ((reg & I2c_irq_tx_over) != 0)
        {
            callbackArg = I2c_irq_tx_over;
            clearIrqMask = I2c_irq_tx_over;
        }
        else{
            ;/* no deal with */
        }
    }
    // a general call was detected
    else if ((reg & I2c_irq_gen_call) != 0)
    {
        userCallback = instance->listener;
        callbackArg = I2c_irq_gen_call;
        clearIrqMask = I2c_irq_gen_call;
        // update state -- awaiting user to start a slave rx transfer
        switch (instance->state)
        {
        case I2c_state_idle:
            instance->state = I2c_state_rx_req;
            break;
        case I2c_state_master_tx:
            instance->state = I2c_state_master_tx_gen_call;
            break;
        case I2c_state_slave_rx:
            // leave state unchanged; user already has an rx buffer
            // set up to receive data.
            break;
        default:
            // should never reach this clause
            FMSH_ASSERT(false);
            break;
        }
    }
    // rx fifo level at or above threshold
    else if ((reg & I2c_irq_rx_full) != 0)
    {
        /*   // The rx full interrupt should not be unmasked when a DMA
           // interface with hardware handshaking is being used.
           //FMSH_REQUIRE(instance->dmaRx.mode != FMSH_dma_hw_handshake);
           //if(instance->dmaRx.mode == FMSH_dma_sw_handshake)
           {
               // The user must have previously set an rx notifier via
               // FI2cPs_setDmaRxNotifier.
               //FMSH_REQUIRE(instance->dmaRx.notifier != NULL);
               // Disable the Rx full interrupt .. this is re-enabled
               // after the DMA has finished the current transfer (via a
               // callback set in the DMA driver by the user).
              // FI2cPs_maskIrq(dev, I2c_irq_rx_full);
               // Notify the DMA that the Rx FIFO has data to be read.
               // This function and its arguments are set by the user via
               // the FI2cPs_setNotifier_sourceReady() function.
               //(instance->dmaRx.notifier)(instance->dmaRx.dmac,
               //                           instance->dmaRx.channel,
               //                           false, false);
           }
          else */
        {
            if ((instance->state == I2c_state_idle) ||
                (instance->state == I2c_state_slave_tx) ||
                (instance->state == I2c_state_slave_bulk_tx))
            {
                // sanity check: rxBuffer should be NULL in these states
                FMSH_ASSERT(instance->rxBuffer == NULL);
                // inform the user listener function of the event
                userCallback = instance->listener;
                callbackArg = I2c_irq_rx_full;
                switch (instance->state)
                {
                case I2c_state_idle:
                    instance->state = I2c_state_rx_req;
                    break;
                case I2c_state_slave_tx:
                    instance->state = I2c_state_slave_tx_rx_req;
                    break;
                case I2c_state_slave_bulk_tx:
                    instance->state = I2c_state_slave_bulk_tx_rx_req;
                    break;
                default:
                    break;
                }
            }
            else
            {
                FMSH_ASSERT(instance->rxBuffer != NULL);
                // does the rx buffer need to be word-aligned?
                if (instance->rxAlign == false)
                {
                    // align buffer:
                    tmp = (uint8_t *)instance->rxBuffer;
                    // repeat until either the buffer is aligned, there
                    // is no more space in the rx buffer or there is no
                    // more data to read from the rx fifo
                    while ((((UINTPTR)tmp) & 0x3) && (instance->rxRemain > 0) &&
                           (FI2cPs_isRxFifoEmpty(dev) == false))
                    {
                        *tmp++ = I2C_IN16P(portmap->data_cmd);
                        instance->rxRemain--;
                    }
                    instance->rxBuffer = tmp;
                    if (((UINTPTR)tmp & 0x3) == 0)
                    {
                        instance->rxAlign = true;
                    }
                }  // instance->rxAlign == false
                // This code is only executed when the rx buffer is
                // word-aligned as I2C_FIFO_READ works efficiently with
                // a word-aligned buffer.
                if (instance->rxAlign == true)
                {
                    maxBytes = FI2cPs_getRxFifoLevel(dev);
                    I2C_FIFO_READ(maxBytes);
                }  // instance->rxAlign == true
                // if the rx buffer is full
                if (instance->rxRemain == 0)
                {
                    // Prepare to call the user callback function to
                    // notify it that the current transfer has finished.
                    // For an rx or back-to-back transfer, the number of
                    // bytes received is passed as an argument to the
                    // listener function.
                    userCallback = instance->rxCallback;
                    callbackArg = instance->rxLength;
                    // Flush the instance->rxHold regsiter to the rx
                    // buffer.
                    (void)FI2cPs_flushRxHold(dev);
                    // transfer complete
                    instance->rxBuffer = NULL;
                    instance->rxCallback = NULL;
                    // restore rx threshold to user-specified value
                    I2C_OUTP(instance->rxThreshold, portmap->rx_tl);
                    // update state
                    switch (instance->state)
                    {
                    case I2c_state_master_rx:
                        // End of master-receiver transfer.  Ensure
                        // that the tx empty interrupt is disabled.
                        FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
                    case I2c_state_slave_rx:
                        // return to idle state
                        instance->state = I2c_state_idle;
                        break;
                    case I2c_state_back2back:
                        // Back-to-back transfer is complete if
                        // there is no more data to send.  Else, the
                        // callback function is not called until all
                        // bytes have been transmitted.  Note that
                        // txCallback is cleared when all bytes have
                        // been sent and/or received.
                        if (instance->txCallback == NULL)
                        {
                            instance->state = I2c_state_idle;
                            FMSH_ASSERT(FI2cPs_isIrqMasked(
                                            dev, I2c_irq_tx_empty) == true);
                        }
                        else
                        {
                            userCallback = NULL;
                        }
                        break;
                    case I2c_state_slave_tx_rx:
                        instance->state = I2c_state_slave_tx;
                        break;
                    case I2c_state_slave_bulk_tx_rx:
                        instance->state = I2c_state_slave_bulk_tx;
                        break;
                    case I2c_state_master_tx_slave_rx:
                        instance->state = I2c_state_master_tx;
                        break;
                    default:
                        // this clause should never be reached
                        FMSH_ASSERT(false);
                        break;
                    }
                }  // remain == 0
                else if (instance->rxRemain < (instance->rxThreshold + 1))
                {
                    reg = 0;
                    FMSH_BIT_SET(reg, I2C_RX_TL_RX_TL,
                                 (instance->rxRemain - 1));
                    I2C_OUTP(reg, portmap->rx_tl);
                }
                else{
                    ;/* no deal with */
                }
            }  // instance->rxBuffer != NULL
        }
    }  // (reg & I2c_irq_rx_full) != 0
    // read-request transfer completed (tx fifo may still contain data)
    else if ((reg & I2c_irq_rx_done) != 0)
    {
        clearIrqMask = I2c_irq_rx_done;
        switch (instance->state)
        {
        case I2c_state_slave_tx:
            // return to idle state if tx transfer finished
            if (instance->txRemain == 0)
            {
                instance->state = I2c_state_idle;
                FMSH_ASSERT(FI2cPs_isIrqMasked(dev, I2c_irq_tx_empty) == true);
                callbackArg = 0;
                // call user tx callback function
                userCallback = instance->txCallback;
                // clear tx buffer and callback function pointers
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
            }
            break;
        case I2c_state_slave_tx_rx:
            // return to slave-rx state if tx transfer finished
            if (instance->txRemain == 0)
            {
                instance->state = I2c_state_slave_rx;
                callbackArg = 0;
                // call user tx callback function
                userCallback = instance->txCallback;
                // clear tx buffer and callback function pointers
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
            }
            break;
        case I2c_state_slave_bulk_tx:
            // mask tx empty interrupt
            FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
            // return to idle state
            instance->state = I2c_state_idle;
            // call user tx callback function
            userCallback = instance->txCallback;
            // number of bytes left unsent
            callbackArg = instance->txRemain + FI2cPs_getTxFifoLevel(dev);
            // clear tx buffer and callback function pointers
            instance->txBuffer = NULL;
            instance->txCallback = NULL;
            break;
        case I2c_state_slave_bulk_tx_rx:
            // mask tx empty interrupt
            FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
            // return to slave rx state
            instance->state = I2c_state_slave_rx;
            // call user tx callback function
            userCallback = instance->txCallback;
            // number of bytes left unsent
            callbackArg = instance->txRemain + FI2cPs_getTxFifoLevel(dev);
            // clear tx buffer and callback function pointers
            instance->txBuffer = NULL;
            instance->txCallback = NULL;
            break;
        default:
            // should not get rx_done in any other driver state
            FMSH_ASSERT(false);
            break;
        }
    }
    // read request received
    else if ((reg & I2c_irq_rd_req) != 0)
    {
        switch (instance->state)
        {
        case I2c_state_idle:
            clearIrqMask = I2c_irq_rd_req;
            instance->state = I2c_state_rd_req;
            userCallback = instance->listener;
            callbackArg = I2c_irq_rd_req;
            break;
        case I2c_state_slave_rx:
            clearIrqMask = I2c_irq_rd_req;
            instance->state = I2c_state_slave_rx_rd_req;
            userCallback = instance->listener;
            callbackArg = I2c_irq_rd_req;
            break;
        case I2c_state_slave_tx:
        case I2c_state_slave_tx_rx:
            clearIrqMask = I2c_irq_rd_req;
            // remain in the current state and write the next byte
            // from the tx buffer to the tx fifo
            I2C_FIFO_WRITE(1);
            break;
        case I2c_state_slave_bulk_tx_rx:
            // A read request has occurred because, even though we
            // are performing a slave bulk transfer, the system did
            // not keep the tx FIFO from emptying.  This interrupt
            // is therefore treated the same as a tx empty
            // interrupt.
            maxBytes = MIN(param->tx_buffer_depth - FI2cPs_getTxFifoLevel(dev),
                           instance->txRemain);
            // buffer should be word-aligned (done by
            // FI2cPs_slaveBulkTransmit)
            if (instance->txRemain > 0)
            {
                clearIrqMask = I2c_irq_rd_req;
                I2C_FIFO_WRITE(maxBytes);
            }
            else
            {
                // tx buffer has all been sent in bulk mode yet the
                // master is still requesting more data.  We need to
                // call the tx callback function first and then pass
                // read request to the user listener.
                // update state
                instance->state = I2c_state_slave_rx;
                // mask tx empty interrupt
                FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
                // call user callback function with no bytes left to
                // send
                userCallback = instance->txCallback;
                callbackArg = 0;
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
            }
            break;
        case I2c_state_slave_bulk_tx:
            // A read request has occurred because, even though we
            // are performing a slave bulk transfer, the system did
            // not keep the tx FIFO from emptying.  This interrupt
            // is therefore treated the same as a tx empty
            // interrupt.
            maxBytes = MIN(param->tx_buffer_depth - FI2cPs_getTxFifoLevel(dev),
                           instance->txRemain);
            // buffer should be word-aligned (done by
            // FI2cPs_slaveBulkTransmit)
            if (instance->txRemain > 0)
            {
                clearIrqMask = I2c_irq_rd_req;
                I2C_FIFO_WRITE(maxBytes);
            }
            else
            {
                // tx buffer has all been sent in bulk mode yet the
                // master is still requesting more data.  We need to
                // call the tx callback function first and then pass
                // read request to the user listener.
                // update state
                instance->state = I2c_state_idle;
                // mask tx empty interrupt
                FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
                FMSH_ASSERT(FI2cPs_isIrqMasked(dev, I2c_irq_tx_empty) == true);
                // call user callback function with no bytes left to
                // send
                userCallback = instance->txCallback;
                callbackArg = 0;
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
            }
            break;
        default:
            // should not get rd_req in any other driver state
            FMSH_ASSERT(false);
            break;
        }
    }
    // tx fifo level at or below threshold
    else if ((reg & I2c_irq_tx_empty) != 0)
    {
        // The tx empty interrupt should never be unmasked when we are
        // using DMA with hardware handshaking.
        // FMSH_REQUIRE(instance->dmaTx.mode != FMSH_dma_hw_handshake);
        // if(instance->dmaTx.mode == FMSH_dma_sw_handshake)
        {
            // The user must have previously set a tx notifier.
            // FMSH_REQUIRE(instance->dmaTx.notifier != NULL);
            // Disable the tx empty interrupt .. this is re-enabled
            // after the DMA has finished the current transfer (via a
            // callback set by the user).
            // FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
            // Notify the dma that the tx fifo is ready to receive mode
            // data.  This function and its arguments are set by the
            // user via the FI2cPs_setNotifier_destinationReady()
            // function.
            /*(instance->dmaTx.notifier)(instance->dmaTx.dmac,
                                       instance->dmaTx.channel,
                                       false, false);*/
        }
        // else
        if (instance->txRemain == 0)
        {
            // default: call callback function with zero as argument (no
            // bytes left to send)
            // tx callback function
            userCallback = instance->txCallback;
            // number of bytes left to transmit
            callbackArg = 0;
            FI2cPs_maskIrq(dev, I2c_irq_tx_empty);
            // restore user-specfied tx threshold value
            I2C_OUTP(instance->txThreshold, portmap->tx_tl);
            // update driver state
            switch (instance->state)
            {
            case I2c_state_master_tx:
                // return to idle state at end of tx transfer
                instance->state = I2c_state_idle;
                FMSH_ASSERT(FI2cPs_isIrqMasked(dev, I2c_irq_tx_empty) == true);
                // transfer complete
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
                break;
            case I2c_state_master_tx_slave_rx:
                // return to slave-rx state if slave rx transfer
                // is still in progress
                instance->state = I2c_state_slave_rx;
                // transfer complete
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
                break;
            case I2c_state_master_rx:
                // Reset tx buffer and callback function pointers
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
                // For a master-rx transfer, the callback is not
                // called until the last byte has been received.
                userCallback = NULL;
                callbackArg = NULL;
                break;
            case I2c_state_slave_bulk_tx:
            case I2c_state_slave_bulk_tx_rx:
                // For slave bulk transfers, the callback is not
                // called until rx_done has been received.
                userCallback = NULL;
                callbackArg = NULL;
                break;
            case I2c_state_back2back:
                if (instance->rxCallback == NULL)
                {
                    // If there is no more data to receive either,
                    // return to the idle state and call callback
                    // with the number of bytes received.
                    instance->state = I2c_state_idle;
                    FMSH_ASSERT(FI2cPs_isIrqMasked(dev, I2c_irq_tx_empty) ==
                                true);
                    callbackArg = instance->rxLength;
                }
                else
                {
                    // Otherwise, if there is still data to receive,
                    // do not call the user callback function.
                    userCallback = NULL;
                    callbackArg = NULL;
                }
                // Reset tx buffer and callback function pointers
                instance->txBuffer = NULL;
                instance->txCallback = NULL;
                break;
            default:
                // we shouldn't get a tx_empty interrupt in any
                // other driver state.
                FMSH_ASSERT(false);
                break;
            }
        }
        else
        {
            switch (instance->state)
            {
            case I2c_state_master_tx:
            case I2c_state_slave_bulk_tx:
            case I2c_state_slave_bulk_tx_rx:
            case I2c_state_master_tx_slave_rx:
                // slave-transmitter or master-transmitter
                maxBytes = param->tx_buffer_depth - FI2cPs_getTxFifoLevel(dev);
                // buffer should already be word-aligned
                I2C_FIFO_WRITE(maxBytes);
                break;
            case I2c_state_master_rx:
                maxBytes = MIN(
                    (param->tx_buffer_depth - FI2cPs_getTxFifoLevel(dev)),
                    instance->txRemain);
                for (i = 0; i < maxBytes - 1; i++)
                {
                    I2C_OUTP(0x100, portmap->data_cmd);
                }
                if (maxBytes == instance->txRemain)
                {
                    I2C_OUTP(0x300, portmap->data_cmd);
                }
                else
                {
                    I2C_OUTP(0x100, portmap->data_cmd);
                }

                instance->txRemain -= maxBytes;
                break;
            case I2c_state_back2back:
                maxBytes = MIN(
                    (param->tx_buffer_depth - FI2cPs_getTxFifoLevel(dev)),
                    instance->txRemain);
                I2C_FIFO_WRITE16(maxBytes);
                instance->txRemain -= maxBytes;
                break;
            default:
                // we shouldn't get a tx_empty interrupt in any
                // other driver state.
                FMSH_ASSERT(false);
                break;
            }
            // If the tx buffer is empty, set the tx threshold to no
            // bytes in fifo.  This is to ensure the the tx callback
            // function, if any, is only called when the current
            // transfer has been completed by the FMSH_apb_i2c device.
            if (instance->txRemain == 0)
            {
                I2C_OUTP(0x0, portmap->tx_tl);
            }
        }
    }
    // start condition detected
    else if ((reg & I2c_irq_start_det) != 0)
    {
        userCallback = instance->listener;
        callbackArg = I2c_irq_start_det;
        clearIrqMask = I2c_irq_start_det;
    }
    // stop condition detected
    else if ((reg & I2c_irq_stop_det) != 0)
    {
        userCallback = instance->listener;
        callbackArg = I2c_irq_stop_det;
        clearIrqMask = I2c_irq_stop_det;
    }
    // i2c bus activity
    else if ((reg & I2c_irq_activity) != 0)
    {
        userCallback = instance->listener;
        callbackArg = I2c_irq_activity;
        clearIrqMask = I2c_irq_activity;
    }
    // i2c bus scl stuck at low
    else if ((reg & I2c_scl_stuck_at_low) != 0)
    {
        userCallback = instance->listener;
        callbackArg = I2c_scl_stuck_at_low;
        clearIrqMask = I2c_scl_stuck_at_low;
    }

    else
    {
        // If we've reached this point, either the enabling and
        // disabling of I2C interrupts is not being handled properly or
        // this function is being called unnecessarily.
        retval = false;
    }

    // call the user listener function, if it has been set
    if (userCallback != NULL)
    {
        userCallback(dev, callbackArg);
    }

    // If the driver is still in one of these states, the user listener
    // function has not correctly handled a device event/interrupt.
    FMSH_REQUIRE(instance->state != I2c_state_rx_req);
    FMSH_REQUIRE(instance->state != I2c_state_rd_req);
    FMSH_REQUIRE(instance->state != I2c_state_slave_tx_rx_req);
    FMSH_REQUIRE(instance->state != I2c_state_slave_rx_rd_req);
    FMSH_REQUIRE(instance->state != I2c_state_master_tx_gen_call);
    FMSH_REQUIRE(instance->state != I2c_state_error);

    // clear the serviced interrupt
    if (clearIrqMask != 0)
    {
        FI2cPs_clearIrq(dev, clearIrqMask);
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function identifies the current highest priority active
 *  interrupt, if any, and forwards it to a user-provided listener
 *  function for processing.  This allows a user absolute control over
 *  how each I2C interrupt is processed.
 *
 *  None of the other Interrupt API functions can be used with this
 *  interrupt handler.  This is because they are symbiotic with the
 *  FI2cPs_irqHandler() interrupt handler.  All Command and Status API
 *  functions, however, can be used within the user listener function.
 *  This is in contrast to FI2cPs_irqHandler(), where FI2cPs_read(),
 *  FI2cPs_write() and FI2cPs_issueRead() cannot be used within the user
 *  listener function.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  true        -- an interrupt was processed
 *  false       -- no interrupt was processed
 *
 * @note
 *  Accesses the following DW_apb_i2c register(s)/bit field(s):
 *   - ic_intr_stat/all bits
 *   - ic_clr_rx_under/clr_rx_under
 *   - ic_clr_rx_over/clr_rx_over
 *   - ic_clr_tx_over/clr_tx_over
 *   - ic_clr_rd_req/clr_rd_req
 *   - ic_clr_tx_abrt/clr_tx_abrt
 *   - ic_clr_rx_done/clr_rx_done
 *   - ic_clr_activity/clr_activity
 *   - ic_clr_stop_det/clr_stop_det
 *   - ic_clr_start_det/clr_start_det
 *   - ic_clr_gen_call/clr_gen_call
 *
 *  This function is part of the Interrupt API and should not be called
 *  when using the I2C in a poll-driven manner.
 * WARNINGS
 *  The user listener function is run in interrupt context and, as such,
 *  care must be taken to ensure that any data shared between it and
 *  normal code is adequately protected from corruption.  Depending on
 *  the target platform, spinlocks, mutexes or semaphores may be used to
 *  achieve this.
 * SEE ALSO
 *  FI2cPs_setListener()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_userIrqHandler (FI2cPs_T *dev)
{
    bool retval = false;
    uint32_t reg = 0U;
    int32_t callbackArg;
    u32 fifo_level, i;
    // u32 tmp=0;
    FMSH_callback userCallback;
    FIicPs_Irq_T clearIrqMask;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    // Assume an interrupt will be processed.  userCallback will be set
    // to NULL and retval to false if no active interrupt is found.
    retval = true;
    // userCallback = instance->listener;
    userCallback = NULL;
    callbackArg = 0;
    clearIrqMask = I2c_irq_none;

    // what caused the interrupt?
    reg = I2C_INP(portmap->intr_stat);
    // reg = I2C_INP(portmap->raw_intr_stat);
    //  if a tx transfer was aborted
    if ((reg & I2c_irq_tx_abrt) != 0)
    {
        callbackArg = I2c_irq_tx_abrt;
        clearIrqMask = I2c_irq_tx_abrt;

        fmsh_print_dbg("*** Irq: TX_ABRT...\n");
    }
    // rx fifo overflow
    else if ((reg & I2c_irq_rx_over) != 0)
    {
        callbackArg = I2c_irq_rx_over;
        clearIrqMask = I2c_irq_rx_over;

        fmsh_print_dbg("*** Irq: RX_OVER...\n");
    }
    // rx fifo underflow
    else if ((reg & I2c_irq_rx_under) != 0)
    {
        callbackArg = I2c_irq_rx_under;
        clearIrqMask = I2c_irq_rx_under;

        fmsh_print_dbg("*** Irq: RX_UNDER...\n");
    }
    // tx fifo overflow
    else if ((reg & I2c_irq_tx_over) != 0)
    {
        callbackArg = I2c_irq_tx_over;
        clearIrqMask = I2c_irq_tx_over;

        fmsh_print_dbg("*** Irq: TX_OVER...\n");
    }
    // a general call was detected
    else if ((reg & I2c_irq_gen_call) != 0)
    {
        callbackArg = I2c_irq_gen_call;
        clearIrqMask = I2c_irq_gen_call;

        fmsh_print_dbg("*** Irq: GEN_CALL...\n");
    }
    // rx fifo level at or above threshold
    else if ((reg & I2c_irq_rx_full) != 0)
    {
        callbackArg = I2c_irq_rx_full;

        // read fifo
        fifo_level = FI2cPs_getRxFifoLevel(dev);
        for (i = 0; i < fifo_level; i++)
        {
            // tmp=I2C_INP(portmap->data_cmd);
        }

        fmsh_print_dbg("*** Irq: RX_FULL...\n");
    }
    // read-request transfer completed (Tx FIFO may still contain data)
    else if ((reg & I2c_irq_rx_done) != 0)
    {
        callbackArg = I2c_irq_rx_done;
        clearIrqMask = I2c_irq_rx_done;

        fmsh_print_dbg("*** Irq: RX_DONE...\n");
    }
    // read request received
    else if ((reg & I2c_irq_rd_req) != 0)
    {
        callbackArg = I2c_irq_rd_req;
        clearIrqMask = I2c_irq_rd_req;
        //   FI2cPs_write(dev,0x33);

        // I2C_OUTP(0X00,portmap->data_cmd);
        fmsh_print_dbg("*** Irq: RD_REQ...\n");

        for (i = 0; i < 16; i++)
        {
            I2C_OUTP(i, portmap->data_cmd);
        }
    }
    // tx fifo level at or below threshold
    else if ((reg & I2c_irq_tx_empty) != 0)
    {
        callbackArg = I2c_irq_tx_empty;
        FI2cPs_maskIrq(dev, I2c_irq_tx_empty);  // mask tx_empty

        fmsh_print_dbg("*** Irq: TX_EMPTY...\n");
    }
    // start condition detected
    else if ((reg & I2c_irq_start_det) != 0)
    {
        callbackArg = I2c_irq_start_det;
        clearIrqMask = I2c_irq_start_det;

        fmsh_print_dbg("*** Irq: START_DET...\n");
    }
    // restart condition detected
    else if ((reg & I2c_irq_restart_det) != 0)
    {
        callbackArg = I2c_irq_restart_det;
        clearIrqMask = I2c_irq_restart_det;

        fmsh_print_dbg("*** Irq: RESTART_DET...\n");
    }
    // stop condition detected
    else if ((reg & I2c_irq_stop_det) != 0)
    {
        callbackArg = I2c_irq_stop_det;
        clearIrqMask = I2c_irq_stop_det;

        fmsh_print_dbg("*** Irq: STOP_DET...\n");
    }
    // i2c bus activity
    else if ((reg & I2c_irq_activity) != 0)
    {
        callbackArg = I2c_irq_activity;
        clearIrqMask = I2c_irq_activity;

        fmsh_print_dbg("*** Irq: ACTIVITY...\n");
    }
    else if ((reg & I2c_irq_mst_on_hold) != 0)
    {
        callbackArg = I2c_irq_mst_on_hold;
        clearIrqMask = I2c_irq_mst_on_hold;

        fmsh_print_dbg("*** Irq: MST_ON_HOLD...\n");
        FI2cPs_maskIrq(dev, I2c_irq_mst_on_hold);
    }
    else if ((reg & I2c_scl_stuck_at_low) != 0)
    {
        callbackArg = I2c_scl_stuck_at_low;
        clearIrqMask = I2c_scl_stuck_at_low;

        fmsh_print_dbg("*** Irq: SCL_STUCK_AT_LOW...\n");
    }
    else
    {
        // no active interrupt was found
        retval = false;
        userCallback = NULL;
    }

    // call the user listener function, if there was an active interrupt
    if (userCallback != NULL)
    {
        userCallback(dev, callbackArg);
    }

    // clear any serviced interrupt
    if (clearIrqMask != 0)
    {
        FI2cPs_clearIrq(dev, clearIrqMask);
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  send stop after one byte
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
void FI2cPs_issueSTOP (FI2cPs_T *dev, uint8_t character)
{
    FIicPs_PortMap_T *portmap = NULL;

    uint16_t wSend;

    wSend = (0x200) | (uint16_t)character;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    I2C_OUTP(wSend, portmap->data_cmd);
}

/*****************************************************************************
 *
 * @description
 *  set gen call mode
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
void FI2cPs_setACK_GenCall (FI2cPs_T *dev)
{
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    I2C_OUTP(0x01, portmap->ack_gen_call);
}

/*****************************************************************************
 *
 * @description
 *  send restart
 *
 * @param
 *  dev is i2c device handle.
 *  character is send data.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
void FI2cPs_issueRESTART (FI2cPs_T *dev, uint8_t character)
{
    FIicPs_PortMap_T *portmap = NULL;

    uint16_t wSend;

    wSend = (0x400) | (uint16_t)character;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    I2C_OUTP(wSend, portmap->data_cmd);
}

/*****************************************************************************
 *
 * @description
 *  clear tx abort
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
uint32_t FI2cPs_clearTxAbrt (FI2cPs_T *dev)
{
    FIicPs_PortMap_T *portmap = NULL;
    uint32_t reg = 0U;

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    /* Read IC_CLR_TX_ABRT to clear TX_ABRT*/
    reg = I2C_INP(portmap->clr_tx_abrt);

    return reg;
}

/*****************************************************************************
 *
 * @description
 *  set scl timeout
 *
 * @param
 *  dev is i2c device handle.
 *  timeout is set value.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
int FI2cPs_setSCLTimeout (FI2cPs_T *dev, u32 timeout)
{
    int retval = 0;
    FIicPs_PortMap_T *portmap = NULL;

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        I2C_OUTP(timeout, portmap->scl_low_timeout);
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  set sda timeout
 *
 * @param
 *  dev is i2c device handle.
 *  timeout is set value.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
int FI2cPs_setSDATimeout (FI2cPs_T *dev, u32 timeout)
{
    int retval = 0;
    FIicPs_PortMap_T *portmap = NULL;

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        I2C_OUTP(timeout, portmap->sda_low_timeout);
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}
/*****************************************************************************
 *
 * @description
 *  set sda timeout
 *
 * @param
 *  dev is i2c device handle.
 *  timeout is set value.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
int FI2cPs_enableBusClear (FI2cPs_T *dev)
{
    int retval = 0;
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    if (FI2cPs_isEnabled(dev) == false)
    {
        retval = 0;
        reg = I2C_INP(portmap->con);
        // avoid bus write if possible
        if (FMSH_BIT_GET(reg, I2C_CON_BUS_CLEAR) != 0x1)
        {
            FMSH_BIT_SET(reg, I2C_CON_BUS_CLEAR, 0x1);
            I2C_OUTP(reg, portmap->con);
        }
    }
    else
    {
        retval = -FMSH_EPERM;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  abort master tranfer
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
void FI2cPs_masterAbort (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = 0;
    FMSH_BIT_SET(reg, I2C_ENABLE_ABORT, 0x1);
    I2C_OUTP(reg, portmap->enable);
}

/*****************************************************************************
 *
 * @description
 *  enable sda recovery when sda is stuck at low
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
void FI2cPs_sdaStuckRecoveryEnable (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->enable);
    reg |= (1 << 3);
    I2C_OUTP(reg, portmap->enable);
}

/*****************************************************************************
 *
 * @description
 *  enable sda recovery when sda is stuck at low
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
bool FI2cPs_isSdaStuckNotRecovery (FI2cPs_T *dev)
{
    uint32_t reg = 0U;
    bool retval = false;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    reg = I2C_INP(portmap->status);
    if ((reg & (1 << 11)) != 0)
    {
        retval = true;
    }
    else
    {
        retval = false;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  reset rx instance
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
void FI2cPs_resetRxInstance (FI2cPs_T *dev)
{
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    instance = (FIicPs_Instance_T *)dev->instance;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    I2C_ENTER_CRITICAL_SECTION();

    instance->state = I2c_state_idle;
    instance->intr_mask_save = I2c_irq_rd_req;

    I2C_EXIT_CRITICAL_SECTION();
}

/*****************************************************************************
 *
 * @description
 *  get raw intr reg value
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
uint32_t FI2cPs_getRawIntrStat (FI2cPs_T *dev)
{
    uint32_t retval;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;

    retval = I2C_INP(portmap->raw_intr_stat);

    return retval;
}

/*****************************************************************************
 *
 * @description
 *  setup clock
 *
 * @param
 *  dev is i2c device handle.
 *  ic_clk is freq of i2c controller.
 *
 * @return
 *
 * @note
 *
 ****************************************************************************/
void FI2cPs_ClockSetup (FI2cPs_T *dev, float ic_clk)
{
    uint16_t ss_scl_high, ss_scl_low;
    uint16_t fs_scl_high, fs_scl_low;
    uint16_t hs_scl_high, hs_scl_low;

    // ic_clk is the clock speed (in MHz) that is being supplied to the
    // DW_apb_i2c device.  The correct clock count values are determined
    // by using this inconjunction with the minimum high and low signal
    // hold times as per the I2C bus specification.
    ss_scl_high = ((uint16_t)(((SS_MIN_SCL_HIGH * ic_clk) / 1000) + 1));
    ss_scl_low = ((uint16_t)(((SS_MIN_SCL_LOW * ic_clk) / 1000) + 1));
    fs_scl_high = ((uint16_t)(((FS_MIN_SCL_HIGH * ic_clk) / 1000) + 1));
    fs_scl_low = ((uint16_t)(((FS_MIN_SCL_LOW * ic_clk) / 1000) + 1));
    hs_scl_high = ((uint16_t)(((HS_MIN_SCL_HIGH_100PF * ic_clk) / 1000) + 1));
    hs_scl_low = ((uint16_t)(((HS_MIN_SCL_LOW_100PF * ic_clk) / 1000) + 1));

    (void)FI2cPs_setSclCount(dev, I2c_speed_standard, I2c_scl_high, ss_scl_high);
    (void)FI2cPs_setSclCount(dev, I2c_speed_standard, I2c_scl_low, ss_scl_low);
    (void)FI2cPs_setSclCount(dev, I2c_speed_fast, I2c_scl_high, fs_scl_high);
    (void)FI2cPs_setSclCount(dev, I2c_speed_fast, I2c_scl_low, fs_scl_low);
    (void)FI2cPs_setSclCount(dev, I2c_speed_high, I2c_scl_high, hs_scl_high);
    (void)FI2cPs_setSclCount(dev, I2c_speed_high, I2c_scl_low, hs_scl_low);
}

/**********************************************************************/
/*                    PRIVATE FUNCTIONS                               */
/**********************************************************************/

/***
 * The following functions are all private and as such are not part of
 * the driver's public API.
 ***/

/*****************************************************************************
 *
 * @description
 *   This functions virtually flushes any data in the hold variable to
 *  the buffer (both in the i2c_Instance_T structure).  The 'hold'
 *  variable normally stores up to four data bytes before they are
 *  written to memory (i.e. the user buffer) to optimize bus performace.
 *  Flushing the
 *  (instance->) hold variable only makes sense when the i2c is in
 *  either master-receiver or slave-receiver mode.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  0           if successful
 *  -EPERM      if the i2c is not in a receive mode (master-rx/slave-rx)
 *
 * @note
 * This function comprises part of the interrupt-driven interface and
 *  normally should never need to be called directly.  The
 *  FMSH_i2c_terminate function always calls FMSH_i2c_flushRxHold before
 *  terminating a transfer.
 * SEE ALSO
 *  FMSH_i2c_terminate()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_flushRxHold (FI2cPs_T *dev)
{
    int i, retval;
    uint8_t *tmp;
    uint32_t c, mask;
    uint32_t *buf;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    instance = (FIicPs_Instance_T *)dev->instance;

    FMSH_REQUIRE(instance->rxBuffer != NULL);

    // sanity check .. idx should never be greater than four
    FMSH_ASSERT(instance->rxIdx <= 4);
    retval = 0;
    if (instance->rxIdx != 4)
    {
        // need to handle the case where there is less
        // than four bytes remaining in the rx buffer
        if (instance->rxRemain >= 4)
        {
            buf = (uint32_t *)instance->rxBuffer;
            mask = ((uint32_t)(0xffffffff) >> (8 * instance->rxIdx));
            c = mask & (instance->rxHold >> (8 * instance->rxIdx));
            *buf = (*buf & ~mask) | c;
        }
        else
        {
            // tmp = next free location in rx buffer
            tmp = (uint8_t *)instance->rxBuffer;
            // shift hold so that the least
            // significant byte contains valid data
            c = instance->rxHold >> (8 * instance->rxIdx);
            // write out valid character to rx buffer
            for (i = (4 - instance->rxIdx); i > 0; i--)
            {
                *tmp++ = (uint8_t)(c & 0xff);
                c >>= 8;
            }
        }  // instance->rxRemain <= 4
    }  // instance->rxIdx != 4

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function attempts to automatically discover the hardware
 *  component parameters, if this supported by the i2c in question.
 *  This is usually controlled by the ADD_ENCODED_PARAMS coreConsultant
 *  parameter.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  0           if successful
 *  -ENOSYS     function not supported
 *
 * @note
 * Accesses the following FMSH_apb_i2c register/bitfield(s):
 *   - ic_comp_type
 *   - ic_comp_version
 *   - ic_comp_param_1/all bits
 * This function does not allocate any memory.  An instance of
 *  FIicPs_Param_T must already be allocated and properly referenced from
 *  the relevant compParam FI2cPs_T  structure member.
 * SEE ALSO
 *  FMSH_i2c_init()
 * SOURCE
 *
 ****************************************************************************/
int FI2cPs_autoCompParams (FI2cPs_T *dev)
{
    int retval = 0;
    FIicPs_Param_T *param = NULL;
    FIicPs_PortMap_T *portmap = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    param = (FIicPs_Param_T *)dev->comp_param;
    portmap = (FIicPs_PortMap_T *)dev->base_address;

    dev->comp_version = I2C_INP(portmap->comp_version);

    // only version 1.03 and greater support identification registers
    if ((I2C_INP(portmap->comp_type) == FMSH_APB_I2C) &&
        (FMSH_BIT_GET(I2C_INP(portmap->comp_param_1),
                      I2C_PARAM_ADD_ENCODED_PARAMS) == true))
    {
        retval = 0;
        param->hc_count_values = FMSH_BIT_GET(I2C_INP(portmap->comp_param_1),
                                              I2C_PARAM_HC_COUNT_VALUES);
        param->has_dma = FMSH_BIT_GET(I2C_INP(portmap->comp_param_1),
                                      I2C_PARAM_HAS_DMA);
        param->max_speed_mode = (FIicPs_SpeedMode_T)FMSH_BIT_GET(
            I2C_INP(portmap->comp_param_1), I2C_PARAM_MAX_SPEED_MODE);
        param->rx_buffer_depth = FMSH_BIT_GET(I2C_INP(portmap->comp_param_1),
                                              I2C_PARAM_RX_BUFFER_DEPTH);
        param->rx_buffer_depth++;
        param->tx_buffer_depth = FMSH_BIT_GET(I2C_INP(portmap->comp_param_1),
                                              I2C_PARAM_TX_BUFFER_DEPTH);
        param->tx_buffer_depth++;
    }
    else
    {
        retval = -FMSH_ENOSYS;
    }

    return retval;
}

/*****************************************************************************
 *
 * @description
 *   This function resets/zeros all variables found in the
 *  FIicPs_Instance_T structure, except for FMSH_i2c_statistics.
 *
 * @param
 *  dev is i2c device handle.
 *
 * @return
 *  none
 *
 * @note
 * SEE ALSO
 *  FMSH_i2c_init(), FMSH_i2c_resetStatistics()
 * SOURCE
 *
 ****************************************************************************/
void FI2cPs_resetInstance (FI2cPs_T *dev)
{
    FIicPs_PortMap_T *portmap = NULL;
    FIicPs_Instance_T *instance = NULL;

    I2C_COMMON_REQUIREMENTS(dev);

    portmap = (FIicPs_PortMap_T *)dev->base_address;
    instance = (FIicPs_Instance_T *)dev->instance;

    instance->state = I2c_state_idle;
    instance->intr_mask_save = I2C_INP(portmap->intr_mask);
    instance->txThreshold = FI2cPs_getTxThreshold(dev);
    instance->rxThreshold = FI2cPs_getRxThreshold(dev);
    instance->listener = NULL;
    instance->txCallback = NULL;
    instance->rxCallback = NULL;
    instance->b2bBuffer = NULL;
    instance->txBuffer = NULL;
    instance->txHold = 0;
    instance->txIdx = 0;
    instance->txLength = 0;
    instance->txRemain = 0;
    instance->rxBuffer = NULL;
    instance->rxHold = 0;
    instance->rxIdx = 0;
    instance->rxLength = 0;
    instance->rxRemain = 0;
    instance->rxAlign = false;
}
