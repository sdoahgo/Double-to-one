#include "user_sensor.h"
#include "MDC04IIC.h"
#include "MDC04app.h"
#include "cs1237.h"
#include "sht4x.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "user_hal.h"

SemaphoreHandle_t i2c_mutex = NULL;

volatile float SHT45_TEMP = 0.0f;
volatile float SHT45_HUMIDITY = 0.0f;
volatile float MDC04_VALUE = 0.0f;
volatile int32_t CS1237_VALUE = 0;

volatile bool SHT45_LINK = false;
volatile bool MDC04_LINK = false;
volatile bool CS1237_LINK = false;

static const char *TAG = "user_sensor";

static void user_sensor_task(void *pvParameters)
{
    uint16_t temp_raw = 0;
    uint16_t humidity_raw = 0;
    ui_msg_t msg;

    (void)pvParameters;

    while (1) {
        if (i2c_mutex && xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
            if (SHT45_read_data(high_precision, &temp_raw, &humidity_raw) == ESP_OK) {
                SHT45_TEMP = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
                SHT45_HUMIDITY = -6.0f + 125.0f * ((float)humidity_raw / 65535.0f);
                SHT45_LINK = true;
                UI_DATA.display_temp_c = SHT45_TEMP;
                UI_DATA.display_temp_f = SHT45_TEMP * 1.8f + 32.0f;
                UI_DATA.display_humidity = SHT45_HUMIDITY;
            } else {
                SHT45_LINK = false;
            }

            MDC04_LINK = MDC04_ID();
            MDC04_VALUE = Get_MDC04_Data1();
            UI_DATA.display_capacitance = MDC04_VALUE;
            xSemaphoreGive(i2c_mutex);
        }

        CS1237_VALUE = get_cs1237_val();
        CS1237_LINK = cs1237_check_data(&cs1237_handler1) == 0;
        UI_DATA.display_weight_raw = CS1237_VALUE;

        ESP_LOGI(TAG, "SHT45: %.2f C %.2f %%RH, MDC04_LINK: %d MDC04: %.3f pF, CS1237: %ld",
                 SHT45_TEMP, SHT45_HUMIDITY, MDC04_LINK, MDC04_VALUE, (long)CS1237_VALUE);

        if (ui_msg_queue) {
            msg.type = UI_MSG_UPDATE_SENSOR;
            msg.SHT45_TEMP_value = SHT45_TEMP;
            msg.SHT45_HUMIDITY_value = SHT45_HUMIDITY;
            msg.MDC04_value = MDC04_VALUE;
            msg.CS1237_value = CS1237_VALUE;
            msg.SHT45_link = SHT45_LINK;
            msg.MDC04_link = MDC04_LINK;
            msg.CS1237_link = CS1237_LINK;
            xQueueSend(ui_msg_queue, &msg, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void user_sensor_init(void)
{
    if (i2c_mutex == NULL) {
        i2c_mutex = xSemaphoreCreateMutex();
    }

    ESP_ERROR_CHECK(myi2c_master_init());

    SHT45_LINK = SHT45_init();

    if (i2c_mutex && xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        MDC04_LINK = MDC04_init();
        xSemaphoreGive(i2c_mutex);
    }

    CS1237_LINK = driver_cs1237_init();

    xTaskCreate(user_sensor_task, "user_sensor_task", 1024 * 4, NULL, tskIDLE_PRIORITY + 4, NULL);
}
