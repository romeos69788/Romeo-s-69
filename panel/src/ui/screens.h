#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_WATER = 2,
    SCREEN_ID_HP = 3,
    SCREEN_ID_OUT = 4,
    SCREEN_ID_BOILER = 5,
    SCREEN_ID_SYSTEM = 6,
    SCREEN_ID_WIFI = 7,
    _SCREEN_ID_LAST = 7
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *water;
    lv_obj_t *hp;
    lv_obj_t *out;
    lv_obj_t *boiler;
    lv_obj_t *system;
    lv_obj_t *wifi;
    lv_obj_t *____;
    lv_obj_t *obj0;
    lv_obj_t *_________________;
    lv_obj_t *obj1;
    lv_obj_t *______________;
    lv_obj_t *obj2;
    lv_obj_t *_______________;
    lv_obj_t *obj3;
    lv_obj_t *__________________;
    lv_obj_t *obj4;
    lv_obj_t *________________;
    lv_obj_t *obj5;
    lv_obj_t *__________;
    lv_obj_t *_____;
    lv_obj_t *______1;
    lv_obj_t *______2;
    lv_obj_t *______3;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *______4;
    lv_obj_t *______5;
    lv_obj_t *______6;
    lv_obj_t *______7;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *______8;
    lv_obj_t *______9;
    lv_obj_t *______10;
    lv_obj_t *______11;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *______12;
    lv_obj_t *______13;
    lv_obj_t *______14;
    lv_obj_t *______15;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *______16;
    lv_obj_t *______17;
    lv_obj_t *______18;
    lv_obj_t *______19;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *obj27;
    lv_obj_t *obj28;
    lv_obj_t *______20;
    lv_obj_t *______21;
    lv_obj_t *______22;
    lv_obj_t *______23;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *obj32;
    lv_obj_t *obj33;
    lv_obj_t *______24;
    lv_obj_t *______25;
    lv_obj_t *______26;
    lv_obj_t *______27;
    lv_obj_t *obj34;
    lv_obj_t *obj35;
    lv_obj_t *obj36;
    lv_obj_t *obj37;
    lv_obj_t *obj38;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_water();
void tick_screen_water();

void create_screen_hp();
void tick_screen_hp();

void create_screen_out();
void tick_screen_out();

void create_screen_boiler();
void tick_screen_boiler();

void create_screen_system();
void tick_screen_system();

void create_screen_wifi();
void tick_screen_wifi();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/