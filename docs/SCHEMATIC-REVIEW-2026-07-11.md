# Schematic/PCB review — CONTROL BOARD v1.0

**Rev A check:** 2026-07-11 (2η εικόνα — μετά διόρθωση DEFROST)

---

## ✅ Διορθώθηκε / OK

| Block | Κατάσταση |
|-------|-----------|
| **DEFROST_SIG** | **H2 pin 12** → GPIO14 ✓ (διόρθωση από H1-12) |
| **I2C bus** | U1 + CN5 + H1 — **ίδιο net** `I2C_SDA`/`I2C_SCL` ✓ |
| **I2C → ESP32** | PCB: **H1-3 (IO22/SCL)** · **H1-6 (IO21/SDA)** ✓ |
| **CN_DEFROST** | 3V3 · SING · GND |
| **CN1 / MCP41050** | SPI + AC_POT ✓ |
| **CN3/4/10** | DS18_DATA → H1-13 (IO4) · R13 · 100 nF ✓ |
| **CN_PANEL** | PANEL_TX/RX → H2-9 / H2-8 ✓ |
| **Τροφοδοσία** | F1 · 5V_PRE · C1/C2 · D2 ✓ |
| **Fan spoofer** | Daughterboard ζώνη ✓ |

---

## ⚠️ Έλεγξε πριν fab

### 1. R1 στο DEFROST — πρέπει pull-**UP**

**Σωστό:** `3V3` — **R1 4,7 kΩ** — `DEFROST_SIG` — (opto) — GND  

**Όχι** pull-down προς GND (θα κρατάει το σήμα LOW).

### 2. CN5 — σειρά pin (silk)

**PCB (4 pin):** GND · 3V3 · SDA · SCL (κάτω→πάνω)  

**Schematic CN5 (5P):** EN · SCL · SDA · 3V3 · GND  

- Nets προς ESP32 **σωστά**
- **EN:** OK αν EndPoint onboard έχει EN → 3V3 μόνιμα
- **Σημείωσε στο silk** τη σειρά pin για το πεδίο (διαφορετική από doc PR — OK αν το PCB είναι η αλήθεια)

### 3. U1 DS3231 — 5V_ESP

OK αν module **5 V**. I2C pull-ups σε **3V3** — συνήθως OK.

### 4. Qwiic EndPoint

Έλεγξε footprint **COM-16988** δίπλα CN5 στο PCB (RJ45 προς outdoor SHT40).

---

## ⏳ Ακόμα λείπουν

| Στοιχείο |
|----------|
| **3× διπλά ρελέ** K1–K6 + ζώνη 230 V |
| **F2 1,5 A → 5V_AUX** |
| **CN_BETA** 4P (GPIO17 TX / GPIO16 RX) |
| **Διαγραφή** CN2 CT · CN6 · CN7/8 (αν υπάρχουν ακόμα) |

---

## Pin map σύνοψη (rev A · DevKit 38)

| Net | Header | GPIO |
|-----|--------|------|
| I2C_SCL | H1-3 | 22 |
| I2C_SDA | H1-6 | 21 |
| DS18_DATA | H1-13 | 4 |
| DEFROST_SIG | **H2-12** | **14** |
| PANEL_TX | H2-9 | 25 |
| PANEL_RX | H2-8 | 33 |
| BETA_TX | H1-11 | 17 |
| BETA_RX | H1-12 | 16 |
| POT_CS / SPI | H2-15, H1-2, H1-11, H1-18 | 13, 23, 18, 23 |

---

## Verdict

**Για τα κλειδωμένα connectors: OK** (μετά DEFROST → H2-12).  

**Πριν παραγγελία:** R1 pull-up · CN5 silk · ρελέ · F2 · CN_BETA · EndPoint.

**Προηγούμενο review:** ημερομηνία 1ης εικόνας — DEFROST ήταν στο H1-12 (διορθώθηκε).
