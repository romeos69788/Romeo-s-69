# Alpha (Mitriki ALPHA / CONTROL BOARD v1.x) — pre-order checklist

**Ημερομηνία:** 2026-08-09  
**Ίδια λογική με Beta:** πριν 3ο fab, κλείσε γνωστά bugs.

> Η **φυσική** πλακέτα που ήρθε χωρίς `CN_BETA` ήταν από **παλιό Gerber**.  
> Στο τρέχον EasyEDA (PCB screenshot) το `CN_BETA` **φαίνεται** — πριν πληρωμή: **Gerber viewer**.

---

## Υποχρεωτικά

### 1. `CN_BETA` στο Gerber (όχι μόνο στο schematic)
- Silk/κλέμα: **GND · TX · RX · NC**
- **Χωρίς 5V** σε αυτό το UART
- Preview Gerber: δίπλα στο `7" SCREEN` όπως στο τρέχον PCB editor
- Μην ανεβάσεις παλιό όνομα project

### 2. `DEFROST_SIG` έξω από GPIO12 (ίδιο πρόβλημα με Beta K7)
- Τώρα: H2 **IO12** = `DEFROST_SIG` (+ pull) → in-socket flash strap 1.8 V
- **Fab fix:** μετακίνησε `DEFROST_SIG` σε **input-only** pin:
  - **Πρόταση: GPIO34** (H2 · IO34) — ιδανικό για αισθητήρα
  - Εναλλακτική: GPIO35 / 39 αν ελεύθερα
- **GPIO12 = NC** (χωρίς net)
- Pull: συνήθως **pull-up 4.7k → 3V3** για open-collector / dry contact (όχι pull-down, εκτός αν το θέλει ο αισθητήρας)

### 3. Zener (ίδιο με Beta)
- BOM/silk: **5.6V** (όχι 8.2V στο part αν silk λέει 5.6)
- **Γραμμή (κάθοδος) → +5V** · άνοδος → GND

### 4. DS18 bus — όπως στη Beta
- **Κράτα:** ένα pull-up **4.7k** (`R13` ή όπως ονομάζεται) `DS18_DATA` → 3V3
- **Διαγραφή:** πυκνωτές **DATA → GND** (π.χ. C3, C22 στο schematic)
- **Διαγραφή:** LED που κάθονται στο `DS18_DATA` (αν υπάρχουν)
- Αν LED-1/2/3 είναι μόνο `3V3→R→LED→GND` (πάντα ON): **σβήστα** — αρκετό το LED-F1

### 5. UART pins — ευθυγράμμιση με firmware
| Link | FW (`alpha_pins` / link-test) | Έλεγχος schematic |
|------|-------------------------------|-------------------|
| Alpha ↔ Beta | TX=**GPIO17**, RX=**GPIO16** | H1 IO17/IO16 = `BETA_TX`/`BETA_RX` (σταυρωτά στο καλώδιο προς Beta) |
| Alpha ↔ Viewe `7" SCREEN` | TX=**GPIO25**, RX=**GPIO33** | H2 IO25/IO33 = `PANEL_TX`/`PANEL_RX` |

Αν το sheet έχει PANEL σε λάθος IO (π.χ. 26), **διόρθωσε πριν fab**.

### 6. CT — όχι δεύτερο στην Alpha
- **Μοναδικό CT** = SCT-013 στη **Beta** `OPT1-CURRENT` (γραμμή HP)
- Alpha: **χωρίς** δεύτερο CT connector/ADC για boiler total ή 4kW
- ADC GPIO35 στην Alpha μόνο αν **μεταφέρεις** το CT εδώ — αλλιώς άφησέ το NC / μην βάλεις OPT-CURRENT στην Alpha

---

## Προαιρετικά / καθάρισμα (συζήτηση πριν σβήσεις)

| Block | Σχόλιο |
|-------|--------|
| **MCP41050 + CN AC POT** | Legacy spoof OEM — κράτα αν ακόμα το χρησιμοποιείς· αλλιώς μπορεί να φύγει σε επόμενο rev |
| **AC FAN SPOOFER / 220V DEFROST zone** | Αν δεν ανήκει στην Alpha (orchestrator), υποψήφιο για HP board — **μην** το σβήσεις χωρίς απόφαση |
| **ROOM-NTC CN1** | OK αν το θέλεις· αλλιώς NC |
| Silk **v1.1** ή **rev B** | Να ξεχωρίζει από την πλακέτα χωρίς CN_BETA |

---

## Τελετουργία πριν fab

1. ERC + DRC καθαρά  
2. Gerber preview: **`7" SCREEN` + `CN_BETA` + IO12=NC + DEFROST σε IO34**  
3. Screenshot Gerber → `docs/boards/alpha-beta/` με ημερομηνία  
4. BOM zener 5.6V ευθυγραμμισμένο  

---

## Σχετικά

- Beta checklist: [`PRE-ORDER-CHECKLIST-v2.1.md`](./PRE-ORDER-CHECKLIST-v2.1.md)  
- Bring-up: [`../SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md`](../SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md)  
- CT rule: μόνο SCT-013 στο HP  
