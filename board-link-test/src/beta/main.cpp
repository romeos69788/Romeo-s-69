/*
 * Beta — καθαρό link test (από την αρχή)
 *
 * Songle modules: active LOW
 *   οπλισμός = GPIO LOW  → ψείρα module ON + ρελέ ON
 *   idle      = GPIO HIGH → ψείρα module OFF
 *
 * LED-K μητρικής: στο PCB είναι αντίστροφα από τις ψείρες
 * (ανάβουν στο HIGH / σβήνουν στο LOW). Ίδιο net με IN ·
 * το firmware ΔΕΝ μπορεί να διορθώσει και τα δύο μαζί.
 * Σωστή ένδειξη οπλισμού προς το παρόν = ψείρα module.
 * Διόρθωση LED-K = αλλαγή κυκλώματος στο PCB (5V→R→LED→net).
 *
 * UART0 CN_ALPHA + Serial2 16/17 · PULSE 1..8 · 3s · ACK
 */
#include <Arduino.h>

static const int PIN[8] = {26, 33, 14, 27, 23, 13, 25, 12};

static void offAll()
{
    for (int i = 0; i < 8; i++) {
        pinMode(PIN[i], OUTPUT);
        digitalWrite(PIN[i], HIGH);
    }
}

static void onOne(int k0)
{
    offAll();
    if (k0 < 0 || k0 > 7) {
        return;
    }
    digitalWrite(PIN[k0], LOW);
}

static void chase()
{
    for (int i = 0; i < 8; i++) {
        onOne(i);
        delay(600);
    }
    offAll();
}

static void onPulse(int k, Stream &from)
{
    from.printf("ACK %d\n", k);
    Serial.printf("ACK %d\n", k);
    Serial2.printf("ACK %d\n", k);
    onOne(k - 1);
    delay(3000);
    offAll();
}

static void poll(Stream &s, char *b, uint8_t &n)
{
    while (s.available()) {
        char c = (char)s.read();
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            b[n] = 0;
            n = 0;
            int k = 0;
            if (b[0] && sscanf(b, "PULSE %d", &k) == 1 && k >= 1 && k <= 8) {
                onPulse(k, s);
            }
            continue;
        }
        if (n < 31) {
            b[n++] = c;
        } else {
            n = 0;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    delay(100);
    offAll();
    Serial.println("BETA clean: module LED = armed, LED-K PCB inverted (HW)");
    chase();
    offAll();
}

void loop()
{
    static char a[32], b[32];
    static uint8_t na, nb;
    poll(Serial, a, na);
    poll(Serial2, b, nb);
}
