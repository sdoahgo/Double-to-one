#include "nvs_flash.h"
#include <string.h>
#include "user_hal.h"
#include "esp_log.h"

#define USER_CONFIG "CONFIG"
void  user_get_sys_evn(sys_data *data)
{
    nvs_handle_t my_handle;
    sys_data temp;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    ESP_LOGI("NVS", "nvs_open err= 0x%x\r\n", err);
    size_t required_size;
    err = nvs_get_blob(my_handle, USER_CONFIG, NULL, &required_size);//获取命名空间storage中键USER_CONFIG的大小
    ESP_LOGI("NVS", "nvs_get_blob err= 0x%x\r\n", err);
    err = nvs_get_blob(my_handle, USER_CONFIG, (char *)&temp, &required_size);//获取命名空间storage中键USER_CONFIG的内容
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
    }
    nvs_close(my_handle);
}
void user_save_sys_evn(sys_data *data)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
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

void user_device_init(void)
{
    user_get_sys_evn(&SYS_DATA);
    if (strlen(SYS_DATA.sn) == 0)
    {
        printf("SN = NULL\n");
        strcpy(SYS_DATA.sn, "NULL");
        SYS_DATA.language_id = 0;
        user_save_sys_evn(&SYS_DATA);
    }
}