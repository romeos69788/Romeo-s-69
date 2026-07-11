#include <Arduino.h>
#include <Adafruit_MAX31865.h>
#include <SPI.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "mb_solar_rtd.h"
#include "pin_map.h"

namespace {

constexpr float k_dr_dt = 0.385f;
constexpr float k_rref = 430.0f;
constexpr float k_r_bench_lo = 110.0f;
constexpr float k_r_bench_hi = 170.0f;
constexpr float k_band_lo = 110.0f;
constexpr float k_band_hi = 175.0f;
constexpr float k_band_spread_max = 12.0f;
constexpr float k_r_up_max_idle = 4.0f;
constexpr float k_r_up_max_heat = 6.0f;
constexpr float k_t_up_max_poll = 4.0f;
constexpr float k_t_display_max = 85.0f;
constexpr float k_stuck_t_gap = 8.0f;
constexpr float k_track_idle = 0.35f;
constexpr float k_track_heat = 0.55f;
constexpr float k_track_cool = 0.75f;
constexpr float k_ema_idle = 0.40f;
constexpr float k_ema_heat = 0.60f;
constexpr float k_ema_cool = 0.75f;
constexpr uint32_t k_poll_ms = 400u;
constexpr uint8_t k_reads = 7u;
constexpr uint8_t k_mode_streak_need = 2u;

Adafruit_MAX31865 g_max31865(PIN_CS_SOLAR, &SPI);

bool g_begin_ok = false;
bool g_live = false;
bool g_baseline_set = false;
int16_t g_temp_c_x10 = 0;
float g_r_base = 126.0f;
float g_t_base = 25.0f;
float g_t_ema = NAN;
float g_r_last = NAN;
float g_r_track = NAN;
float g_r_spread = 0.0f;
uint8_t g_rise_streak = 0u;
uint8_t g_fall_streak = 0u;
bool g_heating = false;
bool g_cooling = false;
uint8_t g_last_fault = 0;
uint16_t g_last_rtd_raw = 0;
uint32_t g_next_poll_ms = 0;

void solar_cs_idle()
{
    digitalWrite(PIN_CS_SOLAR, HIGH);
    digitalWrite(PIN_CS_POT, HIGH);
    delayMicroseconds(50);
}

float rtd_to_ohm(uint16_t rtd)
{
    return (static_cast<float>(rtd) / 32768.0f) * k_rref;
}

int16_t temp_to_x10(float t_c)
{
    return static_cast<int16_t>(t_c * 10.0f + (t_c >= 0.0f ? 0.5f : -0.5f));
}

float median_n(float *v, uint8_t n)
{
    std::sort(v, v + n);
    if ((n & 1u) != 0u) {
        return v[n / 2u];
    }
    return (v[n / 2u - 1u] + v[n / 2u]) * 0.5f;
}

uint16_t read_rtd_once()
{
    solar_cs_idle();
    delayMicroseconds(200);
    g_max31865.enableBias(true);
    delay(2);
    const uint16_t rtd = g_max31865.readRTD();
    g_max31865.enableBias(true);
    solar_cs_idle();
    return rtd;
}

float temp_from_r(float r_ohm)
{
    return g_t_base + (r_ohm - g_r_base) / k_dr_dt;
}

void band_limits(float *lo_out, float *hi_out)
{
    float lo = k_band_lo;
    float hi = k_band_hi;
    if (!std::isnan(g_r_track)) {
        lo = g_r_track - 25.0f;
        hi = g_r_track + 18.0f;
    }
    if (lo < 100.0f) {
        lo = 100.0f;
    }
    if (hi > 195.0f) {
        hi = 195.0f;
    }
    if (lo_out != nullptr) {
        *lo_out = lo;
    }
    if (hi_out != nullptr) {
        *hi_out = hi;
    }
}

void update_mode(float r_pick)
{
    if (std::isnan(g_r_track)) {
        g_rise_streak = 0u;
        g_fall_streak = 0u;
        g_heating = false;
        g_cooling = false;
        return;
    }

    if (r_pick > g_r_track + 0.15f) {
        ++g_rise_streak;
        g_fall_streak = 0u;
    } else if (r_pick < g_r_track - 0.15f) {
        ++g_fall_streak;
        g_rise_streak = 0u;
    } else {
        g_rise_streak = 0u;
        g_fall_streak = 0u;
    }

    g_heating = (g_rise_streak >= k_mode_streak_need);
    g_cooling = (g_fall_streak >= k_mode_streak_need);
    if (g_cooling) {
        g_heating = false;
        g_rise_streak = 0u;
    }
}

bool cluster_pick(const float *sorted, uint8_t n, float *r_out, float *spread_out)
{
    uint8_t best_i = 0u;
    uint8_t best_len = 0u;

    for (uint8_t i = 0u; i < n; ++i) {
        for (uint8_t j = i; j < n; ++j) {
            if (sorted[j] - sorted[i] > 12.0f) {
                break;
            }
            const uint8_t len = static_cast<uint8_t>(j - i + 1u);
            if (len > best_len) {
                best_len = len;
                best_i = i;
            }
        }
    }

    if (best_len < 2u) {
        return false;
    }

    const float spread = sorted[best_i + best_len - 1u] - sorted[best_i];
    if (spread > k_band_spread_max) {
        return false;
    }

    if (r_out != nullptr) {
        *r_out = median_n(const_cast<float *>(sorted) + best_i, best_len);
    }
    if (spread_out != nullptr) {
        *spread_out = spread;
    }
    return true;
}

bool r_pick_plausible(float r_pick, float spread)
{
    if (!g_baseline_set) {
        return true;
    }

    const float t_pick = temp_from_r(r_pick);

    if (!std::isnan(g_t_ema) && g_t_ema > (g_t_base + 15.0f) && t_pick < (g_t_ema - k_stuck_t_gap)) {
        return true;
    }

    if (t_pick > k_t_display_max) {
        return false;
    }

    if (!std::isnan(g_t_ema) && t_pick > (g_t_ema + k_t_up_max_poll)) {
        return false;
    }

    if (!std::isnan(g_r_track)) {
        const float dr = r_pick - g_r_track;
        if (dr > 0.0f) {
            const float up_max = g_heating ? k_r_up_max_heat : k_r_up_max_idle;
            if (dr > up_max) {
                return false;
            }
        }
    }

    if (spread > k_band_spread_max) {
        return false;
    }

    return true;
}

void apply_r_pick(float r_pick)
{
    float alpha = k_track_idle;
    if (g_cooling && !std::isnan(g_r_track) && r_pick < g_r_track) {
        alpha = k_track_cool;
    } else if (g_heating && !std::isnan(g_r_track) && r_pick > g_r_track) {
        alpha = k_track_heat;
    }

    if (std::isnan(g_r_track)) {
        g_r_track = r_pick;
    } else {
        g_r_track += alpha * (r_pick - g_r_track);
    }
}

void apply_stuck_recovery(float r_pick)
{
    const float t_pick = temp_from_r(r_pick);
    g_r_track = r_pick;
    g_t_ema = t_pick;
    g_rise_streak = 0u;
    g_fall_streak = 0u;
    g_heating = false;
    g_cooling = true;
    g_temp_c_x10 = temp_to_x10(t_pick);
    g_live = true;
    Serial.printf("[solar] RECOVER stuck T->%.1f R=%.1f\n",
                  static_cast<double>(t_pick),
                  static_cast<double>(r_pick));
}

void update_temp(float t_c)
{
    if (t_c > k_t_display_max) {
        t_c = k_t_display_max;
    }

    float alpha = k_ema_idle;
    if (g_cooling && !std::isnan(g_t_ema) && t_c < g_t_ema) {
        alpha = k_ema_cool;
    } else if (g_heating && !std::isnan(g_t_ema) && t_c > g_t_ema) {
        alpha = k_ema_heat;
    }

    if (std::isnan(g_t_ema)) {
        g_t_ema = t_c;
    } else {
        g_t_ema += alpha * (t_c - g_t_ema);
    }

    if (g_t_ema < -10.0f) {
        g_t_ema = -10.0f;
    } else if (g_t_ema > k_t_display_max) {
        g_t_ema = k_t_display_max;
    }

    g_temp_c_x10 = temp_to_x10(g_t_ema);
    g_live = true;
}

bool read_median_r(float *r_out, float *spread_out)
{
    float band[k_reads];
    uint8_t n_band = 0u;
    float lo = k_band_lo;
    float hi = k_band_hi;
    band_limits(&lo, &hi);

    for (uint8_t i = 0u; i < k_reads; ++i) {
        delay(75);
        const uint16_t rtd = read_rtd_once();
        if (rtd == 0u || rtd >= 32000u) {
            continue;
        }
        const float r = rtd_to_ohm(rtd);
        if (r >= lo && r <= hi) {
            band[n_band++] = r;
        }
    }

    g_last_fault = g_max31865.readFault(MAX31865_FAULT_NONE);
    if (g_last_fault != 0u) {
        g_max31865.clearFault();
    }

    float r_pick = NAN;
    float spread = 0.0f;

    if (n_band >= 2u) {
        std::sort(band, band + n_band);
        if (!cluster_pick(band, n_band, &r_pick, &spread)) {
            return false;
        }
    } else if (n_band == 1u && std::isnan(g_r_track) && band[0] >= 120.0f && band[0] <= 160.0f) {
        r_pick = band[0];
        spread = 0.0f;
    } else if (n_band == 1u && g_cooling && !std::isnan(g_r_track) && band[0] < g_r_track) {
        r_pick = band[0];
        spread = 0.0f;
    } else {
        return false;
    }

    if (g_baseline_set && !std::isnan(g_t_ema) && g_t_ema > (g_t_base + 15.0f)) {
        const float t_pick = temp_from_r(r_pick);
        if (t_pick < (g_t_ema - k_stuck_t_gap) && r_pick >= k_r_bench_lo && r_pick <= (k_r_bench_hi + 25.0f)) {
            apply_stuck_recovery(r_pick);
            if (r_out != nullptr) {
                *r_out = g_r_track;
            }
            if (spread_out != nullptr) {
                *spread_out = spread;
            }
            return true;
        }
    }

    if (!r_pick_plausible(r_pick, spread)) {
        return false;
    }

    update_mode(r_pick);
    apply_r_pick(r_pick);

    g_r_last = g_r_track;
    g_r_spread = spread;
    g_last_rtd_raw = static_cast<uint16_t>((g_r_track / k_rref) * 32768.0f + 0.5f);

    if (r_out != nullptr) {
        *r_out = g_r_track;
    }
    if (spread_out != nullptr) {
        *spread_out = spread;
    }
    return true;
}

void log_raw_burst_blocking()
{
    Serial.println(F("[solar] raw burst:"));
    for (uint8_t i = 0u; i < k_reads; ++i) {
        delay(100);
        const uint16_t rtd = read_rtd_once();
        const float r = (rtd == 0u) ? NAN : rtd_to_ohm(rtd);
        Serial.printf("[solar]   #%u rtd=%u R=%.1f\n",
                      static_cast<unsigned>(i + 1u),
                      static_cast<unsigned>(rtd),
                      static_cast<double>(r));
    }
}

}  // namespace

