#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/uart.h" 
#include "user_hal.h"
#include "user_ble.h"

const static char *TAG_POWER = "BAT";
#define USB_VBUS_DETECT_GPIO GPIO_NUM_10
#define ADC_ARR_SIZE 10
static int bat_adc_arr[ADC_ARR_SIZE];
static bool s_bat_adc_filter_reset_requested;

static void bat_adc_reset_filter(void)
{
    s_bat_adc_filter_reset_requested = true;
}

static bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG_POWER, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG_POWER, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif
  
    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG_POWER, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG_POWER, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG_POWER, "Invalid arg or no memory");
    }

    return calibrated;
}


//进行平滑处理
static int bat_adc_update(int adc_value)
{
    static int count=0;
    static uint8_t last_time_level = 0;
    int aver=0,sum=0;

        if(s_bat_adc_filter_reset_requested || last_time_level == 1)
        {
            count = 0;
            last_time_level = 0; 
            s_bat_adc_filter_reset_requested = false;
            memset(bat_adc_arr, 0, sizeof(bat_adc_arr));
            // user_get_ui_info()->state.charge = user_read_bat_charge_state();
        }
        bat_adc_arr[count++%ADC_ARR_SIZE]=adc_value;
        for (size_t i = 0; i < ADC_ARR_SIZE; i++)
        {
            sum+=bat_adc_arr[i];
        }
        if (count>ADC_ARR_SIZE)
        {
            aver=sum/ADC_ARR_SIZE;
        }
        else
        {aver=sum/count;}
        // return aver;
        // printf("aver = %d\n",aver);
    return aver;
}



#define HYST_MV 5.0f   // 滞回电压（mV），避免边界附近抖动

// 电池电量分级的阈值（单位 mV，按高到低排列）：对�?80%�?0%�?0%�?0%�?% �?
static const float thr[5] = {3980.0f, 3830.0f, 3730.0f, 3630.0f, 3450.0f};

// 当前电池图标编号�?~5），0=100%�?=80%�?=60%�?=40%�?=20%�?=5%
// 初始设为 0，表示默认满电；实际运行时会根据测量值更�?
// static uint8_t s_bat_icon = 0;

/*
 * 根据当前电池电压 v_mv（单�?mV），结合“当前档�?s_bat_icon”，
 * 计算并返回新的电池图标编号（0~5）�?
 *
 * 特点�?
 * - 使用滞回（HYST_MV），避免在临界值附近来回抖动�?
 * - 只有电压跨过阈�?± 滞回范围时才切换档位�?
 */
static uint8_t map_voltage_to_icon(float v_mv)
{
    switch (UI_icon.bat_icon) {
        case 0: // 当前显示 100%
            // 若电压下降并小于 80% 阈�?- 滞回 �?切换�?80% 图标
            if (v_mv < thr[0] - HYST_MV) return 1;
            return 0; // 否则保持 100%

        case 1: // 当前显示 80%
            // 如果电压上升并超�?80% 阈�?+ 滞回 �?回到 100%
            if (v_mv >= thr[0] + HYST_MV) return 0;
            // 如果电压下降并小�?60% 阈�?- 滞回 �?降到 60%
            if (v_mv < thr[1] - HYST_MV) return 2;
            return 1; // 否则保持 80%

        case 2: // 当前显示 60%
            if (v_mv >= thr[1] + HYST_MV) return 1; // 上升�?80%
            if (v_mv <  thr[2] - HYST_MV) return 3; // 下降�?40%
            return 2; // 保持 60%

        case 3: // 当前显示 40%
            if (v_mv >= thr[2] + HYST_MV) return 2; // 上升�?60%
            if (v_mv <  thr[3] - HYST_MV) return 4; // 下降�?20%
            return 3; // 保持 40%

        case 4: // 当前显示 20%
            if (v_mv >= thr[3] + HYST_MV) return 3; // 上升�?40%
            if (v_mv <  thr[4] - HYST_MV) return 5; // 下降�?5%
            return 4; // 保持 20%

        default: // 当前显示 5%（最低档，编�?5�?
            if (v_mv >= thr[4] + HYST_MV) return 4; // 电压上升�?20%
            return 5; // 否则保持 5%
    }
}
volatile float bat_value = 0.0f;
extern int app_get_battery_percent_for_log(void);

esp_err_t user_bat_read_voltage_once(float *battery_mv)
{
    if (battery_mv == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    adc_oneshot_unit_handle_t adc1_handle = NULL;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    esp_err_t ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ret = adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config);
    if (ret != ESP_OK) {
        adc_oneshot_del_unit(adc1_handle);
        return ret;
    }

    adc_cali_handle_t adc1_cali_handle = NULL;
    bool do_calibration1 = adc_calibration_init(ADC_UNIT_1, ADC_ATTEN_DB_12, &adc1_cali_handle);
    if (!do_calibration1) {
        adc_oneshot_del_unit(adc1_handle);
        return ESP_FAIL;
    }

    int adc_raw = 0;
    int voltage = 0;
    ret = adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &adc_raw);
    if (ret == ESP_OK) {
        ret = adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage);
    }

    if (ret == ESP_OK) {
        *battery_mv = (float)voltage * (144.2f / 44.2f);
    }

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_delete_scheme_curve_fitting(adc1_cali_handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_delete_scheme_line_fitting(adc1_cali_handle);
#endif
    adc_oneshot_del_unit(adc1_handle);

    return ret;
}

void bat_task(void *pvParameters)
{
    // uint8_t temp_data1 [2*sizeof(float) + 3];
    // temp_data1[0] = 0xAA;
    // temp_data1[5] = 0xBB;
    // temp_data1[6] = 0x85;


    //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config));
    adc_cali_handle_t adc1_cali_handle = NULL;
    bool do_calibration1 = adc_calibration_init(ADC_UNIT_1, ADC_ATTEN_DB_12, &adc1_cali_handle);

    int adc_raw = 0;
    int voltage = 0;
    float filert_vol_1= 0.0f;
    gpio_set_direction(USB_VBUS_DETECT_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(USB_VBUS_DETECT_GPIO);
    gpio_pulldown_dis(USB_VBUS_DETECT_GPIO);
    bool last_usb_connected = (gpio_get_level(USB_VBUS_DETECT_GPIO) == 1);
    while (1)
    {
        bool usb_connected = (gpio_get_level(USB_VBUS_DETECT_GPIO) == 1);
        if (last_usb_connected && !usb_connected) {
            bat_adc_reset_filter();
        }
        last_usb_connected = usb_connected;

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &adc_raw));
        if (do_calibration1) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage));//读取的ADC数值转换成电压
            filert_vol_1 = bat_adc_update(voltage);//滤波
            filert_vol_1 = filert_vol_1 * (144.2f / 44.2f);
            uint8_t new_bat_icon = map_voltage_to_icon(filert_vol_1);
            if (new_bat_icon != UI_icon.bat_icon) {
                UI_icon.bat_icon = new_bat_icon;
            }
            if (UI_icon.charge_icon != usb_connected) {
                UI_icon.charge_icon = usb_connected;
            }
            bat_value = filert_vol_1;
            ui_msg_t msg = { .type = UI_MSG_UPDATA_BAT_ICON };
            xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
            printf("bat_mV =%.2f bat_pct=%d gpio10=%d charge=%d\n", filert_vol_1, app_get_battery_percent_for_log(), gpio_get_level(USB_VBUS_DETECT_GPIO), UI_icon.charge_icon);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
