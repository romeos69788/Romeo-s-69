# Schematic/PCB review — Alpha rev A (CONTROL BOARD v1.0)

**Τελικός έλεγχος:** 2026-07-11 · **μόνο μητρική Alpha**  
**Scope:** κλειδωμένα connectors + τροφοδοσία · **όχι** ρελέ / CN_BETA / F2 (άλλη φάση ή Beta)

**DRC:** **51/51 nets** · **0 errors** (PCB) — GND ratline resolved 2026-07-11 ✓

---

## ✅ Εγκρίθηκε — Alpha rev A

| Block | PCB/schematic |
|-------|----------------|
| **ESP32 H1/H2** | DevKit 38 ✓ |
| **CN1** | MCP41050 → ROOM-NTC ✓ |
| **CN3 / CN4 / CN10** | DS18 · R13 pull-up (CN10) · 100 nF ✓ |
| **CN5 OUTDOOR-I2C** | 4P silk **GND·3V3·SDA·SCL** · H1-3/6 ✓ |
| **Qwiic EN** | Μόνιμα **3V3** στο EndPoint ✓ |
| **U1 DS3231** | I2C κοινό bus · **5V_ESP** ✓ |
| **CN_DEFROST** | 3V3 · SING · GND · **R1 4,7k pull-up** ✓ |
| **DEFROST_SIG** | **H2-12 → GPIO14** ✓ |
| **CN_PANEL (7″)** | PANEL_TX/RX → H2-9/8 ✓ |
| **BUZZER (U2)** | ✓ |
| **Τροφοδοσία** | 12V module · F1 · 5V_PRE/5V_ESP · C1/C2 · D2 ✓ |
| **Fan spoofer** | Daughterboard ζώνη ✓ |
| **Αφαιρέθηκαν** | CN2 CT · CN6 · CN7/8 — **δεν φαίνονται** ✓ |

**Verdict: OK για fab** — μητρική Alpha rev A (connectors).

**Τελικός έλεγχος πριν κλείδωμα:** [`ALPHA-REV-A-FINAL-LOCK-2026-07-11.md`](ALPHA-REV-A-FINAL-LOCK-2026-07-11.md)

---

## CN5 — τελική σειρά pin (PCB 4P)

| CN5 pin | Silk | Net |
|---------|------|-----|
| **1** | **GND** | GND |
| **2** | **3V3** | 3V3 |
| **3** | **SDA** | I2C_SDA → H1-6 (GPIO21) |
| **4** | **SCL** | I2C_SCL → H1-3 (GPIO22) |

**EN:** όχι στο κλέμα — **3V3 μόνιμα** στο Qwiic EndPoint module.

---

## Pin map Alpha (DevKit 38 · USB κάτω)

| Net | Header | GPIO |
|-----|--------|------|
| I2C_SCL | H1-3 | 22 |
| I2C_SDA | H1-6 | 21 |
| DS18_DATA | H1-13 | 4 |
| DEFROST_SIG | H2-12 | 14 |
| PANEL_TX | H2-9 | 25 |
| PANEL_RX | H2-8 | 33 |
| POT_CS | H2-15 | 13 |
| SPI_SCK / MOSI / MISO | H1-11 / H1-2 / H1-8 | 18 / 23 / 19 |

**H1-11 / H1-12** (GPIO17/16): ελεύθερα — UART προς Beta **άλλη φάση**.

---

## Εκτός scope Alpha (άλλη φάση / Beta)

| Στοιχείο | Σημείωση |
|----------|----------|
| Ρελέ K1–K6 | Όχι σε αυτό το PCB |
| F2 / 5V_AUX | Όχι σε αυτό το PCB (φάση 2) |
| CN_BETA | Β’ μητρική / καλώδιο αργότερα |

---

## PCB layout — GND ratline (U1 DIGITAL CLOCK) — ✓ RESOLVED

**Ήταν:** Design Manager → **Nets 50/51** · **GND** κόκκινο X · ratline **U1 GND** → **ESP32 GND**.

**Τώρα (2026-07-11):** DRC **51/51 nets** · **0 errors** — χρήστης επιβεβαίωσε ✓

**Fab:** OK για παραγγελία Alpha rev A (connectors).

**Troubleshooting (αν ξαναεμφανιστεί):** δες μηνύματα 25–29 ΣΥΖΗΤΗΣΗ.md — συχνά **GND vs GND1** στο schematic ή **λάθος pad** στο U1 footprint.

---

## Ιστορικό review

1. DEFROST ήταν H1-12 → διορθώθηκε σε **H2-12** ✓  
2. I2C shared bus — επιβεβαιώθηκε ✓  
3. R1 pull-up · CN5 silk · EN · U1 5V — επιβεβαιώθηκαν από χρήστη ✓
