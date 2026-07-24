/***************************** Include Files *********************************/
#include <stdlib.h>

#include "fmsh_can_lib.h"
#include "fmsh_common.h"
#include "fmsh_psu_parameters.h"

/************************** Constant Definitions *****************************/
/* CAN Bittiming constants */
/*Bittiming setting from IPMS DS. */
/* CAN Bittiming constants */
/*CANFD Nominal Bit Rate*/
static const struct can_bittiming_const FCanFdPs_bittiming_const = {
    .tseg1_min = 2,
    .tseg1_max = 65,
    .tseg2_min = 1,
    .tseg2_max = 8,
    .sjw_max = 16,
    .brp_min = 1,
    .brp_max = 512,
    .brp_inc = 1,
};
/*CANFD Data Bit Rate*/
static const struct can_bittiming_const FCanFdPs_data_bittiming_const = {
    .tseg1_min = 2,
    .tseg1_max = 65,
    .tseg2_min = 1,
    .tseg2_max = 8,
    .sjw_max = 8,
    .brp_min = 1,
    .brp_max = 512,
    .brp_inc = 1,
};
/* CAN DLC to real data length conversion helpers */

static const u8 dlc2len[] = {0, 1, 2, 3, 4, 5, 6, 7,
			     8, 12, 16, 20, 24, 32, 48, 64};

/*for canfd encode len to dlc*/ 
static const u8 len2dlc[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,		/* 0 - 8 */
			     9, 9, 9, 9,			/* 9 - 12 */
			     10, 10, 10, 10,			/* 13 - 16 */
			     11, 11, 11, 11,			/* 17 - 20 */
			     12, 12, 12, 12,			/* 21 - 24 */
			     13, 13, 13, 13, 13, 13, 13, 13,	/* 25 - 32 */
			     14, 14, 14, 14, 14, 14, 14, 14,	/* 33 - 40 */
			     14, 14, 14, 14, 14, 14, 14, 14,	/* 41 - 48 */
			     15, 15, 15, 15, 15, 15, 15, 15,	/* 49 - 56 */
			     15, 15, 15, 15, 15, 15, 15, 15};	/* 57 - 64 */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define CAN_CALC_SYNC_SEG          1
#define UINT_MAX_CAN               (~0U)
#define clamp(a, min_val, max_val) MIN(MAX((a), (min_val)), (max_val))

/************************** Function Prototypes ******************************/

/*****************************************************************************
 *
 * @description    Initialize can parameters.
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    id is id code odf can 0 or 1.
 * @param    addr is the base address of can device.
 * @param    clk is the operate clock of can.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_init (FCanPs_T *dev, FCanPs_Config *cfg)
{
    u8 retval;

    retval = 0;

    dev->id = cfg->DeviceId;
    dev->CanFd_btc = FCanFdPs_bittiming_const;
    dev->CanFd_data_btc = FCanFdPs_data_bittiming_const;
    dev->ipms_tx_mode = cfg->TxMode;
    dev->base_address = (void *)(cfg->BaseAddress);
    dev->input_clock = cfg->InputClockHz;
    // set default canfd sample point
    dev->bt.sample_point = cfg->sample_point;
    dev->d_bt.sample_point = cfg->d_sample_point;
    return retval;
}
static u32 do_div (u32 n, u32 base)
{
    u32 __base = (base);
    u32 __rem;
    __rem = ((uint64_t)(n)) % __base;
    (n) = ((uint64_t)(n)) / __base;

    return __rem;
}
/*****************************************************************************
 *
 * @description
 * This function set can baud rate.
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    baud is canfd baud rate value.
 * @param    d_baud is canfd data baud rate value.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setBaudRate (FCanPs_T *dev, u32 baud, u32 d_baud)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    struct can_bittiming *bt = &dev->bt;
    struct can_bittiming *dbt = &dev->d_bt;
    struct can_bittiming_const *CanFd_btc = &dev->CanFd_btc;
    struct can_bittiming_const *CanFd_dat_btc = &dev->CanFd_data_btc;

    portmap = (FCanPs_Portmap_T *)dev->base_address;

    bt->bitrate = baud;
    dbt->bitrate = d_baud;

    can_calc_bittiming(dev, bt, CanFd_btc);
    can_calc_bittiming(dev, dbt, CanFd_dat_btc);

    FCanPs_setResetMode(dev, CAN_set); /*enter reset mode*/
    reg = CAN_INP(portmap->reg_grp1);
    if (!FMSH_BIT_GET(reg, CAN_CFG_STAT_RESET))
    {
        fmsh_print("Not in reset mode, cannot set bit timing\n");
        return 1;
    }
    mcan_set_bittiming(dev);
    /*reset off*/
    FCanPs_setResetMode(dev, CAN_clear);
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set bit timing
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
static int mcan_set_bittiming (FCanPs_T *dev)
{
    struct can_bittiming *bt = &dev->bt;
    struct can_bittiming *dbt = &dev->d_bt;
    int reg = 0;
    u32 tdc_value,reset_test, bittiming_temp, data_bittiming_temp;
    FCanPs_Portmap_T *portmap;

    portmap = (FCanPs_Portmap_T *)dev->base_address;

    // check the reset mode
    reg = CAN_INP(portmap->reg_grp1);
    reset_test = FMSH_BIT_GET(reg, CAN_CFG_STAT_RESET);
    if (!reset_test)
    {
        fmsh_print("Not in reset mode, cannot set bit timing\n");
        return -FMSH_EPERM;
    }
    bittiming_temp = ((bt->prop_seg + bt->phase_seg1 + 1 - 2)
                      << bfoCAN_S_Seg_1) |
                     ((bt->phase_seg2 - 1) << bfoCAN_S_Seg_2) |
                     ((bt->sjw - 1) << bfoCAN_S_SJW) |
                     ((bt->brp - 1) << bfoCAN_S_PRESC);
    /*The input bittime setting is incorrect, should be correct*/
    if ((((int)(bt->phase_seg1 + bt->prop_seg + 1) - 2) < 0) ||
        (((int)(bt->phase_seg2) - 1) < 0) || (((int)(bt->sjw) - 1) < 0) ||
        (((int)(bt->brp) - 1) < 0))
    {
        fmsh_print("slow bittime configuration is incorrect\n");
        return -FMSH_EPERM;
    }
    // set the s_bit_rate
    // fmsh_print( "bittiming_temp value is %08x\n",bittiming_temp);
    reg = CAN_INP(portmap->reg_grp3);
    if (reg != bittiming_temp)
    {
        CAN_OUTP(bittiming_temp, portmap->reg_grp3);
    }
    data_bittiming_temp = ((dbt->prop_seg + dbt->phase_seg1 + 1 - 2)
                           << bfoCAN_F_Seg_1) |
                          ((dbt->phase_seg2 - 1) << bfoCAN_F_Seg_2) |
                          ((dbt->sjw - 1) << bfoCAN_F_SJW) |
                          ((dbt->brp - 1) << bfoCAN_F_PRESC);
    // set the f_bit_rate
    // fmsh_print( "test phase_seg1 is %d\n",dbt->phase_seg1);
    if ((((int)(dbt->phase_seg1 + dbt->prop_seg + 1) - 2) < 0) ||
        (((int)(dbt->phase_seg2) - 1) < 0) || (((int)(dbt->sjw) - 1) < 0) ||
        (((int)(dbt->brp) - 1) < 0))
    {
        fmsh_print("fast bittime configuration is incorrect\n");
        return -FMSH_EPERM;
    }
    CAN_OUTP(data_bittiming_temp, portmap->reg_grp4);
#if 1
    //when dbt->brp<=2, it has to enable TDC otherwise disable TDC.
    if(dbt->brp <= 2)
    {
        tdc_value = (1<<bfoCAN_TDC_TDCEN)|((dbt->prop_seg+dbt->phase_seg1+1)<<bfoCAN_TDC_SSPOFF);
    }
    else
    {
        tdc_value = (0<<bfoCAN_TDC_TDCEN);
    }
    CAN_OUTP(tdc_value, portmap->reg_grp5);
#endif   
    FCanPs_setResetMode(dev, CAN_clear); /*exit reset mode*/

    /*print configured slow and fast bit rate*/
    // fmsh_print( "Slow bit rate configuration:
    // %08x\n",CAN_INP(portmap->reg_grp3)); fmsh_print( "Fast bit rate
    // configuration: %08x\n",CAN_INP(portmap->reg_grp4));
    return 0;
}
/*
 * Bit-timing calculation derived from:
 *
 * Code based on LinCAN sources and H8S2638 project
 * Copyright 2004-2006 Pavel Pisa - DCE FELK CVUT cz
 * Copyright 2005      Stanislav Marek
 * email: pisa@cmp.felk.cvut.cz
 *
 * Calculates proper bit-timing parameters for a specified bit-rate
 * and sample-point, which can then be used to set the bit-timing
 * registers of the CAN controller. You can find more information
 * in the header file linux/can/netlink.h.
 */
