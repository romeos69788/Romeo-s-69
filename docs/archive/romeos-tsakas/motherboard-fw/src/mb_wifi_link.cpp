#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "mb_outdoor_dht.h"
#include "mb_wifi_link.h"
#include "mb_mqtt.h"
#include "pin_map.h"
#include "romeos_link_types.h"
#include "romeos_wifi_nvs.h"

namespace {

constexpr char k_nvs_ns[] = "romeos_mb";
constexpr char k_nvs_sp[] = "sp_x10";

WiFiUDP g_udp;
IPAddress g_display_ip;
uint32_t g_last_display_rx_ms = 0;
int16_t g_setpoint_from_display_x10 = 210;
int16_t g_room_from_display_c_x10 = ROMEOS_ROOM_FROM_DISPLAY_INVALID;
uint32_t g_demo_t0_ms = 0;
bool g_alarm_active = false;
uint32_t g_suppress_display_udp_setpoint_until_ms = 0;
bool g_logged_udp_setpoint_suppressed = false;

char s_home_ssid[33]{};
char s_home_pass[65]{};
bool s_have_home_creds = false;

enum class MbPhase : uint8_t {
    ApOnly,
    StaHomeConnecting,
    StaHome,
    FailoverApSta,
};
MbPhase s_phase = MbPhase::ApOnly;
uint32_t s_sta_connect_started_ms = 0;
uint32_t s_home_disconnected_since_ms = 0;
uint32_t s_failover_drop_ap_mark_ms = 0;

constexpr uint32_t k_sta_home_connect_timeout_ms = 45000;
constexpr uint32_t k_home_down_before_failover_ms = 5u * 60u * 1000u;
constexpr uint32_t k_failover_drop_ap_stable_ms = 30000;

void try_seed_home_wifi_from_build_flags()
{
#if defined(ROMEOS_HOME_SSID) && defined(ROMEOS_HOME_PASS)
    char ssid[40]{};
    char pass[68]{};
    bool need_write = true;
    if (romeos_wifi_nvs_load_home(ssid, sizeof(ssid), pass, sizeof(pass))) {
        need_write = (std::strcmp(ssid, ROMEOS_HOME_SSID) != 0) ||
                     (std::strcmp(pass, ROMEOS_HOME_PASS) != 0);
    }
    if (!need_write) {
        return;
    }
    if (romeos_wifi_nvs_save_home(ROMEOS_HOME_SSID, ROMEOS_HOME_PASS)) {
        Serial.println(F("[wifi_nvs] home Wi-Fi written from build flags (empty or mismatch)"));
    } else {
        Serial.println(F("[wifi_nvs] failed writing home Wi-Fi from build flags"));
    }
#endif
}

void restart_mb_udp()
{
    g_udp.stop();
    delay(50);
    if (!g_udp.begin(ROMEOS_UDP_PORT_MB_LISTEN)) {
        Serial.println(F("[wifi] UDP begin failed"));
    }
}

void load_setpoint_from_nvs()
{
    Preferences p;
    if (!p.begin(k_nvs_ns, true)) {
        Serial.println(F("[nvs] no setpoint namespace — using default 21.0 C"));
        return;
    }
    const int16_t v = static_cast<int16_t>(p.getShort(k_nvs_sp, 210));
    p.end();
    if (v >= 50 && v <= 350) {
        g_setpoint_from_display_x10 = v;
        Serial.printf("[nvs] setpoint loaded: %d.%d C\n",
                      v / 10,
                      std::abs(static_cast<int>(v % 10)));
    }
}

void save_setpoint_to_nvs(int16_t sp)
{
    Preferences p;
    if (!p.begin(k_nvs_ns, false)) {
        Serial.println(F("[nvs] open RW failed"));
        return;
    }
    p.putShort(k_nvs_sp, sp);
    p.end();
}

bool relay_coil_active(int pin)
{
    return digitalRead(pin) == LOW;
}

void build_tx(romeos_mb_to_display_v1_t *out)
{
    std::memset(out, 0, sizeof(*out));
    out->magic = ROMEOS_LINK_MAGIC;
    out->version = static_cast<uint8_t>(ROMEOS_LINK_VER);
    out->flags = 0x00u;
    const uint32_t now_ms = millis();
    const bool room_from_display_fresh =
        g_room_from_display_c_x10 != ROMEOS_ROOM_FROM_DISPLAY_INVALID &&
        g_last_display_rx_ms != 0u &&
        (now_ms - g_last_display_rx_ms) < 20000u;
#ifdef ROMEOS_MB_TELEMETRY_OK
    out->flags |= ROMEOS_MB_FLAG_TELEMETRY_VALID;
#else
    /*
     * Χωρίς breadboard αισθητήρες: στέλνουμε έγκυρο bit0 ώστε η οθόνη να μην μένει σε «--»
     * (το debounce streak απαιτεί TELEMETRY_VALID). Τιμές = σταθερά placeholders μέχρι
     * ενεργοποίηση ROMEOS_MB_TELEMETRY_OK + πραγματικούς αισθητήρες.
     */
    out->flags |= ROMEOS_MB_FLAG_TELEMETRY_VALID;
    if (room_from_display_fresh) {
        out->room_display_c_x10 = g_room_from_display_c_x10;
    } else {
        out->room_display_c_x10 = 215; /* 21.5 °C placeholder */
    }
    out->outdoor_c_x10 = 120;        /* 12.0 °C */
    out->solar_c_x10 = 450;          /* 45.0 °C */
    out->boiler_c_x10 = 550;         /* 55.0 °C */
    out->supply_c_x10 = 400;       /* 40.0 °C */
    out->return_c_x10 = 340;       /* 34.0 °C */
    /* bit0: δεξιά panel placeholders · bit1: outdoor CN5 live (OUT corner). */
    out->mb_proto_reserved = 0x01u;
#endif
    if (g_alarm_active) {
        out->flags |= ROMEOS_MB_FLAG_ALARM_ACTIVE;
    }

#ifdef ROMEOS_MB_TELEMETRY_OK
    const uint32_t t = millis() - g_demo_t0_ms;
    const float ph = t * 0.0003f;
    const float s = std::sin(ph);
    if (room_from_display_fresh) {
        out->room_display_c_x10 = g_room_from_display_c_x10;
    } else {
        out->room_display_c_x10 =
            static_cast<int16_t>(200 + static_cast<int>(s * 35.0f));
    }
    out->outdoor_c_x10 =
        static_cast<int16_t>(120 + static_cast<int>(s * 40.0f));
    out->solar_c_x10 =
        static_cast<int16_t>(450 + static_cast<int>(s * 80.0f));
    out->boiler_c_x10 =
        static_cast<int16_t>(550 + static_cast<int>(s * 60.0f));
    out->supply_c_x10 =
        static_cast<int16_t>(400 + static_cast<int>(s * 50.0f));
    out->return_c_x10 =
        static_cast<int16_t>(340 + static_cast<int>(s * 40.0f));
#endif

    out->relay_k1_on = relay_coil_active(PIN_REL_K1) ? 1u : 0u;
    out->relay_k2_on = relay_coil_active(PIN_REL_K2) ? 1u : 0u;
    out->relay_k3_on = relay_coil_active(PIN_REL_K3) ? 1u : 0u;
    out->relay_k4_on = relay_coil_active(PIN_REL_K4) ? 1u : 0u;
    out->relay_k5_on = relay_coil_active(PIN_REL_K5) ? 1u : 0u;
    out->relay_k6_on = relay_coil_active(PIN_REL_K6) ? 1u : 0u;

    out->heat_pump_on = out->relay_k1_on;
    out->pump1_on = out->relay_k2_on;
    out->pump2_on = out->relay_k3_on;
    out->heater_on = out->relay_k4_on;

    out->flow_sig_high =
        static_cast<uint8_t>(digitalRead(PIN_FLOW_SIG) == HIGH ? 1u : 0u);
    out->defrost_active =
        static_cast<uint8_t>(digitalRead(PIN_DEFROST) == HIGH ? 1u : 0u);

    if (mb_outdoor_dht_has_live()) {
        out->outdoor_c_x10 = mb_outdoor_dht_temp_c_x10();
        out->mb_proto_reserved |= 0x02u;
    }

    out->setpoint_c_x10 = mb_wifi_link_get_setpoint_c_x10();
}

void start_softap_only()
{
    WiFi.mode(WIFI_AP);
    const bool ok = WiFi.softAP(ROMEOS_MB_AP_SSID, ROMEOS_MB_AP_PASS, 1, 0, 4);
    Serial.printf("[wifi] SoftAP \"%s\" %s IP=%s ch=1\n",
                  ROMEOS_MB_AP_SSID,
                  ok ? "ok" : "FAIL",
                  WiFi.softAPIP().toString().c_str());
    delay(80);
    restart_mb_udp();
}

void enter_failover_ap_sta()
{
    Serial.println(F("[wifi] home STA lost 5 min — enabling SoftAP + STA (failover)"));
    WiFi.mode(WIFI_AP_STA);
    (void)WiFi.softAP(ROMEOS_MB_AP_SSID, ROMEOS_MB_AP_PASS, 1, 0, 4);
    WiFi.begin(s_home_ssid, s_home_pass);
    s_phase = MbPhase::FailoverApSta;
    s_home_disconnected_since_ms = 0;
    s_failover_drop_ap_mark_ms = 0;
    Serial.printf("[wifi] SoftAP IP=%s  STA reconnecting to home...\n",
                  WiFi.softAPIP().toString().c_str());
    delay(80);
    restart_mb_udp();
}

bool ip_on_mb_softap_client_subnet(const IPAddress &a)
{
    return a[0] == 192 && a[1] == 168 && a[2] == 4;
}

bool display_on_same_lan_as_sta(const IPAddress &disp)
{
    if (disp == IPAddress(0, 0, 0, 0)) {
        return false;
    }
    if (ip_on_mb_softap_client_subnet(disp)) {
        return false;
    }
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    const IPAddress loc = WiFi.localIP();
    if (loc[0] == 0) {
        return false;
    }
    return disp[0] == loc[0] && disp[1] == loc[1] && disp[2] == loc[2];
}

void mb_wifi_tick()
{
    const uint32_t now = millis();

    if (s_phase == MbPhase::StaHomeConnecting) {
        if (WiFi.status() == WL_CONNECTED) {
            s_phase = MbPhase::StaHome;
            s_home_disconnected_since_ms = 0;
            Serial.printf("[wifi] STA home OK ip=%s gw=%s\n",
                          WiFi.localIP().toString().c_str(),
                          WiFi.gatewayIP().toString().c_str());
            restart_mb_udp();
            return;
        }
        if (now - s_sta_connect_started_ms >= k_sta_home_connect_timeout_ms) {
            Serial.println(F("[wifi] home STA timeout — SoftAP only (RomeosMB)"));
            s_phase = MbPhase::ApOnly;
            start_softap_only();
        }
        return;
    }

    if (s_phase == MbPhase::StaHome) {
        if (WiFi.status() == WL_CONNECTED) {
            s_home_disconnected_since_ms = 0;
            return;
        }
        if (s_home_disconnected_since_ms == 0) {
            s_home_disconnected_since_ms = now;
            Serial.println(F("[wifi] STA home disconnected — timer 5 min to failover"));
        } else if (now - s_home_disconnected_since_ms >= k_home_down_before_failover_ms) {
            enter_failover_ap_sta();
        }
        return;
    }

    if (s_phase == MbPhase::FailoverApSta) {
        const bool sta_ok = (WiFi.status() == WL_CONNECTED);
        const bool fresh_rx =
            (g_display_ip != IPAddress(0, 0, 0, 0)) &&
            (now - g_last_display_rx_ms < 4000u);
        const bool same_lan = sta_ok && display_on_same_lan_as_sta(g_display_ip);

        if (same_lan && fresh_rx) {
            if (s_failover_drop_ap_mark_ms == 0) {
                s_failover_drop_ap_mark_ms = now;
                Serial.println(F("[wifi] display on home LAN — will drop SoftAP after 30s stable"));
            } else if (now - s_failover_drop_ap_mark_ms >= k_failover_drop_ap_stable_ms) {
                Serial.println(F("[wifi] dropping SoftAP — STA home only"));
                WiFi.softAPdisconnect(true);
                WiFi.mode(WIFI_STA);
                WiFi.begin(s_home_ssid, s_home_pass);
                s_phase = MbPhase::StaHomeConnecting;
                s_sta_connect_started_ms = now;
                s_failover_drop_ap_mark_ms = 0;
                delay(80);
                restart_mb_udp();
            }
        } else {
            s_failover_drop_ap_mark_ms = 0;
        }
        return;
    }
}

IPAddress udp_broadcast_for_tx()
{
    if (s_phase == MbPhase::ApOnly || s_phase == MbPhase::FailoverApSta) {
        return IPAddress(192, 168, 4, 255);
    }
    const IPAddress loc = WiFi.localIP();
    if (loc[0] == 0) {
        /* Χωρίς STA IP το global broadcast οδηγεί σε endPacket() errors (lwIP). */
        return IPAddress(0, 0, 0, 0);
    }
    return IPAddress(loc[0], loc[1], loc[2], 255);
}

/** UDP TX μόνο όταν υπάρχει έγκυρη διεπαφή (SoftAP ενεργό ή STA με IP). */
bool can_send_mb_udp()
{
    if (s_phase == MbPhase::ApOnly || s_phase == MbPhase::FailoverApSta) {
        return WiFi.softAPIP()[0] != 0;
    }
    if (s_phase == MbPhase::StaHomeConnecting || s_phase == MbPhase::StaHome) {
        return WiFi.status() == WL_CONNECTED && WiFi.localIP()[0] != 0;
    }
    return false;
}

}  // namespace

