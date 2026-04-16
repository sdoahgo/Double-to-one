#ifndef USER_KEY_H
#define USER_KEY_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
    USER_KEY_SHORT_IN_EVT,
    USER_KEY_LONG_IN_EVT,
    USER_KEY_DOUBLE_CLICK_IN_EVT,
    USER_KEY_POWER_LONG_IN_EVT,
    USER_KEY_MULTIPLE_CLICK_IN_EVT,
}user_key_event;

extern QueueHandle_t KeyQueue;
extern volatile uint8_t key_event;
void user_key_init(void);
void switch_ota_and_reboot(void);

#ifdef __cplusplus
}
#endif
#endif
