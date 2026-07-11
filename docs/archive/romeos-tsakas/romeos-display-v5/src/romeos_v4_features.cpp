/*
 * Alarm button (SquareLine name "alarm" → ui_ALARM) + onboard buzzer + room T/RH labels on Screen1.
 */

#include "romeos_v4_features.h"

#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "driver/gpio.h"

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "romeos_v4_alarm_bridge.h"
#include "romeos_v4_clock.h"
#include "romeos_v4_dashboard_mirror.h"
#include "romeos_v4_setpoint_ui.h"
#include "romeos_v4_wifi_ui.h"
#include "romeos_room_sht_touch_bus.h"
#include "screens/ui_Screen1.h"

#ifndef ROMEOS_V4_BUZZER_GPIO
#define ROMEOS_V4_BUZZER_GPIO 17
#endif
/** 1 = HIGH όταν «on», 0 = active-low (ανάστροφο transistor). */
#ifndef ROMEOS_V4_BUZZER_ACTIVE_HIGH
#define ROMEOS_V4_BUZZER_ACTIVE_HIGH 1
#endif
/**
 * 0 = διακοπτόμενο μπιπ (400 ms) — κατάλληλο για μερικά active buzzers.
 * 1 = σταθερό επίπεδο όσο είναι ενεργό το alarm — συχνά καλύτερο για active buzzer.
 * 2 = PWM τόνος (passive piezo) μέσω LEDC· χρειάζεται ROMEOS_V4_BUZZER_LEDC_HZ > 0.
 */
#ifndef ROMEOS_V4_BUZZER_STYLE
#define ROMEOS_V4_BUZZER_STYLE 1
#endif
#ifndef ROMEOS_V4_BUZZER_LEDC_HZ
#define ROMEOS_V4_BUZZER_LEDC_HZ 2700
#endif
/** Βάση αδιαφάνειας κόκκινου alarm· στο UI χρησιμοποιείται το **μισό** (πιο διάφανο). */
#ifndef ROMEOS_V4_ALARM_BG_ACTIVE_OPA
#define ROMEOS_V4_ALARM_BG_ACTIVE_OPA LV_OPA_60
#endif
static constexpr int kBuzzerGpio = ROMEOS_V4_BUZZER_GPIO;
static constexpr bool kBuzzerActiveHigh = ROMEOS_V4_BUZZER_ACTIVE_HIGH != 0;
static constexpr unsigned kBuzzerLedcHz = ROMEOS_V4_BUZZER_LEDC_HZ;

static bool s_alarm_active = false;
static bool s_buzzer_phase = false;
static lv_timer_t *s_buzzer_timer = nullptr;
#if ROMEOS_V4_BUZZER_STYLE == 2
static bool s_buzzer_ledc_attached = false;
#endif

static void buzzer_gpio_write(bool on)
{
    if (kBuzzerGpio < 0) {
        return;
    }
    const int level = (on == kBuzzerActiveHigh) ? HIGH : LOW;
    digitalWrite(kBuzzerGpio, level);
}

static void buzzer_pin_init_output_max_drive(void)
{
    if (kBuzzerGpio < 0) {
        return;
    }
    pinMode(kBuzzerGpio, OUTPUT);
    gpio_set_drive_capability(static_cast<gpio_num_t>(kBuzzerGpio), GPIO_DRIVE_CAP_3);
}

static void buzzer_digital_stop(void)
{
    if (kBuzzerGpio < 0) {
        return;
    }
    buzzer_gpio_write(false);
}

#if ROMEOS_V4_BUZZER_STYLE == 2
static void buzzer_ledc_stop(void)
{
    if (kBuzzerGpio < 0) {
        return;
    }
    if (s_buzzer_ledc_attached) {
        ledcWriteTone(static_cast<uint8_t>(kBuzzerGpio), 0);
        ledcDetach(static_cast<uint8_t>(kBuzzerGpio));
        s_buzzer_ledc_attached = false;
    }
    buzzer_pin_init_output_max_drive();
    buzzer_digital_stop();
}

static void buzzer_ledc_start(void)
{
    if (kBuzzerGpio < 0 || kBuzzerLedcHz == 0u) {
        return;
    }
    buzzer_ledc_stop();
    s_buzzer_ledc_attached = ledcAttach(static_cast<uint8_t>(kBuzzerGpio), kBuzzerLedcHz, 8);
    if (s_buzzer_ledc_attached) {
        ledcWriteTone(static_cast<uint8_t>(kBuzzerGpio), kBuzzerLedcHz);
    }
}
#endif

