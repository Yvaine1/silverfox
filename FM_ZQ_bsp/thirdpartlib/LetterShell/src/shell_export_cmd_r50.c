/* =============================================================
 * 文件作用：R50 端 shell 命令注册表（A53 转发 / R50 本地共用）
 *
 * 新增一条 R50 命令的步骤：
 *   1) 在 R50 工程的头文件中声明真实函数（原型）
 *   2) 在下方"命令注册区"加一行 SHELL_R50_FWD(attr, name, real_func, desc)
 *   3) 描述字符串格式："para:p1 p2 --- 说明"
 * ============================================================= */
#include "shell.h"

/* ============ Forwarder macros ============ */
#if (SHELL_CMD_MASTER == 0)
    #define SHELL_R50_FWD(_attr, _name, _func, _desc) \
        SHELL_EXPORT_CMD(((_attr) & ~SHELL_CMD_TYPE(0xF)) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), _name, _shell_fwd_r50, _desc)
    extern void _shell_fwd_r50(int argc, char *argv[]);
#endif
#if (SHELL_CMD_MASTER == 1)
    #define SHELL_R50_FWD(_attr, _name, _func, _desc) SHELL_EXPORT_CMD(_attr, _name, _func, _desc)
#endif


/* ============ R50 .h add here == */
#if (SHELL_CMD_MASTER == 1)

#include "fmsh_common.h"
#include "string.h"
#include "fmsh_sdmmc_example.h"
#include "eeprom_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "mem_common.h"
#include "cx4e04_config.h"
#include "xrfdc_main.h"
#include "dac4651_main.h"
#include "freq_offset_calibration.h"
#include "nst175_main.h"
#include "shell_cmd_l1c.h"
#include "gpio_main.h"
#include "set_trx_gain.h"
#include "dg_tod_utc.h"

#endif




/* ============ R50 command registrations (add new SHELL_R50_FWD here) ============ */
#if (SHELL_CMD_MASTER == 0) || (SHELL_CMD_MASTER == 1)
extern void metal_rfdc_init(void);


/*nst175 config func*/
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), nst175_get_temp, nst175_get_temp, para ---);
/*freq offset calibration*/
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_get_from_eeprom, freq_off_get_from_eeprom, para: page offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_get_from_cache, freq_off_get_from_cache, para: page offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_set_item, freq_off_set_item, para: page offset data);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_set_reserved, freq_off_set_reserved, para: page data);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_set_cnt, freq_off_set_cnt, para: page data);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_save_page_cfg, freq_off_save_page_cfg, para: apge);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_reset_page, freq_off_reset_page, para: page);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_show_page, freq_off_show_page, para: page);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), freq_off_pages_init, freq_off_pages_init, para ---);
/*dac4651 config func*/
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), get_dac4651_dac, get_dac4651_dac, para ---);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), set_dac4651_dac, set_dac4651_dac, para ---);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), set_dac4651_by_eeprom, set_dac4651_by_eeprom, para ---);
/*cx4e04 config func*/
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), cx4e04_pll_status, cx4e04_pll_status, para ---);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), cx4e04_read, cx4e04_read, para ---addr);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), cx4e04_write, cx4e04_write, para ---addr and dat);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), cx4e04_init, cx4e04_init, para ---);
/*rfdc config func*/
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_reset, rfdc_reset, para: type and tile_id);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_init, rfdc_init, para: device_id);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getTileStatus, rfdc_getTileStatus, para: type and tile_id);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getBlockStatus, rfdc_getBlockStatus, para: type and tile_id and block_id);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getMixerSettings, rfdc_getMixerSettings, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setMixerSettings, rfdc_setMixerSettings, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getpllConfig, rfdc_getpllConfig, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setpllConfig, rfdc_setpllConfig, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getpllLockStatus, rfdc_getpllLockStatus, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getQMCSettings, rfdc_getQMCSettings, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setQMCSettings, rfdc_setQMCSettings, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getNyquistZone, rfdc_getNyquistZone, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setNyquistZone, rfdc_setNyquistZone, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getPwrMode, rfdc_getPwrMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setPwrMode, rfdc_setPwrMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getIntrStatus, rfdc_getIntrStatus, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getDSA, rfdc_getDSA, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setDSA, rfdc_setDSA, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getSignalDetector, rfdc_getSignalDetector, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setSignalDetector, rfdc_setSignalDetector, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getCalibrationMode, rfdc_getCalibrationMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setCalibrationMode, rfdc_setCalibrationMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getLinkCoupling, rfdc_getLinkCoupling, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getDither, rfdc_getDither, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getDecimationFactor, rfdc_getDecimationFactor, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setDecimationFactor, rfdc_setDecimationFactor, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getThresholdSettings, rfdc_getThresholdSettings, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setThresholdSettings, rfdc_setThresholdSettings, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setThresholdClrMode, rfdc_setThresholdClrMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getDecoderMode, rfdc_getDecoderMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setDecoderMode, rfdc_setDecoderMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getDataPathMode, rfdc_getDataPathMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setDataPathMode, rfdc_setDataPathMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getIMRPassMode, rfdc_getIMRPassMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setIMRPassMode, rfdc_setIMRPassMode, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getOutputCurrent, rfdc_getOutputCurrent, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setDACVOP, rfdc_setDACVOP, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getInverseSincFIR, rfdc_getInverseSincFIR, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setInverseSincFIR, rfdc_setInverseSincFIR, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getInterpolationFactor, rfdc_getInterpolationFactor, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setInterpolationFactor, rfdc_setInterpolationFactor, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_MultiTileSync, rfdc_MultiTileSync, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_shutdown, rfdc_shutdown, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_readRegs, rfdc_readRegs, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_startup, rfdc_startup, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_setclkdistribution, rfdc_setclkdistribution, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), rfdc_getclkdistribution, rfdc_getclkdistribution, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), metal_rfdc_writeRegs, metal_rfdc_writeRegs, para: offset and data);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), metal_rfdc_readRegs, metal_rfdc_readRegs, para: offset);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), metal_rfdc_init, metal_rfdc_init, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), start_ul_stat, start_ul_stat, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), stop_ul_stat, stop_ul_stat, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), start_dl_stat, start_dl_stat, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), stop_dl_stat, stop_dl_stat, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), l1c_slot_log, l1c_slot_log, para: log_fn and log_sfn and log_slotBitmap2 and log_slotBitmap1 and log_slotBitmap0);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), l1c_slot_log_flag, l1c_slot_log_flag, para: log_flag);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), l1c_data_cap_flag, l1c_data_cap_flag, para: log_flag);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), get_hw_id, get_hw_id, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), eeprom_set_tx_gain, eeprom_set_tx_gain, para: data);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), eeprom_set_rx_gain, eeprom_set_rx_gain, para: data);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), eeprom_get_tx_gain, eeprom_get_tx_gain, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), eeprom_get_rx_gain, eeprom_get_rx_gain, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), open_pusch_payload_log, open_pusch_payload_log, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), close_pusch_payload_log, close_pusch_payload_log, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), open_pdsch_payload_log, open_pdsch_payload_log, para:);
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), close_pdsch_payload_log, close_pdsch_payload_log, para:);
#ifdef R50_UTC_TEST
SHELL_R50_FWD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), r50_get_utc_time, r50_get_utc_time, para:);
#endif

#endif