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
#define ADC_ARR_SIZE 20
static int bat_adc_arr[ADC_ARR_SIZE];

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

        if(last_time_level == 1)
        {
            count = 0;
            last_time_level = 0; 
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

volatile float bat_value = 0.0f;
void bat_task(void *pvParameters)
{

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // 配置并安装UART驱动
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
    // 配置UART参数
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
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
    float last_voltage = 0.0f;
    uint8_t vol_cnt = 0;
    static float thr[5] = {3980.18f, 3892.09f, 3784.43f, 3624.57f, 3467.98f}; // 80/60/40/20/5%
    static bool armed[5] = {true, true, true, true, true};    // 允许触发的状态
    while (1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &adc_raw));
        if (do_calibration1) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage));//读取的ADC数值转换成电压
            filert_vol_1 = bat_adc_update(voltage);//滤波
            filert_vol_1 = filert_vol_1 * (144.2/44.2);
            if(last_voltage < filert_vol_1){
                vol_cnt++;
                if(vol_cnt == 3){
                    UI_icon.charge_icon = true;
                }
                 else {
                    vol_cnt = 0; // 建议补这一行，避免一次到3后永久为真（可选）
                }
            }
            // if(notify_state)
            // {
            //     // char data_user[16];
            //     memcpy(&temp_data1[1], &filert_vol_1, sizeof(float));
            //     // snprintf(data_user,sizeof(data_user),"bat:%.4f",filert_vol_1);
            //     int rct = user_send_notify((char *)temp_data1, sizeof(temp_data1));
            //     if(rct)
            //     {
            //         ESP_LOGE("BLE", "BLE notify fail\n");
            //     }
            // }

            // 放在你计算出 v_now 之后、更新 last_voltage 之前
            for (int i = 0; i < 5; ++i) {
                if (armed[i]) {
                    // 仅在“自上而下”跨过阈值−HYST 时触发
                    if (last_voltage >= thr[i] && filert_vol_1 < (thr[i] - 30.0f)) {
                        armed[i] = false;  // 解除武装，防抖
                        // TODO: 触发你的动作（更新图标/提示/上报等）
                        // printf("↓ 跨过 %d%% 阈值, v=%.1f mV\n", 100 - (i+1)*20, v_now);
                    }
                } else {
                    // 充电或回升到阈值 + HYST 以上才“重新武装”，允许下次再触发
                    if (filert_vol_1 > (thr[i] + 30.0f)) {
                        armed[i] = true;
                        // printf("↑ 重新武装 %d%% 阈值\n", 100 - (i+1)*20);
                    }
                }
            }
            last_voltage = filert_vol_1;
            bat_value = filert_vol_1;
            printf("bat_mV =%.2f\n",filert_vol_1);
            // memcpy(&temp_data1[1], &filert_vol_1, sizeof(float));
            // uart_write_bytes(UART_NUM_0, (const char*)temp_data1, sizeof(temp_data1));
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

    }
    
}
