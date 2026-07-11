#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Setpoint bar: 10.0–30.0 °C in 0.5 °C steps (400 px). Hides +/- labels. */
void romeos_setpoint_ui_init(void);

#ifdef __cplusplus
}
#endif
