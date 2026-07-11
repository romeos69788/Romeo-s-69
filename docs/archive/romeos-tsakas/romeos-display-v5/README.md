# romeos-display-v5 — othoni_7 (Viewe 7″)

Firmware για την οθόνη **ESP32-S3** (EEZ UI `othoni_7`, 800×480). Οικιακό Wi‑Fi, NTP, αισθητήρας χώρου, alarm UI· UDP προς μητρική όταν μπει στο ίδιο LAN.

**Τελευταία σταθερή κατάσταση (2026-05-21):** UI χωρίς flicker (anti-tear mode 3), home Wi‑Fi `MERCUSYS_Romeos`, NTP ενεργό μετά τη σύνδεση.

## Σημαντικό — τι είναι συνδεδεμένο στο PC

| Chip | USB | Firmware |
|------|-----|----------|
| **ESP32-S3** | USB της **οθόνης Viewe** (CH340) — συνήθως **COM3** | `BOARD_VIEWE_OTHONI_7` |
| **ESP32** (όχι S3) | π.χ. CP210x **COM7** = μητρική / άλλο board | **όχι** αυτό το project |

Αν το upload γράφει `ESP32 not ESP32-S3`, το USB **δεν** είναι στην οθόνη S3.

## Λειτουργία τώρα

- **Οικιακό Wi‑Fi** μόνο (`ROMEOS_WIFI_HOME_ONLY`) — όχι RomeosMB SoftAP, όχι UART προς μητρική.
- **Σπιτι** → γαλάζιο όταν συνδέεται στο αποθηκευμένο SSID (`HOME_ASSOC=yes`).
- **Λεβητα** → γαλάζιο όταν η μητρική απαντά στο ίδιο LAN (UDP + `ROMEOS_MB_LAN_IP`).
- **Ώρα** → SNTP μετά `WiFi OK` (ζώνη Ελλάδας).
- **ALARM** → κόκκινο μόνο το κείμενο.

Λεπτομέρειες Wi‑Fi / NTP: **[docs/WIFI-HOME.md](docs/WIFI-HOME.md)**

## Οικιακό Wi‑Fi (γρήγορα)

1. **Πρώτη φορά:** `copy secrets\wifi_home.ini.example secrets\wifi_home.ini` και συμπλήρωσε SSID/κωδικό (το `.ini` δεν μπαίνει στο git).
2. **Ή** Serial 115200: `WIFI_HOME_SET` → SSID → κωδικός.
3. **Ή** `..\..\scripts\SET-WIFI-DISPLAY-COM3.bat` (κλείσε πρώτα Serial Monitor).

SSID πρέπει να ταιριάζει **ακριβώς** με το router (π.χ. `MERCUSYS_Romeos`). Το ESP βλέπει **2.4 GHz** — αν το δίκτυο είναι μόνο 5 GHz, δεν συνδέεται.

## Όταν υπάρχει μητρική (ίδιο Wi‑Fi σπιτιού)

Στο `platformio.ini` του env `BOARD_VIEWE_OTHONI_7`:

```ini
-DROMEOS_MB_LAN_IP=\"192.168.0.xxx\"
```

Η μητρική στο ίδιο SSID — επικοινωνία **UDP**, όχι καλώδιο UART.

## Build / upload

```powershell
cd "D:\Romeos Tsakas\romeos-display-v5"
pio device list
pio run -e BOARD_VIEWE_OTHONI_7 -t upload --upload-port COM3
pio device monitor --port COM3 -b 115200
```

Serial banner: `*** BUILD 2026-05-21-home-wifi-auto ***`

## Δομή φακέλων

| Φάκελος / αρχείο | Περιεχόμενο |
|-------------------|-------------|
| `src/othoni_ui/` | EEZ export (UI, fonts, images) |
| `src/romeos_eez_runtime.cpp` | Runtime, οθόνες, Wi‑Fi UI, ρολόι |
| `src/romeos_display_link.cpp` | Wi‑Fi STA, UDP telemetry |
| `src/romeos_wifi_nvs.cpp` | NVS home credentials |
| `src/romeos_ntp.cpp` | SNTP |
| `othoni_7/` | Πηγαίο EEZ project (SquareLine) |
| `secrets/` | `wifi_home.ini` (τοπικό, gitignored) |
| `scripts/` | `wifi_home_build_flags.py` (pre-build) |
| `docs/` | Τεκμηρίωση (π.χ. `WIFI-HOME.md`) |

## EEZ → firmware

Εξαγωγή από `othoni_7/src/ui/` → αντιγραφή σε `src/othoni_ui/` · στα `.h`: `#include "lvgl.h"`

## Άλμπουμ backgrounds (επιλογή εικόνας)

Για να δεις **στην πραγματική οθόνη** πώς φαίνεται κάθε background (φωτεινότητα/χρώμα) **χωρίς** 10 ξεχωριστά projects:

1. Βάλε PNG/JPG **800×480** στο `preview_album/images/`
2. `pip install pillow`
3. `pio run -e BOARD_VIEWE_BG_PREVIEW -t upload --upload-port COM3`
4. Swipe αριστερά/δεξιά · όταν διαλέξεις → EEZ project → `BOARD_VIEWE_OTHONI_7`

Λεπτομέρειες: **[preview_album/README.md](preview_album/README.md)**

## Αντιμετώπιση

| Σύμπτωμα | Πιθανή αιτία |
|----------|----------------|
| Flicker / άσπρη οθόνη | Μην βάζεις `CONFIG_LVGL_PORT_AVOID_TEARING_MODE=1` — κράτα mode **3** |
| `PermissionError` COM3 | Κλείσε monitor / άλλο process στο COM3 |
| Κόκκινο Σπιτι | Δεν υπάρχουν credentials ή λάθος SSID/κωδικός / μόνο 5 GHz |
| `disc_reason=201` | AP δεν βρέθηκε — SSID ή ζώνη 2.4 GHz |
| Ώρα λάθος | Περίμενε NTP μετά `WiFi OK` ή έλεγξε router/firewall για NTP |
