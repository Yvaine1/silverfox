#ifndef _FMSH_CAN_COMMON_H_ /* prevent circular inclusions */
#define _FMSH_CAN_COMMON_H_ /* by using protection macros */
#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/

#include "fmsh_common.h"

/************************** Constant Definitions *****************************/
/*use Frame len to distinguish CAN Frame and CANFD Fram in CANFD mode*/
#define CAN_MTU        16
#define CAN_MAX_DLC    8
#define CAN_MAX_DLEN   8
#define CANFD_MTU      72
#define CANFD_MAX_DLEN 64
#define TTSEN_8_32_SHIFT                            \
    24 /*TTSEN bit used for 32 bit register read or \
          write*/

/*for canfd tbuf reg bit change*/
#define CAN_FD_SET_IDE_MASK            0x80
#define CAN_FD_OFF_IDE_MASK            0x7f
#define CAN_FD_SET_BRS_MASK            0x10 /*can fd Bit Rate Switch mask*/
#define CAN_FD_OFF_BRS_MASK            0xef
#define CAN_FD_SET_EDL_MASK            0x20 /*Extended Data Length*/
#define CAN_FD_OFF_EDL_MASK            0xdf
#define CAN_FD_SET_RTR_MASK            0x40
#define CAN_FD_OFF_RTR_MASK            0xbf

/*for canfd rbuf reg bit change*/
#define CAN_FD_SET_DLC_MASK            0x0f
#define CAN_FD_SET_EDL_MASK            0x20

/*for canfd other reg bit change*/
#define CAN_FD_INTR_ALL_MASK           0xff
#define CAN_FD_SET_RIE_MASK            0x80
  
#define CAN_FD_SET_BIT_ERROR_MASK      0x1
#define CAN_FD_SET_FORM_ERROR_MASK     0x2
#define CAN_FD_SET_STUFF_ERROR_MASK    0x3
#define CAN_FD_SET_ACK_ERROR_MASK      0x4
#define CAN_FD_SET_CRC_ERROR_MASK      0x5

/* error class (mask) in can_id */
#define CAN_ERR_TX_TIMEOUT             0x00000001U /* TX timeout (by netdevice driver) */
#define CAN_ERR_LOSTARB                0x00000002U /* lost arbitration    / data[0]    */
#define CAN_ERR_CRTL                   0x00000004U /* controller problems / data[1]    */
#define CAN_ERR_PROT                   0x00000008U /* protocol violations / data[2..3] */
#define CAN_ERR_TRX                    0x00000010U /* transceiver status  / data[4]    */
#define CAN_ERR_ACK                    0x00000020U /* received no ACK on transmission */
#define CAN_ERR_BUSOFF                 0x00000040U /* bus off */
#define CAN_ERR_BUSERROR               0x00000080U /* bus error (may flood!) */
#define CAN_ERR_RESTARTED              0x00000100U /* controller restarted */

/* arbitration lost in bit ... / data[0] */
#define CAN_ERR_LOSTARB_UNSPEC         0x00 /* unspecified */
                                            /* else bit number in bitstream */

/* error status of CAN-controller / data[1] */
#define CAN_ERR_CRTL_UNSPEC            0x00 /* unspecified */
#define CAN_ERR_CRTL_RX_OVERFLOW       0x01 /* RX buffer overflow */
#define CAN_ERR_CRTL_TX_OVERFLOW       0x02 /* TX buffer overflow */
#define CAN_ERR_CRTL_RX_WARNING        0x04 /* reached warning level for RX errors */
#define CAN_ERR_CRTL_TX_WARNING        0x08 /* reached warning level for TX errors */
#define CAN_ERR_CRTL_RX_PASSIVE        0x10 /* reached error passive status RX */
#define CAN_ERR_CRTL_TX_PASSIVE        0x20 /* reached error passive status TX */
/* (at least one error counter exceeds */
/* the protocol-defined level of 127)  */
#define CAN_ERR_CRTL_ACTIVE            0x40 /* recovered to error active state */

