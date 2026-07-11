#include "romeos_eez_runtime.h"

#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "eez_central/screens.h"
#include "lvgl.h"
#include "lvgl_v8_port.h"
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

namespace {

constexpr uint32_t k_col_wifi_ok = 0x4DD0E1u;
constexpr uint32_t k_col_wifi_bad = 0xEF5350u;
constexpr time_t k_min_valid_epoch = 1609459200;
constexpr char k_status_txt[] = "Συνδεδεμένο";
constexpr int kBuzzerGpio = ROMEOS_V4_BUZZER_GPIO;
constexpr bool kBuzzerActiveHigh = ROMEOS_V4_BUZZER_ACTIVE_HIGH != 0;

static bool s_alarm_active = false;
static bool s_have_wifi_prev = false;
static bool s_prev_home = false;
static bool s_colon_visible = true;
static uint32_t s_last_clock_ms = 0;
static uint32_t s_last_colon_ms = 0;
static uint32_t s_last_sensor_ms = 0;

static const char *k_wdays[] = {
    "Κυριακή", "Δευτέρα", "Τρίτη", "Τετάρτη", "Πέμπτη", "Παρασκευή", "Σάββατο",
};

static const char *k_months[] = {
    "Ιανουαρίου", "Φεβρουαρίου", "Μαρτίου",    "Απριλίου",  "Μαΐου",      "Ιουνίου",
    "Ιουλίου",    "Αυγούστου",  "Σεπτεμβρίου", "Οκτωβρίου", "Νοεμβρίου", "Δεκεμβρίου",
};

static void buzzer_gpio_write(bool on)
{
    if (kBuzzerGpio < 0) {
        return;
    }
    const int level = (on == kBuzzerActiveHigh) ? HIGH : LOW;
    digitalWrite(kBuzzerGpio, level);
}

static void buzzer_pin_init(void)
{
    if (kBuzzerGpio < 0) {
        return;
    }
    pinMode(kBuzzerGpio, OUTPUT);
    buzzer_gpio_write(false);
}

static void label_set_if_changed(lv_obj_t *lbl, const char *txt)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl) || txt == nullptr) {
        return;
    }
    const char *const ex = lv_label_get_text(lbl);
    if (ex != nullptr && std::strcmp(ex, txt) == 0) {
        return;
    }
    lv_label_set_text(lbl, txt);
}

