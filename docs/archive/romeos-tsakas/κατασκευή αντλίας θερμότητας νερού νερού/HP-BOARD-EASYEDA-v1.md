# HP CONTROL BOARD v1.0 — οδηγός EasyEDA

**Project name (EasyEDA):** `HP-CONTROL-BOARD-v1.0`  
**Silk title:** `ROMEOS HP BOARD v1.0 — WW 24k R32`  
**Σχετικό handoff:** `SESSION-HANDOFF_2026-06-14.md` · `HANDOFF_2026-06-08.md`  
**Τελευταία ενημέρωση:** 2026-06-14 — schematic πυρήνας ολοκληρωμένος · ERC pending

---

## Σειρά κατασκευής σχηματικού (σιγά-σιγά)

| Βήμα | Block | Κατάσταση |
|------|--------|-----------|
| **0** | ESP32 DevKit H1/H2 + USB άκρη | ✅ κλειστό |
| **1** | Τροφοδοσία — CN1 (μόνο 5 V) | ✅ κλειστό |
| **2** | MCP41050 + U5 (NTC spoof) | ✅ κλειστό |
| **3** | CN4 — DS18B20 νερού | ✅ κλειστό |
| **4** | U6 — αισθητήρας ροής | ✅ κλειστό |
| **5** | CN2 — DEFROST + R9 | ✅ κλειστό |
| **6** | F1 → 5V_ESP | ✅ κλειστό |
| **7** | LED (status, ρελέ, τροφοδοσία) | ✅ κλειστό |
| **8** | CN_OEM opto RUN/FAULT (PC817) | ✅ στο sheet |
| **9** | RL-K3/K4 heat·cool + RL-K1/K2 pump/servo | ✅ στο sheet |
| **10** | CN_SERVO PWM | ✅ στο sheet |
| **11** | MAX485 + CN_BUS RS485 | ✅ (pin4 NC pending) |
| **12** | CN_PRESS — φάση 2 | ⏸ placeholder |
| **13** | ERC + net labels + title block | ⏳ επόμενο |

---

## Block 1 / 7 — Τροφοδοσία (ίδια τεχνική με μητρική)

**Η HP πλακέτα ΔΕΝ έχει είσοδο 12 V.** Το 12 V μένει **έξω** — σε ξεχωριστά modules.

### Φυσική αλυσίδα (εκτός HP PCB)

```
Outdoor +12 V / GND
    → [12V SECURE INPUT SHIELD]     (module 1 — προστασία / ασφάλεια / ανάποδη)
    → [MODULE 12V → 5V]             (module 2 — buck, π.χ. ZX-052 / LM2596)
    → καλώδιο OUT+ / OUT−
    → CN1 στην HP πλακέτα            (μόνο +5 V και GND)
```

**AC FAN SPOOFER:** ξεχωριτό module — **όχι** μέρος αυτής της αλυσίδας.

### Μόνο στο copper της HP πλακέτας (μετά το CN1)

```
CN1 (+5V, GND)  ← από OUT buck module
  → net: 5V_PRE
  → C1 470 µF/16V + C2 10 µF + D2 BZX55C5V6 + LED7+R7 1k (προς GND)
  → F1 1 A  → 5V_ESP  → H2 pin 19 (ESP32)
  → F2 1,5 A → 5V_AUX  → LED, ρελέ, servo, MAX485…
```

**Όχι** στο HP schematic: `CN_PWR`, D1 1N5408, buck IC/module footprint, γραμμές 12 V.

### Κλέμα CN1 (2 pin) — μοναδική είσοδος τροφοδοσίας HP

| Pin | Net | Silk |
|-----|-----|------|
| 1 | **`5V_PRE`** | +5V / IN 5V DC |
| 2 | **`GND`** | GND |

**Footprint:** screw 2P 5,08 mm **ή** JST-XH 2P 2,54 mm.

### Εξαρτήματα στο HP PCB (μετά CN1)

| Ref | Τιμή | Σημείωση |
|-----|------|----------|
| C1 | 470 µF / **16 V** | Electrolytic — κόμβος `5V_PRE` |
| C2 | 10 µF / 16 V | Ceramic ή elec. — παράλληλα |
| D2 | BZX55C5V6 | Zener clamp 5 V |
| LED7 + R7 | LED + **1 kΩ** | Ένδειξη 5 V ON |
| F1 | Fuse **1 A** | `5V_PRE` → `5V_ESP` |
| F2 | Fuse **1,5 A** | `5V_PRE` → `5V_AUX` |

### Nets (EasyEDA)

- `5V_PRE`, `5V_ESP`, `5V_AUX`, `GND` — **όχι** `12V_*` nets στο HP sheet

---

## Block 2 — ESP32 38 pin (κλειδωμένο)

**Footprint:** ESP32 DevKit **38 pin** · USB προς εξωτερική άκρη · **H2** = σειρά 3V3 · **H1** = σειρά GND.

**Πλήρης πίνακας pin 1…19:** `ESP32-38PIN-H1-H2-MAP.md`

| GPIO | Net HP |
|------|--------|
| 4 | `DS18_DATA` |
| 5 | `SERVO_PWM` |
| 13 | `POT_CS` |
| 14 | `DEFROST_SIG` |
| 16 | `RS485_RX` |
| 17 | `RS485_TX` |
| 18 | `SPI_SCK` |
| 19 | `SPI_MISO` |
| 23 | `SPI_MOSI` |
| 25 | `REL_HEAT_CALL` |
| 26 | `REL_PUMP` |
| 27 | `REL_COOL_REV` |
| 32 | `RS485_DE` |
| 33 | `LED_STATUS` |
| 34 | `FLOW_SIG` |
| 35 | `PRESS_DISCH` (φάση 2) |
| 36 | `OEM_RUN_IN` |
| 39 | `OEM_FAULT_IN` |

*Strap / flash pins: **NC** — βλ. MAP.*

---

## Κλέμες (MVP) — pinout placeholder

### CN_WATER_T (3P) — DS18B20 bus

| Pin | Net |
|-----|-----|
| 1 | 3V3 |
| 2 | GND |
| 3 | DS18_DATA (+ R 4k7 → 3V3) |

### CN_FLOW (3P)

| Pin | Net |
|-----|-----|
| 1 | 3V3 |
| 2 | GND |
| 3 | FLOW_SIG |

### CN_SERVO (3P)

| Pin | Net |
|-----|-----|
| 1 | 5V_AUX |
| 2 | GND |
| 3 | SERVO_PWM |

### CN_BUS (4P) — RS485

| Pin | Net |
|-----|-----|
| 1 | RS485_A |
| 2 | RS485_B |
| 3 | GND |
| 4 | NC *(ή GND shield — όχι 12 V)* |

---

## Σημειώσεις PCB (outdoor)

- **Μονή πλευρά ή 2-layer** — 1 oz OK για MVP  
- **Conformal coating** ή θήκη — υγρασία outdoor  
- **Mounting:** 4 τρύπες στήριξης στο κέλυφος AC (μηχανική — ξεχωριστό από nets)  
- **Keep-out:** κάτω από ESP32 antenna (Wi‑Fi **off** στο MVP — μόνο RS485)

---

*Ενημέρωση κατά τη διάρκεια σχεδίασης EasyEDA.*
