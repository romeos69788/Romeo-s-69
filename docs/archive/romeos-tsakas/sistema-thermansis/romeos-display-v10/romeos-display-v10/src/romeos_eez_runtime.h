#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Wi‑Fi, NTP, αισθητήρες, alarm, MIC, boiler icons — μετά το ui_init (μέσα lock). */
void romeos_eez_runtime_after_ui_init(void);

/** Κλήση από setup μετά το unlock· αισθητήρας + NTP + display_link. */
void romeos_eez_runtime_services_init(void);

/** Κύριο loop (χωρίς LVGL lock εκτός αν χρειάζεται εσωτερικά). */
void romeos_eez_runtime_poll(void);

/** Μετά από loadScreen — συγχρονίζει χρώματα/ρολόι μόνο στην τρέχουσα οθόνη. */
void romeos_eez_runtime_on_screen_loaded(void);

#ifdef __cplusplus
}
#endif
