#include "mb_mqtt.h"

#include <Arduino.h>

#if defined(ROMEOS_MQTT_HOST) && defined(ROMEOS_MQTT_USER) && defined(ROMEOS_MQTT_PASS)

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <cstring>

#include "mb_wifi_link.h"
#include "romeos_link_types.h"

namespace {

#ifndef ROMEOS_MQTT_PORT
#define ROMEOS_MQTT_PORT 8883
#endif

#ifndef ROMEOS_MQTT_DEVICE_ID
#define ROMEOS_MQTT_DEVICE_ID "romeos-mb"
#endif

WiFiClientSecure g_tls;
PubSubClient g_mqtt(g_tls);

char g_topic_state[80]{};
char g_topic_cmd[80]{};
uint32_t g_last_connect_try_ms = 0;
uint32_t g_last_publish_ms = 0;
uint32_t g_sta_connected_since_ms = 0;
bool g_have_topics = false;

void ensure_topics()
{
    if (g_have_topics) {
        return;
    }
    snprintf(g_topic_state,
             sizeof(g_topic_state),
             "romeos/%s/state",
             ROMEOS_MQTT_DEVICE_ID);
    snprintf(g_topic_cmd,
             sizeof(g_topic_cmd),
             "romeos/%s/cmd",
             ROMEOS_MQTT_DEVICE_ID);
    g_have_topics = true;
}

void on_mqtt_message(char *topic, byte *payload, unsigned int length)
{
    if (length == 0 || length > 512) {
        return;
    }
    char buf[512];
    if (length >= sizeof(buf)) {
        length = sizeof(buf) - 1;
    }
    std::memcpy(buf, payload, length);
    buf[length] = '\0';

    Serial.printf("[mqtt] cmd topic=%s len=%u json=%s\n",
                  topic ? topic : "(null)",
                  static_cast<unsigned>(length),
                  buf);

    /* DynamicJsonDocument: μνήμη στο heap — αποφεύγει υπερχείλιση stack στο loop task. */
    DynamicJsonDocument doc(256);
    const DeserializationError err = deserializeJson(doc, buf);
    if (err) {
        Serial.printf("[mqtt] cmd JSON parse err: %s\n", err.c_str());
        return;
    }
    if (doc.containsKey("setpoint_c_x10")) {
        if (!doc["setpoint_c_x10"].is<int>() && !doc["setpoint_c_x10"].is<float>()) {
            Serial.println(F("[mqtt] cmd setpoint_c_x10 is not a number"));
            return;
        }
        const int sp = doc["setpoint_c_x10"].as<int>();
        Serial.printf("[mqtt] cmd apply setpoint_c_x10=%d\n", sp);
        mb_wifi_link_notify_remote_setpoint_command_applied(static_cast<int16_t>(sp));
        mb_wifi_link_apply_setpoint_c_x10(static_cast<int16_t>(sp));
    }
    if (doc.containsKey("heater")) {
        if (!doc["heater"].is<int>() && !doc["heater"].is<bool>()) {
            Serial.println(F("[mqtt] cmd heater is not bool/int"));
            return;
        }
        const bool on = doc["heater"].as<int>() != 0;
        Serial.printf("[mqtt] cmd apply heater=%d\n", on ? 1 : 0);
        mb_wifi_link_set_heater(on);
    } else if (doc.containsKey("boiler_on")) {
        if (!doc["boiler_on"].is<int>() && !doc["boiler_on"].is<bool>()) {
            Serial.println(F("[mqtt] cmd boiler_on is not bool/int"));
            return;
        }
        const bool on = doc["boiler_on"].as<int>() != 0;
        Serial.printf("[mqtt] cmd apply boiler_on=%d\n", on ? 1 : 0);
        mb_wifi_link_set_heater(on);
    }
}

bool mqtt_connect()
{
    ensure_topics();

    char client_id[28];
    const uint64_t mac = ESP.getEfuseMac();
    snprintf(client_id,
             sizeof(client_id),
             "mb-%06X",
             static_cast<unsigned>(mac & 0xFFFFFFu));

    if (!g_mqtt.connect(client_id, ROMEOS_MQTT_USER, ROMEOS_MQTT_PASS)) {
        Serial.printf("[mqtt] connect failed rc=%d\n", g_mqtt.state());
        return false;
    }
    if (!g_mqtt.subscribe(g_topic_cmd)) {
        Serial.println(F("[mqtt] subscribe failed"));
        g_mqtt.disconnect();
        return false;
    }
    Serial.println(F("[mqtt] connected + subscribed"));
    return true;
}

void publish_state()
{
    romeos_mb_to_display_v1_t t{};
    mb_wifi_link_fill_telemetry(&t);

    DynamicJsonDocument doc(768);
    doc["v"] = 1;
    doc["flags"] = t.flags;
    doc["room_c_x10"] = t.room_display_c_x10;
    doc["outdoor_c_x10"] = t.outdoor_c_x10;
    doc["solar_c_x10"] = t.solar_c_x10;
    doc["boiler_c_x10"] = t.boiler_c_x10;
    doc["supply_c_x10"] = t.supply_c_x10;
    doc["return_c_x10"] = t.return_c_x10;
    doc["relay_k1"] = t.relay_k1_on;
    doc["relay_k2"] = t.relay_k2_on;
    doc["relay_k3"] = t.relay_k3_on;
    doc["relay_k4"] = t.relay_k4_on;
    doc["relay_k5"] = t.relay_k5_on;
    doc["relay_k6"] = t.relay_k6_on;
    doc["heat_pump"] = t.heat_pump_on;
    doc["pump1"] = t.pump1_on;
    doc["pump2"] = t.pump2_on;
    doc["heater"] = t.heater_on;
    doc["flow_sig_high"] = t.flow_sig_high;
    doc["defrost_active"] = t.defrost_active;
    doc["setpoint_c_x10"] = mb_wifi_link_get_setpoint_c_x10();
    doc["uptime_ms"] = static_cast<uint32_t>(millis());

    char out[900];
    const size_t n = serializeJson(doc, out, sizeof(out));
    if (n == 0 || n >= sizeof(out)) {
        Serial.println(F("[mqtt] state JSON too large"));
        return;
    }
    if (!g_mqtt.publish(g_topic_state, out, true)) {
        Serial.println(F("[mqtt] publish failed"));
    }
}

}  // namespace

