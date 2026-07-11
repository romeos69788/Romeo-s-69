#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_err.h"

#include "mb_mic.h"
#include "pin_map.h"

namespace {

constexpr uint32_t k_sample_rate = 16000u;
constexpr size_t k_raw_count = 128u;
constexpr uint32_t k_poll_ms = 800u;
constexpr uint32_t k_hear_hold_ms = 4000u;
constexpr uint32_t k_cmd_hold_ms = 15000u;
constexpr int32_t k_hear_peak = 500;
constexpr int32_t k_signal_peak = 30;

i2s_chan_handle_t g_rx_chan = nullptr;

struct ActiveCfg {
    int8_t bclk;
    int8_t ws;
    int8_t sd;
    i2s_slot_mode_t slot_mode;
    i2s_std_slot_mask_t slot_mask;
    uint8_t shift;
    bool philips;
};

ActiveCfg g_cfg = {
    PIN_MIC_I2S_BCLK,
    PIN_MIC_I2S_WS,
    PIN_MIC_I2S_SD,
    I2S_SLOT_MODE_MONO,
    I2S_STD_SLOT_LEFT,
    14u,
    true,
};

bool g_begin_ok = false;
bool g_has_signal = false;
uint32_t g_next_poll_ms = 0;
uint32_t g_hear_until_ms = 0;
uint32_t g_cmd_until_ms = 0;
int32_t g_last_rms = 0;
int32_t g_last_peak = 0;
char g_cmd_text[21] = {};

void set_cmd_text(const char *text)
{
    std::snprintf(g_cmd_text, sizeof(g_cmd_text), "%.20s", text ? text : "");
    g_cmd_until_ms = millis() + k_cmd_hold_ms;
}

void stop_i2s()
{
    if (g_rx_chan == nullptr) {
        return;
    }
    i2s_channel_disable(g_rx_chan);
    i2s_del_channel(g_rx_chan);
    g_rx_chan = nullptr;
}

/** Μετά I2S: WS/SD/BCLK → OUTPUT HIGH (K2/K3 LED σβηστά). */
void release_mic_pins()
{
    stop_i2s();
    const int pins[] = {PIN_MIC_I2S_BCLK, PIN_MIC_I2S_WS, PIN_MIC_I2S_SD};
    for (int p : pins) {
        gpio_reset_pin(static_cast<gpio_num_t>(p));
        pinMode(p, OUTPUT);
        digitalWrite(p, HIGH);
    }
}

bool start_cfg(const ActiveCfg &cfg);
bool read_stats(int32_t *rms_out, int32_t *peak_out, int32_t *raw0_out);

bool sample_burst(int32_t *rms_out, int32_t *peak_out, int32_t *raw0_out)
{
    if (!start_cfg(g_cfg)) {
        release_mic_pins();
        return false;
    }
    const bool ok = read_stats(rms_out, peak_out, raw0_out);
    release_mic_pins();
    return ok;
}

i2s_std_slot_config_t slot_cfg_for(const ActiveCfg &cfg)
{
    i2s_std_slot_config_t slot;
    if (cfg.philips) {
        slot = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, cfg.slot_mode);
    } else {
        slot = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, cfg.slot_mode);
    }
    slot.slot_mask = cfg.slot_mask;
    return slot;
}

