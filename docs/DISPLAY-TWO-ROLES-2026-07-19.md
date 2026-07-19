# Δύο οθόνες Viewe — ρόλοι (κλείδωμα 2026-07-19)

**Απόφαση χρήστη:** Υπάρχουν **δύο** Viewe 7″ (ίδιο hardware).  
Η **παλιά** (ήδη στο σπίτι) → **οθόνη μητρικής Alpha** (πίνακας λεβητοστασίου).  
Η **νέα** (παραγγελία) → **θερμοστάτης χώρου**.

---

## Αρχιτεκτονική

```
┌─────────────────────┐     Wi‑Fi UDP / ESP‑NOW      ┌──────────────┐
│  ΟΘΟΝΗ 2 (ΝΕΑ)      │ ───────────────────────────► │              │
│  Θερμοστάτης χώρου  │ ◄─────────────────────────── │    ALPHA     │
│  setpoint · SHT     │                              │  μητρική     │
└─────────────────────┘                              │  καυστήρα    │
                                                     └──────┬───────┘
┌─────────────────────┐     UART CN_PANEL            │      │
│  ΟΘΟΝΗ 1 (ΠΑΛΙΑ)    │ ◄── GPIO25/33 · 115200 ─────┘      │
│  Panel λεβητοστασίου│     5V + GND                        │
│  δίπλα στην Alpha   │                                     │
└─────────────────────┘                                     │
                                                     UART CN_BETA
                                                     GPIO17/16
                                                            │
                                                     ┌──────▼───────┐
                                                     │    BETA      │
                                                     │  ρελέ K1–K8  │
                                                     └──────────────┘
```

| Μονάδα | Hardware | Τοποθεσία | Link προς Alpha | Κύριος ρόλος |
|--------|----------|-----------|-----------------|--------------|
| **Οθόνη 1 — Panel Alpha** | Viewe 7″ **παλιά** | Λεβητοστάσιο · δίπλα Alpha | **CN_PANEL** UART (κύριο) · Wi‑Fi optional | Κατάσταση συστήματος · χειρισμός λεβητοστασίου |
| **Οθόνη 2 — Room** | Viewe 7″ **νέα** | Χώρος (κουτί τοίχου) | **Wi‑Fi UDP / ESP‑NOW** (όχι καλώδιο στο CN_PANEL) | Θερμοκρασία χώρου · **setpoint** |
| **Alpha** | ESP32 DevKit | Λεβητοστάσιο | hub | Αισθητήρες νερού · λογική · MQTT · bridge προς Beta |
| **Beta** | ESP32 DevKit | Outdoor / ρελέ | UART από Alpha | Εκτέλεση ρελέ K1–K8 |

---

## Οθόνη 1 — Panel Alpha (αυτή που διαμορφώνουμε τώρα)

### Hardware link (κλειδωμένο)

| CN_PANEL pin | Net | Alpha |
|--------------|-----|-------|
| 1 | GND | GND |
| 2 | 5V | 5V_AUX / 5V_ESP |
| 3 | TX | **GPIO25** (PANEL_TX) |
| 4 | RX | **GPIO33** (PANEL_RX) |

- **115200 8N1** · crossover: Alpha TX → Viewe RX · Alpha RX ← Viewe TX  
- **Όχι** σύνδεση room οθόνης στο CN_PANEL

### Τι δείχνει / κάνει (στόχος UI)

| Περιοχή | Περιεχόμενο |
|---------|-------------|
| **Κύρια** | Κατάσταση συστήματος (ON/OFF · mode · alarm) |
| **Νερό** | Θερμοκρασίες DS18 από Alpha (προσαγωγή / επιστροφή / ηλιακός / …) |
| **Ρελέ** | Κατάσταση K1–K8 (via Alpha←Beta) · προαιρετικά χειροκίνητο override |
| **HP / Defrost** | Κατάσταση αντλίας · σήμα defrost |
| **Setpoint χώρου** | **Εμφάνιση** μόνο (έρχεται από οθόνη 2) — **όχι** κύριο +/- χώρου |
| **Remote** | Ένδειξη MQTT / cloud |

**Δεν** είναι θερμοστάτης δωματίου — δεν βασίζεται στο SHT της οθόνης για έλεγχο χώρου.

### Βάση firmware

- Ξεκίνημα από **romeos-display-v10** (EEZ: PRYMARY / MENU / HEAT / BOILER / …)
- Νέο role flag: `ROMEOS_DISPLAY_ROLE_ALPHA_PANEL`
- Πρωτεύον link: **UART** στο CN_PANEL (όχι μόνο UDP SoftAP)
- UI: έμφαση οθόνη **BOILER** / νέα **SYSTEM** · HEAT/LIGHT/BLINDS δευτερεύοντα ή κρυφά στο panel role

---

## Οθόνη 2 — Θερμοστάτης χώρου (αργότερα · όταν έρθει)

| Θέμα | Απόφαση |
|------|---------|
| Hardware | Ίδιο Viewe 7″ |
| Link | Wi‑Fi UDP / ESP‑NOW προς Alpha |
| UI | Κοντά στο v10 PRYMARY: setpoint +/- · room temp (SHT) · MENU comfort |
| Role flag | `ROMEOS_DISPLAY_ROLE_ROOM` |

**Δουλειά τώρα:** όχι · περιμένουμε άφιξη · κρατάμε v10 ως βάση room.

---

## Φάσεις διαμόρφωσης Panel Alpha

### Φάση A — κλείδωμα ρόλων ✓ (αυτό το doc)

### Φάση B — UI / EEZ panel (επόμενο βήμα μαζί σου)

1. Τι μένει από v10 στο panel (BOILER, MENU, alarms)
2. Τι βγαίνει / κρύβεται (setpoint χώρου ως κύριο, LIGHT/BLINDS;)
3. Νέα κύρια οθόνη «λεβητοστάσιο»: θερμοκρασίες νερού + ρελέ
4. Build env `BOARD_VIEWE_PANEL` στο `display/`

### Φάση C — UART protocol Alpha ↔ Panel

1. Pin map Viewe (J2 / expansion) για RX/TX προς CN_PANEL
2. Frames: state (temps, relays, alarms) + commands (override)
3. Alpha `alpha_pins.h`: rename Display → Panel

### Φάση D — bridge Alpha ↔ Beta στην οθόνη

Εμφάνιση κατάστασης ρελέ Beta στην panel οθόνη (μέσω Alpha).

---

## Αποφάσεις ανοιχτές (να κλείσουμε στη φάση B)

1. Η panel οθόνη κρατάει **χειροκίνητο setpoint νερού / mode** ή μόνο monitoring;
2. LIGHT / BLINDS στο panel — ναι ή μόνο στη room;
3. UART pins στο Viewe — ποιο connector (φωτο / datasheet);
4. Παράλληλο Wi‑Fi στην panel «παλιά» — ναι ως backup ή UART μόνο;

---

## Σχετικά

- [`display/README.md`](../display/README.md)
- [`ALPHA-REV-A.md`](ALPHA-REV-A.md)
- Archive v10: `docs/archive/romeos-tsakas/romeos-display-v10/`
- Beta relay map: [`BETA-RELAY-MAP.md`](BETA-RELAY-MAP.md) *(αν υπάρχει στο tree)*
- UART Alpha↔Beta: [`BETA-ALPHA-UART-LINK.md`](BETA-ALPHA-UART-LINK.md) *(αν υπάρχει στο tree)*
