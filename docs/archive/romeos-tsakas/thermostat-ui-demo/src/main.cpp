/*
 * Thermostat UI — dark neumorphism style (mid gray), 800x480.
 * Board: Viewe UEDX80480070E-WB-A. Phase: LVGL only (no Wi-Fi to boiler yet).
 */

/* 1 = SquareLine export (src/squareline/). 0 = legacy LVGL layout in this file. */
#ifndef USE_SQUARELINE_UI
#define USE_SQUARELINE_UI 1
#endif

#include <Arduino.h>
#include <Preferences.h>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <esp_display_panel.hpp>
#include <esp_timer.h>
#include <lvgl.h>
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#include <driver/uart.h>
#include <esp_err.h>
#endif

#include "lvgl_v8_port.h"
#include "romeos_room_temp_sensor.h"

#if USE_SQUARELINE_UI
#include "squareline/ui.h"
#include "squareline/screens/ui_Romeos1.h"
#endif

#include "romeos_display_link.h"
#include "romeos_ntp.h"
#include "romeos_wifi_nvs.h"

/* Defaults (board env overrides with -DROMEOS_ROOM_I2C_SDA=…). */
#ifndef ROMEOS_ROOM_I2C_SDA
#define ROMEOS_ROOM_I2C_SDA 43
#endif
#ifndef ROMEOS_ROOM_I2C_SCL
#define ROMEOS_ROOM_I2C_SCL 44
#endif
#if (ROMEOS_ROOM_I2C_SDA == 19) && (ROMEOS_ROOM_I2C_SCL == 20)
#define ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS 1
#else
#define ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS 0
#endif

/* Χωρίς SHT/AHT στην οθόνη: δείξε `room_display` από μητρική όταν η UDP τηλεμετρία είναι «πραγματική» (όχι demo). 0=πάντα «?». */
#ifndef ROMEOS_ROOM_USE_MB_FALLBACK
#define ROMEOS_ROOM_USE_MB_FALLBACK 1
#endif

using namespace esp_panel::drivers;
using namespace esp_panel::board;

namespace {

/* Mid-dark gray palette (neumorphism-style) */
static constexpr uint32_t kBgScreen = 0x454a52;
static constexpr uint32_t kCard = 0x505860;
static constexpr uint32_t kCardInner = 0x4a5158;
static constexpr uint32_t kTextPri = 0xe8eaed;
static constexpr uint32_t kTextSec = 0x9aa0a6;
static constexpr uint32_t kAccentArc = 0x8ab4f8;
static constexpr uint32_t kAccentBtn = 0xc87b4a;
static constexpr uint32_t kArcTrack = 0x3d444d;

float g_setpoint_c = 21.0f;
/*
 * Viewe UEDX80480070E-WB-A: GT911 touch = **I2C_NUM_0**, SDA=**GPIO19**, SCL=**GPIO20**.
 * **GPIO18 / GPIO38** = GT911 INT / RST — ποτέ SHT εκεί.
 * **GPIO10–13** = SPI microSD — όχι για SHT (συχνά i2cRead -1).
 *
 * **Προτεινόμενο (COM4 καθαρό):** SHT **παράλληλα** στο ίδιο I2C με το touch (καλώδια στο ίδιο SDA/SCL, 3V3, GND).
 * Build: `ROMEOS_ROOM_I2C_SDA=19` `ROMEOS_ROOM_I2C_SCL=20` → legacy I2C0 transactions (όχι Wire) → **δεν** αγγίζουμε UART0.
 *
 * **Εναλλακτικό:** SHT σε **GPIO43/44** + `uart_driver_delete(UART0)` → σταθερό SHT αλλά το CH340/COM4 μπορεί να γεμίζει «σκουπίδια» μετά LVGL.
 * Buzzer: **GPIO17**.
 *
 * Overrides: `-DROMEOS_ROOM_I2C_SDA=..` `-DROMEOS_ROOM_I2C_SCL=..` `-DROMEOS_ALARM_BUZZER_PIN=..`
 */
#ifndef ROMEOS_ALARM_BUZZER_PIN
#define ROMEOS_ALARM_BUZZER_PIN 17
#endif
/**
 * Μετά +/-: αγνοούμε setpoint από τηλεμετρία MB μέχρι να στείλει την **ίδια** τιμή (echo UDP/MQTT)
 * ή να περάσει timeout — όχι μόνο χρονόμετρο ms (αποφεύγει «μισό δευτερόλεπτο και πίσω»).
 */
static int16_t s_mb_setpoint_pending_ack_x10 = -1;
static uint32_t s_mb_setpoint_pending_since_ms = 0;

static void mark_local_setpoint_ui_edit()
{
    int tenths = static_cast<int>(g_setpoint_c * 10.0f + (g_setpoint_c >= 0.0f ? 0.5f : -0.5f));
    if (tenths < 50) {
        tenths = 50;
    }
    if (tenths > 350) {
        tenths = 350;
    }
    s_mb_setpoint_pending_ack_x10 = static_cast<int16_t>(tenths);
    s_mb_setpoint_pending_since_ms = millis();
}

lv_obj_t *g_lbl_setpoint_main = nullptr;
lv_obj_t *g_lbl_room_sub = nullptr;
lv_obj_t *g_arc_temp = nullptr;

/* Right panel: sensor values (demo animation; replace with real readings later) */
static constexpr float k_base_solar_c = 87.0f;
static constexpr float k_base_boiler_c = 64.0f;
static constexpr float k_base_supply_c = 40.0f;
static constexpr float k_base_return_c = 34.0f;
static constexpr float k_base_outdoor_c = 12.0f;

float g_temp_solar_c = k_base_solar_c;
float g_temp_boiler_c = k_base_boiler_c;
float g_temp_supply_c = k_base_supply_c;
float g_temp_return_c = k_base_return_c;
float g_temp_outdoor_c = k_base_outdoor_c;
bool g_heat_pump_on = true;
bool g_pump1_on = true;
bool g_pump2_on = true;
bool g_heater_on = true;

lv_obj_t *g_sys_value_lbls[9] = {};

/*
 * Left pad (empirical): ~10px LVGL ≈ 10mm on glass; target 7mm → 7px (10 × 7/10).
 * (The 800/154 formula was wrong here — it gave 39px and ~13mm.)
 */
static constexpr lv_coord_t kRootPadLeftPx = 7;

/* main_row: manual layout only (row flex was resetting child x). */
static constexpr lv_coord_t kUsagePanelW = 256;

static void apply_main_row_layout(lv_obj_t *main_row)
{
    lv_obj_t *lc = lv_obj_get_child(main_row, 0);
    lv_obj_t *usage = lv_obj_get_child(main_row, 1);
    if (lc == nullptr || usage == nullptr) {
        return;
    }
    const lv_coord_t pw = lv_obj_get_content_width(main_row);
    const lv_coord_t ph = lv_obj_get_content_height(main_row);
    const lv_coord_t gap = lv_obj_get_style_pad_column(main_row, 0);
    const lv_coord_t lw = pw - gap - kUsagePanelW;
    if (lw < 8 || ph < 8) {
        return;
    }
    lv_obj_set_size(lc, lw, ph);
    lv_obj_set_size(usage, kUsagePanelW, ph);
    lv_obj_set_pos(lc, 0, 0);
    lv_obj_set_pos(usage, lw + gap, 0);
}

/* Vertical gap in left column; thin bottom bar height (must match scene_bar set_height). */
static constexpr lv_coord_t kLeftColGap = 6;
static constexpr lv_coord_t kSceneBarH = 64;

/* Title + thermo only; reserve bottom strip for scene_bar (sibling on main_row). */
static void layout_left_column(lv_obj_t *left_col)
{
    lv_obj_t *title = lv_obj_get_child(left_col, 0);
    lv_obj_t *thermo = lv_obj_get_child(left_col, 1);
    if (title == nullptr || thermo == nullptr) {
        return;
    }

    const lv_coord_t w = lv_obj_get_content_width(left_col);
    const lv_coord_t h = lv_obj_get_content_height(left_col);
    if (w < 4 || h < 32) {
        return;
    }

    lv_obj_update_layout(title);
    const lv_coord_t title_h = lv_obj_get_height(title);
    const lv_coord_t thermo_h = h - title_h - kLeftColGap - kSceneBarH - kLeftColGap;
    if (thermo_h < 8) {
        return;
    }

    lv_obj_set_width(title, w);
    lv_obj_set_pos(title, 0, 0);

    lv_obj_set_size(thermo, w, thermo_h);
    lv_obj_set_pos(thermo, 0, title_h + kLeftColGap);
}

/*
 * Thin bar: child of main_row so bottom y = ph (same as usage). Width/x match the thermo card
 * (same as old LV_PCT(100) inside left_col), not full main_row lw — that read wider.
 */
static void position_scene_bar(lv_obj_t *main_row, lv_obj_t *scene_bar)
{
    if (main_row == nullptr || scene_bar == nullptr) {
        return;
    }
    lv_obj_t *lc = lv_obj_get_child(main_row, 0);
    lv_obj_t *thermo = (lc != nullptr) ? lv_obj_get_child(lc, 1) : nullptr;
    if (lc == nullptr || thermo == nullptr) {
        return;
    }
    const lv_coord_t ph = lv_obj_get_content_height(main_row);
    if (ph < 8) {
        return;
    }
    const lv_coord_t x_bar = lv_obj_get_x(lc) + lv_obj_get_x(thermo);
    const lv_coord_t w_bar = lv_obj_get_width(thermo);
    if (w_bar < 8) {
        return;
    }
    lv_obj_set_size(scene_bar, w_bar, kSceneBarH);
    lv_obj_set_pos(scene_bar, x_bar, ph - kSceneBarH);
    lv_obj_move_foreground(scene_bar);
}

static void on_main_row_size(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SIZE_CHANGED) {
        return;
    }
    lv_obj_t *main_row = lv_event_get_target(e);
    apply_main_row_layout(main_row);
    lv_obj_t *left_col = lv_obj_get_child(main_row, 0);
    if (left_col != nullptr) {
        layout_left_column(left_col);
    }
    position_scene_bar(main_row, lv_obj_get_child(main_row, 2));
    lv_obj_update_layout(lv_obj_get_parent(main_row));
}

/* Demo: +1 °C/s from base to base+10, then back; On/Off each hold 3 s */
static int s_temp_ramp = 0;
static int s_temp_ramp_dir = 1;
static uint8_t s_onoff_sec_in_phase = 0;
static bool s_onoff_demo_state = true;
static constexpr uint8_t kOnOffHoldSec = 3;

void format_temp_c(char *out, size_t cap, float t)
{
    int tenths = static_cast<int>(t * 10.0f + (t >= 0.0f ? 0.5f : -0.5f));
    int whole = tenths / 10;
    int frac = std::abs(tenths % 10);
    std::snprintf(out, cap, "%d.%d C", whole, frac);
}

/* Ζώνες (0–120 °C): <30 γαλάζιο · 30–45 πορτοκαλί · 45–120 κόκκινο ανοιχτό · <0 λευκό (ένδειξη από −20) */
static uint32_t temp_zone_color_rgb24(float t)
{
    if (t > 120.0f) {
        t = 120.0f;
    }
    if (t < 0.0f) {
        return 0xFFFFFFU;
    }
    if (t < 30.0f) {
        return 0x90CAF9U;
    }
    if (t < 45.0f) {
        return 0xFFCC80U;
    }
    return 0xFFAB91U;
}

static void format_temp_value_recolor(char *out, size_t cap, float t)
{
    const uint32_t rgb = temp_zone_color_rgb24(t);
    std::snprintf(out, cap, "#%06X %.1f# \xC2\xB0""C", static_cast<unsigned>(rgb), static_cast<double>(t));
}

static void refresh_system_panel()
{
    char buf[48];
    float temps[] = {g_temp_solar_c, g_temp_boiler_c, g_temp_supply_c, g_temp_return_c, g_temp_outdoor_c};
    for (int i = 0; i < 5; i++) {
        if (g_sys_value_lbls[i]) {
            format_temp_value_recolor(buf, sizeof(buf), temps[i]);
            lv_label_set_text(g_sys_value_lbls[i], buf);
            lv_obj_set_style_text_color(g_sys_value_lbls[i], lv_color_hex(kTextPri), 0);
        }
    }
    const bool states[] = {g_heat_pump_on, g_pump1_on, g_pump2_on, g_heater_on};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *lbl = g_sys_value_lbls[5 + i];
        if (!lbl) {
            continue;
        }
        if (states[i]) {
            lv_label_set_text(lbl, "On");
            lv_obj_set_style_text_color(lbl, lv_color_hex(0xA5D6A7), 0);
        } else {
            lv_label_set_text(lbl, "Off");
            lv_obj_set_style_text_color(lbl, lv_color_hex(0xEF9A9A), 0);
        }
    }
}

