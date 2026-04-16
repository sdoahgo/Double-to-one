#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"
#include "user_hal.h"

static int16_t currentScreen = -1;
volatile uint8_t UI_page_id = 0;
volatile bool GIF_end_flag = false;
volatile bool calibration_flags = false;
static bool s_boot_anim_started = false;
static bool s_boot_anim_finished = false;

LV_IMG_DECLARE(LEBREW);

const char *const UI_STRING[][2] = {
    {"Temp", "Temp"},
    {"Double-tap Home to switch languages", "Double-tap Home to switch languages"},
    {"", ""},
    {"", ""},
    {"ZEROING...", "ZEROING..."},
    {"ZEROING SUCCEED", "ZEROING SUCCEED"},
    {"Name", "Name"},
    {"Model", "Model"},
    {"Hardware Version", "Hardware Version"},
    {"Software Version", "Software Version"},
    {"Low battery! Please charge!", "Low battery! Please charge!"},
};

static lv_obj_t *getLvglObjectFromIndex(int32_t index) {
    if (index == -1) {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

static void ui_boot_anim_cleanup_cb(lv_timer_t *timer) {
    lv_obj_t *boot_container = (lv_obj_t *)timer->user_data;
    if (boot_container) {
        lv_obj_del(boot_container);
    }
    lv_timer_del(timer);
    loadScreen(UI_FIRST_CYCLING_SCREEN);
    GIF_end_flag = true;
}

static void ui_boot_anim_ready_cb(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_READY) {
        return;
    }
    if (s_boot_anim_finished) {
        return;
    }
    s_boot_anim_finished = true;

    lv_obj_t *boot_container = lv_obj_get_parent(lv_event_get_target(e));
    lv_timer_t *cleanup_timer = lv_timer_create(ui_boot_anim_cleanup_cb, 300, boot_container);
    if (cleanup_timer) {
        lv_timer_set_repeat_count(cleanup_timer, 1);
    }
}

static void ui_play_boot_animation(void) {
    if (s_boot_anim_started) {
        return;
    }
    s_boot_anim_started = true;
    s_boot_anim_finished = false;

    lv_obj_t *boot_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(boot_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(boot_container, 0, 0);
    lv_obj_set_style_border_width(boot_container, 0, 0);
    lv_obj_set_style_radius(boot_container, 0, 0);
    lv_obj_set_style_bg_color(boot_container, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(boot_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(boot_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *gif = lv_gif_create(boot_container);
    lv_gif_set_src(gif, &LEBREW);
    lv_obj_center(gif);
    ((lv_gif_t *)gif)->gif->loop_count = 1;
    lv_obj_add_event_cb(gif, ui_boot_anim_ready_cb, LV_EVENT_READY, NULL);
}

static void ui_load_screen_internal(enum ScreensEnum screenId, lv_scr_load_anim_t anim_type, uint32_t time) {
    UI_page_id = screenId;
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    lv_scr_load_anim(screen, anim_type, time, 0, false);
}

void loadScreen(enum ScreensEnum screenId) {
    ui_load_screen_internal(screenId, LV_SCR_LOAD_ANIM_FADE_IN, 200);
}

void loadScreenNoAnim(enum ScreensEnum screenId) {
    ui_load_screen_internal(screenId, LV_SCR_LOAD_ANIM_NONE, 0);
}

void ui_init() {
    create_screens();
    currentScreen = -1;
    UI_page_id = 0;
    GIF_end_flag = false;
    s_boot_anim_started = false;
    s_boot_anim_finished = false;
    ui_play_boot_animation();
}

void ui_tick() {
    tick_screen(currentScreen);
}

static void ui_start_axis_animation(lv_obj_t *target_object, lv_anim_exec_xcb_t exec_cb,
                                    int32_t start_value, int32_t end_value,
                                    uint32_t anim_time, uint16_t cnt, uint32_t delay) {
    if (!target_object) {
        return;
    }

    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_delay(&animation, delay);
    lv_anim_set_exec_cb(&animation, exec_cb);
    lv_anim_set_var(&animation, target_object);
    lv_anim_set_time(&animation, anim_time);
    lv_anim_set_values(&animation, start_value, end_value);
    lv_anim_set_path_cb(&animation, lv_anim_path_overshoot);
    lv_anim_set_repeat_count(&animation, cnt);
    lv_anim_set_early_apply(&animation, false);
    lv_anim_start(&animation);
}

void left_or_right_Animation(lv_obj_t *target_object, int32_t start_value, int32_t end_value,
                             uint32_t anim_time, uint16_t cnt, uint32_t delay) {
    ui_start_axis_animation(target_object, (lv_anim_exec_xcb_t)lv_obj_set_x,
                            start_value, end_value, anim_time, cnt, delay);
}

void top_or_bottom_Animation(lv_obj_t *target_object, int32_t start_value, int32_t end_value,
                             uint32_t anim_time, uint16_t cnt, uint32_t delay) {
    ui_start_axis_animation(target_object, (lv_anim_exec_xcb_t)lv_obj_set_y,
                            start_value, end_value, anim_time, cnt, delay);
}
