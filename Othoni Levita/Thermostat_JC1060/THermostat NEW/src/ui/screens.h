#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_1A = 1,
    SCREEN_ID_2A = 2,
    SCREEN_ID_3A = 3,
    SCREEN_ID_4A = 4,
    SCREEN_ID_5A = 5,
    SCREEN_ID_6A = 6,
    _SCREEN_ID_LAST = 6
};

typedef struct _objects_t {
    lv_obj_t *1a;
    lv_obj_t *2a;
    lv_obj_t *3a;
    lv_obj_t *4a;
    lv_obj_t *5a;
    lv_obj_t *6a;
} objects_t;

extern objects_t objects;

void create_screen_1a();
void tick_screen_1a();

void create_screen_2a();
void tick_screen_2a();

void create_screen_3a();
void tick_screen_3a();

void create_screen_4a();
void tick_screen_4a();

void create_screen_5a();
void tick_screen_5a();

void create_screen_6a();
void tick_screen_6a();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/