void apply_neumorph_panel(lv_obj_t *obj, uint32_t bg_hex)
{
    const lv_color_t base = lv_color_hex(bg_hex);
    const lv_color_t top_lit = lv_color_lighten(base, LV_OPA_20);

    /* Soft vertical gradient: brighter toward top (light from above) */
    lv_obj_set_style_bg_color(obj, top_lit, 0);
    lv_obj_set_style_bg_grad_color(obj, base, 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

    lv_obj_set_style_radius(obj, 14, 0);

    /* Specular edge on top+left (wider perceived highlight) */
    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, lv_color_lighten(base, LV_OPA_40), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_80, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_LEFT, 0);

    /* Wider, softer cast shadow (bottom-right) */
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x15181c), 0);
    lv_obj_set_style_shadow_width(obj, 28, 0);
    lv_obj_set_style_shadow_spread(obj, 4, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 10, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 12, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_50, 0);
}

#if USE_SQUARELINE_UI
static void squareline_refresh_corner_readouts();
static void squareline_refresh_corner_placeholders();
#endif

/** Όταν true, η γωνία ROOM ακολουθεί `room_display` της μητρικής (δεν υπάρχει SHT στην οθόνη). */
static bool s_room_corner_from_mb = false;

static bool room_corner_ui_has_measurement()
{
    if (g_room_local_valid || romeos_room_temp_sensor_has_live_sample()) {
        return true;
    }
#if ROMEOS_ROOM_USE_MB_FALLBACK
    if (s_room_corner_from_mb && std::isfinite(g_room_temp_c) && romeos_display_link_telemetry_valid()) {
        return true;
    }
#endif
    return false;
}

void refresh_numbers()
{
    char buf[24];
    if (g_lbl_setpoint_main) {
#if USE_SQUARELINE_UI
        int vi = static_cast<int>(g_setpoint_c + (g_setpoint_c >= 0.0f ? 0.5f : -0.5f));
        if (vi < 5) {
            vi = 5;
        }
        if (vi > 35) {
            vi = 35;
        }
        std::snprintf(buf, sizeof(buf), "%d\xc2\xb0", vi);
        lv_label_set_text(g_lbl_setpoint_main, buf);
        /* Ζώνες: ≤20 γαλάζιο, 21–26 πορτοκαλί, 27–35 ανοιχτό κόκκινο */
        uint32_t rgb = 0x87CEEB;
        if (vi > 26) {
            rgb = 0xFFAB91;
        } else if (vi > 20) {
            rgb = 0xFFB74D;
        }
        lv_obj_set_style_text_color(g_lbl_setpoint_main, lv_color_hex(rgb), LV_PART_MAIN);
#else
        format_temp_c(buf, sizeof(buf), g_setpoint_c);
        lv_label_set_text(g_lbl_setpoint_main, buf);
#endif
    }
#if USE_SQUARELINE_UI
    /*
     * Μην αντικαθιστάς τις γωνίες με g_room_temp_c / g_temp_outdoor_c αν δεν υπάρχει
     * έγκυρη τηλεμετρία — αλλιώς κάθε refresh_numbers() (setpoint, on_mb, κ.λπ.)
     * «σβήνει» τα -- και δείχνει νούμερα ντεμο.
     */
    /* ROOM: τοπικός SHT/AHT· ή (αν ROMEOS_ROOM_USE_MB_FALLBACK) room_display από μητρική όταν δεν υπάρχει I2C αισθητήρας. */
    if (room_corner_ui_has_measurement()) {
        squareline_refresh_corner_readouts();
    } else {
        squareline_refresh_corner_placeholders();
    }
#endif
    if (g_lbl_room_sub) {
        if (std::isfinite(g_room_temp_c)) {
            format_temp_c(buf, sizeof(buf), g_room_temp_c);
            char line[32];
            std::snprintf(line, sizeof(line), "Room %s", buf);
            lv_label_set_text(g_lbl_room_sub, line);
        } else {
            lv_label_set_text(g_lbl_room_sub, "Room --");
        }
    }
    if (g_arc_temp) {
        int v = static_cast<int>(g_setpoint_c + 0.5f);
        if (v < 5) {
            v = 5;
        }
        if (v > 35) {
            v = 35;
        }
        lv_arc_set_value(g_arc_temp, v);
    }
}

void on_minus(lv_event_t *e)
{
    LV_UNUSED(e);
    if (g_setpoint_c > 5.0f) {
        g_setpoint_c -= 1.0f;
    }
    mark_local_setpoint_ui_edit();
    refresh_numbers();
    romeos_display_link_on_setpoint_ui_changed();
    char s[20];
    format_temp_c(s, sizeof(s), g_setpoint_c);
    Serial.print("setpoint: ");
    Serial.println(s);
}

void on_plus(lv_event_t *e)
{
    LV_UNUSED(e);
    if (g_setpoint_c < 35.0f) {
        g_setpoint_c += 1.0f;
    }
    mark_local_setpoint_ui_edit();
    refresh_numbers();
    romeos_display_link_on_setpoint_ui_changed();
    char s2[20];
    format_temp_c(s2, sizeof(s2), g_setpoint_c);
    Serial.print("setpoint: ");
    Serial.println(s2);
}

#if USE_SQUARELINE_UI
/* Clock: 24h display, software time. Tap ώρα/λεπτά/AM-PM για επιλογή (αναβόσβηση), βελάκια ρυθμίζουν. */
static uint8_t s_clock_h24 = 19;
static uint8_t s_clock_min = 47;
static uint8_t s_clock_sec = 0;

enum class ClockEdit : uint8_t { None, Hours, Minutes, AmPm };
static ClockEdit s_clock_edit = ClockEdit::None;
static bool s_clock_blink_on = true;

static Preferences s_clock_prefs;
static constexpr const char *k_clock_pref_ns = "tclk";
static uint8_t s_clock_sec_since_save = 0;

/* Ώρα από esp_timer (μονοτόνος χρόνος), όχι από ++ στο LVGL timer: το LVGL μπορεί να χάσει
 * περιόδους όταν το lv_timer_handler καθυστερεί, οπότε το ρολόι έμενε πίσω (δεκάδες λεπτά/μέρες). */
static uint64_t s_clock_mono_us0 = 0;
static uint32_t s_clock_sec0_mod86400 = 0;

static uint32_t squareline_clock_hms_to_sec_mod24(uint8_t h, uint8_t m, uint8_t s)
{
    return (static_cast<uint32_t>(h) * 3600u + static_cast<uint32_t>(m) * 60u + static_cast<uint32_t>(s)) % 86400u;
}

static void squareline_clock_rebase_to_current_hms()
{
    s_clock_sec0_mod86400 = squareline_clock_hms_to_sec_mod24(s_clock_h24, s_clock_min, s_clock_sec);
    s_clock_mono_us0 = esp_timer_get_time();
}

static void squareline_clock_apply_from_monotonic()
{
    const uint64_t now_us = esp_timer_get_time();
    const uint64_t elapsed_s = (now_us - s_clock_mono_us0) / 1000000ULL;
    const uint32_t t =
        static_cast<uint32_t>((static_cast<uint64_t>(s_clock_sec0_mod86400) + elapsed_s) % 86400ULL);
    s_clock_h24 = static_cast<uint8_t>(t / 3600u);
    const uint32_t rem = t % 3600u;
    s_clock_min = static_cast<uint8_t>(rem / 60u);
    s_clock_sec = static_cast<uint8_t>(rem % 60u);
}

static void squareline_prefs_open_rw()
{
    static bool opened = false;
    if (!opened) {
        (void)s_clock_prefs.begin(k_clock_pref_ns, false);
        opened = true;
    }
}

static void squareline_save_clock_to_nvs()
{
    squareline_prefs_open_rw();
    s_clock_prefs.putUChar("h", s_clock_h24);
    s_clock_prefs.putUChar("m", s_clock_min);
    s_clock_prefs.putUChar("s", s_clock_sec);
}

static void squareline_load_clock_from_nvs()
{
    Preferences p;
    if (!p.begin(k_clock_pref_ns, true)) {
        return;
    }
    const uint8_t h = p.getUChar("h", 19);
    const uint8_t m = p.getUChar("m", 47);
    const uint8_t s = p.getUChar("s", 0);
    p.end();
    if (h < 24 && m < 60 && s < 60) {
        s_clock_h24 = h;
        s_clock_min = m;
        s_clock_sec = s;
    }
}

static void squareline_apply_clock_visual_state()
{
    if (ui_Label12) {
        lv_obj_set_style_text_opa(ui_Label12, s_clock_blink_on ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    }
    if (ui_Label11) {
        const bool show = (s_clock_edit != ClockEdit::Hours) || s_clock_blink_on;
        lv_obj_set_style_text_opa(ui_Label11, show ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    }
    if (ui_Label13) {
        const bool show = (s_clock_edit != ClockEdit::Minutes) || s_clock_blink_on;
        lv_obj_set_style_text_opa(ui_Label13, show ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    }
    if (ui_Label3) {
        const bool show = (s_clock_edit != ClockEdit::AmPm) || s_clock_blink_on;
        lv_obj_set_style_text_opa(ui_Label3, show ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    }
}

static void squareline_refresh_clock()
{
    if (!ui_Label11 || !ui_Label13) {
        return;
    }
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%02u", static_cast<unsigned>(s_clock_h24));
    lv_label_set_text(ui_Label11, buf);

    std::snprintf(buf, sizeof(buf), "%02u", static_cast<unsigned>(s_clock_min));
    lv_label_set_text(ui_Label13, buf);

    if (ui_Label3) {
        lv_label_set_text(ui_Label3, (s_clock_h24 >= 12) ? "PM" : "AM");
    }

    squareline_apply_clock_visual_state();
}

static void squareline_clock_tick_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    squareline_clock_apply_from_monotonic();
    squareline_refresh_clock();
    ++s_clock_sec_since_save;
    if (s_clock_sec_since_save >= 60) {
        s_clock_sec_since_save = 0;
        squareline_save_clock_to_nvs();
    }
}

static void squareline_clock_blink_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    s_clock_blink_on = !s_clock_blink_on;
    squareline_apply_clock_visual_state();
}

static void squareline_on_clock_hours_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_clock_edit == ClockEdit::Hours) {
        s_clock_edit = ClockEdit::None;
    } else {
        s_clock_edit = ClockEdit::Hours;
    }
    s_clock_blink_on = true;
    squareline_apply_clock_visual_state();
}

static void squareline_on_clock_minutes_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_clock_edit == ClockEdit::Minutes) {
        s_clock_edit = ClockEdit::None;
    } else {
        s_clock_edit = ClockEdit::Minutes;
    }
    s_clock_blink_on = true;
    squareline_apply_clock_visual_state();
}

static void squareline_on_clock_ampm_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_clock_edit == ClockEdit::AmPm) {
        s_clock_edit = ClockEdit::None;
    } else {
        s_clock_edit = ClockEdit::AmPm;
    }
    s_clock_blink_on = true;
    squareline_apply_clock_visual_state();
}

static void squareline_clock_adjust(bool increment)
{
    if (s_clock_edit == ClockEdit::Hours) {
        if (increment) {
            s_clock_h24 = static_cast<uint8_t>((s_clock_h24 + 1) % 24);
        } else {
            s_clock_h24 = static_cast<uint8_t>((s_clock_h24 + 23) % 24);
        }
    } else if (s_clock_edit == ClockEdit::Minutes) {
        if (increment) {
            s_clock_min = static_cast<uint8_t>((s_clock_min + 1) % 60);
        } else {
            s_clock_min = static_cast<uint8_t>((s_clock_min + 59) % 60);
        }
    } else if (s_clock_edit == ClockEdit::AmPm) {
        /* AM/PM: μετάβαση ±12h (ίδια λειτουργία και στα δύο βελάκια για δίτιμη κατάσταση). */
        s_clock_h24 = static_cast<uint8_t>((s_clock_h24 + 12) % 24);
    } else {
        return;
    }
    squareline_clock_rebase_to_current_hms();
    squareline_refresh_clock();
    squareline_save_clock_to_nvs();
}

/*
 * Δεύτερη οθόνη — πλέγμα 7 γραμμών (SquareLine layout): αριστερά ρολά | δεξιά YPO-1/R (βελάκια)
 * + 5 ζωνών ON/OFF + γραμμή MENU με ← / →. Χωρίς ενδιάμεσα τετράγωνα· κάθετα pad ελάχιστα ώστε
 * να χωράει στο 480px (YPO-1/L + MENU ορατά).
 * Ίδια λογική: ↑/ON πράσινο, ↓/OFF κόκκινο, ουδέτερα λευκό όνομα + γκρι κουμπιά.
 */
static lv_obj_t *s_scr_love = nullptr;
/* Τρίτη οθόνη: απλό δοκιμαστικό περιεχόμενο (ανοίγει από → στη γραμμή Main menu). */
static lv_obj_t *s_scr_love_next = nullptr;
static constexpr uint32_t k_love_scr_anim_ms = 400;
static constexpr uint32_t k_love_bg_rgb = 0x4A5058;
static constexpr lv_coord_t k_shutter_bar_h = 56;
static constexpr lv_coord_t k_shutter_power_d = 65;
/* Ίδιο ύψος γραμμής ρολών / φώτων / Main menu — χρησιμοποιείται και για spacer αριστερά. */
static constexpr lv_coord_t k_shutter_row_outer_h = k_shutter_bar_h + 10;
/* Αριστερή στήλη: translate_x στη μπάρα. Δεξιά: ίδιο ορατό κενό — translate_x στο ζεύγος κουμπιών (+δεξιά). */
static constexpr lv_coord_t k_love_left_bar_nudge_px = -24;
static constexpr lv_coord_t k_love_right_btn_pair_nudge_px = 24;
static constexpr unsigned k_shutter_rows_total = 13u;

