/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */
#ifndef __XILINX_RFDC_API_H_
#define __XILINX_RFDC_API_H_

#include "dg_common.h"
#include "xrfdc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mw_rfdc_pll_settings {
    UINT32 Enabled; /* PLL Enables status (not a setter) */
    double RefClkFreq;
    double SampleRate;
    UINT32 RefClkDivider;
    UINT32 FeedbackDivider;
    UINT32 OutputDivider;
} mw_rfdc_pll_settings;

/***********************************************************************************/
/****   All follow wrapper-funcs are used to define common interfaces for RFDC   ***/
/***********************************************************************************/
extern STATUS mw_rfdc_init(UINT16 rfdc_device_id);
extern STATUS mw_rfdc_interrupt_init(void *callback_ref, void *intr_handler);
extern STATUS mw_rfdc_reset(UINT32 type, INT32 tile_id);
extern STATUS mw_rfdc_restart(UINT32 type, INT32 tile_id);

extern STATUS mw_rfdc_get_pll_config(UINT32 type, UINT32 tile_id, mw_rfdc_pll_settings *pll_config);
extern STATUS mw_rfdc_set_pll_config(UINT32 type, UINT32 tile_id, UINT8 clkSource, double refClkFreq, double samplingRate);
extern STATUS mw_rfdc_get_pll_lock_status(UINT32 type, UINT32 tile_id, UINT32 *lock_status);

extern STATUS mw_rfdc_get_clock_source(UINT32 type, UINT32 tile_id, UINT32 *clock_source);
extern STATUS mw_rfdc_get_MaxSampleRate(UINT32 type, UINT32 tile_id, double *max_sample_rate);
extern STATUS mw_rfdc_get_MinSampleRate(UINT32 type, UINT32 tile_id, double *min_sample_rate);
extern STATUS mw_rfdc_get_tile_status(UINT32 type, UINT32 tile_id, XRFdc_TileStatus *tile_status);
extern STATUS mw_rfdc_get_block_status(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_BlockStatus *block_status);

extern STATUS mw_rfdc_get_mixer_settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Mixer_Settings *mixer_settings);
extern STATUS mw_rfdc_set_mixer_settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Mixer_Settings *mixer_settings);
extern STATUS mw_rfdc_get_QMC_Settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_QMC_Settings *qmc_settings);
extern STATUS mw_rfdc_set_QMC_Settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_QMC_Settings *qmc_settings);
extern STATUS mw_rfdc_get_Nyquist_Zone(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 *nyquist_zone);
extern STATUS mw_rfdc_set_Nyquist_Zone(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 nyquist_zone);
extern STATUS mw_rfdc_get_PwrMode(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Pwr_Mode_Settings *pwr_mode);
extern STATUS mw_rfdc_set_PwrMode(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Pwr_Mode_Settings *pwr_mode);
extern STATUS mw_rfdc_get_ConnectedIData(UINT32 type, UINT32 tile_id, UINT32 block_id, int *connected_idata);
extern STATUS mw_rfdc_get_ConnectedQData(UINT32 type, UINT32 tile_id, UINT32 block_id, int *connected_qdata);
extern STATUS mw_rfdc_IntrEnable(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 intr_mask);
extern STATUS mw_rfdc_IntrDisable(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 intr_mask);
extern STATUS mw_rfdc_get_IntrStatus(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 *intr_status);
extern STATUS mw_rfdc_shutdown(UINT32 type, INT32 tile_id);
extern STATUS mw_rfdc_getclkdistribution(XRFdc_Distribution_System_Settings *DistributionSettingsPtr);
extern STATUS mw_rfdc_setclkdistribution(UINT32 Distribution_index,double ClkFreq, double samplerate,UINT32 row,UINT32 col);

