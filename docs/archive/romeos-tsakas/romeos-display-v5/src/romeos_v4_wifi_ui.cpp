#include "romeos_v4_wifi_ui.h"

#include <cstring>

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "romeos_display_link.h"
#include "screens/ui_Screen1.h"
#include "screens/ui_Screen2.h"
#include "screens/ui_Screen3.h"
#include "screens/ui_Screen4.h"
#include "screens/ui_Screen5.h"
#include "screens/ui_Screen6.h"

/* Ανοιχτό πράσινο / μαλακό κόκκινο — εμφανή σε μικρά fonts και μεγάλα εικονίδια. */
static constexpr uint32_t k_col_wifi_ok = 0xA5D6A7u;
static constexpr uint32_t k_col_wifi_bad = 0xE57373u;
static constexpr uint32_t k_col_status_ok = 0x81C784u;
static constexpr uint32_t k_col_status_bad = 0xEF5350u;

static lv_obj_t *s_wifi_home_sym = nullptr;
static lv_obj_t *s_wifi_mb_sym = nullptr;

static bool s_have_prev = false;
static bool s_prev_home = false;
static bool s_prev_mb = false;

static void wifi_create_symbols_once(void)
{
    if (s_wifi_home_sym != nullptr) {
        return;
    }
    if (ui_Screen1 == nullptr || ui_Label28 == nullptr || ui_Label29 == nullptr) {
        return;
    }

    s_wifi_home_sym = lv_label_create(ui_Screen1);
    lv_label_set_text(s_wifi_home_sym, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(s_wifi_home_sym, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_wifi_home_sym, lv_color_hex(0x888888u), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(s_wifi_home_sym, ui_Label28, LV_ALIGN_OUT_TOP_LEFT, 0, -8);

    s_wifi_mb_sym = lv_label_create(ui_Screen1);
    lv_label_set_text(s_wifi_mb_sym, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(s_wifi_mb_sym, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_wifi_mb_sym, lv_color_hex(0x888888u), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(s_wifi_mb_sym, ui_Label29, LV_ALIGN_OUT_TOP_LEFT, 0, -8);

    lv_obj_move_foreground(s_wifi_home_sym);
    lv_obj_move_foreground(s_wifi_mb_sym);
}

static constexpr char k_status_txt[] = "Συνδεδεμένο";

static void apply_status_label(lv_obj_t *lbl, bool ok)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl)) {
        return;
    }
    const lv_color_t want = lv_color_hex(ok ? k_col_status_ok : k_col_status_bad);
    const lv_color_t cur = lv_obj_get_style_text_color(lbl, LV_PART_MAIN);
    const char *const t = lv_label_get_text(lbl);
    if ((t == nullptr || std::strcmp(t, k_status_txt) != 0)) {
        lv_label_set_text(lbl, k_status_txt);
    }
    if (lv_color_to32(cur) != lv_color_to32(want)) {
        lv_obj_set_style_text_color(lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void apply_wifi_title_row(lv_obj_t *title_lbl, lv_obj_t *sym_lbl, bool ok)
{
    const lv_color_t want = lv_color_hex(ok ? k_col_wifi_ok : k_col_wifi_bad);
    if (title_lbl != nullptr && lv_obj_is_valid(title_lbl)) {
        const lv_color_t cur = lv_obj_get_style_text_color(title_lbl, LV_PART_MAIN);
        if (lv_color_to32(cur) != lv_color_to32(want)) {
            lv_obj_set_style_text_color(title_lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    if (sym_lbl != nullptr && lv_obj_is_valid(sym_lbl)) {
        const lv_color_t cur = lv_obj_get_style_text_color(sym_lbl, LV_PART_MAIN);
        if (lv_color_to32(cur) != lv_color_to32(want)) {
            lv_obj_set_style_text_color(sym_lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

void romeos_v4_wifi_ui_init(void)
{
    if (!lvgl_port_lock(50)) {
        return;
    }
    wifi_create_symbols_once();
    lvgl_port_unlock();
}

void romeos_v4_wifi_status_poll(void)
{
    const bool home = romeos_display_link_home_wifi_assoc();
    const bool mb = romeos_display_link_is_connected();
    if (s_have_prev && home == s_prev_home && mb == s_prev_mb) {
        return;
    }
    s_have_prev = true;
    s_prev_home = home;
    s_prev_mb = mb;

    if (!lvgl_port_lock(40)) {
        return;
    }
    if (ui_Screen1 != nullptr && lv_obj_is_valid(ui_Screen1)) {
        wifi_create_symbols_once();
        apply_status_label(ui_Label1, home);
        apply_status_label(ui_Label3, mb);
        apply_wifi_title_row(ui_Label28, s_wifi_home_sym, home);
        apply_wifi_title_row(ui_Label29, s_wifi_mb_sym, mb);
    }

    apply_status_label(ui_Label2, home);
    apply_status_label(ui_Label5, mb);
    apply_wifi_title_row(ui_Label18, nullptr, home);
    apply_wifi_title_row(ui_Label19, nullptr, mb);

    apply_status_label(ui_Label25, home);
    apply_status_label(ui_Label45, mb);
    apply_wifi_title_row(ui_Label50, nullptr, home);
    apply_wifi_title_row(ui_Label51, nullptr, mb);

    apply_status_label(ui_Label64, home);
    apply_status_label(ui_Label66, mb);
    apply_wifi_title_row(ui_Label71, nullptr, home);
    apply_wifi_title_row(ui_Label72, nullptr, mb);

    apply_status_label(ui_Label86, home);
    apply_status_label(ui_Label88, mb);
    apply_wifi_title_row(ui_Label93, nullptr, home);
    apply_wifi_title_row(ui_Label94, nullptr, mb);

    apply_status_label(ui_Label108, home);
    apply_status_label(ui_Label109, mb);
    apply_wifi_title_row(ui_Label117, nullptr, home);
    apply_wifi_title_row(ui_Label118, nullptr, mb);

    lvgl_port_unlock();
}
