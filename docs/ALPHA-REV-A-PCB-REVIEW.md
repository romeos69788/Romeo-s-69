# Alpha rev A — έλεγχος PCB/schematic (2026-07-11)

**Πηγή:** εικόνες EasyEDA Sheet_1 + PCB layout (χρήστης)  
**Σκοπός:** τι λείπει / τι διορθώνουμε **πριν** JLCPCB — αποφυγή 3ης αναθεώρησης

---

## Τι είναι ήδη καλά

| Block | Κατάσταση |
|-------|-----------|
| **ESP32 DevKit 38 pin** (H1/H2) | Σωστή βάση |
| **12 V shield + buck 12→5 V** (PCB) | Σωστή αλυσίδα (όπως archive) |
| **5V_PRE** φίλτρο C1/C2, D2 Zener, LED-7 | OK |
| **F1** → `5V_ESP` | OK (ένα κλάδο) |
| **MCP41050** + έξοδοι προς AC/NTC spoof | OK (λογική U5 archive) |
| **DS3231** (DIGITAL CLOCK) I2C | OK — κράτα χωρίς LCD 2004 |
| **BUZZER** | OK |
| **AC FAN SPOOFER** ως daughterboard | OK (ξεχωριστό module) |
| **Εισόδοι αισθητήρων** (CN1, CN3/4/10, CN5) | Κλειδωμένες — CN2/CN6/CN7/CN8 αφαιρούνται |

---

## Κρίσιμα — λείπουν (πρέπει να μπουν πριν fab)

### 1. Ρελέ K1–K6 (230 V modules)

**Δεν φαίνονται** στο PCB. Χωρίς αυτά η μητρική **δεν ελέγχει** τίποτα.

| Module | Nets | Λειτουργία |
|--------|------|------------|
| **RL-K1/K2** | REL_K1, REL_K2 | Κυκλοφορητές αδράνειας / μικρού κύκλου |
| **RL-K3/K4** | REL_K3, REL_K4 | Κύριος βρόχος + βάνα ηλιακού |
| **RL-K5/K6** | REL_K5, REL_K6 | 4 kW + spare |

- **3× διπλά** ρελέ modules 5 V (όπως archive §12.4.1).
- **IN-1 / IN-2 / VCC / GND** ανά module — silk όπως πεδίο.
- **230 V IN** ξεχωριστή ζώνη · creepage/clearance.
- **F2 1,5 A** → `5V_AUX` για ρελέ + buzzer + panel (τώρα λείπει δεύτερο fuse).

### 2. CN_PANEL — Viewe 7″ — **ΚΛΕΙΔΩΜΕΝΟ ✓ (2026-07-11)**

**Τοπικός πίνακας λεβητοστασίου** — **Viewe UEDX80480070E-WB-A** (7″, ESP32-S3) σε κουτί δίπλα στη μητρική.

**Κλέμα 4P** (JST-XH 2,54 mm · π.χ. **PA001-4P**):

| CN_PANEL pin | Silk | Net | ESP32 DevKit 38 |
|--------------|------|-----|-----------------|
| **1** | **GND** | **GND** | H1-13 / H1-19 / H2-14 |
| **2** | **5V** | **5V_AUX** | μετά **F2 1,5 A** |
| **3** | **TX** | **PANEL_TX** | **H2-9 / GPIO25** (D25) |
| **4** | **RX** | **PANEL_RX** | **H2-8 / GPIO33** (D33) |

**Πρωτόκολλο:**
- **UART2** Alpha · **115200** 8N1 · **όχι** I2C
- **GPIO25 / GPIO33** = panel μόνο — **όχι** ρελέ K1/K5 (K1→GPIO32, K5→GPIO13)

**Καλώδιο προς Viewe (crossover UART):**
- Alpha **PANEL_TX** → Viewe **RX**
- Alpha **PANEL_RX** ← Viewe **TX**
- **GND** κοινό · **5V_AUX** → τροφοδοσία panel (~**500 mA** peak)

**Σημείωση:** Το Viewe μπορεί επίσης **Wi‑Fi UDP/ESP‑NOW** προς Alpha (υπάρχον firmware) — το **CN_PANEL** είναι **ενσύρματο** backup / κουτί λεβητοστασίου.

**EasyEDA:** πρόσθεσε **CN_PANEL** 4P · net **5V_AUX** από **F2** · **μην** βάλεις ρελέ στο GPIO25/33.

### 3. CN_BETA — δεύτερη μητρική (outdoor / HP)

**Νέο κλέμα 4P** (καλώδιο προς Beta PCB):

| Pin | Net | ESP32 |
|-----|-----|-------|
| 1 | GND | — |
| 2 | BETA_TX | **GPIO17** (UART1 TX) |
| 3 | BETA_RX | **GPIO16** (UART1 RX) |
| 4 | NC | — |

- **Όχι** τροφοδοσία Beta από Alpha — η Beta έχει δικό της CN1 5V.
- Αν απόσταση > ~3 m → **rev B:** RS485 (2× MAX485EPA) αντί για raw UART.

### 4. ~~CT SCT-013 (HP-WATT)~~ — **ΑΦΑΙΡΕΘΗΚΕ (2026-07-11)**

