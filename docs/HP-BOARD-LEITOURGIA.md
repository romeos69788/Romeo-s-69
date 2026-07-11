# HP Outdoor Board — Λειτουργία

> **Status:** placeholder — περιμένει import από `D:\Romeos Tsakas`

Το πρωτότυπο αρχείο αναμένεται εδώ:

```
D:\Romeos Tsakas\κατασκευή αντλίας θερμότητας νερού νερού\HP-BOARD-LEITOURGIA.md
```

## Import

```powershell
cd "D:\ROMEOS 69"
.\docs\scripts\import-from-romeos-tsakas.ps1
```

Μετά το import, αυτό το αρχείο θα αντικατασταθεί/ενημερωθεί από το script.

## Γνωστά (από project inventory)

- RS485 bus: ESP32 UART2 TX=17, RX=16, DE+RE=32
- Transceiver: MAX485EPA (DIP8) — 2 chips ανά link
- CN_BUS A/B/GND προς HP outdoor unit
- **Δεν** χρησιμοποιείται στο Alpha rev A (reserved future)

## Σχετικά

- [`FAN-MOTOR-BOARD.md`](FAN-MOTOR-BOARD.md) — πλακέτα μοτέρ βεντιλατήρα (5 vs 4 καλώδια)
- [`ALPHA-REV-A.md`](ALPHA-REV-A.md) — μητρική καυστήρα
