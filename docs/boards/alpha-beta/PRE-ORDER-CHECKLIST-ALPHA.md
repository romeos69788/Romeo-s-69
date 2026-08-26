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

### 2. `DEFROST_SIG` / GPIO strap
- **Κλειστό 2026-08-26:** H2 pin-12 = **IO14** (όχι IO12) → **OK χωρίς μετακίνηση**
- GPIO12 στο header: βεβαιώσου ότι **δεν** έχει άλλο κρίσιμο net (ιδανικά NC)

### 3. Zener (ίδιο με Beta)
- BOM/silk: **5.6V** · **κάθοδος → +5V** · άνοδος → GND

### 4. DS18 bus
- **Κράτα:** pull-up **4.7k** (`R13`) `DS18_DATA` → 3V3
- **Αφαιρέθηκαν 2026-08-26:** C11 / C22 / C3 100nF από DATA
- Προαιρετικά: σβήσε LED-1/2/3 αν είναι μόνο 3V3 always-on

### 5. UART pins — επιβεβαιωμένα 2026-08-26
| Link | FW | PCB |
|------|-----|-----|
| Alpha ↔ Beta | TX=**17**, RX=**16** | `CN_BETA` GND·TX·RX·NC |
| Alpha ↔ Viewe | TX=**25**, RX=**33** | `7" SCREEN` |

### 6. CT — όχι δεύτερο στην Alpha
- Μόνο SCT-013 στη **Beta** `OPT1-CURRENT`

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
