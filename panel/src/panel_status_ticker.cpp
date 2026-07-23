#include "panel_status_ticker.h"

#include <Arduino.h>
#include <lvgl.h>
#include <math.h>
#include <string.h>

#include "ui/fonts.h"
#include "ui/screens.h"

namespace {

enum class Phase : uint8_t { Enter, Pause, Exit, Idle };

struct Slot {
    lv_obj_t *label = nullptr;
    Phase phase = Phase::Idle;
    uint8_t msg_i = 0;
    uint32_t pause_until_ms = 0;
    float x = 0.f;
    lv_coord_t text_w = 0;
    bool active = false;
    bool handoff_done = false;
};

Slot s_slots[2];
uint8_t s_next_msg = 0;
uint32_t s_last_ms = 0;
bool s_ready = false;

// Alarm bar: x=10 y=434 w=779 h=35 on main → local center x = 390
constexpr lv_coord_t k_bar_w = 779;
constexpr lv_coord_t k_bar_h = 35;
constexpr lv_coord_t k_center = 390;
constexpr float k_speed_px_s = 55.f;
constexpr uint32_t k_pause_ms = 5000;

const char *k_msgs[] = {
    "θερμοκρασίες Νερού  /  OK",
    "αντλία θερμότητας  /  OK",
    "έλεγχος εξόδου  /  OK",
    "θερμοκρασίες Μπόιλερ  /  OK",
    "έλεγχος συστήματος  /  OK",
    "συνδέσεις δικτύου  /  OK",
};
constexpr uint8_t k_msg_count = 6;

lv_coord_t measure_text(const char *txt)
{
    return lv_txt_get_width(txt, strlen(txt), &ui_font_25, 0, LV_TEXT_FLAG_NONE);
}

lv_coord_t label_y_centered()
{
    const lv_coord_t line_h = lv_font_get_line_height(&ui_font_25);
    // Bias 1px up — descenders were clipping the white bar bottom
    lv_coord_t y = (k_bar_h - line_h) / 2 - 1;
    if (y < 0) {
        y = 0;
    }
    return y;
}

void place_label(Slot &s)
{
    if (!s.label || !s.active) {
        return;
    }
    lv_obj_set_pos(s.label, (lv_coord_t)lroundf(s.x), label_y_centered());
}

void start_slot(Slot &s, uint8_t msg_i, float start_x)
{
    const char *txt = k_msgs[msg_i % k_msg_count];
    s.msg_i = msg_i % k_msg_count;
    s.text_w = measure_text(txt);
    s.x = start_x;
    s.phase = Phase::Enter;
    s.active = true;
    s.handoff_done = false;
    s.pause_until_ms = 0;
    lv_label_set_text(s.label, txt);
    lv_obj_clear_flag(s.label, LV_OBJ_FLAG_HIDDEN);
    place_label(s);
    Serial.printf("[ticker] start msg[%u] w=%d\n", (unsigned)s.msg_i, (int)s.text_w);
}

Slot *free_slot()
{
    for (auto &s : s_slots) {
        if (!s.active) {
            return &s;
        }
    }
    return nullptr;
}

void finish_slot(Slot &s)
{
    s.active = false;
    s.phase = Phase::Idle;
    if (s.label) {
        lv_obj_add_flag(s.label, LV_OBJ_FLAG_HIDDEN);
    }
}

void tick_slot(Slot &s, float dt_s, uint32_t now)
{
    if (!s.active) {
        return;
    }

    const float center_x = (float)k_center - (float)s.text_w * 0.5f;

    switch (s.phase) {
    case Phase::Enter: {
        s.x -= k_speed_px_s * dt_s;
        if (s.x <= center_x) {
            s.x = center_x;
            s.phase = Phase::Pause;
            s.pause_until_ms = now + k_pause_ms;
        }
        place_label(s);
        break;
    }
    case Phase::Pause:
        if ((int32_t)(now - s.pause_until_ms) >= 0) {
            s.phase = Phase::Exit;
        }
        break;
    case Phase::Exit: {
        s.x -= k_speed_px_s * dt_s;
        place_label(s);

        // When end of THIS text hits mid-bar → launch next from the right (once)
        const float text_end = s.x + (float)s.text_w;
        if (!s.handoff_done && text_end <= (float)k_center) {
            s.handoff_done = true;
            if (Slot *n = free_slot()) {
                start_slot(*n, (uint8_t)(s.msg_i + 1), (float)k_bar_w);
            }
        }

        // Fully off left edge of bar → recycle
        if (s.x + (float)s.text_w < 0.f) {
            finish_slot(s);
        }
        break;
    }
    case Phase::Idle:
        break;
    }
}

}  // namespace

void panel_status_ticker_begin()
{
    if (!objects.__________ || s_ready) {
        return;
    }

    // Clip children to the white alarm bar
    lv_obj_clear_flag(objects.__________, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(objects.__________, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_pad_all(objects.__________, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(objects.__________, 0, LV_PART_MAIN);

    const lv_coord_t y = label_y_centered();

    for (auto &s : s_slots) {
        s.label = lv_label_create(objects.__________);
        lv_obj_set_style_text_font(s.label, &ui_font_25, LV_PART_MAIN);
        lv_obj_set_style_text_color(s.label, lv_color_hex(0x274157), LV_PART_MAIN);
        lv_obj_set_style_text_opa(s.label, LV_OPA_COVER, LV_PART_MAIN);
        lv_label_set_long_mode(s.label, LV_LABEL_LONG_CLIP);
        lv_obj_set_height(s.label, lv_font_get_line_height(&ui_font_25));
        lv_obj_set_y(s.label, y);
        lv_obj_add_flag(s.label, LV_OBJ_FLAG_HIDDEN);
        s.active = false;
        s.phase = Phase::Idle;
    }

    s_last_ms = millis();
    s_ready = true;
    start_slot(s_slots[0], 0, (float)k_bar_w);
    Serial.println("[ticker] dual-slot idle ticker (centered, smooth)");
}

void panel_status_ticker_apply_ui()
{
    if (!s_ready) {
        return;
    }

    const uint32_t now = millis();
    float dt_s = (now - s_last_ms) / 1000.f;
    s_last_ms = now;
    if (dt_s <= 0.f) {
        return;
    }
    if (dt_s > 0.05f) {
        dt_s = 0.05f;  // clamp after stalls — avoids jumps
    }

    // Snapshot active flags: starting a new slot mid-loop is OK
    for (auto &s : s_slots) {
        tick_slot(s, dt_s, now);
    }

    // If somehow both idle, restart
    if (!s_slots[0].active && !s_slots[1].active) {
        start_slot(s_slots[0], s_next_msg, (float)k_bar_w);
    }
}