void mb_solar_rtd_begin()
{
    solar_cs_idle();
    pinMode(PIN_CS_SOLAR, OUTPUT);
    pinMode(PIN_CS_POT, OUTPUT);
    solar_cs_idle();

    if (!g_max31865.begin(MAX31865_2WIRE)) {
        Serial.println(F("[solar] MAX31865 begin FAILED"));
        g_begin_ok = false;
        return;
    }

    g_max31865.clearFault();
    g_max31865.enable50Hz(true);
    g_max31865.setThresholds(0, 0xFFFF);
    g_max31865.setWires(MAX31865_2WIRE);
    g_max31865.enableBias(true);
    g_max31865.autoConvert(false);
    delay(200);

    float r = NAN;
    if (read_median_r(&r, nullptr)) {
        Serial.printf("[solar] boot 2W R=%.1f spr=%.1f\n",
                      static_cast<double>(r),
                      static_cast<double>(g_r_spread));
    } else {
        Serial.println(F("[solar] boot: waiting for reads..."));
    }

    g_begin_ok = true;
    g_live = false;
    g_baseline_set = false;
    g_t_ema = NAN;
    g_r_track = NAN;
    g_rise_streak = 0u;
    g_fall_streak = 0u;
    g_heating = false;
    g_cooling = false;
    g_next_poll_ms = millis();

    Serial.println(F("[solar] LOCKED 2W cap=85C anti-spike"));
}

