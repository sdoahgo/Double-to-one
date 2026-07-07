#include "nvs_flash.h"
#include <string.h>
#include "user_hal.h"
#include "esp_log.h"

#define STORAGE_NAMESPACE "storage"
#define USER_CONFIG "CONFIG"

ui_product_state_t UI_DATA;
extern NVS_data_t NVS_data;

void user_ui_data_init(void)
{
    memset(&UI_DATA, 0, sizeof(UI_DATA));
    UI_DATA.target_page = UI_PRODUCT_PAGE_BOOT;
    UI_DATA.measure_state = UI_MEASURE_STATE_IDLE;
    UI_DATA.standby_time = UI_STANDBY_TIME_30_MIN;
}

void  user_get_sys_evn(sys_data *data)
{
    nvs_handle_t my_handle;
    sys_data temp;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    ESP_LOGI("NVS", "nvs_open err= 0x%x\r\n", err);
    size_t required_size;
    err = nvs_get_blob(my_handle, USER_CONFIG, NULL, &required_size);//获取命名空间storage中键USER_CONFIG的大�?
    ESP_LOGI("NVS", "nvs_get_blob err= 0x%x\r\n", err);
    err = nvs_get_blob(my_handle, USER_CONFIG, (char *)&temp, &required_size);//获取命名空间storage中键USER_CONFIG的内�?
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "get config_message error 0x%x\r\n", err);
    }
    else
    {
        memcpy(data,(uint8_t*)&temp,sizeof(sys_data));
        ESP_LOGI("NVS", "get config_message OK\r\n");
        ESP_LOGI("NVS", "==============================\r\n");
        ESP_LOGI("NVS", "language_id              :%d\r\n", data->language_id);
        ESP_LOGI("NVS", "sn  :%s\r\n", data->sn);
    }
    nvs_close(my_handle);
}
void user_save_sys_evn(sys_data *data)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    err = nvs_set_blob(my_handle, USER_CONFIG, (char *)data, sizeof(sys_data));
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "set rconfig_message error %d\r\n", err);
    }
    else
    {
        ESP_LOGI("NVS", "set config_message OK\r\n");
    }
    err=nvs_commit(my_handle);
    if (err == ESP_OK)
    {
        ESP_LOGI("NVS", "nvs_commit SUCCESS= 0x%x\r\n", err);
    }
    nvs_close(my_handle);
    user_get_sys_evn(data);
}

// 保存一个数值类型的值到NVS分区
esp_err_t NVS_Write_Num(const char *KeyName,int32_t Value)
{
	nvs_handle_t my_handle;			// NVS 操作句柄
	esp_err_t err;					// 操作错误定义
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);//打开 NVS 分区操作(空间名字（字符串），操作类型（只读还是读写），操作句�?
	if (err != ESP_OK) return err;
	err = nvs_set_i32(my_handle, KeyName, Value);// 写入一�?int32_t 的�?
	if (err != ESP_OK) return err;
	err = nvs_commit(my_handle);// 在设置修改任何操作后，必须使�?nvs_commit 提交修改，否则修改不生效
	if (err != ESP_OK) return err;
	nvs_close(my_handle);// 最后关闭操作句柄，结束修改操作
	return ESP_OK;
}

// 读取一个数值类型的�?
esp_err_t NVS_Read_Num(const char *KeyName, int32_t *ReadValue)
{
	nvs_handle_t my_handle;			// NVS 操作句柄
	esp_err_t err;					// 操作错误定义
	//打开 NVS 分区操作(空间名字（字符串），操作类型（只读还是读写），操作句�?
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) return err;
	// 读取一�?int32_t 的值，其它数值类型操作方式一样，如i8,u8,i16,u16,i32,u32等，调用对应的库函数即可
	int32_t readvalue = 0; // 要读取的值先赋�?，如果NVS没有此字段，默认值为设置的�?
	err = nvs_get_i32(my_handle, KeyName, &readvalue);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {// 读取错误 & 未找到NVS空间或字�?
		return err;
	}
	*ReadValue = readvalue;
	// 在设置修改任何操作后，必须使�?nvs_commit 提交修改，否则修改不生效
	err = nvs_commit(my_handle);
	if (err != ESP_OK) return err;
	// 最后关闭操作句柄，结束修改操作
	nvs_close(my_handle);
	return ESP_OK;
}

