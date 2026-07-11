/*
 * SHT3x / SHT4x on I2C_NUM_0 shared with GT911 / esp_lcd — same legacy driver as ESP32_Display_Panel BusI2C.
 * BOM/design notes use SHT40 (SHT4x): different commands and T/RH formulas than SHT3x — probe SHT4x first.
 */

#include "romeos_room_sht_touch_bus.h"

#include <cmath>
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace {

constexpr i2c_port_t kPort = I2C_NUM_0;

/* SHT3x: 16-bit command MSB first */
constexpr uint16_t kSht3MeasHighRep = 0x2400; /* high repeatability, clock stretch disabled */
constexpr uint16_t kSht3SoftReset = 0x30A2;
constexpr uint16_t kSht3ReadStatus = 0xF32D;

/* SHT4x: single command byte (datasheet) */
constexpr uint8_t kSht4SoftReset = 0x94;
constexpr uint8_t kSht4MeasHighPrec = 0xFD; /* T + RH, high precision */

enum class BusFamily : uint8_t { None = 0, Sht3x = 1, Sht4x = 2, Aht2x = 3 };

uint8_t s_addr7 = 0x44;
BusFamily s_family = BusFamily::None;

uint8_t crc8(const uint8_t *data, int len)
{
    constexpr uint8_t kPoly = 0x31;
    uint8_t crc = 0xFF;
    for (int j = len; j; --j) {
        crc ^= *data++;
        for (int i = 8; i; --i) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ kPoly) : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

bool write_cmd8(uint8_t cmd)
{
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    if (!c) {
        return false;
    }
    i2c_master_start(c);
    i2c_master_write_byte(c, static_cast<uint8_t>((s_addr7 << 1) | I2C_MASTER_WRITE), true);
    i2c_master_write_byte(c, cmd, true);
    i2c_master_stop(c);
    const esp_err_t e = i2c_master_cmd_begin(kPort, c, pdMS_TO_TICKS(150));
    i2c_cmd_link_delete(c);
    return e == ESP_OK;
}

bool write_cmd16(uint16_t cmd)
{
    const uint8_t buf[2] = {static_cast<uint8_t>(cmd >> 8), static_cast<uint8_t>(cmd & 0xFF)};
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    if (!c) {
        return false;
    }
    i2c_master_start(c);
    i2c_master_write_byte(c, static_cast<uint8_t>((s_addr7 << 1) | I2C_MASTER_WRITE), true);
    i2c_master_write(c, buf, 2, true);
    i2c_master_stop(c);
    const esp_err_t e = i2c_master_cmd_begin(kPort, c, pdMS_TO_TICKS(150));
    i2c_cmd_link_delete(c);
    return e == ESP_OK;
}

bool read_n(uint8_t *buf, size_t n)
{
    if (n < 2) {
        return false;
    }
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    if (!c) {
        return false;
    }
    i2c_master_start(c);
    i2c_master_write_byte(c, static_cast<uint8_t>((s_addr7 << 1) | I2C_MASTER_READ), true);
    for (size_t i = 0; i + 1 < n; ++i) {
        i2c_master_read_byte(c, &buf[i], I2C_MASTER_ACK);
    }
    i2c_master_read_byte(c, &buf[n - 1], I2C_MASTER_NACK);
    i2c_master_stop(c);
    const esp_err_t e = i2c_master_cmd_begin(kPort, c, pdMS_TO_TICKS(150));
    i2c_cmd_link_delete(c);
    return e == ESP_OK;
}

bool write_then_read(uint16_t cmd, uint8_t *out, size_t out_len)
{
    const uint8_t wbuf[2] = {static_cast<uint8_t>(cmd >> 8), static_cast<uint8_t>(cmd & 0xFF)};
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    if (!c) {
        return false;
    }
    i2c_master_start(c);
    i2c_master_write_byte(c, static_cast<uint8_t>((s_addr7 << 1) | I2C_MASTER_WRITE), true);
    i2c_master_write(c, wbuf, 2, true);
    i2c_master_start(c);
    i2c_master_write_byte(c, static_cast<uint8_t>((s_addr7 << 1) | I2C_MASTER_READ), true);
    for (size_t i = 0; i + 1 < out_len; ++i) {
        i2c_master_read_byte(c, &out[i], I2C_MASTER_ACK);
    }
    i2c_master_read_byte(c, &out[out_len - 1], I2C_MASTER_NACK);
    i2c_master_stop(c);
    const esp_err_t e = i2c_master_cmd_begin(kPort, c, pdMS_TO_TICKS(150));
    i2c_cmd_link_delete(c);
    return e == ESP_OK;
}

bool sht4x_one_shot_decode(const uint8_t rb[6], float *temp_c, float *rh_pct)
{
    if (rb[2] != crc8(rb, 2) || rb[5] != crc8(rb + 3, 2)) {
        return false;
    }
    const uint16_t rt = static_cast<uint16_t>((static_cast<uint16_t>(rb[0]) << 8) | rb[1]);
    const uint16_t rh_raw = static_cast<uint16_t>((static_cast<uint16_t>(rb[3]) << 8) | rb[4]);
    const float t = -45.0f + 175.0f * static_cast<float>(rt) / 65535.0f;
    float rh = -6.0f + 125.0f * static_cast<float>(rh_raw) / 65535.0f;
    if (rh < 0.0f) {
        rh = 0.0f;
    }
    if (rh > 100.0f) {
        rh = 100.0f;
    }
    if (t < -50.0f || t > 130.0f) {
        return false;
    }
    *temp_c = t;
    *rh_pct = rh;
    return true;
}

bool sht4x_probe_measure(float *temp_c, float *rh_pct)
{
    if (!write_cmd8(kSht4SoftReset)) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(2));
    if (!write_cmd8(kSht4MeasHighPrec)) {
        return false;
    }
    /* Datasheet: high-precision meas typ ~20 ms, max >24 ms — margin για shared bus με GT911. */
    vTaskDelay(pdMS_TO_TICKS(30));
    uint8_t rb[6] = {0};
    if (!read_n(rb, sizeof(rb))) {
        return false;
    }
    return sht4x_one_shot_decode(rb, temp_c, rh_pct);
}

bool sht3x_begin()
{
    if (!write_cmd16(kSht3SoftReset)) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    uint8_t st[3] = {0};
    if (!write_then_read(kSht3ReadStatus, st, sizeof(st))) {
        return false;
    }
    const uint16_t stat = static_cast<uint16_t>((static_cast<uint16_t>(st[0]) << 8) | st[1]);
    if (stat == 0xFFFFU) {
        return false;
    }
    return true;
}

bool write_bytes_aht(const uint8_t *data, size_t len)
{
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    if (!c) {
        return false;
    }
    constexpr uint8_t kAhtAddr7 = 0x38;
    i2c_master_start(c);
    i2c_master_write_byte(c, static_cast<uint8_t>((kAhtAddr7 << 1) | I2C_MASTER_WRITE), true);
    i2c_master_write(c, data, len, true);
    i2c_master_stop(c);
    const esp_err_t e = i2c_master_cmd_begin(kPort, c, pdMS_TO_TICKS(150));
    i2c_cmd_link_delete(c);
    return e == ESP_OK;
}

bool read_bytes_aht(uint8_t *buf, size_t n)
{
    constexpr uint8_t kAhtAddr7 = 0x38;
    i2c_cmd_handle_t c = i2c_cmd_link_create();
    if (!c || n < 1) {
        return false;
    }
    i2c_master_start(c);
    i2c_master_write_byte(c, static_cast<uint8_t>((kAhtAddr7 << 1) | I2C_MASTER_READ), true);
    for (size_t i = 0; i + 1 < n; ++i) {
        i2c_master_read_byte(c, &buf[i], I2C_MASTER_ACK);
    }
    i2c_master_read_byte(c, &buf[n - 1], I2C_MASTER_NACK);
    i2c_master_stop(c);
    const esp_err_t e = i2c_master_cmd_begin(kPort, c, pdMS_TO_TICKS(150));
    i2c_cmd_link_delete(c);
    return e == ESP_OK;
}

bool aht21_probe_measure(float *temp_c, float *rh_pct)
{
    constexpr uint8_t kReset[] = {0xBA};
    (void)write_bytes_aht(kReset, sizeof(kReset));
    vTaskDelay(pdMS_TO_TICKS(40));
    constexpr uint8_t kInit[] = {0xBE, 0x08, 0x00};
    if (!write_bytes_aht(kInit, sizeof(kInit))) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(120));
    constexpr uint8_t kTrig[] = {0xAC, 0x33, 0x00};
    if (!write_bytes_aht(kTrig, sizeof(kTrig))) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(90));
    uint8_t rb[6] = {0};
    if (!read_bytes_aht(rb, sizeof(rb))) {
        return false;
    }
    if ((rb[0] & 0x98) != 0x18) {
        return false;
    }
    const uint32_t raw_h =
        (static_cast<uint32_t>(rb[1]) << 12) | (static_cast<uint32_t>(rb[2]) << 4) | (static_cast<uint32_t>(rb[3]) >> 4);
    const uint32_t raw_t = ((static_cast<uint32_t>(rb[3]) & 0x0FU) << 16) | (static_cast<uint32_t>(rb[4]) << 8) |
                           static_cast<uint32_t>(rb[5]);
    const float rh = (static_cast<float>(raw_h) * 100.0f) / 1048576.0f;
    float t = (static_cast<float>(raw_t) * 200.0f / 1048576.0f) - 50.0f;
    if (t < -40.0f || t > 90.0f || rh < 0.0f || rh > 100.0f) {
        return false;
    }
    *temp_c = t;
    *rh_pct = rh;
    return true;
}

} // namespace

