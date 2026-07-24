/******************************************************************************
*
* Copyright (C) 2015-2019 FMSH, Inc.  All rights reserved.
*
*
******************************************************************************/
/**
 * @file pm_clock.c
 *
 * PM Definitions implementation
 * @addtogroup fpm_apis FmshPM APIs
 * @{
 *****************************************************************************/
#include "pm_clock.h"
#include "pm_common.h"

#define PM_CLOCK_TYPE_DIV0	(1U << PM_CLOCK_DIV0_ID)	/* bits 13:8 */
#define PM_CLOCK_TYPE_DIV1	(1U << PM_CLOCK_DIV1_ID)	/* bits 21:16 */
#define PM_DIV_WIDTH		0x3FU
#define PM_2xDIV_WIDTH		(PM_DIV_WIDTH * PM_DIV_WIDTH)

#define PM_CLOCK_HAS_DIV0(clk)	(0U != ((clk)->type & PM_CLOCK_TYPE_DIV0))
#define PM_CLOCK_HAS_DIV1(clk)	(0U != ((clk)->type & PM_CLOCK_TYPE_DIV1))


/**
 * Pair of multiplexer select value and selected clock input
 */
typedef struct {
	/**
	ID of the clock that is selected with the 'select' value
	*/
	const enum FPmClock clkIn;
	/**
	Select value of the clock multiplexer
	*/
	const u8 select;
} FPmClockSel2ClkIn;

/**
 * MUX select values to clock input mapping
 */
typedef struct {
	/** Mux select to pll mapping at the input of the multiplexer */
	const FPmClockSel2ClkIn* const inputs;
	/** Size of the inputs array*/
	const u8 size;
	/** Number of bits of mux select*/
	const u8 bits;
	/** Number of bits to shift 'bits' in order to get mux select mask*/
	const u8 shift;
} FPmClockMux;

/**
 * Clock model
 */
typedef struct FPmClkModel {
	/** Clock ID*/
	const enum FPmClock id;
	/** Pointer to the mux model*/
	const FPmClockMux* const mux;
	/** Type specifying the available divisors*/
	const u8 type;
	/** Next clock in the list*/
	const struct FPmClkModel* const next;
} FPmClockModel;

/******************************************************************************/
/* Clock multiplexer models */

static const FPmClockSel2ClkIn advSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_APLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 3U,
	},
};

