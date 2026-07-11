#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_prymary() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.prymary = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_1, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 244, 115);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_110, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "22");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 365, 115);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_110, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ".");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 387, 115);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_110, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "5");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 448, 139);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "o");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 462, 141);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_34, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 413, 261);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "+");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 272, 254);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_38, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "-");
        }
        {
            // menu_2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.menu_2 = obj;
            lv_obj_set_pos(obj, 182, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_menu, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // mic
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.mic = obj;
            lv_obj_set_pos(obj, 287, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_mic, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // alarm
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.alarm = obj;
            lv_obj_set_pos(obj, 67, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_alarm, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // humidity
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.humidity = obj;
            lv_obj_set_pos(obj, 41, 292);
            lv_obj_set_size(obj, 24, 24);
            lv_obj_set_style_bg_img_src(obj, &img_humidity, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // humidity_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.humidity_1 = obj;
            lv_obj_set_pos(obj, 41, 155);
            lv_obj_set_size(obj, 24, 24);
            lv_obj_set_style_bg_img_src(obj, &img_humidity, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 41, 88);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "24");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 41, 225);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "22");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 101, 94);
            lv_obj_set_size(obj, 10, 24);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "o");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 101, 230);
            lv_obj_set_size(obj, 10, 24);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "o");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 111, 232);
            lv_obj_set_size(obj, 19, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 111, 95);
            lv_obj_set_size(obj, 19, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 69, 152);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "65");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, 69, 288);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "50");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj15 = obj;
            lv_obj_set_pos(obj, 101, 288);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj16 = obj;
            lv_obj_set_pos(obj, 101, 152);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj17 = obj;
            lv_obj_set_pos(obj, 636, 10);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "23");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj18 = obj;
            lv_obj_set_pos(obj, 700, 4);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj19 = obj;
            lv_obj_set_pos(obj, 716, 10);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "30");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj20 = obj;
            lv_obj_set_pos(obj, 46, 68);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΕΞΩΤΕΡΙΚΗ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj21 = obj;
            lv_obj_set_pos(obj, 46, 210);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΕΣΩΤΕΡΙΚΗ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj22 = obj;
            lv_obj_set_pos(obj, 63, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ALARM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj23 = obj;
            lv_obj_set_pos(obj, 180, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MENU");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj24 = obj;
            lv_obj_set_pos(obj, 295, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MIC");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj25 = obj;
            lv_obj_set_pos(obj, 536, 364);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σπιτι");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj26 = obj;
            lv_obj_set_pos(obj, 684, 364);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Λεβητα");
        }
        {
            // wifi
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.wifi = obj;
            lv_obj_set_pos(obj, 478, 353);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_wifi, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // boiler_@
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.boiler__ = obj;
            lv_obj_set_pos(obj, 627, 353);
            lv_obj_set_size(obj, 39, 40);
            lv_obj_set_style_bg_img_src(obj, &img_boiler, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj27 = obj;
            lv_obj_set_pos(obj, 698, 71);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευη");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj28 = obj;
            lv_obj_set_pos(obj, 675, 94);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x008cec), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "26");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj29 = obj;
            lv_obj_set_pos(obj, 702, 94);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρη");
        }
        {
            // button_minus
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_minus = obj;
            lv_obj_set_pos(obj, 244, 253);
            lv_obj_set_size(obj, 87, 50);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_plus
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_plus = obj;
            lv_obj_set_pos(obj, 383, 253);
            lv_obj_set_size(obj, 87, 50);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_menu
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_menu = obj;
            lv_obj_set_pos(obj, 160, 369);
            lv_obj_set_size(obj, 84, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_alarm
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_alarm = obj;
            lv_obj_set_pos(obj, 46, 369);
            lv_obj_set_size(obj, 88, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_mic
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_mic = obj;
            lv_obj_set_pos(obj, 269, 369);
            lv_obj_set_size(obj, 80, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.obj30 = obj;
            lv_obj_set_pos(obj, 200, 43);
            lv_obj_set_size(obj, 320, 310);
            lv_arc_set_value(obj, 50);
            lv_arc_set_bg_start_angle(obj, 165);
            lv_arc_set_bg_end_angle(obj, 10);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0x008cec), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 8, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0x000000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0x008cec), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_spread(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 40, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 8, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0x008cec), LV_PART_KNOB | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_prymary();
}

void tick_screen_prymary() {
}

void create_screen_menu() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.menu = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_1, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // menu_3
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.menu_3 = obj;
            lv_obj_set_pos(obj, 182, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_menu, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // mic_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.mic_1 = obj;
            lv_obj_set_pos(obj, 287, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_mic, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // alarm_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.alarm_1 = obj;
            lv_obj_set_pos(obj, 67, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_alarm, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // humidity_2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.humidity_2 = obj;
            lv_obj_set_pos(obj, 41, 292);
            lv_obj_set_size(obj, 24, 24);
            lv_obj_set_style_bg_img_src(obj, &img_humidity, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // humidity_3
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.humidity_3 = obj;
            lv_obj_set_pos(obj, 41, 155);
            lv_obj_set_size(obj, 24, 24);
            lv_obj_set_style_bg_img_src(obj, &img_humidity, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj31 = obj;
            lv_obj_set_pos(obj, 41, 88);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "24");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj32 = obj;
            lv_obj_set_pos(obj, 41, 225);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "22");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj33 = obj;
            lv_obj_set_pos(obj, 101, 94);
            lv_obj_set_size(obj, 10, 24);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "o");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj34 = obj;
            lv_obj_set_pos(obj, 101, 230);
            lv_obj_set_size(obj, 10, 24);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "o");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj35 = obj;
            lv_obj_set_pos(obj, 111, 232);
            lv_obj_set_size(obj, 19, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj36 = obj;
            lv_obj_set_pos(obj, 111, 95);
            lv_obj_set_size(obj, 19, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "C");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj37 = obj;
            lv_obj_set_pos(obj, 69, 152);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "65");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj38 = obj;
            lv_obj_set_pos(obj, 69, 288);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "50");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj39 = obj;
            lv_obj_set_pos(obj, 101, 288);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj40 = obj;
            lv_obj_set_pos(obj, 101, 152);
            lv_obj_set_size(obj, 32, 30);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj41 = obj;
            lv_obj_set_pos(obj, 636, 10);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "23");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj42 = obj;
            lv_obj_set_pos(obj, 700, 4);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj43 = obj;
            lv_obj_set_pos(obj, 716, 10);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "30");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj44 = obj;
            lv_obj_set_pos(obj, 46, 68);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΕΞΩΤΕΡΙΚΗ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj45 = obj;
            lv_obj_set_pos(obj, 46, 210);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΕΣΩΤΕΡΙΚΗ");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj46 = obj;
            lv_obj_set_pos(obj, 63, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ALARM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj47 = obj;
            lv_obj_set_pos(obj, 180, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MENU");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj48 = obj;
            lv_obj_set_pos(obj, 295, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MIC");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj49 = obj;
            lv_obj_set_pos(obj, 536, 364);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σπιτι");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj50 = obj;
            lv_obj_set_pos(obj, 684, 364);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Λεβητα");
        }
        {
            // wifi_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.wifi_1 = obj;
            lv_obj_set_pos(obj, 478, 353);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_wifi, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // boiler_@_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.boiler___1 = obj;
            lv_obj_set_pos(obj, 627, 353);
            lv_obj_set_size(obj, 39, 40);
            lv_obj_set_style_bg_img_src(obj, &img_boiler, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj51 = obj;
            lv_obj_set_pos(obj, 698, 71);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευη");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj52 = obj;
            lv_obj_set_pos(obj, 675, 94);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x008cec), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "26");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj53 = obj;
            lv_obj_set_pos(obj, 702, 94);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρη");
        }
        {
            // button_menu_1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_menu_1 = obj;
            lv_obj_set_pos(obj, 160, 369);
            lv_obj_set_size(obj, 84, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_alarm_1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_alarm_1 = obj;
            lv_obj_set_pos(obj, 46, 369);
            lv_obj_set_size(obj, 88, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_mic_1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_mic_1 = obj;
            lv_obj_set_pos(obj, 269, 369);
            lv_obj_set_size(obj, 80, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_θερμανση
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_________ = obj;
            lv_obj_set_pos(obj, 212, 136);
            lv_obj_set_size(obj, 83, 96);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_φωτα
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_____ = obj;
            lv_obj_set_pos(obj, 295, 232);
            lv_obj_set_size(obj, 98, 96);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // HEAT_2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.heat_2 = obj;
            lv_obj_set_pos(obj, 212, 144);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_heat, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj54 = obj;
            lv_obj_set_pos(obj, 211, 212);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΘΕΡΜΑΝΣΗ");
        }
        {
            // ΛΕΒΗΤΑ
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.______ = obj;
            lv_obj_set_pos(obj, 327, 56);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_boiler, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj55 = obj;
            lv_obj_set_pos(obj, 327, 124);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x008cec), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΛΕΒΗΤΑ");
        }
        {
            // LIGTH
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ligth = obj;
            lv_obj_set_pos(obj, 318, 230);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_light, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj56 = obj;
            lv_obj_set_pos(obj, 329, 302);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x008cec), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΦΩΤΑ");
        }
        {
            // ΡΟΛΑ
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.____ = obj;
            lv_obj_set_pos(obj, 425, 144);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_blinds, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj57 = obj;
            lv_obj_set_pos(obj, 437, 213);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΡΟΛΑ");
        }
        {
            // button_ΛΕΒΗΤΑ
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_______ = obj;
            lv_obj_set_pos(obj, 309, 53);
            lv_obj_set_size(obj, 98, 96);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_ρολα_1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button______1 = obj;
            lv_obj_set_pos(obj, 407, 137);
            lv_obj_set_size(obj, 98, 96);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_menu();
}

void tick_screen_menu() {
}

void create_screen_heat() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.heat = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_1_sec, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // menu_4
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.menu_4 = obj;
            lv_obj_set_pos(obj, 182, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_back, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // mic_2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.mic_2 = obj;
            lv_obj_set_pos(obj, 287, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_mic, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // alarm_2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.alarm_2 = obj;
            lv_obj_set_pos(obj, 67, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_alarm, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj58 = obj;
            lv_obj_set_pos(obj, 63, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ALARM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj59 = obj;
            lv_obj_set_pos(obj, 180, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "BACK");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj60 = obj;
            lv_obj_set_pos(obj, 295, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MIC");
        }
        {
            // button_menu_2
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_menu_2 = obj;
            lv_obj_set_pos(obj, 160, 369);
            lv_obj_set_size(obj, 84, 70);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_alarm_2
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_alarm_2 = obj;
            lv_obj_set_pos(obj, 46, 369);
            lv_obj_set_size(obj, 88, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_mic_2
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_mic_2 = obj;
            lv_obj_set_pos(obj, 269, 369);
            lv_obj_set_size(obj, 80, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // HEAT_3
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.heat_3 = obj;
            lv_obj_set_pos(obj, 717, 0);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_heat, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj61 = obj;
            lv_obj_set_pos(obj, 716, 68);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΘΕΡΜΑΝΣΗ");
        }
    }
    
    tick_screen_heat();
}

void tick_screen_heat() {
}

void create_screen_boiler() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.boiler = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_1_sec, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // menu_5
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.menu_5 = obj;
            lv_obj_set_pos(obj, 182, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_menu, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // mic_3
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.mic_3 = obj;
            lv_obj_set_pos(obj, 287, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_mic, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // alarm_3
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.alarm_3 = obj;
            lv_obj_set_pos(obj, 67, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_alarm, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj62 = obj;
            lv_obj_set_pos(obj, 63, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ALARM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj63 = obj;
            lv_obj_set_pos(obj, 180, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MENU");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj64 = obj;
            lv_obj_set_pos(obj, 295, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MIC");
        }
        {
            // button_menu_3
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_menu_3 = obj;
            lv_obj_set_pos(obj, 160, 369);
            lv_obj_set_size(obj, 84, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_alarm_3
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_alarm_3 = obj;
            lv_obj_set_pos(obj, 46, 369);
            lv_obj_set_size(obj, 88, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_mic_3
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_mic_3 = obj;
            lv_obj_set_pos(obj, 269, 369);
            lv_obj_set_size(obj, 80, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ΛΕΒΗΤΑ_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects._______1 = obj;
            lv_obj_set_pos(obj, 736, 0);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_boiler, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj65 = obj;
            lv_obj_set_pos(obj, 736, 68);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x008cec), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΛΕΒΗΤΑ");
        }
    }
    
    tick_screen_boiler();
}

void tick_screen_boiler() {
}

void create_screen_light() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.light = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_1_sec, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // menu_6
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.menu_6 = obj;
            lv_obj_set_pos(obj, 182, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_menu, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // mic_4
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.mic_4 = obj;
            lv_obj_set_pos(obj, 287, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_mic, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // alarm_4
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.alarm_4 = obj;
            lv_obj_set_pos(obj, 67, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_alarm, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj66 = obj;
            lv_obj_set_pos(obj, 63, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ALARM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj67 = obj;
            lv_obj_set_pos(obj, 180, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MENU");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj68 = obj;
            lv_obj_set_pos(obj, 295, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MIC");
        }
        {
            // button_menu_4
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_menu_4 = obj;
            lv_obj_set_pos(obj, 160, 369);
            lv_obj_set_size(obj, 84, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_alarm_4
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_alarm_4 = obj;
            lv_obj_set_pos(obj, 46, 369);
            lv_obj_set_size(obj, 88, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_mic_4
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_mic_4 = obj;
            lv_obj_set_pos(obj, 269, 369);
            lv_obj_set_size(obj, 80, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // LIGTH_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.ligth_1 = obj;
            lv_obj_set_pos(obj, 736, 0);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_light, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj69 = obj;
            lv_obj_set_pos(obj, 747, 72);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x008cec), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΦΩΤΑ");
        }
    }
    
    tick_screen_light();
}

void tick_screen_light() {
}

void create_screen_blinds() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.blinds = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_1_sec, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // menu_7
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.menu_7 = obj;
            lv_obj_set_pos(obj, 182, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_menu, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // mic_5
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.mic_5 = obj;
            lv_obj_set_pos(obj, 287, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_mic, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // alarm_5
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.alarm_5 = obj;
            lv_obj_set_pos(obj, 67, 369);
            lv_obj_set_size(obj, 40, 40);
            lv_obj_set_style_bg_img_src(obj, &img_alarm, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_img_tiled(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj70 = obj;
            lv_obj_set_pos(obj, 63, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ALARM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj71 = obj;
            lv_obj_set_pos(obj, 180, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MENU");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj72 = obj;
            lv_obj_set_pos(obj, 295, 419);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MIC");
        }
        {
            // button_menu_5
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_menu_5 = obj;
            lv_obj_set_pos(obj, 160, 369);
            lv_obj_set_size(obj, 84, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_alarm_5
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_alarm_5 = obj;
            lv_obj_set_pos(obj, 46, 369);
            lv_obj_set_size(obj, 88, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // button_mic_5
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button_mic_5 = obj;
            lv_obj_set_pos(obj, 269, 369);
            lv_obj_set_size(obj, 80, 70);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ΡΟΛΑ_1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects._____1 = obj;
            lv_obj_set_pos(obj, 736, 0);
            lv_obj_set_size(obj, 64, 64);
            lv_obj_set_style_bg_img_src(obj, &img_blinds, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj73 = obj;
            lv_obj_set_pos(obj, 748, 69);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x315275), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "ΡΟΛΑ");
        }
    }
    
    tick_screen_blinds();
}

void tick_screen_blinds() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_prymary,
    tick_screen_menu,
    tick_screen_heat,
    tick_screen_boiler,
    tick_screen_light,
    tick_screen_blinds,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 6) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
    { "110", &ui_font_110 },
    { "15", &ui_font_15 },
    { "20", &ui_font_20 },
    { "25", &ui_font_25 },
    { "50", &ui_font_50 },
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_prymary();
    create_screen_menu();
    create_screen_heat();
    create_screen_boiler();
    create_screen_light();
    create_screen_blinds();
}