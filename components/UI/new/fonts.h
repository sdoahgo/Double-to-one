#ifndef EEZ_LVGL_UI_FONTS_H
#define EEZ_LVGL_UI_FONTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t ui_font_bold_60;
extern const lv_font_t ui_font_bold_40;
extern const lv_font_t ui_font_bold_28;
extern const lv_font_t ui_font_bold_24;
extern const lv_font_t ui_font_bold_20;
extern const lv_font_t ui_font_bold_16;
extern const lv_font_t ui_font_bold_14;
extern const lv_font_t ui_font_bold_12;
extern const lv_font_t ui_font_bold_10;
extern const lv_font_t ui_font_regular_16;
extern const lv_font_t ui_font_regular_14;

#ifndef EXT_FONT_DESC_T
#define EXT_FONT_DESC_T
typedef struct _ext_font_desc_t {
    const char *name;
    const void *font_ptr;
} ext_font_desc_t;
#endif

extern ext_font_desc_t fonts[];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_FONTS_H*/