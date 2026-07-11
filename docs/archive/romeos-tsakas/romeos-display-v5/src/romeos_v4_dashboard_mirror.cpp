#include "romeos_v4_dashboard_mirror.h"

#include <Arduino.h>
#include <cstring>

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "screens/ui_Screen1.h"
#include "screens/ui_Screen2.h"
#include "screens/ui_Screen3.h"
#include "screens/ui_Screen4.h"
#include "screens/ui_Screen5.h"
#include "screens/ui_Screen6.h"

static constexpr uint32_t k_mirror_period_ms = 250;

static void prep_nav_btn(lv_obj_t *btn)
{
    if (btn == nullptr || !lv_obj_is_valid(btn)) {
        return;
    }
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn, 18);
}

static void mirror_one_label(lv_obj_t *dst, lv_obj_t *src)
{
    if (dst == nullptr || src == nullptr || !lv_obj_is_valid(dst) || !lv_obj_is_valid(src)) {
        return;
    }
    const char *const st = lv_label_get_text(src);
    const char *const dt = lv_label_get_text(dst);
    const lv_color_t cs = lv_obj_get_style_text_color(src, LV_PART_MAIN);
    const lv_color_t cd = lv_obj_get_style_text_color(dst, LV_PART_MAIN);
    if (dt != nullptr && st != nullptr && std::strcmp(dt, st) == 0 && lv_color_to32(cs) == lv_color_to32(cd)) {
        return;
    }
    lv_label_set_text(dst, st);
    lv_obj_set_style_text_color(dst, cs, LV_PART_MAIN | LV_STATE_DEFAULT);
}

extern "C" void romeos_v4_dashboard_mirror_init(void)
{
    prep_nav_btn(ui_OPTIONS);
    prep_nav_btn(ui_OPTIONS1);
    prep_nav_btn(ui_OPTIONS2);
    prep_nav_btn(ui_OPTIONS3);
    prep_nav_btn(ui_OPTIONS4);
    prep_nav_btn(ui_OPTIONS5);
    prep_nav_btn(ui_Button2);
    prep_nav_btn(ui_Button3);
    prep_nav_btn(ui_Button4);
    prep_nav_btn(ui_Button5);
}

extern "C" void romeos_v4_dashboard_mirror_poll(void)
{
    static uint32_t s_last_ms = 0;
    const uint32_t now = millis();
    if (now - s_last_ms < k_mirror_period_ms) {
        return;
    }
    s_last_ms = now;

    if (ui_Screen1 == nullptr || !lv_obj_is_valid(ui_Screen1)) {
        return;
    }

    if (!lvgl_port_lock(40)) {
        return;
    }

    lv_obj_t *const h_src = ui_Label30;
    lv_obj_t *const colon_src = ui_Label31;
    lv_obj_t *const m_src = ui_Label32;
    lv_obj_t *const ap_src = ui_Label33;

    lv_obj_t *const hrs[] = {ui_Label20, ui_Label52, ui_Label73, ui_Label95, ui_Label119};
    lv_obj_t *const cols[] = {ui_Label21, ui_Label53, ui_Label74, ui_Label96, ui_Label120};
    lv_obj_t *const mins[] = {ui_Label22, ui_Label54, ui_Label75, ui_Label97, ui_Label121};
    lv_obj_t *const aps[] = {ui_Label24, ui_Label55, ui_Label76, ui_Label98, ui_Label122};

    for (unsigned i = 0; i < 5u; ++i) {
        mirror_one_label(hrs[i], h_src);
        mirror_one_label(cols[i], colon_src);
        mirror_one_label(mins[i], m_src);
        mirror_one_label(aps[i], ap_src);
    }

    lv_obj_t *const room_w[] = {ui_Label17, ui_Label49, ui_Label70, ui_Label92, ui_Label116};
    lv_obj_t *const room_d1[] = {ui_Label37, ui_Label59, ui_Label80, ui_Label102, ui_Label126};
    lv_obj_t *const room_d2[] = {ui_Label38, ui_Label60, ui_Label81, ui_Label103, ui_Label127};
    lv_obj_t *const room_o[] = {ui_Label39, ui_Label61, ui_Label82, ui_Label104, ui_Label128};
    lv_obj_t *const rh_w[] = {ui_Label40, ui_Label62, ui_Label83, ui_Label105, ui_Label129};
    lv_obj_t *const rh_pct[] = {ui_Label41, ui_Label63, ui_Label84, ui_Label106, ui_Label130};

    for (unsigned i = 0; i < 5u; ++i) {
        mirror_one_label(room_w[i], ui_Label23);
        mirror_one_label(room_d1[i], ui_Label11);
        mirror_one_label(room_d2[i], ui_Label12);
        mirror_one_label(room_o[i], ui_Label13);
        mirror_one_label(rh_w[i], ui_Label14);
        mirror_one_label(rh_pct[i], ui_Label15);
    }

    lv_obj_t *const out_w[] = {ui_Label16, ui_Label48, ui_Label69, ui_Label91, ui_Label115};
    lv_obj_t *const out_dot[] = {ui_Label10, ui_Label47, ui_Label68, ui_Label90, ui_Label114};
    lv_obj_t *const out_f[] = {ui_Label6, ui_Label46, ui_Label67, ui_Label89, ui_Label113};

    for (unsigned i = 0; i < 5u; ++i) {
        mirror_one_label(out_w[i], ui_Label9);
        mirror_one_label(out_dot[i], ui_Label8);
        mirror_one_label(out_f[i], ui_Label7);
    }

    lv_obj_t *const deg_o[] = {ui_Label27, ui_Label56, ui_Label77, ui_Label99, ui_Label123};
    lv_obj_t *const rh_val[] = {ui_Label34, ui_Label57, ui_Label78, ui_Label100, ui_Label124};
    lv_obj_t *const rh_sym[] = {ui_Label36, ui_Label58, ui_Label79, ui_Label101, ui_Label125};

    for (unsigned i = 0; i < 5u; ++i) {
        mirror_one_label(deg_o[i], ui_Label110);
        mirror_one_label(rh_val[i], ui_Label111);
        mirror_one_label(rh_sym[i], ui_Label112);
    }

    lvgl_port_unlock();
}
