#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "ui.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

objects_t objects;

/* Setpoint tenths °C: 10.0…40.0 step 0.5 — visual mid (blue/orange) = 20.0 */
#define SP_MIN_X10 100
#define SP_MAX_X10 400
#define SP_MID_X10 200
#define SP_STEP_X10 5
#define SP_DEFAULT_X10 200
#define ARC_POS_MAX 1000
#define ARC_POS_MID 500

static int s_setpoint_x10 = SP_DEFAULT_X10;
static lv_obj_t *s_lab_sp_whole;
static lv_obj_t *s_lab_sp_dot;
static lv_obj_t *s_lab_sp_frac;
static lv_obj_t *s_lab_sp_deg;
static lv_obj_t *s_lab_hh;
static lv_obj_t *s_lab_colon;
static lv_obj_t *s_lab_mm;
static lv_obj_t *s_lab_day;
static lv_obj_t *s_lab_month;
static bool s_wifi_connected = false;
static bool s_arc_updating = false;

static int temp_x10_to_arc_pos(int tx10)
{
    if (tx10 <= SP_MID_X10) {
        return (tx10 - SP_MIN_X10) * ARC_POS_MID / (SP_MID_X10 - SP_MIN_X10);
    }
    return ARC_POS_MID +
           (tx10 - SP_MID_X10) * (ARC_POS_MAX - ARC_POS_MID) / (SP_MAX_X10 - SP_MID_X10);
}

static int arc_pos_to_temp_x10(int pos)
{
    if (pos < 0) {
        pos = 0;
    }
    if (pos > ARC_POS_MAX) {
        pos = ARC_POS_MAX;
    }
    if (pos <= ARC_POS_MID) {
        return SP_MIN_X10 + pos * (SP_MID_X10 - SP_MIN_X10) / ARC_POS_MID;
    }
    return SP_MID_X10 +
           (pos - ARC_POS_MID) * (SP_MAX_X10 - SP_MID_X10) / (ARC_POS_MAX - ARC_POS_MID);
}

static lv_obj_t *make_transparent_btn(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                                      lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_shadow_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    return btn;
}

