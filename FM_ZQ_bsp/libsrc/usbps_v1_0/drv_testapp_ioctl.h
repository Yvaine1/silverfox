/******************************************************************************
 *
 * Copyright (C) 2014-2021 Cadence Design Systems, Inc.
 * All rights reserved worldwide
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 *
 ******************************************************************************
 * drv_testapp_ioctl.h
 *
 *
 *
 *****************************************************************************/

#ifndef DRV_TESTAPP_IOCTL_H
#define DRV_TESTAPP_IOCTL_H

#ifdef __cplusplus
extern "C"
{
#endif
// #include <sys/ioctl.h>

#define CDNS_IOCTL_TYPE   0x23
#define CDNS_IOCTL_RDL    0x1
#define CDNS_IOCTL_WRL    0x2
#define CDNS_IOCTL_RDQ    0x3
#define CDNS_IOCTL_WRQ    0x4

#define CDNS_IOCTL_VTOPHY 0x13
#define CDNS_IOCTL_CFLUSH 0x14
#define CDNS_IOCTL_WMB    0x15
#define CDNS_IOCTL_RMB    0x16
#define CDNS_IOCTL_MB     0x17

typedef struct {
    unsigned long offset;
    unsigned long data;
} ioctl_rdwr_params_t;

typedef struct {
    unsigned long v_addr;
    unsigned long phy_addr;
} ioctl_vtophy_params_t;

typedef struct {
    unsigned long v_addr;
    unsigned long size;
} ioctl_cflush_params_t;

#define CDNS_IOCTL_CMD_RDL \
    _IOR(CDNS_IOCTL_TYPE, CDNS_IOCTL_RDL, ioctl_rdwr_params_t)
#define CDNS_IOCTL_CMD_WRL \
    _IOW(CDNS_IOCTL_TYPE, CDNS_IOCTL_WRL, ioctl_rdwr_params_t)
#define CDNS_IOCTL_CMD_RDQ \
    _IOR(CDNS_IOCTL_TYPE, CDNS_IOCTL_RDQ, ioctl_rdwr_params_t)
#define CDNS_IOCTL_CMD_WRQ \
    _IOW(CDNS_IOCTL_TYPE, CDNS_IOCTL_WRQ, ioctl_rdwr_params_t)

#define CDNS_IOCTL_CMD_VTOPHY \
    _IOR(CDNS_IOCTL_TYPE, CDNS_IOCTL_VTOPHY, ioctl_vtophy_params_t)
#define CDNS_IOCTL_CMD_CACHE_FLUSH \
    _IOW(CDNS_IOCTL_TYPE, CDNS_IOCTL_CFLUSH, ioctl_cflush_params_t)
#define CDNS_IOCTL_CMD_WMB _IO(CDNS_IOCTL_TYPE, CDNS_IOCTL_WMB)
#define CDNS_IOCTL_CMD_RMB _IO(CDNS_IOCTL_TYPE, CDNS_IOCTL_RMB)
#define CDNS_IOCTL_CMD_MB  _IO(CDNS_IOCTL_TYPE, CDNS_IOCTL_MB)

#ifdef __cplusplus
}
#endif

#endif /* DRV_TESTAPP_IOCTL_H */
