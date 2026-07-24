/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */

#include "xrfdc_api.h"
#include <metal/irq.h>

static XRFdc RFdcInst;      /* RFdc driver instance */

XRFdc *getRFdcInstance()
{
    return &RFdcInst;
}

INT32 check_type(UINT32 type)
{
    return ((type == XRFDC_ADC_TILE) || (type == XRFDC_DAC_TILE)) ? 1 : 0;
}

INT32 check_tile_id(UINT32 tile_id)
{
    return ((tile_id >=0) && (tile_id <= XRFDC_TILE_ID_MAX)) ? 1 : 0;
}

INT32 check_block_id(UINT32 block_id)
{
    return ((block_id >=0) && (block_id <= XRFDC_BLOCK_ID_MAX)) ? 1 : 0;
}


STATUS mw_rfdc_init(UINT16 rfdc_device_id)
{
    UINT32 Status;
    XRFdc_Config *ConfigPtr = NULL;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    /* Initialize the RFdc driver. */
    ConfigPtr = XRFdc_LookupConfig(rfdc_device_id);
    if (ConfigPtr == NULL) {
        return (STATUS)XRFDC_FAILURE;
    }

    /* Initializes the controller */
    Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
    if (Status != XRFDC_SUCCESS) {
        return (STATUS)XRFDC_FAILURE;
    }
    return (STATUS)XRFDC_SUCCESS;
}

