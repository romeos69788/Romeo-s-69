#include "romeos_ntp.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <sys/time.h>
#include <time.h>

namespace {

romeos_ntp_synced_fn s_cb = nullptr;
bool s_sntp_running = false;
uint32_t s_sntp_started_ms = 0;
bool s_ntp_sync_done = false;
bool s_ntp_warned_slow = false;

portMUX_TYPE s_pending_mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool s_have_pending = false;
struct tm s_pending_tm {};

constexpr uint32_t k_resync_period_ms = 24u * 3600u * 1000u;

void ntp_sync_notification_cb(struct timeval *tv)
{
    if (!tv || !s_cb) {
        return;
    }
    s_ntp_sync_done = true;
    struct tm local {};
    localtime_r(&tv->tv_sec, &local);
    portENTER_CRITICAL(&s_pending_mux);
    s_pending_tm = local;
    s_have_pending = true;
    portEXIT_CRITICAL(&s_pending_mux);
}

void ntp_stack_start()
{
    if (s_sntp_running) {
        return;
    }
    /* Ελλάδα: EET = UTC+2, EEST = UTC+3 — ρητό EEST-3 ώστε newlib/ESP να μην μαντεύει offset DST. */
    setenv("TZ", "EET-2EEST-3,M3.5.0/3,M10.5.0/4", 1);
    tzset();

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_set_time_sync_notification_cb(ntp_sync_notification_cb);
    esp_sntp_init();
    s_sntp_running = true;
    s_sntp_started_ms = millis();
    s_ntp_sync_done = false;
    s_ntp_warned_slow = false;
    Serial.println(F("[romeos_ntp] SNTP started (pool.ntp.org + time.google.com, TZ=EET-2/EEST-3)"));
}

void ntp_stack_stop()
{
    if (!s_sntp_running) {
        return;
    }
    esp_sntp_stop();
    s_sntp_running = false;
    Serial.println(F("[romeos_ntp] SNTP stopped"));
}

}  // namespace

void romeos_ntp_init(romeos_ntp_synced_fn on_synced_wall)
{
    s_cb = on_synced_wall;
}

void romeos_ntp_poll(void)
{
    if (!s_cb) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        ntp_stack_stop();
        return;
    }

    if (!s_sntp_running) {
        ntp_stack_start();
    }

    bool have = false;
    struct tm copy {};
    portENTER_CRITICAL(&s_pending_mux);
    if (s_have_pending) {
        have = true;
        s_have_pending = false;
        copy = s_pending_tm;
    }
    portEXIT_CRITICAL(&s_pending_mux);

    if (have) {
        s_cb(&copy);
        Serial.printf("[romeos_ntp] wall clock set from NTP: %02d:%02d:%02d\n",
                      copy.tm_hour,
                      copy.tm_min,
                      copy.tm_sec);
    }

    const uint32_t now = millis();
    if (s_sntp_running && !s_ntp_sync_done && !s_ntp_warned_slow && s_sntp_started_ms != 0 &&
        (now - s_sntp_started_ms) >= 60000u) {
        s_ntp_warned_slow = true;
        Serial.println(
            F("[romeos_ntp] no NTP sync after 60s: need route to internet (UDP 123). "
              "RomeosMB-only / captive portal usually blocks this — clock stays manual/NVS."));
    }

    if (s_sntp_running && s_sntp_started_ms != 0 &&
        (now - s_sntp_started_ms) >= k_resync_period_ms) {
        ntp_stack_stop();
        ntp_stack_start();
        Serial.println(F("[romeos_ntp] periodic SNTP restart (24h)"));
    }
}
