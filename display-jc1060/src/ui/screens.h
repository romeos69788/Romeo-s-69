#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ScreensEnum {
    SCREEN_ID_1 = 1,
    SCREEN_ID_2 = 2
};

typedef struct _objects_t {
    lv_obj_t *screen_1;
    lv_obj_t *screen_2;
} objects_t;

extern objects_t objects;

void create_screens(void);
void tick_screen(int screen_index);

#ifdef __cplusplus
}
#endif

#endif
