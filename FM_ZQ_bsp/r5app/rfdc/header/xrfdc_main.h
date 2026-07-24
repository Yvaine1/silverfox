/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 * @file    xrfdc_main.h
 * @brief   This file porvide RFDC shell command access.
 */

#ifndef _XRFDC_MAIN_H_
#define _XRFDC_MAIN_H_

/* Includes ------------------------------------------------------------------*/
#include "dg_common.h"
#include "xrfdc.h"
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define RFDC_NAME_INIT "init"
#define RFDC_INFO_INIT "Initialize RFDC device"

#define RFDC_NAME_RESET "reset"
#define RFDC_INFO_RESET "reset RFDC device"

#define RFDC_NAME_GET_PLL_CONFIG "getpllConfig"
#define RFDC_INFO_GET_PLL_CONFIG "get pll config"

#define RFDC_NAME_SET_PLL_CONFIG "setpllConfig"
#define RFDC_INFO_SET_PLL_CONFIG "set pll config"

#define RFDC_NAME_PLL_LOCK_STATUS "getpllLockStatus"
#define RFDC_INFO_PLL_LOCK_STATUS "get pll lock status"

#define RFDC_NAME_GET_TILE_STATUS "getTileStatus"
#define RFDC_INFO_GET_TILE_STATUS "get tile status"

#define RFDC_NAME_GET_BLOCK_STATUS "getBlockStatus"
#define RFDC_INFO_GET_BLOCK_STATUS "get block status"

#define RFDC_NAME_GET_MIXER_SETTINGS "getMixerSettings"
#define RFDC_INFO_GET_MIXER_SETTINGS "get Mixer Settings"
#define RFDC_NAME_SET_MIXER_SETTINGS "setMixerSettings"
#define RFDC_INFO_SET_MIXER_SETTINGS "set Mixer Settings"

#define RFDC_NAME_GET_QMC_SETTINGS "getQMCSettings"
#define RFDC_INFO_GET_QMC_SETTINGS "get QMC Settings"
#define RFDC_NAME_SET_QMC_SETTINGS "setQMCSettings"
#define RFDC_INFO_SET_QMC_SETTINGS "set QMC Settings"

#define RFDC_NAME_GET_NYQUIST_ZONE "getNyquistZone"
#define RFDC_INFO_GET_NYQUIST_ZONE "get Nyquist Zone"
#define RFDC_NAME_SET_NYQUIST_ZONE "setNyquistZone"
#define RFDC_INFO_SET_NYQUIST_ZONE "set Nyquist Zone"

#define RFDC_NAME_GET_POWER_MODE "getPowerMode"
#define RFDC_INFO_GET_POWER_MODE "get Power Mode"
#define RFDC_NAME_SET_POWER_MODE "setPowerMode"
#define RFDC_INFO_SET_POWER_MODE "set Power Mode"

#define RFDC_NAME_GET_INTERRUPT_STATUS "getIntrStatus"
#define RFDC_INFO_GET_INTERRUPT_STATUS "get Interrupt Status Mask"

#define RFDC_NAME_GET_DSA_SETTINGS "getDSA"
#define RFDC_INFO_GET_DSA_SETTINGS "get DSA"
#define RFDC_NAME_SET_DSA_SETTINGS "setDSA"
#define RFDC_INFO_SET_DSA_SETTINGS "set DSA"

#define RFDC_NAME_GET_SIGNAL_DETECTOR "getSignalDetector"
#define RFDC_INFO_GET_SIGNAL_DETECTOR "get Signal Detector"
#define RFDC_NAME_SET_SIGNAL_DETECTOR "setSignalDetector"
#define RFDC_INFO_SET_SIGNAL_DETECTOR "set Signal Detector"

#define RFDC_NAME_GET_CALIBRATION_MODE "getCalibrationMode"
#define RFDC_INFO_GET_CALIBRATION_MODE "get Calibration Mode"
#define RFDC_NAME_SET_CALIBRATION_MODE "setCalibrationMode"
#define RFDC_INFO_SET_CALIBRATION_MODE "set Calibration Mode"

#define RFDC_NAME_GET_LINKCOUPLING "getLinkCoupling"
#define RFDC_INFO_GET_LINKCOUPLING "get Link Coupling"

#define RFDC_NAME_GET_DITHER "getDither"
#define RFDC_INFO_GET_DITHER "get Dither"
#define RFDC_NAME_SET_DITHER "setDither"
#define RFDC_INFO_SET_DITHER "set Dither"

#define RFDC_NAME_GET_DECIMATION_FACTOR "getDecimationFactor"
#define RFDC_INFO_GET_DECIMATION_FACTOR "get Decimation Factor"
#define RFDC_NAME_SET_DECIMATION_FACTOR "setDecimationFactor"
#define RFDC_INFO_SET_DECIMATION_FACTOR "set Decimation Factor"

