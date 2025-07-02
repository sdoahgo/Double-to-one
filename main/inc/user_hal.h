#ifndef USER_HAL_H
#define USER_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define  TOW_modes 0 //定义为0，是温度，定义为1是TDS
typedef struct 
{
    uint8_t language_id;
    char sn[20];
}sys_data;
extern sys_data SYS_DATA;

typedef enum{
    UI_MSG_UPDATE_TEMP,
    UI_MSG_UPDATE_850,
    UI_MSG_UPDATE_LANUAGE,
    UI_MSG_UPDATE_STATE
}ui_msg_type_t;

typedef struct 
{
    ui_msg_type_t type;
    union {
        int16_t temp_value;       // 4 字节（float类型，通常是4字节）
        uint32_t Laser_value;
        char alert_str[32];     // 32 字节（32个char，每个1字节）
        // 其它数据（如有更大则按更大算）
    } data;
}ui_msg_t;


// extern Sensor_Data sensors_data;
extern QueueHandle_t ui_msg_queue;
void user_get_sys_evn(sys_data *data);
void user_save_sys_evn(sys_data *data);
void user_device_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USER_HAL_H */