#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"
#include "user_hal.h"

#include <string.h>

objects_t objects;

static lv_obj_t *ui_compat_create_panel(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                                        lv_coord_t w, lv_coord_t h, const char *text) {
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_style_bg_opa(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = lv_label_create(panel);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(label, text);
    return panel;
}

static lv_obj_t *ui_compat_create_icon(lv_obj_t *parent, lv_coord_t x, lv_coord_t y) {
    lv_obj_t *icon = lv_img_create(parent);
    lv_obj_set_pos(icon, x, y);
    lv_img_set_src(icon, &img_wifi_2);
    return icon;
}

static void ui_create_compat_objects(void) {
    objects.chinese_bt = ui_compat_create_panel(objects.setting_language, 74, 40, 60, 60, "");
    lv_obj_set_style_bg_color(objects.chinese_bt, lv_color_hex(0xffa851ff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    objects.engilsh_bt = ui_compat_create_panel(objects.setting_language, 186, 40, 60, 60, "");
    lv_obj_set_style_bg_color(objects.engilsh_bt, lv_color_hex(0xff505050),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    objects.language_switch_prompt = lv_label_create(objects.setting_language);
    lv_obj_set_pos(objects.language_switch_prompt, 28, 112);
    lv_obj_set_width(objects.language_switch_prompt, 264);
    lv_label_set_long_mode(objects.language_switch_prompt, LV_LABEL_LONG_WRAP);
    lv_label_set_text(objects.language_switch_prompt, "");

    objects.calibration_bt = ui_compat_create_panel(objects.zerong, 100, 118, 120, 28, UI_STRING[3][0]);
    lv_obj_set_style_bg_color(objects.calibration_bt, lv_color_hex(0xff28f641),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    objects.calibration_prompt = lv_label_create(objects.zerong);
    lv_obj_set_pos(objects.calibration_prompt, 44, 86);
    lv_obj_set_width(objects.calibration_prompt, 232);
    lv_label_set_long_mode(objects.calibration_prompt, LV_LABEL_LONG_WRAP);
    lv_label_set_text(objects.calibration_prompt, UI_STRING[2][0]);

    objects.product_name_panel = ui_compat_create_panel(objects.about_message, 110, 54, 190, 16, "");
    objects.model_panel = ui_compat_create_panel(objects.about_message, 110, 76, 190, 16, "");
    objects.hardware_version_panel = ui_compat_create_panel(objects.about_message, 110, 98, 190, 16, "");
    objects.software_version_panel = ui_compat_create_panel(objects.about_message, 110, 120, 190, 16, "");

    if (objects.obj45) {
        lv_obj_clear_flag(objects.obj45, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(objects.obj45, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_border_width(objects.obj45, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(objects.obj45, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_color_t qr_bg_color = lv_palette_lighten(LV_PALETTE_GREY, 5);
        lv_color_t qr_fg_color = lv_palette_darken(LV_PALETTE_GREY, 4);
        lv_obj_t *qr = lv_qrcode_create(objects.obj45, 83, qr_bg_color, qr_fg_color);

        if (qr) {
            lv_qrcode_update(qr, SYS_DATA.sn, strlen(SYS_DATA.sn));
            lv_obj_center(qr);
            lv_obj_set_style_border_color(qr, qr_fg_color, 0);
            lv_obj_set_style_border_width(qr, 5, 0);
        }
    }

    objects.wifi_1 = ui_compat_create_icon(objects.setting_language, 246, 8);
    objects.wifi_2 = NULL;
    objects.wifi_3 = ui_compat_create_icon(objects.about_message, 246, 8);

    objects.low_power_page = lv_obj_create(lv_layer_top());
    lv_obj_set_pos(objects.low_power_page, 24, 56);
    lv_obj_set_size(objects.low_power_page, 272, 60);
    lv_obj_set_style_bg_color(objects.low_power_page, lv_color_hex(0xff101010),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(objects.low_power_page, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(objects.low_power_page, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(objects.low_power_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(objects.low_power_page, LV_OBJ_FLAG_HIDDEN);
    {
        lv_obj_t *icon = lv_label_create(objects.low_power_page);
        lv_obj_set_pos(icon, 12, 18);
        lv_label_set_text(icon, "LOW");
    }
    {
        lv_obj_t *label = lv_label_create(objects.low_power_page);
        lv_obj_set_pos(label, 78, 18);
        lv_obj_set_width(label, 180);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_label_set_text(label, UI_STRING[10][0]);
    }
}

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_measure() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.measure = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
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
        }
        {
            // Status_Container
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
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
                    // BAT
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat_2);
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 115, 41);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_douzi_1);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Moisture");
            lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 130);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(objects.status_container);
            lv_obj_set_pos(obj, 26, 5);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
    }
    
    tick_screen_measure();
}

void tick_screen_measure() {
}

void create_screen_measure_roasted() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.measure_roasted = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_1 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Moisture");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 37, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Moisture");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 142, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Desity");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.measure_moisture_value = obj;
            lv_obj_set_pos(obj, 35, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 52, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "%");
                }
                {
                    // line_1
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_1 = obj;
                    lv_obj_set_pos(obj, -8, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.measure_density_value = obj;
            lv_obj_set_pos(obj, 129, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000g/");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 66, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "L");
                }
                {
                    // line_2
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_2 = obj;
                    lv_obj_set_pos(obj, 0, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Roasted
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.roasted = obj;
            objects.measure_variant_image = obj;
            lv_obj_set_pos(obj, 235, 44);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_roasted);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.measure_variant_point_1 = obj;
            lv_obj_set_pos(obj, 240, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_point_white);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.measure_variant_point_2 = obj;
            lv_obj_set_pos(obj, 250, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.measure_variant_point_3 = obj;
            lv_obj_set_pos(obj, 260, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.measure_variant_point_4 = obj;
            lv_obj_set_pos(obj, 270, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            // Line_white
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.line_white = obj;
            lv_obj_set_pos(obj, 205, 118);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 101, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_rounded(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.measure_variant_label = obj;
            lv_obj_set_pos(obj, 229, 96);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Roasted");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 56, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 146, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Time");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.measure_aw_value = obj;
            lv_obj_set_pos(obj, 35, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "0.000");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_3
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_3 = obj;
                    lv_obj_set_pos(obj, -5, 28);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.measure_time_value = obj;
            lv_obj_set_pos(obj, 129, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_4
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_4 = obj;
                    lv_obj_set_pos(obj, -5, 29);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 217, 134);
            lv_obj_set_size(obj, 80, 19);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj5 = obj;
                    lv_obj_set_pos(obj, 12, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Measure");
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 106, 103);
            lv_obj_set_size(obj, 18, 6);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_measure_roasted();
}

void tick_screen_measure_roasted() {
}

void create_screen_measure_green() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.measure_green = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_2
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_2 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Moisture");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 37, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Measure");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 142, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Desity");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 35, 58);
            lv_obj_set_size(obj, 65, 39);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 52, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "%");
                }
                {
                    // line_5
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_5 = obj;
                    lv_obj_set_pos(obj, -8, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 129, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000g/");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 66, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "L");
                }
                {
                    // line_6
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_6 = obj;
                    lv_obj_set_pos(obj, 0, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Green
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.green = obj;
            lv_obj_set_pos(obj, 235, 44);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_green);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 250, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_point_white);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 240, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 260, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 270, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            // Line_white_1
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.line_white_1 = obj;
            lv_obj_set_pos(obj, 205, 118);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 101, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_rounded(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 236, 96);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Green");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 56, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 146, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Time");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 35, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "0.000");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_7
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_7 = obj;
                    lv_obj_set_pos(obj, -5, 28);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 129, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_8
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_8 = obj;
                    lv_obj_set_pos(obj, -5, 29);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 217, 134);
            lv_obj_set_size(obj, 80, 19);
            lv_bar_set_value(obj, 0, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj12 = obj;
                    lv_obj_set_pos(obj, 12, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Measure");
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 106, 103);
            lv_obj_set_size(obj, 18, 6);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_measure_green();
}

void tick_screen_measure_green() {
}

void create_screen_measure_parchment() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.measure_parchment = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_3
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_3 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Measure");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, 37, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Measure");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj15 = obj;
            lv_obj_set_pos(obj, 142, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Desity");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 35, 58);
            lv_obj_set_size(obj, 65, 39);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 52, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "%");
                }
                {
                    // line_9
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_9 = obj;
                    lv_obj_set_pos(obj, -8, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 129, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000g/");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 66, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "L");
                }
                {
                    // line_10
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_10 = obj;
                    lv_obj_set_pos(obj, 0, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Parchment
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.parchment = obj;
            lv_obj_set_pos(obj, 235, 44);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_parchment);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 260, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_point_white);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 240, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 250, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 270, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            // Line_white_2
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.line_white_2 = obj;
            lv_obj_set_pos(obj, 205, 118);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 101, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_rounded(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 221, 96);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Parchment");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj16 = obj;
            lv_obj_set_pos(obj, 56, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj17 = obj;
            lv_obj_set_pos(obj, 146, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Time");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 35, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "0.000");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_11
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_11 = obj;
                    lv_obj_set_pos(obj, -5, 28);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 129, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_12
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_12 = obj;
                    lv_obj_set_pos(obj, -5, 29);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj18 = obj;
            lv_obj_set_pos(obj, 217, 134);
            lv_obj_set_size(obj, 80, 19);
            lv_bar_set_value(obj, 0, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj19 = obj;
                    lv_obj_set_pos(obj, 12, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Measure");
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj20 = obj;
            lv_obj_set_pos(obj, 106, 103);
            lv_obj_set_size(obj, 18, 6);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_measure_parchment();
}

void tick_screen_measure_parchment() {
}

void create_screen_measure_dry_cherry() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.measure_dry_cherry = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_4
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_4 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Measure");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj21 = obj;
            lv_obj_set_pos(obj, 37, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Measure");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj22 = obj;
            lv_obj_set_pos(obj, 142, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Desity");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 35, 58);
            lv_obj_set_size(obj, 65, 39);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00.0");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 52, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "%");
                }
                {
                    // line_13
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_13 = obj;
                    lv_obj_set_pos(obj, -8, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 129, 58);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000g/");
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 66, 10);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "L");
                }
                {
                    // line_14
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_14 = obj;
                    lv_obj_set_pos(obj, 0, 38);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // DryCherry
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.dry_cherry = obj;
            lv_obj_set_pos(obj, 235, 44);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_drycherry);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 270, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_point_white);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 240, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 250, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 260, 89);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_piont_black);
        }
        {
            // Line_white_3
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.line_white_3 = obj;
            lv_obj_set_pos(obj, 205, 118);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 101, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_rounded(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 221, 96);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "DryCherry");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj23 = obj;
            lv_obj_set_pos(obj, 56, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj24 = obj;
            lv_obj_set_pos(obj, 146, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffa07aff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Time");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 35, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "0.000");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_15
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_15 = obj;
                    lv_obj_set_pos(obj, -5, 28);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 129, 129);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "00:00");
            {
                lv_obj_t *parent_obj = obj;
                {
                    // line_16
                    lv_obj_t *obj = lv_line_create(parent_obj);
                    objects.line_16 = obj;
                    lv_obj_set_pos(obj, -5, 29);
                    lv_obj_set_size(obj, 73, 1);
                    static lv_point_t line_points[] = {
                        { 0, 0 },
                        { 73, 0 }
                    };
                    lv_line_set_points(obj, line_points, 2);
                    lv_obj_set_style_line_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj25 = obj;
            lv_obj_set_pos(obj, 217, 134);
            lv_obj_set_size(obj, 80, 19);
            lv_bar_set_value(obj, 0, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj26 = obj;
                    lv_obj_set_pos(obj, 12, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Measure");
                }
            }
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj27 = obj;
            lv_obj_set_pos(obj, 106, 103);
            lv_obj_set_size(obj, 18, 6);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff464646), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_measure_dry_cherry();
}

void tick_screen_measure_dry_cherry() {
}

void create_screen_setting() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.setting = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Temp_Container_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.temp_container_1 = obj;
            lv_obj_set_pos(obj, 52, 46);
            lv_obj_set_size(obj, 216, 91);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Status_Container_5
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_5 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
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
                    // BAT_1
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat_1 = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat_2);
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 113, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_setting);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Setting");
            lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 130);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(objects.status_container_5);
            lv_obj_set_pos(obj, 26, 5);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
    }
    
    tick_screen_setting();
}

