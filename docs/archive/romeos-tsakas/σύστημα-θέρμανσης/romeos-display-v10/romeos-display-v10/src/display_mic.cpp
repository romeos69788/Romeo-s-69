#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_err.h"

#include "display_mic.h"
#include "display_mic_pins.h"

namespace {

constexpr uint32_t k_sample_rate = 16000u;
constexpr size_t k_raw_count = 128u;
constexpr uint32_t k_poll_ms = 200u;
constexpr int32_t k_spread_speech = 800;
constexpr int32_t k_spread_quiet = 400;
constexpr uint32_t k_min_speech_ms = 120u;
constexpr uint32_t k_menu_max_ms = 650u;
constexpr uint32_t k_cmd_cooldown_ms = 1800u;

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
    PIN_DISPLAY_MIC_BCLK,
    PIN_DISPLAY_MIC_WS,
    PIN_DISPLAY_MIC_SD,
    I2S_SLOT_MODE_MONO,
    I2S_STD_SLOT_LEFT,
    14u,
    true,
};

bool g_begin_ok = false;
bool g_listening = false;
bool g_has_signal = false;
uint32_t g_next_poll_ms = 0;
int32_t g_last_rms = 0;
int32_t g_last_peak = 0;
int32_t g_last_spread = 0;

bool g_in_speech = false;
uint32_t g_speech_start_ms = 0;
uint32_t g_cmd_cooldown_until = 0;

display_mic_voice_fn g_voice_fn = nullptr;

void stop_i2s()
{
    if (g_rx_chan == nullptr) {
        return;
    }
    i2s_channel_disable(g_rx_chan);
    i2s_del_channel(g_rx_chan);
    g_rx_chan = nullptr;
}

void release_mic_pins()
{
    stop_i2s();
    for (int p : {PIN_DISPLAY_MIC_BCLK, PIN_DISPLAY_MIC_WS, PIN_DISPLAY_MIC_SD}) {
        gpio_reset_pin(static_cast<gpio_num_t>(p));
    }
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
    delay(20);

    for (int pin : {cfg.bclk, cfg.ws, cfg.sd}) {
        gpio_reset_pin(static_cast<gpio_num_t>(pin));
    }

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 6;
    chan_cfg.dma_frame_num = 256;

    if (i2s_new_channel(&chan_cfg, nullptr, &g_rx_chan) != ESP_OK) {
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
        stop_i2s();
        return false;
    }
    if (i2s_channel_enable(g_rx_chan) != ESP_OK) {
        stop_i2s();
        return false;
    }

    delay(40);
    int32_t trash[k_raw_count];
    size_t got = 0;
    i2s_channel_read(g_rx_chan, trash, sizeof(trash), &got, 150);
    return true;
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
                                           150);
    if (err != ESP_OK || got < sizeof(int32_t)) {
        return false;
    }
    if (n_out != nullptr) {
        *n_out = got / sizeof(int32_t);
    }
    return true;
}

int32_t sample_spread(const int32_t *raw, size_t n, uint8_t shift)
{
    int32_t mn = 0;
    int32_t mx = 0;
    for (size_t i = 0; i < n; ++i) {
        const int32_t s = static_cast<int32_t>(raw[i] >> shift);
        if (i == 0u || s < mn) {
            mn = s;
        }
        if (i == 0u || s > mx) {
            mx = s;
        }
    }
    return mx - mn;
}

int32_t sample_peak(const int32_t *raw, size_t n, uint8_t shift)
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

int32_t sample_rms(const int32_t *raw, size_t n, uint8_t shift)
{
    uint64_t sum_sq = 0;
    for (size_t i = 0; i < n; ++i) {
        const int32_t s = static_cast<int32_t>(raw[i] >> shift);
        sum_sq += static_cast<uint64_t>(s) * static_cast<uint64_t>(s);
    }
    return static_cast<int32_t>(std::sqrt(static_cast<double>(sum_sq) / static_cast<double>(n)));
}

