#include "panel_nav.h"

#include <Arduino.h>
#include <lvgl.h>

#include "ui/images.h"
#include "ui/screens.h"
#include "ui/ui.h"

namespace {

// OVER_* only slides the incoming screen (cheaper than MOVE_* which shifts both).
constexpr uint32_t k_anim_ms = 280;
bool s_on_hub = true;
bool s_busy = false;
int s_screen_id = SCREEN_ID_MAIN;

void busy_clear_cb(lv_timer_t *tm)
{
    s_busy = false;
    lv_timer_del(tm);
}

/** Touch flash-backed RGB565 so first paint is less likely to stall mid-slide. */
void prewarm_img(const lv_img_dsc_t *img)
{
    if (!img || !img->data || img->data_size == 0) {
        return;
    }
    const uint8_t *p = static_cast<const uint8_t *>(img->data);
    volatile uint32_t sink = 0;
    for (uint32_t i = 0; i < img->data_size; i += 64) {
        sink += p[i];
    }
    (void)sink;
}

void go_screen(enum ScreensEnum id, lv_scr_load_anim_t anim)
{
    if (s_busy || id == (ScreensEnum)s_screen_id) {
        return;
    }
    s_busy = true;
    s_on_hub = (id == SCREEN_ID_MAIN);
    s_screen_id = (int)id;
    loadScreenAnim(id, anim, k_anim_ms);
    lv_timer_t *t = lv_timer_create(busy_clear_cb, k_anim_ms + 20, nullptr);
    lv_timer_set_repeat_count(t, 1);
}

void on_tile(lv_event_t *e)
{
    // PRESSED: start slide on finger-down (CLICKED waits for release → feels like hitch)
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) {
        return;
    }
    const auto id = (ScreensEnum)(intptr_t)lv_event_get_user_data(e);
    go_screen(id, LV_SCR_LOAD_ANIM_OVER_LEFT);
}

void on_menu(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) {
        return;
    }
    go_screen(SCREEN_ID_MAIN, LV_SCR_LOAD_ANIM_OVER_RIGHT);
}

void wire_tile(lv_obj_t *btn, ScreensEnum id)
{
    if (!btn) {
        return;
    }
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, on_tile, LV_EVENT_PRESSED, (void *)(intptr_t)id);
}

/** Invisible hit target over alarm bar (arrow + Μενού baked in bg). */
void add_menu_hit(lv_obj_t *screen)
{
    if (!screen) {
        return;
    }
    lv_obj_t *btn = lv_btn_create(screen);
    lv_obj_set_pos(btn, 10, 434);
    lv_obj_set_size(btn, 779, 35);
    lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn, on_menu, LV_EVENT_PRESSED, nullptr);
    lv_obj_move_foreground(btn);
}

void also_wire_menu_label(lv_obj_t *label)
{
    if (!label) {
        return;
    }
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, on_menu, LV_EVENT_PRESSED, nullptr);
}

}  // namespace

void panel_nav_begin()
{
    // Detail screens share img_10 — warm it before first tile tap
    prewarm_img(&img_8);
    prewarm_img(&img_10);

    wire_tile(objects.____, SCREEN_ID_WATER);             // ΝΕΡΟ
    wire_tile(objects._________________, SCREEN_ID_HP);   // ΑΝΤΛΙΑ
    wire_tile(objects.______________, SCREEN_ID_OUT);     // ΕΞΟΔΟΙ
    wire_tile(objects._______________, SCREEN_ID_BOILER); // ΜΠΟΙΛΕΡ
    wire_tile(objects.__________________, SCREEN_ID_SYSTEM); // ΣΥΣΤΗΜΑ
    wire_tile(objects.________________, SCREEN_ID_WIFI);  // ΔΙΚΤΥΟ

    add_menu_hit(objects.water);
    add_menu_hit(objects.hp);
    add_menu_hit(objects.out);
    add_menu_hit(objects.boiler);
    add_menu_hit(objects.system);
    add_menu_hit(objects.wifi);

    also_wire_menu_label(objects.obj12);
    also_wire_menu_label(objects.obj17);
    also_wire_menu_label(objects.obj22);
    also_wire_menu_label(objects.obj27);
    also_wire_menu_label(objects.obj32);
    also_wire_menu_label(objects.obj37);

    s_on_hub = true;
    s_busy = false;
    s_screen_id = SCREEN_ID_MAIN;
    Serial.println("[nav] tiles + Μενού (OVER slide, press-to-go)");
}

bool panel_nav_on_hub()
{
    return s_on_hub;
}

bool panel_nav_busy()
{
    return s_busy;
}

int panel_nav_screen_id()
{
    return s_screen_id;
}
