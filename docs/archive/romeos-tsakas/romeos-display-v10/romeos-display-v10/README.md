# romeos-display-v10 — Viewe 7″ + EEZ Studio

Firmware για την οθόνη **ESP32-S3 Viewe UEDX80480070E-WB-A** (800×480). UI από **EEZ Studio** (`romeos-display-v10.eez-project`), runtime από `romeos-display-v5`.

**Τελευταία σταθερή κατάσταση (2026-05-24):** MENU hub, εικονίδια/αισθητήρες/MIC, οικιακό Wi‑Fi, NTP, alarm, UDP προς μητρική.

Serial banner: `*** BUILD 2026-05-24-v10-mic-touch ***`

## Σημαντικό — τι είναι συνδεδεμένο στο PC

| Chip | USB | Firmware |
|------|-----|----------|
| **ESP32-S3** | USB της **οθόνης Viewe** (CH340) — συνήθως **COM3** | `BOARD_VIEWE_V10` |
| **ESP32** (όχι S3) | π.χ. CP210x **COM7** = μητρική / άλλο board | **όχι** αυτό το project |

Αν το upload γράφει `ESP32 not ESP32-S3`, το USB **δεν** είναι στην οθόνη S3.

## Οθόνες (EEZ)

| Οθόνη | Ρόλος |
|-------|--------|
| **PRYMARY** | Κύρια · setpoint +/- · MENU |
| **MENU** | Hub: Θέρμανση, Λέβητα, Φώτα, Ρολά · πίνακες θερμοκρασίας |
| **HEAT / BOILER / LIGHT / BLINDS** | Υποοθόνες · BACK → MENU |

## Λειτουργίες runtime

- **Οικιακό Wi‑Fi** (`ROMEOS_WIFI_HOME_ONLY`) — SSID από NVS ή `secrets/wifi_home.ini`
- **Σπίτι** → γαλάζιο (`#008cec`) όταν συνδέεται στο αποθηκευμένο SSID
- **Λεβήτα** (κάτω δεξιά) → γαλάζιο όταν η μητρική απαντά UDP στο LAN (`ROMEOS_MB_LAN_IP`)
- **Εσωτερική θερμοκρασία/υγρασία** → SHT στην οθόνη (I2C touch bus)
- **Εξωτερική θερμοκρασία** → DHT μητρικής (`outdoor_c_x10` μέσω UDP) · υγρασία `--` (δεν υπάρχει ακόμα στο πρωτόκολλο)
- **Ρολόι / ημερομηνία** → NTP (Ελλάδα) — ξεχωριστά `obj` ανά οθόνη (βλ. [docs/UI_RUNTIME.md](docs/UI_RUNTIME.md))
- **ALARM** → κόκκινο εικονίδιο + κείμενο + buzzer (GPIO 17)
- **MIC** → toggle · κόκκινο εικονίδιο + «MIC» (stub φωνής — Serial `MIC ON/OFF`)
- **BACK** → PRYMARY ή MENU ανάλογα με την οθόνη

Λεπτομέρειες Wi‑Fi: **[docs/WIFI-HOME.md](docs/WIFI-HOME.md)**

## Οικιακό Wi‑Fi (γρήγορα)

1. `copy secrets\wifi_home.ini.example secrets\wifi_home.ini` και συμπλήρωσε SSID/κωδικό.
2. **Ή** Serial 115200: `WIFI_HOME_SET` → SSID → κωδικός.
3. **Ή** `..\..\scripts\SET-WIFI-DISPLAY-COM3.bat` (κλείσε Serial Monitor).

## Build / upload

```powershell
cd "D:\Romeos Tsakas\romeos-display-v10\romeos-display-v10"
pio device list
pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3
pio device monitor --port COM3 -b 115200
```

## Δομή φακέλων

| Φάκελος / αρχείο | Περιεχόμενο |
|-------------------|-------------|
| `src/ui/` | EEZ export (screens, fonts, images) |
| `src/romeos_eez_runtime.cpp` | Runtime: οθόνες, αισθητήρες, alarm, MIC, Wi‑Fi UI |
| `src/main_eez_central.cpp` | Entry point, LVGL init |
| `src/romeos_display_link.cpp` | Wi‑Fi STA, UDP προς μητρική |
| `src/romeos_wifi_nvs.cpp` | NVS home credentials |
| `src/romeos_ntp.cpp` | SNTP |
| `src/romeos_room_sht_touch_bus.cpp` | Αισθητήρας δωματίου (SHT/AHT) |
| `romeos-display-v10.eez-project` | Πηγαίο EEZ project |
| `secrets/` | `wifi_home.ini` (τοπικό, gitignored) |
| `scripts/` | `wifi_home_build_flags.py` |
| `boards/` | Board definition Viewe |
| `docs/` | Τεκμηρίωση |

Κοινά types/link: `../../shared/include/romeos_link_types.h`

## EEZ → firmware

Μετά από export στο `src/ui/`:

1. Στα `.h` / `.c` του UI: `#include <lvgl/lvgl.h>` → `#include "lvgl.h"`
2. Στο `src/ui/ui.c`: `lv_scr_load` + `romeos_eez_runtime_on_screen_loaded()` (όχi μόνο fade anim)
3. Εικονίδιο hub **ΛΕΒΗΤΑ**: `img_boiler1` (όχι `img_boiler`) — βλ. [docs/EEZ_EXPORT.md](docs/EEZ_EXPORT.md)
4. Rebuild: `pio run -e BOARD_VIEWE_V10 -t upload`

## Τεκμηρίωση

| Έγγραφο | Περιεχόμενο |
|---------|-------------|
| [docs/HANDOFF_2026-05-24.md](docs/HANDOFF_2026-05-24.md) | Handoff συνεδρίας · τι διορθώθηκε |
| [docs/UI_RUNTIME.md](docs/UI_RUNTIME.md) | Αντιστοίχιση widgets / αισθητήρες / εικονίδια |
| [docs/EEZ_EXPORT.md](docs/EEZ_EXPORT.md) | Checklist μετά από EEZ export |
| [docs/WIFI-HOME.md](docs/WIFI-HOME.md) | Wi‑Fi, NTP, UDP μητρική |

## Αντιμετώπιση

| Σύμπτωμα | Πιθανή αιτία |
|----------|----------------|
| Flicker / άσπρη οθόνη | Anti-tear mode **3** — μην βάλεις mode 1 |
| `PermissionError` COM3 | Κλείσε monitor / άλλο process |
| Λάθος ημερομηνία στο MENU | Έλεγξε mapping `obj54–56` (όχι `obj47–49`) — [UI_RUNTIME.md](docs/UI_RUNTIME.md) |
| Εξωτερική = εσωτερική | Παλιό firmware ή χωρίς UDP telemetry από μητρική |
| Λάθος εικονίδιο ΛΕΒΗΤΑ | `img_boiler` αντί `img_boiler1` στο hub |
| MIC δεν πατιέται | Χρειάζεται `mic_bind_all_screens()` στο runtime (ή νέο flash) |
