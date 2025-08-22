#include "TF_Luna.h"
#include "ui.h"
#include "driver/gpio.h"
#include "user_hal.h"
#include "user_ble.h"

#ifdef USE_TDS
static TaskHandle_t TF_Luna_task_handle = NULL;
TF_Luna_handler_t TF_Luna_handler={
    .IO_OUT = 10,
    .I2C_NUM = 0,
    .TF_ADDR = TF_Luna_addr,
    .init = TF_Luna_init,
    .read = TF_Luna_read,
    .write = TF_Luna_write,
    .write_byte = TF_Luna_write_byte,
};

void driver_TF_Luna_init(void)
{
    TF_Luna_handler.init(&TF_Luna_handler);
}


esp_err_t TF_Luna_read(const struct TF_Luna_HANDLER *handle, uint8_t reg, uint8_t *data, uint8_t length)
{
    esp_err_t ret = i2c_master_write_read_device(handle->I2C_NUM, handle->TF_ADDR, &reg, 1, data, length,5000/ portTICK_PERIOD_MS);
    return ret;
}

esp_err_t TF_Luna_write(const struct TF_Luna_HANDLER *handle, uint8_t reg, uint8_t *data, uint8_t length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                       //创建任务句柄
    i2c_master_start(cmd);                                                              //起始信号
    i2c_master_write_byte(cmd,(handle->TF_ADDR<<1)|TF_WRITE_BIT,TF_ACK_CHECK_EN);                        //从机地址及读写位,读
    i2c_master_write_byte(cmd, reg, TF_ACK_CHECK_EN);
    i2c_master_write(cmd, data, length, TF_ACK_CHECK_EN);
    i2c_master_stop(cmd);                                                               //终止信号        
    esp_err_t ret = i2c_master_cmd_begin(handle->I2C_NUM, cmd, 1000);        //向i2c_num发送这个数据帧，timeout设置为1000毫秒
    i2c_cmd_link_delete(cmd);                                                           //删除i2c_cmd_handle_t对象，释放资源
    return ret;
}

esp_err_t TF_Luna_write_byte(const struct TF_Luna_HANDLER *handle, uint8_t reg, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg, data};
    ret = i2c_master_write_to_device(handle->I2C_NUM ,handle->TF_ADDR, write_buf,sizeof(write_buf),5000/ portTICK_PERIOD_MS);
    
    return ret;
} 

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(TF_Luna_task_handle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void TF_Luna_init(const struct TF_Luna_HANDLER *handle)
{
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
        // TF_Luna_write_byte(0x1f,0x00); //标准模式
        // TF_Luna_write_byte(0x23,0x00); //连续工作模式
        handle->write_byte(handle, 0x26, 0x05); //帧率10
        vTaskDelay(pdMS_TO_TICKS(100));
        handle->write_byte(handle, 0x28, 0x01); //低功耗模式
    xSemaphoreGive(i2c_mutex);
    }
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << handle->IO_OUT),      
        .mode = GPIO_MODE_INPUT,       
        .pull_up_en = GPIO_PULLUP_DISABLE, 
        .pull_down_en = GPIO_PULLDOWN_DISABLE, 
        .intr_type = GPIO_INTR_POSEDGE, 
    };
    gpio_config(&io_conf);              // 应用配置
    gpio_install_isr_service(0); // 参数0一般用默认
    gpio_isr_handler_add(handle->IO_OUT, gpio_isr_handler, (void*) handle->IO_OUT);
    xTaskCreate(TF_Luna_task, "TF_Luna_task", (1024 * 4), (void *)NULL, (tskIDLE_PRIORITY+5), &TF_Luna_task_handle);
}
extern volatile float GD60914_TEMP;
void TF_Luna_task(void *pvParameters)
{
    uint8_t i =0;
    uint8_t datas[4] = {0};
    uint8_t data = 0;
    // uint8_t temp_data1 [1*sizeof(float) + 2*sizeof(uint16_t) + 3];
    // temp_data1[0] = 0xAA;
    // temp_data1[9] = 0xBB;
    // temp_data1[10] = 0x85;
    char data_user[16];
    vTaskDelay(pdMS_TO_TICKS(500));
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
        esp_err_t err = TF_Luna_handler.read(&TF_Luna_handler, 0x28, &data, 1);
        if(err != ESP_OK)
        {                                                                                                                                
            ESP_LOGE(TF_TAG, "TF read LOW_POWER error: %s", esp_err_to_name(err));
        }
        else{
            ESP_LOGI(TF_TAG, "TF read 0x28: %x", data);
        }
    xSemaphoreGive(i2c_mutex);
    }
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
        esp_err_t err = TF_Luna_handler.read(&TF_Luna_handler, 0x26, &data, 1);
        if(err != ESP_OK)
        {                                                                                                                                
            ESP_LOGE(TF_TAG, "TF read MODE error: %s", esp_err_to_name(err));
        }
        else{
            ESP_LOGI(TF_TAG, "TF read 0x26: %x", data);
        }
    xSemaphoreGive(i2c_mutex);
    }
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        printf("-------%d\n",i++);
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
            esp_err_t err = TF_Luna_handler.read(&TF_Luna_handler, DIST_LOW, datas, 4);
            xSemaphoreGive(i2c_mutex);
        }
        uint16_t DIST = datas[1]<<8 | datas[0];
        uint16_t AMP =  datas[3]<<8 | datas[2];
            // if(notify_state)
            // {
            //     // char data_user[16];
            //     memcpy(&temp_data1[1], &GD60914_TEMP, sizeof(float));
            //     memcpy(&temp_data1[5], &DIST, sizeof(uint16_t));
            //     memcpy(&temp_data1[7], &AMP, sizeof(uint16_t));
            //     // snprintf(data_user,sizeof(data_user),"bat:%.4f",filert_vol_1);
            //     int rct = user_send_notify((char *)temp_data1, sizeof(temp_data1));
            //     if(rct)
            //     {
            //         ESP_LOGE("BLE", "BLE notify fail\n");
            //     }
            // }
        ui_msg_t msg = {
            .type = UI_MSG_UPDATE_850,
            .TF_DIST_value = DIST,
            .TF_AMP_value = AMP,
        };
        xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
        printf("DIST %dcm AMP %d",DIST,AMP);
        if (notify_state)
        {
            int len = snprintf(data_user, sizeof(data_user), "AMP:%d\r\n",AMP);
            int rct = user_send_notify(data_user, strlen(data_user));
            if (rct) {
                ESP_LOGE("BLE", "BLE notify fail\n");
            }
        }
    }
}

#endif