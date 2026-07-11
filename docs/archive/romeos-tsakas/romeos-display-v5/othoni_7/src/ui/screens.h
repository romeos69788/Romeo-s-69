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
    SCREEN_ID_SECONDARY_A = 2,
    SCREEN_ID_HEAT = 3,
    SCREEN_ID_LIGHTS = 4,
    SCREEN_ID_ROLLER_BLINDS = 5,
    SCREEN_ID_HEAT_PUMP = 6,
    _SCREEN_ID_LAST = 6
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *secondary_a;
    lv_obj_t *heat;
    lv_obj_t *lights;
    lv_obj_t *roller_blinds;
    lv_obj_t *heat_pump;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *alarm;
    lv_obj_t *menu;
    lv_obj_t *mic;
    lv_obj_t *plus;
    lv_obj_t *minus;
    lv_obj_t *back;
    lv_obj_t *obj27;
    lv_obj_t *alarm_1;
    lv_obj_t *obj28;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *obj32;
    lv_obj_t *obj33;
    lv_obj_t *obj34;
    lv_obj_t *obj35;
    lv_obj_t *obj36;
    lv_obj_t *obj37;
    lv_obj_t *obj38;
    lv_obj_t *obj39;
    lv_obj_t *obj40;
    lv_obj_t *obj41;
    lv_obj_t *obj42;
    lv_obj_t *obj43;
    lv_obj_t *obj44;
    lv_obj_t *obj45;
    lv_obj_t *obj46;
    lv_obj_t *obj47;
    lv_obj_t *obj48;
    lv_obj_t *obj49;
    lv_obj_t *obj50;
    lv_obj_t *obj51;
    lv_obj_t *mic_1;
    lv_obj_t *back_1;
    lv_obj_t *obj52;
    lv_obj_t *alarm_2;
    lv_obj_t *obj53;
    lv_obj_t *obj54;
    lv_obj_t *obj55;
    lv_obj_t *mic_2;
    lv_obj_t *back_2;
    lv_obj_t *obj56;
    lv_obj_t *alarm_3;
    lv_obj_t *obj57;
    lv_obj_t *obj58;
    lv_obj_t *obj59;
    lv_obj_t *mic_3;
    lv_obj_t *back_3;
    lv_obj_t *obj60;
    lv_obj_t *alarm_4;
    lv_obj_t *obj61;
    lv_obj_t *obj62;
    lv_obj_t *obj63;
    lv_obj_t *mic_4;
    lv_obj_t *back_4;
    lv_obj_t *obj64;
    lv_obj_t *alarm_5;
    lv_obj_t *obj65;
    lv_obj_t *obj66;
    lv_obj_t *obj67;
    lv_obj_t *mic_5;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_secondary_a();
void tick_screen_secondary_a();

void create_screen_heat();
void tick_screen_heat();

void create_screen_lights();
void tick_screen_lights();

void create_screen_roller_blinds();
void tick_screen_roller_blinds();

void create_screen_heat_pump();
void tick_screen_heat_pump();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/