# Schematic/PCB review — CONTROL BOARD v1.0 (2026-07-11)

**Πηγή:** screenshot schematic + PCB layout (Romeos Tsakas)  
**DRC:** 0 errors (PCB) — **δεν** αντικαθιστά λογικό έλεγχο pin map

---

## ✅ Σωστά

| Block | Σχόλιο |
|-------|--------|
| **I2C bus** | `I2C_SDA` / `I2C_SCL` **ίδιο net** → U1 + CN5 + H1 — **σωστό** |
| **H1-3 / H1-6** | IO22 (SCL) / IO21 (SDA) — ταιριάζει με DevKit 38 (USB κάτω) |
| **CN_DEFROST** | 3V3 · SING · GND + **R1 4,7 kΩ** pull-up |
| **CN1** | MCP41050 → AC_POT → ROOM-NTC |
| **CN3/4/10** | DS18_DATA κοινό · **R13** pull-up · **100 nF** (C3/C11/C22) |
| **CN_PANEL (U3)** | PANEL_TX/RX → **H2-9 / H2-8** (IO25/IO33) |
| **Τροφοδοσία** | F1 · C1/C2 · D2 · LED-7 |
| **Fan spoofer** | Ξεχωριστή ζώνη daughterboard |

---

## ⚠️ Διόρθωση — κρίσιμο

### 1. DEFROST_SIG → λάθος pin (πιθανό)

**Στο schematic:** `DEFROST_SIG` → **H1 pin 12**

Σε **ESP32 DevKit 38** (silk USB κάτω, δεξιά σειρά H1):

| H1 pin | GPIO | Σωστό net (rev A) |
|--------|------|-------------------|
| **11** | **17** | BETA_TX (UART) |
| **12** | **16** | BETA_RX (UART) |
| — | — | — |
| **H2 pin 12** | **14** | **DEFROST_SIG** ✓ |

**→ Μετακίνησε `DEFROST_SIG` από H1-12 (ή H1-11) σε H2-12 (IO14 / D14).**

**Επιβεβαίωση:** multimeter από pad `DEFROST_SIG` στο pin του module που γράφει **14** (όχι 16/17).

---

### 2. CN5 — σειρά pin ≠ κλειδωμένο doc

**Κλειδωμένο (PR/docs):**

| Pin | Net |
|-----|-----|
| 1 | GND |
| 2 | 3V3 |
| 3 | SDA |
| 4 | SCL |
| 5 | EN → 3V3 |

**Στο schematic (τώρα):**

| Pin | Net |
|-----|-----|
| 1 | EN |
| 2 | SCL |
| 3 | SDA |
| 4 | 3V3 |
| 5 | GND |

**Nets προς ESP32 είναι σωστά** — αλλά **η σειρά στο κλέμα** είναι **διαφορετική**.  
**→ Ευθυγράμμισε silk/connector** με το πραγματικό καλώδιο Qwiic (ή ενημέρωσε doc αν η σειρά στο PCB είναι η τελική).

**PCB:** φαίνεται **4 pin** (GND/SCL/SDA/3V3) — έλεγξε αν **EN** πάει μόνιμα **3V3** στο EndPoint (OK) ή λείπει.

---

### 3. U1 (DS3231) — τροφοδοσία 5V_ESP

- Pin 5 = **5V_ESP** (όχι 3V3)
- **OK** αν το module σου είναι **5 V** variant
- I2C pull-ups προς **3V3** — συνήθως OK με ESP32
- Αν module είναι **μόνο 3,3 V** → άλλαξε σε **3V3**

---

## ⏳ Λείπουν (ήδη στο checklist rev A)

| Στοιχείο | Κατάσταση |
|----------|-----------|
| **Ρελέ K1–K6** (3× διπλά) | Δεν φαίνονται |
| **F2 → 5V_AUX** | Για CN_PANEL / ρελέ / buzzer |
| **CN_BETA** 4P | UART 16/17 |
| **SparkFun Qwiic EndPoint** | Έλεγξε αν είναι στο PCB δίπλα CN5 |
| **CN2 CT, CN6, CN7/8** | Πρέπει να **μην** υπάρχουν |

---

## 📋 Pin map H1/H2 — rev A (DevKit 38, USB κάτω)

**H2 (αριστερά, pin 1 πάνω):** 3V3, EN, VP(36), VN(39), 34, 35, 32, **33**, **25**, 26, 27, **14**, 12, GND, 13, …

**H1 (δεξιά, pin 1 πάνω):** GND, 23, **22**, TX, RX, **21**, GND, 19, 18, 5, **17**, **16**, **4**, 0, 2, 15, …

---

## Σύνοψη

| Προτεραιότητα | Θέμα |
|---------------|------|
| **🔴 Υψηλή** | DEFROST → **H2-12 (GPIO14)**, όχι H1-11/12 |
| **🟡 Μέτρια** | CN5 pin order — ευθυγράμμιση silk ↔ καλώδιο |
| **🟡 Μέτρια** | U1 5V vs 3V3 — έλεγχος module |
| **🟢 Χαμηλή** | I2C shared bus — **OK** |
| **⏳ Fab** | Ρελέ, F2, CN_BETA, Qwiic EndPoint |
