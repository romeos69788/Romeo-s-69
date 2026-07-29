# Δύο οθόνες — ρόλοι + panel hub

> **Ρόλοι:** υπόθεση εργασίας σταθερή.  
> **Hub 6 πλαισίων panel:** **ΚΛΕΙΔΩΜΕΝΟ** 2026-07-19 → [`PANEL-HUB-6-TILES-LOCK-2026-07-19.md`](PANEL-HUB-6-TILES-LOCK-2026-07-19.md)  
> **Θερμοστάτης JC1060:** bring-up OK 2026-07-29 → [`SESSION-2026-07-29-JC1060-THERMOSTAT.md`](SESSION-2026-07-29-JC1060-THERMOSTAT.md)

**Κατεύθυνση:** δύο οθόνες 7″.  
**Viewe (παλιά)** → panel δίπλα στην **Alpha** (λεβητοστάσιο).  
**JC1060P470C (νέα)** → **θερμοστάτης χώρου** (setpoint · φώτα · ρολά) · **1024×600**.

---

## Αρχιτεκτονική

```
┌─────────────────────┐     Wi‑Fi (C6) / ESP‑NOW    ┌──────────────┐
│  ΟΘΟΝΗ 2 JC1060     │ ───────────────────────────► │              │
│  Θερμοστάτης χώρου  │ ◄─────────────────────────── │    ALPHA     │
│  1024×600 · setpoint│                              │  μητρική     │
│  φώτα · ρολά        │                              │  καυστήρα    │
└─────────────────────┘                              └──────┬───────┘
┌─────────────────────┐     UART CN_PANEL                   │
│  ΟΘΟΝΗ 1 Viewe      │ ◄── GPIO25/33 · 115200 ─────────────┘
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
| **Οθόνη 1 — Panel** | Viewe 7″ · 800×480 · COM4 | **CN_PANEL** UART | Plant Alpha+Beta · hub 6 |
| **Οθόνη 2 — Room** | **JC1060P470C** · ESP32-P4 · **1024×600** · COM14 | Wi‑Fi (C6) / ESP‑NOW | Setpoint · φώτα · ρολά |
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

## Οθόνη 2 — Room (θερμοστάτης)

| | |
|--|--|
| Hardware | JC1060P470C_I_W_Y · ESP32-P4 · MIPI JD9165 · GT911 |
| EEZ | `Othoni Levita/Thermostat_JC1060/` · **1024×600** |
| Firmware | `display-jc1060` · BSP **Old_Panel** |
| Link | Wi‑Fi μέσω C6 / ESP‑NOW · **όχι** CN_PANEL |
| Bring-up | ✓ smoke καθαρή εικόνα 2026-07-29 |

---

## Φάσεις

| Φάση | Κατάσταση |
|------|-----------|
| A Ρόλοι δύο οθονών | σταθερό (Viewe panel · JC1060 room) |
| B Hub 6 πλαισίων | **✓ ΚΛΕΙΔΩΜΕΝΟ** |
| B1 EEZ hub panel | V4 κλειδωμένο · detail screens σε εξέλιξη |
| B2 Περιεχόμενο detail οθονών panel | ανοιχτό |
| E1 JC1060 bring-up + backup | **✓** 2026-07-29 |
| E2 EEZ θερμοστάτη 1024×600 | mockup 2 σελίδες + swipe OK · **όχι κλειδωμένο** (κύκλος/φιδάκι) |
| E3 FW UI + Wi‑Fi C6 | μετά το EEZ |
| C UART protocol Alpha↔Panel | ανοιχτό |
| D Bridge ενδείξεων Alpha↔Beta | ανοιχτό |

---

## Σχετικά

- [`SESSION-2026-07-29-JC1060-THERMOSTAT.md`](SESSION-2026-07-29-JC1060-THERMOSTAT.md)
- [`PANEL-HUB-6-TILES-LOCK-2026-07-19.md`](PANEL-HUB-6-TILES-LOCK-2026-07-19.md)
- [`display/README.md`](../display/README.md)
- [`ALPHA-REV-A.md`](ALPHA-REV-A.md)
