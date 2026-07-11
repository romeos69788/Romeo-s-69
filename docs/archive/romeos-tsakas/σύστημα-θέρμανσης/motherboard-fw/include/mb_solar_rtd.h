#pragma once

#include <stddef.h>
#include <stdint.h>

/** PT100 2-wire στο MAX31865 (SPI CS = GPIO5). RTD+ 2x red, RTD- white. */
void mb_solar_rtd_begin();
void mb_solar_rtd_poll();

bool mb_solar_rtd_begin_ok();
bool mb_solar_rtd_has_live();
int16_t mb_solar_rtd_temp_c_x10();
uint8_t mb_solar_rtd_last_fault();
uint16_t mb_solar_rtd_last_rtd_raw();
float mb_solar_rtd_last_resistance_ohm();

/** Μία φορά: ευθυγράμμιση ένδειξης με CN4 DS18 @ bench. */
void mb_solar_rtd_try_calibrate(int16_t reference_temp_c_x10);
bool mb_solar_rtd_is_calibrated();

void mb_solar_rtd_format_lcd_line(char *buf, size_t len);

void mb_solar_rtd_run_diag();
void mb_solar_rtd_force_poll();
