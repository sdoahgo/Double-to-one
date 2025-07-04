#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "iot_button.h"
#include "user_key.h"
#include "esp_sleep.h"
#include "user_hal.h"


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
        gpio_set_level(4, 0); //关闭传感器电源
        gpio_set_level(3, 1); // 关闭屏幕电源
        gpio_set_level(0, 0);//关闭所有的电源
   }
    // gpio_deep_sleep_hold_dis();	    //在深度睡眠时禁用所有数字gpio pad保持功能 
    // // const gpio_num_t ext_wakeup_pin_0 = GPIO_NUM_5;
    // // ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(ext_wakeup_pin_0, 0));
    // //gpio_set_direction(ext_wakeup_pin_0, GPIO_MODE_INPUT);	//GPIO定向，设置为输入或输出
    // esp_deep_sleep_start();
}
static void button_double_click_cb(void *arg,void *usr_data)
{
    key_event = 2;
    printf("button_double_click_cb\r\n");
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
    // user_deep_sleep_register_gpio_wakeup();
    // user_power_hold();
    
    // Shut down by long press 2 seconds
    button_config_t gpio_btn_cfg = {};
    gpio_btn_cfg.type = BUTTON_TYPE_GPIO;
    gpio_btn_cfg.gpio_button_config.gpio_num = USER_KEY;
    gpio_btn_cfg.gpio_button_config.active_level = 0,
    gpio_btn_cfg.long_press_time = 2000;
    gpio_btn = iot_button_create(&gpio_btn_cfg);
    if(NULL == gpio_btn) {
        // ESP_LOGI(TAG, "Button create failed");
    }
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_LONG_PRESS_START, button_long_press_cb, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_DOUBLE_CLICK, button_double_click_cb, NULL);
} 
