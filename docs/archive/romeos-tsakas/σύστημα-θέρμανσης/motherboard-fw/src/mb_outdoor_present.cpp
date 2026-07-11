#include "mb_outdoor_dht.h"
#include "mb_outdoor_present.h"
#include "mb_outdoor_sensor.h"

int16_t mb_outdoor_temp_whole_x10(int16_t raw_x10)
{
    return static_cast<int16_t>((raw_x10 >= 0 ? raw_x10 + 5 : raw_x10 - 5) / 10 * 10);
}

mb_outdoor_present_t mb_outdoor_cn5_snapshot()
{
    mb_outdoor_present_t out{};
    if (!mb_outdoor_dht_has_live()) {
        return out;
    }
    out.temp_live = true;
    out.humidity_live = true;
    out.temp_c_x10 = mb_outdoor_dht_temp_c_x10();
    out.rh_x10 = mb_outdoor_dht_humidity_rh_x10();
    return out;
}

mb_outdoor_present_t mb_outdoor_cn4_snapshot()
{
    mb_outdoor_present_t out{};
    if (!mb_outdoor_sensor_has_live()) {
        return out;
    }
    out.temp_live = true;
    out.humidity_live = false;
    out.temp_c_x10 = mb_outdoor_sensor_temp_c_x10();
    return out;
}
