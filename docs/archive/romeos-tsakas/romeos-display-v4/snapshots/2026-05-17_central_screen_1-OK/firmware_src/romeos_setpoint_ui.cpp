#include "romeos_setpoint_ui.h"

#include "eez_central/screens.h"

#include <cstdio>

namespace {

constexpr int kMinHalf = 20;
constexpr int kMaxHalf = 60;
constexpr int kDefaultHalf = 45;
/** Πράσινο knob / δεξιά πλευρά τόξου (EEZ central_screen_1). */
constexpr uint32_t k_arc_green = 0x88EE37u;

static int s_half = kDefaultHalf;
static char s_whole_buf[8];

static void style_setpoint_arc(lv_obj_t *arc)
{
    if (arc == nullptr) {
        return;
    }
    lv_arc_set_range(arc, kMinHalf, kMaxHalf);
    /* Δακτύλιος διαφανός· μπίλια πράσινη όπως στο EEZ. */
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(arc, lv_color_hex(k_arc_green), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(arc, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_DEFAULT);
}

static void update_setpoint_labels(int half_deg)
{
    if (half_deg < kMinHalf) {
        half_deg = kMinHalf;
    }
    if (half_deg > kMaxHalf) {
        half_deg = kMaxHalf;
    }

    const int whole = half_deg / 2;
    const bool has_half = (half_deg % 2) != 0;

    if (objects.obj1 != nullptr) {
        std::snprintf(s_whole_buf, sizeof(s_whole_buf), "%d", whole);
        lv_label_set_text(objects.obj1, s_whole_buf);
    }
    if (objects.obj4 != nullptr) {
        lv_label_set_text_static(objects.obj4, ".");
    }
    if (objects.obj5 != nullptr) {
        lv_label_set_text_static(objects.obj5, has_half ? "5" : "0");
    }
    if (objects.obj6 != nullptr) {
        lv_label_set_text_static(objects.obj6, "O");
    }
}

static void apply_setpoint_half(int half)
{
    if (half < kMinHalf) {
        half = kMinHalf;
    }
    if (half > kMaxHalf) {
        half = kMaxHalf;
    }
    s_half = half;

    if (objects.obj8 != nullptr) {
        lv_arc_set_value(objects.obj8, half);
    }
    update_setpoint_labels(half);
}

static void arc_event_cb(lv_event_t *e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_VALUE_CHANGED && code != LV_EVENT_RELEASED) {
        return;
    }
    lv_obj_t *arc = static_cast<lv_obj_t *>(lv_event_get_target(e));
    apply_setpoint_half(static_cast<int>(lv_arc_get_value(arc)));
}

static void step_btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    const intptr_t delta = reinterpret_cast<intptr_t>(lv_event_get_user_data(e));
    apply_setpoint_half(s_half + static_cast<int>(delta));
}

static void wire_step_btn(lv_obj_t *btn, int delta_half)
{
    if (btn == nullptr) {
        return;
    }
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn, 12);
    lv_obj_add_event_cb(btn, step_btn_event_cb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<intptr_t>(delta_half)));
}

}  // namespace

void romeos_setpoint_ui_init(void)
{
    lv_obj_t *arc = objects.obj8;
    if (arc == nullptr) {
        return;
    }

    style_setpoint_arc(arc);
    apply_setpoint_half(kDefaultHalf);

    lv_obj_add_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(arc, arc_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(arc, arc_event_cb, LV_EVENT_RELEASED, nullptr);

    wire_step_btn(objects.minus, -1);
    wire_step_btn(objects.plus, 1);
}
