/*
 * @Copyright: Copyright(C),2016-2022,Nanjing Digitgate Communication Technology Co., LTD. All Rights Reserved. 
 */

#include "xrfdc_main.h"
#include "xrfdc_api.h"
#include "xparameters.h"
#include "fmsh_common_io.h"

#define XRFDC_DEVICE_ID_SIZE 4U

/*
static COMMAND_S rfdc_cmd[] =
{
    {0, RFDC_NAME_INIT,               RFDC_INFO_INIT,               rfdc_init},
    {1, RFDC_NAME_RESET,              RFDC_INFO_RESET,              rfdc_reset},
    {2, RFDC_NAME_GET_PLL_CONFIG,     RFDC_INFO_GET_PLL_CONFIG,     rfdc_getpllConfig},
    {3, RFDC_NAME_SET_PLL_CONFIG,     RFDC_INFO_SET_PLL_CONFIG,     rfdc_setpllConfig},
    {4, RFDC_NAME_PLL_LOCK_STATUS,    RFDC_INFO_PLL_LOCK_STATUS,    rfdc_getpllLockStatus},
    {5, RFDC_NAME_GET_TILE_STATUS,    RFDC_INFO_GET_TILE_STATUS,    rfdc_getTileStatus},
    {6, RFDC_NAME_GET_BLOCK_STATUS,   RFDC_INFO_GET_BLOCK_STATUS,   rfdc_getBlockStatus},
    {7, RFDC_NAME_GET_MIXER_SETTINGS, RFDC_INFO_GET_MIXER_SETTINGS, rfdc_getMixerSettings},
    {8, RFDC_NAME_SET_MIXER_SETTINGS, RFDC_INFO_SET_MIXER_SETTINGS, rfdc_setMixerSettings},
    {9, RFDC_NAME_GET_QMC_SETTINGS,   RFDC_INFO_GET_QMC_SETTINGS,   rfdc_getQMCSettings},
    {10, RFDC_NAME_SET_QMC_SETTINGS,  RFDC_INFO_SET_QMC_SETTINGS,   rfdc_setQMCSettings},
    {11, RFDC_NAME_GET_NYQUIST_ZONE,  RFDC_INFO_GET_NYQUIST_ZONE,   rfdc_getNyquistZone},
    {12, RFDC_NAME_SET_NYQUIST_ZONE,  RFDC_INFO_SET_NYQUIST_ZONE,   rfdc_setNyquistZone},
    {13, RFDC_NAME_GET_POWER_MODE,    RFDC_INFO_GET_POWER_MODE,     rfdc_getPwrMode},
    {14, RFDC_NAME_SET_POWER_MODE,    RFDC_INFO_SET_POWER_MODE,     rfdc_setPwrMode},
    {15, RFDC_NAME_GET_INTERRUPT_STATUS, RFDC_INFO_GET_INTERRUPT_STATUS, rfdc_getIntrStatus},

    {16, RFDC_NAME_GET_DSA_SETTINGS,  RFDC_INFO_GET_DSA_SETTINGS,  rfdc_getDSA},
    {17, RFDC_NAME_SET_DSA_SETTINGS,  RFDC_INFO_SET_DSA_SETTINGS,  rfdc_setDSA},
    {18, RFDC_NAME_GET_SIGNAL_DETECTOR, RFDC_INFO_GET_SIGNAL_DETECTOR, rfdc_getSignalDetector},
    {19, RFDC_NAME_SET_SIGNAL_DETECTOR, RFDC_INFO_SET_SIGNAL_DETECTOR, rfdc_setSignalDetector},
    {20, RFDC_NAME_GET_CALIBRATION_MODE, RFDC_INFO_GET_CALIBRATION_MODE,  rfdc_getCalibrationMode},
    {21, RFDC_NAME_SET_CALIBRATION_MODE, RFDC_INFO_SET_CALIBRATION_MODE,  rfdc_setCalibrationMode},
    {22, RFDC_NAME_GET_LINKCOUPLING,  RFDC_INFO_GET_LINKCOUPLING,  rfdc_getLinkCoupling},
    {23, RFDC_NAME_GET_DITHER, RFDC_INFO_GET_DITHER, rfdc_getDither},
    {24, RFDC_NAME_SET_DITHER, RFDC_INFO_SET_DITHER, rfdc_setDither},
    {25, RFDC_NAME_GET_DECIMATION_FACTOR, RFDC_INFO_GET_DECIMATION_FACTOR, rfdc_getDecimationFactor},
    {26, RFDC_NAME_SET_DECIMATION_FACTOR, RFDC_INFO_SET_DECIMATION_FACTOR, rfdc_setDecimationFactor},
    {27, RFDC_NAME_GET_THRESHOLD_SETTINGS, RFDC_INFO_GET_THRESHOLD_SETTINGS, rfdc_getThresholdSettings},
    {28, RFDC_NAME_SET_THRESHOLD_SETTINGS, RFDC_INFO_SET_THRESHOLD_SETTINGS, rfdc_setThresholdSettings},
    {29, RFDC_NAME_SET_THRESHOLD_CLR_MODE, RFDC_INFO_SET_THRESHOLD_CLR_MODE, rfdc_setThresholdClrMode},

    {30, RFDC_NAME_GET_DECODER_MODE,  RFDC_INFO_GET_DECODER_MODE,  rfdc_getDecoderMode},
    {31, RFDC_NAME_SET_DECODER_MODE,  RFDC_INFO_SET_DECODER_MODE,  rfdc_setDecoderMode},
    {32, RFDC_NAME_GET_DATAPATH_MODE, RFDC_INFO_GET_DATAPATH_MODE, rfdc_getDataPathMode},
    {33, RFDC_NAME_SET_DATAPATH_MODE, RFDC_INFO_SET_DATAPATH_MODE, rfdc_setDataPathMode},
    {34, RFDC_NAME_GET_IMR_PASS_MODE, RFDC_INFO_GET_IMR_PASS_MODE, rfdc_getIMRPassMode},
    {35, RFDC_NAME_SET_IMR_PASS_MODE, RFDC_INFO_SET_IMR_PASS_MODE, rfdc_setIMRPassMode},
    {36, RFDC_NAME_GET_OUTPUTCURRENT, RFDC_INFO_GET_OUTPUTCURRENT, rfdc_getOutputCurrent},
    {37, RFDC_NAME_SET_DAC_VOP,       RFDC_INFO_SET_DAC_VOP,       rfdc_setDACVOP},
    {38, RFDC_NAME_GET_INVERSE_SINC_FIR, RFDC_INFO_GET_INVERSE_SINC_FIR, rfdc_getInverseSincFIR},
    {39, RFDC_NAME_SET_INVERSE_SINC_FIR, RFDC_INFO_SET_INVERSE_SINC_FIR, rfdc_setInverseSincFIR},
    {40, RFDC_NAME_GET_INTERPOLATION_FACTOR, RFDC_INFO_GET_INTERPOLATION_FACTOR, rfdc_getInterpolationFactor},
    {41, RFDC_NAME_SET_INTERPOLATION_FACTOR, RFDC_INFO_SET_INTERPOLATION_FACTOR, rfdc_setInterpolationFactor},

    {42, RFDC_NAME_MTS, RFDC_INFO_MTS, rfdc_MultiTileSync},
    {43, RFDC_NAME_READ_REG, RFDC_INFO_READ_REG, rfdc_readRegs},
    {44, RFDC_NAME_SHUTDOWN, RFDC_INFO_SHUTDOWN, rfdc_shutdown},
    {45, RFDC_NAME_STARTUP, RFDC_INFO_STARTUP, rfdc_startup},
    {46, RFDC_NAME_SETCLKDISTRIBUTION, RFDC_INFO_SETCLKDISTRIBUTION, rfdc_setclkdistribution},
    {47, RFDC_NAME_GETCLKDISTRIBUTION, RFDC_INFO_GETCLKDISTRIBUTION, rfdc_getclkdistribution},
};
#define CMD_NUM  (sizeof(rfdc_cmd) / sizeof(COMMAND_S))
*/



void send_singleecho(void)
{
    metal_rfdc_writeRegs(0xf4,0x1);
    metal_rfdc_writeRegs(0xf0,0x1);
    metal_rfdc_writeRegs(0xec,0x3);
    metal_rfdc_writeRegs(0xe8,0x3);
    metal_rfdc_writeRegs(0xf8,0x3);
    metal_rfdc_writeRegs(0x114,0x0);
    metal_rfdc_writeRegs(0x118,0x0);
    metal_rfdc_writeRegs(0x124,0x0);
    metal_rfdc_writeRegs(0x128,0x0);
    rfdc_setDecoderMode(0,0,2);
    metal_rfdc_writeRegs(0x3e,0xb);
    /*均衡数据*/
    metal_rfdc_writeRegs(0x240,0x00059);
    metal_rfdc_writeRegs(0x248,0x3FFF7);
    metal_rfdc_writeRegs(0x250,0x0005A);
    metal_rfdc_writeRegs(0x258,0x0033B);
    metal_rfdc_writeRegs(0x260,0x3F227);
    metal_rfdc_writeRegs(0x268,0x00CF9);
    metal_rfdc_writeRegs(0x270,0x3EDC6);
    metal_rfdc_writeRegs(0x278,0x018F5);
    metal_rfdc_writeRegs(0x280,0x3E007);
    metal_rfdc_writeRegs(0x288,0x0258A);
    metal_rfdc_writeRegs(0x290,0x3D83C);
    metal_rfdc_writeRegs(0x298,0x0197D);
    metal_rfdc_writeRegs(0x2a0,0x1F148);
    metal_rfdc_writeRegs(0x2a8,0x02A3D);
    metal_rfdc_writeRegs(0x2b0,0x3D13E);
    metal_rfdc_writeRegs(0x2b8,0x02957);
    metal_rfdc_writeRegs(0x2c0,0x3DE33);
    metal_rfdc_writeRegs(0x2c8,0x019D4);
    metal_rfdc_writeRegs(0x2d0,0x3ED8E);
    metal_rfdc_writeRegs(0x2d8,0x00CE7);
    metal_rfdc_writeRegs(0x2e0,0x3F28D);
    metal_rfdc_writeRegs(0x2e8,0x0029F);
    metal_rfdc_writeRegs(0x2f0,0x000AA);
    metal_rfdc_writeRegs(0x2f8,0x3FFDF);
    metal_rfdc_writeRegs(0x300,0x00051);
    metal_rfdc_writeRegs(0x244,0x3FFF8);
    metal_rfdc_writeRegs(0x24c,0x00083);
    metal_rfdc_writeRegs(0x254,0x3FEB3);
    metal_rfdc_writeRegs(0x25c,0x0025B);
    metal_rfdc_writeRegs(0x264,0x3FF1B);
    metal_rfdc_writeRegs(0x26c,0x00026);
    metal_rfdc_writeRegs(0x274,0x00148);
    metal_rfdc_writeRegs(0x27c,0x3FBE5);
    metal_rfdc_writeRegs(0x284,0x005C3);
    metal_rfdc_writeRegs(0x28c,0x3F377);
    metal_rfdc_writeRegs(0x294,0x012A0);
    metal_rfdc_writeRegs(0x29c,0x3C48B);
    metal_rfdc_writeRegs(0x2a4,0x3EBF8);
    metal_rfdc_writeRegs(0x2ac,0x03A6E);
    metal_rfdc_writeRegs(0x2b4,0x3F104);
    metal_rfdc_writeRegs(0x2bc,0x00921);
    metal_rfdc_writeRegs(0x2c4,0x3FD26);
    metal_rfdc_writeRegs(0x2cc,0x001D1);
    metal_rfdc_writeRegs(0x2d4,0x0006F);
    metal_rfdc_writeRegs(0x2dc,0x3FE9E);
    metal_rfdc_writeRegs(0x2e4,0x0021B);
    metal_rfdc_writeRegs(0x2ec,0x3FD54);
    metal_rfdc_writeRegs(0x2f4,0x00147);
    metal_rfdc_writeRegs(0x2fc,0x3FF81);
    metal_rfdc_writeRegs(0x304,0x3FFFF);
}