static FPmClockMux advMux = {
	.inputs = advSel2ClkIn,
	.size = PM_ARRAY_SIZE(advSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn avdSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_APLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static FPmClockMux avdMux = {
	.inputs = avdSel2ClkIn,
	.size = PM_ARRAY_SIZE(avdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn aiodSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_APLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static FPmClockMux aiodMux = {
	.inputs = aiodSel2ClkIn,
	.size = PM_ARRAY_SIZE(aiodSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn vdrSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_VPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_RPLL_TO_FPD,
		.select = 3U,
	},
};

static FPmClockMux vdrMux = {
	.inputs = vdrSel2ClkIn,
	.size = PM_ARRAY_SIZE(vdrSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn dvSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_DPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 1U,
	},
};

static FPmClockMux dvMux = {
	.inputs = dvSel2ClkIn,
	.size = PM_ARRAY_SIZE(dvSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn iovdSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_VPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static FPmClockMux iovdMux = {
	.inputs = iovdSel2ClkIn,
	.size = PM_ARRAY_SIZE(iovdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn ioadSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_APLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static FPmClockMux ioadMux = {
	.inputs = ioadSel2ClkIn,
	.size = PM_ARRAY_SIZE(ioadSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn iodaSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_APLL,
		.select = 3U,
	},
};

static FPmClockMux iodaMux = {
	.inputs = iodaSel2ClkIn,
	.size = PM_ARRAY_SIZE(iodaSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn iorSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	},
};

static FPmClockMux iorMux = {
	.inputs = iorSel2ClkIn,
	.size = PM_ARRAY_SIZE(iorSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn iordFpdSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL_TO_FPD,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL_TO_FPD,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL,
		.select = 3U,
	},
};

static FPmClockMux iordFpdMux = {
	.inputs = iordFpdSel2ClkIn,
	.size = PM_ARRAY_SIZE(iordFpdSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn iordSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL_TO_LPD,
		.select = 3U,
	},
};

static FPmClockMux iordMux = {
	.inputs = iordSel2ClkIn,
	.size = PM_ARRAY_SIZE(iordSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn iorvSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_VPLL_TO_LPD,
		.select = 3U,
	},
};

static FPmClockMux iorvMux = {
	.inputs = iorvSel2ClkIn,
	.size = PM_ARRAY_SIZE(iorvSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn riodSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_RPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_IOPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL_TO_LPD,
		.select = 3U,
	},
};

static FPmClockMux riodMux = {
	.inputs = riodSel2ClkIn,
	.size = PM_ARRAY_SIZE(riodSel2ClkIn),
	.bits = 2U,
	.shift = 0U,
};

static const FPmClockSel2ClkIn iordPsRefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_IOPLL,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_RPLL,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_DPLL_TO_LPD,
		.select = 3U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 4U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 5U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 6U,
	}, {
		.clkIn = PM_CLOCK_EXT_PSS_REF,
		.select = 7U,
	},
};

static FPmClockMux iordPsRefMux = {
	.inputs = iordPsRefSel2ClkIn,
	.size = PM_ARRAY_SIZE(iordPsRefSel2ClkIn),
	.bits = 3U,
	.shift = 0U,
};

static FPmClockMux can0MioMux = {
	.inputs = NULL,		/* NULL is reserved for MIO inputs */
	.size = 0U,
	.bits = 7U,
	.shift = 0U,
};

static FPmClockMux can1MioMux = {
	.inputs = NULL,		/* NULL is reserved for MIO inputs */
	.size = 0U,
	.bits = 7U,
	.shift = 15U,
};

static const FPmClockSel2ClkIn can0Sel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_CAN0_REF,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_CAN0_MIO,
		.select = 1U,
	},
};

static FPmClockMux can0Mux = {
	.inputs = can0Sel2ClkIn,
	.size = PM_ARRAY_SIZE(can0Sel2ClkIn),
	.bits = 1U,
	.shift = 7U,
};

static const FPmClockSel2ClkIn can1Sel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_CAN1_REF,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_CAN1_MIO,
		.select = 1U,
	},
};

static FPmClockMux can1Mux = {
	.inputs = can1Sel2ClkIn,
	.size = PM_ARRAY_SIZE(can1Sel2ClkIn),
	.bits = 1U,
	.shift = 22U,
};

static const FPmClockSel2ClkIn gemTsuSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM_TSU_REF,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_MIO26,
		.select = 1U,
	}, {
		.clkIn = PM_CLOCK_GEM_TSU_REF,
		.select = 2U,
	}, {
		.clkIn = PM_CLOCK_EXT_MIO50_OR_MIO51,
		.select = 3U,
	},
};

static FPmClockMux gemTsuMux = {
	.inputs = gemTsuSel2ClkIn,
	.size = PM_ARRAY_SIZE(gemTsuSel2ClkIn),
	.bits = 2U,
	.shift = 20U,
};

static const FPmClockSel2ClkIn gem0RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM0_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM0_TX_EMIO,
		.select = 1U,
	},
};

static FPmClockMux gem0RefMux = {
	.inputs = gem0RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem0RefSel2ClkIn),
	.bits = 1U,
	.shift = 1U,
};

static const FPmClockSel2ClkIn gem1RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM1_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM1_TX_EMIO,
		.select = 1U,
	},
};

static FPmClockMux gem1RefMux = {
	.inputs = gem1RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem1RefSel2ClkIn),
	.bits = 1U,
	.shift = 6U,
};

static const FPmClockSel2ClkIn gem2RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM2_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM2_TX_EMIO,
		.select = 1U,
	},
};

static FPmClockMux gem2RefMux = {
	.inputs = gem2RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem2RefSel2ClkIn),
	.bits = 1U,
	.shift = 11U,
};

static const FPmClockSel2ClkIn gem3RefSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_GEM3_REF_UNGATED,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_GEM3_TX_EMIO,
		.select = 1U,
	},
};