#define RFDC_NAME_GET_THRESHOLD_SETTINGS "getThresholdSettings"
#define RFDC_INFO_GET_THRESHOLD_SETTINGS "get Threshold Settings"
#define RFDC_NAME_SET_THRESHOLD_SETTINGS "setThresholdSettings"
#define RFDC_INFO_SET_THRESHOLD_SETTINGS "set Threshold Settings"
#define RFDC_NAME_SET_THRESHOLD_CLR_MODE "setThresholdClrMode"
#define RFDC_INFO_SET_THRESHOLD_CLR_MODE "set threshold clear mode"

#define RFDC_NAME_GET_DECODER_MODE "getDecoderMode"
#define RFDC_INFO_GET_DECODER_MODE "get Decoder Mode"
#define RFDC_NAME_SET_DECODER_MODE "setDecoderMode"
#define RFDC_INFO_SET_DECODER_MODE "set Decoder Mode"

#define RFDC_NAME_GET_DATAPATH_MODE "getDataPathMode"
#define RFDC_INFO_GET_DATAPATH_MODE "get Data Path Mode"
#define RFDC_NAME_SET_DATAPATH_MODE "setDataPathMode"
#define RFDC_INFO_SET_DATAPATH_MODE "set Data Path Mode"

#define RFDC_NAME_GET_IMR_PASS_MODE "getIMRPassMode"
#define RFDC_INFO_GET_IMR_PASS_MODE "get IMR filter Mode"
#define RFDC_NAME_SET_IMR_PASS_MODE "setIMRPassMode"
#define RFDC_INFO_SET_IMR_PASS_MODE "set IMR filter Mode"

#define RFDC_NAME_GET_OUTPUTCURRENT "getOutputCurrent"
#define RFDC_INFO_GET_OUTPUTCURRENT "get Output Current"
#define RFDC_NAME_SET_DAC_VOP "setDACVOP"
#define RFDC_INFO_SET_DAC_VOP "set DAC Variable Output Power"

#define RFDC_NAME_GET_INVERSE_SINC_FIR "getInverseSincFIR"
#define RFDC_INFO_GET_INVERSE_SINC_FIR "get Inverse Sinc Filter"
#define RFDC_NAME_SET_INVERSE_SINC_FIR "setInverseSincFIR"
#define RFDC_INFO_SET_INVERSE_SINC_FIR "set Inverse Sinc Filter"

#define RFDC_NAME_GET_INTERPOLATION_FACTOR "getInterpolationFactor"
#define RFDC_INFO_GET_INTERPOLATION_FACTOR "get Interpolation Factor"
#define RFDC_NAME_SET_INTERPOLATION_FACTOR "setInterpolationFactor"
#define RFDC_INFO_SET_INTERPOLATION_FACTOR "set Interpolation Factor"

#define RFDC_NAME_MTS "MultiTileSync"
#define RFDC_INFO_MTS "Multi-Tile Synchronization Init and Config"

#define RFDC_NAME_READ_REG "ReadRegs"
#define RFDC_INFO_READ_REG "Read Registers"

#define RFDC_NAME_SHUTDOWN "shutdown"
#define RFDC_INFO_SHUTDOWN "stops the tile"

#define RFDC_NAME_STARTUP "startup"
#define RFDC_INFO_STARTUP "start up the tile"

#define RFDC_NAME_SETCLKDISTRIBUTION "SetClkDistribution"
#define RFDC_INFO_SETCLKDISTRIBUTION "Set Clk Distribution"

#define RFDC_NAME_GETCLKDISTRIBUTION "GetClkDistribution"
#define RFDC_INFO_GETCLKDISTRIBUTION "Get Clk Distribution"