- **CN2 / HP-WATT / CT:** **διέγραψε** από schematic + PCB.
- **Λόγος:** kWh HP από **WiFi ψηφιακό μετρητή** στον πίνακα της αντλίας — διπλή μέτρηση όχι απαραίτητη.
- **GPIO35** ελεύθερο.

### 5. ~~Ηλιακός — MAX31865 + PT100~~ — **ΑΦΑΙΡΕΘΗΚΕ (2026-07-11)**

- **CN6 / SOLAR-TEMP / MAX31865 (U8):** **διέγραψε** από schematic + PCB.
- **Λόγος:** ξεχωριστό υδραυλικό κύκλωμα ηλιακού + **DS18B20** επιστροφής (~2 m) στο bus **CN3/4/10** — όχι PT100 15 m στην στέγη.
- **GPIO5** (`SOLAR_CS`) ελεύθερο.
- **Πεδίο (εκτός PCB):** δοχείο διαστολής, PRV, μικρός κυκλοφορητής ανακυκλοφορίας · interlock με τριόδη + inverter pump.

### 5b. ~~Flow sensors CN7 / CN8~~ — **ΑΦΑΙΡΕΘΗΚΑΝ (2026-07-11)**

- **CN7 / CN8 / FLOW_SIG:** **διέγραψε** από schematic + PCB.
- **Λόγος:** κύριος βρόχος πάντα ίδιος · ροή από **σταθερή σκάλα** μεγάλου inverter κυκλοφορητή (μνήμη μετά power cycle) · COP tuning με **ΔT** DS18.
- **GPIO34** ελεύθερο.

### 5c. CN3 / CN4 / CN10 — DS18B20 νερού — **ΚΛΕΙΔΩΜΕΝΟ ✓**

- **3P:** 3V3 · GND · SING → **`DS18_DATA`** → **GPIO4**
- Pull-up **4,7 kΩ** μόνο **CN10** · **100 nF** ανά κλέμα · LED τροφοδοσίας 3V3→R→GND

### 6. CN_DEFROST — απόψυξη HP — **ΚΛΕΙΔΩΜΕΝΟ ✓ (2026-07-11)**

**Αρχικά σχεδιασμός:** Beta (HP outdoor board). **Μεταφορά στην Alpha** — η Beta μένει **κυρίως ρελέ**.

| CN_DEFROST pin | Silk | Net | ESP32 DevKit 38 |
|----------------|------|-----|-----------------|
| **1** | **3V3** | **3V3** | H2-1 |
| **2** | **SING** | **DEFROST_SIG** | **H2-12 / GPIO14** (D14) |
| **3** | **GND** | **GND** | H1-1 / H2-14 |

**Πεδίο:**
- **Module απόψυξης** (230 V → opto/LV) — τροφοδοσία AC **εκτός** PCB (ζώνη HV)
- Σήμα όταν **ενεργό πηνίο τριόδης βάνας** εξωτερικής HP = **απόψυξη**
- **R9 4,7 kΩ** pull-up **DEFROST_SIG** → **3V3** (στο PCB Alpha)

**Firmware (interlocks — αργότερα):**
- `DEFROST_SIG` ενεργό → **K2 OFF** · **K4** απομόνωση ηλιακού · **K3 ON** (κύριος κυκλοφορητής)

**Beta:** **όχι** CN_DEFROST — μόνο ρελέ HP/outdoor (UART προς Alpha αν χρειαστεί status).

### 7. ~~Εξωτερική θερμοκρασία (DHT)~~ → **CN5 Qwiic + SHT40** — **ΚΛΕΙΔΩΜΕΝΟ ✓**

- **CN5** 5P: GND · 3V3 · SDA · SCL · EN (EN → **3V3** / H2-1)
- **SparkFun QwiicBus EndPoint** onboard · RJ45 → outdoor EndPoint + **SHT40/41**
- **I2C_SDA/SCL** → GPIO21/22 · κοινό με DS3231
- **Όχι** DHT22 / `DHT_DATA`

### 8. AC opto (inverter feedback) — προαιρετικό αλλά χρήσιμο

- **GPIO36** · 2P ή 3P κλέμα «AC_OPTO» — όπως archive.

---

## Πιθανά λάθη — διόρθωσε πριν routing

### Α. Σύγκρουση ονομάτων CN1/CN2

| Στο schematic | Στο PCB layout |
|---------------|----------------|
| CN1/CN2 = έξοδοι **MCP41050** | CN1 = **ROOM-NTC** · **CN2 αφαιρέθηκε** (CT) |

**→ Μετονόμασε** π.χ.:
- MCP: **CN1 ROOM-NTC** (AC_POT_B/W)
- ~~CN_HP_CT~~ — **αφαιρέθηκε**

### Β. Πυκνωτές DS18 — 100 µF;

Στο schematic φαίνονται **100 µF** στο data line — **πιθανό λάθος**.

- Σωστό: **100 nF** προς GND (ή 4,7 µF max) + **4,7 kΩ** pull-up.
- **100 µF** στο 1-Wire = καθυστερήσεις / αποτυχία ανίχνευσης.

