#pragma once

#include <stdint.h>

#include "romeos_link_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Μετά `ui_init` (μέσα σε LVGL lock): τόξο 10…30 °C, κέντρο 20, ±, σύνδεση με UDP setpoint. */
void romeos_v4_setpoint_ui_init(void);

/** Επιστρέφει τρέχον setpoint σε δέκατα °C (για `romeos_display_link_init`). */
int16_t romeos_v4_setpoint_get_c_x10(void);

/** Ενημέρωση από πακέτο μητρικής (version ≥ 2 → `setpoint_c_x10`). */
void romeos_v4_setpoint_on_mb_packet(const romeos_mb_to_display_v1_t *pkt);

#ifdef __cplusplus
}
#endif