static FPmClockMux gem3RefMux = {
	.inputs = gem3RefSel2ClkIn,
	.size = PM_ARRAY_SIZE(gem3RefSel2ClkIn),
	.bits = 1U,
	.shift = 16U,
};

static const FPmClockSel2ClkIn fpdWdtSel2ClkIn[] = {
	{
		.clkIn = PM_CLOCK_TOPSW_LSBUS,
		.select = 0U,
	}, {
		.clkIn = PM_CLOCK_EXT_SWDT1,
		.select = 1U,
	},
};

static FPmClockMux fpdWdtMux = {
	.inputs = fpdWdtSel2ClkIn,
	.size = PM_ARRAY_SIZE(fpdWdtSel2ClkIn),
	.bits = 1U,
	.shift = 0U,
};

/******************************************************************************/
/* Clock models (only clocks with mux and divisor need to be modeled) */

static FPmClockModel pmClockAcpu = {
	.id = PM_CLOCK_ACPU,
	.mux = &advMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = NULL,
};

static FPmClockModel pmClockDbgTrace = {
	.id = PM_CLOCK_DBG_TRACE,
	.mux = &iodaMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockAcpu,
};

static FPmClockModel pmClockDbgFpd = {
	.id = PM_CLOCK_DBG_FPD,
	.mux = &iodaMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDbgTrace,
};

static FPmClockModel pmClockDpVideo = {
	.id = PM_CLOCK_DP_VIDEO_REF,
	.mux = &vdrMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDbgFpd,
};

static FPmClockModel pmClockDpAudio = {
	.id = PM_CLOCK_DP_AUDIO_REF,
	.mux = &vdrMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDpVideo,
};

static FPmClockModel pmClockDpStc = {
	.id = PM_CLOCK_DP_STC_REF,
	.mux = &vdrMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDpAudio,
};

static FPmClockModel pmClockDdr = {
	.id = PM_CLOCK_DDR_REF,
	.mux = &dvMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDpStc,
};

static FPmClockModel pmClockGpu = {
	.id = PM_CLOCK_GPU_REF,
	.mux = &iovdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDdr,
};

static FPmClockModel pmClockSata = {
	.id = PM_CLOCK_SATA_REF,
	.mux = &ioadMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockGpu,
};

static FPmClockModel pmClockPcie = {
	.id = PM_CLOCK_PCIE_REF,
	.mux = &iordFpdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockSata,
};

static FPmClockModel pmClockGdma = {
	.id = PM_CLOCK_GDMA_REF,
	.mux = &avdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockPcie,
};

static FPmClockModel pmClockDpDma = {
	.id = PM_CLOCK_DPDMA_REF,
	.mux = &avdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockGdma,
};

static FPmClockModel pmClockTopSwMain = {
	.id = PM_CLOCK_TOPSW_MAIN,
	.mux = &avdMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockDpDma,
};

static FPmClockModel pmClockTopSwLsBus = {
	.id = PM_CLOCK_TOPSW_LSBUS,
	.mux = &aiodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockTopSwMain,
};

static FPmClockModel pmClockDbgTstmp = {
	.id = PM_CLOCK_DBG_TSTMP,
	.mux = &iodaMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockTopSwLsBus,
};

static FPmClockModel pmClockUsbSof = {
	.id = PM_CLOCK_USB_SOF_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDbgTstmp,
};

static FPmClockModel pmClockGem0RefUngated = {
	.id = PM_CLOCK_GEM0_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUsbSof,
};

static FPmClockModel pmClockGem1RefUngated = {
	.id = PM_CLOCK_GEM1_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem0RefUngated,
};

static FPmClockModel pmClockGem2RefUngated = {
	.id = PM_CLOCK_GEM2_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem1RefUngated,
};

static FPmClockModel pmClockGem3RefUngated = {
	.id = PM_CLOCK_GEM3_REF_UNGATED,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem2RefUngated,
};

static FPmClockModel pmClockUsb0Bus = {
	.id = PM_CLOCK_USB0_BUS_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockGem3RefUngated,
};

