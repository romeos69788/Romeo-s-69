/**
 * Κοινά δομικά για ασύρματη διασύνδεση μητρική (CONTROL BOARD) ↔ οθόνη (Viewe).
 *
 * Πολιτική συνδέσμου: romeos-design-notes.md — «Επικοινωνία μητρική ↔ οθόνη (ασύρματη)»
 * (SoftAP+STA ή ESP-NOW — το transport υλοποιείται ξεχωριστά).
 *
 * Τα ονόματα πεδίων ευθυγραμμίζονται με το SquareLine screen `ui_Romeos1` και το
 * `thermostat-ui-demo/src/main.cpp` (γωνίες ROOM/OUT, δεξιά Solar/Boiler/Intel/Outlet,
 * HP / Pump1 / Pump2 / Heater).
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROMEOS_LINK_MAGIC 0x524f4d31u /* 'ROM1' */
/** v2: τελευταία πεδία `romeos_mb_to_display_v1_t` — setpoint από μητρική (MQTT/NVS). */
#define ROMEOS_LINK_VER   3u

#define ROMEOS_MB_FLAG_TELEMETRY_VALID 0x01u
#define ROMEOS_MB_FLAG_ALARM_ACTIVE    0x02u
#define ROMEOS_MB_FLAG_MASTER_SHUTDOWN 0x04u

/** Θύρα UDP: οθόνη → μητρική (η μητρική ακούει εδώ). */
#define ROMEOS_UDP_PORT_MB_LISTEN 9000u
/** Θύρα UDP: μητρική → οθόνη (η οθόνη ακούει εδώ). */
#define ROMEOS_UDP_PORT_DISPLAY_LISTEN 9001u

/*
 * Ιδιωτικό SoftAP της μητρικής (όχι οικιακό Wi‑Fi).
 * Άλλαξε το pass από build_flags: -DROMEOS_MB_AP_PASS=\"...\"
 */
#ifndef ROMEOS_MB_AP_SSID
#define ROMEOS_MB_AP_SSID "RomeosMB"
#endif
#ifndef ROMEOS_MB_AP_PASS
#define ROMEOS_MB_AP_PASS "romeos2026"
#endif

/*
 * Όταν οθόνη + μητρική είναι STA στο οικιακό LAN, η οθόνη στέλνει UDP εδώ (όχι στο gateway).
 * Build flag (και τα δύο projects): -DROMEOS_MB_LAN_IP=\"192.168.1.50\"
 * Αν μείνει κενό "", η οθόνη χρησιμοποιεί μόνο RomeosMB (SoftAP) ή πρέπει να το ορίσεις για LAN.
 */
#ifndef ROMEOS_MB_LAN_IP
#define ROMEOS_MB_LAN_IP ""
#endif

/** Μητρική → οθόνη: περιοδική κατάσταση (ενημέρωση ~1–2 Hz ή on-change). */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  version;
    /** bit0: ουσιαστικά δεδομένα έγκυρα · bit1: ενεργός συναγερμός · bit2: master shutdown */
    uint8_t  flags;
    /** v3+: υγρασία εξωτερικού CN5 DHT, δέκατα % (650 = 65,0 %). 0 αν άγνωστο. */
    int16_t outdoor_rh_x10;
    /** Θερμοκρασίες σε δέκατα °C (π.χ. 215 = 21,5 °C) */
    int16_t room_display_c_x10;   /* κεντρική ένδειξη «χώρου» (από MCP spoof / λογική μητρικής) */
    int16_t outdoor_c_x10;        /* DHT CN5 εξωτερικού */
    int16_t solar_c_x10;          /* MAX31865 */
    int16_t boiler_c_x10;         /* DS18 νερού (CN4 κ.λπ.) — μελλοντικά */
    int16_t supply_c_x10;       /* Intel — προσαγωγή / «supply» στο UI */
    int16_t return_c_x10;         /* Outlet — επιστροφή στο UI */
    /** Ρελέ: bit (n-1) = κατάσταση coil όπως το firmware (ενεργό LOW → 1 = ενεργό πηνίο αν έτσι ορίζεις) */
    uint8_t  relay_k1_on;
    uint8_t  relay_k2_on;
    uint8_t  relay_k3_on;
    uint8_t  relay_k4_on;
    uint8_t  relay_k5_on;
    uint8_t  relay_k6_on;
    uint8_t  heat_pump_on;        /* HP στο δεξί πίνακα */
    uint8_t  pump1_on;
    uint8_t  pump2_on;
    uint8_t  heater_on;
    uint8_t  flow_sig_high;       /* FLOW_SIG (Hall) */
    uint8_t  defrost_active;      /* DEFROST_SIG */
    /** v1: spare. v2: + setpoint. bit0=1: placeholders solar/boiler/supply/return (δεξιά «--»). bit1=1: outdoor temp live (OUT). bit2=1: outdoor RH live (CN5 DHT). */
    uint8_t  mb_proto_reserved;
    int16_t  setpoint_c_x10; /* v2 μόνο: 50…350 · αγνοείται αν version<2 */
} romeos_mb_to_display_v1_t;

/**
 * Οθόνη → μητρική: setpoint + (προαιρετικά) θερμοκρασία χώρου από SHT της οθόνης.
 * Παλιό μέγεθος πακέτου = 10 byte (χωρίς room). Νέο = 12 byte — η μητρική δέχεται και τα δύο.
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  version;
    uint8_t  msg_type; /* 1 = setpoint + room report (room πεδίο αν n>=12) */
    uint8_t  reserved[2];
    int16_t  setpoint_c_x10; /* 5…35 °C όπως clamp στο UI */
    int16_t  room_from_display_c_x10; /* δέκατα °C από SHT Viewe· INT16_MIN = άκυρο / παλιά οθόνη */
} romeos_display_to_mb_v1_t;

#define ROMEOS_MSG_SETPOINT 1u
/** Σταθερά: η μητρική αγνοεί το πεδίο room (παλιό firmware οθόνης ή κανένας SHT). */
#define ROMEOS_ROOM_FROM_DISPLAY_INVALID ((int16_t)0x8000)

#ifdef __cplusplus
}
#endif
