# Alpha + Beta — σχέδια & φωτο bench

**Φάκελος:** EasyEDA schematic/PCB + φωτο και των δύο πλακετών (2026-08-07/08).  
Ώστε να μην ξαναανεβαίνουν στο chat κάθε φορά.

| Αρχείο | Τι είναι |
|--------|----------|
| `alpha-v1-schematic-Mitriki-ALPHA.png` | Schematic **Mitriki ALPHA** (v1.0) |
| `alpha-v1-pcb-layout.png` | PCB layout **CONTROL BOARD v1.0** |
| `beta-v2-schematic-Bita-Mitriki.png` | Schematic **Bita Mitriki** (v2.0) |
| `beta-v2-pcb-layout.png` | PCB layout **CONTROL BOARD v2.0** |
| `photo-both-boards-bench-*.png` | Φωτο bench και οι δύο μαζί |

---

## Κλέμες — σύνδεση (από σχέδια)

### Viewe 7″ → **Alpha** κλέμα `7" SCREEN` (U3)

| Silk | Net | Σημείωση |
|------|-----|----------|
| GND | GND | |
| 5V | 5V_ESP | τροφοδοσία οθόνης |
| PANEL_TX | PANEL_TX | Alpha → Viewe RX |
| PANEL_RX | PANEL_RX | Alpha ← Viewe TX |

UART ~115200 · pins ESP πλευράς σχεδίου: δίπλα σε **IO25 / IO33** (επιβεβαίωση στο schematic nets).

### Alpha `CN_BETA` (U5) ↔ Beta `CN_ALPHA` (U5)

**Alpha CN_BETA:** GND · TX · RX · NC  

**Beta CN_ALPHA:** NC · RX · TX · GND  

Καλώδιο (όνομα με όνομα, σταυρωτά TX↔RX):

| Alpha CN_BETA | ↔ | Beta CN_ALPHA |
|---------------|---|----------------|
| GND | — | GND |
| TX | → | RX |
| RX | ← | TX |
| NC | | NC |

**Όχι 5V** σε αυτό το UART κλέμα (και στις δύο πλευρές NC/χωρίς 5V στο link). Κοινό GND υποχρεωτικό. Κάθε πλακέτα έχει δικό της 12→5.

### Beta — ρόλοι ρελέ (silk)

| Module | Κανάλια |
|--------|---------|
| REL 1 | K1 CIRC-1 · K2 CIRC-2 |
| REL 2 | K3 CIRC-3 · K4 CIRC-4 |
| REL 3 | K5 3-WAY · K6 HP |
| REL 4 | K7 4kW · K8 ST-BY |

---

## Σχετικά

- [`SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md`](../SESSION-2026-08-05-CONTROL-BOARD-BRINGUP.md) — power / zener / flash GPIO12  
- [`DISPLAY-TWO-ROLES-2026-07-19.md`](../DISPLAY-TWO-ROLES-2026-07-19.md) — ρόλοι οθονών + Alpha/Beta  
- CT: μόνο SCT-013 στο HP (`OPT1-CURRENT` στη Beta)
