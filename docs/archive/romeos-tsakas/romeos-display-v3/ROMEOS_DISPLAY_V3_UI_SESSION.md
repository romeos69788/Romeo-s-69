# Σημειώσεις UI / LVGL — τι κάναμε στο v3 (για Version 4)

Αυτό το αρχείο περιγράφει **αποφάσεις και αλλαγές** στο `romeos-display-v3` ώστε στο επόμενο export (v4) να ξέρεις τι να ξαναελέγξεις ή να μεταφέρεις. Κώδικας εφαρμογής: **`src/main_v3.cpp`**. SquareLine: **`src/squareline/`** (οθόνη **`ui_THERMOSTAT_3`**).

---

## 1. Setpoint +/− και `lv_btn` (SquareLine)

**Πρόβλημα:** Το SquareLine εξάγει τα +/- ως **`lv_btn`**. Το default theme του LVGL 8.x προσθέτει σε `LV_STATE_PRESSED` **transition**, **transform (grow)** κ.λπ. — εμφανές κυρίως στο **άφηγμα** του δακτύλου («φουσκώνει» όλο το hit area, frame).

**Δεν δουλεύει ασφαλές workaround:** Το `lv_anim_del(btn, nullptr)` πάνω στο κουμπί **δεν** είναι ασφαλές εδώ: εσωτερικά transitions μπορεί να μην δένουν το `var` με το ίδιο object — προκλήθηκαν **άσπρο/μαύρο/artifacts** στην οθόνη.

**Τελική λύση v3:** Μετά το `ui_init()`, η **`v3_replace_step_buttons_with_plain_touch_pads()`** διαγράφει τα SquareLine `lv_btn` και δημιουργεί **διαφανή `lv_obj`** (ίδιο parent, ίδιο μέγεθος/θέση με το export):

| Widget | Μέγεθος | `lv_obj_align(..., LV_ALIGN_CENTER, x, y)` |
|--------|---------|---------------------------------------------|
| `ui_MINUS` | 100 × 98 | `(-205, -21)` |
| `ui_PLUS` | 100 × 98 | `(194, -22)` |

Τα global pointers **`ui_PLUS` / `ui_MINUS`** (C linkage από το SquareLine) **ξαναδείχνουν** στα νέα αντικείμενα. Events: μόνο **`LV_EVENT_CLICKED`** → `on_plus` / `on_minus`. Μετά το strip στο `setup()` γίνεται ακόμα **`lv_obj_move_foreground`** ώστε τα pads να μείνουν πάνω από το τόξο όπου χρειάζεται.

**Για Version 4:** Αν αλλάξουν ονόματα αντικειμένων ή **align/size** στο SquareLine, ενημέρωσε τις σταθερές μέσα στην `v3_make_step_touch_pad` / `v3_replace_step_buttons_with_plain_touch_pads`. Εναλλακτικά, στο SquareLine χρησιμοποίησε από την αρχή **`lv_obj`** αντί για `lv_btn` για τις ζώνες +/- και απόφυγε όλη αυτή την αντικατάσταση στο firmware.

---

## 2. `strip_thermostat_button_pressed_focus_visuals` και +/-

Δεν καλούμε πλέον **`strip_thermostat_button_pressed_focus_visuals()`** από handlers του +/- (παλιά έκανε **redraw όλης** της κάτω μπάρας labels κουμπιών σε κάθε πάτημα). Το strip τρέχει στο boot, μετά το `SCREEN_LOADED` του thermostat, κ.λπ. — όχι στο release των +/-.

---

## 3. Motherboard UDP — setpoint & `refresh_ui`

- **Holdoff μετά τοπικό +/-:** `k_mb_setpoint_holdoff_after_local_ms` = **60000 ms** (1 λεπτό). Μέσα σε αυτό το παράθυρο δεν εφαρμόζεται το `setpoint_c_x10` από το πακέτο MB πάνω στο UI (αποφυγή «σβησίματος» του +0,5 του χρήστη από παλιό πεδίο στη μητρική). Μετά το +/- στέλνονται ακόμα `romeos_display_link_notify_mb_setpoint_c_x10` + `romeos_display_link_on_setpoint_ui_changed` όπως πριν.
- **`on_mb_data_for_link`:** Κλήση **`refresh_ui()` μόνο όταν** το εισερχόμενο setpoint **άλλαξε πραγματικά** το `g_setpoint_c` (και πέρασε το holdoff). Λόγος: πακέτα ~κάθε 400 ms + πλήρης `refresh_ui` προκαλούσαν **τρεμόπαιγμα** μαζί με Wi‑Fi animation.

---

## 4. Wi‑Fi — δείκτες & flicker

- **Αριστερά:** home STA (εικόνα anchor `ui_Image8` όπου υπάρχει). **Δεξιά:** MB UDP (`ui_Image10`).
- Προστέθηκαν **custom arc/dot** widgets, align/nudge constants στο `main_v3.cpp` (π.χ. `k_wifi_nudge_down_px`, `k_wifi_home_nudge_right_px`, `k_wifi_mb_nudge_left_px`). Σημαντικό: **μην** καλείς `remove_style_all` **μετά** το `lv_obj_align_to` πάνω στα ίδια widgets αν χρειάζεσαι σταθερή θέση — είχε σπάσει η θέση / το δεξί εικονίδιο.
- **Αντικρούον flicker κάτω μπάρας:** Στις `v3_apply_wifi_home` / `v3_apply_wifi_mb`, τα **κείμενα** (`ui_Label28`, `ui_Label33`) και **χρώματα** arc ενημερώνονται σε **πρώτο paint** ή **αλλαγή connected/disconnected**. Σε απλή αλλαγή **level** ενημερώνεται κυρίως **opacity** του arc (και skip αν ίδια τιμή) ώστε να μην αναβοσβήνουν συνεχώς τα labels της μπάρας.

---

## 5. Άλλα σημεία SquareLine / LVGL

- **`ui_Label37`:** Κρυφό στο `setup` (ξεχασμένο «OFF» στο κέντρο από export).
- **`ui_Arc1`:** Μετά τη ρύθμιση γωνιών/range, **`lv_obj_move_to_index(ui_Arc1, 0)`** ώστε το τόξο να πάει πίσω από τα ψηφία setpoint.
- **`ui_Arc1`:** `LV_OBJ_FLAG_CLICKABLE` καθαρίζεται ώστε το arc να μην «κλέβει» touches από τα +/-.

---

## Σύνδεση με άλλα αρχεία

- Γενική κατάσταση project & COM: **`ROMEOS_DISPLAY_V3_STATUS.md`**
- Γρήγορη αναφορά: **`QUICKREF_V3.md`**
- Checklist επόμενα: **`TODO_NEXT_V3.md`**

*Τελευταία ενημέρωση: σημειώσεις για μετάβαση σε Version 4 οθόνης.*
