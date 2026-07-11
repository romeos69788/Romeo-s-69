/**
 * Μητρική CONTROL BOARD v1.0 — σκελετός firmware
 *
 * Σκοπός: αρχικοποίηση pin, Serial, I2C/SPI· η λογική υδραυλικού / RF προς οθόνη
 * έρχεται σε επόμενα βήματα (βλ. romeos-design-notes.md).
 */

#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>

#include "mb_bench_relays.h"
#include "mb_buzzer_rtc.h"
#include "mb_mqtt.h"
#include "mb_outdoor_dht.h"
#include "mb_outdoor_present.h"
#include "mb_outdoor_sensor.h"
#include "mb_mic.h"
#include "mb_solar_rtd.h"
#include "mb_uart_link.h"
#include "mb_wifi_link.h"
#include "pin_map.h"
#include "romeos_link_types.h"
#include "romeos_wifi_nvs.h"

#ifndef ROMEOS_MB_MIC_ENABLE
#define ROMEOS_MB_MIC_ENABLE 0
#endif

#ifndef ROMEOS_MB_WIFI_STA
#define ROMEOS_MB_WIFI_STA 1
#endif

#ifdef ROMEOS_MB_BENCH_RELAYS_ON
bool g_bench_relays_hold_on = false;
#endif

namespace {

constexpr uint32_t kSerialBaud = 115200;
constexpr uint32_t kFaultDelayMs = 5u * 60u * 1000u;
constexpr uint8_t kLcdAddr = 0x27;
constexpr uint8_t kLcdCols = 20;
constexpr uint8_t kLcdRows = 4;

LiquidCrystal_I2C g_lcd(kLcdAddr, kLcdCols, kLcdRows);

enum class FaultCode : uint8_t {
    None = 0,
    HeatPumpNoStart,
    SupplySensorNoSignal,
};

FaultCode g_fault_code = FaultCode::None;
char g_fault_text[64] = "OK";
uint32_t g_hp_no_start_since_ms = 0;
uint32_t g_supply_no_signal_since_ms = 0;
uint32_t g_last_lcd_refresh_ms = 0;

/** Όλα τα REL_K* ως έξοδοι· αρχική κατάσταση HIGH (coil inactive). K2/K3 HIGH μέχρι I2S. */
void init_relay_outputs()
{
    const int pins[] = {
        PIN_REL_K1, PIN_REL_K2, PIN_REL_K3,
        PIN_REL_K4, PIN_REL_K5, PIN_REL_K6,
    };
    for (int p : pins) {
        pinMode(p, OUTPUT);
        digitalWrite(p, HIGH);
    }
}

void init_buzzer_pin()
{
    pinMode(PIN_BUZZER_ALARM, OUTPUT);
    digitalWrite(PIN_BUZZER_ALARM, LOW);
}

void init_sensor_inputs()
{
    pinMode(PIN_FLOW_SIG, INPUT);
#if ROMEOS_MB_HAVE_CT_ADC
    pinMode(PIN_CT_ADC, INPUT);
#endif
    pinMode(PIN_BACKUP_THERM, INPUT);
    pinMode(PIN_AC_OPTO, INPUT);
    pinMode(PIN_DEFROST, INPUT);
    pinMode(PIN_DS18_DATA, INPUT);
    pinMode(PIN_DHT_DATA, INPUT);
}

void init_i2c()
{
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
}

void init_status_lcd()
{
    g_lcd.init();
    g_lcd.backlight();
    g_lcd.clear();
}

void init_spi()
{
    // SS = -1: και τα δύο CS (MAX + MCP) ελέγχονται χειροκίνητα
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, -1);
    pinMode(PIN_CS_SOLAR, OUTPUT);
    digitalWrite(PIN_CS_SOLAR, HIGH);
    pinMode(PIN_CS_POT, OUTPUT);
    digitalWrite(PIN_CS_POT, HIGH);
}

