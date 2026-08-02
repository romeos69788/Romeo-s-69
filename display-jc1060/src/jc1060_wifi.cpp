/*
 * Home Wi‑Fi + NTP clock for JC1060 (ESP32-P4 · C6 companion).
 *
 * Note: C6 RF via ESP-Hosted is ~25–30 dB weaker than the Viewe panel STA
 * at the same desk. Prefer BSSID+channel begin; avoid hard disconnect thrash.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "screens.h"
#include "jc1060_wifi.h"

#ifndef ROMEOS_WIFI_SSID
#define ROMEOS_WIFI_SSID ""
#endif
#ifndef ROMEOS_WIFI_PASS
#define ROMEOS_WIFI_PASS ""
#endif

static bool s_want_wifi = false;
static bool s_last_connected = false;
static bool s_ntp_configured = false;
static bool s_ntp_ok = false;
static bool s_ssid_in_scan = false;
static bool s_have_bssid = false;
static int s_ap_channel = 0;
static int s_ap_rssi = 0;
static uint32_t s_wifi_start_ms = 0;
static uint32_t s_last_try_ms = 0;
static uint32_t s_last_scan_ms = 0;
static uint32_t s_last_clock_ms = 0;
static uint32_t s_last_status_ms = 0;

static char s_connect_ssid[33] = {0};
static uint8_t s_bssid[6] = {0};

/* Lab: panel sees home AP on channel 3. */
static constexpr int k_home_channel_hint = 3;

static const char *k_month_el[] = {
    "Ιανουαρίου", "Φεβρουαρίου", "Μαρτίου", "Απριλίου", "Μαΐου", "Ιουνίου",
    "Ιουλίου", "Αυγούστου", "Σεπτεμβρίου", "Οκτωβρίου", "Νοεμβρίου", "Δεκεμβρίου",
};

