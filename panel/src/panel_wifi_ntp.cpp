#include "panel_wifi_ntp.h"

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include <lvgl.h>

#include "ui/screens.h"

#ifndef ROMEOS_WIFI_SSID
#define ROMEOS_WIFI_SSID ""
#endif
#ifndef ROMEOS_WIFI_PASS
#define ROMEOS_WIFI_PASS ""
#endif

namespace {

bool s_wifi_started = false;
bool s_ntp_configured = false;
bool s_synced_once = false;
uint32_t s_wifi_start_ms = 0;
uint32_t s_last_status_ms = 0;

char s_hh[8];
char s_mm[8];
char s_ampm[8];
char s_day[8];
char s_wday[32];
char s_month[40];
bool s_have_ui = false;

const char *k_wday_el[] = {
    "Κυριακή",
    "Δευτέρα",
    "Τρίτη",
    "Τετάρτη",
    "Πέμπτη",
    "Παρασκευή",
    "Σάββατο",
};

const char *k_month_el[] = {
    "Ιανουαρίου",
    "Φεβρουαρίου",
    "Μαρτίου",
    "Απριλίου",
    "Μαΐου",
    "Ιουνίου",
    "Ιουλίου",
    "Αυγούστου",
    "Σεπτεμβρίου",
    "Οκτωβρίου",
    "Νοεμβρίου",
    "Δεκεμβρίου",
};

void fill_buffers(const struct tm &t)
{
    int hour12 = t.tm_hour % 12;
    if (hour12 == 0) {
        hour12 = 12;
    }
    snprintf(s_hh, sizeof(s_hh), "%02d", hour12);
    snprintf(s_mm, sizeof(s_mm), "%02d", t.tm_min);
    snprintf(s_ampm, sizeof(s_ampm), "%s", (t.tm_hour >= 12) ? "PM" : "AM");
    snprintf(s_day, sizeof(s_day), "%d", t.tm_mday);

    const char *wd = (t.tm_wday >= 0 && t.tm_wday <= 6) ? k_wday_el[t.tm_wday] : "—";
    const char *mo = (t.tm_mon >= 0 && t.tm_mon <= 11) ? k_month_el[t.tm_mon] : "—";
    snprintf(s_wday, sizeof(s_wday), "%s", wd);
    snprintf(s_month, sizeof(s_month), "%s", mo);
    s_have_ui = true;
}

}  // namespace

void panel_wifi_ntp_begin()
{
    if (ROMEOS_WIFI_SSID[0] == '\0') {
        Serial.println("[panel-wifi] no ROMEOS_WIFI_SSID — clock stays EEZ placeholder");
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.disconnect(true, true);
    delay(200);

    Serial.println("[panel-wifi] scanning 2.4 GHz…");
    const int n = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
    bool saw = false;
    for (int i = 0; i < n; i++) {
        const String id = WiFi.SSID(i);
        Serial.printf("[panel-wifi]   %2d  rssi=%4d  ch=%2d  %s\n",
                      i,
                      WiFi.RSSI(i),
                      WiFi.channel(i),
                      id.c_str());
        if (id == ROMEOS_WIFI_SSID) {
            saw = true;
        }
    }
    Serial.printf("[panel-wifi] scan done n=%d target=%s %s\n",
                  n,
                  ROMEOS_WIFI_SSID,
                  saw ? "FOUND" : "NOT IN SCAN");

    WiFi.begin(ROMEOS_WIFI_SSID, ROMEOS_WIFI_PASS);
    s_wifi_started = true;
    s_wifi_start_ms = millis();
    Serial.printf("[panel-wifi] connecting to SSID=%s …\n", ROMEOS_WIFI_SSID);
}

void panel_wifi_ntp_poll()
{
    if (!s_wifi_started) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        const uint32_t now = millis();
        if (now - s_last_status_ms >= 5000u) {
            s_last_status_ms = now;
            Serial.printf("[panel-wifi] connecting… status=%d elapsed=%lus\n",
                          (int)WiFi.status(),
                          (unsigned long)((now - s_wifi_start_ms) / 1000u));
        }
        return;
    }

    if (!s_ntp_configured) {
        setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
        tzset();
        configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.google.com", "time.nist.gov");
        s_ntp_configured = true;
        Serial.printf("[panel-wifi] connected IP=%s — NTP start\n", WiFi.localIP().toString().c_str());
    }

    struct tm t {};
    if (!getLocalTime(&t, 0)) {
        return;
    }

    if (!s_synced_once) {
        s_synced_once = true;
        Serial.printf("[panel-wifi] NTP OK %04d-%02d-%02d %02d:%02d:%02d (Athens)\n",
                      t.tm_year + 1900,
                      t.tm_mon + 1,
                      t.tm_mday,
                      t.tm_hour,
                      t.tm_min,
                      t.tm_sec);
    }

    fill_buffers(t);
}

void panel_wifi_ntp_apply_ui()
{
    if (!s_have_ui) {
        return;
    }
    if (objects._____) {
        lv_label_set_text(objects._____, s_hh);
    }
    if (objects.______2) {
        lv_label_set_text(objects.______2, s_mm);
    }
    if (objects.______3) {
        lv_label_set_text(objects.______3, s_ampm);
    }
    if (objects.obj6) {
        lv_label_set_text(objects.obj6, s_day);
    }
    if (objects.obj7) {
        lv_label_set_text(objects.obj7, s_wday);
    }
    if (objects.obj8) {
        lv_label_set_text(objects.obj8, s_month);
    }
}