/*
 *write fpga reg
 *param[out]: parameter, double type
 *return:
 *      ok: return 0
 *    fail: return -1
 *
 */
INT32 rfdc_check_address(UINT32 Offset) {
    return ((0 == (Offset % 4)) && (Offset >= 0) && (Offset <= XPAR_USP_RF_DATA_CONVERTER_0_HIGHADDR-XPAR_USP_RF_DATA_CONVERTER_0_BASEADDR)) ? 0 : 1;
}

void metal_rfdc_writeRegs(UINT32 Offset, UINT32 Data)
{
    if ((rfdc_check_address(Offset))) {
        fmsh_print("invalid address: %u\n", XPAR_USP_RF_DATA_CONVERTER_0_HIGHADDR+Offset);
        return;
    }
    FMSH_WriteReg((UINT32)XPAR_M01_AXI_0_BASEADDR, Offset, Data);
    fmsh_print("reg cnt: %#x \r\n",FMSH_ReadReg((UINT32)XPAR_M01_AXI_0_BASEADDR, Offset));
}

void metal_rfdc_readRegs(UINT32 Offset)
{
    if (rfdc_check_address(Offset)) {
        fmsh_print("invalid address: %u\n", XPAR_USP_RF_DATA_CONVERTER_0_HIGHADDR+Offset);
        return;
    }
    fmsh_print("reg cnt: %#x \r\n",FMSH_ReadReg((UINT32)XPAR_M01_AXI_0_BASEADDR, Offset));
}

/*
 *get parameter for shell cmd
 *param[out]: parameter, double type
 *return:
 *      ok: return 0
 *    fail: return -1
 *
 */
static INT32 xrfdc_get_param_double(CHAR* param_str, double* parameter, CHAR* paramName, void (*usage)(void))
{
    if(!parameter) return -1;
    if(!paramName) return -1;

    double tmp_param = 0.0;
    
    if(param_str != NULL) {
        if((strcmp(param_str,"-h") == 0) || (strcmp(param_str,"-H") == 0)) {
            goto parse_err;
        }
        
        if(1 != sscanf(param_str, "%lf", &tmp_param)) {
            fmsh_print("get parameter %s failed\r\n", paramName);
            goto parse_err;
        }
    }
    else {
        fmsh_print("param parameter %s needed! \r\n", paramName);
        goto parse_err;
    }

    *parameter = tmp_param;
    return 0;

parse_err:
    if(usage)usage();
    return -1;
}

/*
 *get parameter for shell cmd
 *param[out]: parameter, float type
 *return:
 *      ok: return 0
 *    fail: return -1
 *
 */
static INT32 xrfdc_get_param_float(CHAR* param_str, float* parameter, CHAR* paramName, void (*usage)(void))
{
    if(!parameter) return -1;
    if(!paramName) return -1;

    float tmp_param = 0.0;
    
    if(param_str != NULL) {
        if((strcmp(param_str,"-h") == 0) || (strcmp(param_str,"-H") == 0)) {
            goto parse_err;
        }
        
        if(1 != sscanf(param_str, "%f", &tmp_param)) {
            fmsh_print("get parameter %s failed\r\n", paramName);
            goto parse_err;
        }
    }
    else {
        fmsh_print("param parameter %s needed! \r\n", paramName);
        goto parse_err;
    }

    *parameter = tmp_param;
    return 0;

parse_err:
    if(usage)usage();
    return -1;
}

/*
 *get parameter for shell cmd
 *param[out]: parameter, hex mode
 *return:
 *      ok: return 0
 *    fail: return -1
 *
 */
static INT32 xrfdc_get_param_hex(CHAR* param_str, UINT32* parameter, CHAR* param_name, void (*usage)(void))
{
    if(!parameter) return -1;
    if(!param_name) return -1;

    UINT32 tmpParam = 0;
    
    if(param_str != NULL) {
        if((strcmp(param_str,"-h") == 0) || (strcmp(param_str,"-H") == 0)) {
            goto parse_err;
        }

        if(1 != sscanf(param_str, "0x%x", &tmpParam)) {
            fmsh_print("get parameter %s failed\r\n", param_name);
            goto parse_err;
        }
    }
    else {
        fmsh_print("param parameter %s needed! \r\n", param_name);
        goto parse_err;
    }

    *parameter = tmpParam;
    return 0;
    
parse_err:
    if(usage) usage();
    return -1;
}

/*
 *get parameter for shell cmd
 *param[out]: parameter, UINT32 type
 *return:
 *      ok: return 0
 *    fail: return -1
 *
 */
static INT32 xrfdc_get_param(CHAR* param_str, UINT32* parameter, CHAR* param_name, void (*usage)(void))
{
    if(!parameter) return -1;
    if(!param_name) return -1;

    UINT32 tmpParam = 0;
    
    if(param_str != NULL) {
        if((strcmp(param_str,"-h") == 0) || (strcmp(param_str,"-H") == 0)) {
            goto parse_err;
        }
        
        if(1 != sscanf(param_str,"%u", &tmpParam)) {
            fmsh_print("get parameter %s failed\r\n", param_name);
            goto parse_err;
        }
    }
    else {
        fmsh_print("param parameter %s needed! \r\n", param_name);
        goto parse_err;
    }

    *parameter = tmpParam;
    return 0;
    
parse_err:
    if(usage) usage();
    return -1;
}

/*
 * get tile_id for shell cmd
 *
 * return:
 *      ok: return 0
 *    fail: return -1
 *
 */
static INT32 xrfdc_get_tile_id(CHAR* param_str, UINT32* tile_id, void (*usage)(void))
{
    if(!tile_id) return -1;

    UINT32 tmpIdx = 0;
    if (xrfdc_get_param(param_str, &tmpIdx, "tile_id", usage) != 0) return -1;
    if (check_tile_id(tmpIdx) == 0)
    {
        fmsh_print("invalid tile index, tile_id=%u\r\n", tmpIdx);
        goto parse_err;
    }

    *tile_id = tmpIdx;
    return 0;

parse_err:
    if(usage) usage();
    return -1;
}

/*
 * get block_id for shell cmd
 *
 * return:
 *      ok: return 0
 *    fail: return -1
 *
 */
static INT32 xrfdc_get_block_id(CHAR* param_str, UINT32* block_id, void (*usage)(void))
{
    if(!block_id) return -1;

    UINT32 tmpIdx = 0;
    if (xrfdc_get_param(param_str, &tmpIdx, "block_id", usage) != 0) return -1;
    if (check_block_id(tmpIdx) == 0)
    {
        fmsh_print("invalid block index, block_id=%u\r\n", tmpIdx);
        goto parse_err;
    }

    *block_id = tmpIdx;
    return 0;

parse_err:
    if(usage) usage();
    return -1;
}

static void rfdc_init_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_INIT, RFDC_INFO_INIT);
    fmsh_print("Synopsis: %s [DeviceId] <-h/H>\r\n", RFDC_NAME_INIT);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-DeviceId -- contains the ID of the device to look up the configuration for, valid range: [0~%d).\r\n", XRFDC_DEVICE_ID_SIZE);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 \r\n\r\n", RFDC_NAME_INIT);
}

void rfdc_init(UINT16 device_id)
{
    UINT32 retVal;
    retVal = mw_rfdc_init(device_id);
    fmsh_print("XRFDC Initialize %s\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_reset_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_RESET, RFDC_INFO_RESET);
    fmsh_print("Synopsis: %s [type] [tile_id] <-h/H>\r\n", RFDC_NAME_RESET);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: -1, [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 -1 \r\n\r\n", RFDC_NAME_RESET);
}

void rfdc_reset(UINT32 type, INT32 tile_id)
{
    UINT32 retVal;
    retVal= mw_rfdc_reset(type, tile_id);
    fmsh_print("%s tile[%d] Reset %s\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getpllConfig_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_PLL_CONFIG, RFDC_INFO_GET_PLL_CONFIG);
    fmsh_print("Synopsis: %s [type] [tile_id] <-h/H>\r\n", RFDC_NAME_GET_PLL_CONFIG);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 \r\n", RFDC_NAME_GET_PLL_CONFIG);
}

void rfdc_getpllConfig(UINT32 type, INT32 tile_id)
{
    UINT32 retVal;

    mw_rfdc_pll_settings pll_settings;
    memset(&pll_settings, 0, sizeof(mw_rfdc_pll_settings));

    retVal = mw_rfdc_get_pll_config(type, tile_id, &pll_settings);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] pll Settings:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10d|\r\n", "Enabled", pll_settings.Enabled);
        fmsh_print("|%-40s|  %-10.4f|\r\n", "RefClkFreq", pll_settings.RefClkFreq);
        fmsh_print("|%-40s|  %-10.4f|\r\n", "SampleRate", pll_settings.SampleRate);
        fmsh_print("|%-40s|  %-10d|\r\n", "RefClkDivider", pll_settings.RefClkDivider);
        fmsh_print("|%-40s|  %-10d|\r\n", "FeedbackDivider", pll_settings.FeedbackDivider);
        fmsh_print("|%-40s|  %-10d|\r\n", "OutputDivider", pll_settings.OutputDivider);
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_pll_config failed, type=%s, tile_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id);
    }
}

static void rfdc_setpllConfig_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_PLL_CONFIG, RFDC_INFO_SET_PLL_CONFIG);
    fmsh_print("Synopsis: %s [type] [tile_id] [clkSource] [refClkFreq] [samplingRate] <-h/H>\r\n", RFDC_NAME_SET_PLL_CONFIG);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-clkSource -- clock source, 0: external clock; 1: internal pll clock\r\n");
    fmsh_print("\t-refClkFreq -- Reference Clock Frequency in MHz, (102.40625MHz - 1.2GHz)\r\n");
    fmsh_print("\t-samplingRate -- Sampling Rate in MHz(0.1 - 6.554GHz for DAC and 0.5/1.0 - 2.058/4.116GHz for ADC)\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 1 1 1 245.76 786.43\r\n\r\n", RFDC_NAME_SET_PLL_CONFIG);
}

