
#include <math.h>

#include "fmsh_xspi.h"
#include "fmsh_xspi_hw.h"

int FQspiPsu_Phy_Config (FQspiPsu_T* qspiPtr)
{
    int timeout = 0;
    u32 value;

    float clk_period, board_delay;
    float flash_setup, tx_flash_setup;
    int nand2_delay;

    int gate_open_delay, dll_phy_rd_delay;
    u32 dll_phy_gate_open_delay, rd_del_sel;

    float quarter_cycle_period;
    int use_dll_action;
    __attribute__((unused)) int half_cycle_mode;
    float out_cycle_margin;
    int sdr_edge_active;
    float flash_setup_period;

    int each_delay_element;
    int dll_start_point, number_of_elements_tx, number_of_elements_rx;

    u32 dll_phy_dqs_timing_reg, dll_phy_gate_lpbk_ctrl_reg,
        dll_phy_dll_master_ctrl_reg, dll_phy_dll_slave_ctrl_reg,
        mini_dll_phy_ctrl;

    FMSH_ASSERT(qspiPtr != NULL);

    clk_period = 1000000000 / qspiPtr->config.sclk_hz;
    board_delay = qspiPtr->config.board_delay + 15;  //+30
    nand2_delay = QSPI_NAND2_DELAY;
    flash_setup = 6.0;
    tx_flash_setup = 2.5;

    gate_open_delay = (int)(board_delay / clk_period);
    if (gate_open_delay > 15)
    {
        dll_phy_gate_open_delay = 15;
        fmsh_print_err("ERROR: gate_open_delay overflow.\r\n");
    }
    else
    {
        dll_phy_gate_open_delay = gate_open_delay;
    }
    dll_phy_rd_delay = (int)ceil(board_delay / clk_period);
    if (dll_phy_rd_delay < 29)
    {
        rd_del_sel = dll_phy_rd_delay + 3;
    }
    else
    {
        fmsh_print_err("ERROR: rd_del_sel overflow.\r\n");
    }
    dll_phy_gate_lpbk_ctrl_reg = (rd_del_sel << 19) | 0x30 |
                                 dll_phy_gate_open_delay;

    /*
     * check if clock period is bigger than the delay of the whole delay line
     * (with 10% margin - instead of checking 256 elements we check only 230
     *  elements.
     */
    if ((clk_period * 1000) > (int)(230 * 2 * nand2_delay))
    {
        half_cycle_mode = 1;
    }
    else
    {
        half_cycle_mode = 0;
    }
    /*
     * check if half clock period will be locked properly by the DLL -
     * minimum requirement to lock.
     */
    if (((clk_period * 1000) / 2) < (int)(230 * 2 * nand2_delay))
    {
        use_dll_action = 1;
    }
    else
    {
        use_dll_action = 0;
    }
    quarter_cycle_period = clk_period * 1000 / 4;

    /*
     * sampling shift is supposed to provide half clock cycle resolution to
     * cover any flash transfer mode.
     */

    /*
     * maxium outside cycle margin assumed is 0.4 ns.
     *
     */
    out_cycle_margin = 0.4;
    sdr_edge_active = 0;

    if ((flash_setup + out_cycle_margin) >= clk_period)
    {
        flash_setup_period = flash_setup + out_cycle_margin - clk_period;
        sdr_edge_active = 1;
    }
    else if (flash_setup > (clk_period / 2))
    {
        flash_setup_period = flash_setup - (clk_period / 2);
    }
    else
    {
        flash_setup_period = flash_setup;  // 0
    }

    each_delay_element = 2 * nand2_delay;
    /* 80% of period is recommanded */
    dll_start_point = (int)(0.8 * (clk_period * 1000) / each_delay_element);

    /* set sampling point after flash setup + 10% of clock period */
    if (use_dll_action)
    {
        if (flash_setup_period == 0)
        {
            number_of_elements_rx = (int)ceil(
                256 * (flash_setup_period / clk_period));
        }
        else
        {
            number_of_elements_rx = (int)ceil(256 * (flash_setup_period /
                                                     clk_period)) +
                                    25;
        }
    }
    else
    {
        number_of_elements_rx = (int)ceil((flash_setup_period * 1000) /
                                          each_delay_element) +
                                1;
    }

    /* tx direction */
    if (ceil((tx_flash_setup * 1000) / each_delay_element) >
        (ceil(quarter_cycle_period / each_delay_element) -
         ceil((quarter_cycle_period / each_delay_element) / 5)))
    {
        number_of_elements_tx = (int)(ceil(tx_flash_setup * 1000 /
                                           each_delay_element) -
                                      ceil(quarter_cycle_period /
                                           each_delay_element / 5));
    }
    else
    {
        number_of_elements_tx = (int)ceil(quarter_cycle_period /
                                          each_delay_element);
        /* fix bug that qspi may not work correct at ss corner */
        number_of_elements_tx /= 2;
    }
    
    if (number_of_elements_tx >= 256)
    {
        number_of_elements_tx = 255;
    }

    if (number_of_elements_rx >= 256)
    {
        number_of_elements_rx = 255;
    }

    if (dll_start_point >= 256)
    {
        dll_start_point = 255;
    }

    if (use_dll_action)
    {
        dll_phy_dll_master_ctrl_reg = 0x00140000 | dll_start_point;
        if (ceil(tx_flash_setup * 1000 / each_delay_element) >
            (ceil(quarter_cycle_period / each_delay_element)))
        {
            dll_phy_dll_slave_ctrl_reg = 0x00001500 | number_of_elements_rx;
        }
        else
        {
            dll_phy_dll_slave_ctrl_reg = 0x00003300 | number_of_elements_rx;
        }
    }
    else
    {
        dll_phy_dll_master_ctrl_reg = 0x00800000;
        if (ceil(tx_flash_setup * 1000 / each_delay_element) >
            (ceil(quarter_cycle_period / each_delay_element) - 5))
        {
            dll_phy_dll_slave_ctrl_reg = 0x00000500 | number_of_elements_rx;
        }
        else
        {
            dll_phy_dll_slave_ctrl_reg = (number_of_elements_tx << 8) |
                                         number_of_elements_rx;
        }
    }

    if (sdr_edge_active == 0)
    {
        mini_dll_phy_ctrl = 0x00000707;
    }
    else
    {
        mini_dll_phy_ctrl = 0x00200707;
    }

    if (qspiPtr->config.pad_lpbk == 1)
    {
        dll_phy_dqs_timing_reg = 0x00700404;
    }
    else
    {
        dll_phy_dqs_timing_reg = 0x00300404;
    }

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DLL_PHY_CTRL, mini_dll_phy_ctrl);

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PHY_CTRL, 0x00004000);
    fmsh_print_dbg("DEBUG: dll_phy_ctrl_reg is 0x%08x.\r\n", 0x00004000);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PHY_TSEL, 0x00000000);
    fmsh_print_dbg("DEBUG: dll_phy_tsel_reg is 0x%08x.\r\n", 0x00000000);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PHY_DQ_TIMING, 0x00000101);
    fmsh_print_dbg("DEBUG: dll_phy_dq_timing_reg is 0x%08x.\r\n", 0x00000101);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PHY_DQS_TIMING,
                  dll_phy_dqs_timing_reg);  // 0x00300404
    fmsh_print_dbg("DEBUG: dll_phy_dqs_timing_reg is 0x%08x.\r\n",
                   dll_phy_dqs_timing_reg);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PHY_GATE_LPBK_CTRL,
                  dll_phy_gate_lpbk_ctrl_reg);  // 0x280031
    fmsh_print_dbg("DEBUG: dll_phy_gate_lpbk_ctrl_reg is 0x%08x.\r\n",
                   dll_phy_gate_lpbk_ctrl_reg);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PHY_DLL_MASTER_CTRL,
                  dll_phy_dll_master_ctrl_reg);  // 0x800000
    fmsh_print_dbg("DEBUG: dll_phy_dll_master_ctrl_reg is 0x%08x.\r\n",
                   dll_phy_dll_master_ctrl_reg);
    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_PHY_DLL_SLAVE_CTRL,
                  dll_phy_dll_slave_ctrl_reg);  // 0xffff
    fmsh_print_dbg("DEBUG: dll_phy_dll_slave_ctrl_reg is 0x%08x.\r\n",
                   dll_phy_dll_slave_ctrl_reg);

    FMSH_WriteReg(qspiPtr->config.base, QSPI_R_DLL_PHY_CTRL,
                  mini_dll_phy_ctrl | (0x1 << 24));  // 0x1000707
    fmsh_print_dbg("DEBUG: mini_dll_phy_ctrl is 0x%08x.\r\n",
                   mini_dll_phy_ctrl | (0x1 << 24));

    // check dll locked
    while (1)
    {
        value = FMSH_ReadReg(qspiPtr->config.base, QSPI_R_PHY_DLL_OBS_0);
        if (value & 0x1)
        {
            break;
        }

        delay_1us();
        timeout++;
        if (timeout > 100)
        {
            return FMSH_ETIME;
        }
    }

    return FMSH_SUCCESS;
}