/* error in CAN protocol (type) / data[2] */
#define CAN_ERR_PROT_UNSPEC            0x00 /* unspecified */
#define CAN_ERR_PROT_BIT               0x01 /* single bit error */
#define CAN_ERR_PROT_FORM              0x02 /* frame format error */
#define CAN_ERR_PROT_STUFF             0x04 /* bit stuffing error */
#define CAN_ERR_PROT_BIT0              0x08 /* unable to send dominant bit */
#define CAN_ERR_PROT_BIT1              0x10 /* unable to send recessive bit */
#define CAN_ERR_PROT_OVERLOAD          0x20 /* bus overload */
#define CAN_ERR_PROT_ACTIVE            0x40 /* active error announcement */
#define CAN_ERR_PROT_TX                0x80 /* error occurred on transmission */

/* error in CAN protocol (location) / data[3] */
#define CAN_ERR_PROT_LOC_UNSPEC        0x00 /* unspecified */
#define CAN_ERR_PROT_LOC_SOF           0x03 /* start of frame */
#define CAN_ERR_PROT_LOC_ID28_21       0x02 /* ID bits 28 - 21 (SFF: 10 - 3) */
#define CAN_ERR_PROT_LOC_ID20_18       0x06 /* ID bits 20 - 18 (SFF: 2 - 0 )*/
#define CAN_ERR_PROT_LOC_SRTR          0x04 /* substitute RTR (SFF: RTR) */
#define CAN_ERR_PROT_LOC_IDE           0x05 /* identifier extension */
#define CAN_ERR_PROT_LOC_ID17_13       0x07 /* ID bits 17-13 */
#define CAN_ERR_PROT_LOC_ID12_05       0x0F /* ID bits 12-5 */
#define CAN_ERR_PROT_LOC_ID04_00       0x0E /* ID bits 4-0 */
#define CAN_ERR_PROT_LOC_RTR           0x0C /* RTR */
#define CAN_ERR_PROT_LOC_RES1          0x0D /* reserved bit 1 */
#define CAN_ERR_PROT_LOC_RES0          0x09 /* reserved bit 0 */
#define CAN_ERR_PROT_LOC_DLC           0x0B /* data length code */
#define CAN_ERR_PROT_LOC_DATA          0x0A /* data section */
#define CAN_ERR_PROT_LOC_CRC_SEQ       0x08 /* CRC sequence */
#define CAN_ERR_PROT_LOC_CRC_DEL       0x18 /* CRC delimiter */
#define CAN_ERR_PROT_LOC_ACK           0x19 /* ACK slot */
#define CAN_ERR_PROT_LOC_ACK_DEL       0x1B /* ACK delimiter */
#define CAN_ERR_PROT_LOC_EOF           0x1A /* end of frame */
#define CAN_ERR_PROT_LOC_INTERM        0x12 /* intermission */

/* error status of CAN-transceiver / data[4] */
/*                                             CANH CANL */
#define CAN_ERR_TRX_UNSPEC             0x00 /* 0000 0000 */
#define CAN_ERR_TRX_CANH_NO_WIRE       0x04 /* 0000 0100 */
#define CAN_ERR_TRX_CANH_SHORT_TO_BAT  0x05 /* 0000 0101 */
#define CAN_ERR_TRX_CANH_SHORT_TO_VCC  0x06 /* 0000 0110 */
#define CAN_ERR_TRX_CANH_SHORT_TO_GND  0x07 /* 0000 0111 */
#define CAN_ERR_TRX_CANL_NO_WIRE       0x40 /* 0100 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_BAT  0x50 /* 0101 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_VCC  0x60 /* 0110 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_GND  0x70 /* 0111 0000 */
#define CAN_ERR_TRX_CANL_SHORT_TO_CANH 0x80 /* 1000 0000 */

/* controller specific additional information / data[5..7] */