void rfdc_setpllConfig(UINT32 type, UINT32 tile_id, UINT8 clkSource, 
    double refClkFreq, double samplingRate)
{
    UINT32 retVal;
    double MaxSampleRate;
    double MinSampleRate;
    // get ADC/DAC type
    if ((0 != type) && (1 != type))
    {
        rfdc_setpllConfig_usage();
        return;
    }

    if (mw_rfdc_get_MaxSampleRate(type, tile_id, &MaxSampleRate) != OK) {
        fmsh_print("mw_rfdc_get_MaxSampleRate failed, type=%s, tile_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id);
        return;
    }
    if (mw_rfdc_get_MinSampleRate(type, tile_id, &MinSampleRate) != OK) {
        fmsh_print("mw_rfdc_get_MinSampleRate failed, type=%s, tile_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id);
        return;
    }
    if ((samplingRate < MinSampleRate) || (samplingRate > MaxSampleRate)) {
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10.4f|\r\n", "MaxSampleRate", MaxSampleRate);
        fmsh_print("|%-40s|  %-10.4f|\r\n", "MinSampleRate", MinSampleRate);
        fmsh_print("================================================================\r\n");
        fmsh_print("Invalid samplingRate value (%lf), type=%s, tile_id=%d\r\n", samplingRate, (type == XRFDC_ADC_TILE) ? "ADC" : "DAC", tile_id);
        return;
    }
    retVal = mw_rfdc_set_pll_config(type, tile_id, clkSource, refClkFreq, samplingRate);
    fmsh_print("XRFDC Set PLL Config %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getpllLockStatus_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_PLL_LOCK_STATUS, RFDC_INFO_PLL_LOCK_STATUS);
    fmsh_print("Synopsis: %s [type] [tile_id] <-h/H>\r\n", RFDC_NAME_PLL_LOCK_STATUS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 \r\n\r\n", RFDC_NAME_PLL_LOCK_STATUS);
}

void rfdc_getpllLockStatus(UINT32 type, INT32 tile_id)
{
    UINT32 retVal;
    UINT32 lock_status;

    retVal = mw_rfdc_get_pll_lock_status(type, tile_id, &lock_status);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] PLL clock [%s]\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, (lock_status == XRFDC_PLL_LOCKED) ? "Locked" : "Unlock");
    }
    else
    {
        fmsh_print("mw_rfdc_get_pll_lock_status failed, type=%s, tile_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id);
    }
}

static void rfdc_getTileStatus_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_TILE_STATUS, RFDC_INFO_GET_TILE_STATUS);
    fmsh_print("Synopsis: %s [type] [tile_id] <-h/H>\r\n", RFDC_NAME_GET_TILE_STATUS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 \r\n\r\n", RFDC_NAME_GET_TILE_STATUS);
}

void rfdc_getTileStatus(UINT32 type, INT32 tile_id)
{
    UINT32 retVal;

    XRFdc_TileStatus tile_status;
    memset(&tile_status, 0, sizeof(XRFdc_TileStatus));

    retVal = mw_rfdc_get_tile_status(type, tile_id, &tile_status);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] status:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10u|\r\n", "IsEnabled", tile_status.IsEnabled);
        fmsh_print("|%-40s|  %-10u|\r\n", "TileState", tile_status.TileState);
        fmsh_print("|%-40s|0x%-10x|\r\n", "BlockStatusMask", tile_status.BlockStatusMask);
        fmsh_print("|%-40s|  %-10u|\r\n", "PowerUpState", tile_status.PowerUpState);
        fmsh_print("|%-40s|  %-10u|\r\n", "PLLState", tile_status.PLLState);
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_tile_status failed, type=%s, tile_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id);
    }
}

static void rfdc_getBlockStatus_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_BLOCK_STATUS, RFDC_INFO_GET_BLOCK_STATUS);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_BLOCK_STATUS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 \r\n\r\n", RFDC_NAME_GET_BLOCK_STATUS);
}

void rfdc_getBlockStatus(UINT32 type, INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;

    XRFdc_BlockStatus block_status;
    memset(&block_status, 0, sizeof(XRFdc_BlockStatus));

    retVal = mw_rfdc_get_block_status(type, tile_id, block_id, &block_status);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] block[%d] status:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10.4f|\r\n", "SamplingFreq", block_status.SamplingFreq);
        if (type == XRFDC_ADC_TILE) // ADC
        {
            fmsh_print("|%-40s|0x%-10x|\r\n", "AnalogDataPathStatus", block_status.AnalogDataPathStatus);
            fmsh_print("|%-40s|  %-10u|\r\n", "AnalogDataPathStatus [0]: Converter enable/disable", block_status.AnalogDataPathStatus & 0x00000001);

            fmsh_print("|%-40s|0x%-10x|\r\n", "DigitalDataPathStatus", block_status.DigitalDataPathStatus);
            fmsh_print("|%-40s|  %-10u|\r\n", "DigitalDataPathStatus [3:0] FIFO status", block_status.DigitalDataPathStatus & 0x0000000f);
            fmsh_print("|%-40s|  %-10u|\r\n", "DigitalDataPathStatus [7:4] Decimation factor", (block_status.DigitalDataPathStatus & 0x000000f0) >> 4);
            fmsh_print("|%-40s|  %-10u|\r\n", "DigitalDataPathStatus [11:8] Mixer mode", (block_status.DigitalDataPathStatus & 0x00000f00) >> 8);
        }
        else if (type == XRFDC_DAC_TILE) // DAC
        {
            fmsh_print("|%-40s|0x%-10x|\r\n", "AnalogDataPathStatus", block_status.AnalogDataPathStatus);
            fmsh_print("|%-40s|  %-10u|\r\n", "AnalogDataPathStatus [3:0] Inverse sinc enable/disable", block_status.AnalogDataPathStatus & 0x0000000f);
            fmsh_print("|%-40s|  %-10u|\r\n", "AnalogDataPathStatus [7:4] Decoder mode", (block_status.AnalogDataPathStatus & 0x000000f0) >> 4);

            fmsh_print("|%-40s|0x%-10x|\r\n", "DigitalDataPathStatus", block_status.DigitalDataPathStatus);
            fmsh_print("|%-40s|  %-10u|\r\n", "DigitalDataPathStatus [3:0] FIFO status", block_status.DigitalDataPathStatus & 0x0000000f);
            fmsh_print("|%-40s|  %-10u|\r\n", "DigitalDataPathStatus [7:4] Interpolation factor", (block_status.DigitalDataPathStatus & 0x000000f0) >> 4);
            fmsh_print("|%-40s|  %-10u|\r\n", "DigitalDataPathStatus [11:8] Adder status", (block_status.DigitalDataPathStatus & 0x00000f00) >> 8);
            fmsh_print("|%-40s|  %-10u|\r\n", "DigitalDataPathStatus [15:12] Mixer mode", (block_status.DigitalDataPathStatus & 0x0000f000) >> 12);
        }
        fmsh_print("|%-40s|  %-10u|\r\n", "DataPathClocksStatus", block_status.DataPathClocksStatus);
        fmsh_print("|%-40s|  %-10u|\r\n", "IsFIFOFlagsEnabled", block_status.IsFIFOFlagsEnabled);
        fmsh_print("|%-40s|  %-10u|\r\n", "IsFIFOFlagsAsserted", block_status.IsFIFOFlagsAsserted);
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_block_status failed, type=%s, tile_id=%d, block_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
    }
}

static void rfdc_getMixerSettings_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_MIXER_SETTINGS, RFDC_INFO_GET_MIXER_SETTINGS);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_MIXER_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 \r\n\r\n", RFDC_NAME_GET_MIXER_SETTINGS);
}

void rfdc_getMixerSettings(UINT32 type, UINT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
 
    XRFdc_Mixer_Settings mixer_settings;
    memset(&mixer_settings, 0, sizeof(XRFdc_Mixer_Settings));

    retVal = mw_rfdc_get_mixer_settings(type, tile_id, block_id, &mixer_settings);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] block[%d] Mixer Settings:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10.4f|\r\n", "Freq", mixer_settings.Freq);
        fmsh_print("|%-40s|  %-10.4f|\r\n", "PhaseOffset", mixer_settings.PhaseOffset);
        switch (mixer_settings.EventSource)
        {
            case XRFDC_EVNT_SRC_IMMEDIATE: fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_IMMEDIATE");break;
            case XRFDC_EVNT_SRC_SLICE:     fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_SLICE");break;
            case XRFDC_EVNT_SRC_TILE:      fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_TILE");break;
            case XRFDC_EVNT_SRC_SYSREF:    fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_SYSREF");break;
            case XRFDC_EVNT_SRC_MARKER:    fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_MARKER");break;
            case XRFDC_EVNT_SRC_PL:        fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_PL");break;
            default:                       fmsh_print("Invalid mixer_settings.EventSource\r\n");break;
        }
        switch (mixer_settings.CoarseMixFreq)
        {
            case XRFDC_COARSE_MIX_OFF:                     fmsh_print("|%-40s|%-10s|\r\n", "CoarseMixFreq", "OFF");break;
            case XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO:      fmsh_print("|%-40s|%-10s|\r\n", "CoarseMixFreq", "Fs/2");break;
            case XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR:     fmsh_print("|%-40s|%-10s|\r\n", "CoarseMixFreq", "Fs/4");break;
            case XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR: fmsh_print("|%-40s|%-10s|\r\n", "CoarseMixFreq", "-Fs/4");break;
            case XRFDC_COARSE_MIX_BYPASS:                  fmsh_print("|%-40s|%-10s|\r\n", "CoarseMixFreq", "BYPASS");break;
            default:                                       fmsh_print("Invalid mixer_settings.CoarseMixFreq\r\n");break;
        }
        switch (mixer_settings.MixerMode)
        {
            case XRFDC_MIXER_MODE_OFF: fmsh_print("|%-40s|%-20s|\r\n", "MixerMode", "OFF");break;
            case XRFDC_MIXER_MODE_C2C: fmsh_print("|%-40s|%-20s|\r\n", "MixerMode", "Complex to Complex");break;
            case XRFDC_MIXER_MODE_C2R: fmsh_print("|%-40s|%-20s|\r\n", "MixerMode", "Complex to Real");break;
            case XRFDC_MIXER_MODE_R2C: fmsh_print("|%-40s|%-20s|\r\n", "MixerMode", "Real to Complex");break;
            case XRFDC_MIXER_MODE_R2R: fmsh_print("|%-40s|%-20s|\r\n", "MixerMode", "Real to Real");break;
            default:                   fmsh_print("Invalid mixer_settings.MixerMode\r\n");break;
        }
        switch (mixer_settings.FineMixerScale)
        {
            case XRFDC_MIXER_SCALE_AUTO: fmsh_print("|%-40s|%-10s|\r\n", "FineMixerScale", "AUTO");break;
            case XRFDC_MIXER_SCALE_1P0:  fmsh_print("|%-40s|%-10s|\r\n", "FineMixerScale", "1.0");break;
            case XRFDC_MIXER_SCALE_0P7:  fmsh_print("|%-40s|%-10s|\r\n", "FineMixerScale", "0.7");break;
            default:                     fmsh_print("Invalid mixer_settings.FineMixerScale\r\n");break;
        }
        switch (mixer_settings.MixerType)
        {
            case XRFDC_MIXER_TYPE_OFF:      fmsh_print("|%-40s|%-10s|\r\n", "MixerType", "OFF");break;
            case XRFDC_MIXER_TYPE_COARSE:   fmsh_print("|%-40s|%-10s|\r\n", "MixerType", "COARSE");break;
            case XRFDC_MIXER_TYPE_FINE:     fmsh_print("|%-40s|%-10s|\r\n", "MixerType", "FINE");break;
            case XRFDC_MIXER_TYPE_DISABLED: fmsh_print("|%-40s|%-10s|\r\n", "MixerType", "DISABLED");break;
            default:                        fmsh_print("Invalid mixer_settings.MixerType\r\n"); break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_mixer_settings failed, type=%s, tile_id=%d, block_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
    }
}

