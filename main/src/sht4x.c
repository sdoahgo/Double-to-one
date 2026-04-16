#include "sht4x.h"
#include <inttypes.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "user_hal.h"

#define SHT4X_ADDRESS 0x44
#define SHT4X_CMD_START_UP_HEATER_100MS_110mW 0x24

static const char *TAG = "SHT45";
static uint16_t sht4x_cmd_measure_delay_us = SHT4X_MEASUREMENT_DURATION_USEC;

void sensirion_sleep_usec(uint32_t useconds)
{
    esp_rom_delay_us(useconds);
}

bool SHT45_init(void)
{
    int retry_count = 0;
    esp_err_t ret;

    if (i2c_mutex == NULL) {
        ESP_LOGE(TAG, "i2c_mutex not initialized");
        return false;
    }

    while (retry_count < 10) {
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            ret = SHT45_read_serial_number();
            xSemaphoreGive(i2c_mutex);

            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "SHT sensor probing successful");
                return true;
            }

            ESP_LOGW(TAG, "SHT sensor probing failed, retry %d", retry_count + 1);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        retry_count++;
    }

    ESP_LOGE(TAG, "SHT sensor probing failed after %d attempts", retry_count);
    return false;
}

esp_err_t sht4x_heat(void)
{
    const uint8_t cmd = SHT4X_CMD_START_UP_HEATER_100MS_110mW;
    return i2c_master_write_to_device(I2C_NUM_0, SHT4X_ADDRESS, &cmd, 1, 5000 / portTICK_PERIOD_MS);
}

void SHT45_set_cmd(uint8_t cmd)
{
    esp_err_t ret = i2c_master_write_to_device(I2C_NUM_0, SHT4X_ADDRESS, &cmd, 1, 5000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed set cmd: %s", esp_err_to_name(ret));
    }
}

esp_err_t SHT45_read(uint8_t *data_buf, size_t size)
{
    esp_err_t ret = i2c_master_read_from_device(I2C_NUM_0, SHT4X_ADDRESS, data_buf, size, 5000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed SHT45_read: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t SHT45_read_serial_number(void)
{
    uint8_t data_buf[6];
    uint16_t serial_number1;
    uint16_t serial_number2;

    SHT45_set_cmd(read_serial_number);
    sensirion_sleep_usec(sht4x_cmd_measure_delay_us);
    SHT45_read(data_buf, sizeof(data_buf));

    serial_number1 = data_buf[0] << 8 | data_buf[1];
    serial_number2 = data_buf[3] << 8 | data_buf[4];

    if ((data_buf[2] == SHT45_CRC(serial_number1)) && (data_buf[5] == SHT45_CRC(serial_number2))) {
        uint32_t serial_number = ((uint32_t)serial_number1 << 16) | serial_number2;
        ESP_LOGI(TAG, "serial_number:%" PRIx32, serial_number);
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Failed SHT45_read_serial_number:%x %x", serial_number1, serial_number2);
    return ESP_FAIL;
}

uint8_t SHT45_CRC(uint16_t data)
{
    uint8_t crc = 0xFF;

    crc ^= (data >> 8);
    for (int i = 0; i < 8; i++) {
        crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1);
    }

    crc ^= (data & 0xFF);
    for (int i = 0; i < 8; i++) {
        crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1);
    }

    return crc;
}

esp_err_t SHT45_read_data(uint16_t cmd, uint16_t *temperature, uint16_t *humidity)
{
    esp_err_t ret = ESP_OK;
    uint8_t data_buf[6];

    SHT45_set_cmd((uint8_t)cmd);
    vTaskDelay(pdMS_TO_TICKS(10));
    ret = SHT45_read(data_buf, sizeof(data_buf));
    if (ret != ESP_OK) {
        return ret;
    }

    *temperature = data_buf[0] << 8 | data_buf[1];
    *humidity = data_buf[3] << 8 | data_buf[4];
    if ((data_buf[2] != SHT45_CRC(*temperature)) || (data_buf[5] != SHT45_CRC(*humidity))) {
        ESP_LOGE(TAG, "Failed read_data,temperature: %d,humidity: %d", *temperature, *humidity);
        return ESP_FAIL;
    }

    return ret;
}

void SHT45_soft_reset(void)
{
    uint8_t cmd = soft_reset;
    esp_err_t ret = i2c_master_write_to_device(I2C_NUM_0, SHT4X_ADDRESS, &cmd, 1, 5000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed soft reset: %s", esp_err_to_name(ret));
    }
}