void tick_screen_setting() {
}

void create_screen_setting_language() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.setting_language = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_6
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_6 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Language");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj28 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_bar_set_value(obj, 0, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj29 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.setting_language_chinese_img = obj;
            lv_obj_set_pos(obj, 83, 44);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_chinese);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.setting_language_english_img = obj;
            lv_obj_set_pos(obj, 179, 44);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_english);
        }
    }
    
    tick_screen_setting_language();
}

void tick_screen_setting_language() {
}

void create_screen_setting_language_cn() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.setting_language_cn = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *status_obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(status_obj, 252, 17);
            lv_obj_set_size(status_obj, 50, 19);
            lv_obj_set_style_pad_left(status_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(status_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(status_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(status_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(status_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(status_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(status_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *label_obj = lv_label_create(parent_obj);
            lv_obj_set_pos(label_obj, 36, 8);
            lv_obj_set_size(label_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(label_obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(label_obj, "Language");
        }
        {
            lv_obj_t *img_obj = lv_img_create(parent_obj);
            lv_obj_set_pos(img_obj, 301, 8);
            lv_obj_set_size(img_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(img_obj, &img_light);
            lv_img_set_zoom(img_obj, 380);
        }
        {
            lv_obj_t *img_obj = lv_img_create(parent_obj);
            lv_obj_set_pos(img_obj, 246, 8);
            lv_obj_set_size(img_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(img_obj, &img_wifi_2);
        }
        {
            lv_obj_t *img_obj = lv_img_create(parent_obj);
            lv_obj_set_pos(img_obj, 12, 8);
            lv_obj_set_size(img_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(img_obj, &img_back);
        }
        {
            lv_obj_t *img_obj = lv_img_create(parent_obj);
            lv_obj_set_pos(img_obj, 273, 8);
            lv_obj_set_size(img_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(img_obj, &img_bat_2);
        }
        {
            lv_obj_t *img_obj = lv_img_create(parent_obj);
            lv_obj_set_pos(img_obj, 276, 13);
            lv_obj_set_size(img_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(img_obj, &img_100);
        }
        {
            lv_obj_t *bar_obj = lv_bar_create(parent_obj);
            lv_obj_set_pos(bar_obj, 105, 140);
            lv_obj_set_size(bar_obj, 110, 21);
            lv_bar_set_value(bar_obj, 0, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(bar_obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(bar_obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(bar_obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *ok_label = lv_label_create(bar_obj);
                lv_obj_set_pos(ok_label, 40, 1);
                lv_obj_set_size(ok_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_obj_set_style_bg_color(ok_label, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(ok_label, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(ok_label, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(ok_label, "OK");
            }
        }
        {
            lv_obj_t *img_obj = lv_img_create(parent_obj);
            lv_obj_set_pos(img_obj, 83, 44);
            lv_obj_set_size(img_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(img_obj, &img_cn);
        }
        {
            lv_obj_t *img_obj = lv_img_create(parent_obj);
            lv_obj_set_pos(img_obj, 179, 44);
            lv_obj_set_size(img_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(img_obj, &img_en);
        }
    }

    tick_screen_setting_language_cn();
}

void tick_screen_setting_language_cn() {
}

void create_screen_zerong() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Temp_Container_2
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.temp_container_2 = obj;
            lv_obj_set_pos(obj, 52, 46);
            lv_obj_set_size(obj, 216, 91);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Status_Container_7
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_7 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
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
                    // BAT_2
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat_2 = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat_2);
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 113, 39);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_zerong);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Zerong");
            lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 130);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(objects.status_container_7);
            lv_obj_set_pos(obj, 26, 5);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
    }
    
    tick_screen_zerong();
}

void tick_screen_zerong() {
}

void create_screen_zerong_m_d() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_m_d = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_8
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_8 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "M&D Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj30 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj31 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 134, 38);
            lv_obj_set_size(obj, 52, 53);
            lv_img_set_src(obj, &img_zerong_1);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 15, 23);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_m_d);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 91, 91);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Moisture&Density\nZeroing");
        }
    }
    
    tick_screen_zerong_m_d();
}

void tick_screen_zerong_m_d() {
}

void create_screen_zerong_aw() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_aw = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_9
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_9 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj32 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj33 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 134, 38);
            lv_obj_set_size(obj, 52, 53);
            lv_img_set_src(obj, &img_zerong_1);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 19, 19);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_a);
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj34 = obj;
            lv_obj_set_pos(obj, 131, 91);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_recolor(obj, true);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfffafafa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW\nZeroing");
        }
    }
    
    tick_screen_zerong_aw();
}

void tick_screen_zerong_aw() {
}

void create_screen_zerong_m_d_ing() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_m_d_ing = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_10
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_10 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "M&D Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 104, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "M&D Zeroing...");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 130, 55);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_zerong_2);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 17, 26);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_m_d_1);
                }
            }
        }
    }
    
    tick_screen_zerong_m_d_ing();
}

