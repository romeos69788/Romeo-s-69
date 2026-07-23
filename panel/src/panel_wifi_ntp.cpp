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

/** Exact EEZ positions from designer (ρολόι / ρολόι_1 / ρολόι_2 / ρολόι_3). */
void fix_one_clock(lv_obj_t *hh, lv_obj_t *colon, lv_obj_t *mm, lv_obj_t *ampm)
{
    if (hh) {
        lv_obj_set_pos(hh, 642, 12);
        lv_obj_set_size(hh, 39, 33);
        lv_obj_set_style_text_align(hh, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    }
    if (colon) {
        lv_obj_set_pos(colon, 683, 10);
        lv_obj_set_size(colon, 7, 33);
        lv_obj_set_style_text_align(colon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }
    if (mm) {
        lv_obj_set_pos(mm, 692, 12);
        lv_obj_set_size(mm, 36, 33);
        lv_obj_set_style_text_align(mm, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    }
    if (ampm) {
        lv_obj_clear_flag(ampm, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ampm, 737, 12);
        lv_obj_set_size(ampm, 34, 22);
        lv_obj_set_style_text_align(ampm, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    }
}

void fix_clock_layout_once()
{
    if (s_layout_fixed) {
        return;
    }

    fix_one_clock(objects._____, objects.______1, objects.______2, objects.______3);
    fix_one_clock(objects.______4, objects.______5, objects.______6, objects.______7);
    fix_one_clock(objects.______8, objects.______9, objects.______10, objects.______11);
    fix_one_clock(objects.______12, objects.______13, objects.______14, objects.______15);
    fix_one_clock(objects.______16, objects.______17, objects.______18, objects.______19);
    fix_one_clock(objects.______20, objects.______21, objects.______22, objects.______23);
    fix_one_clock(objects.______24, objects.______25, objects.______26, objects.______27);

    Serial.println("[panel-wifi] clock layout on all 7 screens");
    s_layout_fixed = true;
}

void apply_clock_pack(lv_obj_t *hh,
                      lv_obj_t *colon,
                      lv_obj_t *mm,
                      lv_obj_t *ampm,
                      lv_obj_t *day,
                      lv_obj_t *wday,
                      lv_obj_t *month,
                      bool colon_on)
{
    if (hh) {
        lv_label_set_text(hh, s_hh);
    }
    if (colon) {
        lv_obj_set_style_text_opa(colon, colon_on ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
        lv_label_set_text(colon, ":");
    }
    if (mm) {
        lv_label_set_text(mm, s_mm);
    }
    if (ampm) {
        lv_obj_clear_flag(ampm, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ampm, s_ampm);
    }
    if (day) {
        lv_label_set_text(day, s_day);
    }
    if (wday) {
        lv_label_set_text(wday, s_wday);
    }
    if (month) {
        lv_label_set_text(month, s_month);
    }
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

void panel_wifi_ntp_apply_ui(int screen_id)
{
    if (!s_have_ui) {
        return;
    }

    fix_clock_layout_once();

    const bool colon_on = (s_sec % 2) != 0;
    switch (screen_id) {
    case 1:  // SCREEN_ID_MAIN
        apply_clock_pack(objects._____, objects.______1, objects.______2, objects.______3,
                         objects.obj6, objects.obj7, objects.obj8, colon_on);
        break;
    case 2:  // WATER
        apply_clock_pack(objects.______4, objects.______5, objects.______6, objects.______7,
                         objects.obj9, objects.obj10, objects.obj11, colon_on);
        break;
    case 3:  // HP
        apply_clock_pack(objects.______8, objects.______9, objects.______10, objects.______11,
                         objects.obj14, objects.obj15, objects.obj16, colon_on);
        break;
    case 4:  // OUT
        apply_clock_pack(objects.______12, objects.______13, objects.______14, objects.______15,
                         objects.obj19, objects.obj20, objects.obj21, colon_on);
        break;
    case 5:  // BOILER
        apply_clock_pack(objects.______16, objects.______17, objects.______18, objects.______19,
                         objects.obj24, objects.obj25, objects.obj26, colon_on);
        break;
    case 6:  // SYSTEM
        apply_clock_pack(objects.______20, objects.______21, objects.______22, objects.______23,
                         objects.obj29, objects.obj30, objects.obj31, colon_on);
        break;
    case 7:  // WIFI
        apply_clock_pack(objects.______24, objects.______25, objects.______26, objects.______27,
                         objects.obj34, objects.obj35, objects.obj36, colon_on);
        break;
    default:
        break;
    }
}
