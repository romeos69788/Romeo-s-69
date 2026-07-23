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

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(obj, &img_8, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ΝΕΡΟ
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.____ = obj;
            lv_obj_set_pos(obj, 10, 52);
            lv_obj_set_size(obj, 237, 188);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj0 = obj;
                    lv_obj_set_pos(obj, -5, -55);
                    lv_obj_set_size(obj, 68, 32);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x015ffd), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ΝΕΡΟ");
                }
            }
        }
        {
            // Αντλία_Θερμότητας
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects._________________ = obj;
            lv_obj_set_pos(obj, 260, 57);
            lv_obj_set_size(obj, 255, 183);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj1 = obj;
                    lv_obj_set_pos(obj, -12, -55);
                    lv_obj_set_size(obj, 94, 32);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x00d100), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ΑΝΤΛΙΑ");
                }
            }
        }
        {
            // Ελεγχος_Εξόδου
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.______________ = obj;
            lv_obj_set_pos(obj, 525, 58);
            lv_obj_set_size(obj, 261, 182);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj2 = obj;
                    lv_obj_set_pos(obj, -18, -55);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xed0101), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ΕΞΟΔΟΙ");
                }
            }
        }
        {
            // Ελεγχος_μπόιλερ
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects._______________ = obj;
            lv_obj_set_pos(obj, 10, 251);
            lv_obj_set_size(obj, 237, 174);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj3 = obj;
                    lv_obj_set_pos(obj, -7, -51);
                    lv_obj_set_size(obj, 116, 32);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xfe8302), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ΜΠΟΙΛΕΡ");
                }
            }
        }
        {
            // Ελεγχος_Συστήματος
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.__________________ = obj;
            lv_obj_set_pos(obj, 260, 251);
            lv_obj_set_size(obj, 255, 174);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj4 = obj;
                    lv_obj_set_pos(obj, 5, -50);
                    lv_obj_set_size(obj, 157, 32);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x585c61), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ΣΥΣΤΗΜΑ");
                }
            }
        }
        {
            // ΕλεγχοςΣυνδέσεων
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.________________ = obj;
            lv_obj_set_pos(obj, 525, 251);
            lv_obj_set_size(obj, 261, 174);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2196f3), LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_img_opa(obj, 255, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_img_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj5 = obj;
                    lv_obj_set_pos(obj, -16, -50);
                    lv_obj_set_size(obj, 106, 32);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x005cfd), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ΔΙΚΤΥΟ");
                }
            }
        }
        {
            // συναγερμός
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.__________ = obj;
            lv_obj_set_pos(obj, 10, 434);
            lv_obj_set_size(obj, 779, 35);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ρολόι
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects._____ = obj;
            lv_obj_set_pos(obj, 642, 12);
            lv_obj_set_size(obj, 39, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "08");
        }
        {
            // ρολόι_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______1 = obj;
            lv_obj_set_pos(obj, 683, 10);
            lv_obj_set_size(obj, 7, 33);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ρολόι_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______2 = obj;
            lv_obj_set_pos(obj, 692, 12);
            lv_obj_set_size(obj, 42, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "28");
        }
        {
            // ρολόι_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______3 = obj;
            lv_obj_set_pos(obj, 736, 12);
            lv_obj_set_size(obj, 34, 22);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, -172, -214);
            lv_obj_set_size(obj, 37, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "29");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, -265, -212);
            lv_obj_set_size(obj, 130, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευή");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, -71, -213);
            lv_obj_set_size(obj, 148, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_water() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.water = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ρολόι_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______4 = obj;
            lv_obj_set_pos(obj, 642, 12);
            lv_obj_set_size(obj, 39, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "08");
        }
        {
            // ρολόι_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______5 = obj;
            lv_obj_set_pos(obj, 683, 10);
            lv_obj_set_size(obj, 7, 33);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ρολόι_6
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______6 = obj;
            lv_obj_set_pos(obj, 692, 12);
            lv_obj_set_size(obj, 36, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "28");
        }
        {
            // ρολόι_7
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______7 = obj;
            lv_obj_set_pos(obj, 737, 12);
            lv_obj_set_size(obj, 34, 22);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, -172, -214);
            lv_obj_set_size(obj, 37, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "29");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, -265, -212);
            lv_obj_set_size(obj, 130, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευή");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, -71, -213);
            lv_obj_set_size(obj, 148, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 400, 428);
            lv_obj_set_size(obj, 103, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Μενού");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 318, 221);
            lv_obj_set_size(obj, 134, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "WATER");
        }
    }
    
    tick_screen_water();
}

