# Beta rev A — U1–U4 ρελέ + LED-K1…K8

**Ημερομηνία:** 2026-07-12  
**Block:** 5/7 · **κύριος ρόλος Beta** (firmware rev A = **ON**)

---

## Module 4P (κάθε U1–U4)

Τυπικό **2-ch 5 V relay module** (JD-Vcc / opto):

| Pin module | Silk (lock PCB) | Net |
|------------|-----------------|-----|
| **1** | **GND** | **GND** |
| **2** | **IN-1** | **`REL_Kx`** → GPIO |
| **3** | **IN-2** | **`REL_Ky`** → GPIO |
| **4** | **VCC** | **`5V_ESP`** (μετά **F1** · **όχι 3V3**) |

*(Schematic χρήστη 2026-07-12: **pin1=GND · pin4=5V_ESP** — lock silk.)*

⚠️ Αν το footprint σου έχει **άλλη σειρά** pin — **lock silk** στο PCB · η λογική μένει: **VCC=5V_ESP · GND · 2× IN**.

**230 V:** COM/NO/N/C στα ρελέ → ζώνη **IN 220V AC** · **χωριστό** από 5 V.

---

## Αντιστοίχιση module → K → GPIO

*(Πρόταση lock — ευθυγραμμισμένη με Alpha K1–K6 + 2 spare)*

| Module | Silk PCB | Κανάλι | Net | GPIO | H1/H2 |
|--------|----------|--------|-----|------|-------|
| **U1** REL1 | CIRC1 | **K1** IN-1 | `REL_K1` | **32** | H2-7 |
| | CIRC2 | **K2** IN-2 | `REL_K2` | **26** | H2-10 |
| **U2** REL2 | CIRC3 | **K3** IN-1 | `REL_K3` | **27** | H2-11 |
| | CIRC4 | **K4** IN-2 | `REL_K4` | **12** | H2-13 |
| **U3** REL3 | 3-Way | **K5** IN-1 | `REL_K5` | **13** | H2-15 |
| | HP | **K6** IN-2 | `REL_K6` | **15** | H1-16 |
| **U4** REL4 | 4kW | **K7** IN-1 | `REL_K7` | **14** | H2-12 |
| | S-B | **K8** IN-2 | `REL_K8` | **25** | H2-9 |

**Μην χρησιμοποιήσεις:** GPIO **4, 16, 17, 34, 35** (OPT + UART).

---

## LED-K1…K8

Ένδειξη **παράλληλα** με κάθε **`REL_Kx`** (ίδιο net με **IN** του module):

```
REL_Kx (GPIO) ── R_LED 1kΩ ── LED-Kn (+) ── LED-Kn (−) ── GND
```

| LED | Net (με IN) | R (τυπ. **1 kΩ**) |
|-----|-------------|-------------------|
| **LED-K1** | `REL_K1` | R-K1 |
| **LED-K2** | `REL_K2` | R-K2 |
| … | … | … |
| **LED-K8** | `REL_K8` | R-K8 |

**Silk:** **K1**…**K8** δίπλα στο LED · ταιριάζει με **REL1–4** silk.

---

## Τροφοδοσία ρελέ

```
5V_ESP ──┬── U1 VCC
         ├── U2 VCC
         ├── U3 VCC
         └── U4 VCC

GND ─────┴── U1..U4 GND (κοινό)
```

**F1 1,5 A** προστατεύει **ESP32 + όλα τα ρελέ** — OK για 8× coil + ESP.

---

## Λογική coil (bench)

Πολλά modules 5 V: **IN = LOW** → ρελέ **ON** · **HIGH** → OFF.  
**Επιβεβαίωσε** στο bench πριν lock firmware · `#define RELAY_ACTIVE_LOW 1` αν ισχύει.

**Defrost:** λογική στην **Alpha** (GPIO14) → εντολές **UART** → Beta οδηγεί **K2/K3/K4** (όχι DEFROST pin στη Beta).

---

## Checklist schematic — ✓ κλειδωμένο 2026-07-12

- [x] **U1–U4 VCC** = **`5V_ESP`** (όχι 3V3)
- [x] **IN-1/IN-2** → **`REL_K1`…`REL_K8`** → GPIO πίνακα πάνω
- [x] **LED-K1…K8** + **1 kΩ** στο ίδιο net με αντίστοιχο **IN**
- [x] **Δεν** conflict OPT / UART GPIO
- [ ] **230 V** ζώνη ξεχωριστή (έλεγχος DRC)

---

## H1 / H2 — pin 1…19 ↔ GPIO (DevKit 38 · USB κάτω)

**H2** = αριστερά · pin **1 πάνω = 3V3** · pin **19 κάτω = 5V**  
**H1** = δεξιά · pin **1 πάνω = GND**

### H2

| Pin | DevKit | GPIO | Beta net (rev A) |
|-----|--------|------|------------------|
| 1 | 3V3 | — | **3V3** |
| 2 | EN | EN | NC |
| 3 | VP | **36** | ελεύθερο |
| 4 | VN | **39** | ελεύθερο |
| 5 | IO34 | **34** | **BETA_FLOW_SIG** (CN4) |
| 6 | IO35 | **35** | **OPT_CURRENT_SIG** (CN1) |
| 7 | IO32 | **32** | **REL_K1** |
| 8 | IO33 | **33** | ελεύθερο |
| 9 | IO25 | **25** | **REL_K8** |
| 10 | IO26 | **26** | **REL_K2** |
| 11 | IO27 | **27** | **REL_K3** |
| 12 | IO14 | **14** | **REL_K7** |
| 13 | IO12 | **12** | **REL_K4** |
| 14 | GND | — | **GND** |
| 15 | IO13 | **13** | **REL_K5** |
| 16–18 | D2/D3/CMD | flash | **NC** |
| 19 | 5V | — | **5V_ESP** |

### H1

| Pin | DevKit | GPIO | Beta net (rev A) |
|-----|--------|------|------------------|
| 1 | GND | — | **GND** |
| 2 | IO23 | **23** | ελεύθερο |
| 3 | IO22 | **22** | ελεύθερο |
| 4 | TX0 | **1** | USB debug |
| 5 | RX0 | **3** | USB debug |
| 6 | IO21 | **21** | ελεύθερο |
| 7 | GND | — | **GND** |
| 8–10 | IO19/18/5 | 19/18/5 | ελεύθερο |
| 11 | IO17 | **17** | **TX** → CN_ALPHA |
| 12 | IO16 | **16** | **RX** → CN_ALPHA |
| 13 | IO4 | **4** | **BETA_DS18_DATA** |
| 14 | IO0 | **0** | **NC** (boot) |
| 15 | IO2 | **2** | **NC** (strap) |
| 16 | IO15 | **15** | **REL_K6** |
| 17–19 | flash | — | **NC** |

### Ρελέ — σύντομος πίνακας

| K | GPIO | Header · pin |
|---|------|--------------|
| K1 | 32 | **H2-7** |
| K2 | 26 | **H2-10** |
| K3 | 27 | **H2-11** |
| K4 | 12 | **H2-13** |
| K5 | 13 | **H2-15** |
| K6 | 15 | **H1-16** |
| K7 | 14 | **H2-12** |
| K8 | 25 | **H2-9** |

---

## Σχετικά

- [`BETA-REV-A-REVIEW-2026-07-12.md`](BETA-REV-A-REVIEW-2026-07-12.md) §4
- [`beta/README.md`](../beta/README.md)