static void buzzer_apply_alarm_state(void)
{
    if (kBuzzerGpio < 0) {
        return;
    }
#if ROMEOS_V4_BUZZER_STYLE == 2
    if (s_alarm_active) {
        buzzer_ledc_start();
    } else {
        buzzer_ledc_stop();
    }
#elif ROMEOS_V4_BUZZER_STYLE == 1
    buzzer_gpio_write(s_alarm_active);
#else
    if (!s_alarm_active) {
        buzzer_digital_stop();
        s_buzzer_phase = false;
    }
#endif
}

#if ROMEOS_V4_BUZZER_STYLE == 0
static void buzzer_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_alarm_active) {
        buzzer_digital_stop();
        return;
    }
    s_buzzer_phase = !s_buzzer_phase;
    buzzer_gpio_write(s_buzzer_phase);
}
#endif

static lv_opa_t alarm_active_bg_opa_half(void)
{
    unsigned v = static_cast<unsigned>(ROMEOS_V4_ALARM_BG_ACTIVE_OPA) / 2u;
    if (v < static_cast<unsigned>(LV_OPA_10)) {
        return LV_OPA_10;
    }
    return static_cast<lv_opa_t>(v);
}

static void alarm_set_visual(bool active)
{
    if (ui_ALARM == nullptr || !lv_obj_is_valid(ui_ALARM)) {
        return;
    }
    constexpr lv_style_selector_t k_sel = LV_PART_MAIN | LV_STATE_DEFAULT;
    const lv_color_t red = lv_color_hex(0xFF3B30);
    if (active) {
        lv_obj_set_style_bg_color(ui_ALARM, red, k_sel);
        lv_obj_set_style_bg_opa(ui_ALARM, alarm_active_bg_opa_half(), k_sel);
        lv_obj_set_style_border_color(ui_ALARM, red, k_sel);
        lv_obj_set_style_border_width(ui_ALARM, 2, k_sel);
        lv_obj_set_style_border_opa(ui_ALARM, LV_OPA_80, k_sel);
    } else {
        lv_obj_set_style_border_width(ui_ALARM, 0, k_sel);
        lv_obj_set_style_border_opa(ui_ALARM, LV_OPA_TRANSP, k_sel);
        lv_obj_set_style_bg_color(ui_ALARM, lv_color_hex(0xFFFFFF), k_sel);
        lv_obj_set_style_bg_opa(ui_ALARM, LV_OPA_10, k_sel);
    }
}

/** Αφαιρεί border/outline/shadow από default theme (μαύρο «σπιθαμή» σε PRESSED/FOCUSED). */
static void alarm_strip_theme_outline(lv_obj_t *btn)
{
    static const lv_state_t states[] = {LV_STATE_DEFAULT, LV_STATE_PRESSED, LV_STATE_FOCUSED};
    for (lv_state_t s : states) {
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | s);
        lv_obj_set_style_border_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN | s);
        lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN | s);
        lv_obj_set_style_outline_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN | s);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | s);
        lv_obj_set_style_shadow_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN | s);
    }
}

extern "C" void romeos_v4_on_alarm_ui_event(lv_event_t *e)
{
    (void)e;
    s_alarm_active = !s_alarm_active;
    alarm_set_visual(s_alarm_active);
    buzzer_apply_alarm_state();
}

