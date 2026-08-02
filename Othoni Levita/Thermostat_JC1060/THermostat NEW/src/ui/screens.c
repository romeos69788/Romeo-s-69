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

void create_screen_1a() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.1a = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_bg_img_src(obj, &img_1, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // MENU
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.menu = obj;
            lv_obj_set_pos(obj, 56, 501);
            lv_obj_set_size(obj, 189, 79);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ΜΕΝΟΥ
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._____ = obj;
            lv_obj_set_pos(obj, 138, 523);
            lv_obj_set_size(obj, 97, 35);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MENU");
        }
        {
            // +
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects._ = obj;
            lv_obj_set_pos(obj, 458, 282);
            lv_obj_set_size(obj, 98, 85);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // κεντρική_θερμοκρασία
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.____________________ = obj;
            lv_obj_set_pos(obj, 316, 150);
            lv_obj_set_size(obj, 142, 120);
            lv_obj_set_style_text_font(obj, &ui_font_120, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "22");
        }
        {
            // κεντρική_θερμοκρασία_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._____________________1 = obj;
            lv_obj_set_pos(obj, 458, 170);
            lv_obj_set_size(obj, 25, 100);
            lv_obj_set_style_text_font(obj, &ui_font_100, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ".");
        }
        {
            // κεντρική_θερμοκρασία_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._____________________2 = obj;
            lv_obj_set_pos(obj, 483, 150);
            lv_obj_set_size(obj, 63, 121);
            lv_obj_set_style_text_font(obj, &ui_font_120, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "5");
        }
        {
            // κεντρική_θερμοκρασία_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._____________________3 = obj;
            lv_obj_set_pos(obj, 556, 160);
            lv_obj_set_size(obj, 53, 101);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "O");
        }
        {
            // εξωτερική θερμοκρασία
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._____________________ = obj;
            lv_obj_set_pos(obj, 99, 73);
            lv_obj_set_size(obj, 60, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "19");
        }
        {
            // εξωτερική θερμοκρασία_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______________________1 = obj;
            lv_obj_set_pos(obj, 157, 73);
            lv_obj_set_size(obj, 15, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ".");
        }
        {
            // εξωτερική θερμοκρασία_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______________________2 = obj;
            lv_obj_set_pos(obj, 167, 73);
            lv_obj_set_size(obj, 30, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "5");
        }
        {
            // εξωτερική θερμοκρασία_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______________________3 = obj;
            lv_obj_set_pos(obj, 194, 73);
            lv_obj_set_size(obj, 18, 55);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "O");
        }
        {
            // υγρασία
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._______ = obj;
            lv_obj_set_pos(obj, 110, 155);
            lv_obj_set_size(obj, 60, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "43");
        }
        {
            // υγρασία_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.________1 = obj;
            lv_obj_set_pos(obj, 175, 158);
            lv_obj_set_size(obj, 60, 55);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "%");
        }
        {
            // ώρα
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.___ = obj;
            lv_obj_set_pos(obj, 743, 62);
            lv_obj_set_size(obj, 62, 51);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "09");
        }
        {
            // ώρα_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.____1 = obj;
            lv_obj_set_pos(obj, 809, 62);
            lv_obj_set_size(obj, 14, 51);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ώρα_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.____2 = obj;
            lv_obj_set_pos(obj, 823, 62);
            lv_obj_set_size(obj, 62, 51);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "24");
        }
        {
            // ημερομηνία
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.__________ = obj;
            lv_obj_set_pos(obj, 681, 135);
            lv_obj_set_size(obj, 62, 51);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "24");
        }
        {
            // ημερομηνία_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.___________1 = obj;
            lv_obj_set_pos(obj, 764, 148);
            lv_obj_set_size(obj, 180, 45);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
        {
            // εσωτερική_υγρασία
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._________________ = obj;
            lv_obj_set_pos(obj, 577, 483);
            lv_obj_set_size(obj, 60, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "43");
        }
        {
            // εσωτερική_υγρασία_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.__________________1 = obj;
            lv_obj_set_pos(obj, 642, 486);
            lv_obj_set_size(obj, 60, 55);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "%");
        }
        {
            // εσωτερική θερμοκρασία_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______________________4 = obj;
            lv_obj_set_pos(obj, 381, 483);
            lv_obj_set_size(obj, 60, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "23");
        }
        {
            // εσωτερική θερμοκρασία_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______________________5 = obj;
            lv_obj_set_pos(obj, 439, 483);
            lv_obj_set_size(obj, 15, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ".");
        }
        {
            // εσωτερική θερμοκρασία_6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______________________6 = obj;
            lv_obj_set_pos(obj, 449, 483);
            lv_obj_set_size(obj, 30, 55);
            lv_obj_set_style_text_font(obj, &ui_font_50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "5");
        }
        {
            // εσωτερική θερμοκρασία_7
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______________________7 = obj;
            lv_obj_set_pos(obj, 476, 483);
            lv_obj_set_size(obj, 18, 55);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "O");
        }
        {
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 283, 43);
            lv_obj_set_size(obj, 330, 402);
            lv_arc_set_value(obj, 50);
            lv_arc_set_bg_start_angle(obj, 170);
            lv_arc_set_bg_end_angle(obj, 15);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0x2d2b2b), LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // BUTON-
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.buton_ = obj;
            lv_obj_set_pos(obj, 342, 282);
            lv_obj_set_size(obj, 95, 85);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_1a();
}

