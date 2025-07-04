#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
#ifdef USE_TEMP
void create_screen_measure_temp() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.measure_temp = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Temp_Container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.temp_container = obj;
            lv_obj_set_pos(obj, 52, 46);
            lv_obj_set_size(obj, 216, 91);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 192, 59);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "℃");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 144, 43);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "00");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 20, 24);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "000.");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_60, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 132, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "000.00℉");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 1);
                    lv_obj_set_size(obj, 10, 78);
                    lv_bar_set_value(obj, 100, LV_ANIM_OFF);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffff945a), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 20, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, UI_STRING[0][SYS_DATA.language_id]);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_letter_space(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Status_Container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // WIFI
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.wifi = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_wifi);
                }
                {
                    // BAT
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat);
                }
            }
        }
    }
    lv_obj_add_event_cb(objects.measure_temp,ui_event_page_load_tds_screen, LV_EVENT_ALL, NULL);
}

void tick_screen_measure_temp() {
}
#endif

void create_screen_language_page() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.language_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // chinese_bt
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.chinese_bt = obj;
            lv_obj_set_pos(obj, 18, 63);
            lv_obj_set_size(obj, 134, 46);
            lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "中文");
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // engilsh_bt
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.engilsh_bt = obj;
            lv_obj_set_pos(obj, 168, 64);
            lv_obj_set_size(obj, 134, 46);
            lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Engilsh");
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        if(SYS_DATA.language_id)
        {
            lv_obj_set_style_bg_color(objects.chinese_bt, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(objects.engilsh_bt, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        else{
            lv_obj_set_style_bg_color(objects.chinese_bt, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(objects.engilsh_bt, lv_color_hex(0xff505050), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Status_Container_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_1 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // WIFI_1
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.wifi_1 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_wifi);
                }
                {
                    // BAT_1
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat_1 = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat);
                }
            }
        }
        {
            // language_switch_prompt
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.language_switch_prompt = obj;
            lv_obj_set_pos(obj, 0, 52);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, UI_STRING[1][SYS_DATA.language_id]);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    lv_obj_add_event_cb(objects.language_page, ui_event_page_load_tds_screen, LV_EVENT_ALL, NULL);
}

void tick_screen_language_page() {
}

void create_screen_calibration_page() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.calibration_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    // lv_obj_set_style_text_font(obj, &ui_font_regular_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // calibration_bt
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.calibration_bt = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 220, 36);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa851ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, UI_STRING[3][SYS_DATA.language_id]);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Status_Container_2
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_2 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // WIFI_2
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.wifi_2 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_wifi);
                }
                {
                    // BAT_2
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat_2 = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat);
                }
            }
        }
        {
            // Calibration_prompt
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.calibration_prompt = obj;
            lv_obj_set_pos(obj, 0, 52);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, UI_STRING[2][SYS_DATA.language_id]);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    lv_obj_add_event_cb(objects.calibration_page, ui_event_page_load_tds_screen, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(objects.calibration_bt, ui_calibration_event, LV_EVENT_CLICKED, NULL);
}

void tick_screen_calibration_page() {
}