void mb_wifi_link_begin()
{
    g_demo_t0_ms = millis();
    load_setpoint_from_nvs();

    romeos_wifi_nvs_init();
    try_seed_home_wifi_from_build_flags();
    s_have_home_creds =
        romeos_wifi_nvs_load_home(s_home_ssid, sizeof(s_home_ssid), s_home_pass, sizeof(s_home_pass));

    WiFi.persistent(false);

    /*
     * Σημαντικό (ESP32 Arduino 3.x / lwIP): το WiFiUDP::begin πριν από WiFi.mode()
     * μπορεί να αφήσει εσωτερικές ουρές κενές → assert στο πρώτο poll (xQueueSemaphoreTake).
     * Πρώτα mode + STA ή SoftAP, μετά UDP.
     */
    Serial.println(F("[wifi] NVS home Wi‑Fi: Serial → WIFI_HOME_HELP"));
    if (s_have_home_creds) {
        Serial.println(F("[wifi_nvs] home credentials present — trying STA to home first"));
        WiFi.mode(WIFI_STA);
        WiFi.begin(s_home_ssid, s_home_pass);
        s_phase = MbPhase::StaHomeConnecting;
        s_sta_connect_started_ms = millis();
    } else {
        s_phase = MbPhase::ApOnly;
        start_softap_only();
        Serial.println(F("[wifi_nvs] no home Wi‑Fi in NVS — SoftAP only"));
    }

    delay(50);
    if (!g_udp.begin(ROMEOS_UDP_PORT_MB_LISTEN)) {
        Serial.println(F("[wifi] UDP begin failed"));
    }
}

