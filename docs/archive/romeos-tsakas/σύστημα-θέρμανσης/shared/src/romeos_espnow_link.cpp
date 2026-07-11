#include "romeos_espnow_link.h"

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <cstring>
#include <esp_now.h>
#include <esp_wifi.h>

namespace {

constexpr char k_nvs_ns[] = "romeos_en";
constexpr uint8_t k_bcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

romeos_espnow_rx_fn s_rx_fn = nullptr;
bool s_ready = false;

bool peer_add(const uint8_t mac[6])
{
    if (!mac) {
        return false;
    }
    if (esp_now_is_peer_exist(mac)) {
        return true;
    }
    esp_now_peer_info_t peer{};
    std::memcpy(peer.peer_addr, mac, 6);
#if ROMEOS_ESPNOW_COEXIST_STA
    peer.channel = 0; /* ακολουθεί το κανάλι του home STA */
#else
    peer.channel = ROMEOS_ESPNOW_WIFI_CHANNEL;
#endif
    peer.encrypt = false;
    peer.ifidx = WIFI_IF_STA;
    return esp_now_add_peer(&peer) == ESP_OK;
}

void recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    if (!s_rx_fn || !info || !info->src_addr || !data || len <= 0) {
        return;
    }
    s_rx_fn(data, static_cast<size_t>(len), info->src_addr);
}

}  // namespace

bool romeos_espnow_begin(const char *role_tag)
{
    WiFi.persistent(false);
    if (WiFi.getMode() == WIFI_OFF) {
        WiFi.mode(WIFI_STA);
    }
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);

#if ROMEOS_ESPNOW_COEXIST_STA
    /* Μην κόψεις το home STA — ESP-NOW μοιράζεται radio στο ίδιο κανάλι με το AP. */
#else
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect(false);
    }
    delay(50);

    if (esp_wifi_set_channel(ROMEOS_ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
        Serial.println(F("[espnow] set channel failed"));
    }
#endif

    if (esp_now_init() != ESP_OK) {
        esp_now_deinit();
        if (esp_now_init() != ESP_OK) {
            Serial.println(F("[espnow] init failed"));
            return false;
        }
    }

    esp_now_register_recv_cb(recv_cb);
    if (!peer_add(k_bcast)) {
        Serial.println(F("[espnow] broadcast peer add failed"));
    }

    s_ready = true;
    uint8_t mac[6]{};
    romeos_espnow_get_local_mac(mac);
    uint8_t ch = 0;
    wifi_second_chan_t sc{};
    esp_wifi_get_channel(&ch, &sc);
    Serial.printf("[espnow] %s ready ch=%u mac=%02X:%02X:%02X:%02X:%02X:%02X coexist_sta=%d\n",
                  role_tag ? role_tag : "?",
                  static_cast<unsigned>(ch),
                  mac[0],
                  mac[1],
                  mac[2],
                  mac[3],
                  mac[4],
                  mac[5],
                  static_cast<int>(ROMEOS_ESPNOW_COEXIST_STA));
    return true;
}

void romeos_espnow_set_rx_cb(romeos_espnow_rx_fn fn)
{
    s_rx_fn = fn;
}

void romeos_espnow_get_local_mac(uint8_t mac[6])
{
    if (!mac) {
        return;
    }
    WiFi.macAddress(mac);
}

bool romeos_espnow_add_peer(const uint8_t mac[6])
{
    if (!s_ready || !mac) {
        return false;
    }
    return peer_add(mac);
}

bool romeos_espnow_has_peer(const uint8_t mac[6])
{
    if (!mac) {
        return false;
    }
    return esp_now_is_peer_exist(mac);
}

bool romeos_espnow_send(const uint8_t mac[6], const void *data, size_t len)
{
    if (!s_ready || !mac || !data || len == 0) {
        return false;
    }
    if (!esp_now_is_peer_exist(mac)) {
        if (!peer_add(mac)) {
            return false;
        }
    }
    return esp_now_send(mac, static_cast<const uint8_t *>(data), len) == ESP_OK;
}

bool romeos_espnow_send_broadcast(const void *data, size_t len)
{
    return romeos_espnow_send(k_bcast, data, len);
}

bool romeos_espnow_load_peer_mac(const char *nvs_key, uint8_t mac[6])
{
    if (!nvs_key || !mac) {
        return false;
    }
    Preferences p;
    if (!p.begin(k_nvs_ns, true)) {
        return false;
    }
    if (!p.isKey(nvs_key)) {
        p.end();
        return false;
    }
    const size_t n = p.getBytesLength(nvs_key);
    if (n != 6) {
        p.end();
        return false;
    }
    const size_t got = p.getBytes(nvs_key, mac, 6);
    p.end();
    return got == 6;
}

bool romeos_espnow_save_peer_mac(const char *nvs_key, const uint8_t mac[6])
{
    if (!nvs_key || !mac) {
        return false;
    }
    Preferences p;
    if (!p.begin(k_nvs_ns, false)) {
        return false;
    }
    const size_t wrote = p.putBytes(nvs_key, mac, 6);
    p.end();
    return wrote == 6;
}
