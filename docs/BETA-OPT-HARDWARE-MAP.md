# Beta rev A — OPT1–OPT4 hardware map (wired · firmware standby)

**Ημερομηνία:** 2026-07-12  
**Απόφαση χρήστη:** Οι προαιρετικές κλέμες **συνδέονται πλήρως** στο ESP32 (αντιστάσεις, nets, GPIO) · **rev A firmware δεν τις ενεργοποιεί** — standby για αργότερα.

**Σύγκριση Alpha DS18:** [`ALPHA-REV-A-FINAL-LOCK-2026-07-11.md`](ALPHA-REV-A-FINAL-LOCK-2026-07-11.md) · UART: [`BETA-ALPHA-UART-LINK.md`](BETA-ALPHA-UART-LINK.md)

---

## Φιλοσοφία

| Επίπεδο | Rev A |
|---------|-------|
| **Schematic / PCB** | Πλήρης κύκλωμα · σωστά pin · pull-up / divider όπου χρειάζεται |
| **Firmware** | **Δεν** διαβάζει / **δεν** οδηγεί OPT · μόνο ρελέ + UART |
| **Πεδίο** | Μπορείς να αφήσεις headers **κενά** · το board είναι έτοιμο |

---

## Πίνακας κλέματος → ESP32

### CN1 — **OPT1-CURRENT** (2P)

Μελλοντικός αισθητήρας ρεύματος (Hall / CT module / analog out).

| CN1 pin | Silk | Net | Σύνδεση ESP32 |
|---------|------|-----|---------------|
| **1** | **SIG** | **OPT_CURRENT_SIG** | → κύκλωμα divider → **H2-6 · GPIO35** |
| **2** | **GND** | **GND** | **H1-1 / H2-14** |

**Κύκλωμα (standby-safe):**

```
OPT_CURRENT_SIG ──[ R1 ]──┬── GPIO35 (ADC)
                          │
                         [ R2 ]── GND
```

- **R1 / R2:** τελική τιμή **όταν** επιλεγεί module (π.χ. 0–1 V CT → bias 1,65 V · divider όπως archive CT).
- **Rev A:** GPIO35 = **input-only** · **χωρίς** module = ασφαλές (floating με weak bias αν θες — optional 100 kΩ προς GND).

**Firmware (αργότερα):** `analogRead(GPIO35)` · `#define ROMEOS_BETA_OPT_CURRENT 0` στο rev A.

---

### CN2 + CN3 — **OPT2-WATER-T** / **OPT3-WATER-T** (3P each)

Κοινό **OneWire** bus (όπως Alpha CN3/4/10).

| Pin | Silk | Net | Σύνδεση |
|-----|------|-----|---------|
| **1** | **GND** | **GND** | GND |
| **2** | **3V3** | **3V3** | **H2-1 · 3V3** (από ESP32 module) |
| **3** | **SING** | **BETA_DS18_DATA** | **H1-13 · GPIO4** |

**Κύκλωμα:**

- **R_OPT1** **4,7 kΩ** · **BETA_DS18_DATA → 3V3** (pull-up **μία φορά** στο bus — π.χ. κοντά CN2 ή κοντά ESP32).
- CN2-SING και CN3-SIGN **κοινό net** `BETA_DS18_DATA`.

**Firmware (αργότερα):** OneWire στο GPIO4 · `#define ROMEOS_BETA_OPT_DS18 0` στο rev A.

---

### CN4 — **OPT4-WATER-FLOW** (3P)

Μελλοντικός αισθητήρας ροής (π.χ. YF-S201 · pulse).

| Pin | Silk | Net | Σύνδεση ESP32 |
|-----|------|-----|---------------|
| **1** | **GND** | **GND** | GND |
| **2** | **5V** | **5V_ESP** | **μετά F1** (ίδιο rail με ρελέ) |
| **3** | **PULSE** | **BETA_FLOW_SIG** | **H2-5 · GPIO34** |

**Κύκλωμα (αν open-collector output):**

- **R_OPT2** **10 kΩ** · **BETA_FLOW_SIG → 3V3** (pull-up — αν το module δεν έχει εσωτερικό).

**Firmware (αργότερα):** `attachInterrupt` / pulse count στο GPIO34 · `#define ROMEOS_BETA_OPT_FLOW 0` στο rev A.

---

## GPIO — μην πειράζεις (ήδη κλειδωμένα)

| GPIO | Χρήση |
|------|--------|
| **16** | CN_ALPHA **RX** |
| **17** | CN_ALPHA **TX** |
| **4** | **BETA_DS18_DATA** (OPT2/3) |
| **34** | **BETA_FLOW_SIG** (OPT4) |
| **35** | **OPT_CURRENT_ADC** (OPT1) |

**Ρελέ K1–K8:** ξεχωριστά GPIO (block 5/7) — **όχι** 4, 16, 17, 34, 35.

---

## Net names (EasyEDA — πρόταση)

| Net | Περιγραφή |
|-----|-----------|
| `BETA_DS18_DATA` | OneWire OPT2 + OPT3 |
| `BETA_FLOW_SIG` | Pulse flow OPT4 |
| `OPT_CURRENT_SIG` | Πριν divider → GPIO35 |
| `REL_K1` … `REL_K8` | Έξοδοι ρελέ (block 5) |

---

## Checklist schematic (πριν lock block 4)

- [ ] Silk **OPT1-CURRENT** … **OPT4-WATER-FLOW**
- [ ] CN2/CN3 pin order **GND · 3V3 · SING**
- [ ] **R_OPT1** 4,7 kΩ pull-up DS18
- [ ] CN4 **5V_ESP** (όχι 3V3) στο pin τροφοδοσίας flow
- [ ] **OPT_CURRENT_SIG** → divider → **GPIO35**
- [ ] **BETA_FLOW_SIG** → **GPIO34**
- [ ] **Δεν** μοιράζονται GPIO με **REL_*** / UART

---

## Σχετικά

- [`BETA-REV-A-REVIEW-2026-07-12.md`](BETA-REV-A-REVIEW-2026-07-12.md)
- [`beta/README.md`](../beta/README.md)