bool start_cfg(const ActiveCfg &cfg)
{
    stop_i2s();
    delay(40);

    for (int pin : {cfg.bclk, cfg.ws, cfg.sd}) {
        gpio_reset_pin(static_cast<gpio_num_t>(pin));
    }
    delay(10);

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 8;
    chan_cfg.dma_frame_num = 512;

    if (i2s_new_channel(&chan_cfg, nullptr, &g_rx_chan) != ESP_OK) {
        Serial.println(F("[mic] i2s_new_channel FAIL"));
        return false;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(k_sample_rate),
        .slot_cfg = slot_cfg_for(cfg),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = static_cast<gpio_num_t>(cfg.bclk),
            .ws = static_cast<gpio_num_t>(cfg.ws),
            .dout = GPIO_NUM_NC,
            .din = static_cast<gpio_num_t>(cfg.sd),
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    if (i2s_channel_init_std_mode(g_rx_chan, &std_cfg) != ESP_OK) {
        Serial.println(F("[mic] init_std_mode FAIL"));
        stop_i2s();
        return false;
    }
    if (i2s_channel_enable(g_rx_chan) != ESP_OK) {
        Serial.println(F("[mic] channel_enable FAIL"));
        stop_i2s();
        return false;
    }

    delay(80);
    int32_t trash[k_raw_count];
    size_t got = 0;
    i2s_channel_read(g_rx_chan, trash, sizeof(trash), &got, 200);
    return true;
}

bool start_active()
{
    return start_cfg(g_cfg);
}

int32_t peak_from_raw(const int32_t *raw, size_t n, uint8_t shift)
{
    int32_t peak = 0;
    for (size_t i = 0; i < n; ++i) {
        const int32_t s = static_cast<int32_t>(raw[i] >> shift);
        const int32_t a = (s < 0) ? -s : s;
        if (a > peak) {
            peak = a;
        }
    }
    return peak;
}

int32_t best_peak_from_raw(const int32_t *raw, size_t n, uint8_t *shift_out)
{
    static const uint8_t k_shifts[] = {8u, 10u, 11u, 12u, 14u, 16u};
    int32_t best = 0;
    uint8_t best_shift = 14u;
    for (uint8_t sh : k_shifts) {
        const int32_t p = peak_from_raw(raw, n, sh);
        if (p > best) {
            best = p;
            best_shift = sh;
        }
    }
    if (shift_out != nullptr) {
        *shift_out = best_shift;
    }
    return best;
}

bool read_raw(int32_t *raw, size_t cap, size_t *n_out)
{
    if (g_rx_chan == nullptr || raw == nullptr || cap == 0u) {
        return false;
    }
    size_t got = 0;
    const esp_err_t err = i2s_channel_read(g_rx_chan,
                                           raw,
                                           cap * sizeof(int32_t),
                                           &got,
                                           200);
    if (err != ESP_OK || got < sizeof(int32_t)) {
        return false;
    }
    if (n_out != nullptr) {
        *n_out = got / sizeof(int32_t);
    }
    return true;
}

bool read_stats(int32_t *rms_out, int32_t *peak_out, int32_t *raw0_out)
{
    int32_t raw[k_raw_count];
    size_t n = 0;
    if (!read_raw(raw, k_raw_count, &n)) {
        return false;
    }

    uint64_t sum_sq = 0;
    int32_t peak = 0;

    for (size_t i = 0; i < n; ++i) {
        const int32_t s = static_cast<int32_t>(raw[i] >> g_cfg.shift);
        const int32_t a = (s < 0) ? -s : s;
        sum_sq += static_cast<uint64_t>(a) * static_cast<uint64_t>(a);
        if (a > peak) {
            peak = a;
        }
    }

    const int32_t rms = static_cast<int32_t>(std::sqrt(static_cast<double>(sum_sq) / static_cast<double>(n)));
    if (rms_out != nullptr) {
        *rms_out = rms;
    }
    if (peak_out != nullptr) {
        *peak_out = peak;
    }
    if (raw0_out != nullptr && n > 0u) {
        *raw0_out = raw[0];
    }
    return true;
}

int32_t measure_peak(const ActiveCfg &cfg, uint8_t *shift_out)
{
    if (!start_cfg(cfg)) {
        return -1;
    }

    int32_t best = 0;
    uint8_t best_shift = 14u;
    for (uint8_t pass = 0; pass < 5u; ++pass) {
        int32_t raw[k_raw_count];
        size_t n = 0;
        if (!read_raw(raw, k_raw_count, &n)) {
            continue;
        }
        uint8_t sh = 14u;
        const int32_t p = best_peak_from_raw(raw, n, &sh);
        if (p > best) {
            best = p;
            best_shift = sh;
        }
        delay(30);
    }

    if (shift_out != nullptr) {
        *shift_out = best_shift;
    }
    release_mic_pins();
    return best;
}

int32_t hunt_gpio_pins(bool log_all)
{
    struct PinTriplet {
        int8_t b;
        int8_t w;
        int8_t s;
        const char *tag;
    };

    const PinTriplet pins[] = {
        {12, 26, 27, "PCB-B12"},
        {12, 27, 26, "SWAP-2627"},
        {13, 27, 26, "PCB-B13a"},
        {13, 26, 27, "PCB-B13b"},
        {2, 12, 0, "OLD-2-12-0"},
        {2, 26, 27, "B2-2627"},
        {0, 12, 26, "TRY-0-12-26"},
    };

    int32_t best_peak = 0;
    ActiveCfg best_cfg = g_cfg;
    uint8_t best_shift = 14u;
    const char *best_tag = "PCB-B12";

    for (const PinTriplet &pt : pins) {
        ActiveCfg cfg = g_cfg;
        cfg.bclk = pt.b;
        cfg.ws = pt.w;
        cfg.sd = pt.s;
        uint8_t sh = 14u;
        const int32_t p = measure_peak(cfg, &sh);
        if (log_all) {
            Serial.printf("[mic] hunt %s B=%d W=%d S=%d pk=%ld\n",
                          pt.tag,
                          static_cast<int>(pt.b),
                          static_cast<int>(pt.w),
                          static_cast<int>(pt.s),
                          static_cast<long>(p));
        }
        if (p > best_peak) {
            best_peak = p;
            best_cfg = cfg;
            best_shift = sh;
            best_tag = pt.tag;
        }
    }

    g_cfg = best_cfg;
    g_cfg.shift = best_shift;
    if (log_all) {
        Serial.printf("[mic] hunt pick %s peak=%ld B=%d W=%d S=%d\n",
                      best_tag,
                      static_cast<long>(best_peak),
                      static_cast<int>(g_cfg.bclk),
                      static_cast<int>(g_cfg.ws),
                      static_cast<int>(g_cfg.sd));
    }
    return best_peak;
}

int32_t scan_and_select(bool log_all)
{
    struct Candidate {
        ActiveCfg cfg;
        const char *tag;
    };

    const Candidate list[] = {
        {{g_cfg.bclk, g_cfg.ws, g_cfg.sd, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT, 14u, true}, "PHIL-L"},
        {{g_cfg.bclk, g_cfg.ws, g_cfg.sd, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_RIGHT, 14u, true}, "PHIL-R"},
        {{g_cfg.bclk, g_cfg.ws, g_cfg.sd, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH, 14u, true}, "PHIL-STD"},
    };

    int32_t best_peak = 0;
    ActiveCfg best_cfg = g_cfg;
    uint8_t best_shift = g_cfg.shift;
    const char *best_tag = "PHIL-L";

    for (const Candidate &c : list) {
        uint8_t sh = 14u;
        const int32_t p = measure_peak(c.cfg, &sh);
        if (log_all) {
            Serial.printf("[mic] probe %s B=%d W=%d S=%d pk=%ld sh=%u\n",
                          c.tag,
                          static_cast<int>(c.cfg.bclk),
                          static_cast<int>(c.cfg.ws),
                          static_cast<int>(c.cfg.sd),
                          static_cast<long>(p),
                          static_cast<unsigned>(sh));
        }
        if (p > best_peak) {
            best_peak = p;
            best_cfg = c.cfg;
            best_shift = sh;
            best_tag = c.tag;
        }
    }

    g_cfg = best_cfg;
    g_cfg.shift = best_shift;
    release_mic_pins();

    if (log_all) {
        Serial.printf("[mic] pick %s peak=%ld shift=%u\n",
                      best_tag,
                      static_cast<long>(best_peak),
                      static_cast<unsigned>(best_shift));
    }
    return best_peak;
}

void note_levels(int32_t rms, int32_t peak)
{
    g_last_rms = rms;
    g_last_peak = peak;

    if (peak > k_signal_peak) {
        g_has_signal = true;
    }

    const uint32_t now = millis();
    if (peak >= k_hear_peak) {
        g_hear_until_ms = now + k_hear_hold_ms;
    }
}

void log_pin_idle()
{
    pinMode(PIN_MIC_I2S_BCLK, INPUT);
    pinMode(PIN_MIC_I2S_WS, INPUT);
    pinMode(PIN_MIC_I2S_SD, INPUT);
    delay(5);
    Serial.printf("[mic] idle pins B=%d W=%d S=%d\n",
                  digitalRead(PIN_MIC_I2S_BCLK),
                  digitalRead(PIN_MIC_I2S_WS),
                  digitalRead(PIN_MIC_I2S_SD));
}

void run_probe(bool log_all)
{
    Serial.println(F("[mic] === MIC PROBE (clap/speak now) ==="));
    log_pin_idle();
    hunt_gpio_pins(log_all);
    const int32_t peak = scan_and_select(log_all);
    g_begin_ok = (g_rx_chan != nullptr);
    g_has_signal = g_begin_ok && (peak > k_signal_peak);
    g_next_poll_ms = millis();

    Serial.printf("[mic] active B=%d W=%d S=%d peak=%ld signal=%d\n",
                  static_cast<int>(g_cfg.bclk),
                  static_cast<int>(g_cfg.ws),
                  static_cast<int>(g_cfg.sd),
                  static_cast<long>(peak),
                  g_has_signal ? 1 : 0);

    if (!g_has_signal) {
        Serial.println(F("[mic] SD stuck? check INMP441 L/R -> CN1-6 GND"));
        Serial.printf("[mic] PCB v1.0: B=%d W=%d S=%d (K2/K3 LED may blink)\n",
                      PIN_MIC_I2S_BCLK, PIN_MIC_I2S_WS, PIN_MIC_I2S_SD);
    }
}

}  // namespace

