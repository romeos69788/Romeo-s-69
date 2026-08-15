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
| **Πολλαπλές εισόδους αισθητήρων** (CN1–CN8) | Καλή κατεύθυνση — χρειάζεται ευθυγράμμιση ονομάτων/pin |

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

### 2. CN_PANEL — Viewe 7″ (τοπικός πίνακας λεβητοστασίου)

**Νέο κλέμα 4P** (JST-XH 2,54 mm):

| Pin | Net | ESP32 (H2) |
|-----|-----|------------|
| 1 | GND | — |
| 2 | 5V_AUX | μετά F2 |
| 3 | PANEL_TX | **GPIO25** |
| 4 | PANEL_RX | **GPIO33** |

- UART **115200** · **όχι** I2C.
- Τρέφεται από **5V_AUX** (~500 mA peak).

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

### 4. CT SCT-013 (HP-WATT) — μοναδικό CT

- **CN2 silk «HP-WATT»** — σωστή ιδέα.
- Χρειάζεται **κύκλωμα μέτρησης:** SCT-013 1 V out → **offset/divider** → **GPIO35** (ADC).
- **Όχι** δεύτερο CT · **όχι** CT 4 kW πίνακα (TUYA WiFi ξεχωριστά).

### 5. Ηλιακός — MAX31865 + PT100 (SPI)

- **CN6 «SOLAR-TEMP»** — αν είναι 3P DS18-style, **δεν αρκεί** για PT100.
- Χρειάζεται **MAX31865** onboard (8P) + κλέμα **3-wire PT100** (όπως archive §12.6).
- SPI: SCK/MOSI/MISO + **SOLAR_CS** (GPIO5 στο archive).

### 6. Defrost / απόψυξη

- **CN2_DEFROST** (2P) — `DEFROST_SIG` → **GPIO14** (opto από τριόδη βάνα HP).
- Λείπει από τρέχον σχέδιο.

### 7. Εξωτερική θερμοκρασία (DHT)

- **CN5 «EXIT-TEMP»** — OK αν είναι **DHT22** (1 data + 3V3 + GND).
- Net: **GPIO16** · pull-up 10 kΩ.

### 8. AC opto (inverter feedback) — προαιρετικό αλλά χρήσιμο

- **GPIO36** · 2P ή 3P κλέμα «AC_OPTO» — όπως archive.

---

## Πιθανά λάθη — διόρθωσε πριν routing

### Α. Σύγκρουση ονομάτων CN1/CN2

| Στο schematic | Στο PCB layout |
|---------------|----------------|
| CN1/CN2 = έξοδοι **MCP41050** | CN1/CN2 = **A/C-SENSOR** / **HP-WATT** |

**→ Μετονόμασε** π.χ.:
- MCP: **CN_AC_POT_A / CN_AC_POT_B**
- Αισθητήρες: **CN_HP_CT**, **CN_AC_SENSE** (αν χρειάζεται ξεχωριστό από MCP)

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

### Δ. Flow sensors — 5 V vs 3,3 V

PCB: CN7/CN8 **5V** · archive Hall flow: **3,3 V** + GPIO34.

- Έλεγξε datasheet αισθητήρα ροής.
- Αν 3,3 V logic → άλλαξε silk σε **3V3** · net **GPIO34** (και CN7 **ή** CN8 — **μία** ροή ΖΝΧ για MVP).

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
| 5 | SOLAR_CS | MAX31865 |
| 12 | REL_K4 | ή buzzer — **μία** χρήση |
| 13 | POT_CS / REL_K5 | MCP41050 CS |
| 14 | DEFROST_SIG / REL_K3 | επιλογή |
| 15 | BUZZER_ALARM / REL_K6 | archive: buzzer=15 |
| 16 | DHT_DATA / BETA_RX | **σύγκρουση** — BETA_RX=16, DHT=16 OK same board if Beta UART always on |
| 17 | BETA_TX | |
| 18 | SPI_SCK | |
| 19 | SPI_MISO | |
| 21 | I2C_SDA | RTC |
| 22 | I2C_SCL | RTC |
| 23 | SPI_MOSI | |
| 25 | PANEL_TX / REL_K1 | **σύγκρουση** — rev A: **PANEL_TX** (όχι ρελέ στο 25) |
| 26 | REL_K2 | |
| 27 | REL_K3 | |
| 32 | REL_K4 | archive |
| 33 | PANEL_RX / REL_K5 | rev A: **PANEL_RX** |
| 34 | FLOW_SIG | |
| 35 | CT_ADC | SCT-013 |
| 36 | AC_OPTO | |
| 39 | BACKUP_THERM | optional U4 |

**Ενέργεια:** ευθυγράμμισε `alpha_pins.h` **μετά** το κλείδωμα PCB — όχι το αντίστροφο.

---

## Checklist πριν παραγγελία PCB

- [ ] **3× διπλά ρελέ** + 230 V ζώνη + LED ανά κανάλι
- [ ] **F2** → 5V_AUX
- [ ] **CN_PANEL** 4P (Viewe UART)
- [ ] **CN_BETA** 4P (UART 16/17)
- [ ] **MAX31865** + PT100 ηλιακός (όχι μόνο DS18 στο CN6)
- [ ] **CT** κύκλωμα → GPIO35
- [ ] **CN_DEFROST** → GPIO14
- [ ] **Μία** 1-Wire bus DS18 (5 probes) ή τεκμηρίωση ROM
- [ ] Διόρθωση **100 nF** (όχι 100 µF) στα DS18
- [ ] **Μετονόμαση CN** — χωρίς διπλό CN1/CN2
- [ ] **ERC/DRC** + πίνακας pin 1–38 H1/H2 εκτυπωμένος
- [ ] **Antenna keep-out** ESP32
- [ ] Fan Spoofer: **μόνο τρύπες** στη μητρική (αν colocation) — ηλεκτρικό μένει στο module

---

## Σχετικά

- [`ALPHA-REV-A.md`](ALPHA-REV-A.md)
- Archive: `thermostat-ui-demo/docs/romeos-design-notes.md` §12
- Beta HP: [`HP-BOARD-EASYEDA-v1.md`](archive/romeos-tsakas/κατασκευή αντλίας θερμότητας νερού νερού/HP-BOARD-EASYEDA-v1.md)
