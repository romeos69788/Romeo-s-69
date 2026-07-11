# Handoff — INMP441 οθόνη 7″ (PAUSED)

**Ημερομηνία:** 2026-06-28  
**Απόφαση:** Σταματάμε προσωρινά **φωνή/mic στην οθόνη**. I2S bench OK · φωνητικές εντολές **δεν** δούλεψαν στο UI.

---

## Τι δουλεύει

| Στοιχείο | Κατάσταση |
|----------|-----------|
| UI v10 (EEZ), touch, Wi‑Fi, UDP → μητρική | OK |
| INMP441 καλωδίωση **J2** (SCK=18, WS=13, SD=10) | OK |
| I2S firmware `display_mic.cpp` | OK — Serial `MIC HEAR pk=…` (όχι pk=1) |
| Build τελευταίο | `2026-06-08-display-voice-beta` |

## Τι δεν δούλεψε (χρήστης)

- Πάτημα εικονιδίου **MIC** (κόκκινο) + «μενού» / «θέρμανση» → **καμία αντίδραση** στην οθόνη
- Beta logic (διάρκεια ομιλίας) **δεν επαρκεί** — χρειάζεται πραγματική αναγνώριση λέξεων (ESP-SR / Edge Impulse) ή καλύτερο I2S tuning

## Serial (επιβεβαιωμένο από agent)

- `[display_mic] MIC HEAR pk=2169xxx` / `2419xxx` — σταθερό «θόρυβο», όχi pk=1
- `[eez] MIC ON` → παλιό build έτρεχε **GPIO hunt ~30s** (διορθώθηκε στο voice-beta)
- Touch I2C errors στο log όταν τρέχει I2S (pins 19/20 — μην πειράζεις)

## Καλωδίωση (κλειδωμένη για επόμενη φορά)

**Μόνο J2** (όχι J3 για I2S):

| Module | J2 pin | GPIO |
|--------|--------|------|
| VDD | 3.3V | |
| GND + L/R | GND | |
| SCK | 18 | |
| WS | 13 | |
| SD | 10 | |

Μην αγγίζεις: **17** buzzer, **19/20** touch.

## Firmware (frozen)

- `romeos-display-v10/romeos-display-v10/src/display_mic.cpp`
- `include/display_mic_pins.h`
- `romeos_eez_runtime.cpp` — MIC toggle, `voice_cmd_handler` → `loadScreen`
- Serial test: `VOICE MENU`, `VOICE HEAT` (δεν επιβεβαιώθηκε από χρήστη)

## Επόμενο (όταν επιστρέψουμε)

1. Επιβεβαίωση `VOICE MENU` στο Serial → navigation OK χωρίς mic
2. Fix I2S **spread/dynamic range** (τώρα σχεδόν σταθερό peak)
3. **ESP-SR MultiNet** ή keyword model για «μενού» / «θέρμανση»
4. UI feedback «άκουγα…» στην οθόνη

---

**Project:** `sistema-thermansis/romeos-display-v10/romeos-display-v10/` · **COM3** · 115200
