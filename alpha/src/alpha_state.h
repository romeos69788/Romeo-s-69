#pragma once

#include <stdint.h>

// Κατάσταση μητρικής — JSON schema συμβατό με romeos-remote (App.tsx RomeosState).

struct AlphaState {
    uint8_t v = 1;
    uint16_t flags = 0;

    int16_t room_c_x10 = 0;
    int16_t outdoor_c_x10 = 0;
    int16_t solar_c_x10 = 0;
    int16_t boiler_c_x10 = 0;
    int16_t supply_c_x10 = 0;
    int16_t return_c_x10 = 0;

    uint8_t relay_k1 = 0;
    uint8_t relay_k2 = 0;
    uint8_t relay_k3 = 0;
    uint8_t relay_k4 = 0;
    uint8_t relay_k5 = 0;
    uint8_t relay_k6 = 0;

    uint8_t heat_pump = 0;
    uint8_t pump1 = 0;
    uint8_t pump2 = 0;
    uint8_t heater = 0;

    uint8_t flow_sig_high = 0;
    uint8_t defrost_active = 0;

    int16_t setpoint_c_x10 = 210;  // default 21.0 °C

    uint32_t uptime_ms = 0;
};

struct AlphaCommand {
    bool has_setpoint = false;
    int16_t setpoint_c_x10 = 0;

    bool has_heater = false;
    uint8_t heater = 0;
};