void tick_screen_zerong_m_d_ing() {
}

void create_screen_zerong_aw_ing() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_aw_ing = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_11
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_11 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 118, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "A Zeroing...");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 130, 55);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_zerong_2);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 22, 22);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_a_1);
                }
            }
        }
    }
    
    tick_screen_zerong_aw_ing();
}

void tick_screen_zerong_aw_ing() {
}

void create_screen_zerong_m_d_ok() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_m_d_ok = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_12
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_12 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "M&D Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj35 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj36 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 9, 41);
            lv_obj_set_size(obj, 303, 123);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "You will be starting the calibration process, please make sure that you have fully understood the calibration procedure before starting the calibration process.");
        }
    }
    
    tick_screen_zerong_m_d_ok();
}

void tick_screen_zerong_m_d_ok() {
}

void create_screen_zerong_aw_ok() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_aw_ok = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_13
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_13 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj37 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj38 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 9, 41);
            lv_obj_set_size(obj, 303, 123);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "You will be starting the calibration process, please make sure that you have fully understood the calibration procedure before starting the calibration process.");
        }
    }
    
    tick_screen_zerong_aw_ok();
}

void tick_screen_zerong_aw_ok() {
}

void create_screen_zerong_aw_warm() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_aw_warm = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_14
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_14 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj39 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj40 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 138, 35);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_warming);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 46, 89);
            lv_obj_set_size(obj, 229, 60);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Are you sure you want to zeroing now?");
        }
    }
    
    tick_screen_zerong_aw_warm();
}

