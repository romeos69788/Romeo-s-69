#include "romeos_v4_clock.h"

#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "screens/ui_Screen1.h"

static constexpr time_t k_min_valid_epoch = 1609459200; /* 2021-01-01 */

extern "C" void romeos_v4_clock_prepare_ui(void)
{
    if (!lvgl_port_lock(50)) {
        return;
    }
    if (ui_Label33 != nullptr && lv_obj_is_valid(ui_Label33)) {
        lv_obj_clear_flag(ui_Label33, LV_OBJ_FLAG_HIDDEN);
        const lv_color_t gold = lv_color_hex(0xCB9B30);
        const lv_color_t cur = lv_obj_get_style_text_color(ui_Label33, LV_PART_MAIN);
        if (lv_color_to32(cur) != lv_color_to32(gold)) {
            lv_obj_set_style_text_color(ui_Label33, gold, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
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
    if (ui_Screen1 == nullptr || !lv_obj_is_valid(ui_Screen1)) {
        lvgl_port_unlock();
        return;
    }
    if (ui_Label30 != nullptr && lv_obj_is_valid(ui_Label30)) {
        char hbuf[8];
        snprintf(hbuf, sizeof(hbuf), "%02d", tm->tm_hour % 24);
        const char *const ex = lv_label_get_text(ui_Label30);
        if (ex == nullptr || std::strcmp(ex, hbuf) != 0) {
            lv_label_set_text(ui_Label30, hbuf);
        }
    }
    if (ui_Label32 != nullptr && lv_obj_is_valid(ui_Label32)) {
        char mbuf[8];
        snprintf(mbuf, sizeof(mbuf), "%02d", tm->tm_min);
        const char *const ex = lv_label_get_text(ui_Label32);
        if (ex == nullptr || std::strcmp(ex, mbuf) != 0) {
            lv_label_set_text(ui_Label32, mbuf);
        }
    }
    if (ui_Label33 != nullptr && lv_obj_is_valid(ui_Label33)) {
        lv_obj_clear_flag(ui_Label33, LV_OBJ_FLAG_HIDDEN);
        const char *const ap = (tm->tm_hour < 12) ? "AM" : "PM";
        const char *const ex = lv_label_get_text(ui_Label33);
        if (ex == nullptr || std::strcmp(ex, ap) != 0) {
            lv_label_set_text(ui_Label33, ap);
        }
        const lv_color_t gold = lv_color_hex(0xCB9B30);
        const lv_color_t cur = lv_obj_get_style_text_color(ui_Label33, LV_PART_MAIN);
        if (lv_color_to32(cur) != lv_color_to32(gold)) {
            lv_obj_set_style_text_color(ui_Label33, gold, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    lvgl_port_unlock();
}

extern "C" void romeos_v4_clock_on_ntp_sync(const struct tm *tm)
{
    clock_apply_tm(tm);
}

extern "C" void romeos_v4_clock_poll(void)
{
    static uint32_t s_last_ms = 0;
    const uint32_t now = millis();
    if (now - s_last_ms < 1000u) {
        return;
    }
    s_last_ms = now;

    const time_t t = time(nullptr);
    if (t < k_min_valid_epoch) {
        return;
    }
    struct tm lt {};
    localtime_r(&t, &lt);
    clock_apply_tm(&lt);
}
