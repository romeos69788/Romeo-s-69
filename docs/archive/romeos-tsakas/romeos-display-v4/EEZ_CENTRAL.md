# EEZ central_screen_1 — Viewe 7″ (romeos-display-v4)

Τρέχον firmware: **`BOARD_VIEWE_EEZ_CENTRAL`**

## Build / upload

```powershell
cd "d:\Romeos Tsakas\romeos-display-v4"
pio run -e BOARD_VIEWE_EEZ_CENTRAL -t upload
```

Port: **COM3** (`platformio.ini`).

## Αρχεία

| Ρόλος | Διαδρομή |
|--------|-----------|
| Entry | `src/main_eez_central.cpp` |
| UI export | `src/eez_central/` |
| Setpoint (arc + +/-) | `src/romeos_setpoint_ui.cpp` |
| Wi‑Fi, NTP, sensor, alarm | `src/romeos_eez_runtime.cpp` |
| EEZ project | `D:\OTHONI New VERSION\blakc_scren\central_screen_1.eez-project` |

## Λειτουργίες (τελευταία επιβεβαίωση)

- Setpoint **10.0–30.0 °C**, βήμα **0.5**, κείμενο **22.5** / **21.0**
- Arc **διαφανής** + πράσινη **μπίλια** `#88EE37` (knob)
- Κουμπιά **minus** / **plus**
- Wi‑Fi σπιτιού (NVS): `obj10` + `obj26` — γαλάζιο / κόκκινο
- Ρολόι **24ωρο** + **AM/PM**, `:` αναβοσβήνει
- Ημερομηνία: `obj2`, `obj3`, `obj25`
- Δωμάτιο SHT: `obj15`, `obj18`
- Alarm: `alarm_button` / `alarm`, buzzer **GPIO 17**

## EEZ → firmware (μετά από αλλαγές στο Studio)

1. **Build** στο `central_screen_1.eez-project`
2. Αντιγραφή `blakc_scren\src\ui\` → `romeos-display-v4\src\eez_central\`
3. Αντικατάσταση `#include <lvgl/lvgl.h>` → `#include "lvgl.h"` (σε όλα τα `.c`/`.h`)
4. **Flash trim:** στο `screens.c` δες ποιο `img_*` χρησιμοποιείται (τώρα `img_32`). Κράτα μόνο το αντίστοιχο `ui_image_*.c` και ενημέρωσε `images.c` / `images.h`
5. Build/upload — αν >16 MB, διέγραψε unused fonts/images

**Σημαντικό:** Μην βάζεις ξανά πράσινο arc στο `romeos_setpoint_ui.cpp` — ο χρήστης θέλει διαφανές τόξο και πράσινο μόνο στη μπίλια.

## Αντιστοίχιση widgets (central_screen_1)

| Widget | Object |
|--------|--------|
| Setpoint ακέραιο | `obj1` |
| `.` / δεκαδικό | `obj4`, `obj5` |
| `°` | `obj6` |
| Arc (touch) | `obj8` |
| +/- | `minus`, `plus` |
| Ώρα / `:` / λεπτά / AM-PM | `obj11`–`obj14` |
| Ημέρα / αριθμός / μήνας | `obj2`, `obj3`, `obj25` |
| Wi‑Fi κείμενο | `obj10`, `obj26` |
| Θερμοκρ. / υγρασία δωματίου | `obj15`, `obj18` |
| Συναγερμός κείμενο | `obj7` |

## Snapshot επαναφοράς

**Τρέχον:** `snapshots/2026-05-17_central_screen_1-OK/RESTORE.txt`

## Wi‑Fi (Serial 115200)

`WIFI_HOME_SET` · `WIFI_HOME_SHOW` · `WIFI_HOME_CLR`
