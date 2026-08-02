#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_COUNT 6

enum ScreensEnum {
    SCREEN_ID_1 = 1, /* home */
    SCREEN_ID_2 = 2, /* menu */
    SCREEN_ID_3 = 3, /* heat pump */
    SCREEN_ID_4 = 4, /* lights */
    SCREEN_ID_5 = 5, /* heating */
    SCREEN_ID_6 = 6  /* shutters */
};

typedef struct _objects_t {
    lv_obj_t *screens[SCREEN_COUNT];
    lv_obj_t *btn_menu;
    lv_obj_t *btn_back;
    lv_obj_t *btn_plus;
    lv_obj_t *btn_minus;
    lv_obj_t *arc_setpoint;
    lv_obj_t *wifi_overlay;
    lv_obj_t *wifi_overlay_2;
    lv_obj_t *btn_hp;
    lv_obj_t *btn_lights;
    lv_obj_t *btn_heat;
    lv_obj_t *btn_rola;
} objects_t;

extern objects_t objects;

void create_screens(void);
void tick_screen(int screen_index);
void ui_set_wifi_connected(bool connected);
void ui_clock_set(const char *hh, const char *mm, const char *day, const char *month, bool colon_visible);

#ifdef __cplusplus
}
#endif

#endif
