/******************************************************************************
*
* Copyright (C) 2015-2019 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/

/*****************************************************************************/
/**
 * @file pm_api_sys.h
 * PM API System implementation
 * @addtogroup fpm_apis FmshPM APIs
 *
 *****************************************************************************/

#ifndef PM_API_SYS_H
#define PM_API_SYS_H

#include "fmsh_common_types.h"
#include "pm_status.h"
#include "fmsh_ipi.h"
#include "pm_defs.h"
#include "pm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

FPmStatus FPm_InitFmshpm(IpiPsu *IpiInst);

void FPm_SuspendFinalize(void);

enum FPmBootStatus FPm_GetBootStatus(void);

/* System-level API function declarations */
FPmStatus FPm_RequestSuspend(const enum FPmNodeId target,
			   const enum FPmRequestAck ack,
			   const u32 latency,
			   const u8 state);

FPmStatus FPm_SelfSuspend(const enum FPmNodeId nid,
			const u32 latency,
			const u8 state,
			const u64 address);

FPmStatus FPm_ForcePowerDown(const enum FPmNodeId target,
			   const enum FPmRequestAck ack);

FPmStatus FPm_AbortSuspend(const enum FPmAbortReason reason);

FPmStatus FPm_RequestWakeUp(const enum FPmNodeId target,
			  const bool setAddress,
			  const u64 address,
			  const enum FPmRequestAck ack);

FPmStatus FPm_SetWakeUpSource(const enum FPmNodeId target,
			    const enum FPmNodeId wkup_node,
			    const u8 enable);

FPmStatus FPm_SystemShutdown(u32 type, u32 subtype);

FPmStatus FPm_SetConfiguration(const u32 address);

FPmStatus FPm_InitFinalize(void);

/* Callback API function */
/*
 * pm_init_suspend - Init suspend callback arguments (save for custom handling)
 */
struct pm_init_suspend {
	volatile bool received;			/**< Has init suspend callback been received/handled */
	enum FPmSuspendReason reason;	/**< Reason of initializing suspend */
	u32 latency;					/**< Maximum allowed latency */
	u32 state;						/**< Targeted sleep/suspend state */
	u32 timeout;					/**< Period of time the client has to response */
};

/*
 * pm_acknowledge - Acknowledge callback arguments (save for custom handling)
 */
struct pm_acknowledge {
	volatile bool received;		/**< Has acknowledge argument been received? */
	enum FPmNodeId node;		/**< Node argument about which the acknowledge is */
	FPmStatus status;				/**< Acknowledged status */
	u32 opp;					/**< Operating point of node in question */
};


/**
 * FPm_Notifier - Notifier structure registered with a callback by app
 */
typedef struct FPm_Ntfier {
	/**
	 *  Custom callback handler to be called when the notification is
	 *  received. The custom handler would execute from interrupt
	 *  context, it shall return quickly and must not block! (enables
	 *  event-driven notifications)
	 */
	void (*const callback)(struct FPm_Ntfier* const notifier);
	enum FPmNodeId node; /**< Node argument (the node to receive notifications about) */
	enum FPmNotifyEvent event;	/**< Event argument (the event type to receive notifications about) */
	u32 flags;	/**< Flags */
	/**
	 *  Operating point of node in question. Contains the value updated
	 *  when the last event notification is received. User shall not
	 *  modify this value while the notifier is registered.
	 */
	volatile u32 oppoint;
	/**
	 *  How many times the notification has been received - to be used
	 *  by application (enables polling). User shall not modify this
	 *  value while the notifier is registered.
	 */
	volatile u32 received;
	/**
	 *  Pointer to next notifier in linked list. Must not be modified
	 *  while the notifier is registered. User shall not ever modify
	 *  this value.
	 */
	struct FPm_Ntfier* next;
} FPm_Notifier;


/**
 * FPm_NodeStatus - struct containing node status information
 */
typedef struct FPm_NdStatus {
	u32 status;			/**< Node power state */
	u32 requirements;	/**< Current requirements asserted on the node (slaves only) */
	u32 usage;			/**< Usage information (which master is currently using the slave) */
} FPm_NodeStatus;

/**
 * FSecure_AesParams
 */
typedef struct {
	u32 Src; /**< Source address */
    u32 SrcByteLen; /**< Size */
	u32 Iv; /**< initialization vector */
    u32 IvByteLen; /**< Size: 12 or 16 */
	u32 Key; /**< Key */
    u32 KeyByteLen; /**< Size: 16 or 32 */
	u32 Dst; /**< Destination address */
	u32 AesOp; /**< Aes operation: GCM_ENCRYPT or GCM_DECRYPT*/
	u32 KeySrc; /**< Key source: SECURE_CSU_AES_KEY_SRC_DEV or SECURE_CSU_AES_KEY_SRC_KUP*/
}FSecure_AesParams;