void mb_solar_rtd_poll()
{
    if (!g_begin_ok) {
        return;
    }

    const uint32_t now = millis();
    if (now < g_next_poll_ms) {
        return;
    }
    g_next_poll_ms = now + k_poll_ms;

    float r = NAN;
    if (!read_median_r(&r, nullptr)) {
        return;
    }

    if (!g_baseline_set) {
        return;
    }

    update_temp(temp_from_r(g_r_track));

    static uint32_t last_log = 0;
    if (now - last_log >= 5000u) {
        last_log = now;
        Serial.printf("[solar] R=%.1f T=%d.%d h=%u c=%u\n",
                      static_cast<double>(g_r_track),
                      g_temp_c_x10 / 10,
                      std::abs(static_cast<int>(g_temp_c_x10 % 10)),
                      g_heating ? 1u : 0u,
                      g_cooling ? 1u : 0u);
    }
}

bool mb_solar_rtd_begin_ok()
{
    return g_begin_ok;
}

bool mb_solar_rtd_has_live()
{
    return g_live && g_baseline_set;
}

int16_t mb_solar_rtd_temp_c_x10()
{
    return g_temp_c_x10;
}

uint8_t mb_solar_rtd_last_fault()
{
    return g_last_fault;
}

uint16_t mb_solar_rtd_last_rtd_raw()
{
    return g_last_rtd_raw;
}

