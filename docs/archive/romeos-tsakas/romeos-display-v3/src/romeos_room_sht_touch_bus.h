#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SHT3x or SHT4x (e.g. SHT40) on the same legacy I2C master as the panel touch (I2C_NUM_0,
 * typically SDA=19 / SCL=20). Call only after Board::begin() has installed the touch I2C driver.
 * Do not use Arduino Wire on those pins in parallel.
 */
bool romeos_room_sht_touch_bus_begin(uint8_t i2c_addr7);
/** SHT στο 0x44 / 0x45· αν αποτύχει, δοκιμή AHT20/AHT21 στο 0x38 (συχνά breakout «SHT» που είναι AHT). */
bool romeos_room_sht_touch_bus_begin_auto(void);
bool romeos_room_sht_touch_bus_read_both(float *temp_c, float *rh_pct);
void romeos_room_sht_touch_bus_soft_reset(void);
/** After begin: "SHT4x", "SHT3x", "AHT2x", or "none". */
const char *romeos_room_sht_touch_bus_family_name(void);

#ifdef __cplusplus
}
#endif
