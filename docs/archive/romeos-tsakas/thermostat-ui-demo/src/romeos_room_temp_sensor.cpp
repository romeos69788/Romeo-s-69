/*
 * Θερμοκρασία χώρου (ROOM) — SHT4x (SHT40) ή SHT3x στο ίδιο I2C με GT911 (GPIO19/20),
 * ή SHT31 σε ξεχωριστό Wire1 (SDA/SCL από build flags).
 * Το poll γίνεται από loop(), όχι από LVGL timer, για λιγότερες συγκρούσεις στο I2C0.
 */

#include "romeos_room_temp_sensor.h"

#include <Arduino.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

#ifndef ROMEOS_ROOM_I2C_SDA
#define ROMEOS_ROOM_I2C_SDA 43
#endif
#ifndef ROMEOS_ROOM_I2C_SCL
#define ROMEOS_ROOM_I2C_SCL 44
#endif
#if (ROMEOS_ROOM_I2C_SDA == 19) && (ROMEOS_ROOM_I2C_SCL == 20)
#define ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS 1
#else
#define ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS 0
#endif

#ifndef ROMEOS_ROOM_TEMP_CORR_X10
#define ROMEOS_ROOM_TEMP_CORR_X10 0
#endif

#if ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS
#include "romeos_room_sht_touch_bus.h"
#endif

#if !ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS
#include <Adafruit_SHT31.h>
#include <Wire.h>
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#include <driver/uart.h>
#include <esp_err.h>
#endif
#endif

float g_room_temp_c = std::nanf("");
bool g_room_sensor_ready = false;
bool g_room_local_valid = false;

static constexpr float k_room_temp_corr_c =
    static_cast<float>(ROMEOS_ROOM_TEMP_CORR_X10) / 10.0f;

static bool s_room_sensor_live = false;
static RomeosRoomTempUiRefreshFn s_ui_refresh = nullptr;

#if !ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS
static TwoWire s_sht_wire{1};
static Adafruit_SHT31 s_room_adafruit{&s_sht_wire};
#endif

bool romeos_room_temp_sensor_has_live_sample()
{
    return s_room_sensor_live;
}

void romeos_room_temp_sensor_set_ui_refresh_callback(RomeosRoomTempUiRefreshFn fn)
{
    s_ui_refresh = fn;
}

#if ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS
#include <driver/i2c.h>
#include <esp_err.h>

static bool i2c0_ping_addr7(uint8_t addr7)
{
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    if (!c) {
        return false;
    }
    i2c_master_start(c);
    i2c_master_write_byte(c,
                          static_cast<uint8_t>((static_cast<uint32_t>(addr7) << 1) | I2C_MASTER_WRITE),
                          true);
    i2c_master_stop(c);
    const esp_err_t e = i2c_master_cmd_begin(I2C_NUM_0, c, pdMS_TO_TICKS(120));
    i2c_cmd_link_delete(c);
    return e == ESP_OK;
}

static void log_i2c0_scan()
{
    Serial.println(F("[i2c0] scan I2C_NUM_0 (8..0x77):"));
    uint8_t n = 0;
    for (int a = 8; a < 0x78; ++a) {
        if (i2c0_ping_addr7(static_cast<uint8_t>(a))) {
            Serial.printf("[i2c0]  ACK 0x%02X", a);
            if (a == 0x44 || a == 0x45) {
                Serial.print("  (SHT?)");
            }
            if (a == 0x38) {
                Serial.print("  (AHT20/21?)");
            }
            if (a == 0x5d || a == 0x14) {
                Serial.print("  (GT911)");
            }
            Serial.println();
            ++n;
        }
    }
    if (n == 0) {
        Serial.println(F("[i2c0]  no ACK — wiring or wrong COM (CH340 needs USB_CDC_ON_BOOT=0)."));
    }
}

static bool read_both_shared(float *raw_tc, float *rh)
{
    return romeos_room_sht_touch_bus_read_both(raw_tc, rh);
}

static void reinit_shared()
{
    romeos_room_sht_touch_bus_soft_reset();
    g_room_sensor_ready = romeos_room_sht_touch_bus_begin_auto();
}

#else

static bool read_both_shared(float *raw_tc, float *rh)
{
    return s_room_adafruit.readBoth(raw_tc, rh);
}

static void reinit_shared()
{
    if (!s_room_adafruit.begin(0x44)) {
        (void)s_room_adafruit.begin(0x45);
    }
}

#endif