float mb_solar_rtd_last_resistance_ohm()
{
    return g_r_last;
}

bool mb_solar_rtd_is_calibrated()
{
    return g_baseline_set;
}

void mb_solar_rtd_try_calibrate(int16_t reference_temp_c_x10)
{
    if (g_baseline_set || std::isnan(g_r_track)) {
        return;
    }
    if (g_r_track < k_r_bench_lo || g_r_track > k_r_bench_hi) {
        return;
    }
    if (g_r_spread > k_band_spread_max) {
        return;
    }

    g_r_base = g_r_track;
    g_t_base = static_cast<float>(reference_temp_c_x10) / 10.0f;
    g_t_ema = g_t_base;
    g_baseline_set = true;
    g_temp_c_x10 = reference_temp_c_x10;
    g_live = true;

    Serial.printf("[solar] LIVE R0=%.1f T0=%.1fC\n",
                  static_cast<double>(g_r_base),
                  static_cast<double>(g_t_base));
}

void mb_solar_rtd_format_lcd_line(char *buf, size_t len)
{
    if (buf == nullptr || len == 0u) {
        return;
    }

    if (!g_begin_ok) {
        std::snprintf(buf, len, "SOLAR MAX31865 FAIL");
        return;
    }

    if (g_live && g_baseline_set) {
        const int w = g_temp_c_x10 / 10;
        const int f = std::abs(static_cast<int>(g_temp_c_x10 % 10));
        std::snprintf(buf, len, "SOLAR PT100 %2d.%dC", w, f);
        return;
    }

    if (!std::isnan(g_r_last)) {
        std::snprintf(buf,
                      len,
                      "SOLAR R=%d init",
                      static_cast<int>(g_r_last + 0.5f));
        return;
    }

    std::snprintf(buf, len, "SOLAR reading...");
}

void mb_solar_rtd_force_poll()
{
    g_next_poll_ms = 0;
    mb_solar_rtd_poll();
}

void mb_solar_rtd_run_diag()
{
    Serial.println(F("[solar] === SOLAR DIAG ==="));
    Serial.printf("[solar] 2W live=%d R=%.1f R0=%.1f T0=%.1f spr=%.1f fault=0x%02X\n",
                  g_live ? 1 : 0,
                  static_cast<double>(g_r_last),
                  static_cast<double>(g_r_base),
                  static_cast<double>(g_t_base),
                  static_cast<double>(g_r_spread),
                  static_cast<unsigned>(g_last_fault));
    log_raw_burst_blocking();

    char lcd[21];
    mb_solar_rtd_format_lcd_line(lcd, sizeof(lcd));
    Serial.printf("[solar] LCD: %s\n", lcd);
}
