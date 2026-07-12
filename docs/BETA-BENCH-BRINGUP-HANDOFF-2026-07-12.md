# Beta rev A — bench bring-up (μετά άφιξη πλακετών)

**Ημερομηνία lock:** 2026-07-12  
**Αναμενόμενη άφιξη:** ~τέλος Ιουλίου 2026  
**Κατάσταση:** Παραγγελία JLCPCB — χρήστης επιβεβαίωσε checkout

---

## Τι παραγγέλθηκε

| Board | Project EasyEDA | Rev | Σημείωση |
|-------|-----------------|-----|----------|
| **Beta** | Bita Mitriki · CONTROL BOARD v2.0 | rev A | 8 ρελέ · CN_ALPHA · OPT1–4 wired |
| **Alpha** | Mitriki ALPHA | rev A | **CN_BETA** 4P · νέο Gerber (ίδιο cart) |

**Firmware rev A Beta:** μόνο ρελέ + UART προς Alpha · OPT sensors **standby** (`#define 0`).

---

## Πριν το πρώτο power-on (πάγκος)

### Μηχανικό

- [ ] Mounting holes + standoffs
- [ ] ESP32 DevKit 38P σωστά seated (H1/H2)
- [ ] **Όχι** 230 V στο bench μέχρι να περάσει low-voltage bring-up

### Τροφοδοσία (χωρίς φορτίο πεδίου)

- [ ] **12 V DC** στο CN9 (ή όπως wired) → buck → **5V_ESP**
- [ ] Μέτρηση: **5V_ESP** @ U1–U4 pin4 · **3V3** @ H2-1
- [ ] LED-F1 / D1 Zener — τάση OK

### UART link (Alpha ↔ Beta)

- [ ] **1× flat 4P** · crossover TX/RX
- [ ] Pin1 **GND** · pin2 **TX** · pin3 **RX** · pin4 **NC**
- [ ] **Όχι** 5V στο link

→ [`BETA-ALPHA-UART-LINK.md`](BETA-ALPHA-UART-LINK.md)

---

## Βήματα bench test (σειρά)

### 1. Low-voltage only

1. ESP32 boot · USB ή 12 V (όχι ρελέ coils στο πεδίο ακόμα)
2. Serial monitor — boot log
3. Flash **Beta rev A firmware** (relay + UART stub)

### 2. Ρελέ K1–K8 (χωρίς 230 V)

| K | Πεδίο | GPIO | Έλεγχος |
|---|-------|------|---------|
| K1 | Κυκλοφορητής 1 | 32 | click + LED-K1 |
| K2 | Κυκλοφορητής 2 | 26 | click + LED-K2 |
| K3 | Κυκλοφορητής 3 | 27 | click + LED-K3 |
| K4 | Κυκλ. ηλιακού | 12 | click + LED-K4 |
| K5 | Τρίοδη βάνα | 13 | click + LED-K5 |
| K6 | HP | 15 (H1-16) | click + LED-K6 |
| K7 | 4 kW | 14 | click + LED-K7 |
| K8 | ST-BY spare | 25 | click + LED-K8 |

→ Πλήρης πίνακας: [`BETA-RELAY-MAP.md`](BETA-RELAY-MAP.md)

### 3. UART Alpha ↔ Beta

- [ ] Alpha στέλνει εντολή ρελέ → Beta εκτελεί
- [ ] Status / ACK (όταν οριστεί πρωτόκολλο)

### 4. OPT headers (optional — hardware wired)

| Κλέμα | Module | GPIO | Rev A firmware |
|-------|--------|------|----------------|
| CN1 | SCT-013 20A/1V | 35 | **off** |
| CN2/CN3 | DS18B20 | 4 | **off** |
| CN4 | YF-B10 flow | 34 | **off** |

→ [`BETA-OPT-HARDWARE-MAP.md`](BETA-OPT-HARDWARE-MAP.md)

### 5. Πεδίο (μετά bench OK)

- [ ] 230 V ζώνη · clearance/creepage
- [ ] Contactor / κυκλοφορητές / HP / 4kW — **ένα-ένα**
- [ ] **K8 ST-BY** — spare · μην συνδέσεις φορτίο χωρίς σχέδιο

---

## Αγορές / αποθήκη (μην ξεχάσεις)

- **1× SCT-013 20A/1V** μόνο (HP line) — **όχι** δεύτερο CT
- YF-B10 flow — optional
- DS18B20 — optional (κυρίως Alpha)

→ [`hardware/SHOPPING-REMINDERS.md`](hardware/SHOPPING-REMINDERS.md)

---

## Docs lock (2026-07-12)

Όλα κλειδωμένα πριν παραγγελία:

- CN_ALPHA / CN9 / power
- CN1–CN4 OPT
- U1–U4 + LED-K1…K8 + K1–K8 field map
- PCB synced · **52/52 nets · 0 DRC**

→ [`BETA-PRE-FAB-FINAL-CHECKLIST-2026-07-12.md`](BETA-PRE-FAB-FINAL-CHECKLIST-2026-07-12.md)

---

## Επόμενη συνεδρία (όταν έρθουν οι πλακέτες)

1. Φωτο PCB + continuity γρήγορος έλεγχος
2. Flash firmware · bench relay test
3. UART link με Alpha
4. Απόφαση για ενεργοποίηση OPT στο firmware rev B

---

*Handoff session 8 · 2026-07-12*
