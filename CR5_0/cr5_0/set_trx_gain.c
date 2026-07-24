#include "fmsh_common.h"
#include "freq_offset_calibration.h"

#include "set_trx_gain.h"

#define DSA_INDEX_MIN 0
#define DSA_INDEX_MAX 0x3f

#define RX_DSA_LEVEL1 1
#define RX_DSA_LEVEL2 2

#define TRX_GAIN_REG_OFFSET      4
#define TRX_GAIN_REG_START       (DSA_INDEX_MIN+TRX_GAIN_REG_OFFSET)
#define TRX_GAIN_REG_END         (DSA_INDEX_MAX+TRX_GAIN_REG_OFFSET)

#define TX_GAIN_PAGE              5
#define RX_LEVEL1_GAIN_PAGE       6
#define RX_LEVEL2_GAIN_PAGE       7

#define FPGA_BASEADDR             0x80000000
#define RX_LEVEL1_DSA_FPGA_OFFSET 0x118
#define RX_LEVEL2_DSA_FPGA_OFFSET 0x114
#define TX_DSA_FPGA_OFFSET        0x124

uint8_t get_closest_tx_dsa_index(int32_t data)
{
    uint8_t left = TRX_GAIN_REG_START;
    uint8_t right = TRX_GAIN_REG_END;
    uint8_t mid;
    int32_t mid_gain;
    uint8_t result = 0;

    while(left <= right)
    {
        mid = left + ((right - left) >> 1);
        mid_gain = freq_off_get_from_eeprom(TX_GAIN_PAGE, mid);

        if(mid_gain == data)
        {
            return (mid-TRX_GAIN_REG_OFFSET)*2;
        }

        if(mid_gain > data)
        {
            left = mid + 1;
        }
        else
        {
            result = mid;
            right = mid - 1;
        }
    }

    return (result-TRX_GAIN_REG_OFFSET)*2;
}

uint8_t get_closest_rx_dsa_index(int32_t data, uint8_t level)
{
    uint8_t left = TRX_GAIN_REG_START;
    uint8_t right = TRX_GAIN_REG_END;
    uint8_t mid;
    int32_t mid_gain;
    uint8_t result = 0;

    while(left <= right)
    {
        mid = left + ((right - left) >> 1);
        if(level == RX_DSA_LEVEL2)
        {
            mid_gain = freq_off_get_from_eeprom(RX_LEVEL2_GAIN_PAGE, mid);
        }
        else
        {
            mid_gain = freq_off_get_from_eeprom(RX_LEVEL1_GAIN_PAGE, mid);
        }

        if(mid_gain == data)
        {
            return (mid-TRX_GAIN_REG_OFFSET)*2;
        }

        if(mid_gain > data)
        {
            left = mid + 1;
        }
        else
        {
            result = mid;
            right = mid - 1;
        }
    }

    return (result-TRX_GAIN_REG_OFFSET) * 2;
}

static void eeprom_write_tx_gain(uint16_t idx, int16_t val)
{
    tx_gain_eeprom_t tx_gain = {0};
    tx_gain.dsa_idx  = idx;
    tx_gain.gain_val = val;
    if (0 != eeprom_set_tx_dsa(&tx_gain))
    {
        fmsh_print("write tx gain failed!\r\n", __func__);
    }
}

static void eeprom_write_rx_gain(rx_gain_eeprom_t* rx_gain)
{
    if (0 != eeprom_set_rx_dsa(&rx_gain->level1, RX_DSA_LEVEL1))
    {
        fmsh_print("write rx gain level1 failed!\r\n", __func__);
    }

    if (0 != eeprom_set_rx_dsa(&rx_gain->level2, RX_DSA_LEVEL2))
    {
        fmsh_print("write rx gain level2 failed!\r\n", __func__);
    }
}

void eeprom_get_tx_gain(void)
{
    tx_gain_eeprom_t tx_gain = {0};
    int state = FMSH_FAILURE;
    state = eeprom_get_tx_dsa(&tx_gain);
    if (FMSH_SUCCESS != state)
    {
        fmsh_print("get tx gain failed!\r\n");
        return;
    }
    fmsh_print("tx: dsa %d, gain %d\r\n", tx_gain.dsa_idx, tx_gain.gain_val);
}

void eeprom_get_rx_gain(void)
{
    rx_gain_eeprom_t rx_gain = {0};
    int state = FMSH_FAILURE;
    state = eeprom_get_rx_dsa(&rx_gain.level1, RX_GAIN_LEVEL_1);
    if (FMSH_SUCCESS != state)
    {
        fmsh_print("get rx level1 gain failed!\r\n");
        return;
    }
    state = eeprom_get_rx_dsa(&rx_gain.level2, RX_GAIN_LEVEL_2);
    if (FMSH_SUCCESS != state)
    {
        fmsh_print("get rx level2 gain failed!\r\n");
        return;
    }
    fmsh_print("rx: level1 dsa %d, gain %d, level2 dsa %d, gain %d\r\n", 
        rx_gain.level1.dsa_idx, rx_gain.level1.gain_val,rx_gain.level2.dsa_idx, rx_gain.level2.gain_val);
}

