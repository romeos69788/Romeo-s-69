/*
 * romeos-display-v3 — SquareLine `ui_THERMOSTAT_3` + board/LVGL stack (από v2).
 * Χωρίς day-phase strip / τριπλό τεχνητό arc· το setpoint οδηγεί το εξαγόμενο `ui_Arc1`.
 */

#include <Arduino.h>
#include <Preferences.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

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

float g_setpoint_c = 20.0f;

#ifndef ROMEOS_ALARM_BUZZER_PIN
#define ROMEOS_ALARM_BUZZER_PIN 17
#endif

static constexpr int k_setpoint_min_c = 5;
static constexpr int k_setpoint_max_c = 35;
/** Βήμα ρύθμισης setpoint στο UI (και βήμα τόξου σε «μισούς βαθμούς»: arc range 10…70 = 5…35 °C). */
static constexpr float k_setpoint_step_c = 0.5f;
/** Μετά το +/-, μην αντικαθιστάς το setpoint από UDP (η μητρική στέλνει παλιά τιμή κάθε ~400 ms). Μεγαλύτερο παράθυρο = λιγότερες «επιστροφές» σε παλιά τιμή MB. */
static constexpr uint32_t k_mb_setpoint_holdoff_after_local_ms = 60000;
/**
 * Περιστροφή ολόκληρου του τόξου (μοίρες). 0 = LVGL default.
 * Αν στα 20 °C η κουκίδα δεν πέφτει στο οπτικό κέντρο του wallpaper, δοκίμα ±3…±15 εδώ ή στο platformio: -DROMEOS_V3_ARC_ROT_DEG=…
 */
#ifndef ROMEOS_V3_ARC_ROT_DEG
#define ROMEOS_V3_ARC_ROT_DEG 0
#endif

uint8_t s_h24 = 12;
uint8_t s_min = 0;
uint8_t s_sec = 0;
uint8_t s_weekday = 1;
uint8_t s_day = 1;
uint8_t s_month = 5;
uint16_t s_year = 2026;

Preferences s_prefs;
static constexpr const char *k_ns = "v3clk";
uint8_t s_sec_since_save = 0;

bool s_alarm_demo_active = false;
bool s_alarm_blink_on = true;
int s_last_room_tenths = 0x7FFF;

static constexpr lv_coord_t k_panel_w = 800;
static constexpr lv_coord_t k_panel_h = 480;
static lv_obj_t *s_sub_placeholder_scr[4] = {};
static unsigned s_strip_btn_count = 0;
/** Μετά τοπική αλλαγή +/-: μέχρι αυτό το millis() αγνοούμε `setpoint_c_x10` από πακέτα μητρικής. */
static uint32_t s_mb_setpoint_holdoff_until_ms = 0;

/** Wi‑Fi «σπιτιού» (STA στο αποθηκευμένο SSID) — ίδιο πορτοκαλί με `ui_Label28` «ΘΕΡΜΑΝΣΗ». */
static constexpr uint32_t k_wifi_home_active_rgb = 0xFAB647;
/** Σύνδεση με μητρική (UDP) — ίδιο χρώμα με `ui_Label33` «ΑΝΤΛΙΑ / ΘΕΡΜ.». */
static constexpr uint32_t k_wifi_mb_active_rgb = 0xE8A942;
static constexpr uint32_t k_wifi_idle_rgb = 0xFFFFFF;
/** Λεπτή διόρθωση θέσης έναντι των SquareLine `ui_Image8` / `ui_Image10` (θετικό Y = κάτω). */
static constexpr lv_coord_t k_wifi_nudge_down_px = 5;
static constexpr lv_coord_t k_wifi_home_nudge_right_px = 5;
static constexpr lv_coord_t k_wifi_mb_nudge_left_px = 5;

