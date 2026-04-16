#ifndef GD60914_H
#define GD60914_H

#ifdef __cplusplus
extern "C" {
#endif

extern volatile float GD60914_TEMP;

void GD60914_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif
