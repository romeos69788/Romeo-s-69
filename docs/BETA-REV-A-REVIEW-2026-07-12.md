# Beta rev A — έλεγχος schematic/PCB (Bita Mitriki · CONTROL BOARD v2.0)

**Ημερομηνία:** 2026-07-12  
**Πηγές:** schematic + PCB layout EasyEDA (χρήστης)  
**Σύγκριση με:** Alpha rev A locked · [`beta/README.md`](../beta/README.md)

---

## 1. Τι φαίνεται στο board (inventory)

| Block | Schematic | PCB | Σχόλιο |
|-------|-----------|-----|--------|
| ESP32 DevKit 38 (H1/H2) | ✓ | ✓ | ⚠ διπλό ESP32 με Alpha — δες §3 |
| 12 V IN + buck 12→5 V | ✓ | ✓ | 🔄 **έλεγχος block 3/7** |
| F1 1,5 A → 5V_ESP | ✓ | ✓ | OK (ρελέ rail) |
| C3/C4 + D1 Zener + LED-F1 | ✓ | ✓ | OK |
| **U1–U4** 4P → **2-ch relay modules** | ✓ | ✓ | **8 κανάλια** · OK |
| **LED-K1…K8** + R | ✓ | ✓ | ένδειξη ρελέ · OK |
| **CN1** AC FLOW (2P) | ✓ | ✓ | 🔴 **αφαίρεση** — αποφάσεις rev A |
| **CN2/CN3** WATER-TEMP DS18 | ✓ | ✓ | 🔴 **αφαίρεση** — DS18 στην Alpha |
| **CN4** WATER FLOW (3P) | ✓ | ✓ | 🔴 **αφαίρεση** — flow αφαιρέθηκε |
| **CN9** 2P + **F1** 1,5 A | ✓ | ✓ | ✓ **κλειδωμένο** 2026-07-12 |
| Silk REL1–REL4 | — | ✓ | βλ. πίνακα §4 |
| **CN_ALPHA / UART** (U5) προς Alpha | ✓ | ✓ | ✓ **κλειδωμένο** 2026-07-12 |
| CN_DEFROST | ✗ | ✗ | ✓ σωστά (μένει Alpha) |
| Mounting holes + pads | — | ✓ | OK |

---

## 2. Σύγκρουση με κλειδωμένες αποφάσεις ROMEOS 69

> **Ενημέρωση 2026-07-12 (χρήστης):** Οι **4 κλέμες πάνω** (CN1–CN4) είναι **προαιρετικές / μελλοντικές** — **δεν** μπαίνουν στο αρχικό firmware. Σκοπός: **μην φορτώνουμε την Alpha** (ήδη φορτωμένη). **Βήμα-βήμα lock** — όχι «διέγραψε όλα».

| Κλέμα (τώρα) | Σκοπός (χρήστης) | Firmware rev A | Σημείωση |
|--------------|------------------|----------------|----------|
| **CN1** 2P | Μελλοντικός **αισθητήρας ρεύματος** (κατανάλωση) | **Όχι** στον κώδικα | Προαιρετικό · WiFi meter ήδη για HP kWh |
| **CN2 / CN3** 3P | **Νερό DS18** (αν χρειαστεί εδώ) | **Όχι** στον κώδικα | Αισθητήρες νερού κυρίως **Alpha** |
| **CN4** 3P | **Ροή νερού** (αν χρειαστεί) | **Όχι** στον κώδικα | Flow αφαιρέθηκε από Alpha scope — optional εδώ |
| **DEFROST** | — | — | ✓ μένει **Alpha** μόνο |

**Συμφωνία ρόλου Beta:** **μόνο διαχείριση ρελέ** στο firmware · οι 4 κλέμες = **hardware option** για αργότερα.

### Πρόταση ονομασίας (να μην μπερδεύονται με Alpha)

| Τώρα | Προτεινόμενο silk | Σημείωση |
|------|-------------------|----------|
| CN1 | **CN_OPT1 CURRENT** (2P) | όχι «CN1» όπως Alpha ROOM-NTC |
| CN2 | **CN_OPT2 WATER-T** | DS18 optional |
| CN3 | **CN_OPT3 WATER-T** | DS18 optional |
| CN4 | **CN_OPT4 WATER-FLOW** | flow optional |

*(Ή κράτα CN1–4 στη Beta αλλά silk **OPT / FUTURE** — αρκεί να διαφέρουν από Alpha.)*

---

## 2b. Παλιά σύσταση «διέγραψε» — **ακυρώθηκε**

Οι κλέμες **μένουν** στο PCB ως optional headers · **NC** στο firmware rev A.

---

## 3. Αρχιτεκτονική — αποφάσισε πριν fab

### Επιλογή Α — **Slave Beta** (συνιστάται από docs)

```
Alpha ESP32 (λογική, sensors, MQTT)
    │ UART 115200 (GPIO16/17)
    ▼
Beta: μόνο ρελέ drivers (+ optional ESP32 ως UART slave)
```

