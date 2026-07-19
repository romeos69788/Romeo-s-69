# Δύο οθόνες Viewe — ρόλοι + panel hub

> **Ρόλοι:** υπόθεση εργασίας σταθερή.  
> **Hub 6 πλαισίων panel:** **ΚΛΕΙΔΩΜΕΝΟ** 2026-07-19 → [`PANEL-HUB-6-TILES-LOCK-2026-07-19.md`](PANEL-HUB-6-TILES-LOCK-2026-07-19.md)

**Κατεύθυνση:** δύο Viewe 7″.  
**Παλιά** → panel δίπλα στην **Alpha** (λεβητοστάσιο).  
**Νέα** → **θερμοστάτης χώρου** (setpoint · φώτα · ρολά).

---

## Αρχιτεκτονική

```
┌─────────────────────┐     Wi‑Fi UDP / ESP‑NOW      ┌──────────────┐
│  ΟΘΟΝΗ 2 (ΝΕΑ)      │ ───────────────────────────► │              │
│  Θερμοστάτης χώρου  │ ◄─────────────────────────── │    ALPHA     │
│  setpoint · SHT     │                              │  μητρική     │
│  φώτα · ρολά        │                              │  καυστήρα    │
└─────────────────────┘                              └──────┬───────┘
┌─────────────────────┐     UART CN_PANEL                   │
│  ΟΘΟΝΗ 1 (ΠΑΛΙΑ)    │ ◄── GPIO25/33 · 115200 ─────────────┘
│  Panel · hub 6 tiles│     5V + GND
└─────────────────────┘                                     │
                                                     UART CN_BETA
                                                            │
                                                     ┌──────▼───────┐
                                                     │    BETA      │
                                                     │  ρελέ K1–K8  │
                                                     └──────────────┘
```

| Μονάδα | Hardware | Link προς Alpha | Ρόλος |
|--------|----------|-----------------|-------|
| **Οθόνη 1 — Panel** | Viewe **παλιά** | **CN_PANEL** UART | Plant Alpha+Beta · hub 6 |
| **Οθόνη 2 — Room** | Viewe **νέα** | Wi‑Fi / ESP‑NOW | Setpoint · φώτα · ρολά |
| **Alpha** | ESP32 | hub | Αισθητήρες · λογική · MQTT · bridge |
| **Beta** | ESP32 | UART από Alpha | Ρελέ K1–K8 |

---

## Οθόνη 1 — Panel Alpha

### Hardware link (κλειδωμένο PCB)

| CN_PANEL pin | Net | Alpha |
|--------------|-----|-------|
| 1 | GND | GND |
| 2 | 5V | 5V_AUX / 5V_ESP |
| 3 | TX | **GPIO25** |
| 4 | RX | **GPIO33** |

### UX hub — **ΚΛΕΙΔΩΜΕΝΟ**

Κύρια: **6 ισομεγέθη πλαίσια** → πάτημα → detail οθόνη.

| # | Πλαίσιο |
|---|---------|
| 1 | Θερμοκρασίες νερού |
| 2 | Αντλία θερμότητας |
| 3 | Έξοδοι / κυκλοφορητές |
| 4 | Μπόιλερ / διαχωριστής |
| 5 | Σύστημα / συναγερμοί |
| 6 | Συνδέσεις / επικοινωνίες |

Πλήρες lock: [`PANEL-HUB-6-TILES-LOCK-2026-07-19.md`](PANEL-HUB-6-TILES-LOCK-2026-07-19.md)

### Firmware βάση

- **Σβήσιμο** παλιού room UI σε αυτή τη συσκευή
- Drivers Viewe/LVGL από v10 OK να μείνουν
- Role: `ROMEOS_DISPLAY_ROLE_ALPHA_PANEL`
- Πρωτεύον link: UART CN_PANEL

---

## Οθόνη 2 — Room (αργότερα)

Ίδιο Viewe · Wi‑Fi/ESP‑NOW · setpoint + comfort · **όχι** CN_PANEL.

---

## Φάσεις

| Φάση | Κατάσταση |
|------|-----------|
| A Ρόλοι δύο οθονών | υπόθεση εργασίας |
| B Hub 6 πλαισίων | **✓ ΚΛΕΙΔΩΜΕΝΟ** |
| B2 Περιεχόμενο detail οθονών | συζήτηση επόμενη |
| C UART protocol Alpha↔Panel | ανοιχτό |
| D Bridge ενδείξεων Alpha↔Beta | ανοιχτό |

---

## Σχετικά

- [`PANEL-HUB-6-TILES-LOCK-2026-07-19.md`](PANEL-HUB-6-TILES-LOCK-2026-07-19.md)
- [`display/README.md`](../display/README.md)
- [`ALPHA-REV-A.md`](ALPHA-REV-A.md)
