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
bool s_layout_fixed = false;
int s_sec = 0;

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
    // 24-hour digits + visible AM/PM
    snprintf(s_hh, sizeof(s_hh), "%02d", t.tm_hour);
    snprintf(s_mm, sizeof(s_mm), "%02d", t.tm_min);
    snprintf(s_ampm, sizeof(s_ampm), "%s", (t.tm_hour >= 12) ? "PM" : "AM");
    s_sec = t.tm_sec;
    snprintf(s_day, sizeof(s_day), "%d", t.tm_mday);

    const char *wd = (t.tm_wday >= 0 && t.tm_wday <= 6) ? k_wday_el[t.tm_wday] : "—";
    const char *mo = (t.tm_mon >= 0 && t.tm_mon <= 11) ? k_month_el[t.tm_mon] : "—";
    snprintf(s_wday, sizeof(s_wday), "%s", wd);
    snprintf(s_month, sizeof(s_month), "%s", mo);
    s_have_ui = true;
}

/** Match hour↔colon gap to colon↔minutes gap (EEZ right side). Keep AM/PM visible. */
void fix_clock_layout_once()
{
    if (s_layout_fixed || !objects._____ || !objects.______1 || !objects.______2) {
        return;
    }

    const lv_coord_t colon_x = lv_obj_get_x(objects.______1);
    const lv_coord_t min_x = lv_obj_get_x(objects.______2);
    const lv_font_t *font = lv_obj_get_style_text_font(objects.______1, LV_PART_MAIN);
    const lv_coord_t colon_tw =
        font ? lv_txt_get_width(":", 1, font, 0, LV_TEXT_FLAG_NONE)
             : lv_obj_get_width(objects.______1);

    // Same clear space: end of ':' glyph → start of minutes
    lv_coord_t gap = min_x - (colon_x + colon_tw);
    if (gap < 0) {
        gap = 0;
    }

    constexpr lv_coord_t k_hour_w = 42;
    const lv_coord_t hour_x = colon_x - gap - k_hour_w;

    lv_obj_set_pos(objects._____, hour_x, lv_obj_get_y(objects._____));
    lv_obj_set_size(objects._____, k_hour_w, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(objects._____, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

    if (objects.______3) {
        lv_obj_clear_flag(objects.______3, LV_OBJ_FLAG_HIDDEN);
    }

    Serial.printf("[panel-wifi] clock gap=%d px (hour|:%d|:|min)\n", (int)gap, (int)gap);
    s_layout_fixed = true;
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

    fix_clock_layout_once();

    if (objects._____) {
        lv_label_set_text(objects._____, s_hh);
    }
    if (objects.______1) {
        // Blink colon: on odd seconds, off even (1s on / 1s off)
        const bool colon_on = (s_sec % 2) != 0;
        lv_obj_set_style_text_opa(objects.______1,
                                  colon_on ? LV_OPA_COVER : LV_OPA_TRANSP,
                                  LV_PART_MAIN);
        lv_label_set_text(objects.______1, ":");
    }
    if (objects.______2) {
        lv_label_set_text(objects.______2, s_mm);
    }
    if (objects.______3) {
        lv_obj_clear_flag(objects.______3, LV_OBJ_FLAG_HIDDEN);
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
