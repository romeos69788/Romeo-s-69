#include "alpha_pins.h"
#include "alpha_state.h"
#include "mb_mqtt.h"

#include <Arduino.h>

#ifndef ROMEOS_WIFI_SSID
#define ROMEOS_WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef ROMEOS_WIFI_PASS
#define ROMEOS_WIFI_PASS "YOUR_WIFI_PASS"
#endif

namespace {

AlphaState g_state;

void initRelays() {
    for (size_t i = 0; i < alpha::kRelayCount; ++i) {
        pinMode(alpha::kRelayPins[i], OUTPUT);
        digitalWrite(alpha::kRelayPins[i], LOW);
    }
}

void syncRelayState() {
    g_state.relay_k1 = digitalRead(alpha::kRelayK1) == HIGH ? 1 : 0;
    g_state.relay_k2 = digitalRead(alpha::kRelayK2) == HIGH ? 1 : 0;
    g_state.relay_k3 = digitalRead(alpha::kRelayK3) == HIGH ? 1 : 0;
    g_state.relay_k4 = digitalRead(alpha::kRelayK4) == HIGH ? 1 : 0;
    g_state.relay_k5 = digitalRead(alpha::kRelayK5) == HIGH ? 1 : 0;
    g_state.relay_k6 = digitalRead(alpha::kRelayK6) == HIGH ? 1 : 0;
    g_state.heater = g_state.relay_k4;
}

void applyHeater(bool on) {
    digitalWrite(alpha::kRelayK4, on ? HIGH : LOW);
    g_state.heater = on ? 1 : 0;
    g_state.relay_k4 = g_state.heater;
}

void onRemoteCommand(const AlphaCommand& cmd) {
    if (cmd.has_setpoint) {
        const int16_t sp = constrain(cmd.setpoint_c_x10, 50, 350);
        g_state.setpoint_c_x10 = sp;
        Serial.printf("[cmd] setpoint -> %d.%d C\n", sp / 10, abs(sp % 10));
        // TODO: UDP/ESP-NOW sync προς οθόνες · UART προς Beta
    }
    if (cmd.has_heater) {
        applyHeater(cmd.heater != 0);
        Serial.printf("[cmd] heater -> %s\n", cmd.heater ? "ON" : "OFF");
    }
}

void readSensors() {
    // TODO: DS18B20 · NTC · WiFi meter (HP kWh) integration
    g_state.flow_sig_high = 0;  // rev A: no flow sensor — pump speed manual
    g_state.defrost_active =
        digitalRead(alpha::kDefrostSig) == HIGH ? 1 : 0;
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println();
    Serial.println("ROMEOS 69 Alpha rev A — motherboard-fw");

    pinMode(alpha::kDefrostSig, INPUT);
    initRelays();
    syncRelayState();

    mbMqttSetCommandHandler(onRemoteCommand);
    mbMqttBegin(ROMEOS_WIFI_SSID, ROMEOS_WIFI_PASS);

    g_state.v = 1;
    g_state.setpoint_c_x10 = 210;
}

void loop() {
    readSensors();
    syncRelayState();
    g_state.uptime_ms = millis();

    mbMqttLoop();
    mbMqttPublishState(g_state);

    delay(50);
}
