#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "GD60914.h"

volatile float GD60914_TEMP = 0.0f;

void GD60914_task(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