static lv_obj_t *make_label(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,
                            lv_coord_t w, lv_coord_t h,
                            const lv_font_t *font, const char *text)
{
    lv_obj_t *lab = lv_label_create(parent);
    lv_obj_set_pos(lab, x, y);
    lv_obj_set_size(lab, w, h);
    lv_obj_set_style_text_font(lab, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lab, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(lab, text);
    return lab;
}

static void refresh_setpoint_ui(void)
{
    if (s_setpoint_x10 < SP_MIN_X10) {
        s_setpoint_x10 = SP_MIN_X10;
    }
    if (s_setpoint_x10 > SP_MAX_X10) {
        s_setpoint_x10 = SP_MAX_X10;
    }

    const int whole = s_setpoint_x10 / 10;
    const int frac = s_setpoint_x10 % 10;
    char whole_s[8];
    char frac_s[4];
    snprintf(whole_s, sizeof(whole_s), "%d", whole);
    snprintf(frac_s, sizeof(frac_s), "%d", frac);

    if (s_lab_sp_whole) {
        lv_label_set_text(s_lab_sp_whole, whole_s);
    }
    if (s_lab_sp_frac) {
        lv_label_set_text(s_lab_sp_frac, frac_s);
    }

    /* Midpoint of last shift: was 490 (too right) → 448 (too left) → 469 */
    if (s_lab_sp_whole && s_lab_sp_dot && s_lab_sp_frac && s_lab_sp_deg) {
        const lv_coord_t dot_x = 469;
        const lv_coord_t whole_w = 160;
        lv_obj_set_pos(s_lab_sp_whole, dot_x - whole_w, 150);
        lv_obj_set_size(s_lab_sp_whole, whole_w, 120);
        lv_obj_set_style_text_align(s_lab_sp_whole, LV_TEXT_ALIGN_RIGHT, 0);
        lv_label_set_long_mode(s_lab_sp_whole, LV_LABEL_LONG_CLIP);

        lv_obj_set_pos(s_lab_sp_dot, dot_x, 170);
        lv_obj_set_pos(s_lab_sp_frac, dot_x + 22, 150);
        lv_obj_set_pos(s_lab_sp_deg, dot_x + 82, 160);
    }

    if (objects.arc_setpoint) {
        s_arc_updating = true;
        lv_arc_set_value(objects.arc_setpoint, temp_x10_to_arc_pos(s_setpoint_x10));
        s_arc_updating = false;
    }
}

static void setpoint_adjust(int delta_x10)
{
    s_setpoint_x10 += delta_x10;
    refresh_setpoint_ui();
}

static void on_minus(lv_event_t *e)
{
    (void)e;
    setpoint_adjust(-SP_STEP_X10);
}

static void on_plus(lv_event_t *e)
{
    (void)e;
    setpoint_adjust(+SP_STEP_X10);
}

static void on_arc_changed(lv_event_t *e)
{
    if (s_arc_updating) {
        return;
    }
    lv_obj_t *arc = lv_event_get_target(e);
    int tx10 = arc_pos_to_temp_x10(lv_arc_get_value(arc));
    tx10 = ((tx10 + SP_STEP_X10 / 2) / SP_STEP_X10) * SP_STEP_X10;
    if (tx10 < SP_MIN_X10) {
        tx10 = SP_MIN_X10;
    }
    if (tx10 > SP_MAX_X10) {
        tx10 = SP_MAX_X10;
    }
    s_setpoint_x10 = tx10;
    refresh_setpoint_ui();
}

static void on_goto(lv_event_t *e)
{
    enum ScreensEnum id = (enum ScreensEnum)(intptr_t)lv_event_get_user_data(e);
    loadScreen(id);
}

static lv_obj_t *make_wifi_overlay(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t size)
{
    lv_obj_t *ov = lv_obj_create(parent);
    lv_obj_set_pos(ov, x, y);
    lv_obj_set_size(ov, size, size);
    lv_obj_set_style_radius(ov, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ov, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(ov, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ov, 0, 0);
    lv_obj_set_style_pad_all(ov, 0, 0);
    lv_obj_set_style_shadow_opa(ov, 0, 0);
    lv_obj_clear_flag(ov, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(ov, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *icon = lv_label_create(ov);
    lv_label_set_text(icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0x22c55e), 0);
    lv_obj_center(icon);
    return ov;
}

void ui_set_wifi_connected(bool connected)
{
    s_wifi_connected = connected;
    lv_obj_t *ovs[2] = { objects.wifi_overlay, objects.wifi_overlay_2 };
    for (int i = 0; i < 2; i++) {
        if (!ovs[i]) {
            continue;
        }
        if (connected) {
            lv_obj_clear_flag(ovs[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(ovs[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static lv_obj_t *make_image_screen(const lv_img_dsc_t *img)
{
    lv_obj_t *obj = lv_obj_create(NULL);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_img_src(obj, img, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static void add_back_to_menu(lv_obj_t *parent)
{
    lv_obj_t *btn = make_transparent_btn(parent, 56, 501, 189, 79);
    lv_obj_add_event_cb(btn, on_goto, LV_EVENT_CLICKED, (void *)(intptr_t)SCREEN_ID_2);
    /* Detail BGs have house icon further right than menu — keep clear gap like MENU */
    make_label(parent, 158, 531, 90, 35, &lv_font_montserrat_28, "BACK");
}

static void create_screen_home(void)
{
    objects.screens[0] = make_image_screen(&img_1);
    lv_obj_t *parent = objects.screens[0];

    objects.arc_setpoint = lv_arc_create(parent);
    lv_obj_set_pos(objects.arc_setpoint, 283, 43);
    lv_obj_set_size(objects.arc_setpoint, 330, 402);
    lv_arc_set_range(objects.arc_setpoint, 0, ARC_POS_MAX);
    lv_arc_set_bg_start_angle(objects.arc_setpoint, 170);
    lv_arc_set_bg_end_angle(objects.arc_setpoint, 15);
    lv_arc_set_mode(objects.arc_setpoint, LV_ARC_MODE_NORMAL);
    lv_obj_set_style_arc_opa(objects.arc_setpoint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(objects.arc_setpoint, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    /* Knob size ≈ pad*2; was 0 (tiny). Shadow needs width+spread+ofs to read on photo BG. */
    lv_obj_set_style_bg_color(objects.arc_setpoint, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(objects.arc_setpoint, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(objects.arc_setpoint, LV_RADIUS_CIRCLE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(objects.arc_setpoint, 10, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(objects.arc_setpoint, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(objects.arc_setpoint, lv_color_hex(0x2d2b2b), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(objects.arc_setpoint, 18, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(objects.arc_setpoint, 2, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(objects.arc_setpoint, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(objects.arc_setpoint, 4, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(objects.arc_setpoint, on_arc_changed, LV_EVENT_VALUE_CHANGED, NULL);

    make_label(parent, 99, 73, 60, 55, &ui_font_50, "19");
    make_label(parent, 157, 73, 15, 55, &ui_font_50, ".");
    make_label(parent, 167, 73, 30, 55, &ui_font_50, "5");
    make_label(parent, 194, 73, 18, 55, &lv_font_montserrat_24, "O");

    make_label(parent, 110, 155, 60, 55, &ui_font_50, "43");
    make_label(parent, 175, 158, 60, 55, &lv_font_montserrat_28, "%");

    s_lab_hh = make_label(parent, 743, 62, 62, 51, &ui_font_50, "--");
    s_lab_colon = make_label(parent, 809, 62, 14, 51, &ui_font_50, ":");
    s_lab_mm = make_label(parent, 823, 62, 62, 51, &ui_font_50, "--");

    s_lab_day = make_label(parent, 681, 135, 80, 55, &lv_font_montserrat_48, "--");
    s_lab_month = make_label(parent, 764, 148, 200, 45, &ui_font_30, "—");

    make_label(parent, 577, 483, 60, 55, &ui_font_50, "43");
    make_label(parent, 642, 486, 60, 55, &lv_font_montserrat_28, "%");
    make_label(parent, 381, 483, 60, 55, &ui_font_50, "23");
    make_label(parent, 439, 483, 15, 55, &ui_font_50, ".");
    make_label(parent, 449, 483, 30, 55, &ui_font_50, "5");
    make_label(parent, 476, 483, 18, 55, &lv_font_montserrat_24, "O");

    s_lab_sp_whole = make_label(parent, 309, 150, 160, 120, &ui_font_120, "20");
    lv_obj_set_style_text_align(s_lab_sp_whole, LV_TEXT_ALIGN_RIGHT, 0);
    s_lab_sp_dot = make_label(parent, 469, 170, 30, 100, &ui_font_100, ".");
    s_lab_sp_frac = make_label(parent, 491, 150, 70, 121, &ui_font_120, "0");
    s_lab_sp_deg = make_label(parent, 551, 160, 53, 101, &lv_font_montserrat_28, "O");

    objects.btn_menu = make_transparent_btn(parent, 56, 501, 189, 79);
    lv_obj_add_event_cb(objects.btn_menu, on_goto, LV_EVENT_CLICKED, (void *)(intptr_t)SCREEN_ID_2);
    make_label(parent, 138, 523, 97, 35, &lv_font_montserrat_28, "MENU");

    objects.btn_minus = make_transparent_btn(parent, 342, 282, 95, 85);
    lv_obj_add_event_cb(objects.btn_minus, on_minus, LV_EVENT_CLICKED, NULL);
    objects.btn_plus = make_transparent_btn(parent, 458, 282, 98, 85);
    lv_obj_add_event_cb(objects.btn_plus, on_plus, LV_EVENT_CLICKED, NULL);

    /* +2px more left (783) vs previous 785 */
    objects.wifi_overlay = make_wifi_overlay(parent, 783, 514, 52);

    refresh_setpoint_ui();
}

static void create_screen_menu(void)
{
    objects.screens[1] = make_image_screen(&img_2);
    lv_obj_t *parent = objects.screens[1];

    objects.btn_back = make_transparent_btn(parent, 56, 501, 189, 79);
    lv_obj_add_event_cb(objects.btn_back, on_goto, LV_EVENT_CLICKED, (void *)(intptr_t)SCREEN_ID_1);
    make_label(parent, 137, 531, 97, 35, &lv_font_montserrat_28, "BACK");

    make_label(parent, 252, 230, 247, 37, &ui_font_28, "Αντλία Θερμότητας");
    make_label(parent, 598, 230, 86, 37, &ui_font_28, "Φώτα");
    make_label(parent, 306, 456, 139, 37, &ui_font_28, "Θέρμανση");
    make_label(parent, 603, 456, 77, 37, &ui_font_28, "Ρολά");

    objects.btn_hp = make_transparent_btn(parent, 252, 71, 247, 206);
    lv_obj_add_event_cb(objects.btn_hp, on_goto, LV_EVENT_CLICKED, (void *)(intptr_t)SCREEN_ID_3);

    objects.btn_lights = make_transparent_btn(parent, 518, 71, 247, 206);
    lv_obj_add_event_cb(objects.btn_lights, on_goto, LV_EVENT_CLICKED, (void *)(intptr_t)SCREEN_ID_4);

    objects.btn_heat = make_transparent_btn(parent, 252, 295, 247, 206);
    lv_obj_add_event_cb(objects.btn_heat, on_goto, LV_EVENT_CLICKED, (void *)(intptr_t)SCREEN_ID_5);

    objects.btn_rola = make_transparent_btn(parent, 518, 295, 247, 206);
    lv_obj_add_event_cb(objects.btn_rola, on_goto, LV_EVENT_CLICKED, (void *)(intptr_t)SCREEN_ID_6);

    objects.wifi_overlay_2 = make_wifi_overlay(parent, 800, 520, 50);
    if (s_wifi_connected) {
        ui_set_wifi_connected(true);
    }
}

static void create_detail_screen(int index, const lv_img_dsc_t *img)
{
    objects.screens[index] = make_image_screen(img);
    add_back_to_menu(objects.screens[index]);
}

void create_screens(void)
{
    create_screen_home();
    create_screen_menu();
    /* EEZ order: 3=HP, 4=lights, 5=heating, 6=shutters */
    create_detail_screen(2, &img_3);
    create_detail_screen(3, &img_4);
    create_detail_screen(4, &img_5);
    create_detail_screen(5, &img_6);
}

void tick_screen(int screen_index)
{
    (void)screen_index;
}

void ui_clock_set(const char *hh, const char *mm, const char *day, const char *month, bool colon_visible)
{
    if (s_lab_hh && hh) {
        lv_label_set_text(s_lab_hh, hh);
    }
    if (s_lab_mm && mm) {
        lv_label_set_text(s_lab_mm, mm);
    }
    if (s_lab_day && day) {
        lv_label_set_text(s_lab_day, day);
    }
    if (s_lab_month && month) {
        lv_label_set_text(s_lab_month, month);
    }
    if (s_lab_colon) {
        lv_label_set_text(s_lab_colon, ":");
        lv_obj_set_style_text_opa(s_lab_colon,
                                  colon_visible ? LV_OPA_COVER : LV_OPA_TRANSP,
                                  LV_PART_MAIN);
    }
}
