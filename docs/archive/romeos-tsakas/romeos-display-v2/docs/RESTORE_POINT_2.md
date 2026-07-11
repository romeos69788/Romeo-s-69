# Restore Point 2 — v2 UI baseline (καφεδεχόμενο / rollback)

**Ημερομηνία:** 2026-05-01 (βραδινό checkpoint)  
**Project:** `romeos-display-v2`  
**Board env:** `BOARD_VIEWE_UEDX80480070E_WB_A`

Σκοπός: **ένα σημείο αναφοράς** ώστε αύριο (ή μετά) να μπορείτε να γυρίσετε πίσω **αντίγραφα συγκεκριμένων αρχείων** χωρίς να ξανατρέχετε ολόκληρες διαδικασίες — ιδίως για τις αλλαγές γύρω από το **πάτημα / focus / theme** στην οθόνη thermostat.

---

## Τι «κλειδώνει» αυτό το restore point

- Firmware entry: **`src/main_clean.cpp`** (το `src/main.cpp` **δεν** μεταγλωττίζεται στο v2 — `platformio.ini` `build_src_filter`).
- SquareLine UI: **`src/squareline/screens/ui_THERMOSTAT_1.c`** / **`.h`**, **`src/squareline/ui.c`** / **`ui.h`**, helpers, fonts, images όπως στο `src/squareline/`.
- LVGL config: **`src/lv_conf.h`** (π.χ. `LV_THEME_DEFAULT_GROW`, `LV_THEME_DEFAULT_TRANSITION_TIME`).
- On-screen marker: **`CLEAN-V2 BASIC+CF n=…`** + ημερομηνία ώρας build (κάτω δεξιά) — ώστε να επιβεβαιώνεται ότι έχει ανέβει το σωστό .bin.
- Θέμα εκκίνησης UI: στο **`src/squareline/ui.c`** χρησιμοποιείται **`lv_theme_basic_init`** όταν `LV_USE_THEME_BASIC` (αποφυγή default-theme pressed/grow/filter στα `lv_btn`).
- Στο **`main_clean.cpp`**: `strip_thermostat_button_pressed_focus_visuals`, `lv_obj_remove_style` για pressed/focus slots, **`LV_OBJ_FLAG_CLICK_FOCUSABLE` off**, αφαίρεση κουμπιών από default **`lv_group`**, ίδιο strip για **`ui_Label1` / `ui_Label2`**, `LV_EVENT_SCREEN_LOADED` για ξανά-strip.

Προηγούμενο restore: [`RESTORE_POINT_1.md`](./RESTORE_POINT_1.md) (2026-04-28).

---

## Πώς να επαναφέρετε (rollback) γρήγορα

1. **Αντίγραφο ασφαλείας:** κρατήστε αντίγραφο του φακέλου `romeos-display-v2/` (ή zip) **πριν** μεγάλες αλλαγές αύριο.
2. **Επιλεκτική επαναφορά αρχείων:** επαναφέρετε τις **καταστάσεις** των παρακάτω αρχείων από το αντίγραφο αυτού του checkpoint (ή από zip που θα φτιάξετε απόρριζα από εδώ):
   - `src/main_clean.cpp`
   - `src/squareline/ui.c`
   - `src/lv_conf.h`
   - (αν ξαναγίνει export SquareLine) `src/squareline/screens/ui_THERMOSTAT_1.c` / `.h` και σχετικά `ui.*` — μόνο αν το πειράξετε εσείς.
3. **`pio run`** και ξανά **Upload** στο **COM της Viewe** (όχι μητρικής).
4. Αν χρησιμοποιείτε git με commits: `git tag restore-point-2-2026-05-01` πάνω στο commit που αντιστοιχεί σε αυτή την κατάσταση, ώστε `git checkout restore-point-2-2026-05-01 -- <αρχεία>`.

---

## Σύντομος χάρτης φακέλων `romeos-display-v2`

| Διαδρομή | Ρόλος |
|----------|--------|
| `platformio.ini` | Env board, `build_src_filter` (+ κοινά `shared` όπου ορισμένο), flags Wi‑Fi / αισθητήρα. |
| `src/main_clean.cpp` | **Κύριο firmware v2:** thermostat, ρολόι, arcs, Wi‑Fi widgets, strip/focus, `setup`/`loop`. |
| `src/main.cpp` | Αποκλεισμένο από build v2 — **μην** το μπερδεύετε ως entry. |
| `src/lvgl_v8_port.cpp` / `.h` | LVGL 8 + LCD + touch (GT911 κ.λπ.). |
| `src/lv_conf.h` | Ρυθμίσεις LVGL (χρώμα, themes, features). |
| `src/squareline/` | Export SquareLine: `ui.c`, `screens/ui_THERMOSTAT_1.*`, `images/`, `fonts/`. |
| `src/romeos_*.cpp` / `.h` | NTP, link προς μητρική (όταν ενεργό), room temp. |
| `src/board_override/` | Overrides timing / board headers Viewe. |
| `docs/` | Handoff, Wi‑Fi v2, **RESTORE_POINT_*.md**, σημειώσεις. |
| `scripts/` | Βοηθητικά (π.χ. `gen_day_phase_assets.py`). |

---

## Σημείωση για αύριο

Αν το οπτικό θέμα στο πάτημα **εξακολουθεί** μετά το `BASIC+CF`, το επόμενο βήμα είναι διαγνωστικό (επιβεβαίωση COM + φωτογραφία οθόνης + τι ακριβώς «πλαίσιο»), όχι τυφλή επανάληψη theme — αυτό το αρχείο σας δίνει **σταθερό σημείο** για να μην χαθεί η τρέχουσα βάση κώδικα.

Καληνύχτα.
