/**
 * Μητρική CONTROL BOARD v1.0 — σκελετός firmware
 *
 * Σκοπός: αρχικοποίηση pin, Serial, I2C/SPI· η λογική υδραυλικού / RF προς οθόνη
 * έρχεται σε επόμενα βήματα (βλ. romeos-design-notes.md).
 */

#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>

#include "mb_mqtt.h"
#include "mb_outdoor_dht.h"
#include "mb_uart_link.h"
#include "mb_wifi_link.h"
#include "pin_map.h"
#include "romeos_link_types.h"
#include "romeos_wifi_nvs.h"

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

/** Όλα τα REL_K* ως έξοδοι· αρχική κατάσταση HIGH (συχνά coil inactive). */
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
    const uint32_t now = millis();
    if (!force && (now - g_last_lcd_refresh_ms < 1000u)) {
        return;
    }
    g_last_lcd_refresh_ms = now;

    if (g_fault_code != FaultCode::None) {
        lcd_print_line(0, "ALARM ACTIVE");
        lcd_print_line(1, g_fault_text);
        lcd_print_line(2, "Check boiler room");
        lcd_print_line(3, "Romeos motherboard");
        return;
    }

    if (mb_outdoor_dht_has_live()) {
        const int16_t tx10 = mb_outdoor_dht_temp_c_x10();
        const int16_t hx10 = mb_outdoor_dht_humidity_rh_x10();
        char line1[21];
        char line2[21];
        std::snprintf(line1,
                      sizeof(line1),
                      "OUT T %2d.%01dC RH %2d%%",
                      tx10 / 10,
                      std::abs(static_cast<int>(tx10 % 10)),
                      hx10 / 10);
        std::snprintf(line2, sizeof(line2), "CN5 DHT -> Viewe 7in");
        lcd_print_line(0, "OUTDOOR SENSOR LIVE");
        lcd_print_line(1, line1);
        lcd_print_line(2, line2);
        lcd_print_line(3, "LCD2004 bring-up OK");
        return;
    }

    lcd_print_line(0, "CN5 DHT waiting...");
    lcd_print_line(1, "Reading outdoor air");
    lcd_print_line(2, "LCD2004 connected");
    lcd_print_line(3, "Romeos motherboard");
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
            set_fault(FaultCode::HeatPumpNoStart, "Heat pump OFF >5min");
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
            set_fault(FaultCode::SupplySensorNoSignal, "Supply sensor no sig");
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
                } else if (strcmp(g_rel_cmd_buf, "REL ALL OFF") == 0) {
                    relay_all_off();
                } else if (strcmp(g_rel_cmd_buf, "REL ALL ON") == 0) {
                    relay_all_on();
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

void setup()
{
    Serial.begin(kSerialBaud);
    delay(200);
    pin_map_log_header();
    Serial.println(F(
        "[boot] Ίδιο firmware πρωτεύον/εφεδρικό — SoftAP+UDP προς οθόνη (ανταλλαγή module)"));
    Serial.println(F("[boot] setup start"));
    Serial.printf("[link] sizeof(romeos_mb_to_display_v1_t)=%u\n",
                  static_cast<unsigned>(sizeof(romeos_mb_to_display_v1_t)));
    Serial.printf("[link] sizeof(romeos_display_to_mb_v1_t)=%u\n",
                  static_cast<unsigned>(sizeof(romeos_display_to_mb_v1_t)));

    init_relay_outputs();
#ifdef ROMEOS_MB_BENCH_RELAYS_ON
    relay_all_on();
    Serial.println(F("[rel_test] BENCH: all 6 relays ON at boot (held)"));
#endif
    init_buzzer_pin();
    init_sensor_inputs();
    mb_outdoor_dht_begin();
    init_i2c();
    init_status_lcd();
    init_spi();

    mb_uart_link_begin();
    mb_wifi_link_begin();
    mb_wifi_link_set_alarm_active(false);
    mb_mqtt_begin();
    refresh_fault_lcd(true);

    Serial.println(F("[boot] setup done — loop"));
    Serial.println(F("[rel_test] bench test: type REL HELP (115200)"));
}

void loop()
{
    poll_relay_test_serial();
    mb_outdoor_dht_poll();
    romeos_wifi_nvs_poll_serial();
    mb_uart_link_poll();
    mb_wifi_link_poll();
    mb_mqtt_poll();
    update_fault_logic();
    refresh_fault_lcd();

    static uint32_t last = 0;
    const uint32_t now = millis();
    if (now - last < 5000) {
        return;
    }
    last = now;
    if (mb_outdoor_dht_has_live()) {
        const int16_t tx10 = mb_outdoor_dht_temp_c_x10();
        Serial.printf("[tick] millis=%lu flow=%d defrost=%d out=%d.%dC\n",
                      static_cast<unsigned long>(now),
                      digitalRead(PIN_FLOW_SIG),
                      digitalRead(PIN_DEFROST),
                      tx10 / 10,
                      std::abs(static_cast<int>(tx10 % 10)));
    } else {
        Serial.printf("[tick] millis=%lu flow=%d defrost=%d out=--\n",
                      static_cast<unsigned long>(now),
                      digitalRead(PIN_FLOW_SIG),
                      digitalRead(PIN_DEFROST));
    }
}