/* Exported functions --------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

void rfdc_init(UINT16 device_id);
void rfdc_reset(UINT32 type, INT32 tile_id);
void rfdc_getpllConfig(UINT32 type, INT32 tile_id);
void rfdc_setpllConfig(UINT32 type, UINT32 tile_id, UINT8 clkSource, double refClkFreq,
                              double samplingRate);
void rfdc_getpllLockStatus(UINT32 type, INT32 tile_id);
void rfdc_getTileStatus(UINT32 type, INT32 tile_id);
void rfdc_getBlockStatus(UINT32 type, INT32 tile_id, UINT32 block_id);
void rfdc_getMixerSettings(UINT32 type, UINT32 tile_id, UINT32 block_id);
void rfdc_setMixerSettings(UINT32 type, UINT32 tile_id, UINT32 block_id, 
    double Freq, double PhaseOffset, UINT32 EventSource, 
    UINT32 CoarseMixFreq, UINT32 MixerMode, UINT32 FineMixerScale, UINT32 MixerType);
void rfdc_getQMCSettings(UINT32 type, INT32 tile_id, UINT32 block_id);
void rfdc_setQMCSettings(UINT32 type, UINT32 tile_id, UINT32 block_id, 
    UINT32 EnablePhase, UINT32 EnableGain, double GainCorrectionFactor, 
    double PhaseCorrectionFactor, INT32 OffsetCorrectionFactor, UINT32 EventSource);
void rfdc_getNyquistZone(UINT32 type, INT32 tile_id, UINT32 block_id);
void rfdc_setNyquistZone(UINT32 type, UINT32 tile_id ,UINT32 block_id, UINT32 nyquistZone);
void rfdc_getPwrMode(UINT32 type, INT32 tile_id, UINT32 block_id);
void rfdc_setPwrMode(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 DisableIPControl, UINT32 PwrMode);
void rfdc_getIntrStatus(UINT32 type, INT32 tile_id, UINT32 block_id);
void rfdc_shutdown(UINT32 type, INT32 tile_id);
void rfdc_startup(UINT32 type, INT32 tile_id);
void rfdc_getclkdistribution(void);
void rfdc_setclkdistribution(UINT32 row, UINT32 col, double ClkFreq, double samplerate);

////////////only for ADC///////////////////////////
void rfdc_getDSA(UINT32 tile_id, UINT32 block_id);
void rfdc_setDSA(UINT32 tile_id,UINT32 block_id, UINT32 DisableRTS, float Attenuation);
void rfdc_getSignalDetector(UINT32 tile_id, UINT32 block_id);
void rfdc_setSignalDetector(UINT32 tile_id, UINT32 block_id, UINT32 Mode, 
    UINT32 TimeConstant, UINT32 Flush, UINT32 EnableIntegrator, 
    UINT32 Threshold, UINT32 ThreshOnTriggerCnt, UINT32 ThreshOffTriggerCnt, 
    UINT32 HysteresisEnable);
void rfdc_getCalibrationMode(UINT32 tile_id, UINT32 block_id);
void rfdc_setCalibrationMode(UINT32 tile_id, UINT32 block_id, UINT8 CalibrationMode);
void rfdc_getLinkCoupling(UINT32 type, INT32 block_id);
void rfdc_getDither(UINT32 type, INT32 block_id);
void rfdc_getDecimationFactor(UINT32 type, INT32 tile_id);
void rfdc_setDither(UINT32 type, INT32 block_id, UINT32 Dither);
void rfdc_setDecimationFactor(UINT32 tile_id, UINT32 block_id, UINT32 DecimationFactor);
void rfdc_getThresholdSettings(UINT32 type, INT32 block_id);
void rfdc_setThresholdSettings(UINT32 tile_id, UINT32 block_id, XRFdc_Threshold_Settings threshold_settings);
void rfdc_setThresholdClrMode(UINT32 tile_id, UINT32 block_id, UINT32 ThresholdToUpdate, UINT32 ClrMode);

////////////only for DAC///////////////////////////
void rfdc_getDecoderMode(UINT32 tile_id, UINT32 block_id);
void rfdc_setDecoderMode(UINT32 tile_id, UINT32 block_id, UINT32 DecoderMode);
void rfdc_getDataPathMode(UINT32 tile_id, UINT32 block_id);
void rfdc_setDataPathMode(UINT32 tile_id, UINT32 block_id, UINT32 DataPathMode);
void rfdc_getIMRPassMode(UINT32 tile_id, UINT32 block_id);
void rfdc_setIMRPassMode(UINT32 tile_id, UINT32 block_id, UINT32 IMRPassMode);
void rfdc_getOutputCurrent(INT32 tile_id, UINT32 block_id);
void rfdc_setDACVOP(INT32 tile_id, UINT32 block_id, UINT32 uACurrent);
void rfdc_getInverseSincFIR(INT32 tile_id, UINT32 block_id);
void rfdc_setInverseSincFIR(UINT32 tile_id, UINT32 block_id, UINT16 InverseSincFIR);
void rfdc_getInterpolationFactor(INT32 tile_id, UINT32 block_id);
void rfdc_setInterpolationFactor(INT32 tile_id, UINT32 block_id, UINT32 InterpolationFactor);

////////////for MTS////////////////////////////////
void rfdc_MultiTileSync(UINT32 type, UINT32 RefTile, UINT32 TilesBitMask, INT32 TargetLatency);

////////////only for debug/////////////////////////
void rfdc_readRegs(UINT32 BaseAddr, UINT32 RegAddr, UINT32 Mask);
void metal_rfdc_writeRegs(UINT32 Offset, UINT32 Data);
void metal_rfdc_readRegs(UINT32 Offset);
#ifdef __cplusplus
}
#endif

#endif /*_XRFDC_MAIN_H_*/
