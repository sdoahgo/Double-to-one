#include "GD60914.h"
#include "user_hal.h"
#include "user_ble.h"

SemaphoreHandle_t i2c_mutex;

esp_err_t myi2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;  
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0,
    };

    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG1, "i2c_param_config error: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG1, "i2c%d_master_int", I2C_MASTER_NUM);

    err = i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG1, "i2c_driver_install error: %s", esp_err_to_name(err));
    }
    return err;
}

GD60914_handler_t GD60914_handler={
    .I2C_NUM = 0,
    .GD60914_ADDR = GD60914_addr,
    .read = GD60914_read,
};

esp_err_t GD60914_read(const struct GD60914_HANDLER *handle, uint8_t reg, uint8_t *data, uint8_t length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                       //创建任务句柄
    i2c_master_start(cmd);                                                              //起始信号
    i2c_master_write_byte(cmd,(handle->GD60914_ADDR << 1)|WRITE_BIT,ACK_CHECK_EN);                        //从机地址及读写位,读
    i2c_master_write_byte(cmd,reg,ACK_CHECK_EN);

    i2c_master_start(cmd);                                                              //起始信号
    i2c_master_write_byte(cmd,(handle->GD60914_ADDR<<1)|READ_BIT,ACK_CHECK_EN);                        //从机地址及读写位,读

    i2c_master_read(cmd, data, length, I2C_MASTER_LAST_NACK);                                 //数据位(数组)，读取到data数组上，个应答表示还要读取

    i2c_master_stop(cmd);                                                               //终止信号        
    esp_err_t ret = i2c_master_cmd_begin(handle->I2C_NUM, cmd, 1000);        //向i2c_num发送这个数据帧，timeout设置为1000毫秒
    i2c_cmd_link_delete(cmd);                                                           //删除i2c_cmd_handle_t对象，释放资源

    return ret;
}

void myi2c_Init(void)
{
    i2c_mutex = xSemaphoreCreateMutex();
    esp_err_t error = myi2c_master_init();
    ESP_LOGI(TAG1,"1111%s",esp_err_to_name(error));
    printf("1111111\n");
}

volatile float GD60914_TEMP = 0.0f;
void GD60914_task(void *pvParameters)
{
    int16_t temperature = 0;
    float temperature_value =0;
    uint8_t GD60914_data[2]={0};
    // uint8_t temp_data1 [1*sizeof(float) + 3];
    // temp_data1[0] = 0xAA;
    // temp_data1[5] = 0xBB;
    // temp_data1[6] = 0x85;
    while (1)
    {
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
            esp_err_t ret = GD60914_handler.read(&GD60914_handler, 0x1F, GD60914_data, 2);
            xSemaphoreGive(i2c_mutex);
        }
        temperature = (GD60914_data[1]<<8) | (GD60914_data[0]) ;
        // printf("0x1F :%x\n",temperature);
        vTaskDelay(pdMS_TO_TICKS(500));
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
            esp_err_t ret = GD60914_handler.read(&GD60914_handler, 0x1C, GD60914_data, 2);
            xSemaphoreGive(i2c_mutex);
        }
        temperature = ((GD60914_data[1]<<8) | GD60914_data[0]);
        // temperature_value = temperature/10.0; 
        temperature_value = 1.7126926 + 1.05611670*temperature;
        GD60914_TEMP = temperature_value / 10.0f;
        // printf("0x1C :%d %.1f\n",temperature,temperature_value);
            // if(notify_state)
            // {
            //     // char data_user[16];
            //     memcpy(&temp_data1[1], &temperature_value, sizeof(float));
            //     // snprintf(data_user,sizeof(data_user),"bat:%.4f",filert_vol_1);
            //     int rct = user_send_notify((char *)temp_data1, sizeof(temp_data1));
            //     if(rct)
            //     {
            //         ESP_LOGE("BLE", "BLE notify fail\n");
            //     }
            // }
        ui_msg_t msg = {
            .type = UI_MSG_UPDATE_TEMP,
            .temp_value = temperature_value,
        };
        xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
    }
}