void tick_screen_water() {
}

void create_screen_hp() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.hp = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ρολόι_8
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______8 = obj;
            lv_obj_set_pos(obj, 642, 12);
            lv_obj_set_size(obj, 39, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "08");
        }
        {
            // ρολόι_9
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______9 = obj;
            lv_obj_set_pos(obj, 683, 10);
            lv_obj_set_size(obj, 7, 33);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ρολόι_10
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______10 = obj;
            lv_obj_set_pos(obj, 692, 12);
            lv_obj_set_size(obj, 36, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "28");
        }
        {
            // ρολόι_11
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______11 = obj;
            lv_obj_set_pos(obj, 737, 12);
            lv_obj_set_size(obj, 34, 22);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, -172, -214);
            lv_obj_set_size(obj, 37, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "29");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj15 = obj;
            lv_obj_set_pos(obj, -265, -212);
            lv_obj_set_size(obj, 130, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευή");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj16 = obj;
            lv_obj_set_pos(obj, -71, -213);
            lv_obj_set_size(obj, 148, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj17 = obj;
            lv_obj_set_pos(obj, 400, 428);
            lv_obj_set_size(obj, 103, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Μενού");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj18 = obj;
            lv_obj_set_pos(obj, 360, 202);
            lv_obj_set_size(obj, 134, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "H/P");
        }
    }
    
    tick_screen_hp();
}

void tick_screen_hp() {
}

