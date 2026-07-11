#include "romeos_setpoint_ui.h"

#include "eez_central/screens.h"

#include <cstdio>

namespace {

/** Bar range in half-degrees: 20 = 10.0 °C, 60 = 30.0 °C (step 0.5 °C). */
constexpr int kBarMinHalf = 20;
constexpr int kBarMaxHalf = 60;
constexpr int kDefaultHalf = 45; /* 22.5 °C */

static char s_whole_buf[8];

static void update_setpoint_labels(int half_deg)
{
    if (half_deg < kBarMinHalf) {
        half_deg = kBarMinHalf;
    }
    if (half_deg > kBarMaxHalf) {
        half_deg = kBarMaxHalf;
    }

    const int whole = half_deg / 2;
    const bool has_half = (half_deg % 2) != 0;

    if (objects.obj2 != nullptr) {
        std::snprintf(s_whole_buf, sizeof(s_whole_buf), "%d", whole);
        lv_label_set_text(objects.obj2, s_whole_buf);
    }

    if (objects.obj5 != nullptr) {
        lv_obj_clear_flag(objects.obj5, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_static(objects.obj5, ".");
    }

    if (objects.obj6 != nullptr) {
        lv_obj_clear_flag(objects.obj6, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_static(objects.obj6, has_half ? "5" : "0");
    }

    if (objects.obj7 != nullptr) {
        lv_label_set_text_static(objects.obj7, "O");
    }
}

static int touch_x_to_half_deg(lv_obj_t *bar, lv_point_t p)
{
    lv_area_t area;
    lv_obj_get_coords(bar, &area);
    const lv_coord_t w = lv_obj_get_width(bar);
    if (w <= 0) {
        return kDefaultHalf;
    }

    int32_t rel = p.x - area.x1;
    if (rel < 0) {
        rel = 0;
    }
    if (rel > w) {
        rel = w;
    }

    const int span = kBarMaxHalf - kBarMinHalf;
    return kBarMinHalf + static_cast<int>((rel * span + w / 2) / w);
}

static void apply_setpoint_half(int half)
{
    lv_obj_t *bar = objects.temperature_controler;
    if (bar == nullptr) {
        return;
    }
    if (half < kBarMinHalf) {
        half = kBarMinHalf;
    }
    if (half > kBarMaxHalf) {
        half = kBarMaxHalf;
    }
    lv_bar_set_value(bar, half, LV_ANIM_OFF);
    update_setpoint_labels(half);
}

static void temp_bar_event_cb(lv_event_t *e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING && code != LV_EVENT_RELEASED) {
        return;
    }

    lv_obj_t *bar = static_cast<lv_obj_t *>(lv_event_get_target(e));
    lv_indev_t *indev = lv_indev_get_act();
    if (indev == nullptr) {
        return;
    }

    lv_point_t p;
    lv_indev_get_point(indev, &p);

    apply_setpoint_half(touch_x_to_half_deg(bar, p));
}

static void step_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    const intptr_t delta = reinterpret_cast<intptr_t>(lv_event_get_user_data(e));
    lv_obj_t *bar = objects.temperature_controler;
    if (bar == nullptr) {
        return;
    }
    apply_setpoint_half(static_cast<int>(lv_bar_get_value(bar)) + static_cast<int>(delta));
}

static void make_step_label_tappable(lv_obj_t *obj, int delta_half)
{
    if (obj == nullptr) {
        return;
    }
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(obj, 24);
    lv_obj_add_event_cb(obj, step_btn_event_cb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<intptr_t>(delta_half)));
}

} // namespace

void romeos_setpoint_ui_init(void)
{
    lv_obj_t *bar = objects.temperature_controler;
    if (bar == nullptr) {
        return;
    }

    lv_bar_set_range(bar, kBarMinHalf, kBarMaxHalf);
    lv_bar_set_value(bar, kDefaultHalf, LV_ANIM_OFF);
    update_setpoint_labels(kDefaultHalf);

    make_step_label_tappable(objects.obj8, -1);
    make_step_label_tappable(objects.obj9, 1);

    lv_obj_add_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_add_event_cb(bar, temp_bar_event_cb, LV_EVENT_ALL, nullptr);
}
