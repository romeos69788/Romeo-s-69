#include <Arduino.h>
#include <DHT.h>
#include <cmath>

#include "mb_outdoor_dht.h"
#include "pin_map.h"

namespace {

DHT g_dht(PIN_DHT_DATA, DHT22);

bool g_live = false;
int16_t g_temp_c_x10 = 0;
int16_t g_humidity_rh_x10 = 0;
uint32_t g_last_poll_ms = 0;
uint8_t g_fail_streak = 0;
bool g_logged_ok = false;

constexpr uint32_t k_poll_interval_ms = 2500u;
constexpr uint8_t k_fail_before_stale = 3u;

}  // namespace

void mb_outdoor_dht_begin()
{
    g_dht.begin();
    g_last_poll_ms = 0;
}

void mb_outdoor_dht_poll()
{
    const uint32_t now = millis();
    if (g_last_poll_ms != 0u && (now - g_last_poll_ms) < k_poll_interval_ms) {
        return;
    }
    g_last_poll_ms = now;

    const float t_c = g_dht.readTemperature();
    const float rh = g_dht.readHumidity();
    if (std::isnan(t_c) || std::isnan(rh)) {
        if (g_fail_streak < 255u) {
            ++g_fail_streak;
        }
        if (g_fail_streak >= k_fail_before_stale) {
            g_live = false;
        }
        Serial.println(F("[dht] CN5 read fail"));
        return;
    }

    g_fail_streak = 0;
    g_live = true;
    g_temp_c_x10 =
        static_cast<int16_t>(t_c * 10.0f + (t_c >= 0.0f ? 0.5f : -0.5f));
    g_humidity_rh_x10 = static_cast<int16_t>(rh * 10.0f + 0.5f);

    if (!g_logged_ok) {
        g_logged_ok = true;
        Serial.println(F("[dht] CN5 outdoor sensor OK"));
    }
    Serial.printf("[dht] T=%.1f C RH=%.1f %%\n", t_c, rh);
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
