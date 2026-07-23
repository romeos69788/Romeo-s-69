#include "panel_status_ticker.h"

#include <Arduino.h>
#include <lvgl.h>
#include <string.h>

#include "ui/fonts.h"
#include "ui/screens.h"

namespace {

enum class Phase : uint8_t { Enter, Pause, Exit };

lv_obj_t *s_label = nullptr;
Phase s_phase = Phase::Enter;
uint8_t s_msg_i = 0;
uint32_t s_pause_until_ms = 0;
lv_coord_t s_text_w = 0;
lv_coord_t s_x = 0;
uint32_t s_last_ms = 0;

// Alarm bar on main: x=10 y=434 w=779 h=35 → local center ≈ 390
constexpr lv_coord_t k_bar_w = 779;
constexpr lv_coord_t k_center = 390;
constexpr lv_coord_t k_speed_px_s = 70;  // scroll speed
constexpr uint32_t k_pause_ms = 5000;

const char *k_msgs[] = {
    "Θερμοκρασίες νερού — OK",
    "Αντλία θερμότητας | ON",
};
constexpr uint8_t k_msg_count = 2;

lv_coord_t measure_text(const char *txt)
{
    return lv_txt_get_width(txt, strlen(txt), &ui_font_25, 0, LV_TEXT_FLAG_NONE);
}

void load_message(uint8_t idx)
{
    s_msg_i = idx % k_msg_count;
    const char *txt = k_msgs[s_msg_i];
    lv_label_set_text(s_label, txt);
    s_text_w = measure_text(txt);
    // start just off the right edge of the bar
    s_x = k_bar_w;
    lv_obj_set_pos(s_label, s_x, 2);
    s_phase = Phase::Enter;
    s_last_ms = millis();
    Serial.printf("[ticker] msg[%u] w=%d «%s»\n", (unsigned)s_msg_i, (int)s_text_w, txt);
}

}  // namespace

void panel_status_ticker_begin()
{
    if (!objects.__________ || s_label) {
        return;
    }

    // Clip ticker to the alarm bar button area
    lv_obj_clear_flag(objects.__________, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(objects.__________, LV_OBJ_FLAG_OVERFLOW_VISIBLE);  // then clip via parent size
    // LVGL clips children to parent when overflow is not visible — clear overflow visible:
    lv_obj_clear_flag(objects.__________, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    s_label = lv_label_create(objects.__________);
    lv_obj_set_style_text_font(s_label, &ui_font_25, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_label, lv_color_hex(0x274157), LV_PART_MAIN);
    lv_obj_set_style_text_opa(s_label, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_long_mode(s_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_height(s_label, 31);

    load_message(0);
    Serial.println("[ticker] bottom-bar idle ticker started (test)");
}

void panel_status_ticker_apply_ui()
{
    if (!s_label) {
        return;
    }

    const uint32_t now = millis();
    uint32_t dt = now - s_last_ms;
    if (dt > 100) {
        dt = 100;  // avoid jumps after stall
    }
    s_last_ms = now;

    const lv_coord_t center_x = k_center - s_text_w / 2;  // label left when text centered

    switch (s_phase) {
    case Phase::Enter: {
        // move left until centered
        const lv_coord_t step = (lv_coord_t)((k_speed_px_s * (int)dt) / 1000);
        s_x -= (step > 0 ? step : 1);
        if (s_x <= center_x) {
            s_x = center_x;
            s_phase = Phase::Pause;
            s_pause_until_ms = now + k_pause_ms;
        }
        lv_obj_set_x(s_label, s_x);
        break;
    }
    case Phase::Pause:
        if ((int32_t)(now - s_pause_until_ms) >= 0) {
            s_phase = Phase::Exit;
        }
        break;
    case Phase::Exit: {
        const lv_coord_t step = (lv_coord_t)((k_speed_px_s * (int)dt) / 1000);
        s_x -= (step > 0 ? step : 1);
        lv_obj_set_x(s_label, s_x);

        // When the *end* of the text reaches the middle → start next from the right
        const lv_coord_t text_end = s_x + s_text_w;
        if (text_end <= k_center) {
            load_message((uint8_t)(s_msg_i + 1));
        } else if (s_x + s_text_w < 0) {
            // fully gone — safety
            load_message((uint8_t)(s_msg_i + 1));
        }
        break;
    }
    }
}
