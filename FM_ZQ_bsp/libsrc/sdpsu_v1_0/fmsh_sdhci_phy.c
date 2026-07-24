#include <math.h>
#include <string.h>

#include "fmsh_sdhci.h"
#include "fmsh_sdhci_hw.h"

struct dll_phy_output {
    unsigned int cp_clk_wr_delay;         // phy_dll_slave_ctrl_reg
    unsigned int cp_clk_wrdqs_delay;      // phy_dll_slave_ctrl_reg
    unsigned int cp_data_select_oe_end;   // phy_dq_timing_reg
    unsigned int cp_dll_bypass_mode;      // phy_dll_master_ctrl_reg
    unsigned int cp_dll_locked_mode;
    unsigned int cp_dll_start_point;      // phy_dll_master_ctrl_reg
    unsigned int cp_gate_cfg_always_on;   // phy_gate_lpbk_ctrl_reg
    unsigned int cp_io_mask_always_on;    // phy_dq_timing_reg
    unsigned int cp_io_mask_end;          // phy_dq_timing_reg
    unsigned int cp_io_mask_start;        // phy_dq_timing_reg
    unsigned int cp_rd_del_sel;           // phy_gate_lpbk_ctrl_reg
    unsigned int cp_read_dqs_cmd_delay;   // phy_dll_slave_ctrl_reg
    unsigned int cp_read_dqs_delay;       // phy_dll_slave_ctrl_reg
    unsigned int cp_sw_half_cycle_shift;  // phy_wr_deskew_pd_ctrl_0_reg
    unsigned int cp_sync_method;          // phy_gate_lpbk_ctrl_reg
    unsigned int cp_underrun_supress;     // phy_gate_lpbk_ctrl_reg
    unsigned int cp_use_ext_lpbk_dqs;     // phy_dqs_timing_reg
    unsigned int cp_use_lpbk_dqs;         // phy_dqs_timing_reg
    unsigned int cp_use_phony_dqs;        // phy_dqs_timing_reg
    unsigned int cp_use_phony_dqs_cmd;    // phy_dqs_timing_reg
    unsigned int sdhc_extended_wr_mode;   // hrs09
    unsigned int sdhc_extended_rd_mode;   // hrs09
    unsigned int sdhc_hcsdclkadj;         // hrs10
    unsigned int sdhc_idelay_val;         // hrs07
    unsigned int sdhc_rdcmd_en;           // hrs09
    unsigned int sdhc_rddata_en;          // hrs09
    unsigned int sdhc_rw_compensate;      // hrs07
    unsigned int sdhc_wrcmd0_dly;         // hrs16
    unsigned int sdhc_wrcmd0_sdclk_dly;   // hrs16
    unsigned int sdhc_wrcmd1_dly;         // hrs16
    unsigned int sdhc_wrcmd1_sdclk_dly;   // hrs16
    unsigned int sdhc_wrdata0_dly;        // hrs16
    unsigned int sdhc_wrdata0_sdclk_dly;  // hrs16
    unsigned int sdhc_wrdata1_dly;        // hrs16
    unsigned int sdhc_wrdata1_sdclk_dly;  // hrs16
};

#ifdef DLL_PHY_PRESET_VALUE

struct dll_phy_output sd_ds_id = {
    0,   // int cp_clk_wr_delay;//phy_dll_slave_ctrl_reg
    0,   // int cp_clk_wrdqs_delay;//phy_dll_slave_ctrl_reg
    1,   // int cp_data_select_oe_end;//phy_dq_timing_reg
    1,   // int cp_dll_bypass_mode;//phy_dll_master_ctrl_reg
    3,   // int cp_dll_locked_mode;
    4,   // int cp_dll_start_point;//phy_dll_master_ctrl_reg
    1,   // int cp_gate_cfg_always_on;//phy_gate_lpbk_ctrl_reg
    0,   // int cp_io_mask_always_on;//phy_dq_timing_reg
    0,   // int cp_io_mask_end;//phy_dq_timing_reg
    0,   // int cp_io_mask_start;//phy_dq_timing_reg
    52,  // int cp_rd_del_sel;//phy_gate_lpbk_ctrl_reg
    0,   // int cp_read_dqs_cmd_delay;//phy_dll_slave_ctrl_reg
    0,   // int cp_read_dqs_delay;//phy_dll_slave_ctrl_reg
    0,   // int cp_sw_half_cycle_shift;//phy_wr_deskew_pd_ctrl_0_reg
    1,   // int cp_sync_method;//phy_gate_lpbk_ctrl_reg
    1,   // int cp_underrun_supress;//phy_gate_lpbk_ctrl_reg
    0,   /// 1 //int cp_use_ext_lpbk_dqs;//phy_dqs_timing_reg
    1,   // int cp_use_lpbk_dqs;//phy_dqs_timing_reg
    1,   // int cp_use_phony_dqs;//phy_dqs_timing_reg
    1,   // int cp_use_phony_dqs_cmd;//phy_dqs_timing_reg
    1,   // int sdhc_extended_wr_mode;//hrs09
    1,   // int sdhc_extended_rd_mode;//hrs09
    1,   // int sdhc_hcsdclkadj;//hrs10
    0,   // int sdhc_idelay_val;//hrs07
    1,   // int sdhc_rdcmd_en;//hrs09
    1,   // int sdhc_rddata_en;//hrs09
    8,   // int sdhc_rw_compensate;//hrs07
    0,   // int sdhc_wrcmd0_dly;//hrs16
    0,   // int sdhc_wrcmd0_sdclk_dly;//hrs16
    0,   // int sdhc_wrcmd1_dly;//hrs16
    0,   // int sdhc_wrcmd1_sdclk_dly;//hrs16
    0,   // int sdhc_wrdata0_dly;//hrs16
    0,   // int sdhc_wrdata0_sdclk_dly;//hrs16
    0,   // int sdhc_wrdata1_dly;//hrs16
    0,   // int sdhc_wrdata1_sdclk_dly;//hrs16
};