static void apply_wifi_img_tint(bool ok)
{
    if (objects.wifi_home_img == nullptr || !lv_obj_is_valid(objects.wifi_home_img)) {
        return;
    }
    if (ok) {
        lv_obj_set_style_img_recolor_opa(objects.wifi_home_img, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_img_recolor(objects.wifi_home_img, lv_color_hex(k_col_wifi_bad), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(objects.wifi_home_img, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void apply_wifi_status_label(bool ok)
{
    if (objects.obj14 == nullptr || !lv_obj_is_valid(objects.obj14)) {
        return;
    }
    const lv_color_t want = lv_color_hex(ok ? k_col_wifi_ok : k_col_wifi_bad);
    label_set_if_changed(objects.obj14, k_status_txt);
    const lv_color_t cur = lv_obj_get_style_text_color(objects.obj14, LV_PART_MAIN);
    if (lv_color_to32(cur) != lv_color_to32(want)) {
        lv_obj_set_style_text_color(objects.obj14, want, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void wifi_ui_apply(bool home)
{
    if (!lvgl_port_lock(40)) {
        return;
    }
    apply_wifi_status_label(home);
    apply_wifi_img_tint(home);
    lvgl_port_unlock();
}

static void clock_apply_tm(const struct tm *tm)
{
    if (tm == nullptr) {
        return;
    }
    if (!lvgl_port_lock(50)) {
        return;
    }

    char hbuf[8];
    char mbuf[8];
    std::snprintf(hbuf, sizeof(hbuf), "%02d", tm->tm_hour);
    std::snprintf(mbuf, sizeof(mbuf), "%02d", tm->tm_min);
    label_set_if_changed(objects.obj15, hbuf);
    label_set_if_changed(objects.obj17, mbuf);
    if (objects.obj18 != nullptr && lv_obj_is_valid(objects.obj18)) {
        lv_obj_clear_flag(objects.obj18, LV_OBJ_FLAG_HIDDEN);
        const char *const ap = (tm->tm_hour < 12) ? "AM" : "PM";
        label_set_if_changed(objects.obj18, ap);
    }

    if (objects.obj3 != nullptr && tm->tm_wday >= 0 && tm->tm_wday <= 6) {
        label_set_if_changed(objects.obj3, k_wdays[tm->tm_wday]);
    }
    if (objects.obj4 != nullptr) {
        char dbuf[8];
        std::snprintf(dbuf, sizeof(dbuf), "%d", tm->tm_mday);
        label_set_if_changed(objects.obj4, dbuf);
    }
    if (objects.obj29 != nullptr && tm->tm_mon >= 0 && tm->tm_mon <= 11) {
        label_set_if_changed(objects.obj29, k_months[tm->tm_mon]);
    }

    lvgl_port_unlock();
}

static void colon_blink_tick(uint32_t now)
{
    if (now - s_last_colon_ms < 500u) {
        return;
    }
    s_last_colon_ms = now;
    s_colon_visible = !s_colon_visible;

    if (!lvgl_port_lock(30)) {
        return;
    }
    if (objects.obj16 != nullptr && lv_obj_is_valid(objects.obj16)) {
        if (s_colon_visible) {
            lv_obj_clear_flag(objects.obj16, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(objects.obj16, LV_OBJ_FLAG_HIDDEN);
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

static void format_room_temp(float t, char *whole_out, size_t whole_len)
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
    if (now - s_last_sensor_ms < 3000u) {
        return;
    }
    s_last_sensor_ms = now;

    if (std::strcmp(romeos_room_sht_touch_bus_family_name(), "none") == 0) {
        return;
    }

    float t_c = NAN;
    float rh = NAN;
    if (!romeos_room_sht_touch_bus_read_both(&t_c, &rh)) {
        return;
    }

    char tbuf[16];
    char rhbuf[16];
    format_room_temp(t_c, tbuf, sizeof(tbuf));
    if (std::isfinite(rh)) {
        const int rhi = static_cast<int>(std::lround(std::fmax(0.0f, std::fmin(100.0f, rh))));
        std::snprintf(rhbuf, sizeof(rhbuf), "%d", rhi);
    } else {
        std::snprintf(rhbuf, sizeof(rhbuf), "--");
    }

    if (!lvgl_port_lock(80)) {
        return;
    }
    label_set_if_changed(objects.obj19, tbuf);
    label_set_if_changed(objects.obj22, rhbuf);
    lvgl_port_unlock();
}

static void alarm_set_icon(bool active)
{
    if (objects.alarm_img == nullptr || !lv_obj_is_valid(objects.alarm_img)) {
        return;
    }
    if (active) {
        lv_obj_set_style_img_recolor(objects.alarm_img, lv_color_hex(0xFF3B30), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(objects.alarm_img, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_img_recolor_opa(objects.alarm_img, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void alarm_on_click(lv_event_t *e)
{
    (void)e;
    s_alarm_active = !s_alarm_active;
    alarm_set_icon(s_alarm_active);
    buzzer_gpio_write(s_alarm_active);
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

}  // namespace

void romeos_eez_runtime_after_ui_init(void)
{
    buzzer_pin_init();

    if (objects.alarm != nullptr && lv_obj_is_valid(objects.alarm)) {
        lv_obj_move_foreground(objects.alarm);
        lv_obj_add_flag(objects.alarm, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(objects.alarm, 20);
        lv_obj_add_event_cb(objects.alarm, alarm_on_click, LV_EVENT_CLICKED, nullptr);
    }
    alarm_set_icon(false);
    apply_wifi_status_label(false);
    apply_wifi_img_tint(false);
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
}
