# Κατασκευή αντλίας θερμότητας νερού-νερού (Romeos)

**Τελευταία ενημέρωση φακέλου:** 2026-06-14  
**EasyEDA project:** `HP-CONTROL-BOARD-v1.0` · `ROMEOS HP BOARD v1.0 — WW 24k R32`

---

## Από πού ξεκινάς

| Αν θέλεις… | Διάβασε |
|------------|---------|
| **Να εξηγήσεις σε φίλο τι κάνει η πλακέτα** | `HP-BOARD-APLA-GIA-FILO.md` |
| **Λειτουργία HP ↔ κλιματιστικό (ολοκληρωμένο)** | `HP-BOARD-LEITOURGIA.md` |
| **Να συνεχίσεις σχεδίαση EasyEDA** | `SESSION-HANDOFF_2026-06-14.md` → `HP-BOARD-EASYEDA-v1.md` |
| **Pinout ESP32 H1/H2** | `ESP32-38PIN-H1-H2-MAP.md` |
| **Ιδέα WW, cascade, υδραυλικά, unit** | `HANDOFF_2026-06-08.md` |

**Για επανέναρξη chat:** *«συνέχισε από SESSION-HANDOFF_2026-06-14»*

---

## Αρχεία φακέλου

| Αρχείο | Περιγραφή |
|--------|-----------|
| `README.md` | Αυτός ο δείκτης |
| `HANDOFF_2026-06-08.md` | Αρχικό handoff: μετατροπή unit, 3 ESP32, υδραυλικά, Equation S-AC-10-OUT |
| `SESSION-HANDOFF_2026-06-14.md` | **Σύνοψη σχεδίασης + σημερινής συζήτησης** |
| `HP-BOARD-EASYEDA-v1.md` | Οδηγός blocks, κλέμες, τροφοδοσία CN1 |
| `ESP32-38PIN-H1-H2-MAP.md` | Κλειδωμένο pin 1…19 H1/H2 |
| `HP-BOARD-LEITOURGIA.md` | Τεχνική περιγραφή λειτουργίας |
| `HP-BOARD-APLA-GIA-FILO.md` | Φιλοσοφία λειτουργίας — απλά λόγια |

**Σχετικά projects (εκτός φακέλου):** `thermostat-ui-demo/docs/romeos-design-notes.md` (μητρική), `motherboard-fw/`

---

## Κατάσταση σχεδίου (σύντομα)

- **Schematic HP:** πυρήνας **ολοκληρωμένος** στο EasyEDA (screenshot 2026-06-13/14).
- **PCB:** υπάρχει layout από Save As μητρικής — χρειάζεται sync + routing + silk διορθώσεις.
- **Πριν fab:** ERC, 6 μικρές διορθώσεις (βλ. SESSION handoff), DRC.

---

*Romeos — WW heat pump outdoor controller.*