struct dll_phy_output output = {
    0,   // 255,//int cp_clk_wr_delay;//phy_dll_slave_ctrl_reg
    0,   // 255,// int cp_clk_wrdqs_delay;//phy_dll_slave_ctrl_reg
    1,   // int cp_data_select_oe_end;//phy_dq_timing_reg
    1,   // int cp_dll_bypass_mode;//phy_dll_master_ctrl_reg
    3,   // int cp_dll_locked_mode;
    4,   // int cp_dll_start_point;//phy_dll_master_ctrl_reg
    1,   // int cp_gate_cfg_always_on;//phy_gate_lpbk_ctrl_reg
    0,   // int cp_io_mask_always_on;//phy_dq_timing_reg
    0,   // int cp_io_mask_end;//phy_dq_timing_reg
    0,   // int cp_io_mask_start;//phy_dq_timing_reg
    52,  // int cp_rd_del_sel;//phy_gate_lpbk_ctrl_reg
    0,   // int cp_read_dqs_cmd_delay;//phy_dll_slave_ctrl_reg
    0,   // int cp_read_dqs_delay;//phy_dll_slave_ctrl_reg
    0,   // int cp_sw_half_cycle_shift;//phy_wr_deskew_pd_ctrl_0_reg
    1,   // int cp_sync_method;//phy_gate_lpbk_ctrl_reg
    1,   // int cp_underrun_supress;//phy_gate_lpbk_ctrl_reg
    0,   /// 1,//int cp_use_ext_lpbk_dqs;//phy_dqs_timing_reg
    1,   // int cp_use_lpbk_dqs;//phy_dqs_timing_reg
    1,   // int cp_use_phony_dqs;//phy_dqs_timing_reg
    1,   // int cp_use_phony_dqs_cmd;//phy_dqs_timing_reg
    1,   // 0, //int sdhc_extended_wr_mode;//hrs09
    1,   // 0, //int sdhc_extended_rd_mode;//hrs09
    5,   // 9, //int sdhc_hcsdclkadj;//hrs10
    0,   /// 0//int sdhc_idelay_val;//hrs07
    1,   // int sdhc_rdcmd_en;//hrs09
    1,   // int sdhc_rddata_en;//hrs09
    8,   // 9, //int sdhc_rw_compensate;//hrs07
    0,   // 1, //int sdhc_wrcmd0_dly;//hrs16
    0,   // int sdhc_wrcmd0_sdclk_dly;//hrs16
    0,   // int sdhc_wrcmd1_dly;//hrs16
    0,   // int sdhc_wrcmd1_sdclk_dly;//hrs16
    0,   // 1, //int sdhc_wrdata0_dly;//hrs16
    0,   // int sdhc_wrdata0_sdclk_dly;//hrs16
    0,   // int sdhc_wrdata1_dly;//hrs16
    0,   // int sdhc_wrdata1_sdclk_dly;//hrs16
};

#else

#define FULL_CLOCK_MODE 0
#define HALF_CLOCK_MODE 2
#define SATURATION_MODE 3
#define DLL_ENABLE      0
#define DLL_DISABLE     1

#define DELAY_ELEMENT   28

struct self_param {
    u32 mode;
    u32 sdmclk_ps;
    u32 sdclk_ps;
    u32 iocell_output_delay;
    u32 iocell_input_delay;
    u32 delay_element;

    u32 tune_cmd : 1;
    u32 tune_dat : 1;
    u32 strobe_dat : 1;
    u32 strobe_cmd : 1;
    u32 ddr : 1;
    u32 sdr : 1;
    u32 dll_locked_in_full_clock_mode : 2;
    u32 dll_locked_in_half_clock_mode : 2;
    u32 dll_in_bypass_mode : 2;