/********************************************************************/
/*
 * Global data declarations
 ********************************************************************/
extern struct pm_init_suspend pm_susp;
extern struct pm_acknowledge pm_ack;

void FPm_InitSuspendCb(const enum FPmSuspendReason reason,
		       const u32 latency,
		       const u32 state,
		       const u32 timeout);

void FPm_AcknowledgeCb(const enum FPmNodeId node,
		       const FPmStatus status,
		       const u32 oppoint);

void FPm_NotifyCb(const enum FPmNodeId node,
		const enum FPmNotifyEvent event,
		  const u32 oppoint);

/* API functions for managing PM Slaves */
FPmStatus FPm_RequestNode(const enum FPmNodeId node,
			const u32 capabilities,
			const u32 qos,
			const enum FPmRequestAck ack);
FPmStatus FPm_ReleaseNode(const enum FPmNodeId node);
FPmStatus FPm_SetRequirement(const enum FPmNodeId nid,
			   const u32 capabilities,
			   const u32 qos,
			   const enum FPmRequestAck ack);
FPmStatus FPm_SetMaxLatency(const enum FPmNodeId node,
			  const u32 latency);

/* Miscellaneous API functions */
FPmStatus FPm_GetApiVersion(u32 *version);

FPmStatus FPm_GetNodeStatus(const enum FPmNodeId node,
			  FPm_NodeStatus *const nodestatus);

FPmStatus FPm_RegisterNotifier(FPm_Notifier* const notifier);
FPmStatus FPm_UnregisterNotifier(FPm_Notifier* const notifier);

FPmStatus FPm_GetOpCharacteristic(const enum FPmNodeId node,
				const enum FPmOpCharType type,
				u32* const result);

/* Direct-Control API functions */
FPmStatus FPm_ResetAssert(const enum FPmReset reset,
			const enum FPmResetAction resetaction);

FPmStatus FPm_ResetGetStatus(const enum FPmReset reset, u32 *status);

FPmStatus FPm_MmioWrite(const u32 address, const u32 mask, const u32 value);

FPmStatus FPm_MmioRead(const u32 address, u32 *const value);

/* Clock API */
FPmStatus FPm_ClockEnable(const enum FPmClock clock);
FPmStatus FPm_ClockDisable(const enum FPmClock clock);
FPmStatus FPm_ClockGetStatus(const enum FPmClock clock, u32 *const status);

FPmStatus FPm_ClockSetDivider(const enum FPmClock clock, const u32 divider);
FPmStatus FPm_ClockGetDivider(const enum FPmClock clock, u32 *const divider);

FPmStatus FPm_ClockSetParent(const enum FPmClock clock,
			   const enum FPmClock parent);
FPmStatus FPm_ClockGetParent(const enum FPmClock clock,
			   enum FPmClock *const parent);

FPmStatus FPm_ClockSetRate(const enum FPmClock clock, const u32 rate);
FPmStatus FPm_ClockGetRate(const enum FPmClock clock, u32 *const rate);

/* PLL API */
FPmStatus FPm_PllSetParameter(const enum FPmNodeId node,
			    const enum FPmPllParam parameter,
			    const u32 value);
FPmStatus FPm_PllGetParameter(const enum FPmNodeId node,
			    const enum FPmPllParam parameter,
			    u32 *const value);

FPmStatus FPm_PllSetMode(const enum FPmNodeId node, const enum FPmPllMode mode);
FPmStatus FPm_PllGetMode(const enum FPmNodeId node, enum FPmPllMode* const mode);

/* PIN Mux control API */
FPmStatus FPm_PinCtrlRequest(const u32 pin);
FPmStatus FPm_PinCtrlRelease(const u32 pin);

FPmStatus FPm_PinCtrlSetFunction(const u32 pin, const enum FPmPinFn fn);
FPmStatus FPm_PinCtrlGetFunction(const u32 pin, enum FPmPinFn* const fn);

FPmStatus FPm_PinCtrlSetParameter(const u32 pin,
				const enum FPmPinParam param,
				const u32 value);
FPmStatus FPm_PinCtrlGetParameter(const u32 pin,
				const enum FPmPinParam param,
				u32* const value);

/* SECURE API */
FPmStatus FPm_SecureSha(const u32 SrcAddr,
			    const u32 DestAddr,
			    const u32 MessageByteLen);
FPmStatus FPm_SecureRsa(const u32 SrcAddr, const u32 SrcSize, 
                u32 ModAddr, const u32 Flags);
FPmStatus FPm_SecureAes(const u32 SrcAddr);

FPmStatus FPm_FeatureCheck(const u32 apiId);

FPmStatus FPm_FpgaLoad(const u32 AddrHigh, const u32 AddrLow,
			const u32 KeyAddr, const u32 Flags);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* PM_API_SYS_H */