enum class ShutterPos : uint8_t {
    Unknown = 0,
    Up,   /* ON */
    Down, /* OFF */
};

struct ShutterRowWidgets {
    lv_obj_t *name_lab = nullptr;
    lv_obj_t *on_btn = nullptr;
    lv_obj_t *on_txt = nullptr;
    lv_obj_t *off_btn = nullptr;
    lv_obj_t *off_txt = nullptr;
};

struct ShutterBtnPayload {
    uint8_t row = 0;
    bool is_on = false;
};

static ShutterRowWidgets s_shutter_rows[k_shutter_rows_total];
static ShutterPos s_shutter_pos[k_shutter_rows_total]{};
static ShutterBtnPayload s_shutter_click_on[k_shutter_rows_total];
static ShutterBtnPayload s_shutter_click_off[k_shutter_rows_total];

/* Montserrat στο LVGL δεν περιλαμβάνει ελληνικά — μόνο Latin/ASCII εδώ (αλλιώς «τετράγωνα»).
 * Για ελληνικά χρειάζεται custom font (lv_font_conv + ελληνικό subset). */
/* Αριστερά: 7 ρολά — ονόματα όπως στο SquareLine (Latin). */
static const char *const k_love_left_names[7] = {
    "SALONI",
    "RM-L",
    "RM-R",
    "SOF-L",
    "SOF-R",
    "YPO-S",
    "YPO-1/L",
};
/* Δεξιά: 1 γραμμή ρολό (YPO-1/R) + 5 φώτα ON/OFF — indices 8..12 στο s_shutter_rows. */
static const char *const k_love_right_shutter_name = "YPO-1/R";
static const char *const k_love_right_light_names[5] = {
    "SALONI",
    "1/FL",
    "2/FL",
    "-1/FL",
    "EXIT",
};

