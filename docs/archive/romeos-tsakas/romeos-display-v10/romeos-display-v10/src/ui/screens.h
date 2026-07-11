#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_PRYMARY = 1,
    SCREEN_ID_MENU = 2,
    SCREEN_ID_HEAT = 3,
    SCREEN_ID_BOILER = 4,
    SCREEN_ID_LIGHT = 5,
    SCREEN_ID_BLINDS = 6,
    _SCREEN_ID_LAST = 6
};

typedef struct _objects_t {
    lv_obj_t *prymary;
    lv_obj_t *menu;
    lv_obj_t *heat;
    lv_obj_t *boiler;
    lv_obj_t *light;
    lv_obj_t *blinds;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *menu_2;
    lv_obj_t *mic;
    lv_obj_t *alarm;
    lv_obj_t *humidity;
    lv_obj_t *humidity_1;
    lv_obj_t *home;
    lv_obj_t *out_temp;
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
    lv_obj_t *obj27;
    lv_obj_t *wifi;
    lv_obj_t *boiler__;
    lv_obj_t *obj28;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *button_minus;
    lv_obj_t *button_plus;
    lv_obj_t *button_menu;
    lv_obj_t *button_alarm;
    lv_obj_t *button_mic;
    lv_obj_t *mic_1;
    lv_obj_t *alarm_1;
    lv_obj_t *humidity_2;
    lv_obj_t *humidity_3;
    lv_obj_t *home_1;
    lv_obj_t *out_temp_1;
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
    lv_obj_t *back;
    lv_obj_t *heat_2;
    lv_obj_t *obj48;
    lv_obj_t *______;
    lv_obj_t *obj49;
    lv_obj_t *ligth;
    lv_obj_t *obj50;
    lv_obj_t *____;
    lv_obj_t *obj51;
    lv_obj_t *button_________;
    lv_obj_t *button___________;
    lv_obj_t *button_____;
    lv_obj_t *button_menu_1;
    lv_obj_t *button_alarm_1;
    lv_obj_t *button_mic_1;
    lv_obj_t *obj52;
    lv_obj_t *obj53;
    lv_obj_t *wifi_1;
    lv_obj_t *boiler_wifi;
    lv_obj_t *obj54;
    lv_obj_t *obj55;
    lv_obj_t *obj56;
    lv_obj_t *obj57;
    lv_obj_t *button_______;
    lv_obj_t *mic_2;
    lv_obj_t *alarm_2;
    lv_obj_t *obj58;
    lv_obj_t *obj59;
    lv_obj_t *obj60;
    lv_obj_t *back_1;
    lv_obj_t *heat_3;
    lv_obj_t *obj61;
    lv_obj_t *button_menu_2;
    lv_obj_t *button_alarm_2;
    lv_obj_t *button_mic_2;
    lv_obj_t *mic_3;
    lv_obj_t *alarm_3;
    lv_obj_t *obj62;
    lv_obj_t *obj63;
    lv_obj_t *obj64;
    lv_obj_t *back_2;
    lv_obj_t *button_menu_3;
    lv_obj_t *button_alarm_3;
    lv_obj_t *button_mic_3;
    lv_obj_t *_______1;
    lv_obj_t *obj65;
    lv_obj_t *mic_4;
    lv_obj_t *alarm_4;
    lv_obj_t *obj66;
    lv_obj_t *obj67;
    lv_obj_t *obj68;
    lv_obj_t *back_3;
    lv_obj_t *button_menu_4;
    lv_obj_t *button_alarm_4;
    lv_obj_t *button_mic_4;
    lv_obj_t *ligth_1;
    lv_obj_t *obj69;
    lv_obj_t *mic_5;
    lv_obj_t *alarm_5;
    lv_obj_t *obj70;
    lv_obj_t *obj71;
    lv_obj_t *obj72;
    lv_obj_t *back_4;
    lv_obj_t *button_menu_5;
    lv_obj_t *button_alarm_5;
    lv_obj_t *button_mic_5;
    lv_obj_t *_____1;
    lv_obj_t *obj73;
} objects_t;

extern objects_t objects;

void create_screen_prymary();
void tick_screen_prymary();

void create_screen_menu();
void tick_screen_menu();

void create_screen_heat();
void tick_screen_heat();

void create_screen_boiler();
void tick_screen_boiler();

void create_screen_light();
void tick_screen_light();

void create_screen_blinds();
void tick_screen_blinds();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/