void create_screen_about_page() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.about_page = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Product_name_Panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.product_name_panel = obj;
            lv_obj_set_pos(obj, 107, 43);
            lv_obj_set_size(obj, 195, 17);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 65, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Temperature Tester");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -9, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, UI_STRING[6][SYS_DATA.language_id]);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Model_Panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.model_panel = obj;
            lv_obj_set_pos(obj, 107, 65);
            lv_obj_set_size(obj, 195, 17);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 65, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Temperature Tester");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -9, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, UI_STRING[7][SYS_DATA.language_id]);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Hardware_Version_Panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.hardware_version_panel = obj;
            lv_obj_set_pos(obj, 107, 87);
            lv_obj_set_size(obj, 195, 17);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 152, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "0.0.1");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -9, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, UI_STRING[8][SYS_DATA.language_id]);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Software_Version_Panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.software_version_panel = obj;
            lv_obj_set_pos(obj, 107, 109);
            lv_obj_set_size(obj, 195, 17);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 152, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "0.0.1");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, -9, -12);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, UI_STRING[9][SYS_DATA.language_id]);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // QR_Panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.qr_panel = obj;
            lv_obj_set_pos(obj, 18, 43);
            lv_obj_set_size(obj, 83, 83);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_color_t bg_color = lv_palette_lighten(LV_PALETTE_GREY, 5);
            // 背景色：灰色调，调亮（5级），用于二维码的底色
            lv_color_t fg_color = lv_palette_darken(LV_PALETTE_GREY, 4);
            // 前景色：灰色调，调暗（4级），用于二维码的点（数据色）
            lv_obj_t* qr = lv_qrcode_create(obj, 83, bg_color, fg_color);
            // 创建二维码对象
            // 父对象是 ui_QRPanel，二维码尺寸 242x242，指定背景和前景色
            lv_qrcode_update(qr, "S/N: LB25020800000001", strlen("S/N: LB25020800000001"));
            // 用指定字符串生成二维码数据（这里是一个 S/N 编号）
            // lv_obj_set_size(qr, 242, 242);
            // 可以不设置尺寸，前面创建时已经指定了
            lv_obj_set_align(qr, LV_ALIGN_CENTER);
            // 设置二维码在父容器居中显示
            /*Add a border with fg_color*/
            lv_obj_set_style_border_color(qr, fg_color, 0);
            // 设置二维码边框颜色为前景色（就是暗灰色）
            lv_obj_set_style_border_width(qr, 5, 0);
            // 设置边框宽度为5像素
        }
        {
            // SN
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.sn = obj;
            lv_obj_set_pos(obj, 112, 134);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "S/N : LB20250000000001");
            lv_obj_set_style_text_font(obj, &ui_font_bold_10, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Status_Container_3
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_3 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // WIFI_3
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.wifi_3 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_wifi);
                }
                {
                    // BAT_3
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat_3 = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat);
                }
            }
        }
    }
    lv_obj_add_event_cb(objects.about_page, ui_event_page_load_tds_screen, LV_EVENT_ALL, NULL);
}

void tick_screen_about_page() {
}
#ifdef USE_TDS
void create_screen_measure_tds() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.measure_tds = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // TDS_container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.tds_container = obj;
            lv_obj_set_pos(obj, 26, 46);
            lv_obj_set_size(obj, 170, 91);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 1);
                    lv_obj_set_size(obj, 10, 78);
                    lv_bar_set_value(obj, 100, LV_ANIM_OFF);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa851ff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 19, 4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "TDS");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 19, 24);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "00.");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_60, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 105, 43);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "00");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_40, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 153, 56);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "%");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // TDS_Temp_Container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.tds_temp_container = obj;
            lv_obj_set_pos(obj, 214, 73);
            lv_obj_set_size(obj, 88, 55);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;//0
                {
                    lv_obj_t *obj = lv_bar_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 2);
                    lv_obj_set_size(obj, 10, 49);
                    lv_bar_set_value(obj, 100, LV_ANIM_OFF);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffff945a), LV_PART_INDICATOR | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);//1
                    lv_obj_set_pos(obj, 16, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "000.");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);//2
                    lv_obj_set_pos(obj, 57, 4);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "00℃");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);//3
                    lv_obj_set_pos(obj, 16, 32);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "000.");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);//4
                    lv_obj_set_pos(obj, 57, 36);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "00℉");
                    lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Status_Container_4
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_4 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // WIFI_4
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.wifi_4 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_wifi);
                }
                {
                    // BAT_4
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat_4 = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat);
                }
            }
        }
    }
    lv_obj_add_event_cb(objects.measure_tds, ui_event_page_load_tds_screen, LV_EVENT_ALL, NULL);
}

void tick_screen_measure_tds() {
}
#endif
void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex) {
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_wifi);
                }
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat);
                }
            }
        }
    }
}

void tick_user_widget_status_bar(int startWidgetIndex) {
}


void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    #ifdef USE_TEMP
    create_screen_measure_temp();
    #endif
    create_screen_language_page();
    create_screen_calibration_page();
    create_screen_about_page();
    #ifdef USE_TDS
    create_screen_measure_tds();
    #endif
}

typedef void (*tick_screen_func_t)();

tick_screen_func_t tick_screen_funcs[] = {
    #ifdef USE_TEMP
    tick_screen_measure_temp,
    #endif
    tick_screen_language_page,
    tick_screen_calibration_page,
    tick_screen_about_page,
    #ifdef USE_TDS
    tick_screen_measure_tds,
    #endif
    0,
};

void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