void eeprom_set_tx_gain(int32_t data)
{
    int32_t gain_value_max = freq_off_get_from_eeprom(TX_GAIN_PAGE, TRX_GAIN_REG_START);
    int32_t gain_value_min = freq_off_get_from_eeprom(TX_GAIN_PAGE, TRX_GAIN_REG_END);
    if(data < gain_value_min || data > gain_value_max)
    {
        fmsh_print("Error: tx gain value must be between %d and %d\r\n", gain_value_min, gain_value_max);
        return;
    }

    uint8_t dsa_index = get_closest_tx_dsa_index(data);
    FMSH_WriteReg((uint32_t)FPGA_BASEADDR, TX_DSA_FPGA_OFFSET, dsa_index);
    eeprom_write_tx_gain(dsa_index, data);
    fmsh_print("tx gain set ok, dsa %#x \r\n",FMSH_ReadReg((uint32_t)FPGA_BASEADDR, TX_DSA_FPGA_OFFSET));
}

void eeprom_set_rx_gain(int32_t data)
{
    int32_t gain_value_max = freq_off_get_from_eeprom(RX_LEVEL2_GAIN_PAGE, TRX_GAIN_REG_START);
    int32_t gain_value_min = freq_off_get_from_eeprom(RX_LEVEL1_GAIN_PAGE, TRX_GAIN_REG_END);
    if(data < gain_value_min || data > gain_value_max)
    {
        fmsh_print("Error: rx gain value must be between %d and %d\r\n", gain_value_min, gain_value_max);
        return;
    }

    int32_t gain_value_mid = freq_off_get_from_eeprom(RX_LEVEL1_GAIN_PAGE, TRX_GAIN_REG_START);
    uint8_t dsa_index;
    rx_gain_eeprom_t rx_gain;
    if(data > gain_value_mid)
    {
        dsa_index = get_closest_rx_dsa_index(data, RX_DSA_LEVEL2);
        FMSH_WriteReg((uint32_t)FPGA_BASEADDR, RX_LEVEL1_DSA_FPGA_OFFSET, DSA_INDEX_MIN);
        FMSH_WriteReg((uint32_t)FPGA_BASEADDR, RX_LEVEL2_DSA_FPGA_OFFSET, dsa_index);

        rx_gain.level1.dsa_idx  = DSA_INDEX_MIN;
        rx_gain.level1.gain_val = data;
        rx_gain.level2.dsa_idx  = dsa_index;
        rx_gain.level2.gain_val = data;

        fmsh_print("rx gain set ok, dsa level1 %#x, level2 %#x\r\n",
            FMSH_ReadReg((uint32_t)FPGA_BASEADDR, RX_LEVEL1_DSA_FPGA_OFFSET),
            FMSH_ReadReg((uint32_t)FPGA_BASEADDR, RX_LEVEL2_DSA_FPGA_OFFSET));
    }
    else
    {
        dsa_index = get_closest_rx_dsa_index(data, RX_DSA_LEVEL1);
        FMSH_WriteReg((uint32_t)FPGA_BASEADDR, RX_LEVEL2_DSA_FPGA_OFFSET, DSA_INDEX_MAX * 2);
        FMSH_WriteReg((uint32_t)FPGA_BASEADDR, RX_LEVEL1_DSA_FPGA_OFFSET, dsa_index);

        rx_gain.level1.dsa_idx  = dsa_index;
        rx_gain.level1.gain_val = data;
        rx_gain.level2.dsa_idx  = DSA_INDEX_MAX * 2;
        rx_gain.level2.gain_val = data;

        fmsh_print("rx gain set ok, dsa level1 %#x, level2 %#x\r\n",
            FMSH_ReadReg((uint32_t)FPGA_BASEADDR, RX_LEVEL1_DSA_FPGA_OFFSET),
            FMSH_ReadReg((uint32_t)FPGA_BASEADDR, RX_LEVEL2_DSA_FPGA_OFFSET));
    }
    eeprom_write_rx_gain(&rx_gain);
}

void shell_cmd_eeprom_read(uint16_t addr)
{
    uint16_t data = 0;
    int ret = eeprom_read_bytes(addr, 2, &data, 2);
    if(FMSH_FAILURE == ret)
    {
        fmsh_print("addr %x read fail\r\n", addr);
        return;
    }
    fmsh_print("page %x read: %d\r\n", addr, data);
    return;
}

void shell_cmd_eeprom_write(uint16_t addr, int16_t data)
{
    int16_t w_data = data;
    int ret = eeprom_write_bytes(addr, 2, &w_data, 2);
    if(FMSH_FAILURE == ret)
    {
        fmsh_print("addr %x write fail\r\n", addr);
        return;
    }
    fmsh_print("addr %x write: %d\r\n", addr, data);
    return;
}