esp_err_t NVS_Write_Float(const char *key, float value)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(handle, key, &value, sizeof(float));
    err = nvs_commit(handle);// 在设置修改任何操作后，必须使�?nvs_commit 提交修改，否则修改不生效
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}

esp_err_t NVS_Read_Float(const char *key, float *value)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    size_t size = sizeof(float);
    err = nvs_get_blob(handle, key, value, &size);
    // err = nvs_commit(handle);// 在设置修改任何操作后，必须使�?nvs_commit 提交修改，否则修改不生效
    nvs_close(handle);
    return err;
}


// 保存一个字符串到NVS分区
esp_err_t NVS_Write_Str(const char *KeyName, const char *str)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;
    err = nvs_set_str(my_handle, KeyName, str); // 写入字符�?
    if (err != ESP_OK) {
        nvs_close(my_handle);
        return err;
    }
    err = nvs_commit(my_handle);
    nvs_close(my_handle);
    return err;
}

// 从NVS分区读取字符�?
esp_err_t NVS_Read_Str(const char *KeyName, char *out_str, size_t max_len)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    size_t required_size = 0;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    // 第一次调用获取字符串所需长度（包含结尾\0�?
    err = nvs_get_str(my_handle, KeyName, NULL, &required_size);
    if (err != ESP_OK) {
        nvs_close(my_handle);
        return err;
    }

    if (required_size > max_len) { // 用户传入的缓冲区太小
        nvs_close(my_handle);
        return ESP_ERR_NVS_VALUE_TOO_LONG;
    }

    // 读取字符串到 out_str
    err = nvs_get_str(my_handle, KeyName, out_str, &required_size);
    nvs_close(my_handle);
    return err;
}