// only for ADC
extern STATUS mw_rfdc_get_DSA(UINT32 tile_id, UINT32 block_id, XRFdc_DSA_Settings *dsa_settings);
extern STATUS mw_rfdc_set_DSA(UINT32 tile_id, UINT32 block_id, XRFdc_DSA_Settings *dsa_settings);
extern STATUS mw_rfdc_get_SignalDetector(UINT32 tile_id, UINT32 block_id, XRFdc_Signal_Detector_Settings *signal_detector);
extern STATUS mw_rfdc_set_SignalDetector(UINT32 tile_id, UINT32 block_id, XRFdc_Signal_Detector_Settings *signal_detector);
extern STATUS mw_rfdc_get_CalibrationMode(UINT32 tile_id, UINT32 block_id, UINT8 *calibration_mode);
extern STATUS mw_rfdc_set_CalibrationMode(UINT32 tile_id, UINT32 block_id, UINT8 calibration_mode);
extern STATUS mw_rfdc_get_LinkCoupling(UINT32 tile_id, UINT32 block_id, UINT32 *mode);
extern STATUS mw_rfdc_get_Dither(UINT32 tile_id, UINT32 block_id, UINT32 *mode);
extern STATUS mw_rfdc_set_Dither(UINT32 tile_id, UINT32 block_id, UINT32 mode);
extern STATUS mw_rfdc_get_DecimationFactor(UINT32 tile_id, UINT32 block_id, UINT32 *decimation_factor);
extern STATUS mw_rfdc_set_DecimationFactor(UINT32 tile_id, UINT32 block_id, UINT32 decimation_factor);
extern STATUS mw_rfdc_get_CalCoefficients(UINT32 tile_id, UINT32 block_id, UINT32 CalibrationBlock, XRFdc_Calibration_Coefficients *coeff);
extern STATUS mw_rfdc_set_CalCoefficients(UINT32 tile_id, UINT32 block_id, UINT32 CalibrationBlock, XRFdc_Calibration_Coefficients *coeff);
extern STATUS mw_rfdc_get_ThresholdSettings(UINT32 tile_id, UINT32 block_id, XRFdc_Threshold_Settings *threshold_settings);
extern STATUS mw_rfdc_set_ThresholdSettings(UINT32 tile_id, UINT32 block_id, XRFdc_Threshold_Settings *threshold_settings);
extern STATUS mw_rfdc_set_ThresholdClrMode(UINT32 tile_id, UINT32 block_id, UINT32 thresholdToUpdate, UINT32 ClrMode);
extern STATUS mw_rfdc_ThresholdStickyClear(UINT32 tile_id, UINT32 block_id, UINT32 thresholdToUpdate);

// only for DAC
extern STATUS mw_rfdc_get_DecoderMode(UINT32 tile_id, UINT32 block_id, UINT32 *decoder_mode);
extern STATUS mw_rfdc_set_DecoderMode(UINT32 tile_id, UINT32 block_id, UINT32 decoder_mode);
extern STATUS mw_rfdc_get_DataPathMode(UINT32 tile_id, UINT32 block_id, UINT32 *datapath_mode);
extern STATUS mw_rfdc_set_DataPathMode(UINT32 tile_id, UINT32 block_id, UINT32 datapath_mode);
extern STATUS mw_rfdc_get_IMRPassMode(UINT32 tile_id, UINT32 block_id, UINT32 *imr_pass_mode);
extern STATUS mw_rfdc_set_IMRPassMode(UINT32 tile_id, UINT32 block_id, UINT32 imr_pass_mode);
extern STATUS mw_rfdc_get_OutputCurrent(UINT32 tile_id, UINT32 block_id, UINT32 *output_current);
extern STATUS mw_rfdc_set_DACVOP(UINT32 tile_id, UINT32 block_id, UINT32 uACurrent);
extern STATUS mw_rfdc_get_InverseSincFIR(UINT32 tile_id, UINT32 block_id, UINT16 *mode);
extern STATUS mw_rfdc_set_InverseSincFIR(UINT32 tile_id, UINT32 block_id, UINT16 mode);
extern STATUS mw_rfdc_get_InterpolationFactor(UINT32 tile_id, UINT32 block_id, UINT32 *interpolation_factor);
extern STATUS mw_rfdc_set_InterpolationFactor(UINT32 tile_id, UINT32 block_id, UINT32 interpolation_factor);

// for MTS
extern STATUS mw_rfdc_run_MTS(UINT32 type, XRFdc_MultiConverter_Sync_Config *ConfigPtr, INT32 *PLL_CodesPtr, INT32 *T1_CodesPtr);
extern STATUS mw_rfdc_MTS_Sysref_Config(XRFdc_MultiConverter_Sync_Config *DACSyncConfigPtr, XRFdc_MultiConverter_Sync_Config *ADCSyncConfigPtr, UINT32 SysRefEnable);
extern STATUS mw_rfdc_get_MTSEnable(UINT32 type, UINT32 tile_id, UINT32 *enable);

// for debug
extern STATUS mw_rfdc_read_Reg(UINT32 BaseAddr, UINT32 RegAddr, UINT16 Mask, UINT16 *read_reg);

typedef enum
{
  DG_SHM_XRFDC_ID_INIT = 65
} DG_SHM_XRFDC_ID_MAP;

/* RFdc driver instance */
XRFdc *getRFdcInstance();
INT32 check_type(UINT32 type);
INT32 check_tile_id(UINT32 tile_id);
INT32 check_block_id(UINT32 block_id);

#ifdef __cplusplus
}
#endif

#endif /* __XILINX_RFDC_API_H_ */