void tick_screen_zerong_aw_warm() {
}

void create_screen_zerong_m_d_success() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_m_d_success = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_15
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_15 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "M&D Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj41 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj42 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 123, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Success");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 130, 40);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_zerong_2);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 17, 26);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_m_d_2);
                }
            }
        }
    }
    
    tick_screen_zerong_m_d_success();
}

void tick_screen_zerong_m_d_success() {
}

void create_screen_zerong_aw_success() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.zerong_aw_success = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_16
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_16 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "AW Zerong");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.obj43 = obj;
            lv_obj_set_pos(obj, 105, 140);
            lv_obj_set_size(obj, 110, 21);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff00ff0d), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj44 = obj;
                    lv_obj_set_pos(obj, 40, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "OK");
                }
            }
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 123, 105);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Success");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 130, 40);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_zerong_2);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 22, 22);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_a_2);
                }
            }
        }
    }
    
    tick_screen_zerong_aw_success();
}

void tick_screen_zerong_aw_success() {
}

void create_screen_about() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.about = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Temp_Container_3
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.temp_container_3 = obj;
            lv_obj_set_pos(obj, 52, 46);
            lv_obj_set_size(obj, 216, 91);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Status_Container_17
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_17 = obj;
            lv_obj_set_pos(obj, 250, 8);
            lv_obj_set_size(obj, 54, 20);
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
                    // BAT_3
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    objects.bat_3 = obj;
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat_2);
                }
            }
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 113, 41);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_about);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 131, 130);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "About");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(objects.status_container_17);
            lv_obj_set_pos(obj, 26, 5);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
    }
    
    tick_screen_about();
}

