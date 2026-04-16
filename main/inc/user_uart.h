#ifndef USER_UART_H
#define USER_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    float a1;
    float b1;
    float a2;
    float b2;
    float turning_point;
}fitting_t;

void user_spiffs_init(void);
void user_uart_init(void);
void user_uart_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* __USER_HAL_H */