void mb_mic_begin()
{
    g_begin_ok = true;
    g_has_signal = false;
    g_next_poll_ms = millis();
    release_mic_pins();
    log_pin_idle();
    Serial.println(F("[mic] GPIO hunt at boot..."));
    const int32_t hunted = hunt_gpio_pins(true);
    (void)hunted;

    int32_t rms = 0;
    int32_t peak = 0;
    int32_t raw0 = 0;
    if (sample_burst(&rms, &peak, &raw0)) {
        note_levels(rms, peak);
        Serial.printf("[mic] B=%d W=%d S=%d peak=%ld raw0=0x%08lX\n",
                      static_cast<int>(g_cfg.bclk),
                      static_cast<int>(g_cfg.ws),
                      static_cast<int>(g_cfg.sd),
                      static_cast<long>(peak),
                      static_cast<unsigned long>(static_cast<uint32_t>(raw0)));
    }
    if (!g_has_signal) {
        Serial.println(F("[mic] all GPIO combos pk=1 -> try 2nd INMP441"));
    }
}

void mb_mic_poll()
{
    if (!g_begin_ok) {
        return;
    }

    const uint32_t now = millis();
    if (now < g_next_poll_ms) {
        return;
    }
    g_next_poll_ms = now + k_poll_ms;

    int32_t rms = 0;
    int32_t peak = 0;
    if (!sample_burst(&rms, &peak, nullptr)) {
        return;
    }
    note_levels(rms, peak);
}