void tick_screen_about() {
}

void create_screen_about_message() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.about_message = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 172);
    lv_obj_add_event_cb(obj, action_slide_to_the_left, LV_EVENT_GESTURE, (void *)0);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Status_Container_18
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.status_container_18 = obj;
            lv_obj_set_pos(obj, 252, 17);
            lv_obj_set_size(obj, 50, 19);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 36, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_bold_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "About");
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 304, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_light);
            lv_img_set_zoom(obj, 380);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 246, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_wifi_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 12, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_back);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 273, 8);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_bat_2);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 276, 13);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img_100);
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            lv_obj_set_pos(obj, 0, 37);
            lv_obj_set_size(obj, 320, 135);
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
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.obj45 = obj;
                    lv_obj_set_pos(obj, 16, 17);
                    lv_obj_set_size(obj, 83, 83);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 109, 17);
                    lv_obj_set_size(obj, 195, 17);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -9, -14);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Name");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 115, -15);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, " RoastSee");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 109, 39);
                    lv_obj_set_size(obj, 195, 17);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -9, -15);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Model");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 115, -15);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, " RoastSee");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 109, 61);
                    lv_obj_set_size(obj, 195, 17);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -9, -13);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Hardware Version");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 147, -15);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "0.0.1");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 109, 83);
                    lv_obj_set_size(obj, 195, 17);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -9, -15);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Software Version");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 147, -15);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &ui_font_bold_12, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "0.0.1");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 98, 110);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_bold_10, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "S/N : LB20250000000001");
                }
            }
        }
    }
    
    tick_screen_about_message();
}