void mb_wifi_link_poll()
{
    mb_wifi_tick();

    const int n = g_udp.parsePacket();
    /* Πυρήνας πακέτου 10 byte (παλιά οθόνη)· +2 byte room_from_display (νέα οθόνη). */
    constexpr int k_display_udp_core = 10;
    if (n >= k_display_udp_core) {
        romeos_display_to_mb_v1_t msg{};
        std::memset(&msg, 0, sizeof(msg));
        const int r =
            g_udp.read(reinterpret_cast<uint8_t *>(&msg), sizeof(msg));
        if (r < k_display_udp_core) {
            goto skip_display_rx;
        }
        if (r < static_cast<int>(sizeof(romeos_display_to_mb_v1_t))) {
            msg.room_from_display_c_x10 = ROMEOS_ROOM_FROM_DISPLAY_INVALID;
        }
        if (msg.magic == ROMEOS_LINK_MAGIC &&
            (msg.version == 1u || msg.version == static_cast<uint8_t>(ROMEOS_LINK_VER))) {
            g_display_ip = g_udp.remoteIP();
            g_last_display_rx_ms = millis();
            if (msg.room_from_display_c_x10 != ROMEOS_ROOM_FROM_DISPLAY_INVALID) {
                g_room_from_display_c_x10 = msg.room_from_display_c_x10;
            }
            if (msg.msg_type == ROMEOS_MSG_SETPOINT) {
                static bool s_logged_mqtt_udp_ignore;
                if (mb_mqtt_is_connected()) {
                    if (!s_logged_mqtt_udp_ignore) {
                        s_logged_mqtt_udp_ignore = true;
                        Serial.println(F(
                            "[link] ignore display UDP setpoint (MQTT connected — app/remote authority)"));
                    }
                } else {
                    s_logged_mqtt_udp_ignore = false;

                    const uint32_t now = millis();
                    int16_t sp = msg.setpoint_c_x10;
                    if (sp < 50) {
                        sp = 50;
                    }
                    if (sp > 350) {
                        sp = 350;
                    }

                    /*
                     * Χωρίς MQTT: κρατάμε μικρό “hold” μετά από εντολή που ήρθε από αλλού (μελλοντικά)
                     * ώστε να μην γίνεται NVS thrash από out-of-order UDP.
                     */
                    if (now < g_suppress_display_udp_setpoint_until_ms &&
                        sp != g_setpoint_from_display_x10) {
                        if (!g_logged_udp_setpoint_suppressed) {
                            g_logged_udp_setpoint_suppressed = true;
                            Serial.printf(
                                "[link] ignore display UDP setpoint %d.%d C (hold; mb=%d.%d C)\n",
                                sp / 10,
                                std::abs(static_cast<int>(sp % 10)),
                                g_setpoint_from_display_x10 / 10,
                                std::abs(static_cast<int>(g_setpoint_from_display_x10 % 10)));
                        }
                    } else {
                        g_logged_udp_setpoint_suppressed = false;
                        mb_wifi_link_apply_setpoint_c_x10(sp);
                    }
                }
            }
        }
    }
skip_display_rx:

    static uint32_t last_tx = 0;
    const uint32_t now = millis();
    if (now - last_tx < 200u) {
        return;
    }
    last_tx = now;

    if (!can_send_mb_udp()) {
        return;
    }

    romeos_mb_to_display_v1_t tx{};
    build_tx(&tx);

    const bool have_client =
        g_display_ip != IPAddress(0, 0, 0, 0) &&
        (now - g_last_display_rx_ms < 10000u);

    if (have_client) {
        if (g_udp.beginPacket(g_display_ip, ROMEOS_UDP_PORT_DISPLAY_LISTEN)) {
            g_udp.write(reinterpret_cast<const uint8_t *>(&tx), sizeof(tx));
            g_udp.endPacket();
        }
    } else {
        const IPAddress bcast = udp_broadcast_for_tx();
        if (bcast[0] != 0 &&
            g_udp.beginPacket(bcast, ROMEOS_UDP_PORT_DISPLAY_LISTEN)) {
            g_udp.write(reinterpret_cast<const uint8_t *>(&tx), sizeof(tx));
            g_udp.endPacket();
        }
    }
}

