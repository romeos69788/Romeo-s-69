# FINAL PRE-ORDER REVIEW — Alpha + Beta (2026-08-26)

**Πηγές:** EasyEDA screenshots (schematic+PCB ×2) + checklists + `alpha_pins.h` + session 2026-08-08  
**Verdict:** **ΜΗΝ παραγγέλνεις ακόμα** μέχρι να κλείσουν τα **BLOCKER** παρακάτω.

---

## Φιλοσοφία (υπενθύμιση)

| | **Alpha (Mitriki ALPHA)** | **Beta (Bita Mitriki)** |
|--|--------------------------|-------------------------|
| Ρόλος | Εγκέφαλος λεβητοστασίου | Έξοδοι ρελέ + αισθητήρες πεδίου |
| Έχει | Sensors (DS18/NTC), RTC, buzzer, I2C, Viewe UART, Wi‑Fi/ESP‑NOW, UART master → Beta | 8× Songle, CT HP, flow, water DS18, UART slave |
| **Δεν** έχει | Ρελέ 230 V, CT boiler/4kW | Panel 7″, RTC |
| Link | `CN_BETA` | `CN_ALPHA` |
| CT | **Όχι** δεύτερο CT | **Μόνο** SCT-013 στο `OPT1-CURRENT` (γραμμή HP) |

Καλώδιο UART (χωρίς 5V):

```
Alpha CN_BETA TX  →  Beta CN_ALPHA RX
Alpha CN_BETA RX  ←  Beta CN_ALPHA TX
Alpha CN_BETA GND —  Beta CN_ALPHA GND
NC                —  NC
```

FW αναφορά: Alpha **TX=GPIO17, RX=GPIO16** · Panel **TX=25, RX=33**.

---

## BLOCKER — διόρθωσε πριν fab

### A) Alpha — επικοινωνία / pins

| # | Πρόβλημα | Τι να κάνεις |
|---|----------|--------------|
| A1 | ~~DEFROST στο GPIO12~~ | **Κλειστό 2026-08-26:** H2 pin-12 = **IO14** (όχι IO12) · OK, χωρίς μετακίνηση |
| A2 | ~~CN_BETA με 5V~~ | **Κλειστό:** GND · TX · RX · NC |
| A3 | ~~Διπλό U5~~ | **Κλειστό:** κλέμα = U5/`CN_BETA` · MCP41050 μόνο chip code |
| A4 | UART `CN_BETA` → ESP | **Επιβεβαιωμένο χρήστη:** TX/RX σωστά (17/16) |
| A5 | `7" SCREEN` | **Επιβεβαιωμένο χρήστη** |
| A6 | DS18: **C11 / C22 / C3 100nF στο DATA** | **ΑΦΑΙΡΕΣΕ** τα τρία · κράτα μόνο **R13 4.7k** |
| A7 | Zener | **OK** χρήστη |

### B) Beta — επικοινωνία / pins

| # | Πρόβλημα | Τι να κάνεις |
|---|----------|--------------|
| B1 | **Επιβεβαίωσε** `REL_K7` = silk **IO19** μόνο | IO12 **χωρίς** net ρελέ |
| B2 | Αν OPT2/κάτι ακόμα δένει σε **GPIO12** | Μετακίνησε · GPIO12 μόνο NC ή ασφαλές input χωρίς strap conflict στο boot |
| B3 | Zener schematic **8V2** vs silk 5.6V | Ίδιο με Alpha → **5V6** |
| B4 | PCB: επιβεβαίωσε `CN_ALPHA` | `NC · RX · TX · GND` · RX→ESP RX net, TX→ESP TX · σταυρωτά στο καλώδιο προς Alpha |

### C) Και τα δύο — τελετουργία

1. ERC + **DRC = 0** (Beta ήδη OK)  
2. **Gerber viewer** οπτικά: `CN_BETA` + `CN_ALPHA` + `7" SCREEN`  
3. Screenshot Gerber → `docs/boards/alpha-beta/` με ημερομηνία  
4. Μην ανεβάσεις παλιό όνομα project  

---

## OK / αποδεκτό (μην το «χαλάς»)