bool read_stats(int32_t *rms_out, int32_t *peak_out, int32_t *spread_out, int32_t *raw0_out)
{
    int32_t raw[k_raw_count];
    size_t n = 0;
    if (!read_raw(raw, k_raw_count, &n)) {
        return false;
    }

    const int32_t peak = sample_peak(raw, n, g_cfg.shift);
    const int32_t rms = sample_rms(raw, n, g_cfg.shift);
    const int32_t spread = sample_spread(raw, n, g_cfg.shift);

    if (rms_out != nullptr) {
        *rms_out = rms;
    }
    if (peak_out != nullptr) {
        *peak_out = peak;
    }
    if (spread_out != nullptr) {
        *spread_out = spread;
    }
    if (raw0_out != nullptr && n > 0u) {
        *raw0_out = raw[0];
    }
    return true;
}

bool sample_burst(int32_t *rms_out, int32_t *peak_out, int32_t *spread_out, int32_t *raw0_out)
{
    if (!start_cfg(g_cfg)) {
        release_mic_pins();
        return false;
    }
    const bool ok = read_stats(rms_out, peak_out, spread_out, raw0_out);
    release_mic_pins();
    return ok;
}

int32_t measure_spread(const ActiveCfg &cfg)
{
    if (!start_cfg(cfg)) {
        release_mic_pins();
        return -1;
    }
    int32_t raw[k_raw_count];
    size_t n = 0;
    if (!read_raw(raw, k_raw_count, &n)) {
        release_mic_pins();
        return -1;
    }
    const int32_t spread = sample_spread(raw, n, cfg.shift);
    release_mic_pins();
    return spread;
}

void pick_best_slot()
{
    struct Candidate {
        i2s_std_slot_mask_t mask;
        const char *tag;
    };
    const Candidate cands[] = {
        {I2S_STD_SLOT_LEFT, "LEFT"},
        {I2S_STD_SLOT_RIGHT, "RIGHT"},
    };

    int32_t best = -1;
    ActiveCfg best_cfg = g_cfg;
    const char *best_tag = "LEFT";

    for (const Candidate &c : cands) {
        ActiveCfg cfg = g_cfg;
        cfg.slot_mask = c.mask;
        const int32_t sp = measure_spread(cfg);
        Serial.printf("[display_mic] slot %s spread=%ld\n", c.tag, static_cast<long>(sp));
        if (sp > best) {
            best = sp;
            best_cfg = cfg;
            best_tag = c.tag;
        }
        delay(20);
    }

    g_cfg = best_cfg;
    Serial.printf("[display_mic] slot pick %s spread=%ld\n", best_tag, static_cast<long>(best));
}

void fire_voice_cmd(const char *cmd)
{
    if (cmd == nullptr || g_voice_fn == nullptr) {
        return;
    }
    const uint32_t now = millis();
    if (now < g_cmd_cooldown_until) {
        return;
    }
    g_cmd_cooldown_until = now + k_cmd_cooldown_ms;
    Serial.printf("[display_mic] VOICE -> %s\n", cmd);
    g_voice_fn(cmd);
}

void process_speech_end(uint32_t now)
{
    if (!g_in_speech) {
        return;
    }
    const uint32_t dur = now - g_speech_start_ms;
    g_in_speech = false;

    if (dur < k_min_speech_ms) {
        Serial.printf("[display_mic] speech too short (%lums)\n", static_cast<unsigned long>(dur));
        return;
    }

    const char *cmd = (dur <= k_menu_max_ms) ? "menu" : "heat";
    Serial.printf("[display_mic] speech %lums spread=%ld -> %s\n",
                  static_cast<unsigned long>(dur),
                  static_cast<long>(g_last_spread),
                  cmd);
    fire_voice_cmd(cmd);
}

