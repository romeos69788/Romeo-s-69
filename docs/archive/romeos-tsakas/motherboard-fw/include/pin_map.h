/**
 * pin_map.h — ESP-32D DevKit 38 pin (USB κάτω)
 *
 * Πηγή αλήθειας: Romeos — romeos-design-notes.md §12.2.2, §12.19.16
 * Οι τιμές είναι GPIO αριθμοί (Arduino esp32 core).
 */
#pragma once

#include <Arduino.h>

// --- I2C (LCD, U9 RTC) ---
static constexpr int PIN_I2C_SDA = 21;
static constexpr int PIN_I2C_SCL = 22;

// --- SPI (κοινό bus: MAX31865 + MCP41050) ---
static constexpr int PIN_SPI_MOSI = 23;
static constexpr int PIN_SPI_MISO = 19;
static constexpr int PIN_SPI_SCK  = 18;
static constexpr int PIN_CS_SOLAR = 5;   // MAX31865
static constexpr int PIN_CS_POT   = 13;  // MCP41050

// --- Αισθητήρες / είσοδοι ---
static constexpr int PIN_DS18_DATA = 4;
static constexpr int PIN_DHT_DATA  = 16;
static constexpr int PIN_FLOW_SIG  = 34;  // input-only
static constexpr int PIN_CT_ADC    = 35;  // input-only (ADC)· χωρίς CT στο PCB → ROMEOS_MB_HAVE_CT_ADC=0
static constexpr int PIN_BACKUP_THERM = 39;  // input-only (VN)
static constexpr int PIN_AC_OPTO   = 36;  // input-only (VP)
static constexpr int PIN_DEFROST   = 14;

// --- Έξοδοι ρελέ (λογική coil: επιβεβαίωση active LOW/HIGH στο bench) ---
static constexpr int PIN_REL_K1 = 25;
static constexpr int PIN_REL_K2 = 26;
static constexpr int PIN_REL_K3 = 27;
static constexpr int PIN_REL_K4 = 32;
static constexpr int PIN_REL_K5 = 33;
static constexpr int PIN_REL_K6 = 17;

// --- Buzzer ---
static constexpr int PIN_BUZZER_ALARM = 15;

// --- INMP441 I2S (CN1 → H2 pins 13/16/17) — MIC_PCB_LOCK_2026-05-08.md ---
static constexpr int PIN_MIC_I2S_WS   = 12;  // H2-13
static constexpr int PIN_MIC_I2S_BCLK = 2;   // H2-16 (D2)
static constexpr int PIN_MIC_I2S_SD   = 0;   // H2-17 (D3/boot) — 10k pull-up στο PCB

/**
 * 0 = χωρίς μετασχηματιστή ρεύματος στο PIN_CT_ADC (ορφανό net / pending PCB).
 * 1 = ενεργός έλεγχος «supply signal» από analogRead(PIN_CT_ADC) στο fault logic.
 * Override: build_flags -DROMEOS_MB_HAVE_CT_ADC=1
 */
#ifndef ROMEOS_MB_HAVE_CT_ADC
#define ROMEOS_MB_HAVE_CT_ADC 0
#endif

// --- Flash / strap — μη χρήση ως GPIO ---
// CLK(6), D0(7), D1(8), D2(16 left strap), D3(11), CMD(18) — δεν ορίζονται εδώ

inline void pin_map_log_header()
{
    Serial.println(F("[pin_map] ESP32 38-pin CONTROL BOARD — see romeos-design-notes §12.2.2"));
}
