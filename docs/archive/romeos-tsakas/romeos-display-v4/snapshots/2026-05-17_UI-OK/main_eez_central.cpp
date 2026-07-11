/*
 * romeos-display-v4 — δοκιμή EEZ Studio export «central_screen» (800×480).
 * Board Viewe + LVGL + ui_init/ui_tick μόνο· χωρίς Wi‑Fi / μητρική.
 */

#include <Arduino.h>
#include <cassert>
#include <cstdio>

#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "lvgl_v8_port.h"
#include "eez_central/ui.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("========================================");
    Serial.println("*** ROMEOS-DISPLAY-V4 / EEZ central_screen ***");
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
        Serial.println("[eez] ERROR: lvgl_port_lock failed");
        return;
    }
    ui_init();
    lvgl_port_unlock();
    Serial.println("[eez] ui_init done — central_screen on display.");
}

void loop()
{
    if (lvgl_port_lock(0)) {
        ui_tick();
        lvgl_port_unlock();
    }
    delay(5);
}
