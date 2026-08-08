# Beta (+ Alpha) — pre-order checklist πριν 3ο fab

**Ημερομηνία:** 2026-08-08  
**Στόχος:** να μην ξαναέρθει πλακέτα με γνωστά bugs από bring-up.

---

## Υποχρεωτικά πριν Upload Gerber (Beta v2 → v2.1)

### 1. LED-K πολικότητα (ήδη το κάνεις)
- **Από:** `REL_Kx → R → LED → GND` (active HIGH → ανάποδα από Songle)
- **Σε:** `5V_ESP → R 1k → LED(+) → LED(−) → REL_Kx`
- Ίδιο και στα **4** ζεύγη (U1…U4)

### 2. Οπτική αντιστοιχία αριστερά/δεξιά
Δύο ισοδύναμες λύσεις — διάλεξε **μία** και κλείδωσέ την:

| Λύση | Τι αλλάζεις |
|------|-------------|
| **A (που έκανες)** | LED-K1 cathode → `REL_K2`, LED-K2 → `REL_K1` (και αντίστοιχα ζεύγη) |
| **B (πιο καθαρή)** | Διόρθωσε **IN-1/IN-2** στο header ώστε αριστερό Songle = `REL_K1` · τότε LED-K1 → `REL_K1` χωρίς σταύρωμα |

Αν μείνεις στην A: σημείωσε στο schematic comment «silk = φυσική θέση, net ονόματα σταυρωτά επίτηδες».

### 3. GPIO12 = REL_K7 — **πρέπει να φύγει**
- Σήμερα **K7 (4kW) = GPIO12** → strap MTDI → in-socket flash βλέπει **1.8 V** → `Manufacturer ff`
- Workaround τώρα: βγάλε ESP, flash USB, ξαναβάλε
- **Fab fix (κλείδωμα 2026-08-08):** `REL_K7` → **GPIO19** (H1 · silk **IO19** · επιβεβαιωμένο NC στο PCB)
  - Όχι GPIO32 — εκεί είναι ήδη **REL_K1**
- **GPIO12** (H2 · IO12): **NC** (σβήσε το track, χωρίς net)
- Έλεγχος: κανένα `REL_Kx` σε **GPIO0, 2, 12, 15**

### 4. Zener — BOM + silk + φορά
- Silk πλακέτας: **ZENER 5.6V**
- Schematic συχνά: **BZX55C8V2 (8.2V)** → **ευθυγράμμισε**
- Πρόταση: **BZX55C5V6**, silk `5V6`
- **Φορά (κλειδωμένο):** γραμμή/**κάθοδος** → **+5 V** · άνοδος → **GND**
- (Ανάποδα = σαν δίοδος 0,7 V · καθίζηση buck / καμένο F1)

### 5. LED πάνω σε αισθητήρες (DS18 + FLOW) — κλείδωμα 2026-08-09
- **LED-1 + LED-2** (+ σειριακές R τους): ήταν `3V3 → R → LED → BETA_DS18_DATA` → φορτώνουν 1-Wire  
  → **ΔΙΑΓΡΑΦΗ** από schematic/PCB (μην αφήνεις DNP πάνω στο DATA)
- **Κράτα:** `R9` 4.7k pull-up `BETA_DS18_DATA` → `3V3`
- **Μην** βάζεις πυκνωτή DATA→GND στο DS18 (false presence) — αν υπάρχει C στο SING, **αφαίρεσέ τον**
- **LED-4** (CN4 water-flow): αν κάθοδος/άνοδος αγγίζει `BETA_FLOW_SIG` → **ΔΙΑΓΡΑΦΗ** LED-4 + R σειράς  
  → **Κράτα:** pull-up (π.χ. R16 10k) `BETA_FLOW_SIG` → `3V3` για open-collector / Hall  
  → C στο FLOW (π.χ. 100nF) προαιρετικό· αν ο αισθητήρας δίνει γρήγορους παλμούς και «χάνει», αφαίρεσέ τον αργότερα
- Ένδειξη «ζωντανός αισθητήρας» = firmware / οθόνη · όχι LED πάνω στο bus

### 6. Ονοματολογία silk
- REL2/3/4 συχνά γράφουν «K1/K2» αντί K3–K8 → μπερδεύει συναρμολόγηση
- LED-5…LED-8 vs LED-K5…K8 → ενιαίο **LED-K5…K8**
- Έλεγχος: LED δίπλα σε κάθε U* να ανήκουν στα **ίδια** nets με το header (όχι U3 LEDs σε K7/K8 κατά λάθος στο sheet)

### 7. Header Songle pin order
Επιβεβαίωσε 1:1 με το φυσικό module που αγοράζεις:

`GND · IN1 · IN2 · VCC` **ή** `VCC · IN1 · IN2 · GND`

Αν το footprint έχει `IN-2` αριστερά από `IN-1`, αυτό **είναι** η ρίζα του οπτικού X.

### 8. CN_ALPHA
- Μόνο `NC · RX · TX · GND` — **χωρίς 5V**
- Silk καθαρό TX/RX (από πλευράς ESP)
- Προαιρετικά: test pads TX/RX/GND δίπλα στο κλέμα

### 9. CT
- Μόνο **OPT1-CURRENT** / SCT-013 στο HP — **όχι** δεύτερο CT στο PCB

---

## Alpha (αν παραγγέλνεται μαζί)

| # | Έλεγχος |
|---|---------|
| 1 | Gerber από **τρέχον** EasyEDA — φαίνεται **`7" SCREEN` και `CN_BETA`** στο preview |
| 2 | `CN_BETA` = `GND · TX · RX · NC` — όχι 5V |
| 3 | Μην ανεβάσεις παλιό όνομα project (έτσι ήρθε χωρίς CN_BETA) |
| 4 | Αν `DEFROST` / άλλο net σε **GPIO12** με pull-up → μετακίνησε (ίδιο strap πρόβλημα) |
| 5 | Zener φορά/silk όπως Beta |

---

## Προαιρετικά (καλά να μπουν αν αγγίζεις PCB)

- Μεταξωτό βέλος πολικότητας σε **όλα** τα electrolytic (C3/C4) + πραγματικές τιμές BOM
- Fuse silk: τιμή + ότι είναι στον κλάδο **5V** (όχι 12V)
- Σειριακό `REV` στο copper: **v2.1** ώστε να ξεχωρίζει από την πλακέτα που έχεις τώρα
- Ένα LED status ESP (GPIO ελεύθερο) αν δεν υπάρχει

---

## Τελετουργία πριν πληρωμή fab (μην παραλείψεις)

1. EasyEDA **ERC** καθαρό  
2. **PCB Design Rule Check** καθαρό  
3. Export Gerber → άνοιξε στο **Gerber viewer** · μέτρησε οπτικά: CN_ALPHA/CN_BETA, 4× relay headers, zener, LED paths  
4. Screenshot Gerber preview → κράτα στο `docs/boards/alpha-beta/` με ημερομηνία  
5. BOM: zener 5.6V, F1, modules pinout note  

---

## Τι ΔΕΝ χρειάζεται αλλαγή για να δουλεύει το link test

- UART Alpha IO17/16 ↔ Beta CN_ALPHA (σταυρωτά TX↔RX) — **δουλεύει**
- Songle active LOW — σωστό
- Ρόλοι K1…K8 (CIRC / 3-WAY / HP / 4kW / ST-BY) — OK ως ανάθεση

---

## Σχετικά

- Bring-up: [`SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md`](../SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md)  
- Pin/LED notes: [`boards/alpha-beta/README.md`](./README.md)  
- CT rule: μόνο SCT-013 στο HP  
