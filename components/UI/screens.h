#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define USE_TDS
#define USE_TEMP

// typedef struct _objects_t {
//     lv_obj_t *measure_tds;
//     lv_obj_t *measure_temp;
//     lv_obj_t *language_page;
//     lv_obj_t *calibration_page;
//     lv_obj_t *about_page;
//     lv_obj_t *bat;
//     lv_obj_t *bat_1;
//     lv_obj_t *bat_2;
//     lv_obj_t *bat_3;
//     lv_obj_t *bat_4;
//     lv_obj_t *calibration_bt;
//     lv_obj_t *calibration_prompt;
//     lv_obj_t *chinese_bt;
//     lv_obj_t *engilsh_bt;
//     lv_obj_t *hardware_version_panel;
//     lv_obj_t *language_switch_prompt;
//     lv_obj_t *model_panel;
//     lv_obj_t *product_name_panel;
//     lv_obj_t *qr_panel;
//     lv_obj_t *sn;
//     lv_obj_t *software_version_panel;
//     lv_obj_t *status_container;
//     lv_obj_t *status_container_1;
//     lv_obj_t *status_container_2;
//     lv_obj_t *status_container_3;
//     lv_obj_t *status_container_4;
//     lv_obj_t *tds_container;
//     lv_obj_t *tds_temp_container;
//     lv_obj_t *temp_container;
//     lv_obj_t *wifi;
//     lv_obj_t *wifi_1;
//     lv_obj_t *wifi_2;
//     lv_obj_t *wifi_3;
//     lv_obj_t *wifi_4;
// } objects_t;

// typedef struct _objects_t {
//     lv_obj_t *measure_tds;
//     lv_obj_t *status_container_4;
//     lv_obj_t *bat_4;
//     lv_obj_t *wifi_4;
//     lv_obj_t *tds_container;
//     lv_obj_t *tds_temp_container;
//     lv_obj_t *measure_temp;
//     lv_obj_t *status_container;
//     lv_obj_t *bat;
//     lv_obj_t *wifi;
//     lv_obj_t *temp_container;
//     lv_obj_t *language_page;
//     lv_obj_t *calibration_page;
//     lv_obj_t *about_page;
//     lv_obj_t *bat_1;
//     lv_obj_t *bat_2;
//     lv_obj_t *bat_3;
//     lv_obj_t *calibration_bt;
//     lv_obj_t *calibration_prompt;
//     lv_obj_t *chinese_bt;
//     lv_obj_t *engilsh_bt;
//     lv_obj_t *hardware_version_panel;
//     lv_obj_t *language_switch_prompt;
//     lv_obj_t *model_panel;
//     lv_obj_t *product_name_panel;
//     lv_obj_t *qr_panel;
//     lv_obj_t *sn;
//     lv_obj_t *software_version_panel;
//     lv_obj_t *status_container_1;
//     lv_obj_t *status_container_2;
//     lv_obj_t *status_container_3;
//     lv_obj_t *wifi_1;
//     lv_obj_t *wifi_2;
//     lv_obj_t *wifi_3;
// } objects_t;

typedef struct _objects_t {
    #ifdef USE_TDS
    lv_obj_t *measure_tds;
    #endif

    #ifdef USE_TEMP
    lv_obj_t *measure_temp;
    #endif

    lv_obj_t *language_page;
    // lv_obj_t *calibration_page;
    lv_obj_t *about_page;
    #ifdef USE_TEMP
    lv_obj_t *bat;
    #endif

    lv_obj_t *bat_1;
    // lv_obj_t *bat_2;
    lv_obj_t *bat_3;
    #ifdef USE_TDS
    lv_obj_t *bat_4;
    #endif
    // lv_obj_t *calibration_bt;
    // lv_obj_t *calibration_prompt;
    lv_obj_t *chinese_bt;
    lv_obj_t *engilsh_bt;
    lv_obj_t *hardware_version_panel;
    lv_obj_t *language_switch_prompt;
    lv_obj_t *model_panel;
    lv_obj_t *product_name_panel;
    lv_obj_t *qr_panel;
    lv_obj_t *sn;
    lv_obj_t *software_version_panel;
    #ifdef USE_TEMP
    lv_obj_t *status_container;
    #endif
    lv_obj_t *status_container_1;
    lv_obj_t *status_container_2;
    lv_obj_t *status_container_3;
    #ifdef USE_TDS
    lv_obj_t *status_container_4;
    lv_obj_t *tds_container;
    lv_obj_t *tds_temp_container;
    #endif
    #ifdef USE_TEMP
    lv_obj_t *temp_container;
    lv_obj_t *wifi;
    #endif
    lv_obj_t *wifi_1;
    // lv_obj_t *wifi_2;
    lv_obj_t *wifi_3;
    #ifdef USE_TDS
    lv_obj_t *wifi_4;
    #endif
} objects_t;

extern objects_t objects;

// enum ScreensEnum {
//     SCREEN_ID_MEASURE_TDS = 1,
//     SCREEN_ID_MEASURE_TEMP = 2,
//     SCREEN_ID_LANGUAGE_PAGE = 3,
//     SCREEN_ID_CALIBRATION_PAGE = 4,
//     SCREEN_ID_ABOUT_PAGE = 5,
//     SCREEN_ID_STATUS_BAR = 6,
// };

#ifdef USE_TDS
    enum ScreensEnum {
        SCREEN_ID_MEASURE_TDS = 1,
        // SCREEN_ID_MEASURE_TEMP = 2,
        SCREEN_ID_LANGUAGE_PAGE = 2,
        SCREEN_ID_CALIBRATION_PAGE = 3,
        SCREEN_ID_ABOUT_PAGE = 4,
        SCREEN_ID_STATUS_BAR = 5,
    };
#endif
#ifdef USE_TEMP
    enum ScreensEnum {
        // SCREEN_ID_MEASURE_TDS = 1,
        // SCREEN_ID_MEASURE_TEMP = 1,
        // SCREEN_ID_LANGUAGE_PAGE = 2,
        // SCREEN_ID_CALIBRATION_PAGE = 3,
        // SCREEN_ID_ABOUT_PAGE = 4,
        // SCREEN_ID_STATUS_BAR = 5,
        SCREEN_ID_MEASURE_TEMP = 1,
        SCREEN_ID_LANGUAGE_PAGE = 2,
        SCREEN_ID_ABOUT_PAGE = 3,
        SCREEN_ID_STATUS_BAR = 4,
    };
#endif

void create_screen_measure_temp();
void tick_screen_measure_temp();

void create_screen_language_page();
void tick_screen_language_page();

// void create_screen_calibration_page();
void tick_screen_calibration_page();

void create_screen_about_page();
void tick_screen_about_page();

void create_screen_measure_tds();
void tick_screen_measure_tds();

void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_status_bar(int startWidgetIndex);

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/