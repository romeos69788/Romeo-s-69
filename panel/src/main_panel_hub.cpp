/*
 * Panel Alpha — hub preview flash (COM4).
 * EEZ: Othoni_Levita_2 (hub V3 LOCKED) · 6 tiles.
 */

#include <Arduino.h>
#include <cassert>

#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "lvgl_v8_port.h"
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
    Serial.println("*** ROMEOS PANEL HUB PREVIEW ***");
    Serial.println("*** Othoni_Levita_2 V3 LOCKED · COM4 ***");
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
    lvgl_port_unlock();

    Serial.println("[panel] ui_init done — hub on glass. Look at the Viewe.");
}

void loop()
{
    if (lvgl_port_lock(0)) {
        ui_tick();
        lvgl_port_unlock();
    }
    delay(5);
}
