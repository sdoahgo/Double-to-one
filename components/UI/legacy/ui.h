#ifndef EEZ_LVGL_UI_GUI_H
#define EEZ_LVGL_UI_GUI_H

#include <lvgl.h>
#include "user_hal.h"


#if defined(EEZ_FOR_LVGL)
#include <eez/flow/lvgl_api.h>
#endif

#if !defined(EEZ_FOR_LVGL)
#include "screens.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern volatile uint8_t UI_page_id;

extern volatile bool GIF_end_flag; //gif完成动画标志
extern volatile uint8_t Screens_ID;
extern volatile bool calibration_flags;
/* Future EEZ Studio product pages are reserved in ui_product_page_t / UI_DATA. */
/*设置对象的宽度和高度为其父对象的 100%。
将背景颜色设置为黑色。
设置对象的内边距为 0（无内边距）。
去除对象的边框。
禁用对象的滚动条。*/
#define UI_PARENT_INIT(PARENT)	    do{lv_obj_set_size(PARENT,LV_PCT(100),LV_PCT(100));\
    lv_obj_set_style_bg_color(PARENT,lv_color_black(),0); \
    lv_obj_set_style_pad_all(PARENT,0,0); \
	lv_obj_set_style_border_width(PARENT, 0, 0); \
	lv_obj_set_scrollbar_mode(PARENT, LV_SCROLLBAR_MODE_OFF);}while(0)

void lv_example_gif2(void);
void ui_init();
void ui_tick();
void left_or_right_Animation(lv_obj_t * TargetObject, int32_t start_value, int32_t end_value, uint32_t anim_time, uint16_t cnt, uint32_t delay);
void ui_event_page_load_tds_screen(lv_event_t * e);
void top_or_bottom_Animation(lv_obj_t * TargetObject, int32_t start_value, int32_t end_value, uint32_t anim_time, uint16_t cnt, uint32_t delay);
void ui_calibration_event(lv_event_t * e);
// void ui_calibration_success(ui_msg_t *msg);

const static char* const UI_STRING[][2] = {
    {"温度", "Temp"},//0
    {"双击Home键切换语言", "Double-tap the Home button\nto switch languages"},//1
    {"双击Home键进行校准", "Double-tap the Home button\nto Zeroing"},//2
    {"校准", "ZEROING"},//3
    {"校准中", "ZEROING..."},//4
    {"校准成功", "ZEROING SUCCEED"},//5
    {"产品名", "Name"},//6
    {"型号", "Model"},//7
    {"硬件版本", "Hardware Version"},//8
    {"内核版本", "Kernal Version"},//9
    {"电量低\n请充电", "Low battery!\nPlease charge!"}//10
};

#if !defined(EEZ_FOR_LVGL)
void loadScreen(enum ScreensEnum screenId);
#endif

#ifdef __cplusplus
}
#endif

#endif // EEZ_LVGL_UI_GUI_H