void romeos_room_temp_sensor_after_board_begin()
{
    g_room_local_valid = false;
    s_room_sensor_live = false;
    g_room_temp_c = std::nanf("");

#if ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS
    /* Άφησε το GT911 / host να σταθεροποιηθεί πριν από SHT/AHT στο ίδιο bus. */
    delay(280);
    log_i2c0_scan();
    Serial.println(F("[room_temp] init: shared I2C0 — SHT4x/SHT3x, fallback AHT20/21 @0x38"));
    for (int attempt = 0; attempt < 5 && !g_room_sensor_ready; ++attempt) {
        if (attempt > 0) {
            delay(150);
            Serial.printf("[room_temp] retry init attempt %d/5\n", attempt + 1);
        }
        g_room_sensor_ready = romeos_room_sht_touch_bus_begin_auto();
    }
    Serial.printf("[room_temp] family=%s  %s\n",
                  romeos_room_sht_touch_bus_family_name(),
                  g_room_sensor_ready ? "ready" : "NOT FOUND");
#else
    const int sda = ROMEOS_ROOM_I2C_SDA;
    const int scl = ROMEOS_ROOM_I2C_SCL;
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    if (sda == 43 && scl == 44) {
        if (uart_is_driver_installed(UART_NUM_0)) {
            (void)uart_driver_delete(UART_NUM_0);
        }
    }
#endif
    s_sht_wire.begin(sda, scl);
    s_sht_wire.setClock(100000);
    Serial.printf("[room_temp] Wire1 SDA=%d SCL=%d\n", sda, scl);
    g_room_sensor_ready = s_room_adafruit.begin(0x44);
    if (!g_room_sensor_ready) {
        g_room_sensor_ready = s_room_adafruit.begin(0x45);
    }
    Serial.printf("[room_temp] Adafruit SHT31 %s\n", g_room_sensor_ready ? "ready" : "NOT FOUND");
#endif

    if (g_room_sensor_ready) {
        float t0 = NAN;
        float rh0 = NAN;
        if (read_both_shared(&t0, &rh0)) {
            Serial.printf("[room_temp] first read: T=%.2f RH=%.1f%%\n",
                          static_cast<double>(t0),
                          static_cast<double>(rh0));
        } else {
            Serial.println(F("[room_temp] first read FAILED (CRC/bus)."));
        }
    }
    Serial.flush();
}

#ifdef ROMEOS_ROOM_SENSOR_DEBUG
static void serial_diag(const char *tag, float raw_tc, float rh, float filt_c, float shown_c)
{
    static uint32_t s_last_ms = 0;
    const uint32_t now = millis();
    if (now - s_last_ms < 2000u) {
        return;
    }
    s_last_ms = now;
    if (std::isfinite(rh)) {
        Serial.printf("[room_temp] %s raw=%.3f rh=%.1f%% filt=%.3f shown=%.3f\n",
                      tag,
                      static_cast<double>(raw_tc),
                      static_cast<double>(rh),
                      static_cast<double>(filt_c),
                      static_cast<double>(shown_c));
    } else {
        Serial.printf("[room_temp] %s raw=%.3f rh=nan filt=%.3f shown=%.3f\n",
                      tag,
                      static_cast<double>(raw_tc),
                      static_cast<double>(filt_c),
                      static_cast<double>(shown_c));
    }
}
#endif

void romeos_room_temp_sensor_poll_in_app_loop()
{
    static uint32_t s_last_poll_ms = 0;
    const uint32_t now = millis();
    if (now - s_last_poll_ms < 2000u) {
        return;
    }
    s_last_poll_ms = now;

    if (!g_room_sensor_ready) {
#if ROMEOS_ROOM_I2C_SHARED_TOUCH_BUS
        /* Το scan στο boot βρήκε μόνο GT911: αν ο αισθητήρας «κάθισε» μετά ή η επαφή είναι intermittent, ξαναδοκιμάζουμε. */
        static uint32_t s_last_late_detect_ms = 0;
        if (now - s_last_late_detect_ms >= 45000u) {
            s_last_late_detect_ms = now;
            if (romeos_room_sht_touch_bus_begin_auto()) {
                g_room_sensor_ready = true;
                Serial.printf("[room_temp] late I2C detect OK  family=%s\n",
                              romeos_room_sht_touch_bus_family_name());
                Serial.flush();
            }
        }
#endif
        return;
    }

    static uint8_t s_ok_streak = 0;
    static uint32_t s_last_bad_log_ms = 0;
    static uint8_t s_bad_rows = 0;
    static bool s_have_filtered = false;
    static float s_filtered_tc = 0.0f;
    static uint8_t s_spike_candidate_streak = 0;

    float rh = NAN;
    float raw_tc = NAN;
    const float tc = read_both_shared(&raw_tc, &rh) ? raw_tc : NAN;

    if (!std::isfinite(tc) || tc < -40.0f || tc > 90.0f) {
        s_ok_streak = 0;
        if (now - s_last_bad_log_ms >= 2000u) {
            s_last_bad_log_ms = now;
            Serial.printf("[room_temp] bad read tc=%f\n", static_cast<double>(tc));
            Serial.flush();
        }
        if (++s_bad_rows >= 5u) {
            s_bad_rows = 0;
            reinit_shared();
        }
        return;
    }
    s_bad_rows = 0;

    if (s_have_filtered) {
        const float delta = std::fabs(tc - s_filtered_tc);
        if (delta > 4.0f) {
            if (++s_spike_candidate_streak < 2u) {
#ifdef ROMEOS_ROOM_SENSOR_DEBUG
                serial_diag("spike_rej", tc, rh, s_filtered_tc, s_filtered_tc + k_room_temp_corr_c);
#endif
                return;
            }
        } else {
            s_spike_candidate_streak = 0;
        }
        s_filtered_tc = s_filtered_tc * 0.8f + tc * 0.2f;
    } else {
        s_filtered_tc = tc;
        s_have_filtered = true;
        s_spike_candidate_streak = 0;
    }

    g_room_temp_c = s_filtered_tc + k_room_temp_corr_c;
    s_room_sensor_live = true;
#ifdef ROMEOS_ROOM_SENSOR_DEBUG
    serial_diag("tick", tc, rh, s_filtered_tc, g_room_temp_c);
#endif

    if (s_ok_streak < 255) {
        ++s_ok_streak;
    }

    if (s_ui_refresh) {
        s_ui_refresh();
    }

    if (!g_room_local_valid) {
        if (s_ok_streak < 3u) {
            return;
        }
        g_room_local_valid = true;
    }
}
