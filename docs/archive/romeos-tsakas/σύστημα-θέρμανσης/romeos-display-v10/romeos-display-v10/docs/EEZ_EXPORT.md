# EEZ Studio — checklist μετά από export

Όταν κάνεις export από `romeos-display-v10.eez-project` στο `src/ui/`, ακολούθησε **πάντα** τα παρακάτω πριν το upload.

## 1. Includes LVGL

Σε όλα τα `src/ui/*.h` και `src/ui/*.c`:

```diff
- #include <lvgl/lvgl.h>
+ #include "lvgl.h"
```

Γρήγορα (PowerShell από root project):

```powershell
Get-ChildItem src\ui -Include *.h,*.c -Recurse | ForEach-Object {
  (Get-Content $_.FullName -Raw) -replace '#include <lvgl/lvgl.h>', '#include "lvgl.h"' | Set-Content $_.FullName -NoNewline
}
```

## 2. `src/ui/ui.c` — φόρτωση οθόνης

Το EEZ ξαναγράφει `loadScreen` με fade. Πρέπει να μείνει:

```c
#include "../romeos_eez_runtime.h"

void loadScreen(enum ScreensEnum screenId) {
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    lv_scr_load(screen);
    romeos_eez_runtime_on_screen_loaded();
}
```

## 3. Εικονίδιο ΛΕΒΗΤΑ (hub)

Στο `screens.c`, για **κεντρικό MENU** και **κεφαλίδα οθόνης boiler**:

- `objects.______` (MENU hub)
- `objects._______1` (οθόνη boiler)

```c
lv_obj_set_style_bg_img_src(obj, &img_boiler1, LV_PART_MAIN | LV_STATE_DEFAULT);
```

**Όχι** `&img_boiler` — αυτό είναι το sync icon (status).

Κράτα `&img_boiler` μόνο για: `boiler__`, `boiler_wifi`.

Το runtime εφαρμόζει και `boiler_hub_icons_apply()` — αλλά καλό είναι να είναι σωστό και στο export.

## 4. Έλεγχος `screens.h` / νέα ονόματα

Αν το EEZ άλλαξε ονόματα κουμπιών hub, ενημέρωσε το `bind_hub_buttons()` στο `romeos_eez_runtime.cpp`.

## 5. Build

```powershell
pio run -e BOARD_VIEWE_V10 -t upload --upload-port COM3
```

Έλεγξε Serial banner (άλλαξε την γραμμή `BUILD` στο `main_eez_central.cpp` όταν κάνεις σημαντική αλλαγή).

## 6. Smoke test

- [ ] PRYMARY: setpoint +/-, MENU, ρολόι
- [ ] MENU: hub 4 εικονίδια, εσωτερική ≠ εξωτερική (αν υπάρχει μητρική)
- [ ] BACK από υποοθόνη → MENU
- [ ] ALARM κόκκινο + buzzer
- [ ] MIC toggle κόκκινο
- [ ] «Σπίτι» γαλάζιο με Wi‑Fi

Αναλυτικές αντιστοιχίσεις widgets: [UI_RUNTIME.md](UI_RUNTIME.md)
