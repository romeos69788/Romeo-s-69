# Alpha rev A — τελικός έλεγχος πριν κλείδωμα fab

**Ημερομηνία:** 2026-07-11  
**Πηγές:** schematic + PCB layout (EasyEDA · χρήστης)  
**DRC:** **51/51 nets** · **0 errors** ✓  
**Scope αυτού PCB:** μητρική Alpha · sensors + comm + τροφοδοσία · **όχι** ρελέ / CN_BETA / F2

**Εικόνες:** [`alpha-rev-a/images/`](alpha-rev-a/images/README.md) — export PNG πριν fab

---

## 1. Τι υπάρχει στο board (inventory)

| Block | Schematic | PCB | Κατάσταση |
|-------|-----------|-----|-----------|
| ESP32 DevKit 38 (H1/H2) | ✓ | ✓ | OK |
| 12 V in + buck 12→5 V | ✓ | ✓ | OK |
| F1 → 5V_PRE / 5V_ESP | ✓ | ✓ | OK |
| C1/C2 + D2 Zener + LED-7 | ✓ | ✓ | OK |
| **CN9** 5V out | ✓ | ✓ | OK |
| **MCP41050** (SPI pot) | ✓ | ✓ | OK |
| **CN1** ROOM-NTC (2P) | ✓ | ⚠ silk | Έλεγξε §3.1 |
| **CN3 / CN4 / CN10** DS18 | ✓ | ✓ | OK |
| **CN5** outdoor I2C | ✓ | ⚠ 4P/5P | Έλεγξε §3.2 |
| **U1** DS3231 (DIGITAL CLOCK) | ✓ | ✓ | OK · I2C κοινό |
| **CN_DEFROST** | ✓ | ⚠ pin order | Έλεγξε §3.3 |
| **CN_PANEL** 7″ Viewe | ✓ (U3) | ✓ | OK |
| **BUZZER** (U2) | ✓ | ✓ | OK |
| **AC FAN SPOOFER** zone | ✓ | ✓ | OK (daughterboard) |
| IN 220V AC terminal | — | ✓ | OK (ζώνη HV) |
| IN/OUT 12V DC | — | ✓ | OK |

---

## 2. Τι **δεν** πρέπει να υπάρχει (αποφάσεις rev A)

| Στοιχείο | Schematic | PCB | Verdict |
|----------|-----------|-----|---------|
| **CN2** CT / SCT-013 | ✗ | ✗ | ✓ σωστά αφαιρέθηκε |
| **CN6** MAX31865 / PT100 | ✗ | ✗ | ✓ |
| **CN7 / CN8** flow | ✗ | ✗ | ✓ |
| **Ρελέ K1–K6** | ✗ | ✗ | ✓ φάση 2 / Beta |
| **CN_BETA** UART | ✗ | ✗ | ✓ φάση 2 |
| **F2 / 5V_AUX** | ✗ | ✗ | ✓ φάση 2 |
| DHT22 / LCD 2004 | ✗ | ✗ | ✓ |

**Δεν λείπει κλέμα που είχαμε κλειδώσει** — εκτός από items φάσης 2 (ρελέ, Beta, F2).

---

## 3. ⚠ Έλεγξε πριν κλείδωμα (silk / ονομασίες)

### 3.1 CN1 — ROOM-NTC (κρίσιμο)

**Κλειδωμένο:** **2P** · silk **ROOM-NTC** · nets **AC_POT_B / AC_POT_W** από MCP41050.

**Στο PCB layout** φαίνεται silk **«A/C-SENSOR»** 3P (SING·GND·3V3) — **δεν** ταιριάζει με την απόφαση CN1.

**Ενέργεια (μία από τις δύο):**
- Αν το **CN1** είναι **2P** δίπλα στο MCP41050 → **διόρθωσε silk** σε **ROOM-NTC** (όχι A/C-SENSOR).
- Αν υπάρχει **πραγματικό 3P** αισθητήρα A/C → **αφαίρεσέ το** ή μετονόμασε (rev A **δεν** έχει A/C probe connector).

### 3.2 CN5 — outdoor I2C

**Κλειδωμένο (τελικό):** **4P silk** · **GND·3V3·SDA·SCL** · **EN hardwired 3V3** στο Qwiic EndPoint (όχι pin στο κλέμα).

**Στο PCB** φαίνεται **5P** (με EN / διπλό 3V3). Ηλεκτρικά OK αν EN→3V3 στο module· **ευθυγράμμισε silk** με 4P αν θες τεκμηρίωση fab.

### 3.3 CN_DEFROST — σειρά pin

**Κλειδωμένο:**

| Pin | Silk |
|-----|------|
| 1 | 3V3 |
| 2 | SING |
| 3 | GND |

**Στο PCB silk** φαίνεται **GND·SING·3V3** (διαφορετική σειρά). **Διόρθωσε silk** ή σημείωσε στο κουτί καλωδίωσης — λάθος σειρά = λάθος τροφοδοσία module.

### 3.4 R1 — DEFROST pull-up

**Κλειδωμένο:** **R1 4,7 kΩ pull-up** DEFROST_SIG → **3V3** (όχι pull-down).

Επιβεβαίωσε στο schematic ότι είναι pull-up (επιβεβαιώθηκε από χρήστη μήνυμα 23).

### 3.5 DS18 πυκνωτές

**100 nF** ανά κλέμα (όχι 100 µF) · **R13 4,7 kΩ pull-up μόνο CN10**.

---

## 4. Pin map — locked (ESP32 DevKit 38 · USB κάτω)

| Net | H1/H2 | GPIO | Σημείωση |
|-----|-------|------|----------|
| DS18_DATA | H1-13 | 4 | κοινό bus CN3/4/10 |
| I2C_SDA | H1-6 | 21 | U1 + CN5 |
| I2C_SCL | H1-3 | 22 | U1 + CN5 |
| DEFROST_SIG | H2-12 | 14 | CN_DEFROST |
| PANEL_TX | H2-9 | 25 | CN_PANEL |
| PANEL_RX | H2-8 | 33 | CN_PANEL |
| POT_CS | H2-15 | 13 | MCP41050 |
| SPI_SCK | H1-11 | 18 | MCP41050 |
| SPI_MOSI | H1-2 | 23 | MCP41050 |
| SPI_MISO | H1-8 | 19 | MCP41050 |
| BUZZER_ALARM | U2 | 15* | *επιβεβαίωσε net στο schematic |
| *(ελεύθερο)* | — | 5 | χωρίς MAX31865 |
| *(ελεύθερο)* | H1-11/12 | 17/16 | UART Beta · φάση 2 |
| *(ελεύθερο)* | — | 34 | χωρίς flow |
| *(ελεύθερο)* | — | 35 | χωρίς CT |
| *(ελεύθερο)* | — | 32,26,27,12 | ρελέ · φάση 2 |

---

## 5. Φάση 2 — **δεν** μπαίνουν τώρα (σκόπιμα)

| Στοιχείο | Γιατί όχι rev A |
|----------|-----------------|
| **3× διπλά ρελέ** K1–K6 | Beta board / rev B |
| **CN_BETA** 4P UART | Καλώδιο προς outdoor αργότερα |
| **F2 1,5 A → 5V_AUX** | Panel τώρα από **5V_ESP** (U3) — OK bench, χωρίς ξεχωριστό fuse |
| **AC opto** GPIO36 | Προαιρετικό · archive |

---

## 6. Προτάσεις (προαιρετικά — **όχι** υποχρεωτικά για fab)

| # | Πρόταση | Λόγος |
|---|---------|-------|
| 1 | Silk **«REV A»** + ημερομηνία | ιχνηλασιμότητα |
| 2 | Silk **GPIO16/17 «BETA UART»** κοντά H1-11/12 | μελλοντικό καλώδιο |
| 3 | **Test points** TP: 3V3, 5V_ESP, GND, DS18_DATA | bench debug |
| 4 | **Antenna keep-out** κάτω από ESP32 | Wi‑Fi |
| 5 | **Fiducials** (JLCPCB) | assembly |
| 6 | Export PNG → `docs/alpha-rev-a/images/` | ιστορικό repo |
| 7 | **Gerber preview** (JLCPCB viewer) | τελικός οπτικός έλεγχος |

**Δεν προτείνουμε** νέους αισθητήρες / κλέμες πέρα από τα κλειδωμένα — το design είναι πλήρες για rev A scope.

---

## 7. Checklist κλειδώματος

- [x] DRC **51/51** · **0 errors**
- [x] CN2 / CN6 / CN7 / CN8 **αφαιρέθηκαν**
- [x] CN3/4/10 DS18 · CN5 I2C · CN_DEFROST · CN_PANEL · U1 RTC · MCP41050 · Buzzer
- [ ] **CN1 silk** = ROOM-NTC 2P (όχι A/C-SENSOR 3P)
- [ ] **CN_DEFROST silk** σειρά pin 1-3V3 / 2-SING / 3-GND
- [ ] **CN5 silk** 4P (αν θες τεκμηρίωση)
- [ ] **100 nF** (όχι µF) στα DS18
- [ ] Export εικόνες → `docs/alpha-rev-a/images/`
- [ ] Gerber preview OK

---

## 8. Verdict

**OK για fab** — μητρική Alpha rev A (connectors + τροφοδοσία), **αφού** διορθωθούν τα **silk** του §3 (CN1, CN_DEFROST, προαιρετικά CN5).

Ρελέ · Beta · F2 → **άλλο PCB / φάση 2**.

---

## Σχετικά

- [`SCHEMATIC-REVIEW-2026-07-11.md`](SCHEMATIC-REVIEW-2026-07-11.md)
- [`ALPHA-REV-A.md`](ALPHA-REV-A.md)
- [`ΣΥΖΗΤΗΣΗ.md`](ΣΥΖΗΤΗΣΗ.md) — μηνύματα 7–30