void tick_screen_about_message() {
}

void create_user_widget_status_bar(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
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
                    lv_img_set_src(obj, &img_wifi_2);
                }
                {
                    lv_obj_t *obj = lv_img_create(parent_obj);
                    lv_obj_set_pos(obj, 23, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_img_set_src(obj, &img_bat_2);
                }
            }
        }
    }
}

void tick_user_widget_status_bar(int startWidgetIndex) {
    (void)startWidgetIndex;
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_measure,
    tick_screen_measure_roasted,
    tick_screen_measure_green,
    tick_screen_measure_parchment,
    tick_screen_measure_dry_cherry,
    tick_screen_setting,
    tick_screen_setting_language,
    tick_screen_zerong,
    tick_screen_zerong_m_d,
    tick_screen_zerong_aw,
    tick_screen_zerong_m_d_ing,
    tick_screen_zerong_aw_ing,
    tick_screen_zerong_m_d_ok,
    tick_screen_zerong_aw_ok,
    tick_screen_zerong_aw_warm,
    tick_screen_zerong_m_d_success,
    tick_screen_zerong_aw_success,
    tick_screen_about,
    tick_screen_about_message,
    tick_screen_setting_language_cn,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
    { "bold_60", &ui_font_bold_60 },
    { "bold_40", &ui_font_bold_40 },
    { "bold_28", &ui_font_bold_28 },
    { "bold_24", &ui_font_bold_24 },
    { "bold_20", &ui_font_bold_20 },
    { "bold_16", &ui_font_bold_16 },
    { "bold_14", &ui_font_bold_14 },
    { "bold_12", &ui_font_bold_12 },
    { "bold_10", &ui_font_bold_10 },
    { "regular_16", &ui_font_regular_16 },
    { "regular_14", &ui_font_regular_14 },
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_measure();
    create_screen_measure_roasted();
    create_screen_measure_green();
    create_screen_measure_parchment();
    create_screen_measure_dry_cherry();
    create_screen_setting();
    create_screen_setting_language();
    create_screen_zerong();
    create_screen_zerong_m_d();
    create_screen_zerong_aw();
    create_screen_zerong_m_d_ing();
    create_screen_zerong_aw_ing();
    create_screen_zerong_m_d_ok();
    create_screen_zerong_aw_ok();
    create_screen_zerong_aw_warm();
    create_screen_zerong_m_d_success();
    create_screen_zerong_aw_success();
    create_screen_about();
    create_screen_about_message();
    create_screen_setting_language_cn();
    ui_create_compat_objects();
}
