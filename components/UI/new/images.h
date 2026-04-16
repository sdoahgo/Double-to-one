#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_wifi;
extern const lv_img_dsc_t img_bat;
extern const lv_img_dsc_t img_recharge;
extern const lv_img_dsc_t img_brightness_1;
extern const lv_img_dsc_t img_brightness_2;
extern const lv_img_dsc_t img_brightness_3;
extern const lv_img_dsc_t img_arrows;
extern const lv_img_dsc_t img_light;
extern const lv_img_dsc_t img_douzi_1;
extern const lv_img_dsc_t img_100;
extern const lv_img_dsc_t img_back;
extern const lv_img_dsc_t img_line;
extern const lv_img_dsc_t img_roasted;
extern const lv_img_dsc_t img_point_white;
extern const lv_img_dsc_t img_piont_black;
extern const lv_img_dsc_t img_green;
extern const lv_img_dsc_t img_parchment;
extern const lv_img_dsc_t img_drycherry;
extern const lv_img_dsc_t img_setting;
extern const lv_img_dsc_t img_chinese;
extern const lv_img_dsc_t img_english;
extern const lv_img_dsc_t img_cn;
extern const lv_img_dsc_t img_en;
extern const lv_img_dsc_t img_bat_2;
extern const lv_img_dsc_t img_wifi_2;
extern const lv_img_dsc_t img_zerong;
extern const lv_img_dsc_t img_zerong_1;
extern const lv_img_dsc_t img_m_d;
extern const lv_img_dsc_t img_a;
extern const lv_img_dsc_t img_zerong_2;
extern const lv_img_dsc_t img_a_1;
extern const lv_img_dsc_t img_m_d_1;
extern const lv_img_dsc_t img_warming;
extern const lv_img_dsc_t img_m_d_2;
extern const lv_img_dsc_t img_a_2;
extern const lv_img_dsc_t img_about;
extern const lv_img_dsc_t bat5Percent;
extern const lv_img_dsc_t bat20Percent;
extern const lv_img_dsc_t bat40Percent;
extern const lv_img_dsc_t bat60Percent;
extern const lv_img_dsc_t bat80Percent;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[36];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/