static void rfdc_setMixerSettings_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_MIXER_SETTINGS, RFDC_INFO_SET_MIXER_SETTINGS);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] [Freq] [PhaseOffset] [EventSource] [CoarseMixFreq] [MixerMode] [FineMixerScale] [MixerType] <-h/H>\r\n", RFDC_NAME_SET_MIXER_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-Freq -- NCO Frequency in MHz, valid range: [-Fs, Fs]\r\n");
    fmsh_print("\t-PhaseOffset -- NCO phase offset, valid range: [-180, 180)\r\n");
    fmsh_print("\t-EventSource -- Event Source:\r\n");
    fmsh_print("\t                0, XRFDC_EVNT_SRC_IMMEDIATE\r\n");
    fmsh_print("\t                1, XRFDC_EVNT_SRC_SLICE\r\n");
    fmsh_print("\t                2, XRFDC_EVNT_SRC_TILE\r\n");
    fmsh_print("\t                3, XRFDC_EVNT_SRC_SYSREF\r\n");
    fmsh_print("\t                4, XRFDC_EVNT_SRC_MARKER\r\n");
    fmsh_print("\t                5, XRFDC_EVNT_SRC_PL\r\n");
    fmsh_print("\t-CoarseMixFreq -- Coarse Mixer Frequency:\r\n");
    fmsh_print("\t                  0, XRFDC_COARSE_MIX_OFF\r\n");
    fmsh_print("\t                  2, XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO\r\n");
    fmsh_print("\t                  4, XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR\r\n");
    fmsh_print("\t                  8, XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR\r\n");
    fmsh_print("\t                  16, XRFDC_COARSE_MIX_BYPASS\r\n");
    fmsh_print("\t-MixerMode -- Mixer mode for fine or coarse mixer:\r\n");
    fmsh_print("\t              0, XRFDC_MIXER_MODE_OFF\r\n");
    fmsh_print("\t              1, XRFDC_MIXER_MODE_C2C\r\n");
    fmsh_print("\t              2, XRFDC_MIXER_MODE_C2R\r\n");
    fmsh_print("\t              3, XRFDC_MIXER_MODE_R2C\r\n");
    fmsh_print("\t              4, XRFDC_MIXER_MODE_R2R\r\n");
    fmsh_print("\t-FineMixerScale -- NCO output scale:\r\n");
    fmsh_print("\t                   0, AUTO\r\n");
    fmsh_print("\t                   1, 1.0\r\n");
    fmsh_print("\t                   2, 0.7\r\n");
    fmsh_print("\t-MixerType -- Mixer type indicates fine or coarse mixer:\r\n");
    fmsh_print("\t              0, OFF\r\n");
    fmsh_print("\t              1, COARSE\r\n");
    fmsh_print("\t              2, FINE\r\n");
    fmsh_print("\t              3, DISABLE\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 -1200 0 2 16 3 1 2\r\n\r\n", RFDC_NAME_SET_MIXER_SETTINGS);
}

void rfdc_setMixerSettings(UINT32 type, UINT32 tile_id, UINT32 block_id, 
    double Freq, double PhaseOffset, UINT32 EventSource, 
    UINT32 CoarseMixFreq, UINT32 MixerMode, UINT32 FineMixerScale, UINT32 MixerType)
{
    UINT32 retVal;

    XRFdc_Mixer_Settings mixer_settings;
    memset(&mixer_settings, 0, sizeof(XRFdc_Mixer_Settings));

    mixer_settings.Freq = Freq;
    mixer_settings.PhaseOffset = PhaseOffset;
    mixer_settings.EventSource = EventSource;
    mixer_settings.CoarseMixFreq = CoarseMixFreq;
    mixer_settings.MixerMode = MixerMode;
    mixer_settings.FineMixerScale = FineMixerScale;
    mixer_settings.MixerType = MixerType;
    retVal = mw_rfdc_set_mixer_settings(type, tile_id, block_id, &mixer_settings);
    fmsh_print("XRFDC Set Mixer Settings %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getQMCSettings_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_QMC_SETTINGS, RFDC_INFO_GET_QMC_SETTINGS);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_QMC_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 \r\n\r\n", RFDC_NAME_GET_QMC_SETTINGS);
}

void rfdc_getQMCSettings(UINT32 type, INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;

    XRFdc_QMC_Settings qmc_settings;
    memset(&qmc_settings, 0, sizeof(XRFdc_QMC_Settings));

    retVal = mw_rfdc_get_QMC_Settings(type, tile_id, block_id, &qmc_settings);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] block[%d] QMC Settings:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10u|\r\n", "EnablePhase", qmc_settings.EnablePhase);
        fmsh_print("|%-40s|  %-10u|\r\n", "EnableGain", qmc_settings.EnableGain);
        fmsh_print("|%-40s|  %-10.4f|\r\n", "GainCorrectionFactor", qmc_settings.GainCorrectionFactor);
        fmsh_print("|%-40s|  %-10.4f|\r\n", "PhaseCorrectionFactor", qmc_settings.PhaseCorrectionFactor);
        fmsh_print("|%-40s|  %-10d|\r\n", "OffsetCorrectionFactor", qmc_settings.OffsetCorrectionFactor);
        switch (qmc_settings.EventSource)
        {
            case XRFDC_EVNT_SRC_IMMEDIATE: fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_IMMEDIATE");break;
            case XRFDC_EVNT_SRC_SLICE:     fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_SLICE");break;
            case XRFDC_EVNT_SRC_TILE:      fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_TILE");break;
            case XRFDC_EVNT_SRC_SYSREF:    fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_SYSREF");break;
            case XRFDC_EVNT_SRC_MARKER:    fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_MARKER");break;
            case XRFDC_EVNT_SRC_PL:        fmsh_print("|%-40s|%-10s|\r\n", "EventSource", "XRFDC_EVNT_SRC_PL");break;
            default:                       fmsh_print("Invalid EventSource\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_QMC_Settings failed, type=%s, tile_id=%d, block_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
    }
}

static void rfdc_setQMCSettings_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_QMC_SETTINGS, RFDC_INFO_SET_QMC_SETTINGS);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] [EnablePhase] [EnableGain] [GainCorrectionFactor] [PhaseCorrectionFactor] [OffsetCorrectionFactor] [EventSource] <-h/H>\r\n", RFDC_NAME_SET_QMC_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-EnablePhase -- indicates phase enable(1)/disable(0).\r\n");
    fmsh_print("\t-EnableGain -- indicates gain enable(1)/disable(0).\r\n");
    fmsh_print("\t-GainCorrectionFactor -- range: [0, 2.0)\r\n");
    fmsh_print("\t-PhaseCorrectionFactor -- range: (-26.5, 26.5), unit: degrees.\r\n");
    fmsh_print("\t-OffsetCorrectionFactor -- add a fixed LSB value to the sampled signal.\r\n");
    fmsh_print("\t-EventSource -- Event Source:\r\n");
    fmsh_print("\t                0, XRFDC_EVNT_SRC_IMMEDIATE\r\n");
    fmsh_print("\t                1, XRFDC_EVNT_SRC_SLICE\r\n");
    fmsh_print("\t                2, XRFDC_EVNT_SRC_TILE\r\n");
    fmsh_print("\t                3, XRFDC_EVNT_SRC_SYSREF\r\n");
    fmsh_print("\t                4, XRFDC_EVNT_SRC_MARKER\r\n");
    fmsh_print("\t                5, XRFDC_EVNT_SRC_PL\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 1 1 2.0 20.5 10 2\r\n\r\n", RFDC_NAME_SET_QMC_SETTINGS);
}

void rfdc_setQMCSettings(UINT32 type, UINT32 tile_id, UINT32 block_id, 
    UINT32 EnablePhase, UINT32 EnableGain, double GainCorrectionFactor, 
    double PhaseCorrectionFactor, INT32 OffsetCorrectionFactor, UINT32 EventSource)
{
    UINT32 retVal;

    XRFdc_QMC_Settings qmc_settings;
    memset(&qmc_settings, 0, sizeof(XRFdc_QMC_Settings));

    qmc_settings.EnablePhase = EnablePhase;
    qmc_settings.EnableGain = EnableGain;
    qmc_settings.GainCorrectionFactor = GainCorrectionFactor;
    qmc_settings.PhaseCorrectionFactor = PhaseCorrectionFactor;
    qmc_settings.OffsetCorrectionFactor = OffsetCorrectionFactor;
    qmc_settings.EventSource = EventSource;
    retVal = mw_rfdc_set_QMC_Settings(type, tile_id, block_id, &qmc_settings);
    fmsh_print("XRFDC Set QMC Settings %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getNyquistZone_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_NYQUIST_ZONE, RFDC_INFO_GET_NYQUIST_ZONE);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_NYQUIST_ZONE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 \r\n\r\n", RFDC_NAME_GET_NYQUIST_ZONE);
}

void rfdc_getNyquistZone(UINT32 type, INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT32 nyquistZone;

    retVal = mw_rfdc_get_Nyquist_Zone(type, tile_id, block_id, &nyquistZone);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] block[%d]:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (nyquistZone)
        {
            case XRFDC_ODD_NYQUIST_ZONE: fmsh_print("|%-40s|%-10s|\r\n", "NyquistZone", "Odd");break;
            case XRFDC_EVEN_NYQUIST_ZONE:fmsh_print("|%-40s|%-10s|\r\n", "NyquistZone", "Even");break;
            default:                     fmsh_print("Invalid nyquist_zone.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_Nyquist_Zone failed, type=%s, tile_id=%d, block_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
    }
}

static void rfdc_setNyquistZone_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_NYQUIST_ZONE, RFDC_INFO_SET_NYQUIST_ZONE);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] [nyquistZone] <-h/H>\r\n", RFDC_NAME_SET_NYQUIST_ZONE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-nyquistZone -- valid value: 1: Odd; 2: Even.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 1\r\n\r\n", RFDC_NAME_SET_NYQUIST_ZONE);
}

void rfdc_setNyquistZone(UINT32 type, UINT32 tile_id ,UINT32 block_id, UINT32 nyquistZone)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_Nyquist_Zone(type, tile_id, block_id, nyquistZone);
    fmsh_print("XRFDC Set Nyquist Zone %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getPwrMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_POWER_MODE, RFDC_INFO_GET_POWER_MODE);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_POWER_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 \r\n\r\n", RFDC_NAME_GET_POWER_MODE);
}

void rfdc_getPwrMode(UINT32 type, INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;

    XRFdc_Pwr_Mode_Settings pwr_mode;
    memset(&pwr_mode, 0, sizeof(XRFdc_Pwr_Mode_Settings));

    retVal = mw_rfdc_get_PwrMode(type, tile_id, block_id, &pwr_mode);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] block[%d] Power Mode:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10u|\r\n", "DisableIPControl", pwr_mode.DisableIPControl);
        switch (pwr_mode.PwrMode)
        {
            case XRFDC_PWR_MODE_OFF: fmsh_print("|%-40s|%-10s|\r\n", "PwrMode", "PowerMode_OFF");break;
            case XRFDC_PWR_MODE_ON:  fmsh_print("|%-40s|%-10s|\r\n", "PwrMode", "PowerMode_ON");break;
            default:                 fmsh_print("Invalid PwrMode.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_PwrMode failed, type=%s, tile_id=%d, block_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
    }
}