### Beta
- LED-K: `5V → R → LED → REL_Kx` (active LOW) ✓  
- Επίτηδες σταύρωμα LED-Kx ↔ REL_Ky ανά ζεύγος (οπτική αριστερά/δεξιά) ✓ — κράτα comment στο sheet  
- DS18: **R9 4.7k**, χωρίς LED/C στο bus (στο τελευταίο Beta sheet) ✓  
- Flow: **R16 10k** ✓  
- CT μόνο OPT1 ✓  
- `CN_ALPHA` χωρίς 5V στο PCB silk ✓  
- 4× dual relay + ρόλοι CIRC / 3-WAY / HP / 4kW / ST-BY ✓  

### Alpha
- `CN_BETA` **υπάρχει** στο PCB layout (βελτίωση vs παλιά παραγγελία) ✓  
- `7" SCREEN` υπάρχει ✓  
- I2C RTC / outdoor / buzzer ✓  
- Ένα CT connector στην Alpha **δεν** φαίνεται — σωστό ✓  

---

## Προαιρετικά ΑΦΑΙΡΕΣΕ (καθάρισμα, όχι blocker αν βιάζεσαι)

| Πού | Τι | Γιατί |
|-----|-----|--------|
| Alpha | **LED-1, LED-2, LED-3** (3V3 πάντα ON) | Άχρηστα · αρκεί LED-F1 |
| Alpha | **LED-7** pre-fuse αν θες μόνο post-fuse | Καλλωπισμός |
| Alpha | **MCP41050 + CN1 AC POT** αν δεν spoofάρεις πλέον OEM | Floating SPI στο sheet · ή **σύνδεσε** SPI σε ESP ή **σβήσε** το block |
| Alpha | AC FAN SPOOFER / 220V defrost zone | Μόνο αν αποφασίσεις ότι ανήκει στην HP board — **συζήτηση**, μην σβήνεις βιαστικά |
| Beta | Silk «REL 2 K1/K2» → γράψε K3/K4 κ.λπ. | Σαφήνεια συναρμολόγησης |
| Beta | LED-5/LED-6 → **LED-K5/K6** | Ονοματολογία |

---

## Προαιρετικά ΠΡΟΣΘΕΣΕ

| Τι | Γιατί |
|----|--------|
| Silk **v2.1** (Beta) / **v1.1** (Alpha) | Ξεχωρίζει από παλιές πλακέτες |
| Βέλος καθόδου στο zener | Αποφυγή ανάποδης κόλλησης |
| Test pads TX/RX/GND δίπλα CN_ALPHA / CN_BETA | Bench χωρίς βύσμα |
| Σημείωση στο sheet: «LED silk = φυσική θέση· nets σταυρωτά επίτηδες» | Για τον εαυτό σου σε 6 μήνες |

---

## Πίνακας καλωδίου Alpha↔Beta (τελικός έλεγχος στο Gerber)

| Alpha `CN_BETA` silk | Καλώδιο | Beta `CN_ALPHA` silk | ESP Alpha | ESP Beta |
|----------------------|---------|----------------------|-----------|----------|
| GND | — | GND | GND | GND |
| TX | → | RX | **GPIO17** | RX net (16 ή 3 — όπως στο PCB σου· FW Serial2 αν 16) |
| RX | ← | TX | **GPIO16** | TX net |
| NC | | NC | — | — |

**Απαγορεύεται 5V** σε αυτό το κλέμα.

---

## FW μετά το fab (υπενθύμιση — όχι για Gerber)

- Beta: `REL_K7` = **GPIO19** · pair-swap LED όπως hardware  
- Alpha: DEFROST = **GPIO34** · DS18 χωρίς C · Panel 25/33 · Beta 17/16  
- `alpha_pins.h` σήμερα ακόμα έχει παλιά relays στο Alpha — αγνόησέ τα για το PCB (ρελέ μόνο στη Beta)

---

## Σύνοψη go / no-go

| | Κατάσταση |
|--|-----------|
| **Beta PCB DRC** | OK (0) — συνέχισε fixes A/B blockers στο schematic |
| **Beta έτοιμη για fab;** | **Ναι** — DRC 0 · χρήστης OK · Gerber preview πριν πληρωμή |
| **Alpha έτοιμη για fab;** | **Ναι** — DEFROST=IO14 OK · CN_BETA OK · DS18 caps αφαιρέθηκαν · Gerber preview πριν πληρωμή |

**Επόμενο βήμα:** Gerber viewer και τα δύο · screenshots στο φάκελο · παραγγελία.  
**Session:** [`SESSION-2026-08-26-PREORDER-LOCK.md`](../SESSION-2026-08-26-PREORDER-LOCK.md)
