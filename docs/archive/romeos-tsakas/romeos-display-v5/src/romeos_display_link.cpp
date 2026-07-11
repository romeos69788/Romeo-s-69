#include "romeos_display_link.h"
#include "romeos_ntp.h"
#include "romeos_wifi_nvs.h"

/* Ορίζεται στο platformio.ini του romeos-display-v2: χωρίς UDP/WiFi προς παλιά μητρική. */
#if defined(ROMEOS_DISPLAY_STANDALONE) && ROMEOS_DISPLAY_STANDALONE

#include <Arduino.h>
#include <WiFi.h>

void romeos_display_link_init(romeos_on_mb_data_fn on_mb,
                              romeos_get_setpoint_x10_fn get_setpoint_x10,
                              romeos_get_room_c_x10_fn get_room_c_x10)
{
    (void)on_mb;
    (void)get_setpoint_x10;
    (void)get_room_c_x10;
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    Serial.println(F("[romeos_link] STANDALONE (romeos-display-v2): WiFi off, no motherboard UDP."));
}

bool romeos_display_link_is_connected(void)
{
    return false;
}

bool romeos_display_link_home_wifi_assoc(void)
{
    return false;
}

bool romeos_display_link_telemetry_valid(void)
{
    return false;
}

void romeos_display_link_on_setpoint_ui_changed(void) {}

void romeos_display_link_notify_mb_setpoint_c_x10(int16_t sp_c_x10)
{
    (void)sp_c_x10;
}

void romeos_display_link_poll(void)
{
    /* Χωρίς Wi‑Fi· NTP μέσω romeos_ntp αν προστεθεί STA αργότερα. */
}

void romeos_display_link_on_home_wifi_saved_to_nvs(void) {}

#else

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <cstring>
#include <esp_err.h>
#include <esp_wifi.h>

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
bool g_warned_lan_ip = false;
/** Τελευταίος λόγος αποσύνδεσης STA (WIFI_REASON_*), από WiFi event. */
uint8_t g_sta_disc_reason = 0;
bool s_disp_wifi_logged = false;
bool s_udp_listening = false;

constexpr uint32_t k_rx_stale_ms = 10000;
constexpr uint8_t k_telemetry_streak_need = 5;
constexpr uint32_t k_tx_interval_ms = 400;
constexpr uint32_t k_wifi_retry_ms = 8000;

