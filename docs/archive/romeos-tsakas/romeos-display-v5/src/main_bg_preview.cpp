/*
 * Background album preview — full-screen 800×480 images.
 * Navigation: swipe/tap (raw GT911 + LVGL zones), Serial n/p.
 */

#include <Arduino.h>
#include <cassert>
#include <cstdio>
#include <cstring>

#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "lvgl_v8_port.h"
#include "preview_album/generated/preview_album_data.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

constexpr int k_swipe_px = 25;
constexpr int k_edge_tap_px = 100;

static Board *g_board = nullptr;
static Touch *g_tp = nullptr;
static lv_obj_t *g_img = nullptr;
static size_t g_index = 0;

static int16_t g_press_x = 0;
static int16_t g_press_y = 0;
static int16_t g_last_x = 0;
static int16_t g_last_y = 0;
static bool g_touch_down = false;

static void show_index_unlocked(size_t idx)
{
    if (preview_album_count == 0 || g_img == nullptr) {
        return;
    }
    if (idx >= preview_album_count) {
        idx = preview_album_count - 1;
    }
    g_index = idx;
    lv_img_set_src(g_img, preview_album_images[g_index]);
    Serial.printf("[album] %u/%u %s\n", static_cast<unsigned>(g_index + 1),
                  static_cast<unsigned>(preview_album_count),
                  preview_album_names[g_index]);
}

static void show_index_locked(size_t idx)
{
    if (!lvgl_port_lock(100)) {
        return;
    }
    show_index_unlocked(idx);
    lvgl_port_unlock();
}

static void show_next()
{
    if (preview_album_count == 0) {
        return;
    }
    show_index_locked((g_index + 1) % preview_album_count);
}

static void show_prev()
{
    if (preview_album_count == 0) {
        return;
    }
    show_index_locked((g_index + preview_album_count - 1) % preview_album_count);
}

static void apply_touch_navigation(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    const int dx = static_cast<int>(x1) - static_cast<int>(x0);
    const int dy = static_cast<int>(y1) - static_cast<int>(y0);

    if (dx <= -k_swipe_px || dy <= -k_swipe_px) {
        show_next();
    } else if (dx >= k_swipe_px || dy >= k_swipe_px) {
        show_prev();
    } else if (x1 < k_edge_tap_px) {
        show_prev();
    } else if (x1 >= (800 - k_edge_tap_px)) {
        show_next();
    }
}

static void on_zone_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    const bool next_zone = lv_event_get_user_data(e) != nullptr;
    if (next_zone) {
        show_next();
    } else {
        show_prev();
    }
}

static void make_touch_zone(lv_obj_t *scr, bool is_next)
{
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 400, 480);
    if (is_next) {
        lv_obj_align(btn, LV_ALIGN_RIGHT_MID, 0, 0);
    } else {
        lv_obj_align(btn, LV_ALIGN_LEFT_MID, 0, 0);
    }
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_STATE_PRESSED);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, on_zone_clicked, LV_EVENT_CLICKED, is_next ? (void *)1 : nullptr);
    lv_obj_move_foreground(btn);
}

static void poll_raw_touch()
{
    if (g_tp == nullptr) {
        return;
    }

    TouchPoint pt;
    const int n = g_tp->readPoints(&pt, 1, 0);

    if (n > 0) {
        if (!g_touch_down) {
            g_press_x = pt.x;
            g_press_y = pt.y;
            g_touch_down = true;
        }
        g_last_x = pt.x;
        g_last_y = pt.y;
        return;
    }

    if (!g_touch_down) {
        return;
    }
    g_touch_down = false;
    apply_touch_navigation(g_press_x, g_press_y, g_last_x, g_last_y);
}

static void poll_serial()
{
    while (Serial.available() > 0) {
        const int c = Serial.read();
        if (c < 0) {
            break;
        }
        if (c == 'n' || c == 'N' || c == '>' || c == '\r') {
            show_next();
        } else if (c == 'p' || c == 'P' || c == '<') {
            show_prev();
        } else if (c >= '1' && c <= '9') {
            show_index_locked(static_cast<size_t>(c - '1'));
        } else if (c == '0') {
            show_index_locked(9);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("========================================");
    Serial.println("*** ROMEOS BG PREVIEW ALBUM ***");
    Serial.println("*** BUILD 2026-05-23-bg-album-touch2 ***");
    Serial.printf("[album] %u image(s)\n", static_cast<unsigned>(preview_album_count));
    Serial.println("[album] Touch: swipe ή tap αριστερά/δεξιά · Serial: n/p ή < >");
    Serial.println("========================================");

    g_board = new Board();
    g_board->init();
#if LVGL_PORT_AVOID_TEARING_MODE != 0
    {
        auto *lcd = g_board->getLCD();
        assert(lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM));
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && defined(CONFIG_IDF_TARGET_ESP32S3)
        auto *lcd_bus = lcd->getBus();
        if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
            static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(
                static_cast<unsigned>(lcd->getFrameWidth() * 10));
        }
#endif
    }
#endif
    assert(g_board->begin());
    g_tp = g_board->getTouch();
    if (g_tp == nullptr) {
        Serial.println("[album] WARN: no touch controller");
    } else {
        Serial.println("[album] GT911 / touch OK");
    }

    assert(lvgl_port_init(g_board->getLCD(), g_tp));

    if (!lvgl_port_lock(-1)) {
        Serial.println("[album] ERROR: lvgl_port_lock failed");
        return;
    }

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    g_img = lv_img_create(scr);
    lv_obj_set_size(g_img, 800, 480);
    lv_obj_center(g_img);
    lv_obj_clear_flag(g_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_img, LV_OBJ_FLAG_SCROLLABLE);

    if (preview_album_count == 0) {
        lv_obj_t *lbl = lv_label_create(scr);
        lv_label_set_text(lbl, "No images.\npreview_album/images/");
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(lbl);
    } else {
        show_index_unlocked(0);
        make_touch_zone(scr, false);
        make_touch_zone(scr, true);
    }

    lvgl_port_unlock();
}

void loop()
{
    poll_serial();
    poll_raw_touch();
    delay(10);
}
