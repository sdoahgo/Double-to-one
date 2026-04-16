#ifndef SHT4X_H
#define SHT4X_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHT4X_MEASUREMENT_DURATION_USEC 10000

#define high_precision 0xFD
#define medium_precision 0xF6
#define lowest_precision 0xE0
#define read_serial_number 0x89
#define soft_reset 0x94

void sensirion_sleep_usec(uint32_t useconds);
bool SHT45_init(void);
esp_err_t sht4x_heat(void);
void SHT45_set_cmd(uint8_t cmd);
esp_err_t SHT45_read(uint8_t *data_buf, size_t size);
esp_err_t SHT45_read_serial_number(void);
uint8_t SHT45_CRC(uint16_t data);
esp_err_t SHT45_read_data(uint16_t cmd, uint16_t *temperature, uint16_t *humidity);
void SHT45_soft_reset(void);

#ifdef __cplusplus
}
#endif

#endif
