#include "romeos_eez_runtime.h"

#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "driver/gpio.h"

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "othoni_ui/screens.h"
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

namespace {

constexpr uint32_t k_col_green = 0x4DD0E1u;
constexpr uint32_t k_col_white = 0xFFFFFFu;
constexpr uint32_t k_col_red = 0xFF3B30u;
constexpr time_t k_min_valid_epoch = 1609459200;
constexpr int kBuzzerGpio = ROMEOS_V4_BUZZER_GPIO;
constexpr bool kBuzzerActiveHigh = ROMEOS_V4_BUZZER_ACTIVE_HIGH != 0;

static bool s_alarm_active = false;
static bool s_have_wifi_prev = false;
static bool s_prev_home = false;
static bool s_colon_visible = true;
static uint32_t s_last_clock_ms = 0;
static uint32_t s_last_colon_ms = 0;
static uint32_t s_last_sensor_ms = 0;

static float s_setpoint = 22.5f;
static bool s_setpoint_dirty = true;

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

static void label_set_if_changed(lv_obj_t *lbl, const char *txt)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl) || txt == nullptr) return;
    const char *const ex = lv_label_get_text(lbl);
    if (ex != nullptr && std::strcmp(ex, txt) == 0) return;
    lv_label_set_text(lbl, txt);
}

static void set_text_color(lv_obj_t *lbl, uint32_t hex)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl)) return;
    const lv_color_t want = lv_color_hex(hex);
    const lv_color_t cur = lv_obj_get_style_text_color(lbl, LV_PART_MAIN);
    if (lv_color_to32(cur) != lv_color_to32(want)) {
        lv_obj_set_style_text_color(lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void update_setpoint_display(void)
{
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

    int arc_val = static_cast<int>(std::round((s_setpoint - 10.0f) * 2.0f));
    if (arc_val < 0) arc_val = 0;
    if (arc_val > 40) arc_val = 40;
    if (objects.obj5 != nullptr && lv_obj_is_valid(objects.obj5)) {
        lv_arc_set_value(objects.obj5, arc_val);
    }
    s_setpoint_dirty = false;
}

static void on_arc_value_changed(lv_event_t *e)
{
    (void)e;
    if (objects.obj5 == nullptr || !lv_obj_is_valid(objects.obj5)) return;
    int arc_val = lv_arc_get_value(objects.obj5);
    if (arc_val < 0) arc_val = 0;
    if (arc_val > 40) arc_val = 40;
    float new_sp = 10.0f + arc_val * 0.5f;
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

    if (objects.obj24 != nullptr && tm->tm_wday >= 0 && tm->tm_wday <= 6) {
        label_set_if_changed(objects.obj24, k_wdays[tm->tm_wday]);
    }
    if (objects.obj25 != nullptr) {
        char dbuf[8];
        std::snprintf(dbuf, sizeof(dbuf), "%d", tm->tm_mday);
        label_set_if_changed(objects.obj25, dbuf);
    }
    if (objects.obj26 != nullptr && tm->tm_mon >= 0 && tm->tm_mon <= 11) {
        label_set_if_changed(objects.obj26, k_months[tm->tm_mon]);
    }

    lvgl_port_unlock();
}

static void colon_blink_tick(uint32_t now)
{
    if (now - s_last_colon_ms < 500u) return;
    s_last_colon_ms = now;
    s_colon_visible = !s_colon_visible;
    if (!lvgl_port_lock(30)) return;
    if (objects.obj22 != nullptr && lv_obj_is_valid(objects.obj22)) {
        if (s_colon_visible) {
            lv_obj_clear_flag(objects.obj22, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(objects.obj22, LV_OBJ_FLAG_HIDDEN);
        }
    }
    lvgl_port_unlock();
}

static void clock_poll(void)
{
    const uint32_t now = millis();
    if (now - s_last_clock_ms < 1000u) {
        colon_blink_tick(now);
        return;
    }
    s_last_clock_ms = now;
    const time_t t = time(nullptr);
    if (t >= k_min_valid_epoch) {
        struct tm lt {};
        localtime_r(&t, &lt);
        clock_apply_tm(&lt);
    }
    colon_blink_tick(now);
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
    label_set_if_changed(objects.obj6, tbuf);
    label_set_if_changed(objects.obj14, rhbuf);
    lvgl_port_unlock();
}

static void alarm_set_visual(bool active)
{
    if (objects.obj18 != nullptr && lv_obj_is_valid(objects.obj18)) {
        set_text_color(objects.obj18, active ? k_col_red : k_col_white);
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

static void menu_on_click(lv_event_t *e)
{
    (void)e;
    Serial.println("[eez] MENU clicked — navigate to options screen (TBD)");
}

}  // namespace

void romeos_eez_runtime_after_ui_init(void)
{
    buzzer_pin_init();
    alarm_bind_btn(objects.alarm);
    alarm_set_visual(false);

    set_text_color(objects.obj16, k_col_white);
    set_text_color(objects.obj17, k_col_white);

    if (objects.menu != nullptr && lv_obj_is_valid(objects.menu)) {
        lv_obj_add_flag(objects.menu, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(objects.menu, 16);
        lv_obj_add_event_cb(objects.menu, menu_on_click, LV_EVENT_CLICKED, nullptr);
    }

    if (objects.plus != nullptr && lv_obj_is_valid(objects.plus)) {
        lv_obj_add_flag(objects.plus, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(objects.plus, 20);
        lv_obj_add_event_cb(objects.plus, on_plus_click, LV_EVENT_CLICKED, nullptr);
    }
    if (objects.minus != nullptr && lv_obj_is_valid(objects.minus)) {
        lv_obj_add_flag(objects.minus, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(objects.minus, 20);
        lv_obj_add_event_cb(objects.minus, on_minus_click, LV_EVENT_CLICKED, nullptr);
    }
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
    wifi_ui_apply(romeos_display_link_home_wifi_assoc());
    Serial.println(F("[eez] Wi‑Fi home (NVS) + NTP + room sensor ready."));
}

void romeos_eez_runtime_poll(void)
{
    alarm_buzzer_keepalive();

    if (romeos_wifi_nvs_poll_serial()) {
        romeos_display_link_on_home_wifi_saved_to_nvs();
    }
    romeos_display_link_poll();
    romeos_ntp_poll();

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

static void wifi_ui_apply(bool home)
{
    if (!lvgl_port_lock(40)) return;
    set_text_color(objects.obj16, home ? k_col_green : k_col_white);
    lvgl_port_unlock();
}