static void squareline_uiverse_shutter_bar_style(lv_obj_t *o)
{
    lv_obj_set_style_radius(o, 14, LV_PART_MAIN);
    const lv_color_t top = lv_color_hex(0x383838);
    const lv_color_t bot = lv_color_hex(0x1A1A1A);
    lv_obj_set_style_bg_color(o, top, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(o, bot, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_border_width(o, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(o, lv_color_lighten(top, LV_OPA_30), LV_PART_MAIN);
    lv_obj_set_style_border_opa(o, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_border_side(o, LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_LEFT, LV_PART_MAIN);

    lv_obj_set_style_shadow_color(o, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(o, 18, LV_PART_MAIN);
    lv_obj_set_style_shadow_spread(o, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_x(o, 3, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(o, 6, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(o, LV_OPA_40, LV_PART_MAIN);

    lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
}

static const lv_border_side_t k_shutter_btn_border_full = static_cast<lv_border_side_t>(
    LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT);

static void squareline_shutter_btn_style_neutral(lv_obj_t *circ, lv_obj_t *lab)
{
    lv_obj_set_style_border_width(circ, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(circ, lv_color_hex(0x090909), LV_PART_MAIN);
    lv_obj_set_style_border_opa(circ, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_side(circ, k_shutter_btn_border_full, LV_PART_MAIN);

    lv_obj_set_style_bg_color(circ, lv_color_hex(0x171717), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(circ, lv_color_hex(0x444245), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(circ, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(circ, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_shadow_color(circ, lv_color_hex(0x141414), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(circ, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_spread(circ, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_x(circ, 7, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(circ, 7, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(circ, LV_OPA_40, LV_PART_MAIN);

    lv_obj_set_style_text_color(lab, lv_color_hex(0xA5A5A5), LV_PART_MAIN);
}

static void squareline_shutter_btn_style_green(lv_obj_t *circ, lv_obj_t *lab)
{
    lv_obj_set_style_border_width(circ, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(circ, lv_color_hex(0x00D700), LV_PART_MAIN);
    lv_obj_set_style_border_opa(circ, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_side(circ, k_shutter_btn_border_full, LV_PART_MAIN);

    lv_obj_set_style_bg_color(circ, lv_color_hex(0x171717), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(circ, lv_color_hex(0x444245), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(circ, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(circ, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_shadow_width(circ, 4, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_x(circ, 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(circ, 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(circ, LV_OPA_20, LV_PART_MAIN);

    lv_obj_set_style_text_color(lab, lv_color_hex(0x00D700), LV_PART_MAIN);
}

static void squareline_shutter_btn_style_red(lv_obj_t *circ, lv_obj_t *lab)
{
    lv_obj_set_style_border_width(circ, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(circ, lv_color_hex(0xE62828), LV_PART_MAIN);
    lv_obj_set_style_border_opa(circ, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_side(circ, k_shutter_btn_border_full, LV_PART_MAIN);

    lv_obj_set_style_bg_color(circ, lv_color_hex(0x171717), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(circ, lv_color_hex(0x444245), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(circ, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(circ, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_shadow_width(circ, 4, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_x(circ, 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(circ, 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(circ, LV_OPA_20, LV_PART_MAIN);

    lv_obj_set_style_text_color(lab, lv_color_hex(0xFF3B3B), LV_PART_MAIN);
}

static void squareline_shutter_row_refresh(uint8_t row_idx)
{
    if (row_idx >= k_shutter_rows_total) {
        return;
    }
    ShutterRowWidgets &w = s_shutter_rows[row_idx];
    if (!w.name_lab || !w.on_btn || !w.on_txt || !w.off_btn || !w.off_txt) {
        return;
    }

    switch (s_shutter_pos[row_idx]) {
    case ShutterPos::Unknown:
        squareline_shutter_btn_style_neutral(w.on_btn, w.on_txt);
        squareline_shutter_btn_style_neutral(w.off_btn, w.off_txt);
        lv_obj_set_style_text_color(w.name_lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        break;
    case ShutterPos::Up:
        squareline_shutter_btn_style_green(w.on_btn, w.on_txt);
        squareline_shutter_btn_style_neutral(w.off_btn, w.off_txt);
        lv_obj_set_style_text_color(w.name_lab, lv_color_hex(0x00D700), LV_PART_MAIN);
        break;
    case ShutterPos::Down:
        squareline_shutter_btn_style_neutral(w.on_btn, w.on_txt);
        squareline_shutter_btn_style_red(w.off_btn, w.off_txt);
        lv_obj_set_style_text_color(w.name_lab, lv_color_hex(0xFF3B3B), LV_PART_MAIN);
        break;
    default:
        break;
    }
}

static void squareline_shutter_row_btn_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    auto *p = static_cast<ShutterBtnPayload *>(lv_event_get_user_data(e));
    if (p->row >= k_shutter_rows_total) {
        return;
    }
    s_shutter_pos[p->row] = p->is_on ? ShutterPos::Up : ShutterPos::Down;
    squareline_shutter_row_refresh(p->row);
}

static lv_obj_t *squareline_shutter_on_off_btn_create(lv_obj_t *parent,
                                                      uint8_t row_idx,
                                                      bool is_on_button,
                                                      lv_obj_t **out_label,
                                                      bool lights_on_off_labels)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_set_size(c, k_shutter_power_d, k_shutter_power_d);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_pad_all(c, 0, LV_PART_MAIN);
    lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(c, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *lab = lv_label_create(c);
    if (lights_on_off_labels) {
        lv_label_set_text(lab, is_on_button ? "ON" : "OFF");
        lv_obj_set_style_text_font(lab, &lv_font_montserrat_22, LV_PART_MAIN);
    } else {
        lv_label_set_text(lab, is_on_button ? LV_SYMBOL_UP : LV_SYMBOL_DOWN);
        lv_obj_set_style_text_font(lab, &lv_font_montserrat_28, LV_PART_MAIN);
    }
    lv_obj_clear_flag(lab, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(lab);

    ShutterBtnPayload *payload =
        is_on_button ? &s_shutter_click_on[row_idx] : &s_shutter_click_off[row_idx];
    payload->row = row_idx;
    payload->is_on = is_on_button;
    lv_obj_add_event_cb(c, squareline_shutter_row_btn_cb, LV_EVENT_CLICKED, payload);
    if (out_label) {
        *out_label = lab;
    }
    return c;
}

/* Γεμίζει ένα οριζόντιο strip (bar + 2 κουμπιά) — parent είναι ήδη flex row.
 * bar_translate_x: μετατόπιση μπάρας (αρνητικό = αριστερά).
 * btn_pair_translate_x: ίδια μετατόπιση και στα δύο κουμπιά (θετικό = δεξιά) — χωρίς ενδιάμεσο pair,
 * ώστε το flex πλάτος της μπάρας να μένει όπως πριν (όχι στένεμα / wrap ονομάτων). */
static void squareline_shutter_strip_populate(lv_obj_t *strip,
                                              const char *name,
                                              uint8_t row_idx,
                                              const lv_font_t *name_font,
                                              bool lights_on_off_labels,
                                              lv_coord_t bar_translate_x = 0,
                                              lv_coord_t btn_pair_translate_x = 0)
{
    lv_obj_set_width(strip, LV_PCT(100));
    lv_obj_set_height(strip, k_shutter_row_outer_h);
    lv_obj_set_style_bg_opa(strip, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(strip, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(strip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(strip,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(strip, 8, LV_PART_MAIN);
    lv_obj_clear_flag(strip, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *bar = lv_obj_create(strip);
    lv_obj_set_height(bar, k_shutter_bar_h);
    lv_obj_set_flex_grow(bar, 1);
    squareline_uiverse_shutter_bar_style(bar);

    lv_obj_t *lab = lv_label_create(bar);
    lv_label_set_text(lab, name);
    const lv_font_t *name_f = name_font ? name_font : &lv_font_montserrat_28;
    lv_obj_set_style_text_font(lab, name_f, LV_PART_MAIN);
    lv_obj_set_style_text_color(lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_DOT);
    lv_obj_set_style_max_height(lab, lv_font_get_line_height(name_f), LV_PART_MAIN);
    lv_obj_set_width(lab, LV_PCT(92));
    lv_obj_align(lab, LV_ALIGN_LEFT_MID, 8, 0);

    if (bar_translate_x != 0) {
        lv_obj_set_style_translate_x(bar, bar_translate_x, LV_PART_MAIN);
    }

    lv_obj_t *on_txt = nullptr;
    lv_obj_t *off_txt = nullptr;
    lv_obj_t *on_c =
        squareline_shutter_on_off_btn_create(strip, row_idx, true, &on_txt, lights_on_off_labels);
    lv_obj_t *off_c =
        squareline_shutter_on_off_btn_create(strip, row_idx, false, &off_txt, lights_on_off_labels);

    if (btn_pair_translate_x != 0) {
        lv_obj_set_style_translate_x(on_c, btn_pair_translate_x, LV_PART_MAIN);
        lv_obj_set_style_translate_x(off_c, btn_pair_translate_x, LV_PART_MAIN);
    }

    if (bar_translate_x != 0 || btn_pair_translate_x != 0) {
        lv_obj_add_flag(strip, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    }

    ShutterRowWidgets &R = s_shutter_rows[row_idx];
    R.name_lab = lab;
    R.on_btn = on_c;
    R.on_txt = on_txt;
    R.off_btn = off_c;
    R.off_txt = off_txt;
    squareline_shutter_row_refresh(row_idx);
}

static lv_obj_t *squareline_shutter_row_create(lv_obj_t *parent,
                                              const char *name,
                                              uint8_t row_idx,
                                              const lv_font_t *name_font = nullptr,
                                              bool lights_on_off_labels = false)
{
    lv_obj_t *row = lv_obj_create(parent);
    squareline_shutter_strip_populate(row, name, row_idx, name_font, lights_on_off_labels, 0, 0);
    return row;
}

static void squareline_love_go_to_main()
{
#if USE_SQUARELINE_UI
    lv_scr_load_anim(ui_Romeos1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, k_love_scr_anim_ms, 0, false);
#endif
}

static void squareline_love_next_screen_on_back_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (!s_scr_love) {
        return;
    }
    lv_scr_load_anim(s_scr_love, LV_SCR_LOAD_ANIM_MOVE_RIGHT, k_love_scr_anim_ms, 0, false);
}

static constexpr uint32_t k_welcome3_bg_rgb = 0x5C5C5C;

static void squareline_love_next_screen_ensure_created()
{
    if (s_scr_love_next) {
        return;
    }
    s_scr_love_next = lv_obj_create(nullptr);
    lv_obj_set_size(s_scr_love_next, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_scr_love_next, lv_color_hex(k_welcome3_bg_rgb), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr_love_next, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_flex_flow(s_scr_love_next, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(s_scr_love_next, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_right(s_scr_love_next, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_top(s_scr_love_next, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(s_scr_love_next, 8, LV_PART_MAIN);

    lv_obj_t *mid = lv_obj_create(s_scr_love_next);
    lv_obj_set_width(mid, LV_PCT(100));
    lv_obj_set_flex_grow(mid, 1);
    lv_obj_set_style_bg_opa(mid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(mid, 0, LV_PART_MAIN);
    lv_obj_clear_flag(mid, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(mid);
    lv_label_set_text(title, "WELCOME 3");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(title, 4, LV_PART_MAIN);
    lv_obj_center(title);

    lv_obj_t *nb = lv_btn_create(s_scr_love_next);
    lv_obj_set_size(nb, 280, 52);
    lv_obj_set_style_bg_color(nb, lv_color_hex(0x4A5058), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(nb, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(nb, 10, LV_PART_MAIN);
    lv_obj_t *nl = lv_label_create(nb);
    lv_label_set_text(nl, LV_SYMBOL_LEFT "  Back");
    lv_obj_set_style_text_font(nl, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_center(nl);
    lv_obj_add_event_cb(nb, squareline_love_next_screen_on_back_cb, LV_EVENT_CLICKED, nullptr);
}

static void squareline_love_nav_left_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    squareline_love_go_to_main();
}

static void squareline_love_nav_to_next_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    squareline_love_next_screen_ensure_created();
    lv_scr_load_anim(s_scr_love_next, LV_SCR_LOAD_ANIM_MOVE_LEFT, k_love_scr_anim_ms, 0, false);
}

static lv_obj_t *squareline_shutter_nav_arrow_btn_create(lv_obj_t *parent,
                                                         const char *sym,
                                                         lv_event_cb_t cb)
{
    lv_obj_t *c = lv_obj_create(parent);
    lv_obj_set_size(c, k_shutter_power_d, k_shutter_power_d);
    lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_pad_all(c, 0, LV_PART_MAIN);
    lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(c, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(c, 18);
    lv_obj_t *lab = lv_label_create(c);
    lv_label_set_text(lab, sym);
    lv_obj_set_style_text_font(lab, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_clear_flag(lab, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(lab);
    squareline_shutter_btn_style_neutral(c, lab);
    lv_obj_add_event_cb(c, cb, LV_EVENT_CLICKED, nullptr);
    return c;
}

static void squareline_shutter_nav_strip_populate(lv_obj_t *row,
                                                  const char *menu_title,
                                                  lv_coord_t arrow_pair_translate_x = 0)
{
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, k_shutter_row_outer_h);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 8, LV_PART_MAIN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *bar = lv_obj_create(row);
    lv_obj_set_height(bar, k_shutter_bar_h);
    lv_obj_set_flex_grow(bar, 1);
    squareline_uiverse_shutter_bar_style(bar);

    lv_obj_t *lab = lv_label_create(bar);
    lv_label_set_text(lab, menu_title);
    lv_obj_set_style_text_font(lab, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(lab, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_label_set_long_mode(lab, LV_LABEL_LONG_DOT);
    lv_obj_set_style_max_height(lab, lv_font_get_line_height(&lv_font_montserrat_28), LV_PART_MAIN);
    lv_obj_set_width(lab, LV_PCT(92));
    lv_obj_align(lab, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t *nav_l = squareline_shutter_nav_arrow_btn_create(row, LV_SYMBOL_LEFT, squareline_love_nav_left_cb);
    lv_obj_t *nav_r = squareline_shutter_nav_arrow_btn_create(row, LV_SYMBOL_RIGHT, squareline_love_nav_to_next_cb);
    if (arrow_pair_translate_x != 0) {
        lv_obj_set_style_translate_x(nav_l, arrow_pair_translate_x, LV_PART_MAIN);
        lv_obj_set_style_translate_x(nav_r, arrow_pair_translate_x, LV_PART_MAIN);
        lv_obj_add_flag(row, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    }
}

static void squareline_love_screen_ensure_created()
{
    if (s_scr_love) {
        return;
    }
    s_scr_love = lv_obj_create(nullptr);
    lv_obj_set_size(s_scr_love, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_scr_love, lv_color_hex(k_love_bg_rgb), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_scr_love, LV_OPA_COVER, LV_PART_MAIN);
    /* Scroll μόνο στο body — αλλιώς το scroll της οθόνης «κλέβει» αφές κοντά στα κουμπιά (Main menu →). */
    lv_obj_clear_flag(s_scr_love, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(s_scr_love, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(s_scr_love, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_right(s_scr_love, 10, LV_PART_MAIN);
    /* 7 × k_shutter_row_outer_h (66) = 462px — με μικρό κάθετο pad χωράει στο 480px χωρίς κόψιμο. */
    lv_obj_set_style_pad_top(s_scr_love, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(s_scr_love, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_row(s_scr_love, 0, LV_PART_MAIN);

    lv_obj_t *body = lv_obj_create(s_scr_love);
    lv_obj_set_width(body, LV_PCT(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(body, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(body, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(body, 0, LV_PART_MAIN);
    lv_obj_clear_flag(body, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *grid = lv_obj_create(body);
    lv_obj_set_width(grid, LV_PCT(100));
    lv_obj_set_flex_grow(grid, 1);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(grid, 0, LV_PART_MAIN);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    for (unsigned band = 0; band < 7u; band++) {
        lv_obj_t *r = lv_obj_create(grid);
        lv_obj_set_width(r, LV_PCT(100));
        lv_obj_set_height(r, k_shutter_row_outer_h);
        lv_obj_set_style_bg_opa(r, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(r, 0, LV_PART_MAIN);
        lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(r,
                              LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(r, 24, LV_PART_MAIN);
        lv_obj_clear_flag(r, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(r, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

        lv_obj_t *lc = lv_obj_create(r);
        lv_obj_set_flex_grow(lc, 1);
        lv_obj_set_style_bg_opa(lc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(lc, 0, LV_PART_MAIN);
        lv_obj_clear_flag(lc, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(lc, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

        lv_obj_t *rc = lv_obj_create(r);
        lv_obj_set_flex_grow(rc, 1);
        lv_obj_set_style_bg_opa(rc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(rc, 0, LV_PART_MAIN);
        lv_obj_clear_flag(rc, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(rc, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

        squareline_shutter_strip_populate(lc,
                                          k_love_left_names[band],
                                          static_cast<uint8_t>(band),
                                          nullptr,
                                          false,
                                          k_love_left_bar_nudge_px,
                                          0);

        if (band == 0u) {
            squareline_shutter_strip_populate(rc,
                                              k_love_right_shutter_name,
                                              7,
                                              nullptr,
                                              false,
                                              0,
                                              k_love_right_btn_pair_nudge_px);
        } else if (band >= 1u && band <= 5u) {
            const unsigned li = band - 1u;
            squareline_shutter_strip_populate(rc,
                                              k_love_right_light_names[li],
                                              static_cast<uint8_t>(8u + li),
                                              &lv_font_montserrat_28,
                                              true,
                                              0,
                                              k_love_right_btn_pair_nudge_px);
        } else {
            squareline_shutter_nav_strip_populate(rc, "MENU", k_love_right_btn_pair_nudge_px);
        }
    }

    lv_obj_update_layout(s_scr_love);
}

static void squareline_on_arrow_left(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_clock_edit != ClockEdit::None) {
        squareline_clock_adjust(false);
        return;
    }
}

static void squareline_on_arrow_right(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_clock_edit != ClockEdit::None) {
        squareline_clock_adjust(true);
        return;
    }
#if USE_SQUARELINE_UI
    squareline_love_screen_ensure_created();
    lv_scr_load_anim(s_scr_love, LV_SCR_LOAD_ANIM_MOVE_LEFT, k_love_scr_anim_ms, 0, false);
#endif
}

/* --- Γωνίες κεντρικού πάνελ: ROOM / OUT (από μητρική όταν έγκυρα δεδομένα, αλλιώς «--») · HOME κάτω-αριστερά · ESP32 = link μητρικής --- */
struct SquarelineWifiInd {
    lv_obj_t *cont = nullptr;
    lv_obj_t *dot = nullptr;
    lv_obj_t *arc[3] = {};
    lv_obj_t *caption = nullptr;
    uint8_t step = 0;
    bool connected = true;
};

static SquarelineWifiInd s_wifi_home;
static SquarelineWifiInd s_wifi_esp;
/** Παλμός τόξων ESP32 όταν link μητρικής OK: 1→2→3→2→1… */
static uint8_t s_wifi_arc_level = 1;
static int8_t s_wifi_arc_dir = 1;
/** Ίδιος παλμός για HOME όταν STA στο οικιακό SSID (οπτική συνέπεια με ESP32). */
static uint8_t s_wifi_home_arc_level = 1;
static int8_t s_wifi_home_arc_dir = 1;
/** Ταχύτητα παλμού τόξων σήματος (ms). */
static constexpr uint32_t k_wifi_status_timer_ms = 500;
/** Περίοδος ενημέρωσης θερμικών ετικετών (χωρίς ντεμο· placeholders όσο δεν υπάρχουν έγκυρα δεδομένα). */
static constexpr uint32_t k_thermal_ui_tick_ms = 1000;
static constexpr uint32_t k_ui_placeholder_rgb = 0xB0BEC5;
static constexpr uint32_t k_boiler_on_rgb = 0xA5D6A7;
static constexpr uint32_t k_boiler_off_rgb = 0xFFAB91;

static uint32_t squareline_room_corner_rgb(int vi)
{
    if (vi <= 20) {
        return 0xB3E5FC;
    }
    if (vi <= 25) {
        return 0xFFCC80;
    }
    return 0xFFAB91;
}

static void squareline_set_label_color32(lv_obj_t *o, uint32_t rgb)
{
    if (!o) {
        return;
    }
    lv_obj_set_style_text_color(o, lv_color_hex(rgb), LV_PART_MAIN);
}

static void squareline_refresh_corner_readouts()
{
    if (!ui_Romeos1) {
        return;
    }
    char nb[16];
    if (!std::isfinite(g_room_temp_c)) {
        if (ui_Room) {
            /* Όχι «--»· μερικές ρυθμίσεις LVGL/γραμματοσειρά εμφανίζουν «κουτάκια». */
            lv_label_set_text(ui_Room, "?");
            squareline_set_label_color32(ui_Room, k_ui_placeholder_rgb);
        }
        if (ui_room8) {
            lv_label_set_text(ui_room8, "ROOM");
            squareline_set_label_color32(ui_room8, k_ui_placeholder_rgb);
        }
        if (ui_LabelO1) {
            lv_label_set_text(ui_LabelO1, "\xc2\xb0");
            squareline_set_label_color32(ui_LabelO1, k_ui_placeholder_rgb);
        }
    } else {
        int room_i = static_cast<int>(g_room_temp_c + (g_room_temp_c >= 0.0f ? 0.5f : -0.5f));
        if (room_i < 0) {
            room_i = 0;
        }
        if (room_i > 35) {
            room_i = 35;
        }
        const uint32_t cr = squareline_room_corner_rgb(room_i);
        std::snprintf(nb, sizeof(nb), "%d", room_i);
        if (ui_Room) {
            lv_label_set_text(ui_Room, nb);
            squareline_set_label_color32(ui_Room, cr);
        }
        if (ui_room8) {
            lv_label_set_text(ui_room8, "ROOM");
            squareline_set_label_color32(ui_room8, cr);
        }
        if (ui_LabelO1) {
            lv_label_set_text(ui_LabelO1, "\xc2\xb0");
            squareline_set_label_color32(ui_LabelO1, cr);
        }
    }

    if (!std::isfinite(g_temp_outdoor_c)) {
        if (ui_Out) {
            lv_label_set_text(ui_Out, "--");
            squareline_set_label_color32(ui_Out, k_ui_placeholder_rgb);
        }
        if (ui_room2) {
            lv_label_set_text(ui_room2, "OUT");
            squareline_set_label_color32(ui_room2, k_ui_placeholder_rgb);
        }
        if (ui_LabelO2) {
            lv_label_set_text(ui_LabelO2, "\xc2\xb0");
            squareline_set_label_color32(ui_LabelO2, k_ui_placeholder_rgb);
        }
    } else {
        int out_i = static_cast<int>(g_temp_outdoor_c + (g_temp_outdoor_c >= 0.0f ? 0.5f : -0.5f));
        out_i = std::clamp(out_i, -20, 60);
        const uint32_t co = temp_zone_color_rgb24(g_temp_outdoor_c);
        std::snprintf(nb, sizeof(nb), "%d", out_i);
        if (ui_Out) {
            lv_label_set_text(ui_Out, nb);
            squareline_set_label_color32(ui_Out, co);
        }
        if (ui_room2) {
            lv_label_set_text(ui_room2, "OUT");
            squareline_set_label_color32(ui_room2, co);
        }
        if (ui_LabelO2) {
            lv_label_set_text(ui_LabelO2, "\xc2\xb0");
            squareline_set_label_color32(ui_LabelO2, co);
        }
    }
}

static void squareline_refresh_corner_placeholders()
{
    if (!ui_Romeos1) {
        return;
    }
    if (room_corner_ui_has_measurement() && std::isfinite(g_room_temp_c)) {
        int room_i = static_cast<int>(g_room_temp_c + (g_room_temp_c >= 0.0f ? 0.5f : -0.5f));
        room_i = std::clamp(room_i, 0, 35);
        const uint32_t cr = squareline_room_corner_rgb(room_i);
        char nb[16];
        std::snprintf(nb, sizeof(nb), "%d", room_i);
        if (ui_Room) {
            lv_label_set_text(ui_Room, nb);
            squareline_set_label_color32(ui_Room, cr);
        }
        if (ui_room8) {
            lv_label_set_text(ui_room8, "ROOM");
            squareline_set_label_color32(ui_room8, cr);
        }
        if (ui_LabelO1) {
            lv_label_set_text(ui_LabelO1, "\xc2\xb0");
            squareline_set_label_color32(ui_LabelO1, cr);
        }
    } else {
        if (ui_Room) {
            lv_label_set_text(ui_Room, "?");
            squareline_set_label_color32(ui_Room, k_ui_placeholder_rgb);
        }
        if (ui_room8) {
            lv_label_set_text(ui_room8, "ROOM");
            squareline_set_label_color32(ui_room8, k_ui_placeholder_rgb);
        }
        if (ui_LabelO1) {
            lv_label_set_text(ui_LabelO1, "\xc2\xb0");
            squareline_set_label_color32(ui_LabelO1, k_ui_placeholder_rgb);
        }
    }
    if (ui_Out) {
        lv_label_set_text(ui_Out, "--");
        squareline_set_label_color32(ui_Out, k_ui_placeholder_rgb);
    }
    if (ui_room2) {
        lv_label_set_text(ui_room2, "OUT");
        squareline_set_label_color32(ui_room2, k_ui_placeholder_rgb);
    }
    if (ui_LabelO2) {
        lv_label_set_text(ui_LabelO2, "\xc2\xb0");
        squareline_set_label_color32(ui_LabelO2, k_ui_placeholder_rgb);
    }
}

static void squareline_apply_right_temp_row_placeholder(lv_obj_t *num_lbl,
                                                        lv_obj_t *deg_lbl,
                                                        lv_obj_t *name_lbl)
{
    if (!num_lbl || !deg_lbl || !name_lbl) {
        return;
    }
    lv_label_set_text(num_lbl, "--");
    squareline_set_label_color32(num_lbl, k_ui_placeholder_rgb);
    squareline_set_label_color32(deg_lbl, k_ui_placeholder_rgb);
    squareline_set_label_color32(name_lbl, k_ui_placeholder_rgb);
}

static void squareline_refresh_right_temp_placeholders()
{
    squareline_apply_right_temp_row_placeholder(ui_Solar, ui_solarO, ui_Solar2);
    squareline_apply_right_temp_row_placeholder(ui_Boiler, ui_boilerO2, ui_Boiler3);
    squareline_apply_right_temp_row_placeholder(ui_Intel2, ui_intelO3, ui_Boiler2);
    squareline_apply_right_temp_row_placeholder(ui_Outlet3, ui_outletO2, ui_autlet4);
}

/** Όταν false: εμφάνιση ON/Off από g_heater_on. Όταν true (χωρίς σύνδεση): «-» (όχι em dash U+2014: λείπει από Montserrat). */
static void squareline_refresh_boiler_row(bool disconnected)
{
    if (!ui_BOILE2 || !ui_BOILE4) {
        return;
    }
    if (disconnected) {
        lv_label_set_text(ui_BOILE2, "-");
        squareline_set_label_color32(ui_BOILE2, k_ui_placeholder_rgb);
        squareline_set_label_color32(ui_BOILE4, k_ui_placeholder_rgb);
        return;
    }
    if (g_heater_on) {
        lv_label_set_text(ui_BOILE2, "ON");
        squareline_set_label_color32(ui_BOILE2, k_boiler_on_rgb);
        squareline_set_label_color32(ui_BOILE4, k_boiler_on_rgb);
    } else {
        lv_label_set_text(ui_BOILE2, "Off");
        squareline_set_label_color32(ui_BOILE2, k_boiler_off_rgb);
        squareline_set_label_color32(ui_BOILE4, k_boiler_off_rgb);
    }
}

static void squareline_corner_degree_labels_style()
{
    if (ui_LabelO1) {
        lv_obj_set_style_text_font(ui_LabelO1, &lv_font_montserrat_24, LV_PART_MAIN);
    }
    if (ui_LabelO2) {
        lv_obj_set_style_text_font(ui_LabelO2, &lv_font_montserrat_24, LV_PART_MAIN);
    }
}

static void squareline_wifi_apply_visual(SquarelineWifiInd *w)
{
    if (!w || !w->cont || !w->dot) {
        return;
    }
    constexpr uint32_t k_green = 0x66BB6A;
    constexpr uint32_t k_red = 0xFFAB91;
    if (!w->connected) {
        lv_obj_set_style_bg_color(w->dot, lv_color_hex(k_red), LV_PART_MAIN);
        for (int i = 0; i < 3; i++) {
            if (w->arc[i]) {
                lv_obj_set_style_arc_color(w->arc[i], lv_color_hex(k_red), LV_PART_MAIN);
                lv_obj_set_style_arc_opa(w->arc[i], LV_OPA_COVER, LV_PART_MAIN);
            }
        }
        if (w->caption) {
            squareline_set_label_color32(w->caption, k_red);
        }
        return;
    }
    lv_obj_set_style_bg_color(w->dot, lv_color_hex(k_green), LV_PART_MAIN);
    if (w->caption) {
        squareline_set_label_color32(w->caption, k_green);
    }
    for (int i = 0; i < 3; i++) {
        if (!w->arc[i]) {
            continue;
        }
        lv_obj_set_style_arc_color(w->arc[i], lv_color_hex(k_green), LV_PART_MAIN);
        const bool on = (w->step > static_cast<uint8_t>(i));
        lv_obj_set_style_arc_opa(w->arc[i], on ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    }
}

static void squareline_wifi_link_status_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    /* HOME: πράσινο + παλμός τόξων όταν STA στο αποθηκευμένο οικιακό SSID (ίδιο στυλ με ESP32). */
    const bool home_assoc = romeos_display_link_home_wifi_assoc();
    s_wifi_home.connected = home_assoc;
    if (home_assoc) {
        int hlev = static_cast<int>(s_wifi_home_arc_level) + static_cast<int>(s_wifi_home_arc_dir);
        if (hlev > 3) {
            hlev = 3;
            s_wifi_home_arc_dir = -1;
        }
        if (hlev < 1) {
            hlev = 1;
            s_wifi_home_arc_dir = 1;
        }
        s_wifi_home_arc_level = static_cast<uint8_t>(hlev);
        s_wifi_home.step = s_wifi_home_arc_level;
    } else {
        s_wifi_home.step = 0;
        s_wifi_home_arc_level = 1;
        s_wifi_home_arc_dir = 1;
    }
    squareline_wifi_apply_visual(&s_wifi_home);

    const bool mb_ok = romeos_display_link_is_connected();
    s_wifi_esp.connected = mb_ok;
    if (mb_ok) {
        int lev = static_cast<int>(s_wifi_arc_level) + static_cast<int>(s_wifi_arc_dir);
        if (lev > 3) {
            lev = 3;
            s_wifi_arc_dir = -1;
        }
        if (lev < 1) {
            lev = 1;
            s_wifi_arc_dir = 1;
        }
        s_wifi_arc_level = static_cast<uint8_t>(lev);
        s_wifi_esp.step = s_wifi_arc_level;
    } else {
        s_wifi_esp.step = 0;
        s_wifi_arc_level = 1;
        s_wifi_arc_dir = 1;
    }
    squareline_wifi_apply_visual(&s_wifi_esp);
}

static void squareline_wifi_build_one(SquarelineWifiInd *w, lv_coord_t ax, lv_coord_t ay, lv_obj_t *caption_lbl)
{
    if (!ui_Romeos1) {
        return;
    }
    w->caption = caption_lbl;
    w->cont = lv_obj_create(ui_Romeos1);
    lv_obj_set_size(w->cont, 120, 108);
    lv_obj_align(w->cont, LV_ALIGN_CENTER, ax, ay);
    lv_obj_set_style_bg_opa(w->cont, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(w->cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(w->cont, LV_OBJ_FLAG_SCROLLABLE);

    static const int k_ws[3] = {34, 54, 74};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *a = lv_arc_create(w->cont);
        lv_obj_set_size(a, k_ws[i], k_ws[i]);
        /* Χωρίς κουκκίδα· τα τόξα χαμηλότερα για κενό από τις ετικέτες HOME / ESP32 */
        lv_obj_align(a, LV_ALIGN_CENTER, 0, 8);
        /* Ορθογώνιο φόντο του widget (theme) — διαφανές, αλλιώς φαίνεται μπλε «κηλίδα» */
        lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_arc_set_bg_angles(a, 210, 330);
        lv_arc_set_range(a, 0, 100);
        lv_arc_set_mode(a, LV_ARC_MODE_NORMAL);
        /* Μετά το bg: value=min ώστε το INDICATOR (default 135–270, συχνά μπλε) να μηδενίσει μήκος */
        lv_arc_set_value(a, 0);
        lv_obj_set_style_arc_width(a, 4, LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(a, true, LV_PART_MAIN);
        lv_obj_set_style_arc_color(a, lv_color_hex(0x66BB6A), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_width(a, 0, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_obj_set_style_opa(a, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_clear_flag(a, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_width(a, 0, LV_PART_KNOB);
        lv_obj_set_style_height(a, 0, LV_PART_KNOB);
        w->arc[i] = a;
    }
    /* Πράσινη/κόκκινη τελεία κάτω από το μικρό τόξο — ρητό χρώμα, όχι default λευκό */
    w->dot = lv_obj_create(w->cont);
    lv_obj_set_size(w->dot, 9, 9);
    lv_obj_set_style_radius(w->dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(w->dot, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(w->dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(w->dot, lv_color_hex(0x66BB6A), LV_PART_MAIN);
    lv_obj_clear_flag(w->dot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(w->dot, LV_OBJ_FLAG_CLICKABLE);
    /*
     * Προηγουμένως OUT_BOTTOM_MID+5 έβγαζε την τελεία πολύ χαμηλά (πάνω στα HOME/ESP32).
     * Κέντρο μικρού τόξου + 2px κάτω ≈ βάση σήματος.
     */
    lv_obj_align_to(w->dot, w->arc[0], LV_ALIGN_CENTER, 0, 2);
    lv_obj_move_foreground(w->dot);
    squareline_wifi_apply_visual(w);
    lv_obj_move_foreground(w->cont);
}

/** Μόνιμο: `__DATE__`/`__TIME__` αλλάζουν κάθε build — αν δεν αλλάζει η γραμμή κάτω-δεξιά, δεν έγινε flash στη σωστή οθόνη/COM. */
static void squareline_permanent_fw_marker()
{
    if (!ui_Romeos1) {
        return;
    }
    char ver[44];
    std::snprintf(ver,
                  sizeof(ver),
                  "Build %s %s",
                  __DATE__,
                  __TIME__);
    lv_obj_t *lbl = lv_label_create(ui_Romeos1);
    lv_label_set_text(lbl, ver);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xECEFF1), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lbl, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(lbl, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_move_foreground(lbl);
}

static void squareline_corners_wifi_init()
{
    squareline_corner_degree_labels_style();
    /* Στατικά PNG κρυφά — μόνο δυναμικά τόξα (ίδιες θέσεις με αρχικό σχέδιο). */
    if (ui_Image10) {
        lv_obj_add_flag(ui_Image10, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui_Image2) {
        lv_obj_add_flag(ui_Image2, LV_OBJ_FLAG_HIDDEN);
    }
    s_wifi_home.connected = false;
    s_wifi_home.step = 0;
    s_wifi_home_arc_level = 1;
    s_wifi_home_arc_dir = 1;
    s_wifi_esp.connected = romeos_display_link_is_connected();
    s_wifi_arc_level = 1;
    s_wifi_arc_dir = 1;
    s_wifi_esp.step = s_wifi_esp.connected ? 1 : 0;
    squareline_wifi_build_one(&s_wifi_home, -324, 56, ui_room3);
    squareline_wifi_build_one(&s_wifi_esp, -29, 54, ui_room4);
    squareline_wifi_apply_visual(&s_wifi_home);
    squareline_wifi_apply_visual(&s_wifi_esp);
    if (ui_room3) {
        lv_obj_move_foreground(ui_room3);
    }
    if (ui_room4) {
        lv_obj_move_foreground(ui_room4);
    }
    lv_timer_create(squareline_wifi_link_status_timer_cb, k_wifi_status_timer_ms, nullptr);
    squareline_refresh_corner_placeholders();
    squareline_permanent_fw_marker();
}

/* --- Επάνω δεξιά: αντλία θερμότητας (Image5+6), κυκλοφορητές 1/2 (Image3+Label10, Image4+Label2) — από μητρική όταν συνδεδεμένο --- */
static constexpr uint32_t k_pump_spin_timer_ms = 100;
static constexpr uint32_t k_pump_green_rgb = 0xA5D6A7;
static constexpr uint32_t k_pump_red_rgb = 0xFFAB91;
static uint16_t s_pump_spin_angle_01 = 0;

static void squareline_pump_img_recolor(lv_obj_t *img, uint32_t rgb)
{
    if (!img) {
        return;
    }
    lv_obj_set_style_img_recolor(img, lv_color_hex(rgb), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
}

static void squareline_pump_img_pivot_center(lv_obj_t *img)
{
    if (!img) {
        return;
    }
    lv_obj_update_layout(img);
    const lv_coord_t w = lv_obj_get_width(img);
    const lv_coord_t h = lv_obj_get_height(img);
    lv_img_set_pivot(img, w / 2, h / 2);
}

static void squareline_top_right_pumps_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (romeos_display_link_is_connected()) {
        const bool hp_on = g_heat_pump_on;
        const bool c1_on = g_pump1_on;
        const bool c2_on = g_pump2_on;
        const bool any_spin = hp_on || c1_on || c2_on;
        if (any_spin) {
            s_pump_spin_angle_01 =
                static_cast<uint16_t>((s_pump_spin_angle_01 + 72u) % 3600u);
        }
        if (hp_on) {
            lv_img_set_angle(ui_Image5, s_pump_spin_angle_01);
            lv_img_set_angle(ui_Image6, s_pump_spin_angle_01);
            squareline_pump_img_recolor(ui_Image5, k_pump_green_rgb);
            squareline_pump_img_recolor(ui_Image6, k_pump_green_rgb);
        } else {
            lv_img_set_angle(ui_Image5, 0);
            lv_img_set_angle(ui_Image6, 0);
            squareline_pump_img_recolor(ui_Image5, k_pump_red_rgb);
            squareline_pump_img_recolor(ui_Image6, k_pump_red_rgb);
        }
        if (c1_on) {
            lv_img_set_angle(ui_Image3, s_pump_spin_angle_01);
            squareline_pump_img_recolor(ui_Image3, k_pump_green_rgb);
            squareline_set_label_color32(ui_Label10, k_pump_green_rgb);
        } else {
            lv_img_set_angle(ui_Image3, 0);
            squareline_pump_img_recolor(ui_Image3, k_pump_red_rgb);
            squareline_set_label_color32(ui_Label10, k_pump_red_rgb);
        }
        if (c2_on) {
            lv_img_set_angle(ui_Image4, s_pump_spin_angle_01);
            squareline_pump_img_recolor(ui_Image4, k_pump_green_rgb);
            squareline_set_label_color32(ui_Label2, k_pump_green_rgb);
        } else {
            lv_img_set_angle(ui_Image4, 0);
            squareline_pump_img_recolor(ui_Image4, k_pump_red_rgb);
            squareline_set_label_color32(ui_Label2, k_pump_red_rgb);
        }
        return;
    }
    /* Χωρίς ζωντανό link: όλα off, κόκκινο — χωρίς ντεμο. */
    lv_img_set_angle(ui_Image5, 0);
    lv_img_set_angle(ui_Image6, 0);
    squareline_pump_img_recolor(ui_Image5, k_pump_red_rgb);
    squareline_pump_img_recolor(ui_Image6, k_pump_red_rgb);
    lv_img_set_angle(ui_Image3, 0);
    squareline_pump_img_recolor(ui_Image3, k_pump_red_rgb);
    squareline_set_label_color32(ui_Label10, k_pump_red_rgb);
    lv_img_set_angle(ui_Image4, 0);
    squareline_pump_img_recolor(ui_Image4, k_pump_red_rgb);
    squareline_set_label_color32(ui_Label2, k_pump_red_rgb);
}

static void squareline_top_right_pumps_init()
{
    if (!ui_Romeos1) {
        return;
    }
    s_pump_spin_angle_01 = 0;
    squareline_pump_img_pivot_center(ui_Image3);
    squareline_pump_img_pivot_center(ui_Image4);
    squareline_pump_img_pivot_center(ui_Image5);
    squareline_pump_img_pivot_center(ui_Image6);
    squareline_top_right_pumps_timer_cb(nullptr);
    lv_timer_create(squareline_top_right_pumps_timer_cb, k_pump_spin_timer_ms, nullptr);
}

/* --- Δεξιά στήλη: 4 θερμοκρασίες + boiler — από μητρική όταν έγκυρα δεδομένα, αλλιώς «--» / «—» --- */

static void squareline_apply_right_temp_row(lv_obj_t *num_lbl, lv_obj_t *deg_lbl, lv_obj_t *name_lbl, float temp_c)
{
    if (!num_lbl || !deg_lbl || !name_lbl) {
        return;
    }
    if (!std::isfinite(temp_c)) {
        squareline_apply_right_temp_row_placeholder(num_lbl, deg_lbl, name_lbl);
        return;
    }
    const uint32_t rgb = temp_zone_color_rgb24(temp_c);
    int vi = static_cast<int>(temp_c + (temp_c >= 0.0f ? 0.5f : -0.5f));
    vi = std::clamp(vi, -20, 120);
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%d", vi);
    lv_label_set_text(num_lbl, buf);
    squareline_set_label_color32(num_lbl, rgb);
    squareline_set_label_color32(deg_lbl, rgb);
    squareline_set_label_color32(name_lbl, rgb);
}

static void squareline_refresh_right_temp_labels()
{
    squareline_apply_right_temp_row(ui_Solar, ui_solarO, ui_Solar2, g_temp_solar_c);
    squareline_apply_right_temp_row(ui_Boiler, ui_boilerO2, ui_Boiler3, g_temp_boiler_c);
    squareline_apply_right_temp_row(ui_Intel2, ui_intelO3, ui_Boiler2, g_temp_supply_c);
    squareline_apply_right_temp_row(ui_Outlet3, ui_outletO2, ui_autlet4, g_temp_return_c);
}

static void squareline_thermal_ui_tick_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    const bool conn = romeos_display_link_is_connected();
    const bool tel = romeos_display_link_telemetry_valid();
    if (!conn) {
        if (!room_corner_ui_has_measurement()) {
            squareline_refresh_corner_placeholders();
        } else {
            squareline_refresh_corner_readouts();
        }
        squareline_refresh_right_temp_placeholders();
        squareline_refresh_boiler_row(true);
        return;
    }
    if (!tel) {
        if (!room_corner_ui_has_measurement()) {
            squareline_refresh_corner_placeholders();
        } else {
            squareline_refresh_corner_readouts();
        }
        squareline_refresh_right_temp_placeholders();
        squareline_refresh_boiler_row(false);
        return;
    }
    squareline_refresh_corner_readouts();
    squareline_refresh_right_temp_labels();
    squareline_refresh_boiler_row(false);
}

static void squareline_room_temp_on_sensor_ui_refresh()
{
    if (lvgl_port_lock(50)) {
        refresh_numbers();
        lvgl_port_unlock();
    }
}

static void squareline_thermal_ui_tick_init()
{
    if (!ui_Romeos1) {
        return;
    }
    romeos_room_temp_sensor_set_ui_refresh_callback(squareline_room_temp_on_sensor_ui_refresh);
    squareline_thermal_ui_tick_cb(nullptr);
    lv_timer_create(squareline_thermal_ui_tick_cb, k_thermal_ui_tick_ms, nullptr);
}

static void squareline_alarm_init();

static void squareline_bind_romeos_controls()
{
    /* Λευκό «O» του σχεδίου δίπλα στο setpoint — κρύβεται, το ° είναι στο ui_CenterC με ίδιο χρώμα. */
    if (ui_LabelO) {
        lv_obj_add_flag(ui_LabelO, LV_OBJ_FLAG_HIDDEN);
    }

    if (ui_LabelPlin) {
        lv_obj_add_flag(ui_LabelPlin, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(ui_LabelPlin, 24);
        lv_obj_add_event_cb(ui_LabelPlin, on_minus, LV_EVENT_CLICKED, nullptr);
    }
    if (ui_LabelSin) {
        lv_obj_add_flag(ui_LabelSin, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(ui_LabelSin, 24);
        lv_obj_add_event_cb(ui_LabelSin, on_plus, LV_EVENT_CLICKED, nullptr);
    }

    if (ui_Label3) {
        lv_obj_clear_flag(ui_Label3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Label3, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(ui_Label3, 12);
        lv_obj_add_event_cb(ui_Label3, squareline_on_clock_ampm_clicked, LV_EVENT_CLICKED, nullptr);
    }

    if (ui_Label11) {
        lv_obj_add_flag(ui_Label11, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(ui_Label11, 12);
        lv_obj_add_event_cb(ui_Label11, squareline_on_clock_hours_clicked, LV_EVENT_CLICKED, nullptr);
    }
    if (ui_Label13) {
        lv_obj_add_flag(ui_Label13, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(ui_Label13, 12);
        lv_obj_add_event_cb(ui_Label13, squareline_on_clock_minutes_clicked, LV_EVENT_CLICKED, nullptr);
    }

    if (ui_Image8) {
        lv_obj_add_flag(ui_Image8, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(ui_Image8, 20);
        lv_obj_add_event_cb(ui_Image8, squareline_on_arrow_left, LV_EVENT_CLICKED, nullptr);
    }
    if (ui_Image9) {
        lv_obj_add_flag(ui_Image9, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(ui_Image9, 20);
        lv_obj_add_event_cb(ui_Image9, squareline_on_arrow_right, LV_EVENT_CLICKED, nullptr);
    }

    squareline_refresh_clock();
    lv_timer_create(squareline_clock_tick_cb, 1000, nullptr);
    lv_timer_create(squareline_clock_blink_cb, 500, nullptr);

    squareline_alarm_init();
    squareline_corners_wifi_init();
    squareline_top_right_pumps_init();
    squareline_thermal_ui_tick_init();
}

static void squareline_clock_ui_init_after_load()
{
    squareline_load_clock_from_nvs();
    squareline_clock_rebase_to_current_hms();
    squareline_refresh_clock();
}

/* --- Συναγερμός (ui_Image7): χωρίς αυτόματο ντεμο· κόκκινη κατάσταση από μελλοντική λογική μητρικής · πάτημα = επαναφορά --- */
static constexpr uint32_t k_alarm_green_rgb = 0x81C784;  /* ανοιχτό πράσινο */
static constexpr uint32_t k_alarm_red_rgb = 0xC62828;
static constexpr lv_opa_t k_alarm_overlay_flash_opa = LV_OPA_50;
static const int k_alarm_buzzer_pin = ROMEOS_ALARM_BUZZER_PIN;
static constexpr uint32_t k_alarm_buzzer_on_ms = 3u * 60u * 1000u;
static constexpr uint32_t k_alarm_buzzer_off_ms = 1u * 60u * 1000u;

static lv_obj_t *s_alarm_overlay = nullptr;
static lv_timer_t *s_alarm_blink_timer = nullptr;
static lv_timer_t *s_alarm_buzzer_timer = nullptr;
static bool s_alarm_ringing = false;
static bool s_alarm_overlay_flash_phase = false;
static bool s_alarm_fault_active = false;
static bool s_alarm_acknowledged = false;
static bool s_alarm_buzzer_enabled = false;
static bool s_alarm_buzzer_phase_on = true;
static uint32_t s_alarm_buzzer_phase_ms = 0;

static void squareline_alarm_set_icon_color(uint32_t rgb)
{
    if (!ui_Image7) {
        return;
    }
    lv_obj_set_style_img_recolor(ui_Image7, lv_color_hex(rgb), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(ui_Image7, LV_OPA_COVER, LV_PART_MAIN);
}

static void squareline_alarm_overlay_blink_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (!s_alarm_overlay || !s_alarm_ringing) {
        return;
    }
    s_alarm_overlay_flash_phase = !s_alarm_overlay_flash_phase;
    lv_obj_set_style_bg_opa(s_alarm_overlay,
        s_alarm_overlay_flash_phase ? k_alarm_overlay_flash_opa : LV_OPA_TRANSP,
        LV_PART_MAIN);
}

static void squareline_alarm_buzzer_write(bool on)
{
    if (k_alarm_buzzer_pin < 0) {
        return;
    }
    digitalWrite(k_alarm_buzzer_pin, on ? HIGH : LOW);
}

static void squareline_alarm_buzzer_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (!s_alarm_buzzer_enabled) {
        squareline_alarm_buzzer_write(false);
        return;
    }
    const uint32_t now = millis();
    const uint32_t phase_ms = s_alarm_buzzer_phase_on ? k_alarm_buzzer_on_ms : k_alarm_buzzer_off_ms;
    if (now - s_alarm_buzzer_phase_ms >= phase_ms) {
        s_alarm_buzzer_phase_ms = now;
        s_alarm_buzzer_phase_on = !s_alarm_buzzer_phase_on;
    }
    squareline_alarm_buzzer_write(s_alarm_buzzer_phase_on);
}

static void squareline_alarm_buzzer_enable(bool enabled)
{
    s_alarm_buzzer_enabled = enabled;
    s_alarm_buzzer_phase_on = true;
    s_alarm_buzzer_phase_ms = millis();
    squareline_alarm_buzzer_timer_cb(nullptr);
}

static void squareline_alarm_overlay_eat_touch(lv_event_t *e)
{
    LV_UNUSED(e);
    /* Καταναλώνει την αφή στο κόκκινο layer· το εικονίδιο είναι πιο μπροστά και πατιέται κανονικά. */
}

static void squareline_alarm_stop_ringers_ui()
{
    if (s_alarm_blink_timer) {
        lv_timer_del(s_alarm_blink_timer);
        s_alarm_blink_timer = nullptr;
    }
    if (s_alarm_overlay) {
        lv_obj_set_style_bg_opa(s_alarm_overlay, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_add_flag(s_alarm_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    s_alarm_ringing = false;
    s_alarm_overlay_flash_phase = false;
}

static void squareline_alarm_enter_alarm_state()
{
    if (!ui_Romeos1 || !ui_Image7) {
        return;
    }
    s_alarm_ringing = true;
    squareline_alarm_set_icon_color(k_alarm_red_rgb);

    if (!s_alarm_overlay) {
        s_alarm_overlay = lv_obj_create(ui_Romeos1);
        lv_obj_set_size(s_alarm_overlay, LV_PCT(100), LV_PCT(100));
        lv_obj_align(s_alarm_overlay, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_bg_color(s_alarm_overlay, lv_color_hex(0xE53935), LV_PART_MAIN);
        lv_obj_set_style_border_width(s_alarm_overlay, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(s_alarm_overlay, 0, LV_PART_MAIN);
        lv_obj_clear_flag(s_alarm_overlay, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(s_alarm_overlay, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(s_alarm_overlay, squareline_alarm_overlay_eat_touch, LV_EVENT_PRESSED, nullptr);
        lv_obj_add_event_cb(s_alarm_overlay, squareline_alarm_overlay_eat_touch, LV_EVENT_CLICKED, nullptr);
    }
    lv_obj_clear_flag(s_alarm_overlay, LV_OBJ_FLAG_HIDDEN);
    s_alarm_overlay_flash_phase = true;
    lv_obj_set_style_bg_opa(s_alarm_overlay, k_alarm_overlay_flash_opa, LV_PART_MAIN);
    lv_obj_move_foreground(s_alarm_overlay);
    lv_obj_move_foreground(ui_Image7);

    if (!s_alarm_blink_timer) {
        s_alarm_blink_timer = lv_timer_create(squareline_alarm_overlay_blink_cb, 500, nullptr);
    }
}

static void squareline_alarm_on_icon_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (!s_alarm_ringing) {
        return;
    }
    squareline_alarm_stop_ringers_ui();
    squareline_alarm_buzzer_enable(false);
    s_alarm_acknowledged = true;
    squareline_alarm_set_icon_color(k_alarm_green_rgb);
}

static bool s_alarm_demo_forced = false;

static void squareline_alarm_apply_remote_state(bool active)
{
    if (active && !s_alarm_fault_active) {
        s_alarm_acknowledged = false;
    }
    s_alarm_fault_active = active;
    if (!active) {
        if (s_alarm_demo_forced) {
            /* Το demo είναι τοπικό — η μητρική μπορεί να στέλνει alarm=0 ανά πάσα στιγμή. */
            return;
        }
        s_alarm_acknowledged = false;
        squareline_alarm_stop_ringers_ui();
        squareline_alarm_buzzer_enable(false);
        squareline_alarm_set_icon_color(k_alarm_green_rgb);
        return;
    }
    if (s_alarm_acknowledged) {
        return;
    }
    if (!s_alarm_ringing) {
        squareline_alarm_enter_alarm_state();
    }
    squareline_alarm_buzzer_enable(true);
}

static void squareline_alarm_set_demo_forced(bool enabled)
{
    s_alarm_demo_forced = enabled;
    if (enabled) {
        s_alarm_acknowledged = false;
        squareline_alarm_apply_remote_state(true);
        Serial.println(F("[alarm_demo] FORCED ON"));
    } else {
        s_alarm_acknowledged = false;
        s_alarm_fault_active = false;
        squareline_alarm_stop_ringers_ui();
        squareline_alarm_buzzer_enable(false);
        squareline_alarm_set_icon_color(k_alarm_green_rgb);
        Serial.println(F("[alarm_demo] FORCED OFF"));
    }
}

static void poll_alarm_demo_serial()
{
    static String s_line;
    while (Serial.available() > 0) {
        const char ch = static_cast<char>(Serial.read());
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            String line = s_line;
            s_line = "";
            line.trim();
            line.toUpperCase();
            if (line == "ALARM_DEMO_ON") {
                squareline_alarm_set_demo_forced(true);
            } else if (line == "ALARM_DEMO_OFF") {
                squareline_alarm_set_demo_forced(false);
            }
            continue;
        }
        if (s_line.length() < 48) {
            s_line += ch;
        }
    }
}

static void squareline_alarm_init()
{
    if (!ui_Image7) {
        return;
    }
    lv_obj_add_flag(ui_Image7, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(ui_Image7, 20);
    lv_obj_add_event_cb(ui_Image7, squareline_alarm_on_icon_clicked, LV_EVENT_CLICKED, nullptr);
    if (k_alarm_buzzer_pin >= 0) {
        pinMode(k_alarm_buzzer_pin, OUTPUT);
        squareline_alarm_buzzer_write(false);
    }
    s_alarm_buzzer_timer = lv_timer_create(squareline_alarm_buzzer_timer_cb, 200, nullptr);
    LV_UNUSED(s_alarm_buzzer_timer);
    squareline_alarm_stop_ringers_ui();
    squareline_alarm_buzzer_enable(false);
    squareline_alarm_set_icon_color(k_alarm_green_rgb);
}
#endif

void demo_temp_timer(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (!std::isfinite(g_room_temp_c)) {
        g_room_temp_c = g_setpoint_c;
    }
    float diff = g_setpoint_c - g_room_temp_c;
    if (std::fabs(diff) < 0.05f) {
        g_room_temp_c = g_setpoint_c;
    } else {
        g_room_temp_c += diff * 0.03f;
    }
    refresh_numbers();
}

static void demo_system_panel_timer(lv_timer_t *t)
{
    LV_UNUSED(t);

    if (s_temp_ramp_dir > 0) {
        if (s_temp_ramp < 10) {
            ++s_temp_ramp;
        } else {
            s_temp_ramp_dir = -1;
        }
    } else {
        if (s_temp_ramp > 0) {
            --s_temp_ramp;
        } else {
            s_temp_ramp_dir = 1;
        }
    }

    const float d = static_cast<float>(s_temp_ramp);
    g_temp_solar_c = k_base_solar_c + d;
    g_temp_boiler_c = k_base_boiler_c + d;
    g_temp_supply_c = k_base_supply_c + d;
    g_temp_return_c = k_base_return_c + d;
    g_temp_outdoor_c = k_base_outdoor_c + d;

    ++s_onoff_sec_in_phase;
    if (s_onoff_sec_in_phase >= kOnOffHoldSec) {
        s_onoff_sec_in_phase = 0;
        s_onoff_demo_state = !s_onoff_demo_state;
    }
    g_heat_pump_on = s_onoff_demo_state;
    g_pump1_on = s_onoff_demo_state;
    g_pump2_on = s_onoff_demo_state;
    g_heater_on = s_onoff_demo_state;

    refresh_system_panel();
}

lv_obj_t *make_round_btn(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    const lv_coord_t k = 56;
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, k, k);
    lv_obj_set_style_radius(btn, k / 2, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(kAccentBtn), 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xa06430), LV_STATE_PRESSED);
    lv_obj_set_style_shadow_color(btn, lv_color_hex(0x15181c), 0);
    lv_obj_set_style_shadow_width(btn, 14, 0);
    lv_obj_set_style_shadow_spread(btn, 2, 0);
    lv_obj_set_style_shadow_ofs_x(btn, 5, 0);
    lv_obj_set_style_shadow_ofs_y(btn, 6, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_40, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(kTextPri), 0);
    lv_obj_center(lbl);
    return btn;
}

#if 0 && DEMO_PLACEHOLDER_LABELS
static constexpr uint32_t kDemoTextCream = 0xe8e6e3;

/*
 * Bottom row buttons + top-right 3 icons are drawn in ui_reference.png — same size
 * there needs a PNG edit (GIMP etc.); LVGL cannot resize one baked-in button/icon.
 */
static void demo_lbl(
    lv_obj_t *parent, const char *txt, lv_coord_t x, lv_coord_t y, const lv_font_t *font)
{
    lv_obj_t *lb = lv_label_create(parent);
    lv_label_set_text(lb, txt);
    lv_obj_set_style_text_font(lb, font, 0);
    lv_obj_set_style_text_color(lb, lv_color_hex(kDemoTextCream), 0);
    lv_obj_set_style_bg_opa(lb, LV_OPA_TRANSP, 0);
    lv_obj_set_pos(lb, x, y);
    lv_obj_clear_flag(lb, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(lb);
}

static void build_demo_placeholder_labels(lv_obj_t *scr)
{
    /* --- Centre of gauge: largest built-in Montserrat (48); centred in dial box --- */
    static constexpr lv_coord_t kDialBoxX = 62;
    static constexpr lv_coord_t kDialBoxY = 38;
    static constexpr lv_coord_t kDialBoxW = 276;
    static constexpr lv_coord_t kDialBoxH = 276;

    lv_obj_t *dial = lv_obj_create(scr);
    lv_obj_set_size(dial, kDialBoxW, kDialBoxH);
    lv_obj_set_pos(dial, kDialBoxX, kDialBoxY);
    lv_obj_set_style_bg_opa(dial, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(dial, 0, 0);
    lv_obj_clear_flag(dial, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *main_deg = lv_label_create(dial);
    lv_label_set_text(main_deg, "24\xc2\xb0");
    lv_obj_set_style_text_font(main_deg, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(main_deg, lv_color_hex(kDemoTextCream), 0);
    lv_obj_center(main_deg);
    lv_obj_move_foreground(dial);

    /* --- Top-left / top-right of left card (~2x previous demo sizes) --- */
    demo_lbl(scr, "21\xc2\xb0", 24, 18, &lv_font_montserrat_40);
    demo_lbl(scr, "ROOM", 24, 62, &lv_font_montserrat_22);
    demo_lbl(scr, "8\xc2\xb0", 318, 18, &lv_font_montserrat_40);
    demo_lbl(scr, "OUT", 318, 62, &lv_font_montserrat_22);

    /* --- Right list: value in light pill (right a bit), name in track (left a bit), y tuned --- */
    static constexpr lv_coord_t kRowValX = 458;
    static constexpr lv_coord_t kRowNameX = 548;
    static constexpr lv_coord_t kRowY0 = 100;
    static constexpr lv_coord_t kRowDy = 60;

    demo_lbl(scr, "85\xc2\xb0""C", kRowValX, kRowY0 + 0 * kRowDy, &lv_font_montserrat_48);
    demo_lbl(scr, "SOLAR", kRowNameX, kRowY0 + 0 * kRowDy, &lv_font_montserrat_40);

    demo_lbl(scr, "64\xc2\xb0""C", kRowValX, kRowY0 + 1 * kRowDy, &lv_font_montserrat_48);
    demo_lbl(scr, "BOILER", kRowNameX, kRowY0 + 1 * kRowDy, &lv_font_montserrat_40);

    demo_lbl(scr, "42\xc2\xb0""C", kRowValX, kRowY0 + 2 * kRowDy, &lv_font_montserrat_48);
    demo_lbl(scr, "INTEL", kRowNameX, kRowY0 + 2 * kRowDy, &lv_font_montserrat_40);

    demo_lbl(scr, "35\xc2\xb0""C", kRowValX, kRowY0 + 3 * kRowDy, &lv_font_montserrat_48);
    demo_lbl(scr, "OUTLET", kRowNameX, kRowY0 + 3 * kRowDy, &lv_font_montserrat_40);

    demo_lbl(scr, "ON", kRowValX, kRowY0 + 4 * kRowDy, &lv_font_montserrat_48);
    demo_lbl(scr, "BOILER", kRowNameX, kRowY0 + 4 * kRowDy, &lv_font_montserrat_40);
}
#endif

static void on_mb_from_link(const romeos_mb_to_display_v1_t *p)
{
    /* Ίδια λογική με romeos_display_link_telemetry_valid() (debounce streak), όχι μόνο flags ενός πακέτου. */
    const bool tel = romeos_display_link_telemetry_valid();
    const bool mb_right_temps_demo =
        (p->mb_proto_reserved & 0x01u) != 0u; /* μητρική: σταθερά placeholders, όχι πραγματικοί αισθητήρες */
    const bool alarm_active = (p->flags & ROMEOS_MB_FLAG_ALARM_ACTIVE) != 0u;
    g_heat_pump_on = p->heat_pump_on != 0;
    g_pump1_on = p->pump1_on != 0;
    g_pump2_on = p->pump2_on != 0;
    g_heater_on = p->heater_on != 0;

#if ROMEOS_ROOM_USE_MB_FALLBACK
    static bool s_logged_mb_room_ui = false;
    if (g_room_sensor_ready && (romeos_room_temp_sensor_has_live_sample() || g_room_local_valid)) {
        s_room_corner_from_mb = false;
    }
    if (!tel) {
        s_room_corner_from_mb = false;
        if (!g_room_sensor_ready && !romeos_room_temp_sensor_has_live_sample() && !g_room_local_valid) {
            g_room_temp_c = std::nanf("");
        }
    } else if (tel && mb_right_temps_demo) {
        s_room_corner_from_mb = false;
        if (!g_room_sensor_ready && !romeos_room_temp_sensor_has_live_sample() && !g_room_local_valid) {
            g_room_temp_c = std::nanf("");
        }
    } else if (tel && !mb_right_temps_demo && !g_room_sensor_ready) {
        const int16_t rdx = p->room_display_c_x10;
        if (rdx > -150 && rdx < 450) {
            g_room_temp_c = rdx / 10.0f;
            s_room_corner_from_mb = true;
            if (!s_logged_mb_room_ui) {
                s_logged_mb_room_ui = true;
                Serial.println(
                    F("[room_temp] ROOM από μητρική (room_display) — δεν ανιχνεύτηκε SHT/AHT στο I2C της οθόνης."));
            }
        }
    }
#endif

    if (tel) {
        /* Δεξιά/OUT: τηλεμετρία MB. Γωνία ROOM: SHT οθόνης ή (fallback) room_display όταν ROMEOS_ROOM_USE_MB_FALLBACK. */
        if (mb_right_temps_demo) {
            g_temp_outdoor_c = NAN;
            g_temp_solar_c = NAN;
            g_temp_boiler_c = NAN;
            g_temp_supply_c = NAN;
            g_temp_return_c = NAN;
        } else {
            g_temp_outdoor_c = p->outdoor_c_x10 / 10.0f;
            g_temp_solar_c = p->solar_c_x10 / 10.0f;
            g_temp_boiler_c = p->boiler_c_x10 / 10.0f;
            g_temp_supply_c = p->supply_c_x10 / 10.0f;
            g_temp_return_c = p->return_c_x10 / 10.0f;
        }
    }

    /* v2 MB→οθόνη: setpoint — μετά +/- αγνοούμε παλιό spx μέχρι echo· μετά timeout όχι snap σε παλιά τιμή. */
    if (p->version >= 2u) {
        const int16_t spx = p->setpoint_c_x10;
        if (spx >= 50 && spx <= 350) {
            const float next = spx / 10.0f;
            bool apply_mb = true;
            if (s_mb_setpoint_pending_ack_x10 >= 0) {
                const uint32_t age = millis() - s_mb_setpoint_pending_since_ms;
                if (age < 20000u) {
                    if (std::abs(static_cast<int>(spx) - static_cast<int>(s_mb_setpoint_pending_ack_x10)) >
                        1) {
                        apply_mb = false;
                    } else {
                        s_mb_setpoint_pending_ack_x10 = -1;
                    }
                } else {
                    s_mb_setpoint_pending_ack_x10 = -1;
                    if (std::fabs(next - g_setpoint_c) > 0.05f) {
                        apply_mb = false;
                    }
                }
            }
            if (apply_mb && next != g_setpoint_c) {
                g_setpoint_c = next;
                romeos_display_link_on_setpoint_ui_changed();
            }
        }
    }

    lvgl_port_lock(-1);
#if USE_SQUARELINE_UI
    if (!s_alarm_demo_forced) {
        squareline_alarm_apply_remote_state(alarm_active);
    }
    if (!tel) {
        squareline_refresh_corner_placeholders();
        squareline_refresh_right_temp_placeholders();
        squareline_refresh_boiler_row(false);
    } else {
        /* Όταν mb_right_temps_demo, τα g_temp_* είναι NaN → «--» στα δεξιά / OUT. */
        squareline_refresh_corner_readouts();
        squareline_refresh_right_temp_labels();
        squareline_refresh_boiler_row(false);
    }
#else
    refresh_system_panel();
#endif
    refresh_numbers();
    lvgl_port_unlock();
}

static int16_t get_sp_x10_for_link()
{
    int tenths = static_cast<int>(g_setpoint_c * 10.0f +
                                   (g_setpoint_c >= 0.0f ? 0.5f : -0.5f));
    if (tenths < 50) {
        tenths = 50;
    }
    if (tenths > 350) {
        tenths = 350;
    }
    return static_cast<int16_t>(tenths);
}

static int16_t get_room_x10_for_link()
{
    if ((!romeos_room_temp_sensor_has_live_sample() && !g_room_local_valid) ||
        !std::isfinite(g_room_temp_c)) {
        return ROMEOS_ROOM_FROM_DISPLAY_INVALID;
    }
    int v = static_cast<int>(g_room_temp_c * 10.0f + (g_room_temp_c >= 0.0f ? 0.5f : -0.5f));
    v = std::clamp(v, 0, 450);
    return static_cast<int16_t>(v);
}

void build_ui()
{
#if USE_SQUARELINE_UI
    ui_init();
    g_lbl_setpoint_main = ui_CenterC;
    refresh_numbers();
    squareline_clock_ui_init_after_load();
    squareline_bind_romeos_controls();
    return;
#else

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(kBgScreen), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *root = lv_obj_create(scr);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(root, kRootPadLeftPx, 0);
    lv_obj_set_style_pad_right(root, 10, 0);
    lv_obj_set_style_pad_top(root, 1, 0);
    lv_obj_set_style_pad_bottom(root, 1, 0);
    lv_obj_set_style_pad_row(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);

    lv_obj_t *main_row = lv_obj_create(root);
    lv_obj_set_width(main_row, LV_PCT(100));
    lv_obj_set_flex_grow(main_row, 1);
    /* No row flex — children positioned in apply_main_row_layout (flex was resetting x) */
    lv_obj_set_style_pad_column(main_row, 0, 0);
    lv_obj_set_style_bg_opa(main_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_row, 0, 0);
    lv_obj_clear_flag(main_row, LV_OBJ_FLAG_SCROLLABLE);

    /* ----- Left column ----- */
    lv_obj_t *left_col = lv_obj_create(main_row);
    lv_obj_set_style_layout(left_col, 0, 0);
    lv_obj_set_style_bg_opa(left_col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left_col, 0, 0);
    lv_obj_clear_flag(left_col, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(left_col);
    lv_label_set_text(title, "CLIMATE");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(kTextSec), 0);
    lv_obj_set_style_text_letter_space(title, 2, 0);

    lv_obj_t *thermo = lv_obj_create(left_col);
    apply_neumorph_panel(thermo, kCard);
    lv_obj_set_flex_flow(thermo, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(thermo, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(thermo, 12, 0);
    lv_obj_clear_flag(thermo, LV_OBJ_FLAG_SCROLLABLE);

    g_arc_temp = nullptr;
    g_lbl_setpoint_main = nullptr;
    g_lbl_room_sub = nullptr;

    lv_obj_t *thermo_only = lv_label_create(thermo);
    lv_label_set_text(thermo_only, "TEST");
    lv_obj_set_style_text_font(thermo_only, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(thermo_only, lv_color_hex(0xffffff), 0);

    /*
     * LOCKED (final): right-hand “system status” panel — layout (full column height),
     * width 256, positions, sizes, fonts, colours, demo timers/recolour.
     * Do not change unless explicitly reopened.
     */
    /* ----- Right: system status (temps + on/off) ----- */
    lv_obj_t *usage = lv_obj_create(main_row);
    apply_neumorph_panel(usage, kCard);
    /* No left border — avoids thin vertical highlight line beside the left column */
    lv_obj_set_style_border_side(usage, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_flex_flow(usage, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(usage, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(usage, 8, 0);
    lv_obj_set_style_pad_row(usage, 2, 0);
    lv_obj_clear_flag(usage, LV_OBJ_FLAG_SCROLLABLE);

    static const char *const k_sys_names[] = {
        "Solar",
        "Boiler",
        "Supply",
        "Return",
        "Out",
        "Heat Pump",
        "Pump 1",
        "Pump 2",
        "Heater",
    };
    for (int i = 0; i < 9; i++) {
        lv_obj_t *r = lv_obj_create(usage);
        lv_obj_set_width(r, LV_PCT(100));
        lv_obj_set_height(r, 46);
        lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(r, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(r, 0, 0);
        lv_obj_t *nm = lv_label_create(r);
        lv_label_set_text(nm, k_sys_names[i]);
        lv_obj_set_style_text_font(nm, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(nm, lv_color_hex(kTextPri), 0);
        lv_obj_set_flex_grow(nm, 1);
        lv_obj_t *val = lv_label_create(r);
        lv_obj_set_style_text_font(val, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_align(val, LV_TEXT_ALIGN_RIGHT, 0);
        if (i < 5) {
            lv_label_set_recolor(val, true);
            lv_obj_set_style_text_color(val, lv_color_hex(kTextPri), 0);
        } else {
            lv_label_set_recolor(val, false);
        }
        g_sys_value_lbls[i] = val;
    }

    /* Thin bottom bar: child of main_row so bottom edge = main_row bottom (= usage bottom). */
    lv_obj_t *scene_bar = lv_obj_create(main_row);
    lv_obj_set_height(scene_bar, kSceneBarH);
    apply_neumorph_panel(scene_bar, kCardInner);
    lv_obj_clear_flag(scene_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(main_row, on_main_row_size, LV_EVENT_SIZE_CHANGED, nullptr);
    lv_obj_add_flag(main_row, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_flag(root, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_update_layout(root);
    apply_main_row_layout(main_row);
    layout_left_column(left_col);
    position_scene_bar(main_row, scene_bar);
    lv_obj_update_layout(root);

    refresh_numbers();
    refresh_system_panel();
    lv_timer_create(demo_temp_timer, 500, nullptr);
    lv_timer_create(demo_system_panel_timer, 1000, nullptr);
#endif
}

#if USE_SQUARELINE_UI
static void ntp_apply_wall_time(const struct tm *tm)
{
    if (!tm) {
        return;
    }
    s_clock_h24 = static_cast<uint8_t>(tm->tm_hour % 24);
    s_clock_min = static_cast<uint8_t>(tm->tm_min % 60);
    s_clock_sec = static_cast<uint8_t>(tm->tm_sec % 60);
    squareline_clock_rebase_to_current_hms();
    if (lvgl_port_lock(50)) {
        squareline_refresh_clock();
        lvgl_port_unlock();
    }
    squareline_save_clock_to_nvs();
}
#endif

} // namespace

void setup()
{
#ifndef ROMEOS_DISABLE_BOOT_BEEP
    /* Απόδειξη ότι τρέχει **αυτό** το firmware χωρίς να βασίζεσαι στο Serial (ενεργό buzzer GPIO17). */
    if (ROMEOS_ALARM_BUZZER_PIN >= 0) {
        pinMode(ROMEOS_ALARM_BUZZER_PIN, OUTPUT);
        for (int i = 0; i < 3; ++i) {
            digitalWrite(ROMEOS_ALARM_BUZZER_PIN, HIGH);
            delay(45);
            digitalWrite(ROMEOS_ALARM_BUZZER_PIN, LOW);
            delay(90);
        }
    }
#endif
    Serial.begin(115200);
    /* Χρόνος ώστε να προλάβει να ανοίξει το Serial Monitor πριν χαθούν οι πρώτες γραμμές. */
    delay(800);
    Serial.println();
    Serial.println(F("======== ROMEOS VIEWE (thermostat-ui-demo) ========"));
    Serial.printf("Firmware build: %s %s\n", __DATE__, __TIME__);
    Serial.printf("ROOM pins SDA=GPIO%d SCL=GPIO%d  shared_touch_I2C0=%d\n",
                  ROMEOS_ROOM_I2C_SDA,
                  ROMEOS_ROOM_I2C_SCL,
                  ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS);
#if defined(ARDUINO_USB_CDC_ON_BOOT) && ARDUINO_USB_CDC_ON_BOOT
    Serial.println(F("NOTE: USB_CDC_ON_BOOT=1 -> Serial on USB device port, not CH340/COM."));
#else
    Serial.println(F("Serial -> UART0 (CH340). Open that COM in monitor."));
#endif
    Serial.println(
        F("If STILL empty: try the OTHER COM for this USB (e.g. \"USB JTAG/serial\"), or wrong USB cable/port."));
    Serial.flush();
    Serial.println("Thermostat UI — neumorph dark");

    Serial.println("Initializing board");
    Board *board = new Board();
    board->init();
#if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 20);
    }
#endif
#endif
    assert(board->begin());

    romeos_room_temp_sensor_after_board_begin();

    Serial.println("Initializing LVGL");
    lvgl_port_init(board->getLCD(), board->getTouch());

    Serial.println("Building UI");
    lvgl_port_lock(-1);
    build_ui();
    lvgl_port_unlock();

#if USE_SQUARELINE_UI
    romeos_ntp_init(ntp_apply_wall_time);
#else
    romeos_ntp_init(nullptr);
#endif
    romeos_display_link_init(on_mb_from_link, get_sp_x10_for_link, get_room_x10_for_link);
    Serial.printf("UI build: %s %s (αν δεν αλλάζει μετά από κάθε compile, λάθος COM ή project)\n",
                  __DATE__,
                  __TIME__);

    Serial.println("Ready");
}

void loop()
{
    romeos_wifi_nvs_poll_serial();
    poll_alarm_demo_serial();
    romeos_display_link_poll();
    romeos_room_temp_sensor_poll_in_app_loop();
    delay(5);
}
