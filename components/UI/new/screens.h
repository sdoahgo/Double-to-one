#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MEASURE = 1,
    SCREEN_ID_MEASURE_ROASTED = 2,
    SCREEN_ID_MEASURE_GREEN = 3,
    SCREEN_ID_MEASURE_PARCHMENT = 4,
    SCREEN_ID_MEASURE_DRY_CHERRY = 5,
    SCREEN_ID_SETTING = 6,
    SCREEN_ID_SETTING_LANGUAGE = 7,
    SCREEN_ID_ZERONG = 8,
    SCREEN_ID_ZERONG_M_D = 9,
    SCREEN_ID_ZERONG_AW = 10,
    SCREEN_ID_ZERONG_M_D_ING = 11,
    SCREEN_ID_ZERONG_AW_ING = 12,
    SCREEN_ID_ZERONG_M_D_OK = 13,
    SCREEN_ID_ZERONG_AW_OK = 14,
    SCREEN_ID_ZERONG_AW_WARM = 15,
    SCREEN_ID_ZERONG_M_D_SUCCESS = 16,
    SCREEN_ID_ZERONG_AW_SUCCESS = 17,
    SCREEN_ID_ABOUT = 18,
    SCREEN_ID_ABOUT_MESSAGE = 19,
    SCREEN_ID_SETTING_LANGUAGE_CN = 20,
    _SCREEN_ID_LAST = 20
};

typedef struct _objects_t {
    lv_obj_t *measure;
    lv_obj_t *measure_roasted;
    lv_obj_t *measure_green;
    lv_obj_t *measure_parchment;
    lv_obj_t *measure_dry_cherry;
    lv_obj_t *setting;
    lv_obj_t *setting_language;
    lv_obj_t *zerong;
    lv_obj_t *zerong_m_d;
    lv_obj_t *zerong_aw;
    lv_obj_t *zerong_m_d_ing;
    lv_obj_t *zerong_aw_ing;
    lv_obj_t *zerong_m_d_ok;
    lv_obj_t *zerong_aw_ok;
    lv_obj_t *zerong_aw_warm;
    lv_obj_t *zerong_m_d_success;
    lv_obj_t *zerong_aw_success;
    lv_obj_t *about;
    lv_obj_t *about_message;
    lv_obj_t *setting_language_cn;
    lv_obj_t *setting_language_chinese_img;
    lv_obj_t *setting_language_english_img;
    lv_obj_t *temp_container;
    lv_obj_t *status_container;
    lv_obj_t *bat;
    lv_obj_t *status_container_1;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *line_1;
    lv_obj_t *line_2;
    lv_obj_t *roasted;
    lv_obj_t *line_white;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *line_3;
    lv_obj_t *line_4;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *status_container_2;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *line_5;
    lv_obj_t *line_6;
    lv_obj_t *green;
    lv_obj_t *line_white_1;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *line_7;
    lv_obj_t *line_8;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *status_container_3;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *line_9;
    lv_obj_t *line_10;
    lv_obj_t *parchment;
    lv_obj_t *line_white_2;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *line_11;
    lv_obj_t *line_12;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *status_container_4;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *line_13;
    lv_obj_t *line_14;
    lv_obj_t *dry_cherry;
    lv_obj_t *line_white_3;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *line_15;
    lv_obj_t *line_16;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *obj27;
    lv_obj_t *temp_container_1;
    lv_obj_t *status_container_5;
    lv_obj_t *bat_1;
    lv_obj_t *status_container_6;
    lv_obj_t *obj28;
    lv_obj_t *obj29;
    lv_obj_t *temp_container_2;
    lv_obj_t *status_container_7;
    lv_obj_t *bat_2;
    lv_obj_t *status_container_8;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *status_container_9;
    lv_obj_t *obj32;
    lv_obj_t *obj33;
    lv_obj_t *obj34;
    lv_obj_t *status_container_10;
    lv_obj_t *status_container_11;
    lv_obj_t *status_container_12;
    lv_obj_t *obj35;
    lv_obj_t *obj36;
    lv_obj_t *status_container_13;
    lv_obj_t *obj37;
    lv_obj_t *obj38;
    lv_obj_t *status_container_14;
    lv_obj_t *obj39;
    lv_obj_t *obj40;
    lv_obj_t *status_container_15;
    lv_obj_t *obj41;
    lv_obj_t *obj42;
    lv_obj_t *status_container_16;
    lv_obj_t *obj43;
    lv_obj_t *obj44;
    lv_obj_t *temp_container_3;
    lv_obj_t *status_container_17;
    lv_obj_t *bat_3;
    lv_obj_t *status_container_18;
    lv_obj_t *obj45;
    lv_obj_t *low_power_page;
    lv_obj_t *chinese_bt;
    lv_obj_t *engilsh_bt;
    lv_obj_t *language_switch_prompt;
    lv_obj_t *calibration_prompt;
    lv_obj_t *calibration_bt;
    lv_obj_t *product_name_panel;
    lv_obj_t *model_panel;
    lv_obj_t *hardware_version_panel;
    lv_obj_t *software_version_panel;
    lv_obj_t *wifi_1;
    lv_obj_t *wifi_2;
    lv_obj_t *wifi_3;
    lv_obj_t *measure_variant_image;
    lv_obj_t *measure_variant_label;
    lv_obj_t *measure_variant_point_1;
    lv_obj_t *measure_variant_point_2;
    lv_obj_t *measure_variant_point_3;
    lv_obj_t *measure_variant_point_4;
    lv_obj_t *measure_moisture_value;
    lv_obj_t *measure_density_value;
    lv_obj_t *measure_aw_value;
    lv_obj_t *measure_time_value;
} objects_t;

