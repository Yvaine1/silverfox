#ifndef _FMSH_CAN_PUBLIC_H_ /* prevent circular inclusions */
#define _FMSH_CAN_PUBLIC_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "fmsh_can_common.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**
 * DESCRIPTION
 *  This is a generic data type used for 1-bit wide bitfields which have
 *  a "set/clear" property.  This is used when modifying registers
 *  within a peripheral's memory map.
 */
enum FCanPs_state {
    CAN_set = 1,
    CAN_clear = 0,
    CAN_err = -1,
};


/************************** Function Prototypes ******************************/
FCanPs_Config *FCanPs_LookupConfig(u16 DeviceId);

u8 FCanPs_init(FCanPs_T *dev, FCanPs_Config *cfg);

u8 FCanPs_setBaudRate(FCanPs_T *dev, u32 baud, u32 d_baud);

u8 FCanPs_setFD_ISOMode(FCanPs_T *dev, enum FCanPs_state state);

u8 FCanPs_setFilter(FCanPs_T *dev, u32 mask, u32 order, enum can_acf_mode mode, u32 id, enum FCanPs_state state);

u8 can_dlc2len(u8 can_dlc);

u8 get_can_dlc(u8 can_dlc);

//mode
u8 FCanPs_setResetMode(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setExtTestMode(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setIntTestMode(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTpssMode(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTsssMode(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setListenOnlyMode(FCanPs_T *dev, enum FCanPs_state state);

u8 FCanPs_setErrLimit(FCanPs_T *dev, u8 ewl);

u8 FCanPs_setAlmostFull(FCanPs_T *dev, u8 warning_limit);

u8 FCanPs_setReceiveBufferOverflowMode(FCanPs_T *dev, enum FCanPs_state state);

//transmit
u8 FCanPs_setTransmitBufferSelect (FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTransmitBufferNext(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTransmitBufferPointer(FCanPs_T *dev, u8 address);
u8 FCanPs_setTransmitSecondaryAbort(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTransmitPrimaryAbort(FCanPs_T *dev, enum FCanPs_state state);
void FCanPs_setXmitMode(FCanPs_T* dev, enum can_tx_mode tx_mode);
u8 FCanPs_FrameTransmit(FCanPs_T* dev, u32 can_id, u32 *tbuf, u8 len, enum can_mode fd_mode, enum FCanPs_state cia_en);
u8 FCanPs_TTCANtransmissionRequest(FCanPs_T *dev, u8 tb_address, u32 id, enum FCanPs_state ide);
u8 FCanPs_TPEtransmissionRequest(FCanPs_T *dev);
u8 FCanPs_TSONEtransmissionRequest(FCanPs_T *dev);
u8 FCanPs_TSALLtransmissionRequest(FCanPs_T *dev);
u32 FCanPs_getTransmissionCompleteStatus(FCanPs_T *dev);
u8 FCanPs_frameReceive(FCanPs_T *dev, u32 *rbuf);
u8 FCanPs_releaseReceiveBuffer(FCanPs_T *dev);

//interrupt
u8 FCanPs_setReciveInterrupt(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setReciveBufferOverrunInterrupt(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setReceiveBufferFullInterrupt (FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setReceiveBufferAlmostFullInterrupt (FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTransmissionPrimaryInterrupt (FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTransmissionSecondaryInterrupt (FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setErrorInterrupt (FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setArbitrationLostInterrupt(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setErrorPassiveInterrupt(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setBusErrorInterrupt(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setWatchTriggerInterrupt(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTimeTriggerInterrupt(FCanPs_T *dev, enum FCanPs_state state);

u32 FCanPs_clearIntFlage(u32 reg);
u32 FCanPs_clearTTCANIntFlage(u32 reg);

//status
u8 FCanPs_getKindOfError(FCanPs_T *dev);
u8 FCanPs_getReceiveErrorCount(FCanPs_T *dev);
u8 FCanPs_getTransmitErrorCount(FCanPs_T *dev);
u8 FCanPs_getBusStatus(FCanPs_T *dev);

//TTCAN
u8 FCanPs_setREF_ID(FCanPs_T *dev, u32 id);
u8 FCanPs_setREF_IDE(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTransmitTriggerPointer(FCanPs_T *dev, u8 address);
u8 FCanPs_setTriggerTime(FCanPs_T *dev, u16 tt);
u8 FCanPs_setTTCAN_TimePrescaler(FCanPs_T *dev, u8 presc);
u8 FCanPs_setTimeTrigger(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTriggerType(FCanPs_T *dev, enum can_trigger_type ttype);
u8 FCanPs_setTransmitBufferSlotEmpty(FCanPs_T *dev, enum FCanPs_state state);
u8 FCanPs_setTransmitBufferSlotFilled(FCanPs_T *dev, enum FCanPs_state state);

//Cia603
u8 FCanPs_setTimeStamping(FCanPs_T *dev, enum FCanPs_state state);

u8 lpd_can0_enter_apbRefRst();
u8 lpd_can1_enter_apbRefRst();
u8 lpd_can0_exit_apbRefRst();
u8 lpd_can1_exit_apbRefRst();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
