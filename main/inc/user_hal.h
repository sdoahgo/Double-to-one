#ifndef USER_HAL_H
#define USER_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui.h"


#define fitting_value_a1  "fit_a1"
#define fitting_value_b1  "fit_b1"
#define fitting_value_a2  "fit_a2"
#define fitting_value_b2  "fit_b2"
#define nvs_turning_point "turning_point"


extern SemaphoreHandle_t i2c_mutex;



typedef struct 
{
    uint8_t language_id;
    int16_t temp_calibration_value;
    uint8_t GIF_VALUE;
    char sn[20];
}sys_data;

typedef struct
{
    sys_data config_data;
    float fit_A1;
    float fit_B1;
    float fit_A2;
    float fit_B2;
    float turning_point;
}NVS_data_t;

extern sys_data SYS_DATA;
extern NVS_data_t NVS_data;

typedef enum {
    UI_PRODUCT_PAGE_BOOT = 0,
    UI_PRODUCT_PAGE_MEASURE_BEFORE,
    UI_PRODUCT_PAGE_MEASURE_AFTER,
    UI_PRODUCT_PAGE_MEASURE_OVERRANGE,
    UI_PRODUCT_PAGE_LANGUAGE,
    UI_PRODUCT_PAGE_STANDBY,
    UI_PRODUCT_PAGE_INFO,
    UI_PRODUCT_PAGE_BATTERY,
    UI_PRODUCT_PAGE_PLAN_G,
    UI_PRODUCT_PAGE_LOW_BATTERY,
} ui_product_page_t;

typedef enum {
    UI_MEASURE_STATE_IDLE = 0,
    UI_MEASURE_STATE_BEFORE,
    UI_MEASURE_STATE_AFTER,
    UI_MEASURE_STATE_OVERRANGE,
} ui_measure_state_t;

typedef enum {
    UI_STANDBY_TIME_30_MIN = 30,
    UI_STANDBY_TIME_60_MIN = 60,
    UI_STANDBY_TIME_90_MIN = 90,
} ui_standby_time_t;

typedef struct
{
    ui_product_page_t target_page;
    ui_measure_state_t measure_state;
    ui_standby_time_t standby_time;
    bool charge_state;
    bool low_battery;
    bool ble_connected;
    uint8_t bat_level;
    float display_temp_c;
    float display_temp_f;
    float display_humidity;
    float display_capacitance;
    int32_t display_weight_raw;
} ui_product_state_t;

extern ui_product_state_t UI_DATA;

typedef enum{
    UI_MSG_UPDATE_TEMP,
    UI_MSG_UPDATE_850,
    UI_MSG_UPDATE_SENSOR,
    UI_MSG_UPDATE_CHARGE_STATE,
    UI_MSG_UPDATE_STANDBY_TIME,
    UI_MSG_SWITCH_PRODUCT_PAGE,
    UI_MSG_UPDATE_LANUAGE,
    UI_MSG_UPDATE_STATE,
    UI_MSG_UPDATA_WIFI_BLE_ICON,
    UI_MSG_UPDATA_BAT_ICON,
}ui_msg_type_t;

typedef struct 
{
    ui_msg_type_t type;
    // union {
    //     int16_t temp_value;       // 4 字节（float类型，通常是4字节）
    //     uint16_t TF_value;
    //     char alert_str[32];     // 32 字节（32个char，每个1字节）
    //     // 其它数据（如有更大则按更大算）
    // } data;
    int16_t temp_value; 
    #ifdef USE_TDS
    uint16_t TF_DIST_value;
    uint16_t TF_AMP_value;
    float TF_TDS_value;
    #endif
    float SHT45_TEMP_value;
    float SHT45_HUMIDITY_value;
    float MDC04_value;
    int32_t CS1237_value;
    bool SHT45_link;
    bool MDC04_link;
    bool CS1237_link;
    bool charge_state_value;
    ui_standby_time_t standby_time_value;
    ui_product_page_t product_page_value;
    ui_measure_state_t measure_state_value;
}ui_msg_t;

typedef struct 
{
    bool ble_icon;
    bool charge_icon;
    uint8_t bat_icon; //0 - 100, 1 - 80, 2 - 60, 3 -40, 4 -20 , 5 - 5
}ui_icon_t;
extern ui_icon_t UI_icon;

// extern Sensor_Data sensors_data;
extern QueueHandle_t ui_msg_queue;
void user_get_sys_evn(sys_data *data);
void user_save_sys_evn(sys_data *data);
void user_device_init(void);
void user_ui_data_init(void);
esp_err_t NVS_Write_Num(const char *KeyName,int32_t Value);
esp_err_t NVS_Read_Num(const char *KeyName, int32_t *ReadValue);
esp_err_t NVS_Write_Str(const char *KeyName, const char *str);
esp_err_t NVS_Read_Str(const char *KeyName, char *out_str, size_t max_len);
esp_err_t NVS_Write_Float(const char *key, float value);
esp_err_t NVS_Read_Float(const char *key, float *value);

#ifdef __cplusplus
}
#endif

#endif /* __USER_HAL_H */