    u32 sdclk_ps_min;
    u32 sdclk_ps_max;
    u32 sdmclk_calc_ps;
    u32 phy_sdclk_delay;
    u32 dll_max_value;

    u32 t_cmd_input_min;
    u32 t_cmd_input_max;
    u32 t_cmd_output_min;
    u32 t_cmd_output_max;
    u32 phy_cmd_o_delay;

    u32 t_dat_input_min;
    u32 t_dat_input_max;
    u32 t_dat_output_min;
    u32 t_dat_output_max;
    u32 phy_dat_o_delay;
};

static __no_init struct dll_phy_output output;
static __no_init struct self_param self;

static int lock_dll_in_full_mode (void)
{
    u32 element_in_sdmclk;

    self.delay_element = DELAY_ELEMENT;

    element_in_sdmclk = (u32)ceil(self.sdmclk_ps / self.delay_element);
    self.sdmclk_calc_ps = self.delay_element * element_in_sdmclk;

    if (element_in_sdmclk > 256)
    {
        return 0;
    }

    return 1;
}

static int lock_dll_in_half_mode (void)
{
    u32 element_in_sdmclk;

    self.delay_element = 2 * DELAY_ELEMENT;

    element_in_sdmclk = (u32)ceil((float)self.sdmclk_ps / self.delay_element);
    self.sdmclk_calc_ps = self.delay_element * element_in_sdmclk;

    if (element_in_sdmclk > 256)
    {
        return 0;
    }

    return 1;
}

static int enable_dll_bypass_mode (void)
{
    self.delay_element = DELAY_ELEMENT;
    self.dll_in_bypass_mode = 1;
    self.dll_max_value = 256;

    output.cp_dll_bypass_mode = DLL_DISABLE;
    output.cp_dll_locked_mode = SATURATION_MODE;

    return 0;
}

static int clock_setting (void)
{
    if (self.sdmclk_ps == self.sdclk_ps)
    {
        output.sdhc_extended_wr_mode = 0;
        output.sdhc_extended_rd_mode = 0;
    }
    else
    {
        output.sdhc_extended_wr_mode = 1;
        output.sdhc_extended_rd_mode = 1;
    }

    // configure_dll
    if (output.sdhc_extended_wr_mode)
    {
        (void)enable_dll_bypass_mode();
    }
    else if (lock_dll_in_full_mode())
    {
        self.dll_locked_in_full_clock_mode = 1;
        self.dll_max_value = 255;

        output.cp_dll_bypass_mode = DLL_ENABLE;
        output.cp_dll_locked_mode = FULL_CLOCK_MODE;
    }
    else if (lock_dll_in_half_mode())
    {
        self.dll_locked_in_half_clock_mode = 1;
        self.dll_max_value = 127;

        output.cp_dll_bypass_mode = DLL_ENABLE;
        output.cp_dll_locked_mode = HALF_CLOCK_MODE;
    }
    else
    {
        (void)enable_dll_bypass_mode();
    }

    return 0;
}

