#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "iot_button.h"
#include "user_key.h"
#include "esp_sleep.h"
#include "user_hal.h"
#include "ui.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"

#define USER_KEY 1
#define USER_ON_OFF 0
#define USER_LCD_POWER 4
#define USER_KEY_TEMP_DISABLE 0
#define USER_KEY_CONFIRM_MS 1000
#define USER_KEY_POWER_OFF_MS 2000

QueueHandle_t KeyQueue;
volatile uint8_t key_event = 0;

static void button_single_click_cb(void *arg, void *usr_data)
{
    uint8_t event = 0;
    printf("button_single_click_cb\r\n");
    key_event = event;
    if (xQueueSend(KeyQueue, &event, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("Send KeyQueue failed!\n");
    }
}

static void button_long_press_cb(void *arg, void *usr_data)
{
    uint8_t event = 1;
    printf("button_long_press_cb\r\n");

    if (xQueueSend(KeyQueue, &event, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("Send KeyQueue failed!\n");
    }
}

static void button_power_long_press_cb(void *arg, void *usr_data)
{
    uint8_t event = USER_KEY_POWER_LONG_IN_EVT;
    printf("button_power_long_press_cb\r\n");

    if (xQueueSend(KeyQueue, &event, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("Send KeyQueue failed!\n");
    }
}

static void button_double_click_cb(void *arg, void *usr_data)
{
    uint8_t event = 2;
    key_event = event;
    printf("button_double_click_cb\r\n");
    if (xQueueSend(KeyQueue, &event, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("Send KeyQueue failed!\n");
    }
}

static void button_multiple_click_cb(void *arg, void *usr_data)
{
    uint8_t event = 3;
    key_event = event;
    printf("button_multiple_click_cb\r\n");
    if (xQueueSend(KeyQueue, &event, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("Send KeyQueue failed!\n");
    }
}

void user_key_init(void)
{
    KeyQueue = xQueueCreate(10, sizeof(uint8_t));
    if (KeyQueue == NULL) {
        ESP_LOGE("KeyQueue", "Failed to create queue");
    } else {
        ESP_LOGI("KeyQueue", "Queue created successfully");
    }

#if USER_KEY_TEMP_DISABLE
    ESP_LOGW("KeyQueue", "Key input is temporarily disabled for bring-up");
    return;
#endif

    button_handle_t gpio_btn;
    button_config_t gpio_btn_cfg = {};
    gpio_btn_cfg.type = BUTTON_TYPE_GPIO;
    gpio_btn_cfg.gpio_button_config.gpio_num = USER_KEY;
    gpio_btn_cfg.gpio_button_config.active_level = 0;
    gpio_btn_cfg.long_press_time = USER_KEY_CONFIRM_MS;
    gpio_btn_cfg.short_press_time = 300;
    gpio_btn = iot_button_create(&gpio_btn_cfg);

    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_LONG_PRESS_START, button_long_press_cb, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_DOUBLE_CLICK, button_double_click_cb, NULL);

    button_event_config_t long_press_power_cfg = {
        .event = BUTTON_LONG_PRESS_START,
        .event_data.long_press.press_time = USER_KEY_POWER_OFF_MS,
    };
    iot_button_register_event_cb(gpio_btn, long_press_power_cfg, button_power_long_press_cb, NULL);

    button_event_config_t ec = {
        .event = BUTTON_MULTIPLE_CLICK,
        .event_data.multiple_clicks.clicks = 6,
    };
    iot_button_register_event_cb(gpio_btn, ec, button_multiple_click_cb, NULL);
}

void switch_ota_and_reboot(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (!running) {
        ESP_LOGE("OTA_SWITCH", "No running partition!");
        return;
    }

    const esp_partition_t *next = NULL;
    if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0) {
        next = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    } else if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_1) {
        next = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    } else {
        ESP_LOGE("OTA_SWITCH", "Running from non-OTA partition (subtype=%d)", running->subtype);
        return;
    }

    if (!next) {
        ESP_LOGE("OTA_SWITCH", "Next OTA partition not found!");
        return;
    }

    esp_err_t err = esp_ota_set_boot_partition(next);
    if (err != ESP_OK) {
        ESP_LOGE("OTA_SWITCH", "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI("OTA_SWITCH", "Boot set to subtype=%d addr=0x%08lx, restarting...",
             next->subtype, (unsigned long)next->address);

    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}
