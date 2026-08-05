/*
 * Quick bench: ALL documented relays ON (active LOW).
 * K1–K6 = GPIO 25,26,27,32,33,17 (motherboard pin_map).
 */
#include <Arduino.h>

static const int kRelays[] = {25, 26, 27, 32, 33, 17};

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("=== RELAY BENCH: K1-K6 ALL ON (LOW) ===");
    for (int p : kRelays) {
        pinMode(p, OUTPUT);
        digitalWrite(p, LOW);
    }
    Serial.println("Held ON — check LED-K1..K6 + module LEDs");
}

void loop()
{
    delay(1000);
}
