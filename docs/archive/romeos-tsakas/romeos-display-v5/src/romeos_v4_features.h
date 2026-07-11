#pragma once

/** After `ui_init()` while LVGL mutex is held: events/timers on SquareLine widgets. */
void romeos_v4_after_ui_init(void);

/** I2C room sensor: call after `Board::begin()` (before or after LVGL). */
void romeos_v4_init_room_sensor(void);

/** Periodic: room sensor read (blocks briefly), LVGL label refresh when Screen1 active. */
void romeos_v4_loop(void);

/** Όπως v2/v3 `loop()`: με STYLE=1 ξανα-assert HIGH στο buzzer όσο το alarm είναι ON (αν κάτι άλλο «πειράξει» το pin). */
void romeos_v4_alarm_buzzer_keepalive(void);