static int can_update_sample_point (const struct can_bittiming_const *btc,
                                    u32 sample_point_nominal, u32 tseg,
                                    u32 *tseg1_ptr, u32 *tseg2_ptr,
                                    u32 *sample_point_error_ptr)
{
    u32 sample_point_error, best_sample_point_error = UINT_MAX_CAN;
    u32 sample_point, best_sample_point = 0;
    u32 tseg1, tseg2;
    int i;

    for (i = 0; i <= 1; i++)
    {
        tseg2 = tseg + CAN_CALC_SYNC_SEG -
                (sample_point_nominal * (tseg + CAN_CALC_SYNC_SEG)) / 1000 - i;
        tseg2 = clamp(tseg2, btc->tseg2_min,
                      btc->tseg2_max);  // ensure tseg2 is in range
        tseg1 = tseg - tseg2;
        if (tseg1 > btc->tseg1_max)
        {
            tseg1 = btc->tseg1_max;
            tseg2 = tseg - tseg1;
        }

        sample_point = 1000 * (tseg + CAN_CALC_SYNC_SEG - tseg2) /
                       (tseg + CAN_CALC_SYNC_SEG);
        sample_point_error = abs(sample_point_nominal - sample_point);

        if ((sample_point <= sample_point_nominal) &&
            (sample_point_error < best_sample_point_error))
        {
            best_sample_point = sample_point;
            best_sample_point_error = sample_point_error;
            *tseg1_ptr = tseg1;
            *tseg2_ptr = tseg2;
        }
    }

    if (sample_point_error_ptr)
    {
        *sample_point_error_ptr = best_sample_point_error;
    }

    return best_sample_point;
}
/*
 * @description    calculate can bit timing.
 *
 * @para    dev is a can device handle.
 *
 * @return    return 0 if sucessful.
 *
 * @note    see can driver of linux -- dev.c/sja1000.c
 */
