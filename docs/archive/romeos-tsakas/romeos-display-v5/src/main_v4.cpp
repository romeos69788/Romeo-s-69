/*
 * romeos-display-v4 — καθαρό entrypoint.
 * Board (Viewe) + LVGL port + SquareLine `ui_init` → φορτώνει `ui_Screen1`.
 * Δεν μεταφέρεται λογική από v3· ό,τι χρειάζεται για λειτουργίες θα προστίθεται εδώ ή σε νέα modules.
 */

#include <Arduino.h>
#include <cassert>
#include <cstdio>

#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "lvgl_v8_port.h"
#include "romeos_display_link.h"
#include "romeos_ntp.h"
#include "romeos_wifi_nvs.h"
#include "romeos_v4_clock.h"
#include "romeos_v4_dashboard_mirror.h"
#include "romeos_v4_features.h"
#include "romeos_v4_setpoint_ui.h"
#include "romeos_v4_wifi_ui.h"
#include "squareline/ui.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println("========================================");
    Serial.println("*** ROMEOS-DISPLAY-V4 (Viewe) firmware ***");
    Serial.println("*** SquareLine: WITE THERMOSTAT / Screen1 default ***");
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

    romeos_v4_init_room_sensor();

    assert(lvgl_port_init(board->getLCD(), board->getTouch()));

    if (!lvgl_port_lock(-1)) {
        Serial.println("[v4] ERROR: lvgl_port_lock failed");
        return;
    }
    ui_init();
    romeos_v4_after_ui_init();
    lvgl_port_unlock();
    Serial.println("[v4] ui_init + after_ui_init done (LVGL task owns refresh).");

    /* Wi‑Fi σπιτιού (NVS + STA) και UDP προς μητρική (setpoint + telemetry). */
    romeos_ntp_init(romeos_v4_clock_on_ntp_sync);
    romeos_display_link_init(romeos_v4_setpoint_on_mb_packet, romeos_v4_setpoint_get_c_x10, nullptr);
    romeos_v4_wifi_status_poll();
    Serial.println(F("[v4] Serial 115200: WIFI_HOME_SET / WIFI_HOME_SHOW / WIFI_HOME_CLR · MB→οθόνη UDP :9001 (magic 4B ή πλήρες πακέτο)"));
}

void loop()
{
    romeos_v4_alarm_buzzer_keepalive();
    if (romeos_wifi_nvs_poll_serial()) {
        romeos_display_link_on_home_wifi_saved_to_nvs();
    }
    romeos_display_link_poll();
    romeos_v4_clock_poll();
    romeos_v4_wifi_status_poll();
    romeos_v4_dashboard_mirror_poll();
    romeos_v4_loop();
    delay(5);
}
