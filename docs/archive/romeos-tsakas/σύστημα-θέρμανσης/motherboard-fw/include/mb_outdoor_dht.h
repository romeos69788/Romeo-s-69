#pragma once

#include <stdint.h>

/** DHT22 / AM2302 στο CN5 (GPIO16 / DHT_DATA). */
void mb_outdoor_dht_begin();
void mb_outdoor_dht_poll();

bool mb_outdoor_dht_has_live();
int16_t mb_outdoor_dht_temp_c_x10();
int16_t mb_outdoor_dht_humidity_rh_x10();
/** 0/1 τελευταία idle κατάσταση GPIO16 πριν read · 255=άγνωστο. */
uint8_t mb_outdoor_dht_last_idle_level();
void mb_outdoor_dht_log_diagnostics();
/** Άμεση ανάγνωση (Serial DHT DIAG). */
void mb_outdoor_dht_force_poll();