void lcd_print_line(uint8_t row, const char *text)
{
    if (row >= kLcdRows) {
        return;
    }
    g_lcd.setCursor(0, row);
    char line[21];
    std::snprintf(line, sizeof(line), "%-20.20s", text ? text : "");
    g_lcd.print(line);
}

void refresh_fault_lcd(bool force = false)
{
    /*
     * LCD 2004 — ΜΟΝΟ μητρική (όχι Viewe 7″).
     * Κανονικά: status/diag (τελικό κείμενο γραμμών TBD).
     * Βλάβη: ALARM + μήνυμα εδώ + buzzer (PIN_BUZZER_ALARM).
     * Γραμμή 0: CN5 εξωτερικό T+RH (→ Viewe). Γραμμή 1: CN4 DS18.
     * Γραμμή 2: ηλιακός PT100 (MAX31865). Γραμμή 3: ώρα/ημερ. DS3231 (U9).
     */
    const uint32_t now = millis();
    if (!force && (now - g_last_lcd_refresh_ms < 1000u)) {
        return;
    }
    g_last_lcd_refresh_ms = now;

    if (g_fault_code != FaultCode::None) {
        lcd_print_line(0, "!!! ALARM ACTIVE !!!");
        lcd_print_line(1, g_fault_text);
        lcd_print_line(2, "Check boiler room");
        lcd_print_line(3, "Romeos MB v1.0");
        return;
    }

    const mb_outdoor_present_t cn5 = mb_outdoor_cn5_snapshot();
    const mb_outdoor_present_t cn4 = mb_outdoor_cn4_snapshot();
    char line0[21];
    char line1[21];
    char line2[21];
    char line3[21];

    if (cn5.temp_live) {
        const int t_c = mb_outdoor_temp_whole_x10(cn5.temp_c_x10) / 10;
        std::snprintf(line0,
                      sizeof(line0),
                      "EXTERIOR %2dC RH %2d%%",
                      t_c,
                      cn5.rh_x10 / 10);
    } else {
        std::snprintf(line0, sizeof(line0), "EXTERIOR -- RH --");
    }

    if (cn4.temp_live) {
        const int t_c = mb_outdoor_temp_whole_x10(cn4.temp_c_x10) / 10;
        std::snprintf(line1, sizeof(line1), "CN4 DS18 bench %2dC", t_c);
    } else {
        std::snprintf(line1, sizeof(line1), "CN4 DS18 bench --");
    }

    mb_solar_rtd_format_lcd_line(line2, sizeof(line2));
    mb_rtc_format_lcd_line(line3, sizeof(line3));

    lcd_print_line(0, line0);
    lcd_print_line(1, line1);
    lcd_print_line(2, line2);
    lcd_print_line(3, line3);
}

void set_fault(FaultCode code, const char *text)
{
    if (g_fault_code == code) {
        return;
    }
    g_fault_code = code;
    std::snprintf(g_fault_text, sizeof(g_fault_text), "%s", text ? text : "Fault");
    mb_wifi_link_set_alarm_active(g_fault_code != FaultCode::None);
    refresh_fault_lcd(true);
    Serial.printf("[fault] %s\n", g_fault_text);
}

void clear_fault_if_needed()
{
    if (g_fault_code == FaultCode::None) {
        return;
    }
    g_fault_code = FaultCode::None;
    std::snprintf(g_fault_text, sizeof(g_fault_text), "OK");
    mb_wifi_link_set_alarm_active(false);
    refresh_fault_lcd(true);
    Serial.println(F("[fault] cleared"));
}

