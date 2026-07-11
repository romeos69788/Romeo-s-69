# UI runtime — αντιστοιχίσεις widgets (v10)

Το EEZ δίνει generic ονόματα (`obj8`, `______`, κ.λπ.). Το `romeos_eez_runtime.cpp` χρησιμοποιεί **σταθερά IDs** — μετά από EEZ re-export, έλεγξε ότι τα `objects.*` δείχνουν στα ίδια widgets.

## Οθόνες

| Enum | Αρχείο EEZ | `objects.*` root |
|------|------------|------------------|
| PRYMARY | `create_screen_prymary` | `objects.prymary` |
| MENU | `create_screen_menu` | `objects.menu` |
| HEAT | `create_screen_heat` | `objects.heat` |
| BOILER | `create_screen_boiler` | `objects.boiler` |
| LIGHT | `create_screen_light` | `objects.light` |
| BLINDS | `create_screen_blinds` | `objects.blinds` |

## Ρολόι / ημερομηνία

**Μην μπερδεύεις PRYMARY με MENU** — τα ίδια αριθμητικά `obj` σημαίνουν διαφορετικά πράγματα.

### PRYMARY

| Widget | Σκοπός |
|--------|--------|
| `obj18`, `obj20` | Ώρα (ώρα, λεπτά) |
| `obj28` | Ημέρα εβδομάδας |
| `obj29` | Αριθμός ημέρας |
| `obj30` | Μήνας |

### MENU

| Widget | Σκοπός |
|--------|--------|
| `obj41`, `obj42` | Ώρα |
| `obj54` | Ημέρα εβδομάδας |
| `obj55` | Αριθμός ημέρας |
| `obj56` | Μήνας |

**Λάθος mapping (παλιό bug):** `obj47`–`obj49` στο MENU είναι ALARM / MIC / BACK — **όχι** ημερομηνία.

## Θερμοκρασία / υγρασία

Θέση Y: πάνω πάνελ ≈ εξωτερική (Y~68), κάτω ≈ εσωτερική (Y~205).

| Widget | PRYMARY | MENU | Πηγή δεδομένων |
|--------|---------|------|----------------|
| Θερμοκρασία εξωτερική | `obj8` | `obj31` | UDP `outdoor_c_x10` (μητρική) |
| Θερμοκρασία εσωτερική | `obj9` | `obj32` | SHT οθόνης |
| Υγρασία εξωτερική | `obj14` | `obj37` | `--` (δεν υπάρχει στο UDP v1) |
| Υγρασία εσωτερική | `obj15` | `obj38` | SHT οθόνης |

Χρώμα υγρασίας εσωτερικής: cyan `#4DD0E1`.

## Εικονίδια boiler (δύο assets)

| Asset EEZ | C symbol | Χρήση |
|-----------|----------|--------|
| `boiler` | `img_boiler` | Sync icon 40×40 — status «Λεβήτα» (`boiler__`, `boiler_wifi`) |
| `BOILER` | `img_boiler1` | Hub 64×64 — κεντρικό MENU + κεφαλίδα οθόνης boiler (`______`, `_______1`) |

Το runtime καλεί `boiler_hub_icons_apply()` μετά από κάθε `loadScreen` για να μην χαθεί μετά export.

## Wi‑Fi / μητρική (κείμενα)

| Widget | Οθόνη | Σκοπός |
|--------|-------|--------|
| `obj26` | PRYMARY | «Σπίτι» |
| `obj52` | MENU | «Σπίτι» |
| `obj27` | PRYMARY | «Λεβητα» (label) |
| `obj53` | MENU | «Λεβητα» |

Χρώμα `#008cec` όταν active (home Wi‑Fi / MB link).

## Footer — ALARM / BACK / MIC

| Οθόνη | ALARM label | MIC label | BACK label |
|-------|-------------|-----------|------------|
| PRYMARY | `obj23` | `obj25` | `obj24` (MENU) |
| MENU | `obj45` | `obj46` | `obj47` |
| HEAT | `obj58` | `obj59` | `obj60` |
| BOILER | `obj62` | `obj63` | `obj64` |
| LIGHT | `obj66` | `obj67` | `obj68` |
| BLINDS | `obj70` | `obj71` | `obj72` |

Εικονίδια alarm: `alarm`, `alarm_1` … `alarm_5`  
Εικονίδια mic: `mic`, `mic_1` … `mic_5`  
Κουμπιά hit: `button_alarm*`, `button_mic*`, `button_menu_*` (BACK)

**Alarm:** `bg_img_recolor` κόκκινο.  
**MIC:** ίδιο pattern · toggle στο runtime.

## MENU hub (κουμπιά EEZ)

| Widget | Προορισμός |
|--------|------------|
| `button_________` | HEAT |
| `button_______` | BOILER |
| `button_____` | LIGHT |
| `button___________` | BLINDS |

## Setpoint

| Widget | Σκοπός |
|--------|--------|
| `obj7` | `lv_bar` 10–30 °C |
| `button_plus` / `button_minus` | Αλλαγή setpoint |