void tick_screen_1a() {
}

void create_screen_2a() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.2a = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_bg_img_src(obj, &img_2, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // MENU_1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.menu_1 = obj;
            lv_obj_set_pos(obj, 56, 501);
            lv_obj_set_size(obj, 189, 79);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ΜΕΝΟΥ_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______1 = obj;
            lv_obj_set_pos(obj, 137, 531);
            lv_obj_set_size(obj, 97, 35);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "BACK");
        }
        {
            // Αντλίαθερμότητας
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.________________ = obj;
            lv_obj_set_pos(obj, 252, 230);
            lv_obj_set_size(obj, 247, 37);
            lv_obj_set_style_text_font(obj, &ui_font_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Αντλία Θερμότητας");
        }
        {
            // FOTA
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fota = obj;
            lv_obj_set_pos(obj, 598, 230);
            lv_obj_set_size(obj, 86, 37);
            lv_obj_set_style_text_font(obj, &ui_font_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Φώτα");
        }
        {
            // THERMANSI
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.thermansi = obj;
            lv_obj_set_pos(obj, 306, 456);
            lv_obj_set_size(obj, 139, 37);
            lv_obj_set_style_text_font(obj, &ui_font_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Θέρμανση");
        }
        {
            // ROLA
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.rola = obj;
            lv_obj_set_pos(obj, 603, 456);
            lv_obj_set_size(obj, 77, 37);
            lv_obj_set_style_text_font(obj, &ui_font_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Ρολά");
        }
        {
            // HP
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.hp = obj;
            lv_obj_set_pos(obj, 252, 71);
            lv_obj_set_size(obj, 247, 206);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // LIGHTS
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.lights = obj;
            lv_obj_set_pos(obj, 518, 71);
            lv_obj_set_size(obj, 247, 206);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ROLA1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.rola1 = obj;
            lv_obj_set_pos(obj, 518, 295);
            lv_obj_set_size(obj, 247, 206);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // HEAT
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.heat = obj;
            lv_obj_set_pos(obj, 252, 295);
            lv_obj_set_size(obj, 247, 206);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_2a();
}

void tick_screen_2a() {
}

void create_screen_3a() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.3a = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_bg_img_src(obj, &img_3, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    tick_screen_3a();
}

void tick_screen_3a() {
}

void create_screen_4a() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.4a = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_bg_img_src(obj, &img_4, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    tick_screen_4a();
}

void tick_screen_4a() {
}

void create_screen_5a() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.5a = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_bg_img_src(obj, &img_5, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    tick_screen_5a();
}

void tick_screen_5a() {
}

void create_screen_6a() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.6a = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_bg_img_src(obj, &img_6, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    tick_screen_6a();
}

void tick_screen_6a() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_1a,
    tick_screen_2a,
    tick_screen_3a,
    tick_screen_4a,
    tick_screen_5a,
    tick_screen_6a,
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
    { "25", &ui_font_25 },
    { "26", &ui_font_26 },
    { "28", &ui_font_28 },
    { "50", &ui_font_50 },
    { "100", &ui_font_100 },
    { "120", &ui_font_120 },
    { "30", &ui_font_30 },
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
    create_screen_1a();
    create_screen_2a();
    create_screen_3a();
    create_screen_4a();
    create_screen_5a();
    create_screen_6a();
}