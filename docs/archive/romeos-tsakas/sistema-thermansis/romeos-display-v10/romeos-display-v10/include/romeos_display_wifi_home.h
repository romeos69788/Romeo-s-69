#pragma once

/** Οικιακό Wi‑Fi (internet/NTP) — ανεξάρτητο από ESP-NOW link προς μητρική. */
void romeos_display_wifi_home_begin(void);
void romeos_display_wifi_home_poll(void);
bool romeos_display_wifi_home_assoc(void);
void romeos_display_wifi_home_on_nvs_saved(void);
