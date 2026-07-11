#include "mb_mqtt.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#ifndef ROMEOS_MQTT_DEVICE_ID
#define ROMEOS_MQTT_DEVICE_ID "romeos-mb"
#endif

#ifndef ROMEOS_FW_VERSION
#define ROMEOS_FW_VERSION 1
#endif

#ifndef ROMEOS_MQTT_PORT
#define ROMEOS_MQTT_PORT 8883
#endif

namespace {

WiFiClientSecure g_tls;
PubSubClient g_mqtt(g_tls);

char g_stateTopic[64];
char g_cmdTopic[64];

MqttCommandHandler g_onCommand;
uint32_t g_lastPublishMs = 0;
constexpr uint32_t kPublishIntervalMs = 5000;

void buildTopics() {
    snprintf(g_stateTopic, sizeof(g_stateTopic), "romeos/%s/state", ROMEOS_MQTT_DEVICE_ID);
    snprintf(g_cmdTopic, sizeof(g_cmdTopic), "romeos/%s/cmd", ROMEOS_MQTT_DEVICE_ID);
}

void handleCommandPayload(char* payload, unsigned int length) {
    if (!g_onCommand) {
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
        Serial.printf("[mqtt] cmd JSON error: %s\n", err.c_str());
        return;
    }

    AlphaCommand cmd;
    if (doc["setpoint_c_x10"].is<int>()) {
        cmd.has_setpoint = true;
        cmd.setpoint_c_x10 = doc["setpoint_c_x10"].as<int16_t>();
    }
    if (doc["heater"].is<int>()) {
        cmd.has_heater = true;
        cmd.heater = doc["heater"].as<uint8_t>() ? 1 : 0;
    }

    if (cmd.has_setpoint || cmd.has_heater) {
        g_onCommand(cmd);
    }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, g_cmdTopic) != 0) {
        return;
    }
    // PubSubClient needs null-terminated buffer
    char buf[256];
    const size_t n = min(length, sizeof(buf) - 1);
    memcpy(buf, payload, n);
    buf[n] = '\0';
    handleCommandPayload(buf, n);
}

void reconnectMqtt() {
#if defined(ROMEOS_BENCH) || !defined(ROMEOS_MQTT_HOST)
    return;
#else
    if (g_mqtt.connected()) {
        return;
    }
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    Serial.print("[mqtt] connecting… ");
    if (g_mqtt.connect(ROMEOS_MQTT_DEVICE_ID)) {
        Serial.println("ok");
        g_mqtt.subscribe(g_cmdTopic);
    } else {
        Serial.printf("fail rc=%d\n", g_mqtt.state());
    }
#endif
}

}  // namespace

bool mbMqttBegin(const char* wifiSsid, const char* wifiPass) {
    buildTopics();

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPass);

    g_tls.setInsecure();  // bench · production: CA cert

#if defined(ROMEOS_BENCH) || !defined(ROMEOS_MQTT_HOST)
    Serial.println("[mqtt] bench mode — no broker (compile with ROMEOS_MQTT_HOST)");
    return true;
#else
    g_mqtt.setServer(ROMEOS_MQTT_HOST, ROMEOS_MQTT_PORT);
    g_mqtt.setCallback(onMqttMessage);
    g_mqtt.setBufferSize(512);
    return true;
#endif
}

void mbMqttLoop() {
#if !defined(ROMEOS_BENCH) && defined(ROMEOS_MQTT_HOST)
    if (!g_mqtt.connected()) {
        reconnectMqtt();
    }
    g_mqtt.loop();
#endif
}

void mbMqttPublishState(const AlphaState& state) {
#if defined(ROMEOS_BENCH) || !defined(ROMEOS_MQTT_HOST)
    (void)state;
    return;
#else
    if (!g_mqtt.connected()) {
        return;
    }
    const uint32_t now = millis();
    if (now - g_lastPublishMs < kPublishIntervalMs) {
        return;
    }
    g_lastPublishMs = now;

    JsonDocument doc;
    doc["v"] = ROMEOS_FW_VERSION;
    doc["flags"] = state.flags;
    doc["room_c_x10"] = state.room_c_x10;
    doc["outdoor_c_x10"] = state.outdoor_c_x10;
    doc["solar_c_x10"] = state.solar_c_x10;
    doc["boiler_c_x10"] = state.boiler_c_x10;
    doc["supply_c_x10"] = state.supply_c_x10;
    doc["return_c_x10"] = state.return_c_x10;
    doc["relay_k1"] = state.relay_k1;
    doc["relay_k2"] = state.relay_k2;
    doc["relay_k3"] = state.relay_k3;
    doc["relay_k4"] = state.relay_k4;
    doc["relay_k5"] = state.relay_k5;
    doc["relay_k6"] = state.relay_k6;
    doc["heat_pump"] = state.heat_pump;
    doc["pump1"] = state.pump1;
    doc["pump2"] = state.pump2;
    doc["heater"] = state.heater;
    doc["flow_sig_high"] = state.flow_sig_high;
    doc["defrost_active"] = state.defrost_active;
    doc["setpoint_c_x10"] = state.setpoint_c_x10;
    doc["uptime_ms"] = state.uptime_ms;

    char payload[512];
    const size_t n = serializeJson(doc, payload, sizeof(payload));
    g_mqtt.publish(g_stateTopic, payload, n);
#endif
}

void mbMqttSetCommandHandler(MqttCommandHandler handler) {
    g_onCommand = std::move(handler);
}

bool mbMqttConnected() {
#if defined(ROMEOS_BENCH) || !defined(ROMEOS_MQTT_HOST)
    return false;
#else
    return g_mqtt.connected();
#endif
}

const char* mbMqttStateTopic() {
    return g_stateTopic;
}

const char* mbMqttCmdTopic() {
    return g_cmdTopic;
}
