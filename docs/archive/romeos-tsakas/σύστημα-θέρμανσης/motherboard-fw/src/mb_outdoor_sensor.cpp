#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <WiFi.h>
#include <cmath>
#include <cstring>

#include "driver/gpio.h"

#include "mb_outdoor_sensor.h"
#include "pin_map.h"

namespace {

/** CN4=GPIO4 μόνο — CN5=GPIO16 είναι DHT (mb_outdoor_dht), όχι 1-Wire. */
constexpr int k_candidate_pins[] = {PIN_DS18_DATA};
constexpr size_t k_candidate_count = sizeof(k_candidate_pins) / sizeof(k_candidate_pins[0]);

OneWire *g_ow = nullptr;
DallasTemperature *g_sensors = nullptr;

bool g_live = false;
int16_t g_temp_c_x10 = 0;
uint32_t g_last_poll_ms = 0;
uint32_t g_last_ok_ms = 0;
uint32_t g_last_diag_ms = 0;
uint32_t g_last_pin_scan_ms = 0;
uint8_t g_fail_streak = 0;
bool g_logged_ok = false;
int g_active_gpio = PIN_DS18_DATA;
bool g_have_rom = false;
DeviceAddress g_rom{};

constexpr uint32_t k_poll_interval_ms = 2000u;
constexpr uint8_t k_read_attempts = 10u;
constexpr uint8_t k_fail_before_stale = 15u;
constexpr uint32_t k_stale_after_ms = 60000u;
/** 12-bit: 750 ms — πιο αξιόπιστο για DS18B20. */
constexpr uint32_t k_convert_ms = 780u;
constexpr uint32_t k_rescan_other_pin_ms = 20000u;
constexpr size_t k_max_devices = 8u;
/** Bench bypass — GPIO2 (D2), ΟΧΙ GPIO17 (= REL_K6). */
constexpr int k_bypass_diag_pin = 2;

void log_rom(const uint8_t rom[8])
{
    Serial.printf("%02x%02x%02x%02x%02x%02x%02x%02x",
                  rom[0],
                  rom[1],
                  rom[2],
                  rom[3],
                  rom[4],
                  rom[5],
                  rom[6],
                  rom[7]);
}

int16_t float_c_to_x10(float t_c)
{
    return static_cast<int16_t>(t_c * 10.0f + (t_c >= 0.0f ? 0.5f : -0.5f));
}

bool valid_temp_c(float t_c)
{
    return t_c != DEVICE_DISCONNECTED_C && !std::isnan(t_c) && t_c > -40.0f && t_c < 85.0f;
}

bool scratchpad_plausible(const uint8_t sp[9], float t_c)
{
    if (sp[0] == 0u && sp[1] == 0u && sp[2] == 0u && sp[3] == 0u) {
        return false;
    }
    if (t_c == 0.0f && sp[0] == 0u && sp[1] == 0u) {
        return false;
    }
    return true;
}

const char *pin_label(int pin)
{
    if (pin == PIN_DS18_DATA) {
        return "CN4/GPIO4";
    }
    if (pin == PIN_DHT_DATA) {
        return "CN5/GPIO16";
    }
    if (pin == k_bypass_diag_pin) {
        return "BYPASS/GPIO2";
    }
    return "?";
}

void bus_prepare(int pin)
{
    gpio_reset_pin(static_cast<gpio_num_t>(pin));
    gpio_set_direction(static_cast<gpio_num_t>(pin), GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(static_cast<gpio_num_t>(pin), 1);
    gpio_set_pull_mode(static_cast<gpio_num_t>(pin), GPIO_PULLUP_ONLY);
}

void bus_release(int pin)
{
    gpio_set_pull_mode(static_cast<gpio_num_t>(pin), GPIO_FLOATING);
    gpio_set_direction(static_cast<gpio_num_t>(pin), GPIO_MODE_INPUT);
    pinMode(pin, INPUT);
}

void release_bus()
{
    if (g_sensors != nullptr) {
        delete g_sensors;
        g_sensors = nullptr;
    }
    if (g_ow != nullptr) {
        delete g_ow;
        g_ow = nullptr;
    }
    g_have_rom = false;
    std::memset(g_rom, 0, sizeof(g_rom));
}

bool bind_pin(int pin)
{
    if (pin == g_active_gpio && g_ow != nullptr) {
        return true;
    }
    release_bus();
    g_active_gpio = pin;
    bus_prepare(pin);
    g_ow = new OneWire(pin);
    g_sensors = new DallasTemperature(g_ow);
    g_sensors->begin();
    g_sensors->setResolution(12);
    g_sensors->setWaitForConversion(false);
    return true;
}

bool ow_presence(int pin)
{
    bus_prepare(pin);
    OneWire ow(pin);
    const bool pr = ow.reset();
    bus_release(pin);
    return pr;
}

void ow_select(OneWire &ow, const uint8_t *rom)
{
    if (rom != nullptr) {
        ow.write(0x55);
        for (uint8_t i = 0; i < 8u; ++i) {
            ow.write(rom[i]);
        }
    } else {
        ow.write(0xCC);
    }
}

bool read_scratchpad_after_convert(int pin, const uint8_t *rom, uint8_t sp_out[9])
{
    bus_prepare(pin);
    OneWire ow(pin);

    noInterrupts();
    const bool pr1 = ow.reset();
    if (pr1) {
        ow_select(ow, rom);
        ow.write(0x44);
    }
    interrupts();
    if (!pr1) {
        bus_release(pin);
        return false;
    }

    delay(k_convert_ms);

    noInterrupts();
    const bool pr2 = ow.reset();
    if (pr2) {
        ow_select(ow, rom);
        ow.write(0xBE);
    }
    uint8_t sp[9] = {};
    if (pr2) {
        for (uint8_t i = 0; i < 9u; ++i) {
            sp[i] = static_cast<uint8_t>(ow.read());
        }
    }
    interrupts();
    bus_release(pin);

    if (!pr2) {
        return false;
    }
    if (sp_out != nullptr) {
        std::memcpy(sp_out, sp, 9);
    }
    return OneWire::crc8(sp, 8) == sp[8];
}

bool decode_scratchpad(const uint8_t sp[9], float *t_c)
{
    if (t_c == nullptr) {
        return false;
    }
    const int16_t raw = static_cast<int16_t>((sp[1] << 8) | sp[0]);
    const float t_sp = static_cast<float>(raw) / 16.0f;
    if (!valid_temp_c(t_sp) || !scratchpad_plausible(sp, t_sp)) {
        return false;
    }
    *t_c = t_sp;
    return true;
}

bool read_temp_c_once(int pin, const uint8_t *rom, float *t_c, uint8_t sp_out[9])
{
    if (t_c == nullptr) {
        return false;
    }

    WiFi.setSleep(WIFI_PS_NONE);

    uint8_t sp[9] = {};
    if (!read_scratchpad_after_convert(pin, rom, sp)) {
        return false;
    }
    if (!decode_scratchpad(sp, t_c)) {
        return false;
    }
    if (sp_out != nullptr) {
        std::memcpy(sp_out, sp, 9);
    }
    return true;
}

bool read_temp_c_robust_rom(int pin, const uint8_t *rom, float *t_c, uint8_t sp_out[9])
{
    for (uint8_t attempt = 0; attempt < k_read_attempts; ++attempt) {
        if (attempt > 0u) {
            delay(40);
        }
        if (read_temp_c_once(pin, rom, t_c, sp_out)) {
            return true;
        }
    }
    return false;
}

bool rom_crc_ok(const uint8_t addr[8])
{
    if (addr[0] == 0u) {
        return false;
    }
    return OneWire::crc8(addr, 7) == addr[7];
}

void log_gpio_line_test(int pin)
{
    gpio_reset_pin(static_cast<gpio_num_t>(pin));
    pinMode(pin, INPUT);
    bus_release(pin);
    delay(5);
    const int idle = digitalRead(pin);

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(2);
    const int driven_low = digitalRead(pin);

    bus_prepare(pin);
    delay(2);
    const int od_high = digitalRead(pin);

    Serial.printf("[ds18] %s line idle=%d driven_low=%d od_release=%d\n",
                  pin_label(pin),
                  idle,
                  driven_low,
                  od_high);
}

void dallas_lib_probe(int pin)
{
    bus_prepare(pin);
    OneWire ow(pin);
    DallasTemperature dt(&ow);
    dt.begin();
    const int device_count = dt.getDeviceCount();
    const int ds18_count = dt.getDS18Count();
    Serial.printf("[ds18] %s Dallas count=%d ds18=%d\n",
                  pin_label(pin),
                  device_count,
                  ds18_count);

    if (device_count > 0) {
        dt.setResolution(12);
        dt.setWaitForConversion(false);
        dt.requestTemperatures();
        delay(k_convert_ms);
        for (int idx = 0; idx < device_count && idx < 4; ++idx) {
            const float t_c = dt.getTempCByIndex(static_cast<uint8_t>(idx));
            Serial.printf("[ds18] %s Dallas[%d] T=%.2f C\n", pin_label(pin), idx, t_c);
        }
    }
    bus_release(pin);
}

size_t enumerate_devices(int pin, DeviceAddress roms[], size_t max_roms)
{
    if (roms == nullptr || max_roms == 0u) {
        return 0;
    }

    bus_prepare(pin);
    OneWire ow(pin);
    size_t count = 0;

    for (uint8_t attempt = 0; attempt < 3u; ++attempt) {
        ow.reset_search();
        delay(25);
        count = 0;
        while (count < max_roms) {
            DeviceAddress addr{};
            if (!ow.search(addr)) {
                break;
            }
            if (!rom_crc_ok(addr)) {
                continue;
            }
            std::memcpy(roms[count], addr, sizeof(DeviceAddress));
            ++count;
        }
        if (count > 0u) {
            break;
        }
    }

    bus_release(pin);
    return count;
}

bool read_temp_c_robust(int pin, float *t_c, uint8_t sp_out[9])
{
    DeviceAddress roms[k_max_devices];
    const size_t device_count = enumerate_devices(pin, roms, k_max_devices);

    if (device_count > 0u) {
        for (size_t i = 0; i < device_count; ++i) {
            float t_try = DEVICE_DISCONNECTED_C;
            uint8_t sp_try[9] = {};
            if (!read_temp_c_robust_rom(pin, roms[i], &t_try, sp_try)) {
                continue;
            }
            if (t_c != nullptr) {
                *t_c = t_try;
            }
            if (sp_out != nullptr) {
                std::memcpy(sp_out, sp_try, 9);
            }
            std::memcpy(g_rom, roms[i], sizeof(DeviceAddress));
            g_have_rom = true;
            return true;
        }
        return false;
    }

    for (uint8_t attempt = 0; attempt < k_read_attempts; ++attempt) {
        if (attempt > 0u) {
            delay(40);
        }
        if (read_temp_c_once(pin, nullptr, t_c, sp_out)) {
            return true;
        }
    }
    return false;
}

bool read_rom_single(int pin, uint8_t rom_out[8])
{
    bus_prepare(pin);
    OneWire ow(pin);

    noInterrupts();
    const bool pr = ow.reset();
    uint8_t rom[8] = {};
    if (pr) {
        ow.write(0x33);
        for (uint8_t i = 0; i < 8u; ++i) {
            rom[i] = static_cast<uint8_t>(ow.read());
        }
    }
    interrupts();
    bus_release(pin);

    if (!pr) {
        return false;
    }
    if (rom_out != nullptr) {
        std::memcpy(rom_out, rom, 8);
    }
    return rom_crc_ok(rom);
}

void discover_rom(int pin)
{
    g_have_rom = false;
    std::memset(g_rom, 0, sizeof(g_rom));
    bus_prepare(pin);
    OneWire ow(pin);

    for (uint8_t attempt = 0; attempt < 20u; ++attempt) {
        ow.reset_search();
        delay(20);
        if (!ow.search(g_rom)) {
            continue;
        }
        if (!rom_crc_ok(g_rom)) {
            continue;
        }
        g_have_rom = true;
        break;
    }
    bus_release(pin);
}

void log_probe(int pin)
{
    bus_release(pin);
    delay(5);
    const int idle = digitalRead(pin);
    const bool presence = ow_presence(pin);
    Serial.printf("[ds18] %s idle=%d presence=%d\n",
                  pin_label(pin),
                  idle,
                  presence ? 1 : 0);
}

bool probe_pin(int pin, float *t_c_out, uint8_t sp_out[9], bool verbose)
{
    if (verbose) {
        log_probe(pin);
    }
    float t_c = DEVICE_DISCONNECTED_C;
    uint8_t sp[9] = {};
    DeviceAddress roms[k_max_devices];
    const size_t device_count = enumerate_devices(pin, roms, k_max_devices);
    if (verbose) {
        Serial.printf("[ds18] %s ROM count=%u\n", pin_label(pin), static_cast<unsigned>(device_count));
    }

    if (device_count > 0u) {
        for (size_t i = 0; i < device_count; ++i) {
            if (verbose) {
                Serial.printf("[ds18] %s ROM[%u]=", pin_label(pin), static_cast<unsigned>(i));
                log_rom(roms[i]);
                Serial.printf(" fam=%02x\n", roms[i][0]);
            }
            if (read_temp_c_robust_rom(pin, roms[i], &t_c, sp)) {
                std::memcpy(g_rom, roms[i], sizeof(DeviceAddress));
                g_have_rom = true;
                break;
            }
            if (verbose) {
                uint8_t sp_dbg[9] = {};
                (void)read_scratchpad_after_convert(pin, roms[i], sp_dbg);
                Serial.printf("[ds18] %s ROM[%u] fail sp=%02x %02x %02x crc=%02x\n",
                              pin_label(pin),
                              static_cast<unsigned>(i),
                              sp_dbg[0],
                              sp_dbg[1],
                              sp_dbg[2],
                              sp_dbg[8]);
            }
        }
    } else if (!read_temp_c_robust(pin, &t_c, sp)) {
        if (verbose) {
            uint8_t sp_dbg[9] = {};
            (void)read_scratchpad_after_convert(pin, nullptr, sp_dbg);
            Serial.printf("[ds18] %s skip-ROM fail sp=%02x %02x %02x crc=%02x\n",
                          pin_label(pin),
                          sp_dbg[0],
                          sp_dbg[1],
                          sp_dbg[2],
                          sp_dbg[8]);
        }
        return false;
    }

    if (!valid_temp_c(t_c)) {
        return false;
    }
    if (t_c_out != nullptr) {
        *t_c_out = t_c;
    }
    if (sp_out != nullptr) {
        std::memcpy(sp_out, sp, 9);
    }
    if (verbose) {
        Serial.printf("[ds18] %s OK T=%.1f C\n", pin_label(pin), t_c);
    }
    return true;
}

void apply_live_reading(float t_c, const uint8_t sp[9])
{
    g_fail_streak = 0;
    g_live = true;
    g_last_ok_ms = millis();
    g_temp_c_x10 = float_c_to_x10(t_c);

    if (!g_logged_ok) {
        g_logged_ok = true;
        Serial.printf("[ds18] LIVE on %s\n", pin_label(g_active_gpio));
    }
    Serial.printf("[ds18] T=%.1f C raw=%02x%02x (%s)\n",
                  t_c,
                  sp[1],
                  sp[0],
                  pin_label(g_active_gpio));
}

bool select_working_pin(bool verbose)
{
    for (size_t i = 0; i < k_candidate_count; ++i) {
        const int pin = k_candidate_pins[i];
        float t_c = DEVICE_DISCONNECTED_C;
        uint8_t sp[9] = {};
        if (!probe_pin(pin, &t_c, sp, verbose)) {
            continue;
        }
        bind_pin(pin);
        discover_rom(pin);
        apply_live_reading(t_c, sp);
        Serial.printf("[ds18] active bus: %s (GPIO%d)\n", pin_label(pin), pin);
        return true;
    }
    g_active_gpio = PIN_DS18_DATA;
    bind_pin(PIN_DS18_DATA);
    if (verbose) {
        Serial.println(F("[ds18] no valid read on CN4"));
    }
    return false;
}

}  // namespace

void mb_outdoor_sensor_begin()
{
    g_last_poll_ms = 0;
    g_last_ok_ms = 0;
    g_last_pin_scan_ms = 0;
    g_fail_streak = 0;
    g_live = false;
    g_logged_ok = false;

    WiFi.setSleep(WIFI_PS_NONE);
    delay(500);
    Serial.println(F("[ds18] CN4 real sensor (GPIO4)"));
    bind_pin(PIN_DS18_DATA);
    (void)select_working_pin(true);
}

void mb_outdoor_sensor_poll()
{
    const uint32_t now = millis();
    if (g_last_poll_ms != 0u && (now - g_last_poll_ms) < k_poll_interval_ms) {
        if (g_live && g_last_ok_ms != 0u && (now - g_last_ok_ms) > k_stale_after_ms) {
            g_live = false;
            Serial.println(F("[ds18] stale — rescanning"));
        }
        return;
    }
    g_last_poll_ms = now;

    uint8_t sp[9] = {};
    float t_c = DEVICE_DISCONNECTED_C;
    bool ok = false;
    if (g_have_rom) {
        ok = read_temp_c_robust_rom(g_active_gpio, g_rom, &t_c, sp);
    }
    if (!ok) {
        ok = read_temp_c_robust(g_active_gpio, &t_c, sp);
    }
    if (!ok) {
        if (g_fail_streak < 255u) {
            ++g_fail_streak;
        }
        if (g_fail_streak >= k_fail_before_stale) {
            g_live = false;
        }

        if (now - g_last_diag_ms >= 10000u) {
            g_last_diag_ms = now;
            log_probe(g_active_gpio);
            discover_rom(g_active_gpio);
        }

        if (g_fail_streak >= 4u &&
            (g_last_pin_scan_ms == 0u || (now - g_last_pin_scan_ms) >= k_rescan_other_pin_ms)) {
            g_last_pin_scan_ms = now;
            for (size_t i = 0; i < k_candidate_count; ++i) {
                const int pin = k_candidate_pins[i];
                if (pin == g_active_gpio) {
                    continue;
                }
                float t_alt = DEVICE_DISCONNECTED_C;
                uint8_t sp_alt[9] = {};
                if (probe_pin(pin, &t_alt, sp_alt, true)) {
                    bind_pin(pin);
                    apply_live_reading(t_alt, sp_alt);
                    return;
                }
            }
        }
        return;
    }

    apply_live_reading(t_c, sp);
}

bool mb_outdoor_sensor_is_link_test()
{
    return false;
}

bool mb_outdoor_sensor_has_live()
{
    return g_live;
}

int16_t mb_outdoor_sensor_temp_c_x10()
{
    return g_temp_c_x10;
}

bool mb_outdoor_sensor_has_humidity()
{
    return false;
}

void mb_outdoor_sensor_force_poll()
{
    g_last_poll_ms = 0;
    mb_outdoor_sensor_poll();
}

int mb_outdoor_sensor_active_gpio()
{
    return g_active_gpio;
}

void mb_outdoor_sensor_boot_probe_no_wifi()
{
    Serial.println(F("[ds18] pre-WiFi probe..."));
    WiFi.mode(WIFI_OFF);
    delay(100);
    if (select_working_pin(true)) {
        Serial.println(F("[ds18] pre-WiFi: sensor OK"));
    } else {
        Serial.println(F("[ds18] pre-WiFi: no read yet"));
    }
}

void mb_outdoor_sensor_run_diag()
{
    Serial.println(F("[ds18] === FULL DIAG ==="));
    WiFi.setSleep(WIFI_PS_NONE);

    for (size_t i = 0; i < k_candidate_count; ++i) {
        const int pin = k_candidate_pins[i];
        log_probe(pin);
        log_gpio_line_test(pin);
        dallas_lib_probe(pin);

        DeviceAddress roms[k_max_devices];
        const size_t n = enumerate_devices(pin, roms, k_max_devices);
        Serial.printf("[ds18] %s devices=%u\n", pin_label(pin), static_cast<unsigned>(n));

        if (n == 0u) {
            uint8_t rom[8] = {};
            bus_prepare(pin);
            OneWire ow(pin);
            const bool pr = ow.reset();
            if (pr) {
                ow.write(0x33);
                for (uint8_t i = 0; i < 8u; ++i) {
                    rom[i] = static_cast<uint8_t>(ow.read());
                }
            }
            bus_release(pin);
            Serial.printf("[ds18] %s READ-ROM=", pin_label(pin));
            log_rom(rom);
            Serial.printf(" crc_ok=%d fam=%02x\n",
                          rom_crc_ok(rom) ? 1 : 0,
                          rom[0]);

            float t_c = DEVICE_DISCONNECTED_C;
            uint8_t sp[9] = {};
            if (rom_crc_ok(rom) && read_temp_c_robust_rom(pin, rom, &t_c, sp)) {
                Serial.printf("[ds18] %s match-ROM OK T=%.1f C\n", pin_label(pin), t_c);
            } else if (read_temp_c_robust_rom(pin, nullptr, &t_c, sp)) {
                Serial.printf("[ds18] %s skip-ROM OK T=%.1f C\n", pin_label(pin), t_c);
            } else {
                uint8_t sp_dbg[9] = {};
                (void)read_scratchpad_after_convert(pin, nullptr, sp_dbg);
                Serial.printf("[ds18] %s skip-ROM fail sp=%02x %02x %02x crc=%02x\n",
                              pin_label(pin),
                              sp_dbg[0],
                              sp_dbg[1],
                              sp_dbg[2],
                              sp_dbg[8]);
            }
            continue;
        }

        for (size_t d = 0; d < n; ++d) {
            Serial.printf("[ds18] %s dev[%u] ROM=", pin_label(pin), static_cast<unsigned>(d));
            log_rom(roms[d]);
            Serial.printf(" fam=%02x\n", roms[d][0]);

            float t_c = DEVICE_DISCONNECTED_C;
            uint8_t sp[9] = {};
            if (read_temp_c_robust_rom(pin, roms[d], &t_c, sp)) {
                Serial.printf("[ds18] %s dev[%u] OK T=%.1f C\n",
                              pin_label(pin),
                              static_cast<unsigned>(d),
                              t_c);
            } else {
                uint8_t sp_dbg[9] = {};
                (void)read_scratchpad_after_convert(pin, roms[d], sp_dbg);
                Serial.printf("[ds18] %s dev[%u] fail sp=%02x %02x %02x crc=%02x\n",
                              pin_label(pin),
                              static_cast<unsigned>(d),
                              sp_dbg[0],
                              sp_dbg[1],
                              sp_dbg[2],
                              sp_dbg[8]);
            }
        }
    }

    Serial.println(F("[ds18] --- bypass net (GPIO2/D2, not CN4) ---"));
    {
        const int pin = k_bypass_diag_pin;
        log_probe(pin);
        log_gpio_line_test(pin);
        dallas_lib_probe(pin);
        float t_c = DEVICE_DISCONNECTED_C;
        uint8_t sp[9] = {};
        if (probe_pin(pin, &t_c, sp, true)) {
            Serial.printf("[ds18] %s BYPASS SUCCESS T=%.1f C\n", pin_label(pin), t_c);
        }
    }

    Serial.println(F("[ds18] === END DIAG ==="));
    g_last_poll_ms = 0;
    g_logged_ok = false;
    (void)select_working_pin(true);
    mb_outdoor_sensor_poll();
}
