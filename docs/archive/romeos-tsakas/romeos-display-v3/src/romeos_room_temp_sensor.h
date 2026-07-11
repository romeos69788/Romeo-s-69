#pragma once

#ifdef __cplusplus

extern float g_room_temp_c;
extern bool g_room_sensor_ready;
extern bool g_room_local_valid;

/** True μετά την πρώτη έγκυρη μέτρηση (πριν το debounce 3 δειγμάτων). */
bool romeos_room_temp_sensor_has_live_sample();

using RomeosRoomTempUiRefreshFn = void (*)();
void romeos_room_temp_sensor_set_ui_refresh_callback(RomeosRoomTempUiRefreshFn fn);

/** Μετά `Board::begin()`: I2C scan (shared bus) + init αισθητήρα. Χωρίς LVGL. */
void romeos_room_temp_sensor_after_board_begin();

/** Κλήση από `loop()` — εσωτερικά ~2 s, I2C εκτός LVGL timer. */
void romeos_room_temp_sensor_poll_in_app_loop();

#endif
