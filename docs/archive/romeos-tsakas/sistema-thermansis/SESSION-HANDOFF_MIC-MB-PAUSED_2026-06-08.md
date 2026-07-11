# Handoff — INMP441 μητρική (PAUSED) → οθόνη 7″

**Ημερομηνία:** 2026-06-08  
**Απόφαση:** Σταματάμε προσωρινά mic στη **μητρική**. Το ίδιο module δοκιμάζεται στην **οθόνη 7″** (Viewe v10). Επιστροφή στη μητρική αργότερα (PCB trace / CN1).

---

## Τι δουλεύει στη μητρική

| Στοιχείο | Κατάσταση |
|----------|-----------|
| CN4 DS18, CN5 DHT | OK |
| Solar PT100 | Paused (άλλο θέμα) |
| Wi‑Fi / MQTT / UDP → 7″ | OK |
| Mic firmware (I2S IDF, burst mode) | OK software |
| **Ήχος mic** | **FAIL** — `peak=1`, `raw0=0xFFFFFFFF` |

## Τι δοκιμάστηκε (hardware)

- 2+ modules INMP441, L/R → CN1-6, 3.29 V στο CN1
- 7 GPIO combos στο firmware — όλα `pk=1`
- Dupont bypass (αναφέρθηκε) — ίδιο αποτέλεσμα στο τελευταίο τσεκ

## Πιθανή αιτία (μητρική)

1. **PCB v1.0:** CN1 I2S_WS/SD μοιράζονται **GPIO26/27** με **REL_K2/K3** + LED → σύγκρουση + «φάρος» K2
2. **Trace CN1 → ESP** ενδεχομένως σπασμένο / λάθος net
3. ~~Modules OK — επαλήθευση με bench στην οθόνη~~ → **2026-06-28:** bench οθόνης δείχνει **I2S δεδομένα (όχι pk=1)** → **modules πιθανόν OK**, πρόβλημα **μητρική PCB/CN1**

## Firmware μητρικής (frozen)

- `motherboard-fw/src/mb_mic.cpp` — burst I2S, GPIO hunt
- `pin_map.h`: BCLK=12, WS=26, SD=27 (PCB v1.0)
- Μην αλλάζεις pinout χωρίς νέο PCB

## Επόμενο (μητρική — όταν επιστρέψουμε)

1. Dupont module → ESP **12/26/27** vs **2/12/0** (old doc)
2. Continuity CN1-3/4/5 → ESP pads
3. PCB rev2: mic σε GPIO **χωρίς** REL_K2/K3

---

**Project:** `sistema-thermansis/motherboard-fw` · COM7 · 115200
ermansis/motherboard-fw` · COM7 · 115200
