#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <cmath>

#include "mb_outdoor_dht.h"
#include "pin_map.h"

namespace {

DHT g_dht22(PIN_DHT_DATA, DHT22);
DHT g_dht11(PIN_DHT_DATA, DHT11);

bool g_live = false;
bool g_using_dht11 = false;
int16_t g_temp_c_x10 = 0;
int16_t g_humidity_rh_x10 = 0;
uint32_t g_last_poll_ms = 0;
uint8_t g_fail_streak = 0;
uint8_t g_last_idle_level = 255;
bool g_logged_ok = false;
uint32_t g_last_diag_log_ms = 0;

constexpr uint32_t k_poll_interval_ms = 2500u;
constexpr uint8_t k_fail_before_stale = 3u;
constexpr uint8_t k_try_dht11_after_fails = 12u;

bool read_dht_values(float *t_c, float *rh)
{
    if (t_c == nullptr || rh == nullptr) {
        return false;
    }

    g_last_idle_level =
        static_cast<uint8_t>(digitalRead(PIN_DHT_DATA) == HIGH ? 1u : 0u);

    /* ESP32: Wi-Fi modem sleep σπάει το one-wire timing του DHT. */
    WiFi.setSleep(WIFI_PS_NONE);

    portDISABLE_INTERRUPTS();
    DHT &sensor = g_using_dht11 ? g_dht11 : g_dht22;
    *t_c = sensor.readTemperature();
    *rh = sensor.readHumidity();
    portENABLE_INTERRUPTS();

    return !std::isnan(*t_c) && !std::isnan(*rh);
}

void log_dht_diag(const char *reason)
{
    const uint32_t now = millis();
    if (now - g_last_diag_log_ms < 5000u) {
        return;
    }
    g_last_diag_log_ms = now;
    Serial.printf("[dht] %s GPIO16 idle=%u fails=%u type=%s\n",
                  reason,
                  static_cast<unsigned>(g_last_idle_level),
                  static_cast<unsigned>(g_fail_streak),
                  g_using_dht11 ? "DHT11" : "DHT22");
    if (g_last_idle_level == 0u) {
        Serial.println(F("[dht] hint: DATA stuck LOW — check CN5 SING/GND swap, U11, pull-up R14"));
    } else {
        Serial.println(F("[dht] hint: line HIGH but no frame — wiring 3.3V, sensor type, cable length"));
    }
}

}  // namespace

void mb_outdoor_dht_begin()
{
    pinMode(PIN_DHT_DATA, INPUT_PULLUP);
    g_dht22.begin();
    g_dht11.begin();
    g_last_poll_ms = 0;
    g_using_dht11 = false;
    /* DHT22 power-up / settle after boot. */
    delay(2000);
    Serial.println(F("[dht] CN5 init GPIO16 (DHT22, fallback DHT11)"));
    log_dht_diag("init");
}

void mb_outdoor_dht_poll()
{
    const uint32_t now = millis();
    if (g_last_poll_ms != 0u && (now - g_last_poll_ms) < k_poll_interval_ms) {
        return;
    }
    g_last_poll_ms = now;

    if (!g_using_dht11 && g_fail_streak >= k_try_dht11_after_fails) {
        g_using_dht11 = true;
        Serial.println(F("[dht] switching to DHT11 read (same pin)"));
    }

    float t_c = NAN;
    float rh = NAN;
    if (!read_dht_values(&t_c, &rh)) {
        if (g_fail_streak < 255u) {
            ++g_fail_streak;
        }
        if (g_fail_streak >= k_fail_before_stale) {
            g_live = false;
        }
        log_dht_diag("read fail");
        return;
    }

    g_fail_streak = 0;
    g_live = true;
    g_temp_c_x10 =
        static_cast<int16_t>(t_c * 10.0f + (t_c >= 0.0f ? 0.5f : -0.5f));
    g_humidity_rh_x10 = static_cast<int16_t>(rh * 10.0f + 0.5f);

    if (!g_logged_ok) {
        g_logged_ok = true;
        Serial.printf("[dht] CN5 OK type=%s\n", g_using_dht11 ? "DHT11" : "DHT22");
    }
    Serial.printf("[dht] T=%.1f C RH=%.1f %% idle=%u\n",
                  t_c,
                  rh,
                  static_cast<unsigned>(g_last_idle_level));
}

bool mb_outdoor_dht_has_live()
{
    return g_live;
}

int16_t mb_outdoor_dht_temp_c_x10()
{
    return g_temp_c_x10;
}

int16_t mb_outdoor_dht_humidity_rh_x10()
{
    return g_humidity_rh_x10;
}

uint8_t mb_outdoor_dht_last_idle_level()
{
    return g_last_idle_level;
}

void mb_outdoor_dht_log_diagnostics()
{
    log_dht_diag("manual DIAG");
}

void mb_outdoor_dht_force_poll()
{
    g_last_poll_ms = 0;
    mb_outdoor_dht_poll();
}