### Γ. DS18 — ένα bus ή πολλά;

Archive: **5× DS18B20** στο **ίδιο** 1-Wire (**GPIO4**, **μία** CN4 3P).

Τώρα: **3× WATER-TEMP** + άλλα — OK **αν**:
- όλα τα **SING** → **ίδιο net `DS18_DATA`** (GPIO4), **ή**
- ξεχωριστό GPIO ανά probe (σπατάλη pins — **όχι** συνιστάται).

**Πρόταση:** **CN4_WATER 3P** μόνο · 5 probes παράλληλα στο πεδίο.

### Δ. ~~Flow sensors CN7/CN8~~ — **αφαιρέθηκαν** (βλ. §5b)

### Ε. Qwiic EndPoint

- Επιπλέον I2C breakout — **OK για bench**, όχι απαραίτητο στο τελικό.
- Μην μοιράζει GPIO με ρελέ (παλιό v1.0: mic σύγκρουση K2/K3).

### Σ. F2 / 5V_AUX

- Σήμερα: F1 → `5V_ESP` μόνο.
- Χρειάζεται: **F2 1,5 A** → `5V_AUX` (ρελέ, buzzer, RTC VCC αν 5 V, **CN_PANEL**).

---

## Pin budget — πρέπει να κλειδώσει στο schematic

Κάνε **πίνακα H1/H2** με κάθε net πριν το fab. Προτεινόμενη αντιστοίχιση (ROMEOS 69 + archive):

| GPIO | Net | Σημείωση |
|------|-----|----------|
| 4 | DS18_DATA | 1-Wire |
| 5 | *(ελεύθερο)* | χωρίς MAX31865 rev A |
| 12 | REL_K4 | ή buzzer — **μία** χρήση |
| 13 | POT_CS / REL_K5 | MCP41050 CS |
| 14 | **DEFROST_SIG** | **CN_DEFROST** — όχι ρελέ |
| 15 | BUZZER_ALARM / REL_K6 | archive: buzzer=15 |
| 16 | BETA_RX | UART Beta (όχι DHT) |
| 17 | BETA_TX | |
| 18 | SPI_SCK | |
| 19 | SPI_MISO | |
| 21 | I2C_SDA | RTC |
| 22 | I2C_SCL | RTC |
| 23 | SPI_MOSI | |
| 25 | **PANEL_TX** | **CN_PANEL** · UART2 TX — **όχι** ρελέ |
| 26 | REL_K2 | |
| 27 | REL_K3 | defrost → ON |
| 32 | REL_K1 | TBD (rev A: K4 = GPIO12) |
| 33 | **PANEL_RX** | **CN_PANEL** · UART2 RX — **όχι** ρελέ |
| 34 | *(ελεύθερο)* | χωρίς flow rev A |
| 35 | *(ελεύθερο)* | χωρίς CT rev A |
| 36 | AC_OPTO | |
| 39 | BACKUP_THERM | optional U4 |

**Ενέργεια:** ευθυγράμμισε `alpha_pins.h` **μετά** το κλείδωμα PCB — όχι το αντίστροφο.

---

## Checklist πριν παραγγελία PCB

- [ ] **3× διπλά ρελέ** + 230 V ζώνη + LED ανά κανάλι
- [ ] **F2** → 5V_AUX
- [ ] **CN_PANEL** 4P → GPIO25/33 + **5V_AUX** (F2) — **ΚΛΕΙΔΩΜΕΝΟ** · πρόσθεσε στο EasyEDA
- [ ] **CN_BETA** 4P (UART 16/17)
- [ ] ~~**MAX31865** + PT100 ηλιακός~~ — **αφαιρέθηκε** (DS18 SOLAR-RETURN στο bus)
- [ ] **Διέγραψε CN6** + MAX31865 από EasyEDA
- [ ] ~~**CT** κύκλωμα → GPIO35~~ — **αφαιρέθηκε**
- [ ] **CN_DEFROST** 3P → GPIO14 (**R9** 4,7 kΩ pull-up) — **όχι** στη Beta
- [ ] **Μία** 1-Wire bus DS18 (5 probes) ή τεκμηρίωση ROM
- [ ] Διόρθωση **100 nF** (όχι 100 µF) στα DS18
- [ ] **Διέγραψε CN2** (HP-WATT) + CT parts από EasyEDA
- [ ] **Διέγραψε CN7 + CN8** + FLOW_SIG από EasyEDA
- [ ] **ERC/DRC** + πίνακας pin 1–38 H1/H2 εκτυπωμένος
- [ ] **Antenna keep-out** ESP32
- [ ] Fan Spoofer: **μόνο τρύπες** στη μητρική (αν colocation) — ηλεκτρικό μένει στο module

---

## Σχετικά

- [`ALPHA-REV-A.md`](ALPHA-REV-A.md)
- Archive: `thermostat-ui-demo/docs/romeos-design-notes.md` §12
- Beta HP: [`HP-BOARD-EASYEDA-v1.md`](archive/romeos-tsakas/κατασκευή αντλίας θερμότητας νερού νερού/HP-BOARD-EASYEDA-v1.md)
