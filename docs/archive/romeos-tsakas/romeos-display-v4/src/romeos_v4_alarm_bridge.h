#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Κλήση από `ui_event_ALARM` στο SquareLine `ui_Screen1.c` (ίδιο μοτίβο με OPTIONS). */
void romeos_v4_on_alarm_ui_event(lv_event_t *e);

#ifdef __cplusplus
}
#endif
