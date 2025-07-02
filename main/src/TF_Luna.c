#include "TF_Luna.h"
#include "ui.h"

void TF_Luna_task(void *pvParameters)
{
    while (1)
    {
        
        if(calibration_flags)
        {
            ui_msg_t msg = {
            .type = UI_MSG_UPDATE_850,
            .data.Laser_value = 0,
            };
            vTaskDelay(pdMS_TO_TICKS(2000));
            xQueueSend(ui_msg_queue, &msg, portMAX_DELAY);
            calibration_flags = false;
        }
        else{
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}