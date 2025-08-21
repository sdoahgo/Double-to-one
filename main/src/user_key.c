#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "iot_button.h"
#include "user_key.h"
#include "esp_sleep.h"
#include "user_hal.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"


#define USER_KEY 1
QueueHandle_t KeyQueue;
volatile uint8_t key_event = 0;


static void button_single_click_cb(void *arg,void *usr_data)
{
   printf("button_single_click_cb\r\n");
   key_event = 0;
    if (xQueueSend(KeyQueue, &key_event , pdMS_TO_TICKS(100)) != pdPASS) {
        printf("Send KeyQueue failed!\n");
    }
}
 

static void button_long_press_cb(void *arg,void *usr_data)
{
   printf("button_long_press_cb\r\n");
   if(GIF_end_flag)
   {
        gpio_set_level(5, 0); //关闭传感器电源
        gpio_set_level(3, 1); // 关闭屏幕电源
        gpio_set_level(0, 0);//关闭所有的电源
   }
}
static void button_double_click_cb(void *arg,void *usr_data)
{
    key_event = 2;
    printf("button_double_click_cb\r\n");
    if (xQueueSend(KeyQueue, &key_event , pdMS_TO_TICKS(100)) != pdPASS) {
        printf("Send KeyQueue failed!\n");
    }
}
static void button_multiple_click_cb(void *arg,void *usr_data)
{
    key_event = 3;
    printf("button_multiple_click_cb\r\n");
    if (xQueueSend(KeyQueue, &key_event , pdMS_TO_TICKS(100)) != pdPASS) {
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
    button_handle_t gpio_btn ;
    button_config_t gpio_btn_cfg = {};
    gpio_btn_cfg.type = BUTTON_TYPE_GPIO;
    gpio_btn_cfg.gpio_button_config.gpio_num = USER_KEY;
    gpio_btn_cfg.gpio_button_config.active_level = 0,
    gpio_btn_cfg.long_press_time = 2000;
    gpio_btn_cfg.short_press_time = 300;
    gpio_btn = iot_button_create(&gpio_btn_cfg);

    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_LONG_PRESS_START, button_long_press_cb, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_DOUBLE_CLICK, button_double_click_cb, NULL);
    button_event_config_t ec = {
        .event = BUTTON_MULTIPLE_CLICK,
        .event_data.multiple_clicks.clicks = 6,
    };
    iot_button_register_event_cb(gpio_btn, ec, button_multiple_click_cb, NULL);
}


void switch_ota_and_reboot(void)
{
    // 获取当前正在运行的分区
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (!running) {
        ESP_LOGE("OTA_SWITCH", "No running partition!");  // 出错保护
        return;
    }

    // 找到“另一半”的 OTA 分区
    const esp_partition_t *next = NULL;
    if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0) {
        // 当前在 ota_0 → 下次启动设为 ota_1
        next = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    } else if (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_1) {
        // 当前在 ota_1 → 下次启动设为 ota_0
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

    // 打印日志提示切换成功
    ESP_LOGI("OTA_SWITCH", "Boot set to subtype=%d addr=0x%08lx, restarting...",
             next->subtype, (unsigned long)next->address);

    fflush(stdout);                
    vTaskDelay(pdMS_TO_TICKS(100)); 
    esp_restart();                  
}
