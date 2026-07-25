#include "panel_nav.h"

#include <Arduino.h>
#include <string.h>

#include <esp_heap_caps.h>
#include <lvgl.h>

#include "ui/images.h"
#include "ui/screens.h"
#include "ui/ui.h"

namespace {

constexpr uint32_t k_anim_ms = 300;
bool s_on_hub = true;
bool s_busy = false;
int s_screen_id = SCREEN_ID_MAIN;

lv_img_dsc_t s_img8_ram;
lv_img_dsc_t s_img10_ram;
uint8_t *s_img8_buf = nullptr;
uint8_t *s_img10_buf = nullptr;

// Reusable snapshot buffer in PSRAM (800×480 RGB565 ≈ 768 KB)
lv_img_dsc_t s_snap_dsc;
uint8_t *s_snap_buf = nullptr;
uint32_t s_snap_cap = 0;
lv_obj_t *s_slide_img = nullptr;

bool promote_to_psram(const lv_img_dsc_t *src, lv_img_dsc_t *dst, uint8_t **buf_out)
{
    if (!src || !src->data || src->data_size == 0) {
        return false;
    }
    uint8_t *buf = static_cast<uint8_t *>(
        heap_caps_malloc(src->data_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!buf) {
        return false;
    }
    memcpy(buf, src->data, src->data_size);
    *dst = *src;
    dst->data = buf;
    *buf_out = buf;
    return true;
}

void apply_bg(lv_obj_t *screen, const lv_img_dsc_t *img)
{
    if (screen && img) {
        lv_obj_set_style_bg_img_src(screen, img, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

bool ensure_snap_buf(uint32_t need)
{
    if (s_snap_buf && s_snap_cap >= need) {
        return true;
    }
    if (s_snap_buf) {
        heap_caps_free(s_snap_buf);
        s_snap_buf = nullptr;
        s_snap_cap = 0;
    }
    s_snap_buf = static_cast<uint8_t *>(
        heap_caps_malloc(need, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!s_snap_buf) {
        return false;
    }
    s_snap_cap = need;
    return true;
}

void anim_x_cb(void *var, int32_t v)
{
    lv_obj_set_x(static_cast<lv_obj_t *>(var), (lv_coord_t)v);
}

void slide_ready_cb(lv_anim_t *a)
{
    lv_obj_t *img = static_cast<lv_obj_t *>(a->var);
    if (img) {
        lv_obj_del(img);
    }
    if (s_slide_img == img) {
        s_slide_img = nullptr;
    }
    s_busy = false;
}

/**
 * Avoid lv_scr_load_anim (mid-hitch on RGB direct-mode):
 * snapshot old → instant load new → slide snapshot off.
 * Snapshot buffer lives in PSRAM (not LVGL malloc).
 */
void go_screen(enum ScreensEnum id, int32_t slide_end_x)
{
    if (s_busy || id == (ScreensEnum)s_screen_id) {
        return;
    }

    lv_obj_t *from = lv_scr_act();
    lv_obj_t *to = ((lv_obj_t **)&objects)[id - 1];
    if (!from || !to) {
        return;
    }

    s_busy = true;
    s_on_hub = (id == SCREEN_ID_MAIN);
    s_screen_id = (int)id;

    const uint32_t need = lv_snapshot_buf_size_needed(from, LV_IMG_CF_TRUE_COLOR);
    bool have_snap = false;
    if (need > 0 && ensure_snap_buf(need)) {
        have_snap = (lv_snapshot_take_to_buf(from, LV_IMG_CF_TRUE_COLOR, &s_snap_dsc, s_snap_buf, s_snap_cap) ==
                     LV_RES_OK);
    }

    loadScreenAnim(id, LV_SCR_LOAD_ANIM_NONE, 0);

    if (!have_snap) {
        Serial.println("[nav] no snapshot — instant");
        s_busy = false;
        return;
    }

    lv_obj_t *img = lv_img_create(to);
    lv_img_set_src(img, &s_snap_dsc);
    lv_obj_set_pos(img, 0, 0);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(img, LV_OBJ_FLAG_FLOATING);
    lv_obj_move_foreground(img);
    s_slide_img = img;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_values(&a, 0, slide_end_x);
    lv_anim_set_time(&a, k_anim_ms);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, slide_ready_cb);
    lv_anim_start(&a);
}

void on_tile(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) {
        return;
    }
    const auto id = (ScreensEnum)(intptr_t)lv_event_get_user_data(e);
    go_screen(id, -((int32_t)LV_HOR_RES));
}

void on_menu(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_PRESSED) {
        return;
    }
    go_screen(SCREEN_ID_MAIN, (int32_t)LV_HOR_RES);
}

void wire_tile(lv_obj_t *btn, ScreensEnum id)
{
    if (!btn) {
        return;
    }
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, on_tile, LV_EVENT_PRESSED, (void *)(intptr_t)id);
}

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
    const bool ok8 = promote_to_psram(&img_8, &s_img8_ram, &s_img8_buf);
    const bool ok10 = promote_to_psram(&img_10, &s_img10_ram, &s_img10_buf);
    if (ok8 && ok10) {
        apply_bg(objects.main, &s_img8_ram);
        apply_bg(objects.water, &s_img10_ram);
        apply_bg(objects.hp, &s_img10_ram);
        apply_bg(objects.out, &s_img10_ram);
        apply_bg(objects.boiler, &s_img10_ram);
        apply_bg(objects.system, &s_img10_ram);
        apply_bg(objects.wifi, &s_img10_ram);
        Serial.println("[nav] bg → PSRAM");
    }

    // Pre-alloc snapshot buffer so first tap isn't a big alloc hitch
    const uint32_t snap_need = (uint32_t)LV_HOR_RES * (uint32_t)LV_VER_RES * sizeof(lv_color_t);
    if (ensure_snap_buf(snap_need)) {
        Serial.printf("[nav] snapshot buf %u B PSRAM\n", (unsigned)snap_need);
    } else {
        Serial.println("[nav] snapshot buf FAIL");
    }

    wire_tile(objects.____, SCREEN_ID_WATER);
    wire_tile(objects._________________, SCREEN_ID_HP);
    wire_tile(objects.______________, SCREEN_ID_OUT);
    wire_tile(objects._______________, SCREEN_ID_BOILER);
    wire_tile(objects.__________________, SCREEN_ID_SYSTEM);
    wire_tile(objects.________________, SCREEN_ID_WIFI);

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
    Serial.println("[nav] snapshot-slide ready");
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