void update_fault_logic()
{
#ifdef ROMEOS_MB_BENCH_RELAYS_ON
    /* Bench: K1 ON χωρίς AC opto — μην ενεργοποιείς ψευδο-συναγερμό bring-up. */
    if (g_fault_code != FaultCode::None) {
        clear_fault_if_needed();
    }
    return;
#endif
    const uint32_t now = millis();
    const bool hp_commanded = (digitalRead(PIN_REL_K1) == LOW);
    const bool hp_feedback_on = (digitalRead(PIN_AC_OPTO) == HIGH);

    if (hp_commanded && !hp_feedback_on) {
        if (g_hp_no_start_since_ms == 0) {
            g_hp_no_start_since_ms = now;
        } else if (now - g_hp_no_start_since_ms >= kFaultDelayMs) {
            set_fault(FaultCode::HeatPumpNoStart, "ERR heat pump no AC");
            return;
        }
    } else {
        g_hp_no_start_since_ms = 0;
    }

#if ROMEOS_MB_HAVE_CT_ADC
    const int supply_adc = analogRead(PIN_CT_ADC);
    const bool supply_has_signal = (supply_adc > 20);
    if (!supply_has_signal) {
        if (g_supply_no_signal_since_ms == 0) {
            g_supply_no_signal_since_ms = now;
        } else if (now - g_supply_no_signal_since_ms >= kFaultDelayMs) {
            set_fault(FaultCode::SupplySensorNoSignal, "ERR supply sensor");
            return;
        }
    } else {
        g_supply_no_signal_since_ms = 0;
    }
#else
    g_supply_no_signal_since_ms = 0;
#endif

    clear_fault_if_needed();
}

static int relay_pin_for_channel(int ch)
{
    switch (ch) {
    case 1: return PIN_REL_K1;
    case 2: return PIN_REL_K2;
    case 3: return PIN_REL_K3;
    case 4: return PIN_REL_K4;
    case 5: return PIN_REL_K5;
    case 6: return PIN_REL_K6;
    default: return -1;
    }
}

void relay_set_channel(int ch, bool on)
{
    if (pin_map_relay_channel_mic_shared(ch)) {
        Serial.printf("[rel_test] K%d blocked — GPIO shared with CN1 mic (WS/SD)\n", ch);
        return;
    }
    const int pin = relay_pin_for_channel(ch);
    if (pin < 0) {
        return;
    }
    digitalWrite(pin, on ? LOW : HIGH);
    Serial.printf("[rel_test] K%d %s (GPIO %d = %s)\n",
                  ch,
                  on ? "ON" : "OFF",
                  pin,
                  on ? "LOW" : "HIGH");
}

void relay_all_off()
{
    for (int ch = 1; ch <= 6; ++ch) {
        relay_set_channel(ch, false);
    }
}

void relay_all_on()
{
    for (int ch = 1; ch <= 6; ++ch) {
        relay_set_channel(ch, true);
    }
}

char g_rel_cmd_buf[48];
uint8_t g_rel_cmd_len = 0;

void relay_test_print_help()
{
    Serial.println(F("[rel_test] REL HELP"));
    Serial.println(F("[rel_test] REL 1 ON  ... REL 6 ON   (LED bright + relay click)"));
    Serial.println(F("[rel_test] REL 1 OFF ... REL 6 OFF"));
    Serial.println(F("[rel_test] REL ALL ON  | REL ALL OFF"));
    Serial.println(F("[rel_test] DS18 DIAG — CN4 1-Wire scan"));
    Serial.println(F("[rel_test] CN5 DIAG — DHT22/DHT11 on GPIO16"));
    Serial.println(F("[rel_test] SOLAR DIAG — MAX31865 PT100 GPIO5 CS"));
    Serial.println(F("[rel_test] MIC PROBE | MIC CMD <text> | MIC HELP"));
    mb_buzzer_rtc_print_help();
}

