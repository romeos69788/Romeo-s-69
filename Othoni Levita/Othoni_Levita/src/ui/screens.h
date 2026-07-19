#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN_1 = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *main_1;
    lv_obj_t *water;
    lv_obj_t *obj0;
    lv_obj_t *heat_pump;
    lv_obj_t *obj1;
    lv_obj_t *outputs;
    lv_obj_t *obj2;
    lv_obj_t *boiler_;
    lv_obj_t *obj3;
    lv_obj_t *system;
    lv_obj_t *obj4;
    lv_obj_t *network;
    lv_obj_t *obj5;
    lv_obj_t *cl_oc_k;
    lv_obj_t *cl_oc_k_1;
    lv_obj_t *cl_oc_k_2;
    lv_obj_t *data;
    lv_obj_t *data_1;
    lv_obj_t *data_2;
    lv_obj_t *cl_oc_k_3;
} objects_t;

extern objects_t objects;

void create_screen_main_1();
void tick_screen_main_1();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/