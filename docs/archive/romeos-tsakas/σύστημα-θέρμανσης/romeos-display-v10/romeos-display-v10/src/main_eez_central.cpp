/*
 * romeos-display-v10 — EEZ Studio export (800×480).
 * Setpoint +/-, Wi‑Fi, NTP, room SHT, alarm, MENU hub, MIC toggle (voice stub).
 */

#include <Arduino.h>
#include <cassert>
#include <cstdio>

#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "lvgl_v8_port.h"
#include "ui/ui.h"
#include "ui/screens.h"
#include "romeos_eez_runtime.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

static void romeos_disable_ui_scroll(lv_obj_t *obj)
{
    if (obj == nullptr) {
        return;
    }
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);

    const uint32_t count = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < count; i++) {
        romeos_disable_ui_scroll(lv_obj_get_child(obj, i));
    }
}

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("========================================");
    Serial.println("*** ROMEOS-DISPLAY-V10 ***");
    Serial.println("*** BUILD 2026-06-08-espnow-wifi-dual-ui-v10 ***");
    Serial.printf("[eez] LVGL avoid-tear mode %d (3=direct RGB)\n", LVGL_PORT_AVOID_TEARING_MODE);
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
    romeos_disable_ui_scroll(objects.prymary);
    romeos_disable_ui_scroll(objects.menu);
    romeos_disable_ui_scroll(objects.heat);
    romeos_disable_ui_scroll(objects.boiler);
    romeos_disable_ui_scroll(objects.light);
    romeos_disable_ui_scroll(objects.blinds);
    romeos_eez_runtime_after_ui_init();
    lvgl_port_unlock();

    romeos_eez_runtime_services_init();
    Serial.println("[eez] ui_init done — setpoint, Wi‑Fi, NTP, sensor, alarm, MIC.");
}

void loop()
{
    if (lvgl_port_lock(0)) {
        ui_tick();
        lvgl_port_unlock();
    }
    romeos_eez_runtime_poll();
    delay(5);
}