static int can_calc_bittiming (FCanPs_T *dev, struct can_bittiming *bt,
                               const struct can_bittiming_const *btc)
{
    u32 bitrate;              /* current bitrate */
    u32 bitrate_error;        /* difference between current and nominal value */
    u32 best_bitrate_error = UINT_MAX_CAN;
    u32 sample_point_error;   /* difference between current and nominal value */
    u32 best_sample_point_error = UINT_MAX_CAN;
    u32 sample_point_nominal; /* nominal sample point */
    u32 best_tseg = 0;        /* current best value for tseg */
    u32 best_brp = 0;         /* current best value for brp */
    u32 brp, tsegall, tseg, tseg1 = 0, tseg2 = 0;
    u64 v64;
    /* Use CiA recommended sample points */
    if (bt->sample_point)
    {
        sample_point_nominal = bt->sample_point;
    }
    else
    {
#if 0
		if (bt->bitrate > 800000)
			sample_point_nominal = 750;
		else if (bt->bitrate > 500000)
			sample_point_nominal = 800;
		else
			sample_point_nominal = 875;
#endif
        // due to the canfd test tool sample point is 80%
        sample_point_nominal = 800;
    }

    /* tseg even = round down, odd = round up */
    for (tseg = (btc->tseg1_max + btc->tseg2_max) * 2 + 1;
         tseg >= (btc->tseg1_min + btc->tseg2_min) * 2; tseg--)
    {
        tsegall = CAN_CALC_SYNC_SEG + tseg / 2;

        /* Compute all possible tseg choices (tseg=tseg1+tseg2) */
        brp = dev->input_clock / (tsegall * bt->bitrate) + tseg % 2;

        /* choose brp step which is possible in system */
        brp = (brp / btc->brp_inc) * btc->brp_inc;
        if ((brp < btc->brp_min) || (brp > btc->brp_max))
        {
            continue;
        }
        // caculate the brp

        bitrate = dev->input_clock / (brp * tsegall);
        // use brp to caculate bitrate
        bitrate_error = abs(bt->bitrate - bitrate);

        /* tseg brp biterror */
        if (bitrate_error > best_bitrate_error)
        {
            continue;
        }

        /* reset sample point error if we have a better bitrate */
        if (bitrate_error < best_bitrate_error)
        {
            best_sample_point_error = UINT_MAX_CAN;
        }

        can_update_sample_point(btc, sample_point_nominal, tseg / 2, &tseg1,
                                &tseg2, &sample_point_error);
        if (sample_point_error > best_sample_point_error)
        {
            continue;
        }

        best_sample_point_error = sample_point_error;
        best_bitrate_error = bitrate_error;
        best_tseg = tseg / 2;
        best_brp = brp;

        if( (bitrate_error == 0) && (sample_point_error == 0) )
        {
            break;
        }
    }

    /* real sample point */
    bt->sample_point = can_update_sample_point(btc, sample_point_nominal,
                                               best_tseg, &tseg1, &tseg2, NULL);

    v64 = (u64)best_brp * 1000 * 1000 * 1000;
    do_div(v64, dev->input_clock);
    bt->tq = (u32)v64;
    bt->prop_seg = tseg1 / 2;
    bt->phase_seg1 = tseg1 - bt->prop_seg;
    bt->phase_seg2 = tseg2;

    /* check for sjw user settings */
    if( (!bt->sjw) || (!btc->sjw_max) )
    {
        bt->sjw = 1;
    }
    else
    {
        /* bt->sjw is at least 1 -> sanitize upper bound to sjw_max */
        if (bt->sjw > btc->sjw_max)
        {
            bt->sjw = btc->sjw_max;
        }
        /* bt->sjw must not be higher than tseg2 */
        if (tseg2 < bt->sjw)
        {
            bt->sjw = tseg2;
        }
    }

    bt->brp = best_brp;

    /* real bitrate */
    bt->bitrate = dev->input_clock /
                  (bt->brp * (CAN_CALC_SYNC_SEG + tseg1 + tseg2));

    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set Reset Mode
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setResetMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    if (FMSH_BIT_GET(reg, CAN_CFG_STAT_RESET) != state)
    {
        FMSH_BIT_SET(reg, CAN_CFG_STAT_RESET, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set Almost Full Warning Limit
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    warning_limit is a pointer to the receive buffer almost full warning limit number.(1~16)
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setAlmostFull (FCanPs_T *dev, u8 warning_limit)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    FMSH_BIT_SET(reg, CAN_LIMIT_AFWL, warning_limit);
    CAN_OUTP(reg, portmap->reg_grp2);

    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set Programmable Error Warning Limit
 * Programmable Error Warning Limit = (EWL+1)*8. Set EWL=11->Error Warning=96
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setErrLimit(FCanPs_T *dev, u8 ewl)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *) dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    FMSH_BIT_SET(reg, CAN_LIMIT_EWL, ewl);
    CAN_OUTP(reg, portmap->reg_grp2); 
    
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set recive interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setReciveInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_RTIE_RIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_RTIE_RIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set recive buffer overrun interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setReciveBufferOverrunInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_RTIE_ROIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_RTIE_ROIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set recive buffer full interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setReceiveBufferFullInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_RTIE_RFIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_RTIE_RFIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set recive buffer almost full interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setReceiveBufferAlmostFullInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_RTIE_RAFIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_RTIE_RAFIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set transmission primary interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmissionPrimaryInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_RTIE_TPIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_RTIE_TPIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set transmission secondary interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmissionSecondaryInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_RTIE_TSIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_RTIE_TSIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set error interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setErrorInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_RTIE_EIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_RTIE_EIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set set Error Passive Interrupts 
 *
 * @param    dev is a pointer to the instance of can device.
 * 
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setErrorPassiveInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_ERRINT_EPIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_ERRINT_EPIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set set Bus Error Interrupts 
 *
 * @param    dev is a pointer to the instance of can device.
 * 
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setBusErrorInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_ERRINT_BEIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_ERRINT_BEIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set set Watch Trigger Interrupts 
 *
 * @param    dev is a pointer to the instance of can device.
 * 
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setWatchTriggerInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    if (FMSH_BIT_GET(reg, CAN_TTCFG_WTIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_TTCFG_WTIE, state);
        CAN_OUTP(reg, portmap->reg_grp7);
    }
    
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set set Time Trigger Interrupts 
 *
 * @param    dev is a pointer to the instance of can device.
 * 
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTimeTriggerInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    if (FMSH_BIT_GET(reg, CAN_TTCFG_TTIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_TTCFG_TTIE, state);
        CAN_OUTP(reg, portmap->reg_grp7);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set External Loop Back Test mode
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setExtTestMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    // set External Loop Back Test mode
    if (FMSH_BIT_GET(reg, CAN_CFG_STAT_LBME) != state)
    {
        FMSH_BIT_SET(reg, CAN_CFG_STAT_LBME, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }
    // set Self-ACKnowledge

    if (FMSH_BIT_GET(reg, CAN_RCTRL_SACK) != state)
    {
        FMSH_BIT_SET(reg, CAN_RCTRL_SACK, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Internal Loop Back mode
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setIntTestMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    // set Internal Loop Back Test mode
    if (FMSH_BIT_GET(reg, CAN_CFG_STAT_LBMI) != state)
    {
        FMSH_BIT_SET(reg, CAN_CFG_STAT_LBMI, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Transmission Primary Single Shot mode
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTpssMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    // set Transmission Primary Single Shot mode
    if (FMSH_BIT_GET(reg, CAN_CFG_STAT_TPSS) != state)
    {
        FMSH_BIT_SET(reg, CAN_CFG_STAT_TPSS, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Transmission Secondary Single Shot mode
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTsssMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    // set Transmission Primary Single Shot mode
    if (FMSH_BIT_GET(reg, CAN_CFG_STAT_TSSS) != state)
    {
        FMSH_BIT_SET(reg, CAN_CFG_STAT_TSSS, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Listen Only Mode
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setListenOnlyMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    // set Listen Only Mode
    if (FMSH_BIT_GET(reg, CAN_TCMD_LOM) != state)
    {
        FMSH_BIT_SET(reg, CAN_TCMD_LOM, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}
/****************************************************************************/
/**
 *
 * This function is used to select g_CAN0/1 Transmit Buffer, then send back to
 *PC.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0, PTB Transmit Buffer Select
 *           CAN_set = 1    STB Transmit Buffer Select
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u8 FCanPs_setTransmitBufferSelect (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    // set
    if (FMSH_BIT_GET(reg, CAN_TCMD_TBSEL) != state)
    {
        FMSH_BIT_SET(reg, CAN_TCMD_TBSEL, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}
/****************************************************************************/
/**
 *
 * This function is used to mark STB slot filled, select next slot. 
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0, no action
 *           CAN_set = 1,   STB slot filled, select next slot       
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u8 FCanPs_setTransmitBufferNext (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    // set
    if (FMSH_BIT_GET(reg, CAN_TCTRL_TSNEXT) != state)
    {
        FMSH_BIT_SET(reg, CAN_TCTRL_TSNEXT, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }

    return 0;
}

/****************************************************************************/
/**
 *
 * This function is used to set g_CAN0/1 test mode, then send back to PC.
 *
 * @param    dev is a pointer to the instance of g_CAN0/1.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u8 fmsh_can_test_mode_select (FCanPs_T *dev, enum can_test_mode mode)
{

    switch (mode)
    {
    case ExtTestMode:
        FCanPs_setExtTestMode(dev, CAN_set);
        FCanPs_setIntTestMode(dev, CAN_clear);
        break;

    case IntTestMode:
        FCanPs_setExtTestMode(dev, CAN_clear);
        FCanPs_setIntTestMode(dev, CAN_set);
        break;

    case TPSSMode:
        FCanPs_setTpssMode(dev, CAN_set);
        FCanPs_setTsssMode(dev, CAN_clear);
        // use PTB
        FCanPs_setTransmitBufferSelect(dev, CAN_clear);
        break;

    case TSSSMode:
        FCanPs_setTpssMode(dev, CAN_clear);
        FCanPs_setTsssMode(dev, CAN_set);
        // use STB
        FCanPs_setTransmitBufferSelect(dev, CAN_set);
        break;

    case ListenOnlyMode:
        FCanPs_setListenOnlyMode(dev, CAN_set);
        break;

    default:
        break;
    }
    return 0;
}

/****************************************************************************/
/**
 *
 * This function is used to enable/disable acceptance filter, then send back to
 *PC.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0, disable acceptance filter
 *           CAN_set = 1    enable acceptance filter
 *
 * @param    dev is a pointer to the instance of g_CAN0/1.
 *
 * @param    order is the number of acceptance filter. 0~15 (0~2 due to
 *Synthesize parameter)
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ****************************************************************************/
u8 fmsh_can_enable_ACF (FCanPs_T *dev, enum FCanPs_state state, u32 order)
{
    u32 enable, reg = 0;
    FCanPs_Portmap_T *portmap;

    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp6);
    if (order < 8)
    {
        enable = FMSH_BIT_GET(reg, CAN_ACF_EN_0);
        if (state)
        {
            enable = enable | (1 << order);
        }
        else
        {
            enable = enable & (~(1 << order));
        }
        FMSH_BIT_SET(reg, CAN_ACF_EN_0, enable);
        CAN_OUTP(reg, portmap->reg_grp6);
    }
    else
    {
        enable = FMSH_BIT_GET(reg, CAN_ACF_EN_1);
        order = order - 8;
        if (state)
        {
            enable = enable | (1 << order);
        }
        else
        {
            enable = enable & (~(1 << order));
        }
        FMSH_BIT_SET(reg, CAN_ACF_EN_1, enable);
        CAN_OUTP(reg, portmap->reg_grp6);
    }

    return 0;
}

/*****************************************************************************
 *
 * @description
 * NOTICE: before set ACF mode, SELMASK should be 1 and ACFADR need to be
 *setting This function set ACF mode.
 * @param    mode:
 *           se_acf_mode = 0, //acf accepts both standard and extended frames
 *           s_acf_mode = 1,  //acf accepts only standard frames
 *           e_acf_mode = 2   //acf accepts only extended frames
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 fmsh_ACF_mode_select (FCanPs_T *dev, enum can_acf_mode mode)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;

    portmap = (FCanPs_Portmap_T *)dev->base_address;
    FCanPs_setResetMode(dev, CAN_set);
    reg = CAN_INP(portmap->acf);
    switch (mode)
    {
    case se_acf_mode:
        reg = reg & (~(1 << 30));
        break;

    case s_acf_mode:
        reg = reg | (1 << 30);
        reg = reg & (~(1 << 29));
        break;

    case e_acf_mode:
        reg = reg | (1 << 30);
        reg = reg | (1 << 29);
        break;
    default:
        break;
    }
    CAN_OUTP(reg, portmap->acf);
    FCanPs_setResetMode(dev, CAN_clear);
    return 0;
}

/*****************************************************************************
 *
 * @description
 *
 * This function set ACODE value.
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    id is a accept id for ACF
 *
 * @param    order is the number of acceptance filter. 0~15 (0~2 due to
 *Synthesize parameter)
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 fmsh_ACODE_set (FCanPs_T *dev, u32 id, u32 order)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;

    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp6);
    FMSH_BIT_CLEAR(reg, CAN_ACFCTRL_SELMASK);
    FMSH_BIT_SET(reg, CAN_ACFCTRL_ACFADR, order);
    CAN_OUTP(reg, portmap->reg_grp6);
    FCanPs_setResetMode(dev, CAN_set);
    CAN_OUTP(id, portmap->acf);
    FCanPs_setResetMode(dev, CAN_clear);
    return 0;
}

/*****************************************************************************
 *
 * @description
 *
 * This function set AMASK value.
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    mask is a accept mask for ACF
 *
 * @param    order is the number of acceptance filter. 0~15 (0~2 due to
 *Synthesize parameter)
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 fmsh_AMASK_set (FCanPs_T *dev, u32 mask, u32 order)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;

    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp6);
    FMSH_BIT_SET(reg, CAN_ACFCTRL_SELMASK, 1);
    FMSH_BIT_SET(reg, CAN_ACFCTRL_ACFADR, order);
    CAN_OUTP(reg, portmap->reg_grp6);
    FCanPs_setResetMode(dev, CAN_set);
    CAN_OUTP(mask, portmap->acf);
    FCanPs_setResetMode(dev, CAN_clear);

    return 0;
}

/*****************************************************************************
 *
 * @description
 *
 * This function set Recive Filter.
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    mask is a accept mask for acceptance filters.
 *           the bits of mask = 1, acceptance filter check for these bits of receive identifier disabled.
 *           the bits of mask = 0, acceptance filter check for these bits of receive identifier enable.
 *
 * @param    order is the serial number of acceptance filter to be used. the range of order is 0~2
 *           order = 0, use acceptance filter 0 to filter recive frame.  
 *           order = 1, use acceptance filter 1 to filter recive frame.
 *           order = 2, use acceptance filter 2 to filter recive frame.   
 *
 * @param    mode:
 *           se_acf_mode = 0, //acceptance accepts both standard and extended frames
 *           s_acf_mode = 1,  //acceptance accepts only standard frames
 *           e_acf_mode = 2   //acceptance accepts only extended frames
 *
 * @param    id is a accept id for acceptance filters
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0, disable acceptance filter
 *           CAN_set = 1    enable acceptance filter
 * 
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setFilter(FCanPs_T *dev, u32 mask, u32 order, enum can_acf_mode mode, u32 id, enum FCanPs_state state)
{
    fmsh_AMASK_set(dev, mask, order);
    fmsh_ACF_mode_select(dev, mode);
    fmsh_ACODE_set(dev, id, order);
    fmsh_can_enable_ACF(dev, state, order);
    
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set ISO or non-ISO Mode
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setFD_ISOMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    FCanPs_setResetMode(dev, CAN_set);
    if (FMSH_BIT_GET(reg, CAN_TCTRL_FD_ISO) != state)
    {
        FMSH_BIT_SET(reg, CAN_TCTRL_FD_ISO, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }
    FCanPs_setResetMode(dev, CAN_clear);
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Receive buffer Overflow Mode
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_clear = 0, In case of a full RBUF when a new message is received,the oldest message will be overwritten.
 *           CAN_set = 1, The new message will not be stored.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setReceiveOverflowMode (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    if (FMSH_BIT_GET(reg, CAN_RCTRL_ROM) != state)
    {
        FMSH_BIT_SET(reg, CAN_RCTRL_ROM, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set TSA bit to abort send STB messages
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmitSecondaryAbort (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    if (FMSH_BIT_GET(reg, CAN_TCMD_TSA) != state)
    {
        FMSH_BIT_SET(reg, CAN_TCMD_TSA, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set TPA bit to abort send PTB messages
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmitPrimaryAbort (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    if (FMSH_BIT_GET(reg, CAN_TCMD_TPA) != state)
    {
        FMSH_BIT_SET(reg, CAN_TCMD_TPA, state);
        CAN_OUTP(reg, portmap->reg_grp1);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set ALIE bit to enable lost interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setArbitrationLostInterrupt (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp2);
    if (FMSH_BIT_GET(reg, CAN_ERRINT_ALIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_ERRINT_ALIE, state);
        CAN_OUTP(reg, portmap->reg_grp2);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set all interrupt flage bit to 0
 *
 * @param reg is the value of all interrupt flage
 *
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FCanPs_clearIntFlage (u32 reg)
{
    // for RTIF
    FMSH_BIT_CLEAR(reg, CAN_RTIF_RIF);
    FMSH_BIT_CLEAR(reg, CAN_RTIF_ROIF);
    FMSH_BIT_CLEAR(reg, CAN_RTIF_RFIF);
    FMSH_BIT_CLEAR(reg, CAN_RTIF_RAFIF);
    FMSH_BIT_CLEAR(reg, CAN_RTIF_TPIF);
    FMSH_BIT_CLEAR(reg, CAN_RTIF_TSIF);
    FMSH_BIT_CLEAR(reg, CAN_RTIF_EIF);
    FMSH_BIT_CLEAR(reg, CAN_RTIF_AIF);
    // for ERRINT
    FMSH_BIT_CLEAR(reg, CAN_ERRINT_EPIF);
    FMSH_BIT_CLEAR(reg, CAN_ERRINT_ALIF);
    FMSH_BIT_CLEAR(reg, CAN_ERRINT_BEIF);

    return reg;
}

/*****************************************************************************
 *
 * @description
 * This function set all TTCAN interrupt flage bit to 0
 *
 * @param reg is the value of all interrupt flage
 *
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u32 FCanPs_clearTTCANIntFlage (u32 reg)
{
    // for TTCFG
    FMSH_BIT_CLEAR(reg, CAN_TTCFG_TTIF);
    FMSH_BIT_CLEAR(reg, CAN_TTCFG_TEIF);
    FMSH_BIT_CLEAR(reg, CAN_TTCFG_WTIF);

    return reg;
}

/*****************************************************************************
 *
 * @description
 * This function set TBPTR address
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    address is pointer to a TB message slot
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmitBufferPointer (FCanPs_T *dev, u8 address)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    // due to STB_SLOTS parameter is 16
    if (address <= 16)
    {
        FMSH_BIT_SET(reg, CAN_TBSLOT_TBPTR, address);
    }
    else
    {
        fmsh_print(
            "TBPTR address is biger than STB_SLOTS parameter, nothing "
            "happened");
    }
    CAN_OUTP(reg, portmap->reg_grp7);
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set TBE bit to set TB slot to "Empty"
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmitBufferSlotEmpty (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    if (FMSH_BIT_GET(reg, CAN_TBSLOT_TBE) != state)
    {
        FMSH_BIT_SET(reg, CAN_TBSLOT_TBE, state);
        CAN_OUTP(reg, portmap->reg_grp7);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set TBE bit to set TB slot to "Filled"
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmitBufferSlotFilled (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    if (FMSH_BIT_GET(reg, CAN_TBSLOT_TBF) != state)
    {
        FMSH_BIT_SET(reg, CAN_TBSLOT_TBF, state);
        CAN_OUTP(reg, portmap->reg_grp7);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set TTPTR address
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    address is pointer to a TB message slot
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTransmitTriggerPointer (FCanPs_T *dev, u8 address)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp8);
    // due to STB_SLOTS parameter is 16
    if (address <= 16)
    {
        FMSH_BIT_SET(reg, CAN_TRIG_CFG_0_TTPTR, address);
    }
    else
    {
        fmsh_print(
            "TTPTR address is biger than STB_SLOTS parameter, nothing "
            "happened");
    }
    CAN_OUTP(reg, portmap->reg_grp8);
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set REF_ID for reference message
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    id is the id of reference message,notice that id(2:0) is not tested
 *and foce to be 0. these bits are used for up to 8 potential time masters.
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setREF_ID (FCanPs_T *dev, u32 id)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->ref_msg);
    FMSH_BIT_SET(reg, CAN_REF_MSG_REF_ID, id);
    CAN_OUTP(reg, portmap->ref_msg);
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set REF_IDE for reference message
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setREF_IDE (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->ref_msg);
    if (FMSH_BIT_GET(reg, CAN_REF_MSG_3_REF_IDE) != state)
    {
        FMSH_BIT_SET(reg, CAN_REF_MSG_3_REF_IDE, state);
        CAN_OUTP(reg, portmap->ref_msg);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Trigger Time for reference message
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    tt is the trigger time
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTriggerTime (FCanPs_T *dev, u16 tt)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp8);
    FMSH_BIT_SET(reg, CAN_TT_TRIG, tt);
    CAN_OUTP(reg, portmap->reg_grp8);
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set watch trigger interrupt
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setWTIE (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    if (FMSH_BIT_GET(reg, CAN_TTCFG_WTIE) != state)
    {
        FMSH_BIT_SET(reg, CAN_TTCFG_WTIE, state);
        CAN_OUTP(reg, portmap->reg_grp7);
    }
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set TTCAN Time Prescaler
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    presc is the PRESCaler
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTTCAN_TimePrescaler (FCanPs_T *dev, u8 presc)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    FMSH_BIT_SET(reg, CAN_TTCFG_T_PRESC, presc);
    CAN_OUTP(reg, portmap->reg_grp7);
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Time Trigger
 *
 * @param    dev is a pointer to the instance of can device.
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTimeTrigger(FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp7);
    if (FMSH_BIT_GET(reg, CAN_TTCFG_TTEN) != state)
    {
        FMSH_BIT_SET(reg, CAN_TTCFG_TTEN, state);
        CAN_OUTP(reg, portmap->reg_grp7);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set Trigger Type
 * @param    dev is a pointer to the instance of can device.
 * @param    presc is the PRESCaler
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTriggerType (FCanPs_T *dev, enum can_trigger_type ttype)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp8);
    FMSH_BIT_SET(reg, CAN_TRIG_CFG_1_TTYPE, ttype);
    CAN_OUTP(reg, portmap->reg_grp8);
    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set Time Stamping
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @param    state:
 *           CAN_err = -1,
 *           CAN_clear = 0,
 *           CAN_set = 1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_setTimeStamping (FCanPs_T *dev, enum FCanPs_state state)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp6);
    if (FMSH_BIT_GET(reg, CAN_TIMECFG_TIMEEN) != state)
    {
        FMSH_BIT_SET(reg, CAN_TIMECFG_TIMEEN, state);
        CAN_OUTP(reg, portmap->reg_grp6);
    }
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function get Kind Of Error
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   Kind Of Error
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_getKindOfError (FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp5);
    reg = FMSH_BIT_GET(reg, CAN_EALCAP_KOER);
    return reg;
}
/*****************************************************************************
 *
 * @description
 * This function get Receive Error Count
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   Receive Error Count
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_getReceiveErrorCount (FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp5);
    reg = FMSH_BIT_GET(reg, CAN_RECNT);
    return reg;
}
/*****************************************************************************
 *
 * @description
 * This function get Transmit Error Count
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   Transmit Error Count
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_getTransmitErrorCount (FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp5);
    reg = FMSH_BIT_GET(reg, CAN_TECNT);
    return reg;
}
/*****************************************************************************
 *
 * @description
 * This function get Bus Status
 *
 * @param    dev is a pointer to the instance of can device.
 *
 * @return   1 if bus off, otherwise 0
 *
 * @note     None.
 *
 ******************************************************************************/
u8 FCanPs_getBusStatus (FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *)dev->base_address;
    reg = CAN_INP(portmap->reg_grp1);
    reg = FMSH_BIT_GET(reg, CAN_CFG_STAT_BUSOFF);
    return reg;
}
/*****************************************************************************
*
* @description
* This function is used to get data length from can_dlc with sanitized can_dlc
*
* @param    can_dlc is an appropriate data length code.
*
* @return   the sanitized data length.
*
* @note     None.
*
****************************************************************************/
u8 can_dlc2len(u8 can_dlc)
{
    return dlc2len[can_dlc & 0x0F];
}
/****************************************************************************
 *
 * @description
 * this function helper macro to cast a given data length code (dlc)
 * to __u8 and ensure the dlc value to be max. 8 bytes.
 *
 * @param    can_dlc is a sanitized data length.
 *
 * @return   the sanitized data length small than CAN_MAX_DLC.
 *
 * @note     None.
 *
 *
***************************************************************************/
u8 get_can_dlc(u8 can_dlc)
{
    return can_dlc < CAN_MAX_DLC ? can_dlc : CAN_MAX_DLC;
}
/****************************************************************************
*
* @description
* This function is used to map the sanitized data length to an appropriate data length code
*
* @param    len is the sanitized data length.
* 
* @return   an appropriate data length code.
*
* @note     None.
*
****************************************************************************/
/* map the sanitized data length to an appropriate data length code */
u8 can_len2dlc(u8 len)
{
    if (len > 64)
    {
        return 0xF;
    }
    return len2dlc[len];
}
/****************************************************************************
*
* @description
* This function set TX mode
* This part could also be removed, when only one mode is used
*
* @param    dev is a pointer to the instance of g_CAN0/g_CAN1.
* 
* @param    tx_mode:full_mode, TTCAN Mode send Can Frame
*                   stb_fifo, STB Fifo Mode send Can Frame  
*                   stb_prio, STB Priority Mode send Can Frame
*                   ptb_mode, PTB Mode send Can Frame
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
void FCanPs_setXmitMode(FCanPs_T* dev, enum can_tx_mode tx_mode)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *) dev->base_address; 
    
    switch(tx_mode)
    {
        case full_mode:
            //set full mode
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_SET(reg, CAN_TCTRL_TTTBM, 1);
            CAN_OUTP(reg, portmap->reg_grp1);             
            //fmsh_print( "Full can mode\n");
            break;
        
        case stb_fifo:
            //close TTCAN mode
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_CLEAR(reg, CAN_TCTRL_TTTBM);
            CAN_OUTP(reg, portmap->reg_grp1); 
            //enable FIFO mode
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_CLEAR(reg, CAN_TCTRL_TSMODE);
            CAN_OUTP(reg, portmap->reg_grp1); 
            /*select TBSEL, frame writen in STB */
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_SET(reg, CAN_TCMD_TBSEL, 1);
            CAN_OUTP(reg, portmap->reg_grp1); 
            //fmsh_print( "FIFO mode\n");
            break;
            
        case stb_prio:
            //close TTCAN mode
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_CLEAR(reg, CAN_TCTRL_TTTBM);
            CAN_OUTP(reg, portmap->reg_grp1); 
            //enable Priority mode
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_SET(reg, CAN_TCTRL_TSMODE, 1);
            CAN_OUTP(reg, portmap->reg_grp1); 
            /*select TBSEL, frame writen in STB */
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_SET(reg, CAN_TCMD_TBSEL, 1);
            CAN_OUTP(reg, portmap->reg_grp1); 
            //fmsh_print( "Priority mode\n");	
            break;  
            
        case ptb_mode:
            /*select TBSEL, frame writen in PTB */
            reg = CAN_INP(portmap->reg_grp1);
            FMSH_BIT_CLEAR(reg, CAN_TCMD_TBSEL);
            CAN_OUTP(reg, portmap->reg_grp1); 
            //fmsh_print( "PTB mode\n");		
            
            break; 
        default:
	    break;         
    }
}
/*****************************************************************************
*
* @description
* This function returns the status of Transmission Complete Status.
*
* @param    dev is a pointer to the instance of can device.
*
* @return   status: 0/1
*
* @note     None.
*
******************************************************************************/
u32 FCanPs_getTransmissionCompleteStatus(FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) dev->base_address; 
    reg = CAN_INP(portmap->reg_grp1);
    /*use TACTIVE to juge the transmission is done*/
    /*TPE has the same function? need to test*/
    reg = FMSH_BIT_GET(reg, CAN_CFG_STAT_TACTIVE);

    return reg;
}
/****************************************************************************
*
* @description
* This function sends can frame message.
*
* @param    dev is a pointer to the instance of g_CAN0/g_CAN1.
*
* @param    can_id:bit 0-28, CAN identifier (11/29 bit)
*                  bit 29, error message frame flag (0 = data frame, 1 = error message)
*                  bit 30, remote transmission request flag (1 = rtr frame)
*                  bit 31, frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
*
* @param    tbuf is buffer of message data                        
*
* @param    len is length of message
*
* @param    fd_mode:can2, can frame
*                   canfd, canfd frame
*
* @param    cia_en:1 if add cia603 time stamp to can frame, otherwise 0
*                  
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 FCanPs_FrameTransmit(FCanPs_T* dev, u32 can_id, u32 *tbuf, u8 len, enum can_mode fd_mode, enum FCanPs_state cia_en)
{
    //u32 reg;
    u32 i;
    u32 *p;
    u32 id, ttsen, ctl;
    u8 frame_type, flage;
    //u8  tb_address = 0;
    FCanPs_Portmap_T *portmap;
    portmap = (FCanPs_Portmap_T *) dev->base_address;
    p = tbuf;
    if(fd_mode == canfd)
    {
        frame_type = CANFD_MTU;
        flage = CANFD_BRS;
        if (len > 64)
        {
            len = 64;
        }
    }
    else
    {
        frame_type = CAN_MTU;
        flage = 0;
        if (len > 8)
        {
            len = 8;
        }
    }
    if(cia_en)
    {
        ttsen = 0x80000000;
    }
    else
    {
        ttsen = 0x00000000;
    }        
    if(can_id & CAN_EFF_FLAG)
    {
        /*extended id length, CAN_EFF_FLAG is set in the msb*/
        id =can_id & CAN_EFF_MASK;
	id|=ttsen;	
    }
    else
    {
        /*standard id*/
        id =can_id & CAN_SFF_MASK;
	id|=ttsen;	
    }
    //set the id for the frame
    //fmsh_print("can_id=%x\n",can_id);
    ctl = can_len2dlc(len);
            
    //use length to judge the frame type is canfd or can
    if(frame_type == CANFD_MTU)
    {
        if(can_id & CAN_EFF_FLAG)
        {
            ctl |= CAN_FD_SET_IDE_MASK;	/*IDE=1 if IDenfitier Extension*/
        }
        else
        {
            ctl &= CAN_FD_OFF_IDE_MASK;/*IDE=0*/
        }
        //SET IDE BIT IN CONTROL(TBUF)
        if(flage == CANFD_BRS)
        {
            ctl |= CAN_FD_SET_BRS_MASK;
        }
        //SET BRS BIT IN CONTROL(TBUF) rate change
        ctl |= CAN_FD_SET_EDL_MASK;
        //SET FDF BIT IN CONTROL(TBUF) frame type
        for(i = 0; i < len/4; i+=1)
        {   
            //tx data buffer offset is tbuf[2] (0x58)
            //reg = CAN_INP(portmap->tbuf[2+i]);
            //fmsh_print("CAN FD original DATA 0x%08x\n", reg);
            CAN_OUTP(*(p+i),portmap->tbuf[2+i]);
            //reg = CAN_INP(portmap->tbuf[2+i]);
            //fmsh_print("data in transmit register is:0x%08x\n", reg);
         }
    }
    /* Transmit can 2.0 frame under canfd mode*/
    else
    {
        ctl &= CAN_FD_OFF_EDL_MASK;//set Extended Data  Length 0, force BRS to be set 0 ;
        ctl &= CAN_FD_OFF_BRS_MASK;
        if(can_id & CAN_EFF_FLAG)
        {
            ctl |= CAN_FD_SET_IDE_MASK;	/*IDE=1 if IDenfitier Extension*/
        }
        else
        {
            ctl &= CAN_FD_OFF_IDE_MASK;/*IDE=0*/
        }
        if(can_id & CAN_RTR_FLAG)
        {
            ctl |= CAN_FD_SET_RTR_MASK;
            CAN_OUTP(id, portmap->tbuf[0]);
            CAN_OUTP(ctl, portmap->tbuf[1]);
        }
        else
        {
            ctl &= CAN_FD_OFF_RTR_MASK;
            //reg = CAN_INP(portmap->tbuf[2]);
            //fmsh_print("CAN original DATA 0x%08x\n", reg);
            CAN_OUTP(*p,portmap->tbuf[2]);
            //reg = CAN_INP(portmap->tbuf[2]);
            //fmsh_print("CAN updata DATA 0x%08x\n", reg);
                             
            //reg = CAN_INP(portmap->tbuf[3]);
            //fmsh_print("CAN original DATA 0x%08x\n", reg);
            CAN_OUTP(*(p+1),portmap->tbuf[3]);
            //reg = CAN_INP(portmap->tbuf[3]);
            //fmsh_print("CAN updata DATA 0x%08x\n", reg);
                             
            //reg = CAN_INP(portmap->reg_grp1);
            //fmsh_print( "TCMD is 0x%02x after writing data into resiger\n", ((reg >> 8) & 0xff));
            //reg = CAN_INP(portmap->reg_grp2);
            //fmsh_print( "Interrupt flag register is 0x%02x after writing data into resiger\n", ((reg >> 8) & 0xff));
        }
    }
    CAN_OUTP(id, portmap->tbuf[0]);
    CAN_OUTP(ctl, portmap->tbuf[1]);
                
    //reg = CAN_INP(portmap->tbuf[0]);
    //fmsh_print("ID 0x%08x\n", reg);
    //reg = CAN_INP(portmap->tbuf[1]);
    //fmsh_print("Control 0x%08x\n", reg);
                
    //reg = CAN_INP(portmap->reg_grp1);
    //fmsh_print("TCMD is 0x%02x before set TPE=1\n", ((reg >> 8) & 0xff));
    return 0;
             
}
/*****************************************************************************
*
* @description
* This function receives message, read the receive buffer data.
*
* @param    dev is a pointer to the instance of can device.
* @param    rbuf is buffer of message data to readback into
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FCanPs_frameReceive(FCanPs_T *dev, u32 *rbuf)
{
    u32 *p;
    //u32 reg, rx_status;
    u32 dlc, control;
    u32 can_id_tmp, can_id;
    int i;
    u8 len;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) dev->base_address; 
    p = rbuf;
    control = CAN_INP(portmap->rbuf[1]) & 0xff;
    can_id_tmp = CAN_INP(portmap->rbuf[0]);
    dlc = control & CAN_FD_SET_DLC_MASK;
    *p++ = CAN_INP(portmap->rbuf[0]);
    *p++ = CAN_INP(portmap->rbuf[1]);
    /*change the CANFD or CAN2.0 data into socketcan data format*/
    if(control & CAN_FD_SET_EDL_MASK)
    {
        len = can_dlc2len(dlc);
    }
    else
    {
        len = get_can_dlc(dlc);
    }
    /*change the CANFD id into socketcan id format*/
    /*canfd*/
    if(control & CAN_FD_SET_EDL_MASK)
    {
        /*extended format*/
        can_id = can_id_tmp;
        if(control & CAN_FD_SET_IDE_MASK)
        {
            can_id |= CAN_EFF_FLAG;
        }
        else
        {
            can_id &= (~CAN_EFF_FLAG);
        }
     /*bit 29,error message not defined here*/
     }
     /*can2.0*/
     else
     {
         can_id = can_id_tmp;
         if(control & CAN_FD_SET_IDE_MASK)
         {
             can_id |= CAN_EFF_FLAG;
         }
         else
         {
             can_id &= (~CAN_EFF_FLAG);
         }
         /*deal with RTR in can2.0*/
         if(control & CAN_FD_SET_RTR_MASK)
         {
             can_id |=CAN_RTR_FLAG ;
         }
     }
            
     /* Data*/
     /*CANFD frames*/
     if(control&CAN_FD_SET_EDL_MASK)
     {
         for (i = 0; i < len/4; i += 1) 
         {       
             *(p+i) = CAN_INP(portmap->rbuf[2+i]);
         }
     }
     else
     {
         /*skb reads the received datas, if the RTR bit not set.*/
	 if(!(control&CAN_FD_SET_RTR_MASK))
         {    
             *p++ = CAN_INP(portmap->rbuf[2]); 
             *p++ = CAN_INP(portmap->rbuf[3]);
         }
     }
    
    return 0;
}
/*****************************************************************************
*
* @description
* This function set Release Receive Buffer
*
* @param    dev is a pointer to the instance of can device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FCanPs_releaseReceiveBuffer(FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) dev->base_address;
    /*reset RREL to release RB slot, next RBUF will be updated.*/
    reg = CAN_INP(portmap->reg_grp1);
    FMSH_BIT_SET(reg, CAN_RCTRL_RREL, 1);
    CAN_OUTP(reg, portmap->reg_grp1);
    return 0;
}
/*****************************************************************************
*
* @description
* This function set TTCAN Transmission Request
*
* @param    dev is a pointer to the instance of can device.
*
* @param    tb_address is pointer to a TB message slot (STB:1~15)
*
* @param    id is reference message ID for TTCAN Function
*
* @param    ide:CAN_set, reference message type is extend
*               CAN_clear, reference message type is standard
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FCanPs_TTCANtransmissionRequest(FCanPs_T *dev, u8 tb_address, u32 id, enum FCanPs_state ide)
{
    //u32 reg = 0;
    //FCanPs_Portmap_T *portmap;
    
    //portmap = (FCanPs_Portmap_T *) dev->base_address; 
    
    //set TBF
    FCanPs_setTransmitBufferSlotFilled(dev, CAN_set);
    //point to the TB to be sent
    FCanPs_setTransmitTriggerPointer(dev, tb_address);
    //set reference message ID
    FCanPs_setREF_ID(dev, id);
    //set reference message type;0 = standard  1= extend
    FCanPs_setREF_IDE(dev, ide);

    return 0;
}
/*****************************************************************************
*
* @description
* This function set PTB Transmission Request
*
* @param    dev is a pointer to the instance of can device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FCanPs_TPEtransmissionRequest(FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) dev->base_address; 
    
    /*set TPE to transmit data. update statistic*/
    reg = CAN_INP(portmap->reg_grp1);
    FMSH_BIT_SET(reg, CAN_TCMD_TPE, 1);
    CAN_OUTP(reg, portmap->reg_grp1);

    return 0;
}
/*****************************************************************************
*
* @description
* This function set STB One Frame Transmission Request
*
* @param    dev is a pointer to the instance of can device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FCanPs_TSONEtransmissionRequest(FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) dev->base_address; 
    
    /*set TSONE to transmit data. update statistic*/
    reg = CAN_INP(portmap->reg_grp1);
    FMSH_BIT_SET(reg, CAN_TCMD_TSONE, 1);
    CAN_OUTP(reg, portmap->reg_grp1);
    
    return 0;
}
/*****************************************************************************
*
* @description
* This function set STB All Frames Transmission Request
*
* @param    dev is a pointer to the instance of can device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FCanPs_TSALLtransmissionRequest(FCanPs_T *dev)
{
    u32 reg = 0;
    FCanPs_Portmap_T *portmap;
    
    portmap = (FCanPs_Portmap_T *) dev->base_address; 
    
    /*set TSONE to transmit data. update statistic*/
    reg = CAN_INP(portmap->reg_grp1);
    FMSH_BIT_SET(reg, CAN_TCMD_TSALL, 1);
    CAN_OUTP(reg, portmap->reg_grp1);
    
    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function set CRL_APB.RST_LPD_IOU2.CAN0_RESET to reset can0
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 lpd_can0_enter_apbRefRst ()
{
    u32 reg;

    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg | 0x00080);

    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function clear CRL_APB.RST_LPD_IOU2.CAN0_RESET to runn can0
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 lpd_can0_exit_apbRefRst ()
{
    u32 reg;

    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg & ~0x00080);

    return 0;
}

/*****************************************************************************
 *
 * @description
 * This function set CRL_APB.RST_LPD_IOU2.CAN1_RESET to reset can1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 lpd_can1_enter_apbRefRst ()
{
    u32 reg;

    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg | 0x00100);

    return 0;
}
/*****************************************************************************
 *
 * @description
 * This function clear CRL_APB.RST_LPD_IOU2.CAN1_RESET to run can1
 *
 * @return   0 if successful, otherwise 1.
 *
 * @note     None.
 *
 ******************************************************************************/
u8 lpd_can1_exit_apbRefRst ()
{
    u32 reg;

    reg = FMSH_ReadReg(FPS_CRL_APB_BASEADDR, 0x238);
    FMSH_WriteReg(FPS_CRL_APB_BASEADDR, 0x238, reg & ~0x00100);

    return 0;
}