static FPmClockModel pmClockUsb1Bus = {
	.id = PM_CLOCK_USB1_BUS_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUsb0Bus,
};

static FPmClockModel pmClockQSpi = {
	.id = PM_CLOCK_QSPI_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUsb1Bus,
};

static FPmClockModel pmClockSdio0 = {
	.id = PM_CLOCK_SDIO0_REF,
	.mux = &iorvMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockQSpi,
};

static FPmClockModel pmClockSdio1 = {
	.id = PM_CLOCK_SDIO1_REF,
	.mux = &iorvMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSdio0,
};

static FPmClockModel pmClockUart0 = {
	.id = PM_CLOCK_UART0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSdio1,
};

static FPmClockModel pmClockUart1 = {
	.id = PM_CLOCK_UART1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUart0,
};

static FPmClockModel pmClockSpi0 = {
	.id = PM_CLOCK_SPI0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockUart1,
};

static FPmClockModel pmClockSpi1 = {
	.id = PM_CLOCK_SPI1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSpi0,
};

static FPmClockModel pmClockCan0Ref = {
	.id = PM_CLOCK_CAN0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockSpi1,
};

static FPmClockModel pmClockCan1Ref = {
	.id = PM_CLOCK_CAN1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockCan0Ref,
};

static FPmClockModel pmClockCpuR5 = {
	.id = PM_CLOCK_CPU_R5,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockCan1Ref,
};

static FPmClockModel pmClockIouSwitch = {
	.id = PM_CLOCK_IOU_SWITCH,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockCpuR5,
};

static FPmClockModel pmClockCsuPll = {
	.id = PM_CLOCK_CSU_PLL,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockIouSwitch,
};

static FPmClockModel pmClockPcap = {
	.id = PM_CLOCK_PCAP,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockCsuPll,
};

static FPmClockModel pmClockLpdSwitch = {
	.id = PM_CLOCK_LPD_SWITCH,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockPcap,
};

static FPmClockModel pmClockLpdLsBus = {
	.id = PM_CLOCK_LPD_LSBUS,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockLpdSwitch,
};

static FPmClockModel pmClockDbgLpd = {
	.id = PM_CLOCK_DBG_LPD,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockLpdLsBus,
};

static FPmClockModel pmClockNand = {
	.id = PM_CLOCK_NAND_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDbgLpd,
};

static FPmClockModel pmClockAdma = {
	.id = PM_CLOCK_ADMA_REF,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockNand,
};

static FPmClockModel pmClockPl0 = {
	.id = PM_CLOCK_PL0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockAdma,
};

static FPmClockModel pmClockPl1 = {
	.id = PM_CLOCK_PL1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl0,
};

static FPmClockModel pmClockPl2 = {
	.id = PM_CLOCK_PL2_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl1,
};

static FPmClockModel pmClockPl3 = {
	.id = PM_CLOCK_PL3_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl2,
};

static FPmClockModel pmClockGemTsuRef = {
	.id = PM_CLOCK_GEM_TSU_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPl3,
};

static FPmClockModel pmClockDll = {
	.id = PM_CLOCK_DLL_REF,
	.mux = &iorMux,
	.type = 0U,
	.next = &pmClockGemTsuRef,
};

static FPmClockModel pmClockPsSysMon = {
	.id = PM_CLOCK_PSSYSMON_REF,
	.mux = &riodMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockDll,
};

static FPmClockModel pmClockI2C0 = {
	.id = PM_CLOCK_I2C0_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockPsSysMon,
};

static FPmClockModel pmClockI2C1 = {
	.id = PM_CLOCK_I2C1_REF,
	.mux = &iordMux,
	.type = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1,
	.next = &pmClockI2C0,
};

static FPmClockModel pmClockTimeStamp = {
	.id = PM_CLOCK_TIMESTAMP_REF,
	.mux = &iordPsRefMux,
	.type = PM_CLOCK_TYPE_DIV0,
	.next = &pmClockI2C1,
};

static FPmClockModel pmClockCan0Mio = {
	.id = PM_CLOCK_CAN0_MIO,
	.mux = &can0MioMux,
	.type = 0U,
	.next = &pmClockTimeStamp,
};