void poll_relay_test_serial()
{
    while (Serial.available() > 0) {
        const char c = static_cast<char>(Serial.read());
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            g_rel_cmd_buf[g_rel_cmd_len] = '\0';
            if (g_rel_cmd_len > 0) {
                if (strcmp(g_rel_cmd_buf, "REL HELP") == 0) {
                    relay_test_print_help();
                } else if (strcmp(g_rel_cmd_buf, "DS18 DIAG") == 0) {
                    mb_outdoor_sensor_run_diag();
                } else if (strcmp(g_rel_cmd_buf, "CN5 DIAG") == 0 ||
                           strcmp(g_rel_cmd_buf, "DHT DIAG") == 0) {
                    mb_outdoor_dht_log_diagnostics();
                    mb_outdoor_dht_force_poll();
                } else if (strcmp(g_rel_cmd_buf, "SOLAR DIAG") == 0) {
                    mb_solar_rtd_run_diag();
                } else if (mb_mic_handle_serial(g_rel_cmd_buf)) {
                    refresh_fault_lcd(true);
                } else if (mb_buzzer_rtc_handle_serial(g_rel_cmd_buf)) {
                    /* handled */
                } else if (strcmp(g_rel_cmd_buf, "REL ALL OFF") == 0) {
                    relay_all_off();
#ifdef ROMEOS_MB_BENCH_RELAYS_ON
                    ::g_bench_relays_hold_on = false;
#endif
                } else if (strcmp(g_rel_cmd_buf, "REL ALL ON") == 0) {
                    relay_all_on();
#ifdef ROMEOS_MB_BENCH_RELAYS_ON
                    ::g_bench_relays_hold_on = true;
#endif
                } else if (strncmp(g_rel_cmd_buf, "REL ", 4) == 0) {
                    int ch = 0;
                    char action[8] = {};
                    if (sscanf(g_rel_cmd_buf + 4, "%d %7s", &ch, action) == 2) {
                        if (ch >= 1 && ch <= 6) {
                            if (strcmp(action, "ON") == 0) {
                                relay_set_channel(ch, true);
                            } else if (strcmp(action, "OFF") == 0) {
                                relay_set_channel(ch, false);
                            } else {
                                Serial.println(F("[rel_test] use ON or OFF"));
                            }
                        } else {
                            Serial.println(F("[rel_test] channel 1-6"));
                        }
                    } else {
                        Serial.println(F("[rel_test] format: REL 1 ON"));
                    }
                }
            }
            g_rel_cmd_len = 0;
            continue;
        }
        if (g_rel_cmd_len + 1 < sizeof(g_rel_cmd_buf)) {
            g_rel_cmd_buf[g_rel_cmd_len++] = c;
        }
    }
}

}  // namespace

#ifdef ROMEOS_MB_BENCH_RELAYS_ON
void mb_bench_relays_quiet_enter()
{
    for (int ch = 1; ch <= 6; ++ch) {
        if (pin_map_relay_channel_mic_shared(ch)) {
            continue;
        }
        const int pin = (ch == 1) ? PIN_REL_K1 :
                        (ch == 2) ? PIN_REL_K2 :
                        (ch == 3) ? PIN_REL_K3 :
                        (ch == 4) ? PIN_REL_K4 :
                        (ch == 5) ? PIN_REL_K5 : PIN_REL_K6;
        digitalWrite(pin, HIGH);
    }
    delay(25);
}

void mb_bench_relays_quiet_leave()
{
    if (!g_bench_relays_hold_on) {
        return;
    }
    for (int ch = 1; ch <= 6; ++ch) {
        if (pin_map_relay_channel_mic_shared(ch)) {
            continue;
        }
        const int pin = (ch == 1) ? PIN_REL_K1 :
                        (ch == 2) ? PIN_REL_K2 :
                        (ch == 3) ? PIN_REL_K3 :
                        (ch == 4) ? PIN_REL_K4 :
                        (ch == 5) ? PIN_REL_K5 : PIN_REL_K6;
        digitalWrite(pin, LOW);
    }
}
#endif

