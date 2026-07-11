#include "romeos_v4_setpoint_ui.h"

#include <Arduino.h>
#include <cstdio>
#include <cstring>

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "romeos_display_link.h"
#include "screens/ui_Screen1.h"

static constexpr int k_sp_min_c = 10;
static constexpr int k_sp_max_c = 30;
static constexpr int k_sp_default_c = 20;

static int s_sp_c = k_sp_default_c;
static bool s_arc_event_muted = false;
/** Όταν false, αγνοούμε setpoint από UDP μέχρι η μητρική να στείλει την ίδια ακέραια τιμή (επιβεβαίωση)· όχι χρονόμετρο — αλλιώς επανερχόταν παλιό MB (π.χ. 24). */
static bool s_follow_mb_setpoint = true;

static int clamp_c(int c)
{
    if (c < k_sp_min_c) {
        return k_sp_min_c;
    }
    if (c > k_sp_max_c) {
        return k_sp_max_c;
    }
    return c;
}

/** Αναδρομικό mutex LVGL — ασφαλές από loop() ή από LVGL event task. */
static void apply_visual(int c)
{
    if (!lvgl_port_lock(60)) {
        return;
    }
    if (ui_Screen1 != nullptr && lv_obj_is_valid(ui_Screen1)) {
        if (ui_Arc1 != nullptr && lv_obj_is_valid(ui_Arc1)) {
            const int cur = static_cast<int>(lv_arc_get_value(ui_Arc1));
            if (cur != c) {
                s_arc_event_muted = true;
                lv_arc_set_value(ui_Arc1, c);
                s_arc_event_muted = false;
            }
        }
        if (ui_Label35 != nullptr && lv_obj_is_valid(ui_Label35)) {
            char b[12];
            snprintf(b, sizeof(b), "%d", c);
            const char *const ex = lv_label_get_text(ui_Label35);
            if (ex == nullptr || std::strcmp(ex, b) != 0) {
                lv_label_set_text(ui_Label35, b);
            }
        }
    }
    lvgl_port_unlock();
}

static void set_setpoint_c(int c, bool from_user)
{
    const int nc = clamp_c(c);
    const bool changed = (nc != s_sp_c);
    if (!changed && !from_user) {
        return;
    }
    s_sp_c = nc;
    apply_visual(s_sp_c);
    if (from_user && changed) {
        s_follow_mb_setpoint = false;
        romeos_display_link_on_setpoint_ui_changed();
    }
}

static void on_arc_value(lv_event_t *e)
{
    if (s_arc_event_muted) {
        return;
    }
    lv_obj_t *arc = lv_event_get_target(e);
    if (arc == nullptr || !lv_obj_is_valid(arc)) {
        return;
    }
    const int v = static_cast<int>(lv_arc_get_value(arc));
    set_setpoint_c(v, true);
}

static void on_plus(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    set_setpoint_c(s_sp_c + 1, true);
}

static void on_minus(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    set_setpoint_c(s_sp_c - 1, true);
}

extern "C" void romeos_v4_setpoint_ui_init(void)
{
    s_sp_c = k_sp_default_c;

    if (ui_Arc1 != nullptr && lv_obj_is_valid(ui_Arc1)) {
        lv_arc_set_range(ui_Arc1, k_sp_min_c, k_sp_max_c);
        lv_arc_set_mode(ui_Arc1, LV_ARC_MODE_NORMAL);
        lv_obj_clear_flag(ui_Arc1, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(ui_Arc1, on_arc_value, LV_EVENT_VALUE_CHANGED, nullptr);
    }

    if (ui_PLUS != nullptr && lv_obj_is_valid(ui_PLUS)) {
        lv_obj_add_flag(ui_PLUS, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ui_PLUS, on_plus, LV_EVENT_CLICKED, nullptr);
    }
    if (ui_MINUS != nullptr && lv_obj_is_valid(ui_MINUS)) {
        lv_obj_add_flag(ui_MINUS, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ui_MINUS, on_minus, LV_EVENT_CLICKED, nullptr);
    }

    apply_visual(s_sp_c);
}

extern "C" int16_t romeos_v4_setpoint_get_c_x10(void)
{
    return static_cast<int16_t>(s_sp_c * 10);
}

extern "C" void romeos_v4_setpoint_on_mb_packet(const romeos_mb_to_display_v1_t *pkt)
{
    if (pkt == nullptr || pkt->version < 2u) {
        return;
    }
    const int mb_c = clamp_c(static_cast<int>((pkt->setpoint_c_x10 + 5) / 10));

    if (!s_follow_mb_setpoint) {
        if (mb_c != s_sp_c) {
            return;
        }
        s_follow_mb_setpoint = true;
    }

    if (mb_c == s_sp_c) {
        return;
    }
    s_sp_c = mb_c;
    apply_visual(s_sp_c);
}