void trim_cstr(char *s)
{
    if (!s) {
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

/** SSID του AP στο οποίο είμαστε συνδεδεμένοι — προτιμάται esp_wifi (πιο αξιόπιστο από WiFi.SSID() σε ESP32). */
bool fill_sta_assoc_ssid(char *out, size_t cap)
{
    if (!out || cap < 2) {
        return false;
    }
    out[0] = '\0';
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    wifi_ap_record_t ap{};
    if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
        size_t i = 0;
        for (; i < sizeof(ap.ssid) && i < cap - 1U; ++i) {
            const char c = static_cast<char>(ap.ssid[i]);
            if (c == '\0') {
                break;
            }
            out[i] = c;
        }
        out[i] = '\0';
        if (out[0] != '\0') {
            return true;
        }
    }
    std::strncpy(out, WiFi.SSID().c_str(), cap - 1);
    out[cap - 1] = '\0';
    return out[0] != '\0';
}

bool connected_to_home_ssid()
{
    if (WiFi.status() != WL_CONNECTED || !g_has_home_creds) {
        return false;
    }
    char cur[33]{};
    if (!fill_sta_assoc_ssid(cur, sizeof(cur))) {
        return false;
    }
    char want[33]{};
    std::strncpy(want, g_home_ssid, sizeof(want) - 1);
    want[sizeof(want) - 1] = '\0';
    trim_cstr(cur);
    trim_cstr(want);
    return std::strcmp(cur, want) == 0;
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

void on_wifi_event(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
        g_sta_disc_reason = static_cast<uint8_t>(info.wifi_sta_disconnected.reason);
        s_udp_listening = false;
    }
}

/** UDP.begin μόνο με ενεργό Wi‑Fi — αλλιώς crash (lwIP) στο ESP32 Arduino 3.x. */
void ensure_udp_listen(void)
{
    if (s_udp_listening) {
        return;
    }
    if (WiFi.getMode() == WIFI_OFF || WiFi.status() != WL_CONNECTED) {
        return;
    }
    if (g_udp.begin(ROMEOS_UDP_PORT_DISPLAY_LISTEN)) {
        s_udp_listening = true;
    } else {
        Serial.println(F("[romeos_link] UDP listen failed"));
    }
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
    if (!romeos_wifi_nvs_has_home()) {
        romeos_wifi_nvs_apply_build_defaults();
    }
    g_has_home_creds =
        romeos_wifi_nvs_load_home(g_home_ssid, sizeof(g_home_ssid), g_home_pass, sizeof(g_home_pass));
    if (g_has_home_creds) {
        trim_cstr(g_home_ssid);
        /* Μην κάνεις trim στο password — κενά στο τέλος μπορεί να είναι μέρος του κωδικού. */
    }

    WiFi.persistent(false);
    WiFi.onEvent(on_wifi_event);

    if (g_has_home_creds) {
        wifi_start_home();
    }
#if defined(ROMEOS_WIFI_HOME_ONLY) && ROMEOS_WIFI_HOME_ONLY
    else {
        WiFi.mode(WIFI_OFF);
        Serial.println(F("[romeos_link] HOME_ONLY: no home Wi‑Fi in NVS yet."));
        Serial.println(F("[romeos_link] Serial 115200 → WIFI_HOME_HELP (SSID + password, 2.4 GHz)"));
    }
#else
    else {
        wifi_start_romeosmb();
    }
#endif

    ensure_udp_listen();

    Serial.println(F("[romeos_link] NVS home Wi‑Fi: Serial → WIFI_HOME_HELP"));
#if defined(ROMEOS_WIFI_HOME_ONLY) && ROMEOS_WIFI_HOME_ONLY
    Serial.println(F("[romeos_link] Μόνο οικιακό Wi‑Fi — μητρική αργότερα στο ίδιο LAN (UDP)"));
#endif
    if (g_has_home_creds) {
        Serial.printf(F("[romeos_link] Home Wi‑Fi NVS: \"%s\" — connecting (2.4 GHz)…\n"), g_home_ssid);
        Serial.println(F("[romeos_link] «Σπιτι» γαλάζιο + NTP όταν συνδεθεί στο σπίτι."));
        Serial.println(F("[romeos_link] LAN: set ROMEOS_MB_LAN_IP to motherboard IP on home subnet"));
        Serial.printf("[romeos_link] LAN build: ROMEOS_MB_LAN_IP=\"%s\"\n", ROMEOS_MB_LAN_IP);
    }
}

bool romeos_display_link_is_connected(void)
{
    const uint32_t now = millis();
    return WiFi.status() == WL_CONNECTED && g_last_rx_ms != 0u &&
           (now - g_last_rx_ms) < k_rx_stale_ms;
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

    /* Με αποθηκευμένο home SSID δεν κάνουμε fallback στο RomeosMB — αλλιώς «κολλάει» σε AP που δεν υπάρχει. */
    static uint32_t s_home_diag_ms = 0;
    if (g_has_home_creds && g_use_home_sta && WiFi.status() != WL_CONNECTED &&
        (now - s_home_diag_ms >= 30000)) {
        s_home_diag_ms = now;
        Serial.printf(
            "[romeos_link] home still offline ssid=\"%s\" wifi_status=%d last_disc_reason=%u "
            "(2=AUTH_EXPIRE/NO_AUTH 15=4WAY_TIMEOUT 201=NO_AP — δες 2.4GHz & κωδικό)\n",
            g_home_ssid,
            static_cast<int>(WiFi.status()),
            static_cast<unsigned>(g_sta_disc_reason));
    }

    if (WiFi.status() != WL_CONNECTED) {
        s_disp_wifi_logged = false;
        g_telemetry_streak = 0;
        if (now - g_last_wifi_retry_ms >= k_wifi_retry_ms) {
            g_last_wifi_retry_ms = now;
            Serial.println(F("[romeos_link] WiFi reconnecting..."));
            WiFi.disconnect(true);
            if (g_use_home_sta && g_has_home_creds) {
                WiFi.begin(g_home_ssid, g_home_pass);
            }
#if !defined(ROMEOS_WIFI_HOME_ONLY) || !ROMEOS_WIFI_HOME_ONLY
            else {
                WiFi.begin(ROMEOS_MB_AP_SSID, ROMEOS_MB_AP_PASS);
            }
#endif
        }
    } else {
        ensure_udp_listen();
        if (!s_disp_wifi_logged) {
            s_disp_wifi_logged = true;
            char ap_ssid[33]{};
            (void)fill_sta_assoc_ssid(ap_ssid, sizeof(ap_ssid));
            Serial.printf("[romeos_link] WiFi OK ssid=\"%s\" (arduino=\"%s\") ip=%s gw=%s\n",
                          ap_ssid,
                          WiFi.SSID().c_str(),
                          WiFi.localIP().toString().c_str(),
                          WiFi.gatewayIP().toString().c_str());
            Serial.printf("[romeos_link] HOME_ASSOC=%s want=\"%s\" use_home_sta=%d\n",
                          connected_to_home_ssid() ? "yes" : "no",
                          g_home_ssid,
                          static_cast<int>(g_use_home_sta));
        }
    }

    if (WiFi.status() == WL_CONNECTED &&
        g_last_rx_ms != 0 &&
        (now - g_last_rx_ms) >= k_rx_stale_ms) {
        g_telemetry_streak = 0;
    }

    if (!s_udp_listening) {
        return;
    }
    const int n = g_udp.parsePacket();
    if (n <= 0) {
        /* nothing */
    } else if (n >= static_cast<int>(sizeof(romeos_mb_to_display_v1_t))) {
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
    } else if (n >= 4) {
        /* Μητρική: ελάχιστο «σήμα» ζωής (χωρίς πλήρες πακέτο) — αρκούν 4 byte magic. */
        uint8_t buf[16]{};
        const int r = g_udp.read(buf, sizeof(buf));
        if (r >= 4) {
            uint32_t m = 0;
            std::memcpy(&m, buf, sizeof(m));
            if (m == ROMEOS_LINK_MAGIC) {
                g_last_rx_ms = now;
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

void romeos_display_link_on_home_wifi_saved_to_nvs(void)
{
    char ssid[33]{};
    char pass[65]{};
    const bool had = g_has_home_creds;
    const bool ok = romeos_wifi_nvs_load_home(ssid, sizeof(ssid), pass, sizeof(pass));

    if (!ok) {
        g_has_home_creds = false;
        g_home_ssid[0] = '\0';
        g_home_pass[0] = '\0';
        if (had) {
            WiFi.disconnect(true);
            g_last_wifi_retry_ms = millis();
#if defined(ROMEOS_WIFI_HOME_ONLY) && ROMEOS_WIFI_HOME_ONLY
            WiFi.mode(WIFI_OFF);
            Serial.println(F("[romeos_link] home creds cleared — Wi‑Fi off"));
#else
            Serial.println(F("[romeos_link] home creds cleared — STA RomeosMB"));
            wifi_start_romeosmb();
#endif
            s_disp_wifi_logged = false;
        }
        return;
    }

    trim_cstr(ssid);

    if (had && std::strcmp(g_home_ssid, ssid) == 0 && std::strcmp(g_home_pass, pass) == 0) {
        return;
    }

    std::strncpy(g_home_ssid, ssid, sizeof(g_home_ssid) - 1);
    g_home_ssid[sizeof(g_home_ssid) - 1] = '\0';
    std::strncpy(g_home_pass, pass, sizeof(g_home_pass) - 1);
    g_home_pass[sizeof(g_home_pass) - 1] = '\0';
    g_has_home_creds = true;
    g_last_wifi_retry_ms = millis();
    WiFi.disconnect(true);
    wifi_start_home();
    Serial.printf("[romeos_link] home Wi‑Fi from NVS applied (no reboot): \"%s\"\n", g_home_ssid);
    s_disp_wifi_logged = false;
}

#endif /* !ROMEOS_DISPLAY_STANDALONE */
