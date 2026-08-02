#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_COUNT 6

enum ScreensEnum {
    SCREEN_ID_1 = 1,
    SCREEN_ID_2 = 2,
    SCREEN_ID_3 = 3,
    SCREEN_ID_4 = 4,
    SCREEN_ID_5 = 5,
    SCREEN_ID_6 = 6
};

typedef struct _objects_t {
    lv_obj_t *screens[SCREEN_COUNT];
} objects_t;

extern objects_t objects;

void create_screens(void);
void tick_screen(int screen_index);

#ifdef __cplusplus
}
#endif

#endif
