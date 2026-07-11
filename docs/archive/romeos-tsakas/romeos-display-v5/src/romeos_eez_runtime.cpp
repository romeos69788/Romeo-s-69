#include "romeos_eez_runtime.h"

#include <Arduino.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "driver/gpio.h"

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "othoni_ui/screens.h"
#include "othoni_ui/ui.h"
#include "romeos_display_link.h"
#include "romeos_ntp.h"
#include "romeos_room_sht_touch_bus.h"
#include "romeos_wifi_nvs.h"

#ifndef ROMEOS_V4_BUZZER_GPIO
#define ROMEOS_V4_BUZZER_GPIO 17
#endif
#ifndef ROMEOS_V4_BUZZER_ACTIVE_HIGH
#define ROMEOS_V4_BUZZER_ACTIVE_HIGH 1
#endif
#ifndef ROMEOS_V4_BUZZER_STYLE
#define ROMEOS_V4_BUZZER_STYLE 1
#endif

static void wifi_ui_apply(bool home);
static void wifi_ui_apply_unlocked(bool home);

namespace {

constexpr uint32_t k_col_cyan = 0x4DD0E1u;
constexpr uint32_t k_col_white = 0xFFFFFFu;
constexpr uint32_t k_col_red = 0xFF3B30u;
constexpr int k_sp_min_c = 10;
constexpr int k_sp_max_c = 30;
/** Arc 0…40 ↔ 10.0…30.0 °C (βήμα 0,5) — πρέπει να ταιριάζει με το πλήρες τόξο LVGL. */
constexpr int k_arc_min = 0;
constexpr int k_arc_max = 40;
constexpr time_t k_min_valid_epoch = 1609459200;
constexpr int kBuzzerGpio = ROMEOS_V4_BUZZER_GPIO;
constexpr bool kBuzzerActiveHigh = ROMEOS_V4_BUZZER_ACTIVE_HIGH != 0;

static bool s_alarm_active = false;
static bool s_have_wifi_prev = false;
static bool s_prev_home = false;
static uint32_t s_last_clock_ms = 0;
static uint32_t s_last_sensor_ms = 0;

static float s_setpoint = 22.5f;
static bool s_setpoint_dirty = true;
static bool s_arc_sync_muted = false;
static bool s_mb_link_prev = false;

static int setpoint_to_arc_value(float sp)
{
    int v = static_cast<int>(std::lround((sp - static_cast<float>(k_sp_min_c)) * 2.0f));
    if (v < k_arc_min) v = k_arc_min;
    if (v > k_arc_max) v = k_arc_max;
    return v;
}

static float arc_value_to_setpoint(int arc_val)
{
    if (arc_val < k_arc_min) arc_val = k_arc_min;
    if (arc_val > k_arc_max) arc_val = k_arc_max;
    return static_cast<float>(k_sp_min_c) + arc_val * 0.5f;
}

static const char *k_wdays[] = {
    "Κυριακη", "Δευτερα", "Τριτη", "Τεταρτη", "Πεμπτη", "Παρασκευη", "Σαββατο",
};
static const char *k_months[] = {
    "Ιανουαριου", "Φεβρουαριου", "Μαρτιου", "Απριλιου", "Μαϊου", "Ιουνιου",
    "Ιουλιου", "Αυγουστου", "Σεπτεμβρη", "Οκτωβριου", "Νοεμβριου", "Δεκεμβριου",
};

static void buzzer_gpio_write(bool on)
{
    if (kBuzzerGpio < 0) return;
    digitalWrite(kBuzzerGpio, (on == kBuzzerActiveHigh) ? HIGH : LOW);
}

static void buzzer_pin_init(void)
{
    if (kBuzzerGpio < 0) return;
    pinMode(kBuzzerGpio, OUTPUT);
    buzzer_gpio_write(false);
}

/** Ενημέρωση μόνο στην ορατή οθόνη — αλλιώς RGB mode 3 κάνει full/partial flicker. */
static bool obj_on_active_screen(lv_obj_t *obj)
{
    if (obj == nullptr || !lv_obj_is_valid(obj)) {
        return false;
    }
    return lv_obj_get_screen(obj) == lv_scr_act();
}

static void label_set_if_changed(lv_obj_t *lbl, const char *txt)
{
    if (!obj_on_active_screen(lbl) || txt == nullptr) return;
    const char *const ex = lv_label_get_text(lbl);
    if (ex != nullptr && std::strcmp(ex, txt) == 0) return;
    lv_label_set_text(lbl, txt);
}

static void set_text_color(lv_obj_t *lbl, uint32_t hex)
{
    if (!obj_on_active_screen(lbl)) return;
    const lv_color_t want = lv_color_hex(hex);
    const lv_color_t cur = lv_obj_get_style_text_color(lbl, LV_PART_MAIN);
    if (lv_color_to32(cur) != lv_color_to32(want)) {
        lv_obj_set_style_text_color(lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void set_text_color_force(lv_obj_t *lbl, uint32_t hex)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl)) return;
    const lv_color_t want = lv_color_hex(hex);
    const lv_color_t cur = lv_obj_get_style_text_color(lbl, LV_PART_MAIN);
    if (lv_color_to32(cur) != lv_color_to32(want)) {
        lv_obj_set_style_text_color(lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void humidity_label_set(lv_obj_t *lbl, const char *txt)
{
    label_set_if_changed(lbl, txt);
    set_text_color(lbl, k_col_cyan);
}

static void update_setpoint_display(void)
{
    if (!obj_on_active_screen(objects.obj0)) {
        s_setpoint_dirty = false;
        return;
    }

    int whole = static_cast<int>(s_setpoint);
    int frac = static_cast<int>(std::round((s_setpoint - whole) * 10.0f));
    if (frac >= 10) { frac = 0; whole++; }
    if (frac < 0) { frac = 0; }

    char buf[8];
    std::snprintf(buf, sizeof(buf), "%d", whole);
    label_set_if_changed(objects.obj0, buf);

    label_set_if_changed(objects.obj1, ".");

    std::snprintf(buf, sizeof(buf), "%d", frac);
    label_set_if_changed(objects.obj2, buf);

    if (objects.obj5 != nullptr && lv_obj_is_valid(objects.obj5)) {
        const int arc_val = setpoint_to_arc_value(s_setpoint);
        const int cur = static_cast<int>(lv_arc_get_value(objects.obj5));
        if (cur != arc_val) {
            s_arc_sync_muted = true;
            lv_arc_set_value(objects.obj5, arc_val);
            s_arc_sync_muted = false;
        }
    }
    s_setpoint_dirty = false;
}

static void on_arc_value_changed(lv_event_t *e)
{
    (void)e;
    if (s_arc_sync_muted) return;
    if (objects.obj5 == nullptr || !lv_obj_is_valid(objects.obj5)) return;
    const int arc_val = static_cast<int>(lv_arc_get_value(objects.obj5));
    const float new_sp = arc_value_to_setpoint(arc_val);
    if (std::fabs(new_sp - s_setpoint) > 0.01f) {
        s_setpoint = new_sp;
        s_setpoint_dirty = true;
    }
}

static void on_plus_click(lv_event_t *e)
{
    (void)e;
    s_setpoint += 0.5f;
    if (s_setpoint > 30.0f) s_setpoint = 30.0f;
    s_setpoint_dirty = true;
}

static void on_minus_click(lv_event_t *e)
{
    (void)e;
    s_setpoint -= 0.5f;
    if (s_setpoint < 10.0f) s_setpoint = 10.0f;
    s_setpoint_dirty = true;
}

static void clock_apply_tm(const struct tm *tm)
{
    if (tm == nullptr) return;
    if (!lvgl_port_lock(50)) return;

    char hbuf[8], mbuf[8];
    std::snprintf(hbuf, sizeof(hbuf), "%02d", tm->tm_hour);
    std::snprintf(mbuf, sizeof(mbuf), "%02d", tm->tm_min);
    label_set_if_changed(objects.obj21, hbuf);
    label_set_if_changed(objects.obj23, mbuf);
    label_set_if_changed(objects.obj42, hbuf);
    label_set_if_changed(objects.obj44, mbuf);

    if (objects.obj24 != nullptr && tm->tm_wday >= 0 && tm->tm_wday <= 6) {
        label_set_if_changed(objects.obj24, k_wdays[tm->tm_wday]);
        label_set_if_changed(objects.obj45, k_wdays[tm->tm_wday]);
    }
    if (objects.obj25 != nullptr) {
        char dbuf[8];
        std::snprintf(dbuf, sizeof(dbuf), "%d", tm->tm_mday);
        label_set_if_changed(objects.obj25, dbuf);
        label_set_if_changed(objects.obj46, dbuf);
    }
    if (objects.obj26 != nullptr && tm->tm_mon >= 0 && tm->tm_mon <= 11) {
        label_set_if_changed(objects.obj26, k_months[tm->tm_mon]);
        label_set_if_changed(objects.obj47, k_months[tm->tm_mon]);
    }

    lvgl_port_unlock();
}

static void clock_poll(void)
{
    const uint32_t now = millis();
    if (now - s_last_clock_ms < 1000u) {
        return;
    }
    s_last_clock_ms = now;
    const time_t t = time(nullptr);
    if (t >= k_min_valid_epoch) {
        struct tm lt {};
        localtime_r(&t, &lt);
        clock_apply_tm(&lt);
    }
}

static void format_temp(float t, char *whole_out, size_t whole_len)
{
    if (!std::isfinite(t)) {
        std::snprintf(whole_out, whole_len, "--");
        return;
    }
    const int w = static_cast<int>(std::lround(t));
    std::snprintf(whole_out, whole_len, "%d", w);
}

static void sensor_poll(void)
{
    const uint32_t now = millis();
    if (now - s_last_sensor_ms < 3000u) return;
    s_last_sensor_ms = now;

    if (std::strcmp(romeos_room_sht_touch_bus_family_name(), "none") == 0) return;

    float t_c = NAN, rh = NAN;
    if (!romeos_room_sht_touch_bus_read_both(&t_c, &rh)) return;

    char tbuf[16], rhbuf[16];
    format_temp(t_c, tbuf, sizeof(tbuf));
    if (std::isfinite(rh)) {
        const int rhi = static_cast<int>(std::lround(std::fmax(0.0f, std::fmin(100.0f, rh))));
        std::snprintf(rhbuf, sizeof(rhbuf), "%d", rhi);
    } else {
        std::snprintf(rhbuf, sizeof(rhbuf), "--");
    }

    if (!lvgl_port_lock(80)) return;
    label_set_if_changed(objects.obj9, tbuf);
    label_set_if_changed(objects.obj35, tbuf);
    humidity_label_set(objects.obj12, rhbuf);
    humidity_label_set(objects.obj38, rhbuf);
    lvgl_port_unlock();
}

static void alarm_set_visual(bool active)
{
    const uint32_t col = active ? k_col_red : k_col_white;
    set_text_color(objects.obj18, col);
    set_text_color(objects.obj28, col);
    set_text_color(objects.obj53, col);
    set_text_color(objects.obj57, col);
    set_text_color(objects.obj61, col);
    set_text_color(objects.obj65, col);
}

static void humidity_ui_apply_active(void)
{
    static lv_obj_t *const labels[] = {
        objects.obj12, objects.obj13, objects.obj14, objects.obj15,
        objects.obj33, objects.obj34, objects.obj38, objects.obj39,
    };
    for (lv_obj_t *lbl : labels) {
        if (obj_on_active_screen(lbl)) {
            set_text_color_force(lbl, k_col_cyan);
        }
    }
}

static void boiler_ui_apply_active(bool mb_on_wifi)
{
    const uint32_t col = mb_on_wifi ? k_col_cyan : k_col_white;
    if (obj_on_active_screen(objects.obj17)) {
        set_text_color_force(objects.obj17, col);
    }
    if (obj_on_active_screen(objects.obj41)) {
        set_text_color_force(objects.obj41, col);
    }
}

static void alarm_on_click(lv_event_t *e)
{
    (void)e;
    s_alarm_active = !s_alarm_active;
    alarm_set_visual(s_alarm_active);
    buzzer_gpio_write(s_alarm_active);
}

static void alarm_bind_btn(lv_obj_t *btn)
{
    if (btn == nullptr || !lv_obj_is_valid(btn)) return;
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn, 16);
    lv_obj_add_event_cb(btn, alarm_on_click, LV_EVENT_CLICKED, nullptr);
}

static void alarm_buzzer_keepalive(void)
{
    if (kBuzzerGpio >= 0 && s_alarm_active) {
        buzzer_gpio_write(true);
    }
}

static void on_ntp_sync(const struct tm *tm)
{
    clock_apply_tm(tm);
}

static void bind_clickable(lv_obj_t *btn, lv_event_cb_t cb, int pad)
{
    if (btn == nullptr || !lv_obj_is_valid(btn)) return;
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn, pad);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
}

static void ensure_secondary_hub_buttons(void);

static void menu_on_click(lv_event_t *e)
{
    (void)e;
    /* Χωρίς lvgl_port_lock — το handler τρέχει στο LVGL task· το lock μπλοκάρει το animation */
    loadScreen(SCREEN_ID_SECONDARY_A);
    Serial.println("[eez] MENU → secondary_a");
}

static void back_on_click(lv_event_t *e)
{
    (void)e;
    loadScreen(SCREEN_ID_MAIN);
    Serial.println("[eez] BACK → main");
}

static void back_sub_on_click(lv_event_t *e)
{
    (void)e;
    loadScreen(SCREEN_ID_SECONDARY_A);
    Serial.println("[eez] BACK → secondary_a");
}

static void nav_screen_on_click(lv_event_t *e)
{
    const auto id = static_cast<ScreensEnum>(
        reinterpret_cast<intptr_t>(lv_event_get_user_data(e)));
    loadScreen(id);
    Serial.printf("[eez] hub → screen %d\n", static_cast<int>(id));
}

static lv_obj_t *screen_inner_panel(lv_obj_t *screen_root)
{
    if (screen_root == nullptr || !lv_obj_is_valid(screen_root)) return nullptr;
    if (lv_obj_get_child_cnt(screen_root) < 1) return nullptr;
    return lv_obj_get_child(screen_root, 0);
}

static void ensure_secondary_hub_buttons(void)
{
    static lv_obj_t *s_hub_btns[4] = {};
    if (s_hub_btns[0] != nullptr && lv_obj_is_valid(s_hub_btns[0])) return;

    lv_obj_t *panel = screen_inner_panel(objects.secondary_a);
    if (panel == nullptr) return;

    struct HubZone {
        lv_coord_t x;
        lv_coord_t y;
        lv_coord_t w;
        lv_coord_t h;
        ScreensEnum screen;
    };
    static const HubZone zones[] = {
        {230, 95, 155, 135, SCREEN_ID_HEAT},
        {365, 95, 155, 135, SCREEN_ID_LIGHTS},
        {230, 215, 155, 135, SCREEN_ID_ROLLER_BLINDS},
        {365, 215, 155, 135, SCREEN_ID_HEAT_PUMP},
    };

    for (size_t i = 0; i < sizeof(zones) / sizeof(zones[0]); ++i) {
        lv_obj_t *btn = lv_btn_create(panel);
        lv_obj_set_pos(btn, zones[i].x, zones[i].y);
        lv_obj_set_size(btn, zones[i].w, zones[i].h);
        lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(
            btn, nav_screen_on_click, LV_EVENT_CLICKED,
            reinterpret_cast<void *>(static_cast<intptr_t>(zones[i].screen)));
        s_hub_btns[i] = btn;
    }
}

static void mic_on_click(lv_event_t *e)
{
    (void)e;
    Serial.println("[eez] MIC clicked — voice assistant (TBD)");
}

static void configure_setpoint_arc(void)
{
    if (objects.obj5 == nullptr || !lv_obj_is_valid(objects.obj5)) return;
    lv_arc_set_range(objects.obj5, k_arc_min, k_arc_max);
    lv_arc_set_mode(objects.obj5, LV_ARC_MODE_NORMAL);
    lv_obj_clear_flag(objects.obj5, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(objects.obj5, lv_color_hex(k_col_white), LV_PART_KNOB | LV_STATE_DEFAULT);
}

}  // namespace

void romeos_eez_runtime_after_ui_init(void)
{
    buzzer_pin_init();
    alarm_bind_btn(objects.alarm);
    alarm_set_visual(false);

    set_text_color_force(objects.obj16, k_col_white);
    humidity_ui_apply_active();
    boiler_ui_apply_active(false);

    configure_setpoint_arc();

    /* Σταθερό ":" — το blink (HIDDEN ή αλλαγή κειμένου) σε RGB anti-tear προκαλεί μαύρο/άσπρο flicker. */
    if (objects.obj22 != nullptr && lv_obj_is_valid(objects.obj22)) {
        lv_label_set_text(objects.obj22, ":");
    }
    if (objects.obj43 != nullptr && lv_obj_is_valid(objects.obj43)) {
        lv_label_set_text(objects.obj43, ":");
    }

    ensure_secondary_hub_buttons();

    bind_clickable(objects.menu, menu_on_click, 16);
    bind_clickable(objects.back, back_on_click, 16);
    bind_clickable(objects.mic, mic_on_click, 16);
    bind_clickable(objects.plus, on_plus_click, 20);
    bind_clickable(objects.minus, on_minus_click, 20);

    alarm_bind_btn(objects.alarm_1);
    alarm_bind_btn(objects.alarm_2);
    alarm_bind_btn(objects.alarm_3);
    alarm_bind_btn(objects.alarm_4);
    alarm_bind_btn(objects.alarm_5);
    bind_clickable(objects.mic_1, mic_on_click, 16);
    bind_clickable(objects.mic_2, mic_on_click, 16);
    bind_clickable(objects.mic_3, mic_on_click, 16);
    bind_clickable(objects.mic_4, mic_on_click, 16);
    bind_clickable(objects.mic_5, mic_on_click, 16);
    bind_clickable(objects.back_1, back_sub_on_click, 16);
    bind_clickable(objects.back_2, back_sub_on_click, 16);
    bind_clickable(objects.back_3, back_sub_on_click, 16);
    bind_clickable(objects.back_4, back_sub_on_click, 16);

    set_text_color(objects.obj40, k_col_white);

    if (objects.obj5 != nullptr && lv_obj_is_valid(objects.obj5)) {
        lv_obj_add_event_cb(objects.obj5, on_arc_value_changed, LV_EVENT_VALUE_CHANGED, nullptr);
    }

    update_setpoint_display();
}

void romeos_eez_runtime_services_init(void)
{
    delay(120);
    const bool ok = romeos_room_sht_touch_bus_begin_auto();
    Serial.printf("[eez] room I2C sensor: %s (%s)\n", ok ? "OK" : "NOT FOUND", romeos_room_sht_touch_bus_family_name());

    romeos_ntp_init(on_ntp_sync);
    romeos_display_link_init(nullptr, nullptr, nullptr);
    if (lvgl_port_lock(40)) {
        wifi_ui_apply_unlocked(romeos_display_link_home_wifi_assoc());
        boiler_ui_apply_active(romeos_display_link_is_connected());
        lvgl_port_unlock();
    }
    if (romeos_display_link_home_wifi_assoc()) {
        Serial.println(F("[eez] Home Wi‑Fi OK — NTP θα ρυθμίσει την ώρα μόλις συγχρονιστεί."));
    } else {
        Serial.println(F("[eez] Home Wi‑Fi: όχι συνδεδεμένο — Serial → WIFI_HOME_HELP (χωρίς reboot)."));
        Serial.println(F("[eez] «Σπιτι» γαλάζιο μόνο όταν συνδεθεί στο αποθηκευμένο SSID."));
    }
}

void romeos_eez_runtime_poll(void)
{
    alarm_buzzer_keepalive();

    if (romeos_wifi_nvs_poll_serial()) {
        romeos_display_link_on_home_wifi_saved_to_nvs();
    }
    romeos_display_link_poll();
    romeos_ntp_poll();

    const bool mb_ok = romeos_display_link_is_connected();
    if (mb_ok != s_mb_link_prev) {
        s_mb_link_prev = mb_ok;
        if (lvgl_port_lock(40)) {
            boiler_ui_apply_active(mb_ok);
            lvgl_port_unlock();
        }
    }

    const bool home = romeos_display_link_home_wifi_assoc();
    if (!s_have_wifi_prev || home != s_prev_home) {
        s_have_wifi_prev = true;
        s_prev_home = home;
        wifi_ui_apply(home);
    }

    clock_poll();
    sensor_poll();

    if (s_setpoint_dirty) {
        if (lvgl_port_lock(30)) {
            update_setpoint_display();
            lvgl_port_unlock();
        }
    }
}

static void wifi_ui_apply_unlocked(bool home)
{
    const uint32_t col = home ? k_col_cyan : k_col_white;
    if (obj_on_active_screen(objects.obj16)) {
        set_text_color_force(objects.obj16, col);
    }
    if (obj_on_active_screen(objects.obj40)) {
        set_text_color_force(objects.obj40, col);
    }
}

static void wifi_ui_apply(bool home)
{
    if (!lvgl_port_lock(40)) return;
    wifi_ui_apply_unlocked(home);
    lvgl_port_unlock();
}

void romeos_eez_runtime_on_screen_loaded(void)
{
    if (!lvgl_port_lock(50)) return;
    humidity_ui_apply_active();
    wifi_ui_apply_unlocked(s_prev_home);
    boiler_ui_apply_active(s_mb_link_prev);
    alarm_set_visual(s_alarm_active);
    if (s_setpoint_dirty) {
        update_setpoint_display();
    }
    lvgl_port_unlock();
}
