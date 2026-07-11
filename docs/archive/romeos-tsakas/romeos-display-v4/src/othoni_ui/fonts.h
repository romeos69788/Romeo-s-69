#ifndef EEZ_LVGL_UI_FONTS_H
#define EEZ_LVGL_UI_FONTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t ui_font_120;
extern const lv_font_t ui_font_60;
extern const lv_font_t ui_font_70;
extern const lv_font_t ui_font_50;
extern const lv_font_t ui_font_40;
extern const lv_font_t ui_font_30;
extern const lv_font_t ui_font_140;
extern const lv_font_t ui_font_90;
extern const lv_font_t ui_font_20;
extern const lv_font_t ui_font_45;
extern const lv_font_t ui_font_15;
extern const lv_font_t ui_font_35;
extern const lv_font_t ui_font_25;

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