static bool ssid_eq_ci(const char *a, const char *b)
{
    if (!a || !b) {
        return false;
    }
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static const char *active_ssid(void)
{
    return s_connect_ssid[0] ? s_connect_ssid : ROMEOS_WIFI_SSID;
}

static bool wifi_scan_pick_target(void)
{
    Serial.println("[jc1060-wifi] scanning 2.4 GHz (long dwell)…");
    bool saw = false;
    int total_n = 0;
    s_last_scan_ms = millis();

    /* Keep last known BSSID unless we find a fresher beacon. */
    for (int pass = 0; pass < 2 && !saw; pass++) {
        const int n = WiFi.scanNetworks(false, true, false, 500);
        if (n < 0) {
            Serial.printf("[jc1060-wifi] scan pass %d failed (%d)\n", pass, n);
            delay(200);
            continue;
        }
        total_n += n;
        for (int i = 0; i < n; i++) {
            const String id = WiFi.SSID(i);
            const int ch = WiFi.channel(i);
            const int rssi = WiFi.RSSI(i);
            Serial.printf("[jc1060-wifi]   %2d  rssi=%4d  ch=%2d  enc=%d  %s\n",
                          i, rssi, ch, (int)WiFi.encryptionType(i), id.c_str());
            if (id.length() > 0 && ssid_eq_ci(id.c_str(), ROMEOS_WIFI_SSID)) {
                if (!saw || rssi > s_ap_rssi) {
                    strncpy(s_connect_ssid, id.c_str(), sizeof(s_connect_ssid) - 1);
                    s_connect_ssid[sizeof(s_connect_ssid) - 1] = '\0';
                    s_ap_channel = ch;
                    s_ap_rssi = rssi;
                    const uint8_t *b = WiFi.BSSID(i);
                    if (b) {
                        memcpy(s_bssid, b, 6);
                        s_have_bssid = true;
                    }
                    saw = true;
                }
            }
        }
        WiFi.scanDelete();
        if (!saw) {
            delay(150);
        }
    }

    Serial.printf("[jc1060-wifi] scan rows~%d want=\"%s\" %s exact=\"%s\" ch=%d rssi=%d "
                  "bssid=%02x:%02x:%02x:%02x:%02x:%02x pass_len=%u\n",
                  total_n, ROMEOS_WIFI_SSID, saw ? "FOUND" : "NOT IN SCAN",
                  saw ? s_connect_ssid : "-", s_ap_channel, s_ap_rssi,
                  s_bssid[0], s_bssid[1], s_bssid[2], s_bssid[3], s_bssid[4], s_bssid[5],
                  (unsigned)strlen(ROMEOS_WIFI_PASS));
    s_ssid_in_scan = saw;
    return saw;
}

static void wifi_begin_associate(void)
{
    if (s_connect_ssid[0] == '\0') {
        strncpy(s_connect_ssid, ROMEOS_WIFI_SSID, sizeof(s_connect_ssid) - 1);
        s_connect_ssid[sizeof(s_connect_ssid) - 1] = '\0';
    }

    const char *ssid = active_ssid();
    const int ch = s_ap_channel > 0 ? s_ap_channel : k_home_channel_hint;

    /* Soft stop only — hard erase crashes ESP-Hosted semaphore on P4+C6. */
    WiFi.disconnect(false, false);
    delay(100);

    if (s_have_bssid && ch > 0) {
        WiFi.begin(ssid, ROMEOS_WIFI_PASS, ch, s_bssid);
        Serial.printf("[jc1060-wifi] begin SSID=\"%s\" ch=%d bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
                      ssid, ch,
                      s_bssid[0], s_bssid[1], s_bssid[2],
                      s_bssid[3], s_bssid[4], s_bssid[5]);
    } else if (ch > 0) {
        WiFi.begin(ssid, ROMEOS_WIFI_PASS, ch);
        Serial.printf("[jc1060-wifi] begin SSID=\"%s\" ch=%d\n", ssid, ch);
    } else {
        WiFi.begin(ssid, ROMEOS_WIFI_PASS);
        Serial.printf("[jc1060-wifi] begin SSID=\"%s\"\n", ssid);
    }
    s_last_try_ms = millis();
}

static void wifi_begin_now(bool do_scan)
{
    if (do_scan) {
        (void)wifi_scan_pick_target();
    }
    wifi_begin_associate();
}

void jc1060_wifi_begin(void)
{
    if (ROMEOS_WIFI_SSID[0] == '\0') {
        Serial.println("[jc1060-wifi] no SSID — icon stays blue");
        ui_set_wifi_connected(false);
        return;
    }

    s_want_wifi = true;
    s_wifi_start_ms = millis();
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
#if defined(WIFI_POWER_19_5dBm)
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
#endif
    delay(500);
    wifi_begin_now(/*do_scan=*/true);
}

static void update_clock_from_time(const struct tm &t)
{
    char hh[8];
    char mm[8];
    char day[8];
    snprintf(hh, sizeof(hh), "%02d", t.tm_hour);
    snprintf(mm, sizeof(mm), "%02d", t.tm_min);
    snprintf(day, sizeof(day), "%d", t.tm_mday);

    const char *month = (t.tm_mon >= 0 && t.tm_mon <= 11) ? k_month_el[t.tm_mon] : "—";
    const bool colon_on = (t.tm_sec % 2) != 0;
    ui_clock_set(hh, mm, day, month, colon_on);
}

void jc1060_wifi_tick(void)
{
    if (!s_want_wifi) {
        return;
    }

    const bool connected = (WiFi.status() == WL_CONNECTED);
    if (connected != s_last_connected) {
        s_last_connected = connected;
        ui_set_wifi_connected(connected);
        if (connected) {
            Serial.printf("[jc1060-wifi] OK ssid=\"%s\" ip=%s rssi=%d\n",
                          active_ssid(),
                          WiFi.localIP().toString().c_str(),
                          WiFi.RSSI());
            if (!s_ntp_configured) {
                setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
                tzset();
                configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4",
                             "pool.ntp.org", "time.google.com", "time.nist.gov");
                s_ntp_configured = true;
                Serial.println("[jc1060-wifi] NTP start (Athens)");
            }
        } else {
            Serial.println("[jc1060-wifi] lost link — icon blue");
            s_ntp_ok = false;
        }
    }

    if (!connected) {
        const uint32_t now = millis();
        if (now - s_last_status_ms >= 5000U) {
            s_last_status_ms = now;
            Serial.printf("[jc1060-wifi] status=%d elapsed=%lus ssid=\"%s\"\n",
                          (int)WiFi.status(),
                          (unsigned long)((now - s_wifi_start_ms) / 1000U),
                          active_ssid());
        }

        /* If we have BSSID: re-associate without scan for a while (avoids hosted crash). */
        if (s_have_bssid && (now - s_last_scan_ms) < 45000U) {
            if (now - s_last_try_ms >= 12000U) {
                wifi_begin_associate();
            }
            return;
        }

        if (now - s_last_try_ms >= 25000U) {
            wifi_begin_now(/*do_scan=*/true);
        }
        return;
    }

    if (!s_ntp_configured) {
        return;
    }

    if (millis() - s_last_clock_ms < 250U) {
        return;
    }
    s_last_clock_ms = millis();

    struct tm t {};
    if (!getLocalTime(&t, 0)) {
        return;
    }
    if (!s_ntp_ok) {
        s_ntp_ok = true;
        Serial.printf("[jc1060-wifi] NTP OK %04d-%02d-%02d %02d:%02d:%02d\n",
                      t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                      t.tm_hour, t.tm_min, t.tm_sec);
    }
    update_clock_from_time(t);
}
