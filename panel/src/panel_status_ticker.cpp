#include "panel_status_ticker.h"

#include <Arduino.h>
#include <lvgl.h>
#include <math.h>
#include <string.h>

#include "ui/fonts.h"
#include "ui/screens.h"

namespace {

enum class Phase : uint8_t { Enter, Pause, Exit, Gap };

lv_obj_t *s_label = nullptr;
Phase s_phase = Phase::Gap;
uint8_t s_msg_i = 0;
uint32_t s_pause_until_ms = 0;
uint32_t s_gap_until_ms = 0;
float s_x = 0.f;
float s_accum = 0.f;  // sub-pixel accumulator — avoids left/right jitter
lv_coord_t s_text_w = 0;
uint32_t s_last_ms = 0;
bool s_ready = false;

constexpr lv_coord_t k_bar_w = 779;
constexpr lv_coord_t k_bar_h = 35;
constexpr lv_coord_t k_center = 390;
constexpr float k_speed_px_s = 32.f;  // slower enter/exit (was 48)
constexpr uint32_t k_pause_ms = 5000;
constexpr uint32_t k_gap_ms = 500;

const char *k_msgs[] = {
    "θερμοκρασίες Νερού  /  ΟΚ",
    "Αντλία θερμότητας  /  ΟΝ",
    "Ελεγχος Εξόδων  /  ΟΚ",
    "θερμοκρασίες Μπόιλερ /  ΟΚ",
    "Ελεγχος Συστήματος /  ΟΚ",
    "Συνδέσεις Δικτύου /  ΟΚ",
};
constexpr uint8_t k_msg_count = 6;

lv_coord_t measure_text(const char *txt)
{
    return lv_txt_get_width(txt, strlen(txt), &ui_font_25, 0, LV_TEXT_FLAG_NONE);
}

lv_coord_t label_y_centered()
{
    const lv_coord_t line_h = lv_font_get_line_height(&ui_font_25);
    lv_coord_t y = (k_bar_h - line_h) / 2 - 1;
    return (y < 0) ? 0 : y;
}

void place_label()
{
    if (!s_label) {
        return;
    }
    // Integer pixel only when crossed — kills 1px left/right shimmer
    const lv_coord_t xi = (lv_coord_t)floorf(s_x + 0.5f);
    lv_obj_set_x(s_label, xi);
}

void start_enter()
{
    const char *txt = k_msgs[s_msg_i % k_msg_count];
    s_text_w = measure_text(txt);
    s_x = (float)k_bar_w;
    s_accum = 0.f;
    s_phase = Phase::Enter;
    lv_label_set_text(s_label, txt);
    lv_obj_set_width(s_label, s_text_w + 4);
    lv_obj_clear_flag(s_label, LV_OBJ_FLAG_HIDDEN);
    place_label();
}

void tick(float dt_s, uint32_t now)
{
    const float center_x = (float)k_center - (float)s_text_w * 0.5f;

    switch (s_phase) {
    case Phase::Enter: {
        s_x -= k_speed_px_s * dt_s;
        if (s_x <= center_x) {
            s_x = center_x;
            s_phase = Phase::Pause;
            s_pause_until_ms = now + k_pause_ms;
        }
        place_label();
        break;
    }
    case Phase::Pause:
        if ((int32_t)(now - s_pause_until_ms) >= 0) {
            s_phase = Phase::Exit;
        }
        break;
    case Phase::Exit: {
        s_x -= k_speed_px_s * dt_s;
        place_label();
        if (s_x + (float)s_text_w < 0.f) {
            lv_obj_add_flag(s_label, LV_OBJ_FLAG_HIDDEN);
            s_msg_i = (uint8_t)((s_msg_i + 1) % k_msg_count);
            s_phase = Phase::Gap;
            s_gap_until_ms = now + k_gap_ms;
        }
        break;
    }
    case Phase::Gap:
        if ((int32_t)(now - s_gap_until_ms) >= 0) {
            start_enter();
        }
        break;
    }
}

}  // namespace

void panel_status_ticker_begin()
{
    if (!objects.__________ || s_ready) {
        return;
    }

    lv_obj_clear_flag(objects.__________, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(objects.__________, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_pad_all(objects.__________, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(objects.__________, 0, LV_PART_MAIN);
    // Parent button must not steal scroll/gesture feel from labels
    lv_obj_clear_flag(objects.__________, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    s_label = lv_label_create(objects.__________);
    lv_obj_set_style_text_font(s_label, &ui_font_25, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_label, lv_color_hex(0x274157), LV_PART_MAIN);
    lv_obj_set_style_text_opa(s_label, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_long_mode(s_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_height(s_label, lv_font_get_line_height(&ui_font_25));
    lv_obj_set_y(s_label, label_y_centered());
    lv_obj_add_flag(s_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_label, LV_OBJ_FLAG_CLICKABLE);

    s_last_ms = millis();
    s_ready = true;
    s_msg_i = 0;
    start_enter();
    Serial.println("[ticker] single-slot smooth R→L");
}

void panel_status_ticker_apply_ui(bool hub_visible)
{
    if (!s_ready) {
        return;
    }

    if (!hub_visible) {
        s_last_ms = millis();
        return;
    }

    const uint32_t now = millis();
    float dt_s = (now - s_last_ms) / 1000.f;
    s_last_ms = now;
    if (dt_s <= 0.f) {
        return;
    }
    // Soft clamp — hard 50ms clamp caused rubber-band feel after stalls
    if (dt_s > 0.033f) {
        dt_s = 0.033f;
    }

    tick(dt_s, now);
}
