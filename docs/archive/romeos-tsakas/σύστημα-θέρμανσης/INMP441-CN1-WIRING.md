# INMP441 → CN1 μητρικής — χάρτης καλωδίωσης

**Module:** INMP441 breakout (6 pin, όπως στη φωτό σου)  
**Κλέμα πλακέτας:** CN1 (6 pin, δίπλα στο «MIC»)

Οι ονομασίες **δεν ταιριάζουν 1:1** — είναι **ίδια σήματα, διαφορετικά labels**. Σύνδεσε **από pin αριθμό**, όχι από όνομα.

---

## 1. Module INMP441 (η πλακέτα σου)

| Pin module | Ίδιο με… | Τι είναι |
|------------|----------|----------|
| **VDD** | 3V3 | Τροφοδοσία **3,3 V** |
| **GND** | GND | Γείωση |
| **SCK** | BCLK | I2S bit clock |
| **WS** | LRCK | I2S word select (L/R frame) |
| **SD** | DOUT | I2S δεδομένα (έξοδος mic) |
| **L/R** | — | Κανάλι: **GND = αριστερό** (standard) |

---

## 2. CN1 μητρικής (pin 1 = πάνω)

| Pin CN1 | Στο silkscreen PCB σου | Πραγματικό σήμα | → Module INMP441 |
|---------|------------------------|-----------------|------------------|
| **1** | «SCL» ⚠️ | **3V3** (όχι I2C!) | **VDD** |
| **2** | GND | GND | **GND** |
| **3** | I2S_BCLK | BCLK | **SCK** |
| **4** | I2S_LRCK | WS / LRCLK | **WS** |
| **5** | I2S_SD | Data in (ESP) | **SD** |
| **6** | GND | GND | **L/R** |

⚠️ **Pin 1 γράφει «SCL» στο PCB αλλά είναι 3V3** — λάθος/παλιό label στο silkscreen. **Μην** το συνδέσεις σε I2C SCL (GPIO22).

**Πριν βάλεις VDD:** multimeter CN1 pin **1–2** → πρέπει **~3,3 V** (όχι 5 V).

---

## 3. Πίνακας «ίδιο πράγμα, άλλο όνομα»

| Module (INMP441) | CN1 (PCB) | ESP32 GPIO (v1.0 PCB) |
|------------------|-----------|------------------------|
| SCK | I2S_BCLK (pin 3) | **GPIO12** (H2-13) |
| WS | I2S_LRCK (pin 4) | **GPIO26** (H2-10) |
| SD | I2S_SD (pin 5) | **GPIO27** (H2-11) |
| VDD | pin 1 (3V3) | — |
| GND | pin 2 ή 6 | — |
| L/R | pin 6 (GND) | — |

---

## 4. Σύνδεση με καλώδια (dupont)

```
CN1-1 (3V3)  ──────►  VDD
CN1-2 (GND)  ──────►  GND
CN1-3 (BCLK) ──────►  SCK
CN1-4 (LRCK) ──────►  WS
CN1-5 (SD)   ──────►  SD
CN1-6 (GND)  ──────►  L/R
```

Η τρύπα/κενό **πάνω στο mic** (πίσω πλευρά) = είσοδος ήχου — **μην την καλύπτεις**.

---

## 5. Firmware (ήδη στο pin_map.h)

```cpp
PIN_MIC_I2S_BCLK = 12;  // CN1 pin 3
PIN_MIC_I2S_WS   = 26;  // CN1 pin 4
PIN_MIC_I2S_SD   = 27;  // CN1 pin 5
```

⚠️ GPIO26/27 = REL_K2/K3 στο ίδιο PCB — σύγκρουση hardware.

Πηγή lock: `romeos-display-v4/μητρική πλακέτα/MIC_PCB_LOCK_2026-05-08.md`

---

## 6. LCD γραμμή 3 (bench)

| Ένδειξη | Σημαίνει |
|---------|----------|
| `MIC wait signal...` | I2S OK, σιωπή |
| `MIC idle rms=XXX` | Δεδομένα, χαμηλός ήχος |
| `MIC HEAR rms=XXX` | Άκουσε φωνή/χτύπημα |
| `CMD: <κείμενο>` | Εντολή που καταχώρησε ο επεξεργαστής |

**Serial @ 115200:** `MIC DIAG` · `MIC CMD ανάψε θέρμανση` · `MIC HELP`

---

## 7. Επόμενο βήμα

1. Σύνδεσε όπως πάνω (6 καλώδια).
2. Reset ESP32.
3. Bench test I2S — firmware ακολουθεί.

---

*2026-06-08 — Romeos CONTROL BOARD v1.0*