void user_device_init(void)
{
    esp_err_t err;
    user_ui_data_init();
    user_get_sys_evn(&SYS_DATA);
    if (strlen(SYS_DATA.sn) == 0)
    // if(strncmp(SYS_DATA.sn, "NULL", 4) == 0)
    {
        ESP_LOGE("NVS","SN = NULL=============\n");
        strcpy(SYS_DATA.sn, "NULL");
        SYS_DATA.language_id = 0;
        SYS_DATA.temp_calibration_value = 0.0f;
        SYS_DATA.GIF_VALUE = 1;
        user_save_sys_evn(&SYS_DATA);
        NVS_data.fit_A1 = 1.0f;
        NVS_data.fit_B1 = 0.0f;
        NVS_data.fit_A2 = 1.0f;
        NVS_data.fit_B2 = 0.0f;
        NVS_data.turning_point = 60.0f;
        NVS_data.moisture_first_aiy = 3.435f;
        NVS_data.moisture_diff = 0.0f;
        NVS_Write_Float(fitting_value_a1,1.0f);
        NVS_Write_Float(fitting_value_b1,0.0f);
        NVS_Write_Float(fitting_value_a2,1.0f);
        NVS_Write_Float(fitting_value_b2,0.0f);
        NVS_Write_Float(nvs_turning_point, NVS_data.turning_point);
        NVS_Write_Float(nvs_moisture_first_aiy, NVS_data.moisture_first_aiy);
        NVS_Write_Float(nvs_moisture_diff, NVS_data.moisture_diff);
    }
    else{
        err = NVS_Read_Float(fitting_value_a1, &NVS_data.fit_A1);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            NVS_data.fit_A1 = 1.0f;
            ESP_LOGW("NVS", "fit_A1 not found, set default 0");
            NVS_Write_Float(fitting_value_a1, NVS_data.fit_A1);
        } else if (err != ESP_OK) {
            ESP_LOGE("NVS", "Read fit_A1 failed: 0x%x", err);
        }else{
            ESP_LOGE("NVS", "Read fit_A1 success: 0x%x", err);
        }

        err = NVS_Read_Float(fitting_value_b1, &NVS_data.fit_B1);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            NVS_data.fit_B1 = 0.0f;
            ESP_LOGW("NVS", "fit_B1 not found, set default 0");
            NVS_Write_Float(fitting_value_b1, NVS_data.fit_B1);
        } else if (err != ESP_OK) {
            ESP_LOGE("NVS", "Read fit_B1 failed: 0x%x", err);
        }else{
            ESP_LOGE("NVS", "Read fit_B1 success: 0x%x", err);
        }

        err = NVS_Read_Float(fitting_value_a2, &NVS_data.fit_A2);
        if (err == ESP_ERR_NVS_NOT_FOUND){
            NVS_data.fit_A2 = 1.0f;
            ESP_LOGW("NVS", "fit_A2 not found, set default 0");
            NVS_Write_Float(fitting_value_a2, NVS_data.fit_A2);
        } else if (err != ESP_OK){
            ESP_LOGE("NVS", "Read fit_A2 failed: 0x%x", err);
        }else{
            ESP_LOGE("NVS", "Read fit_A2 success: 0x%x", err);
        }

        err = NVS_Read_Float(fitting_value_b2, &NVS_data.fit_B2);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            NVS_data.fit_B2 = 0.0f;
            ESP_LOGW("NVS", "fit_B2 not found, set default 0");
            NVS_Write_Float(fitting_value_b2, NVS_data.fit_B2);
        } else if (err != ESP_OK) {
            ESP_LOGE("NVS", "Read fit_B2 failed: 0x%x", err);
        }else{
            ESP_LOGE("NVS", "Read fit_B2 success: 0x%x", err);
        }

        err = NVS_Read_Float(nvs_turning_point, &NVS_data.turning_point);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            NVS_data.turning_point = 60.0f;
            ESP_LOGW("NVS", "turning_point not found, set default 0");
            NVS_Write_Float(nvs_turning_point, NVS_data.turning_point);
        } else if (err != ESP_OK) {
            ESP_LOGE("NVS", "Read turning_point failed: 0x%x", err);
        }else{
            ESP_LOGE("NVS", "Read turning_point success: 0x%x", err);
        }

        err = NVS_Read_Float(nvs_moisture_first_aiy, &NVS_data.moisture_first_aiy);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            NVS_data.moisture_first_aiy = 3.435f;
            ESP_LOGW("NVS", "moisture_first_aiy not found, set default");
            NVS_Write_Float(nvs_moisture_first_aiy, NVS_data.moisture_first_aiy);
        } else if (err != ESP_OK) {
            ESP_LOGE("NVS", "Read moisture_first_aiy failed: 0x%x", err);
        } else {
            ESP_LOGE("NVS", "Read moisture_first_aiy success: 0x%x", err);
        }

        err = NVS_Read_Float(nvs_moisture_diff, &NVS_data.moisture_diff);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            NVS_data.moisture_diff = 0.0f;
            ESP_LOGW("NVS", "moisture_diff not found, set default");
            NVS_Write_Float(nvs_moisture_diff, NVS_data.moisture_diff);
        } else if (err != ESP_OK) {
            ESP_LOGE("NVS", "Read moisture_diff failed: 0x%x", err);
        } else {
            ESP_LOGE("NVS", "Read moisture_diff success: 0x%x", err);
        }
        
    }
    NVS_data.config_data = SYS_DATA;
    ESP_LOGI("NVS", "fit_A1 = %f, fit_B1 = %f\r\n" ,NVS_data.fit_A1,NVS_data.fit_B1);
    ESP_LOGI("NVS", "fit_A2 = %f, fit_B2 = %f\r\n" ,NVS_data.fit_A2,NVS_data.fit_B2);
    ESP_LOGI("NVS", "turning_point = %f\r\n" ,NVS_data.turning_point);
    ESP_LOGI("NVS", "moisture_first_aiy = %f, moisture_diff = %f\r\n",
             NVS_data.moisture_first_aiy, NVS_data.moisture_diff);
}