- **Alpha:** προσθήκη **CN_BETA** 4P (GND · TX17 · RX16 · NC)
- **Beta:** προσθήκη **CN_ALPHA** 4P (ίδια nets)
- **Όχι** δεύτερο DS18/flow στη Beta
- Αν Beta κρατά **ESP32:** firmware = δέχεται εντολές UART · οδηγεί IN-1/IN-2 ρελέ · στέλνει ACK/status

### Επιλογή Β — **Standalone Beta ESP32**

- Delta ESP32 και στις δύο πλακέτες — χρειάζεται **σαφές** ποιος κάνει τι (σύγχυση firmware)
- **Όχι** διπλοί αισθητήρες anyway

**Πρόταση:** Επιλογή Α · κράτα ESP32 στη Beta μόνο αν το κουτί είναι μακριά — αλλά **αφαίρεσε sensors** από Beta schematic.

---

## 4. Ρελέ — silk PCB (αφετηρία κλειδώματος)

PCB silk (όπως φαίνεται):

| Module | Silk | Κανάλια | Προτεινόμενο K |
|--------|------|---------|----------------|
| **U1 / REL1** | CIRC1 · CIRC2 | IN-1 · IN-2 | **K1 · K2** (μικροί κυκλοφορητές) |
| **U2 / REL2** | CIRC3 · CIRC4 | IN-1 · IN-2 | **K3 · K4** (κύριος + ηλιακή βάνα;) |
| **U3 / REL3** | 3-Way · HP | IN-1 · IN-2 | **K5 · K6** ή HP heat call + 3-way |
| **U4 / REL4** | 4kW · S-B | IN-1 · IN-2 | **4 kW heater · spare** |

⚠️ **Δεν locked** — αύριο ορίζουμε **ακριβώς** με πεδίο (defrost interlock από Alpha GPIO14).

**Archive mapping (αναφορά):**

| Ρελέ | Λειτουργία (docs) |
|------|-------------------|
| K2 | Μικρός κυκλοφορητής · defrost → OFF |
| K3 | Κύριος κυκλοφορητής · defrost → ON |
| K4 | Heater / solar valve |
| K1, K5, K6 | HP OEM / spare |

**Defrost logic (Alpha):** `DEFROST_SIG` → K2 OFF · K4 solar OFF · K3 ON — υλοποιείται στο **firmware Alpha** · εντολές UART → Beta.

---

## 5. Τι λείπει πριν fab

| # | Item | Προτεραιότητα |
|---|------|----------------|
| 1 | **CN_ALPHA** 4P UART (GND · RX · TX · NC) | 🔴 κρίσιμο — **επόμενο βήμα** |
| 2 | ~~Αφαίρεση CN1–CN4~~ | ✓ **μένουν** — optional · όχι firmware rev A |
| 2b | **Rename silk** CN1–CN4 (OPT / FUTURE) | 🟡 να συμφωνήσουμε |
| 3 | **Pin map** H1/H2 → REL_K1…K8 (πίνακας GPIO) | 🔴 πριν routing lock |
| 4 | **230 V** ζώνη · creepage · silk «IN 220V AC» | 🟡 έλεγχος DRC |
| 5 | **ERC/DRC** · nets πλήρης | 🔴 |
| 6 | Silk **BETA rev A** / ημερομηνία | 🟡 |
| 7 | **CN_BETA** στην **Alpha** (αν Επιλογή Α) | 🔴 ζεύγος καλωδίου |

---

## 6. Τι είναι ήδη καλά ✓

- **4× διπλά ρελέ headers** (U1–U4) — σωστή κατεύθυνση
- **8× LED** ένδειξης (LED-K1…K8)
- **F1 1,5 A** για 5V ρελέ rail
- **Τροφοδοσία** 12V → buck → 5V (ανεξάρτητη από Alpha)
- **Mounting holes** με pads
- **Όχι** DEFROST / CT / MAX31865

---

## 7. Verdict (προσωρινό)

**Όχι fab ακόμα** — βήμα-βήμα:

1. ~~Καθάρισμα sensors~~ → **CN1–4 optional OK** (χρήστης)
2. **CN_ALPHA UART** + pin map ρελέ
3. DRC → Gerber

---

## 8. Session αύριο — ερωτήσεις

1. Beta **μένει ESP32** ή **pure relay** + UART level shifter;
2. Πού τοποθετείται φυσικά (outdoor κοντά HP vs λεβητοστάσι);
3. **K1–K8** → ακριβής πεδίο wiring (contactor, 4kW, 3-way, HP OEM);
4. UART πρωτόκολλο (π.χ. `REL K3 ON\n` / JSON / binary);
5. **CN9** τι είναι στο schematic;

---

## Σχετικά

- [`beta/README.md`](../beta/README.md)
- [`ALPHA-REV-A-FINAL-LOCK-2026-07-11.md`](ALPHA-REV-A-FINAL-LOCK-2026-07-11.md)
- [`ΣΥΖΗΤΗΣΗ.md`](ΣΥΖΗΤΗΣΗ.md) — μήνυμα 38
