#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_10;
extern const lv_img_dsc_t img_fire;
extern const lv_img_dsc_t img_a10;
extern const lv_img_dsc_t img___;
extern const lv_img_dsc_t img_menu;
extern const lv_img_dsc_t img_wi_fi;
extern const lv_img_dsc_t img_microphone;
extern const lv_img_dsc_t img_alarm;
extern const lv_img_dsc_t img_home;
extern const lv_img_dsc_t img_thermometer;
extern const lv_img_dsc_t img_out_home;
extern const lv_img_dsc_t img_drop;
extern const lv_img_dsc_t img_fondo;
extern const lv_img_dsc_t img_foto_1;
extern const lv_img_dsc_t img_wi;
extern const lv_img_dsc_t img_wfi;
extern const lv_img_dsc_t img_home1;
extern const lv_img_dsc_t img_out;
extern const lv_img_dsc_t img_bolier;
extern const lv_img_dsc_t img_12;
extern const lv_img_dsc_t img_menu1;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[21];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/