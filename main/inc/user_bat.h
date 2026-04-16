#ifndef USER_BAT_
#define USER_BAT_

#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif

void bat_task(void *pvParameters);
esp_err_t user_bat_read_voltage_once(float *battery_mv);
extern volatile float bat_value;


#ifdef __cplusplus
}
#endif
#endif 
