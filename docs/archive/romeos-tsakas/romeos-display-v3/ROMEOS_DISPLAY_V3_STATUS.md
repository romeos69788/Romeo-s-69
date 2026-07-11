# Romeos display v3 — σημείο συνέχειας

Έλεγχος όταν επιστρέψεις: άνοιγε **μόνο** το `romeos-display-v3` στο PlatformIO/Cursor. Το **`romeos-display-v2` είναι κλειδωμένο** (μην το επεξεργάζεσαι εκτός αν το ζητήσεις ρητά).

---

## Υλικό & COM

| Συσκευή | Σειριακή (στο δικό σου PC) | Σημείωση |
|--------|----------------------------|----------|
| Οθόνη Viewe (ESP32-S3) | **COM3** | Flash / monitor **από αυτό το project** |
| Μητρική | **COM7** | Άλλο firmware (motherboard-fw) — **όχι** flash οθόνης εκεί |

Το `platformio.ini` έχει `upload_port` / `monitor_port` = COM3. Αν αλλάξει USB, `pio device list` και ενημέρωσε τις δύο γραμμές.

---

## Τι είναι το project

- Firmware για την **Viewe** με SquareLine export **`ui_THERMOSTAT_3`** (16-bit LVGL, `LV_COLOR_16_SWAP=0`).
- Κώδικας εφαρμογής: **`src/main_v3.cpp`** (το `main.cpp` αποκλείεται από `build_src_filter`).
- UI αρχεία: **`src/squareline/`** (αντίγραφο από `DISPLAY_A` όταν γίνεται re-export).

### Νέο export από SquareLine

1. Εξαγωγή στο φάκελο **`DISPLAY_A/`** (ή όπου το έχεις).
2. Αντέγραψε όλα τα περιεχόμενα στο **`src/squareline/`** (overwrite).
3. Αν το `ui.h` ξαναβάλει μόνο `lvgl/lvgl.h`, κράτα το patch με `__has_include("lvgl.h")` όπως τώρα (ή ισοδύναμο).
4. Το **`src/squareline/lv_i18n.h`** είναι stub — κράτα το αν το SquareLine ζητά `lv_i18n.h`.

---

## Build & flash

```bash
cd "D:\Romeos Tsakas\romeos-display-v3"
pio run -e BOARD_VIEWE_UEDX80480070E_WB_A
pio run -e BOARD_VIEWE_UEDX80480070E_WB_A -t upload
pio device monitor -e BOARD_VIEWE_UEDX80480070E_WB_A
```

Custom board JSON: **`boards/`** (αντιγραμμένα από v2). Shared: **`../shared/`** (π.χ. `romeos_wifi_nvs.cpp`, headers link).

---

## Λειτουργίες που δουλεύουν (τελευταία γνωστή κατάσταση)

- Ρολόι / ημερομηνία (NTP + Preferences namespace **`v3clk`**).
- Alarm demo + buzzer GPIO **17** (όπου ορίζεται).
- Θερμοκρασία χώρου από **SHT** (I2C touch bus 19/20 — ίδιο με v2).
- Πλοήγηση **HEATING / LIGHS / HEAT_PUMP / BLINDS** → placeholder οθόνες + **BACK**.
- Έλεγχος ότι τρέχει v3: στο **Serial** (115200) εμφανίζεται `*** ROMEOS-DISPLAY-V3 ... ***` στο boot (δεν υπάρχει πλέον σήμα στην οθόνη).

---

## Setpoint (+/−) & τόξο & UDP

- Βήμα setpoint: **0,5 °C** (`k_setpoint_step_c`). Στρογγύλεμα στο μισό βαθμό: `snap_setpoint_half_deg()`.
- **`ui_Arc1`**: `lv_arc_set_range(10, 70)` → τιμή arc = **βαθμοί × 2** (10 = 5,0 °C … 70 = 35,0 °C).
- **Holdoff μετά τοπικό +/-:** **`k_mb_setpoint_holdoff_after_local_ms` = 60000 ms** (1 λεπτό): μέσα σε αυτό το παράθυρο δεν εφαρμόζεται το `setpoint_c_x10` από UDP πάνω στο UI. Στέλνεται αμέσως ενημέρωση στη μητρική με `romeos_display_link_notify_mb_setpoint_c_x10` + `romeos_display_link_on_setpoint_ui_changed`.
- **`on_mb_data_for_link`:** `refresh_ui()` **μόνο** όταν το setpoint από το πακέτο **άλλαξε πραγματικά** το `g_setpoint_c` (αποφυγή τρεμοπαίγματος κάθε ~400 ms).
- **+/− στο firmware:** Μετά το `ui_init()` τα SquareLine `lv_btn` **αντικαθίστανται** από διαφανή `lv_obj` (`v3_replace_step_buttons_with_plain_touch_pads`) — δες **`ROMEOS_DISPLAY_V3_UI_SESSION.md`**. Μετά το strip: `lv_obj_move_foreground(ui_PLUS/MINUS)`.

Σταθερές στο **`main_v3.cpp`**: `k_setpoint_step_c`, `k_mb_setpoint_holdoff_after_local_ms`, όρια `k_setpoint_min_c` / `k_setpoint_max_c`.

**Wi‑Fi (σύντομα):** custom arc/dot, nudges align, ελαφρύ update labels μόνο σε connectivity change — λεπτομέρειες στο **`ROMEOS_DISPLAY_V3_UI_SESSION.md`**.

---

## Προς συνέχιση / ιδέες

- Labels **εξωτερικής θερμοκρασίας / υγρασίας** (π.χ. 18–24 στο export): ακόμα όχι πλήρης σύνδεση με `romeos_mb_to_display_v1_t` (πεδία `outdoor_c_x10`, κ.λπ.) — μόνο setpoint από MB μετά το holdoff.
- Αν μετά το holdoff το setpoint «τραβάει» πάλι λάθος, έλεγχος **motherboard-fw** ότι ενημερώνει και ξαναστέλνει το σωστό `setpoint_c_x10` στο UDP.
- Προαιρετικά: μικρότερο holdoff (τώρα 60 s) ή έξυπνο merge (μόνο αν κοντά στην τοπική τιμή).

---

## Αρχεία-κλειδιά

| Αρχείο | Ρόλος |
|--------|--------|
| `platformio.ini` | Env Viewe, COM, libs, `build_src_filter` |
| `src/main_v3.cpp` | Λογική UI, setpoint, alarm, placeholders, UDP callback, Wi‑Fi widgets, αντικατάσταση +/- |
| `ROMEOS_DISPLAY_V3_UI_SESSION.md` | Τι κάναμε στο UI/LVGL (για μετάβαση σε v4 export) |
| `src/squareline/` | SquareLine generated |
| `src/lvgl_v8_port.*`, `lv_conf.h`, `esp_panel_*`, `board_override/` | Οδήγηση πίνακα |
| `src/romeos_display_link.*`, `romeos_ntp.*`, `romeos_room_*` | Ίδια βάση με v2 |

---

*Τελευταία ενημέρωση: ευθυγράμμιση με holdoff 60 s, MB `refresh_ui` μόνο σε αλλαγή setpoint, σημειώσεις UI στο `ROMEOS_DISPLAY_V3_UI_SESSION.md`.*
