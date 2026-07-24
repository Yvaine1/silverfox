#ifndef SET_TRX_GAIN_H
#define SET_TRX_GAIN_H
#include <stdlib.h>

uint8_t get_closest_tx_dsa_index(int32_t data);
uint8_t get_closest_rx_dsa_index(int32_t data, uint8_t level);
void eeprom_set_tx_gain(int32_t data);
void eeprom_set_rx_gain(int32_t data);
void eeprom_get_tx_gain(void);
void eeprom_get_rx_gain(void);
void shell_cmd_eeprom_read(uint16_t addr);
void shell_cmd_eeprom_write(uint16_t addr, int16_t data);

#endif  // SET_TRX_GAIN_H