void romeos_v4_after_ui_init(void)
{
    romeos_v4_clock_prepare_ui();
    romeos_v4_wifi_ui_init();
    romeos_v4_setpoint_ui_init();
    romeos_v4_dashboard_mirror_init();

    if (kBuzzerGpio >= 0) {
#if ROMEOS_V4_BUZZER_STYLE == 2
        buzzer_ledc_stop();
#else
        buzzer_pin_init_output_max_drive();
        buzzer_digital_stop();
#endif
    }

    if (ui_ALARM != nullptr && lv_obj_is_valid(ui_ALARM)) {
        /* SquareLine: ui_Label11…15 μετά το alarm → move_foreground ώστε το κουμπί να είναι top-most. */
        lv_obj_move_foreground(ui_ALARM);
        lv_obj_add_flag(ui_ALARM, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ui_ALARM, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_ALARM, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_ext_click_area(ui_ALARM, 20);
        lv_obj_set_style_bg_opa(ui_ALARM, LV_OPA_10, LV_PART_MAIN | LV_STATE_DEFAULT);
        alarm_strip_theme_outline(ui_ALARM);
    }

#if ROMEOS_V4_BUZZER_STYLE == 0
    if (s_buzzer_timer == nullptr) {
        s_buzzer_timer = lv_timer_create(buzzer_timer_cb, 400, nullptr);
    }
#endif
}

void romeos_v4_init_room_sensor(void)
{
    delay(120);
    const bool ok = romeos_room_sht_touch_bus_begin_auto();
    Serial.printf("[v4] room I2C sensor: %s (%s)\n", ok ? "OK" : "NOT FOUND", romeos_room_sht_touch_bus_family_name());
}

static void format_one_decimal(float t, char *whole_out, size_t whole_len, char *frac_out, size_t frac_len)
{
    if (!std::isfinite(t)) {
        snprintf(whole_out, whole_len, "--");
        snprintf(frac_out, frac_len, "-");
        return;
    }
    const float ta = std::fabs(t);
    int w = static_cast<int>(std::floor(ta + 1e-4f));
    int frac = static_cast<int>(std::round((ta - std::floor(ta)) * 10.0f));
    if (frac >= 10) {
        frac = 0;
        w++;
    }
    if (t < 0.0f) {
        snprintf(whole_out, whole_len, "-%d", w);
    } else {
        snprintf(whole_out, whole_len, "%d", w);
    }
    snprintf(frac_out, frac_len, "%d", frac);
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

void romeos_v4_loop(void)
{
    static uint32_t s_last_ms = 0;
    const uint32_t now = millis();
    if (now - s_last_ms < 3000u) {
        return;
    }
    s_last_ms = now;

    if (strcmp(romeos_room_sht_touch_bus_family_name(), "none") == 0) {
        return;
    }

    float t_c = NAN;
    float rh = NAN;
    if (!romeos_room_sht_touch_bus_read_both(&t_c, &rh)) {
        return;
    }

    if (!lvgl_port_lock(80)) {
        return;
    }

    if (ui_Screen1 != nullptr && lv_obj_is_valid(ui_Screen1)) {
        char wbuf[16];
        char fbuf[8];
        format_one_decimal(t_c, wbuf, sizeof(wbuf), fbuf, sizeof(fbuf));

        if (ui_Label23 != nullptr && lv_obj_is_valid(ui_Label23) && ui_Label11 != nullptr && lv_obj_is_valid(ui_Label11) &&
            ui_Label12 != nullptr && lv_obj_is_valid(ui_Label12)) {
            label_set_if_changed(ui_Label23, wbuf);
            label_set_if_changed(ui_Label11, ".");
            label_set_if_changed(ui_Label12, fbuf);
        }

        if (ui_Label9 != nullptr && lv_obj_is_valid(ui_Label9) && ui_Label8 != nullptr && lv_obj_is_valid(ui_Label8) &&
            ui_Label7 != nullptr && lv_obj_is_valid(ui_Label7)) {
            label_set_if_changed(ui_Label9, wbuf);
            label_set_if_changed(ui_Label8, ".");
            label_set_if_changed(ui_Label7, fbuf);
        }

        char rhbuf[16];
        if (std::isfinite(rh)) {
            const int rhi = static_cast<int>(std::lround(std::fmax(0.0f, std::fmin(100.0f, rh))));
            snprintf(rhbuf, sizeof(rhbuf), "%d", rhi);
        } else {
            snprintf(rhbuf, sizeof(rhbuf), "--");
        }
        if (ui_Label111 != nullptr && lv_obj_is_valid(ui_Label111)) {
            label_set_if_changed(ui_Label111, rhbuf);
        }
        if (ui_Label14 != nullptr && lv_obj_is_valid(ui_Label14)) {
            label_set_if_changed(ui_Label14, rhbuf);
        }
    }

    lvgl_port_unlock();
}

void romeos_v4_alarm_buzzer_keepalive(void)
{
#if ROMEOS_V4_BUZZER_STYLE == 1
    if (kBuzzerGpio >= 0 && s_alarm_active) {
        buzzer_gpio_write(true);
    }
#endif
}
