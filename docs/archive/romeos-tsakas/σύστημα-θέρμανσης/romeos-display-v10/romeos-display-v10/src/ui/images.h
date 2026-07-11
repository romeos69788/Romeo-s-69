#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_1;
extern const lv_img_dsc_t img_1_a;
extern const lv_img_dsc_t img_menu;
extern const lv_img_dsc_t img_mic;
extern const lv_img_dsc_t img_alarm;
extern const lv_img_dsc_t img_humidity;
extern const lv_img_dsc_t img_home;
extern const lv_img_dsc_t img_out_temp;
extern const lv_img_dsc_t img_wifi;
extern const lv_img_dsc_t img_boiler;
extern const lv_img_dsc_t img_back;
extern const lv_img_dsc_t img_heat;
extern const lv_img_dsc_t img_boiler1;
extern const lv_img_dsc_t img_light;
extern const lv_img_dsc_t img_blinds;
extern const lv_img_dsc_t img_1_sec;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[16];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/