extern "C" {

const char *romeos_room_sht_touch_bus_family_name(void)
{
    switch (s_family) {
    case BusFamily::Sht4x:
        return "SHT4x";
    case BusFamily::Sht3x:
        return "SHT3x";
    case BusFamily::Aht2x:
        return "AHT2x";
    default:
        return "none";
    }
}

bool romeos_room_sht_touch_bus_begin(uint8_t i2c_addr7)
{
    s_addr7 = i2c_addr7;
    s_family = BusFamily::None;

    float t = NAN;
    float rh = NAN;
    if (sht4x_probe_measure(&t, &rh)) {
        s_family = BusFamily::Sht4x;
        return true;
    }

    if (sht3x_begin()) {
        s_family = BusFamily::Sht3x;
        return true;
    }

    s_family = BusFamily::None;
    return false;
}

bool romeos_room_sht_touch_bus_begin_auto(void)
{
    if (romeos_room_sht_touch_bus_begin(0x44)) {
        return true;
    }
    if (romeos_room_sht_touch_bus_begin(0x45)) {
        return true;
    }
    s_addr7 = 0x38;
    s_family = BusFamily::None;
    float t = NAN;
    float rh = NAN;
    if (aht21_probe_measure(&t, &rh)) {
        s_family = BusFamily::Aht2x;
        return true;
    }
    s_family = BusFamily::None;
    return false;
}

void romeos_room_sht_touch_bus_soft_reset(void)
{
    if (s_family == BusFamily::Sht4x) {
        (void)write_cmd8(kSht4SoftReset);
        vTaskDelay(pdMS_TO_TICKS(2));
        return;
    }
    if (s_family == BusFamily::Aht2x) {
        constexpr uint8_t kReset[] = {0xBA};
        (void)write_bytes_aht(kReset, sizeof(kReset));
        vTaskDelay(pdMS_TO_TICKS(40));
        return;
    }
    (void)write_cmd16(kSht3SoftReset);
    vTaskDelay(pdMS_TO_TICKS(10));
}

bool romeos_room_sht_touch_bus_read_both(float *temp_c, float *rh_pct)
{
    if (temp_c == nullptr || rh_pct == nullptr) {
        return false;
    }
    *temp_c = std::nanf("");
    *rh_pct = std::nanf("");

    if (s_family == BusFamily::Sht4x) {
        if (!write_cmd8(kSht4MeasHighPrec)) {
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(30));
        uint8_t rb[6] = {0};
        if (!read_n(rb, sizeof(rb))) {
            return false;
        }
        return sht4x_one_shot_decode(rb, temp_c, rh_pct);
    }

    if (s_family == BusFamily::Aht2x) {
        constexpr uint8_t kTrig[] = {0xAC, 0x33, 0x00};
        if (!write_bytes_aht(kTrig, sizeof(kTrig))) {
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(90));
        uint8_t rb[6] = {0};
        if (!read_bytes_aht(rb, sizeof(rb))) {
            return false;
        }
        if ((rb[0] & 0x98) != 0x18) {
            return false;
        }
        const uint32_t raw_h =
            (static_cast<uint32_t>(rb[1]) << 12) | (static_cast<uint32_t>(rb[2]) << 4) |
            (static_cast<uint32_t>(rb[3]) >> 4);
        const uint32_t raw_t = ((static_cast<uint32_t>(rb[3]) & 0x0FU) << 16) |
                               (static_cast<uint32_t>(rb[4]) << 8) | static_cast<uint32_t>(rb[5]);
        *rh_pct = (static_cast<float>(raw_h) * 100.0f) / 1048576.0f;
        *temp_c = (static_cast<float>(raw_t) * 200.0f / 1048576.0f) - 50.0f;
        return std::isfinite(*temp_c) && *temp_c >= -40.0f && *temp_c <= 90.0f;
    }

    if (s_family != BusFamily::Sht3x) {
        return false;
    }

    if (!write_cmd16(kSht3MeasHighRep)) {
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(25));

    uint8_t rb[6] = {0};
    if (!read_n(rb, sizeof(rb))) {
        return false;
    }
    if (rb[2] != crc8(rb, 2) || rb[5] != crc8(rb + 3, 2)) {
        return false;
    }

    int32_t stemp = static_cast<int32_t>((static_cast<uint32_t>(rb[0]) << 8) | rb[1]);
    stemp = ((4375 * stemp) >> 14) - 4500;
    *temp_c = static_cast<float>(stemp) / 100.0f;

    uint32_t shum = (static_cast<uint32_t>(rb[3]) << 8) | rb[4];
    shum = (625U * shum) >> 12;
    *rh_pct = static_cast<float>(shum) / 100.0f;
    return true;
}

} // extern "C"