static int calculate_cmd_out (void)
{
    int t;
    u32 n;
    u32 dll_cmd_wr_delay;

    t = self.phy_cmd_o_delay - self.phy_sdclk_delay;
    t -= self.t_cmd_output_min;

    output.cp_clk_wrdqs_delay = 0;

    if (t < 0)
    {
        // delay write CMD
        if (output.sdhc_extended_wr_mode)
        {
            u32 n_half_cycle = (u32)ceil((float)-t * 2 / self.sdmclk_ps);

            output.sdhc_wrcmd0_dly = (n_half_cycle + 1) / 2;
            output.sdhc_wrcmd1_dly = ((n_half_cycle + 1) % 2) +
                                     output.sdhc_wrcmd0_dly - 1;
        }
    }

    if (output.sdhc_extended_wr_mode == 0)
    {
        output.sdhc_wrcmd0_dly = 1;

        u32 t_cmd_o_setup = self.t_cmd_output_max;
        u32 t_cmd_o_hold = self.t_cmd_output_min;
        u32 t_cmd_o_hold_margin = (u32)(0.25 * (t_cmd_o_setup - t_cmd_o_hold));

        t_cmd_o_hold = (u32)ceil(t_cmd_o_hold + t_cmd_o_hold_margin);

        u32 no_dll_setting = 0;

        if (self.dll_locked_in_full_clock_mode ||
            self.dll_locked_in_half_clock_mode)
        {
            n = (u32)ceil((float)256 * t_cmd_o_hold / self.sdmclk_calc_ps);
            if (n <= self.dll_max_value)
            {
                dll_cmd_wr_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else if (self.dll_in_bypass_mode)
        {
            n = (u32)ceil((float)t_cmd_o_hold / self.delay_element) - 1;
            if (n <= self.dll_max_value)
            {
                dll_cmd_wr_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else{
            ;/* no deal with */
        }
        
        if (no_dll_setting)
        {
            dll_cmd_wr_delay = 255;
        }
    }
    else
    {
        dll_cmd_wr_delay = 0;
    }

    output.cp_clk_wrdqs_delay = dll_cmd_wr_delay;

    return 0;
}

static int calculate_cmd_in (void)
{
    output.cp_gate_cfg_always_on = 1;
    output.cp_underrun_supress = 1;
    output.cp_io_mask_always_on = 0;
    output.cp_io_mask_end = (int)floor(
        (2 * (self.iocell_output_delay + self.iocell_input_delay)) /
        self.sdmclk_ps);
    if (output.cp_io_mask_end >= 8)
    {
        output.cp_io_mask_end = 7;
    }
    if (self.strobe_cmd)
    {
        if (output.cp_io_mask_end > 0)
        {
            output.cp_io_mask_end -= 1;
        }
    }
    output.cp_sync_method = 1;
    output.cp_rd_del_sel = 52;
    output.cp_use_ext_lpbk_dqs = 1;  // 0;
    output.cp_use_lpbk_dqs = 1;
    if (self.strobe_cmd)
    {
        output.cp_use_phony_dqs_cmd = 0;
    }
    else
    {
        output.cp_use_phony_dqs_cmd = 1;
    }

    // Strobe CMD line with strobe shifted by 1/4 clock cycle
    if (self.strobe_cmd)
    {
        output.cp_read_dqs_cmd_delay = 64;
    }
    else
    {
        u32 n;
        u32 t_cmd_i_setup;
        if (self.ddr)
        {
            t_cmd_i_setup = self.sdclk_ps / 2 - self.t_cmd_input_min;
        }
        else
        {
            t_cmd_i_setup = self.sdclk_ps - self.t_cmd_input_min;
        }
        // u32 t_cmd_i_hold = self.t_cmd_input_max - self.sdclk_ps;
        u32 t_cmd_i_hold_margin = (u32)(0.25 * (t_cmd_i_setup));

        u32 no_dll_setting = 0;

        if (self.dll_locked_in_full_clock_mode ||
            self.dll_locked_in_half_clock_mode)
        {
            n = (u32)ceil((float)256 * t_cmd_i_hold_margin /
                          self.sdmclk_calc_ps);
            if (n <= self.dll_max_value)
            {
                output.cp_read_dqs_cmd_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else if (self.dll_in_bypass_mode)
        {
            n = (u32)ceil((float)t_cmd_i_hold_margin / self.delay_element) - 1;
            if (n <= self.dll_max_value)
            {
                output.cp_read_dqs_cmd_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else{
            ;/* no deal with */
        }

        if (no_dll_setting)
        {
            output.cp_read_dqs_cmd_delay = 255;
        }
    }

    return 0;
}

static int calculate_data_out (void)
{
    int t;
    u32 n;
    u32 dll_dat_wr_delay;

    t = self.phy_dat_o_delay - self.phy_sdclk_delay;
    t -= self.t_dat_output_min;

    output.cp_clk_wr_delay = 0;

    if (self.ddr)
    {
        output.sdhc_wrdata0_sdclk_dly = 1;
        output.sdhc_wrdata1_sdclk_dly = 1;
    }

    if (t < 0)
    {
        if (output.sdhc_extended_wr_mode == 1)
        {
            u32 n_half_cycle = (u32)ceil((float)-t * 2 / self.sdmclk_ps);

            if (self.ddr)
            {
                output.sdhc_wrdata0_dly = (n_half_cycle + 1) / 2;
                output.sdhc_wrdata1_dly = (n_half_cycle + 1) / 2;
            }
            else
            {
                output.sdhc_wrdata0_dly = (n_half_cycle + 1) / 2;
                output.sdhc_wrdata1_dly = ((n_half_cycle + 1) % 2) +
                                          output.sdhc_wrdata0_dly - 1;
            }
        }
    }

    if (output.sdhc_extended_wr_mode == 0)
    {
        if (self.sdr)
        {
            output.sdhc_wrdata0_dly = 1;
        }

        u32 t_dat_o_setup = self.t_dat_output_max;
        u32 t_dat_o_hold = self.t_dat_output_min;
        u32 t_dat_o_hold_margin = (u32)(0.25 * (t_dat_o_setup - t_dat_o_hold));

        t_dat_o_hold = (u32)ceil((float)t_dat_o_hold + t_dat_o_hold_margin);

        u32 no_dll_setting = 0;

        if (self.dll_locked_in_full_clock_mode ||
            self.dll_locked_in_half_clock_mode)
        {
            n = (u32)ceil((float)256 * t_dat_o_hold / self.sdmclk_calc_ps);
            if (n <= self.dll_max_value)
            {
                dll_dat_wr_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else if (self.dll_in_bypass_mode)
        {
            n = (u32)ceil((float)t_dat_o_hold / self.delay_element) - 1;
            if (n <= self.dll_max_value)
            {
                dll_dat_wr_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else{
            ;/* no deal with */
        }

        if (no_dll_setting)
        {
            dll_dat_wr_delay = 255;
        }

        output.cp_clk_wr_delay = dll_dat_wr_delay;
    }
    else
    {
        output.cp_clk_wr_delay = 0;
    }

    return 0;
}

static int calculate_data_in (void)
{
    u32 n, hcsdclkadj, sdhc_hcsdclkadj;

    if (self.strobe_dat)
    {
        output.cp_use_phony_dqs = 0;
    }
    else
    {
        output.cp_use_phony_dqs = 1;
    }

    if (self.strobe_dat)
    {
        output.cp_read_dqs_delay = 64;
    }
    else
    {
        u32 n;
        u32 t_dat_i_setup;
        if (self.ddr)
        {
            t_dat_i_setup = self.sdclk_ps / 2 - self.t_dat_input_min;
        }
        else
        {
            t_dat_i_setup = self.sdclk_ps - self.t_dat_input_min;
        }
        // u32 t_dat_i_hold = self.t_dat_input_max - self.sdclk_ps;
        u32 t_dat_i_hold_margin = (u32)(0.25 * (t_dat_i_setup));

        u32 no_dll_setting = 0;

        if (self.dll_locked_in_full_clock_mode ||
            self.dll_locked_in_half_clock_mode)
        {
            n = (u32)ceil((float)256 * t_dat_i_hold_margin /
                          self.sdmclk_calc_ps);
            if (n <= self.dll_max_value)
            {
                output.cp_read_dqs_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else if (self.dll_in_bypass_mode)
        {
            n = (u32)ceil((float)t_dat_i_hold_margin / self.delay_element) - 1;
            if (n <= self.dll_max_value)
            {
                output.cp_read_dqs_delay = n;
            }
            else
            {
                no_dll_setting = 1;
            }
        }
        else
        {
            
        }

        if (no_dll_setting)
        {
            output.cp_read_dqs_delay = 255;
        }
    }

    if (self.strobe_dat)
    {
        hcsdclkadj = 0;
        hcsdclkadj += self.iocell_input_delay;
        hcsdclkadj += floor(self.delay_element / 2);
        hcsdclkadj += self.sdclk_ps / 2;
        hcsdclkadj += self.sdclk_ps / 2 + self.delay_element;
        hcsdclkadj += ceil((float)hcsdclkadj / self.sdmclk_ps) *
                          self.sdmclk_ps -
                      hcsdclkadj;
        hcsdclkadj += self.sdmclk_ps * 5;
        hcsdclkadj += self.sdclk_ps;

        sdhc_hcsdclkadj = (u32)floor(hcsdclkadj /
                                     self.sdclk_ps);  // # - 1 # - 2;
    }
    else
    {
        hcsdclkadj = 0;
        hcsdclkadj += 2 * self.sdmclk_ps;
        hcsdclkadj += self.iocell_output_delay;
        hcsdclkadj += self.iocell_input_delay;
        hcsdclkadj += floor(self.delay_element / 2);
        hcsdclkadj += self.delay_element;
        hcsdclkadj += floor(self.delay_element / 2);
        hcsdclkadj += 0;
        if (self.sdmclk_ps == self.sdclk_ps)
        {
            n = (hcsdclkadj - 2 * self.sdmclk_ps) / self.sdclk_ps;
        }
        else
        {
            n = hcsdclkadj / self.sdclk_ps;
        }

        hcsdclkadj = hcsdclkadj % self.sdclk_ps;
        hcsdclkadj += self.sdclk_ps / 2;
        hcsdclkadj += (ceil((float)hcsdclkadj / self.sdmclk_ps) *
                           self.sdmclk_ps -
                       hcsdclkadj);
        hcsdclkadj += self.sdmclk_ps;
        hcsdclkadj += self.sdmclk_ps;
        hcsdclkadj += self.sdmclk_ps;
        hcsdclkadj += self.sdmclk_ps;

        if ((self.sdclk_ps / self.sdmclk_ps) > 1)
        {
            u32 tmp1 = hcsdclkadj;
            u32 tmp2 = (u32)floor(tmp1 / self.sdclk_ps) * self.sdclk_ps +
                       self.sdclk_ps - self.sdmclk_ps;
            if (tmp1 == tmp2)
            {
                tmp2 += self.sdclk_ps;
            }
            u32 tmp = tmp2 - tmp1;
            hcsdclkadj += tmp;
        }

        hcsdclkadj += self.sdmclk_ps;
        hcsdclkadj += self.sdclk_ps;

        sdhc_hcsdclkadj = (u32)floor(hcsdclkadj / self.sdclk_ps);  // # - 1 # -
                                                                   // 2

        sdhc_hcsdclkadj += n;

        if ((self.sdclk_ps / self.sdmclk_ps) >= 2)
        {
            if ((self.mode == UHS_DDR50) || (self.mode == MMC_HS52_DDR))
            {
                sdhc_hcsdclkadj -= 2;
            }
            else
            {
                sdhc_hcsdclkadj -= 1;
            }
        }
        else if ((self.sdclk_ps / self.sdmclk_ps) == 1)
        {
            sdhc_hcsdclkadj += 2;
        }
        else{
            ;/* no deal with */
        }
        
        if (self.tune_dat)
        {
            sdhc_hcsdclkadj -= 1;
        }
    }

    if (sdhc_hcsdclkadj > 15)
    {
        sdhc_hcsdclkadj = 15;
    }
    output.sdhc_hcsdclkadj = sdhc_hcsdclkadj;

    return 0;
}

static int calculate_io (void)
{
    u32 sdhc_rw_compensate = (u32)floor((self.iocell_input_delay +
                                         self.iocell_output_delay) /
                                        self.sdmclk_ps) +
                             output.sdhc_wrdata0_dly + 5 + 3;
    u32 cp_io_mask_start;
    u32 sdhc_clkf = (self.sdmclk_ps == self.sdclk_ps) ? 0 : 1;

    if (sdhc_clkf == 0)
    {
        if (sdhc_rw_compensate > 10)
        {
            cp_io_mask_start = 2 * (sdhc_rw_compensate - 10);
        }
        else
        {
            cp_io_mask_start = 0;
        }
    }
    else
    {
        cp_io_mask_start = 0;
    }

    if (self.mode == UHS_SDR104)
    {
        cp_io_mask_start += 1;
    }

    if ((self.mode == UHS_SDR50) && (sdhc_clkf == 0))
    {
        cp_io_mask_start += 1;
    }

    output.sdhc_rw_compensate = sdhc_rw_compensate;
    output.sdhc_idelay_val = (int)floor((2 * self.iocell_input_delay) /
                                        self.sdmclk_ps);
    output.cp_io_mask_start = cp_io_mask_start;

    return 0;
}

static int calculate (void)
{
    output.cp_data_select_oe_end = 1;
    output.cp_dll_start_point = 4;

    (void)calculate_cmd_out();
    (void)calculate_cmd_in();
    (void)calculate_data_out();
    (void)calculate_data_in();
    (void)calculate_io();

    return 0;
}

static int init (u32 mode, u32 freq)
{
    self.sdclk_ps = (u32)((((float)1e9) / freq) * 1000);
    self.iocell_input_delay = 2500;
    self.iocell_output_delay = 2500;
    self.mode = mode;
    
    switch (mode)
    {
    case SDMMC_DS_ID:
        // self.sdclk_ps = 1e6/0.4;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(5e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 5e3);
        self.t_dat_output_min = (u32)(5e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 5e3);
        self.t_cmd_input_min = (u32)(self.sdclk_ps / 2 + 50e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + self.sdclk_ps / 2);
        self.t_dat_input_min = (u32)(self.sdclk_ps / 2 + 50e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + self.sdclk_ps / 2);
        self.sdclk_ps_min = (u32)(1e6 / 0.4);
        self.sdclk_ps_max = (u32)(1e6 / 0.1);
        break;
    case SDMMC_DS:
        // self.sdclk_ps = 1e6/25;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(5e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 5e3);
        self.t_dat_output_min = (u32)(5e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 5e3);
        self.t_cmd_input_min = (u32)(self.sdclk_ps / 2 + 14e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + self.sdclk_ps / 2);
        self.t_dat_input_min = (u32)(self.sdclk_ps / 2 + 14e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + self.sdclk_ps / 2);
        self.sdclk_ps_min = (u32)(1e6 / 25);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case SD_HS:
        // self.sdclk_ps = 1e6/50;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(2e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 6e3);
        self.t_dat_output_min = (u32)(2e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 6e3);
        self.t_cmd_input_min = (u32)(14e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 2.5e3);
        self.t_dat_input_min = (u32)(14e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 2.5e3);
        self.sdclk_ps_min = (u32)(1e6 / 50);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case UHS_SDR12:
        // self.sdclk_ps = 1e6/25;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(0.8e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_dat_output_min = (u32)(0.8e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_cmd_input_min = (u32)(14e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.t_dat_input_min = (u32)(14e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.sdclk_ps_min = (u32)(1e6 / 25);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case UHS_SDR25:
        // self.sdclk_ps = 1e6/50;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(0.8e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_dat_output_min = (u32)(0.8e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_cmd_input_min = (u32)(14e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.t_dat_input_min = (u32)(14e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.sdclk_ps_min = (u32)(1e6 / 50);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case UHS_SDR50:
        // self.sdclk_ps = 1e6/100;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(0.8e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_dat_output_min = (u32)(0.8e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_cmd_input_min = (u32)(7.5e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.t_dat_input_min = (u32)(7.5e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.sdclk_ps_min = (u32)(1e6 / 100);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case UHS_SDR104:
        // self.sdclk_ps = 1e6/200;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(0.8e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 1.4e3);
        self.t_dat_output_min = (u32)(0.8e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 1.4e3);
        self.t_cmd_input_min = (u32)(1e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 1e3);
        self.t_dat_input_min = (u32)(1e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 1e3);
        self.sdclk_ps_min = (u32)(1e6 / 200);
        self.sdclk_ps_max = (u32)(1e6 / 100);
        break;
    case UHS_DDR50:
        // self.sdclk_ps = 1e6/50;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(0.8e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_dat_output_min = (u32)(0.8e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_cmd_input_min = (u32)(13.7e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.t_dat_input_min = (u32)(13.7e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.sdclk_ps_min = (u32)(1e6 / 50);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;

    case MMC_HS26:
        // self.sdclk_ps = 1e6/25;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(3e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_dat_output_min = (u32)(3e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_cmd_input_min = (u32)(self.sdclk_ps - 11.7e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 8.3e3);
        self.t_dat_input_min = (u32)(self.sdclk_ps - 11.7e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 8.3e3);
        self.sdclk_ps_min = (u32)(1e6 / 25);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case MMC_HS52:
        // self.sdclk_ps = 1e6/50;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(3e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_dat_output_min = (u32)(3e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_cmd_input_min = (u32)(13.7e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 2.5e3);
        self.t_dat_input_min = (u32)(13.7e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 2.5e3);
        self.sdclk_ps_min = (u32)(1e6 / 50);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case MMC_HS52_DDR:
        // self.sdclk_ps = 1e6/50;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(3e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_dat_output_min = (u32)(3e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 3e3);
        self.t_cmd_input_min = (u32)(13.7e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 2.5e3);
        self.t_dat_input_min = (u32)(7e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 1.5e3);
        self.sdclk_ps_min = (u32)(1e6 / 50);
        self.sdclk_ps_max = (u32)(1e6 / 0.4);
        break;
    case MMC_HS200:
        // self.sdclk_ps = 1e6/200;
        self.phy_sdclk_delay = (u32)(2 * self.sdmclk_ps);
        self.phy_cmd_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.phy_dat_o_delay = (u32)(2.5 * self.sdmclk_ps);
        self.t_cmd_output_min = (u32)(0.8e3);
        self.t_cmd_output_max = (u32)(self.sdclk_ps - 1.4e3);
        self.t_dat_output_min = (u32)(0.8e3);
        self.t_dat_output_max = (u32)(self.sdclk_ps - 1.4e3);
        self.t_cmd_input_min = (u32)(1e3);
        self.t_cmd_input_max = (u32)(self.sdclk_ps + 1e3);
        self.t_dat_input_min = (u32)(1e3);
        self.t_dat_input_max = (u32)(self.sdclk_ps + 1e3);
        self.sdclk_ps_min = 1e6 / 200;
        self.sdclk_ps_max = 1e6 / 100;
        break;

    default:
        return FMSH_EINVAL;
    };

    return 0;
}

int FSdPsu_Phy_Calc (FSdPsu_T *sdPtr, u32 mode, u32 freq)
{
    int ret;

    (void)memset(&output, 0, sizeof(struct dll_phy_output));
    (void)memset(&self, 0, sizeof(struct self_param));

    // init
    self.sdmclk_ps = (u32)((((float)1e9) / sdPtr->config.input_clock_hz) *
                           1000);
    ret = init(mode, freq);
    if (ret)
    {
        return ret;
    }

    output.sdhc_rdcmd_en = 1;
    output.sdhc_rddata_en = 1;

    self.ddr = 0;
    self.tune_cmd = 0;
    self.tune_dat = 0;

    switch (mode)
    {
    case UHS_DDR50:
    case MMC_HS52_DDR:
        self.ddr = 1;
        break;
    case UHS_SDR104:
    case MMC_HS200:
        self.tune_cmd = 1;
        self.tune_dat = 1;
        break;
    default:
        break;
    };

    self.sdr = !self.ddr;

    // clock setting
    ret = clock_setting();
    if (ret)
    {
        fmsh_print_err("Failed to get clock setting!\r\n");
        return ret;
    }
    (void)calculate();

    return 0;
}

#endif

/***************************************************************************/
static int FSdPsu_Phy_SetReg (FSdPsu_T *sdPtr, u16 reg, u32 value)
{
    u32 addr;

    addr = sdPtr->config.base + reg;

    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS04, addr);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS05, value);

    return 0;
}

static int FSdPsu_Phy_GetReg (FSdPsu_T *sdPtr, u16 reg, u32 *value)
{
    u32 addr;

    addr = sdPtr->config.base + reg;

    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS04, addr);
    *value = FMSH_ReadReg(sdPtr->config.base, SDHCI_HRS05);

    return 0;
}

int FSdPsu_Phy_Config (FSdPsu_T *sdPtr, u32 mode)
{
    u32 value;
    struct dll_phy_output *out;

#ifdef DLL_PHY_PRESET_VALUE
    if (mode == SDMMC_DS_ID)
    {
        out = &sd_ds_id;
    }
    else
    {
        out = &output;
    }
#else
    (void)FSdPsu_Phy_Calc(sdPtr, mode, sdPtr->host.sdclk);
    out = &output;
#endif

    // phy sw reset
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS09, 0x0);

    value = (out->cp_io_mask_always_on << 31) | (out->cp_io_mask_end << 27) |
            (out->cp_io_mask_start << 24) | (out->cp_data_select_oe_end);
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_DQ_TIMING, value);
    
    // config SDHCI_PHY_DQS_TIMING
    value = (out->cp_use_ext_lpbk_dqs << 22) | (out->cp_use_lpbk_dqs << 21) |
            (out->cp_use_phony_dqs << 20) | (out->cp_use_phony_dqs_cmd << 19);
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_DQS_TIMING, value);

    // config SDHCI_PHY_GATE_LPBK_CTRL
    value = (out->cp_sync_method << 31) | (out->cp_rd_del_sel << 19) |
            (out->cp_underrun_supress << 18) |
            (out->cp_gate_cfg_always_on << 6);
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_GATE_LPBK_CTRL, value);

    // config SDHCI_PHY_DLL_MASTER_CTRL
    value = (out->cp_dll_bypass_mode << 23) | (0x1 << 20) |
            (out->cp_dll_start_point);
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_DLL_MASTER_CTRL, value);

    // config SDHCI_PHY_DLL_SLAVE_CTRL
    value = (out->cp_read_dqs_cmd_delay << 24) |
            (out->cp_clk_wrdqs_delay << 16) | (out->cp_clk_wr_delay << 8) |
            (out->cp_read_dqs_delay);
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_DLL_SLAVE_CTRL, value);

    // config SDHCI_PHY_WR_DESKEW_PD_CTRL_0
    value = (out->cp_sw_half_cycle_shift << 4);
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_WR_DESKEW_PD_CTRL_0, value);

    // config SDHCI_PHY_CTRL
    int div = 0;
    if (out->sdhc_extended_rd_mode)
    {
        div = sdPtr->config.input_clock_hz / sdPtr->host.sdclk;
        if (div > 31)
        {
            div = 31;
        }
        div--;
    }
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_CTRL, div << 4);

    // release reset & wait for init comp
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS09, 0x1);
    while (1)
    {
        value = FMSH_ReadReg(sdPtr->config.base, SDHCI_HRS09);
        if (value & 0x2)
        {
            break;
        }
    }

    // config HRS09
    value = 0xf1c00001;
    value |= (out->sdhc_rddata_en << 16) | (out->sdhc_rdcmd_en << 15) |
             (out->sdhc_extended_wr_mode << 3) |
             (out->sdhc_extended_rd_mode << 2);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS09, value);

    // config HRS10
    value = (out->sdhc_hcsdclkadj << 16);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS10, value);

    // config HRS16
    value = (out->sdhc_wrdata1_sdclk_dly << 28) |
            (out->sdhc_wrdata0_sdclk_dly << 24) |
            (out->sdhc_wrcmd1_sdclk_dly << 20) |
            (out->sdhc_wrcmd0_sdclk_dly << 16) | (out->sdhc_wrdata1_dly << 12) |
            (out->sdhc_wrdata0_dly << 8) | (out->sdhc_wrcmd1_dly << 4) |
            (out->sdhc_wrcmd0_dly);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS16, value);

    // config HRS07
    value = (out->sdhc_rw_compensate << 16) | (out->sdhc_idelay_val);
    FMSH_WriteReg(sdPtr->config.base, SDHCI_HRS07, value);

    return 0;
}

int FSdPsu_Phy_SetDqsDelay (FSdPsu_T *sdPtr, u8 value)
{
    u32 reg;

    (void)FSdPsu_Phy_GetReg(sdPtr, SDHCI_PHY_DLL_SLAVE_CTRL, &reg);
    reg &= ~(0xff0000ff);
    reg |= (value << 24) | value;
    (void)FSdPsu_Phy_SetReg(sdPtr, SDHCI_PHY_DLL_SLAVE_CTRL, reg);

    return 0;
}
