#pragma once

#include <stdint.h>

#ifndef ROMEOS_MB_OUTDOOR_LINK_TEST
#define ROMEOS_MB_OUTDOOR_LINK_TEST 0
#endif

/** DS18B20 CN4 (GPIO4). CN5 (GPIO16) = DHT — βλ. mb_outdoor_dht.h. */
void mb_outdoor_sensor_begin();
void mb_outdoor_sensor_poll();

bool mb_outdoor_sensor_has_live();
int16_t mb_outdoor_sensor_temp_c_x10();
bool mb_outdoor_sensor_has_humidity();
bool mb_outdoor_sensor_is_link_test();

void mb_outdoor_sensor_force_poll();
/** Serial: αριθμός ROM, ανάγνωση κάθε DS18 με Match ROM (πολλαπλά bus). */
void mb_outdoor_sensor_run_diag();
/** Κλήση πριν WiFi.begin — λιγότερος RF θόρυβος στο 1-Wire. */
void mb_outdoor_sensor_boot_probe_no_wifi();
/** -1 αν δεν βρέθηκε ακόμα · 4=CN4 · 16=CN5. */
int mb_outdoor_sensor_active_gpio();