void mb_mqtt_begin()
{
    ensure_topics();
    g_mqtt.setCallback(on_mqtt_message);
    g_mqtt.setBufferSize(1024);
    g_mqtt.setKeepAlive(60);
    g_mqtt.setSocketTimeout(20);
    g_tls.setInsecure();
    g_mqtt.setServer(ROMEOS_MQTT_HOST, ROMEOS_MQTT_PORT);
    Serial.printf("[mqtt] broker %s:%d device=%s (TLS insecure dev)\n",
                  ROMEOS_MQTT_HOST,
                  static_cast<int>(ROMEOS_MQTT_PORT),
                  ROMEOS_MQTT_DEVICE_ID);
}

bool mb_mqtt_is_connected()
{
    return g_mqtt.connected();
}

void mb_mqtt_poll()
{
    const uint32_t now = millis();

    if (WiFi.status() != WL_CONNECTED) {
        g_sta_connected_since_ms = 0;
        if (g_mqtt.connected()) {
            g_mqtt.disconnect();
        }
        return;
    }

    if (g_sta_connected_since_ms == 0) {
        g_sta_connected_since_ms = now;
    }
    /* Λίγο delay μετά το STA — σταθεροποίηση TCP/IP πριν TLS στο PubSubClient. */
    if (now - g_sta_connected_since_ms < 2500u) {
        return;
    }

    if (!g_mqtt.connected()) {
        if (now - g_last_connect_try_ms < 5000u) {
            return;
        }
        g_last_connect_try_ms = now;
        yield();
        (void)mqtt_connect();
        yield();
        return;
    }

    g_mqtt.loop();

    if (now - g_last_publish_ms < 2000u) {
        return;
    }
    g_last_publish_ms = now;
    publish_state();
}

#else

void mb_mqtt_begin()
{
    Serial.println(F("[mqtt] disabled — define ROMEOS_MQTT_HOST, ROMEOS_MQTT_USER, ROMEOS_MQTT_PASS in build_flags"));
}

void mb_mqtt_poll()
{
}

bool mb_mqtt_is_connected()
{
    return false;
}

#endif
