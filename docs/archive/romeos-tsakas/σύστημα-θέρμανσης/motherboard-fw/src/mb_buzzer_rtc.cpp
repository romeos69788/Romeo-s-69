#include "mb_buzzer_rtc.h"

#include <cstdio>
#include <cstring>
#include <Wire.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include <sys/time.h>
#include <time.h>

#include "driver/gpio.h"
#include "pin_map.h"

namespace {

constexpr uint8_t kDs3231Addr = 0x68;
constexpr uint32_t kBeepMs = 450;
constexpr uint32_t kPauseMs = 200;
constexpr uint8_t kBootBeepCount = 3;
constexpr uint8_t kLedcResolutionBits = 10;

#ifndef ROMEOS_MB_BUZZER_HZ
#define ROMEOS_MB_BUZZER_HZ 4000
#endif

/** 0 = PWM tone (παθητικό). 1 = DC HIGH (KY-012 ενεργό — προεπιλογή PCB). */
#ifndef ROMEOS_MB_BUZZER_ACTIVE
#define ROMEOS_MB_BUZZER_ACTIVE 1
#endif

enum class BuzzerMode : uint8_t {
    Tone = 0,
    ActiveHigh = 1,
    ActiveLow = 2,
};

BuzzerMode g_buzzer_mode =
    (ROMEOS_MB_BUZZER_ACTIVE != 0) ? BuzzerMode::ActiveHigh : BuzzerMode::Tone;
uint32_t g_buzzer_hz = ROMEOS_MB_BUZZER_HZ;
bool g_buzzer_ledc_attached = false;

uint8_t bcd_to_dec(uint8_t v)
{
    return static_cast<uint8_t>(((v >> 4) & 0x0Fu) * 10u + (v & 0x0Fu));
}

uint8_t dec_to_bcd(uint8_t v)
{
    return static_cast<uint8_t>(((v / 10u) << 4) | (v % 10u));
}

void buzzer_pin_drive_max()
{
    gpio_set_drive_capability(static_cast<gpio_num_t>(PIN_BUZZER_ALARM), GPIO_DRIVE_CAP_3);
}

void buzzer_hw_stop()
{
    if (g_buzzer_ledc_attached) {
        ledcWriteTone(PIN_BUZZER_ALARM, 0);
        ledcDetach(PIN_BUZZER_ALARM);
        g_buzzer_ledc_attached = false;
    }
    pinMode(PIN_BUZZER_ALARM, OUTPUT);
    buzzer_pin_drive_max();
    if (g_buzzer_mode == BuzzerMode::ActiveLow) {
        digitalWrite(PIN_BUZZER_ALARM, HIGH);
    } else {
        digitalWrite(PIN_BUZZER_ALARM, LOW);
    }
}

void buzzer_hw_start()
{
    if (g_buzzer_mode == BuzzerMode::ActiveHigh) {
        if (g_buzzer_ledc_attached) {
            ledcWriteTone(PIN_BUZZER_ALARM, 0);
            ledcDetach(PIN_BUZZER_ALARM);
            g_buzzer_ledc_attached = false;
        }
        pinMode(PIN_BUZZER_ALARM, OUTPUT);
        buzzer_pin_drive_max();
        digitalWrite(PIN_BUZZER_ALARM, HIGH);
        return;
    }
    if (g_buzzer_mode == BuzzerMode::ActiveLow) {
        if (g_buzzer_ledc_attached) {
            ledcWriteTone(PIN_BUZZER_ALARM, 0);
            ledcDetach(PIN_BUZZER_ALARM);
            g_buzzer_ledc_attached = false;
        }
        pinMode(PIN_BUZZER_ALARM, OUTPUT);
        buzzer_pin_drive_max();
        digitalWrite(PIN_BUZZER_ALARM, LOW);
        return;
    }

    if (g_buzzer_ledc_attached) {
        ledcWriteTone(PIN_BUZZER_ALARM, g_buzzer_hz);
        return;
    }
    pinMode(PIN_BUZZER_ALARM, OUTPUT);
    buzzer_pin_drive_max();
    digitalWrite(PIN_BUZZER_ALARM, LOW);
    if (!ledcAttach(PIN_BUZZER_ALARM, g_buzzer_hz, kLedcResolutionBits)) {
        Serial.println(F("[buzzer] ledc attach failed — fallback DC HIGH"));
        g_buzzer_mode = BuzzerMode::ActiveHigh;
        digitalWrite(PIN_BUZZER_ALARM, HIGH);
        return;
    }
    g_buzzer_ledc_attached = true;
    ledcWriteTone(PIN_BUZZER_ALARM, g_buzzer_hz);
}

void buzzer_log_pin(const char *tag)
{
    const int level = digitalRead(PIN_BUZZER_ALARM);
    Serial.printf("[buzzer] %s GPIO15=%d (μέτρα SING-GND: ON~3.3V OFF~0V)\n", tag, level);
}

void beep_once()
{
    buzzer_hw_start();
    buzzer_log_pin("beep-on");
    delay(kBeepMs);
    buzzer_hw_stop();
}

bool i2c_probe(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

void run_buzzer_hold()
{
    g_buzzer_mode = BuzzerMode::ActiveHigh;
    Serial.println(F("[buzzer] HOLD 5s — SING=HIGH (KY-012)"));
    Serial.println(F("[buzzer] πολύμετρο: SING-GND πρέπει ~3.3V, VCC-GND ~5V"));
    buzzer_hw_start();
    buzzer_log_pin("hold");
    delay(5000);
    buzzer_hw_stop();
    buzzer_log_pin("off");
    Serial.println(F("[buzzer] HOLD done — BUZZ OFF"));
}

void run_buzzer_square_wave(uint32_t hz, uint32_t ms)
{
    Serial.printf("[buzzer] square wave %lu Hz %lu ms\n",
                  static_cast<unsigned long>(hz),
                  static_cast<unsigned long>(ms));
    if (g_buzzer_ledc_attached) {
        ledcDetach(PIN_BUZZER_ALARM);
        g_buzzer_ledc_attached = false;
    }
    pinMode(PIN_BUZZER_ALARM, OUTPUT);
    buzzer_pin_drive_max();
    const uint32_t half_us = (1000000UL / hz) / 2UL;
    const uint32_t end_ms = millis() + ms;
    while (millis() < end_ms) {
        digitalWrite(PIN_BUZZER_ALARM, HIGH);
        delayMicroseconds(half_us);
        digitalWrite(PIN_BUZZER_ALARM, LOW);
        delayMicroseconds(half_us);
    }
    buzzer_hw_stop();
}

const char *buzzer_mode_label()
{
    switch (g_buzzer_mode) {
    case BuzzerMode::ActiveHigh:
        return "active-high";
    case BuzzerMode::ActiveLow:
        return "active-low";
    default:
        return "tone-pwm";
    }
}

void run_buzzer_diag()
{
    Serial.println(F("[buzzer] DIAG — 3 δοκιμές x 350 ms (άκου ποια ακούγεται)"));
    const BuzzerMode saved = g_buzzer_mode;

    g_buzzer_mode = BuzzerMode::ActiveHigh;
    Serial.println(F("[buzzer] 1/3 ACTIVE HIGH (KY-012)"));
    buzzer_hw_start();
    delay(350);
    buzzer_hw_stop();
    delay(250);

    g_buzzer_mode = BuzzerMode::ActiveLow;
    Serial.println(F("[buzzer] 2/3 ACTIVE LOW (αντεστραμμένο module)"));
    buzzer_hw_start();
    delay(350);
    buzzer_hw_stop();
    delay(250);

    g_buzzer_mode = BuzzerMode::Tone;
    Serial.printf("[buzzer] 3/4 TONE PWM %lu Hz\n", static_cast<unsigned long>(g_buzzer_hz));
    buzzer_hw_start();
    delay(350);
    buzzer_hw_stop();
    delay(250);

    Serial.println(F("[buzzer] 4/4 SQUARE 2700 Hz"));
    run_buzzer_square_wave(2700, 350);

    g_buzzer_mode = saved;
    buzzer_hw_stop();
    Serial.println(F("[buzzer] DIAG done — BUZZ MODE ACTIVE / LOW / TONE / WAVE"));
}

void run_buzzer_sweep()
{
    Serial.println(F("[buzzer] sweep 1500..5000 Hz (200 ms/step)"));
    constexpr uint32_t freqs[] = {1500, 2000, 2500, 2731, 3000, 3500, 4000, 4500, 5000};
    for (const uint32_t hz : freqs) {
        g_buzzer_hz = hz;
        Serial.printf("[buzzer] freq %lu Hz\n", static_cast<unsigned long>(hz));
        buzzer_hw_start();
        delay(200);
        buzzer_hw_stop();
        delay(120);
    }
    Serial.println(F("[buzzer] sweep done — BUZZ FREQ <Hz> για να κλειδώσεις"));
}

struct RtcSnapshot {
    bool ok = false;
    bool halted = false;
    uint16_t year = 0;
    uint8_t mon = 0;
    uint8_t mday = 0;
    uint8_t hour = 0;
    uint8_t min = 0;
    uint8_t sec = 0;
    uint8_t wday = 0;
};

bool rtc_read_snapshot(RtcSnapshot *out)
{
    if (out == nullptr) {
        return false;
    }
    *out = RtcSnapshot{};

    if (!i2c_probe(kDs3231Addr)) {
        return false;
    }

    Wire.beginTransmission(kDs3231Addr);
    Wire.write(0x00);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    if (Wire.requestFrom(static_cast<int>(kDs3231Addr), 7) != 7) {
        return false;
    }

    const uint8_t sec_raw = Wire.read();
    const uint8_t min_raw = Wire.read();
    const uint8_t hour_raw = Wire.read();
    const uint8_t wday = Wire.read();
    const uint8_t mday_raw = Wire.read();
    const uint8_t mon_raw = Wire.read();
    const uint8_t year_raw = Wire.read();

    out->halted = (sec_raw & 0x80u) != 0;
    out->sec = bcd_to_dec(static_cast<uint8_t>(sec_raw & 0x7Fu));
    out->min = bcd_to_dec(min_raw);

    const bool hour24 = (hour_raw & 0x40u) == 0;
    if (hour24) {
        out->hour = bcd_to_dec(static_cast<uint8_t>(hour_raw & 0x3Fu));
    } else {
        uint8_t hour12 = bcd_to_dec(static_cast<uint8_t>(hour_raw & 0x1Fu));
        if (hour_raw & 0x20u) {
            hour12 = static_cast<uint8_t>((hour12 % 12u) + 12u);
        }
        out->hour = hour12;
    }

    out->wday = wday;
    out->mday = bcd_to_dec(mday_raw);
    out->mon = bcd_to_dec(static_cast<uint8_t>(mon_raw & 0x1Fu));
    out->year = static_cast<uint16_t>(2000u + bcd_to_dec(year_raw));
    out->ok = true;
    return true;
}

void format_rtc_lcd_line(char *buf, size_t len)
{
    if (buf == nullptr || len == 0) {
        return;
    }

    RtcSnapshot rtc;
    if (!rtc_read_snapshot(&rtc)) {
        std::snprintf(buf, len, "RTC U9 --");
        return;
    }
    if (rtc.halted) {
        std::snprintf(buf, len, "RTC halted");
        return;
    }

    std::snprintf(buf,
                  len,
                  "%02u/%02u/%04u %02u:%02u:%02u",
                  static_cast<unsigned>(rtc.mday),
                  static_cast<unsigned>(rtc.mon),
                  static_cast<unsigned>(rtc.year),
                  static_cast<unsigned>(rtc.hour),
                  static_cast<unsigned>(rtc.min),
                  static_cast<unsigned>(rtc.sec));
}

bool rtc_write_from_tm(const struct tm *lt)
{
    if (lt == nullptr || !i2c_probe(kDs3231Addr)) {
        return false;
    }

    const uint8_t sec = static_cast<uint8_t>(lt->tm_sec);
    const uint8_t min = static_cast<uint8_t>(lt->tm_min);
    const uint8_t hour = static_cast<uint8_t>(lt->tm_hour);
    const uint8_t mday = static_cast<uint8_t>(lt->tm_mday);
    const uint8_t mon = static_cast<uint8_t>(lt->tm_mon + 1);
    const int year = lt->tm_year + 1900;
    if (year < 2000 || year > 2099 || mon < 1 || mon > 12 || mday < 1 || mday > 31) {
        return false;
    }

    const uint8_t wday = static_cast<uint8_t>(lt->tm_wday + 1); /* DS3231: 1=Sunday */

    Wire.beginTransmission(kDs3231Addr);
    Wire.write(0x00);
    Wire.write(dec_to_bcd(sec)); /* CH=0 — ξεκινά το ρολόι */
    Wire.write(dec_to_bcd(min));
    Wire.write(dec_to_bcd(hour)); /* 24h */
    Wire.write(wday);
    Wire.write(dec_to_bcd(mday));
    Wire.write(dec_to_bcd(mon));
    Wire.write(dec_to_bcd(static_cast<uint8_t>(year - 2000)));
    return Wire.endTransmission() == 0;
}

bool s_sntp_running = false;
bool s_ntp_sync_done = false;
bool s_ntp_warned_slow = false;
uint32_t s_sntp_started_ms = 0;
portMUX_TYPE s_pending_mux = portMUX_INITIALIZER_UNLOCKED;
volatile bool s_have_pending = false;
struct tm s_pending_tm {};

void ntp_sync_notification_cb(struct timeval *tv)
{
    if (!tv) {
        return;
    }
    s_ntp_sync_done = true;
    struct tm local {};
    localtime_r(&tv->tv_sec, &local);
    portENTER_CRITICAL(&s_pending_mux);
    s_pending_tm = local;
    s_have_pending = true;
    portEXIT_CRITICAL(&s_pending_mux);
}

void ntp_stack_start()
{
    if (s_sntp_running) {
        return;
    }
    setenv("TZ", "EET-2EEST-3,M3.5.0/3,M10.5.0/4", 1);
    tzset();

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_set_time_sync_notification_cb(ntp_sync_notification_cb);
    esp_sntp_init();
    s_sntp_running = true;
    s_sntp_started_ms = millis();
    s_ntp_sync_done = false;
    s_ntp_warned_slow = false;
    Serial.println(F("[rtc] SNTP started (TZ=EET/EEST) — θα γράψει DS3231 U9"));
}

void ntp_stack_stop()
{
    if (!s_sntp_running) {
        return;
    }
    esp_sntp_stop();
    s_sntp_running = false;
    Serial.println(F("[rtc] SNTP stopped"));
}

void apply_pending_rtc_write()
{
    bool have = false;
    struct tm copy {};
    portENTER_CRITICAL(&s_pending_mux);
    if (s_have_pending) {
        have = true;
        s_have_pending = false;
        copy = s_pending_tm;
    }
    portEXIT_CRITICAL(&s_pending_mux);

    if (!have) {
        return;
    }

    if (!rtc_write_from_tm(&copy)) {
        Serial.println(F("[rtc] DS3231 write FAILED"));
        return;
    }

    Serial.printf("[rtc] DS3231 set from NTP: %04d-%02d-%02d %02d:%02d:%02d\n",
                  copy.tm_year + 1900,
                  copy.tm_mon + 1,
                  copy.tm_mday,
                  copy.tm_hour,
                  copy.tm_min,
                  copy.tm_sec);
}

void rtc_ntp_poll_impl(void)
{
    if (WiFi.status() != WL_CONNECTED) {
        ntp_stack_stop();
        return;
    }

    if (!s_sntp_running) {
        ntp_stack_start();
    }

    apply_pending_rtc_write();

    const uint32_t now = millis();
    if (s_sntp_running && !s_ntp_sync_done && !s_ntp_warned_slow && s_sntp_started_ms != 0 &&
        (now - s_sntp_started_ms) >= 60000u) {
        s_ntp_warned_slow = true;
        Serial.println(F("[rtc] no NTP sync after 60s — έλεγχος internet / router"));
    }
}

}  // namespace

void mb_rtc_format_lcd_line(char *buf, size_t len)
{
    format_rtc_lcd_line(buf, len);
}

void mb_rtc_ntp_poll(void)
{
    rtc_ntp_poll_impl();
}

void mb_buzzer_set(bool on)
{
    if (on) {
        buzzer_hw_start();
    } else {
        buzzer_hw_stop();
    }
}

void mb_buzzer_boot_chime()
{
    Serial.printf("[buzzer] boot chime — 3 beeps (%s @ %lu Hz)\n",
                  buzzer_mode_label(),
                  static_cast<unsigned long>(g_buzzer_hz));
    for (uint8_t i = 0; i < kBootBeepCount; ++i) {
        beep_once();
        if (i + 1 < kBootBeepCount) {
            delay(kPauseMs);
        }
    }
    buzzer_hw_stop();
    Serial.println(F("[buzzer] boot chime done"));
}

void mb_buzzer_rtc_print_help()
{
    Serial.println(F("[buzzer] BUZZ ON  | BUZZ OFF | BUZZ TEST (3 beeps)"));
    Serial.println(F("[buzzer] BUZZ SWEEP — δοκιμή συχνοτήτων (πιο δυνατό)"));
    Serial.println(F("[buzzer] BUZZ FREQ 4000 — συχνότητα Hz (tone mode)"));
    Serial.println(F("[buzzer] BUZZ MODE ACTIVE | BUZZ MODE LOW | BUZZ MODE TONE"));
    Serial.println(F("[buzzer] BUZZ HOLD — 5s ON για μέτρηση πολύμετρο"));
    Serial.println(F("[buzzer] BUZZ WAVE — τετραγωνικό 2700 Hz 1s"));
    Serial.println(F("[buzzer] I2C SCAN — I2C bus (LCD 0x27, DS3231 0x68)"));
    Serial.println(F("[buzzer] RTC READ — ώρα από U9 DS3231"));
    Serial.println(F("[buzzer] NTP→RTC αυτόματα όταν Wi‑Fi home συνδεδεμένο"));
    Serial.println(F("[buzzer] Boot: 3 beeps αυτόματα με 12 V ON"));
}

static void run_i2c_scan()
{
    Serial.println(F("[i2c] scan SDA=21 SCL=22"));
    uint8_t found = 0;
    for (uint8_t addr = 0x08; addr < 0x78; ++addr) {
        if (!i2c_probe(addr)) {
            continue;
        }
        Serial.printf("[i2c] device 0x%02X", addr);
        if (addr == 0x27) {
            Serial.print(F(" (LCD 2004)"));
        } else if (addr == kDs3231Addr) {
            Serial.print(F(" (DS3231 RTC)"));
        } else if (addr == 0x57) {
            Serial.print(F(" (AT24C32 EEPROM on RTC module?)"));
        }
        Serial.println();
        ++found;
    }
    if (found == 0) {
        Serial.println(F("[i2c] no devices — έλεγχος I2C / τροφοδοσία modules"));
    } else {
        Serial.printf("[i2c] total %u device(s)\n", static_cast<unsigned>(found));
    }
}

static void run_rtc_read()
{
    RtcSnapshot rtc;
    if (!rtc_read_snapshot(&rtc)) {
        Serial.println(F("[rtc] DS3231 NOT FOUND at 0x68 — U9 / μπαταρία / header"));
        return;
    }

    if (rtc.halted) {
        Serial.println(F("[rtc] WARNING: clock halt (CH) bit set — ώρα σταματημένη;"));
    }

    Serial.printf("[rtc] DS3231 OK  %04u-%02u-%02u %02u:%02u:%02u  wday=%u\n",
                  rtc.year,
                  static_cast<unsigned>(rtc.mon),
                  static_cast<unsigned>(rtc.mday),
                  static_cast<unsigned>(rtc.hour),
                  static_cast<unsigned>(rtc.min),
                  static_cast<unsigned>(rtc.sec),
                  static_cast<unsigned>(rtc.wday));
}

bool mb_buzzer_rtc_handle_serial(const char *line)
{
    if (line == nullptr || line[0] == '\0') {
        return false;
    }

    if (strcmp(line, "BUZZ HELP") == 0 || strcmp(line, "I2C HELP") == 0 ||
        strcmp(line, "RTC HELP") == 0) {
        mb_buzzer_rtc_print_help();
        return true;
    }
    if (strcmp(line, "BUZZ ON") == 0) {
        mb_buzzer_set(true);
        Serial.printf("[buzzer] ON (%s @ %lu Hz)\n",
                      buzzer_mode_label(),
                      static_cast<unsigned long>(g_buzzer_hz));
        buzzer_log_pin("on");
        return true;
    }
    if (strcmp(line, "BUZZ OFF") == 0) {
        mb_buzzer_set(false);
        Serial.println(F("[buzzer] OFF"));
        return true;
    }
    if (strcmp(line, "BUZZ TEST") == 0) {
        mb_buzzer_boot_chime();
        return true;
    }
    if (strcmp(line, "BUZZ SWEEP") == 0) {
        run_buzzer_sweep();
        return true;
    }
    if (strcmp(line, "BUZZ MODE TONE") == 0) {
        g_buzzer_mode = BuzzerMode::Tone;
        Serial.println(F("[buzzer] mode=tone-pwm (παθητικό / PWM)"));
        return true;
    }
    if (strcmp(line, "BUZZ MODE ACTIVE") == 0) {
        g_buzzer_mode = BuzzerMode::ActiveHigh;
        Serial.println(F("[buzzer] mode=active-high (KY-012, SING=HIGH)"));
        return true;
    }
    if (strcmp(line, "BUZZ MODE LOW") == 0) {
        g_buzzer_mode = BuzzerMode::ActiveLow;
        Serial.println(F("[buzzer] mode=active-low (SING=LOW)"));
        return true;
    }
    if (strcmp(line, "BUZZ DIAG") == 0) {
        run_buzzer_diag();
        return true;
    }
    if (strcmp(line, "BUZZ HOLD") == 0) {
        run_buzzer_hold();
        return true;
    }
    if (strcmp(line, "BUZZ WAVE") == 0) {
        run_buzzer_square_wave(2700, 1000);
        return true;
    }
    if (strncmp(line, "BUZZ FREQ ", 10) == 0) {
        const unsigned long hz = strtoul(line + 10, nullptr, 10);
        if (hz < 500 || hz > 10000) {
            Serial.println(F("[buzzer] FREQ range 500..10000 Hz"));
            return true;
        }
        g_buzzer_hz = static_cast<uint32_t>(hz);
        g_buzzer_mode = BuzzerMode::Tone;
        Serial.printf("[buzzer] freq=%lu Hz mode=tone-pwm\n",
                      static_cast<unsigned long>(g_buzzer_hz));
        return true;
    }
    if (strcmp(line, "I2C SCAN") == 0) {
        run_i2c_scan();
        return true;
    }
    if (strcmp(line, "RTC READ") == 0) {
        run_rtc_read();
        return true;
    }
    return false;
}
