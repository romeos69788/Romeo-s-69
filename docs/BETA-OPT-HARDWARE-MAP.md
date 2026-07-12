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

### CN1 — **OPT1-CURRENT** (2P) — **SCT-013** (1 V output)

**Module (χρήστης):** [SCT-013 AliExpress 1005007531832172](https://www.aliexpress.com/item/1005007531832172.html)  
**Τύπος:** split-core CT · **έξοδος τάσης** (εσωτερικό burden) · **όχι** mA έξοδος  
**Φυσικό:** μπλε clamp · καλώδιο ~1 m · **βύσμα 3,5 mm TRS** στο τέλος

| Ετικέτα CT | Έκδοση | Κλίμακα |
|------------|--------|---------|
| **20 A / 1 V** (φωτο χρήστη) | 1 V RMS @ 20 A | **50 mV / A** |
| **10 A / 1 V** (link cart) | 1 V RMS @ 10 A | **100 mV / A** |

**→ Διάλεξε μία έκδοση** πριν lock BOM · για γραμμή HP/outdoor συνήθως **20 A / 1 V** · **μην** παραγγείλεις δεύτερο CT (βλ. shopping note).

**Σύνδεση στο PCB (CN1 2P):**

| CN1 pin | Silk | Σύνδεση |
|---------|------|---------|
| **1** | **SIG** | Ένα καλώδιο CT (μετά **C_CT1**) → bias → **GPIO35** |
| **2** | **GND** | Άλλο καλώδιο CT → **GND** |

**Πεδίο:** κόψε TRS jack **ή** βάλε **3,5 mm socket** στο κουτί — στο PCB μένει **2P** screw terminal.

**Κύκλωμα bias AC → ADC (ESP32 GPIO35):**

```
3V3 ── R_CT1 10k ──┬── OPT_CURRENT_ADC ── GPIO35 (H2-6)
                    │
                   R_CT2 10k
                    │
                   GND

CN1-1 (SIG) ── C_CT1 10µF ──► κόμβος OPT_CURRENT_ADC
CN1-2       ───────────────► GND
```

| Ref | Τιμή | Ρόλος |
|-----|------|-------|
| **R_CT1** (**R13** στο schematic χρήστη) | **10 kΩ** | 3V3 → κόμβος ADC |
| **R_CT2** (**R14**) | **10 kΩ** | Κόμβος ADC → GND |
| **C_CT1** (**C6**) | **10 µF** (ηλεκτρολυτικός) | CN1-1 (CT) → κόμβος ADC · **+** προς CT |

**Σύνδεσεις (όπως στο EasyEDA χρήστη):**

| Από | → | Προς |
|-----|---|------|
| **H2-1** | `3V3` | **R13** → **`OPT_CURRENT_SIG`** |
| **R14** | | **`OPT_CURRENT_SIG`** → **GND** |
| **H2-6** | | **`OPT_CURRENT_SIG`** (= **GPIO35**) |
| **CN1-1** | **SIG** | **C6 (+)** → **C6 (−)** → **`OPT_CURRENT_SIG`** |
| **CN1-2** | **GND** | **GND** (= **H1-1**) |

```
3V3 (H2-1) ── R13 10k ──┬── OPT_CURRENT_SIG ── H2-6 (GPIO35)
                        │
                       R14 10k ── GND

CN1-1 ── C6 10µF (+→−) ──► OPT_CURRENT_SIG
CN1-2 ── GND
```

- **Μόνο ένας** αγωγός μέσα στο CT (L **ή** N — **όχι** και τα δύο).
- Έξοδος CT = **AC** · firmware (αργότερα): **πολλαπλά δείγματα** → **RMS** (50 Hz Ελλάδα).
- **Rev A firmware:** `#define ROMEOS_BETA_OPT_CURRENT 0` — κύκλωμα onboard · κώδικας off.
- **Όχι** επιπλέον burden resistor (το **1 V** model το έχει μέσα).

**Firmware (αργότερα):** `I_rms = V_rms × (20 ή 10)` ανά έκδοση CT.

---

### CN2 + CN3 — **OPT2-WATER-T** / **OPT3-WATER-T** (3P each)

**Αισθητήρης:** **DS18B20** (1-Wire) · **κοινό bus** στα δύο κλέματα (όπως Alpha CN3/4/10).

| Pin | Silk | Net | Σύνδεση |
|-----|------|-----|---------|
| **1** | **3V3** | **3V3** | **H2-1 · 3V3** |
| **2** | **GND** | **GND** | **H1-1 / H2-14** |
| **3** | **SING** | **`BETA_DS18_DATA`** | **H1-13 · GPIO4** (κοινό και στα δύο) |

*(Σειρά pin **κλειδωμένη** στο schematic χρήστη: **3V3 · GND · SING** — όχι GND·3V3·SING.)*

**Pull-up (μία φορά στο bus):**

| Ref | Τιμή | Σύνδεση |
|-----|------|---------|
| **R9** (**R15** alt.) | **4,7 kΩ** | **`BETA_DS18_DATA` → 3V3** |

**Optional (στο schematic χρήστη — OK):**

| Ref | Τιμή | Σύνδεση |
|-----|------|---------|
| **C1, C2** | **100 nF** | **`BETA_DS18_DATA` → GND** (decoupling bus) |

⚠️ **Όχι R13** — η **R13** είναι ήδη 10 kΩ για CN1 (CT).

**Σύνδεσεις:**

| Από | → | Προς |
|-----|---|------|
| **CN2 pin 3** | **SING** | **`BETA_DS18_DATA`** |
| **CN3 pin 3** | **SING** | **`BETA_DS18_DATA`** (ίδιο net) |
| **H1-13** | | **`BETA_DS18_DATA`** (= **GPIO4**) |
| **R15** 4,7 kΩ | | **`BETA_DS18_DATA` → 3V3** |
| **CN2/3 pin 2** | **3V3** | **3V3** (H2-1) |
| **CN2/3 pin 1** | **GND** | **GND** |

```
3V3 (H2-1) ──┬── CN2 pin 2 ── CN3 pin 2
             │
            R15 4k7
             │
             └── BETA_DS18_DATA ── H1-13 (GPIO4)
                      ▲              ▲
                 CN2 pin 3      CN3 pin 3 (SING)

CN2/3 pin 1 ── GND
```

**Silk PCB:**

| Κλέμα | Block silk |
|-------|------------|
| **CN2** | **OPT2-WATER-T** |
| **CN3** | **OPT3-WATER-T** |

**Firmware rev A:** `#define ROMEOS_BETA_OPT_DS18 0` — hardware wired · κώδικας off.

**Firmware (αργότερα):** OneWire στο **GPIO4** · έως **2 probes** (CN2 + CN3) · ξεχωριστά ROM IDs.

---

### CN4 — **OPT4-WATER-FLOW** (3P) — **YF-B10** (Hall brass)

**Module (χρήστης):** [YF-B10 flow AliExpress 1005005588692911](https://www.aliexpress.com/item/1005005588692911.html)  
**Τύπος:** Hall effect · ορείχαλκος · **3/4"** (επιλογή χρήστη) · **2–50 L/min** (ετικέτα)  
**Καλώδιο module (3 wires):**

| Χρώμα | Σύνδεση CN4 |
|-------|-------------|
| **Κόκκινο** | **5V** (`5V_ESP`) |
| **Μαύρο** | **GND** |
| **Κίτρινο** | **PULSE** (`BETA_FLOW_SIG`) |

**Σειρά pin CN4 (lock silk PCB):**

| Pin | Silk | Net | Σύνδεση ESP32 |
|-----|------|-----|---------------|
| **1** | **GND** | **GND** | GND |
| **2** | **5V** | **5V_ESP** | **μετά F1** (H2-19 rail) |
| **3** | **PULSE** | **`BETA_FLOW_SIG`** | **H2-5 · GPIO34** |

**Pull-up (open-collector Hall — YF-B10):**

| Ref | Τιμή | Σύνδεση |
|-----|------|---------|
| **R12** (ή **R_FLOW**) | **10 kΩ** | **`BETA_FLOW_SIG` → 3V3** |

⚠️ Pull-up σε **3V3** — **όχι 5V** στο GPIO34 (max 3,3 V input).

**Σύνδεσεις:**

```
5V_ESP ── CN4 pin 2 (5V) ── κόκκινο module
GND    ── CN4 pin 1 ── μαύρο module

3V3 ── R12 10k ── BETA_FLOW_SIG ── H2-5 (GPIO34)
                        ▲
                   CN4 pin 3 ── κίτρινο module
```

**Πεδίο:** βέλος στο module = **κατεύθυνση ροής** · μην αντιστρέψεις.

**Firmware rev A:** `#define ROMEOS_BETA_OPT_FLOW 0` — hardware wired · pulse count off.

**Firmware (αργότερα):** `attachInterrupt` GPIO34 · pulses/L → L/min (calibration ανά probe).

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
| `OPT_CURRENT_ADC` | Μετά bias (GPIO35) · πριν = CN1 SIG |
| `REL_K1` … `REL_K8` | Έξοδοι ρελέ (block 5) |

---

## Checklist schematic (πριν lock block 4)

- [ ] Silk **OPT1-CURRENT** … **OPT4-WATER-FLOW**
- [ ] CN2/CN3 pin order **GND · 3V3 · SING**
- [ ] **CN2/CN3:** **R15** 4,7 kΩ · **`BETA_DS18_DATA` → 3V3** · **H1-13 GPIO4**
- [ ] CN4 **5V_ESP** (όχι 3V3) στο pin τροφοδοσίας flow
- [ ] **CN1 SCT-013:** **R_CT1/R_CT2** 10 kΩ · **C_CT1** 10 µF · → **GPIO35**
- [ ] **BETA_FLOW_SIG** → **GPIO34**
- [ ] **Δεν** μοιράζονται GPIO με **REL_*** / UART

---

## Σχετικά

- [`BETA-REV-A-REVIEW-2026-07-12.md`](BETA-REV-A-REVIEW-2026-07-12.md)
- [`beta/README.md`](../beta/README.md)