extern objects_t objects;

void create_screen_measure();
void tick_screen_measure();

void create_screen_measure_roasted();
void tick_screen_measure_roasted();

void create_screen_measure_green();
void tick_screen_measure_green();

void create_screen_measure_parchment();
void tick_screen_measure_parchment();

void create_screen_measure_dry_cherry();
void tick_screen_measure_dry_cherry();

void create_screen_setting();
void tick_screen_setting();

void create_screen_setting_language();
void tick_screen_setting_language();

void create_screen_setting_language_cn();
void tick_screen_setting_language_cn();

void create_screen_zerong();
void tick_screen_zerong();

void create_screen_zerong_m_d();
void tick_screen_zerong_m_d();

void create_screen_zerong_aw();
void tick_screen_zerong_aw();

void create_screen_zerong_m_d_ing();
void tick_screen_zerong_m_d_ing();

void create_screen_zerong_aw_ing();
void tick_screen_zerong_aw_ing();

void create_screen_zerong_m_d_ok();
void tick_screen_zerong_m_d_ok();

void create_screen_zerong_aw_ok();
void tick_screen_zerong_aw_ok();

void create_screen_zerong_aw_warm();
void tick_screen_zerong_aw_warm();

void create_screen_zerong_m_d_success();
void tick_screen_zerong_m_d_success();

void create_screen_zerong_aw_success();
void tick_screen_zerong_aw_success();

void create_screen_about();
void tick_screen_about();

void create_screen_about_message();
void tick_screen_about_message();

void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_status_bar(int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#define SCREEN_ID_MEASURE_TEMP SCREEN_ID_MEASURE
#define SCREEN_ID_LANGUAGE_PAGE SCREEN_ID_SETTING_LANGUAGE
#define SCREEN_ID_CALIBRATION_PAGE SCREEN_ID_ZERONG
#define SCREEN_ID_ABOUT_PAGE SCREEN_ID_ABOUT_MESSAGE
#define SCREEN_ID_LOWPOWER_PAGE SCREEN_ID_ABOUT_MESSAGE
#define UI_FIRST_CYCLING_SCREEN SCREEN_ID_MEASURE_TEMP
#define UI_LAST_CYCLING_SCREEN SCREEN_ID_ABOUT_PAGE

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/