static void rfdc_setPwrMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_POWER_MODE, RFDC_INFO_SET_POWER_MODE);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] [DisableIPControl] [PwrMode] <-h/H>\r\n", RFDC_NAME_SET_POWER_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-DisableIPControl -- disable IP RTS control of the power mode.\r\n");
    fmsh_print("\t                     0, enable RTS control\r\n");
    fmsh_print("\t                     1, disable RTS control\r\n");
    fmsh_print("\t-PwrMode -- indicates power up(1)/power down(0).\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 0 1\r\n\r\n", RFDC_NAME_SET_POWER_MODE);
}

void rfdc_setPwrMode(UINT32 type, UINT32 tile_id, UINT32 block_id, UINT32 DisableIPControl, UINT32 PwrMode)
{
    UINT32 retVal;

    XRFdc_Pwr_Mode_Settings pwr_mode;
    memset(&pwr_mode, 0, sizeof(XRFdc_Pwr_Mode_Settings));

    pwr_mode.DisableIPControl = DisableIPControl;
    pwr_mode.PwrMode = PwrMode;
    retVal = mw_rfdc_set_PwrMode(type, tile_id, block_id, &pwr_mode);
    fmsh_print("XRFDC Set Power Mode %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getIntrStatus_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_INTERRUPT_STATUS, RFDC_INFO_GET_INTERRUPT_STATUS);
    fmsh_print("Synopsis: %s [type] [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_INTERRUPT_STATUS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0 \r\n\r\n", RFDC_NAME_GET_INTERRUPT_STATUS);
}

void rfdc_getIntrStatus(UINT32 type, INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT32 IntrStatus;

    retVal = mw_rfdc_get_IntrStatus(type, tile_id, block_id, &IntrStatus);
    if (OK == retVal)
    {
        fmsh_print("OK: %s tile[%d] block[%d]:\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|0x%-10x|\r\n", "IntrStatus", IntrStatus);
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_IntrStatus failed, type=%s, tile_id=%d, block_id=%d\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, block_id);
    }
}

static void rfdc_getDSA_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_DSA_SETTINGS, RFDC_INFO_GET_DSA_SETTINGS);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_DSA_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_DSA_SETTINGS);
}

void rfdc_getDSA(UINT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;

    XRFdc_DSA_Settings dsa_settings;
    memset(&dsa_settings, 0, sizeof(XRFdc_DSA_Settings));

    retVal = mw_rfdc_get_DSA(tile_id, block_id, &dsa_settings);
    if (OK == retVal)
    {
        fmsh_print("OK: ADC tile[%d] block[%d] DSA Settings:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|  %-10u|\r\n", "DisableRTS", dsa_settings.DisableRTS);
        fmsh_print("|%-40s|  %-10.4f|\r\n", "Attenuation", dsa_settings.Attenuation);
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_DSA failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setDSA_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_DSA_SETTINGS, RFDC_INFO_SET_DSA_SETTINGS);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [DisableRTS] [Attenuation] <-h/H>\r\n", RFDC_NAME_SET_DSA_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-DisableRTS -- Disable RTS control from setting attenuation.\r\n");
    fmsh_print("\t-Attenuation -- unit: dB\r\n");
    fmsh_print("\t                Range 0 - 11 dB with 0.5 dB resolution ES1 Si.\r\n");
    fmsh_print("\t                Range 0 - 27 dB with 1 dB resolution for Production Si.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1 1\r\n\r\n", RFDC_NAME_SET_DSA_SETTINGS);
}