int16_t mb_wifi_link_get_setpoint_c_x10()
{
    return g_setpoint_from_display_x10;
}

void mb_wifi_link_apply_setpoint_c_x10(int16_t sp)
{
    if (sp < 50) {
        sp = 50;
    }
    if (sp > 350) {
        sp = 350;
    }
    if (sp == g_setpoint_from_display_x10) {
        return;
    }
    g_setpoint_from_display_x10 = sp;
    save_setpoint_to_nvs(sp);
    Serial.printf("[link] setpoint %d.%d C (saved NVS)\n",
                  sp / 10,
                  std::abs(static_cast<int>(sp % 10)));
}

void mb_wifi_link_notify_remote_setpoint_command_applied(int16_t sp_c_x10)
{
    int16_t sp = sp_c_x10;
    if (sp < 50) {
        sp = 50;
    }
    if (sp > 350) {
        sp = 350;
    }
    /* Hold window: αρκετό για 1–2 RTT UDP + UI refresh, μικρό ώστε τοπικό arc να “πιάνει” μετά. */
    g_suppress_display_udp_setpoint_until_ms = millis() + 8000u;
    g_logged_udp_setpoint_suppressed = false;
    (void)sp;
}

void mb_wifi_link_set_heater(bool on)
{
    const bool before = relay_coil_active(PIN_REL_K4);
    if (before == on) {
        return;
    }
    /* Active-low relay board: LOW=coil ON, HIGH=OFF */
    digitalWrite(PIN_REL_K4, on ? LOW : HIGH);
    Serial.printf("[link] heater %s (relay K4)\n", on ? "ON" : "OFF");
}

bool mb_wifi_link_get_heater(void)
{
    return relay_coil_active(PIN_REL_K4);
}

void mb_wifi_link_fill_telemetry(romeos_mb_to_display_v1_t *out)
{
    if (!out) {
        return;
    }
    build_tx(out);
}

void mb_wifi_link_set_alarm_active(bool active)
{
    g_alarm_active = active;
}