void process_spread(int32_t spread, uint32_t now)
{
    if (spread > k_spread_speech) {
        if (!g_in_speech) {
            g_in_speech = true;
            g_speech_start_ms = now;
            Serial.println(F("[display_mic] speech start..."));
        }
        return;
    }

    if (g_in_speech && spread < k_spread_quiet) {
        process_speech_end(now);
    }
}

}  // namespace

void display_mic_init()
{
    g_begin_ok = true;
    g_has_signal = false;
    g_listening = false;
    release_mic_pins();

    Serial.printf("[display_mic] J2 fixed B=%d W=%d S=%d\n",
                  PIN_DISPLAY_MIC_BCLK,
                  PIN_DISPLAY_MIC_WS,
                  PIN_DISPLAY_MIC_SD);
    pick_best_slot();

    int32_t rms = 0;
    int32_t peak = 0;
    int32_t spread = 0;
    int32_t raw0 = 0;
    if (sample_burst(&rms, &peak, &spread, &raw0)) {
        g_last_rms = rms;
        g_last_peak = peak;
        g_last_spread = spread;
        g_has_signal = (spread > 50);
        Serial.printf("[display_mic] ready pk=%ld rms=%ld spread=%ld raw0=0x%08lX\n",
                      static_cast<long>(peak),
                      static_cast<long>(rms),
                      static_cast<long>(spread),
                      static_cast<unsigned long>(static_cast<uint32_t>(raw0)));
    } else {
        Serial.println(F("[display_mic] init burst FAIL"));
    }
}

void display_mic_set_listening(bool on)
{
    g_listening = on;
    g_in_speech = false;
    g_next_poll_ms = 0;
    g_cmd_cooldown_until = 0;

    if (on) {
        Serial.println(F("[display_mic] LISTENING — «μενού» (σύντομα) / «θέρμανση» (μακρύτερα)"));
    } else {
        Serial.println(F("[display_mic] listen OFF"));
    }
}

bool display_mic_is_listening()
{
    return g_listening;
}

void display_mic_set_voice_handler(display_mic_voice_fn fn)
{
    g_voice_fn = fn;
}

void display_mic_poll()
{
    if (!g_begin_ok || !g_listening) {
        return;
    }

    const uint32_t now = millis();
    if (now < g_next_poll_ms) {
        return;
    }
    g_next_poll_ms = now + k_poll_ms;

    int32_t rms = 0;
    int32_t peak = 0;
    int32_t spread = 0;
    int32_t raw0 = 0;
    if (!sample_burst(&rms, &peak, &spread, &raw0)) {
        Serial.println(F("[display_mic] burst FAIL"));
        return;
    }

    g_last_rms = rms;
    g_last_peak = peak;
    g_last_spread = spread;
    if (spread > 50) {
        g_has_signal = true;
    }

    process_spread(spread, now);

    if (g_in_speech) {
        Serial.printf("[display_mic] ... spread=%ld pk=%ld\n",
                      static_cast<long>(spread),
                      static_cast<long>(peak));
    }
}

bool display_mic_begin_ok()
{
    return g_begin_ok;
}

bool display_mic_has_signal()
{
    return g_has_signal;
}

int32_t display_mic_last_peak()
{
    return g_last_peak;
}

bool display_mic_handle_serial(const char *line)
{
    if (line == nullptr) {
        return false;
    }
    if (strcmp(line, "VOICE MENU") == 0 || strcmp(line, "MIC MENU") == 0) {
        fire_voice_cmd("menu");
        return true;
    }
    if (strcmp(line, "VOICE HEAT") == 0 || strcmp(line, "MIC HEAT") == 0) {
        fire_voice_cmd("heat");
        return true;
    }
    if (strcmp(line, "MIC HELP") == 0) {
        Serial.println(F("[display_mic] VOICE MENU | VOICE HEAT | MIC ON via UI"));
        Serial.println(F("[display_mic] voice: short=menu long=heat (beta)"));
        return true;
    }
    return false;
}
