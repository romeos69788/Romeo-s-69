/*
 * Panel Alpha — hub V4 LOCKED (Othoni_Levita_4) · COM4.
 * Live clock/date + idle bottom-bar status ticker test.
 */

#include <Arduino.h>
#include <cassert>

#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "lvgl_v8_port.h"
#include "panel_nav.h"
#include "panel_status_ticker.h"
#include "panel_wifi_ntp.h"
#include "ui/ui.h"
#include "ui/screens.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

static void disable_scroll(lv_obj_t *obj)
{
    if (obj == nullptr) {
        return;
    }
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    const uint32_t n = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < n; i++) {
        disable_scroll(lv_obj_get_child(obj, i));
    }
}

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("========================================");
    Serial.println("*** ROMEOS PANEL HUB V4 LOCKED ***");
    Serial.println("*** nav slides + clock + ticker ***");
    Serial.println("========================================");

    auto *board = new Board();
    board->init();
#if LVGL_PORT_AVOID_TEARING_MODE != 0
    {
        auto *lcd = board->getLCD();
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
    assert(board->begin());
    assert(lvgl_port_init(board->getLCD(), board->getTouch()));

    if (!lvgl_port_lock(-1)) {
        Serial.println("[panel] ERROR: lvgl_port_lock failed");
        return;
    }
    ui_init();
    disable_scroll(objects.main);
    disable_scroll(objects.water);
    disable_scroll(objects.hp);
    disable_scroll(objects.out);
    disable_scroll(objects.boiler);
    disable_scroll(objects.system);
    disable_scroll(objects.wifi);
    panel_nav_begin();
    panel_status_ticker_begin();
    lvgl_port_unlock();

    panel_wifi_ntp_begin();
    Serial.println("[panel] ui_init done — nav + Wi‑Fi clock + ticker…");
}

void loop()
{
    panel_wifi_ntp_poll();

    if (lvgl_port_lock(0)) {
        // During slide: skip label churn so LVGL can focus on the animation
        if (!panel_nav_busy()) {
            panel_wifi_ntp_apply_ui(panel_nav_screen_id());
            panel_status_ticker_apply_ui(panel_nav_on_hub());
            ui_tick();
        }
        lvgl_port_unlock();
    }
    delay(5);
}
