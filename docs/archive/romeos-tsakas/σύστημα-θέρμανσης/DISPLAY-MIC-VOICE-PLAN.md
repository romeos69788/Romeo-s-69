# Φωνή INMP441 — οθόνη 7″ (Viewe v10)

**Στόχος:** Ίδιο module INMP441 · εντολές «**μενού**» → `SCREEN_ID_MENU`, «**θέρμανση**» → `SCREEN_ID_HEAT`.

**Project:** `sistema-thermansis/romeos-display-v10/romeos-display-v10/`  
**UI runtime:** `romeos_eez_runtime.cpp` — `loadScreen(SCREEN_ID_*)`

---

## Φάση 1 — I2S bench (πρώτο)

1. Σύνδεση module (dupont) στο **ESP32-S3** της Viewe — **3 ελεύθερα GPIO** (όχι RGB/touch pins 19/20).
2. Port `mb_mic` logic από μητρική (IDF I2S burst) → `display_mic.cpp`
3. Serial: `peak`, `MIC HEAR` — επιβεβαίωση ότι **τουλάχιστον ένα module** δουλεύει

**Κλέματα πλακέτας (Viewe 7″ — από φωτο PCB):**

| Κλέμα | Πού είναι | Τι κάνει |
|-------|-----------|----------|
| **J3** | Header με ετικέτα **J3**, pins 42/41/40… LEDK/LEDA | **RGB οθόνης** — μόνο τροφή |
| **J2** | Άλλο header: 3.3V, RST, 4…19, TX, RX, GND | **Όλα τα σήματα mic** |

**Σύνδεση INMP441 (μόνο J2 + τροφή):**

| Module | **J2** (ετικέτα στο PCB) | GPIO |
|--------|--------------------------|------|
| VDD | **3.3V** | τροφή |
| GND | **GND** (κάτω) | |
| L/R | **GND** (ίδιο) | |
| SCK | **18** | GPIO18 |
| WS | **13** | GPIO13 |
| SD | **10** | GPIO10 |

*(10/13 = SD card — OK αν δεν έχεις κάρτα. Μην αγγίξεις **17** buzzer, **19/20** touch, **TX/RX**.)*

**J3 — μόνο αν χρειάζεσαι επιπλέον 3.3V/GND** (LEDA/LEDK = backlight, όχι για I2S).

---

## Φάση 2 — Navigation (χωρίς AI)

Προσωρινά για demo:

- Serial: `VOICE MENU` / `VOICE HEAT` → `loadScreen(...)`
- Ή απλό **peak threshold + button** «άκουσα φωνή» για UI test

---

## Φάση 3 — Λέξεις-κλειδιά

| Λέξη | Ενέργεια |
|------|----------|
| **μενού** | `loadScreen(SCREEN_ID_MENU)` |
| **θέρμανση** | `loadScreen(SCREEN_ID_HEAT)` |

Επιλογές engine (μελλοντικά):

- **ESP-SR** MultiNet (ελληνικά / custom commands)
- **Edge Impulse** keyword spotting
- Απλό MVP: χειροκίνητο match μετά από εγγραφή 3× κάθε λέξη

---

## Φάση 4 — UI feedback

- Εικονίδιο / label «άκουγα…» όταν `peak > threshold`
- Toast «Μενού» / «Θέρμανση» μετά αναγνώριση

---

## Σύνδεση με μητρική

Η μητρική **δεν** χρειάζεται mic για αυτό το bench. Link UDP/MQTT για θερμοκρασίες παραμένει.

---

**Τελευταία δοκιμή:** I2S bench OK · φωνή/UI **χωρίς ανταπόκριση** — **PAUSED** (2026-06-28).

---

## Κατάσταση (2026-06-28)

| Φάση | Κατάσταση |
|------|-----------|
| 1 I2S bench J2 | ✓ DONE |
| 2 Serial VOICE MENU/HEAT | κώδικας υπάρχει · **δεν επιβεβαιώθηκε** |
| 3 Λέξεις «μενού»/«θέρμανση» | **PAUSED** — χρειάζεται ESP-SR ή tuning |
| 4 UI feedback | **PAUSED** |

Handoff: [`SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md`](SESSION-HANDOFF_MIC-DISPLAY-PAUSED_2026-06-28.md)