struct V3WifiOrbs {
    lv_obj_t *cont = nullptr;
    lv_obj_t *dot = nullptr;
    lv_obj_t *arc[3] = {nullptr, nullptr, nullptr};
    uint8_t level = 0;
    int8_t dir = 1;
};
static V3WifiOrbs s_wifi_home{};
static V3WifiOrbs s_wifi_mb{};
static uint8_t s_wifi_home_vis_level = 0xFF;
static bool s_wifi_home_vis_conn = false;
static uint8_t s_wifi_mb_vis_level = 0xFF;
static bool s_wifi_mb_vis_conn = false;

static void v3_wifi_build_orbs(V3WifiOrbs *w)
{
    static const int k_w[3] = {28, 42, 56};
    for (int i = 0; i < 3; ++i) {
        lv_obj_t *a = lv_arc_create(w->cont);
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
        w->arc[i] = a;
    }

    w->dot = lv_obj_create(w->cont);
    lv_obj_set_size(w->dot, 8, 8);
    lv_obj_set_style_radius(w->dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(w->dot, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(w->dot, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(w->dot, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_outline_width(w->dot, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(w->dot, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(w->dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align_to(w->dot, w->arc[0], LV_ALIGN_CENTER, 0, 1);
    lv_obj_clear_flag(w->dot, LV_OBJ_FLAG_CLICKABLE);
}

static void v3_apply_wifi_home(uint8_t level, bool connected)
{
    if (!s_wifi_home.dot) {
        return;
    }
    if (level == s_wifi_home_vis_level && connected == s_wifi_home_vis_conn) {
        return;
    }
    const uint8_t prev_level = s_wifi_home_vis_level;
    const bool was_conn = s_wifi_home_vis_conn;
    s_wifi_home_vis_level = level;
    s_wifi_home_vis_conn = connected;

    const uint32_t rgb = connected ? k_wifi_home_active_rgb : k_wifi_idle_rgb;
    const lv_color_t c = lv_color_hex(rgb);
    /* Μην αγγίζεις Label28 / χρώματα τόξων σε κάθε βήμα animation — προκαλούσε invalidate στην κάτω μπάρα
     * (ΘΕΡΜΑΝΣΗ + εικονίδια) κάθε ~900 ms. Παλέτα μόνο στην πρώτη ζωγραφιά ή όταν αλλάζει συνδεσιμότητα. */
    const bool first_paint = (prev_level == 0xFFu);
    const bool conn_changed = (was_conn != connected);
    if (first_paint || conn_changed) {
        lv_obj_set_style_bg_color(s_wifi_home.dot, c, LV_PART_MAIN);
        for (int i = 0; i < 3; ++i) {
            if (s_wifi_home.arc[i]) {
                lv_obj_set_style_arc_color(s_wifi_home.arc[i], c, LV_PART_MAIN);
            }
        }
        if (ui_Label28) {
            lv_obj_set_style_text_color(ui_Label28, c, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    for (int i = 0; i < 3; ++i) {
        lv_obj_t *const a = s_wifi_home.arc[i];
        if (!a) {
            continue;
        }
        const lv_opa_t want =
            connected ? (level > static_cast<uint8_t>(i) ? LV_OPA_COVER : LV_OPA_TRANSP) : LV_OPA_COVER;
        if (lv_obj_get_style_arc_opa(a, LV_PART_MAIN) != want) {
            lv_obj_set_style_arc_opa(a, want, LV_PART_MAIN);
        }
    }
}

static void v3_apply_wifi_mb(uint8_t level, bool connected)
{
    if (!s_wifi_mb.dot) {
        return;
    }
    if (level == s_wifi_mb_vis_level && connected == s_wifi_mb_vis_conn) {
        return;
    }
    const uint8_t prev_level = s_wifi_mb_vis_level;
    const bool was_conn = s_wifi_mb_vis_conn;
    s_wifi_mb_vis_level = level;
    s_wifi_mb_vis_conn = connected;

    const uint32_t rgb = connected ? k_wifi_mb_active_rgb : k_wifi_idle_rgb;
    const lv_color_t c = lv_color_hex(rgb);
    const bool first_paint = (prev_level == 0xFFu);
    const bool conn_changed = (was_conn != connected);
    if (first_paint || conn_changed) {
        lv_obj_set_style_bg_color(s_wifi_mb.dot, c, LV_PART_MAIN);
        for (int i = 0; i < 3; ++i) {
            if (s_wifi_mb.arc[i]) {
                lv_obj_set_style_arc_color(s_wifi_mb.arc[i], c, LV_PART_MAIN);
            }
        }
        if (ui_Label33) {
            lv_obj_set_style_text_color(ui_Label33, c, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    for (int i = 0; i < 3; ++i) {
        lv_obj_t *const a = s_wifi_mb.arc[i];
        if (!a) {
            continue;
        }
        const lv_opa_t want =
            connected ? (level > static_cast<uint8_t>(i) ? LV_OPA_COVER : LV_OPA_TRANSP) : LV_OPA_COVER;
        if (lv_obj_get_style_arc_opa(a, LV_PART_MAIN) != want) {
            lv_obj_set_style_arc_opa(a, want, LV_PART_MAIN);
        }
    }
}

static void v3_wifi_home_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    const bool ok = romeos_display_link_home_wifi_assoc();
    if (!ok) {
        s_wifi_home.level = 0;
        s_wifi_home.dir = 1;
        v3_apply_wifi_home(0, false);
        return;
    }
    int next = static_cast<int>(s_wifi_home.level) + static_cast<int>(s_wifi_home.dir);
    if (next > 3) {
        next = 3;
        s_wifi_home.dir = -1;
    } else if (next < 0) {
        next = 0;
        s_wifi_home.dir = 1;
    }
    s_wifi_home.level = static_cast<uint8_t>(next);
    v3_apply_wifi_home(s_wifi_home.level, true);
}

static void v3_wifi_mb_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    const bool ok = romeos_display_link_is_connected();
    if (!ok) {
        s_wifi_mb.level = 0;
        s_wifi_mb.dir = 1;
        v3_apply_wifi_mb(0, false);
        return;
    }
    int next = static_cast<int>(s_wifi_mb.level) + static_cast<int>(s_wifi_mb.dir);
    if (next > 3) {
        next = 3;
        s_wifi_mb.dir = -1;
    } else if (next < 0) {
        next = 0;
        s_wifi_mb.dir = 1;
    }
    s_wifi_mb.level = static_cast<uint8_t>(next);
    v3_apply_wifi_mb(s_wifi_mb.level, true);
}

static void build_v3_wifi_status_widgets()
{
    if (!ui_THERMOSTAT_3) {
        return;
    }
    /* Αριστερά: αντικατάσταση στατικού wifi image — STA οικιακού Wi‑Fi.
     * Σημαντικό: ΜΗΝ καλείς `lv_obj_align` πριν το `remove_style_all` στο ίδιο obj —
     * το `remove_style_all` σβήνει align/pos και το widget πήγαινε (0,0) → πάνω αριστερά πάνω στο ρολόι. */
    s_wifi_home.cont = lv_obj_create(ui_THERMOSTAT_3);
    lv_obj_remove_style_all(s_wifi_home.cont);
    lv_obj_set_size(s_wifi_home.cont, 88, 84);
    lv_obj_set_style_bg_opa(s_wifi_home.cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_wifi_home.cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(s_wifi_home.cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_wifi_home.cont, LV_OBJ_FLAG_CLICKABLE);
    if (ui_Image8) {
        lv_obj_align_to(
            s_wifi_home.cont,
            ui_Image8,
            LV_ALIGN_CENTER,
            k_wifi_home_nudge_right_px,
            k_wifi_nudge_down_px);
        lv_obj_add_flag(ui_Image8, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_align(
            s_wifi_home.cont,
            LV_ALIGN_CENTER,
            -351 + k_wifi_home_nudge_right_px,
            128 + k_wifi_nudge_down_px);
    }
    v3_wifi_build_orbs(&s_wifi_home);
    if (ui_HEATING) {
        lv_obj_move_to_index(s_wifi_home.cont, lv_obj_get_index(ui_HEATING));
    }

    /* Δεξιά: σύνδεση με μητρική (UDP) — όχι COM7 απευθείας, λογική σύνδεση firmware. */
    s_wifi_mb.cont = lv_obj_create(ui_THERMOSTAT_3);
    lv_obj_remove_style_all(s_wifi_mb.cont);
    lv_obj_set_size(s_wifi_mb.cont, 88, 84);
    lv_obj_set_style_bg_opa(s_wifi_mb.cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_wifi_mb.cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(s_wifi_mb.cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_wifi_mb.cont, LV_OBJ_FLAG_CLICKABLE);
    if (ui_Image10) {
        lv_obj_align_to(
            s_wifi_mb.cont,
            ui_Image10,
            LV_ALIGN_CENTER,
            -k_wifi_mb_nudge_left_px,
            k_wifi_nudge_down_px);
        lv_obj_add_flag(ui_Image10, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_align(
            s_wifi_mb.cont,
            LV_ALIGN_CENTER,
            345 - k_wifi_mb_nudge_left_px,
            126 + k_wifi_nudge_down_px);
    }
    v3_wifi_build_orbs(&s_wifi_mb);
    if (ui_HEAT_PUMP) {
        lv_obj_move_to_index(s_wifi_mb.cont, lv_obj_get_index(ui_HEAT_PUMP));
    }

    v3_apply_wifi_home(0, false);
    v3_apply_wifi_mb(0, false);
    lv_timer_create(v3_wifi_home_timer_cb, 900, nullptr);
    lv_timer_create(v3_wifi_mb_timer_cb, 900, nullptr);
}

float snap_setpoint_half_deg(float c)
{
    c = std::clamp(c, static_cast<float>(k_setpoint_min_c), static_cast<float>(k_setpoint_max_c));
    const float q = std::lroundf(c * 2.0f) * 0.5f;
    return std::clamp(q, static_cast<float>(k_setpoint_min_c), static_cast<float>(k_setpoint_max_c));
}

static const char *const k_gr_wd[7] = {
    "ΚΥΡΙΑΚΗ", "ΔΕΥΤΕΡΑ", "ΤΡΙΤΗ", "ΤΕΤΑΡΤΗ", "ΠΕΜΠΤΗ", "ΠΑΡΑΣΚΕΥΗ", "ΣΑΒΒΑΤΟ",
};

static const char *const k_gr_month_gen[13] = {
    "",
    "ΙΑΝΟΥΑΡΙΟΥ",
    "ΦΕΒΡΟΥΑΡΙΟΥ",
    "ΜΑΡΤΙΟΥ",
    "ΑΠΡΙΛΙΟΥ",
    "ΜΑΪΟΥ",
    "ΙΟΥΝΙΟΥ",
    "ΙΟΥΛΙΟΥ",
    "ΑΥΓΟΥΣΤΟΥ",
    "ΣΕΠΤΕΜΒΡΙΟΥ",
    "ΟΚΤΩΒΡΙΟΥ",
    "ΝΟΕΜΒΡΙΟΥ",
    "ΔΕΚΕΜΒΡΙΟΥ",
};

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
    s_h24 = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("h", 12), 0, 23));
    s_min = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("m", 0), 0, 59));
    s_sec = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("s", 0), 0, 59));
    s_weekday = static_cast<uint8_t>(s_prefs.getUChar("wd", 1) % 7);
    s_month = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("mo", 5), 1, 12));
    s_year = static_cast<uint16_t>(std::clamp<int>(s_prefs.getUShort("y", 2026), 2000, 2099));
    s_day = static_cast<uint8_t>(std::clamp<int>(s_prefs.getUChar("d", 1), 1, days_in_month(s_year, s_month)));
    g_setpoint_c = snap_setpoint_half_deg(
        std::clamp(s_prefs.getFloat("sp", 20.0f), static_cast<float>(k_setpoint_min_c), static_cast<float>(k_setpoint_max_c)));
}

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
            lv_obj_set_style_color_filter_opa(btn, LV_OPA_TRANSP, sel);
            lv_obj_set_style_transform_width(btn, 0, sel);
            lv_obj_set_style_transform_height(btn, 0, sel);
        }
    }
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
}

/**
 * SquareLine εξάγει +/- ως `lv_btn`· το default theme βάζει grow/transform στο release.
 * Αντικατάσταση με διαφανή `lv_obj` (ίδιο parent, align, μέγεθος με export) — χωρίς theme κουμπιού.
 */
static lv_obj_t *v3_make_step_touch_pad(lv_obj_t *prev, lv_coord_t align_x, lv_coord_t align_y)
{
    if (!prev) {
        return nullptr;
    }
    lv_obj_t *const parent = lv_obj_get_parent(prev);
    if (!parent) {
        return nullptr;
    }
    lv_obj_del(prev);
    lv_obj_t *const o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, 100, 98);
    lv_obj_align(o, LV_ALIGN_CENTER, align_x, align_y);
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(o, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(o, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(o, 0, LV_PART_MAIN);
    lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    return o;
}

static void v3_replace_step_buttons_with_plain_touch_pads()
{
    if (!ui_THERMOSTAT_3) {
        return;
    }
    ui_MINUS = v3_make_step_touch_pad(ui_MINUS, -205, -21);
    ui_PLUS = v3_make_step_touch_pad(ui_PLUS, 194, -22);
}

unsigned strip_thermostat_button_pressed_focus_visuals()
{
    lv_obj_t *btns[] = {
        ui_HEATING,
        ui_LIGHS,
        ui_HEAT_PUMP,
        ui_BLINDS,
        ui_ALARM,
        ui_MINUS,
        ui_PLUS,
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
    for (lv_obj_t *lab : {ui_Label1, ui_Label3, ui_Label4, ui_Label5}) {
        strip_one_button_pressed_focus_visuals(lab);
        if (lab && g) {
            lv_group_remove_obj(lab);
        }
    }
    if (ui_THERMOSTAT_3) {
        lv_obj_clear_flag(ui_THERMOSTAT_3, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_clear_flag(ui_THERMOSTAT_3, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }
    if (ui_Arc1) {
        lv_obj_clear_flag(ui_Arc1, LV_OBJ_FLAG_CLICKABLE);
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

void apply_alarm_visual()
{
    if (!ui_ALARM) {
        return;
    }
    constexpr lv_style_selector_t k_sel = LV_PART_MAIN | LV_STATE_DEFAULT;
    const lv_color_t red = lv_color_hex(0xFF3B30);

    if (s_alarm_demo_active) {
        lv_obj_set_style_bg_color(ui_ALARM, red, k_sel);
        lv_obj_set_style_bg_opa(ui_ALARM, LV_OPA_30, k_sel);
        lv_obj_set_style_border_color(ui_ALARM, red, k_sel);
        lv_obj_set_style_border_width(ui_ALARM, 2, k_sel);
        lv_obj_set_style_border_opa(ui_ALARM, LV_OPA_80, k_sel);
    } else {
        lv_obj_set_style_bg_opa(ui_ALARM, LV_OPA_TRANSP, k_sel);
        lv_obj_set_style_border_width(ui_ALARM, 0, k_sel);
        lv_obj_set_style_border_opa(ui_ALARM, LV_OPA_TRANSP, k_sel);
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

int16_t get_setpoint_x10_for_link()
{
    const float sp = snap_setpoint_half_deg(g_setpoint_c);
    int tenths = static_cast<int>(std::lroundf(sp * 10.0f));
    tenths = std::clamp(tenths, k_setpoint_min_c * 10, k_setpoint_max_c * 10);
    return static_cast<int16_t>(tenths);
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

void refresh_ui();

static bool mb_udp_may_apply_setpoint_from_motherboard()
{
    const uint32_t now = millis();
    const uint32_t until = s_mb_setpoint_holdoff_until_ms;
    /* Μετά το +/- το until είναι στο μέλλον· unsigned diff όταν now < until δίνει τεράστιο uint32 → αρνητικό int32_t. */
    return static_cast<int32_t>(now - until) >= 0;
}

void on_mb_data_for_link(const romeos_mb_to_display_v1_t *p)
{
    if (!p || p->magic != ROMEOS_LINK_MAGIC) {
        return;
    }
    if (!romeos_display_link_telemetry_valid()) {
        return;
    }
    bool sp_changed = false;
    /* Κρίσιμο: χωρίς holdoff, κάθε πακέτο MB ξαναγράφει το sp (π.χ. 240) και «σβήνει» το +0,5 του χρήστη. */
    if (p->version >= 2 && mb_udp_may_apply_setpoint_from_motherboard()) {
        const int sp = p->setpoint_c_x10;
        if (sp >= k_setpoint_min_c * 10 && sp <= k_setpoint_max_c * 10) {
            const float incoming = snap_setpoint_half_deg(static_cast<float>(sp) / 10.0f);
            if (std::fabs(incoming - g_setpoint_c) >= 0.05f) {
                g_setpoint_c = incoming;
                sp_changed = true;
            }
        }
    }
    /* Μην καλείς `refresh_ui()` σε κάθε πακέτο (~400 ms): πλήρης αναζωγραφίζει ρολόι/labels/τόξο και «τρεμοπαίζει»
     * μαζί με το animation Wi‑Fi (~900 ms). Ανανέωση μόνο όταν το setpoint από MB άλλαξε πραγματικά. */
    if (sp_changed && lvgl_port_lock(30)) {
        refresh_ui();
        lvgl_port_unlock();
    }
}

void on_back_to_main(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (ui_THERMOSTAT_3) {
        lv_scr_load(ui_THERMOSTAT_3);
    }
}

void on_nav_sub_placeholder(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    const uintptr_t tag = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e));
    if (tag < 1u || tag > 4u) {
        return;
    }
    const unsigned ix = static_cast<unsigned>(tag - 1u);
    if (ix >= 4 || !s_sub_placeholder_scr[ix]) {
        return;
    }
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

void refresh_ui()
{
    char b[48];
    auto label_set_if_changed = [](lv_obj_t *lbl, const char *txt) {
        if (!lbl || !txt) {
            return;
        }
        if (lv_label_get_text(lbl) != nullptr && std::strcmp(lv_label_get_text(lbl), txt) == 0) {
            return;
        }
        lv_label_set_text(lbl, txt);
    };

    /* Setpoint digits + arc */
    {
        const int t = static_cast<int>(g_setpoint_c * 10.0f + (g_setpoint_c >= 0.0f ? 0.5f : -0.5f));
        const int hi = t / 10;
        const int lo = std::abs(t % 10);
        std::snprintf(b, sizeof(b), "%d", hi);
        label_set_if_changed(ui_Label1, b);
        label_set_if_changed(ui_Label3, ".");
        std::snprintf(b, sizeof(b), "%d", lo);
        label_set_if_changed(ui_Label4, b);
        label_set_if_changed(ui_Label5, "O");
        if (ui_Arc1) {
            /* Πρέπει να ταιριάζει με `lv_arc_set_range(..., min*2, max*2)` στο setup: μία μονάδα = 0,5 °C. */
            const int arc_v = std::clamp(
                static_cast<int>(std::lroundf(g_setpoint_c * 2.0f)), k_setpoint_min_c * 2, k_setpoint_max_c * 2);
            if (lv_arc_get_value(ui_Arc1) != arc_v) {
                lv_arc_set_value(ui_Arc1, arc_v);
            }
        }
    }

    /* Clock 12h — Label12 / Label13 ":" / Label14 / Label15 ΑΜ|ΠΜ */
    unsigned h12 = static_cast<unsigned>(s_h24 % 12);
    if (h12 == 0u) {
        h12 = 12u;
    }
    std::snprintf(b, sizeof(b), "%02u", h12);
    label_set_if_changed(ui_Label12, b);
    label_set_if_changed(ui_Label13, ":");
    std::snprintf(b, sizeof(b), "%02u", static_cast<unsigned>(s_min));
    label_set_if_changed(ui_Label14, b);
    label_set_if_changed(ui_Label15, (s_h24 >= 12) ? "ΠΜ" : "ΑΜ");

    /* Date line */
    if (ui_Label36 && s_month >= 1 && s_month <= 12) {
        std::snprintf(
            b,
            sizeof(b),
            "%s  %u  %s",
            k_gr_wd[s_weekday % 7],
            static_cast<unsigned>(s_day),
            k_gr_month_gen[s_month]);
        label_set_if_changed(ui_Label36, b);
    }

    /* Room corner xx.x + O */
    if (std::isfinite(g_room_temp_c)) {
        const int tenths = static_cast<int>(g_room_temp_c * 10.0f + (g_room_temp_c >= 0.0f ? 0.5f : -0.5f));
        const int hi = tenths / 10;
        const int lo = std::abs(tenths % 10);
        std::snprintf(b, sizeof(b), "%d", hi);
        label_set_if_changed(ui_Label38, b);
        label_set_if_changed(ui_Label39, ".");
        std::snprintf(b, sizeof(b), "%d", lo);
        label_set_if_changed(ui_Label40, b);
        label_set_if_changed(ui_Label41, "O");
    } else {
        label_set_if_changed(ui_Label38, "?");
        label_set_if_changed(ui_Label39, "");
        label_set_if_changed(ui_Label40, "");
        label_set_if_changed(ui_Label41, "");
    }
}

void on_minus(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    g_setpoint_c = snap_setpoint_half_deg(g_setpoint_c - k_setpoint_step_c);
    s_mb_setpoint_holdoff_until_ms = millis() + k_mb_setpoint_holdoff_after_local_ms;
    refresh_ui();
    save_state();
    romeos_display_link_on_setpoint_ui_changed();
    romeos_display_link_notify_mb_setpoint_c_x10(get_setpoint_x10_for_link());
}

void on_plus(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    g_setpoint_c = snap_setpoint_half_deg(g_setpoint_c + k_setpoint_step_c);
    s_mb_setpoint_holdoff_until_ms = millis() + k_mb_setpoint_holdoff_after_local_ms;
    refresh_ui();
    save_state();
    romeos_display_link_on_setpoint_ui_changed();
    romeos_display_link_notify_mb_setpoint_c_x10(get_setpoint_x10_for_link());
}

void on_alarm(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    s_alarm_demo_active = !s_alarm_demo_active;
    s_alarm_blink_on = true;
    if (ROMEOS_ALARM_BUZZER_PIN >= 0) {
        digitalWrite(ROMEOS_ALARM_BUZZER_PIN, s_alarm_demo_active ? HIGH : LOW);
    }
    apply_alarm_visual();
    Serial.printf("[UI v3] ALARM demo %s\n", s_alarm_demo_active ? "ON" : "OFF");
}

void bind_ui()
{
    if (ui_MINUS) {
        lv_obj_add_event_cb(ui_MINUS, on_minus, LV_EVENT_CLICKED, nullptr);
    }
    if (ui_PLUS) {
        lv_obj_add_event_cb(ui_PLUS, on_plus, LV_EVENT_CLICKED, nullptr);
    }
    if (ui_HEATING) {
        lv_obj_add_event_cb(
            ui_HEATING, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(1)));
    }
    if (ui_LIGHS) {
        lv_obj_add_event_cb(
            ui_LIGHS, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(2)));
    }
    if (ui_HEAT_PUMP) {
        lv_obj_add_event_cb(
            ui_HEAT_PUMP, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(3)));
    }
    if (ui_BLINDS) {
        lv_obj_add_event_cb(
            ui_BLINDS, on_nav_sub_placeholder, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(4)));
    }
    if (ui_ALARM) {
        lv_obj_add_event_cb(ui_ALARM, on_alarm, LV_EVENT_CLICKED, nullptr);
    }
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
    if (!tm) {
        return;
    }
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
    Serial.println("========================================");
    Serial.println("*** ROMEOS-DISPLAY-V3 (Viewe) FIRMWARE ***");
    Serial.println("========================================");
    Serial.println("If the UI still shows CLEAN-V2, this UART is NOT the display ESP,");
    Serial.println("or upload used the wrong COM. Unplug other USB/COM; flash the Viewe port (COM3).");

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
    /* SquareLine: ξεχασμένο «OFF» στο κέντρο — χωρίς νέο export. */
    if (ui_Label37) {
        lv_obj_add_flag(ui_Label37, LV_OBJ_FLAG_HIDDEN);
    }
    v3_replace_step_buttons_with_plain_touch_pads();
    if (ui_THERMOSTAT_3) {
        lv_obj_add_event_cb(
            ui_THERMOSTAT_3, on_thermostat_screen_loaded_strip_styles, LV_EVENT_SCREEN_LOADED, nullptr);
    }
    s_strip_btn_count = strip_thermostat_button_pressed_focus_visuals();
    Serial.printf("[UI v3] strip buttons n=%u\n", s_strip_btn_count);

    if (ui_Arc1) {
        lv_arc_set_mode(ui_Arc1, LV_ARC_MODE_NORMAL);
        lv_arc_set_bg_angles(ui_Arc1, 135, 405);
        /* Ένα βήμα τόξου = 0,5 °C (τιμές 10…70 → 5,0…35,0 °C). 20 °C = τιμή 40 = μέσο [10,70] → κουκίδα στη μέση του τόξου 135°…405°. */
        lv_arc_set_range(ui_Arc1, k_setpoint_min_c * 2, k_setpoint_max_c * 2);
        lv_arc_set_rotation(ui_Arc1, static_cast<int32_t>(ROMEOS_V3_ARC_ROT_DEG));
        /* SquareLine: το arc μετά τα labels → κουκίδα πάνω στα ψηφία· τόξο από πίσω. */
        lv_obj_move_to_index(ui_Arc1, 0);
    }

    load_state();
    /* Μία φορά μετά από αυτό το update: baseline setpoint 20 °C (κουκίδα στο μαθηματικό κέντρο τόξου). Μετά το +/- αποθηκεύεται κανονικά. */
    {
        prefs_open_rw();
        if (!s_prefs.getBool("v3_sp20_once", false)) {
            g_setpoint_c = snap_setpoint_half_deg(20.0f);
            (void)s_prefs.putBool("v3_sp20_once", true);
            save_state();
        }
    }
    build_placeholder_subscreens();
    build_alarm_fx();
    bind_ui();
    refresh_ui();
    strip_thermostat_button_pressed_focus_visuals();
    if (ui_PLUS) {
        lv_obj_move_foreground(ui_PLUS);
    }
    if (ui_MINUS) {
        lv_obj_move_foreground(ui_MINUS);
    }

    build_v3_wifi_status_widgets();

    lv_timer_create(tick_1s_cb, 1000, nullptr);
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