void rfdc_setDSA(UINT32 tile_id,UINT32 block_id, UINT32 DisableRTS, float Attenuation)
{
    UINT32 retVal;

    XRFdc_DSA_Settings dsa_settings;
    memset(&dsa_settings, 0, sizeof(XRFdc_DSA_Settings));

    dsa_settings.DisableRTS = DisableRTS;
    dsa_settings.Attenuation = Attenuation;
    retVal = mw_rfdc_set_DSA(tile_id, block_id, &dsa_settings);
    fmsh_print("XRFDC Set DSA %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getSignalDetector_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_SIGNAL_DETECTOR, RFDC_INFO_GET_SIGNAL_DETECTOR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_SIGNAL_DETECTOR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_SIGNAL_DETECTOR);
}

void rfdc_getSignalDetector(UINT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;

    XRFdc_Signal_Detector_Settings signal_detector;
    memset(&signal_detector, 0, sizeof(XRFdc_Signal_Detector_Settings));

    retVal = mw_rfdc_get_SignalDetector(tile_id, block_id, &signal_detector);
    if (OK == retVal)
    {
        fmsh_print("OK: ADC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (signal_detector.Mode)
        {
            case XRFDC_SIGDET_MODE_AVG: fmsh_print("|%-40s|%-10s|\r\n", "Mode", "Average");break;
            case XRFDC_SIGDET_MODE_RNDM:fmsh_print("|%-40s|%-10s|\r\n", "Mode", "Random");break;
            default:                    fmsh_print("Invalid Mode.\r\n");break;
        }
        switch (signal_detector.TimeConstant)
        {
            case XRFDC_SIGDET_TC_2_0:  fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^0 Cycles");break;
            case XRFDC_SIGDET_TC_2_2:  fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^2 Cycles");break;
            case XRFDC_SIGDET_TC_2_4:  fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^4 Cycles");break;
            case XRFDC_SIGDET_TC_2_8:  fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^8 Cycles");break;
            case XRFDC_SIGDET_TC_2_12: fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^12 Cycles");break;
            case XRFDC_SIGDET_TC_2_14: fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^14 Cycles");break;
            case XRFDC_SIGDET_TC_2_16: fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^16 Cycles");break;
            case XRFDC_SIGDET_TC_2_18: fmsh_print("|%-40s|%-10s|\r\n", "TimeConstant", "2^18 Cycles");break;
            default:                   fmsh_print("Invalid TimeConstant.\r\n");break;
        }
        fmsh_print("|%-40s|  %-10u|\r\n", "Flush", signal_detector.Flush);
        fmsh_print("|%-40s|  %-10u|\r\n", "EnableIntegrator", signal_detector.EnableIntegrator);
        fmsh_print("|%-40s|  %-10u|\r\n", "Threshold", signal_detector.Threshold);
        fmsh_print("|%-40s|  %-10u|\r\n", "ThreshOnTriggerCnt", signal_detector.ThreshOnTriggerCnt);
        fmsh_print("|%-40s|  %-10u|\r\n", "ThreshOffTriggerCnt", signal_detector.ThreshOffTriggerCnt);
        fmsh_print("|%-40s|  %-10u|\r\n", "HysteresisEnable", signal_detector.HysteresisEnable);
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_SignalDetector failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setSignalDetector_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_SIGNAL_DETECTOR, RFDC_INFO_SET_SIGNAL_DETECTOR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [Mode] [TimeConstant] [Flush] [EnableIntegrator] [Threshold] [ThreshOnTriggerCnt] [ThreshOffTriggerCnt] [HysteresisEnable] <-h/H>\r\n", RFDC_NAME_SET_SIGNAL_DETECTOR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-Mode -- signal detected mode.\r\n");
    fmsh_print("\t         0: Average\r\n");
    fmsh_print("\t         1: Random\r\n");
    fmsh_print("\t-TimeConstant -- time constant of leaky integrator.\r\n");
    fmsh_print("\t                 0: 2^0 Cycles\r\n");
    fmsh_print("\t                 1: 2^2 Cycles\r\n");
    fmsh_print("\t                 2: 2^4 Cycles\r\n");
    fmsh_print("\t                 3: 2^8 Cycles\r\n");
    fmsh_print("\t                 4: 2^12 Cycles\r\n");
    fmsh_print("\t                 5: 2^14 Cycles\r\n");
    fmsh_print("\t                 6: 2^16 Cycles\r\n");
    fmsh_print("\t                 7: 2^18 Cycles\r\n");
    fmsh_print("\t-Flush -- Flush the leaky integrator.\r\n");
    fmsh_print("\t-EnableIntegrator -- Enable the leaky integrator.\r\n");
    fmsh_print("\t-Threshold -- the threshold for signal detection. unit: LSB\r\n");
    fmsh_print("\t-ThreshOnTriggerCnt -- num of times value must exceed threshold before turn on.\r\n");
    fmsh_print("\t-ThreshOffTriggerCnt -- num of times value must be less than threshold before turn off.\r\n");
    fmsh_print("\t-HysteresisEnable -- Enable hysteresis on signal on.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1 1 1 1 2 2 2 1\r\n\r\n", RFDC_NAME_SET_SIGNAL_DETECTOR);
}

void rfdc_setSignalDetector(UINT32 tile_id, UINT32 block_id, UINT32 Mode, 
    UINT32 TimeConstant, UINT32 Flush, UINT32 EnableIntegrator, 
    UINT32 Threshold, UINT32 ThreshOnTriggerCnt, UINT32 ThreshOffTriggerCnt, 
    UINT32 HysteresisEnable)
{
    UINT32 retVal;

    XRFdc_Signal_Detector_Settings signal_detector;
    memset(&signal_detector, 0, sizeof(XRFdc_Signal_Detector_Settings));

    signal_detector.Mode = Mode;
    signal_detector.TimeConstant = TimeConstant;
    signal_detector.Flush = Flush;
    signal_detector.EnableIntegrator = EnableIntegrator;
    signal_detector.Threshold = Threshold;
    signal_detector.ThreshOnTriggerCnt = ThreshOnTriggerCnt;
    signal_detector.ThreshOffTriggerCnt = ThreshOffTriggerCnt;
    signal_detector.HysteresisEnable = HysteresisEnable;
    retVal = mw_rfdc_set_SignalDetector(tile_id, block_id, &signal_detector);
    fmsh_print("XRFDC Set Signal Detector %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getCalibrationMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_CALIBRATION_MODE, RFDC_INFO_GET_CALIBRATION_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_CALIBRATION_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_CALIBRATION_MODE);
}

void rfdc_getCalibrationMode(UINT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;

    UINT8 CalibrationMode;

    retVal = mw_rfdc_get_CalibrationMode(tile_id, block_id, &CalibrationMode);
    if (OK == retVal)
    {
        fmsh_print("OK: ADC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (CalibrationMode)
        {
            case XRFDC_CALIB_MODE_AUTO: fmsh_print("|%-40s|%-10s|\r\n", "CalibrationMode", "XRFDC_CALIB_MODE_AUTO");break;
            case XRFDC_CALIB_MODE1:     fmsh_print("|%-40s|%-10s|\r\n", "CalibrationMode", "XRFDC_CALIB_MODE1");break;
            case XRFDC_CALIB_MODE2:     fmsh_print("|%-40s|%-10s|\r\n", "CalibrationMode", "XRFDC_CALIB_MODE2");break;
            default:                    fmsh_print("Invalid CalibrationMode.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_CalibrationMode failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setCalibrationMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_CALIBRATION_MODE, RFDC_INFO_SET_CALIBRATION_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [CalibrationMode] <-h/H>\r\n", RFDC_NAME_SET_CALIBRATION_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-CalibrationMode -- 0: XRFDC_CALIB_MODE_AUTO\r\n");
    fmsh_print("\t                    1: XRFDC_CALIB_MODE1, from 0.4*Fs to Fs/2\r\n");
    fmsh_print("\t                    2: XRFDC_CALIB_MODE2, from 0 to 0.4*Fs\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_CALIBRATION_MODE);
}

void rfdc_setCalibrationMode(UINT32 tile_id, UINT32 block_id, UINT8 CalibrationMode)
{
    UINT32 retVal;
    retVal = mw_rfdc_set_CalibrationMode(tile_id, block_id, CalibrationMode);
    fmsh_print("XRFDC Set Calibration Mode %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getLinkCoupling_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_LINKCOUPLING, RFDC_INFO_GET_LINKCOUPLING);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_LINKCOUPLING);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_LINKCOUPLING);
}

void rfdc_getLinkCoupling(UINT32 tile_id, INT32 block_id)
{
    UINT32 retVal;
    UINT32 LinkCoupling;

    retVal = mw_rfdc_get_LinkCoupling(tile_id, block_id, &LinkCoupling);
    if (OK == retVal)
    {
        fmsh_print("OK: ADC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (LinkCoupling)
        {
            case XRFDC_LINK_COUPLING_DC: fmsh_print("|%-40s|%-10s|\r\n", "LinkCoupling", "DC");break;
            case XRFDC_LINK_COUPLING_AC: fmsh_print("|%-40s|%-10s|\r\n", "LinkCoupling", "AC");break;
            default:                     fmsh_print("Invalid LinkCoupling.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_LinkCoupling failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_getDither_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_DITHER, RFDC_INFO_GET_DITHER);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_DITHER);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_DITHER);
}

void rfdc_getDither(UINT32 tile_id, INT32 block_id)
{
    UINT32 retVal;
    UINT32 Dither;
    CHAR *param_opt = NULL;

    retVal = mw_rfdc_get_Dither(tile_id, block_id, &Dither);
    if (OK == retVal)
    {
        fmsh_print("OK: ADC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (Dither)
        {
            case 0:  fmsh_print("|%-40s|%-10s|\r\n", "Dither", "Disable Dither");break;
            case 1:  fmsh_print("|%-40s|%-10s|\r\n", "Dither", "Enable Dither");break;
            default: fmsh_print("Invalid Dither.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_Dither failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setDither_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_DITHER, RFDC_INFO_SET_DITHER);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [Dither] <-h/H>\r\n", RFDC_NAME_SET_DITHER);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-Dither -- ADC dither mode.\r\n");
    fmsh_print("\t                   0: Disable\r\n");
    fmsh_print("\t                   1: Enable\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_DITHER);
}

void rfdc_setDither(UINT32 tile_id, INT32 block_id, UINT32 Dither)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_Dither(tile_id, block_id, Dither);
    fmsh_print("XRFDC Set Dither %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getDecimationFactor_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_DECIMATION_FACTOR, RFDC_INFO_GET_DECIMATION_FACTOR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_DECIMATION_FACTOR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_DECIMATION_FACTOR);
}

void rfdc_getDecimationFactor(UINT32 tile_id, INT32 block_id)
{
    UINT32 retVal;
    UINT32 DecimationFactor;

    retVal = mw_rfdc_get_DecimationFactor(tile_id, block_id, &DecimationFactor);
    if (OK == retVal)
    {
        fmsh_print("OK: ADC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (DecimationFactor)
        {
            case 0:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "Decimation is OFF");break;
            case 1:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "1X decimation factor");break;
            case 2:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "2X decimation factor");break;
            case 3:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "3X decimation factor");break;
            case 4:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "4X decimation factor");break;
            case 5:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "5X decimation factor");break;
            case 6:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "6X decimation factor");break;
            case 8:  fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "8X decimation factor");break;
            case 10: fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "10X decimation factor");break;
            case 12: fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "12X decimation factor");break;
            case 16: fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "16X decimation factor");break;
            case 20: fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "20X decimation factor");break;
            case 24: fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "24X decimation factor");break;
            case 40: fmsh_print("|%-40s|%-10s|\r\n", "DecimationFactor", "40X decimation factor");break;
            default: fmsh_print("Invalid DecimationFactor.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_DecimationFactor failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setDecimationFactor_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_DECIMATION_FACTOR, RFDC_INFO_SET_DECIMATION_FACTOR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [DecimationFactor] <-h/H>\r\n", RFDC_NAME_SET_DECIMATION_FACTOR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-DecimationFactor -- ADC decimation factor.\r\n");
    fmsh_print("\t                   0: Decimation is OFF\r\n");
    fmsh_print("\t                   1: 1X decimation factor\r\n");
    fmsh_print("\t                   2: 2X decimation factor\r\n");
    fmsh_print("\t                   3: 3X decimation factor\r\n");
    fmsh_print("\t                   4: 4X decimation factor\r\n");
    fmsh_print("\t                   5: 5X decimation factor\r\n");
    fmsh_print("\t                   6: 6X decimation factor\r\n");
    fmsh_print("\t                   8: 8X decimation factor\r\n");
    fmsh_print("\t                  10: 10X decimation factor\r\n");
    fmsh_print("\t                  12: 12X decimation factor\r\n");
    fmsh_print("\t                  16: 16X decimation factor\r\n");
    fmsh_print("\t                  20: 20X decimation factor\r\n");
    fmsh_print("\t                  24: 24X decimation factor\r\n");
    fmsh_print("\t                  40: 40X decimation factor\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_DECIMATION_FACTOR);
}

void rfdc_setDecimationFactor(UINT32 tile_id, UINT32 block_id, UINT32 DecimationFactor)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_DecimationFactor(tile_id, block_id, DecimationFactor);
    fmsh_print("XRFDC Set Decimation Factor %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getThresholdSettings_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_THRESHOLD_SETTINGS, RFDC_INFO_GET_THRESHOLD_SETTINGS);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_THRESHOLD_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_THRESHOLD_SETTINGS);
}

void rfdc_getThresholdSettings(UINT32 tile_id, INT32 block_id)
{
    UINT32 retVal;

    XRFdc_Threshold_Settings threshold_settings;
    memset(&threshold_settings, 0, sizeof(XRFdc_Threshold_Settings));

    retVal = mw_rfdc_get_ThresholdSettings(tile_id, block_id, &threshold_settings);
    if (OK == retVal)
    {
        fmsh_print("OK: ADC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (threshold_settings.UpdateThreshold)
        {
            case XRFDC_UPDATE_THRESHOLD_0:    fmsh_print("|%-40s|%-40s|\r\n", "UpdateThreshold", "Update for threshold0");break;
            case XRFDC_UPDATE_THRESHOLD_1:    fmsh_print("|%-40s|%-40s|\r\n", "UpdateThreshold", "Update for threshold1");break;
            case XRFDC_UPDATE_THRESHOLD_BOTH: fmsh_print("|%-40s|%-40s|\r\n", "UpdateThreshold", "Update for threshold0 and threshold1");break;
            default:                          fmsh_print("Invalid UpdateThreshold.\r\n");break;
        }
        for (size_t i = 0; i < 2; i++)
        {
            switch (threshold_settings.ThresholdMode[i])
            {
                case XRFDC_TRSHD_OFF:          fmsh_print("|%-40s|[%u]%-40s|\r\n", "ThresholdMode", i, "OFF");break;
                case XRFDC_TRSHD_STICKY_OVER:  fmsh_print("|%-40s|[%u]%-40s|\r\n", "ThresholdMode", i, "sticky-over");break;
                case XRFDC_TRSHD_STICKY_UNDER: fmsh_print("|%-40s|[%u]%-40s|\r\n", "ThresholdMode", i, "sticky-under");break;
                case XRFDC_TRSHD_HYSTERISIS:   fmsh_print("|%-40s|[%u]%-40s|\r\n", "ThresholdMode", i, "hysteresis");break;
                default:                       fmsh_print("Invalid ThresholdMode[%u].\r\n", i);break;
            }
        }
        for (size_t i = 0; i < 2; i++)
        {
            fmsh_print("|%-40s|[%u]%-10u|\r\n", "ThresholdAvgVal", i, threshold_settings.ThresholdAvgVal[i]);
        }
        for (size_t i = 0; i < 2; i++)
        {
            fmsh_print("|%-40s|[%u]%-10u|\r\n", "ThresholdUnderVal", i, threshold_settings.ThresholdUnderVal[i]);
        }
        for (size_t i = 0; i < 2; i++)
        {
            fmsh_print("|%-40s|[%u]%-10u|\r\n", "ThresholdOverVal", i, threshold_settings.ThresholdOverVal[i]);
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_ThresholdSettings failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setThresholdSettings_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_THRESHOLD_SETTINGS, RFDC_INFO_SET_THRESHOLD_SETTINGS);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [UpdateThreshold] [ThresholdMode:0~1] [ThresholdAvgVal:0~1] [ThresholdUnderVal:0~1] [ThresholdOverVal:0~1] <-h/H>\r\n", RFDC_NAME_SET_THRESHOLD_SETTINGS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-UpdateThreshold -- Select which threshold to update, in hex mode.\r\n");
    fmsh_print("\t                    0x1: Update for threshold0\r\n");
    fmsh_print("\t                    0x2: Update for threshold1\r\n");
    fmsh_print("\t                    0x4: Update for threshold0 and threshold1\r\n");
    fmsh_print("\t-ThresholdMode -- entry[0] for Threshold0, entry[1] for Threshold1. Range:[0~3]\r\n");
    fmsh_print("\t                 0: OFF\r\n");
    fmsh_print("\t                 1: sticky-over\r\n");
    fmsh_print("\t                 2: sticky-under\r\n");
    fmsh_print("\t                 3: hysteresis\r\n");
    fmsh_print("\t-ThresholdAvgVal -- Threshold average value. Mesured in 14-bit unsigned LSB.\r\n");
    fmsh_print("\t-ThresholdUnderVal -- Under threshold value. Mesured in 14-bit unsigned LSB.\r\n");
    fmsh_print("\t-ThresholdOverVal -- Over threshold value. Mesured in 14-bit unsigned LSB.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0x1 2 2 20 20 10 10 50 50\r\n\r\n", RFDC_NAME_SET_THRESHOLD_SETTINGS);
}

/*void rfdc_setThresholdSettings(UINT32 tile_id, UINT32 block_id, UINT32 UpdateThreshold, UINT32 ThresholdMode[0],UINT32 ThresholdMode[1],
                                      UINT32 ThresholdAvgVal[0], UINT32 ThresholdAvgVal[1], UINT32 ThresholdUnderVal[0],UINT32 ThresholdUnderVal[1], 
                                      UINT32 ThresholdOverVal[0] ,UINT32 ThresholdOverVal[1])
*/
void rfdc_setThresholdSettings(UINT32 tile_id, UINT32 block_id, XRFdc_Threshold_Settings threshold_settings)
{
    UINT32 retVal;

    // XRFdc_Threshold_Settings threshold_settings;
    memset(&threshold_settings, 0, sizeof(XRFdc_Threshold_Settings));
/*
    threshold_settings.UpdateThreshold = UpdateThreshold;
    memcpy(threshold_settings.ThresholdMode, ThresholdMode, sizeof(UINT32) * 2);
    memcpy(threshold_settings.ThresholdAvgVal, ThresholdAvgVal, sizeof(UINT32) * 2);
    memcpy(threshold_settings.ThresholdUnderVal, ThresholdUnderVal, sizeof(UINT32) * 2);
    memcpy(threshold_settings.ThresholdOverVal, ThresholdOverVal, sizeof(UINT32) * 2);
*/
    retVal = mw_rfdc_set_ThresholdSettings(tile_id, block_id, &threshold_settings);
    fmsh_print("XRFDC Set Threshold Settings %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_setThresholdClrMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_THRESHOLD_CLR_MODE, RFDC_INFO_SET_THRESHOLD_CLR_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [ThresholdToUpdate] [ClrMode] <-h/H>\r\n", RFDC_NAME_SET_THRESHOLD_CLR_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-UpdateThreshold -- Select which threshold to update, in hex mode.\r\n");
    fmsh_print("\t                    0x1: Update for threshold0\r\n");
    fmsh_print("\t                    0x2: Update for threshold1\r\n");
    fmsh_print("\t                    0x4: Update for threshold0 and threshold1\r\n");
    fmsh_print("\t-ClrMode -- Clear mode can be manual (register write) or auto clear(by update event)\r\n");
    fmsh_print("\t                 1: Manual clear\r\n");
    fmsh_print("\t                 2: Auto clear\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0x1 1\r\n\r\n", RFDC_NAME_SET_THRESHOLD_CLR_MODE);
}

void rfdc_setThresholdClrMode(UINT32 tile_id, UINT32 block_id, UINT32 ThresholdToUpdate, UINT32 ClrMode)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_ThresholdClrMode(tile_id, block_id, ThresholdToUpdate, ClrMode);
    fmsh_print("XRFDC Set threshold clear mode %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getDecoderMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_DECODER_MODE, RFDC_INFO_GET_DECODER_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_DECODER_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_DECODER_MODE);
}

void rfdc_getDecoderMode(UINT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT32 DecoderMode;

    retVal = mw_rfdc_get_DecoderMode(tile_id, block_id, &DecoderMode);
    if (OK == retVal)
    {
        fmsh_print("OK: DAC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (DecoderMode)
        {
            case XRFDC_DECODER_MAX_SNR_MODE:       fmsh_print("|%-40s|%-10s|\r\n", "DecoderMode", "MAX_SNR");break;
            case XRFDC_DECODER_MAX_LINEARITY_MODE: fmsh_print("|%-40s|%-10s|\r\n", "DecoderMode", "MAX_Linearity");break;
            default:                               fmsh_print("Invalid DecoderMode.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_DecoderMode failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setDecoderMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_DECODER_MODE, RFDC_INFO_SET_DECODER_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [DecoderMode] <-h/H>\r\n", RFDC_NAME_SET_DECODER_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-DecoderMode -- 1: Maximum SNR, for non-randomized decoder.\r\n");
    fmsh_print("\t                2: Maximum linearity, for randomized decoder.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_DECODER_MODE);
}

void rfdc_setDecoderMode(UINT32 tile_id, UINT32 block_id, UINT32 DecoderMode)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_DecoderMode(tile_id, block_id, DecoderMode);
    fmsh_print("XRFDC Set Decoder Mode %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getDataPathMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_DATAPATH_MODE, RFDC_INFO_GET_DATAPATH_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_DATAPATH_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_DATAPATH_MODE);
}

void rfdc_getDataPathMode(UINT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT32 DataPathMode;

    retVal = mw_rfdc_get_DataPathMode(tile_id, block_id, &DataPathMode);
    if (OK == retVal)
    {
        fmsh_print("OK: DAC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (DataPathMode)
        {
            case XRFDC_DATAPATH_MODE_DUC_0_FSDIVTWO:     fmsh_print("|%-40s|%-10s|\r\n", "DataPathMode", "Full Nyquist DUC");break;
            case XRFDC_DATAPATH_MODE_DUC_0_FSDIVFOUR:    fmsh_print("|%-40s|%-10s|\r\n", "DataPathMode", "IMR low pass");break;
            case XRFDC_DATAPATH_MODE_FSDIVFOUR_FSDIVTWO: fmsh_print("|%-40s|%-10s|\r\n", "DataPathMode", "IMR high pass");break;
            case XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO:   fmsh_print("|%-40s|%-10s|\r\n", "DataPathMode", "DUC bypass");break;
            default:                                     fmsh_print("Invalid DataPathMode.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_DataPathMode failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setDataPathMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_DATAPATH_MODE, RFDC_INFO_SET_DATAPATH_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [DataPathMode] <-h/H>\r\n", RFDC_NAME_SET_DATAPATH_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-DataPathMode -- 1: XRFDC_DATAPATH_MODE_DUC_0_FSDIVTWO, 0~Fs/2\r\n");
    fmsh_print("\t                 2: XRFDC_DATAPATH_MODE_DUC_0_FSDIVFOUR, 0~Fs/4\r\n");
    fmsh_print("\t                 3: XRFDC_DATAPATH_MODE_FSDIVFOUR_FSDIVTWO, Fs/4~Fs/2\r\n");
    fmsh_print("\t                 4: XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO, DUC bypass\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_DATAPATH_MODE);
}

void rfdc_setDataPathMode(UINT32 tile_id, UINT32 block_id, UINT32 DataPathMode)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_DataPathMode(tile_id, block_id, DataPathMode);
    fmsh_print("XRFDC Set Data Path Mode %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getIMRPassMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_IMR_PASS_MODE, RFDC_INFO_GET_IMR_PASS_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_IMR_PASS_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_IMR_PASS_MODE);
}

void rfdc_getIMRPassMode(UINT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT32 IMRPassMode;
   
    retVal = mw_rfdc_get_IMRPassMode(tile_id, block_id, &IMRPassMode);
    if (OK == retVal)
    {
        fmsh_print("OK: DAC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (IMRPassMode)
        {
            case XRFDC_DAC_IMR_MODE_LOWPASS:  fmsh_print("|%-40s|%-10s|\r\n", "IMRPassMode", "IMR low pass");break;
            case XRFDC_DAC_IMR_MODE_HIGHPASS: fmsh_print("|%-40s|%-10s|\r\n", "IMRPassMode", "IMR high pass");break;
            default:                          fmsh_print("Invalid IMRPassMode.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_IMRPassMode failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setIMRPassMode_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_IMR_PASS_MODE, RFDC_INFO_SET_IMR_PASS_MODE);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [IMRPassMode] <-h/H>\r\n", RFDC_NAME_SET_IMR_PASS_MODE);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-IMRPassMode -- 0: low pass filter\r\n");
    fmsh_print("\t                1: high pass filter\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_IMR_PASS_MODE);
}

void rfdc_setIMRPassMode(UINT32 tile_id, UINT32 block_id, UINT32 IMRPassMode)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_IMRPassMode(tile_id, block_id, IMRPassMode);
    fmsh_print("XRFDC Set IMR Filter Mode %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getOutputCurrent_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_OUTPUTCURRENT, RFDC_INFO_GET_OUTPUTCURRENT);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_OUTPUTCURRENT);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_OUTPUTCURRENT);
}

void rfdc_getOutputCurrent(INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT32 OutputCurent;

    retVal = mw_rfdc_get_OutputCurrent(tile_id, block_id, &OutputCurent);
    if (OK == retVal)
    {
        fmsh_print("OK: DAC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        fmsh_print("|%-40s|%-10u|\r\n", "OutputCurent[uA]", OutputCurent);
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_OutputCurrent failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setDACVOP_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_DAC_VOP, RFDC_INFO_SET_DAC_VOP);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [uACurrent] <-h/H>\r\n", RFDC_NAME_SET_DAC_VOP);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-uACurrent -- The current in uA. Value will be rounded to nearest increment.\r\n");
    fmsh_print("\t-             Range: [6425, 32000] for ES1 silicon.\r\n");
    fmsh_print("\t              Range: [2250, 40500] for production silicon.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 32000\r\n\r\n", RFDC_NAME_SET_DAC_VOP);
}

void rfdc_setDACVOP(INT32 tile_id, UINT32 block_id, UINT32 uACurrent)
{
    UINT32 retVal;

    retVal = mw_rfdc_set_DACVOP(tile_id, block_id, uACurrent);
    fmsh_print("XRFDC Set DAC VOP %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getInverseSincFIR_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_INVERSE_SINC_FIR, RFDC_INFO_GET_INVERSE_SINC_FIR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_INVERSE_SINC_FIR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_INVERSE_SINC_FIR);
}

void rfdc_getInverseSincFIR(INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT16 InverseSincFIR;

    retVal = mw_rfdc_get_InverseSincFIR(tile_id, block_id, &InverseSincFIR);
    if (OK == retVal)
    {
        fmsh_print("OK: DAC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (InverseSincFIR)
        {
            case 0: fmsh_print("|%-40s|%-10s|\r\n", "InverseSincFIR", "Disable");break;
            case 1: fmsh_print("|%-40s|%-10s|\r\n", "InverseSincFIR", "first Nyquist Zone");break;
            case 2: fmsh_print("|%-40s|%-10s|\r\n", "InverseSincFIR", "Second Nyquist Zone");break;
            default:fmsh_print("Invalid InverseSincFIR.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_InverseSincFIR failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setInverseSincFIR_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_INVERSE_SINC_FIR, RFDC_INFO_SET_INVERSE_SINC_FIR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [InverseSincFIR] <-h/H>\r\n", RFDC_NAME_SET_INVERSE_SINC_FIR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-InverseSincFIR -- inverse sinc filter status.\r\n");
    fmsh_print("\t                   0: Disable\r\n");
    fmsh_print("\t                   1: first Nyquist Zone\r\n");
    fmsh_print("\t                   2: Second Nyquist Zone\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_INVERSE_SINC_FIR);
}

void rfdc_setInverseSincFIR(UINT32 tile_id, UINT32 block_id, UINT16 InverseSincFIR)
{
    UINT32 retVal;
    retVal = mw_rfdc_set_InverseSincFIR(tile_id, block_id, InverseSincFIR);
    fmsh_print("XRFDC Set Inverse Sinc Filter %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_getInterpolationFactor_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_GET_INTERPOLATION_FACTOR, RFDC_INFO_GET_INTERPOLATION_FACTOR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] <-h/H>\r\n", RFDC_NAME_GET_INTERPOLATION_FACTOR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0\r\n\r\n", RFDC_NAME_GET_INTERPOLATION_FACTOR);
}

void rfdc_getInterpolationFactor(INT32 tile_id, UINT32 block_id)
{
    UINT32 retVal;
    UINT32 InterpolationFactor;

    retVal = mw_rfdc_get_InterpolationFactor(tile_id, block_id, &InterpolationFactor);
    if (OK == retVal)
    {
        fmsh_print("OK: DAC tile[%d] block[%d]:\r\n", tile_id, block_id);
        fmsh_print("================================================================\r\n");
        switch (InterpolationFactor)
        {
            case 0:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "Interpolation is OFF");break;
            case 1:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "1X interpolation factor");break;
            case 2:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "2X interpolation factor");break;
            case 3:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "3X interpolation factor");break;
            case 4:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "4X interpolation factor");break;
            case 5:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "5X interpolation factor");break;
            case 6:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "6X interpolation factor");break;
            case 8:  fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "8X interpolation factor");break;
            case 10: fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "10X interpolation factor");break;
            case 12: fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "12X interpolation factor");break;
            case 16: fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "16X interpolation factor");break;
            case 20: fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "20X interpolation factor");break;
            case 24: fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "24X interpolation factor");break;
            case 40: fmsh_print("|%-40s|%-10s|\r\n", "InterpolationFactor", "40X interpolation factor");break;
            default: fmsh_print("Invalid InterpolationFactor.\r\n");break;
        }
        fmsh_print("================================================================\r\n");
    }
    else
    {
        fmsh_print("mw_rfdc_get_InterpolationFactor failed, tile_id=%d, block_id=%d\r\n", tile_id, block_id);
    }
}

static void rfdc_setInterpolationFactor_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SET_INTERPOLATION_FACTOR, RFDC_INFO_SET_INTERPOLATION_FACTOR);
    fmsh_print("Synopsis: %s [tile_id] [block_id] [InterpolationFactor] <-h/H>\r\n", RFDC_NAME_SET_INTERPOLATION_FACTOR);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-tile_id -- tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-block_id -- block index, valid range: [0~%d].\r\n", XRFDC_BLOCK_ID_MAX);
    fmsh_print("\t-InterpolationFactor -- DAC interpolation factor.\r\n");
    fmsh_print("\t                   0: Interpolation is OFF\r\n");
    fmsh_print("\t                   1: 1X interpolation factor\r\n");
    fmsh_print("\t                   2: 2X interpolation factor\r\n");
    fmsh_print("\t                   3: 3X interpolation factor\r\n");
    fmsh_print("\t                   4: 4X interpolation factor\r\n");
    fmsh_print("\t                   5: 5X interpolation factor\r\n");
    fmsh_print("\t                   6: 6X interpolation factor\r\n");
    fmsh_print("\t                   8: 8X interpolation factor\r\n");
    fmsh_print("\t                  10: 10X interpolation factor\r\n");
    fmsh_print("\t                  12: 12X interpolation factor\r\n");
    fmsh_print("\t                  16: 16X interpolation factor\r\n");
    fmsh_print("\t                  20: 20X interpolation factor\r\n");
    fmsh_print("\t                  24: 24X interpolation factor\r\n");
    fmsh_print("\t                  40: 40X interpolation factor\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 1\r\n\r\n", RFDC_NAME_SET_INTERPOLATION_FACTOR);
}

void rfdc_setInterpolationFactor(INT32 tile_id, UINT32 block_id, UINT32 InterpolationFactor)
{
    UINT32 retVal;
 
    retVal = mw_rfdc_set_InterpolationFactor(tile_id, block_id, InterpolationFactor);
    fmsh_print("XRFDC Set Interpolation Factor %s!\r\n", (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_MultiTileSync_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_MTS, RFDC_INFO_MTS);
    fmsh_print("Synopsis: %s [type] [RefTile] [TilesBitMask] [TargetLatency] <-h/H>\r\n", RFDC_NAME_MTS);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-type -- indicates ADC/DAC: 0, RF-ADC ; 1, RF-DAC\r\n");
    fmsh_print("\t-RefTile -- reference tile index, valid range: [0~%d].\r\n", XRFDC_TILE_ID_MAX);
    fmsh_print("\t-TilesBitMask -- tiles to sync bit mask. BitX enables MTS for TileX. Tile0 must always be enabled.\r\n");
    fmsh_print("\t-TargetLatency -- set the target relative latency.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 0xF 296\r\n\r\n", RFDC_NAME_MTS);
}

void rfdc_MultiTileSync(UINT32 type, UINT32 RefTile, UINT32 TilesBitMask, INT32 TargetLatency)
{
    UINT32 retVal;

    XRFdc_MultiConverter_Sync_Config MTS_config;
    memset(&MTS_config, 0, sizeof(XRFdc_MultiConverter_Sync_Config));

    MTS_config.Tiles = TilesBitMask;
    MTS_config.RefTile = RefTile;
    MTS_config.Target_Latency = TargetLatency;
    MTS_config.SysRef_Enable = 1;
    retVal = mw_rfdc_run_MTS(type, &MTS_config, (INT32 *)NULL, (INT32 *)NULL);
    fmsh_print("XRFDC %s Multi tiles synchronization run %s!\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_readRegs_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_READ_REG, RFDC_INFO_READ_REG);
    fmsh_print("Synopsis: %s [BaseAddr] [RegAddr] [Mask] <-h/H>\r\n", RFDC_NAME_READ_REG);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-BaseAddr -- Address for a Block.\r\n");
    fmsh_print("\t-RegAddr -- Register offset address.\r\n");
    fmsh_print("\t-Mask -- Bit mask value.\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0x1c000 0x228 0xffff \r\n\r\n", RFDC_NAME_READ_REG);
}

void rfdc_readRegs(UINT32 BaseAddr, UINT32 RegAddr, UINT32 Mask)
{
    UINT32 retVal;
    UINT16 ReadReg;

    retVal= mw_rfdc_read_Reg(BaseAddr, RegAddr, Mask, &ReadReg);
    if (OK == retVal)
    {
        fmsh_print("Register[0x%x + 0x%x]=0x%x, Mask=0x%x\r\n", BaseAddr, RegAddr, ReadReg, Mask);
    }
    else
    {
        fmsh_print("mw_rfdc_read_Reg failed, BaseAddr=0x%x, RegAdrr=0x%x, Mask=0x%x\r\n", BaseAddr, RegAddr, Mask);
    }
}

static void rfdc_shutdown_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SHUTDOWN, RFDC_INFO_SHUTDOWN);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-Type -- is ADC or DAC. 0 for ADC and 1 for DAC.\r\n");
    fmsh_print("\t-Tile_Id -- Valid values are 0-3, and -1..\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 \r\n\r\n", RFDC_NAME_SHUTDOWN);
}

void rfdc_shutdown(UINT32 type, INT32 tile_id)
{
    UINT32 retVal;

    retVal= mw_rfdc_shutdown(type, tile_id);
    fmsh_print("%s tile[%d] shutdown %s\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, (retVal == OK) ? "Success" : "Failed");
}

static void rfdc_startup_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_STARTUP, RFDC_INFO_STARTUP);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-Type -- is ADC or DAC. 0 for ADC and 1 for DAC.\r\n");
    fmsh_print("\t-Tile_Id -- Valid values are 0-3, and -1..\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 \r\n\r\n", RFDC_NAME_STARTUP);
}

void rfdc_startup(UINT32 type, INT32 tile_id)
{
    UINT32 retVal;

    retVal= mw_rfdc_restart(type, tile_id);
    fmsh_print("%s tile[%d] startup %s\r\n", ((type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), tile_id, (retVal == OK) ? "Success" : "Failed");
}


static void rfdc_setclkdistribution_usage(void)
{
    fmsh_print("Name: %s - %s\r\n", RFDC_NAME_SETCLKDISTRIBUTION, RFDC_INFO_SETCLKDISTRIBUTION);
    fmsh_print("Param:\r\n");
    fmsh_print("\t-h/H  --  Show usage\r\n");
    fmsh_print("\t-Distribution_index  Valid values are 0-7 \r\n");
    fmsh_print("\t-ClkFreq  unit:MHz \r\n");
    fmsh_print("\t-row  -- is ADC or DAC. 0 for ADC and 1 for DAC.\r\n");
    fmsh_print("\t-col --  Valid values are 0-3.\r\n");
    fmsh_print("\t-samplerate unit:MHz\r\n");
    fmsh_print("Example:\r\n");
    fmsh_print("\t %s 0 0 \r\n\r\n", RFDC_NAME_STARTUP);
}

void rfdc_setclkdistribution(UINT32 row, UINT32 col, double ClkFreq, double samplerate)
{
    UINT32 retVal;
    UINT32 index;

    retVal= mw_rfdc_setclkdistribution(index,ClkFreq, samplerate,row,col);
    fmsh_print("%s \r\n", (retVal == OK) ? "Success" : "Failed");
}

void rfdc_getclkdistribution(void)
{
    UINT32 retVal = ERROR;
    int i;
    XRFdc_Distribution_System_Settings DistributionSystemSettingsPtr;
    XRFdc_Distribution_Settings DistributionSettingsPtr;


    retVal= mw_rfdc_getclkdistribution(&DistributionSystemSettingsPtr);
    if (retVal != OK)
    {
        fmsh_print("getclkdistribution Failed \r\n");
    }
    else 
    {
        for (i = 0; i < 8; i++ )
        {
            DistributionSettingsPtr = DistributionSystemSettingsPtr.Distributions[i];
            fmsh_print("DistributionSettingsPtr[%d]\r\n",i);
            fmsh_print("SourceType:%u SourceTileId:%u EdgeTileIds[0]:%u EdgeTileIds[1]:%u \r\n",DistributionSettingsPtr.SourceType,DistributionSettingsPtr.SourceTileId,DistributionSettingsPtr.EdgeTileIds[0],
            DistributionSettingsPtr.EdgeTileIds[1]);
            fmsh_print("EdgeTypes[0]:%u EdgeTypes[1]:%u DistRefClkFreq:%lf DistributedClock%u \r\n",DistributionSettingsPtr.EdgeTypes[0],DistributionSettingsPtr.EdgeTypes[1],
            DistributionSettingsPtr.DistRefClkFreq,DistributionSettingsPtr.DistributedClock);
            fmsh_print("SampleRates[0][0]:%lf SampleRates[0][1]:%lf SampleRates[0][2]:%lf SampleRates[0][3]:%lf \r\n",DistributionSettingsPtr.SampleRates[0][0],DistributionSettingsPtr.SampleRates[0][1],
            DistributionSettingsPtr.SampleRates[0][2],DistributionSettingsPtr.SampleRates[0][3]);
            fmsh_print("SampleRates[1][0]:%lf SampleRates[1][1]:%lf SampleRates[1][2]:%lf SampleRates[1][3]:%lf \r\n",DistributionSettingsPtr.SampleRates[1][0],DistributionSettingsPtr.SampleRates[1][1],
            DistributionSettingsPtr.SampleRates[1][2],DistributionSettingsPtr.SampleRates[1][3]);
            fmsh_print("ShutdownMode:%u MaxDelay:%hhu MinDelay:%hhu IsDelayBalanced:%hhu \r\n",DistributionSettingsPtr.ShutdownMode,DistributionSettingsPtr.Info.MaxDelay,DistributionSettingsPtr.Info.MinDelay,
            DistributionSettingsPtr.Info.IsDelayBalanced);
            fmsh_print("Source:%u UpperBound:%hhu LowerBound:%hhu \r\n",DistributionSettingsPtr.Info.Source,DistributionSettingsPtr.Info.UpperBound,DistributionSettingsPtr.Info.LowerBound);
            int m,n;
            for (m=0; m < 2; m++)
            {
                for (n=0;n < 4; n++)
                {
                    fmsh_print("\r\nXRFdc_Tile_Clock_Settings[%d][%d] \r\n",m,n);
                    fmsh_print("SourceType:%hhu SourceTile:%hhu PLLEnable:%u RefClkFreq:%lf \r\n",DistributionSettingsPtr.Info.ClkSettings[m][n].SourceType,DistributionSettingsPtr.Info.ClkSettings[m][n].SourceTile,
                    DistributionSettingsPtr.Info.ClkSettings[m][n].PLLEnable,DistributionSettingsPtr.Info.ClkSettings[m][n].RefClkFreq);
                    fmsh_print("SampleRate:%lf DivisionFactor:%hhu DistributedClock:%hhu Delay:%hhu \r\n",DistributionSettingsPtr.Info.ClkSettings[m][n].SampleRate,DistributionSettingsPtr.Info.ClkSettings[m][n].DivisionFactor,
                    DistributionSettingsPtr.Info.ClkSettings[m][n].DistributedClock,DistributionSettingsPtr.Info.ClkSettings[m][n].Delay);
                }
            }
            fmsh_print("\r\n");
        }
    } 
}

