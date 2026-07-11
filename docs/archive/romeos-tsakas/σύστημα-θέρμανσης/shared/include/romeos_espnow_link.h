/**
 * ESP-NOW transport — peer link μητρική ↔ οθόνη (χωρίς router / LAN UDP).
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ROMEOS_ESPNOW_WIFI_CHANNEL
#define ROMEOS_ESPNOW_WIFI_CHANNEL 1
#endif

#ifndef ROMEOS_ESPNOW_COEXIST_STA
#define ROMEOS_ESPNOW_COEXIST_STA 0
#endif

typedef void (*romeos_espnow_rx_fn)(const uint8_t *data, size_t len, const uint8_t src_mac[6]);

bool romeos_espnow_begin(const char *role_tag);
void romeos_espnow_set_rx_cb(romeos_espnow_rx_fn fn);

void romeos_espnow_get_local_mac(uint8_t mac[6]);
bool romeos_espnow_add_peer(const uint8_t mac[6]);
bool romeos_espnow_has_peer(const uint8_t mac[6]);

bool romeos_espnow_send(const uint8_t mac[6], const void *data, size_t len);
bool romeos_espnow_send_broadcast(const void *data, size_t len);

bool romeos_espnow_load_peer_mac(const char *nvs_key, uint8_t mac[6]);
bool romeos_espnow_save_peer_mac(const char *nvs_key, const uint8_t mac[6]);

#ifdef __cplusplus
}
#endif
