#include "romeos_display_link.h"
#include "romeos_ntp.h"
#include "romeos_wifi_nvs.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <cstring>

namespace {

WiFiUDP g_udp;
romeos_on_mb_data_fn g_on_mb = nullptr;
romeos_get_setpoint_x10_fn g_get_sp = nullptr;
romeos_get_room_c_x10_fn g_get_room = nullptr;
uint32_t g_last_rx_ms = 0;
uint32_t g_last_tx_ms = 0;
uint32_t g_last_wifi_retry_ms = 0;
uint8_t g_last_mb_flags = 0;
static uint8_t g_telemetry_streak = 0;

char g_home_ssid[33]{};
char g_home_pass[65]{};
bool g_has_home_creds = false;
bool g_use_home_sta = false;
uint32_t g_home_connect_started_ms = 0;
bool g_home_fallback_done = false;
bool g_warned_lan_ip = false;
bool s_disp_wifi_logged = false;

constexpr uint32_t k_rx_stale_ms = 10000;
constexpr uint8_t k_telemetry_streak_need = 5;
constexpr uint32_t k_tx_interval_ms = 400;
constexpr uint32_t k_wifi_retry_ms = 8000;
constexpr uint32_t k_home_connect_timeout_ms = 45000;

bool connected_to_home_ssid()
{
    if (WiFi.status() != WL_CONNECTED || !g_has_home_creds) {
        return false;
    }
    return strcmp(WiFi.SSID().c_str(), g_home_ssid) == 0;
}

IPAddress mb_tx_destination()
{
    if (WiFi.status() != WL_CONNECTED) {
        return IPAddress(0, 0, 0, 0);
    }
    if (connected_to_home_ssid()) {
        IPAddress ip{};
        if (ROMEOS_MB_LAN_IP[0] != '\0' && ip.fromString(ROMEOS_MB_LAN_IP)) {
            return ip;
        }
        if (!g_warned_lan_ip) {
            g_warned_lan_ip = true;
            Serial.println(
                F("[romeos_link] home Wi‑Fi OK but ROMEOS_MB_LAN_IP empty — add to platformio.ini"));
        }
        return IPAddress(0, 0, 0, 0);
    }
    IPAddress gw = WiFi.gatewayIP();
    if (gw == IPAddress(0, 0, 0, 0)) {
        return IPAddress(192, 168, 4, 1);
    }
    return gw;
}

void wifi_start_home()
{
    g_use_home_sta = true;
    g_home_fallback_done = false;
    g_home_connect_started_ms = millis();
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(g_home_ssid, g_home_pass);
    Serial.printf("[romeos_link] STA -> home \"%s\"\n", g_home_ssid);
}

void wifi_start_romeosmb()
{
    g_use_home_sta = false;
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ROMEOS_MB_AP_SSID, ROMEOS_MB_AP_PASS);
    Serial.printf("[romeos_link] STA -> AP \"%s\"\n", ROMEOS_MB_AP_SSID);
}

}  // namespace

void romeos_display_link_init(romeos_on_mb_data_fn on_mb,
                              romeos_get_setpoint_x10_fn get_setpoint_x10,
                              romeos_get_room_c_x10_fn get_room_c_x10)
{
    g_on_mb = on_mb;
    g_get_sp = get_setpoint_x10;
    g_get_room = get_room_c_x10;
    g_last_rx_ms = 0;
    g_last_tx_ms = 0;
    g_last_wifi_retry_ms = 0;
    g_telemetry_streak = 0;
    g_warned_lan_ip = false;

    romeos_wifi_nvs_init();
    g_has_home_creds =
        romeos_wifi_nvs_load_home(g_home_ssid, sizeof(g_home_ssid), g_home_pass, sizeof(g_home_pass));

    WiFi.persistent(false);

    if (g_has_home_creds) {
        wifi_start_home();
    } else {
        wifi_start_romeosmb();
    }

    if (!g_udp.begin(ROMEOS_UDP_PORT_DISPLAY_LISTEN)) {
        Serial.println(F("[romeos_link] UDP listen failed"));
    }

    Serial.println(F("[romeos_link] NVS home Wi‑Fi: Serial → WIFI_HOME_HELP"));
    if (g_has_home_creds) {
        Serial.println(F("[romeos_link] LAN: set ROMEOS_MB_LAN_IP to motherboard IP on home subnet"));
        Serial.printf("[romeos_link] LAN build: ROMEOS_MB_LAN_IP=\"%s\"\n", ROMEOS_MB_LAN_IP);
    }
}

bool romeos_display_link_is_connected(void)
{
    return WiFi.status() == WL_CONNECTED &&
           (millis() - g_last_rx_ms) < k_rx_stale_ms;
}

