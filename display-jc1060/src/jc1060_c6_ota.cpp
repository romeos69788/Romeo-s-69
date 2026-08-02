/*
 * Push matching ESP-Hosted slave firmware to on-board ESP32-C6 over SDIO.
 * Host expects 2.12.11; Guition ships older JC-C6-slave → WiFi RPC fails.
 */

#include <Arduino.h>
#include <WiFi.h>
#include "esp32-hal-hosted.h"
#include "jc1060_c6_ota.h"

/* PlatformIO embed: data/c6_slave_2.12.11.bin */
extern const uint8_t c6_slave_start[] asm("_binary_data_c6_slave_2_12_11_bin_start");
extern const uint8_t c6_slave_end[] asm("_binary_data_c6_slave_2_12_11_bin_end");

/* JC1060P470C SDIO → C6 (same as Guition / ESPHome) */
static constexpr int8_t kClk = 18;
static constexpr int8_t kCmd = 19;
static constexpr int8_t kD0 = 14;
static constexpr int8_t kD1 = 15;
static constexpr int8_t kD2 = 16;
static constexpr int8_t kD3 = 17;
static constexpr int8_t kRst = 54;

bool jc1060_c6_ensure_slave(void)
{
#if !defined(CONFIG_ESP_HOSTED_ENABLED)
    Serial.println("[c6-ota] CONFIG_ESP_HOSTED_ENABLED off — cannot update C6");
    return false;
#else
    hostedSetPins(kClk, kCmd, kD0, kD1, kD2, kD3, kRst);

    /* Bring up hosted stack (WiFi STA path) */
    WiFi.mode(WIFI_STA);
    delay(300);

    if (!hostedIsInitialized()) {
        Serial.println("[c6-ota] hosted not initialized after WiFi.mode");
        if (!hostedInitWiFi()) {
            Serial.println("[c6-ota] hostedInitWiFi FAILED");
            return false;
        }
    }

    uint32_t hm = 0, hn = 0, hp = 0;
    uint32_t sm = 0, sn = 0, sp = 0;
    hostedGetHostVersion(&hm, &hn, &hp);
    hostedGetSlaveVersion(&sm, &sn, &sp);
    Serial.printf("[c6-ota] host=%u.%u.%u  slave(cached)=%u.%u.%u\n", hm, hn, hp, sm, sn, sp);

    const bool need = hostedHasUpdate() || (sm == 0 && sn == 0 && sp == 0);
    if (!need) {
        Serial.println("[c6-ota] slave OK — no update");
        return true;
    }

    const size_t len = (size_t)(c6_slave_end - c6_slave_start);
    Serial.printf("[c6-ota] updating C6 from embedded image (%u bytes)…\n", (unsigned)len);

    if (!hostedBeginUpdate()) {
        Serial.println("[c6-ota] begin FAILED (C6 may need UART flash)");
        return false;
    }

    const uint8_t *p = c6_slave_start;
    size_t left = len;
    constexpr size_t kChunk = 2048;
    unsigned dots = 0;
    while (left > 0) {
        const size_t n = left > kChunk ? kChunk : left;
        if (!hostedWriteUpdate(const_cast<uint8_t *>(p), (uint32_t)n)) {
            Serial.println("\n[c6-ota] write FAILED");
            return false;
        }
        p += n;
        left -= n;
        if ((++dots % 32) == 0) {
            Serial.print('.');
        }
    }
    Serial.println();

    if (!hostedEndUpdate()) {
        Serial.println("[c6-ota] end FAILED");
        return false;
    }

    (void)hostedActivateUpdate();
    Serial.println("[c6-ota] OK — restarting host to activate C6…");
    delay(200);
    ESP.restart();
    return true; /* not reached */
#endif
}
