# Panel hub EEZ — πρώτη οπτική έκδοση

**Ημερομηνία:** 2026-07-19  
**Path PC:** `D:\ROMEOS 69\Othoni Levita\Othoni_Levita`  
**Hardware:** Viewe 7″ · **COM4** (χρήστης)  
**Κατάσταση οπτικού:** υπό συζήτηση · δομή 6 πλαισίων ήδη κλειδωμένη

---

## Τι φαίνεται στο screenshot

| Θέση | Label | Χρώμα / εικονίδιο |
|------|-------|-------------------|
| Πάνω Α | **WATER** | μπλε · σταγόνα + κύματα |
| Πάνω Μ | **HEAT PUMP** | πράσινο · outdoor unit |
| Πάνω Δ | **OUTPUTS** | κόκκινο · power symbol |
| Κάτω Α | **BOILER** | πορτοκαλί · κυλινδρικό μπόιλερ |
| Κάτω Μ | **SYSTEM** | γκρι · γρανάζι |
| Κάτω Δ | **NETWORK** | γαλάζιο · nodes |

Κάτω μπάρα: ημερομηνία + ώρα.

---

## Σχόλια agent (όχι lock)

- Grid 2×3 + chevron `>` · σωστό μοτίβο hub→detail
- Αγγλικά labels · συνεπή με AI brief
- Χρωματικός διαχωρισμός τομέων · καλό για λεβητοστάσιο
- Μικτά στυλ icons (3D HP/boiler vs flat power/gear) — αισθητική επιλογή χρήστη
- Επόμενο τεχνικό: PlatformIO upload COM4 · navigation σε detail screens

---

## Build / upload (στο PC του χρήστη)

```powershell
cd "D:\ROMEOS 69\Othoni Levita\Othoni_Levita"
pio run -t upload --upload-port COM4
pio device monitor --port COM4
```

*(Προσάρμοσε `-e` στο env του `platformio.ini` αν υπάρχει συγκεκριμένο Viewe env.)*

---

*Cloud agent δεν έχει πρόσβαση στο D:\ — χρειάζεται git add από το PC ή copy στο repo.*