STATUS mw_rfdc_interrupt_init(void *callback_ref, void *intr_handler)
{
    XRFdc *RFdcInstPtr = getRFdcInstance();

   /*
    * Setup the handler for the RFdc that will be called from the
    * interrupt context when an RFdc interrupt occurs, specify a pointer to
    * the RFdc driver instance as the callback reference so the handler is
    * able to access the instance data
    */
    XRFdc_SetStatusHandler(RFdcInstPtr, callback_ref, (XRFdc_StatusHandler)intr_handler);
    /* Get interrupt ID from RFDC metal device */
    int irq = (intptr_t)RFdcInstPtr->device->irq_info;
    if (irq < 0) {
        fmsh_print("ERROR: Failed to request interrupt for %s.\r\n", RFdcInstPtr->device->name);
        return XRFDC_FAILURE;
    }

    if (metal_irq_register(irq, (metal_irq_handler)XRFdc_IntrHandler, RFdcInstPtr) != 0)
    {
        fmsh_print("Failed to register interrupt handler. irq = %d\r\n", irq);
        return XRFDC_FAILURE;
    } else {
        fmsh_print("registered IPI interrupt.\r\n");
    }

    metal_irq_enable(irq);
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_reset(UINT32 type, INT32 tile_id)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    // tile_id: -1, all tile; [0~3], tile0 ~ tile3
    Status = XRFdc_Reset(RFdcInstPtr, type, tile_id);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_shutdown(UINT32 type, INT32 tile_id)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    // tile_id: -1, all tile; [0~3], tile0 ~ tile3
    Status = XRFdc_Shutdown(RFdcInstPtr, type, tile_id);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_restart(UINT32 type, INT32 tile_id)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    // tile_id: -1, all tile; [0~3], tile0 ~ tile3
    Status = XRFdc_StartUp(RFdcInstPtr, type, tile_id);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_setclkdistribution(UINT32 Distribution_index,double ClkFreq, double samplerate,UINT32 row,UINT32 col)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_Distribution_Settings DistributionSettingsPtr;
    XRFdc_Distribution_System_Settings DistributionSystemSettingsPtr;

    Status = XRFdc_GetClkDistribution(RFdcInstPtr,&DistributionSystemSettingsPtr);
    Distribution_index %= 8;
    memcpy(&DistributionSettingsPtr,&DistributionSystemSettingsPtr.Distributions[Distribution_index],sizeof(XRFdc_Distribution_Settings));
    DistributionSettingsPtr.DistRefClkFreq = ClkFreq;
    DistributionSettingsPtr.SampleRates[row][col] = samplerate;
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("GetClkDistribution failed\r\n");
        return XRFDC_FAILURE;
    }
    
    Status = XRFdc_SetClkDistribution(RFdcInstPtr, &DistributionSettingsPtr);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("SetClkDistribution failed\r\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_getclkdistribution(XRFdc_Distribution_System_Settings *DistributionSettingsPtr)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    Status = XRFdc_GetClkDistribution(RFdcInstPtr,DistributionSettingsPtr);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("GetClkDistribution failed\r\n");
        return XRFDC_FAILURE;
    }

    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_pll_config(UINT32 type, UINT32 tile_id, mw_rfdc_pll_settings *pll_config)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_PLL_Settings PLLSettings;
    memset(&PLLSettings, 0, sizeof(XRFdc_PLL_Settings));
    Status = XRFdc_GetPLLConfig(RFdcInstPtr, type, tile_id, &PLLSettings);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    pll_config->Enabled = PLLSettings.Enabled;
    pll_config->RefClkFreq = PLLSettings.RefClkFreq;
    pll_config->SampleRate = PLLSettings.SampleRate;
    pll_config->RefClkDivider = PLLSettings.RefClkDivider;
    pll_config->FeedbackDivider = PLLSettings.FeedbackDivider;
    pll_config->OutputDivider = PLLSettings.OutputDivider;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_pll_config(UINT32 type, UINT32 tile_id, UINT8 clkSource, double refClkFreq, double samplingRate)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    Status = XRFdc_DynamicPLLConfig(RFdcInstPtr, type, tile_id, clkSource, refClkFreq, samplingRate);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_pll_lock_status(UINT32 type, UINT32 tile_id, UINT32 *lock_status)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    Status = XRFdc_GetPLLLockStatus(RFdcInstPtr, type, tile_id, lock_status);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_clock_source(UINT32 type, UINT32 tile_id, UINT32 *clock_source)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 ClockSource;
    Status = XRFdc_GetClockSource(RFdcInstPtr, type, tile_id, &ClockSource);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    *clock_source = ClockSource;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_MaxSampleRate(UINT32 type, UINT32 tile_id, double *max_sample_rate)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    double MaxSampleRate;

    Status = XRFdc_GetMaxSampleRate(RFdcInstPtr, type, tile_id, &MaxSampleRate);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    *max_sample_rate = MaxSampleRate;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_MinSampleRate(UINT32 type, UINT32 tile_id, double *min_sample_rate)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    double MinSampleRate;

    Status = XRFdc_GetMinSampleRate(RFdcInstPtr, type, tile_id, &MinSampleRate);
    if (Status != XRFDC_SUCCESS) {
        return XRFDC_FAILURE;
    }
    *min_sample_rate = MinSampleRate;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_tile_status(UINT32 type, UINT32 tile_id, XRFdc_TileStatus *tile_status)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_IPStatus IPStatus;

    if (!tile_status)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (type != XRFDC_ADC_TILE && type != XRFDC_DAC_TILE)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&IPStatus, 0, sizeof(XRFdc_IPStatus));
    Status = XRFdc_GetIPStatus(RFdcInstPtr, &IPStatus);
    if (Status != XRFDC_SUCCESS)
    {
        fmsh_print("XRFdc_GetIPStatus failed!");
        return XRFDC_FAILURE;
    }
    if (type == XRFDC_ADC_TILE) // ADC
    {
        tile_status->IsEnabled = IPStatus.ADCTileStatus[tile_id].IsEnabled;
        tile_status->TileState = IPStatus.ADCTileStatus[tile_id].TileState;
        tile_status->BlockStatusMask = IPStatus.ADCTileStatus[tile_id].BlockStatusMask;
        tile_status->PowerUpState = IPStatus.ADCTileStatus[tile_id].PowerUpState;
        tile_status->PLLState = IPStatus.ADCTileStatus[tile_id].PLLState;
    }
    else if (type == XRFDC_DAC_TILE) // DAC
    {
        tile_status->IsEnabled = IPStatus.DACTileStatus[tile_id].IsEnabled;
        tile_status->TileState = IPStatus.DACTileStatus[tile_id].TileState;
        tile_status->BlockStatusMask = IPStatus.DACTileStatus[tile_id].BlockStatusMask;
        tile_status->PowerUpState = IPStatus.DACTileStatus[tile_id].PowerUpState;
        tile_status->PLLState = IPStatus.DACTileStatus[tile_id].PLLState;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_block_status(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_BlockStatus *block_status)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_BlockStatus BlockStatus;

    if (!block_status)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&BlockStatus, 0, sizeof(XRFdc_BlockStatus));
    Status = XRFdc_GetBlockStatus(RFdcInstPtr, type, tile_id, block_id, &BlockStatus);
    if (Status != XRFDC_SUCCESS)
    {
        fmsh_print("XRFdc_GetBlockStatus failed!\n");
        return XRFDC_FAILURE;
    }
    block_status->SamplingFreq = BlockStatus.SamplingFreq;
    block_status->AnalogDataPathStatus = BlockStatus.AnalogDataPathStatus;
    block_status->DigitalDataPathStatus = BlockStatus.DigitalDataPathStatus;
    block_status->DataPathClocksStatus = BlockStatus.DataPathClocksStatus;
    block_status->IsFIFOFlagsEnabled = BlockStatus.IsFIFOFlagsEnabled;
    block_status->IsFIFOFlagsAsserted = BlockStatus.IsFIFOFlagsAsserted;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_mixer_settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Mixer_Settings *mixer_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_Mixer_Settings MixerSettings;

    if (!mixer_settings)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&MixerSettings, 0, sizeof(XRFdc_Mixer_Settings));
    Status = XRFdc_GetMixerSettings(RFdcInstPtr, type, tile_id, block_id, &MixerSettings);
    if (Status != XRFDC_SUCCESS)
    {
        fmsh_print("XRFdc_GetMixerSettings failed!\n");
        return XRFDC_FAILURE;
    }
    mixer_settings->Freq = MixerSettings.Freq;
    mixer_settings->PhaseOffset = MixerSettings.PhaseOffset;
    mixer_settings->EventSource = MixerSettings.EventSource;
    mixer_settings->CoarseMixFreq = MixerSettings.CoarseMixFreq;
    mixer_settings->MixerMode = MixerSettings.MixerMode;
    mixer_settings->FineMixerScale = MixerSettings.FineMixerScale;
    mixer_settings->MixerType = MixerSettings.MixerType;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_mixer_settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Mixer_Settings *mixer_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (!mixer_settings)
    {
        //
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetMixerSettings(RFdcInstPtr, type, tile_id, block_id, mixer_settings);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetMixerSettings failed!\n");
        return XRFDC_FAILURE;
    }
    XRFdc_ResetNCOPhase(RFdcInstPtr, type, tile_id, block_id);
    XRFdc_UpdateEvent(RFdcInstPtr, type, tile_id, block_id, XRFDC_EVENT_MIXER);
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_QMC_Settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_QMC_Settings *qmc_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_QMC_Settings QMCSettings;

    if (!qmc_settings)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&QMCSettings, 0, sizeof(XRFdc_QMC_Settings));
    Status = XRFdc_GetQMCSettings(RFdcInstPtr, type, tile_id, block_id, &QMCSettings);
    if (Status != XRFDC_SUCCESS)
    {
        fmsh_print("XRFdc_GetQMCSettings failed!\n");
        return XRFDC_FAILURE;
    }
    qmc_settings->EnablePhase = QMCSettings.EnablePhase;
    qmc_settings->EnableGain = QMCSettings.EnableGain;
    qmc_settings->GainCorrectionFactor = QMCSettings.GainCorrectionFactor;
    qmc_settings->PhaseCorrectionFactor = QMCSettings.PhaseCorrectionFactor;
    qmc_settings->OffsetCorrectionFactor = QMCSettings.OffsetCorrectionFactor;
    qmc_settings->EventSource = QMCSettings.EventSource;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_QMC_Settings(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_QMC_Settings *qmc_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (!qmc_settings)
    {
        //
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetQMCSettings(RFdcInstPtr, type, tile_id, block_id, qmc_settings);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetQMCSettings failed!\n");
        return XRFDC_FAILURE;
    }
    XRFdc_UpdateEvent(RFdcInstPtr, type, tile_id, block_id, XRFDC_EVENT_QMC);
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_Nyquist_Zone(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 *nyquist_zone)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 NyquistZone;

    if (!nyquist_zone)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetNyquistZone(RFdcInstPtr, type, tile_id, block_id, &NyquistZone);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetNyquistZone failed!\n");
        return XRFDC_FAILURE;
    }
    *nyquist_zone = NyquistZone;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_Nyquist_Zone(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 nyquist_zone)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetNyquistZone(RFdcInstPtr, type, tile_id, block_id, nyquist_zone);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetNyquistZone failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_PwrMode(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Pwr_Mode_Settings *pwr_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_Pwr_Mode_Settings Settings;

    if (!pwr_mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&Settings, 0, sizeof(XRFdc_Pwr_Mode_Settings));
    Status = XRFdc_GetPwrMode(RFdcInstPtr, type, tile_id, block_id, &Settings);
    if (Status != XRFDC_SUCCESS)
    {
        fmsh_print("XRFdc_GetPwrMode failed!\n");
        return XRFDC_FAILURE;
    }
    memcpy(pwr_mode, &Settings, sizeof(XRFdc_Pwr_Mode_Settings));
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_PwrMode(UINT32 type, UINT32 tile_id, UINT32 block_id, XRFdc_Pwr_Mode_Settings *pwr_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (!pwr_mode)
    {
        //
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetPwrMode(RFdcInstPtr, type, tile_id, block_id, pwr_mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetPwrMode failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_getConnectedIData(UINT32 type, UINT32 tile_id, UINT32 block_id, int *connected_idata)
{
    int ConnectedIData = XRFDC_BLK_ID_NONE;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    ConnectedIData = XRFdc_GetConnectedIData(RFdcInstPtr, type, tile_id, block_id);
    if (ConnectedIData == XRFDC_BLK_ID_NONE) {
        fmsh_print("XRFdc_GetConnectedIData failed!\n");
        return XRFDC_FAILURE;
    }
    *connected_idata = ConnectedIData;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_getConnectedQData(UINT32 type, UINT32 tile_id, UINT32 block_id, int *connected_qdata)
{
    int ConnectedQData = XRFDC_BLK_ID_NONE;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    ConnectedQData = XRFdc_GetConnectedQData(RFdcInstPtr, type, tile_id, block_id);
    if (ConnectedQData == XRFDC_BLK_ID_NONE) {
        fmsh_print("XRFdc_GetConnectedQData failed!\n");
        return XRFDC_FAILURE;
    }
    *connected_qdata = ConnectedQData;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_IntrEnable(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 intr_mask)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_IntrEnable(RFdcInstPtr, type, tile_id, block_id, intr_mask);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_IntrEnable failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_IntrDisable(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 intr_mask)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_IntrDisable(RFdcInstPtr, type, tile_id, block_id, intr_mask);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_IntrDisable failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_IntrStatus(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 *intr_status)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 IntrStatus;

    if (!intr_status)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetIntrStatus(RFdcInstPtr, type, tile_id, block_id, &IntrStatus);
    if (Status != XRFDC_SUCCESS)
    {
        fmsh_print("XRFdc_GetIntrStatus failed!\n");
        return XRFDC_FAILURE;
    }
    *intr_status = IntrStatus;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_DSA(UINT32 tile_id, UINT32 block_id, XRFdc_DSA_Settings *dsa_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_DSA_Settings DSA_Settings;

    if (!dsa_settings)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&DSA_Settings, 0, sizeof(XRFdc_DSA_Settings));
    Status = XRFdc_GetDSA(RFdcInstPtr, tile_id, block_id, &DSA_Settings);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetDSA failed!\n");
        return XRFDC_FAILURE;
    }
    dsa_settings->DisableRTS = DSA_Settings.DisableRTS;
    dsa_settings->Attenuation = DSA_Settings.Attenuation;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_DSA(UINT32 tile_id, UINT32 block_id, XRFdc_DSA_Settings *dsa_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (!dsa_settings)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetDSA(RFdcInstPtr, tile_id, block_id, dsa_settings);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetDSA failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_SignalDetector(UINT32 tile_id, UINT32 block_id, XRFdc_Signal_Detector_Settings *signal_detector)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_Signal_Detector_Settings SignalDetector;

    if (!signal_detector)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&SignalDetector, 0, sizeof(XRFdc_Signal_Detector_Settings));
    Status = XRFdc_GetSignalDetector(RFdcInstPtr, tile_id, block_id, &SignalDetector);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetSignalDetector failed!\n");
        return XRFDC_FAILURE;
    }
    memcpy(signal_detector, &SignalDetector, sizeof(XRFdc_Signal_Detector_Settings));
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_SignalDetector(UINT32 tile_id, UINT32 block_id, XRFdc_Signal_Detector_Settings *signal_detector)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (!signal_detector)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetSignalDetector(RFdcInstPtr, tile_id, block_id, signal_detector);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetSignalDetector failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_CalibrationMode(UINT32 tile_id, UINT32 block_id, UINT8 *calibration_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT8 CalibrationMode;

    if (!calibration_mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetCalibrationMode(RFdcInstPtr, tile_id, block_id, &CalibrationMode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetCalibrationMode failed!\n");
        return XRFDC_FAILURE;
    }
    *calibration_mode = CalibrationMode;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_CalibrationMode(UINT32 tile_id, UINT32 block_id, UINT8 calibration_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetCalibrationMode(RFdcInstPtr, tile_id, block_id, calibration_mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetCalibrationMode failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_LinkCoupling(UINT32 tile_id, UINT32 block_id, UINT32 *mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 LinkCoupling;

    if (!mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetLinkCoupling(RFdcInstPtr, tile_id, block_id, &LinkCoupling);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetLinkCoupling failed!\n");
        return XRFDC_FAILURE;
    }
    *mode = LinkCoupling;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_Dither(UINT32 tile_id, UINT32 block_id, UINT32 *mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 Dither;

    if (!mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetDither(RFdcInstPtr, tile_id, block_id, &Dither);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetDither failed!\n");
        return XRFDC_FAILURE;
    }
    *mode = Dither;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_Dither(UINT32 tile_id, UINT32 block_id, UINT32 mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetDither(RFdcInstPtr, tile_id, block_id, mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetDither failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_DecimationFactor(UINT32 tile_id, UINT32 block_id, UINT32 *decimation_factor)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 DecimationFactor;

    if (!decimation_factor)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetDecimationFactor(RFdcInstPtr, tile_id, block_id, &DecimationFactor);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetDecimationFactor failed!\n");
        return XRFDC_FAILURE;
    }
    *decimation_factor = DecimationFactor;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_DecimationFactor(UINT32 tile_id, UINT32 block_id, UINT32 decimation_factor)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetDecimationFactor(RFdcInstPtr, tile_id, block_id, decimation_factor);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetDecimationFactor failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_CalCoefficients(UINT32 tile_id, UINT32 block_id, UINT32 CalibrationBlock, XRFdc_Calibration_Coefficients *coeff)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_Calibration_Coefficients Coeff;

    if (!coeff)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&Coeff, 0, sizeof(XRFdc_Calibration_Coefficients));
    Status = XRFdc_GetCalCoefficients(RFdcInstPtr, tile_id, block_id, CalibrationBlock, &Coeff);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetCalCoefficients failed!\n");
        return XRFDC_FAILURE;
    }
    memcpy(coeff, &Coeff, sizeof(XRFdc_Calibration_Coefficients));
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_CalCoefficients(UINT32 tile_id, UINT32 block_id, UINT32 CalibrationBlock, XRFdc_Calibration_Coefficients *coeff)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (!coeff)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetCalCoefficients(RFdcInstPtr, tile_id, block_id, CalibrationBlock, coeff);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetCalCoefficients failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_ThresholdSettings(UINT32 tile_id, UINT32 block_id, XRFdc_Threshold_Settings *threshold_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    XRFdc_Threshold_Settings ThresholdSettings;

    if (!threshold_settings)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    memset(&ThresholdSettings, 0, sizeof(XRFdc_Threshold_Settings));
    Status = XRFdc_GetThresholdSettings(RFdcInstPtr, tile_id, block_id, &ThresholdSettings);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetThresholdSettings failed!\n");
        return XRFDC_FAILURE;
    }
    memcpy(threshold_settings, &ThresholdSettings, sizeof(XRFdc_Threshold_Settings));
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_ThresholdSettings(UINT32 tile_id, UINT32 block_id, XRFdc_Threshold_Settings *threshold_settings)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (!threshold_settings)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetThresholdSettings(RFdcInstPtr, tile_id, block_id, threshold_settings);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetThresholdSettings failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_ThresholdClrMode(UINT32 tile_id, UINT32 block_id, UINT32 thresholdToUpdate, UINT32 clrMode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetThresholdClrMode(RFdcInstPtr, tile_id, block_id, thresholdToUpdate, clrMode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetThresholdClrMode failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_ThresholdStickyClear(UINT32 tile_id, UINT32 block_id, UINT32 thresholdToUpdate)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_ThresholdStickyClear(RFdcInstPtr, tile_id, block_id, thresholdToUpdate);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_ThresholdStickyClear failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_DecoderMode(UINT32 tile_id, UINT32 block_id, UINT32 *decoder_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 DecoderMode;

    if (!decoder_mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetDecoderMode(RFdcInstPtr, tile_id, block_id, &DecoderMode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetDecoderMode failed!\n");
        return XRFDC_FAILURE;
    }
    *decoder_mode = DecoderMode;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_DecoderMode(UINT32 tile_id, UINT32 block_id, UINT32 decoder_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetDecoderMode(RFdcInstPtr, tile_id, block_id, decoder_mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetDecoderMode failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_DataPathMode(UINT32 tile_id, UINT32 block_id, UINT32 *datapath_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 DataPathMode;

    if (!datapath_mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetDataPathMode(RFdcInstPtr, tile_id, block_id, &DataPathMode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetDataPathMode failed!\n");
        return XRFDC_FAILURE;
    }
    *datapath_mode = DataPathMode;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_DataPathMode(UINT32 tile_id, UINT32 block_id, UINT32 datapath_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetDataPathMode(RFdcInstPtr, tile_id, block_id, datapath_mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetDataPathMode failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_IMRPassMode(UINT32 tile_id, UINT32 block_id, UINT32 *imr_pass_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 IMRPassMode;

    if (!imr_pass_mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetIMRPassMode(RFdcInstPtr, tile_id, block_id, &IMRPassMode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetIMRPassMode failed!\n");
        return XRFDC_FAILURE;
    }
    *imr_pass_mode = IMRPassMode;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_IMRPassMode(UINT32 tile_id, UINT32 block_id, UINT32 imr_pass_mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetIMRPassMode(RFdcInstPtr, tile_id, block_id, imr_pass_mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetIMRPassMode failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_OutputCurrent(UINT32 tile_id, UINT32 block_id, UINT32 *output_current)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 OutputCurent;

    if (!output_current)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetOutputCurr(RFdcInstPtr, tile_id, block_id, &OutputCurent);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetOutputCurr failed!\n");
        return XRFDC_FAILURE;
    }
    *output_current = OutputCurent;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_DACVOP(UINT32 tile_id, UINT32 block_id, UINT32 uACurrent)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetDACVOP(RFdcInstPtr, tile_id, block_id, uACurrent);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetDACVOP failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_InverseSincFIR(UINT32 tile_id, UINT32 block_id, UINT16 *mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT16 Mode;

    if (!mode)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetInvSincFIR(RFdcInstPtr, tile_id, block_id, &Mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetInvSincFIR failed!\n");
        return XRFDC_FAILURE;
    }
    *mode = Mode;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_InverseSincFIR(UINT32 tile_id, UINT32 block_id, UINT16 mode)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetInvSincFIR(RFdcInstPtr, tile_id, block_id, mode);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetInvSincFIR failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_InterpolationFactor(UINT32 tile_id, UINT32 block_id, UINT32 *interpolation_factor)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    UINT32 InterpolationFactor;

    if (!interpolation_factor)
    {
        // print
        return XRFDC_FAILURE;
    }
    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetInterpolationFactor(RFdcInstPtr, tile_id, block_id, &InterpolationFactor);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_GetInterpolationFactor failed!\n");
        return XRFDC_FAILURE;
    }
    *interpolation_factor = InterpolationFactor;
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_set_InterpolationFactor(UINT32 tile_id, UINT32 block_id, UINT32 interpolation_factor)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (tile_id > XRFDC_TILE_ID_MAX || block_id > XRFDC_BLOCK_ID_MAX)
    {
        // print
        return XRFDC_FAILURE;
    }
    Status = XRFdc_SetInterpolationFactor(RFdcInstPtr, tile_id, block_id, interpolation_factor);
    if (Status != XRFDC_SUCCESS) {
        fmsh_print("XRFdc_SetInterpolationFactor failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_run_MTS(UINT32 type, XRFdc_MultiConverter_Sync_Config *ConfigPtr, INT32 *PLL_CodesPtr, INT32 *T1_CodesPtr)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (ConfigPtr == NULL)
    {
        fmsh_print("parameter pointer is NULL.\n");
        return XRFDC_FAILURE;
    }
    Status = XRFdc_MultiConverter_Init(ConfigPtr, PLL_CodesPtr, T1_CodesPtr, ConfigPtr->RefTile);
    if (Status != XRFDC_MTS_OK) {
        fmsh_print("Multi-Tile init failed. MTS Error code is %u \n", Status);
        return XRFDC_FAILURE;
    }
    Status = XRFdc_MultiConverter_Sync(RFdcInstPtr, type, ConfigPtr);
    if (Status != XRFDC_MTS_OK) {
        fmsh_print("Multi-Tile-Sync run failed. MTS Error code is %u \n", Status);
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_MTS_Sysref_Config(XRFdc_MultiConverter_Sync_Config *DACSyncConfigPtr, XRFdc_MultiConverter_Sync_Config *ADCSyncConfigPtr, UINT32 SysRefEnable)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (DACSyncConfigPtr == NULL || ADCSyncConfigPtr == NULL)
    {
        fmsh_print("parameter pointer is NULL.\n");
        return XRFDC_FAILURE;
    }
    Status = XRFdc_MTS_Sysref_Config(RFdcInstPtr, DACSyncConfigPtr, ADCSyncConfigPtr, SysRefEnable);
    if (Status != XRFDC_MTS_OK) {
        fmsh_print("XRFdc_MTS_Sysref_Config failed!\n");
        return XRFDC_FAILURE;
    }
    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_get_MTSEnable(UINT32 type, UINT32 tile_id, UINT32 *enable)
{
    UINT32 Status;
    XRFdc *RFdcInstPtr = getRFdcInstance();

    if (enable == NULL)
    {
        fmsh_print("parameter pointer is NULL.\n");
        return XRFDC_FAILURE;
    }
    Status = XRFdc_GetMTSEnable(RFdcInstPtr, type, tile_id, enable);
    if (Status != XRFDC_MTS_OK) {
        fmsh_print("XRFdc_GetMTSEnable failed!\n");
        return XRFDC_FAILURE;
    }

    return XRFDC_SUCCESS;
}

STATUS mw_rfdc_read_Reg(UINT32 BaseAddr, UINT32 RegAddr, UINT16 Mask, UINT16 *read_reg)
{
    UINT16 ReadReg;
    XRFdc *RFdcInstPtr = getRFdcInstance();
    if (!read_reg)
    {
        // print
        return XRFDC_FAILURE;
    }
    ReadReg = XRFdc_RDReg(RFdcInstPtr, BaseAddr, RegAddr, Mask);
    *read_reg = ReadReg;
    return XRFDC_SUCCESS;
}
