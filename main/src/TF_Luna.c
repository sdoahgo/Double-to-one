#include "TF_Luna.h"
#include "ui.h"
#include "driver/gpio.h"


static TaskHandle_t TF_Luna_task_handle = NULL;

esp_err_t TF_Luna_read(uint8_t reg, uint8_t *data, uint8_t length)
{
    esp_err_t ret = i2c_master_write_read_device(I2C_NUM_0,TF_Luna_addr,&reg,1,data,length,5000/ portTICK_PERIOD_MS);
    return ret;
}

esp_err_t TF_Luna_write(uint8_t reg, uint8_t *data, uint8_t length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                       //创建任务句柄
    i2c_master_start(cmd);                                                              //起始信号
    i2c_master_write_byte(cmd,(TF_Luna_addr<<1)|TF_WRITE_BIT,TF_ACK_CHECK_EN);                        //从机地址及读写位,读
    i2c_master_write_byte(cmd, reg, TF_ACK_CHECK_EN);
    i2c_master_write(cmd, data, length, TF_ACK_CHECK_EN);
    i2c_master_stop(cmd);                                                               //终止信号        
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000);        //向i2c_num发送这个数据帧，timeout设置为1000毫秒
    i2c_cmd_link_delete(cmd);                                                           //删除i2c_cmd_handle_t对象，释放资源
    return ret;
}

esp_err_t TF_Luna_write_byte(uint8_t reg, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg, data};
    ret = i2c_master_write_to_device(I2C_NUM_0,TF_Luna_addr, write_buf,sizeof(write_buf),5000/ portTICK_PERIOD_MS);
    
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

void TF_Luna_init(void)
{
    // TF_Luna_write_byte(0x1f,0x00); //标准模式
    // TF_Luna_write_byte(0x23,0x00); //连续工作模式
    TF_Luna_write_byte(0x26,0x05); //帧率10
    TF_Luna_write_byte(0x28,0x01); //低功耗模式

    xTaskCreate(TF_Luna_task, "TF_Luna_task", (1024 * 4), (void *)NULL, (tskIDLE_PRIORITY+5), &TF_Luna_task_handle);
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TF_IO_OUT),      
        .mode = GPIO_MODE_INPUT,       
        .pull_up_en = GPIO_PULLUP_DISABLE, 
        .pull_down_en = GPIO_PULLDOWN_DISABLE, 
        .intr_type = GPIO_INTR_POSEDGE, 
    };
    gpio_config(&io_conf);              // 应用配置
    gpio_install_isr_service(0); // 参数0一般用默认
    gpio_isr_handler_add(TF_IO_OUT, gpio_isr_handler, (void*)TF_IO_OUT);
}

void TF_Luna_task(void *pvParameters)
{
    uint8_t i =0;
    uint8_t datas[4] = {0};
    uint8_t data = 0;
    vTaskDelay(pdMS_TO_TICKS(500));
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
        esp_err_t err = TF_Luna_read(0x28, &data, 1);
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
        esp_err_t err = TF_Luna_read(0x26, &data, 1);
        if(err != ESP_OK)
        {                                                                                                                                
            ESP_LOGE(TF_TAG, "TF read MODE error: %s", esp_err_to_name(err));
        }
        else{
            ESP_LOGI(TF_TAG, "TF read 0x26: %x", data);
        }
    xSemaphoreGive(i2c_mutex);
    }
    // err = TF_Luna_read(SN_BASE, datas, 4);
    // if(err != ESP_OK)
    // {                                                                                                                                
    //     ESP_LOGE(TF_TAG, "TF read sn error: %s", esp_err_to_name(err));
    // }
    // printf("data: ");
    // for (int i = 0; i < 4; i++) {
    //     printf("%02X ", datas[i]);
    // }
    // printf("\n");
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        printf("-------%d\n",i++);
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY)) {
            esp_err_t err = TF_Luna_read(DIST_LOW, datas, 4);
            xSemaphoreGive(i2c_mutex);
        }
        uint16_t DIST = datas[1]<<8 | datas[0];
        uint16_t AMP =  datas[3]<<8 | datas[2];
        ui_msg_t msg = {
            .type = UI_MSG_UPDATE_850,
            .TF_DIST_value = DIST,
            .TF_AMP_value = AMP,
        };
        xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
        printf("DIST %dcm AMP %d",DIST,AMP);
        // if(err != ESP_OK)
        // {                                                                                                                                
        //     ESP_LOGE(TF_TAG, "TF read sn error: %s", esp_err_to_name(err));
        // }
        // printf("SN_ID: ");
        // for (int i = 0; i < 10; i++) {
        //     printf("%02X ", datas[i]);
        // }
        // printf("\n");
        // if(calibration_flags)
        // {
        //     ui_msg_t msg = {
        //     .type = UI_MSG_UPDATE_850,
        //     .data.Laser_value = 0,
        //     };
        //     vTaskDelay(pdMS_TO_TICKS(2000));
        //     xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
        //     calibration_flags = false;
        // }
        // else{
        //     vTaskDelay(pdMS_TO_TICKS(2000));
        // }
    }
}