#pragma once

#include <cstddef>
#include <stdint.h>

// ROMEOS 69 Alpha rev A — pin map (μητρική καυστήρα)
// Επιβεβαιωμένα από hardware inventory · υπόλοιπα TBD στο PCB.

namespace alpha {

// --- GPIO34 / GPIO35: ελεύθερα (rev A — χωρίς flow / CT) ---
// kWh HP από WiFi μετρητή πίνακα · ροή από σταθερή σκάλα inverter κυκλοφορητή

// --- Ρελέ K1–K6 (TBD: επιβεβαίωσε με rev A schematic) ---
constexpr int kRelayK1 = 32;  // TBD — spare / K1 until relay lock session
constexpr int kRelayK2 = 26;
constexpr int kRelayK3 = 27;  // κύριος κυκλοφορητής · defrost → ON
constexpr int kRelayK4 = 12;  // heater / solar valve (συχνά K4 στο remote)
constexpr int kRelayK5 = 13;
constexpr int kRelayK6 = 15;

// --- Defrost (CN_DEFROST · opto από τριόδη βάνα HP) ---
constexpr int kDefrostSig = 14;

// --- OneWire DS18B20 (TBD: bus pin) ---
constexpr int kOneWireBus = 4;

// --- Alpha ↔ Beta (UART) ---
constexpr int kBetaUartNum = 1;
constexpr int kBetaTx = 17;
constexpr int kBetaRx = 16;
constexpr uint32_t kBetaBaud = 115200;

// --- Displays: UART / ESP-NOW (TBD) ---
constexpr int kDisplayUartNum = 2;
constexpr int kDisplayTx = 25;
constexpr int kDisplayRx = 33;
constexpr uint32_t kDisplayBaud = 115200;

// --- HP outdoor RS485 (ΜΗ χρησιμοποιείται στο rev A · future) ---
// UART2 TX=17, RX=16, DE+RE=32 — κρατάμε για μελλοντική ενσωμάτωση HP board.

constexpr int kRelayPins[] = {
    kRelayK1, kRelayK2, kRelayK3, kRelayK4, kRelayK5, kRelayK6,
};
constexpr size_t kRelayCount = sizeof(kRelayPins) / sizeof(kRelayPins[0]);

}  // namespace alpha
