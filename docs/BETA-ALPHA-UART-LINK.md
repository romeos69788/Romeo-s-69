# Alpha ↔ Beta — σύνδεση UART (CN_BETA / CN_ALPHA)

**Κλείδωμα:** 2026-07-12  
**Πρωτόκολλο:** UART **115200** 8N1 · **όχι** τροφοδοσία από πλακέτα σε πλακέτα

---

## Τι λείπει τώρα

| Πλακέτα | Κλέμα | Κατάσταση |
|---------|-------|-----------|
| **Alpha (1)** | **CN_BETA** 4P | ⏳ **δεν μπήκε** στο rev A fab — **rev B** ή προσωρινό καλώδιο στα H1-11/12 |
| **Beta (2)** | **CN_ALPHA** 4P | ⏳ **πρόσθεσε** πριν fab Beta |

---

## Κλέμα 4P (και τις δύο πλευρές)

**Τύπος:** JST-XH **2,54 mm** 4P (π.χ. **PA001-4P** / **ZX-XH2.54-4PZZ-Y**)

| Pin | Silk (ίδιο label και στις 2 πλακέτες) | Net Alpha | Net Beta | ESP32 DevKit 38 |
|-----|----------------------------------------|-----------|----------|-----------------|
| **1** | **GND** | GND | GND | H1-1 / H2-14 |
| **2** | **TX** | **BETA_TX** | **ALPHA_TX** | **H1-11 / GPIO17** (UART1 TX) |
| **3** | **RX** | **BETA_RX** | **ALPHA_RX** | **H1-12 / GPIO16** (UART1 RX) |
| **4** | **NC** | NC | NC | — |

**Designator:**
- **Alpha:** `CN_BETA` · silk **«TO BETA»**
- **Beta:** `CN_ALPHA` · silk **«TO ALPHA»**

**EN / 3V3 / 5V:** **όχι** στο κλέμα — κάθε πλακέτα έχει **δική της** τροφοδοσία.

---

## Καλώδιο (crossover UART)

**Straight 1↔1, 2↔2, 3↔3 δεν δουλεύει** — χρειάζεται **αντιστροφή TX/RX**:

```
Alpha CN_BETA          Beta CN_ALPHA
─────────────────────────────────────
pin 1 GND      ─────── pin 1 GND
pin 2 TX       ─────── pin 3 RX
pin 3 RX       ─────── pin 2 TX
pin 4 NC       ─────── pin 4 NC
```

**4 αγώγοι** (π.χ. JST-XH 4P flat · μήκος TBD — αν **>3 m** σκέψου RS485 αργότερα).

---

## Firmware (σύντομα)

| ESP32 | UART | Ρόλος |
|-------|------|-------|
| **Alpha** | UART1 · TX=17 · RX=16 | Master — εντολές ρελέ · MQTT λογική |
| **Beta** | UART1 · TX=17 · RX=16 | Slave — οδηγεί IN-1/IN-2 ρελέ · ACK/status |

*(Πρωτόκολο περιεχομένου — επόμενο βήμα lock.)*

---

## EasyEDA — Beta (τώρα)

1. **Place → Connector** 4P κοντά στο ESP32 (π.χ. δίπλα H1-11/12)
2. Net **pin 2 → H1-11 (GPIO17)** · **pin 3 → H1-12 (GPIO16)** · **pin 1 → GND**
3. Silk: **CN_ALPHA · TO ALPHA**
4. **Μην** μοιράσεις GPIO17/16 με ρελέ ή OPT sensors

---

## EasyEDA — Alpha

**Αν rev A ήδη παραγγέλθηκε χωρίς CN_BETA:**

- **Προσωρινά:** dupont από **H1-11 (TX)** · **H1-12 (RX)** · **GND** → καλώδιο προς Beta
- **Rev B Alpha:** πρόσθεσε **CN_BETA** 4P με τον πίνακα πάνω

---

## Έλεγχος μετά wiring

1. Κοινό **GND** μόνο (όχι κοινό 5V)
2. Κάθε πλακέτα τροφοδοτείται μόνη της
3. Serial test: Alpha `Serial1` hello → Beta `Serial1` receive

---

## Σχετικά

- [`BETA-REV-A-REVIEW-2026-07-12.md`](BETA-REV-A-REVIEW-2026-07-12.md)
- [`ALPHA-REV-A-PCB-REVIEW.md`](ALPHA-REV-A-PCB-REVIEW.md) § CN_BETA
