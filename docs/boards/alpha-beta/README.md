# Alpha + Beta — σχέδια & φωτο bench

**Φάκελος:** EasyEDA schematic/PCB + φωτο πλακετών + πίσω πλευρά Viewe panel.  
Ώστε να μην ξαναανεβαίνουν στο chat κάθε φορά.

| Αρχείο | Τι είναι |
|--------|----------|
| `alpha-v1-schematic-Mitriki-ALPHA.png` | Schematic **Mitriki ALPHA** (v1.0) |
| `alpha-v1-pcb-layout.png` | PCB layout **CONTROL BOARD v1.0** |
| `beta-v2-schematic-Bita-Mitriki.png` | Schematic **Bita Mitriki** (v2.0) |
| `beta-v2-pcb-layout.png` | PCB layout **CONTROL BOARD v2.0** |
| `photo-both-boards-bench-*.png` | Φωτο bench και οι δύο μαζί |
| `viewe-panel-back-J2-pins.png` | Viewe πίσω · μπάρα **J2** (TX/RX/GND/…) |
| `viewe-panel-back-J3-pins.png` | Viewe πίσω · μπάρα **J3** (LEDK/LEDA/…) |

---

## Κλέμες — σύνδεση (από σχέδια)

### Viewe 7″ πίσω — μπάρες J2 / J3

**J2** (από πάνω προς κάτω, χρήσιμα για UART προς Alpha):

| Silk J2 | Χρήση για link Alpha |
|---------|----------------------|
| 3.3V | **Μην** γιατροφοδοτήσεις την οθόνη από εδώ για το 4-pin CN (η Alpha δίνει **5V**) |
| … | GPIO / LCD (άσχετα με UART link) |
| **TX** | TX της Viewe → πάει στο **PANEL_RX** της Alpha |
| **RX** | RX της Viewe ← από **PANEL_TX** της Alpha |
| **GND** | κοινό GND |

**J3:** κυρίως numbered ESP pins + **LEDK** / **LEDA** (στη φωτο ήδη jumper μωβ/μπλε) · όχι το βασικό UART κλέμα για `7" SCREEN`.

### Viewe ↔ **Alpha** κλέμα `7" SCREEN` (U3)

| Alpha silk | ↔ | Viewe J2 |
|------------|---|----------|
| GND | — | GND |
| 5V | → | **προσοχή:** Viewe συχνά θέλει 5V στο δικό της power path · αν η πλακέτα οθόνης παίρνει 5V από άλλο connector/USB, μην διπλοτροφοδοτείς · αλλιώς 5V στο κατάλληλο pad (όχι 3.3V του J2) |
| PANEL_TX | → | **RX** |
| PANEL_RX | ← | **TX** |

UART ~115200 · σταυρωτά TX↔RX · κοινό GND.  
ESP πλευράς Alpha: nets δίπλα σε **IO25 / IO33** (επιβεβαίωση στο schematic).

### Alpha `CN_BETA` (U5) ↔ Beta `CN_ALPHA` (U5)

**Alpha CN_BETA:** GND · TX · RX · NC  

**Beta CN_ALPHA:** NC · RX · TX · GND  

| Alpha CN_BETA | ↔ | Beta CN_ALPHA |
|---------------|---|----------------|
| GND | — | GND |
| TX | → | RX |
| RX | ← | TX |
| NC | | NC |

**Όχι 5V** σε αυτό το UART κλέμα. Κοινό GND υποχρεωτικό. Κάθε πλακέτα έχει δικό της 12→5.

### Beta — ρόλοι ρελέ (silk)

| Module | Κανάλια |
|--------|---------|
| REL 1 | K1 CIRC-1 · K2 CIRC-2 |
| REL 2 | K3 CIRC-3 · K4 CIRC-4 |
| REL 3 | K5 3-WAY · K6 HP |
| REL 4 | K7 4kW · K8 ST-BY |

---

## Σημείωση παραγγελίας (2026-08-08)

Η **φυσική Alpha v1** που ήρθε από fab ήταν από **παλιότερο Gerber** (χωρίς κλέμα `CN_BETA`) — ανέβηκε κατά λάθος αποθηκευμένο παλιό όνομα project πριν προστεθεί το κλέμα.  
**Beta v2** OK. Νέα παραγγελία Alpha με το **τρέχον** EasyEDA (με `CN_BETA` δίπλα στο `7" SCREEN`) · συνέχεια ~Σεπτέμβριο μετά διακοπές.

Πριν ξανα-upload: επιβεβαίωση στο Gerber preview ότι φαίνονται **και** `7" SCREEN` **και** `CN_BETA` (GND·TX·RX·NC).

---

## Σχετικά

- [`SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md`](../SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md) — power / zener / flash GPIO12  
- [`DISPLAY-TWO-ROLES-2026-07-19.md`](../DISPLAY-TWO-ROLES-2026-07-19.md) — ρόλοι οθονών + Alpha/Beta  
- CT: μόνο SCT-013 στο HP (`OPT1-CURRENT` στη Beta)
