# ROMEOS 69

**Τοπικός φάκελος:** `D:\ROMEOS 69`

Σύστημα ελέγχου θέρμανσης / αντλίας θερμότητας.

## Δομή

```
D:\ROMEOS 69\
├── alpha\          ← μητρική καυστήρα (ESP32 · Alpha rev A)
├── beta\           ← πλακέτα εξωτερικού / αίθριου
├── display\        ← οθόνες θερμοστάτη (room)
├── panel\          ← Viewe panel λεβητοστασίου (hub preview · COM4)
├── Othoni Levita\  ← EEZ Studio project (hub UI)
├── docs\           ← τεκμηρίωση + ΣΥΖΗΤΗΣΗ.md
├── secrets\        ← κωδικοί / credentials (ΔΕΝ στο git)
├── shared\         ← κοινός κώδικας (π.χ. romeos-remote)
└── README.md
```

## Γρήγορη εκκίνηση

### Alpha firmware

```powershell
cd "D:\ROMEOS 69\alpha"
pio run -e alpha_rev_a_bench
```

### iPhone remote

```powershell
cd "D:\ROMEOS 69\shared\romeos-remote"
npm install
npm run dev
```

### Sync με GitHub

```powershell
cd "D:\ROMEOS 69"
.\docs\scripts\sync-romos69.ps1
```

## Συζήτηση & docs

- [`docs/ΣΥΖΗΤΗΣΗ.md`](docs/ΣΥΖΗΤΗΣΗ.md) — ιστορικό συζητήσεων Cursor
- [`docs/SESSION-2026-07-19.md`](docs/SESSION-2026-07-19.md) — panel hub V1 στο γυαλί · περιμένουμε V2
- [`docs/panel-hub/HUB-VERSIONS.md`](docs/panel-hub/HUB-VERSIONS.md) — εκδόσεις οπτικού hub
- [`docs/PANEL-HUB-6-TILES-LOCK-2026-07-19.md`](docs/PANEL-HUB-6-TILES-LOCK-2026-07-19.md) — δομή 6 πλαισίων
- [`docs/ALPHA-REV-A.md`](docs/ALPHA-REV-A.md) — hardware Alpha rev A
- [`docs/FAN-MOTOR-BOARD.md`](docs/FAN-MOTOR-BOARD.md) — πλακέτα βεντιλατήρα (5 vs 4 καλώδια)
- [`docs/HP-BOARD-LEITOURGIA.md`](docs/HP-BOARD-LEITOURGIA.md) — HP outdoor board
- [`docs/SYNOLOGI-IMPORT.md`](docs/SYNOLOGI-IMPORT.md) — import από `D:\Romeos Tsakas`
- [`docs/ΣΥνδεσμοι.md`](docs/ΣΥνδεσμοι.md) — GitHub, PR, links

### Panel hub flash (COM4)

```powershell
cd "D:\ROMEOS 69\panel"
pio run -e BOARD_VIEWE_PANEL -t upload
```

## GitHub

https://github.com/romeos69788/Romeo-s-69
