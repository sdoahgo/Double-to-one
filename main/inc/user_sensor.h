#ifndef USER_SENSOR_H
#define USER_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile float SHT45_TEMP;
extern volatile float SHT45_HUMIDITY;
extern volatile float MDC04_VALUE;
extern volatile int32_t CS1237_VALUE;

extern volatile bool SHT45_LINK;
extern volatile bool MDC04_LINK;
extern volatile bool CS1237_LINK;

void user_sensor_init(void);

#ifdef __cplusplus
}
#endif

#endif