/* special address description flags for the CAN_ID */
#define CAN_EFF_FLAG                   0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG                   0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG                   0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK                   0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK                   0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK                   0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/* for canfd brs bit */
#define CANFD_BRS                      0x01 /* bit rate switch (second bitrate for payload data) */
#define CANFD_ESI                      0x02 /* error state indicator of the transmitting node */
/**************************** Type Definitions *******************************/
// can_model include (0)can2.0 frame (1)canfd frame
enum can_mode { can2 = 0, canfd = 1 };

// can_tx_model
enum can_tx_mode { full_mode = 0, stb_fifo = 1, stb_prio = 2, ptb_mode = 3 };

// can_test_model
enum can_test_mode {
    ExtTestMode = 0,
    IntTestMode = 1,
    TPSSMode = 2,
    TSSSMode = 3,
    ListenOnlyMode = 4
};

// for fliter
enum can_acf_mode {
    se_acf_mode = 0,  // acf accepts both standard and extended frames
    s_acf_mode = 1,   // acf accepts only standard frames
    e_acf_mode = 2    // acf accepts only extended frames
};

// for ttcan trigger type
enum can_trigger_type {
    imm_t = 0,
    tim_t = 1,
    sst_t = 2,
    t_start_t = 3,
    t_stop_t = 4
};

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
    u16 DeviceId;     /**< Unique ID  of device */
    u32 BaseAddress;  /**< Base address of device (IPIF) */
    u32 InputClockHz; /**< Input clock frequency */
    enum can_tx_mode TxMode;
    // for canfd sample point
    u32 sample_point;
    u32 d_sample_point;

} FCanPs_Config;

/**
 * CAN bit-timing parameters
 *
 * For further information, please read chapter "8 BIT TIMING
 * REQUIREMENTS" of the "Bosch CAN Specification version 2.0"
 * at http://www.semiconductors.bosch.de/pdf/can2spec.pdf.
 */

// descript both slow bit timing and fast bit timing
struct can_bittiming {
    u32 bitrate;      /*Slow bit-rate in bits/second */
    u32 sample_point; /*Slow sample point in one-tenth of a percent */
    u32 tq;           /*Slow Time quanta (TQ) in nanoseconds */
    u32 prop_seg;     /*Slow Propagation segment in TQs */
    u32 phase_seg1;   /*Slow Phase buffer segment 1 in TQs */
    u32 phase_seg2;   /*Slow Phase buffer segment 2 in TQs */
    u32 sjw;          /*Slow Synchronisation jump width in TQs */
    u32 brp;          /*Slow Bit-rate prescaler */
};

/**
 * CAN harware-dependent bit-timing constant
 *
 * Used for calculating and checking bit-timing parameters
 */

struct can_bittiming_const {
    char name[16]; /* Name of the CAN controller hardware */
    u32 tseg1_min; /*Slow time segement 1 = prop_seg + phase_seg1 */
    u32 tseg1_max;
    u32 tseg2_min; /*Slow time segement 2 = phase_seg2 */
    u32 tseg2_max;
    u32 sjw_max;   /*Slow synchronisation jump width */
    u32 brp_min;   /*Slow bit-rate prescaler */
    u32 brp_max;
    u32 brp_inc;
};
/**
 * DESCRIPTION
 *  This is the primary structure used when dealing with all devices.
 *  It serves as a hardware abstraction layer for driver code and also
 *  allows this code to support more than one device of the same type
 *  simultaneously.  This structure needs to be initialized with
 *  meaningful values before a pointer to it is passed to a driver
 *  initialization function.
 * PARAMETERS
 *  baseAddress     physical base address of device
 *  instance        device private data structure pointer
 *  compVersion     device version identification number
 *  compType        device identification number
 */
typedef struct FCanPs {
    void *base_address;
    u32 input_clock;
    enum can_tx_mode ipms_tx_mode;
    struct can_bittiming bt;
    struct can_bittiming d_bt;
    struct can_bittiming_const CanFd_btc;
    struct can_bittiming_const CanFd_data_btc;
    u32 comp_version;
    u32 comp_type;
    u32 id;
} FCanPs_T;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