void create_screen_out() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.out = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ρολόι_12
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______12 = obj;
            lv_obj_set_pos(obj, 642, 12);
            lv_obj_set_size(obj, 39, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "08");
        }
        {
            // ρολόι_13
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______13 = obj;
            lv_obj_set_pos(obj, 683, 10);
            lv_obj_set_size(obj, 7, 33);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ρολόι_14
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______14 = obj;
            lv_obj_set_pos(obj, 692, 12);
            lv_obj_set_size(obj, 36, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "28");
        }
        {
            // ρολόι_15
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______15 = obj;
            lv_obj_set_pos(obj, 737, 12);
            lv_obj_set_size(obj, 34, 22);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj19 = obj;
            lv_obj_set_pos(obj, -172, -214);
            lv_obj_set_size(obj, 37, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "29");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj20 = obj;
            lv_obj_set_pos(obj, -265, -212);
            lv_obj_set_size(obj, 130, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευή");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj21 = obj;
            lv_obj_set_pos(obj, -71, -213);
            lv_obj_set_size(obj, 148, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj22 = obj;
            lv_obj_set_pos(obj, 400, 428);
            lv_obj_set_size(obj, 103, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Μενού");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj23 = obj;
            lv_obj_set_pos(obj, 360, 202);
            lv_obj_set_size(obj, 134, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "OUT");
        }
    }
    
    tick_screen_out();
}

void tick_screen_out() {
}

void create_screen_boiler() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.boiler = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ρολόι_16
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______16 = obj;
            lv_obj_set_pos(obj, 642, 12);
            lv_obj_set_size(obj, 39, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "08");
        }
        {
            // ρολόι_17
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______17 = obj;
            lv_obj_set_pos(obj, 683, 10);
            lv_obj_set_size(obj, 7, 33);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ρολόι_18
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______18 = obj;
            lv_obj_set_pos(obj, 692, 12);
            lv_obj_set_size(obj, 36, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "28");
        }
        {
            // ρολόι_19
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______19 = obj;
            lv_obj_set_pos(obj, 737, 12);
            lv_obj_set_size(obj, 34, 22);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj24 = obj;
            lv_obj_set_pos(obj, -172, -214);
            lv_obj_set_size(obj, 37, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "29");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj25 = obj;
            lv_obj_set_pos(obj, -265, -212);
            lv_obj_set_size(obj, 130, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευή");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj26 = obj;
            lv_obj_set_pos(obj, -71, -213);
            lv_obj_set_size(obj, 148, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj27 = obj;
            lv_obj_set_pos(obj, 400, 428);
            lv_obj_set_size(obj, 103, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Μενού");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj28 = obj;
            lv_obj_set_pos(obj, 360, 202);
            lv_obj_set_size(obj, 134, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "BOILER");
        }
    }
    
    tick_screen_boiler();
}

void tick_screen_boiler() {
}

void create_screen_system() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.system = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ρολόι_20
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______20 = obj;
            lv_obj_set_pos(obj, 642, 12);
            lv_obj_set_size(obj, 39, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "08");
        }
        {
            // ρολόι_21
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______21 = obj;
            lv_obj_set_pos(obj, 683, 10);
            lv_obj_set_size(obj, 7, 33);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ρολόι_22
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______22 = obj;
            lv_obj_set_pos(obj, 692, 12);
            lv_obj_set_size(obj, 36, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "28");
        }
        {
            // ρολόι_23
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______23 = obj;
            lv_obj_set_pos(obj, 737, 12);
            lv_obj_set_size(obj, 34, 22);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj29 = obj;
            lv_obj_set_pos(obj, -172, -214);
            lv_obj_set_size(obj, 37, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "29");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj30 = obj;
            lv_obj_set_pos(obj, -265, -212);
            lv_obj_set_size(obj, 130, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευή");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj31 = obj;
            lv_obj_set_pos(obj, -71, -213);
            lv_obj_set_size(obj, 148, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj32 = obj;
            lv_obj_set_pos(obj, 400, 428);
            lv_obj_set_size(obj, 103, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Μενού");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj33 = obj;
            lv_obj_set_pos(obj, 360, 202);
            lv_obj_set_size(obj, 134, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM");
        }
    }
    
    tick_screen_system();
}

void tick_screen_system() {
}

void create_screen_wifi() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.wifi = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_img_src(obj, &img_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ρολόι_24
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______24 = obj;
            lv_obj_set_pos(obj, 642, 12);
            lv_obj_set_size(obj, 39, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "08");
        }
        {
            // ρολόι_25
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______25 = obj;
            lv_obj_set_pos(obj, 683, 10);
            lv_obj_set_size(obj, 7, 33);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, ":");
        }
        {
            // ρολόι_26
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______26 = obj;
            lv_obj_set_pos(obj, 692, 12);
            lv_obj_set_size(obj, 36, 33);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "28");
        }
        {
            // ρολόι_27
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.______27 = obj;
            lv_obj_set_pos(obj, 737, 12);
            lv_obj_set_size(obj, 34, 22);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AM");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj34 = obj;
            lv_obj_set_pos(obj, -172, -214);
            lv_obj_set_size(obj, 37, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "29");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj35 = obj;
            lv_obj_set_pos(obj, -265, -212);
            lv_obj_set_size(obj, 130, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Παρασκευή");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj36 = obj;
            lv_obj_set_pos(obj, -71, -213);
            lv_obj_set_size(obj, 148, 32);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x274157), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_25, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Σεπτεμβρίου");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj37 = obj;
            lv_obj_set_pos(obj, 400, 428);
            lv_obj_set_size(obj, 103, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Μενού");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj38 = obj;
            lv_obj_set_pos(obj, 360, 202);
            lv_obj_set_size(obj, 134, 38);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x212121), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "WIFI");
        }
    }
    
    tick_screen_wifi();
}

void tick_screen_wifi() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_water,
    tick_screen_hp,
    tick_screen_out,
    tick_screen_boiler,
    tick_screen_system,
    tick_screen_wifi,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 7) {
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
    { "28", &ui_font_28 },
    { "30", &ui_font_30 },
    { "26", &ui_font_26 },
    { "25", &ui_font_25 },
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
    create_screen_main();
    create_screen_water();
    create_screen_hp();
    create_screen_out();
    create_screen_boiler();
    create_screen_system();
    create_screen_wifi();
}