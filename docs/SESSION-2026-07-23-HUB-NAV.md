# Σύνοψη — hub V4 multi-screen nav (2026-07-23 βράδυ)

**Θέμα:** Panel Alpha · 6 οθόνες λεπτομέρειας + ολίσθηση · COM4

## Τι δουλεύει

| | |
|--|--|
| **EEZ** | `Othoni_Levita_4` · 7 οθόνες: `main` + `water` `hp` `out` `boiler` `system` `wifi` |
| **Firmware** | `panel/` · `panel_nav.cpp` · slim `img_8` (hub) + `img_10` (detail) |
| **Tile → detail** | `OVER_LEFT` · ξεκινά στο **πάτημα** (όχι στο άφημα) |
| **Μενού → hub** | κάτω μπάρα (βελάκι + «Μενού») · `OVER_RIGHT` |
| **Ρολόι** | NTP σε όλες τις οθόνες (ενεργή μόνο) |
| **Ticker** | idle status μόνο στο hub |

Wire export: `panel/scripts/wire_hub_v4_screens.py`

## Ανοιχτό (δεύτερος χρόνος)

Μικρό **lag / ψιλοκόλλημα** στην εναλλαγή οθόνης — αποδεκτό προς το παρόν· βελτίωση αργότερα (πιθανά: PSRAM bg, μικρότερη κίνηση, ή λιγότερο βαρύ redraw σε direct-mode).

## Flash

```powershell
cd "D:\ROMEOS 69\panel"
pio run -e BOARD_VIEWE_PANEL -t upload
```

*Συνεδρία κλεισίματος 2026-07-23 · docs + cloud sync*
