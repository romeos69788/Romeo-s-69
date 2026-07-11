/*
 * romeos-display-v2 clean runtime:
 * - Uses the new SquareLine export as the single source of UI truth
 * - Keeps only board/LVGL driver stack from existing project
 * - No legacy extra screens or old navigation logic
 */

#include <Arduino.h>
#include <Preferences.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <esp_heap_caps.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "lvgl_v8_port.h"
#include "romeos_display_link.h"
#include "romeos_ntp.h"
#include "romeos_room_temp_sensor.h"
#include "romeos_wifi_nvs.h"
#include "squareline/ui.h"

using namespace esp_panel::board;

namespace {

float g_setpoint_c = 24.0f;
#ifndef ROMEOS_ALARM_BUZZER_PIN
#define ROMEOS_ALARM_BUZZER_PIN 17
#endif

enum class EditField : uint8_t { None, Hours, Minutes, AmPm, Weekday, Day, Month, Year };
EditField s_edit = EditField::None;
bool s_blink_on = true;

uint8_t s_h24 = 19;
uint8_t s_min = 0;
uint8_t s_sec = 0;
uint8_t s_weekday = 1; /* 0=SUN ... 6=SAT */
uint8_t s_day = 15;
uint8_t s_month = 10;
uint16_t s_year = 2026;

Preferences s_prefs;
static constexpr const char *k_ns = "v2clk";
uint8_t s_sec_since_save = 0;

/* Runtime RGB565 images converted from SquareLine TRUE_COLOR_ALPHA assets. */
lv_img_dsc_t s_bg_rgb565 {};
lv_img_dsc_t s_wifi_rgb565 {};
uint8_t *s_bg_rgb565_data = nullptr;
uint8_t *s_wifi_rgb565_data = nullptr;
bool s_rgb565_images_ready = false;
/* Arc / knob: same TRUE_COLOR_ALPHA→RGB565 as wallpaper (SquareLine may still emit 4 Bpp in src/squareline/images). */
lv_img_dsc_t s_arc_hot_rgb565 {};
lv_img_dsc_t s_arc_col_rgb565 {};
lv_img_dsc_t s_knob_rgb565 {};
uint8_t *s_arc_hot_rgb565_data = nullptr;
uint8_t *s_arc_col_rgb565_data = nullptr;
uint8_t *s_knob_rgb565_data = nullptr;
bool s_arc_assets_conv_ok = false;
bool s_buzzer_on = false;
uint32_t s_buzzer_until_ms = 0;
int s_last_room_tenths = 0x7FFF;
bool s_alarm_demo_active = false;
bool s_alarm_blink_on = true;
static unsigned s_thermostat_strip_btn_count;

struct AlarmUiFx {
    lv_obj_t *unused = nullptr;
} s_alarm_fx {};

struct HomeWifiAnim {
    lv_obj_t *cont = nullptr;
    lv_obj_t *dot = nullptr;
    lv_obj_t *arc[3] = {nullptr, nullptr, nullptr};
    uint8_t level = 0;
    int8_t dir = 1;
};
HomeWifiAnim s_home_wifi {};
/* Last Wi-Fi widget visual applied — skip redundant style writes to cut invalidation/flicker. */
static uint8_t s_home_wifi_vis_level = 0xFF;
static bool s_home_wifi_vis_conn = false;

struct HubWifiAnim {
    lv_obj_t *cont = nullptr;
    lv_obj_t *dot = nullptr;
    lv_obj_t *arc[3] = {nullptr, nullptr, nullptr};
    uint8_t level = 0;
    int8_t dir = 1;
};
HubWifiAnim s_hub_wifi {};
static uint8_t s_hub_wifi_vis_level = 0xFF;
static bool s_hub_wifi_vis_conn = false;

/*
 * Day/night strip (5 Ikones): ONE clip + ONE lv_img. Clip position/size and img alignment are set once in
 * build_day_phase_image(). sync_day_phase_image() only decodes the slot PNG to RGB565 and lv_img_set_src —
 * it does not move the clip, resize widgets, or change zoom except re-applying the same pivot/zoom after set_src.
 * Regenerate assets with scripts/gen_day_phase_assets.py; all five slots must match k_day_photo_asset_* px.
 */
static constexpr lv_coord_t k_day_photo_clip_x = 530;
static constexpr lv_coord_t k_day_photo_clip_y = 55;
static constexpr lv_coord_t k_day_photo_clip_w = 250;
static constexpr lv_coord_t k_day_photo_clip_h = 259;
static constexpr int k_day_photo_asset_w = 250;
static constexpr int k_day_photo_asset_h = 259;

lv_obj_t *s_day_phase_clip = nullptr;
lv_obj_t *s_day_phase_img = nullptr;
lv_img_dsc_t s_day_phase_rgb565 {};
uint8_t *s_day_phase_rgb565_data = nullptr;
static uint8_t s_day_phase_slot = 0xFF; /* 0..4 */

static const lv_img_dsc_t *const k_day_phase_src_alpha[5] = {
    &ui_img_day_p0,
    &ui_img_day_p1,
    &ui_img_day_p2,
    &ui_img_day_p3,
    &ui_img_day_p4,
};

/** Placeholder full-screen panels (HEATING / LIGHTS / HEAT PUMP / BLINDS) — same size as panel. */
static constexpr lv_coord_t k_panel_w = 800;
static constexpr lv_coord_t k_panel_h = 480;
static lv_obj_t *s_sub_placeholder_scr[4] = {};

bool convert_true_color_alpha_to_rgb565(const lv_img_dsc_t &src, lv_img_dsc_t &dst, uint8_t *&dst_data);

/** Hour bands from PNG names in squareline-arc-import/5 Ikones. */
uint8_t day_phase_slot_from_h24(uint8_t h24)
{
    if (h24 >= 23u || h24 < 6u) {
        return 0u; /* 23.00-6.00 */
    }
    if (h24 < 8u) {
        return 1u; /* 6.00-8.00 */
    }
    if (h24 < 17u) {
        return 2u; /* 8.00-17.00 */
    }
    if (h24 < 20u) {
        return 3u; /* 17.00-20.00 */
    }
    return 4u; /* 20.00-23.00 */
}

void sync_day_phase_image()
{
    if (!s_day_phase_img || !s_rgb565_images_ready) {
        return;
    }
    const uint8_t slot = day_phase_slot_from_h24(s_h24);
    if (slot == s_day_phase_slot && s_day_phase_rgb565.data != nullptr) {
        return;
    }

    const lv_img_dsc_t *const slot_src = k_day_phase_src_alpha[slot];
    /* Aliased day slots (same 800x480 AB) must not be decoded into a 250x259 clip — wrong stride → stable vertical banding. */
    if (slot_src->header.w != k_day_photo_asset_w || slot_src->header.h != k_day_photo_asset_h) {
        if (s_day_phase_rgb565_data != nullptr) {
            heap_caps_free(s_day_phase_rgb565_data);
            s_day_phase_rgb565_data = nullptr;
        }
        s_day_phase_rgb565 = {};
        s_day_phase_slot = slot;
        lv_obj_add_flag(s_day_phase_img, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    heap_caps_free(s_day_phase_rgb565_data);
    s_day_phase_rgb565_data = nullptr;
    s_day_phase_rgb565 = {};

    if (!convert_true_color_alpha_to_rgb565(*k_day_phase_src_alpha[slot], s_day_phase_rgb565, s_day_phase_rgb565_data)) {
        s_day_phase_slot = 0xFF;
        return;
    }
    /* Locked layout: clip/img pos+size fixed in build_day_phase_image — this path only swaps pixel buffer (set_src). */
    if (s_day_phase_rgb565.header.w != k_day_photo_asset_w || s_day_phase_rgb565.header.h != k_day_photo_asset_h) {
        Serial.printf(
            "[UI] day phase slot %u decoded %ux%u (expected %dx%d)\n",
            static_cast<unsigned>(slot),
            static_cast<unsigned>(s_day_phase_rgb565.header.w),
            static_cast<unsigned>(s_day_phase_rgb565.header.h),
            k_day_photo_asset_w,
            k_day_photo_asset_h);
        heap_caps_free(s_day_phase_rgb565_data);
        s_day_phase_rgb565_data = nullptr;
        s_day_phase_rgb565 = {};
        s_day_phase_slot = 0xFF;
        lv_obj_add_flag(s_day_phase_img, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    s_day_phase_slot = slot;
    lv_img_set_src(s_day_phase_img, &s_day_phase_rgb565);
    /* Re-assert pivot/zoom after set_src so hour-band swaps never pick up stale transform from LVGL. */
    lv_img_set_pivot(s_day_phase_img, k_day_photo_asset_w / 2, k_day_photo_asset_h / 2);
    lv_img_set_zoom(s_day_phase_img, 256);
    lv_obj_clear_flag(s_day_phase_img, LV_OBJ_FLAG_HIDDEN);
}

void build_day_phase_image()
{
    if (!ui_THERMOSTAT_1) {
        return;
    }
    for (unsigned i = 0; i < 5; ++i) {
        const lv_img_dsc_t *d = k_day_phase_src_alpha[i];
        if (d->header.w != k_day_photo_asset_w || d->header.h != k_day_photo_asset_h) {
            Serial.printf(
                "[UI] ui_img_day_p%u is %ux%u (all slots must be %dx%d)\n",
                i,
                static_cast<unsigned>(d->header.w),
                static_cast<unsigned>(d->header.h),
                k_day_photo_asset_w,
                k_day_photo_asset_h);
        }
    }

    s_day_phase_clip = lv_obj_create(ui_THERMOSTAT_1);
    lv_obj_remove_style_all(s_day_phase_clip);
    lv_obj_set_size(s_day_phase_clip, k_day_photo_clip_w, k_day_photo_clip_h);
    lv_obj_set_pos(s_day_phase_clip, k_day_photo_clip_x, k_day_photo_clip_y);
    lv_obj_set_style_bg_opa(s_day_phase_clip, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_day_phase_clip, 0, LV_PART_MAIN);
    /* Default: children clipped to parent (do not set LV_OBJ_FLAG_OVERFLOW_VISIBLE). */
    lv_obj_clear_flag(s_day_phase_clip, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_day_phase_clip, LV_OBJ_FLAG_CLICKABLE);

    s_day_phase_img = lv_img_create(s_day_phase_clip);
    lv_obj_add_flag(s_day_phase_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_center(s_day_phase_img);
    /* Asset size matches clip — native scale (256 = 100% in LVGL 8). */
    lv_img_set_zoom(s_day_phase_img, 256);
    lv_img_set_pivot(s_day_phase_img, k_day_photo_asset_w / 2, k_day_photo_asset_h / 2);
    lv_obj_clear_flag(s_day_phase_img, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_day_phase_img, LV_OBJ_FLAG_CLICKABLE);

    if (ui_Image3) {
        lv_obj_move_to_index(s_day_phase_clip, (int32_t)lv_obj_get_index(ui_Image3));
    }
    sync_day_phase_image();
}

struct TempArcVisual {
    lv_obj_t *cont = nullptr;
    lv_obj_t *ring_img = nullptr;
    lv_obj_t *cover_arc = nullptr;
    lv_obj_t *dot = nullptr;
};
TempArcVisual s_temp_arc {};
/** SquareLine TH_A export: SEC_OUT (outer), PRIMARY (thick), SEC_IN (inner) — one knob value, +/- moves setpoint. */
static lv_obj_t *s_arc_sec_out = nullptr;
static lv_obj_t *s_arc_primary = nullptr;
static lv_obj_t *s_arc_sec_in = nullptr;

static constexpr int k_setpoint_min_c = 5;
static constexpr int k_setpoint_max_c = 35;
static constexpr int k_arc_center_x = -24;
static constexpr int k_arc_center_y = -65;
static constexpr int k_arc_size_px = 252;
static constexpr int k_arc_thickness_px = 14;
static constexpr int k_arc_radius_px = (k_arc_size_px / 2) - (k_arc_thickness_px / 2);
static constexpr int k_arc_canvas_half = k_arc_size_px / 2;
static constexpr int k_arc_deg_start = 135; /* left-bottom (-) */
static constexpr int k_arc_deg_end = 405;   /* right-bottom (+) */

int16_t get_setpoint_x10_for_link()
{
    int tenths = static_cast<int>(g_setpoint_c * 10.0f + (g_setpoint_c >= 0.0f ? 0.5f : -0.5f));
    tenths = std::clamp(tenths, k_setpoint_min_c * 10, k_setpoint_max_c * 10);
    return static_cast<int16_t>(tenths);
}

void sync_triple_arcs_from_setpoint();
void update_temp_arc_visual()
{
    sync_triple_arcs_from_setpoint();
}

void build_temp_arc_visual()
{
    /* Kept for setup order; real work in build_triple_setpoint_arcs(). */
}

void sync_triple_arcs_from_setpoint()
{
    if (!s_arc_primary || !s_arc_sec_out || !s_arc_sec_in) {
        return;
    }
    const int v = std::clamp(static_cast<int>(g_setpoint_c + 0.5f), k_setpoint_min_c, k_setpoint_max_c);
    lv_arc_set_value(s_arc_sec_out, v);
    lv_arc_set_value(s_arc_primary, v);
    lv_arc_set_value(s_arc_sec_in, v);
}

static void style_arc_common_span(lv_obj_t *a)
{
    lv_arc_set_mode(a, LV_ARC_MODE_NORMAL);
    lv_arc_set_bg_angles(a, 135, 405);
    lv_arc_set_range(a, k_setpoint_min_c, k_setpoint_max_c);
}

static const lv_img_dsc_t *arc_hot_img()
{
    return s_arc_assets_conv_ok ? static_cast<const lv_img_dsc_t *>(&s_arc_hot_rgb565) : &ui_img_hot_png;
}
static const lv_img_dsc_t *arc_col_img()
{
    return s_arc_assets_conv_ok ? static_cast<const lv_img_dsc_t *>(&s_arc_col_rgb565) : &ui_img_col_png;
}
static const lv_img_dsc_t *arc_knob_img()
{
    return s_arc_assets_conv_ok ? static_cast<const lv_img_dsc_t *>(&s_knob_rgb565) : &ui_img_chatgpt1_32x32_png;
}

/**
 * Runtime clone of `squareline-arc-import/TH_A/.../ui_Screen1.c` arcs on `ui_THERMOSTAT_1`
 * (export typo PRYMARY → s_arc_primary). Z-order: arcs just above screen BG, below labels.
 */
void build_triple_setpoint_arcs()
{
    if (!ui_THERMOSTAT_1 || s_arc_primary) {
        return;
    }

    s_arc_sec_out = lv_arc_create(ui_THERMOSTAT_1);
    lv_obj_set_size(s_arc_sec_out, 275, 275);
    lv_obj_set_pos(s_arc_sec_out, -21, -43);
    lv_obj_set_align(s_arc_sec_out, LV_ALIGN_CENTER);
    style_arc_common_span(s_arc_sec_out);
    lv_obj_set_style_bg_opa(s_arc_sec_out, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_sec_out, 2, LV_PART_MAIN);
    lv_obj_set_style_arc_img_src(s_arc_sec_out, arc_hot_img(), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_sec_out, 2, LV_PART_INDICATOR);
    lv_obj_set_style_arc_img_src(s_arc_sec_out, arc_col_img(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_arc_sec_out, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_width(s_arc_sec_out, 0, LV_PART_KNOB);
    lv_obj_set_style_border_opa(s_arc_sec_out, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(s_arc_sec_out, 0, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(s_arc_sec_out, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_outline_width(s_arc_sec_out, 0, LV_PART_KNOB);
    lv_obj_set_style_outline_opa(s_arc_sec_out, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(s_arc_sec_out, LV_OBJ_FLAG_CLICKABLE);

    s_arc_primary = lv_arc_create(ui_THERMOSTAT_1);
    lv_obj_set_size(s_arc_primary, 250, 250);
    lv_obj_set_pos(s_arc_primary, -21, -43);
    lv_obj_set_align(s_arc_primary, LV_ALIGN_CENTER);
    style_arc_common_span(s_arc_primary);
    lv_obj_set_style_border_opa(s_arc_primary, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(s_arc_primary, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_primary, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_img_src(s_arc_primary, arc_hot_img(), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_primary, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_img_src(s_arc_primary, arc_col_img(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_arc_primary, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_bg_img_src(s_arc_primary, arc_knob_img(), LV_PART_KNOB);
    lv_obj_set_style_pad_left(s_arc_primary, 8, LV_PART_KNOB);
    lv_obj_set_style_pad_right(s_arc_primary, 8, LV_PART_KNOB);
    lv_obj_set_style_pad_top(s_arc_primary, 8, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(s_arc_primary, 8, LV_PART_KNOB);
    lv_obj_set_style_border_width(s_arc_primary, 0, LV_PART_KNOB);
    lv_obj_set_style_border_opa(s_arc_primary, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(s_arc_primary, 0, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(s_arc_primary, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_outline_width(s_arc_primary, 0, LV_PART_KNOB);
    lv_obj_set_style_outline_opa(s_arc_primary, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(s_arc_primary, LV_OBJ_FLAG_CLICKABLE);

    s_arc_sec_in = lv_arc_create(ui_THERMOSTAT_1);
    lv_obj_set_size(s_arc_sec_in, 205, 205);
    lv_obj_set_pos(s_arc_sec_in, -22, -42);
    lv_obj_set_align(s_arc_sec_in, LV_ALIGN_CENTER);
    style_arc_common_span(s_arc_sec_in);
    lv_obj_set_style_bg_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_sec_in, 2, LV_PART_MAIN);
    lv_obj_set_style_arc_img_src(s_arc_sec_in, arc_hot_img(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_border_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_shadow_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_arc_sec_in, 2, LV_PART_INDICATOR);
    lv_obj_set_style_arc_img_src(s_arc_sec_in, arc_col_img(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_width(s_arc_sec_in, 0, LV_PART_KNOB);
    lv_obj_set_style_border_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(s_arc_sec_in, 0, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_outline_width(s_arc_sec_in, 0, LV_PART_KNOB);
    lv_obj_set_style_outline_opa(s_arc_sec_in, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(s_arc_sec_in, LV_OBJ_FLAG_CLICKABLE);

    /* Bottom of child list = drawn over screen BG, under first label (setpoint ring). */
    lv_obj_move_to_index(s_arc_sec_out, 0);
    lv_obj_move_to_index(s_arc_primary, 1);
    lv_obj_move_to_index(s_arc_sec_in, 2);

    sync_triple_arcs_from_setpoint();
}

int16_t get_room_x10_for_link()
{
    if (!std::isfinite(g_room_temp_c)) {
        return ROMEOS_ROOM_FROM_DISPLAY_INVALID;
    }
    int tenths = static_cast<int>(g_room_temp_c * 10.0f + (g_room_temp_c >= 0.0f ? 0.5f : -0.5f));
    tenths = std::clamp(tenths, 0, 450);
    return static_cast<int16_t>(tenths);
}

void on_mb_data_for_link(const romeos_mb_to_display_v1_t *p)
{
    LV_UNUSED(p);
}

void apply_home_wifi_visual(uint8_t level, bool connected)
{
    if (!s_home_wifi.dot) {
        return;
    }
    if (level == s_home_wifi_vis_level && connected == s_home_wifi_vis_conn) {
        return;
    }
    s_home_wifi_vis_level = level;
    s_home_wifi_vis_conn = connected;

    const uint32_t rgb = connected ? 0x4FC3F7 : 0xFF7043;
    const lv_color_t c = lv_color_hex(rgb);
    lv_obj_set_style_bg_color(s_home_wifi.dot, c, LV_PART_MAIN);
    for (int i = 0; i < 3; ++i) {
        if (!s_home_wifi.arc[i]) {
            continue;
        }
        const bool on = level > static_cast<uint8_t>(i);
        lv_obj_set_style_arc_color(s_home_wifi.arc[i], c, LV_PART_MAIN);
        /* Fully hide inactive arcs (no semi-transparent blend — avoids theme tint / blue artifacts). */
        lv_obj_set_style_arc_opa(s_home_wifi.arc[i], on ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    }
}

void home_wifi_anim_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    const bool home_ok = romeos_display_link_home_wifi_assoc();
    if (!home_ok) {
        s_home_wifi.level = 0;
        s_home_wifi.dir = 1;
        apply_home_wifi_visual(0, false);
        return;
    }

    int next = static_cast<int>(s_home_wifi.level) + static_cast<int>(s_home_wifi.dir);
    if (next > 3) {
        next = 3;
        s_home_wifi.dir = -1;
    } else if (next < 0) {
        next = 0;
        s_home_wifi.dir = 1;
    }
    s_home_wifi.level = static_cast<uint8_t>(next);
    apply_home_wifi_visual(s_home_wifi.level, true);
}

void apply_hub_wifi_visual(uint8_t level, bool connected)
{
    if (!s_hub_wifi.dot) {
        return;
    }
    if (level == s_hub_wifi_vis_level && connected == s_hub_wifi_vis_conn) {
        return;
    }
    s_hub_wifi_vis_level = level;
    s_hub_wifi_vis_conn = connected;

    const uint32_t rgb = connected ? 0xFFC107 : 0xFF7043; /* HUB: amber/orange when link is alive */
    const lv_color_t c = lv_color_hex(rgb);
    lv_obj_set_style_bg_color(s_hub_wifi.dot, c, LV_PART_MAIN);
    for (int i = 0; i < 3; ++i) {
        if (!s_hub_wifi.arc[i]) {
            continue;
        }
        const bool on = level > static_cast<uint8_t>(i);
        lv_obj_set_style_arc_color(s_hub_wifi.arc[i], c, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(s_hub_wifi.arc[i], on ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    }
}

void hub_wifi_anim_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    const bool hub_ok = romeos_display_link_is_connected();
    if (!hub_ok) {
        s_hub_wifi.level = 0;
        s_hub_wifi.dir = 1;
        apply_hub_wifi_visual(0, false);
        return;
    }

    int next = static_cast<int>(s_hub_wifi.level) + static_cast<int>(s_hub_wifi.dir);
    if (next > 3) {
        next = 3;
        s_hub_wifi.dir = -1;
    } else if (next < 0) {
        next = 0;
        s_hub_wifi.dir = 1;
    }
    s_hub_wifi.level = static_cast<uint8_t>(next);
    apply_hub_wifi_visual(s_hub_wifi.level, true);
}

/* lv_obj enables CLICK_FOCUSABLE by default: each tap sends LV_EVENT_FOCUSED (lv_indev.c → indev_click_focus).
   With SCROLL_ON_FOCUS that scrolls the hierarchy (lv_obj.c) and causes broad invalidation — looks like a
   soft frame on the button and nearby labels. Clearing CLICK_FOCUSABLE stops that path for touch UI. */
static void strip_one_button_pressed_focus_visuals(lv_obj_t *btn)
{
    if (!btn) {
        return;
    }
    static const lv_state_t k_states[] = {
        LV_STATE_PRESSED,
        LV_STATE_FOCUSED,
        LV_STATE_PRESSED | LV_STATE_FOCUSED,
        LV_STATE_FOCUS_KEY,
    };
    static const lv_part_t k_parts[] = {LV_PART_MAIN, LV_PART_INDICATOR};
    /* LVGL resolves props by scanning styles in array order; first style whose state matches the object wins.
       Default theme registers PRESSED/FOCUS_KEY via lv_obj_add_style *before* any later local styles, so
       lv_obj_set_style_* alone never overrides theme pressed/grow/outline — remove those slots first. */
    for (const lv_part_t part : k_parts) {
        for (const lv_state_t st : k_states) {
            const lv_style_selector_t sel =
                static_cast<lv_style_selector_t>(static_cast<uint32_t>(part) | static_cast<uint32_t>(st));
            lv_obj_remove_style(btn, nullptr, sel);
        }
    }
    for (const lv_part_t part : k_parts) {
        for (const lv_state_t st : k_states) {
            const lv_style_selector_t sel =
                static_cast<lv_style_selector_t>(static_cast<uint32_t>(part) | static_cast<uint32_t>(st));
            lv_obj_set_style_border_width(btn, 0, sel);
            lv_obj_set_style_border_opa(btn, LV_OPA_TRANSP, sel);
            lv_obj_set_style_outline_width(btn, 0, sel);
            lv_obj_set_style_outline_opa(btn, LV_OPA_TRANSP, sel);
            lv_obj_set_style_outline_pad(btn, 0, sel);
            lv_obj_set_style_shadow_width(btn, 0, sel);
            lv_obj_set_style_shadow_opa(btn, LV_OPA_TRANSP, sel);
            lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, sel);
            /* Theme pressed: color filter + transform grow (see lv_theme_default.c for lv_btn). */
            lv_obj_set_style_color_filter_opa(btn, LV_OPA_TRANSP, sel);
            lv_obj_set_style_transform_width(btn, 0, sel);
            lv_obj_set_style_transform_height(btn, 0, sel);
        }
    }
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
}

/** Όλα τα lv_btn της THERMOSTAT: αφαίρεση θεματικών PRESSED/FOCUS + ρητό «καθαρό» local στυλ. */
unsigned strip_thermostat_button_pressed_focus_visuals()
{
    lv_obj_t *btns[] = {
        ui_HEATING,
        ui_LIGHTS,
        ui_HEAT_PUMP,
        ui_BLINDS,
        ui_ALARM,
        ui_Button_MINUS,
        ui_Button_PLUS,
        ui_Button_CLOCK_LEFT,
        ui_Button_CLOCK_RIGHT,
        ui_Button11,
        ui_Button12,
        ui_Button13,
        ui_Button14,
        ui_Button15,
        ui_Button16,
        ui_Button17,
    };
    unsigned n = 0;
    lv_group_t *const g = lv_group_get_default();
    for (lv_obj_t *btn : btns) {
        if (btn) {
            ++n;
        }
        strip_one_button_pressed_focus_visuals(btn);
        if (btn) {
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            if (g) {
                lv_group_remove_obj(btn);
            }
        }
    }
    /* Setpoint readout: same focus/press chrome + no click-to-focus (avoids sibling invalidation). */
    for (lv_obj_t *lab : {ui_Label1, ui_Label2}) {
        strip_one_button_pressed_focus_visuals(lab);
        if (lab && g) {
            lv_group_remove_obj(lab);
        }
    }
    if (ui_THERMOSTAT_1) {
        lv_obj_clear_flag(ui_THERMOSTAT_1, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_clear_flag(ui_THERMOSTAT_1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }
    return n;
}

static void on_thermostat_screen_loaded_strip_styles(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SCREEN_LOADED) {
        return;
    }
    strip_thermostat_button_pressed_focus_visuals();
}

void build_home_wifi_anim()
{
    if (!ui_THERMOSTAT_1) {
        return;
    }
    if (ui_Image1) {
        lv_obj_add_flag(ui_Image1, LV_OBJ_FLAG_HIDDEN);
    }

    s_home_wifi.cont = lv_obj_create(ui_THERMOSTAT_1);
    lv_obj_set_size(s_home_wifi.cont, 92, 84);
    lv_obj_align(s_home_wifi.cont, LV_ALIGN_CENTER, -343, -213);
    lv_obj_set_style_bg_opa(s_home_wifi.cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_home_wifi.cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(s_home_wifi.cont, LV_OBJ_FLAG_SCROLLABLE);

    static const int k_w[3] = {28, 42, 56};
    for (int i = 0; i < 3; ++i) {
        lv_obj_t *a = lv_arc_create(s_home_wifi.cont);
        lv_obj_remove_style_all(a);
        lv_obj_set_size(a, k_w[i], k_w[i]);
        lv_obj_align(a, LV_ALIGN_CENTER, 0, 6);
        lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_arc_set_bg_angles(a, 210, 330);
        lv_arc_set_range(a, 0, 100);
        lv_arc_set_value(a, 0);
        lv_obj_set_style_arc_width(a, 3, LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(a, true, LV_PART_MAIN);
        /* Strip default theme indicator/knob (often blue) — we only draw the bg arc. */
        lv_obj_set_style_arc_width(a, 0, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(a, lv_color_hex(0x101820), LV_PART_INDICATOR);
        lv_obj_set_style_opa(a, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_width(a, 0, LV_PART_KNOB);
        lv_obj_set_style_height(a, 0, LV_PART_KNOB);
        lv_obj_clear_flag(a, LV_OBJ_FLAG_CLICKABLE);
        s_home_wifi.arc[i] = a;
    }

    s_home_wifi.dot = lv_obj_create(s_home_wifi.cont);
    lv_obj_set_size(s_home_wifi.dot, 8, 8);
    lv_obj_set_style_radius(s_home_wifi.dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_home_wifi.dot, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(s_home_wifi.dot, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(s_home_wifi.dot, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_outline_width(s_home_wifi.dot, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(s_home_wifi.dot, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_home_wifi.dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align_to(s_home_wifi.dot, s_home_wifi.arc[0], LV_ALIGN_CENTER, 0, 1);
    lv_obj_clear_flag(s_home_wifi.dot, LV_OBJ_FLAG_CLICKABLE);

    s_home_wifi.level = 0;
    s_home_wifi.dir = 1;
    s_home_wifi_vis_level = 0xFF;
    s_home_wifi_vis_conn = false;
    apply_home_wifi_visual(0, false);
    lv_timer_create(home_wifi_anim_cb, 900, nullptr);
}

void build_hub_wifi_anim()
{
    if (!ui_THERMOSTAT_1) {
        return;
    }
    if (ui_Image3) {
        lv_obj_add_flag(ui_Image3, LV_OBJ_FLAG_HIDDEN);
    }

    s_hub_wifi.cont = lv_obj_create(ui_THERMOSTAT_1);
    lv_obj_set_size(s_hub_wifi.cont, 92, 84);
    lv_obj_align(s_hub_wifi.cont, LV_ALIGN_CENTER, 352, -214);
    lv_obj_set_style_bg_opa(s_hub_wifi.cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_hub_wifi.cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_hub_wifi.cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(s_hub_wifi.cont, LV_OBJ_FLAG_SCROLLABLE);

    static const int k_w[3] = {28, 42, 56};
    for (int i = 0; i < 3; ++i) {
        lv_obj_t *a = lv_arc_create(s_hub_wifi.cont);
        lv_obj_remove_style_all(a);
        lv_obj_set_size(a, k_w[i], k_w[i]);
        lv_obj_align(a, LV_ALIGN_CENTER, 0, 6);
        lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_arc_set_bg_angles(a, 210, 330);
        lv_arc_set_range(a, 0, 100);
        lv_arc_set_value(a, 0);
        lv_obj_set_style_arc_width(a, 3, LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(a, true, LV_PART_MAIN);
        lv_obj_set_style_arc_width(a, 0, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(a, lv_color_hex(0x101820), LV_PART_INDICATOR);
        lv_obj_set_style_opa(a, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_width(a, 0, LV_PART_KNOB);
        lv_obj_set_style_height(a, 0, LV_PART_KNOB);
        lv_obj_clear_flag(a, LV_OBJ_FLAG_CLICKABLE);
        s_hub_wifi.arc[i] = a;
    }

    s_hub_wifi.dot = lv_obj_create(s_hub_wifi.cont);
    lv_obj_set_size(s_hub_wifi.dot, 8, 8);
    lv_obj_set_style_radius(s_hub_wifi.dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_hub_wifi.dot, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(s_hub_wifi.dot, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(s_hub_wifi.dot, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_outline_width(s_hub_wifi.dot, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(s_hub_wifi.dot, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_hub_wifi.dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align_to(s_hub_wifi.dot, s_hub_wifi.arc[0], LV_ALIGN_CENTER, 0, 1);
    lv_obj_clear_flag(s_hub_wifi.dot, LV_OBJ_FLAG_CLICKABLE);

    s_hub_wifi.level = 0;
    s_hub_wifi.dir = 1;
    s_hub_wifi_vis_level = 0xFF;
    s_hub_wifi_vis_conn = false;
    apply_hub_wifi_visual(0, false);
    lv_timer_create(hub_wifi_anim_cb, 900, nullptr);
}

void apply_alarm_visual()
{
    if (!ui_ALARM) {
        return;
    }
    constexpr lv_style_selector_t k_sel = LV_PART_MAIN | LV_STATE_DEFAULT;
    const lv_color_t red = lv_color_hex(0xFF3B30);
    const lv_color_t white = lv_color_hex(0xFFFFFF);

    if (s_alarm_demo_active) {
        lv_obj_set_style_bg_color(ui_ALARM, red, k_sel);
        lv_obj_set_style_bg_opa(ui_ALARM, LV_OPA_30, k_sel);
        lv_obj_set_style_border_color(ui_ALARM, red, k_sel);
        lv_obj_set_style_border_width(ui_ALARM, 2, k_sel);
        lv_obj_set_style_border_opa(ui_ALARM, LV_OPA_80, k_sel);
        if (ui_Label22) {
            lv_obj_set_style_text_color(ui_Label22, red, k_sel);
        }
    } else {
        lv_obj_set_style_bg_opa(ui_ALARM, LV_OPA_TRANSP, k_sel);
        lv_obj_set_style_border_width(ui_ALARM, 0, k_sel);
        lv_obj_set_style_border_opa(ui_ALARM, LV_OPA_TRANSP, k_sel);
        if (ui_Label22) {
            lv_obj_set_style_text_color(ui_Label22, white, k_sel);
        }
    }
}

void alarm_blink_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (!s_alarm_demo_active) {
        return;
    }
    s_alarm_blink_on = !s_alarm_blink_on;
    apply_alarm_visual();
}

void build_alarm_fx()
{
    if (!ui_ALARM) {
        return;
    }
    apply_alarm_visual();
    lv_timer_create(alarm_blink_cb, 380, nullptr);
}

uint8_t days_in_month(uint16_t y, uint8_t m)
{
    static const uint8_t dim[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t d = dim[(m - 1) % 12];
    const bool leap = ((y % 4u) == 0u) && (((y % 100u) != 0u) || ((y % 400u) == 0u));
    if (m == 2 && leap) {
        d = 29;
    }
    return d;
}

/** Μετάβαση 23:59 -> 00:00 — ημέρα/μήνας/έτος και όνομα ημέρας (s_weekday) συμβαδίζουν με το ρολόι. */
void bump_calendar_at_midnight()
{
    s_weekday = static_cast<uint8_t>((static_cast<unsigned>(s_weekday) + 1u) % 7u);
    const uint8_t dim = days_in_month(s_year, s_month);
    if (s_day < dim) {
        s_day = static_cast<uint8_t>(s_day + 1u);
        return;
    }
    s_day = 1;
    if (s_month >= 12u) {
        s_month = 1;
        s_year = (s_year >= 2099u) ? 2000u : static_cast<uint16_t>(s_year + 1u);
    } else {
        s_month = static_cast<uint8_t>(s_month + 1u);
    }
}

void prefs_open_rw()
{
    static bool opened = false;
    if (!opened) {
        (void)s_prefs.begin(k_ns, false);
        opened = true;
    }
}

void save_state()
{
    prefs_open_rw();
    s_prefs.putUChar("h", s_h24);
    s_prefs.putUChar("m", s_min);
    s_prefs.putUChar("s", s_sec);
    s_prefs.putUChar("wd", s_weekday);
    s_prefs.putUChar("d", s_day);
    s_prefs.putUChar("mo", s_month);
    s_prefs.putUShort("y", s_year);
    s_prefs.putFloat("sp", g_setpoint_c);
}

void load_state()
{
    prefs_open_rw();
    s_h24 = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("h", 19), 0, 23));
    s_min = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("m", 0), 0, 59));
    s_sec = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("s", 0), 0, 59));
    s_weekday = static_cast<uint8_t>(s_prefs.getUChar("wd", 1) % 7);
    s_month = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("mo", 10), 1, 12));
    s_year = static_cast<uint16_t>(std::clamp<int>(s_prefs.getUShort("y", 2026), 2000, 2099));
    s_day = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("d", 15), 1, days_in_month(s_year, s_month)));
    g_setpoint_c = std::clamp(s_prefs.getFloat("sp", 24.0f), static_cast<float>(k_setpoint_min_c), static_cast<float>(k_setpoint_max_c));
}

void apply_blink_state()
{
    /*
     * Αναβόσβημα μόνο στο επιλεγμένο πεδίο: εναλλαγή χρώματος κειμένου (όχι TRANSP, όχι outline στο label).
     * Outline στο label έκανε invalidate περιοχή που περιλάμβανε το αόρατο κουμπί από πάνω και
     * «αναβόσβηνε» το πράσινο πλαίσιο του SquareLine.
     */
    const lv_color_t normal = lv_color_hex(0xFFFFFF);
    const lv_color_t dim = lv_color_hex(0x3D4A5C);
    constexpr lv_style_selector_t k_sel = LV_PART_MAIN | LV_STATE_DEFAULT;

    auto paint = [&](lv_obj_t *lbl, EditField fld) {
        if (!lbl) {
            return;
        }
        lv_obj_set_style_text_opa(lbl, LV_OPA_COVER, k_sel);
        lv_obj_set_style_outline_width(lbl, 0, k_sel);
        if (s_edit == EditField::None || fld != s_edit) {
            lv_obj_set_style_text_color(lbl, normal, k_sel);
            lv_obj_invalidate(lbl);
            return;
        }
        lv_obj_set_style_text_color(lbl, s_blink_on ? normal : dim, k_sel);
        lv_obj_invalidate(lbl);
    };

    paint(ui_Label12, EditField::Hours);
    paint(ui_Label16, EditField::Minutes);
    paint(ui_Label17, EditField::AmPm);
    paint(ui_Label4, EditField::Weekday);
    paint(ui_Label6, EditField::Day);
    paint(ui_Label9, EditField::Month);
    paint(ui_Label11, EditField::Year);
}

void clock_blink_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (s_edit == EditField::None) {
        return;
    }
    s_blink_on = !s_blink_on;
    apply_blink_state();
}

void refresh_ui()
{
    char b[16];
    auto label_set_if_changed = [](lv_obj_t *lbl, const char *txt) {
        if (!lbl || !txt) {
            return;
        }
        if (lv_label_get_text(lbl) != nullptr && std::strcmp(lv_label_get_text(lbl), txt) == 0) {
            return;
        }
        lv_label_set_text(lbl, txt);
    };

    if (ui_Label1) {
        const int sp = std::clamp(static_cast<int>(g_setpoint_c + 0.5f), k_setpoint_min_c, k_setpoint_max_c);
        std::snprintf(b, sizeof(b), "%d", sp);
        label_set_if_changed(ui_Label1, b);
    }
    if (ui_Label12) {
        std::snprintf(b, sizeof(b), "%02u", static_cast<unsigned>(s_h24));
        label_set_if_changed(ui_Label12, b);
    }
    if (ui_Label16) {
        std::snprintf(b, sizeof(b), "%02u", static_cast<unsigned>(s_min));
        label_set_if_changed(ui_Label16, b);
    }
    if (ui_Label17) {
        label_set_if_changed(ui_Label17, (s_h24 >= 12) ? "PM" : "AM");
    }
    static const char *const k_wd[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
    if (ui_Label4) {
        label_set_if_changed(ui_Label4, k_wd[s_weekday % 7]);
    }
    if (ui_Label6) {
        std::snprintf(b, sizeof(b), "%02u", static_cast<unsigned>(s_day));
        label_set_if_changed(ui_Label6, b);
    }
    if (ui_Label9) {
        std::snprintf(b, sizeof(b), "%02u", static_cast<unsigned>(s_month));
        label_set_if_changed(ui_Label9, b);
    }
    if (ui_Label11) {
        std::snprintf(b, sizeof(b), "%04u", static_cast<unsigned>(s_year));
        label_set_if_changed(ui_Label11, b);
    }
    if (ui_Label3) {
        if (std::isfinite(g_room_temp_c)) {
            std::snprintf(b, sizeof(b), "%.1f", static_cast<double>(g_room_temp_c));
            label_set_if_changed(ui_Label3, b);
        } else {
            label_set_if_changed(ui_Label3, "?");
        }
    }

    apply_blink_state();
    update_temp_arc_visual();
    sync_day_phase_image();
}

bool convert_true_color_alpha_to_rgb565(const lv_img_dsc_t &src, lv_img_dsc_t &dst, uint8_t *&dst_data)
{
    const uint32_t pixels = static_cast<uint32_t>(src.header.w) * static_cast<uint32_t>(src.header.h);
    const uint32_t need3 = pixels * 3u; /* LV_COLOR_DEPTH 16: RGB565 + A per LV_IMG_CF_TRUE_COLOR_ALPHA */
    const uint32_t need4 = pixels * 4u; /* SquareLine default export often uses LV_COLOR_DEPTH 32 → B,G,R,A bytes */
    const uint32_t dst_bytes = pixels * 2u;
    if (!src.data || pixels == 0u) {
        return false;
    }
    const uint32_t bpp = (src.data_size >= need4) ? 4u : ((src.data_size >= need3) ? 3u : 0u);
    if (bpp == 0u) {
        return false;
    }

    /* Internal + aligned: large PSRAM framebuffers + unaligned src can interact badly with SW blend on RGB direct mode. */
    dst_data = static_cast<uint8_t *>(
        heap_caps_aligned_alloc(16, dst_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    if (!dst_data) {
        dst_data = static_cast<uint8_t *>(heap_caps_malloc(dst_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    }
    if (!dst_data) {
        dst_data = static_cast<uint8_t *>(std::malloc(dst_bytes));
    }
    if (!dst_data) {
        return false;
    }

    if (bpp == 4u) {
        for (uint32_t i = 0, si = 0, di = 0; i < pixels; ++i, si += 4u, di += 2u) {
            const uint8_t b = src.data[si];
            const uint8_t g = src.data[si + 1u];
            const uint8_t r = src.data[si + 2u];
            const lv_color_t c = lv_color_make(r, g, b);
            dst_data[di] = static_cast<uint8_t>(c.full & 0xFFu);
            dst_data[di + 1u] = static_cast<uint8_t>((c.full >> 8) & 0xFFu);
        }
    } else {
        for (uint32_t i = 0, si = 0, di = 0; i < pixels; ++i, si += 3u, di += 2u) {
            dst_data[di] = src.data[si];
            dst_data[di + 1u] = src.data[si + 1u];
        }
    }

    dst.header.always_zero = 0;
    dst.header.w = src.header.w;
    dst.header.h = src.header.h;
    dst.header.cf = LV_IMG_CF_TRUE_COLOR;
    dst.data_size = dst_bytes;
    dst.data = dst_data;
    return true;
}

void try_apply_rgb565_images()
{
    if (!s_rgb565_images_ready) {
        const bool bg_ok = convert_true_color_alpha_to_rgb565(ui_img_thermostat_ab_png, s_bg_rgb565, s_bg_rgb565_data);
        const bool wifi_ok = convert_true_color_alpha_to_rgb565(ui_img_1757177593, s_wifi_rgb565, s_wifi_rgb565_data);
        s_rgb565_images_ready = bg_ok && wifi_ok;
    }
    if (!s_rgb565_images_ready) {
        return;
    }

    if (!s_arc_assets_conv_ok) {
        const bool ho = convert_true_color_alpha_to_rgb565(ui_img_hot_png, s_arc_hot_rgb565, s_arc_hot_rgb565_data);
        const bool co = convert_true_color_alpha_to_rgb565(ui_img_col_png, s_arc_col_rgb565, s_arc_col_rgb565_data);
        const bool kn = convert_true_color_alpha_to_rgb565(ui_img_chatgpt1_32x32_png, s_knob_rgb565, s_knob_rgb565_data);
        s_arc_assets_conv_ok = ho && co && kn;
        if (!s_arc_assets_conv_ok) {
            Serial.println("[UI] arc HOT/COL/knob RGB565 convert failed; arcs keep flash assets");
        }
    }

    lv_obj_set_style_bg_img_src(ui_THERMOSTAT_1, &s_bg_rgb565, LV_PART_MAIN | LV_STATE_DEFAULT);
    if (ui_Image1) {
        lv_img_set_src(ui_Image1, &s_wifi_rgb565);
        lv_obj_set_x(ui_Image1, -343);
        lv_obj_set_y(ui_Image1, -213);
        lv_img_set_zoom(ui_Image1, 300);
        lv_obj_clear_flag(ui_Image1, LV_OBJ_FLAG_HIDDEN);
    }
    /* Right-side Wi-Fi is the custom hub widget; keep SquareLine Image3 hidden (avoids double stack / glitches). */
    if (ui_Image3) {
        lv_obj_add_flag(ui_Image3, LV_OBJ_FLAG_HIDDEN);
    }
}

void on_back_to_main(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (ui_THERMOSTAT_1) {
        lv_scr_load(ui_THERMOSTAT_1);
    }
}

void on_nav_sub_placeholder(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    /* user_data = screen_index + 1 (never nullptr — tag 0 would be ambiguous with HEATING index 0). */
    const uintptr_t tag = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e));
    if (tag < 1u || tag > 4u) {
        return;
    }
    const unsigned ix = static_cast<unsigned>(tag - 1u);
    if (ix >= 4 || !s_sub_placeholder_scr[ix]) {
        return;
    }
    s_edit = EditField::None;
    s_blink_on = true;
    apply_blink_state();
    lv_scr_load(s_sub_placeholder_scr[ix]);
}

void build_placeholder_subscreens()
{
    static const char *const k_titles[4] = {
        "HEATING CONTROL",
        "LIGHTING CONTROL",
        "HEAT PUMP CONTROL",
        "BLINDS CONTROL",
    };
    for (unsigned i = 0; i < 4; ++i) {
        lv_obj_t *scr = lv_obj_create(nullptr);
        s_sub_placeholder_scr[i] = scr;
        lv_obj_set_size(scr, k_panel_w, k_panel_h);
        lv_obj_remove_style_all(scr);
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x1A1F28), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *title = lv_label_create(scr);
        lv_label_set_text(title, k_titles[i]);
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN);
        lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(title, k_panel_w - 40);
        lv_obj_align(title, LV_ALIGN_CENTER, 0, -24);

        lv_obj_t *back = lv_btn_create(scr);
        lv_obj_set_size(back, 112, 48);
        lv_obj_align(back, LV_ALIGN_BOTTOM_RIGHT, -20, -18);
        lv_obj_set_style_bg_color(back, lv_color_hex(0x3D4A5C), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(back, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(back, 8, LV_PART_MAIN);
        lv_obj_set_style_border_width(back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_t *bl = lv_label_create(back);
        lv_label_set_text(bl, "BACK");
        lv_obj_set_style_text_color(bl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(bl, &lv_font_montserrat_20, LV_PART_MAIN);
        lv_obj_center(bl);
        strip_one_button_pressed_focus_visuals(back);
        lv_obj_add_event_cb(back, on_back_to_main, LV_EVENT_CLICKED, nullptr);
    }
}

void on_minus(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    g_setpoint_c = std::max(static_cast<float>(k_setpoint_min_c), g_setpoint_c - 1.0f);
    refresh_ui();
}

void on_plus(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    g_setpoint_c = std::min(static_cast<float>(k_setpoint_max_c), g_setpoint_c + 1.0f);
    refresh_ui();
}

void select_field(EditField f)
{
    s_edit = (s_edit == f) ? EditField::None : f;
    s_blink_on = true;
    apply_blink_state();
}

void on_select_hours(lv_event_t *e)   { if (lv_event_get_code(e) == LV_EVENT_CLICKED) select_field(EditField::Hours); }
void on_select_minutes(lv_event_t *e) { if (lv_event_get_code(e) == LV_EVENT_CLICKED) select_field(EditField::Minutes); }
void on_select_ampm(lv_event_t *e)    { if (lv_event_get_code(e) == LV_EVENT_CLICKED) select_field(EditField::AmPm); }
void on_select_weekday(lv_event_t *e) { if (lv_event_get_code(e) == LV_EVENT_CLICKED) select_field(EditField::Weekday); }
void on_select_day(lv_event_t *e)     { if (lv_event_get_code(e) == LV_EVENT_CLICKED) select_field(EditField::Day); }
void on_select_month(lv_event_t *e)   { if (lv_event_get_code(e) == LV_EVENT_CLICKED) select_field(EditField::Month); }
void on_select_year(lv_event_t *e)    { if (lv_event_get_code(e) == LV_EVENT_CLICKED) select_field(EditField::Year); }

void adjust(bool inc)
{
    switch (s_edit) {
    case EditField::Hours:
        s_h24 = static_cast<uint8_t>((s_h24 + (inc ? 1 : 23)) % 24);
        break;
    case EditField::Minutes:
        s_min = static_cast<uint8_t>((s_min + (inc ? 1 : 59)) % 60);
        break;
    case EditField::AmPm:
        s_h24 = static_cast<uint8_t>((s_h24 + 12) % 24);
        break;
    case EditField::Weekday:
        s_weekday = static_cast<uint8_t>((s_weekday + (inc ? 1 : 6)) % 7);
        break;
    case EditField::Day: {
        const uint8_t maxd = days_in_month(s_year, s_month);
        s_day = inc ? ((s_day >= maxd) ? 1 : s_day + 1) : ((s_day <= 1) ? maxd : s_day - 1);
        break;
    }
    case EditField::Month:
        s_month = inc ? ((s_month >= 12) ? 1 : s_month + 1) : ((s_month <= 1) ? 12 : s_month - 1);
        s_day = std::min<uint8_t>(s_day, days_in_month(s_year, s_month));
        break;
    case EditField::Year:
        s_year = inc ? ((s_year >= 2099) ? 2000 : s_year + 1) : ((s_year <= 2000) ? 2099 : s_year - 1);
        s_day = std::min<uint8_t>(s_day, days_in_month(s_year, s_month));
        break;
    case EditField::None:
    default:
        return;
    }
    /* Manual time/date correction should take effect cleanly from a minute boundary. */
    s_sec = 0;
    s_blink_on = true;
    refresh_ui();
    save_state();
}

void on_arrow_left(lv_event_t *e)  { if (lv_event_get_code(e) == LV_EVENT_CLICKED) adjust(false); }
void on_arrow_right(lv_event_t *e) { if (lv_event_get_code(e) == LV_EVENT_CLICKED) adjust(true); }

void on_alarm(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    s_alarm_demo_active = !s_alarm_demo_active;
    s_alarm_blink_on = true;
    if (ROMEOS_ALARM_BUZZER_PIN >= 0) {
        digitalWrite(ROMEOS_ALARM_BUZZER_PIN, s_alarm_demo_active ? HIGH : LOW);
    }
    apply_alarm_visual();
    Serial.printf("[UI] ALARM demo %s\n", s_alarm_demo_active ? "ON" : "OFF");
}

void bind_ui()
{
    if (ui_Button7) lv_obj_add_event_cb(ui_Button7, on_minus, LV_EVENT_CLICKED, nullptr);
    if (ui_Button8) lv_obj_add_event_cb(ui_Button8, on_plus, LV_EVENT_CLICKED, nullptr);
    if (ui_Button9) lv_obj_add_event_cb(ui_Button9, on_arrow_left, LV_EVENT_CLICKED, nullptr);
    if (ui_Button10) lv_obj_add_event_cb(ui_Button10, on_arrow_right, LV_EVENT_CLICKED, nullptr);
    if (ui_Button11) lv_obj_add_event_cb(ui_Button11, on_select_weekday, LV_EVENT_CLICKED, nullptr);
    if (ui_Button12) lv_obj_add_event_cb(ui_Button12, on_select_day, LV_EVENT_CLICKED, nullptr);
    if (ui_Button13) lv_obj_add_event_cb(ui_Button13, on_select_month, LV_EVENT_CLICKED, nullptr);
    if (ui_Button14) lv_obj_add_event_cb(ui_Button14, on_select_year, LV_EVENT_CLICKED, nullptr);
    if (ui_Button15) lv_obj_add_event_cb(ui_Button15, on_select_hours, LV_EVENT_CLICKED, nullptr);
    if (ui_Button16) lv_obj_add_event_cb(ui_Button16, on_select_minutes, LV_EVENT_CLICKED, nullptr);
    if (ui_Button17) lv_obj_add_event_cb(ui_Button17, on_select_ampm, LV_EVENT_CLICKED, nullptr);
    /* Left column: tag = screen index + 1 — matches k_titles[] in build_placeholder_subscreens(). */
    if (ui_HEATING) {
        lv_obj_add_event_cb(
            ui_HEATING, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(1)));
    }
    if (ui_Button3) {
        lv_obj_add_event_cb(
            ui_Button3, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(2)));
    }
    if (ui_Button4) {
        lv_obj_add_event_cb(
            ui_Button4, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(3)));
    }
    if (ui_BLINDS) {
        lv_obj_add_event_cb(
            ui_BLINDS, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(4)));
    }
    if (ui_Button6) lv_obj_add_event_cb(ui_Button6, on_alarm, LV_EVENT_CLICKED, nullptr);
}

void tick_1s_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    s_sec = static_cast<uint8_t>((s_sec + 1) % 60);
    bool ui_needs_refresh = false;
    if (s_sec == 0) {
        s_min = static_cast<uint8_t>((s_min + 1) % 60);
        ui_needs_refresh = true;
        if (s_min == 0) {
            const uint8_t prev_h = s_h24;
            s_h24 = static_cast<uint8_t>((s_h24 + 1u) % 24u);
            if (prev_h == 23u) {
                bump_calendar_at_midnight();
                save_state();
            }
        }
    }
    if (ui_needs_refresh) {
        refresh_ui();
    }
    if (++s_sec_since_save >= 60) {
        s_sec_since_save = 0;
        save_state();
    }
}

void ntp_apply_wall_time(const struct tm *tm)
{
    if (!tm) return;
    s_h24 = static_cast<uint8_t>(tm->tm_hour % 24);
    s_min = static_cast<uint8_t>(tm->tm_min % 60);
    s_sec = static_cast<uint8_t>(tm->tm_sec % 60);
    s_weekday = static_cast<uint8_t>(tm->tm_wday % 7);
    s_day = static_cast<uint8_t>(std::clamp(tm->tm_mday, 1, 31));
    s_month = static_cast<uint8_t>(std::clamp(tm->tm_mon + 1, 1, 12));
    s_year = static_cast<uint16_t>(std::clamp(tm->tm_year + 1900, 2000, 2099));

    if (lvgl_port_lock(50)) {
        refresh_ui();
        lvgl_port_unlock();
    }
    save_state();
}

} // namespace

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("romeos-display-v2 clean boot");

    Board *board = new Board();
    board->init();
    assert(board->begin());
    romeos_room_temp_sensor_after_board_begin();
    if (ROMEOS_ALARM_BUZZER_PIN >= 0) {
        pinMode(ROMEOS_ALARM_BUZZER_PIN, OUTPUT);
        digitalWrite(ROMEOS_ALARM_BUZZER_PIN, LOW);
    }

    lvgl_port_init(board->getLCD(), board->getTouch());

    lvgl_port_lock(-1);
    ui_init();
    if (ui_THERMOSTAT_1) {
        lv_obj_add_event_cb(
            ui_THERMOSTAT_1, on_thermostat_screen_loaded_strip_styles, LV_EVENT_SCREEN_LOADED, nullptr);
    }
    s_thermostat_strip_btn_count = strip_thermostat_button_pressed_focus_visuals();
    Serial.printf("[UI] basic theme + btn strip n=%u (wrong COM if no change on display)\n", s_thermostat_strip_btn_count);
    try_apply_rgb565_images();
    build_temp_arc_visual();
    build_home_wifi_anim();
    build_hub_wifi_anim();
    build_alarm_fx();
    /* ROOM: ui_Label3 = αριθμός (με δέκατα), ui_Label5 = ° — οριζόντια απόσταση από τελευταίο ψηφίο (px). */
    static constexpr lv_coord_t k_room_degree_gap_px = 14;
    if (ui_Label5) {
        lv_obj_set_style_translate_x(ui_Label5, k_room_degree_gap_px, LV_PART_MAIN);
    }

    load_state();
    build_placeholder_subscreens();
    build_triple_setpoint_arcs();
    build_day_phase_image();
    bind_ui();
    refresh_ui();
    strip_thermostat_button_pressed_focus_visuals();
    /* On-screen firmware marker and build modification time. */
    lv_obj_t *mk = lv_label_create(ui_THERMOSTAT_1);
    char mk_text[96];
    std::snprintf(
        mk_text,
        sizeof(mk_text),
        "CLEAN-V2 BASIC+CF n=%u  %s %s",
        s_thermostat_strip_btn_count,
        __DATE__,
        __TIME__);
    lv_label_set_text(mk, mk_text);
    lv_obj_set_style_text_color(mk, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(mk, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(mk, LV_ALIGN_BOTTOM_RIGHT, -6, -4);
    lv_timer_create(tick_1s_cb, 1000, nullptr);
    lv_timer_create(clock_blink_cb, 500, nullptr);
    lvgl_port_unlock();

    romeos_ntp_init(ntp_apply_wall_time);
    romeos_display_link_init(on_mb_data_for_link, get_setpoint_x10_for_link, get_room_x10_for_link);
}

void loop()
{
    if (romeos_wifi_nvs_poll_serial()) {
        romeos_display_link_on_home_wifi_saved_to_nvs();
    }
    romeos_display_link_poll();
    romeos_room_temp_sensor_poll_in_app_loop();
    if (s_alarm_demo_active && ROMEOS_ALARM_BUZZER_PIN >= 0) {
        digitalWrite(ROMEOS_ALARM_BUZZER_PIN, HIGH);
    }
    if (s_buzzer_on && static_cast<int32_t>(millis() - s_buzzer_until_ms) >= 0) {
        digitalWrite(ROMEOS_ALARM_BUZZER_PIN, LOW);
        s_buzzer_on = false;
    }
    if (std::isfinite(g_room_temp_c)) {
        const int room_tenths = static_cast<int>(g_room_temp_c * 10.0f + (g_room_temp_c >= 0.0f ? 0.5f : -0.5f));
        if (room_tenths != s_last_room_tenths) {
            s_last_room_tenths = room_tenths;
            if (lvgl_port_lock(20)) {
                refresh_ui();
                lvgl_port_unlock();
            }
        }
    }
    delay(5);
}




