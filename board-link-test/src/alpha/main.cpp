/*
 * Alpha: PULSE 1..8 στη Beta (UART2 IO17 TX / IO16 RX).
 * Διαβάζει ACK από Serial2 → USB monitor.
 */
#include <Arduino.h>

static constexpr int PIN_BETA_TX = 17; /* → Beta RX */
static constexpr int PIN_BETA_RX = 16; /* ← Beta TX */
static constexpr uint32_t kBaud = 115200;
static constexpr uint32_t kPulseMs = 3000;
static constexpr uint32_t kGapMs = 400;

void setup()
{
    Serial.begin(115200);
    delay(400);
    Serial2.begin(kBaud, SERIAL_8N1, PIN_BETA_RX, PIN_BETA_TX);
    Serial.println();
    Serial.println("=== ALPHA link test: PULSE + wait ACK ===");
    Serial.println("Serial2 115200 TX=IO17 RX=IO16");
    delay(2000); /* Beta boot + chase */
}

void loop()
{
    for (int k = 1; k <= 8; k++) {
        while (Serial2.available() > 0) {
            (void)Serial2.read();
        }
        char msg[24];
        snprintf(msg, sizeof(msg), "PULSE %d\n", k);
        Serial2.print(msg);
        Serial.printf("[alpha] sent %s", msg);

        const uint32_t t0 = millis();
        bool got = false;
        char line[32];
        uint8_t len = 0;
        while (millis() - t0 < 800) {
            while (Serial2.available() > 0) {
                const char c = (char)Serial2.read();
                if (c == '\r') {
                    continue;
                }
                if (c == '\n') {
                    line[len] = '\0';
                    len = 0;
                    if (line[0] != '\0') {
                        Serial.printf("[alpha] got: %s\n", line);
                        got = true;
                    }
                    break;
                }
                if (len + 1 < sizeof(line)) {
                    line[len++] = c;
                } else {
                    len = 0;
                }
            }
            if (got) {
                break;
            }
        }
        if (!got) {
            Serial.println("[alpha] no ACK (check TX↔RX + GND on CN_ALPHA)");
        }
        delay(kPulseMs + kGapMs);
    }
    Serial.println("[alpha] cycle done — repeat in 2s");
    delay(2000);
}