void setup()
{
    Serial.begin(kSerialBaud);
    delay(200);
    pin_map_log_header();
    Serial.println(F(
        "[boot] Ίδιο firmware πρωτεύον/εφεδρικό — SoftAP+UDP προς οθόνη (ανταλλαγή module)"));
    Serial.println(F("[boot] setup start"));
#ifdef ROMEOS_MB_BUILD_ID
    Serial.printf("[boot] build=%s\n", ROMEOS_MB_BUILD_ID);
#endif
    Serial.printf("[link] sizeof(romeos_mb_to_display_v1_t)=%u\n",
                  static_cast<unsigned>(sizeof(romeos_mb_to_display_v1_t)));
    Serial.printf("[link] sizeof(romeos_display_to_mb_v1_t)=%u\n",
                  static_cast<unsigned>(sizeof(romeos_display_to_mb_v1_t)));

    init_relay_outputs();
    init_buzzer_pin();
    init_sensor_inputs();
    mb_outdoor_sensor_begin();
    mb_outdoor_sensor_boot_probe_no_wifi();
    mb_outdoor_dht_begin();
    init_i2c();
    init_status_lcd();
    lcd_print_line(0, "Romeos MB boot...");
    lcd_print_line(1, "Init sensors...");
    lcd_print_line(2, "SOLAR starting...");
    lcd_print_line(3, "RTC U9 waiting...");
    init_spi();
    mb_solar_rtd_begin();
#if ROMEOS_MB_MIC_ENABLE
    Serial.println(F("[mic] burst sample — K2 LED off between reads"));
    mb_mic_begin();
#else
    Serial.println(F("[mic] PAUSED — skip boot I2S hunt"));
#endif

    mb_uart_link_begin();
    mb_wifi_link_begin();
    mb_wifi_link_set_alarm_active(false);
#if ROMEOS_MB_WIFI_STA
    mb_mqtt_begin();
#endif
    refresh_fault_lcd(true);

    mb_buzzer_boot_chime();

    Serial.println(F("[boot] setup done — loop"));
    Serial.println(F("[rel_test] bench test: type REL HELP (115200)"));
}

void loop()
{
    poll_relay_test_serial();
    mb_outdoor_dht_poll();
    mb_outdoor_sensor_poll();
    mb_solar_rtd_poll();
    mb_mic_poll();

    if (!mb_solar_rtd_is_calibrated()) {
        mb_solar_rtd_force_poll();
        if (mb_outdoor_sensor_has_live()) {
            mb_solar_rtd_try_calibrate(mb_outdoor_sensor_temp_c_x10());
        }
    }

    romeos_wifi_nvs_poll_serial();
    mb_uart_link_poll();
    mb_wifi_link_poll();
#if ROMEOS_MB_WIFI_STA
    mb_rtc_ntp_poll();
    mb_mqtt_poll();
#endif
    update_fault_logic();
    refresh_fault_lcd();

    static uint32_t last = 0;
    const uint32_t now = millis();
    if (now - last < 5000) {
        return;
    }
    last = now;
    const mb_outdoor_present_t cn5 = mb_outdoor_cn5_snapshot();
    const mb_outdoor_present_t cn4 = mb_outdoor_cn4_snapshot();
    if (cn5.temp_live) {
        const int t5 = mb_outdoor_temp_whole_x10(cn5.temp_c_x10) / 10;
        Serial.printf("[tick] millis=%lu CN5=%dC RH=%d%%",
                      static_cast<unsigned long>(now),
                      t5,
                      cn5.rh_x10 / 10);
    } else {
        Serial.printf("[tick] millis=%lu CN5=--",
                      static_cast<unsigned long>(now));
    }
    if (cn4.temp_live) {
        const int t4 = mb_outdoor_temp_whole_x10(cn4.temp_c_x10) / 10;
        Serial.printf(" CN4=%dC", t4);
    } else {
        Serial.printf(" CN4=--");
    }
    if (mb_solar_rtd_has_live()) {
        Serial.printf(" SOLAR=%d.%dC",
                      mb_solar_rtd_temp_c_x10() / 10,
                      std::abs(static_cast<int>(mb_solar_rtd_temp_c_x10() % 10)));
    } else {
        Serial.printf(" SOLAR=--");
    }
    Serial.printf(" flow=%d defrost=%d\n",
                  digitalRead(PIN_FLOW_SIG),
                  digitalRead(PIN_DEFROST));
}