bool mb_mic_begin_ok()
{
    return g_begin_ok;
}

bool mb_mic_has_signal()
{
    return g_has_signal;
}

void mb_mic_format_lcd_line(char *buf, size_t len)
{
    if (buf == nullptr || len == 0u) {
        return;
    }

    if (!g_begin_ok) {
        std::snprintf(buf, len, "MIC I2S FAIL");
        return;
    }

    const uint32_t now = millis();
    if (g_cmd_text[0] != '\0' && now < g_cmd_until_ms) {
        std::snprintf(buf, len, "CMD: %s", g_cmd_text);
        return;
    }

    if (now < g_hear_until_ms) {
        std::snprintf(buf, len, "MIC HEAR pk=%ld", static_cast<long>(g_last_peak));
        return;
    }

    if (!g_has_signal) {
        std::snprintf(buf, len, "MIC pk=1 try 2nd mod");
        return;
    }

    std::snprintf(buf, len, "MIC ok rms=%ld", static_cast<long>(g_last_rms));
}

void mb_mic_run_diag()
{
    run_probe(true);
    release_mic_pins();

    int32_t rms = 0;
    int32_t peak = 0;
    int32_t raw0 = 0;
    if (read_stats(&rms, &peak, &raw0)) {
        note_levels(rms, peak);
        Serial.printf("[mic] now rms=%ld peak=%ld raw0=%ld\n",
                      static_cast<long>(rms),
                      static_cast<long>(peak),
                      static_cast<long>(raw0));
    }

    char lcd[21];
    mb_mic_format_lcd_line(lcd, sizeof(lcd));
    Serial.printf("[mic] LCD L3: %s\n", lcd);
}

bool mb_mic_handle_serial(const char *line)
{
    if (line == nullptr) {
        return false;
    }
    if (strcmp(line, "MIC DIAG") == 0 || strcmp(line, "MIC PROBE") == 0) {
        mb_mic_run_diag();
        return true;
    }
    if (strncmp(line, "MIC CMD ", 8) == 0) {
        const char *text = line + 8;
        while (*text == ' ') {
            ++text;
        }
        if (*text == '\0') {
            Serial.println(F("[mic] usage: MIC CMD <text>"));
            return true;
        }
        set_cmd_text(text);
        g_hear_until_ms = millis() + k_hear_hold_ms;
        Serial.printf("[mic] CMD ok -> LCD: %s\n", g_cmd_text);
        return true;
    }
    if (strcmp(line, "MIC HELP") == 0) {
        Serial.println(F("[mic] MIC PROBE  (speak during scan)"));
        Serial.println(F("[mic] MIC CMD <text>"));
        return true;
    }
    return false;
}
