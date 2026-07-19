#pragma once

#include <cstddef>
#include <stdint.h>

// ROMEOS 69 Alpha rev A — pin map (μητρική καυστήρα)
// Επιβεβαιωμένα από hardware inventory · υπόλοιπα TBD στο PCB.

namespace alpha {

// --- CT (μοναδικό · SCT-013 20A/1V στη γραμμή αντλίας) ---
constexpr int kCtHpAdc = 35;  // ADC1_CH7 · μόνο HP line

// --- Ρελέ K1–K6 (TBD: επιβεβαίωσε με rev A schematic) ---
constexpr int kRelayK1 = 26;
constexpr int kRelayK2 = 27;
constexpr int kRelayK3 = 14;
constexpr int kRelayK4 = 12;  // heater (συχνά K4 στο remote)
constexpr int kRelayK5 = 13;
constexpr int kRelayK6 = 15;

// --- OneWire DS18B20 (TBD: bus pin) ---
constexpr int kOneWireBus = 4;

// --- Flow sensor (digital) ---
constexpr int kFlowSig = 34;

// --- Alpha ↔ Beta (UART) ---
constexpr int kBetaUartNum = 1;
constexpr int kBetaTx = 17;
constexpr int kBetaRx = 16;
constexpr uint32_t kBetaBaud = 115200;

// --- Alpha ↔ Panel οθόνη λεβητοστασίου (CN_PANEL UART) ---
// Room thermostat = άλλη Viewe · Wi‑Fi/ESP‑NOW · όχι αυτά τα pins.
// Docs: docs/DISPLAY-TWO-ROLES-2026-07-19.md
constexpr int kPanelUartNum = 2;
constexpr int kPanelTx = 25;  // CN_PANEL pin 3 · H2-9
constexpr int kPanelRx = 33;  // CN_PANEL pin 4 · H2-8
constexpr uint32_t kPanelBaud = 115200;

// Legacy aliases (παλιά ονομασία «Display»)
constexpr int kDisplayUartNum = kPanelUartNum;
constexpr int kDisplayTx = kPanelTx;
constexpr int kDisplayRx = kPanelRx;
constexpr uint32_t kDisplayBaud = kPanelBaud;

// --- HP outdoor RS485 (ΜΗ χρησιμοποιείται στο rev A · future) ---
// UART2 TX=17, RX=16, DE+RE=32 — κρατάμε για μελλοντική ενσωμάτωση HP board.

constexpr int kRelayPins[] = {
    kRelayK1, kRelayK2, kRelayK3, kRelayK4, kRelayK5, kRelayK6,
};
constexpr size_t kRelayCount = sizeof(kRelayPins) / sizeof(kRelayPins[0]);

}  // namespace alpha
