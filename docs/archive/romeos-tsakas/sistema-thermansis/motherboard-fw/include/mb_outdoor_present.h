#pragma once

#include <stdint.h>

/** CN5 DHT — εξωτερικό OUT (Viewe 7″ + γραμμή 1 LCD). */
struct mb_outdoor_present_t {
    bool temp_live;
    bool humidity_live;
    int16_t temp_c_x10;
    int16_t rh_x10;
};

/** CN5 DHT μόνο (AM2302). */
mb_outdoor_present_t mb_outdoor_cn5_snapshot();

/** CN4 DS18 — μόνο LCD γραμμή 2 (bench / νερό αργότερα). */
mb_outdoor_present_t mb_outdoor_cn4_snapshot();

int16_t mb_outdoor_temp_whole_x10(int16_t raw_x10);
