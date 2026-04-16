#ifndef EEZ_LVGL_UI_GUI_H
#define EEZ_LVGL_UI_GUI_H

#include <stdbool.h>
#include <stdint.h>
#include <lvgl.h>

#include "screens.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_init();
void ui_tick();

void loadScreen(enum ScreensEnum screenId);
void loadScreenNoAnim(enum ScreensEnum screenId);
void left_or_right_Animation(lv_obj_t *target_object, int32_t start_value, int32_t end_value,
                             uint32_t anim_time, uint16_t cnt, uint32_t delay);
void top_or_bottom_Animation(lv_obj_t *target_object, int32_t start_value, int32_t end_value,
                             uint32_t anim_time, uint16_t cnt, uint32_t delay);

extern volatile uint8_t UI_page_id;
extern volatile bool GIF_end_flag;
extern volatile bool calibration_flags;
extern const char *const UI_STRING[][2];

#ifdef __cplusplus
}
#endif

#endif // EEZ_LVGL_UI_GUI_H
