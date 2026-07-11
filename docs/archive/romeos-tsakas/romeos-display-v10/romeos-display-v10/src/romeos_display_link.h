#pragma once

#include "romeos_link_types.h"

using romeos_on_mb_data_fn = void (*)(const romeos_mb_to_display_v1_t *pkt);
using romeos_get_setpoint_x10_fn = int16_t (*)();
/** INT16_MIN / ROMEOS_ROOM_FROM_DISPLAY_INVALID = καμία έγκυρη μέτρηση SHT προς τη μητρική. */
using romeos_get_room_c_x10_fn = int16_t (*)();

void romeos_display_link_init(romeos_on_mb_data_fn on_mb,
                              romeos_get_setpoint_x10_fn get_setpoint_x10,
                              romeos_get_room_c_x10_fn get_room_c_x10 = nullptr);
void romeos_display_link_poll();
bool romeos_display_link_is_connected();
/** STA συνδεδεμένο στο αποθηκευμένο home SSID (Preferences)· όχι RomeosMB. */
bool romeos_display_link_home_wifi_assoc(void);
/** Wi‑Fi+UDP ζωντανό και διαδοχικά πακέτα με bit0 στα flags (debounce — έγκυρες θερμοκρασίες). */
bool romeos_display_link_telemetry_valid();
void romeos_display_link_on_setpoint_ui_changed();

/**
 * Ενημέρωση τοπικού setpoint UI από μητρική (π.χ. MQTT στο MB) ώστε το UDP setpoint
 * που στέλνει η οθόνη να μην “αναιρεί” αμέσως την αλλαγή.
 */
void romeos_display_link_notify_mb_setpoint_c_x10(int16_t sp_c_x10);

/** Μετά από επιτυχή `WIFI_HOME_SET` ή `WIFI_HOME_CLR` στο Serial — ξαναφόρτωσε NVS και ξεκίνα STA (χωρίς reboot). */
void romeos_display_link_on_home_wifi_saved_to_nvs(void);