static FPmClockModel pmClockCan0 = {
	.id = PM_CLOCK_CAN0,
	.mux = &can0Mux,
	.type = 0U,
	.next = &pmClockCan0Mio,
};

static FPmClockModel pmClockCan1Mio = {
	.id = PM_CLOCK_CAN1_MIO,
	.mux = &can1MioMux,
	.type = 0U,
	.next = &pmClockCan0,
};

static FPmClockModel pmClockCan1 = {
	.id = PM_CLOCK_CAN1,
	.mux = &can1Mux,
	.type = 0U,
	.next = &pmClockCan1Mio,
};

static FPmClockModel pmClockGemTsu = {
	.id = PM_CLOCK_GEM_TSU,
	.mux = &gemTsuMux,
	.type = 0U,
	.next = &pmClockCan1,
};

static FPmClockModel pmClockGem0Ref = {
	.id = PM_CLOCK_GEM0_REF,
	.mux = &gem0RefMux,
	.type = 0U,
	.next = &pmClockGemTsu,
};

static FPmClockModel pmClockGem1Ref = {
	.id = PM_CLOCK_GEM1_REF,
	.mux = &gem1RefMux,
	.type = 0U,
	.next = &pmClockGem0Ref,
};

static FPmClockModel pmClockGem2Ref = {
	.id = PM_CLOCK_GEM2_REF,
	.mux = &gem2RefMux,
	.type = 0U,
	.next = &pmClockGem1Ref,
};

static FPmClockModel pmClockGem3Ref = {
	.id = PM_CLOCK_GEM3_REF,
	.mux = &gem3RefMux,
	.type = 0U,
	.next = &pmClockGem2Ref,
};

static FPmClockModel pmClockFpdWdt = {
	.id = PM_CLOCK_WDT,
	.mux = &fpdWdtMux,
	.type = 0U,
	.next = &pmClockGem3Ref,
};

static const FPmClockModel* const head = &pmClockFpdWdt;

/****************************************************************************/
/**
 * @brief  Get clock structure by clock ID
 *
 * @param  id ID of the target clock
 *
 * @return Returns pointer to the found clock or NULL
 *
 * @note   None
 *
 ****************************************************************************/
static const FPmClockModel* FPm_GetClockById(const enum FPmClock id)
{
	const FPmClockModel* clk = head;

	while (clk != NULL) {
		if (clk->id == id) {
			break;
		}
		clk = clk->next;
	}

	return clk;
}

/****************************************************************************/
/**
 * @brief  Get parent clock ID for a given clock ID and mux select value
 *
 * @param  clockId ID of the target clock
 * @param  select Mux select value
 * @param  parentId Location to store parent clock ID
 *
 * @return Returns FPMST_SUCCESS if parent clock ID is found, FPMST_INVALID_PARAM
 * otherwise.
 *
 * @note   None
 *
 ****************************************************************************/
FPmStatus FPm_GetClockParentBySelect(const enum FPmClock clockId,
				   const u32 select,
				   enum FPmClock* const parentId)
{
	const FPmClockModel* const clk = FPm_GetClockById(clockId);
	FPmStatus status = FPMST_INVALID_PARAM;
	u32 i;

	if ((NULL == clk) || (NULL == clk->mux)) {
		goto done;
	}

	if (NULL == clk->mux->inputs) {
		/* MIO mux */
		if (select <= 0x4DU) {
			*parentId = PM_CLOCK_EXT_MIO0;
			*parentId += select;
			status = FPMST_SUCCESS;
		}
		/* else select parameter is invalid (out of scope) */
		goto done;
	}

