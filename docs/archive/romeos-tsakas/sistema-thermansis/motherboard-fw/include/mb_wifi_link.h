#pragma once

#include <stdint.h>

#include "romeos_link_types.h"

/** SoftAP + UDP · μητρική → οθόνη (βλ. romeos_link_types.h). */
void mb_wifi_link_begin();
void mb_wifi_link_poll();

/** Setpoint 5…35 °C σε δέκατα (NVS + τελευταίο από οθόνη). */
int16_t mb_wifi_link_get_setpoint_c_x10();

/** Clamp 5…35 °C, NVS, ενημέρωση εσωτερικής τιμής (οθόνη, MQTT, κ.λπ.). */
void mb_wifi_link_apply_setpoint_c_x10(int16_t sp_c_x10);

/**
 * Σήμανση ότι setpoint ήρθε από MQTT remote command.
 * Χρησιμοποιείται για να αγνοηθούν προσωρινά “παλιά” UDP setpoints από την οθόνη
 * (το display poll στέλνει ~κάθε 400ms και μπορεί να αναιρεί αμέσως την αλλαγή).
 */
void mb_wifi_link_notify_remote_setpoint_command_applied(int16_t sp_c_x10);

/** Heater/boiler relay (K4): true=ON (coil active), false=OFF. */
void mb_wifi_link_set_heater(bool on);
bool mb_wifi_link_get_heater(void);

/** Γέμισμα πακέτου μητρική→οθόνη (ίδια λογική με UDP TX). */
void mb_wifi_link_fill_telemetry(romeos_mb_to_display_v1_t *out);

/** Κατάσταση συναγερμού για αποστολή προς οθόνη (flags bit1). */
void mb_wifi_link_set_alarm_active(bool active);
