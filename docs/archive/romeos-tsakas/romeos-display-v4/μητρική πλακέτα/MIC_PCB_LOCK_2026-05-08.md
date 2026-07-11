# Κλείδωμα μικροφώνου INMP441 — μητρική (CN1 + H2)

**Ημερομηνία:** 2026-05-08 (τελικό — από φωτό ESP32 38-pin + schematic H1/H2 + CN1).

---

## 1. Κλέμα μικροφώνου — **CN1** (6 pin)

Part: **ZX-XH2.54-6PZZ-GW** (ή ισοδύναμο 2.54 mm 1×6).

| Pin CN1 | Net (σχέδιο) | Silkscreen PCB* | → INMP441 module |
|---------|--------------|-----------------|------------------|
| **1** | `3V3` | μπορεί να γράφει «SCL» ⚠️ | **VDD** |
| **2** | `GND` | GND | **GND** |
| **3** | `MIC_I2S_BCLK` | I2S_BCLK | **SCK** |
| **4** | `MIC_I2S_WS` | I2S_LRCK | **WS** |
| **5** | `MIC_I2S_SD` | I2S_SD | **SD** |
| **6** | `GND` | GND | **L/R** (αριστερό κανάλι) |

\* Το silkscreen «SCL» στο pin 1 είναι **λάθος label** — το net είναι **3V3**. Βλ. `sistema-thermansis/INMP441-CN1-WIRING.md`.

---

## 2. Πού συνδέονται τα nets στο ESP — **CONTROL BOARD v1.0** (PCB 22/03/2026)

Με **USB του DevKit προς τα κάτω**, **αριστερή σειρά = H2**:

| Net CN1 | GPIO | **H2 pin** | Σημείωση |
|---------|------|------------|----------|
| `I2S_BCLK` (pin 3) | **GPIO12** | **13** (IO12) | R31 |
| `I2S_LRCK` (pin 4) | **GPIO26** | **10** (IO26) | R30 → GND |
| `I2S_SD` (pin 5) | **GPIO27** | **11** (IO27) | |

⚠️ **GPIO26/27 = `REL_K2` / `REL_K3`** στο ίδιο PCB — **σύγκρουση** με ρελέ· για mic bench το firmware **δεν** οδηγεί K2/K3 ως OUTPUT.

*(Παλιό σχέδιο 2026-05-08: BCLK=GPIO2, WS=GPIO12, SD=GPIO0 — **δεν** ταιριάζει με το τυπωμένο v1.0.)*

---

## 3. Αντιστάσεις (υποχρεωτικά στο PCB)

| Ref | Τιμή | Σύνδεση |
|-----|------|---------|
| **R30** (`R_MIC_WS`) | 10 kΩ | `I2S_LRCK` (GPIO26) → GND |
| **R31** (`R_MIC_BOOT`) | 10 kΩ | `I2S_BCLK` (GPIO12) — δες schematic |

Προαιρετικό: **100 nF** μεταξύ CN1 pin 1–2.

---

## 4. Routing EasyEDA (checklist)

- [ ] CN1 pin 3 → trace → **H2-16**
- [ ] CN1 pin 4 → trace → **H2-13**
- [ ] CN1 pin 5 → trace → **H2-17**
- [ ] CN1 pin 1 → `3V3`, pin 2 & 6 → `GND`
- [ ] R_MIC_WS, R_MIC_BOOT κοντά στο H2
- [ ] **Όχι** σύνδεση H2-18
- [ ] ERC/DRC

---

## 5. Firmware (`pin_map.h`)

```cpp
PIN_MIC_I2S_BCLK = 12;  // CN1-3 → H2-13
PIN_MIC_I2S_WS   = 26;  // CN1-4 → H2-10
PIN_MIC_I2S_SD   = 27;  // CN1-5 → H2-11
```

---

## 6. BOM

**2×** ίδιο module INMP441 — 1× στο **CN1** μητρικής, 1× στον θερμοστάτη 7″ (ξεχωριστή σύνδεση εκεί).
