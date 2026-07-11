#pragma once

#include "alpha_state.h"

#include <functional>

// MQTT bridge προς HiveMQ Cloud · topics romeos/<device>/state|cmd

using MqttCommandHandler = std::function<void(const AlphaCommand&)>;

bool mbMqttBegin(const char* wifiSsid, const char* wifiPass);
void mbMqttLoop();
void mbMqttPublishState(const AlphaState& state);
void mbMqttSetCommandHandler(MqttCommandHandler handler);
bool mbMqttConnected();
const char* mbMqttStateTopic();
const char* mbMqttCmdTopic();
