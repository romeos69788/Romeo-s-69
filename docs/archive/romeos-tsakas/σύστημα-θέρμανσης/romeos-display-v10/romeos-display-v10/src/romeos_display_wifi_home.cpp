#include "romeos_display_wifi_home.h"

#include "romeos_wifi_home_defaults.h"
#include "romeos_wifi_nvs.h"

#include <Arduino.h>
#include <WiFi.h>
#include <cstring>

namespace {

char g_home_ssid[33]{};
char g_home_pass[65]{};
bool g_have_home_creds = false;
uint32_t g_last_wifi_retry_ms = 0;
bool g_logged_wifi_ok = false;

constexpr uint32_t k_wifi_retry_ms = 8000;

void trim_cstr(char *s)
{
    if (s == nullptr) {
        return;
    }
    size_t n = std::strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t') {
        ++i;
    }
    if (i > 0) {
        std::memmove(s, s + i, n - i + 1);
    }
}

bool connected_to_home_ssid()
{
    if (WiFi.status() != WL_CONNECTED || !g_have_home_creds) {
        return false;
    }
    char cur[33]{};
    std::strncpy(cur, WiFi.SSID().c_str(), sizeof(cur) - 1);
    char want[33]{};
    std::strncpy(want, g_home_ssid, sizeof(want) - 1);
    trim_cstr(cur);
    trim_cstr(want);
    return std::strcmp(cur, want) == 0;
}

void reload_home_creds_from_nvs()
{
    g_have_home_creds =
        romeos_wifi_nvs_load_home(g_home_ssid, sizeof(g_home_ssid), g_home_pass, sizeof(g_home_pass));
    if (g_have_home_creds) {
        trim_cstr(g_home_ssid);
    }
}

}  // namespace

void romeos_display_wifi_home_begin(void)
{
    romeos_wifi_nvs_init();
    if (!romeos_wifi_nvs_has_home()) {
        romeos_wifi_nvs_apply_build_defaults();
    }
    reload_home_creds_from_nvs();
    if (!g_have_home_creds) {
        Serial.println(F("[wifi_home] no home creds — Serial → WIFI_HOME_HELP"));
        return;
    }

    WiFi.persistent(false);
    if (WiFi.getMode() == WIFI_OFF) {
        WiFi.mode(WIFI_STA);
    }
    WiFi.setSleep(false);
    WiFi.begin(g_home_ssid, g_home_pass);
    Serial.printf("[wifi_home] STA -> \"%s\" (internet/NTP — link MB = ESP-NOW)\n", g_home_ssid);
}

void romeos_display_wifi_home_poll(void)
{
    if (!g_have_home_creds) {
        return;
    }

    const uint32_t now = millis();
    if (WiFi.status() != WL_CONNECTED) {
        g_logged_wifi_ok = false;
        if (now - g_last_wifi_retry_ms >= k_wifi_retry_ms) {
            g_last_wifi_retry_ms = now;
            WiFi.disconnect(true);
            WiFi.begin(g_home_ssid, g_home_pass);
        }
        return;
    }

    if (!g_logged_wifi_ok && connected_to_home_ssid()) {
        g_logged_wifi_ok = true;
        Serial.printf("[wifi_home] OK ip=%s gw=%s\n",
                      WiFi.localIP().toString().c_str(),
                      WiFi.gatewayIP().toString().c_str());
    }
}

bool romeos_display_wifi_home_assoc(void)
{
    return connected_to_home_ssid();
}

void romeos_display_wifi_home_on_nvs_saved(void)
{
    const bool had = g_have_home_creds;
    char old_ssid[33]{};
    if (had) {
        std::strncpy(old_ssid, g_home_ssid, sizeof(old_ssid) - 1);
    }

    reload_home_creds_from_nvs();
    if (!g_have_home_creds) {
        if (had) {
            WiFi.disconnect(true);
            Serial.println(F("[wifi_home] creds cleared — Wi‑Fi off"));
        }
        return;
    }

    if (had && std::strcmp(old_ssid, g_home_ssid) == 0) {
        return;
    }

    g_logged_wifi_ok = false;
    g_last_wifi_retry_ms = 0;
    WiFi.disconnect(true);
    WiFi.begin(g_home_ssid, g_home_pass);
    Serial.printf("[wifi_home] NVS updated — reconnecting to \"%s\"\n", g_home_ssid);
}
