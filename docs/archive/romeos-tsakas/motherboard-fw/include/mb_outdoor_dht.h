#pragma once

#include <stdint.h>

/** DHT22 / AM2302 στο CN5 (GPIO16 / DHT_DATA). */
void mb_outdoor_dht_begin();
void mb_outdoor_dht_poll();

bool mb_outdoor_dht_has_live();
int16_t mb_outdoor_dht_temp_c_x10();
int16_t mb_outdoor_dht_humidity_rh_x10();