bool romeos_display_link_home_wifi_assoc(void)
{
    return connected_to_home_ssid();
}

bool romeos_display_link_telemetry_valid(void)
{
    return romeos_display_link_is_connected() &&
           (g_telemetry_streak >= k_telemetry_streak_need);
}

void romeos_display_link_on_setpoint_ui_changed(void)
{
    g_last_tx_ms = 0;
}

void romeos_display_link_notify_mb_setpoint_c_x10(int16_t sp_c_x10)
{
    (void)sp_c_x10;
    /* Αναγκάσε άμεσο UDP setpoint (αν STA/UDP ενεργά) ώστε να μην μείνει παλιά τιμή στο air. */
    g_last_tx_ms = 0;
}

void romeos_display_link_poll(void)
{
    const uint32_t now = millis();

    if (g_has_home_creds && g_use_home_sta && !g_home_fallback_done &&
        WiFi.status() != WL_CONNECTED &&
        (now - g_home_connect_started_ms >= k_home_connect_timeout_ms)) {
        g_home_fallback_done = true;
        Serial.println(F("[romeos_link] home STA timeout — fallback RomeosMB"));
        WiFi.disconnect(true);
        wifi_start_romeosmb();
        g_last_wifi_retry_ms = now;
    }

    if (WiFi.status() != WL_CONNECTED) {
        s_disp_wifi_logged = false;
        g_telemetry_streak = 0;
        if (now - g_last_wifi_retry_ms >= k_wifi_retry_ms) {
            g_last_wifi_retry_ms = now;
            Serial.println(F("[romeos_link] WiFi reconnecting..."));
            WiFi.disconnect(true);
            if (g_use_home_sta) {
                WiFi.begin(g_home_ssid, g_home_pass);
            } else {
                WiFi.begin(ROMEOS_MB_AP_SSID, ROMEOS_MB_AP_PASS);
            }
        }
    } else {
        if (!s_disp_wifi_logged) {
            s_disp_wifi_logged = true;
            Serial.printf("[romeos_link] WiFi OK ssid=\"%s\" ip=%s gw=%s\n",
                          WiFi.SSID().c_str(),
                          WiFi.localIP().toString().c_str(),
                          WiFi.gatewayIP().toString().c_str());
        }
    }

    if (WiFi.status() == WL_CONNECTED &&
        g_last_rx_ms != 0 &&
        (now - g_last_rx_ms) >= k_rx_stale_ms) {
        g_telemetry_streak = 0;
    }

    const int n = g_udp.parsePacket();
    if (n >= static_cast<int>(sizeof(romeos_mb_to_display_v1_t))) {
        romeos_mb_to_display_v1_t pkt{};
        const int r =
            g_udp.read(reinterpret_cast<uint8_t *>(&pkt), sizeof(pkt));
        if (r == static_cast<int>(sizeof(pkt)) &&
            pkt.magic == ROMEOS_LINK_MAGIC &&
            (pkt.version == 1u || pkt.version == static_cast<uint8_t>(ROMEOS_LINK_VER))) {
            g_last_rx_ms = now;
            g_last_mb_flags = pkt.flags;
            if ((pkt.flags & ROMEOS_MB_FLAG_TELEMETRY_VALID) != 0u) {
                if (g_telemetry_streak < k_telemetry_streak_need) {
                    ++g_telemetry_streak;
                }
            } else {
                g_telemetry_streak = 0;
            }
            if (g_on_mb) {
                g_on_mb(&pkt);
            }
        }
    }

    if (WiFi.status() != WL_CONNECTED || g_get_sp == nullptr) {
        romeos_ntp_poll();
        return;
    }

    if (now - g_last_tx_ms < k_tx_interval_ms) {
        romeos_ntp_poll();
        return;
    }
    g_last_tx_ms = now;

    romeos_display_to_mb_v1_t out{};
    out.magic = ROMEOS_LINK_MAGIC;
    out.version = static_cast<uint8_t>(ROMEOS_LINK_VER);
    out.msg_type = ROMEOS_MSG_SETPOINT;
    out.setpoint_c_x10 = g_get_sp();
    out.room_from_display_c_x10 = ROMEOS_ROOM_FROM_DISPLAY_INVALID;
    if (g_get_room != nullptr) {
        out.room_from_display_c_x10 = g_get_room();
    }

    const IPAddress dst = mb_tx_destination();
    if (dst != IPAddress(0, 0, 0, 0)) {
        if (g_udp.beginPacket(dst, ROMEOS_UDP_PORT_MB_LISTEN)) {
            g_udp.write(reinterpret_cast<const uint8_t *>(&out), sizeof(out));
            g_udp.endPacket();
        }
    }

    romeos_ntp_poll();
}