	for (i = 0U; i < clk->mux->size; i++) {
		if (clk->mux->inputs[i].select == select) {
			*parentId = clk->mux->inputs[i].clkIn;
			status = FPMST_SUCCESS;
			break;
		}
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Get mux select value for given clock and clock parent IDs
 *
 * @param  clockId ID of the target clock
 * @param  parentId ID of the parent clock
 * @param  select Location to store mux select value
 *
 * @return Returns FPMST_SUCCESS if select value is found, FPMST_INVALID_PARAM
 * otherwise.
 *
 * @note   None
 *
 ****************************************************************************/
FPmStatus FPm_GetSelectByClockParent(const enum FPmClock clockId,
				   const enum FPmClock parentId,
				   u32* const select)
{
	const FPmClockModel* const clk = FPm_GetClockById(clockId);
	FPmStatus status = FPMST_INVALID_PARAM;
	u32 i;

	if ((NULL == clk) || (NULL == clk->mux)) {
		goto done;
	}

	if (NULL == clk->mux->inputs) {
		/* MIO mux */
		u32 mioSel = parentId - PM_CLOCK_EXT_MIO0;
		if (mioSel <= 0x4DU) {
			*select = mioSel;
			status = FPMST_SUCCESS;
		}
		/* else parentId parameter is invalid (out of scope) */
		goto done;
	}

	for (i = 0U; i < clk->mux->size; i++) {
		if (clk->mux->inputs[i].clkIn == parentId) {
			*select = clk->mux->inputs[i].select;
			status = FPMST_SUCCESS;
			break;
		}
	}

done:
	return status;
}

/****************************************************************************/
/**
 * @brief  Get number of divider that a given clock has
 *
 * @param  clock ID of the target clock
 *
 * @return Encoded clock divider types. If the clock ID is invalid zero is
 * returned.
 *
 * @note   None
 *
 ****************************************************************************/
u8 FPm_GetClockDivType(const enum FPmClock clock)
{
	const FPmClockModel* const clk = FPm_GetClockById(clock);
	u8 divs = 0U;

	if (NULL == clk) {
		goto done;
	}
	divs = clk->type;

done:
	return divs;
}

/****************************************************************************/
/**
 * @brief  Map effective divider value for given clock on DIV0 and DIV1 dividers
 *
 * @param  clock ID of the target clock
 * @param  div Effective divider value
 * @param  div0 Location to store mapped DIV0 value
 * @param  div1 Location to store mapped DIV1 value
 *
 * @return Encoded mask of mapped dividers
 *
 * @note   The effective divider value may not be mappable on 2x 6-bit wide
 * dividers. This is the case if a given divider value is higher than 6-bit
 * divider (requires 2xdividers), but its a prime number (cannot be divided
 * to get 2x divider values).
 *
 ****************************************************************************/
u8 FPm_MapDivider(const enum FPmClock clock,
		       const u32 div,
		       u32* const div0,
		       u32* const div1)
{
	const FPmClockModel* const clk = FPm_GetClockById(clock);
	u32 d0, d1 = 0U;
	u8 mapped = 0U;

	if ((NULL == clk) || (NULL == div0) || (NULL == div1)) {
		goto done;
	}

	/* Check if clock has no divider */
	if (!PM_CLOCK_HAS_DIV0(clk) && !PM_CLOCK_HAS_DIV1(clk)) {
		goto done;
	}

	/* Check if given div value is out of range */
	if (((!PM_CLOCK_HAS_DIV1(clk)) && (div > PM_DIV_WIDTH)) ||
	    (div > PM_2xDIV_WIDTH)) {
		goto done;
	}

	/* Check if divider fits in Div0 only */
	if (div <= PM_DIV_WIDTH) {
		*div0 = div;
		mapped = PM_CLOCK_TYPE_DIV0;
		if (PM_CLOCK_HAS_DIV1(clk)) {
			*div1 = 1U;
			mapped |= PM_CLOCK_TYPE_DIV1;
		}
		goto done;
	}
	/* Divider has to be configured using both DIV0 and DIV1 */
	for (d0 = 2U; d0 <= ((PM_DIV_WIDTH/2U) + 1U); d0++) {
		if (0U == (div % d0)) {
			d1 = div / d0;
			break;
		}
	}
	/* Check if div is prime number > width (d1 would not be assigned) */
	if (0U == d1) {
		goto done;
	}

	*div0 = d0;
	*div1 = d1;
	mapped = PM_CLOCK_TYPE_DIV0 | PM_CLOCK_TYPE_DIV1;

done:
	return mapped;
}
 /** @} */
