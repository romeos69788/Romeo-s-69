#include "romeos_eez_runtime.h"

#include <Arduino.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "lvgl.h"
#include "lvgl_v8_port.h"
#include "ui/screens.h"
#include "ui/images.h"
#include "ui/ui.h"
#include "romeos_display_link.h"
#include "romeos_ntp.h"
#include "romeos_room_sht_touch_bus.h"
#include "romeos_wifi_nvs.h"

#ifndef ROMEOS_V4_BUZZER_GPIO
#define ROMEOS_V4_BUZZER_GPIO 17
#endif
#ifndef ROMEOS_V4_BUZZER_ACTIVE_HIGH
#define ROMEOS_V4_BUZZER_ACTIVE_HIGH 1
#endif

static void wifi_ui_apply(bool home);
static void wifi_ui_apply_unlocked(bool home);

namespace {

constexpr uint32_t k_col_cyan = 0x4DD0E1u;
constexpr uint32_t k_col_white = 0xFFFFFFu;
constexpr uint32_t k_col_red = 0xFF3B30u;
/** Ίδιο μπλε με εικονίδιο Wi‑Fi / bar setpoint (EEZ 0x008cec). */
constexpr uint32_t k_col_wifi = 0x008CECu;
/** Κείμενο footer / ετικέτες στο EEZ v10. */
constexpr uint32_t k_col_label = 0x315275u;
constexpr int k_sp_min_c = 10;
constexpr int k_sp_max_c = 30;
constexpr time_t k_min_valid_epoch = 1609459200;
constexpr int kBuzzerGpio = ROMEOS_V4_BUZZER_GPIO;
constexpr bool kBuzzerActiveHigh = ROMEOS_V4_BUZZER_ACTIVE_HIGH != 0;

static bool s_alarm_active = false;
static bool s_mic_active = false;
static bool s_have_wifi_prev = false;
static bool s_mb_pkt_fresh = false;
static romeos_mb_to_display_v1_t s_mb_pkt{};
static bool s_prev_home = false;
static uint32_t s_last_clock_ms = 0;
static uint32_t s_last_sensor_ms = 0;

static float s_setpoint = 22.5f;
static bool s_setpoint_dirty = true;
static bool s_mb_link_prev = false;

static const char *k_wdays[] = {
    "Κυριακη", "Δευτερα", "Τριτη", "Τεταρτη", "Πεμπτη", "Παρασκευη", "Σαββατο",
};
static const char *k_months[] = {
    "Ιανουαριου", "Φεβρουαριου", "Μαρτιου", "Απριλιου", "Μαϊου", "Ιουνιου",
    "Ιουλιου", "Αυγουστου", "Σεπτεμβρη", "Οκτωβριου", "Νοεμβριου", "Δεκεμβριου",
};

static void buzzer_gpio_write(bool on)
{
    if (kBuzzerGpio < 0) return;
    digitalWrite(kBuzzerGpio, (on == kBuzzerActiveHigh) ? HIGH : LOW);
}

static void buzzer_pin_init(void)
{
    if (kBuzzerGpio < 0) return;
    pinMode(kBuzzerGpio, OUTPUT);
    buzzer_gpio_write(false);
}

static bool obj_on_active_screen(lv_obj_t *obj)
{
    if (obj == nullptr || !lv_obj_is_valid(obj)) {
        return false;
    }
    return lv_obj_get_screen(obj) == lv_scr_act();
}

static void label_set_if_changed(lv_obj_t *lbl, const char *txt)
{
    if (!obj_on_active_screen(lbl) || txt == nullptr) return;
    const char *const ex = lv_label_get_text(lbl);
    if (ex != nullptr && std::strcmp(ex, txt) == 0) return;
    lv_label_set_text(lbl, txt);
}

static void set_text_color(lv_obj_t *lbl, uint32_t hex)
{
    if (!obj_on_active_screen(lbl)) return;
    const lv_color_t want = lv_color_hex(hex);
    const lv_color_t cur = lv_obj_get_style_text_color(lbl, LV_PART_MAIN);
    if (lv_color_to32(cur) != lv_color_to32(want)) {
        lv_obj_set_style_text_color(lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void set_text_color_force(lv_obj_t *lbl, uint32_t hex)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl)) return;
    const lv_color_t want = lv_color_hex(hex);
    const lv_color_t cur = lv_obj_get_style_text_color(lbl, LV_PART_MAIN);
    if (lv_color_to32(cur) != lv_color_to32(want)) {
        lv_obj_set_style_text_color(lbl, want, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void humidity_label_set(lv_obj_t *lbl, const char *txt)
{
    label_set_if_changed(lbl, txt);
    set_text_color(lbl, k_col_cyan);
}

static void update_setpoint_display(void)
{
    if (!obj_on_active_screen(objects.obj0)) {
        s_setpoint_dirty = false;
        return;
    }

    int whole = static_cast<int>(s_setpoint);
    int frac = static_cast<int>(std::round((s_setpoint - whole) * 10.0f));
    if (frac >= 10) { frac = 0; whole++; }
    if (frac < 0) { frac = 0; }

    char buf[8];
    std::snprintf(buf, sizeof(buf), "%d", whole);
    label_set_if_changed(objects.obj0, buf);
    label_set_if_changed(objects.obj1, ".");
    std::snprintf(buf, sizeof(buf), "%d", frac);
    label_set_if_changed(objects.obj2, buf);

    if (objects.obj7 != nullptr && lv_obj_is_valid(objects.obj7)) {
        int v = static_cast<int>(std::lround(s_setpoint));
        if (v < k_sp_min_c) v = k_sp_min_c;
        if (v > k_sp_max_c) v = k_sp_max_c;
        lv_bar_set_value(objects.obj7, v, LV_ANIM_OFF);
    }
    s_setpoint_dirty = false;
}

static void on_plus_click(lv_event_t *e)
{
    (void)e;
    s_setpoint += 0.5f;
    if (s_setpoint > 30.0f) s_setpoint = 30.0f;
    s_setpoint_dirty = true;
}

static void on_minus_click(lv_event_t *e)
{
    (void)e;
    s_setpoint -= 0.5f;
    if (s_setpoint < 10.0f) s_setpoint = 10.0f;
    s_setpoint_dirty = true;
}

static void clock_apply_tm(const struct tm *tm)
{
    if (tm == nullptr) return;
    if (!lvgl_port_lock(50)) return;

    char hbuf[8], mbuf[8];
    std::snprintf(hbuf, sizeof(hbuf), "%02d", tm->tm_hour);
    std::snprintf(mbuf, sizeof(mbuf), "%02d", tm->tm_min);
    /* PRYMARY: obj18/20=ώρα, obj28/29/30=ημερομηνία (πάνω δεξιά). */
    label_set_if_changed(objects.obj18, hbuf);
    label_set_if_changed(objects.obj20, mbuf);
    if (tm->tm_wday >= 0 && tm->tm_wday <= 6) {
        label_set_if_changed(objects.obj28, k_wdays[tm->tm_wday]);
    }
    {
        char dbuf[8];
        std::snprintf(dbuf, sizeof(dbuf), "%d", tm->tm_mday);
        label_set_if_changed(objects.obj29, dbuf);
    }
    if (tm->tm_mon >= 0 && tm->tm_mon <= 11) {
        label_set_if_changed(objects.obj30, k_months[tm->tm_mon]);
    }

    /* MENU: obj41/42=ώρα· obj54/55/56=ημερομηνία — ΟΧΙ obj47/48/49 (BACK/ΘΕΡΜΑΝΣΗ/ΛΕΒΗΤΑ). */
    label_set_if_changed(objects.obj41, hbuf);
    label_set_if_changed(objects.obj42, mbuf);
    if (tm->tm_wday >= 0 && tm->tm_wday <= 6) {
        label_set_if_changed(objects.obj54, k_wdays[tm->tm_wday]);
    }
    {
        char dbuf[8];
        std::snprintf(dbuf, sizeof(dbuf), "%d", tm->tm_mday);
        label_set_if_changed(objects.obj55, dbuf);
    }
    if (tm->tm_mon >= 0 && tm->tm_mon <= 11) {
        label_set_if_changed(objects.obj56, k_months[tm->tm_mon]);
    }

    lvgl_port_unlock();
}

static void clock_poll(void)
{
    const uint32_t now = millis();
    if (now - s_last_clock_ms < 1000u) {
        return;
    }
    s_last_clock_ms = now;
    const time_t t = time(nullptr);
    if (t >= k_min_valid_epoch) {
        struct tm lt {};
        localtime_r(&t, &lt);
        clock_apply_tm(&lt);
    }
}

static void format_temp(float t, char *whole_out, size_t whole_len)
{
    if (!std::isfinite(t)) {
        std::snprintf(whole_out, whole_len, "--");
        return;
    }
    const int w = static_cast<int>(std::lround(t));
    std::snprintf(whole_out, whole_len, "%d", w);
}

static void format_temp_x10(int16_t t_x10, char *whole_out, size_t whole_len)
{
    if (t_x10 == INT16_MIN) {
        std::snprintf(whole_out, whole_len, "--");
        return;
    }
    format_temp(static_cast<float>(t_x10) / 10.0f, whole_out, whole_len);
}

static bool mb_outdoor_temp_valid(const romeos_mb_to_display_v1_t &pkt)
{
    if ((pkt.flags & ROMEOS_MB_FLAG_TELEMETRY_VALID) == 0u) {
        return false;
    }
    const bool demo_placeholders = (pkt.mb_proto_reserved & 0x01u) != 0u;
    const bool outdoor_live = (pkt.mb_proto_reserved & 0x02u) != 0u;
    if (demo_placeholders && !outdoor_live) {
        return false;
    }
    const int16_t t = pkt.outdoor_c_x10;
    if (t == INT16_MIN) {
        return false;
    }
    return t >= -400 && t <= 600;
}

static void on_mb_telemetry(const romeos_mb_to_display_v1_t *pkt)
{
    if (pkt == nullptr) {
        return;
    }
    s_mb_pkt = *pkt;
    s_mb_pkt_fresh = true;
}

/** Κεντρικό ΛΕΒΗΤΑ (MENU + οθόνη boiler): asset BOILER 64×64 — όχι sync icon. */
static void boiler_hub_icons_apply(void)
{
    static lv_obj_t *const hub_imgs[] = {objects.______, objects._______1};
    for (lv_obj_t *img : hub_imgs) {
        if (img == nullptr || !lv_obj_is_valid(img)) {
            continue;
        }
        lv_obj_set_style_bg_img_src(img, &img_boiler1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_img_recolor_opa(img, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void room_sensor_ui_poll(void)
{
    const uint32_t now = millis();
    if (now - s_last_sensor_ms < 3000u) {
        return;
    }
    s_last_sensor_ms = now;

    if (std::strcmp(romeos_room_sht_touch_bus_family_name(), "none") == 0) {
        return;
    }

    float t_c = NAN, rh = NAN;
    if (!romeos_room_sht_touch_bus_read_both(&t_c, &rh)) {
        return;
    }

    char tbuf[16], rhbuf[16];
    format_temp(t_c, tbuf, sizeof(tbuf));
    if (std::isfinite(rh)) {
        const int rhi = static_cast<int>(std::lround(std::fmax(0.0f, std::fmin(100.0f, rh))));
        std::snprintf(rhbuf, sizeof(rhbuf), "%d", rhi);
    } else {
        std::snprintf(rhbuf, sizeof(rhbuf), "--");
    }

    if (!lvgl_port_lock(80)) {
        return;
    }
    /* Εσωτερική μόνο — SHT στην οθόνη (obj9/obj15 primary, obj32/obj38 menu). */
    label_set_if_changed(objects.obj9, tbuf);
    label_set_if_changed(objects.obj32, tbuf);
    humidity_label_set(objects.obj15, rhbuf);
    humidity_label_set(objects.obj38, rhbuf);
    lvgl_port_unlock();
}

static void outdoor_sensor_ui_poll(void)
{
    static uint32_t s_last_outdoor_ms = 0;
    const uint32_t now = millis();
    if (now - s_last_outdoor_ms < 3000u) {
        return;
    }
    s_last_outdoor_ms = now;

    char tbuf[16];
    const char *rh_ext = "--";

    if (romeos_display_link_telemetry_valid() && s_mb_pkt_fresh &&
        mb_outdoor_temp_valid(s_mb_pkt)) {
        format_temp_x10(s_mb_pkt.outdoor_c_x10, tbuf, sizeof(tbuf));
    } else {
        std::snprintf(tbuf, sizeof(tbuf), "--");
    }

    if (!lvgl_port_lock(80)) {
        return;
    }
    label_set_if_changed(objects.obj8, tbuf);
    label_set_if_changed(objects.obj31, tbuf);
    label_set_if_changed(objects.obj14, rh_ext);
    label_set_if_changed(objects.obj37, rh_ext);
    set_text_color(objects.obj14, k_col_cyan);
    set_text_color(objects.obj37, k_col_cyan);
    lvgl_port_unlock();
}

/** EEZ v10: εικονίδια alarm ως lv_img + bg_img_src — χρειάζεται bg_img_recolor. */
static void alarm_img_tint(lv_obj_t *img, bool active)
{
    if (img == nullptr || !lv_obj_is_valid(img)) return;
    if (!obj_on_active_screen(img)) return;
    if (active) {
        lv_obj_set_style_bg_img_recolor(img, lv_color_hex(k_col_red), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_img_recolor_opa(img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_img_recolor_opa(img, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void alarm_label_tint(lv_obj_t *lbl, bool active)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl)) return;
    set_text_color_force(lbl, active ? k_col_red : k_col_label);
}

static void alarm_set_visual(bool active)
{
    alarm_label_tint(objects.obj23, active);
    alarm_label_tint(objects.obj45, active);
    alarm_label_tint(objects.obj58, active);
    alarm_label_tint(objects.obj62, active);
    alarm_label_tint(objects.obj66, active);
    alarm_label_tint(objects.obj70, active);

    alarm_img_tint(objects.alarm, active);
    alarm_img_tint(objects.alarm_1, active);
    alarm_img_tint(objects.alarm_2, active);
    alarm_img_tint(objects.alarm_3, active);
    alarm_img_tint(objects.alarm_4, active);
    alarm_img_tint(objects.alarm_5, active);
}

static void mic_img_tint(lv_obj_t *img, bool active)
{
    if (img == nullptr || !lv_obj_is_valid(img)) {
        return;
    }
    if (!obj_on_active_screen(img)) {
        return;
    }
    if (active) {
        lv_obj_set_style_bg_img_recolor(img, lv_color_hex(k_col_red), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_img_recolor_opa(img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_img_recolor_opa(img, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void mic_label_tint(lv_obj_t *lbl, bool active)
{
    if (lbl == nullptr || !lv_obj_is_valid(lbl)) {
        return;
    }
    if (!obj_on_active_screen(lbl)) {
        return;
    }
    set_text_color_force(lbl, active ? k_col_red : k_col_label);
}

static void mic_set_visual(bool active)
{
    mic_label_tint(objects.obj25, active);
    mic_label_tint(objects.obj46, active);
    mic_label_tint(objects.obj59, active);
    mic_label_tint(objects.obj63, active);
    mic_label_tint(objects.obj67, active);
    mic_label_tint(objects.obj71, active);

    mic_img_tint(objects.mic, active);
    mic_img_tint(objects.mic_1, active);
    mic_img_tint(objects.mic_2, active);
    mic_img_tint(objects.mic_3, active);
    mic_img_tint(objects.mic_4, active);
    mic_img_tint(objects.mic_5, active);
}

static void humidity_ui_apply_active(void)
{
    static lv_obj_t *const labels[] = {
        objects.obj14, objects.obj15, objects.obj37, objects.obj38,
    };
    for (lv_obj_t *lbl : labels) {
        if (obj_on_active_screen(lbl)) {
            set_text_color_force(lbl, k_col_cyan);
        }
    }
}

static void boiler_img_tint(lv_obj_t *img, bool mb_on_wifi)
{
    if (img == nullptr || !lv_obj_is_valid(img)) return;
    if (!obj_on_active_screen(img)) return;
    if (mb_on_wifi) {
        lv_obj_set_style_bg_img_recolor(img, lv_color_hex(k_col_wifi), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_img_recolor_opa(img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_img_recolor_opa(img, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

static void boiler_ui_apply_active(bool mb_on_wifi)
{
    const uint32_t col = mb_on_wifi ? k_col_wifi : k_col_label;
    if (obj_on_active_screen(objects.obj27)) {
        set_text_color_force(objects.obj27, col);
    }
    if (obj_on_active_screen(objects.obj53)) {
        set_text_color_force(objects.obj53, col);
    }
    if (obj_on_active_screen(objects.obj49)) {
        set_text_color_force(objects.obj49, col);
    }
    boiler_img_tint(objects.boiler__, mb_on_wifi);
    boiler_img_tint(objects.boiler_wifi, mb_on_wifi);
}

static void alarm_on_click(lv_event_t *e)
{
    (void)e;
    s_alarm_active = !s_alarm_active;
    alarm_set_visual(s_alarm_active);
    buzzer_gpio_write(s_alarm_active);
}

static void alarm_bind_btn(lv_obj_t *btn)
{
    if (btn == nullptr || !lv_obj_is_valid(btn)) return;
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn, 16);
    lv_obj_add_event_cb(btn, alarm_on_click, LV_EVENT_CLICKED, nullptr);
}

static void alarm_buzzer_keepalive(void)
{
    if (kBuzzerGpio >= 0 && s_alarm_active) {
        buzzer_gpio_write(true);
    }
}

static void on_ntp_sync(const struct tm *tm)
{
    clock_apply_tm(tm);
}

static void bind_clickable(lv_obj_t *btn, lv_event_cb_t cb, int pad)
{
    if (btn == nullptr || !lv_obj_is_valid(btn)) return;
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn, pad);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
}

static void menu_on_click(lv_event_t *e)
{
    (void)e;
    loadScreen(SCREEN_ID_MENU);
    Serial.println("[eez] MENU → menu");
}

static void back_on_click(lv_event_t *e)
{
    (void)e;
    loadScreen(SCREEN_ID_PRYMARY);
    Serial.println("[eez] BACK → prymary");
}

static void back_sub_on_click(lv_event_t *e)
{
    (void)e;
    loadScreen(SCREEN_ID_MENU);
    Serial.println("[eez] BACK → menu");
}

static void nav_screen_on_click(lv_event_t *e)
{
    const auto id = static_cast<ScreensEnum>(
        reinterpret_cast<intptr_t>(lv_event_get_user_data(e)));
    loadScreen(id);
    Serial.printf("[eez] hub → screen %d\n", static_cast<int>(id));
}

static void mic_on_click(lv_event_t *e);

/** EEZ: opacity 0 στο κύριο part μπλοκάρει hit-test — κρατάμε cover + διάφανο bg. */
static void footer_hit_btn_prepare(lv_obj_t *btn)
{
    if (btn == nullptr || !lv_obj_is_valid(btn)) {
        return;
    }
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_ext_click_area(btn, 20);
    lv_obj_move_foreground(btn);
}

static void mic_touch_bind(lv_obj_t *obj)
{
    if (obj == nullptr || !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(obj, 20);
    lv_obj_add_event_cb(obj, mic_on_click, LV_EVENT_CLICKED, nullptr);
    lv_obj_move_foreground(obj);
}

static void mic_bind_all_screens(void)
{
    static lv_obj_t *const btns[] = {
        objects.button_mic,
        objects.button_mic_1,
        objects.button_mic_2,
        objects.button_mic_3,
        objects.button_mic_4,
        objects.button_mic_5,
    };
    static lv_obj_t *const imgs[] = {
        objects.mic,
        objects.mic_1,
        objects.mic_2,
        objects.mic_3,
        objects.mic_4,
        objects.mic_5,
    };
    static lv_obj_t *const labels[] = {
        objects.obj25,
        objects.obj46,
        objects.obj59,
        objects.obj63,
        objects.obj67,
        objects.obj71,
    };

    for (lv_obj_t *btn : btns) {
        footer_hit_btn_prepare(btn);
        lv_obj_add_event_cb(btn, mic_on_click, LV_EVENT_CLICKED, nullptr);
    }
    for (lv_obj_t *img : imgs) {
        mic_touch_bind(img);
    }
    for (lv_obj_t *lbl : labels) {
        mic_touch_bind(lbl);
    }
}

static void mic_on_click(lv_event_t *e)
{
    (void)e;
    s_mic_active = !s_mic_active;
    mic_set_visual(s_mic_active);
    Serial.printf("[eez] MIC %s\n", s_mic_active ? "ON" : "OFF");
}

static void bind_hub_btn(lv_obj_t *btn, ScreensEnum id)
{
    if (btn == nullptr || !lv_obj_is_valid(btn)) return;
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn, 12);
    lv_obj_add_event_cb(
        btn, nav_screen_on_click, LV_EVENT_CLICKED,
        reinterpret_cast<void *>(static_cast<intptr_t>(id)));
}

static void bind_hub_buttons(void)
{
    bind_hub_btn(objects.button_________, SCREEN_ID_HEAT);
    bind_hub_btn(objects.button_______, SCREEN_ID_BOILER);
    bind_hub_btn(objects.button_____, SCREEN_ID_LIGHT);
    bind_hub_btn(objects.button___________, SCREEN_ID_BLINDS);
}

}  // namespace

void romeos_eez_runtime_after_ui_init(void)
{
    buzzer_pin_init();
    alarm_bind_btn(objects.button_alarm);
    alarm_set_visual(false);
    mic_set_visual(false);

    boiler_hub_icons_apply();
    set_text_color_force(objects.obj26, k_col_label);
    humidity_ui_apply_active();
    boiler_ui_apply_active(false);
    outdoor_sensor_ui_poll();

    bind_hub_buttons();

    bind_clickable(objects.button_menu, menu_on_click, 16);
    mic_bind_all_screens();
    bind_clickable(objects.button_plus, on_plus_click, 20);
    bind_clickable(objects.button_minus, on_minus_click, 20);

    /* BACK: το EEZ βάζει διαφανές button_menu_* πάνω από το εικονίδιο back. */
    bind_clickable(objects.button_menu_1, back_on_click, 16);
    bind_clickable(objects.button_menu_2, back_sub_on_click, 16);
    bind_clickable(objects.button_menu_3, back_sub_on_click, 16);
    bind_clickable(objects.button_menu_4, back_sub_on_click, 16);
    bind_clickable(objects.button_menu_5, back_sub_on_click, 16);
    bind_clickable(objects.back, back_on_click, 16);
    bind_clickable(objects.back_1, back_sub_on_click, 16);
    bind_clickable(objects.back_2, back_sub_on_click, 16);
    bind_clickable(objects.back_3, back_sub_on_click, 16);
    bind_clickable(objects.back_4, back_sub_on_click, 16);

    alarm_bind_btn(objects.button_alarm_1);
    alarm_bind_btn(objects.button_alarm_2);
    alarm_bind_btn(objects.button_alarm_3);
    alarm_bind_btn(objects.button_alarm_4);
    alarm_bind_btn(objects.button_alarm_5);
    if (objects.obj7 != nullptr && lv_obj_is_valid(objects.obj7)) {
        lv_bar_set_range(objects.obj7, k_sp_min_c, k_sp_max_c);
        lv_obj_set_style_bg_color(objects.obj7, lv_color_hex(k_col_label), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(objects.obj7, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(objects.obj7, lv_color_hex(k_col_wifi), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
    update_setpoint_display();
}

void romeos_eez_runtime_services_init(void)
{
    delay(120);
    const bool ok = romeos_room_sht_touch_bus_begin_auto();
    Serial.printf("[eez] room I2C sensor: %s (%s)\n", ok ? "OK" : "NOT FOUND", romeos_room_sht_touch_bus_family_name());

    romeos_ntp_init(on_ntp_sync);
    romeos_display_link_init(on_mb_telemetry, nullptr, nullptr);
    if (lvgl_port_lock(40)) {
        wifi_ui_apply_unlocked(romeos_display_link_home_wifi_assoc());
        boiler_ui_apply_active(romeos_display_link_is_connected());
        lvgl_port_unlock();
    }
    if (romeos_display_link_home_wifi_assoc()) {
        Serial.println(F("[eez] Home Wi‑Fi OK — NTP θα ρυθμίσει την ώρα μόλις συγχρονιστεί."));
    } else {
        Serial.println(F("[eez] Home Wi‑Fi: όχι συνδεδεμένο — Serial → WIFI_HOME_HELP (χωρίς reboot)."));
        Serial.println(F("[eez] «Σπιτι» γαλάζιο μόνο όταν συνδεθεί στο αποθηκευμένο SSID."));
    }
}

void romeos_eez_runtime_poll(void)
{
    alarm_buzzer_keepalive();

    if (romeos_wifi_nvs_poll_serial()) {
        romeos_display_link_on_home_wifi_saved_to_nvs();
    }
    romeos_display_link_poll();
    romeos_ntp_poll();

    const bool mb_ok = romeos_display_link_is_connected();
    if (mb_ok != s_mb_link_prev) {
        s_mb_link_prev = mb_ok;
        if (lvgl_port_lock(40)) {
            boiler_ui_apply_active(mb_ok);
            lvgl_port_unlock();
        }
    }

    const bool home = romeos_display_link_home_wifi_assoc();
    if (!s_have_wifi_prev || home != s_prev_home) {
        s_have_wifi_prev = true;
        s_prev_home = home;
        wifi_ui_apply(home);
    }

    clock_poll();
    room_sensor_ui_poll();
    outdoor_sensor_ui_poll();

    if (s_setpoint_dirty) {
        if (lvgl_port_lock(30)) {
            update_setpoint_display();
            lvgl_port_unlock();
        }
    }
}

static void wifi_ui_apply_unlocked(bool home)
{
    const uint32_t col = home ? k_col_wifi : k_col_label;
    if (obj_on_active_screen(objects.obj26)) {
        set_text_color_force(objects.obj26, col);
    }
    if (obj_on_active_screen(objects.obj52)) {
        set_text_color_force(objects.obj52, col);
    }
}

static void wifi_ui_apply(bool home)
{
    if (!lvgl_port_lock(40)) return;
    wifi_ui_apply_unlocked(home);
    lvgl_port_unlock();
}

void romeos_eez_runtime_on_screen_loaded(void)
{
    if (!lvgl_port_lock(50)) return;
    humidity_ui_apply_active();
    wifi_ui_apply_unlocked(s_prev_home);
    boiler_ui_apply_active(s_mb_link_prev);
    alarm_set_visual(s_alarm_active);
    mic_set_visual(s_mic_active);
    boiler_hub_icons_apply();
    if (s_setpoint_dirty) {
        update_setpoint_display();
    }
    lvgl_port_unlock();
}
